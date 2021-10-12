static char sccsid[] = "@(#)36	1.81  src/bos/kernel/vmm/POWER/v_getsubs1.c, sysvmm, bos412, 9444b412 10/20/94 11:14:43";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	v_pagein, v_fpagein, v_cpagein, v_dalloc, v_falloc,
 *              v_pagein_ublk
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

/*
 * lower level routines associated with bring pages into
 * virtual memory.
 */

#include "vmsys.h"
#include <jfs/inode.h>
#include <sys/errno.h>
#include <sys/low.h>
#include <sys/user.h>
#include <sys/mstsave.h>
#include <sys/except.h>
#include <sys/adspace.h>
#include <sys/inline.h>
#include <sys/trchkid.h>
#include <sys/syspest.h>
#include "mplock.h"

/*
 * v_pagein(sidx,pagex,store,sregval)
 *
 * initiate the page in of the specified page.
 *
 * on entry the free page frame list must be non-empty.
 *
 * RETURN VALUES
 *      0       - ok page fault satisfied without i/o
 *
 *      VM_WAIT - ok i/o needed.
 *
 *      ENOSPC  - disk full.
 *
 *      ENOMEM  - no memory for xpt
 */

v_pagein(sidx,pagex,store,sregval)
int     sidx;   /* index in scb table */
int     pagex;    /* page number in scb */
int     store;  /* 0 if load non-zero if store */
uint    sregval;  /* segment register value */
{

        union xptentry * xpt, * v_findxpt();
        int k,n,m,rc,key,nfr,pdtx, pno, sid;
	int sidio, pnoio;

	ASSERT(!scb_mseg(sidx));
	SCB_LOCKHOLDER(sidx);

        /* is it a persistent or client segment
         */
	if (scb_pseg(sidx))
	{
                return(v_fpagein(sidx,pagex,store,sregval));
	}

        if (scb_clseg(sidx))
        {
                return(v_cpagein(sidx,pagex,store,sregval));
        }

        /* working storage  segment.
         * get address of xpt entry for pagex. growxpt if necessary.
         */

	sid = ITOS(sidx, 0);
	pno = pagex;
        xpt = v_findxpt(sidx,pno);

	/* indicate copy-on-write needed from source SCB (for mmap).
         * If needed, don't grow xpt
	 */
	if (scb_cow(sidx) && (xpt == NULL || xpt->word == 0))
		return ENOENT;

        if (xpt == NULL)
        {
                if (rc = v_growxpt(sidx,pno))
                {
			return(rc);
                }
                xpt = v_findxpt(sidx,pno);
        }

        /* if xpt entry is null check out parent
         */
        if (xpt->word == 0 && scb_parent(sidx))
        {
                if ((rc = getparent(sidx,pno,xpt)) >= 0)
                        return(rc);
        }

	/* If sparse segment and no xpt 
	 * then return EFAULT as this is a stray fault.
	 */
	if (scb_sparse(sidx) && (xpt->word == 0))
	{
		return(EFAULT);
	}

	/* is it mapped ?
	 */
        if (xpt->mapblk)
        {
		/* getvmpage returns VM_SPARSE for
		 * vm_mapped pages that are 
		 * to be zero filled.
		 */
		rc = getvmpage(sidx,pno,xpt);
		if (rc != VM_SPARSE)
			return rc;
        }

        /* get default storage protect key ?
         */
        if (xpt->word == 0)
        {
                key = (pno <= scb_sysbr(sidx)) ? scb_defkey(sidx):pf_kerkey;
                xpt->spkey = key;
        }
        key = xpt->spkey;

	/* do we have to allocate a disk block ? do so
	 * only if store access is allowed to the page.
	 */
	if (xpt->cdaddr == 0)
	{
		/* if no store access set key to RDONLY. 
		 */
		if (nostore(sregval,xpt))
		{
			/* check for no access allowed.
			 */
			if (key == KERKEY && SRTOKEY(sregval))
				return(EXCEPT_PROT);

			key = RDONLY;
		}
		else
		{
			if ((rc = v_dalloc(sidx,pno,xpt)) != 0)
				return(rc);
		}
		xpt->zerobit = 1;
	}

        /* at this point xpt points to xpt entry of the
         * page we want. also the zerobit is set. we
         * need a page frame to satisfy request.
         */
        nfr = v_getframe(sid,pno);
	pft_devid(nfr) = pdtx = PDTIND(xpt->word);
	pft_dblock(nfr) = PDTBLK(xpt->word);

	/* Insert the page at its I/O address and make it
	 * r/w to initialize the page.
	 */
	pft_key(nfr) = (key == RDONLY) ? KERKEY : key;
        sidio = IOSID(sid);
        pnoio = pno;
	P_ENTER(STARTIO,sidio,pnoio,nfr,pft_key(nfr),pft_wimg(nfr));
        pft_pagein(nfr) = 1;

        /* is the page zeros ?
         */
        if (xpt->zerobit)
        {
                xpt->zerobit = 0;

                /* zero page.
                 */
                ZEROPAGE(sidio,pnoio);
		vmpf_zerofills += 1;

		/* Insert page at normal address with correct key.
		 */
		pft_pagein(nfr) = 0;
		pft_key(nfr) = key;
		P_ENTER(IODONE,sid,pno,nfr,pft_key(nfr),pft_wimg(nfr));
                pft_inuse(nfr) = 1;

		/* Modified bit is clear.  If this is a read/write page
		 * we need to set the modbit.
                 */
		if (key != RDONLY)
			SETMOD(nfr);

		/* Put on scb list.
		 */
                v_insscb(sidx,nfr);

		/* performance trace hook - zero filled page.
	 	*/
		TRCHKL4T(HKWD_VMM_ZFOD,sid,scb_sibits(sidx),pno,nfr);

                return(0);
        }

	/* Put on I/O part of scb list.
         */
        v_insscb(sidx,nfr);

        /* enque i/o.
         */
	PDT_MPLOCK();

        v_pdtqiox(pdtx,nfr,1);

	PDT_MPUNLOCK();

	v_wait(&pft_waitlist(nfr));
        return(VM_WAIT);
}

