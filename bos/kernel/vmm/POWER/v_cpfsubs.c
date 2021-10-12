static char sccsid[] = "@(#)95	1.19  src/bos/kernel/vmm/POWER/v_cpfsubs.c, sysvmm, bos411, 9436D411a 9/8/94 04:10:31";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	v_cpfprot, v_cpfprotdone
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
#include <sys/syspest.h>
#include <sys/user.h>
#include <sys/mstsave.h>
#include <sys/low.h>
#include <sys/inline.h>
#include "mplock.h"

/*
 * v_cpfprot(sidx, pagex, nfr)
 * 
 * send a message to the client segment strategy
 * routine that a store has been attempted to
 * a readonly page. 
 * 
 * this routine supports client segment end of
 * file.
 * 
 * INPUT PARAMETERS
 *
 * (1) sidx - segment index for client segment
 *
 * (2) pagex - page number within client segment
 *
 * (3) nfr - page frame number
 *
 * the page protection message is sent to the client segment
 * strategy routine with a buf-struct.  v_cpfprotdone is 
 * specified as the interrupt handler for the returned buf 
 * struct. 
 *
 *  RETURN VALUE 
 *
 *       VM_WAIT - v_wait for free buf struct or message
 *          	   buf to return.
 *
 *  NOTE:	- in mpsafe we want to release the vmm_mpsafe_lock
 *		across the strategy routine. we can do it here
 *		since the only caller is v_pfprot which returns
 *		after the call to v_cpfprot. So even if the 
 *		the strategy is not called (case no buf struct)
 *		we release the lock. We also need to start the i/o's
 *		since it is the responsibility of the lock holder.
 *
 *		- in mpeff scb lock is held on entry but release in
 *		the routine (thus not held on exit) so no lock is held
 *		across the strategy routine. 
 */
v_cpfprot(sidx,pagex,nfr)
int sidx;	/* segment index */
int pagex;	/* page number */
int nfr;	/* page frame number */
{
	int pdtx, backtrack;
	struct buf * bp;
	void	v_cpfprotdone();
	struct mstsave *mst;
	int exsid;
	int sidio, pnoio;
	struct mstsave * curcsa;

	SCB_LOCKHOLDER(sidx);

	/* get pdt index of client segment.
	 */
	pdtx = scb_devid(sidx);
	ASSERT(pdtx != NULL);

	/* takes the pdt global lock for accessing pdt bufs.
	 */
	PDT_MPLOCK();

	/* if no bufstructs v_wait on global wait
	 * anchor pf_bufwait.
	 */
	if ((bp = pdt_bufstr(pdtx)) == NULL)
	{
		v_wait(&pf_bufwait);
		PDT_MPUNLOCK();

		/* Need to unlock here too since the caller
		 * assume we unlock it. (Cf release across
		 * the strategy routine).
		 */
		SCB_MPUNLOCK_S(sidx);
		VMM_MPSAFE_UNLOCK();
		return(VM_WAIT);
	}

	/* allocate buf-struct
	 * Move the update of the pdt list here so that we
	 * can unlock earlier.
	 */
	pdt_bufstr(pdtx) = bp->av_forw;
	bp->av_forw = 0;

	PDT_MPUNLOCK();

	/* Insert page at I/O virtual address with current
	 * protection key (strategy routine doesn't access the
	 * page so it doesn't have to be r/w).
	 * Remove all APT entries so future alias faults get correct
	 * protection key.
         */
        sidio = IOSID(ITOS(sidx,pagex));
        pnoio = BASEPAGE(pagex);
	pft_inuse(nfr) = 0;
	v_delaptall(nfr);
        P_ENTER(STARTIO,sidio,pnoio,nfr,pft_key(nfr),pft_wimg(nfr));
	pft_pagein(nfr) = 1;

	/*
	 * Set pfprot bit so the frame won't get discarded.
	 * This is necessary to avoid losing modifications to the
	 * page since NFS can issue a vm_flushp() while the protection
	 * fault is pending (the v_write() will skip the modified page
	 * because it is in I/O state and v_release() would mark it for
	 * discard except we prevent this by setting pft_pfprot).  By
	 * doing this we assume that other page delete operations such
	 * as from vms_delete() or vm_umount() won't occur while a
	 * protection fault is pending (presumably from a use count)
	 * otherwise we would leave the segment in pending I/O delete
	 * state forever.
	 */
	pft_pfprot(nfr) = 1;

	/* fill in buf-struct.
	 * strategy routine needs the gnode
	 * pointer, block number and count.
	 */  
	bp->b_vp = (struct vnode *)scb_gnptr(sidx);
	bp->b_blkno = (PSIZE/512)*pagex;
	bp->b_bcount = PSIZE;

	/* 
	 * we use b_forw to hold the sidx and b_back
	 * to hold pagex. set protection fault on store 
	 * in buffer. also, set v_cpfprotdone as iodone.
	 * set flags to indicate that vmm buffers are mp safe
	 */    
	bp->b_forw = (struct buf *)sidx;
	bp->b_back = (struct buf *)pagex;
#ifdef _POWER_MP
	bp->b_flags = B_MPSAFE | B_MPSAFE_INITIAL | B_PFSTORE | B_PFPROT;
#else /* _POWER_MP */
	bp->b_flags = B_PFSTORE | B_PFPROT;
#endif /* _POWER_MP */
	bp->b_iodone = v_cpfprotdone;

	/* if mmap fault, tell client strategy routine
	 * to fail if store is beyond end-of-file.
	 */
	mst = (struct mstsave *) &u.u_save;
	exsid = SRTOSID(mst->except[ESRVAL]);
	if (scb_mseg(STOI(exsid)))
		bp->b_flags |= B_PFEOF;

	/* wait on scb waitlist. 
	 */
	v_wait(&scb_waitlist(sidx));

	/* We release the vmm_mpsafe_lock and start the pending i/o's.
	 * The strategy routine can be safely called w/o any serialization.
	 * We release the scb lock before calling the strategy routine.
	 * So, we enter the routine w/ the lock but we exit w/o !
	 */
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	/* do not allow page faults in client code.
	 */
	curcsa = CSA;
	backtrack = curcsa->backt;
	curcsa->backt = 0; 
	(*pdt_strategy(pdtx)) (bp);

	/* restore backtrack ok state.
	 */
	curcsa->backt = backtrack;

	/* return VM_WAIT.
	 */
	return(VM_WAIT);
}

