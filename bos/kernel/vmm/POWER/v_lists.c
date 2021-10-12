static char sccsid[] = "@(#)97	1.49  src/bos/kernel/vmm/POWER/v_lists.c, sysvmm, bos41J, 9511A_all 3/7/95 16:39:55";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	v_insfree, v_delscb, v_insscb, v_mtioscb,
 *		v_mfioscb, v_getframe,
 *		v_inspft, v_delpft, v_lookup, v_lookupx, v_reload,
 *		v_getfree, v_insapt, v_delapt, v_delaptall, v_aptkey
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
 * routines for manipulating various page frame lists:
 */

#include "vmsys.h"
#include <sys/syspest.h>
#include <sys/trchkid.h>
#include <sys/errno.h>
#include "mplock.h"

int vmm_max_rpages = 0;

/*
 * v_insfree
 * put page frame on free list, increment vmker.numfrb and set
 * pft_free to 1.
 * operations in this routine are protected by the vmker and scb lock.
 */

v_insfree(nfr)
int nfr;        /* page frame number to put on free list */
{
        int nxt,prv;

	/*
	 * In general, callers of v_insfree should hold the SCB lock.
	 * This is not true on the path from vm_relframe because the
	 * frame is not associated with any SCB.
	 *
	 * SCB_LOCKHOLDER(STOI(pft_ssid(nfr)));
	 */
	VMKER_MPLOCK();

        pft_free(nfr) = 1;
        pft_inuse(nfr) = 0;
        vmker.numfrb += 1;

	/* put at head of list */
	nxt = pft_freefwd(FBANCH);
	pft_freefwd(nfr) = nxt;
	pft_freebwd(nxt) = nfr;
	pft_freebwd(nfr) = FBANCH;
	pft_freefwd(FBANCH) = nfr;

	while(pf_freewait != NULL)
	{
		v_ready(&pf_freewait);
	}

	VMKER_MPUNLOCK();

	/* can be called via vcs_ and so must return a value */
	return 0;
}

/*
 * v_delscb(sidx,nfr)
 * remove page frame from scb list, decrement scb_npages
 * and set pft_slist to 0. the page frame is assumed NOT
 * to be in the i/o part of the list. to remove a page frame
 * is in the i/o sublist use v_mfioscb followed by this
 * routine.
 */

v_delscb(sidx,nfr)
int sidx;       /* index in scb array */
int nfr;        /* page frame number to delete from list*/
{
        int nxt,prv;

	SCB_LOCKHOLDER(sidx);
	ASSERT(STOI(pft_ssid(nfr)) == sidx && pft_slist(nfr));

	/* performance trace hook - page delete.
	 */
	TRCHKL4T(HKWD_VMM_DELETE,pft_ssid(nfr),scb_sibits(sidx),
		pft_spage(nfr),nfr);

	/* update counts 
	 */
	ASSERT(!pft_free(nfr));

	scb_npages(sidx) += -1;

	if (!scb_compseg(sidx))
		FETCH_AND_ADD(vmker.numperm, -1);

	if (scb_clseg(sidx))
		FETCH_AND_ADD(vmker.numclient, -1);
	else if (scb_compress(sidx))
		FETCH_AND_ADD(vmker.numcompress, -1);

        pft_slist(nfr) = 0;
        nxt = pft_sidfwd(nfr);
        prv = pft_sidbwd(nfr);

        /* was it at head of list ? */
        if (scb_sidlist(sidx) == nfr)
        {
                scb_sidlist(sidx) = nxt;
        }
        else
        {
                pft_sidfwd(prv) = nxt;
        }

        if (nxt >= 0)
                pft_sidbwd(nxt) = prv;

        pft_sidfwd(nfr) = pft_sidbwd(nfr) = -1;
}

/*
 * v_insscb(sidx,nfr)
 * insert page frame on scb list, increment scb_npages,
 * and set pft_slist to 1. the page is inserted after
 * pages in i/o state, if any. if the page frame being
 * inserted is in pageout state the non-fifo field is
 * set to one and otherwise to zero.
 */