/*
 * v_pagein_ublk(sidx,pno,sregval)
 *
 * initiate the page in of the specified u-block page.
 *
 * on entry the free page frame list must be non-empty.
 *
 * v_pagein_ublk() is a variant of v_pagein(). v_pagein_ublk()
 * is used only for u-block page and is faster due to the following :
 * 1. The xpt entry is null and v_pagein_ublk() doesn't call getparent
 *    to check out the ancestor because the ancestor shouldn't
 *    have fsid's pinned pages. Nor did it call getparent() just to 
 *    inherit the PTE page protect bits from the forker fsid since we
 *    know the PTE page protect bits for the u-block page is KERKEY.
 * 2. It doesn't allocate a disk block because the
 *    page is later long term pinned in the memory.
 * 3. v_pagein_ublk() doesn't zero the u-block page since the page will
 *    be written over by COPYPAGE later.
 * 4. For fork performance reason, long-term pin the u-block page
 *    here instead of in procdup() (m_fork.c)
 *
 * RETURN VALUES
 *      0       - ok page fault satisfied without i/o
 *
 *      ENOMEM  - no memory for xpt or too many pinned pages.
 */

v_pagein_ublk(sidx,pno,sregval)
int     sidx;   /* index in scb table */
int     pno;    /* page number in scb */
uint    sregval;  /* segment register value */
{

        union xptentry * xpt, * v_findxpt();
        int rc,nfr,sid;
	int sidio;

	SCB_LOCKHOLDER(sidx);

	/* Check the pinned page threshold, if we exceed the max number
	 * of pinned pages allowed, return error.
	 */	
	if (pf_pfavail <= vmker.pfrsvdblks)
		return ENOMEM;

        /* The process private segment is a working storage segment.
         * get address of xpt entry for pno. growxpt if necessary.
         */

	sid = ITOS(sidx, 0);
        xpt = v_findxpt(sidx,pno);

        if (xpt == NULL)
        {
                if (rc = v_growxpt(sidx,pno))
                {
			return(rc);
                }
                xpt = v_findxpt(sidx,pno);
        }

	/*
	 * For u-block page, don't call getparent() just to inherit
	 * the page protection bits from the forker fsid since we
	 * know the page protection bits for the u-block page are 
	 * pf_kerkey. We can not use KERKEY because of the HW
	 * problem with fetch protection on some machines.
         */
        xpt->spkey = pf_kerkey;

        /* at this point xpt points to xpt entry of the
         * page we want. also the zerobit is not set. we
         * need a page frame to satisfy request.
         */
        nfr = v_getframe(sid,pno);

	/* 
	 * For u-block page, pft_devid(nfr) = 0
	 * and pft_dblock(nfr) = 0. 
	 */


	/* For performance, long-term pin the u-block page
	 * here instead of in procdup() (m_fork.c) 
	 * The pft_devid and pft_dblock are set to zero 
	 * as the u-block page is long-term pinned (it has no 
	 * disk block assigned).
	 */
	pft_devid(nfr) = 0;
	pft_dblock(nfr) = 0;

	/* Mark the frame pinned and adjust statistics. 
 	 */	
	pft_pincount(nfr) = LTPIN_INC;
	fetch_and_add(&pf_pfavail, -1);

	/* Insert page at normal address with correct key.
	 */

	pft_key(nfr) = xpt->spkey;
	P_ENTER(NORMAL,sid,pno,nfr,pft_key(nfr),pft_wimg(nfr));
        pft_inuse(nfr) = 1;

	/* Put on scb list.
	*/
        v_insscb(sidx,nfr);

	/* performance trace hook - zero filled page.
	 */
	TRCHKL4T(HKWD_VMM_ZFOD,sid,scb_sibits(sidx),pno,nfr);

        return(0);
} /* end v_pagein_ublk */

/* v_fpagein(sidx,pagex,store,sregval)
 * 
 * page in for a persistent segment. see above for description
 * of parameters and return values.
 *
 */

