static char sccsid[] = "@(#)44	1.75.1.2  src/bos/kernel/vmm/POWER/v_putsubs.c, sysvmm, bos41J, 9513A_all 3/17/95 14:05:45";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	v_pageout, v_spaceok, v_write, v_qmodify, v_qpages
 *		v_extmem, v_hfblru, v_lruwait
 *
 * ORIGINS: 27 83
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

/*
 * VMM routines related to pushing pages out of main
 * memory. these programs run with back-track enabled.
 */

#include "vmsys.h"
#include <sys/errno.h>
#include <sys/proc.h>
#include <sys/syspest.h>
#include <sys/systm.h>
#include <sys/inline.h>
#include "mplock.h"

#define MINFREE		4	/* minimum free-frames after fblru local i/o */
#define	MAXREVS		2	/* maximum number of fblru revolutions */
#define	NUMSTEAL	4	/* number of pages replaced during syssi  */

/*
 * v_pageout(sidx,nfr,fblru)
 *
 * pageout processing for page frame specified. if the page
 * is modified the page is enqued for i/o on the device PDT
 * specified in the page frame table, and the pdt is put
 * on the list of pdts with pending io. 
 *
 * if fblru is specified, and the page has been re-referenced,
 * the page is not affected, and the value 1 is returned.
 *
 * the parameter fblru specifies the disposition of the page
 * when i/o for the page (if any) is complete as follows. if
 * fblru is true the page is put on the free list, otherwise
 * it is not put on the free list, but its refbit is reset
 * to zero.
 *
 * RETURN VALUES - 0 ok.
 *		 - 1 fblru specified but page was re-referenced.
 */

int
v_pageout(sidx,nfr,fblru)
int sidx;       /* index in scb table */
int nfr;        /* page frame number */
int fblru;      /* 1 if called from v_fblru, 0 otherwise */
{

        int sidio,pnoio,pdtx, ipri, modbit, reref;
	extern struct cache_line_lock swhash_lock[];

	SCB_LOCKHOLDER(sidx);

	/* update repaging rate data if fblru
         */
        if (fblru)
                addrepage(nfr);

        /* Put page at I/O virtual address. for fblru, in MP
	 * the ref bit is re-tested after it is given its I/O
	 * address. if it has been referenced, the page must be
	 * left addressable at its normal address, because the
	 * page may be in a fixed stack or have been touched in
	 * a backtrack section. Note that the SWHASH lock is acquired
	 * before changing to the I/O address so that any re-reference
 	 * in another processor made at interrupt level will be
	 * blocked in the reload code until we have the chance
	 * in this cpu of putting the address back to normal.
         */
        sidio = IOSID(pft_ssid(nfr));
        pnoio = pft_spage(nfr);

#ifdef _POWER_MP
	if (fblru)
	{
		SWHASH_MPLOCK(ipri,sidio,pnoio);
	}
#endif /* _POWER_MP */

	pft_inuse(nfr) = 0;

	/*
	 * Remove all alias page table entries since we assume no
	 * aliases exist while frame is in I/O state.
	 */
	if (pft_alist(nfr) != APTNULL)
		v_delaptall(nfr);
	P_ENTER(STARTIO,sidio,pnoio,nfr,pft_key(nfr),pft_wimg(nfr));
        pft_pageout(nfr) = 1;

#ifdef _POWER_MP
	if (fblru)
	{
		if(reref = ISREF(nfr))
		{
			modbit = ISMOD(nfr);
			pft_pageout(nfr) = 0;
			P_ENTER(IODONE,pft_ssid(nfr),pft_spage(nfr),
				nfr,pft_key(nfr),pft_wimg(nfr));
			if (modbit) SETMOD(nfr);
			pft_inuse(nfr) = 1;
		}
		SWHASH_MPUNLOCK(ipri,sidio,pnoio);
		if (reref) return 1;
	}
#endif /* _POWER_MP */


        /* if page is not modified put on free list
	 * or not according as fblru is true or not.
         */
        if (!ISMOD(nfr))
        {
                if (fblru)
                {
			pft_pageout(nfr) = 0;
			v_relframe(sidx, nfr);
                }
                else
                {
			pft_pageout(nfr) = 0;
			P_ENTER(IODONE,pft_ssid(nfr),pft_spage(nfr),
					nfr,pft_key(nfr),pft_wimg(nfr));
			pft_inuse(nfr) = 1;
                }
                return 0;
        }

	/* catch invalid page-outs.
	 */
	assert(!(pft_devid(nfr) == 0 && pft_dblock(nfr) == 0));

#ifdef _VMMDEBUG
	/* catch invalid page outs of working storage
	 */
	ASSERT( pf_npgspaces || !scb_wseg(STOI(pft_ssid(nfr))) );
#endif /* _VMMDEBUG */

        /* move page frame to io part of scb list.
	 * reset newbit flag.
         */
	pft_newbit(nfr) = 0;
        v_mtioscb(sidx,nfr);

        /* enque page frame and pdt for i/o.
         */
        pdtx = pft_devid(nfr);
	if (fblru) pft_fblru(nfr) = 1;

	PDT_MPLOCK();

        v_pdtqiox(pdtx,nfr,0);

	PDT_MPUNLOCK();

	/* set segment modified bit
	 */
	scb_chgbit(sidx) = 1;

        return 0;
}

/*
 * fblru in general:
 *
 * chooses for replacement enough page frames so that the
 * number of free blocks will be pf_maxfree after i/o for
 * replaced pages is complete.
 *
 * page replacement algorithm is simple clock algorithm:
 * the cursor pf_lruptr points to the next real page frame
 * to be examined. if its refbit is zero it is chosen for
 * replacement. otherwise its refbit is reset and the
 * cursor is advanced to the next frame.
 *
 * this program may page fault and executes at VMM interrupt
 * level with back-tracking enabled. the only page faults
 * that can occur are for modified pages of deferred-update
 * or journalled segments with homeok false. for such pages
 * it is necessary to allocate a paging space disk block for
 * the page and to record its address in a lock table entry.
 *
 * RETURN VALUES - none.
 */

int bailouts = 0;

/* v_lfblru()
 *
 * fblru for working or persistent frame request.
 */
