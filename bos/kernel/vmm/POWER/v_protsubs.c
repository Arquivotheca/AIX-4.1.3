static char sccsid[] = "@(#)94	1.10.1.6  src/bos/kernel/vmm/POWER/v_protsubs.c, sysvmm, bos41J, 9513A_all 3/24/95 13:28:00";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	v_protect, v_protectp, v_protectap
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
#include <sys/inline.h>
#include <sys/syspest.h>
#include "mplock.h"

/*
 * v_protect(sid,pno,xpt,key)
 *
 * sets storage protection key of the page specified for
 * working storage segments.
 *
 * the page will be touched if it has a NULL xpt or if a
 * dummy parent may have the page defined or if it is
 * a vmapped page to force it in.  for vmapped pages, a 
 * check will be made to determime if the key is already
 * set.  if the key is set, no further processing will be
 * performed.  if the key is not set, the page will be 
 * unmapped and the key set through a call to v_unvmap().
 * 
 * the key for logically zero pages will only be set if the
 * specified key is different from the default segment key.
 *
 * in all other cases, the xpt storage protection key will
 * be set to the specified key.  if the specified page is
 * in memory, the protection key will be set in the pft
 * provided that the page is not in a pagein state and the
 * specified key is RDONLY.  in this case, the readonly flag
 * will be set in pft and the page will be made RDONLY when
 * the pagein operation completes. 
 *
 * this procedure executes at VMM level with back-tracking
 * enabled and is called via vcs_protect.
 *
 * RETURN VALUES
 *
 *      0	- ok
 *
 *      ENOSPC	- out of paging space blocks.
 *
 */

v_protect(sid,pno,key)
int     sid;    /* segment id */
int     pno;    /* page number in segment */
int     key;    /* key to be used for page */
{
        int nfr, sidx, srsave, rc, mapx, tsid;
	union xptentry	*xpt, *v_findxpt();

	/* get sidx.
	 */
	sidx = STOI(sid);

	SCB_LOCKHOLDER(sidx);

	/* get xpt. touch page if NULL xpt or parent might have page
	 * or page is vmapped. 
	 */
	if ((xpt = v_findxpt(sidx,pno)) == NULL ||
	    (xpt->word == 0 && scb_parent(sidx)) ||
	     xpt->mapblk)
	{
		srsave = chgsr(TEMPSR, SRVAL(sid,0,0));
		TOUCH((TEMPSR << L2SSIZE) + (pno << L2PSIZE));
		(void)chgsr(TEMPSR, srsave);

		/* grow xpt if NULL.
		 */
		if (xpt == NULL && (xpt = v_findxpt(sidx,pno)) == NULL)
		{
			
			if (rc = v_growxpt(sidx,pno))
				return(rc);
			xpt = v_findxpt(sidx,pno);
			ASSERT(xpt != NULL);
		}
	}

	/* get nfr.
	 */
        nfr = v_lookup(sid,pno);

	/* vmapped page. check if key is already set. if not
	 * set, call v_unvmap() to unmap page and set key.
	 */
 	if (xpt->mapblk)
	{
		ASSERT(nfr >= 0);

		/* do nothing if key already set.
		 */
		if (key == xpt->spkey && key == pft_key(nfr))
			return(0);

		/* insure vmap block is in by touching it.
		 */
                mapx = xpt->cdaddr;
                TOUCH(&pta_vmap[mapx]);

		/* v_unvmap() will set pft key from xpt.
		 */
		xpt->spkey = key;
		return(v_unvmap(sidx,pno,xpt,nfr));
	}	

	/* set key in pno's xpt.
	 */
	xpt->spkey = key;

	/* is the page in ?
	 */
        if (nfr >= 0)
	{
		/* check for pagein state and key to be set
                 * to RDONLY.  if so, set rdonly so the page will
		 * be made RDONLY by pfend.
	 	 */ 
        	if(pft_pagein(nfr) && key == RDONLY)
	 	{
                        /* set readonly in pft.
                         */
                        pft_rdonly(nfr) = 1;
		}
		else
		{
			/* check if key is already set.
		 	 */
			if (pft_key(nfr) != key)
			{
				/* set key in pft and invalidate tlb.
		 	 	*/
        			pft_key(nfr) = key;
				P_PAGE_PROTECT(nfr, key);
			}
		}
	}

        return(0);
}

