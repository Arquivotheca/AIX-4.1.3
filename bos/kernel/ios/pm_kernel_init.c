static char sccsid[] = "@(#)18  1.2  src/bos/kernel/ios/pm_kernel_init.c, sysios, bos41J, 9517A_all 4/24/95 08:31:56";
/*
 *   COMPONENT_NAME: SYSIOS
 *
 *   FUNCTIONS: Power Management Kernel Initialization Code
 *              pm_kernel_init
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef PM_SUPPORT
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/pm.h>

extern struct _pm_kernel_data	pm_kernel_data;

/*
 * NAME:  pm_kernel_init
 *
 * FUNCTION:  Initialize data of PM kernel part.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called at system intialization through init_tbl[]
 *      defined sys/init.h.
 *
 * NOTES:
 *
 * RETURN:
 *      none.
 *
 * EXTERNAL PROCEDURES CALLED:
 *      lock_alloc/simple_lock_init routines
 */
void
pm_kernel_init()
{
	lock_alloc(&(pm_kernel_data.lock), LOCK_ALLOC_PIN,
					PM_KERNEL_LOCK_CLASS, -1);
	simple_lock_init(&(pm_kernel_data.lock));
	lock_alloc(&(pm_kernel_data.planar_lock), LOCK_ALLOC_PIN,
					PM_KERNEL_LOCK_CLASS, -1);
	simple_lock_init(&(pm_kernel_data.planar_lock));
}
#endif /* PM_SUPPORT */
