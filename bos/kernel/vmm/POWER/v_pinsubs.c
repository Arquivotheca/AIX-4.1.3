static char sccsid[] = "@(#)38	1.17.1.14  src/bos/kernel/vmm/POWER/v_pinsubs.c, sysvmm, bos41J, 9507A 2/2/95 15:59:10";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	v_pin, v_unpin
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
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
#include <sys/mstsave.h>
#include <sys/intr.h>
#include <sys/syspest.h>
#include <sys/atomic_op.h>
#include <sys/except.h>
#include <sys/inline.h>
#include "vm_mmap.h"
#include "mplock.h"

#define	AtomicAdd(x,y)	(fetch_and_add(x,y))
#define	AtomicFetch(x)	(fetch_and_nop(x))
#define	AtomicStore(x)	(fetch_and_or(x,0))

/*
 * v_pin(sid,pno,typ)
 *
 * adds one to the pincount of the page specified provided
 * the page is in memory.
 *
 * disk blocks will be allocated for working storage pages which have
 * no backing storage unless they are being long-term pinned.
 *
 * RETURN VALUES
 *
 *      0       - ok
 *
 *	ENOMEM  - too many pages already pinned.
 *
 *	ENOSPC  - out of paging disk space.
 */

v_pin(sid,pno,typ)
int     sid;    /* segment id */
int     pno;    /* page number in segment */
int     typ;	/* pin type (short, long term) */
{

        int nfr, sidx, msr;
	uint srsave, pincnt;
	union xptentry *xpt, *v_findxpt();
	int rc, ssid, ssidx, spage, spagex, vaddr;
	int ppkey;
	int object, pgobj;
	vm_offset_t offset, pgoff;
	vm_prot_t type, prot;
	uint storeaddr;

	xpt = NULL;

	/* get sid.
	 */
	sidx = STOI(sid);

#ifdef _VMMDEBUG
	if(sid != vmker.vmmsrval)
		SCB_LOCKHOLDER(sidx);
#endif

	/* touch the page.
	 */
	srsave = chgsr(TEMPSR, SRVAL(sid,0,0));
	TOUCH((TEMPSR << L2SSIZE) + (pno << L2PSIZE));
	(void)chgsr(TEMPSR, srsave);

	if (scb_mseg(sidx))
	{
		/*
		 * Acquire address space lock to perform lookup.
		 */
		rc = v_vmm_lock(&U.U_adspace_lock,0);
		if (rc == VM_WAIT)
			return rc;

		/*
		 * Request read access to the memory we are pinning
		 * (we don't know which direction the DMA operation is
		 * going and the DMA services handle attempts to write to
		 * read-only memory).
		 */
		type = VM_PROT_READ;
		vaddr = pno << L2PSIZE;
		rc = vm_map_lookup(&U.U_map, sid, vaddr, type, &object,
				   &offset, &prot, &pgobj, &pgoff);
			
		/*
		 * Relinquish address space lock.
		 */
		v_vmm_unlock(&U.U_adspace_lock,0);

		/*
		 * The lookup may fail even though a touch was done above
		 * because the touch was done with srkey 0.
		 */
		if (rc != KERN_SUCCESS)
		{
			return(EFAULT);
		}

		/* determine source sid, source pno, and page protection bits.
		 */
		if (pgobj != INVLSID)
		{
			/* This is a MAP_PRIVATE mapping and we need to
			 * ensure that a r/w page exists for the paging SCB.
			 * Copy-on-write semantics require that we atomically
			 * store to the page via the private mapping to ensure
			 * that a private r/w page is created.  Note that it
			 * is OK to use srkey 0 for this store because we are
			 * just trying to catch RDONLY mappings of the page.
			 * This method causes pins to fail for mappings with
			 * PROT_READ access which are legal if the direction
			 * of the DMA operation is from memory.  We prefer
			 * this restriction to the alternative which is to add
			 * a lot of pager logic to deal with the transition
			 * from RDONLY to a private r/w page.  Note that the
			 * use of a store here presumes that the protection
			 * fault handler creates the r/w page so there is no
			 * potential for a ping-pong deadlock.
			 */
			srsave = chgsr(TEMPSR, SRVAL(sid,0,0));
			storeaddr = (TEMPSR << L2SSIZE) + (pno << L2PSIZE);
			AtomicStore(storeaddr);
			(void)chgsr(TEMPSR, srsave);

			ssid = pgobj;
			spage = pgoff >> L2PSIZE;
			ssidx = STOI(ssid);
			spagex = spage;
		}
		else
		{
			ssidx = STOI(object);
			spagex = offset >> L2PSIZE;
			ssid = ITOS(ssidx, spagex);
			spage = BASEPAGE(spagex);
		}
		ppkey = VMPROT2PP(prot);
	}
	else
	{
		ssid = sid;
		ssidx = STOI(ssid);
		spage = pno;
	}

	/* get nfr.
	 */
        nfr = v_lookup(ssid,spage);
	ASSERT(nfr >= 0);

	/* a pin/unpin on page 1 (FBANCH) corrupts the free list
	 */
	if (nfr == FBANCH)	
		return EINVAL;

	/* check for working storage and page not pinned.
	 */
	if (scb_wseg(ssidx) && !pft_pincount(nfr))
	{
		/* get xpt.
		 */
		xpt = v_findxpt(ssidx,spage);
		assert(xpt != NULL);

		/* un-vmap the page if vmapped. v_unvmap() may
		 * fault on vmap block.
		 */
 		if (xpt->mapblk)
		{
			if (v_unvmap(ssidx,spage,xpt,nfr))
				return(ENOSPC);
			goto common;
		}	

		/* allocate a disk block if page does not have backing store.
		 * for long-term pins, we would just free it again below.
		 */
 		if (xpt->cdaddr == 0)
		{
			if (v_dalloc(ssidx,spage,xpt))
				return(ENOSPC);
			pft_devid(nfr) = PDTIND(xpt->word);
			pft_dblock(nfr) = PDTBLK(xpt->word);

			/*
			 * We want to raise the protection on the page but we
			 * only know the correct key for the base address.
			 * Entering the base address using STARTIO causes
			 * other aliases to be removed so that a re-fault gets
			 * the correct protection.  This is more efficient than
			 * simply removing all page table entries since we
			 * avoid the fault for the base address and since no
			 * cache flush occurs (the page is read-only and thus
			 * is not modified).
			 */
			pft_key(nfr) = xpt->spkey;
			P_ENTER(STARTIO,pft_ssid(nfr),pft_spage(nfr),nfr,
				pft_key(nfr),pft_wimg(nfr));
		}
	}

	common:

#ifndef _POWER_MP
	msr = disable();
#endif

	/* check for non-pinned page and pinned page threshold.
	 */
	if (!pft_pincount(nfr) && pf_pfavail <= vmker.pfrsvdblks)
	{
#ifndef _POWER_MP
		mtmsr(msr);
#endif
		return(ENOMEM);
	}

	if (typ == SHORT_TERM)
	{
		/* if mmap region, enter into alias page table.  may fail
		 * if table is full of pinned entries.
		 */
		if (scb_mseg(sidx))
		{
			rc = v_insapt(APTPIN,sid,pno,nfr,ppkey,pft_wimg(nfr));
			if (rc != 0)
			{
#ifndef _POWER_MP
				mtmsr(msr);
#endif
				return(rc);
			}
		}

		/* increment pincount and adjust count of unpinned pages
		 */
		pincnt = AtomicAdd(&pft_pincount(nfr), STPIN_INC);
		assert(((pincnt+STPIN_INC) & STPIN_OVERFLOW) == 0);
	}
	else /* LONG_TERM */
	{
		/* if an unpinned or short-term pinned page is being
		 * long-term pinned, release its backing storage.
		 * only working storage segment pages can get here.
		 */
		if (xpt == NULL)
		{
			xpt = v_findxpt(ssidx,spage);
			assert(xpt != NULL);
		}

		/* if the page has backing storage allocated, free it.
		 * re-long-term pinning a page bypasses this code, as
		 * do first pins on non-working storage segments.
		 */
		if (xpt->cdaddr != 0)
		{
			int	pdtx,dblk,xptdisk;

			xptdisk = xpt->cdaddr;

			pdtx = PDTIND(xptdisk);
			dblk = PDTBLK(xptdisk);

			PG_MPLOCK(pdtx);

			v_dfree(pdtx,dblk);

			PG_MPUNLOCK(pdtx);

			scb_npsblks(ssidx) += -1;

			if (PSEARLYALLOC(ssidx,spage))
				scb_npseablks(ssidx) += 1;
			else
				FETCH_AND_ADD(vmker.psfreeblks, 1);

			/* the xpt must be set to a non-zero value because
			 * other code, v_inherit for example, compares the
			 * word to zero which simultaneously checks both
			 * xpt->zerobit and xpt->cdaddr (page existence).
			 */
			xpt->cdaddr     = 0;
			xpt->zerobit    = 1;
			pft_devid(nfr)  = 0;
			pft_dblock(nfr) = 0;
		}

		pincnt = AtomicAdd(&pft_pincount(nfr), LTPIN_INC);
		assert(((pincnt+LTPIN_INC) & LTPIN_OVERFLOW) == 0);
	}

	/* If the pre-incremented pin count field was all zero, adjust
	 * pf_pfavail.  This test checks long-term and short-term at once.
	 */
	if (pincnt == 0)
		(void)AtomicAdd(&pf_pfavail, -1);

#ifndef _POWER_MP
	mtmsr(msr);
#endif
        return(0);
}