v_fpagein(sidx,pagex,store,sregval)
int     sidx;   /* index in scb table */
int     pagex;    /* page number in segment */
int     store;  /* 0 if load non-zero if store */
uint    sregval;  /* segment register value */
{
        union xptentry * xpt, * v_findiblk(),dumxpt ;
        int k,n,m,rc,nfr,zkey, pdtx, dblk, zpage,lw, temp;
	int sid, pno, readonly, ag, ipgperag, fperpage;
	struct inode *ip;
	int sidio, pnoio;

	SCB_LOCKHOLDER(sidx);
	
	/* check if the segment is inactive.
	 */
	if (scb_inactive(sidx))
	{
		return(ESTALE);
	}

	/* check if store allowed.
	 */
	if (store && SRTOKEY(sregval))
	{
		return(EXCEPT_PROT);
	}

	/* get sid and page number relative to sid
	 * normal value for pdtx is in scb.
	 */
	readonly = 0;
	sid = ITOS(sidx, pagex);
	pno = BASEPAGE(pagex);
	pdtx = scb_devid(sidx);

	/* get the file system's fragments per page.
	 */
	fperpage = pdt_fperpage(pdtx);

	/* a log is NOT a file so it does not have an external
	 * page table (inode + indirect blocks). however, the disk
	 * block associated with page p of a log is block p.
	 */
	if (scb_logseg(sidx))
	{
		dblk = pagex;
		zpage = 0;
		goto common;
	}


	/* if its journalled of deferred update must check for
	 * page being in extension memory.
	 */
	if (scb_jseg(sidx) | scb_defseg(sidx))
	{
		LW_MPLOCK_S();

		/* lockword has both home and paging space disk
		 * address.
		 */
		if (lw = v_findlw(sid,pno))
		{
			/* Once found, the lockword is protected
			 * by the scb lock.
			 */
			LW_MPUNLOCK_S();

			if(temp = lword[lw].extmem)
			{
				pdtx = PDTIND(temp);
				dblk = PDTBLK(temp);
			}
			else
			{
				dblk = lword[lw].home;
			}
			zpage = 0;
			goto common;
		}

		LW_MPUNLOCK_S();
	}

	/* an inode segment does NOT have an external page table.
	 * the disk block associated with a page can be computed.
	 * the inodes of the first allocation group begin at 
	 * INODES_B. all others at the first block of their group.
	 */
	if (scb_inoseg(sidx))
	{
		/* calculate number of pages of inodes per ag and
		 * the ag number containing pagex.
		 */
		ipgperag = scb_iagsize(sidx) / (PSIZE / sizeof(struct dinode));
		ag = pagex / ipgperag;

		if (ag == 0)
			dblk = (INODES_B + pagex) * fperpage;
		else
			dblk = ag * scb_agsize(sidx) +
					(pagex - ag * ipgperag) * fperpage;
		zpage = 0;
		goto common;
	}


	/* if the segment is being committed..wait unless segment
	 * is journalled.
	 */
	if (scb_combit(sidx) && !scb_jseg(sidx))
	{
		v_wait(&pf_extendwait);
		vmpf_extendwts += 1;
		return (VM_WAIT);
	}

	/* get xpt for pagex
	 */
        if ((xpt = v_findiblk(sidx,pagex)) == NULL)
        {
		zpage = 1;
		if (scb_combit(sidx))
		{
			v_wait(&pf_extendwait);
			vmpf_extendwts += 1;
			return (VM_WAIT);
		}

		/* if we are not going to allocate a disk block
		 * don't growiblk.
		 */
		if (!store && !scb_jseg(sidx))
		{
			pdtx = dblk = 0;
			readonly = 1;
			goto common;
		}

		/* check if a prior page of the segment contains a
		 * partial allocation and if so promote the prior
		 * page to a full block allocation.
		 */
		if (scb_compress(sidx) == 0)
		{
			if (rc = v_priorfrag(sidx,pagex))
				return(rc);
		}

		/* ok to grow indirect blocks
		 * and to allocate a disk block for pagex
		 * In mp we can fault on the xpt after the findiblk.
		 */
                if (rc = v_growiblk(sidx,pagex))
                        return(rc);
                xpt = v_findiblk(sidx,pagex);
		dblk = xpt->fragptr;
		goto common;
        }

	/* allocate disk blocks if necessary. we can't if combit
	 * is set. we don't unless it is a store or the segment
	 * is journalled.
	 */
	if (zpage = (xpt->fragptr == 0))
	{
		if (scb_combit(sidx))
		{
			v_wait(&pf_extendwait);
			vmpf_extendwts += 1;
			return (VM_WAIT);
		}

		if (store || scb_jseg(sidx)) 
		{

			/* check if a prior page of the segment contains a
			 * partial allocation and if so promote the prior
			 * page to a full block allocation.
			 */
			if (scb_compress(sidx) == 0)
			{
				if (rc = v_priorfrag(sidx,pagex))
					return(rc);
			}

			/* allocate a block for pagex.
			 */
			FS_MPLOCK_S(pdtx);

			if (rc = v_falloc(sidx,pagex,xpt,fperpage))
			{
				FS_MPUNLOCK_S(pdtx);
				return(rc);
			}

			dblk = xpt->fragptr;

			FS_MPUNLOCK_S(pdtx);
		}
		else
		{
			pdtx = dblk = 0;
		}

                /* if load caused page fault and the segment
                 * is not a journalled segment set the storage
                 * key to read-only. if a store occurs subsequently
                 * the key will be changed.
                 */
                readonly = !store && !scb_jseg(sidx);
	}
	else
	{
		dblk = xpt->fragptr;

		/* should the last page be made read-only ?
		 */
		readonly = lpreadonly(sidx,pagex);
	}


        /* at this point pdtx and dblk and the zpage flag
	 * are set. we need a page frame to satisfy request.
         */
	common:
        nfr = v_getframe(sid,pno);
        pft_devid(nfr) = pdtx;
        pft_dblock(nfr) = dblk;

	/* Insert the page at its I/O address and make it
	 * r/w to initialize the page.
	 */
	pft_key(nfr) = readonly ? KERKEY : FILEKEY;
        sidio = IOSID(sid);
        pnoio = pno;
	P_ENTER(STARTIO,sidio,pnoio,nfr,pft_key(nfr),pft_wimg(nfr));
        pft_pagein(nfr) = 1;

        /* init some bits for journal segments
         */
        if(scb_jseg(sidx))
        {
                pft_homeok(nfr) = 1;
                pft_journal(nfr) = 1;
        }

        /* is the page zeros ?
         */
        if (zpage)
        {
		pft_newbit(nfr) = 1;

                /* zero page.
                 */
                ZEROPAGE(sidio,pnoio);
		vmpf_zerofills += 1;

		/* Insert page at normal address with correct key.
		 * This also clears the modified bit so a readonly
		 * page is marked unmodified.
                 */
		pft_pagein(nfr) = 0;
		pft_key(nfr) = readonly ? RDONLY : FILEKEY;
		P_ENTER(IODONE,sid,pno,nfr,pft_key(nfr),pft_wimg(nfr));
                pft_inuse(nfr) = 1;

		/* Modified bit is clear.  If this is a read/write page
		 * we need to set the modbit.
                 */
		if (!readonly)
			SETMOD(nfr);

		/* put on scb list.
		 */
                v_insscb(sidx,nfr);

		/* performance trace hook - zero filled page.
	 	*/
		TRCHKL4T(HKWD_VMM_ZFOD,sid,scb_sibits(sidx),pno,nfr);

                return(0);
        }

	/* Put on I/O part of scb list.
	 * Also set rdonly bit so page will get correct protection key
	 * at page-fault end time.
         */
        pft_rdonly(nfr) = readonly;
        v_insscb(sidx,nfr);

        /* enque i/o.
         */
	PDT_MPLOCK();

        v_pdtqiox(pdtx,nfr,1);

	PDT_MPUNLOCK();

	v_wait(&pft_waitlist(nfr));
        return(VM_WAIT);
}

