static char sccsid[] = "@(#)63	1.1  src/bos/kernel/ios/POWER/sys_resource.c, sysios, bos411, 9428A410j 3/12/93 19:00:20";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: sys_res_acc()
 *
 * ORIGINS: 27
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

#include <sys/types.h>
#include <sys/syspest.h> 	/* for assert */
#include <sys/systemcfg.h> 	/* defines system config structure */
#include <sys/sys_resource.h> 	/* defines system resource structure */

volatile struct sys_resource *sys_resource_ptr= &sys_resource;


/*
 * NAME:  sys_res_acc()
 *
 * FUNCTION:  
 *	Returns a pointer to the system resource structure for PowerPC
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called on the interrupt or process levels, and
 *	is meant to give kernel extensions access to the system resources
 *
 *      It cannot page fault.
 *
 * NOTES:  
 * 	It should be noted that not all ranges within the system resource
 *	structure are mapped, i.e. only portions of it actually occupy page
 *	table entries to map the virtual to real address.
 *
 * RETURN VALUE DESCRIPTION:
 *	volatile struct sys_resource *	- pointer to system resource structure
 *
 * EXTERNAL PROCEDURES CALLED:
 */
volatile struct sys_resource *
sys_res_att()
{

	assert(__power_pc());	/* this should only be called for PPC */
	return(sys_resource_ptr);		
}