v_insscb(sidx,nfr)
int sidx;       /* index in scb array */
int nfr;        /* page frame number to insert in list */
{
        int io,head,iotail,nxt;

	SCB_LOCKHOLDER(sidx);

	ASSERT(!pft_slist(nfr) && !pft_free(nfr));

	/* performance trace hook - page assign.
	 */
	TRCHKL4T(HKWD_VMM_ASSIGN,pft_ssid(nfr),scb_sibits(sidx),
		pft_spage(nfr),nfr);

	/* update counts
	 */
	scb_npages(sidx) += 1;

	if (!scb_compseg(sidx))
		FETCH_AND_ADD(vmker.numperm, 1);

	if (scb_clseg(sidx))
		FETCH_AND_ADD(vmker.numclient, 1);
	else if (scb_compress(sidx))
		FETCH_AND_ADD(vmker.numcompress, 1);

        pft_slist(nfr) = 1;
        pft_nonfifo(nfr) = (pft_pageout(nfr)) ? 1 : 0;
        io = (pft_pagein(nfr) | pft_pageout(nfr));
        head = scb_sidlist(sidx);
        iotail = (head < 0) ? -1 : pft_sidbwd(head);

        /*
         * was there i/o previously ? insert after iotail if so
         */

        if (iotail >= 0)
        {
                nxt = pft_sidfwd(iotail);
                pft_sidfwd(nfr) = nxt;
                pft_sidfwd(iotail) = nfr;
                pft_sidbwd(nfr) = iotail;
                if (nxt >= 0)
                         pft_sidbwd(nxt) = nfr;
                if (io)
                {
                        pft_sidbwd(head) = nfr;  /* nfr is iotail */
                }
                return;
        }

        /*
         * no i/o previously. insert at head of list.
         */

        scb_sidlist(sidx) = nfr;
        pft_sidfwd(nfr) = head;
        if (head >= 0)
                pft_sidbwd(head) = nfr;

        if (io)
        {
                pft_sidbwd(nfr) = nfr;  /* nfr is iotail */
        }
        else
        {
                pft_sidbwd(nfr) = -1;
        }
        return;
}

/*
 * v_mtioscb(sidx,nfr)
 * move a page frame to the tail of the i/o part of an scb
 * list. on entry the page frame is on the scb list but
 * NOT currently in the i/o part of the list. if the
 * segment is in pageout state the non-fifo field is
 * initialized to one and otherwise to zero.
 */

v_mtioscb(sidx,nfr)
int sidx;       /* index in scb array */
int nfr;        /* page frame number to move */
{
        int io,head,iotail,nxt,prv;

	SCB_LOCKHOLDER(sidx);
	ASSERT(STOI(pft_ssid(nfr)) == sidx && pft_slist(nfr));

        /*
         * if pageout state set non-fifo to 1, otherwise 0.
         */
        pft_nonfifo(nfr) = (pft_pageout(nfr)) ? 1 : 0;

        /*
         * is it already at head ?
         */
        head = scb_sidlist(sidx);
        if (head == nfr)
        {
                pft_sidbwd(nfr) = nfr;
                return;
        }

        /*
         * remove it from list.
         */
        nxt = pft_sidfwd(nfr);
        prv = pft_sidbwd(nfr);  /* this is non-null */
        pft_sidfwd(prv) = nxt;
        if (nxt >= 0)
                pft_sidbwd(nxt) = prv;

        /*
         * re-insert after iotail if it is non-null
         */
        iotail = pft_sidbwd(head);
        if (iotail >= 0)
        {
                nxt = pft_sidfwd(iotail);
                pft_sidfwd(iotail) = nfr;
                pft_sidbwd(nfr) = iotail;
                pft_sidfwd(nfr) = nxt;
                if (nxt >= 0)
                        pft_sidbwd(nxt) = nfr;
                pft_sidbwd(head) = nfr;  /* nfr is new iotail */
                return;
        }

        /*
         * iotail was null. insert at head of list.
         */
        scb_sidlist(sidx) = nfr;
        pft_sidfwd(nfr) = head;  /* this is non-null */
        pft_sidbwd(head) = nfr;
        pft_sidbwd(nfr) = nfr;  /* nfr is new iotail */

        return;
}

/*
 * v_mfioscb(sidx,nfr)
 * move a page frame from the i/o part of an scb list
 * to the non-i/o part of the list. on entry the page
 * is in the i/o part of the list. if the page was in
 * pageout state and the segment is a not a working
 * storage segment iolevel processing is done, and the
 * non-fifo cleared. non-fifo field was set by v_mtioscb
 * or v_insscb when it was moved to the io sublist.
 */

