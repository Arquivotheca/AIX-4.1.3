static char sccsid[] = "@(#)50	1.56  src/bos/kernel/vmm/POWER/v_relsubs.c, sysvmm, bos41J, 9521B_all 5/24/95 14:28:58";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	v_delete, v_release, v_relframe, v_inherit
 *		v_relalias, v_invalidate
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
#include <sys/syspest.h>
#include "mplock.h"

/*
 * v_delete(sid,pfirst,npages)
 *
 * releases the virtual memory resources associated with the
 * range of addresses specified. paging space disk blocks 
 * are freed.
 *
 * this procedure is only used by vms_delete to delete segments.
 * for working storage segments all of the pages specified must be
 * covered by one direct block of the external page table, and
 * the direct block is assumed to have been allocated on entry.
 * for client or persistent segments there is no limitation.
 *
 * the deletion of a sid must be serialized with cross-memory
 * attach (all segment types). when sid is a working segment
 * it is also necessary to serialize with the delete of his
 * sibling and the inheritance by their common parent (i.e
 * their grandparent).
 *
 * serialization with  delete/inherit is done before cross-memory
 * attach and is implemented by means of the "delete bit". a delete
 * is blocked if the delete bit of the parent is set and the bit
 * was not previously set by the caller. an inherit is blocked if
 * the delete bit is set in the child and the bit was not set
 * previously by the caller. if blocked, the calling process is
 * v_waited, and this procedure is re-invoked by the back-track
 * mechanism when the blocking condition is cleared.
 *
 * once delete/inherit serialization is accomplished the child
 * is detached from his parent so that sid is in the same position
 * relative to cross-memory as the other types of segments. if
 * the cross-memory count is non-zero, the value VM_XMEMCNT is
 * returned, but nothing else is done (actual deletion of sid will
 * occur later). however, when  sid is a working storage segment
 * his sibling's inheritance is NOT delayed by xmem count.
 *
 * RETURN VALUES
 *
 *      0               - ok
 *
 *      VM_XMEMCNT      - ok xmem attach count not-zero.
 *
 *      VM_WAIT         - process was v_waited. this return value
 *                        doesn't make it back to caller.
 */

v_delete(sid,pfirst,npages)
int sid;        /* segment id */
int pfirst;     /* first page */
int npages;     /* number of pages */
{
        int work,child,psid,psidx,sidx;

        sidx =  STOI(sid);
        work = scb_wseg(sidx);

	SCB_LOCKHOLDER(sidx);

        /* if sid is a working segment and it has a parent
         * check the parent's delete bit  in case sibling
         * is already deleting his segment or the parent is
         * inheriting from his parent.
         */
        if(work && (psid = scb_parent(sidx)))
        {
                psidx = STOI(psid);

		SCB_MPLOCK(psidx);

                /*
                 * is sid still a child ? if not a previous call
                 * to v_delete established sid for deletion ahead
                 * of sibling and inheritance from grandparent.
		 * i.e fault in v_release below and backtrack
                 */
                child = scb_left(psidx) == sid || scb_right(psidx) == sid;

                if (child)
                {
                        /* if delete bit is set sibling got to
                         * delete first or grandparent got to
                         * inherit first.
                         */
                        if (scb_delete(psidx))
                        {
                                v_wait(&pf_deletewait);
				SCB_MPUNLOCK(psidx);
                                return(VM_WAIT);
                        }

                        /* remove child pointer in parent to sid and
                         * set delete bit in parent.
                         */
                        scb_delete(psidx) = 1;
                        if (scb_left(psidx) == sid)
                                scb_left(psidx) = 0;
                        else
                                scb_right(psidx) = 0;
                }

		SCB_MPUNLOCK(psidx);
        }

        /* now check for cross-memory activity. if there
         * is we just return. vms_delete handles the rest.
	 * It is OK to test this unserialized on MP-eff --
	 * we may put the SCB on the delete pending list
	 * unnecessarily but it will eventually get deleted.
         */
        if (scb_xmemcnt(sidx) != 0)
                return(VM_XMEMCNT);

        v_release(sid,pfirst,npages,V_NOKEEP);

        return 0;
}

