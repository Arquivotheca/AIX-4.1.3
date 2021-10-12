static char sccsid[] = "@(#)61	1.23.1.16  src/bos/kernel/vmm/POWER/v_segsubs.c, sysvmm, bos41J, 9519A_all 5/9/95 15:55:41";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	v_create, v_freeseg, v_freescb, v_promote, v_pscount
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
#include <sys/trchkid.h>
#include <sys/syspest.h>
#ifdef _VMM_MP_EFF
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#endif /* _VMM_MP_EFF */
#include "mplock.h"

/*
 * v_create(sid,type,device,size)
 *
 * VMM critical section code used to create a segment.
 * this program works in conjunction with vms_create.
 * see vms_create for description of parameters which
 * are checked for validity there.
 *
 * FUNCTION
 *
 *  (1) allocate a free segment control block. return
 *      if it is marked delete pending. vms_create will
 *      delete the segment and call again.
 *
 *  (2) allocate a root block xpt for working storage
 *      segments. the xpt is not initialized.
 *
 *  (3) clear scb except for pointer to the xpt and
 *      if the segment is not a working segment, the
 *      scb_devid field.
 *
 *   (4) if the segment is a persistent logtype segment
 *       the segment is entered into the pf_logsidx
 *       array.
 *
 * this program runs at VMM interrupt level with back-tracking
 * enabled. it is only called by vms_create.
 *
 * RETURN values
 *
 *   0      - ok  (but sid may have to be deleted)
 *
 *   ENODEV - device is not in pdt table
 *
 *   ENOMEM - out of sids or xpt space.
 */

v_create(sid,type,device,size)
int * sid;      /* set to sid of created segment */
int   type;     /* type of segment to create     */
union {
        dev_t   dev;    /* dev_t of block device */
        int ( *ptr)();  /* pointer to strategy routine */
        } device;
int   size;     /* user region size or size of initial xpt */
{
        int rc,sidx,xptsize,xptaddr,basetype,pdtx,log,devtype;
	int k,m;

	ALLOC_LOCKHOLDER();

	/* first look for a delete pending scb with a zero
	 * xmem attach count.
	 * In mp no need to lock the scb's since xmemcnt cannot
	 * increase.
	 */
	m = 0;
	for (k = pf_sidxmem; k > 0; k = scb_free(k))
	{
		if (scb_xmemcnt(k) == 0)
		{
			*sid = ITOS(k,0);
			if (m > 0)
				scb_free(m) = scb_free(k);
			else
				pf_sidxmem = scb_free(k);
			return 0;
		}
		m = k;
	}

        /* check that an scb is available.
         */
        if ( (sidx = pf_sidfree)  < 0)
        {
                if (rc = getscbs())
                        return(rc);
		sidx = pf_sidfree;
        }

	/* touch sid now in case of page-fault
	 */
	TOUCH(sid);

        /* xpt needed for working segments. check device parameter
         * for other types.
         */
        log = -1;
        basetype = type & ( V_WORKING | V_PERSISTENT | V_CLIENT | V_MAPPING );
        switch(basetype)
        {
        case V_WORKING:
                if( (xptaddr = v_getxpt(XPT1K,V_NOTOUCH)) == 0 )
                        return(ENOMEM);
                pdtx = 0;
                break;

        case V_PERSISTENT:
                /* make sure device was mounted
                 */
                devtype = ( type & V_LOGSEG) ? D_LOGDEV : D_FILESYSTEM;
                if ((pdtx = v_devpdtx(devtype,device)) < 0)
		{
                        return(ENODEV);
		}

                /* if its a logseg check space available in pf_logsidx.
		 * Need the lw lock to protect pf_logsidx[].
		 * we keep it until after bzero so that v_sync() will work
		 * and so that the entry is still free.
                 */
                if (type & V_LOGSEG)
                {
			LW_MPLOCK();
                        for(log = 0; log < MAXLOGS ; log++)
                        {
                                if(pf_logsidx[log] == 0)
                                        break;
                        }

                        if (log == MAXLOGS)
			{
				LW_MPUNLOCK();
                                return(ENOMEM);
			}
                }
                xptaddr = 0;
                break;

        case V_CLIENT:
                /* make sure device was mounted
                 */
                if ((pdtx = v_devpdtx(D_REMOTE,device)) < 0)
		{
                        return(ENODEV);
		}

                xptaddr = 0;
		break;

	case V_MAPPING:
		 /* Mapping segments don't have XPTs and don't own any
		  * pages (used soley for addressing purposes).
		  */
                xptaddr = 0;
		pdtx = 0;
		break;
        }

        /* allocate scb and zero it.
	 * No need to lock the scb.
         */
        pf_sidfree = scb_free(sidx);
#ifndef _VMM_MP_EFF
        bzero(&vmmdseg.scb[sidx],sizeof(struct scb));
#else  /* _VMM_MP_EFF */
	/* we don't want to zero the scb lock which may be held by
	 * the lru when scanning the scb's. This must also be done
	 * when the locks are instrumented since the lock field 
	 * is no longer a lockword but a pointer.
	 * NOTE the lock MUST BE the last field of the scb struct.
	 */
        bzero(&vmmdseg.scb[sidx],sizeof(struct scb) - sizeof(Simple_lock));
#endif /* _VMM_MP_EFF */

        /* fill in fields in scb
	 * setting of valid bit is postponed to the end of vms_create.
	 */
        scb_vxpto(sidx) = xptaddr;
        scb_devid(sidx) = pdtx;

	/* for persistent or client segments fill in serial number
         */
        if (basetype != V_WORKING)
        {
                pf_nxtscbnum += 1;
                scb_serial(sidx) = pf_nxtscbnum;

        }

        /* if  a log put in pf_logsidx array
         */
        if (log >= 0)
	{
                pf_logsidx[log] = sidx;
		LW_MPUNLOCK();
	}

	/* set sid and return
	 */
	*sid = ITOS(sidx,0);
        return(0);
}