v_mfioscb(sidx,nfr)
int sidx;       /* index in scb array */
int nfr;        /* page frame number to move/delete from list*/
{
        int k,nxt,prv,head,newhead,iotail;

	SCB_LOCKHOLDER(sidx);
	ASSERT(STOI(pft_ssid(nfr)) == sidx && pft_slist(nfr));

        head = scb_sidlist(sidx);
        iotail = pft_sidbwd(head);

        /*
         * was nfr at head of list ?
         */
        if (head == nfr)
        {
                /*
                 * update iolevel if pageout and not working storage
                 */
                if (scb_wseg(sidx) == 0 && pft_pageout(nfr))
                {
                        scb_iolev(sidx) += pft_nonfifo(nfr);
                }
                pft_nonfifo(nfr) = 0;

                /*
                 * only thing on iolist ?
                 */
                if (iotail == nfr)
                {
                        /* no move needed */
                        pft_sidbwd(nfr) = -1; /* iotail null */
                        return;
                }

                /*
                 * delete nfr from head of list
                 */
                newhead = pft_sidfwd(nfr);
                scb_sidlist(sidx) = newhead; /* non-null */
                pft_sidbwd(newhead) = iotail; /* iotail unchanged */

                /*
                 * re-insert nfr after iotail
                 */
                nxt = pft_sidfwd(iotail);
                pft_sidfwd(iotail) = nfr;
                pft_sidbwd(nfr) = iotail;
                pft_sidfwd(nfr) = nxt;
                if (nxt >= 0)
                        pft_sidbwd(nxt) = nfr;
                return;
        }

        /*
         * nfr not at head.
         * update nonfifo count of next previous page frame
         * in pageout state or the iolevel of the segment, if
         * there is no such page frame.
         */
        prv = pft_sidbwd(nfr);
        if (scb_wseg(sidx) == 0 && pft_pageout(nfr))
        {
                for(k = prv;  ; k = pft_sidbwd(k))
                {
                        if(pft_pageout(k))
                        {
                                pft_nonfifo(k) += pft_nonfifo(nfr);
                                break;
                        }
                        /*
                         * no previous frame if k is head
                         */
                        if (k == head)
                        {
                                scb_iolev(sidx) += pft_nonfifo(nfr);
                                break;
                        }
                }
        }
        pft_nonfifo(nfr) = 0;

        if (iotail == nfr)
        {
                /* don't have to move */
                pft_sidbwd(head) = prv;  /* new iotail */
                return;
        }

        /*
         * remove from list. it is neither first nor last in list.
         */
        nxt = pft_sidfwd(nfr);  /* this is non-null */
        pft_sidfwd(prv) = nxt;
        pft_sidbwd(nxt) = prv;

        /*
         * re-insert nfr after iotail
         */
        nxt = pft_sidfwd(iotail);
        pft_sidfwd(iotail) = nfr;
        pft_sidbwd(nfr) = iotail;
        pft_sidfwd(nfr) = nxt;
        if (nxt >= 0)
                pft_sidbwd(nxt) = nfr;
        return;
}

/*
 * v_getframe(sid,page)
 *
 * on entry the free frame list is not empty. this
 * procedure takes the first frame off the free list
 * and returns its page frame number. 
 *
 * input parameters
 *		sid - segment id
 *		page - page within sid.
 *
 * the following fields in the pft are initialized
 *
 * (1) words 0, 2, and 3 of the hardware pft   
 *     (everything but hash pointers) set to zero.   
 *
 * (2) pft_swbits , pft_logage (pft_pincount) and
 *     pft_waitlist set to zero.
 *
 * (3) pft_ssid set to sid, pft_pagex set to page
 *     number within scb.
 *
 * RETURN VALUE - page frame allocated.
 */

int 
v_getframe(sid,page)
int sid;
int page;
{
	int nfr,nxt,sidx,prv;
	struct ppda* p_ppda;

	SCB_LOCKHOLDER(STOI(sid));

	VMKER_MPLOCK();

	if (vmm_max_rpages < vmker.nrpages - vmker.numfrb)
		vmm_max_rpages = vmker.nrpages - vmker.numfrb;

        /* remove head of free list.
         */
        nfr = pft_freefwd(FBANCH);
	assert(nfr != FBANCH);
        nxt = pft_freefwd(nfr);
        pft_freefwd(FBANCH) = nxt;
        pft_freebwd(nxt) = FBANCH;
#ifndef _VMM_MP_EFF
	vmker.numfrb += -1;
#else 
	p_ppda = PPDA;
	USE_RESERVATION(p_ppda);
#endif
 
	VMKER_MPUNLOCK();

	ASSERT(!pft_slist(nfr));

        /* init various fields to zero.
         */
        pft_freefwd(nfr) = pft_freebwd(nfr) = 0;
	pft_swbits(nfr) = pft_swbits2(nfr) = 0;

	/* init source sid and source pagex fields
	 */
	pft_ssid(nfr) = sid;
	pft_pagex(nfr) = SCBPNO(sid,page);

	/* all pages managed on the free list use
	 * default storage control attributes.
         */
	pft_wimg(nfr) = WIMG_DEFAULT;

	/* insert carefully on s/w hash chain.
	 */
	v_inspft(sid,page,nfr);

        return(nfr);
}