/*
 * v_release(sid,pfirst,npages,flags)
 *
 * all page frames in memory associated with the range of virtual
 * addresses specified are discarded. frees disk blocks if the
 * segment is a working storage segment.
 *
 * if the segment is to be retained the flag V_KEEP should
 * be set to true and to false if the segment is being deleted.
 * if V_KEEP is true for working storage segments the value of
 * the pages is zero after the operation, and the storage protect
 * protect keys set to their default values.
 *
 * for working segments all of the pages specified must be
 * covered by one direct block of the external page table, and
 * the direct block is assumed to have been allocated on entry.
 * for client or persistent segments there is no limitation.
 *
 * if V_KEEP is true none of the pages should be page-fixed if
 * the segment is a working storage segment. if not the value
 * EINVAL is returned .
 *
 * if a page is currently on the page in or out state the real page
 * frame is not freed until the i/o is complete, but the logical
 * contents of the virtual page is reset immediately.
 *
 * If there may not be an xpt for the range, then the flag
 * V_CHECKXPT should be set to make the check for the xpt here.
 * (The caller may not be able to serialize with fork tree xpt
 * manipulation.)

 * this procedure runs on fixed stack at VMM interrupt level with
 * back track enabled. this procedure is used by v_delete and
 * vm_release.
 *
 * RETURN VALUE
 *
 *      0       - ok
 *	EINVAL  - a page fixed page was encountered in a working
 *		  storage segment but V_KEEP is true.
 *
 */

int 
v_release(sid,pfirst,npages,flags)
int     sid;    /* segment id */
int     pfirst; /* first page number */
int     npages; /* number of pages to release */
int	flags;	/* state handling flags */
{
        int rc,first,last,sidx,psid,psidx,work,child;
        union xptentry *v_findxpt();

        /* nothing to do ?
         */
        if (npages <= 0)
                return(0);

        /* get index in scb.
         */
        sidx =  STOI(sid);

	SCB_LOCKHOLDER(sidx);

	/* Check for xpt, if required.
	 */
	if (flags & V_CHECKXPT && !v_findxpt(sidx, pfirst))
		return VM_NOXPT;

	/* set last to last page we have to handle
	 */
        first = pfirst;
        last = pfirst + npages - 1;
        work = scb_wseg(sidx);

        /* try to use maxvpn and minvpn to limit search
         */

        if (work)
        {
		if (first > scb_maxvpn(sidx) && first < scb_minvpn(sidx))
			first = scb_minvpn(sidx);

		else if (last > scb_maxvpn(sidx) && last < scb_minvpn(sidx))
			last = scb_maxvpn(sidx);

		/* if the first page number is greater than the
		 * last page number there will not be any resources
		 * to release.
		 */
		
		if (first > last)
			return 0;

        }
        else
        {
                last = MIN(last,scb_maxvpn(sidx));
        }

        /* release the page frames in [first,last].
	 * operation can fail if a page-fixed page is encountered
	 * in a working storage segment.
         */
	if ( rc = v_relpages(sidx,first,last,flags & V_KEEP))
		return rc;

        /* process the xpt for the interval [first,last]
         */
	if (scb_vxpto(sidx) == 0)
		return 0;
        return( v_releasexpt(sidx,first,last,flags & V_KEEP));
}

/*
 * v_relpages(sidx,first,last,keep)
 *
 * releases all pages of the segment whose page number is
 * in the interval [first,last]. pages in i/o state are
 * marked for discard. all others are removed from the
 * hash chain and scb list and put at head of free list.
 *
 * RETURN VALUE
 *
 *      0       - ok
 *	EINVAL  - a page fixed page was encountered in a working
 *		  storage segment but keep is true.
 */

static
v_relpages(sidx,first,last,keep)
int     sidx;   /* index in scb table */
int     first;  /* first page number */
int     last;   /* last page number  */
int	keep;   /* segment is being kept (not deleted) */
{
        int k,nfr,sid, test;

	SCB_LOCKHOLDER(sidx);

        /* if the number of pages is small compared to
         * number of pages in memory process the pages
         * in the interval [first,last] using v_lookup.
         */

	test = (keep == 0) ? 0 : scb_wseg(sidx);
        if (last - first < scb_npages(sidx) >> 3)
        {
                for (k = first; k <= last; k++)
                {
                        if ((nfr = v_lookup(ITOS(sidx,k),BASEPAGE(k))) > 0)
			{
				if (test & pft_inuse(nfr) && pft_pincount(nfr))
					return EINVAL;
                                v_relframe(sidx,nfr);
			}
                }
                return 0;
        }

        /* number of pages is large compared to pages in
         * memory so we scan pages on scb_list and delete
         * those that fall in the interval [first,last].
         */

        for(k = scb_sidlist(sidx); k >= 0; )
        {
                nfr = k;
                k = pft_sidfwd(k);
                if (pft_pagex(nfr) >= first && pft_pagex(nfr) <=  last)
		{
			if (test & pft_inuse(nfr) && pft_pincount(nfr))
				return EINVAL;
                        v_relframe(sidx,nfr);
		}
        }

	return 0;
}

