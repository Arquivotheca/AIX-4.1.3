static char sccsid[] = "@(#)73	1.1.1.5  src/bos/kernel/vmm/POWER/xmem_pwr.c, sysvmm, bos41J, 145887.bos 3/13/95 18:43:49";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	xmemdma_pwr
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
#include <sys/low.h>
#include <sys/intr.h>
#include <sys/xmem.h>
#include <sys/adspace.h>
#include <sys/inline.h>
#include <sys/syspest.h>

#ifdef _POWER_RS

#define	ACC_PERM_MASK_PWR  0xE5EAE555

/*
 * NAME: xmemdma_pwr (dp,xaddr,flags)
 *
 * FUNCTION: This is the Power version of xmemdma(), and replaces
 *	     the xmemdma() export at system initialization time.
 *
 *	     prepare a page for dma i/o or process a page
 *           after dma operation is complete. if hide is true
 *           the cache for the page containing the specified
 *           address is flushed and the page made invalid (i.e.
 *           not addressable with any virtual address). if hide
 *           is false the page is made valid and any processes
 *           waited on the page are readied.
 *
 *	     this service also performs access checking on a page if
 *           the XMEM_ACC_CHK flag is set.  Also, it will allow Read-Only
 *	     page access only if the XMEM_WRITE_ONLY flag is set.
 *
 *           the modified bit for the page is set unless the page has
 *           a read-only storage key or the page has a pager-io address.
 *
 * EXECUTION ENVIRONMENT:
 *
 * This may be called from a process or an interrupt handler.
 * but the page specified must be in memory and either pinned
 * or in pager i/o state.
 *
 * NOTES:
 *
 * This is an internal kernel routine.
 *
 * RETURN VALUE DESCRIPTION:
 *
 *              real address of page.
 *		XMEM_FAIL = access violation
 */

int db_xmem_cnt = 0;

