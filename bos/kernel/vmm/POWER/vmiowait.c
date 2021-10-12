static char sccsid[] = "@(#)63	1.5  src/bos/kernel/vmm/POWER/vmiowait.c, sysvmm, bos411, 9428A410j 2/7/94 07:49:39";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: 	vms_iowait
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include  "vmsys.h"
#include <sys/types.h>
#include <sys/except.h>
#include <sys/errno.h>
#include <sys/lockl.h>
#include <sys/systm.h>

/*
 * vms_iowait(sid)
 *
 * caller is placed in wait-state until all page-outs
 * in the segment specified are complete.
 *
 * input params:
 *
 * sid - segment id 
 *
 * Return values  0     ok.
 *
 *                EIO - one or more pages not written because
 *			of i/o error.
 */

vms_iowait(sid)
int     sid;
{
	int rc;
	int hadlock ;

	/* free kernel lock if we have it 
	 */
	if (hadlock = IS_LOCKED(&kernel_lock))
		unlockl(&kernel_lock);

	/* wait for i/o to finish.
	 */
	rc = vcs_iowait_excp(sid);

	/* re-acquire kernel lock 
	 */
	if (hadlock) 
		lockl(&kernel_lock,LOCK_SHORT);

	return ((rc == VM_WAIT) ? 0 : rc);
}