/*
 * v_invalidate(sid,pfirst,npages)
 *
 * invalidates all pages in the specified address range
 * for the segment.
 *
 * RETURN VALUE
 *
 *      0
 */

int
v_invalidate(sid,pfirst,npages)
int     sid;    /* segment id */
int     pfirst; /* first page number */
int     npages; /* number of pages to release */
{
        int sidx,first,last,k,nfr,curkey;
	int sidio, pnoio;

        /* nothing to do ?
         */
        if (npages <= 0)
                return(0);

        /* get index in scb. set last to last page we have to handle
         */
        sidx =  STOI(sid);

	SCB_LOCKHOLDER(sidx);

        first = pfirst;
        last = pfirst + npages - 1;

        /* try to use maxvpn to limit search
         */
	last = MIN(last,scb_maxvpn(sidx));

	for (k = first; k <= last; k++)
	{
		/* if the segment is deferred-update check extension
		 * memory (v_extmem will bring the page in).
		 */
		if (scb_defseg(sidx))
		{
			nfr = v_extmem(sidx,k);
		}
		else
		{
			nfr = v_lookup(ITOS(sidx,k),BASEPAGE(k));
		}

		if (nfr >= 0)
		{
			/* if home disk block is uninitialized
			 * zero-fill page (and leave it modified),
			 * otherwise release the page frame.
			 */
			if (pft_newbit(nfr) && pft_dblock(nfr) != 0)
			{
				/*
				 * zero it.
				 */
				curkey = pft_key(nfr);
				pft_key(nfr) = KERKEY;
				sidio = IOSID(ITOS(sidx,k));
				pnoio = BASEPAGE(k);
				pft_inuse(nfr) = 0;
				P_ENTER(STARTIO,sidio,pnoio,nfr,pft_key(nfr),
					pft_wimg(nfr));
				pft_pagein(nfr) = 1;
				ZEROPAGE(sidio,pnoio);
				pft_pagein(nfr) = 0;
				pft_key(nfr) = curkey;
				P_ENTER(IODONE,ITOS(sidx,k),BASEPAGE(k),nfr,
					pft_key(nfr),pft_wimg(nfr));
				pft_inuse(nfr) = 1;
				SETMOD(nfr);
			}
			else
			{
				v_relframe(sidx,nfr);
			}
		}
	}

	return(0);
}

/*
 * v_relalias(asid,afirst,ssid,sfirst,slast)
 *
 * ping-pong pages aliased at asid back to source ssid
 * that fall in page range [sfirst,slast] of source object
 *
 * non-backtrack critical section
 * VMMSR loaded
 *
 * RETURN VALUE
 *
 *      0
 */
v_relalias(asid,afirst,ssid,sfirst,slast)
int     asid;   /* alias sid */
int     afirst; /* alias starting page number */
int     ssid;   /* source sid */
int     sfirst; /* first page number -- pagex */
int     slast;  /* last page number -- pagex */
{
	int ssidx;
        int k,nfr,apno;

        /* if the number of pages is small compared to
         * number of pages in memory process the pages
         * in the interval [first,last] using v_lookup.
         */
	ssidx = STOI(ssid);

	SCB_LOCKHOLDER(ssidx);

        if (slast - sfirst < scb_npages(ssidx) >> 3)
        {
                for (k = sfirst; k <= slast; k++)
                {
                        if ((nfr = v_lookup(ITOS(ssidx,k),BASEPAGE(k))) > 0)
			{
				/* only ping-pong pages aliased at asid
				 */
				apno = afirst + k - sfirst;
				v_delapt(APTREG,asid,apno,nfr);
				P_REMOVE(asid,apno,nfr);
			}
		}
                return 0;
        }

        /* number of pages is large compared to pages in
         * memory so we scan pages on scb list and ping-pong
         * those that fall in the interval [first,last].
         */

        for(k = scb_sidlist(ssidx); k >= 0; )
        {
                nfr = k;
                k = pft_sidfwd(k);
                if (pft_pagex(nfr) >= sfirst && pft_pagex(nfr) <=  slast)
		{
			apno = afirst + pft_pagex(nfr) - sfirst;
			v_delapt(APTREG,asid,apno,nfr);
			P_REMOVE(asid,apno,nfr);
		}
	}

	return 0;
}