/*
 * getscbs()
 * add a page of scbs to free list and page fix them.
 * returns 0 OK, ENOMEM if not enough scbs.
 *
 * note:
 * do not assume that scbs are page aligned nor power of 2 in size.
 * do assume that scbs are less than a page in size.
 */

static int
getscbs()
{
        int k,first,last,nscbs,sid,pno;

	ALLOC_LOCKHOLDER();

        /* calculate the number of scbs to add -- bound by one page,
         * and limit sid index to (NUMSIDS - 1).
         */

        nscbs = MIN( PSIZE/sizeof(struct scb), NUMSIDS - pf_hisid);
        if (nscbs <= 0 )
                return(ENOMEM);

        /* add scbs to free list (touches them)
         */
        first = last = -1;
        for(k = pf_hisid; k < pf_hisid + nscbs; k++)
        {
                /* don't use scb if its sid is out of range
                 */
                if (ITOS(k,0) < SIDLIMIT)
                {
#ifdef _VMM_MP_EFF
			/*
			 * Initialize SCB lock with no instrumentation
			 * (allocation for the instrumentation must be done
			 * at base level).  SCB locks are initialized when
			 * first allocated and from that point on the lock
			 * word cannot be reset because an SCB lock may be
			 * acquired even when the SCB has been deleted.
			 */
			*((simple_lock_data *) &scb_lock(k))=SIMPLE_LOCK_AVAIL;
#endif /* _VMM_MP_EFF */
                        if (first < 0)
                        {
                                first = last = k;
                        }
                        else
                        {
                                scb_free(last) = k;
                                last = k;
                        }
                }
        }
        scb_free(last) = -1;

        /* page fix them.
         */
        sid = SRTOSID(vmker.vmmsrval);
        pno = (((uint)&vmmdseg.scb[last+1] & SOFFSET) - 1) >> L2PSIZE;

	/* WARNING: in mp, the scb lock of the vmmdseg is not needed
	 * (and would induce a lock hierarchy pb) since nobody will try
	 * to steal it (protected against lru by touch in a cs).
	 */
        if (v_pin(sid,pno,LONG_TERM))
                return(ENOMEM);

        pf_sidfree = first;    /* ok to do this now */
        pf_hisid += nscbs;

        return(0);
}

