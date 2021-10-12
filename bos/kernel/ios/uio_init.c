static char sccsid[] = "@(#)43	1.1  src/bos/kernel/ios/uio_init.c, sysios, bos411, 9428A410j 6/17/94 16:21:34";
/*
 * COMPONENT_NAME: (SYSIOS) pinu/unpinu initialization
 *
 * FUNCTIONS: uio_init
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
/*
 * Copyright (C) Bull S.A. 1994
 *
 * LEVEL 1,5 Years Confidential and Proprietary Information
 */

#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>

extern Simple_lock pinu_lock;



/*
 * NAME:	uioinit
 *
 * FUNCTION:	Initialize the pinu/unpinu service by allocating and
 *	initialising the pinu_lock.
 *
 * EXECUTION ENVIRONMENT:  This routine is called at system initialization
 *	   by an entry in a pointer-to-function array defined in <sys/init.h>
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:  none
 *
 * EXTERNAL PROCEDURES CALLED:  lock_alloc(), simple_lock_init()
 */
void
uio_init()
{

#ifdef _POWER_MP
	lock_alloc(&pinu_lock, LOCK_ALLOC_PIN,
			UPHYSIO_LOCK_CLASS, -1);	/* Allocate the lock */
	simple_lock_init(&pinu_lock);			/* Initialize the lock*/
#endif /* _POWER_MP */

}  /* end uio_init */