/*
 * v_relframe(sidx,nfr)
 *
 * releases page frame nfr. if page is in i/o state it
 * is marked for discard. otherwise it is removed from
 * the hash chain and current scb list and put on the free
 * list if needed. the tlb is purged.
 */

v_relframe(sidx,nfr)
int     sidx;   /* index in scb table */
int     nfr;    /* page frame number  */
{
        int pno,sid,pinapt;

	ASSERT(!scb_mseg(sidx));

	/* because of v_freiblk/v_purge
	 */
	SCB_LOCKHOLDER(sidx);

        /* if io state mark for discard if this is not
	 * a client page with a pending protection fault.
         */
        if (pft_pagein(nfr) | pft_pageout(nfr))
        {
		if (!pft_pfprot(nfr))
			pft_discard(nfr) = 1;
                return;
        }

        /* Remove all alias page table entries.
         */
	if (pft_alist(nfr) == APTNULL)
		pinapt = 0;
	else
		pinapt = v_delaptall(nfr);

	/* if page was pinned increment count of unpinned pages.
	 * avoid journalled pages for which logage overlays pincount
	 * and avoid pages which have a pinned apt entry (unpinned later).
	 * On MP-eff the pincount is decremented without locking so we may
	 * incorrectly increment pf_pfavail here but this shouldn't happen
	 * often enough to worry about it.
	 */
	if (pft_pincount(nfr) && pft_inuse(nfr) && !scb_jseg(sidx) && !pinapt)
		fetch_and_add(&pf_pfavail, 1);

        /* Remove from the software PFT.
         */
        v_delpft(nfr);

	/* Remove all hardware page table entries.
	 */
	P_REMOVE_ALL(nfr);

        /* remove from scb list. put page frame on the free list,
	 * unless page has a pinned alias page table entry (freed later).
         */
	ASSERT(!pft_free(nfr));
        v_delscb(sidx,nfr);
	if (!pinapt)
		v_insfree(nfr);
}

/*
 * v_releasexpt(sidx,first,last,keep)
 *
 * segment must be a working storage segment.
 * processes the xpt for the interval of pages [first,last]
 * which must be covered by one xpt direct block as follows:
 *
 *     (a) all paging space blocks are freed.
 *
 *     (b) the use counts on map blocks is decremented and
 *         map blocks whose use count is zero are freed.
 *
 *     (c) if keep is true page is logically set to zero
 *         and the storage protect to the default value. 
 *
 *  RETURN VALUES
 *
 *      0       - ok
 *
 */