/*
 * getames()
 * add a page of ames to free list and page fix them.
 * returns 0 OK, ENOMEM if not enough ames.
 *
 * note:
 * do not assume that ames are page aligned nor power of 2 in size.
 * do assume that ames are less than a page in size.
 */

static int
getames()
{
        int k,first,last,numames,sid,pno;

	AME_LOCKHOLDER();

        /* calculate the number of ames to add -- bound by one page,
         * and limit ame index to (NUMAMES - 1).
         */

        numames = MIN( PSIZE/sizeof(vmmdseg.ame[0]), NUMAMES - pf_hiame);
        if (numames <= 0 )
                return(ENOMEM);

        /* add ames to free list (touches them)
	 * pf_amefree anchors list and contains index of 1st free ame,
	 * 1st word of each ame on list contains index of next free ame,
	 * last ame on list has index -1.
         */
        first = last = -1;
        for(k = pf_hiame; k < pf_hiame + numames; k++)
        {
		if (first < 0)
		{
			first = last = k;
		}
		else
		{
			ame_free(last) = k;
			last = k;
		}
        }
        ame_free(last) = -1;

        /* page fix them
         */
        sid = SRTOSID(vmker.vmmsrval);
	if (first == 0)
	{
		/* first page of ames might span 2 page frames
		 * so we need to make sure first of these gets
		 * pinned too.  don't have this problem with scbs
		 * since 1st entries get pinned in vmsi.
		 */
		pno = (((uint)&vmmdseg.ame[0] & SOFFSET) - 1) >> L2PSIZE;
		v_pin(sid,pno,LONG_TERM);
	}
        pno = (((uint)&vmmdseg.ame[last+1] & SOFFSET) - 1) >> L2PSIZE;

	/* WARNING: in mp, the scb lock of the vmmdseg is not needed
	 * (and would induce a lock hierarchy pb) since nobody will try
	 * to steal it (protected against lru by touch in a cs).
	 */
        if (v_pin(sid,pno,LONG_TERM))
		return(ENOMEM);
	
        pf_amefree = first;    /* ok to do this now */
        pf_hiame += numames;

        return(0);
}

/*
 * v_getame()
 *
 * allocate an address map entry.
 */
caddr_t
v_getame()
{
        int amex;

	AME_LOCKHOLDER();

        /* check that an ame is available.
         */
        if ( (amex = pf_amefree)  < 0)
        {
                if (getames())
                        return((caddr_t)NULL);
		amex = pf_amefree;
        }

	/* remove ame from free list and zero it.
	 * We could have release the ame_lock before 
	 * the bzero call but it is only 16 word
	 * and we can unlock specificaly in the wrapper.
	 */
	pf_amefree = ame_free(amex);
        bzero(&vmmdseg.ame[amex],sizeof(vmmdseg.ame[0]));
	return( (caddr_t) &vmmdseg.ame[amex] );
}

/*
 * v_freeame(ame)
 *
 * free an area map entry.
 */
v_freeame(ame)
caddr_t ame;
{
        int amex;

	AME_LOCKHOLDER();

	/* place ame on free list.
	 */
	amex = ((uint) ame - (uint) &vmmdseg.ame[0]) / sizeof(vmmdseg.ame[0]);
	ame_free(amex) = pf_amefree;
	pf_amefree = amex;

	return 0;
}

