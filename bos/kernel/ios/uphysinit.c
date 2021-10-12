static char sccsid[] = "@(#)04	1.1  src/bos/kernel/ios/uphysinit.c, sysios, bos411, 9428A410j 7/27/93 21:54:50";

/*
 * COMPONENT_NAME: (SYSIOS) uphysio initialization
 *
 * FUNCTIONS: uphysinit
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
/*
 * Copyright (C) Bull S.A. 1993
 *
 * LEVEL 1,5 Years Confidential and Proprietary Information
 */

#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>

extern Simple_lock uphysio_lock;



/*
 * NAME:	uphysinit
 *
 * FUNCTION:	Initialize the uphysio service by allocating and
 *	initialising the uphysio_lock.
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
uphysinit()
{

#ifdef _POWER_MP
	lock_alloc(&uphysio_lock, LOCK_ALLOC_PIN,
			UPHYSIO_LOCK_CLASS, -1);	/* Allocate the lock */
	simple_lock_init(&uphysio_lock);		/* Initialize the lock*/
#endif /* _POWER_MP */

}  /* end uphysinit */