/*
 * v_getfree()
 *
 * Similar to v_getframe but used by performance tools to steal frames
 * from the free list -- they are not assigned a sid,pno.  Frames allocated
 * in this manner are equivalent to frames corresponding to bad memory
 * (no s/w bits set) until they are placed back on the free list.
 *
 * RETURN VALUE - page frame allocated or FBANCH if no free frames.
 */

int
v_getfree()
{
	int nfr, nxt;

	VMKER_MPLOCK();

        /* remove head of free list.
         */
        nfr = pft_freefwd(FBANCH);
	if (nfr == FBANCH)
	{
		VMKER_MPUNLOCK();
		return(FBANCH);
	}
        nxt = pft_freefwd(nfr);
        pft_freefwd(FBANCH) = nxt;
        pft_freebwd(nxt) = FBANCH;

	vmker.numfrb += -1;

	VMKER_MPUNLOCK();

	ASSERT(!pft_slist(nfr));

	/* init various fields to zero.
	 */
        pft_freefwd(nfr) = pft_freebwd(nfr) = 0;
	pft_swbits(nfr) = pft_swbits2(nfr) = 0;

	return(nfr);
}

#ifdef _POWER_MP
/*
 * Storage defined for SWHASH MP locks.
 * These locks must reside in V=R storage.  They are padded
 * to ensure that each lock resides in a different cache line
 * (assumed to be a maximum of 64 bytes).  This padding must
 * also be large enough to contain the lock instrumentation
 * data that is recorded by the rsimple_lock/unlock services.
 */
struct cache_line_lock swhash_lock[NUM_SWHASH_LOCKS] = { SIMPLE_LOCK_AVAIL };

#endif /* _POWER_MP */

/*
 * v_inspft(sid,pno,nfr)
 *
 * Insert a page frame on the software hash chain.
 */

v_inspft(sid,pno,nfr)
int     sid;    /* segment id */
int     pno;    /* page number in segment */
int     nfr;    /* page frame number    */
{
	int ipri;
	int hash;

	SCB_LOCKHOLDER(STOI(sid));

	ASSERT(v_lookup(sid,pno) == -1);

        hash = SWHASH(sid,pno);
	SWHASH_MPLOCK(ipri,sid,pno);

	/*
	 * Use 'volatile' so the compiler keeps the assignments in order
	 * so the hash chain update is atomic.
	 */
        pft_next(nfr) = *(volatile int *)&hattab[hash];
        hattab[hash] = nfr;

	SWHASH_MPUNLOCK(ipri,sid,pno);
}

/*
 * v_delpft(nfr)
 *
 * Delete a page frame from the software hash chain.
 */

v_delpft(nfr)
int     nfr;    /* page frame number */
{
	int ipri;
	int sid, pno;
	int hash, prev;

	sid = pft_ssid(nfr);
	pno = pft_spage(nfr);

	SCB_LOCKHOLDER(STOI(sid));

        hash = SWHASH(sid,pno);
	SWHASH_MPLOCK(ipri,sid,pno);
        prev = SWFIRSTNFR(hash);
        if (prev == nfr)
        {
                hattab[hash] = pft_next(nfr);
        }
        else  /* not at head of list */
        {
                while(pft_next(prev) != nfr)
                {
                        prev = pft_next(prev);
                }
                pft_next(prev) = pft_next(nfr);
        }
	SWHASH_MPUNLOCK(ipri,sid,pno);
}

/*
 * v_lookup(sid,pno)
 *
 * Determine if a page is resident using the software and alias hashes.
 * Return the page frame number or -1 if not found.
 *
 * The VMM data segment must be addressible on entry.
 */