/*
 * v_freeseg(sidx,xmem)
 *
 * free the external page table and the scb of the segment
 * provided the xmem parameter is not equal to VM_XMEMCNT
 * and there is no discarded i/o for the segment which is
 * not yet complete.
 *
 * if xmem is equal to VM_XMEMCNT, the segment is marked
 * "delete pending" and the scb is put on the sidxmem list.
 *
 * if xmem is not equal to VM_XMEMCNT but there is discarded
 * i/o which has not completed the scb is freed but the
 * scb is marked in iodelete state. the scb will be freed
 * when all such i/o is done in v_pfend.
 *
 * this program runs at VMM interrupt level with back-tracking
 * enabled.
 *
 * RETURN VALUES - 0
 */

v_freeseg(sidx,xmem)
int     sidx;   /* index in scb table */
int     xmem;   /* xmem parameter     */
{
        struct xptroot * xptr;
        int  k,firstblk,lastblk,xptsize, msr;

	SCB_LOCKHOLDER(sidx);
	ALLOC_LOCKHOLDER();

        /* Release paging space allocation for segments requiring early
         * paging space allocation.
         */
        if (scb_psearlyalloc(sidx))
        {
                FETCH_AND_ADD(vmker.psfreeblks, scb_npseablks(sidx));
                scb_npseablks(sidx) = 0;
                scb_psearlyalloc(sidx) = 0;
        }

        /* if xmem is given as VM_XMEMCNT mark scb in delete
         * pending state and put on xmem free list 
         */
        if(xmem == VM_XMEMCNT)
        {
		scb_free(sidx) = pf_sidxmem;
		scb_delpend(sidx) = 1;
		pf_sidxmem = sidx;
		return 0;
        }

        /* no xpt to free if client or persistent segment
         */
        if (scb_vxpto(sidx) == 0)
                return (v_freescb(sidx,0));

        /* sid has a rooted xpt (a working segment)
         * first free up xpt direct blocks up to maxvpn.
         */
        xptr = (struct xptroot *) scb_vxpto(sidx);
        lastblk = scb_maxvpn(sidx) >> L2XPTENT;
        for(k = 0; k <= lastblk; k++)
        {
                if (xptr->xptdir[k] == NULL)
                        continue;
                v_freexpt(XPT1K,xptr->xptdir[k]);
                xptr->xptdir[k] = NULL;
        }

        /* free up xpt blocks for  [minvpn,SEGSIZE/PSIZE - 1]
         */
        firstblk = scb_minvpn(sidx) >> L2XPTENT;
        firstblk = MAX(firstblk,lastblk + 1);
        for(k = firstblk; k < XPTENT; k ++)
        {
                if (xptr->xptdir[k] == NULL)
                        continue;
                v_freexpt(XPT1K,xptr->xptdir[k]);
                        xptr->xptdir[k] = NULL;
        }

        /* free the root and the scb.
         */
        v_freexpt(XPT1K,xptr);
        v_freescb(sidx,0);
	return 0;

}

/*
 * v_freescb(sidx, xmem)
 *
 * for persistent and working segments puts the scb specified
 * by sidx on the free list unless there is pending i/o in which
 * case the segment is marked in iodelete state. the value zero
 * is returned.       
 * 
 * for client segments if there is no pending i/o sidx is put
 * on the scb free list and any process which may have been
 * previously v_waited on the scb for pending i/o is made ready.
 * if there is pending i/o the segment is marked in iodelete
 * state ,the caller v_waited and the value VM_WAIT is returned.
 *
 * if xmem is equal to VM_XMEMCNT, the segment is marked
 * "delete pending" and the scb is put on the sidxmem list.
 *
 * this procedure is called by vm_delete for client and persistent
 * segments and by v_freeseg (above) for working storage segments.
 * it is also called by v_pfend when all i/o is completed.
 *
 * return values 
 *		0 - ok
 *		VM_WAIT - process is v_waited until all i/o is done
 *			- this return value does not make it back 
 */
