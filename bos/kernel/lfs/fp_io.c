static char sccsid[] = "@(#)43	1.35  src/bos/kernel/lfs/fp_io.c, syslfs, bos411, 9428A410j 2/28/94 08:14:32";
/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS:	fp_opencount, fp_shmat, fp_shmdt
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/user.h>
#include	<sys/fs_locks.h>
#include	<sys/dir.h>
#include	<sys/errno.h>
#include	<sys/fp_io.h>
#include	<sys/fullstat.h>
#include	<sys/malloc.h>
#include	<sys/pathname.h>
#include	<sys/pri.h>
#include	<sys/systm.h>
#include	<unistd.h>
#include	<sys/user.h>
#include	<sys/vnode.h>
#include	<sys/syspest.h>
#include	<sys/shm.h>
#include	<sys/adspace.h>
#include	<sys/seg.h>

int
fp_opencount(fp)
struct	file	*fp;
{
	ASSERT(fp != NULL);
	ASSERT(fp->f_count > 0);
	return fp->f_count;
}

int
fp_shmat(fp, addr, request, hp)
register struct file	*fp;
caddr_t			addr;
int			request;
vmhandle_t		*hp;		/* pointer to the returned	*/
					/* handle created in the VNOP	*/
					/* by the virtual memory manager */
{
	register struct vnode	*vp;
	int klock;	                /* save kernel_lock state */
	int rc;

	if ((klock = IS_LOCKED(&kernel_lock)) != 0)
		unlockl(&kernel_lock);

	vp = fp->f_vnode;

	if (vp->v_vntype != VREG)
		rc = EBADF;
	else {
		rc = VNOP_MAP(vp, addr, SEGSIZE, 0, request, fp->f_cred);
		*hp = SRVAL(VTOGP(vp)->gn_seg, 0, 0);
	}

	if (klock)
		lockl(&kernel_lock, LOCK_SHORT);

	return rc;
}


int
fp_shmdt(fp, flags)
register struct file *fp;
int flags;
{
	register struct vnode *	vp;
	int klock;	                /* save kernel_lock state */
	int rc;

	if ((klock = IS_LOCKED(&kernel_lock)) != 0)
		unlockl(&kernel_lock);

	vp = fp->f_vnode;

	if (vp->v_vntype != VREG)
		rc = EBADF;
	else
		rc = VNOP_UNMAP(vp,flags,fp->f_cred);

	if (klock)
		lockl(&kernel_lock, LOCK_SHORT);

	return rc;
}