v_releasexpt(sidx,first,last,keep)
int     sidx;   /* index in scb table */
int     first;  /* first page number */
int     last;   /* last page number  */
int     keep;   /* disposition of disk blocks */
{

        union xptentry * xpt, * v_findxpt();
        int p,pdtx,dblk,mapx,defkey,sysbr, xptdisk, prevcnt;

	SCB_LOCKHOLDER(sidx);

        /* process the interval [first,last] for working segment
         */
        defkey = scb_defkey(sidx);
        sysbr =  scb_sysbr(sidx);
        xpt = v_findxpt(sidx,first);
	ASSERT(xpt && (first >> L2XPTENT) == (last >> L2XPTENT));

        for (p = first; p <= last; p += 1, xpt += 1)
        {
		xptdisk = xpt->cdaddr;
		if (xptdisk == 0 && !keep)
			continue;

                /* mark page as logically zero. set key to default.
                 */
		if (keep)
		{
                	xpt->zerobit = 1;
                	xpt->spkey = (p <= sysbr) ? defkey : pf_kerkey;
		}

                if(xptdisk == 0)
                        continue;

                /* mapblock or disk block ?
                 */
                if (xpt->mapblk)
                {
			/*
			 * Decrement usecount and free it if now 0.
			 * We touch the vmap block to avoid scoreboarding
			 * the lock (vmap blks do not cross page boundary).
			 */
			mapx = xptdisk;

			TOUCH(&pta_vmap[mapx]);

			FETCH_AND_ADD_R(pta_vmap[mapx].count, -1, prevcnt);
			ASSERT(prevcnt > 0);

			if (prevcnt == 1)
			{
				VMAP_MPLOCK();
				pta_vmap[mapx]._u.next = pta_vmapfree;
				pta_vmapfree = mapx;
				VMAP_MPUNLOCK();
			}

			xpt->mapblk = 0;
		}

		/* xpt pointed to disk block. free it.
 		 */
		else
		{
			pdtx = PDTIND(xptdisk);
			dblk = PDTBLK(xptdisk);

			PG_MPLOCK(pdtx);

			v_dfree(pdtx,dblk);

			PG_MPUNLOCK(pdtx);

                        /* update paging space statistics.
                         */
                        scb_npsblks(sidx) += -1;
                        if (PSEARLYALLOC(sidx,p))
                                scb_npseablks(sidx) += 1;
                        else
                                FETCH_AND_ADD(vmker.psfreeblks, 1);
		}

		/* set cdaddr to zero regardless of value of keep
		 * in case of page -fault 
		 */
		xpt->cdaddr = 0;

	}
        return 0;
}

/*
 * v_inherit(psid,pfirst,npages)
 *
 * resources of the parent segment psid for the range of pages
 * specified are passed to the surviving child. all of the
 * pages  must be covered by a single direct xpt block which is
 * NOT NULL for psid , but which may be NULL for the child.
 * this procedure is only called from vms_delete which doesn't
 * bother calling if parent xpt is NULL.
 *
 * each page of the parent is given to the child if the child
 * does not already have the page. otherwise the page is freed
 * i.e. v_released.
 *
 * deletes and inherits are serialized by means of the delete
 * and bit in scbs. an inherit is blocked if the child is in
 * delete state. (see also v_delete)
 *
 * RETURN VALUES
 *
 *      0       - ok
 *
 *      VM_WAIT - child of parent in delete state. this return
 *                value never makes it back to caller. calling
 *                process is v-waited.
 */