int
v_freescb(sidx, xmem)
int     sidx;   /* index in scb table */
int	xmem;	/* xmem parameter	*/
{
        int k;

	SCB_LOCKHOLDER(sidx);
	ALLOC_LOCKHOLDER();

        /* if xmem is given as VM_XMEMCNT mark scb in delete
         * pending state and put on xmem free list 
         */
        if(xmem == VM_XMEMCNT)
        {
		scb_free(sidx) = pf_sidxmem;
		scb_delpend(sidx) = 1;
		pf_sidxmem = sidx;
		return 0;
        }

        /* any pending i/o ?
         */
        if (scb_sidlist(sidx) >= 0)
        {
                scb_iodelete(sidx) = 1;
		if (!scb_clseg(sidx))
			return 0;
		/* client segment. must wait for io to complete
		 */
		v_wait(&scb_delwait(sidx));
		return VM_WAIT;
        }

	/* if client segment v_ready any waitors     
	 */
	if (scb_clseg(sidx))
	{
		while(scb_delwait(sidx) != NULL)
		{
			v_ready(&scb_delwait(sidx));
		}
	}

	/* performance trace hook.
	 */
	TRCHKL2T(HKWD_VMM_SEGDELETE,ITOS(sidx,0),scb_sibits(sidx));

        /* put on free list
         */
        scb_valid(sidx) = 0;
        scb_delpend(sidx) = 0;
        scb_free(sidx) = pf_sidfree;
        pf_sidfree = sidx;

        /* if its a log remove from pf_logsidx array.
	 * Need the lw lock to protect pf_logsidx.
         */
        if (scb_logseg(sidx))
	{
		LW_MPLOCK();
                for (k = 0; k < MAXLOGS; k ++)
                        if (pf_logsidx[k] == sidx)
                        {
                                pf_logsidx[k] = 0;
                                break;
                        }
		LW_MPUNLOCK();
	}

        return 0;
}
/*
 * v_promote(psidx)
 *
 * replace parent segment specified by psidx with his surviving
 * child in fork tree. free the external page table and scb of
 * psidx.
 *
 * this program runs at VMM interrupt level with back-tracking
 * enabled.
 *
 * RETURN VALUES - 0
 */
v_promote(psidx)
int psidx;      /* index in scb of parent segment */
{

        int psid,csid,csidx,gpsid,gpsidx;
	int pcow;

	/* get copy on write attribute.
	 * No need to lock, cannot change under delete bit
	 */
	pcow = scb_cow(psidx);

	ASSERT(scb_delete(psidx));

        /* determine surviving child. remember childs grandparent
	 * No need to lock the parent to determine the child,
	 * both parent and surviving child are marked delete.
         */

        csid = (scb_left(psidx) == 0) ? scb_right(psidx) : scb_left(psidx);
        csidx = STOI(csid);
	ASSERT(scb_delete(csidx)); /* set in v_inherit */
	ASSERT(STOI(scb_parent(csidx)) == psidx);

	SCB_MPLOCK_S(csidx);
	SCB_MPLOCK_S(psidx);

        gpsid = scb_parent(psidx);

        /* free xpt of parent and scb (could page fault).
         * the delete bit remains set in the parent until
         * all done (the xmem attach count on parent is
         * always zero (it was always a dummy segment).
         */
	ALLOC_MPLOCK_S();
        v_freeseg(psidx,0);
	ALLOC_MPUNLOCK_S();

        /* replace parent psid in tree with child csid.
         */
        scb_parent(csidx) = gpsid;
        if(gpsid != 0)
        {
                psid = ITOS(psidx,0);
                gpsidx = STOI(gpsid);

		SCB_MPLOCK(gpsidx);

                if (scb_left(gpsidx) == psid)
                        scb_left(gpsidx) = csid;
                else
                        scb_right(gpsidx) = csid;

		SCB_MPUNLOCK(gpsidx);
        }
	else
	{
		/* transfer copy-on-write state (mmap).
		 */
		scb_cow(csidx) = pcow;
	}

	SCB_MPUNLOCK_S(psidx);

        /* turn off delete bit in child (set by inherit).
         * wakeup any processes waiting on pf_deletewait
         */

        scb_delete(csidx) = 0;
        while(pf_deletewait)
        {
                v_ready(&pf_deletewait);
        }

	SCB_MPUNLOCK_S(csidx);

	return 0;
}

