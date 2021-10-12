static char sccsid[] = "@(#)02	1.64  src/bos/kernel/vmm/POWER/v_disksubs.c, sysvmm, bos411, 9434B411a 8/19/94 15:40:23";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	
 *	 v_pfsio, v_pdtqiox, v_pdtsio, v_getcio, v_requeio
 *
 * ORIGINS: 27 83
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

#include "vmsys.h"
#include <sys/errno.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/syspest.h>
#include <sys/low.h>
#include <sys/mstsave.h>
#include <sys/trchkid.h>
#include <sys/dasd.h>
#include "mplock.h"

/*
 * v_pfsio()
 *
 * initiates i/o for list of pdts chained from pf_iotail.
 *
 * RETURN VALUES - none
 *
 * NOTE: in mp the calls to v_pfsio() are deferred at vmm_mpsafe_lock
 * 	 unlock time so that the lock can be safely released across
 *	 the strategy routine. The lock is reacquired with a locktry
 *	 because if the lock is held, v_pfsio() will be called again by
 *	 the new lock holder at unlock time.
 *	 ON ENTRY WE HOLD THE vmm_mpsafe_lock,
 *	 BUT WE ALWAYS RETURN WITH THE LOCK RELEASED.
 */
v_pfsio()
{
	int head;
#ifdef _VMM_MP_EFF
	int last, nfr, next;
	struct ppda *ppda_p = PPDA;
#endif

	VMM_MPSAFE_LOCKHOLDER();

	PDT_MPLOCK();

#ifdef _VMM_MP_EFF
	/*
	 * Check if I/O has been intermediately queued to the ppda.
	 */
	if ((last = ppda_p->ppda_qio) > 0)
	{
		/*
		 * Now queue the I/O to the PDTs.
		 */
		for (nfr = pft_nextio(last); ; nfr = next)
		{
			next = pft_nextio(nfr);
			pft_nextio(nfr) = 0;

        		v_pdtqueue(pft_devid(nfr),nfr);

			if (nfr == last)
				break;
		}
		ppda_p->ppda_qio = 0;
	}
#endif /* _VMM_MP_EFF */

	while(pf_iotail >= 0)
	{
		/* remove head of list
		 */
		head = pdt_nextio(pf_iotail);
		if (head == pf_iotail)
		{
			pf_iotail = -1;
		}
		else
		{
			pdt_nextio(pf_iotail) = pdt_nextio(head);
		}
		pdt_nextio(head) = -1; /* takes it off list */

		/* start i/o for frames on pdt = head.
		 * the vmm_mpsafe_lock MAY HAVE BEEN RELEASED on return
		 * from v_pdtsio.
		 * WARNING: We may release and reaquire the pdt lock
		 * in v_pdtsio() if we call the strategy routine.
		 */
		v_pdtsio(head, V_KEEP);
		
		PDT_LOCKHOLDER();

#ifdef _VMM_MP_SAFE
		if (!VMM_MPSAFE_LOCKTRY())
			return;
#endif
	}

#ifdef _VMM_MP_EFF
	/* clear flag indicating that there are I/Os to start
	 */
	ppda_p->ppda_sio = 0;
#endif

	PDT_MPUNLOCK();

	VMM_MPSAFE_SUNLOCK();
}


static int compiotail = -1;
static struct thread ** compwlist = 0;

/*
 * v_pdtqiox(pdtx, nfr, diskok)
 *
 * if diskok is true:
 * adds nfr to the list of page frames requiring i/o to the PDT
 * entry specified. assigns an i/o virtual address for the page.
 * if pdtx is not on  i/o list puts it at tail of list.
 *
 * if diskok is false: 
 * if page is being written and may require re-allocation
 * of disk , adds nfr to the list of pages to be processed by
 * the compression kproc which will compress and re-allocate
 * disk if necessary, and then requeue the i/o. otherwise
 * queues nfr as above.
 */
	
