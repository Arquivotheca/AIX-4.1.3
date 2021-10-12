static char sccsid[] = "@(#)29	1.66  src/bos/kernel/vmm/POWER/v_pfend.c, sysvmm, bos411, 9436D411a 9/7/94 11:13:50";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	v_pfend, v_except
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * routines related to processing pager i/o completions. these
 * routines execute as interrupt handlers at VMM level and may
 * NOT page fault.
 */

#include "vmsys.h"
#include <sys/types.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/buf.h>
#include <sys/except.h>
#include <sys/inline.h>
#include <sys/trchkid.h>
#include <sys/syspest.h>
#include "mplock.h"

/*
 * v_pfend(bp)
 *
 * this routine gets control for all i/o completions initiated
 * by the VMM. bp is the address of the buf structure which
 * is associated with the i/o request for one page.
 *
 * NORMAL (no error) i/o completion processing is as follows:
 *
 * (1) processes waiting for the i/o completion are made ready.
 *     the processes may be (a) on the page frame wait list
 *     (b) for persistent and client segments waiting for the
 *     i/o level that is attained by this i/o complete (c) if
 *     this is a pageout , waiting for a free page frame.
 *
 * (2) if discard is set, the page frame is removed from the
 *     hash list and segment list and put at the head of the
 *     free list. if in addition the segment is in iodelete
 *     state the scb is freed if this is the last page frame 
 *     of the segment.	
 *
 * (3) pageouts: the fblru bit is relevant.
 *
 *     if the fblru bit is not set the page is made addressable
 *     at its normal virtual address.
 *
 *     if the fblru bit is set the page is put on the free
 *     list. pf_numpout is decremented and waitors for a
 *     free frame are made ready.
 *
 * (4) for pageins, the modified bit is cleared.
 *
 * (5) for both pagein and pageouts the overlays (unions) in
 *     the hardware pft non-fifo and waitlist are cleared.
 *     also if the page is not put on the free list, the fields
 *     which the freelist pointers overlay (pincount and logage)
 *     in the software pft  are cleared.
 *
 * (6) hidden states are handled as follows. if the page frame
 *     is NOT hidden, the page is given its normal virtual address.
 *     otherwise the page retains its io virtual virtual address.
 *
 *  NON-NORMAL (i/o error) processing is as follows:
 *  follows.
 *
 * (1) for pagein page is released and any waitors for the
 *     page are sent exception with the error value.
 *
 * (2) for pageout the page is kept unless it was discarded.
 *     ALL processes waiting for page-outs to complete 
 *     on the segment (i.e. callers of v_iowait) are made ready
 *     and sent an exception with the error value. waitors on
 *     the page frame are NOT sent an exception.
 *
 *  RETURN VALUES - none
 */

void
v_pfend(bp)
struct buf *bp; /* pointer to buf struct */
{

        struct proc *pp;
        int srsave,nfr,sid,sidx,error,sidio,pnoio,pdtx;
	int bcount, compact, next;

        /* make vmmdseg addressable at normal place
         */
        srsave = chgsr(VMMSR,vmker.vmmsrval);

        /* get real page frame and error status from buf
         * nfr overlays hash link which is not used by VMM.
         * put buf struct back on free list.
         */
        nfr = (int) bp->b_forw;
        error = (bp->b_flags & B_ERROR) ? bp->b_error : 0;
	compact = (bp->b_flags & B_COMPACTED) ? 1 : 0;
	bcount = bp->b_bcount;

#ifdef _POWER_MP
	bp->b_flags |= B_DONE; /* due to funneling ? Cf iodone_offlevel */
#endif /* _POWER_MP */

	/* spin lock on the vmm_mpsafe_lock since
	 * we cannot block at ih.
	 */
	VMM_MPSAFE_SLOCK();

	PDT_MPLOCK();

        pdtx = pft_devid(nfr);
        bp->av_forw = pdt_bufstr(pdtx);
        pdt_bufstr(pdtx) = bp;

	/* wakeup waitors for a free buf struct.
	 */
	while (pf_bufwait)
	{
		v_ready(&pf_bufwait);
	}

