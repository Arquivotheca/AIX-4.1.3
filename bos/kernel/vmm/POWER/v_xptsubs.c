static char sccsid[] = "@(#)83	1.24  src/bos/kernel/vmm/POWER/v_xptsubs.c, sysvmm, bos41J, 9516A_all 4/17/95 15:19:05";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: 	v_getxpt, v_freexpt, v_growxpt, v_findxpt
 *		v_getvmap
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

#include "vmsys.h"
#include <sys/errno.h>
#include <sys/syspest.h>
#include "mplock.h"

/*
 * NAME: v_getxpt
 *
 * FUNCTION: allocate an XPT block
 *
 * size of block to be allocated is specified by giving index of
 * free list anchor (use the defines XPT128,...XPT4K). a 4k block
 * may be allocated but later freed in smaller units provided ALL
 * of the subsequent frees are of the same size. the allocated
 * block is touched here if V_TOUCH is specified.
 *
 * EXECUTION ENVIRONMENT:
 *
 *     VMM Critical Region : Yes
 *     Runs on Fixed Stack : Yes
 *     May Page Fault      : Yes
 *     May Backtrack       : Yes
 *
 *      VMM Data Segment ID is loaded in : VMMSR
 *      PTA Segment ID is loaded in      : PTASR
 *
 * NOTES:
 *
 *      1. v_getxpt  does not validate its input parameters
 *      nor does it initialize the XPT block.
 *
 * RETURN VALUE DESCRIPTION:
 *
 *  XPT block address:  address of the XPT block that is allocated
 *                      or zero if there is no space.
 */

int  v_getxpt (blk_size,touch)
uint blk_size;    /* specifies free list to use  */
uint touch;   /* 0 ==> DO NOT touch XPT blk. 1 ==> touch XPT blk */
{
        uint    blk;                    /* free block number */
        uint    idx;                    /* index into Area Page Map */
        uint    nxt;


	ALLOC_LOCKHOLDER();

        /* restart from top if all free-lists were empty */
        loop:

        idx = pta_anchor[blk_size];
        /* is free-list for blk_size non-null ? */
        if ( idx != NULL)
        {
                TOUCH (&pta_apm[idx]);
                if (touch)
                        TOUCH (&pta_page[idx]);

        /*
         * touch the next APM entry in case we allocate the last
         * block on the page. if it is last it will be removed
         * from its free list and next will become head of list.
         */
                nxt = pta_apm[idx].fwd;
                TOUCH(&pta_apm[nxt]);
                return (allocxpt(blk_size));  /* allocate it */
        }

        /*
         * blk_size free list was empty. if 4k list is not empty
         * we take a block from there and move it to the blk_size
         * list.
         */

        idx = pta_anchor[XPT4K];
        if (idx != NULL)
        {
                /* note that blk_size != 4k if we are here */
                TOUCH (&pta_apm[idx]);

                /* touch next apm (we have to move idx). */
                nxt = pta_apm[idx].fwd;
                TOUCH (&pta_apm[nxt]);

                /* touch the page if requested */
                if (touch)
                        TOUCH(&pta_page[idx]);

                /* move idx to head of empty list for blk-size */
                delapm(XPT4K,idx);
                pta_anchor[blk_size] = idx;
                pta_apm[idx].fwd = pta_apm[idx].bwd = NULL;

                /* allocate the block */
                return(allocxpt(blk_size));
        }

        /*
         * both lists (or just 4k) are empty.
         */

        /* any more space ? */
        if (pta_hiapm >= SEGSIZE/PSIZE)
                return(0);

        /* get a page worth of apms. first call to getapm will
        page fault and cause backtrack.                        */

        getapm();
        goto loop;  /* try again */
}

/*
 * NAME: v_freexpt
 *
 * FUNCTION: free an XPT block
 *
 * this routine frees the specified XPT block. if page (containing
 * the XPT block) becomes FREE (no blocks allocated), then its APM
 * is moved to the 4Kb free-list. if the number of blocks on the
 * 4Kb free-list (pta_freecnt) >= APM4KFREE, the resource associated
 * with the block are freed and the block is placed at the tail of
 * the 4Kb free-list; otherwise, the block is placed at the head of
 * 4Kb free-list and the resources are not released. if page was FULL
 * its APM is inserted on the free list corresponding to the size
 * specified. the block being freed normally has the same size as
 * the size with which it was allocated. however the block may be
 * less in the case when the storage was initially allocated in a
 * 4k unit and all frees from the 4k block have the same size.
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *     VMM Critical Region : Yes
 *     Runs on Fixed Stack : Yes
 *     May Page Fault      : Yes
 *     May Backtrack       : Yes
 *
 *      VMM Data Segment ID is loaded in : VMMSR
 *      PTA Segment ID is loaded in      : PTASR
 *
 * NOTES:
 *
 *      1. v_freexpt() does not touch the page mapped by the APM
 *         descriptor .
 *
 * RETURN VALUE DESCRIPTION:    None
 *
 */