static
v_lfblru(ws)
int ws;		/* true for working request - false if persistent request */
{
        int     nb, ic, lru, sidx, progress, newfree, cpu;
	int 	rev, start, fileonly;

	/* fileonly if repaging of computational too big.
	 */
	if (ws)
		fileonly = (pf_rpgcnt[RPCOMP] > pf_rpgcnt[RPFILE]);
	else
		fileonly = (pf_rpgcnt[RPCOMP] >= pf_rpgcnt[RPFILE]);

	fileonly &= (vmker.numperm >= pf_minperm);
        fileonly |= (vmker.numperm >= vmker.maxperm);


	/* To make progress we want at least MINFREE free pages
	 * after local i/o completes.
	 */
	progress = (vmker.numfrb + pf_numpout - pf_numremote >= MINFREE);
	nb = pf_maxfree - vmker.numfrb - pf_numpout;
		
	/* steal at least (pf_maxfree - pf_minfree) pages
	 * if pages are to be stolen.
	 */
	if (nb > 0 && nb < pf_maxfree - pf_minfree)
		nb = pf_maxfree - pf_minfree;

	/* adjust repaging rate only if pages are
	 * to be replaced.
	 */
	if (nb > 0 || !progress)
	{
		pf_rpgcnt[RPFILE] = (vmker.rpdecay * pf_rpgcnt[RPFILE]) / 100;
		pf_rpgcnt[RPCOMP] = (vmker.rpdecay * pf_rpgcnt[RPCOMP]) / 100;
	}

        for (ic = rev = 0, lru = start = pf_lruptr; ic < nb || !progress;
		lru += 1)
        {
		vmpf_scans += 1;

		/* if MAXREVS revolutions of the page frame table have
		 * occurred then try one more sweep if fileonly is set;
		 * otherwise, set pf_freewake and bail out.
		 */
		if (lru == start)
		{
			rev += 1;
			if (rev > MAXREVS)
			{
				if (fileonly && !progress)
				{
					fileonly = 0;
					rev = 1;
				}
				else
				{
					pf_freewake = 1;
					break;
				}
			}
		}

		/* Skip over memory holes.
		 */
                if (lru == pf_skiplru)
		{
			pf_lruidx++;
			if (pf_lruidx >= vmint_num(VMINT_BADMEM))
			{
				/* Wrap back to the beginning.
				 */
#ifdef _POWER_MP
				/*
				 * If another CPU is in a stackfix critical
				 * section then we must halt page-replacement
				 * to avoid scanning it's stack page twice and
				 * stealing it.
				 * For MP-safe we must bail out rather than
				 * spin because we hold the MP-safe lock and
				 * the other CPU may be spinning on it.  We set
				 * the lru cursor so the stackfix condition is
				 * immediately checked again when starting
				 * page-replacement.
				 * For MP-eff we don't hold any locks so we
				 * can just spin waiting for the other CPUs
				 * to get out of their critical sections.
				 */
				for(cpu = 0; cpu < NCPUS();  cpu++)
				{
					if (cpu == CPUID)
						continue;
#ifdef _VMM_MP_EFF
					while (teststackfix(cpu));
#else /* _VMM_MP_EFF */
					if (teststackfix(cpu))
					{
						pf_lruptr = lru;
						bailouts++;
						return(VM_NOWAIT);
					}
#endif /* _VMM_MP_EFF */
				}
#endif /* _POWER_MP */

				vmpf_cycles += 1;
				lru = pf_firstnf;
				pf_lruidx = 0;
			}
			else
			{
				/* Skip over this hole.
				 */
				lru = vmint_end(VMINT_BADMEM,pf_lruidx-1);
			}
			pf_skiplru = vmint_start(VMINT_BADMEM,pf_lruidx);
		}

		/* Continue if fileonly pages are to be replaced
		 * and this is a computational  page.
		 */
                sidx = STOI(pft_ssid(lru));
		if (fileonly && scb_compseg(sidx))
			continue;

		/* log writes are controlled by v_writelogp() since
		 * they are written in a ping pong fashion.
	 	 */
		if (scb_logseg(sidx))
			continue;
		
		/* skip over modified pages of files of being committed
		 * if they are compressed.
		 * In mp eff those tests are first done in-flight and
		 * then confirmed if necessary under lock.
		 */
		if (scb_combit(sidx) && scb_compress(sidx) && ISMOD(lru))
			continue;

                if (pft_inuse(lru) == 0 || pft_pincount(lru) != 0)
                        continue;

                if (ISREF(lru))
                        continue;

#ifdef _VMM_MP_EFF
		/* Now we lock the scb and confirm the tests:
		 * must check that the page did not change state.
		 * we can safely lock the scb since it has already
		 * been allocated and thus the lock is initialized.
		 * refbit is tested again in v_pageout.
		 */
		SCB_MPLOCK_S(sidx);

		/* if mapping has changed, skip the page
		 */
		if(!pft_slist(lru) || sidx != STOI(pft_ssid(lru)))
		{
			SCB_MPUNLOCK_S(sidx);
			continue;
		}

		if (scb_combit(sidx) && scb_compress(sidx) && ISMOD(lru))
		{
			SCB_MPUNLOCK_S(sidx);
			continue;
		}

                if (pft_inuse(lru) == 0 || pft_pincount(lru) != 0)
		{
			SCB_MPUNLOCK_S(sidx);
			continue;
		}
#endif /* _VMM_MP_EFF */

                /* if journal or deferred update check xmemok.
                 */
                if (scb_jseg(sidx) | scb_defseg(sidx))
                {
                        if (xmemok(sidx,lru) == 0)
			{
				SCB_MPUNLOCK_S(sidx);
                                continue;
			}
                }

		/* page out can fail for MP if page is re-referenced.
		 */
                if (v_pageout(sidx,lru,FBLRU))
		{
			SCB_MPUNLOCK_S(sidx);
			continue;
		}

		SCB_MPUNLOCK_S(sidx);

		vmpf_pgsteals += 1;
                ic += 1;

		/* Recalculate progress.
		 */
		if (!progress)
		{
			newfree = vmker.numfrb + pf_numpout - pf_numremote;
			progress = newfree >= MINFREE;
		}
	}

        pf_lruptr = lru;

#ifndef _POWER_MP
	/* initiate i/o if any i/o enqued.
	 */
	if (pf_iotail >= 0)
		v_pfsio();
#endif /* _POWER_MP */

        return 0;
}

/* v_cfblru()
 *
 * fblru for client or compressed frame request.
 */
