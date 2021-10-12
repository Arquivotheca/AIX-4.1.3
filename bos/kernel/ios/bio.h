/* @(#)06  1.11  src/bos/kernel/ios/bio.h, sysios, bos411, 9428A410j 9/15/93 11:58:27 */

#ifndef _h_BIO
#define _h_BIO

/*
 * COMPONENT_NAME: (SYSIOS) Block I/O header file
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Level 1, 5 years, Bull confidental information
 */                                                                   
/*
 * @BULL_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: bio.h,v $
 * $EndLog$
 */

#define	NBUF	1000			/* #buffers in buffer pool	*/

#define	NHBUF	128			/* #entries in hash table	*/

struct hbuf {
	int     b_flags;
	struct  buf *b_forw;
	struct  buf *b_back;
};
extern struct hbuf *hbuf;

#endif  /* _h_BIO */
