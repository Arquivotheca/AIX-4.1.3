/* @(#)67	1.6  src/bos/kernel/sys/pin.h, sysvmm, bos411, 9428A410j 6/16/90 00:33:35 */
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_PIN
#define _H_PIN

/*
**	PIN kernel service
*/

int
pincode(int (*func)());

#define PIN_SUCC	 0 		/*  return value if successful        */
#define MEM_INSUFF	ENOMEM		/*  insufficient memory avail.        */
#define MEM_INVAL	EINVAL		/*  specified range of memory invalid
					 *  or length parameter negative      */

/*
**	UNPIN kernel service -- additional defines
*/

int
unpincode(int (*func)());

#define UNPIN_SUCC	 0		/*  return value if successful        */
#define UNPIN_UNDERFL	EINVAL		/*  one or pages were not pinned      */

#endif	/* _H_PIN */