static
v_cfblru(client)
int client; /* true if request is for a client page frame */
{
        int     nb, ic, lru, sidx, fileonly, clientonly, clientcomp;
	int 	rev, start, limit, total, progress, cpu;

	/* replace file pages only unless repaging files is
	 * bigger than computational repaging or number of
	 * file pages is small.
	 */
	fileonly = (pf_rpgcnt[RPCOMP] >= pf_rpgcnt[RPFILE]);
	fileonly &= (vmker.numperm >= pf_minperm);

	/* If we are over limit and this is a request for a 
	 * client page, replace only client pages. if the total
	 * number of client and compressed pages is too big,
	 * replace  only client or compressed pages.
	 */
	limit = (7*vmker.maxclient) >> 3;
	total = vmker.numclient + vmker.numcompress;
	if (client && vmker.numclient >= limit - pf_minfree)
	{
		fileonly = 0;
		clientonly = 1;
		clientcomp = 0;
		nb = vmker.numclient - limit  + pf_maxfree
			- pf_numremote;
	}
	else
	if (total >= vmker.maxclient - pf_minfree)
	{
		fileonly = 0;
		clientonly = 0;
		clientcomp = 1;
		nb = vmker.maxclient - total  + pf_maxfree
			- pf_numremote;
	}
	else
	{
		clientonly = 0;
		clientcomp = 0;
        	nb = pf_maxfree - vmker.numfrb - pf_numpout;
	}

	/* steal at least (pf_maxfree - pf_minfree) pages
	 * if pages are to be stolen.
	 */
	if (nb > 0 && nb < pf_maxfree - pf_minfree)
		nb = pf_maxfree - pf_minfree;

	/* adjust repaging rate only if pages are
	 * to be replaced.
	 */
	if (nb > 0)
	{
		pf_rpgcnt[RPFILE] = (vmker.rpdecay * pf_rpgcnt[RPFILE]) / 100;
		pf_rpgcnt[RPCOMP] = (vmker.rpdecay * pf_rpgcnt[RPCOMP]) / 100;
	}

	/* if the request is for a compressed page, forward progress
	 * requires at least one non-client page steal. since we can not
	 * guarantee that remote page outs ever complete, progress is
	 * set to true for the client case.
	 */
	progress = (client) ? 1 : 0;

        for (ic = rev = 0,lru = start = pf_lruptr; ic < nb || !progress; lru++)
        {
		vmpf_scans += 1;

		/* if MAXREVS revolutions of the page frame table have
		 * occurred then try one more sweep if fileonly is set;
		 * otherwise, set pf_freewake and bail out.
		 */
		if (lru == start)
		{
			rev += 1;
			if (rev > MAXREVS)
			{
				if (fileonly)
				{
					fileonly = 0;
					rev = 1;
				}
				else
				{
					pf_freewake = 1;
					break;
				}
			}
		}

		/* Skip over memory holes.
		 */
                if (lru == pf_skiplru)
		{
			pf_lruidx++;
			if (pf_lruidx >= vmint_num(VMINT_BADMEM))
			{
				/* Wrap back to the beginning.
				 */
#ifdef _POWER_MP
				/*
				 * If another CPU is in a stackfix critical
				 * section then we must halt page-replacement
				 * to avoid scanning it's stack page twice and
				 * stealing it.
				 * For MP-safe we must bail out rather than
				 * spin because we hold the MP-safe lock and
				 * the other CPU may be spinning on it.  We set
				 * the lru cursor so the stackfix condition is
				 * immediately checked again when starting
				 * page-replacement.
				 * For MP-eff we don't hold any locks so we
				 * can just spin waiting for the other CPUs
				 * to get out of their critical sections.
				 */
				for(cpu = 0; cpu < NCPUS();  cpu++)
				{
					if (cpu == CPUID)
						continue;
#ifdef _VMM_MP_EFF
					while (teststackfix(cpu));
#else /* _VMM_MP_EFF */
					if (teststackfix(cpu))
					{
						pf_lruptr = lru;
						bailouts++;
						return(VM_NOWAIT);
					}
#endif /* _VMM_MP_EFF */
				}
#endif /* _POWER_MP */

				vmpf_cycles += 1;
				lru = pf_firstnf;
				pf_lruidx = 0;
			}
			else
			{
				/* Skip over this hole.
				 */
				lru = vmint_end(VMINT_BADMEM,pf_lruidx-1);
			}
			pf_skiplru = vmint_start(VMINT_BADMEM,pf_lruidx);
		}

		/* Continue if clientonly pages are to be replaced
		 * and this is not client.
		 */
                sidx = STOI(pft_ssid(lru));
		if (clientonly && !scb_clseg(sidx))
			continue;

		/* continue if client or compressed pages are to be
		 * replaced and this is neither.
		 */
		if (clientcomp && !(scb_clseg(sidx) || scb_compress(sidx)))
			continue;

		/* Continue if fileonly pages are to be replaced
		 * and this is a computational  page.
		 */
		if (fileonly && scb_compseg(sidx))
			continue;

		/* log writes are controlled by v_writelogp() since
		 * they are written in a ping pong fashion.
	 	 */
		if (scb_logseg(sidx))
			continue;

		/* skip over modified pages of files of being committed
		 * if they are compressed.
		 * In mp eff those tests are first done in-flight and
		 * then confirmed if necessary under lock.
		 */
		if (scb_combit(sidx) && scb_compress(sidx) && ISMOD(lru))
			continue;

                if (pft_inuse(lru) == 0 || pft_pincount(lru) != 0)
                        continue;

		if (ISREF(lru))
			continue;

#ifdef _VMM_MP_EFF
		/* Now we lock the scb and confirm the tests:
		 * must check that the page did not change state.
		 * we can safely lock the scb since it has already
		 * been allocated and thus the lock is initialized.
		 * refbit is tested again in v_pageout.
		 */
		SCB_MPLOCK_S(sidx);

		/* if mapping has changed, skip the page
		 */
		if(!pft_slist(lru) || sidx != STOI(pft_ssid(lru)))
		{
			SCB_MPUNLOCK_S(sidx);
			continue;
		}

		if (scb_combit(sidx) && scb_compress(sidx) && ISMOD(lru))
		{
			SCB_MPUNLOCK_S(sidx);
			continue;
		}

                if (pft_inuse(lru) == 0 || pft_pincount(lru) != 0)
		{
			SCB_MPUNLOCK_S(sidx);
			continue;
		}
#endif /* _VMM_MP_EFF */

                /* if journal or deferred update check xmemok.
                 */
                if (scb_jseg(sidx) | scb_defseg(sidx))
                {
                        if (xmemok(sidx,lru) == 0)
			{
				SCB_MPUNLOCK_S(sidx);
				continue;
			}
                }

		/* page out can failed for MP if page is re-referenced.
		 */
                if (v_pageout(sidx,lru,FBLRU))
		{
			SCB_MPUNLOCK_S(sidx);
			continue;
		}
		SCB_MPUNLOCK_S(sidx);

		vmpf_pgsteals += 1;
                ic += 1;
		if (progress == 0)
		{
			progress = !scb_clseg(sidx);
		}
        }

        pf_lruptr = lru;

#ifndef _POWER_MP
	/* initiate i/o if any i/o enqued.
	 */
	if (pf_iotail >= 0)
		v_pfsio();
#endif /* _POWER_MP */

        return 0;
}

/* v_hfblru()
 *
 * fblru for hibernation support.
 */