/*
 * v_cpfprotdone(bp)
 *
 * this routine gets control as an interrupt handler
 * for page protection messages previously sent to the
 * client segment strategy routine. it will make the
 * page read-write and ready all processes on the segment
 * waitlist and all process waiting for buf structs.
 *
 * bp is the address of the corresponding buf-struct.
 *
 *  RETURN VALUE 
 *		None
 *
 */

void
v_cpfprotdone(bp)
struct buf *bp;
{
        int srsave,sidx,pdtx,pagex,nfr;
	struct thread *tp;
	int error;
	int sid, pno;
	int ismod;

        /* make vmmdseg addressable at normal place
         */
        srsave = chgsr(VMMSR,vmker.vmmsrval);

        /* 
         * sidx overlays hash link forward field and 
	 * pagex overlays hash link backward field (see
         * v_cfprot). put buf struct back on free list.
         */
        sidx = (int) bp->b_forw;
        pagex = (int) bp->b_back;

	/* Grab the vmm_mmpsafe_lock spin, ih cannot block.
	 */
	VMM_MPSAFE_SLOCK();

        pdtx = scb_devid(sidx);

	PDT_MPLOCK();

        bp->av_forw = pdt_bufstr(pdtx);
        pdt_bufstr(pdtx) = bp;
        error = (bp->b_flags & B_ERROR) ? bp->b_error : 0;

#ifdef _POWER_MP
        bp->b_flags |= B_DONE; /* due to funneling (Cf iodone_offlevel) */
#endif /* _POWER_MP */

	/* wakeup any waitors for a free buf-struct.
	 */
	while (pf_bufwait)
	{
		v_ready(&pf_bufwait);
	}

	PDT_MPUNLOCK();

	SCB_MPLOCK(sidx);

	/* Lookup page frame.
	 */
	sid = ITOS(sidx,pagex);
	pno = BASEPAGE(pagex);
	nfr = v_lookup(sid,pno);
	assert(nfr >= 0);

	pft_pagein(nfr) = 0;
	pft_pfprot(nfr) = 0;

	/* Wake up the waitor waiting for the message
	 * buf-struct to return.  If error, send them
	 * an exception before making them runnable.
	 */
	while (scb_waitlist(sidx) != NULL)
	{
		tp = scb_waitlist(sidx)->t_next;
		if (error)
			v_except(tp,error,nfr,0);
		v_ready(&scb_waitlist(sidx));
	}

	/* wakeup any waitors who faulted on the frame while it
	 * was in i/o state.  no need to send an exception if an
	 * error occurred -- the frame stays r/o and waitor re-faults.
	 */
	while(pft_waitlist(nfr) != NULL)
	{
		v_ready(&pft_waitlist(nfr));
	}

	/*
         * Page frame should not have been discarded (we set pfprot
	 * to prevent this) so we don't need to handle that here.
         */

	/* Raise page protection to r/w (if no error) and
	 * insert page at normal virtual address.
	 * Also, preserve the modification status of the page.
	 */
	if (!error)
		pft_key(nfr) = FILEKEY;
	ismod = ISMOD(nfr);
	P_ENTER(IODONE,sid,pno,nfr,pft_key(nfr),pft_wimg(nfr));
	pft_inuse(nfr) = 1;
	if (ismod)
		SETMOD(nfr);
		
        /* restore sreg and return
         */
	SCB_MPUNLOCK(sidx);
	VMM_MPSAFE_UNLOCK();
        (void)chgsr(VMMSR,srsave);
        return ;
}