	/* update i/o completion counter
	 */
        vmpf_numiodone++;

	/* scan gather list and update i/o counter
	 */
	for (next = nfr; next != 0; next = pft_nextio(next))
	{
        	/* update i/o counter
         	 */
        	pdt_iocnt(pdtx)--;
	}

	/* wakeup waitors for all i/o to complete on a device.
	 */
        if (pf_devwait && pdt_iocnt(pdtx) == 0)
        {
                while(pf_devwait != NULL)
                {
                        v_ready(&pf_devwait);
                }
        }

#ifndef _VMM_MP_SAFE
        /* re-drive i/o if necessary.
         */
        if(pdt_iotail(pdtx) >= 0)
                v_pdtsio(pdtx, V_NOKEEP);
	else
		PDT_MPUNLOCK();
#endif /* _VMM_MP_SAFE */

	/* upcompact compressed file frames in gather list
	 * (the frames are in I/O address space)
	 */
	if (compact)
		v_uncompact(nfr, pdtx, bcount, error);

	/* scan gather list and scatter frames
	 */
        sid = pft_ssid(nfr);
        sidx = STOI(sid);

	SCB_MPLOCK(sidx);

	for ( ; nfr != 0; nfr = next)
	{
		/* remember next frame on the gather list
		 * (e.g., frame may be released by endpagein()/endpageout().)
		 */
		next = pft_nextio(nfr);
		pft_nextio(nfr) = 0;

        	/* move the page frame from i/o part of scb list
         	 * this clears non-fifo field and updates scb_iolev.
         	 */

        	v_mfioscb(sidx,nfr);

		/* call appropriate routine 
	 	 */
		if (pft_pagein(nfr))
		{
			endpagein(sid,sidx,nfr,error);
			vmpf_pageins += 1;
			if (pdt_type(pdtx) == D_PAGING)
				vmpf_pgspgins += 1;
		}
		else
		{
			endpageout(sid,sidx,nfr,error);
			vmpf_pageouts += 1;
			if (pdt_type(pdtx) == D_PAGING)
				vmpf_pgspgouts += 1;
		}
	}

	SCB_MPUNLOCK(sidx);

	/* wake up waitors for a free frame.
	 * pf_freewake is set when we couldn't find enough frames to
	 * steal in v_fblru(), so we wake everyone up and try again.
	 * In mp, pf_freewake does not need to be serialized 
	 * (and pf_freewait is protected by the proc_int_lock).
	 */
	if (pf_freewake)
	{
		pf_freewake = 0;
		while(pf_freewait != NULL)
		{
			v_ready(&pf_freewait);
		}
	}

#ifdef _VMM_MP_SAFE
        /* re-drive i/o if necessary
	 * NOTE THAT THE vmm_mpsafe_lock MAY BE RELEASED IN v_pdtsio.
         */
        if(pdt_iotail(pdtx) >= 0)
                v_pdtsio(pdtx,V_NOKEEP);

	/* unlock calls v_pfsio because it is the responsability 
	 * of the vmm_mpsafe_lock holder to do so (Cf locktry in v_pfsio).
	 * we have several cases (1) we did not call v_pdtsio, thus we still
	 * hold the lock and lock try is successful since it allows recursion.
	 * (2) we have called v_pdtsio and thus we may have released the lock
	 * but not start the pending i/o's. if another thread has the lock
	 * it will start the i/o's at unlock time, and thus if lock try fails
	 * we can just return. Otherwise, we grab the lock and release it to
	 * call v_pfsio.
	 */
	if (VMM_MPSAFE_LOCKTRY())
		VMM_MPSAFE_UNLOCK();

#endif /* _VMM_MP_SAFE */

        /* restore sreg and return
         */
        (void)chgsr(VMMSR,srsave);
        return ;
}

/*
 * endpagein(sid,sidx,nfr,error)
 *
 * page in completion processing for page specified. see
 * description of v_pfend for what is done.
 *
 * RETURN VALUES - none
 */