v_hfblru(newfree, criteria)
int newfree;	/* number of new free frames */
int criteria;	/* lru criteria */
{
        int     nb, ic, lru, sidx, cpu;
	int 	rev, start, fileonly, unmodified;

	/*
	 * We want to steal enough to have 'newfree' frames available.
	 * We use the specified lru criteria to select which frames
	 * to steal.
	 */
	nb = newfree - (vmker.numfrb + pf_numpout);
	fileonly = ((criteria & LRU_FILE) == LRU_FILE);
	unmodified = ((criteria & LRU_UNMOD) == LRU_UNMOD);
		
        for (ic = rev = 0, lru = start = pf_lruptr; ic < nb; lru += 1)
        {
		vmpf_scans += 1;

		/*
		 * If we've made a complete pass and haven't achieved our
		 * goal using the specified criteria then return.
		 */
		if (lru == start)
		{
			rev += 1;
			if (rev > 1)
				break;
		}

		/* Skip over memory holes.
		 */
                if (lru == pf_skiplru)
		{
			pf_lruidx++;
			if (pf_lruidx >= vmint_num(VMINT_BADMEM))
			{
				/* Wrap back to the beginning.
				 */
#ifdef _POWER_MP
				/*
				 * If another CPU is in a stackfix critical
				 * section then we must halt page-replacement
				 * to avoid scanning it's stack page twice and
				 * stealing it.
				 * For MP-safe we must bail out rather than
				 * spin because we hold the MP-safe lock and
				 * the other CPU may be spinning on it.  We set
				 * the lru cursor so the stackfix condition is
				 * immediately checked again when starting
				 * page-replacement.
				 * For MP-eff we don't hold any locks so we
				 * can just spin waiting for the other CPUs
				 * to get out of their critical sections.
				 */
				for(cpu = 0; cpu < NCPUS();  cpu++)
				{
					if (cpu == CPUID)
						continue;
#ifdef _VMM_MP_EFF
					while (teststackfix(cpu));
#else /* _VMM_MP_EFF */
					if (teststackfix(cpu))
					{
						pf_lruptr = lru;
						bailouts++;
						return(VM_NOWAIT);
					}
#endif /* _VMM_MP_EFF */
				}
#endif /* _POWER_MP */

				vmpf_cycles += 1;
				lru = pf_firstnf - 1;
				pf_lruidx = 0;
				pf_skiplru = vmint_start(VMINT_BADMEM,0);
				continue;
			}
			else
			{
				/* Skip over this hole.
				 */
				lru = vmint_end(VMINT_BADMEM,pf_lruidx-1);
				pf_skiplru = vmint_start(VMINT_BADMEM,pf_lruidx);
			}
		}

		/* Continue if fileonly pages are to be replaced
		 * and this is a computational page.
		 */
                sidx = STOI(pft_ssid(lru));
		if (fileonly && scb_compseg(sidx))
			continue;

		/* log writes are controlled by v_writelogp() since
		 * they are written in a ping pong fashion.
	 	 */
		if (scb_logseg(sidx))
			continue;
		
		/* skip over modified pages of files of being committed
		 * if they are compressed.
		 * In mp eff those tests are first done in-flight and
		 * then confirmed if necessary under lock.
		 */
		if (scb_combit(sidx) && scb_compress(sidx) && ISMOD(lru))
			continue;

                if (pft_inuse(lru) == 0 || pft_pincount(lru) != 0)
                        continue;

		/*
		 * Continue if only unmodified pages are to be stolen
		 * and this frame is modified.  Note on MP that we can't
		 * guarantee that modified pages won't get paged out since
		 * another processor may modify the page after the test.
		 * So the LRU_UNMOD criteria is a suggestion but not an
		 * absolute.
		 */
		if (unmodified && ISMOD(lru))
			continue;

		/*
		 * We don't use referenced status as a criteria but for MP
		 * we need to clear it so that v_pageout() succeeds.
		 */
#ifdef _POWER_MP
                (void)ISREF(lru);
#endif /* _POWER_MP */

#ifdef _VMM_MP_EFF
		/* Now we lock the scb and confirm the tests:
		 * must check that the page did not change state.
		 * we can safely lock the scb since it has already
		 * been allocated and thus the lock is initialized.
		 * refbit is tested again in v_pageout.
		 */
		SCB_MPLOCK_S(sidx);

		/* if mapping has changed, skip the page
		 */
		if(!pft_slist(lru) || sidx != STOI(pft_ssid(lru)))
		{
			SCB_MPUNLOCK_S(sidx);
			continue;
		}

		if (scb_combit(sidx) && scb_compress(sidx) && ISMOD(lru))
		{
			SCB_MPUNLOCK_S(sidx);
			continue;
		}

                if (pft_inuse(lru) == 0 || pft_pincount(lru) != 0)
		{
			SCB_MPUNLOCK_S(sidx);
			continue;
		}
#endif /* _VMM_MP_EFF */

                /* if journal or deferred update check xmemok.
                 */
                if (scb_jseg(sidx) | scb_defseg(sidx))
                {
                        if (xmemok(sidx,lru) == 0)
			{
				SCB_MPUNLOCK_S(sidx);
                                continue;
			}
                }

		/* page out can fail for MP if page is re-referenced.
		 */
                if (v_pageout(sidx,lru,FBLRU))
		{
			SCB_MPUNLOCK_S(sidx);
			continue;
		}

		SCB_MPUNLOCK_S(sidx);

		vmpf_pgsteals += 1;
                ic += 1;
	}

        pf_lruptr = lru;

#ifndef _POWER_MP
	/* initiate i/o if any i/o enqued.
	 */
	if (pf_iotail >= 0)
		v_pfsio();
#endif /* _POWER_MP */

        return(ic);
}

#ifndef _VMM_MP_EFF

/*
 * v_spaceok(sidx,need)
 *
 * check if sufficient free frames are available. if not, the appropriate
 * page replacement routine is called to get frames for the free list.
 *
 *  RETURN VALUE
 *
 *      0       - free frame available
 *
 *      VM_WAIT	- free frame not available, or persistent or client segment
 *		  and number of file pages is greater than maximum, or
 *		  client segment and number of client pages is greater
 *		  than maximum.
 */

v_spaceok(sidx, need)
int sidx;
int need;
{
	int suffree, total, client, limit, rc;

	/* See if sufficient free frames are available.
	 */
	suffree = (vmker.numfrb >= need &&
		   vmker.numfrb + pf_numpout > pf_minfree);

	if (!pf_npgspaces)
	{
		/* paging space has not been defined yet, so
		 * call initsteal() to steal selected pages.
		 */
		if (suffree)
			return(0);

		if (rc = initsteal())
			return(rc);
	}
	else if ((client = scb_clseg(sidx)) || scb_compress(sidx))
	{
		/* compressed and client pages share the client
		 * page frame pool. its maximum size is vmker.maxclient.
		 * all of the pages in the pool are allowed to be
		 * compressed pages, but only 0.875 of them can be
		 * client pages. this rule is necessary to guarantee
		 * forward progress when the node is both a server
		 * and client for NFS.
		 */

		if (client)
		{
			limit  = ((7*vmker.maxclient) >> 3);
			if (suffree && vmker.numclient < limit - pf_minfree)
				return 0;
		}
		else
		{
			total = vmker.numclient + vmker.numcompress;
			if (suffree && total < vmker.maxclient - pf_minfree)
				return 0;
		}
				
		if (rc = v_cfblru(client))
			return(rc);

		if (client)
		{
			if (vmker.numclient >= limit)
				goto out;
		}
		else
		{
			total = vmker.numclient + vmker.numcompress;
			if (total >= vmker.maxclient)
				goto out;
		}
	}
	else 
	{
		/* For working storage or persistent storage -
		 * check if sufficient free blocks are available.
		 */
		if (suffree)
			return(0);

		if (rc = v_lfblru(scb_wseg(sidx)))
			return(rc);
	}

	/* return VM_WAIT if there are no enough free frames.
	 */
	if (vmker.numfrb < need)
		goto out;

	return(0);

out:
	vmpf_freewts += 1;
	v_wait(&pf_freewait);
	return(VM_WAIT);
}