int 
v_lookup(sid,pno)
int     sid;    /* segment id */
int     pno;    /* page number in segment */
{
        int hash, nfr;
	int ipri;
	ushort ahash, aptx;

	/*
	 * Search the software PFT hash.
	 */
        hash = SWHASH(sid,pno);

	SWHASH_MPLOCK(ipri,sid,pno);
        for (nfr = SWFIRSTNFR(hash); nfr >= 0; nfr = pft_next(nfr))
        {
                if (pft_ssid(nfr) == (sid & IOSIDMASK) && pft_spage(nfr) == pno)
		{
			SWHASH_MPUNLOCK(ipri,sid,pno);
			return(nfr);
		}
        }

	/*
	 * Search the alias hash.
	 */
        ahash = AHASH(sid,pno);

	for (aptx = ahattab[ahash]; aptx != APTNULL; aptx = apt_next(aptx))
	{
		if (apt_sid(aptx) == sid && apt_pno(aptx) == pno)
		{
			SWHASH_MPUNLOCK(ipri,sid,pno);
			return(apt_nfr(aptx));
		}
	}

	SWHASH_MPUNLOCK(ipri,sid,pno);
        return(-1);
}

/*
 * v_lookupx(sid,pno,&key,&attr)
 *
 * Determine if a page is resident using the software and alias hashes.
 * Return the page frame number or -1 if not found.  Also return page
 * protection key and storage control attributes.
 *
 * The VMM data segment must be addressible on entry.
 */
int 
v_lookupx(sid,pno,key,attr)
int     sid;    /* segment id */
int     pno;    /* page number in segment */
int     *key;   /* returned protection key */
int     *attr;  /* returned storage control attributes */
{
        int hash, nfr;
	int ipri;
	ushort ahash, aptx;

	/*
	 * Search the software PFT hash.
	 */
        hash = SWHASH(sid,pno);

	SWHASH_MPLOCK(ipri,sid,pno);
        for (nfr = SWFIRSTNFR(hash); nfr >= 0; nfr = pft_next(nfr))
        {
                if (pft_ssid(nfr) == (sid & IOSIDMASK) && pft_spage(nfr) == pno)
		{
			SWHASH_MPUNLOCK(ipri,sid,pno);
			*key = pft_key(nfr);
			*attr = pft_wimg(nfr);
			return(nfr);
		}
        }

	/*
	 * Search the alias hash.
	 */
        ahash = AHASH(sid,pno);

	for (aptx = ahattab[ahash]; aptx != APTNULL; aptx = apt_next(aptx))
	{
		if (apt_sid(aptx) == sid && apt_pno(aptx) == pno)
		{
			SWHASH_MPUNLOCK(ipri,sid,pno);
			*key = apt_key(aptx);
			*attr = apt_wimg(aptx);
			return(apt_nfr(aptx));
		}
	}

	SWHASH_MPUNLOCK(ipri,sid,pno);
        return(-1);
}

/*
 * v_reload(sid,pno)
 *
 * This routine is called from the dsis_flih to determine if a fault is
 * a valid reload fault.  It is called with interrupts disabled and with
 * translation disabled (thus all data structures must be accessed using
 * real addresses).  On MP, both the SWHASH and PMAP_NFR locks must be
 * acquired to serialize with page state transitions.
 *
 * This routine determines if a page is resident using the software hash
 * and alias hash and if the frame is in valid state for reloading.
 *
 * If it is a valid reload fault then the frame is entered in the page table
 * and 0 is returned.  Otherwise -1 is returned.
 */
