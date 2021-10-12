static char sccsid[] = "@(#)64	1.15.1.3  src/bos/kernel/lfs/stat.c, syslfs, bos411, 9428A410j 4/19/94 16:15:38";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: statx, fstatx, fp_fstat, vstat
 *
 * ORIGINS: 3, 27
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

#include "sys/file.h"
#include "sys/dir.h"
#include "sys/vattr.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/stat.h"
#include "sys/user.h"
#include "sys/systm.h"
#include "sys/fs_locks.h"
#include "sys/fp_io.h"
#include "sys/errno.h"
#include "sys/syspest.h"
#include "sys/trchkid.h"

/* Definitions */
BUGVDEF(sdebug, 0);
BUGVDEF(rgdebug, 0);

/*
 * Convert inode formats to vnode types
  */
enum vtype iftovt_tab[] = {
	VNON, VFIFO, VCHR, VUNDEF, VDIR, VUNDEF, VBLK, VUNDEF, VREG, VUNDEF,
	VLNK, VUNDEF, VSOCK, VUNDEF, VUNDEF, VBAD
};

int vttoif_tab[] = {
	0, S_IFREG, S_IFDIR, S_IFBLK, S_IFCHR, S_IFLNK, S_IFSOCK, S_IFMT,
	S_IFIFO
};


/*
 * NAME:	statx()		(system call entry point)
 *
 * FUNCTION:	Stat a pathname, according to the provided cmd.
 * 
 * PARAMETERS:	pathname	Path to stat.
 *		statp		Pointer to stat struct.
 *		len		Number of bytes of stat struct to return.
 *		cmd		Stat options.
 *
 * RETURN VALUES: explicitly none, implicitly sets errno;
 */
statx(pathname, statp, len, cmd)
char		*pathname;
struct stat	*statp;
int		 len;
int		 cmd;
{
	struct vnode	*vp;
	struct stat	sbuf;
	register int	error = 0;
	register int	flags;		/* lookup flags */
	register int	lockt;		/* previous state of kernel lock */
	extern		kernel_lock;	/* global kernel lock            */
	struct ucred    *crp;

	if (len < 0  ||  len > STATXSIZE) {
		error = EINVAL;
		goto out;
	}
	if (len == 0)
		len = STATXSIZE;

	if (cmd & ~(STX_LINK|STX_MOUNT|STX_HIDDEN)) {
		error = EINVAL;    /* No weird bits allowed in cmd */
		goto out;
	}

	/* Set up lookup flags based upon the supplied cmd.  */
	flags = 0;
	if (cmd & STX_LINK)
		flags |= L_NOFOLLOW;
	if (cmd & STX_MOUNT)
		flags |= L_NOXMOUNT;
	if (cmd & STX_HIDDEN)
		flags |= L_NOXHIDDEN;

	/* Grab global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);   

	/* Get the user's credentials */
	crp = crref();

	error = lookupname(pathname, USR, L_SEARCH | flags, NULL, &vp, crp);

	if (!error) {

		error = vstat(vp, &sbuf, crp);

		VNOP_RELE(vp);
		if (!error && copyout(&sbuf, statp, len))
			error = EFAULT;
	}

	crfree(crp);
	if (lockt != LOCK_NEST)
		FS_KUNLOCK(&kernel_lock);
out:
	if (error)
		u.u_error = error;
	return error ? -1 : 0;
}

/*
 * NAME:	fstatx()	(system call entry point)
 *
 * FUNCTION:	Stat a file descriptor.
 * 
 * PARAMETERS:	fdes		File descriptor.
 *		statp		Pointer to stat struct.
 *		len		Number of bytes of stat struct to return.
 *		cmd		Stat options (only STX_NORMAL).
 *
 * RETURN VALUES: explicitly none, implicitly sets errno;
 */