#else  /* _VMM_MP_EFF */

/*
 * Storage classes identified to the LRU daemon via lru_requested.
 * Ordered such that higher values have more strict forward
 * progress requirements than lower values.
 */
#define NONE		0
#define	INITSTEAL	1
#define	CLIENT		2
#define	COMPRESS	3
#define PERSISTENT	4
#define WORKING		5

/*
 * v_spaceok(sidx, need)
 *
 * check if sufficient free frames are available. if not, the appropriate
 * page replacement routine is called to get frames for the free list.
 *
 *  RETURN VALUE
 *
 *      0       - free frame available
 *
 *      VM_WAIT	- free frame not available, or persistent or client segment
 *		  and number of file pages is greater than maximum, or
 *		  client segment and number of client pages is greater
 *		  than maximum.
 * MPeff version
 */

v_spaceok(sidx, need)
int sidx;
int need;
{
	int suffree, total, client, limit, new_request;

	SCB_LOCKHOLDER(sidx);

	VMKER_MPLOCK();

	 /*
	  * See if the daemon needs to be started to run fblru.
	  * Holding the VMKER lock ensures that other threads aren't
	  * allocating or deallocating frames so vmker.numfrb isn't changing.
	  * Code at endpageout() decrements pf_numpout before releasing
	  * a fblru frame.  Both of these together ensure that the
	  * unserialized calculation of vmker.numfrb + pf_numpout here
	  * is less than or equal to the "real" sum, thus counting each
	  * frame only once.  So we may start the daemon when the "real"
	  * counts don't warrant it but we will always start it when it
	  * is needed.
	  */
	suffree = vmker.numfrb + pf_numpout > pf_minfree;

	if (!pf_npgspaces)
	{
		/* paging space has not been defined yet, so
		 * call initsteal() to steal selected pages.
		 */
		if (suffree)
			goto nolru;

		/*
		 * We use lru_requested to tell the daemon which
		 * class of storage to steal.  If it is already
		 * set then the daemon has already been posted.
		 * If not then we tell it to run the INITSTEAL fblru.
		 * We will reset lru_requested if the storage class
		 * for this request is more favored (i.e. has more
		 * strict forward progress requirements) than the
		 * previous value.
		 * The daemon may be sleeping in which case we ready
		 * it.  It may already be running another fblru
		 * in which case it will run ours when it finishes.
		 */
		if (lru_requested < INITSTEAL)
		{
			lru_requested = INITSTEAL;
			if (lru_daemon)
				v_ready(&lru_daemon);
		}
	}
	else if ((client = scb_clseg(sidx)) || scb_compress(sidx))
	{
		/* compressed and client pages share the client
		 * page frame pool. its maximum size is vmker.maxclient.
		 * all of the pages in the pool are allowed to be
		 * compressed pages, but only 0.875 of them can be
		 * client pages. this rule is necessary to guarantee
		 * forward progress when the node is both a server
		 * and client for NFS.
		 */

		if (client)
		{
			limit  = ((7*vmker.maxclient) >> 3);
			if (suffree && vmker.numclient < limit - pf_minfree)
				goto nolru;
		}
		else
		{
			total = vmker.numclient + vmker.numcompress;
			if (suffree && total < vmker.maxclient - pf_minfree)
				goto nolru;
		}
				
		new_request = client ? CLIENT : COMPRESS;
		if (lru_requested < new_request)
		{
			lru_requested = new_request;
			if (lru_daemon)
				v_ready(&lru_daemon);
		}

		if (client)
		{
			if (vmker.numclient >= limit)
				goto out;
		}
		else
		{
			total = vmker.numclient + vmker.numcompress;
			if (total >= vmker.maxclient)
				goto out;
		}
	}
	else 
	{
		/* For working storage or persistent storage -
		 * check if sufficient free blocks are available.
		 */
		if (suffree)
			goto nolru;

		new_request = scb_wseg(sidx) ? WORKING : PERSISTENT;
		if (lru_requested < new_request)
		{
			lru_requested = new_request;
			if (lru_daemon)
				v_ready(&lru_daemon);
		}
	}

	/*
	 * Make free frame reservation.  We hold the VMKER lock
	 * so numfrb can't be changing.
	 */
nolru:
	if (vmker.numfrb >= need)
	{
		GET_RESERVATION(PPDA,need);
		VMKER_MPUNLOCK();
		return 0;
	}

out:
	/*
	 * We may not have readied the lru daemon but in 
	 * this case we have pageouts which will free frames
	 * and cause us to wake up and try again.
	 */
	vmpf_freewts += 1;
	v_wait(&pf_freewait);
	VMKER_MPUNLOCK();
	return(VM_WAIT);
}

/*
 * v_lru()
 *
 * Critical section to perform page-replacement.
 * Loops in a VMM bactrack critical section waiting for
 * a request to start page-replacement, initiates it,
 * and then waits again.
 *	
 */
v_lru()
{
	int lrurq;

	/*
	 * Determine the requested storage class from
	 * variable lru_requested and then reset it.
	 */
	VMKER_MPLOCK();

startagain:
	lrurq = lru_requested;
	lru_requested = NONE;

	VMKER_MPUNLOCK();

	/*
	 * Run the LRU routine for the requested storage class.
	 * The default case (NONE) happens on the initial invocation
	 * or when a backtrack fault occurs (we've reset it above and
	 * we restart the critical section) in which case we invoke
	 * LRU as for case WORKING.  Invoking lru on the initial
	 * invocation is safe assuming we have sufficient free frames
	 * so that we don't actually steal anything (we don't yet have
	 * a real paging device so any pageout to paging space would
	 * hang).
	 */
	switch (lrurq)	
	{
		case WORKING:
			v_lfblru(1);
			break;
		case PERSISTENT:
			v_lfblru(0);
			break;
		case CLIENT:
			v_cfblru(1);
			break;
		case COMPRESS:
			v_cfblru(0);
			break;
		case INITSTEAL:
			initsteal();
			break;
		default:
			v_lfblru(1);
	}

	/*
	 * Initiate i/o if any i/o enqueued.
	 */
	if (pf_iotail >= 0)
	{
		v_pfsio();
	}

	/*
	 * If we freed frames, wake up of waitors has already
	 * been done by v_relframe.
	 * Restart LRU if a request has been posted, otherwise
	 * wait for a new request.
	 */
	VMKER_MPLOCK();

	if(lru_requested == NONE)
		v_wait(&lru_daemon);
	else
		goto startagain;

	VMKER_MPUNLOCK();

	return (VM_WAIT);
}

