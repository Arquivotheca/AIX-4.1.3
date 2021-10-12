static char sccsid[] = "@(#)86	1.3  src/bos/kernel/lfs/getdevno.c, syslfs, bos411, 9428A410j 8/27/93 16:20:58";

/* 
 * COMPONENT_NAME: SYSLFS - Logical File System
 * 
 * FUNCTIONS: fp_getdevno
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/vnode.h>
#include <sys/errno.h>
#include <sys/fs_locks.h>

/*
* fp_getdevno - Return the device number and channel number of the
*		device associated with fp.
*/

fp_getdevno(fp,devp,chanp)
struct file *	fp;
dev_t *		devp;
chan_t *	chanp;
{
	struct gnode *gp;
	int klock;		/* entered with kernel lock held */
	int rc = 0;			/* return code */

	if ((klock = IS_LOCKED(&kernel_lock)) != 0)
		unlockl(&kernel_lock);

	FP_LOCK(fp);
	if (fp->f_count <= 0)
	{
		rc = EINVAL;
		goto exit;
	}

	if (fp->f_type == DTYPE_GNODE)
		gp = (struct gnode *) fp->f_vnode;
	else if (fp->f_type == DTYPE_VNODE)
		gp = (struct gnode *) VTOGP(fp->f_vnode);
	else
	{
		rc = EINVAL;
		goto exit;
	}

	if (devp)
		*devp = gp->gn_rdev;

	if (chanp)
		*chanp = gp->gn_chan;

exit:
	FP_UNLOCK(fp);
	if (klock)
		lockl(&kernel_lock, LOCK_SHORT);
	return rc;
}