int
v_pdtqiox(pdtx,nfr,diskok)
int	pdtx;	/* index in PDT */
int	nfr;	/* page frame number */
int     diskok; /* 1 if disk allocation is known to be good */
{
	int last,sidx, frags, fperpage;

	/* device should be defined in pdt_table
	 */
	ASSERT(pdt_type(pdtx));

	/* update repaging stats if pagein.
	 */
	if (pft_pagein(nfr))
		chkrepage(nfr);

	/* if pageout and compression and diskok is false
	 * then queue the page for the compression kproc.
	 * the test for fperpage avoids extension memory
	 * for deferred-update files.
	 */
	sidx = STOI(pft_ssid(nfr));

	SCB_LOCKHOLDER(sidx);
	PDT_LOCKHOLDER();

	fperpage = pdt_fperpage(pdtx);
	if (!diskok && scb_compress(sidx) && pft_pageout(nfr) && fperpage > 1)
	{
		last = compiotail;
		compiotail = nfr;
		if (last < 0)
		{
			pft_nextio(nfr) = nfr;
		}
		else
		{
			pft_nextio(nfr) = pft_nextio(last);
			pft_nextio(last) = nfr;
		}

		/* wakeup kproc ?
	 	 */
		if (compwlist)
			v_ready(&compwlist);

		return;
	}

	/* update stats on type of pageouts.
	 */
	if (pft_fblru(nfr))
	{
		FETCH_AND_ADD(pf_numpout, 1);
		if (!scb_compseg(sidx)) pf_numpermio += 1;
		if (scb_clseg(sidx)) FETCH_AND_ADD(pf_numremote, 1);
	}
	else if (pft_pageout(nfr))
	{
		scb_npopages(sidx) += 1;
	}

#ifdef _VMM_MP_EFF
	/*
	 * For MP-eff queue the I/O to the PPDA.  These frames will
	 * be queued later by v_pfsio() to the PDTs.  This is done to
	 * ensure other processors don't start our I/O before it is
	 * all queued which would prevent us from getting maximum
	 * performance for sequential I/O.
	 */
	v_ppdaqueue(nfr);

	/* set flag to indicate that there are I/Os to start
	 * when exiting from the critical section
	 */
	PPDA->ppda_sio = 1;

#else /* _VMM_MP_EFF */

	v_pdtqueue(pdtx,nfr);

#endif /* _VMM_MP_EFF */

	return 0;
}

/*
 * v_pdtqueue(pdtx, nfr)
 *
 * Adds nfr to the list of page frames requiring i/o to the PDT
 * entry specified.  If pdtx is not on i/o list puts it at tail of list.
 */
static
v_pdtqueue(pdtx,nfr)
int	pdtx;	/* index in PDT */
int	nfr;	/* page frame number */
{
	int last;

	PDT_LOCKHOLDER();

        if (pdt_nextio(pdtx) < 0)
	{
        	last = pf_iotail;
		pf_iotail = pdtx;

		if (last < 0)
		{
			pdt_nextio(pdtx) = pdtx;
		}
		else
		{
			pdt_nextio(pdtx) = pdt_nextio(last);
			pdt_nextio(last) = pdtx;
		}
	}

	/* put nfr at tail of list associated with pdtx
	 * increment i/o count field.
	 */
	pdt_iocnt(pdtx) += 1;
	last = pdt_iotail(pdtx);
	pdt_iotail(pdtx) = nfr;

	if (last < 0)
	{
		pft_nextio(nfr) = nfr;
	}
	else
	{
		pft_nextio(nfr) = pft_nextio(last);
		pft_nextio(last) = nfr;
	}
}

#ifdef _VMM_MP_EFF

/*
 * v_ppdaqueue(nfr)
 *
 * Adds nfr to the list of page frames requiring i/o to the PPDA.
 */
static int
v_ppdaqueue(nfr)
int	nfr;	/* page frame number */
{
	int last;

	ASSERT(nfr != 0);

	/* put nfr at tail of list associated with ppda.
	 */
	last = PPDA->ppda_qio;
	PPDA->ppda_qio = nfr;

	if (last == 0)
	{
		pft_nextio(nfr) = nfr;
	}
	else
	{
		pft_nextio(nfr) = pft_nextio(last);
		pft_nextio(last) = nfr;
	}

	return 0;
}

#endif /* _VMM_MP_EFF */

/*
 * v_pdtsio(pdtx,keep)
 *
 * initiate i/o for page frames on list if buf structs are
 * available.
 * in mp, the lock can be released before the call to the strategy
 * AND NOT REACQUIRED.
 */