int 
v_reload(sid,pno)
int     sid;    /* segment id */
int     pno;    /* page number in segment */
{
        int hash, nfr;
	int *Rhat;
	struct pftsw *Rpft;
	extern int BF_on;
	ushort ahash, aptx;
	ushort *Rahat;
	struct apt *Rapt;
	int key, wimg;
	extern struct cache_line_lock pmap_nfr_lock[];

	/*
	 * Search the software PFT hash.
	 */
	Rhat = (int *) vmrmap_raddr(RMAP_SWHAT);
	Rpft = (struct pftsw *) vmrmap_raddr(RMAP_SWPFT);
        hash = SWHASH(sid,pno);
	SWHASH_INT_MPLOCK(sid,pno);

        for (nfr = *(Rhat+hash); nfr >= 0; nfr = (Rpft+nfr)->_next)
        {
                if ((Rpft+nfr)->_ssid == (sid & IOSIDMASK) &&
		    (Rpft+nfr)->u2.s1._spage == pno)
		{
			key = (Rpft+nfr)->u1.s1._key;
		        wimg = (Rpft+nfr)->u6.s1._wimg;
			/*
			 * Acquire the PMAP_NFR lock to serialize with
			 * page state transitions only for pages
			 * corresponding to real frames.
			 * The IPL control block has a pft entry despite
			 * having an nfr beyond the last physical page
			 * address!
			 */
			if (!PHYS_MEM(nfr))
				goto reload;
			PMAP_NFR_MPLOCK(nfr);
			key = (Rpft+nfr)->u1.s1._key;
		        wimg = (Rpft+nfr)->u6.s1._wimg;
			goto found;
		}
        }

	/*
	 * Search the alias hash.
	 */
	Rahat = (ushort *) vmrmap_raddr(RMAP_AHAT);
	Rapt = (struct apt *) vmrmap_raddr(RMAP_APT);
        ahash = AHASH(sid,pno);

        for (aptx = *(Rahat+ahash); aptx != APTNULL;
	     aptx = (Rapt+aptx)->u3.s3._next)
        {
                if ((Rapt+aptx)->_sid == sid &&
	            (Rapt+aptx)->u1.s1._pno == pno &&
		    (Rapt+aptx)->u2.s2._valid)
		{
			nfr = (Rapt+aptx)->u2.s2._nfr;

			/*
			 * For T=0 mappings (page frame doesn't correspond
			 * to physical memory) skip page state checks.
			 * Fortunately, such mappings are never removed from
			 * the alias page table so no serialization is needed
			 * on MP.
			 */
			if (!PHYS_MEM(nfr))
			{
				key = (Rapt+aptx)->u2.s2._key;
				wimg = (Rapt+aptx)->u2.s2._wimg;
				goto reload;
			}
			/*
			 * Acquire the PMAP_NFR lock to serialize with
			 * page state transitions.
			 */
			PMAP_NFR_MPLOCK(nfr);
			key = (Rapt+aptx)->u2.s2._key;
		        wimg = (Rapt+aptx)->u2.s2._wimg;
			goto found;
		}
	}

	SWHASH_INT_MPUNLOCK(sid,pno);
        return(-1);

found:

	/*
	 * Page is resident. Reload is allowed if:
	 *
	 * - the faulting sid is an I/O sid and
	 *	- the frame is in Pagein or Pageout state
 	 * or:
	 *
	 * - the faulting sid is a normal sid and
	 *	- the frame is in InUse state (not Hidden and not
	 *	  PgAhead)
	 */
	if ( (IOVADDR(sid) &&
		((Rpft+nfr)->u1.s1._pagein ||
		 (Rpft+nfr)->u1.s1._pageout))
	||
	     (!IOVADDR(sid) &&
		(Rpft+nfr)->u1.s1._inuse &&
		!(Rpft+nfr)->u6.s1._xmemcnt &&
		!(Rpft+nfr)->u1.s1._pgahead))
	{
		/*
		 * We only want to perform reloads when absolutely necessary
		 * so the BigFoot tool sees as many faults as possible.  
		 * We can allow the normal fault handler to deal with faults
		 * occuring in the process environment when mstintpri is
		 * INTBASE.  Unfortunately, we can't examine mstintpri since
		 * we're xlate off and MSTs aren't V=R.  We detect faults
		 * in a process environment by determining that CSA points to
		 * an MST other than an interrupt MST in the kernel segment.
		 * We approximate the test for INTBASE faults by avoiding
		 * reloads on non-pinned pages.  This means we won't attempt
		 * to reload pageable pages touched in stackfix critical
		 * sections.  Hopefully, only a few page references are made
		 * during stackfix critical sections so that the carefully
		 * touched pages remain in BigFoot's list and don't get
		 * unmapped (otherwise the page-fault handler will detect a
		 * bad fault and crash the system).
		 */
		if (BF_on && (((int)CSA & SREGMSK) != 0) &&
			(!(Rpft+nfr)->u1.s1._journal &&
			     (Rpft+nfr)->u4.s3.u7._pincount == 0))
		{
			PMAP_NFR_MPUNLOCK(nfr);
			SWHASH_INT_MPUNLOCK(sid,pno);
			return(-1);
		}

reload:
		p_enter(RELOAD, sid, pno, nfr, key, wimg);
		vmker.reloads++;
		/* p_enter has released the PMAP_NFR lock */
		SWHASH_INT_MPUNLOCK(sid,pno);
		return(0);
	}

	PMAP_NFR_MPUNLOCK(nfr);
	SWHASH_INT_MPUNLOCK(sid,pno);
	return(-1);
}

/*
 * v_insapt(type,sid,pno,nfr,key,wimg)
 *
 * Insert an entry in the alias page table.
 *
 * Returns:
 *
 *  0 if entry was made or if type was APTREG and table was full
 *  EAGAIN if the entry already exists but is marked invalid
 *  ENOMEM if type is APTPIN and the entire table is full of pinned entries
 */