static
endpagein(sid,sidx,nfr,error)
int     sid;    /* segment id */
int     sidx;   /* index in scb table */
int     nfr;    /* page frame number */
int     error;  /* error status */
{
	struct thread *tp;
	int sidsync, pnosync;
        int fragsize, nfrags, off;

	SCB_LOCKHOLDER(sidx);

	/* performance trace hook.
	 */
	TRCHKL5T(HKWD_VMM_PFENDIN,sid,scb_sibits(sidx),pft_pagex(nfr),
		 nfr,error);

	/* for compression, expand the page or for fragments 
	 * zero the uncovered portion of the page, if the page 
	 * will be retained.  This will leave the page modified,
	 * which results in an unnecessary cache flush for fragments
	 * on _POWER_RS platforms in the IOSYNC operation below.
	 * We optimize for _POWER_PC by avoiding the extra pathlength
	 * involved in resetting the modified status here since this is
	 * done in the IOSYNC operation. (note : the cache flush is
	 * necessary for compression).
	 */
	if (pft_nfrags(nfr) && !pft_discard(nfr) && !error)
	{
		if (scb_compress(sidx))
		{
			error = v_expandit(nfr);
			pft_rdonly(nfr) = 1;
		}
		else
		{
			fragsize = PSIZE / pdt_fperpage(pft_devid(nfr));
			nfrags = pdt_fperpage(pft_devid(nfr)) - pft_nfrags(nfr);
			off = fragsize*nfrags;
			ZEROFRAG(IOSID(pft_ssid(nfr)),pft_spage(nfr),off);
		}
	}

        pft_pagein(nfr) = 0;

        /* Wakeup all waitors. If error, send them an exception before
	 * they are made runnable.  The loop also clears pft_waitlist.
         */
	while(pft_waitlist(nfr) != NULL)
	{
		tp = pft_waitlist(nfr)->t_next;
		if (error)
			v_except(tp,error,nfr,0);
		v_ready(&pft_waitlist(nfr));
	}

	/* release page frame if there was an error or if it
	 * was discarded. if it was discarded, free the scb
	 * if this is the last page frame of the segment in i/o
	 * delete state.
	 */
	if (pft_discard(nfr) || error)
	{
		v_relframe(sidx,nfr);
		if (scb_iodelete(sidx))
		{
			if (scb_sidlist(sidx) < 0)
			{
				ALLOC_MPLOCK();
				v_freescb(sidx,0);
				ALLOC_MPUNLOCK();
			}
		}
		
		return 0;
	}

        /* init overlay fields.
	 */
	pft_logage(nfr) = 0;

	/* Set protection key to read-only if necessary.
	 */
        if (pft_rdonly(nfr))
        {
                pft_key(nfr) = RDONLY;
                pft_rdonly(nfr) = 0;
        }

	/* Insert page at normal virtual address unless
	 * the page is hidden for page-ahead (use I/O address).
	 * Use IOSYNC option to ensure that instruction cache is sync'd.
         */
	if (pft_pgahead(nfr))
        {
		sidsync = IOSID(sid);
		pnosync = pft_spage(nfr);
        }
	else
	{
		sidsync = sid;
		pnosync = pft_spage(nfr);
	}
	P_ENTER(IOSYNC,sidsync,pnosync,nfr,pft_key(nfr),pft_wimg(nfr));

        /* set pft_inuse state for page.
         */
        pft_inuse(nfr) = 1;
}

/*
 * endpageout(sid,sidx,nfr,error)
 *
 * page out completion processing. see v_pfend for description
 * of function.
 *
 * RETURN VALUES - none
 */