v_pdtsio(pdtx, keep)
int	pdtx;	/* index of device in PDT */
int	keep;	/* reaquire lock after call to strategy */
{
	void v_pfend();
	int nfr, sidio, pnoio, nsios, backtrack, sidx;
	int fperpage, fragsize, nfrags;
	struct buf *bp, *bpfirst;
	struct mstsave * curcsa;
	extern int ram_open();

	PDT_LOCKHOLDER();

	nsios = 0;
	bpfirst = NULL;
	while (pdt_iotail(pdtx) >= 0 && pdt_bufstr(pdtx))
	{
		/* remove first page frame from circular list
		 */
		nfr = pft_nextio(pdt_iotail(pdtx));
		if(nfr == pdt_iotail(pdtx))
		{
			pdt_iotail(pdtx) = -1;
		}
		else
		{
			pft_nextio(pdt_iotail(pdtx)) = pft_nextio(nfr);
		}

		/* clear nextio field.
		 */
		pft_nextio(nfr) = 0;

		/* remove bufstr from list. remember the first one.
		 */
		nsios += 1;
		bp = pdt_bufstr(pdtx);
		pdt_bufstr(pdtx) = bp->av_forw;
		if (bpfirst == NULL)
			bpfirst = bp;

		/* fill in buf struct.
		 * vmm buffers are MP safe
		 */
#ifdef _POWER_MP
		bp->b_flags = B_MPSAFE | B_MPSAFE_INITIAL |
				((pft_pagein(nfr)) ? B_READ : B_WRITE);
#else /* _POWER_MP */
		bp->b_flags = (pft_pagein(nfr)) ? B_READ : B_WRITE;
#endif /* _POWER_MP */

		/* fill in iodone routine.
		 */
		bp->b_iodone = v_pfend;

		/* performance trace hook - start I/O.
	 	*/
		sidx = STOI(pft_ssid(nfr));
		TRCHKL5T(HKWD_VMM_SIO|bp->b_flags,pft_ssid(nfr),
			scb_sibits(sidx),pft_spage(nfr),nfr,bp);

        	/* enable split op for local reads.  Log and meta data 
		 * reads or writes can be split.
         	 */
        	if ((pft_pagein(nfr) && !scb_clseg(sidx)) ||
             	     scb_jseg(sidx) || scb_logseg(sidx))
                	bp->b_flags |= B_SPLIT;

                /* if this is a pagein the storage key can not be readonly. 
		 * readonly key is set up after i/o is done in v_pfend.
                 */
                if (pft_pagein(nfr))
                {
                        ASSERT(pft_key(nfr) != RDONLY);
                }

		if (pdt_type(pdtx) == D_REMOTE)
		{
			if (pft_store(nfr))
			{
				pft_store(nfr) = 0;
				bp->b_flags |= B_PFSTORE;
			}

			/* tell client strategy routine to fail
			 * if it is beyond end-of-file.
			 */
			if (pft_mmap(nfr))
			{
				pft_mmap(nfr) = 0;
				bp->b_flags |= B_PFEOF;
			}

			bp->b_vp = (struct vnode *)scb_gnptr(sidx);
			bp->b_blkno = (PSIZE/512)*pft_pagex(nfr);
			bp->b_bcount = PSIZE;
		}
		else
		{
			bp->b_dev = pdt_device(pdtx);

			/*
			 * try to gather contiguous pages
			 *
			 * (force single page i/o if
			 * no more pages in pdt i/o queue,
			 * log pages,
			 * ramdisk pages (ramdisk doesn't support anything 
			 * other than 4K request).)
	 		 */
			if (pdt_iotail(pdtx) == -1 ||
			    scb_logseg(sidx) ||
			    devsw[major(pdt_device(pdtx))].d_open == ram_open)
			{
				fperpage = pdt_fperpage(pdtx);
				fragsize = PSIZE / fperpage;
				bp->b_blkno = pft_fraddr(nfr) * (fragsize / 512);

				/* prevent further writes to log
		 		 * once an I/O error occurred on the log.
		 		 */
				if (scb_logseg(sidx) && 
				    scb_eio(sidx) && pft_pageout(nfr))
					bp->b_bcount = 0;
				else
				{
					nfrags = fperpage - pft_nfrags(nfr);
					bp->b_bcount = nfrags * fragsize;
				}
			}
			else
				nfr = v_gather(nfr,bp,pdtx);
		}

		/* fill in xmem descriptor in bufstr for I/O address
		 * for cross-memory (non-global) addressing by device driver.
		 */
		sidio = IOSID(pft_ssid(nfr));
		pnoio = pft_spage(nfr);
		bp->b_xmemd.aspace_id = 0; 
		bp->b_xmemd.subspace_id = SRVAL(sidio,0,0);
		bp->b_un.b_addr = pnoio << L2PSIZE;

		/* we use b_forw field to hold page frame number (v_pfend)
		 */    
		bp->b_forw = (struct buf *)nfr;
	}	

	/* invoke strategy routine if list is non-null.       
	 */
	if (bpfirst != NULL)
	{
		/* we release the vmm_mpsafe_lock (without calling v_pfsio)
		 * not to hold it across the strategy routine.
		 * THE LOCK IS NOT REACQUIRED
		 * and the caller has to deal with that.
		 */
		VMM_MPSAFE_SUNLOCK();
	
		/* do not allow page pfaults in device code
		 */
		curcsa = CSA;
		backtrack = curcsa->backt;
		curcsa->backt = 0; 

		bp->av_forw = NULL;

		/* call strategy routine 
		 * thru devstrat() to funnel if needed
		 * e.g. ram fs.
		 * release the pdt lock across the call.
		 */

		PDT_MPUNLOCK();

		VMM_CHKNOLOCK();

		if (pdt_type(pdtx) == D_REMOTE)
			(*pdt_strategy(pdtx)) (bpfirst);
		else
			devstrat(bpfirst);

		if (keep == V_KEEP)
			PDT_MPLOCK();

		/* restore backtrack ok state.
		 */
		curcsa->backt = backtrack;

		/* update statistics on page requests.
		 */
		vmpf_numsios += nsios;
	}
	else if (keep == V_NOKEEP)
		PDT_MPUNLOCK();
}