/*
 * v_cleardata(sid,lastp)
 *
 * free all external page tables of the segment except
 * for the one which covers the last megabyte of the
 * segment. reset scb_maxvpn to -1 and scb_minvpn to lastp.
 *
 * this procedure is only used by vm_cleardata.
 * this program executes on fixed stack at VMM interrupt
 * level with back-tracking enabled.
 *
 * RETURN VALUES - 0
 */

v_cleardata(sid,lastp)
int	sid; 	/* segment id */
int	lastp;	/* value for scb_minvpn */
{
        struct xptroot * xptr;
        int  k,firstblk,lastblk,sidx,endfree;

        /* sid has a rooted xpt (a working segment)
         * first free xpt direct blocks up to maxvpn.
         */
	sidx = STOI(sid);

	SCB_LOCKHOLDER(sidx);
	ALLOC_LOCKHOLDER();

        xptr = (struct xptroot *) scb_vxpto(sidx);
        lastblk = scb_maxvpn(sidx) >> L2XPTENT;
        for(k = 0; k <= lastblk; k++)
        {
                if (xptr->xptdir[k] == NULL)
                        continue;

                v_freexpt(XPT1K,xptr->xptdir[k]);
                xptr->xptdir[k] = NULL;
        }

        /* free up xpt blocks for [minvpn, up to lastp...
	 * but for avoiding freeing the xpt block which
	 * contains the disk block for the current stack
	 * frame itself (lastp+1).
         */
        firstblk = scb_minvpn(sidx) >> L2XPTENT;
	endfree = ((lastp+1) >> L2XPTENT) - 1;
	ASSERT(endfree > 0 && endfree <= XPTENT - 2);
        firstblk = MAX(firstblk,lastblk + 1);
        for(k = firstblk; k < endfree; k ++)
        {
                if (xptr->xptdir[k] == NULL)
                        continue;
                v_freexpt(XPT1K,xptr->xptdir[k]);
                        xptr->xptdir[k] = NULL;
        }

	/* set values for maxvpn and minvpn
	 * the last xpt entry which contains a disk
	 * block corresponds to the current stack
	 * frame.
	 */
	scb_maxvpn(sidx) = -1;
	scb_minvpn(sidx) = lastp + 1;
	return 0;
}

/*
 * v_pscount(sid)
 *
 * returns the number of paging space blocks used by a segment
 * and all of it's ancestors
 *
 * this program runs at VMM interrupt level with back-tracking
 * disabled.
 * it is called thru vcs, or by pgskill but from vmm when running
 * out of paging space while trying to allocate disk blocks.
 *
 * RETURN VALUES
 *	>= 0	- number of paging space blocks
 *	   0    is also returned if the segment is invalid
 *		or is not a working storage segment.
 */

v_pscount(sid)
int	sid;
{
	int	sidx;
	int	npsblks;

	sidx = STOI(sid);
	/* can't return -1 because VM_WAIT is -1
	 */
	if (sidx >= pf_hisid || !scb_valid(sidx) || !scb_wseg(sidx))
		return 0;

	/*
	 * get the number of paging space disk blocks 
	 * used by the segment and its ancestors
	 * For mp, no need to lock. The computation is not accurate
	 * since an operation can be in progress on this tree.
	 */
	npsblks = 0;
	do
	{
		sidx = STOI(sid);
		npsblks += scb_npsblks(sidx);
		if (scb_psearlyalloc(sidx))
			npsblks += scb_npseablks(sidx);
		sid = scb_parent(sidx);
	}
	while (sid);

	return npsblks;
}

/*
 * v_limits(sidx,uplim,downlim)
 *
 * This service changes the limits for a working storage segment that
 * requires early paging space allocation.
 *
 * this program runs at VMM interrupt level with back-tracking
 * disabled.
 *
 * RETURN VALUES
 *      0       Limits were successfully updated.
 *      ENOMEM  Limits could not be updated because paging space
 *              could not be allocated.
 */