static
endpageout(sid,sidx,nfr,error)
int     sid;    /* segment id */
int     sidx;   /* index in scb table */
int     nfr;    /* page frame number */
int     error;  /* error status */
{
	int iolevel;
	int sidsync, pnosync, rc;
	struct proc * pp;

	SCB_LOCKHOLDER(sidx);

	/* performance trace hook.
	 */
	TRCHKL5T(HKWD_VMM_PFENDOUT,sid,scb_sibits(sidx),pft_pagex(nfr),
		 nfr,error);

        /* clear the modified bit (if there was an error we * will set
         * it later because we will keep the page).  For PowerPC, cache
         * management is performed at I/O completion time based on the
         * setting of the h/w modified bit (for _POWER_RS cache management
         * occurs at STARTIO time).  Since I/O is consistent with the data
         * cache we can just clear the modify bit for local I/O and the
         * P_ENTER(IOSYNC) below will just invalidate the i-cache.  For
         * client I/O we must leave the h/w modified bit set so the
         * d-cache gets flushed as well.  We must clear the pft_modbit in
         * either case.
	 */

#ifdef _POWER_PC
	if (__power_pc() && scb_clseg(sidx))
		pft_modbit(nfr) = 0;
	else
#endif
		CLRMOD(nfr);

	/* adjust pageout count.
	 */
	if (pft_fblru(nfr) == 0)
		scb_npopages(sidx) += -1;

        /* check segment iolevel.
         */
	v_iolevel(sidx,nfr,error);

	/* for logs, set I/O write error indicator if an error occurred.
	 */
	if (error && scb_logseg(sidx))
		scb_eio(sidx) = 1;

        /* wakeup all waitors. don't send exception even if error.
         * the loop also clears pft_waitlist.
         */
	while(pft_waitlist(nfr) != NULL)
	{
		v_ready(&pft_waitlist(nfr));
	}

	/* release page frame if it was discarded. free
	 * the scb if this is last page of the segment 
	 * and the segment is in i/o delete state. wakeup
	 * waitors for a free frame.
	 */
	if (pft_discard(nfr))
	{
		if (pft_fblru(nfr))
		{
			/* NOTE that pf_numpout is not protected by the
			 * vmker lock and thus its update must be done
			 * before calling v_relframe to guarantee that
			 * a wake up will occur if in v_spaceok we v_wait
			 * and don't wake up the lru due to a big pf_numpout.
			 * The alternative of decrementing pf_numpout under the
			 * vmker lock at the right place (in v_insfree) is not
			 * easily feasible.
			 */
			FETCH_AND_ADD(pf_numpout, -1);

			if (!scb_compseg(sidx))
				pf_numpermio += -1;
			if (scb_clseg(sidx))
				FETCH_AND_ADD(pf_numremote, -1);
		}

		pft_pageout(nfr) = 0;
		v_relframe(sidx,nfr);
		if (scb_iodelete(sidx))
		{
			if (scb_sidlist(sidx) < 0)
			{
				ALLOC_MPLOCK();
				v_freescb(sidx,0);
				ALLOC_MPUNLOCK();
			}
                }
		return 0;
	}

	/* re-expand page if compression. this will set modbit,
	 * so the P_ENTER below will flush cache as well as clear
	 * the modbit. also set the storage protect to read-only.
	 */
	if (pft_nfrags(nfr) && scb_compress(sidx))
	{
		rc = v_expandit(nfr);
		assert(rc == 0);
		pft_key(nfr) = RDONLY;
	}

	/* now safe to clear pageout state (decompression complete).
	 */
        pft_pageout(nfr) = 0;

       	/* init overlay fields.
       	 */
	pft_logage(nfr) = 0;

        /* if fblru move frame to free list unless
	 * there was an error. we keep the frame otherwise.
	 * wakeup waitors for a free page frame in either case.
         */
        if (pft_fblru(nfr))
        {
                pft_fblru(nfr) = 0;
		/* NOTE that pf_numpout is not protected by the
		 * vmker lock and thus its update must be done
		 * before calling v_relframe to guarantee that
		 * a wake up will occur if in v_spaceok we v_wait
		 * and don't wake up the lru due to a big pf_numpout.
		 * The alternative of decrementing pf_numpout under the
		 * vmker lock at the right place (in v_insfree) is not
		 * easily feasible.
		 */
		FETCH_AND_ADD(pf_numpout, -1);

		if (!scb_compseg(sidx))
			pf_numpermio += -1;
		if (scb_clseg(sidx))
			FETCH_AND_ADD(pf_numremote, -1);

		if (!error)
		{
			v_relframe(sidx,nfr);
			return(0);
		}
		else
		{
			/* normally threads are readied when free frames
			 * are made available but here this may be the last
			 * pageout and any waitor must be unconditionnaly
			 * readied to avoid a possible hang.
			 */
			VMKER_MPLOCK();

                	while(pf_freewait != NULL)
                	{
                        	v_ready(&pf_freewait);
			}

			VMKER_MPUNLOCK();
		}
        }

	/* Insert page at normal virtual address unless
	 * the page is hidden for page-ahead.
	 * the IOSYNC option flushes both caches if the
	 * modbit is set. it also clears the modbit.
         */
	if (pft_pgahead(nfr))
        {
		sidsync = IOSID(sid);
		pnosync = pft_spage(nfr);
        }
	else
	{
		sidsync = sid;
		pnosync = pft_spage(nfr);
	}
	P_ENTER(IOSYNC,sidsync,pnosync,nfr,pft_key(nfr),pft_wimg(nfr));

	/* if there was an error , set the modbit.
	 * we keep frame if there was an error.
	 */
	if (error) SETMOD(nfr);

        /* mark frame inuse.
         */
       	pft_inuse(nfr) = 1;

	return 0;

}

