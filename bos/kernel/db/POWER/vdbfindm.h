/* @(#)74	1.5  src/bos/kernel/db/POWER/vdbfindm.h, sysdb, bos411, 9428A410j 6/16/90 03:04:50 */
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _h_FINDM
#define _h_FINDM

/*
 * memory search routine definitions.
 */
#define NOTFOUND -1			/* returned if not found	*/
#define NOPAGELIMIT -1			/* No limit on paged out mem	*/

/* parameters' array numbers. */
#define ARGP 1
#define STARTP 2
#define ENDP 3
#define ALIGNP 4

#ifndef FINDM
extern caddr_t findit();		/* the routine			*/
#endif /* FINDM */
#endif /* h_FINDM */
