static char sccsid[] = "@(#)74	1.1.1.8  src/bos/kernel/vmm/POWER/xmem_ppc.c, sysvmm, bos411, 9428A410j 5/31/94 10:24:41";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	xmemdma_ppc
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
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

#ifdef _POWER_PC

#define	ACC_PERM_MASK_PPC  0xE5EAE555

/*
 * NAME: xmemdma_ppc (dp,xaddr,flags)
 *
 * FUNCTION: This is the PowerPC version of xmemdma(), and replaces
 *	     the xmemdma() export at system initialization time for
 *	     PowerPC platforms.
 *
 *           if XMEM_UNHIDE is not set, get real address for the page and
 *	     set the MOD bit if the page is to be modified.
 *	     if XMEM_ACC_CHK is set, perform page access checking.
 *	     if XMEM_WRITE_ONLY is set allow an access of Read-Only. 
 *
 * EXECUTION ENVIRONMENT:
 *
 * This may be called from a process or an interrupt handler.
 * but the page specified must be in memory and either pinned
 * or in pager i/o state.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:
 *
 *              real address of page.
 *		XMEM_FAIL - Access violation
 */


int
xmemdma_ppc(
struct xmem     *dp,                    /* cross memory descriptor */
caddr_t         xaddr,                  /* transfer address */
int             flags)                  /* XMEM_HIDE to conditionally set
					 * mod bit; XMEM_ACC_CHK to check
					 * page access; XMEM_WRITE_ONLY if
					 * all that is needed is read-only
					 * page access                    */
{
        uint    sid,oldpri,srvmsave,srval;
        int     nfr, pno;
	uint	xsreg, dsreg;
	uint	shift_count, access;
	uint	ppkey, attr;

	if (!(flags & XMEM_UNHIDE)) {
        	/* disallow if descriptor not in use
	         */
	        assert( dp->aspace_id != XMEM_INVAL );

	        /* make vmmdseg addressable. disable to INTMAX
	         */
	        srvmsave = chgsr(VMMSR,vmker.vmmsrval);
	        oldpri = i_disable(INTMAX);

	        /* determine sid and page number in segment.
	         */
	        if (dp->aspace_id == XMEM_GLOBAL) {
	                srval = mfsri(xaddr);
	        } else if (dp->aspace_id != XMEM_PROC2) {
	                srval = dp->subspace_id;
	        } else {
			xsreg = (uint)xaddr >> L2SSIZE;
			dsreg = (uint)dp->uaddr >> L2SSIZE;
			if (xsreg == dsreg)
				srval = dp->subspace_id;
			else {
				assert(xsreg == dsreg + 1)
				srval = dp->subspace_id2;
			}
		}

		sid  =  SRTOSID(srval);
	        pno  =  ((uint)xaddr & SOFFSET) >> L2PSIZE;

        	/* page must be in memory and either be pinned or in pager
	         * i/o state. in general it can not be found with lra
	         * because it can be hidden so we find it via hash chain.
	         */
		nfr = v_lookupx(sid,pno,&ppkey,&attr);
	        assert(nfr >= 0);
	        assert(pft_pagein(nfr)||pft_pageout(nfr)||pft_pincount(nfr));

		access = XMEM_RDWR;	/* set read-write as default */

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
			access = (ACC_PERM_MASK_PPC >> shift_count) & 0x3; 
			
			if (access == XMEM_NOACC) {
				/*
				 * No Access, clean up and return failure
				 */
	        		i_enable(oldpri);
			        (void)chgsr(VMMSR,srvmsave);
				return(XMEM_FAIL);
			}
		}

                if (access != XMEM_RDONLY) {
                	if (pft_pagein(nfr) == 0 && pft_pageout(nfr) == 0) {
				/*
				 * If this access is anything other than
				 * Read-Only, then set the MOD bit
				 */
				SETMOD(nfr);
			}
		}

        	/* restore priority and VMMSR sreg.
	         */
	        i_enable(oldpri);
	        (void)chgsr(VMMSR,srvmsave);
	        return( (nfr << L2PSIZE) + ((uint)xaddr & (PSIZE - 1)) );
        }

	/*
	 * Return 0 if this was XMEM_UNHIDE
	 */
	return 0;
}
#endif /* _POWER_PC */