/*
 * getparent(sidx,pno,xpt)
 *
 * ancestors of sidx are searched to see if they have
 * defined a version of pno i.e. they a non-zero xpt
 * entry for the page. if no ancestor has defined the
 * page or if the definition of the page is a logically
 * zero page getparent returns -1. however, if the page
 * is logically zero the xpt entry of sidx is set to
 * the xpt entry of the ancestor, so that sidx gets the
 * right storage protect key.
 *
 * otherwise the most recent ancestor is forced to get
 * the page. if the ancestor is immediate parent and
 * the sibling of sidx already has a copy of the page
 * the parents copy of the page and his xpt entry is
 * taken to satisfy the request. otherwise a copy of the
 * page is made for sidx, and a new disk block allocated
 * unless the ancestor's page is vmapped.
 *
 * on entry the sidx must have a parent and the free
 * block list must not be empty.
 *
 * RETURN VALUES
 *
 *      0       - page fault satistfied.
 *
 *      -1      - no ancestor has defined a non-zero page
 *                xpt entry is set if ancestor has logically
 *                zero page.
 *
 *      ENOSPC  - no disk space
 *
 *	ENOENT	- no ancestor has defined a non-zero page
 *		  need to perform copy-on-write from source
 *		  SCB (mmap)
 */

static
getparent(sidx,pno,xpt)

int     sidx;   /* index in scb table */
int     pno;    /* page number in segment */
union   xptentry *xpt;   /* pointer to xpt entry */
{

        int  sid, psid,psidx,ssid,ssidx,vmap,mapx;
        int  p,nfr,newf,newkey,copykey,pdtx,dblk,scopy;
        union xptentry *sxpt , *pxpt, *v_findxpt();
	int sidio, pnoio;
#ifdef _VMM_MP_EFF
	int tsid, tsidx;
#endif /* _VMM_MP_EFF */

	SCB_LOCKHOLDER(sidx);

        /*
         * check ancestors for non-null copy of pno.
         */

#ifndef _VMM_MP_EFF
        for(psid = scb_parent(sidx); psid ; psid = scb_parent(psidx))
#else  /* _VMM_MP_EFF */
        for(tsid = sid = ITOS(sidx, 0), psid = scb_parent(sidx);
	    psid ;
	    psid = scb_parent(psidx))
#endif /* _VMM_MP_EFF */
        {
                psidx = STOI(psid);
#ifdef _VMM_MP_EFF
		tsidx = STOI(tsid);
		ASSERT(scb_left(psidx) == tsid || scb_right(psidx) == tsid);

		/* Lock parent before releasing its child lock.
		 */
		SCB_MPLOCK_S(psidx);
		if(tsid != sid)
			SCB_MPUNLOCK_S(tsidx);
		tsid = psid;
#endif /* _VMM_MP_EFF */

                pxpt = v_findxpt(psidx,pno);
                if (pxpt == NULL || pxpt->word == 0)
                        continue;
                /*
                 * is psids page non-zero ?
                 */
                if (pxpt->cdaddr != 0)
                        break;
                /*
                 * sid gets parents non-zero xpt entry
                 */
                xpt->word = pxpt->word;

		SCB_MPUNLOCK_S(psidx);

                return(-1);
        }


        if (psid == 0)
	{
		/*
		 * indicate copy-on-write needed from source SCB (for mmap).
		 */
		if (scb_cow(psidx))
		{
			SCB_MPUNLOCK_S(psidx);
			return(ENOENT);
		}
		else
		{
			SCB_MPUNLOCK_S(psidx);
			return(-1);
		}
	}

	SCB_LOCKHOLDER(sidx);
	SCB_LOCKHOLDER(psidx);

        /*
         * touch page of ancestor psid
         */

        (void)chgsr(TEMPSR,SRVAL(psid,0,0));
        p = (TEMPSR << L2SSIZE)  + (pno << L2PSIZE);
        TOUCH(p);

        /*
         * if ancestor is parent see if sibling has copy of page.
         * scopy is set to 1 if sibling has a copy or sibling
         * is null, provided ancestor is the immediate parent.
         */

        scopy = 0;
	sid = ITOS(sidx,0);
        if (scb_parent(sidx) == psid)
        {
                /*
                 * get siblings sid
                 */
                ssid = scb_left(psidx);
                if (ssid == sid)
                        ssid = scb_right(psidx);
                /*
                 * if sibling non-null see if he has copy of pno
                 * (null sibling is equivalent to having copy)
		 * no need to lock sibling, accuracy not required.
                 */
                if (ssid)
                {
                        ssidx = ITOS(ssid,0);
                        sxpt = v_findxpt(ssidx,pno);
                        if ( sxpt != NULL && sxpt->cdaddr )
                                scopy = 1;
                }
                else
                {
                        scopy = 1;
                }
        }

        /*
         * if sibling has a copy and ancestor is parent we
         * take parents copy of page and his xpt entry.
         */