int v_freexpt (blk_size, blk_addr)
uint    blk_size;       /* block size - specifies free-list to use */
uint    blk_addr;       /* address of XPT block to free            */
{
        uint    blk;                  /* block number to free            */
        uint    fidx;                 /* APM index that maps blk to free */
        int     nxt,prv,ank,offset;
	int     released;
	uint	pno, sid, sidx;
	union xptentry *v_findxpt();

	ALLOC_LOCKHOLDER();

        /* get the index of the apm covering the block */
        fidx = (blk_addr & SOFFSET) >> L2PSIZE;  /* apm index */

	/* ensure valid page number for XPT block */
	assert(fidx >= FIRSTAPM);

        /* calculate its block number within page */
        offset = blk_addr & POFFSET;
        blk = offset >> (L2PSIZE - blk_size); /* block in page */

        /* was it full ? (also touches the apm)  */
        if (pta_apm[fidx].pmap == ONES)
        {
                /*
                 * mark page as "full" for blk_size. note
                 * that we might backtrack having made the
                 * update. this is ok : the page is marked as
                 * full but possibly in a different way.
                 */
                pta_apm[fidx].pmap = SZBIT >> ((1 << blk_size) - 1);
        }

        /* will the page become free ? if so move to 4k free list */

        if (RESETBIT(pta_apm[fidx].pmap,blk) == 0)
        {
                /* touch everything needed to insert on 4k free-list*/
                ank = pta_anchor[XPT4K];
                TOUCH(&pta_apm[ank]);

		/*
		 * Touch everything needed to free 4KB block.  Need to
		 * touch APM at the tail of the free list as well as the
		 * XPT structure for the PTA page to be released.
		 */
		if (pta_freecnt >= APM4KFREE)
		{
			TOUCH(&pta_apm[pta_freetail]);

			pno = ((uint)&pta_page[fidx] & SOFFSET) >> L2PSIZE;
			sid = SRTOSID(vmker.ptasrval);
			sidx = STOI(sid);
			TOUCH(v_findxpt(sidx,pno));
		}

                /* apm is on a list if it isn't 4k. we have to
                delete it from that list if so.           */

                if (blk_size != XPT4K)
                {
                        /* touch next and previous for delapm */
                        nxt = pta_apm[fidx].fwd;
                        TOUCH(&pta_apm[nxt]);
                        prv = pta_apm[fidx].bwd;
                        TOUCH(&pta_apm[prv]);
                        delapm(blk_size,fidx);
                }

		/* check if the number of free 4k block >= APM4KFREE.
		 * if so, call v_release() to release the resources
		 * associated with the block.
		 */
		released = 0;
		if (pta_freecnt >= APM4KFREE)
		{
			/* Those pages are the disk block pages of a segment
			 * that is deleting and nobody can access them
			 * (and the scb lock of the deleting scb is held).
			 * So in mp, the scb lock of the pta seg is needed
			 * to protect the slist but it is a scb leaf lock.
			 */
			SCB_MPLOCK_S(sidx);
			v_release(sid,pno,1,V_KEEP);
			SCB_MPUNLOCK_S(sidx);
			released = 1;
		}

                /* move it to 4k free list */
                insapm(XPT4K,fidx,released);
                pta_apm[fidx].pmap = 0;  /* mark page as free */
                return;
        }

        /*
         * page will not become free. if it is currently FULL
         * (not on any list) move it to the head of its free list.
         */

        if (FULLPMAP(blk_size,pta_apm[fidx].pmap))
        {
                ank = pta_anchor[blk_size];
                TOUCH(&pta_apm[ank]);
                insapm(blk_size,fidx,0);   /* insert onto free-list */
        }

        /* APM entry is now (or already was) on a free-list  */

        pta_apm[fidx].pmap = RESETBIT(pta_apm[fidx].pmap,blk);

}

/*
 * NAME: allocxpt
 *
 * FUNCTION: allocate an XPT block
 *
 * This routine allocates the specified sized XPT block.
 * It is only called from v_getxpt() after all the data
 * structures needed to allocate the block are in memory.
 *
 * RETURN VALUE DESCRIPTION:
 *
 * XPT block address:  address of the XPT block that is allocated
 *
 */