/*
 * v_getcio(nmax, array[],wait)
 *
 * removes up to nmax page frames from the compio list
 * and returns in array their page frame numbers and returns
 * the number of them. all of the pages removed from the
 * list have the SAME sid. 
 *
 * if there are no pages on the list and the wait parameter
 * is true, the caller is blocked until pages are put on the
 * list.
 *
 * this is called via vcs_getcio() with back-tracking enabled.
 */

int
v_getcio(nmax, array, wait)
int nmax;
int * array;
int wait;
{

	int nfr, sid, nframes;

	/* make sure we don't fault on array
	 */
	TOUCH(array);
	TOUCH(array + nmax - 1);
	
	PDT_MPLOCK();
	
	nframes = 0;
	while (compiotail >= 0 && nframes < nmax)
	{
		/* nfr is at head of circular list
		 */
		nfr = pft_nextio(compiotail);
		if (nframes == 0)
		{
			sid = pft_ssid(nfr);
		}
		else
		{
			if (sid != pft_ssid(nfr))
				break;
		}
		/* remove nfr from list and put into array
		 */
		if (nfr == compiotail)
		{
			compiotail = - 1;
		}
		else
		{
			pft_nextio(compiotail) = pft_nextio(nfr);
		}
		*(array + nframes) = nfr;
		nframes += 1;
	}
		
	if (nframes == 0 && wait)
	{
		v_wait(&compwlist);
		PDT_MPUNLOCK();
		return VM_WAIT;
	}

	PDT_MPUNLOCK();

	return (nframes == 0 && wait) ? VM_WAIT : nframes;
}

/*
 * v_requeio(nframes, array[])
 *
 * requeues page frames for normal disk i/o. initiates io
 * via v_pfsio().
 *
 * nframes is the number of pages, and their page frame
 * numbers are in array[].
 *
 * this is called via vcs_requeio() with back-tracking enabled.
 * 
 */