        if (scopy)
        {
                /*
                 * take parents xpt (set parents to NULL) and update
                 * paging space block counts in scb if not vmapped.
                 */
                xpt->word = pxpt->word;
                pxpt->word = 0;
		if (!xpt->mapblk)
		{
			scb_npsblks(sidx) += 1;
			scb_npsblks(psidx) += -1;

                        if (PSEARLYALLOC(sidx,pno))
                                scb_npseablks(sidx) += -1;
                        if (PSEARLYALLOC(psidx,pno))
                                scb_npseablks(psidx) += 1;
		}
                /*
                 * find page frame and remove from hash and slist
                 * invalidate tlb for psid.
                 */
                nfr = v_lookup(psid,pno);
                v_delpft(nfr);
                v_delscb(psidx,nfr);

		SCB_MPUNLOCK_S(psidx);

                /*
                 * insert in hash and scb list for sid
                 */
                newkey = (xpt->mapblk) ? RDONLY : xpt->spkey;
                P_RENAME(sid,pno,nfr,newkey,pft_wimg(nfr));
                pft_ssid(nfr) = sid;
		v_inspft(sid,pno,nfr);
                v_insscb(sidx,nfr);

		/* performance trace hook - get parent's page.
	 	*/
		TRCHKL4T(HKWD_VMM_GETPARENT,sid,scb_sibits(sidx),pno,nfr);

                return(0);
        }

        /*
         * sibling doesn't have copy or ancestor is not parent
         * so we have to make a copy of the page for sidx.
         * if the ancestor xpt is a vmapblk , we copy the
         * parent's xpt and increment the use count on the
         * vmapblk. otherwise we allocate a disk block for
         * sidx.
	 * We don't need to acquire the vmap lock since 
	 * we hold the lock on the ancestor which holds the count.
         */
        vmap = pxpt->mapblk;
        if(!vmap)
        {
                if (v_dalloc(sidx,pno,xpt))
		{
			SCB_MPUNLOCK_S(psidx);
                        return(ENOSPC);
		}
        }
        else
        {
                mapx = pxpt->cdaddr;
		FETCH_AND_ADD(pta_vmap[mapx].count, 1);
                xpt->word = pxpt->word;
        }

        /*
         * take a page frame from free list.
         */
        newf = v_getframe(sid,pno);
        if(!vmap)
        {
                pft_devid(newf) = PDTIND(xpt->word);
                pft_dblock(newf) = PDTBLK(xpt->word);
		newkey = xpt->spkey = pxpt->spkey;
        }
        else
        {
                pft_devid(newf) = 0;
                pft_dblock(newf) = 0;
		newkey = RDONLY;
        }

	/* Insert the page at its I/O address and make it
	 * r/w to initialize the page.
         */
	pft_key(newf) = KERKEY;
        sidio = IOSID(sid);
        pnoio = pno;
	P_ENTER(STARTIO,sidio,pnoio,newf,pft_key(newf),pft_wimg(newf));
	pft_pagein(newf) = 1;
        COPYPAGE(sidio,pnoio,psid,pno);

	SCB_MPUNLOCK_S(psidx);

	/* Insert page at normal address with correct key.
	 * This also clears the modified bit so if this is
	 * not a vmap page we need to set the modified bit.
         */
	pft_pagein(newf) = 0;
	pft_key(newf) = newkey;
	P_ENTER(IODONE,sid,pno,newf,pft_key(newf),pft_wimg(newf));
        pft_inuse(newf) = 1;
	if (!vmap)
		SETMOD(newf);

	/* Insert on SCB list.
         */
        v_insscb(sidx,newf);

	/* performance trace hook - get copy of ancestor's page.
 	*/
	TRCHKL4T(HKWD_VMM_COPYPARENT,sid,scb_sibits(sidx),pno,newf);


        return(0);
}

/*
 * getvmpage(tsidx,tpno,txpt)
 *
 * initiate page in for a page in a segment which is mapped
 * to a page  via a vmapblk. on entry txpt points to the xpt
 * entry which points to the vmapblk.
 *
 * see v_pagein for description of input paramters and results.
 *
 * the target page is made read-only. a subsequent store will
 * cause allocation of a disk-block for the page and the change
 * of the storage protect key.
 *
 * RETURN VALUES
 *
 *      0       - ok page fault satisfied without i/o
 *
 *      VM_WAIT - ok i/o needed.
 *
 */

static
getvmpage(tsidx,tpno,txpt)
int     tsidx;  /* index in scb table */
int     tpno;   /* page number in segment */
union xptentry * txpt;    /* pointer to xpt entry for tpno */
{

        int tsid,ssid,spno,ssidx,p,pdtx,dblk;
        int mapx,nfr,key,copykey;
        union xptentry *sxpt , *v_findxpt();
	int sidio, pnoio, prevcnt;

	SCB_LOCKHOLDER(tsidx);

        /*
         * get sid and pno and index in scb of source segment.
         * note that INVLSID is used to denote self-referential
         * map, i.e. source and target segments the same.
	 * We don't need to acquire the vmap lock, our scb holds the count.
         */
	tsid = ITOS(tsidx, 0);
        mapx = txpt->cdaddr;

	/* Check for a sparse segment 
	 * that has a vm_mapped page	
	 * that should be zero filled.
	 */
	if (scb_sparse(tsidx) && (pta_vmap[mapx]._u.sid == INVLSID)
		&& (pta_vmap[mapx].torg == pta_vmap[mapx].sorg))  
	{
		/* the xpt entry is no longer used as a pointer
		* to the vmapblk. decrement use count on vmapblk
		* and free it if count is now zero (previous
		* count was 1).
		*/

		TOUCH(&pta_vmap[mapx]);

		txpt->cdaddr = 0;
		txpt->mapblk = 0;

		FETCH_AND_ADD_R(pta_vmap[mapx].count, -1, prevcnt);
		ASSERT(prevcnt > 0);

		if (prevcnt == 1)
		{
			VMAP_MPLOCK();
			pta_vmap[mapx]._u.next = pta_vmapfree;
			pta_vmapfree = mapx;
			VMAP_MPUNLOCK();
		}

		txpt->zerobit = 1;
		return(VM_SPARSE);
	}