v_insapt(type,sid,pno,nfr,key,wimg)
int     type;   /* type of operation */
int     sid;    /* segment id */
int     pno;    /* page number in segment */
int     nfr;    /* page frame number    */
int     key;    /* page protection key */
int     wimg;   /* storage control attributes */
{
	ushort last, aptx, next, hash;
	int alist = 0, wrapped = 0;
	int ipri;

	SCB_LOCKHOLDER(STOI(sid));

	/*
	 * See if the entry already exists.
	 */
	for (aptx = pft_alist(nfr); aptx != APTNULL; aptx = apt_anext(aptx))
	{
		if (apt_sid(aptx) == sid && apt_pno(aptx) == pno)
		{
			/*
			 * Entry exists.  If it is invalid then the alias has
			 * an outstanding pin operation and cannot be re-used
			 * until that operation completes.
			 */
			ASSERT(apt_nfr(aptx) == nfr);
			if (!apt_valid(aptx))
			{
				return(EAGAIN);
			}
		
			/*
			 * Mark the entry as pinned if requested.
			 */
			if (type == APTPIN)
			{
				apt_pinned(aptx) = 1;
			}
			return(0);
		}
	}

#ifdef _POWER_PC
        /*
         * For PowerPC we utilize the aliasing capabilities of the
         * hardware so there is little benefit in creating APT entries
         * for regular mappings.  We still require entries for pinned
	 * alias ranges and so even on PowerPC we need to perform the
	 * check above for an outstanding pin operation.
         * For MP, it is difficult to serialize replacement of APT entries
         * so it's just as well that we don't allow the need for replacement.
         * This check assumes that MP is only ever supported for PowerPC.
         */
        if (__power_pc())
        {
                if (type == APTREG)
                        return(0);
        }
#endif  /* _POWER_PC */

	/* 
	 * If no free entries, find one to replace.
	 * For MP-eff, serialize access to the APT free list.  We don't
	 * replace entries on MP-eff since they are all pinned but we
	 * still have to handle the case of a table full of pinned entries.
	 */
	APT_MPLOCK();
	if (pf_aptfree == APTNULL)
	{
		for (aptx = pf_aptlru; ; aptx++)
		{
			/*
			 * Handle wrap around.
			 */
			if (aptx >= vmrmap_size(RMAP_APT)/sizeof(struct apt))
			{
				aptx = 0;
				wrapped = 1;
			}

			/*
			 * If every entry is pinned return failure if the
			 * request is to pin the new entry.
			 */
			if (wrapped && aptx == pf_aptlru)
			{
				APT_MPUNLOCK();
				if (type == APTPIN)
					return(ENOMEM);
				else
					return(0);
			}

			if (!apt_pinned(aptx))
				break;
		}

		/*
		 * Note that we don't get here on an MP machine but we do
		 * need to avoid recursive locking in order to run an MP
		 * kernel on a UP machine.
		 */
		APT_MPUNLOCK();
		v_delapt(APTREG, apt_sid(aptx), apt_pno(aptx), apt_nfr(aptx));
		APT_MPLOCK();
		pf_aptlru = aptx + 1;
	}

	/*
	 * Allocate a free entry.
	 */
	next = apt_free(pf_aptfree);
	aptx = pf_aptfree;
	pf_aptfree = next;
	APT_MPUNLOCK();

	/*
	 * Initialize the entry.
	 */
	apt_sid(aptx) = sid;
	apt_pno(aptx) = pno;
	apt_key(aptx) = key;
	apt_wimg(aptx) = wimg;
	apt_nfr(aptx) = nfr;
	apt_pinned(aptx) = (type & APTPIN) ? 1 : 0;
	apt_valid(aptx) = 1;
	apt_free(aptx) = APTNULL;

	/*
	 * Add it to the list of aliases for the frame.
	 */
	apt_anext(aptx) = pft_alist(nfr);
	pft_alist(nfr) = aptx;
	
	/*
	 * Finally, add it to the alias hash chain.
	 * Use 'volatile' so the compiler keeps the assignments in order
	 * so the hash chain update is atomic.  For MP, the SWHASH lock is
	 * required to synchronize this update with lookup operations.
	 */
        hash = AHASH(sid,pno);
	SWHASH_MPLOCK(ipri,sid,pno);
        apt_next(aptx) = *(volatile ushort *)&ahattab[hash];
        ahattab[hash] = aptx;
	SWHASH_MPUNLOCK(ipri,sid,pno);
	return 0;
}

/*
 * v_delapt(type,sid,pno,nfr)
 *
 * Delete an entry from the alias page table.
 *
 * Returns:
 *  0 - no pinned apt entry exists for the frame
 *  1 - a pinned apt entry does exist
 */