/*
 * v_protectp(sid,pagex,key)
 * 
 * sets storage protection key of the page specified provided
 * the page is in memory or that the protection key is not
 * already set to the specified key. 
 *
 * if the page is in pagein state and the page is to be
 * made readonly, the readonly flag in the pft will be
 * set but the protection key will not be set. the page
 * will be made readonly in v_pfend based upon the pft  
 * readonly flag. 
 *   
 * this procedure executes at VMM level with back-tracking
 * enabled and is called via vcs_protectp.
 *
 * RETURN VALUES
 *
 *      NONE
 */

v_protectp(sid,pagex,key)
int     sid;    /* base segment id */
int     pagex;  /* page number in segment */
int     key;    /* key to be used for page */
{
        int  sidx,tsid,tpno,nfr;


	sidx = STOI(sid);

	SCB_LOCKHOLDER(sidx);

	/* is the page in memory ?
	 */
        if ((nfr = v_lookup(ITOS(sidx,pagex),BASEPAGE(pagex))) >= 0)
	{
                /* if page in and key to be set to RDONLY
                 * set readonly flag. key will be set to
                 * RDONLY by pfend.
                 */
                if(pft_pagein(nfr) && key == RDONLY)
                {
                        pft_rdonly(nfr) = 1;
                }
		else
		{
			/* check if the key is already set.
		 	 */
			if (pft_key(nfr) != key)
			{
				/* set key in pft and invalidate tlb.
		 	 	*/
        			pft_key(nfr) = key;
				P_PAGE_PROTECT(nfr, key);
			}
		}
	}

	/* return.
	 */
        return(0);
}

/*
 * v_protectap(asid,afirst,ssid,sfirst,npages,key)
 *
 * change protection key for pages aliased at asid that
 * fall in page range [sfirst,sfirst+npages-1] of source object
 *
 * non-backtrack critical section
 * VMMSR loaded
 *
 * RETURN VALUES
 *
 *      0
 */
v_protectap(asid,afirst,ssid,sfirst,npages,key)
int     asid;   /* alias sid */
int     afirst; /* alias first page number */
int     ssid;   /* source sid */
int     sfirst; /* first page number -- pagex */
int     npages; /* number of pages  */
int     key;    /* page protect key */
{
	int ssidx;
        int k,nfr,apno;
	int akey;

        /* if the number of pages is small compared to
         * number of pages in memory process the pages
         * in the interval [sfirst,sfirst+npages-1] using v_lookup.
         */
	ssidx = STOI(ssid);

	SCB_LOCKHOLDER(ssidx);

        if (npages < scb_npages(ssidx) >> 3)
        {
                for (k = sfirst; k < sfirst + npages; k++)
                {
                        if ((nfr = v_lookup(ITOS(ssidx,k),BASEPAGE(k))) > 0)
			{
				/* only change protection for pages
				 * aliased at (asid,apno).
				 */
				apno = afirst + k - sfirst;

				/* set new protection key and invalidate TLB
				 * for alias address.  we never enter an alias
				 * with no-access protection.
				 */
				if (key == UBLKKEY)
				{
					v_delapt(APTREG,asid,apno,nfr);
					P_REMOVE(asid,apno,nfr);
				}
				else
				{
					akey = RDONLYPP(nfr,key);
					v_aptkey(asid,apno,nfr,akey);
					P_PROTECT(asid,apno,nfr,akey);
				}
			}
		}
                return 0;
        }

        /* number of pages is large compared to pages in
         * memory so we scan pages on scb list and change
         * those that fall in the interval [sfirst,sfirst+npages-1].
         */

        for(k = scb_sidlist(ssidx); k >= 0; )
        {
                nfr = k;
                k = pft_sidfwd(k);
                if (pft_pagex(nfr) >= sfirst && pft_pagex(nfr) < sfirst+npages)
		{
			/* only change protection for pages
			 * aliased at asid.
			 */
			apno = afirst + pft_pagex(nfr) - sfirst;

			/* set new protection key and invalidate TLB
			 * for alias address.  we never enter an alias
			 * with no-access protection.
			 */
			if (key == UBLKKEY)
			{
				v_delapt(APTREG,asid,apno,nfr);
				P_REMOVE(asid,apno,nfr);
			}
			else
			{
				akey = RDONLYPP(nfr,key);
				v_aptkey(asid,apno,nfr,akey);
				P_PROTECT(asid,apno,nfr,akey);
			}
		}
        }

	return 0;
}