/*
 * v_iolevel(sidx,nfr,error)
 *
 * check segment i/o completion level for non-working storage
 * segments.  readying processes waiting on the current iolevel.
 *
 * RETURN VALUES - none
 */

static
v_iolevel(sidx,nfr,error)
uint sidx;
int nfr;
int error;
{
	int iolevel;
	struct thread *t, *nextt, *lastt;

	SCB_LOCKHOLDER(sidx);

	/* just return if working storage or no iolevel waitors.
	 */
        if (scb_wseg(sidx) || scb_iowait(sidx) == NULL)
		return;

	/* if error, send an exception to each waiting thread,
	 * and then make it runnable.
	 */
	if (error)
	{
                while (scb_iowait(sidx) != NULL)
		{
			t = scb_iowait(sidx)->t_next;
			v_except(t,error,nfr,1);
			v_ready(&scb_iowait(sidx));
		}

		return;
	}

	/* wakeup iolevel waitors if iolevel has been reached.
	 */
	iolevel = scb_iolev(sidx);
	lastt = scb_iowait(sidx);
	for (t = lastt->t_next; ; t = nextt)
	{
		/* get next before readying the current
		 * thread.
		 */
		nextt = t->t_next;

		/* ready the thread if the iolevel has been
		 * reached for the segment.
		 */
		if ((t->t_polevel - iolevel) <= 0)
			v_readyt(&scb_iowait(sidx),t);

		/* finished if all waitors have been examined.
		 */
		if (t == lastt)
			break;
	}
	return;
}

/* 
 * v_except(pp,error,nfr,pageout)
 *
 * send an exception to the process pointed to by pp.
 */
v_except(tp,error,nfr,pageout)
struct thread *tp;
int error;
int nfr;
int pageout;
{

	int srsave, usermode, sysaddr, sidx, sid;
	struct mstsave *mst;
	uint vaddr, srval, pno;

	/* get addressability to mst of the process that 
	 * gets exception.
	 */
	srsave = chgsr(TEMPSR,tp->t_procp->p_adspace);
	mst = (struct mstsave *)( (TEMPSR << L2SSIZE) +
		((uint)&tp->t_uthreadp->ut_save & SOFFSET) );

	/* get sidx.
	 */
	sidx = STOI(pft_ssid(nfr));

	SCB_LOCKHOLDER(sidx);

	/* if exception occurred on a pageout, construct the
	 * mst except info for synchronous pageout exception.
	 */
	if (pageout)
	{
		sid = ITOS(sidx,pft_pagex(nfr));
		mst->except[EVADDR] = pft_pagex(nfr) << L2PSIZE;
		mst->except[ESRVAL] = SRVAL(sid,0,0);
		mst->except[EISR] = 0;
		mst->except[EORGVADDR] = 0;
		mst->except[EPFTYPE] = EXCEPT_DSI;
	}

	/* determine if usermode and whether exception on
	 * system address.
	 */
	vaddr = mst->except[EVADDR];
	srval = mst->except[ESRVAL];
	usermode = v_isuser(mst);
	pno = (vaddr & SOFFSET) >> L2PSIZE;
	sysaddr = v_issystem(sidx,pno);

	/* set exception value to EIO if not client segment
	 * or if outside errno range.
	 */
	if (!scb_clseg(sidx))
	{
		error = EIO;
	}
	else
	{
		if (error == EXCEPT_EOF)
		{
			/* an mmap'er stored beyond end-of-file.
			 * send an exception to this process only
			 * if it's an mmap'er (if it's not, it will
			 * just re-fault and cause another pagein).
			 */
			if (!scb_mseg(STOI(SRTOSID(srval))))
			{
				(void)chgsr(TEMPSR,srsave);
				return(0);
			}
		}
		else if (error > EXCEPT_ERRNO)
		{
			error = EIO;
		}
	}

	/*  trace the exception.
	 */
	TRCHKL5T(HKWD_VMM_EXCEPT,scb_sibits(sidx),srval,vaddr,error,tp->t_tid);

	/* if kernel mode and sysaddr log and halt; otherwise
	 * call v_vmexcept to process the exception.
	 */
	if (!usermode && sysaddr)
	{
		v_loghalt(error,mst);
	}
	else
	{
		v_vmexcept(error,mst,tp);
	}

	/* restore segment register
	 */
	(void)chgsr(TEMPSR,srsave);
	return 0;

}