v_inherit(psid,pfirst,npages)
int     psid;   /* parent segment id */
int     pfirst; /* first page number */
int     npages; /* number of pages to release */
{
        int csid,csidx,psidx,first,last,p,nfr,childxpt,nchild;
        union xptentry *pxpt, *cxpt, *v_findxpt();
        struct  xptroot *pxptr, *cxptr;
	int rc = 0;

        /* find surviving child, csid.
         */
        psidx = STOI(psid);
	csid = (scb_left(psidx) != 0) ? scb_left(psidx) : scb_right(psidx);
	csidx = STOI(csid);

#ifdef _VMM_MP_EFF
	/*
	 * On MP-eff the lock must be acquired first on the surviving
	 * child and then on the parent.  It's possible for the child
	 * to be deleted and replaced due to an inherit/promote so we
	 * must verify the parent/child relationship after both locks
	 * are held.
	 */
	while(1)
	{
		SCB_MPLOCK_S(csidx);
		SCB_MPLOCK_S(psidx);

		if(scb_left(psidx) == csid || scb_right(psidx) == csid)
			break;

		SCB_MPUNLOCK_S(psidx);
		SCB_MPUNLOCK_S(csidx);

		csid = (scb_left(psidx) != 0) ?
				scb_left(psidx) : scb_right(psidx);
		csidx = STOI(csid);
	}
#endif /* _VMM_MP_EFF */

	SCB_LOCKHOLDER(csidx);
	SCB_LOCKHOLDER(psidx);

        /* if the child delete bit is set we have to wait unless
         * the delete bit was set by a previous call to this
         * procedure. we detect the latter by the number of children
         * the child has: if it is one it was set by v_delete and
         * if it is 2 or 0 by us.
         */

        if (scb_delete(csidx))
        {
                nchild = (scb_left(csidx)) ? 1 : 0;
                nchild = (scb_right(csidx)) ? nchild + 1 : nchild;
                if (nchild == 1)
                {
                        v_wait(&pf_deletewait);
                        rc = VM_WAIT;
			goto out;
                }
        }

        /* we set the delete bit in the child to block deletes
         * of his children (if any).
         */

        scb_delete(csidx) = 1;

	if (npages <= 0)
		goto out;

        /* use maxvpn and minvpn to limit search.
         */

        first = pfirst;
        last = pfirst + npages - 1;

	if (first > scb_maxvpn(psidx) && first < scb_minvpn(psidx))
		first = scb_minvpn(psidx);

	else if (last > scb_maxvpn(psidx) && last < scb_minvpn(psidx))
		last = scb_maxvpn(psidx);

        /* process the pages in the interval [first,last].
         * in the loop parent's page frames in memory are
         * handled but his xpt is dealt with both in the
         * loop and at loop exit. when the child had no
         * xpt he inherits it at loop exit. otherwise all
         * xpt entries which are not inherited by the child
         * in the loop are handled at loop exit.
         */

        /* get xpt pointers for parent and child.
         */
        pxpt = v_findxpt(psidx,first);
        cxpt = v_findxpt(csidx,first);
        childxpt = (cxpt != NULL);

        for (p = first; p <= last; p++ , pxpt++, cxpt++)
        {
                /*
                 * anything to inherit ?
                 */
                if (pxpt->word == 0)
                        continue;

                nfr = v_lookup(psid,p);

                /* does child already have page ?
                 * if so release parents page frame.
                 */

                if (childxpt)
                {
                        if (cxpt->word != 0)
                        {
                                if (nfr > 0)
                                        v_relframe(psidx,nfr);
                                continue;
                        }

                        /* child didn't have page. he gets parents xpt.
                         */
                        cxpt->word = pxpt->word;
                        pxpt->word = 0;
			if (!cxpt->mapblk && cxpt->cdaddr)
			{
				scb_npsblks(csidx) += 1;
				scb_npsblks(psidx) += -1;
                                if (PSEARLYALLOC(csidx,p))
                                        scb_npseablks(csidx) += -1;
                                if (PSEARLYALLOC(psidx,p))
                                        scb_npseablks(psidx) += 1;
			}
                }
		
		/*
		 *  child didn't have xpt and will receive parents direct
		 *  xpt block in a subsequent step.  adjust scb disk block
		 *  counts for both now to reflect the tranfer.
		 */
		else
		{
			if (!pxpt->mapblk && pxpt->cdaddr)
			{
				scb_npsblks(csidx) += 1;
				scb_npsblks(psidx) += -1;
                                if (PSEARLYALLOC(csidx,p))
                                        scb_npseablks(csidx) += -1;
                                if (PSEARLYALLOC(psidx,p))
                                        scb_npseablks(psidx) += 1;
			}
		}

                /* child inherits  page frame unless i/o state.
                 */
                if (nfr < 0)
                        continue;
                if (pft_pagein(nfr) | pft_pageout(nfr))
                {
                        pft_discard(nfr) = 1;
                        continue;
                }

		ASSERT(!pft_free(nfr));

                v_delpft(nfr);
                v_delscb(psidx,nfr);
                P_RENAME(csid,p,nfr,pft_key(nfr),pft_wimg(nfr));
                pft_ssid(nfr) = csid;
		v_inspft(csid,p,nfr);
                v_insscb(csidx,nfr);
        }

        /* clean up what's left of parents xpt if child had xpt
         */
        if (childxpt)
        {
                v_releasexpt(psidx,first,last,V_NOKEEP);
		goto out;
        }

        /* child did not have xpt. we take the direct xpt block
         * from parent and give it to child. it is necessary
         * to set parents root pointer to NULL so that when parent
         * is deleted we don't free the xpt block.
         */

        pxptr = (struct xptroot *) scb_vxpto(psidx);
        cxptr = (struct xptroot *) scb_vxpto(csidx);
        p = pfirst >> L2XPTENT;  /* index in root */
        cxptr->xptdir[p] = pxptr->xptdir[p];
        pxptr->xptdir[p] = NULL;

out:
	SCB_MPUNLOCK_S(psidx);
	SCB_MPUNLOCK_S(csidx);
        return(rc);
}