int
v_limits(sidx,uplim,downlim)
int     sidx;     /* index of scb to be changed */
int     uplim;   /* size in pages of upper limit on growth */
int     downlim; /* size in pages of downward limit on growth */
{
        int psalloc;

	SCB_LOCKHOLDER(sidx);

	/* If changing the uplim requires allocation of paging space,
	 * do that here.
	 */
	if (scb_psearlyalloc(sidx) && !scb_sparse(sidx))
	{
		psalloc = uplim - scb_uplim(sidx);

		/* Verify that the paging space allocation will not put the
		 * system into a low paging space state.  If not then update
		 * the free paging space count of the system and the early
		 * allocation count of the segment.
		 */
		if (FETCH_AND_LIMIT(vmker.psfreeblks, psalloc, pf_npswarn))
		{
			scb_npseablks(sidx) += psalloc;
		}
		else
		{
			return ENOMEM;
		}
	}

	/* adjust uplim and downlim for the segment
	 */
        scb_uplim(sidx) = uplim;
        if (scb_privseg(sidx))
                scb_downlim(sidx) = downlim;

        return 0;
}

/*
 * v_psearlyalloc(sidx)
 *
 * This service turns on early paging space allocation for a segment
 * and allocates paging space for the segment.
 *
 * this program runs at VMM interrupt level with back-tracking
 * disabled.
 *
 *
 * RETURN VALUES
 *      0       The segment was converted to early paging space allocation
 *      ENOMEM  The segment could not be converted to early paging space
 *              allocation because paging space could not be allocated.
 */
int
v_psearlyalloc(sidx)
int     sidx;     /* index of scb to be changed */
{
        int psalloc;

        ASSERT(scb_wseg(sidx));
	SCB_LOCKHOLDER(sidx);

        if (scb_psearlyalloc(sidx))
                return 0;
	/*
	 * sparse segment is just marked as an early allocation segment.
	 * The allocation for these segments takes place during vm_map.
	 */

	if (scb_sparse(sidx))
	{
		scb_psearlyalloc(sidx) = 1; 
		return 0;
	}


        /* For regular segments, early allocation calculation is easy.
         * For process private segments, we assume the segment has no
         * paging space allocated for heap or stack.
         */
        if (!scb_privseg(sidx))
                psalloc = scb_uplim(sidx) + 1 - scb_npsblks(sidx);
        else
                psalloc = scb_uplim(sidx) + 1;

        /* Verify that the paging space allocation will not put the system
         * into a low paging space state.  If not then update the free paging
	 * space count of the system and the early allocation count of the
	 * segment and mark the segment.
         */
	if (FETCH_AND_LIMIT(vmker.psfreeblks, psalloc, pf_npswarn))
	{
		scb_npseablks(sidx) = psalloc;
		scb_psearlyalloc(sidx) = 1;
	}
	else
	{
		return ENOMEM;
	}

        return 0;
}

/*
 * v_pslatealloc(sidx)
 *
 * This service turns off early paging space allocation for a segment
 * and deallocates paging space for the segment.
 *
 * this program runs at VMM interrupt level with back-tracking
 * disabled.
 *
 * RETURN VALUES
 *      0       The segment was converted to late paging space allocation
 */
int
v_pslatealloc(sidx)
int     sidx;     /* index of scb to be changed */
{
        int psalloc;

        ASSERT(scb_wseg(sidx));
	SCB_LOCKHOLDER(sidx);

        if (!scb_psearlyalloc(sidx))
                return 0;

        psalloc = scb_npseablks(sidx);

        /* Update the free paging space count of the system and the
         * early allocation count of the segment and mark the segment.
         */
        FETCH_AND_ADD(vmker.psfreeblks, psalloc);
        scb_npseablks(sidx) = 0;
        scb_psearlyalloc(sidx) = 0;

        return 0;
}