int
v_requeio(nframes, array)
int nframes;
int * array;
{

	int nfr, pdtx, k, sidx;

	/* make sure we don't fault on array
	 */
	TOUCH(array);
	TOUCH(array + nframes - 1);

	sidx = STOI(pft_ssid(array[0]));
	SCB_MPLOCK(sidx);
	PDT_MPLOCK();

	for (k = 0; k < nframes; k++)
	{
		nfr = *(array + k);
		pdtx = pft_devid(nfr);
		v_pdtqiox(pdtx,nfr,1);
	}

	PDT_MPUNLOCK();
	SCB_MPUNLOCK(sidx);

#ifndef _POWER_MP
	v_pfsio();
#endif /* _POWER_MP */

	return 0;
}


/*
 *	v_gather(nfr,bp,pdtx)
 *
 * try to gather contiguous I/O request frames starting with nfr
 * into a single buffer I/O request linked by pft_nextio field. 
 * additional frames will be included if
 * they reside within the same segment and LVM logical track group and
 * are contiguous in virtual address and on-disk.  
 *
 * compaction is performed for contiguous pageouts request gathered
 * in the list for compressed segments.
 *
 * see v_pfend() for scatter of gather list.
 */

/* max. number of compressed pages to coalesce */ 
int	jfs_maxcompact = 8;

int	jfs_cio = 0;		/* number of coalesced i/o */
int	jfs_cpio = 0;		/* number of coalesced i/o pages */