/*
 * v_flush(sid,pfirst,npages)
 *
 * pageout is initiates for all modified pages within the range
 * of virtual addresses and all pages frames in memory associated. 
 * with the range are discarded. the pages for which pageout is
 * initiated will be discarded upon i/o completion.
 *
 * this procedure runs on fixed stack at VMM interrupt level with
 * back track enabled. this procedure is used by vm_flushp.
 *
 * RETURN VALUE
 *
 *      0       - ok
 *
 */

int 
v_flush(sid,pfirst,npages)
int     sid;    /* segment id */
int     pfirst; /* first page number */
int     npages; /* number of pages to flush */
{
        int rc, sidx, first, last, nfr, p, k;

        /* nothing to do ?
         */
        if (npages <= 0)
                return(0);

	sidx = STOI(sid);
	SCB_LOCKHOLDER(sidx);

#ifndef _POWER_MP

	/* initiate pageout for the page range.
	 */
	if (rc = v_write(sid,pfirst,npages,V_FORCE))
		return(rc);
	
	/* release the pages in the page range.
	 */
	return(v_release(sid,pfirst,npages,V_KEEP));
#else
        first = pfirst;
	last = pfirst + npages - 1;
	last = MIN(last,scb_maxvpn(sidx));

        /* we have to check extension memory if the segment
         * is a deferred-update segment.
         */
        if(scb_defseg(sidx))
        {
                for(p = first; p <= last; p++)
                {
                        /* v_extmem checks if page is in extension
                         * memory. if so it touches page to bring
                         * it in and establishes home address in pft.
                         */
                        if ((nfr = v_extmem(sidx,p)) >= 0)
                        {
                                write_relp(sidx,nfr);
                        }
                }

		return 0;
	}	

	/*
         * if the number of pages is small compared to
         * number of pages in memory process the pages
         * in the interval [first,last] using v_lookup.
         */

        if (last - first < scb_npages(sidx) >> 3)
        {
                for (k = first; k <= last; k++)
                {
                        if ((nfr = v_lookup(ITOS(sidx,k),BASEPAGE(k))) >= 0) 
				write_relp(sidx,nfr);
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
                        if (pft_pagex(nfr) >= first && pft_pagex(nfr) <= last)
				write_relp(sidx,nfr);
                }
        }

        return(0);
#endif

}
#ifdef _POWER_MP

/* write_relp(sidx,nfr)
 * 
 * This is called by v_flush. The specified page frame
 * is examined for modification. If the frame is modified
 * it is written out and discarded. If it is not modified 
 * the frame is released.
 * 
 * It is necessary on MP to do this atomically, since
 * a page could be modified right after the check for modbit.
 * This is achieved by removing the addressability of the
 * page and then testing the modification bit.
 */
int
write_relp(sidx, nfr)
int sidx;
int nfr; 
{

	int sidio, pnoio;

	if (!pft_inuse(nfr))
		return 0;

        sidio = IOSID(pft_ssid(nfr));
        pnoio = pft_spage(nfr);


	/* Hide the page to examine the state
 	 */	

	pft_inuse(nfr) = 0;
        P_ENTER(STARTIO,sidio,pnoio,nfr,pft_key(nfr),pft_wimg(nfr));
        pft_pageout(nfr) = 1;
 
	/* Check the modbit. If modified, write it out and
	 * mark for discard after I/O. If the page is not
	 * modified release the frame.
	 */ 

	if (ISMOD(nfr))
	{
		
		v_pageout(sidx,nfr,NOFBLRU);
		pft_discard(nfr) = 1;
	}
	else
	{
        	pft_pageout(nfr) = 0;
		v_relframe(sidx, nfr);
	}

}
#endif 
/*
 * v_inactive(sid)
 *
 * marks the segment as inactive by setting scb_inactive;
 * new page faults within the segment will result in exceptions.
 *
 * this procedure runs on fixed stack at VMM interrupt level without
 * back track enabled. this procedure is used by vms_inactive.
 *
 * RETURN VALUE
 *
 *      0       - ok
 *
 */

int 
v_inactive(sid)
int     sid;    /* segment id */
{
        int sidx;

	/* mark segment as inactive.
	 */
	sidx = STOI(sid);

	SCB_LOCKHOLDER(sidx);

	scb_inactive(sidx) = 1;
	
	return(0);
}