/*
 * v_unpin(sid,pno,typ)
 *
 * subtracts one from the appropriate pincount of the page specified provided
 * the page is already pinned.  if the long-term pincount changes
 * to zero, backing storage will be allocated (regardless of the short-term
 * pin state).  if there is no paging space available, the v_dalloc call
 * will return ENOSPC and v_ltunpin will put the caller on the paging space
 * wait list and return VM_WAIT.  this call cannot be made from the interrupt
 * level.
 * recall that AtomicAdd returns the value of the passed variable _prior to_
 * modification.
 *
 * RETURN VALUES
 *
 *      0       - ok
 *
 *      EINVAL - page is not in memory or pincount is zero
 */

v_unpin(sid,pno,typ)
int     sid;    /* segment id */
int     pno;    /* page number in segment */
int     typ;	/* pin type (short, long term) */
{
        int		 k, msr, nfr, sidx;
	uint		 pincnt;
	union xptentry	*xpt, *v_findxpt();

	sidx = STOI(sid);
	SCB_LOCKHOLDER(sidx);

	nfr = v_lookup(sid,pno);

	if (typ == SHORT_TERM)
	{
		/* page should be in.  unpinning page 1 (FBANCH) corrupts
		 * the free list
		 */
		if((nfr < 0) || (nfr == FBANCH) || !pft_inuse(nfr) ||
		   (pft_pincntst(nfr) == 0))
			return(EINVAL);

		/* decrement pincount and adjust count of pinned pages
		 */
		pincnt  = AtomicAdd(&pft_pincount(nfr), STPIN_DEC);
		pincnt += STPIN_DEC;

		/* if the short-term pincount underflows,
		 * it 'borrows' from long-term.
		 */
		assert((pincnt & STPIN_OVERFLOW) == 0);

		/* If an mmap region, remove alias page table entry.
		 */
		if (scb_mseg(sidx))
			v_delapt(APTPIN,sid,pno,nfr);
	}
	else /* LONG_TERM */
	{
		/* unpin on page 1 (FBANCH) corrupts the free list
	 	*/
		if (nfr == FBANCH)	
			return EINVAL;
		/* page should be in and long-term pinned
		 */
		assert((nfr >= 0) && pft_inuse(nfr) &&
			(pft_pincntlt(nfr) != 0));

		/* Fetch the pincount field for examination.  Because this can
		 * backtrack, you can't change the pincount for a page until
		 * after all backtracking operations have completed.   Else,
		 * the backtrack will assert above.  The pincounts are always
		 * manipulated atomically.
		 */
		pincnt = AtomicFetch(&pft_pincount(nfr));
		if (LongTerm(pincnt) == 1)
		{
			xpt = v_findxpt(sidx,pno);
			assert(xpt != NULL);
			xpt->zerobit = 0;

			/* get a backing page.
		 	 */
			if (v_dalloc(sidx,pno,xpt))
				return(ENOSPC);

			pft_devid(nfr) = PDTIND(xpt->word);
			pft_dblock(nfr) = PDTBLK(xpt->word);
			SETMOD(nfr);
		}
		
		/* decrement the long-term pincount.  If the integer pincount
		 * goes negative, this indicates an error.  This only happens
		 * if either the count gets lowered by someone else between
		 * the 'if' above and the atomic add here or is was already
		 * negative.
		 */
		pincnt  = AtomicAdd(&pft_pincount(nfr), LTPIN_DEC);
		pincnt += LTPIN_DEC;
		assert( (pincnt & LTPIN_OVERFLOW) == 0);
	}

	/* Because of atomic synchronization issues, the variable 'pincnt'
	 * seen here is derived from the result of the atomic decrement of
	 * the pertinent half of the pincount (pincntst or pincntlt).  Now
	 * inspect the whole pincount returned from the atomic operation.
	 * if the page is neither short or long-term pinned, adjust the global
	 * pinned page counter.
	 */
	if (pincnt == 0)
		(void)AtomicAdd(&pf_pfavail, 1);

        return(0);
}
