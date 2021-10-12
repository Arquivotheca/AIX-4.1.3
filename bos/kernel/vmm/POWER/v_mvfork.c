static char sccsid[] = "@(#)12	1.43  src/bos/kernel/vmm/POWER/v_mvfork.c, sysvmm, bos41J, 9507A 2/2/95 16:31:25";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	v_mvfork, v_cpfork
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

/*
 * VMM critical sections code associated with copyseg for fork.
 * see vmforkcopy  for program that uses routine here to copy
 * segments for fork.
 */

#include "vmsys.h"
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/inline.h>
#include <sys/syspest.h>
#include "mplock.h"

/*
 * v_mvfork(psid,fsid,ssid)
 *
 * VMM critical section code for forkcopy.
 *
 * FUNCTION:
 *
 *  (1) moves page frames associated with fsid (forkers
 *      segment) to psid (dummy parent) as follows:
 *
 *      (a) pages in i/o state are marked for discard when
 *          i/o is complete. they are left with fsid.
 *
 *      (b) pages which are page-fixed are left with fsid.
 *          the page frame number of the first such page is
 *          returned.
 *
 *      (c) all other pages are removed from scb list and hash
 *          chain for fsid and put on the same for psid.
 *
 *  (2) psid replaces fsid in the fork tree. psid is made the
 *      parent of fsid and ssid.
 *
 *  (3) swaps the xpt of fsid and psid. sets the maxvpn and
 *      minvpn fields of psid to those of fsid.
 *
 * this operation is serialized with inheritance by the current
 * parent of fsid or a delete of fsid's sibling by the delete bit
 * in the current parent's scb.
 *
 * this program does not page fault but runs at VMM interrupt
 * level and on a fixed stack with back-tracking enabled. back-track
 * is necessary because the caller may have to wait.
 *
 *  RETURN VALUES -
 *
 *	0	- ok
 *
 *      VM_WAIT - parent of fsid is marked in delete state.
 *              - caller is v_waited. this return value never
 *                makes it back to caller.
 */

v_mvfork(psid,fsid,ssid)
int     psid;   /* segment to which pages are moved to. */
int     fsid;   /* segment from which pages are moved from */
int     ssid;   /* sibling of forker (child) */
{
        int k,nfr,phead,fhead,prv,fsidx,psidx,ssidx,nmoved;
        int cpsid, cpsidx, firstf;
	int psalloc;

        /*
         * check that current parent is not in delete state.
         */

        fsidx = STOI(fsid);
        psidx = STOI(psid);

	SCB_LOCKHOLDER(fsidx);
	SCB_LOCKHOLDER(psidx);

        cpsid = scb_parent(fsidx);
        if (cpsid)
        {
                cpsidx = STOI(cpsid);
		SCB_MPLOCK(cpsidx);
                if (scb_delete(cpsidx))
                {
                        v_wait(&pf_deletewait);
			SCB_MPUNLOCK(cpsidx);
                        return(VM_WAIT);
                }
        }

	/*
	 * Process private segments' paging space was allocated for the
	 * data section at segment creation.  We allocate the space for the
	 * stack here.  
	 * Sparse segments' paging space has not been allocated, so we
	 * do so here.
	 * For other segments, all allocation has already been accounted
	 * for.
	 */
#ifdef _VMMDEBUG
	CSA->backt = 0;
#endif
        ssidx = STOI(ssid);
        psidx = STOI(psid);
        if (scb_psearlyalloc(fsidx))
        {
		if (scb_sparse(fsidx))
			psalloc = scb_npsblks(fsidx) + scb_npseablks(fsidx);
		else if (scb_privseg(fsidx))
			psalloc = BTOPG(U_REGION_SIZE) - scb_minvpn(fsidx);
		else
			psalloc = 0;

                if (psalloc > 0)
                {
                        /* Verify that the paging space allocation
                         * will not put the system into a low paging
                         * space state and update the counts.
                         */
                        if (FETCH_AND_LIMIT(vmker.psfreeblks, psalloc,
					    pf_npswarn))
			{
				scb_npseablks(ssidx) += psalloc;
			}
			else
			{
				if(cpsid)
					SCB_MPUNLOCK(cpsidx);
                                return(VM_NOSPACE);
			}
		}

	}

        /*
         * Set up pointers so that psid takes fsid's place in tree.
         */
        scb_parent(psidx) = cpsid;
        if (cpsid)
        {
                if(scb_left(cpsidx) == fsid)
                        scb_left(cpsidx) = psid;
                else
                        scb_right(cpsidx) = psid;
		SCB_MPUNLOCK(cpsidx);
        }

        nmoved = 0;
        phead =  fhead = firstf = -1;

        for(k = scb_sidlist(fsidx); k >= 0; )
        {
                ASSERT(pft_ssid(k) == fsid);
                nfr = k;
                k = pft_sidfwd(k);  /* next time around loop */

                /*
                 * if io mark for discard. leave on list.
                 * compute head of list for loop exit.
                 */
                if (pft_pagein(nfr) || pft_pageout(nfr))
                {
                        pft_discard(nfr) = 1;
                        fhead = (fhead < 0) ? nfr : fhead;
                        continue;
                }

		ASSERT(!pft_free(nfr));

                /*
                 * if pinned leave here. compute head of list
                 * and first fixed frame for loop exit. also,
		 * up page pin count by 1 in case the page 
		 * gets unpinned before it is copied.
                 */
                if (pft_pincount(nfr))
                {
                	fetch_and_add(&pft_pincount(nfr),1);
                        fhead = (fhead < 0) ? nfr : fhead;
                        firstf = (firstf < 0) ? nfr : firstf;
                        continue;
                }

                /*
                 * not i/o and not pinned. remove from scb list
                 * and hash chain . move to hash chain and scb
		 * list for psid.
                 */

                /*
                 * move to hash chain for psid.
                 */
		v_delpft(nfr);
                P_RENAME(psid,pft_spage(nfr),nfr,pft_key(nfr),pft_wimg(nfr));
                pft_ssid(nfr) = psid;
		v_inspft(psid,pft_spage(nfr),nfr);

                /*
                 * remove from fsid scb list
                 */
                nmoved += 1;
                prv = pft_sidbwd(nfr);
                if (prv >= 0)
                        pft_sidfwd(prv) = k;
                if (k >= 0)
                        pft_sidbwd(k) = prv;

                /*
                 * move to scb list for psid
                 */
                pft_sidfwd(nfr) = phead;
                if (phead >= 0)
                        pft_sidbwd(phead) = nfr;
                phead = nfr;
                /*
                 * fix up early paging space allocation count
                 */
                if (PSEARLYALLOC(fsidx,pft_spage(nfr)) && pft_dblock(nfr))
                {
                        scb_npseablks(fsidx) += 1;
                        scb_npseablks(psidx) += -1;
                }
        }

        /*
         * move the pages to psid
         */
        scb_sidlist(psidx) = phead;
        if(phead >= 0)
                pft_sidbwd(phead) = -1;
        scb_npages(psidx) = nmoved;
        scb_maxvpn(psidx) = scb_maxvpn(fsidx);
        scb_minvpn(psidx) = scb_minvpn(fsidx);
        scb_maxvpn(ssidx) = scb_maxvpn(fsidx);
        scb_minvpn(ssidx) = scb_minvpn(fsidx);

        /*
         * fixup scb list for fsid
         */
        scb_npages(fsidx) += -nmoved;
        scb_sidlist(fsidx) = fhead;
        if (fhead >= 0 && pft_pagein(fhead) == 0 && pft_pageout(fhead) == 0)
        {
                pft_sidbwd(fhead) = -1;
        }

        /*
         * set state in psid to newly forked.
         * make psid parent of fsid and ssid.
         */
        scb_left(psidx) = fsid;
        scb_right(psidx) = ssid;
        scb_parent(fsidx) = scb_parent(ssidx) = psid;

	/*
	 * transfer copy-on-write state (mmap).
	 */
	scb_cow(psidx) = scb_cow(fsidx);
	scb_cow(fsidx) = 0;
	
        /*
         * swap the xpt of psid and fsid
         */
        k = scb_vxpto(psidx);
        scb_vxpto(psidx) = scb_vxpto(fsidx);
        scb_vxpto(fsidx) = k;
	k = scb_npsblks(psidx);
	scb_npsblks(psidx) = scb_npsblks(fsidx);
	scb_npsblks(fsidx) = k;

	/*
	 * v_mvfork cannot return -1 since it means VM_WAIT
	 * so return VM_NOTIN if no page-fixed pages remain.
	 */
        return((firstf == -1) ? VM_NOTIN : firstf);
}