fstatx(fdes, statp, len, cmd)
int		 fdes;
struct stat	*statp;
int		 len;
int		 cmd;
{
	struct file	*fp;		/* file pointer from fdes */
	struct stat	sbuf;
	register int	error = 0;
	register int	lockt;		/* previous state of kernel lock */
	extern		kernel_lock;	/* global kernel lock            */

	/* trace file descriptor */
	TRCHKL1T(HKWD_SYSC_FSTAT, fdes);

	if (len < 0  ||  len > STATXSIZE) {
		error = EINVAL;
		goto out;
	}
	if (len == 0)
		len = STATXSIZE;

	/*
	 * Since fstatx() was designed with a cmd argument, its value is 
	 * checked, and since there is only one possible cmd for fstatx 
	 * (STX_NORMAL), any other value is an error.  It may seem odd 
	 * that the system call will fail if a zero (STX_NORMAL) is not
	 * passed in as the fourth argument, but then it's even odder 
	 * that the interface was designed to require a cmd in the first place.
	 */
	if (cmd != STX_NORMAL) {
		error = EINVAL;
		goto out;
	}

	/* Grab global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);   

	if (!(error = getf(fdes, &fp))) {

		if (!(error = (*fp->f_ops->fo_fstat)(fp, &sbuf))) {

			if (fp->f_type == DTYPE_SOCKET && sbuf.st_mode == 0)
			{
				sbuf.st_mode = S_IFSOCK | 0777;
				sbuf.st_type = (ulong_t) VSOCK;
			}

			if (copyout(&sbuf, statp, len))
				error = EFAULT;
		}
		ufdrele(fdes);
	}

	if (lockt != LOCK_NEST)
		FS_KUNLOCK(&kernel_lock);
out:
	if (error)
		u.u_error = error;
	return error ? -1 : 0;
}


/*
 *	Fp fstat service.
 */
int
fp_fstat(fp, statp, len, seg)
register struct file	*fp;
register struct stat	*statp;
register int		 len;
register int		 seg;
{
	struct stat	sbuf;
	register int	error = 0;
	extern		kernel_lock;	/* global kernel lock            */
	int klock;	                /* save kernel_lock state        */

	if (len < 0  ||  len > STATXSIZE)
		return EINVAL;

	if ((klock = IS_LOCKED(&kernel_lock)) != 0)
		unlockl(&kernel_lock);

	if (len == 0)
		len = STATXSIZE;

	seg = FP_SYS;		/* TEMPORARY! (transition from old usage) */

	if (!(error = (*fp->f_ops->fo_fstat)(fp, &sbuf)))
		if (seg == FP_SYS)
			bcopy(&sbuf, statp, len);
		else
			if (copyout(&sbuf, statp, len))
				error = EFAULT;

	if (klock)
		lockl(&kernel_lock, LOCK_SHORT);

	return error;
}

/*
 * Vstat is called by statx() and vno_fstat() (via fstatx) to get the vnode
 * attributes and fill in the stat structure.
 */

vstat(vp, statbuf, crp)
register struct vnode *vp;
register struct stat *statbuf;
struct ucred *crp;
{
	int error;
	struct vattr vat;
	register struct vattr *vattrp = &vat;

	if (error = VNOP_GETATTR(vp, vattrp, crp))
		return error;
	
	if (vp->v_vfsp->vfs_flag & VFS_REMOTE) {
		/*
		 * remote files have peculiar st_dev info.
		 * the st_dev of a remote file can be confused with local
		 * devices, and we don't want that.  What we want is,
		 * given the st_dev of a file, get access to the VFS ops
		 * for the VFS that the file resides on, so that we can
		 * get information about that VFS (e.g. for ustat()).
		 * thus we encode the unique VFS number in the device, along
		 * with a bit marking the dev as a remote device.
		 */
		statbuf->st_dev = SDEV_REMOTE | vp->v_vfsp->vfs_number;
	} else
		statbuf->st_dev	= vattrp->va_dev;
	statbuf->st_ino		= vattrp->va_serialno;
	statbuf->st_mode	= vattrp->va_mode;
	statbuf->st_nlink	= vattrp->va_nlink;
	statbuf->st_uid		= vattrp->va_uid;
	statbuf->st_gid		= vattrp->va_gid;
	statbuf->st_rdev	= vattrp->va_rdev;
	statbuf->st_size	= (off_t) vattrp->va_size;
	statbuf->st_atime	= vattrp->va_atim;
	statbuf->st_mtime	= vattrp->va_mtim;
	statbuf->st_ctime	= vattrp->va_ctim;
	statbuf->st_blksize	= vattrp->va_blocksize;
	statbuf->st_blocks	= vattrp->va_blocks;
	statbuf->st_spare1	= 0;
	statbuf->st_spare2	= 0;
	statbuf->st_spare3	= 0;

	statbuf->st_type	= vattrp->va_type;
	statbuf->st_gen		= vattrp->va_gen;
	statbuf->st_vfstype	= vp->v_vfsp->vfs_type;
	statbuf->st_vfs		= vp->v_vfsp->vfs_number;
	statbuf->st_flag	= (vp->v_flag & V_ROOT) ? FS_VMP : 0;
	if (vp->v_vfsp->vfs_flag & VFS_REMOTE)
		statbuf->st_flag |= FS_REMOTE;

	statbuf->st_access	= 0; /* TEMPORARY until vattr contains access */
	statbuf->st_spare4[0]	= statbuf->st_spare4[1] = 0;

	return 0;
}