        ssid = (pta_vmap[mapx]._u.sid != INVLSID)? pta_vmap[mapx]._u.sid : tsid;
        spno = tpno + (pta_vmap[mapx].sorg - pta_vmap[mapx].torg);
        ssidx = STOI(ssid);

#ifdef _VMM_MP_EFF
	/* Lock the source if it is not a self reference.
	 * Avoid deadlock by bailing out if the source sidx 
	 * is lower than the target sidx and if the lock try
	 * is not successful.
	 */
	if(ssidx > tsidx)
	{
		SCB_MPLOCK_S(ssidx);
	}
	else if (ssidx < tsidx)
		if (SCB_MPLOCK_TRY_S(ssidx) == 0)
			return VM_NOWAIT;
#endif /* _VMM_MP_EFF */

        /*
         * we copy from the source always. i.e. we force
         * source to get copy of the page by touching page.
         */

        (void)chgsr(TEMPSR,SRVAL(ssid,0,0));
        p = (TEMPSR << L2SSIZE)  + (spno << L2PSIZE);
        TOUCH(p);

        /* check for the special target segment for which
         * the page frame of the source page is to be
         * given away to the target page if the page is not
         * modified
         */

        nfr = v_lookup(ssid,spno);
        assert(nfr >= 0);

        if (scb_shrlib(tsidx) && !ISMOD(nfr)) {
                v_delpft(nfr);
                v_delscb(ssidx,nfr);

		if(ssidx != tsidx)
			SCB_MPUNLOCK_S(ssidx);

                pft_ssid(nfr) = tsid;
                pft_pagex(nfr) = SCBPNO(tsid,tpno);
                pft_key(nfr) = RDONLY;
                pft_dblock(nfr) = pft_devid(nfr) = 0;
                P_RENAME(tsid,tpno,nfr,pft_key(nfr),pft_wimg(nfr));
                v_inspft(tsid,tpno,nfr);
                v_insscb(tsidx,nfr);
	} else {
		/*
		 * Allocate a page frame.
		 * Record correct protection key for the page.
		 * Insert the page at its I/O address and make it
		 * r/w to initialize the page.
		 */
		nfr = v_getframe(tsid,tpno);
		pft_devid(nfr) = 0;
		pft_dblock(nfr) = 0;
		pft_key(nfr) = KERKEY;
		sidio = IOSID(tsid);
		pnoio = tpno;
		P_ENTER(STARTIO,sidio,pnoio,nfr,pft_key(nfr),pft_wimg(nfr));
		pft_pagein(nfr) = 1;
		COPYPAGE(sidio,pnoio,ssid,spno);

		if(ssidx != tsidx)
			SCB_MPUNLOCK_S(ssidx);

		/* Insert page at normal address with correct key.
		 * This also clears the modified bit.
		 */
		pft_pagein(nfr) = 0;
		pft_key(nfr) = RDONLY;
		P_ENTER(IODONE,tsid,tpno,nfr,pft_key(nfr),pft_wimg(nfr));
		pft_inuse(nfr) = 1;

		/* Insert on SCB list.
		 */
		v_insscb(tsidx,nfr);
	}
	/* performance trace hook - vmapped page.
 	*/
	TRCHKL4T(HKWD_VMM_VMAP,tsid,scb_sibits(tsidx),tpno,nfr);


        return(0);
}

/*
 * v_cpagein(sidx,pagex)
 *
 * initiate page in for a page in a client segment.
 * see v_pagein for description of input paramters and results.
 *
 * RETURN VALUES
 *      VM_WAIT - ok i/o needed (always).
 */

v_cpagein(sidx,pagex,store,sregval)
int     sidx;   /* index in scb table */
int     pagex;  /* page number in segment */
int     store;  /* 0 if load non-zero if store */
uint    sregval;  /* segment register value */
{
	int nfr, pdtx,pno,sid;
	struct mstsave *mst;
	int exsid;
	int sidio, pnoio;

	SCB_LOCKHOLDER(sidx);

	/* check if the segment is inactive.
	 */
	if (scb_inactive(sidx))
	{
		return(ESTALE);
	}

	/* check if store allowed.
	 */
	if (store && SRTOKEY(sregval))
	{
		return(EXCEPT_PROT);
	}

        /* allocate a page frame. fill in pft entry
         */
	sid = ITOS(sidx, pagex);
	pno = BASEPAGE(pagex);
        nfr = v_getframe(sid,pno);
        pft_devid(nfr) = pdtx = scb_devid(sidx);
        pft_dblock(nfr) = 0;

        /* set store flag if store operation.
         */
	if (store)
	{
		pft_store(nfr) = 1;
	}
	
	/* set mmap flag if fault from mmap'er.
	 */
	mst = (struct mstsave *) &u.u_save;
	exsid = SRTOSID(mst->except[ESRVAL]);

	/*
	 * If this request is due to an mmap fault or any
	 * page-ahead operation then set pft_mmap so the
	 * client strategy routine fails operations beyond
	 * EOF.  This is required to ensure that mmap'ers
	 * get SIBGUS for references beyond EOF.  It is
	 * also useful to avoid creating unnecessary read-only
	 * pages beyond EOF.
	 */
	if (scb_mseg(STOI(exsid)) || PPDA->ppda_no_vwait)
		pft_mmap(nfr) = 1;

	/* Insert page at I/O virtual address
         * and put on scb list.
         */
	pft_key(nfr) = FILEKEY;
        sidio = IOSID(sid);
        pnoio = pno;
        P_ENTER(STARTIO,sidio,pnoio,nfr,pft_key(nfr),pft_wimg(nfr));
        pft_pagein(nfr) = 1;
        v_insscb(sidx,nfr);

        /* initiate i/o.
         */
	PDT_MPLOCK();

        v_pdtqiox(pdtx,nfr,1);

	PDT_MPUNLOCK();

	v_wait(&pft_waitlist(nfr));
        return(VM_WAIT);
}

