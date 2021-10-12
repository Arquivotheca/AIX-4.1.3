/* @(#)03	1.6  src/bos/kernel/sys/except.h, sysproc, bos411, 9428A410j 4/2/93 13:49:48 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_EXCEPT
#define _H_EXCEPT

#include <sys/m_except.h>

/*
 * Structure for registering exception handlers
 */
struct	uexcepth {
	struct	uexcepth	*next;
	int (*handler)();
};
#define	EXCEPT_NOT_HANDLED	-1	/* The exception condition was not resolved */
#define	EXCEPT_HANDLED		0	/* The exception was handled */

/* Programmed I/O Error Recovery Retry count */
#define	PIO_RETRY_COUNT	3		/* retry count of 3 */	

/* Programmed I/O recovery routine action values */
#define PIO_RETRY	0		/* Retry operation */
#define PIO_NO_RETRY	1		/* Do no attempt retry of I/O */

#endif /*_H_EXCEPT*/