int
xmemdma_pwr(
struct xmem     *dp,                    /* cross memory descriptor */
caddr_t         xaddr,                  /* transfer address */
int             flags)                  /* XMEM_HIDE to hide the page and
					 * flush cache; XMEM_UNHIDE to unhide
					 * the page and set the mod bit;
					 * XMEM_ACC_CHK to check page access; 
					 * XMEM_WRITE_ONLY if all that is 
					 * needed is read-only page access */
{
        uint    sid,oldpri,srvmsave,srval;
        int     nfr, pno;
	uint	xsreg, dsreg;
	uint	shift_count, access;
	uint	ppkey, attr;

        /* disallow if descriptor not in use
         */
        assert( dp->aspace_id != XMEM_INVAL );

        /* make vmmdseg addressable. disable to INTMAX
         */
        srvmsave = chgsr(VMMSR,vmker.vmmsrval);
        oldpri = i_disable(INTMAX);

        /* determine sid and page number in segment.
         */
        if (dp->aspace_id == XMEM_GLOBAL)
        {
                srval = mfsri(xaddr);
        }
	else if (dp->aspace_id != XMEM_PROC2)
        {
                srval = dp->subspace_id;
        }
	else
	{
		xsreg = (uint)xaddr >> L2SSIZE;
		dsreg = (uint)dp->uaddr >> L2SSIZE;
		if (xsreg == dsreg)
			srval = dp->subspace_id;
		else
		{
			assert(xsreg == dsreg + 1)
			srval = dp->subspace_id2;
		}
	}

	sid  =  SRTOSID(srval);
        pno  =  ((uint)xaddr & SOFFSET) >> L2PSIZE;

        /* page must be memory and either be pinned or in pager
         * i/o state. in general it can not be found with lra
         * because it can be hidden so we find it via hash chain.
         */
	nfr = v_lookupx(sid,pno,&ppkey,&attr);
        assert(nfr >= 0);
        assert(pft_pagein(nfr)||pft_pageout(nfr)||pft_pincount(nfr));

        /* if hide flush cache and make page invalid.
         */
        if (!(flags & XMEM_UNHIDE))
        {
		if (flags & XMEM_ACC_CHK) {
			/*
			 * Check page protection
			 */

			/*
			 * The mask and state table below represent
			 * the access permission that corresponds to 
			 * each of the 16 possible states of the Segment 
			 * Register Key, Page Protect bits, and the 
			 * XMEM_WRITE_ONLY flag.
               ----------------------------------------------------------------
              |RO |NAC|RW |RW |RO |NAC|NAC|NAC|RO |NAC|RW |RW |RW |RW |RW |RW |
              |1 1|1 0|0 1|0 1|1 1|1 0|1 0|1 0|1 1|1 0|0 1|0 1|0 1|0 1|0 1|0 1|
              |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
              | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|2|2|2|2|2|2|3|3|
              |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
               ----------------------------------------------------------------
			 *
			 * Below is the State Table:
			 *
			 * SR Key | Page Protect | XMEM_WRITE_ONLY|| Access
			 * -------|--------------|----------------||-------
			 *	0 |     0 0      |        0       ||XMEM_RDWR
			 *	0 |     0 0      |        1       ||XMEM_RDWR
			 *	0 |     0 1      |        0       ||XMEM_RDWR
			 *	0 |     0 1      |        1       ||XMEM_RDWR
			 *	0 |     1 0      |        0       ||XMEM_RDWR
			 *	0 |     1 0      |        1       ||XMEM_RDWR
			 *	0 |     1 1      |        0       ||XMEM_NOACC
			 *	0 |     1 1      |        1       ||XMEM_RDONLY
			 *	1 |     0 0      |        0       ||XMEM_NOACC
			 *	1 |     0 0      |        1       ||XMEM_NOACC
			 *	1 |     0 1      |        0       ||XMEM_NOACC
			 *	1 |     0 1      |        1       ||XMEM_RDONLY
			 *	1 |     1 0      |        0       ||XMEM_RDWR
			 *	1 |     1 0      |        1       ||XMEM_RDWR
			 *	1 |     1 1      |        0       ||XMEM_NOACC
			 *	1 |     1 1      |        1       ||XMEM_RDONLY
			 *
			 * Build a shift count from the SRKEY, page protect,
			 * and XMEM_WRITE_FLAG according to the above table.
			 * (and multiply shift count by 2 ... 2 bits per state)
			 */

			shift_count = (SRTOKEY(srval) << 4) |
				      (ppkey << 2) |
				      ((flags & XMEM_WRITE_ONLY) ? 2 : 0);

			/*
			 * The 2 bit access is determined by shifting the
			 * mask right by shift_count
			 */
			access = (ACC_PERM_MASK_PWR >> shift_count) & 0x3; 
			
			if (access == XMEM_NOACC) {
				/*
				 * No Access, clean up and return failure
				 */
	        		i_enable(oldpri);
			        (void)chgsr(VMMSR,srvmsave);
				return(XMEM_FAIL);
			}
		}  /* end if (flags & XMEM_ACC_CHK) */

		/* Do not remove addressability or increment cross memory 
		 * count on log pages.  The only valid faults on log pages
		 * are handled by v_reload().
		 */
		if (!scb_logseg(STOI(pft_ssid(nfr))))
		{
			/* only flush cache and invalidate page on 
		 	 * first hide
		 	 */
			if (pft_xmemcnt(nfr) == 0)
                		P_REMOVE_ALL(nfr);
			pft_xmemcnt(nfr) += 1;
		}
        }
        else
        {
                /* unhide if last unhide request. make page valid.
                 * if no pager i/o ready any processes which may be
                 * may waiting and set modified bit unless RDONLY key.
                 */
		if (pft_xmemcnt(nfr) == 1)
		{
                	P_ENTER(NORMAL,sid,pno,nfr,ppkey,attr);
                	if (pft_pagein(nfr) == 0 && pft_pageout(nfr) == 0)
                	{
                        	while(pft_waitlist(nfr) != NULL)
                        	{
                                	v_ready(&pft_waitlist(nfr));
                        	}
                		if (ppkey != RDONLY)
                        		SETMOD(nfr);
                	}
		}

		/* the following check is for debugging.
		 */
		if (db_xmem_cnt && !scb_logseg(STOI(pft_ssid(nfr))))
			assert(pft_xmemcnt(nfr) != 0);

		/* decrement xmemcnt. check is for debugging.
		 */
		if (pft_xmemcnt(nfr) != 0)
		{
			pft_xmemcnt(nfr) += -1;
		}
        }

        /* restore priority and VMMSR sreg.
         */
        i_enable(oldpri);
        (void)chgsr(VMMSR,srvmsave);
        return( (nfr << L2PSIZE) + ((uint)xaddr & (PSIZE - 1)) );
}
#endif /* _POWER_RS */