/*
 * kproc_wash()
 *
 * LRU daemon run as a kproc.  Must be started before there is a
 * need to perform page-replacement.  Simply invokes a backtrack
 * critical section routine which does the work and never returns.
 */
kproc_wash()
{
	vcs_lru();
	assert(0);
}

#endif /* _VMM_MP_EFF */

/*
 * xmemok(sidx,lru)
 *
 * checks that page frame lru of a journalled or deferred update
 * segment can be paged out as follows:
 *
 * (1) if the page is not modified, ok.
 *
 * (2) if journalled and homeok bit is set, ok.
 *
 * (3) if disk block associated with the page is in paging
 *     space, or page is new ok.
 *
 * (4) otherwise a disk block in paging space is allocated
 *     for the page.
 *
 *  RETURN VALUE
 *
 *      0       - NOT OK (only happens if paging space is full)
 *
 *      1       - OK to pageout lru.
 */

static
xmemok(sidx,lru)
int     sidx;   /* index in scb table */
int     lru;    /* page frame number */
{
        int     pdtx,daddr,lw;
	int rc;

	SCB_LOCKHOLDER(sidx);
#ifdef _VMM_MP_EFF
	ASSERT(PPDA->lru == VM_INLRU);
#endif

        /* ok if modbit == 0  or journal and homeok
         */
        if (!ISMOD(lru) || (scb_jseg(sidx) && pft_homeok(lru)))
                return(1);

        /* ok page if new or if disk block is in paging space 
         */
        pdtx = pft_devid(lru);
        if (pft_newbit(lru) || pdt_type(pdtx) == D_PAGING)
                return(1);

        /* must allocate a paging space disk block. advance pf_lruptr
         * in case we page fault on inserting a lock word.
         */
        pf_lruptr = lru + 1;

	/* insert/find  a lock word (insert returns 0 if no memory)
	 */
	if ((lw = v_insertlw(pft_ssid(lru),pft_spage(lru))) == 0)
		return 0;

	/* Even if we cannot allocate a paging space disk block
	 * the home location will be found in the lockword.
	 */
	ASSERT(pft_dblock(lru));
	lword[lw].home = pft_dblock(lru);

        /* allocate a disk block
	 * The allocation can fail but the page is skipped
	 * and the lock word is in a correct state.
         */
        if (v_pgalloc(&pdtx,&daddr))
		return 0;

	FETCH_AND_ADD(vmker.psfreeblks, -1);

        /* set lockword disk fields and pft disk fields.
         */
	lword[lw].extmem = XPTADDR(pdtx,daddr);
        pft_devid(lru) = pdtx;
        pft_dblock(lru) = daddr;
	return 1;
}

/*
 * initsteal()
 *
 * performs the page replacement function during system
 * initialization (prior to the definition of the first
 * real paging space).  the pages selected for replacement
 * by this routine consist of unmodified pages or 
 * modified file pages which can be written back to their
 * home location. NUMSTEAL pages will be replaced.
 *
 */
static
initsteal()
{
        int     ic,lru,sidx,cpu;
	int	rev,start;

        for (ic = rev = 0, lru = start = pf_lruptr; ic < NUMSTEAL; lru += 1)
        {
		vmpf_scans += 1;

		/* if we've looped MAXREVS times without finding NUMSTEAL
		 * frames and fewer than NUMSTEAL frames remain then
		 * halt and display an LED, otherwise bail out.
		 */
		if (lru == start)
		{
			rev += 1;
			if (rev > MAXREVS)
			{
				if (vmker.numfrb + pf_numpout < NUMSTEAL)
					halt_display(csa, HALT_NOMEM, 0);
				else
					break;
			}

		}

		/* Skip over memory holes.
		 */
                if (lru == pf_skiplru)
		{
			pf_lruidx++;
			if (pf_lruidx >= vmint_num(VMINT_BADMEM))
			{
				/* Wrap back to the beginning.
				 */
#ifdef _POWER_MP
				/*
				 * If another CPU is in a stackfix critical
				 * section then we must halt page-replacement
				 * to avoid scanning it's stack page twice and
				 * stealing it.
				 * For MP-safe we must bail out rather than
				 * spin because we hold the MP-safe lock and
				 * the other CPU may be spinning on it.  We set
				 * the lru cursor so the stackfix condition is
				 * immediately checked again when starting
				 * page-replacement.
				 * For MP-eff we don't hold any locks so we
				 * can just spin waiting for the other CPUs
				 * to get out of their critical sections.
				 */
				for(cpu = 0; cpu < NCPUS();  cpu++)
				{
					if (cpu == CPUID)
						continue;
#ifdef _VMM_MP_EFF
					while (teststackfix(cpu));
#else /* _VMM_MP_EFF */
					if (teststackfix(cpu))
					{
						pf_lruptr = lru;
						bailouts++;
						return(VM_NOWAIT);
					}
#endif /* _VMM_MP_EFF */
				}
#endif /* _POWER_MP */

				vmpf_cycles += 1;
				lru = pf_firstnf;
				pf_lruidx = 0;
			}
			else
			{
				/* Skip over this hole.
				 */
				lru = vmint_end(VMINT_BADMEM,pf_lruidx-1);
			}
			pf_skiplru = vmint_start(VMINT_BADMEM,pf_lruidx);
		}
#ifdef _VMM_MP_EFF
		/* must check that the page did not change state.
		 * we can safely lock the scb since it has been
		 * already allocated and thus the lock initialized.
		 */
                sidx = STOI(pft_ssid(lru));
		SCB_MPLOCK(sidx);
		if(scb_valid(sidx) == 0 || STOI(pft_ssid(lru)) !=  sidx)
		{
			SCB_MPUNLOCK(sidx);
			continue;
		}
#endif /* _VMM_MP_EFF */
                sidx = STOI(pft_ssid(lru));

		/* log writes are controlled by v_writelogp() since
		 * they are written in a ping pong fashion.
	 	 */
		if (scb_logseg(sidx))
		{
			SCB_MPUNLOCK(sidx);
			continue;
		}

                if (pft_inuse(lru) == 0 || pft_pincount(lru) != 0)
		{
			SCB_MPUNLOCK(sidx);
			continue;
		}

                if (ISREF(lru))
		{
			SCB_MPUNLOCK(sidx);
			continue;
		}

		/* skip over modified defer pages or working pages
		 * and modified journalled pages with home ok not set.
		 */
		if (ISMOD(lru))
		{
			if (scb_defseg(sidx) || scb_wseg(sidx))
			{
				SCB_MPUNLOCK(sidx);
				continue;
			}

			if (scb_jseg(sidx) && !pft_homeok(lru))
			{
				SCB_MPUNLOCK(sidx);
				continue;
			}
		}

		/* pageout can fail for MP if page is re-referenced.
		 */
                if (v_pageout(sidx,lru,FBLRU))
		{
			SCB_MPUNLOCK(sidx);
			continue;
		}
                ic += 1;
		vmpf_pgsteals += 1;
		SCB_MPUNLOCK(sidx);
        }

        pf_lruptr = lru;

#ifndef _POWER_MP
	/* initiate i/o if any i/o enqued.
	 */
	if (pf_iotail >= 0)
		v_pfsio();
#endif /* _POWER_MP */

	return 0;
}