/*
 * v_dalloc(sidx,pno,xpt)
 *
 * allocate a disk block for the page in the working 
 * storage segment specified and fill in xpt entry with
 * the block allocated. on entry xpt points to the xpt
 * entry for pno.
 *
 * RETURN VALUES
 *
 *      0       - ok
 *
 *      ENOSPC  - disk full.
 */

int
v_dalloc(sidx,pno,xpt)
int     sidx;   	/* index in scb table */
int     pno;    	/* page number in segment */
union   xptentry *xpt;   /* pointer to xpt entry */
{
        int  sid, rc, pdtx;
	uint block;

	SCB_LOCKHOLDER(sidx);

        /* allocate from paging space if a working segment.
         */
        assert(scb_wseg(sidx));

	if ((rc = v_pgalloc(&pdtx,&block)) != 0)
        	return(rc);

	scb_npsblks(sidx) += 1;
	if (PSEARLYALLOC(sidx,pno))
		scb_npseablks(sidx) += -1;
	else
		FETCH_AND_ADD(vmker.psfreeblks, -1);
        xpt->cdaddr = XPTADDR(pdtx,block);

	/* performance trace hook.
	 */
	sid = ITOS(sidx,pno);
	TRCHKL5T(HKWD_VMM_DALLOC,sid,scb_sibits(sidx),pno,pdtx,block);

	return(0);
}


/*
 * v_falloc(sidx,pno,xpt,nfrags)
 *
 * allocates nfrags fragments for the page pno in the specified 
 * persistent segment.  fills in the xpt entry with the fragments
 * allocated. also frees the fragments currently described by the xpt
 * if these fragment are uncommitted.  if the fragments are committed,
 * they are recorded in movedfrag structures associated with the inode
 * and they will be freed when the inode is next committed.
 *
 * if pno currently has disk resources associated with it which are
 * uncomitted (eg. new) an attempt is made to allocate enough segments
 * adjacent to its current allocation to make up the nfrags needed.
 * otherwise, they are allocated disjointly from the current allocation.
 *
 * in satisfying the request for pno, reallocation of the disk blocks
 * for the pages in the same cluster as pno may be attempted. cluster
 * reallocation results in a contiguous assignment of disk to the 
 * pages in the cluster. cluster reallocation does not apply for 
 * compressed files (re-allocation will occur when they are written
 * to disk) and for certain boundary values for pno.
 *
 * PARAMETERS:
 *
 *	sidx	- segment control block index.
 *	pno	- absolute page number within segment.
 *	xpt	- pointer to xpt entry for pno.
 *	nfrags	- number of fragments required for pno.
 *
 * RETURN VALUES:
 *
 *      0       - ok.
 *      ENOSPC  - disk full.
 *      EDQUOT  - reached disk quota limit.
 *      VM_WAIT - the process must wait.
 *	ENOMEM  - couldn't allocate movedfrag structure
 */

