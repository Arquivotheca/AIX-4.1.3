static char sccsid[] = "@(#)16	1.28  src/bos/kernel/pfs/xix_lckctl.c, syspfs, bos411, 9428A410j 7/7/94 16:53:46";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_lockctl
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "jfs/jfslock.h"
#include "sys/errno.h"
#include "sys/user.h"

/*
 * NAME:	jfs_lockctl(vp, offset, lckdat, cmd, retry_fcn, retry_id, crp)
 *
 * FUNCTION:	the jfs_lockctl is just a function that calls the 
 * 		common_reclock routine. The jfs_lockctl routine was 
 * 		made into an entry into this layer and all of the "real"
 *		code was moved to common_reclock so routines that used to 
 *		call jfs_lockctl from within this layer can now call 
 *		common_reclock and not violate the layering system.
 *
 * PARMETERS:	Vp is a struct vnode pointer for the file to be locked,
 *		offset is a ulong indicating where the lock should start,
 *		lckdat is a pointer to a flock structure with the locking
 *		details, cmd is an integer with the command for locking,
 *		retry_fcn, if non-NULL, is a pointer to a function that gets
 *		called for a sleep locks list entry when the corresponding
 *		blocking lock is released (for NFS and DS server use),
 *		retry_id is a pointer to a "ulong" used to return
 *		the identifier that will be passed as an arguement
 *		to the retry_fcn.  This return value can be used by the
 * 		caller to correlate this VNOP_LOCKCTL() call with
 *		a later (* retry_fcn)() call.
 *		
 *
 * RETURN VALUE:	Zero is returned if fsync completes sucessfully.
 *			An error (errno) is returned if the routine failed.
 */
jfs_lockctl(vp, offset, lckdat, cmd, retry_fcn, retry_id, crp)
struct vnode	*vp;
offset_t	offset;
struct eflock	*lckdat;
int		cmd;
int		(*retry_fcn)();
ulong		*retry_id;
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc;
	struct inode *ip;
	struct gnode *gp;
	extern int iwritelockx();
	extern int iwriteunlockx();

	ip=VTOIP(vp);
	gp=ITOGP(ip);
	IWRITE_LOCK(ip);

	if (ENF_LOCK(ip->i_mode) &&
	    ((gp->gn_mrdcnt && lckdat->l_type == F_WRLCK) || gp->gn_mwrcnt))
		rc = EMFILE;
	else
		rc = common_reclock(gp,ip->i_size,offset,lckdat,(cmd|INOFLCK),
			      retry_fcn,retry_id,iwritelockx,iwriteunlockx);

	IWRITE_UNLOCK(ip);
	return(rc);
}