/*
 * v_write(sid,pfirst,npages,force)
 *
 * initiates page out of virtual storage addresses specified.
 * for non-journalled segments I/O for modified pages is
 * initiated and unchanged pages are left in memory but with
 * their refbit set to zero.
 *
 * input parameters
 *
 *	sid	- base segment id 
 *	pfirst  - first page in scb 
 *	npages  - number of pages starting from pfirst.
 *	force   - initiate i/o always.
 *
 * for journalled segments i/o is initiated for a page only
 * if the parameter force is specified as true or the page
 * was marked by sync processing (the pft_syncpt bit is true)
 * or if the page is in extension memory. if i/o is not started
 * is not initiated the pft_homeok bit is set and the page 
 * page left in memory.
 *
 * for all segment types if a page is written it is written
 * to its home address.
 *
 * for client and persistent storage segments the caller may
 * wait for the i/o initiated by this and prior calls to 
 * complete with the service v_iowait.
 *
 * procedure executes at VMM interrupt level with back-tracking
 * enabled. it is only called by vm_write.
 *
 * RETURN VALUES
 *
 *	0	- ok
 *
 */

v_write(sid,pfirst,npages,force)
int     sid;    /* base segment id */
int     pfirst; /* first page number */
int     npages; /* number of pages */
int     force;  /* 1 to force i/o for journalled segments */
{

        int k,p,sidx,rc,first,last,nfr ;

	/* nothing to do ?
	 */
	if (npages <= 0)
		return 0;

        /* try to use maxvpn and minvpn to limit search
         */
        first = pfirst;
        last = pfirst + npages - 1;
        sidx = STOI(sid);

	SCB_LOCKHOLDER(sidx);

        if (scb_wseg(sidx))
        {
		if (first > scb_maxvpn(sidx) && first < scb_minvpn(sidx))
			first = scb_minvpn(sidx);

		else if (last > scb_maxvpn(sidx) && last < scb_minvpn(sidx))
			last = scb_maxvpn(sidx);
        }
        else
        {
                last = MIN(last,scb_maxvpn(sidx));
        }

        /* we have to check extension memory if the segment
	 * is journalled or deferred-update.
         */
        if(scb_jseg(sidx) || scb_defseg(sidx))
        {
		/*
		 * For .indirect or disk map segments acquire the FS lock
		 * to serialize pageout of pages in these segments with
		 * references in other critical sections.
		 */
		if (scb_indseg(sidx) || scb_dmapseg(sidx))
			FS_MPLOCK_S(scb_devid(sidx));

                for(p = first; p <= last; p++)
                {
			/* v_extmem checks if page is in extension
			 * memory. if so it touches page to bring
			 * it in and establishes home address in pft.
			 */
                        if ((nfr = v_extmem(sidx,p)) >= 0)
			{
                                writep(sidx,nfr,force);
			}
                }

#ifndef _POWER_MP
		/* initiate i/o if any was enqued by above.
		 */
		if (pf_iotail >= 0)
			v_pfsio();
#endif /* _POWER_MP */

		if (scb_indseg(sidx) || scb_dmapseg(sidx))
			FS_MPUNLOCK_S(scb_devid(sidx));

		return 0;
        }

	/* not journalled and not deferred-update.
         * if the number of pages is small compared to
         * number of pages in memory process the pages
         * in the interval [first,last] using v_lookup.
         */

        if (last - first < scb_npages(sidx) >> 3)
        {
                for (k = first; k <= last; k++)
                {
			if ((nfr = v_lookup(ITOS(sidx,k),BASEPAGE(k))) >= 0 &&
					ISMOD(nfr))
                                writep(sidx,nfr,force);
			
                }
        }

        /* number of pages is large compared to pages in
         * memory so we scan pages on scb_list and write
         * those that fall in the interval [first,last].
         */

        else
        {
                for(k = scb_sidlist(sidx); k >= 0; )
                {
                        nfr = k;
                        k = pft_sidfwd(k);
			if (!ISMOD(nfr))
				continue;
                        if (pft_pagex(nfr) >= first && pft_pagex(nfr) <= last)
                                writep(sidx,nfr,force);
                }
        }

#ifndef _POWER_MP
	/* initiate i/o if any was enqued by above.
	 */
	if (pf_iotail >= 0)
		v_pfsio();
#endif /* _POWER_MP */

        return(0);
}

/*
 * writep(sidx,nfr,force)
 * initiate pageout of page specified
 */
writep(sidx,nfr,force)
int     sidx;   /* index in scb table */
int     nfr;    /* page frame number */
int     force;  /* 1 to force i/o for journalled segments */
{

	SCB_LOCKHOLDER(sidx);

        /* check that its in the "inuse" state
         */
        if (!pft_inuse(nfr))
                return;

        /* page it out if not journalled or syncpt is set 
	 * or force is specified. or the page is new.
         */
        if (!scb_jseg(sidx) || force || pft_syncpt(nfr) || pft_newbit(nfr))
        {
		pft_homeok(nfr) = 1;
                pft_syncpt(nfr) = 0;
                v_pageout(sidx,nfr,NOFBLRU);
                return;
        }

        /* journalled segment not force and not syncpt.
         */
        pft_homeok(nfr) = 1;
}

/*
 * v_extmem(sidx,pagex)
 *
 * prepares a page in a deferred-update or journalled segment
 * for writing to its home address.
 *
 * (1) if the page has no extension memory disk block, returns
 *     the page frame number containing the page or -1 if it is
 *     not in memory.
 *
 * (2) if the page has an extension memory disk block:
 *
 *     (a) frees the extension memory disk block.
 *
 *     (b) returns the page frame of the page with the disk
 *         address in the pft set to the home address, the
 *         modified bit set, and the pft_syncpt bit set so
 *         that writep will write the page out.
 *
 */