/* 
 * v_zerofrag(sid,pno,off)
 *
 * zero the portion of the page which is not covered by 
 * disk resources.  upon exit, the page is modified and
 * may be in the cache and/or tlb.
 * 
 */

v_zerofrag(sid,pno,off)
int sid,pno,off;
{
	int len;
	uint srsave, vaddr;

	assert(scb_pseg(STOI(sid)));

	SCB_LOCKHOLDER(STOI(sid));

	/* get addressability to the page.
	 */
	srsave = chgsr(TEMPSR, SRVAL(sid,0,0));

	/* zero the appropriate portion of the page.
	 */
	vaddr = (TEMPSR << L2SSIZE) + (pno << L2PSIZE) + off;
	bzero(vaddr,PSIZE-off);

	/* restore segment register
	 */
	(void)chgsr(TEMPSR,srsave);
}


/*
 *	v_uncompact(nfr,pdtx,count,error)
 *
 * upcompact compacted contiguous frames in gather list
 * for compressed segment
 *
 * see v_gather() for compacted gather list.
 */
static
v_uncompact(nfr,pdtx,count,error)
int nfr;
int pdtx;
int count;
int error; 
{
	int fperpage, fragsize;
	int last, nfrags, nbytes;
	uint srcs, srce, dsts, dste;
	uint srsave;

	if (pft_pagein(nfr) && error)
		return;

	fperpage = pdt_fperpage(pdtx);
	fragsize = PSIZE/fperpage;

	/* get addressability to the segment.
	 */
	srsave = chgsr(TEMPSR, SRVAL(IOSID(pft_ssid(nfr)),0,0));

	srce = (TEMPSR << L2SSIZE) + (pft_spage(nfr) << L2PSIZE) + count;

	for (last = pft_nextio(nfr); last > 0; last = pft_nextio(last))
	{
		nfrags = fperpage - pft_nfrags(last);
		nbytes = fragsize * nfrags;

		srcs = srce - nbytes;
		dsts = (TEMPSR << L2SSIZE) + (pft_spage(last) << L2PSIZE);

		/* check for overlap of source and destination
		 */
		/* move nfrags of the page at a time */
		if (srce <= dsts)
		{
			if (!pft_discard(last)) 
				bcopy(srcs,dsts,nbytes);
			srce = srcs;
		}	
		/* move one fragment from tail of the page at a time */
		else if (srcs != dsts)
		{
			dste = dsts + nbytes;
			for ( ; nfrags > 0; nfrags--)
			{
				srce = srce - fragsize;
				dste = dste - fragsize;
				if (!pft_discard(last)) 
					bcopy(srce,dste,fragsize);
			}
		}
		/* non-compacted pages */
		else
			break;
	}
	
	/* restore segment register
	 */
	ldsr(TEMPSR,srsave);

	return;
}
