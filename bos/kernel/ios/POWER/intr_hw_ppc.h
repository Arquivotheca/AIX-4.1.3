/* @(#)64	1.3  src/bos/kernel/ios/POWER/intr_hw_ppc.h, sysios, bos411, 9428A410j 4/15/94 17:15:27 */
#ifndef _h_INTR_HW_PPC
#define _h_INTR_HW_PPC
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: Internal machine dependent macros and labels used by
 *	      the interrupt management services for PowerPC machines.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_IOCC
#include <sys/iocc.h>
#endif

/* 
 * Structure describing PowerPC memory mapped interrupt registers
 */
volatile struct ppcint {
	ulong xirr_poll;		/* xirr with no side effects	*/
	union {
		ulong xirr;		/* xirr with side effects	*/
		struct {
			uchar cppr;	/* just the CPPR part		*/
			uchar xisr[3];	/* the rest of the word		*/
		} xirr_u;
	} ux;
	ulong dsier;			/* direct store error register	*/
	uchar mfrr;			/* MFRR register		*/
	uchar pad[3];			/* Fill out the word		*/
};

#define i_xirr	ux.xirr
#define i_cppr	ux.xirr_u.cppr

/*
 * Get a BUID number out of the given bid and shift it to make an index
 */
#define GET_BUID_NUM( Bid )	((BUID_SEGREG_MASK & Bid) >> 20)

/*
 * Get a BUID number out of the given bid
 */
#define GET_BUID_MSK( Bid )	(BUID_SEGREG_MASK & Bid)

/*
 *      Creates a virtual memory handle.
 *      Preserve the given buid
 */
#define BID_TO_IOCC(bid)    ((0x1FF00000 & bid) | 0x80000000 )

struct intrerr_ppc {			/* error log description */
	struct  err_rec0        ierr;
		ulong           bid;
		ulong           level;
};

#endif /* _h_INTR_HW_PPC */
