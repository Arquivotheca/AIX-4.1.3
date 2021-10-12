static char sccsid[] = "@(#)71	1.1  src/bos/kernel/ios/selpollinit.c, sysios, bos411, 9428A410j 9/17/93 09:53:52";

/*
 * COMPONENT_NAME: (SYSIOS) select and poll initialization
 *
 * FUNCTIONS: selpollinit
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

extern Simple_lock select_lock;



/*
 * NAME:	selpollinit
 *
 * FUNCTION:	Initialize the select and poll services by allocating and
 *	initialising the select_lock.
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
selpollinit()
{

#ifdef _POWER_MP
	lock_alloc(&select_lock, LOCK_ALLOC_PIN,
			SELPOLL_LOCK_CLASS, -1);	/* Allocate the lock */
	simple_lock_init(&select_lock);			/* Initialize the lock*/
#endif /* _POWER_MP */

}  /* end selpollinit */