v_delapt(type,sid,pno,nfr)
int     type;   /* type of operation */
int     sid;    /* segment id */
int     pno;    /* page number in segment */
int     nfr;    /* page frame number */
{
	ushort hash, aptx, prev, this;
	int ipri;

	/*
	 * Remove the entry from the page frame alias list if it exists.
	 */
	prev = APTNULL;
	for (aptx = pft_alist(nfr); aptx != APTNULL; aptx = apt_anext(aptx))
	{
		if (apt_sid(aptx) == sid && apt_pno(aptx) == pno)
		{
			/*
			 * If entry is pinned and this is a regular delete
			 * just mark the entry invalid -- it will be deleted
			 * later when the frame is unpinned.
			 */
			if (apt_pinned(aptx) && type == APTREG)
			{
				apt_valid(aptx) = 0;
				return(1);
			}

			if (prev == APTNULL)
			{
				pft_alist(nfr) = apt_anext(aptx);
			}
			else  /* not at head of list */
			{
				apt_anext(prev) = apt_anext(aptx);
			}
			break;
		}
		prev = aptx;
	}
	if (aptx == APTNULL)
	{
		ASSERT(type != APTPIN);
		return(0);
	}

	/*
	 * Remove the entry from the alias hash chain.
	 * For MP, the SWHASH lock is required to synchronize the deletion
	 * with lookups.
	 */
        hash = AHASH(sid,pno);
	prev = APTNULL;
	SWHASH_MPLOCK(ipri,sid,pno);
	for (this = ahattab[hash]; this != APTNULL; this = apt_next(this))
	{
		if (this == aptx)
		{
			if (prev == APTNULL)
			{
				ahattab[hash] = apt_next(aptx);
			}
			else	/* not at head of list */
			{
				apt_next(prev) = apt_next(aptx);
			}
		}
		prev = this;
	}
	SWHASH_MPUNLOCK(ipri,sid,pno);

	/*
	 * Wake up any waitors that blocked due to an outstanding pin
	 * operation now that the vaddr and/or frame can be reassigned.
	 */
	if (type == APTPIN && !apt_valid(aptx))
	{
		ASSERT(pft_inuse(nfr) && !pft_free(nfr));
		while (pft_waitlist(nfr) != NULL)
		{
			v_ready(&pft_waitlist(nfr));
		}
	}

	/*
	 * Put the frame on the free list if it was previously released.
	 */
	if (!pft_slist(nfr))
	{
		ASSERT(apt_pinned(aptx));
		ASSERT(pft_inuse(nfr) && !pft_free(nfr));
		v_insfree(nfr);
	}

	/*
	 * Finally, add the entry to the APT free list.
	 */
	apt_valid(aptx) = 0;
	APT_MPLOCK();
	apt_free(aptx) = pf_aptfree;
	pf_aptfree = aptx;
	APT_MPUNLOCK();
	return(0);
}

/*
 * v_delaptall(nfr)
 *
 * Delete all alias page table entries for the given frame.
 *
 * Returns:
 *  0 - if no pinned apt entries were found for the frame
 *  non-zero if pinned apt entries were found
 */

v_delaptall(nfr)
int     nfr;    /* page frame number */
{
	ushort this, next;
	int rc = 0;

	this = pft_alist(nfr);
	while(this != APTNULL)
	{
		next = apt_anext(this);
		rc += v_delapt(APTREG, apt_sid(this), apt_pno(this), nfr);
		this = next;
	}
	return(rc);
}

/*
 * v_aptkey(sid,pno,nfr,key)
 *
 * Change protection key of an entry in the alias page table.
 */

v_aptkey(sid,pno,nfr,key)
int     sid;    /* segment id */
int     pno;    /* page number in segment */
int     nfr;    /* page frame number */
int     key;    /* page protection key */
{
	ushort aptx;

	/*
	 * Find the entry if it exists and change the key.
	 */
	for (aptx = pft_alist(nfr); aptx != APTNULL; aptx = apt_anext(aptx))
	{
		if (apt_sid(aptx) == sid && apt_pno(aptx) == pno)
		{
			apt_key(aptx) = key;
			break;
		}
	}
}

/*
 * v_aptkeyall(nfr,key)
 *
 * Change protection key of all alias page table entries for a frame.
 */

v_aptkeyall(nfr,key)
int     nfr;    /* page frame number */
int     key;    /* page protection key */
{
	ushort aptx;

	for (aptx = pft_alist(nfr); aptx != APTNULL; aptx = apt_anext(aptx))
	{
		apt_key(aptx) = key;
	}
}

