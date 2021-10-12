static char sccsid[] = "@(#)04	1.5.1.3  src/bos/kernel/lfs/getdirent.c, syslfs, bos411, 9428A410j 3/4/94 16:39:06";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: getdirent
 *
 * ORIGINS: 26, 27
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
 *
 */

/* vfs_syscalls.c	1.4 87/03/27 NFSSRC */
/* vfs_syscalls.c 1.1 86/09/25 SMI	*/

#include "sys/param.h"
#include "sys/user.h"
#include "sys/fs_locks.h"
#include "sys/file.h"
#include "sys/uio.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/errno.h"
#include "sys/syspest.h"

BUGVDEF(rddirdbg, 0);

/*
 *	getdirent - get directory entries in a file system
 *		independent format.  Acts in the same manner
 *		as the bsd getdirentries sys call.
 */
getdirent(fd, buf, count)
int	fd;
char	*buf;
unsigned count;
{
	struct file	*fp;
	struct vnode	*vp;
	struct uio	auio;
	struct iovec	aiov;
	int		rval;	/* number of bytes transferred		*/
	int		rc;	/* error return value			*/
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
#ifdef OFFSET_LOCK
	int offset_locked = 0;  /* f_offset field lock indicator        */
#endif

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	BUGLPR(rddirdbg, BUGACT, ("getdirent: fd %d, buf %x, cnt %d\n",
		fd, buf, count));

	if ((rc = getft(fd, &fp, DTYPE_VNODE)) == 0)
	{
		vp = fp->f_vnode;

		if ((fp->f_flag & FREAD) == 0 || vp->v_vntype != VDIR)
			rc = EBADF;
		else
		{

			BUGLPR(rddirdbg, BUGACT,
				("getdirent: fp 0x%x, vp 0x%x\n", fp, vp));

			aiov.iov_base = buf;
			aiov.iov_len = count;
			auio.uio_iov = &aiov;
			auio.uio_iovcnt = 1;
			auio.uio_fmode = fp->f_flag & FMASK;
			auio.uio_segflg = UIO_USERSPACE;
			auio.uio_resid = count;

#ifdef OFFSET_LOCK
			if ((U.U_procp->p_threadcount != 1)
			     || (fp->f_count != 1))
			{
				offset_locked = 1;
				FP_OFFSET_LOCK(fp);
			}
#endif
			auio.uio_offset = fp->f_offset;

			BUGLPR(rddirdbg, BUGACT,
				("getdirent: offset 0x%x, f_dir_off 0x%x\n",
				fp->f_offset, fp->f_dir_off));

			if ((rc = VNOP_READDIR(vp, &auio, fp->f_cred)) == 0)
			{
				fp->f_offset = auio.uio_offset;
				rval = count - auio.uio_resid;
			}

#ifdef OFFSET_LOCK
			if (offset_locked)
				FP_OFFSET_UNLOCK(fp);
#endif

			BUGLPR(rddirdbg, BUGACT,
				("getdirent: nread %d, base %d\n",
				rval, fp->f_offset));
		}
		ufdrele(fd);
	}

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : rval;
}
