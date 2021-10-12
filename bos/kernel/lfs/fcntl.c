static char sccsid[] = "@(#)41	1.31.1.9  src/bos/kernel/lfs/fcntl.c, syslfs, bos41J, 9514A_all 3/29/95 11:10:38";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: kfcntl
 *
 * ORIGINS: 3, 26, 27
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

#include <sys/limits.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/user.h>
#include <sys/syspest.h>
#include <sys/errno.h>
#include <sys/fs_locks.h>
#include <fcntl.h>
#include <sys/flock.h>
#include <sys/sleep.h>
#define	_BSD
#include <sys/ioctl.h>
#include <sys/trchkid.h>
#include <sys/audit.h>

/*
 * NAME: kfcntl	File control system call
 *
 * FUNCTION:	Used to perform controlling operations on open file
 *		descriptors.
 *
 * PARAMETERS:	fdes, cmd, arg.  Fdes is an open file descrptor, cmd
 *		describes the operation to be performed and arg is
 *		additional data (if any) required by the command.
 *
 * RETURN VALUE:	A value is returned based on the command given.
 *			-1 is returned with u_error set if an error occurred.
 */
kfcntl(fdes, cmd, arg)
int	fdes;
int	cmd;
int	arg;
{
	struct file *fp;
	register int locking;
	struct eflock lock_buf;
	struct flock lock_save;	/* save orig flock struct for F_GETLK */
	int i;
	int rval = 0;
	int value;
	int flags;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int tlock;		/* is multi-thread locking required?    */
	int rc = 0;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	if (cmd != F_CLOSEM)
		if (rc = getf(fdes, &fp))
			goto out;

	switch (cmd) {

	case F_DUPFD:
		if (arg < 0 || arg >= OPEN_MAX) {
			rc = EINVAL;
			break;
		}
		if ((rc = ufdalloc(arg, &rval)) != 0) {
			break;
		}
		fp_hold(fp);
		U.U_ufd[rval].fp = fp;
		break;

	case F_GETFD:
		rval = U.U_ufd[fdes].flags & FD_CLOEXEC;
		break;

	case F_SETFD:
	{
		unsigned short *flagp;
		if (tlock = (U.U_procp->p_active > 1))
			U_FD_LOCK();
		flagp = &U.U_ufd[fdes].flags;
		*flagp = (*flagp & ~FD_CLOEXEC) | (arg & FD_CLOEXEC);
		if (tlock)
			U_FD_UNLOCK();
		break;
	}

	case F_GETFL:
		rval = fp->f_flag+FOPEN;
		break;

	case F_SETFL:
		FP_LOCK(fp);	
		fp->f_flag &= ~FFCNTL;
		fp->f_flag |= (arg-FOPEN) & FFCNTL;
		FP_UNLOCK(fp);	
		break;

	case F_GETLK:
		if (fp->f_type != DTYPE_VNODE) {
			rc = EINVAL;	/* can't lock a socket */
			break;
		}

		if (fp->f_vnode->v_vntype == VFIFO) {
			rc = EINVAL;	/* can't lock a pipe */
			break;
		}

		/* copy in the user's file lock request */
		if (copyin(arg, &lock_save, sizeof(struct flock))) {
			rc = EFAULT;
			break;
		}

		/* construct the extended flock structure */
		lock_buf.l_type = lock_save.l_type;
		lock_buf.l_whence = lock_save.l_whence;
		lock_buf.l_start = lock_save.l_start;
		lock_buf.l_len = lock_save.l_len;
		lock_buf.l_sysid = 0;
		lock_buf.l_pid = U.U_procp->p_pid;
		lock_buf.l_vfs = fp->f_vnode->v_vfsp->vfs_type;

		rc = VNOP_LOCKCTL(fp->f_vnode,
					fp->f_offset,
					&lock_buf,
					0, 0, 0,
					fp->f_cred);

		if (rc == EAGAIN) {
			rc = EACCES;
			break;
		}

		/*
		 * if l_type == F_UNLCK, no lock was found, so restore
		 * everything but l_type...
		 */
		if (lock_buf.l_type == F_UNLCK) {
			lock_save.l_type = F_UNLCK;
		}
		else
		{
			lock_save.l_type = lock_buf.l_type;
			lock_save.l_whence = lock_buf.l_whence;
			lock_save.l_start = (off_t) lock_buf.l_start;
			lock_save.l_len = (off_t) lock_buf.l_len;
			lock_save.l_sysid = lock_buf.l_sysid;
			lock_save.l_pid = lock_buf.l_pid;
			lock_save.l_vfs = lock_buf.l_vfs;
		}

		if (copyout(&lock_save, arg, sizeof(struct flock)))
			rc = EFAULT;

		break;

	case F_SETLK:
	case F_SETLKW:
		if (fp->f_type != DTYPE_VNODE) {
			rc = EINVAL;	/* can't lock a socket */
			break;
		}

		if (fp->f_vnode->v_vntype == VFIFO) {
			rc = EINVAL;	/* can't lock a pipe */
			break;
		}

		/* set record lock */
		if (copyin(arg, &lock_save, sizeof(struct flock))) {
			rc = EFAULT;
			break;
		}

		/* check access permissions */
		if ((lock_save.l_type == F_RDLCK && !(fp->f_flag & FREAD))
		||  (lock_save.l_type == F_WRLCK && !(fp->f_flag & FWRITE))) {
			rc = EBADF;
			break;
		}

		/*
		* Since set record lock and set record lock and wait
		* are sooo similar, we handle both of them here.
		*/

		locking = SETFLCK;

		if (cmd == F_SETLKW)
			locking |= SLPFLCK;

		/* construct the extended flock structure */
		lock_buf.l_type = lock_save.l_type;
		lock_buf.l_whence = lock_save.l_whence;
		lock_buf.l_start = lock_save.l_start;
		lock_buf.l_len = lock_save.l_len;
		lock_buf.l_sysid = 0;
		lock_buf.l_pid = U.U_procp->p_pid;
		lock_buf.l_vfs = fp->f_vnode->v_vfsp->vfs_type;

		rc = VNOP_LOCKCTL(fp->f_vnode,
					fp->f_offset,
					&lock_buf,
					locking,
					0, 0,
					fp->f_cred);

		if (rc == EAGAIN) {
			rc = EACCES;
			break;
		}

		if (rc == EINTR) {
			rc = ERESTART;	/* restart (perhaps) */
			break;
		}

		if (tlock = (U.U_procp->p_active > 1))
			U_FD_LOCK();
		U.U_lockflag = 1;
		U.U_ufd[fdes].flags |= UF_FDLOCK;
		if (tlock)
			U_FD_UNLOCK();

		break;

	case F_CLOSEM:
		if (fdes < 0 || fdes >= OPEN_MAX)
		{
			rc = EINVAL;
			break;
		}
			
		if (tlock = (U.U_procp->p_active > 1))
			U_FD_LOCK();

		for (i = fdes; i < U.U_maxofile; i++)
		{
			if ((U.U_ufd[i].flags & UF_CLOSING)
				|| ((fp = U.U_ufd[i].fp) == NULL))
				continue;
			else
			{
				U.U_ufd[i].flags |= UF_CLOSING;
				while (U.U_ufd[i].count != 0)
				{
					assert(curproc->p_threadcount > 1);
					(void) e_sleep_thread(&U.U_fdevent,
						&U.U_fd_lock, LOCK_SIMPLE);
				}

				if (tlock)
					U_FD_UNLOCK();

				closefd(i, 1);
			
				if (tlock)	
					U_FD_LOCK();
			}
		}
		if (tlock)
			U_FD_UNLOCK();
		/* no errors are returned */
		break;

	case F_GETOWN:		/* converted to FIOGETOWN in libc */
	case F_SETOWN:		/* converted to FIOSETOWN in libc */
	default:
		rc = EINVAL;
	}

	if (TRC_ISON(0))
	{
		off_t	start, length;

		switch (cmd)
		{
		case F_GETLK:
		case F_SETLK:
		case F_SETLKW:
			switch (lock_buf.l_whence)
			{
			case 0:
			default:
				start = lock_buf.l_start;
				break;
			case 1:
				start = lock_buf.l_start + fp->f_offset;
				break;
			case 2:
				/*
				* Must call VNOP_GETATTR to correctly
				* determine the starting file offset.
				*/
				start = INT_MAX;
				break;
			}
			length = lock_buf.l_len;

			TRCHKL4T(HKWD_SYSC_LFS|hkwd_SYSC_LOCKFX,
				fdes, (cmd << 16) | lock_buf.l_type,
				start, length);

			break;

		default:
			TRCHKL4T(HKWD_SYSC_FCNTL, fdes, cmd, arg, rval);
		}
	}

	if (cmd != F_CLOSEM)
		ufdrele(fdes);

out:
	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : rval;
}