int
v_falloc(sidx,pno,xpt,nfrags)
int     sidx;
int     pno;
union   xptentry *xpt;
int	nfrags;
{
        int pdtx, sid, fperpage, newfrags, rc, nb, k;
        uint hint, fragno, old[FSCLSIZE];
	int nfr[FSCLSIZE], oldnb;
	struct inode *ip;
        union xptentry prev;
	struct movedfrag *ptr, * v_insertfrag();

	assert(scb_pseg(sidx));
	SCB_LOCKHOLDER(sidx);

	/* get pageing device table index and the number of
	 * fragments per page.
	 */
        pdtx = scb_devid(sidx);

	FS_LOCKHOLDER(pdtx);

	fperpage = pdt_fperpage(pdtx);
	ASSERT(nfrags > 0 && nfrags <= fperpage);

	/* pickup the inode and touch it.
	 */
	ip = GTOIP(scb_gnptr(sidx));
	TOUCH(ip)
	TOUCH((char *)(ip + 1) - 1);


	/* determine the number of new fragments required for the page.
	 */
	newfrags = (xpt->fptr.addr == 0) ? nfrags :
				 nfrags - (fperpage - xpt->fptr.nfrags);
	ASSERT(newfrags > 0);

	/* make sure the user's and/or group's quota can handle
	 * the new fragments.
	 */
	if (rc = v_allocdq(ip,newfrags,fperpage))
		return(rc);

	/* if the page currently has uncommitted fragments, these
	 * fragments may be freed below, so the map page containing
	 * the fragments is touched here to insure that the free 
	 * operation will not fault. if the fragments are old, they
	 * must be remembered in a movedfrag structure.
	 * Note for MP-eff that the caller to v_falloc() must have
	 * acquired the LW lock if v_insertfrag() might be called.
	 */
	if (xpt->newbit)
	{
		v_touchmap(pdtx,xpt->fptr.addr);
	}
	else if (xpt->fptr.addr)
	{
		sid = ITOS(sidx, 0); 
		if ((ptr = v_insertfrag(sid,pno)) == NULL)
			return ENOMEM;
	}

	/* if the page has uncommitted fragments and the page's
	 * allocation is being extended, try to satisfy the request
	 * by expanding the current allocation in-place (i.e. try to
	 * allocate the newfrags fragments immediately following the
	 * current allocation).
 	 */
	if (xpt->newbit)
	{
		if (v_fallocnxt(pdtx,xpt->fragptr,newfrags) == 0)
		{
			/* in-place expansion successful.  zero the
			 * address in the xpt so the current fragments
			 * will not be freed below.
			 */
			fragno = xpt->fptr.addr;
			xpt->fptr.addr = 0;
			goto common;
		}
	}

	/* get the hints that will be used below. the hints are based
	 * on the inode's current resource allocations.  prev is the
	 * previous page's disk allocation and is used in attempting
	 * to allocate the fragments which follow it. prev
	 * is zero if pno is on an xpt boundary or the previous page
	 * has no allocation.  hint is either the first fragment
	 * of the previous page's allocation or the first fragment
	 * of the allocation group containing the disk inode.
	 */
	if ((pno & (PSIZE / 4 - 1)) && (xpt-1)->word)
	{
		prev.word = (xpt-1)->word;
		hint = (xpt-1)->fptr.addr;
	}
	else
	{
		prev.word = 0;
		hint = (ip->i_number / scb_iagsize(sidx)) * scb_agsize(sidx);
	}

	/* if the previous page was on an xpt boundry or had
	 * no allocation, just allocate using hint.
	 */
	if (prev.word == 0)
		goto allocate;

	/* try to allocate the fragments after the prior page's 
	 * allocation.
	 */
	if (v_allocnxt(pdtx,prev.fptr, nfrags) == 0)
	{
		fragno = prev.fptr.addr + fperpage - prev.fptr.nfrags;
		goto common;
	}

       	/* if pno is on an allocation boundary FSCLSIZE (always
	 * a power of 2 ) don't reallocate. ditto if segment
	 * is journalled.
	 */
       	if ((nb = pno & (FSCLSIZE - 1)) == 0 || scb_jseg(sidx))
		goto allocate;

	/* or if compression applies don't go thru reallocation.
	 */
	if (scb_compress(sidx)) goto allocate;

	/* try to reallocate the blocks:  give back oldnb 
	 * fragments prior to pno in the same cluster and
	 * allocate oldnb + nfrags consecutive new frags. 
	 * the pages must be in memory and the blocks new.
	 * remember the blocks and page frame numbers. 
	 * note that old[] does not cover the current page; if
	 * it has fragments, that's handled below at common:
	 */
	oldnb = 0;
	for (k = nb; k > 0; k--)
	{
		if ((xpt - k)->newbit == 0)
			goto allocate;
		old[nb - k] = (xpt - k)->fragptr;
		oldnb += fperpage - (xpt - k)->fptr.nfrags;
		nfr[nb - k] = v_lookup(ITOS(sidx, pno-k), BASEPAGE(pno-k));
		if (nfr[nb - k] < 0 || pft_inuse(nfr[nb - k]) == 0)
			goto allocate;
	}

	/* try to reallocate oldnb + nfrags new fragments.
	 */
	if (v_realloc(pdtx,nb,old,&fragno, oldnb + nfrags) == 0)
	{
		/* fix up xpts and page frames. 
		 */
		for (k = nb; k > 0; k--)
		{
			(xpt - k)->fptr.addr = fragno;
			pft_fraddr(nfr[nb - k]) = fragno;
			SETMOD(nfr[nb - k]);
			fragno += fperpage - (xpt -k)->fptr.nfrags;
		}
		goto common;
	}

	/* allocate nfrags
	 */
	allocate:

	/* try to allocate the block of fragments using a placement
	 * hint.
	 */
       	if (v_psalloc(pdtx,nfrags,hint,&fragno))
               	return(ENOSPC);

	common:

	/* if the page currently has fragments, they must be freed.  if
	 * the fragments are uncommitted, they will be freed here through
	 * a call to v_dfree(); otherwise, the fragments will be recorded
	 * in movedfrag structure to be freed when the inode is committed.
	 */
	if (xpt->fptr.addr)
	{
		if (xpt->newbit)
			v_dfree(pdtx,xpt->fragptr);
		else
		{
			ptr->olddisk[pno & 0x3] = xpt->fragptr;
		}
	}

	/* adjust scb_newdisk if the page had no current allocation 
	 * or a committed current allocation.
	 */
	if (!xpt->newbit)
		scb_newdisk(sidx) += 1;

	/* set the new disk address in the xpt.
	 */
        xpt->newbit = 1;
        xpt->fptr.addr = fragno;
        xpt->fptr.nfrags = fperpage - nfrags;

	/* adjust i_nblocks and i_size to reflect the new allocation.
	 */
	ip->i_nblocks += newfrags;
	ip->i_size = MAX(ip->i_size,
				(pno * PSIZE) + (nfrags * (PSIZE / fperpage)));

	/* update the inode's quota information to reflect the
	 * new fragments.
	 */
	v_upddqblks(ip,newfrags,fperpage);

	/* performance trace hook.
	 */
	sid = ITOS(sidx,pno);
	TRCHKL5T(HKWD_VMM_DALLOC,sid,scb_sibits(sidx),pno,pdtx,xpt->fragptr);

	/* set nohomeok bit in page frame underlying xpt
	 */
	v_nohomeok(xpt);

        return(0);
}

/*
 * lpreadonly(sidx,pno)
 * 
 * returns true if pno is the last page in a file and it must
 * be made read-only because the file is mapped for writing.
 * returns false otherwise.
 */
static int
lpreadonly(sidx, pno)
int sidx; /* index in scb table */
int pno; /* page number in scb */
{
	struct gnode * gp;
	struct inode * ip;

	SCB_LOCKHOLDER(sidx);

	/* check for map by writer
	 */
	gp = scb_gnptr(sidx);
	if (gp->gn_segcnt == 0)
		return 0;

	/* check if pno isn't last page or size is set to end of page.
	 */
	ip = GTOIP(gp);
	if (pno != BTOPN(ip->i_size) || (pno + 1) << L2PSIZE == ip->i_size)
		return 0;

	/* mapped for writing and pagex is last page
	 */
	return 1;
}

