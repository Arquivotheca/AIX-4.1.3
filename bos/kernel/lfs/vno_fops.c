static char sccsid[] = "@(#)89	1.15.1.6  src/bos/kernel/lfs/vno_fops.c, syslfs, bos411, 9428A410j 6/21/94 16:24:01";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: vno_rw, vno_ioctl, vno_select, vno_close, vno_fstat
 *
 * ORIGINS: 3, 26, 27
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
 */

#include "sys/user.h"
#include "sys/errno.h"
#include "sys/vfs.h"
#include "sys/file.h"
#include "sys/syspest.h"
#include "sys/uio.h"
#include "sys/poll.h"
#include "sys/ioctl.h"
#include "sys/vattr.h"
#include "sys/fs_locks.h"

/*
* The functions in this file implement the file ops interfaces for 
* vnodes (ie. files).  When a file is opened (via copen), the f_ops
* field is set up to point to the vnodefops array below.  The read,
* write, ioctl, select, close, and fstat system calls are changed
* to call via the f_ops pointer rather than calling directly.
* This indirection permits a cleaner break with sockets. 
*/

int	vno_rw(), vno_ioctl(), vno_select(), vno_close(), vno_fstat();

struct fileops vnodefops =
{
	vno_rw, vno_ioctl, vno_select, vno_close, vno_fstat
};

vno_rw(fp,rw,uiop,ext)
struct file *fp;
enum uio_rw rw;
struct uio *uiop;
int ext;
{
	struct vnode	*vp = fp->f_vnode;
	register int	error = 0;

	/* POSIX says if a regular file and zero bytes to read      */
	/* or write we shall return zero and have no other results. */

	if ( (vp->v_type == VREG) && (uiop->uio_resid == 0) )
		return 0;

	if ( vp->v_type == VDIR )
	{
		/* can't be writing a directory here */
		error = rdwrdir(fp, uiop, ext);
	}
	else
	{
		error = VNOP_RDWR (vp, rw, fp->f_flag, uiop, ext,
						fp->f_vinfo, NULL, fp->f_cred);
	}
	return error;
}

vno_ioctl(fp,cmd,arg,ext,kflag)
register struct file *fp;
int cmd;
caddr_t arg;
int ext;
int kflag;
{
	register struct vnode *vp = fp->f_vnode;
	struct vattr va;
	int error;
	off_t nbytes;

	switch (vp->v_type)
	{
	case VREG:
	case VFIFO:
	case VDIR:
		switch (cmd)
		{
		case FIONREAD:
			if (error = VNOP_GETATTR(vp, &va, fp->f_cred))
				break;
			nbytes = va.va_size - fp->f_offset; 
			if (copyout(&nbytes,arg,sizeof(arg)))
			{
				error = EFAULT;
				break;
			}

			/* fall through */
		case FIOASYNC:
		case FIONBIO:
			error = 0;
			break;
		default:
			error = ENOTTY;
		}
		break;
	case VCHR:
	case VBLK:
	case VMPC:
		error = VNOP_IOCTL(vp, cmd, arg, fp->f_flag|kflag, ext, fp->f_cred);
		break;
	default:
		error = ENOTTY;
	}

	return error;
}

vno_select(fp, corl, reqevents, rtneventsp, notify)
register struct file *fp;
int    corl;
ushort reqevents;
ushort *rtneventsp;
void (*notify)();
{
	register struct vnode *vp;

	vp = fp->f_vnode;

	switch( vp->v_type )
	{
		case VCHR:
		case VMPC:
		case VFIFO:
			return VNOP_SELECT(vp, corl, reqevents, rtneventsp,
                                   	   notify, fp->f_vinfo, fp->f_cred);
		default:
			/* files always select true */
			*rtneventsp = reqevents & (POLLIN|POLLOUT|POLLPRI);
			return(0);
	}
}

vno_close(fp)
register struct file *fp;
{
	register struct vnode *vp = fp->f_vnode;
	int error;

	ASSERT(vp != NULL);

	error = VNOP_CLOSE(vp, fp->f_flag, fp->f_vinfo, fp->f_cred);
	VNOP_RELE(vp);
	return error;
}

vno_fstat(fp,sbp)
register struct file *fp;
struct stat *sbp;
{
	return vstat(fp->f_vnode, sbp, fp->f_cred);
}