static
v_gather(nfr,bp,pdtx)
int nfr;
struct buf *bp;
int pdtx;
{
	int	sid, sidx;
	int	head, tail, newhead;
	int	fperpage, fragsize, nfrags; 
	uint	ltgmsk;
	int	ltgno;
	int	lstnfrags, nbytes, next; 
	int	nio;
	uint	dst, src, srsave;

	/* init head and tail of gather list.
	 */
	head = tail = nfr;

	sid = pft_ssid(nfr);
	sidx = STOI(sid);

	/* get fragment parameters and
	 * initialize buffer count for nfr.
	 */
	fperpage = pdt_fperpage(pdtx);
	fragsize = PSIZE / fperpage;
	nfrags = fperpage - pft_nfrags(nfr);
	bp->b_bcount = nfrags * fragsize;

	/* get fragments per LVM logical track group mask and
	 * logical track group number of first page.
	 */
	ltgmsk = ~(PGPTRK * fperpage - 1);
	ltgno = pft_fraddr(nfr) & ltgmsk;

	/* restrict number of compressed frames to coalesce
	 * to limit compaction/uncompaction/decode time
	 */
	nio = 1;

	/* scan pdt I/O list for contiguous I/O request frames
	 * with nfr (nfr had been previously removed by caller)
	 */
	while (pdt_iotail(pdtx) >= 0)
	{
		/* get next frame of pdt I/O list.
		 */
		nfr = pft_nextio(pdt_iotail(pdtx));

		/* check for the same I/O direction.
		 */
		if (pft_pagein(nfr) != pft_pagein(head))
			break;

		/* check for the same segment
		 */
		if (pft_ssid(nfr) != sid)
			break;

		/* check for the same LVM logical track group.
		 */
		if ((pft_fraddr(nfr) & ltgmsk) != ltgno)
			break; 

		/* check for the contiguity in memory and on-disk
		 * with head or tail of gather list.
		 */
		nfrags = fperpage - pft_nfrags(nfr);
		/* pages in descending order in i/o list */
		if (newhead = (pft_spage(nfr) == pft_spage(head) - 1))
		{
			if (pft_fraddr(nfr) + nfrags != pft_fraddr(head))
				break;
		}
		/* pages in ascending order in i/o list */
		else if (pft_spage(nfr) == pft_spage(tail) + 1)
		{
			if (pft_fraddr(nfr) != 
			    pft_fraddr(tail) + (fperpage - pft_nfrags(tail)))
				break;
		}
		else
			break;

                /* if this is a pagein the storage key can not be readonly. 
		 * readonly key is set up after i/o is done in v_pfend.
		 */
               	if (pft_pagein(nfr))
                       	ASSERT(pft_key(nfr) != RDONLY);

		/* performance statistics: number of coalesced pages
		 */
		jfs_cpio++;

		/* trace hook.
		 */
		TRCHKL5T(HKWD_VMM_SIO|bp->b_flags,pft_ssid(nfr),
			scb_sibits(sidx),pft_spage(nfr),nfr,bp);

		/* adjust the buffer count to reflect nfr.
		 */
		bp->b_bcount += (nfrags * fragsize);

		/* remove nfr from pdt I/O list.
		 */
		if (nfr == pdt_iotail(pdtx))
			pdt_iotail(pdtx) = -1;
		else
			pft_nextio(pdt_iotail(pdtx)) = pft_nextio(nfr);

		/* add nfr at the appropriate place (head or tail)
		 * on the gather list in ascending order.
		 */
		if (newhead)
		{
			pft_nextio(nfr) = head;
			head = nfr;
		}
		else
		{
			pft_nextio(tail) = nfr;
			pft_nextio(nfr) = 0;
			tail = nfr;
		}

		/* check for threshold for compaction.
		 */
		nio++;
		if (scb_compress(sidx) &&
		    nio >= jfs_maxcompact)
			break;
	}

	/*
	 * for compressed segment requests: 
	 *
	 * the gather list is reorganized into compaction list 
	 * (if compaction and/or uncompaction needs to be done)
	 * such that the head frame points to a list of frames in 
	 * descending page number order so that the pages can be 
	 * uncompacted easily at page fault end.
	 *
	 * also, compact contiguous pageout requests.
	 */
	if (scb_compress(sidx) && nio > 1)
	{
		/* initialize last frame nfrags and 
		 * destination address of the compaction.
		 */
		lstnfrags = fperpage - pft_nfrags(head);
		dst = (TEMPSR << L2SSIZE) + (pft_spage(head) << L2PSIZE);
		dst += (lstnfrags * fragsize);

		/* remove head from gather list and reset it as head of
		 * compaction list: remove each frame from head of remaining 
		 * gather list (ascending order) and insert it after head of 
		 * compaction list (descending order).
		 */ 
		nfr = pft_nextio(head);
		pft_nextio(head) = 0;

		while (nfr != 0)
		{
			/* remember the next frame at head of gather list and 
			 * insert the current frame to the head of compaction
			 * list.
			 */
			next = pft_nextio(nfr);
			pft_nextio(nfr) = pft_nextio(head);
			pft_nextio(head) = nfr;

			/* get nfrags and number of bytes for current page.
			 */
			nfrags = fperpage - pft_nfrags(nfr);
			nbytes = nfrags * fragsize;

			/* check if compaction has been in progress or
			 * compaction is to start (for pageout), or
			 * uncompaction needs to be done (for pagein). 
			 */
			if ((bp->b_flags & B_COMPACTED) || lstnfrags < fperpage)
			{
				/* if compaction is to start or
				 * uncompaction needs to be done
				 * mark buffer as compacted. 
				 */
				if (!(bp->b_flags & B_COMPACTED))
				{
					bp->b_flags |= B_COMPACTED;

				 	/* set up addressability to the segment
					 * for compaction.
					 */
					if (pft_pageout(nfr))
					{
						srsave = chgsr(TEMPSR, 
							       SRVAL(IOSID(sid),0,0));
						src = (TEMPSR << L2SSIZE) +
						      (pft_spage(nfr) << L2PSIZE);
					}
				}

				/* compact the page data.
				 */
				if (pft_pageout(nfr))
				{
					bcopy(src,dst,nbytes);

					/* update source to next page */
					src += PSIZE;
				}
			}

			/* update destination */
			dst += nbytes;

			/* setup for next time around.
			 */
			lstnfrags = nfrags;
			nfr = next;
		}

		/* restore the segment register if we used it.
		 */
		if (pft_pageout(head) &&
		    (bp->b_flags & B_COMPACTED))
			(void)chgsr(TEMPSR,srsave);
	}

finish:
	/* performance statistics: number of coalesced i/o
	 */
	if (pft_nextio(head))
		jfs_cio++;

	/* set the blkno of the buf struct with the block address
	 * of the head of the coalesce list
	 */
	bp->b_blkno = pft_fraddr(head) * (fragsize / 512);

	/* return the head of the coalesce list.
	 */
	return head;
}