static int
allocxpt (blk_size)
uint   blk_size;               /* XPT block size to allocate */
{
        uint   blk;             /* block in page to allocate          */
        uint   idx;             /* index into Area Page Map           */
        uint   xp;              /* addr of allocated XPT block     */

	ALLOC_LOCKHOLDER();

        idx = pta_anchor[blk_size];

        /* special-case 4Kb block size */

        if (blk_size == XPT4K)
        {
                pta_apm[idx].pmap = ONES; /* mark as full */
                delapm(blk_size,idx);
                return((int) &pta_page[idx]);
        }

        /* blk_size is NOT 4K, so find the first free block in page. */
        POSZBIT(blk,pta_apm[idx].pmap);
        pta_apm[idx].pmap = SETBIT(pta_apm[idx].pmap,blk);

        /* if last block in page, delete APM from free-list */
        if (FULLPMAP(blk_size,pta_apm[idx].pmap))
        {
                pta_apm[idx].pmap = ONES; /* mark as full */
                delapm(blk_size,idx);
        }

        xp = (int) &pta_page[idx] + (blk << (L2PSIZE - blk_size));
        return(xp);
}

/*
 * NAME:  v_growxpt
 *
 * FUNCTION: extend a segment's external page table. 
 *
 * EXECUTION ENVIRONMENT:
 *
 *     VMM Critical Region : Yes
 *     Runs on Fixed Stack : Yes
 *     May Page Fault      : Yes
 *     May Backtrack       : Yes
 *
 *      VMM Data Segment ID is loaded in : VMMSR
 *      PTA Segment ID is loaded in      : PTASR
 *
 * EXTERNAL PROCEDURES CALLED   v_getxpt
 *
 * NOTES:
 *
 *      1.  v_growxpt() initializes the xpt block to zeros.
 *
 * RETURN VALUE DESCRIPTION:
 *
 *            0         :  ok
 *
 *            ENOMEM    :   out of pta space.
 */

int v_growxpt(sidx, pno)
uint   sidx;   /* index of scb */
uint   pno;    /* virtual page number to cover.   */
{

        int     k,n;
        uint    newxpt;
        struct  xptroot *xptr;

	SCB_LOCKHOLDER(sidx);

	/* check that it isn't already allocated. this
	 * is necessary because growxpt is called from
	 * vmforkcopy as well as when a page fault occurs.
	 */
        xptr = (struct xptroot *) scb_vxpto(sidx);
        n = pno >> L2XPTENT; 
	if (xptr->xptdir[n] != NULL)
		return 0;

        /* allocate xpt direct block. 
         */
	ALLOC_MPLOCK_S();
        if ((newxpt = v_getxpt(XPT1K,V_TOUCH)) == NULL)
	{
		ALLOC_MPUNLOCK_S();
                return(ENOMEM);
	}
	ALLOC_MPUNLOCK_S();

        /* set pointer in root to the new xpt block.
	 * init to zeros. 
 	 *
	 * NOTE: block_zero requires
	 * cache aligned address and range
	 * 
         */
	block_zero(newxpt, 1024);
        xptr->xptdir[n] = (struct xptdblk *) newxpt;

        return(0);
}


/*
 * NAME: getapm
 *
 * FUNCTION:  initialize a new page of APM desciptors
 *
 *        this routine is called when the 4Kb free-list is empty
 *        from v_getxpt. it will allocate a page of apms and
 *        chain them to the 4kb free list and then increment hiapm.
 *        v_getxpt checks hiapm is ok.
 *
 * NOTES:
 *      1. The "pmap" field in  APM descriptors are NOT initialized
 *         explicitly by getapm() since a 1st reference page fault
 *         will cause the page to be zeroed -- zero is the value
 *         wanted for pmap to indicate zero blocks allocated.
 *
 */

static
getapm ()
{
        uint    head;           /* index of head of 4Kb free-list */
        uint    idx;            /* index into Area Page Map       */
        uint    maxidx;         /* max APM index in the new page  */

	ALLOC_LOCKHOLDER();

        /* touch the new page of APM entries -- always faults first time */

        TOUCH (&pta_apm[pta_hiapm]);

        /* NO MORE PAGE FAULTS ALLOWED */

        head = pta_anchor[XPT4K] = pta_hiapm;  /* init free-list anchor */
        pta_hiapm += APMENT;    /* increment by number of apms per page */
        maxidx = head + APMENT - 1;   /* max APM index in page */

        /* link all of the descriptors together */
        pta_apm[head].bwd = NULL;           /* set head's bwd pointer */

        for (idx = head; idx < maxidx; idx++)
        {
                pta_apm[idx].fwd = idx + 1;
                pta_apm[idx + 1].bwd = idx;
        }

        pta_apm[idx].fwd = NULL;       /* set tail's fwd pointer */
	pta_freetail = idx;	       /* set tail of 4k free list */
}

/*
 * NAME: insapm
 *
 * FUNCTION: if resources have been released (4k block only), insert
 *	     an APM descriptor at the tail of the free-list.  if not
 *	     released, insert descriptor at the head of the specified
 *	     free-list.  this routine is only called by v_freexpt()
 *	     which has touched the apm at the head of the list and, in
 * 	     the case of tail insertion, the apm at the tail of the 
 *	     list prior to the call, so this won't fault
 *
 * RETURN VALUE DESCRIPTION:    None
 *
 */