/*
 * v_cpfork(fsid,ssid,psid,pno,pscount)
 * copies page pno of forker segment (fsid)
 * to page pno of sibling segment (ssid).
 *
 * RETURN VALUES
 *
 *      0       - ok
 *
 *      ENOSPC  - no paging space block.
 */

int
v_cpfork(fsid,ssid,psid,pno,pscount)
int     fsid;		/* segment id of forker */
int     ssid;		/* segment id of sibling */
int	psid;		/* segment id of parent */
int     pno;		/* page number to copy   */
int *	pscount;	/* segment id of parent */
{
        int  *tptr, p;
	int rc, sidx, fsidx, psidx;

	sidx = STOI(ssid);
        fsidx = STOI(fsid);

	SCB_LOCKHOLDER(sidx);
	SCB_LOCKHOLDER(fsidx);

	/* If xptfixup moved some paging space allocation from parent
	 * segment to forker segment, update the counts here in the
	 * critical section.  Clear pscount so this is only done once.
	 * Parent locking could be avoided if fetch_and_add() used. XXX
	 */
	if (*pscount)
	{
		psidx = STOI(psid);
		scb_npsblks(fsidx) += *pscount;
		SCB_MPLOCK(psidx);
		scb_npsblks(psidx) += -(*pscount);
		SCB_MPUNLOCK(psidx);
		*pscount = 0;
	}

        /* check for a free paging space block.
	 * stop forks when paging space reaches danger level.
         */
	if (vmker.psfreeblks <= pf_npswarn)
	{
		rc = ENOSPC;
		goto out;
	}

	/* if there are no free page frames try to get some.
	 */
	if (rc = v_spaceok(sidx, 1))
	{
		goto out;
	}

        /* touch source (forker) page.
         */
	(void)chgsr(TEMPSR, SRVAL(fsid, 0, 0));
	p = (TEMPSR << L2SSIZE) + (pno << L2PSIZE);
	TOUCH(p);

        /* get page in sibling. it should be a new page.
	 * copy the forkers page to sibling.
         * 
         * Use v_pagein_ublk() for u-block page for performance.
         */
        if (scb_privseg(sidx) && pno == (U_BLOCK >> L2PSIZE))
        {

                /* u-block page */
                if (rc = v_pagein_ublk(sidx,pno,SRVAL(ssid,0,0)))
                {
                        goto out;
                }
        } 
	else
        {
                if (rc = v_pagein(sidx,pno,1,SRVAL(ssid,0,0)))
                {
			goto out;
                }
        }

	COPYPAGE(ssid,pno, fsid, pno);

	/* pin count was increased by 1 in v_mvfork()
	 * Now  reduce it by 1.
	 */
	v_unpin(fsid,pno,SHORT_TERM);
out:
	return rc;
}