int
v_extmem(sidx,pagex)
int sidx;
int pagex;
{
        int hash,k, pdtx, dblk,lw, daddr, sid, pno;

	SCB_LOCKHOLDER(sidx);

	/* look for page in memory.
	 */
	sid = ITOS(sidx,pagex);
	pno = BASEPAGE(pagex);

	if ((k = v_lookup(sid,pno)) >= 0)
	{
		/* test for extension memory. 
		 */
		pdtx = pft_devid(k);
		if (pdt_type(pdtx) != D_PAGING)
			return(k);
	}

	/* check lock table to see if it has been assigned a 
	 * disk block in extension memory.
	 */
	LW_MPLOCK_S();
	if ((lw = v_findlw(sid,pno)) == 0 || lword[lw].extmem == 0)
	{
		LW_MPUNLOCK_S();
		return -1;
	}
	LW_MPUNLOCK_S();

	/* page has an extension memory block. 
	 * first get it into memory in normal state.
	 */
	(void)chgsr(TEMPSR,SRVAL(sid,0,0));
	TOUCH(TEMPSR*SEGSIZE + pno*PSIZE);

	/* free the extension memory disk block.
	 */
	daddr = lword[lw].extmem;
	pdtx = PDTIND(daddr);
	dblk = PDTBLK(daddr);

	PG_MPLOCK(pdtx);

	v_dfree(pdtx,dblk);

	PG_MPUNLOCK(pdtx);

	FETCH_AND_ADD(vmker.psfreeblks, 1);
	lword[lw].extmem = 0;

	/* set home address in the page frame and set modified bit
	 * and pft_syncpt so that it will get written out.
	 * set logage in page frame for v_sync.
	 */
	pft_devid(k) = scb_devid(sidx);
	pft_dblock(k) = lword[lw].home;
	pft_logage(k) = lword[lw].log;
	SETMOD(k);
	pft_syncpt(k) = 1;

	return k;
}

/*
 * v_qmodify(sid)
 *
 * returns 1 if the scb_chgbit or if the segment has a
 * modified page in memory. returns 0 otherwise. resets
 * the scb_chgbit.
 *
 */

int
v_qmodify(sid)
int sid;	/* segment id */
{
	int k, sidx;

	sidx = STOI(sid);

	SCB_LOCKHOLDER(sidx);

	if (scb_chgbit(sidx))
	{
		scb_chgbit(sidx) = 0;
		return 1;
	}

	for (k = scb_sidlist(sidx); k >= 0; k = pft_sidfwd(k)) 
	{
		if (ISMOD(k))
			return 1;
	}

	return 0;
}

/*
 * v_qpages(sid)
 *
 * returns number of pages in memory associated with a 
 * segment.
 */
int
v_qpages(sid)
int sid;	/* segment id */
{
	int  sidx;

	/* to get around fs build-backout problems
	 */
	return 0;

	sidx = STOI(sid);
	SCB_LOCKHOLDER(sidx);
	return(scb_npages(sidx));
}

/*
 * addrepage(nfr)
 *
 * add an entry into repaging table.  the entry used is the
 * oldest entry in the table. if the entry was repaged, the
 * repaging rate is decremented.
 *
 * PARAMETERS:
 *	
 *	nfr	 - real frame number of the page.
 *
 * RETURN VALUES:
 *
 *	NONE
 */

int
addrepage(nfr)
int nfr;	/* frame number */
{

	int type, nf, hash, nn, qq;
	uint sidx;

	sidx = STOI(pft_ssid(nfr));
	SCB_LOCKHOLDER(sidx);

	RPT_MPLOCK();

	/* increment nreplaced.
         */
        type = (scb_compseg(sidx)) ? RPCOMP : RPFILE;
        pf_nreplaced[type] += 1;

	/* get next free entry . index 0 and rptsize - 1 are not used.
	 */
	vmker.rptfree += 1;
	if (vmker.rptfree >= vmker.rptsize - 1)
		vmker.rptfree = 1;


	/* if nf is still on hash chain remove it from chain.
	 * otherwise it was repaged, so decrement repaging count.
	 */
	nf = vmker.rptfree;
	if (rpt_next(nf) != vmker.rptsize - 1)
	{
		/* remove nf from hash chain
		 */
		hash = RPHASH(rpt_tag(nf),rpt_pno(nf));
		nn = rphtab[hash];
		qq = NULL;
		while (nn != nf)
		{
			qq = nn;
			nn = rpt_next(nn);
		}

		if (qq != NULL)
			rpt_next(qq) = rpt_next(nf);
		else
			rphtab[hash] = rpt_next(nf);
	}

	/* insert nf at head of hash chain
	 */
	if (scb_wseg(sidx))
	{
		rpt_tag(nf) = pft_dblock(nfr);
		rpt_pno(nf) = pft_devid(nfr);
	}
	else
	{
		rpt_tag(nf) = scb_serial(sidx);
		rpt_pno(nf) = pft_spage(nfr);
	}

	hash = RPHASH(rpt_tag(nf),rpt_pno(nf));
	rpt_next(nf) = rphtab[hash];
	rphtab[hash] = nf;

	RPT_MPUNLOCK();

	return 0;
}

/*
 * chkrepage(sidx, nfr)
 *
 * this procedure is called from v_pdtqio() to see if the page
 * has been repaged.  if so, the repaging rate is incremented,
 * the page removed from the repaging hash-table, and marked
 * as having been repaged by setting next field to a special
 * value.
 *
 * PARAMETERS:
 *	
 *	nfr	 - real frame number of the page.
 *
 * RETURN VALUES:
 *
 *	NONE
 */

int
chkrepage(nfr)
int nfr;	/* frame number */
{
	int type, hash, nn, qq, tag, pno;
	uint sidx;
	struct proc *proc_p= curproc;

	sidx = STOI(pft_ssid(nfr));
	SCB_LOCKHOLDER(sidx);

	/* search table for entry
	 */
	if (scb_wseg(sidx))
	{
		tag = pft_dblock(nfr);
		pno = pft_devid(nfr);
	}
	else
	{
		tag = scb_serial(sidx);
		pno = pft_spage(nfr);
	}

	RPT_MPLOCK();

	hash = RPHASH(tag,pno);
	for (nn = rphtab[hash], qq = NULL; nn != 0; qq = nn,  nn = rpt_next(nn))
	{
		if (rpt_tag(nn) != tag || rpt_pno(nn) != pno)
			continue;

		/* increment repaging rate and total count
		 */
		type = (scb_compseg(sidx)) ? RPCOMP : RPFILE;
		pf_rpgcnt[type] += 1;
		pf_nrepaged[type] += 1;
		vmker.sysrepage += 1;
        	proc_p->p_repage += 1;

		/* remove nn from hash chain 
		 */
		if (qq != NULL)
			rpt_next(qq) = rpt_next(nn);
		else
			rphtab[hash] = rpt_next(nn);

		/* markit as being repaged and return.
		 */
		rpt_tag(nn) = type;
		rpt_next(nn) = vmker.rptsize - 1;	


		break;
	}

	RPT_MPUNLOCK();

	return 0;
}

/*
 * v_lruwait()
 *
 * Wait for all page-replacement I/O to complete.
 *
 * This procedure should be called on a fixed stack with
 * back-tracking enabled.
 *
 * Return values
 *      0       -       no wait
 *      VM_WAIT -       process must wait
 */

v_lruwait()
{
	int lruio;

	/*
	 * If there is outstanding page-replacement I/O then
	 * wait on the free-frame waitlist.  Any fblru pageout
	 * completion will wake us up and we'll make the test
	 * again.  Note that pf_numpout must be updated atomically
	 * to ensure this test works on MP.
	 */
	lruio = FETCH_AND_ADD(pf_numpout,0);
	if (lruio)
	{
		v_wait(&pf_freewait);
		return(VM_WAIT);
	}
	return(0);
}