static
insapm (blk_size,newhead,released)
uint    blk_size;       /* XPT block size - specifies free-list to use */
uint    newhead;        /* index of APM entry to insert (new head) */
int     released;       /* 1 if XPT4K resources released - 0 otherwise */
{

        uint    oldhead;

	ALLOC_LOCKHOLDER();

	/* if resources were released, insert it at the
	 * tail of the free list.
	 */
	if (released)
	{
        	pta_apm[pta_freetail].fwd = newhead;
        	pta_apm[newhead].bwd = pta_freetail;
		pta_freetail = newhead;
		return;
		
	}

        oldhead = pta_anchor[blk_size];
        pta_anchor[blk_size] = newhead;
        pta_apm[newhead].fwd = oldhead;
        pta_apm[newhead].bwd = NULL;

        if (oldhead != NULL)
                pta_apm[oldhead].bwd = newhead;

	/* if 4k block, adjust count of free 4k blocks with
	 * resources.  set freetail if 4k free list was empty.
	 */
	if (blk_size == XPT4K)
	{
		pta_freecnt += 1;

		if (oldhead == NULL)
			pta_freetail = newhead;
	}

}

/*
 * NAME: delapm
 *
 * FUNCTION: delete an APM descriptor from the specified free-list
 *
 * NOTES:
 *
 *      1. The caller of this routine must fault in the APM descriptor
 *         to delete, AND if that descriptor is NOT the only one on the
 *         free-list, then the caller must also fault in the next and
 *         previous descriptors on the list.
 *
 *      2. fwd and bwd fields in are deleted apm are set to null.
 *
 * RETURN VALUE DESCRIPTION:    None
 *
 */

static
delapm (blk_size,idx)
uint    blk_size;       /* index of anchor of free list */
uint    idx;            /* index of APM entry to delete */
{
        uint  nxt, prv;

	ALLOC_LOCKHOLDER();

        nxt = pta_apm[idx].fwd;
        prv = pta_apm[idx].bwd;

        if (prv != NULL)
                pta_apm[prv].fwd = nxt;  /* not at head of list*/
        else
                pta_anchor[blk_size] = nxt;  /* was head of list */

        if (nxt != NULL)
                pta_apm[nxt].bwd = prv;    /* wasn't tail of list */

        /* set fwd and bwd to NULL */
        pta_apm[idx].bwd = pta_apm[idx].fwd = NULL;

	/* if 4k block with allocated resources, adjust 
	 * free resource count.
	 */
	if (blk_size == XPT4K && pta_freecnt > 0)
	{
		pta_freecnt += -1;
	}
}

/*
 * v_findxpt(sidx,pno)
 *
 * on entry the PTA segment is assumed to be addressable.
 * return the address of the xpt entry for page specified,
 * or NULL (0) if there is no xpt for it.
 */

union xptentry *  v_findxpt(sidx,pno)
int     sidx;   /* index in scb table */
int     pno;    /* page number in segment */
{
        int k,n;
        struct xptroot * xptr;
        struct xptdblk * xptd;

	k = pno >> L2XPTENT;  /* index in root */
	xptr = (struct xptroot *) scb_vxpto(sidx);
	xptd = xptr->xptdir[k];
	if (xptd != NULL)
	{
		n = pno & (XPTENT - 1);  /* index in direct block */
		return(&xptd->xpte[n]);
	}
	return(NULL);
}

/*
 * v_getvmap()
 *
 * allocate a vmapblk and return its index. a return value
 * of zero indicates that the pta segment is out of space.
 *
 * Return value : index of vmapblk or 0 if no space.
 */

v_getvmap()
{
        int k,n,nb,ptr,offset;

	VMAP_LOCKHOLDER();
	
        /* take first on free list if not-null
         */

        if(pta_vmapfree)
        {
                k = pta_vmapfree;
                pta_vmapfree = pta_vmap[k]._u.next;
                return(k);
        }

        /* allocate 1k bytes for a block of vmap blocks.
         * the number in the block should be 64.
         */
	ALLOC_MPLOCK_S();
        if  ((ptr = v_getxpt(XPT1K,V_TOUCH)) == 0)
	{
		ALLOC_MPUNLOCK_S();
                return(0);
	}
	ALLOC_MPUNLOCK_S();

        /* put on free list. allocate first one.
         */
        offset = ptr & SOFFSET;
        k  = offset/sizeof(struct vmapblk); /* index of first */
        nb = 1024/sizeof(struct vmapblk);
        for(n = k + 1; n < k + nb - 1; n ++)
        {
                pta_vmap[n]._u.next = n + 1;
        }

        pta_vmap[n]._u.next = 0;
        pta_vmapfree = k + 1;
        return(k);
}
