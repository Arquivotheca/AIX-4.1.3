static char sccsid[] = "@(#)58  1.42.1.15  src/bos/kernel/lfs/rdwr.c, syslfs, bos41J, 9515A_all 4/6/95 17:38:45";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: kreadv, kwritev
 *            fp_read, fp_write, fp_readv, fp_writev,
 *            rdwr, fp_rdwr, rwuio, fp_rwuio, rdwrdir
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
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#include "sys/user.h"
#include <sys/uio.h>
#include "sys/errno.h"
#include "sys/vfs.h"
#include "sys/fs_locks.h"
#include "sys/flock.h"
#include "sys/inode.h"
#include "sys/sysinfo.h"
#include "sys/syspest.h"
#include "sys/malloc.h"
#include "sys/trchkid.h"
#include "sys/file.h"
#include "sys/fp_io.h"
#include "sys/audit.h"
#include "sys/low.h"

static
int
fp_rdwr (	struct file *	fp,
		struct iovec *	iov,
		int		iovcnt,
		int		ext,
		int		seg,
		enum uio_rw	rw,
		int *		countp);

static void
lfs_trace(	int		fd,
		struct file *	fp,
		struct iovec *	iov,
		int		iovcnt,
		long		ext,
		enum uio_rw	rw);

static int
rdwr (		int		fildes,
		struct iovec	*iov,
		int		iovcnt,
		int		ext,
		enum uio_rw	rw,
		int *		countp);

/*
 * NAME:	kreadv()
 *
 * FUNCTION:	Implements all flavors of the read system call.
 *
 * PARMETERS:	fd	file descriptor
 *		iov	pointer to array of iovec structures
 *		iovcnt	number of iovecs
 *		x	device extension, 0 for normal read
 *
 * RETURN VALUE:	success: number of bytes actually read
 *			failure: -1 with u.u_error set
 */

kreadv(fd,iovp,iovcnt,x)
int		fd;
struct iovec *	iovp;
int		iovcnt;
long		x;
{
	int lockt, rval, error;
	struct iovec iov[IOV_MAX];

	if (iovcnt > sizeof(iov)/sizeof(struct iovec)
	||  iovcnt <= 0)
	{
		error = EINVAL;
		goto out;
	}

	if (copyin((caddr_t)iovp, (caddr_t)iov,
		iovcnt * sizeof (struct iovec)))
	{
		error = EFAULT;
		goto out;
	}

	lockt = FS_KLOCK(&kernel_lock,LOCK_SHORT);

	error = rdwr(fd,iov,iovcnt,x,UIO_READ,&rval);

	if (lockt != LOCK_NEST)
		FS_KUNLOCK(&kernel_lock);
out:
	if (error)
		u.u_error = error;
	return error ? -1 : rval;
}

kwritev(fd,iovp,iovcnt,x)
int		fd;
struct iovec *	iovp;
int		iovcnt;
long		x;
{
	int lockt, rval, error;
	struct iovec iov[IOV_MAX];

	if (iovcnt > sizeof(iov)/sizeof(struct iovec)
	||  iovcnt <= 0)
	{
		error = EINVAL;
		goto out;
	}

	if (copyin((caddr_t)iovp, (caddr_t)iov,
		iovcnt * sizeof (struct iovec)))
	{
		error = EFAULT;
		goto out;
	}

	lockt = FS_KLOCK(&kernel_lock,LOCK_SHORT);

	error = rdwr(fd,iov,iovcnt,x,UIO_WRITE,&rval);
out:
	if (lockt != LOCK_NEST)
		FS_KUNLOCK(&kernel_lock);

	if (error)
		u.u_error = error;
	return error ? -1 : rval;
}

int
fp_read(
struct file	*fp,		/* file pointer of file to read from	*/
char		*buf,		/* buffer to put read data into		*/
int		nbytes,		/* number of bytes to read		*/
int		ext,		/* device driver extension data		*/
int		seg,		/* segment containing buf		*/
int		*countp)	/* return address of byte read count	*/
{
	struct iovec	iov;

	iov.iov_base	= (caddr_t) buf;
	iov.iov_len	= nbytes;

	return fp_rdwr(fp, &iov, 1, ext, seg, UIO_READ, countp);
}

int
fp_write(
struct file	*fp,		/* file pointer of file to read from	*/
char		*buf,		/* buffer to take write data from	*/
int		nbytes,		/* number of bytes to write		*/
int		ext,		/* device driver extension data		*/
int		seg,		/* segment containing buf		*/
int		*countp)	/* return address for byte write count	*/
{
	struct iovec	iov;

	iov.iov_base	= (caddr_t) buf;
	iov.iov_len	= nbytes;

	return fp_rdwr(fp, &iov, 1, ext, seg, UIO_WRITE, countp);
}

int
fp_readv(
struct file	*fp,		/* file pointer of file to read from	*/
struct iovec	*iov,		/* I/O vector of buffers to read into	*/
int		iovcnt,		/* number of buffers in I/O vector	*/
int		ext,		/* device driver extension data		*/
int		seg,		/* segment containing buf		*/
int		*countp)	/* return address of byte read count	*/
{
	return fp_rdwr(fp, iov, iovcnt, ext, seg, UIO_READ, countp);
}

int
fp_writev(
struct file	*fp,		/* file pointer of file to write to	*/
struct iovec	*iov,		/* I/O vector of buffers to write from	*/
int		iovcnt,		/* number of buffers in I/O vector	*/
int		ext,		/* device driver extension data		*/
int		seg,		/* segment containing buf		*/
int		*countp)	/* return address of byte write count	*/
{
	return fp_rdwr(fp, iov, iovcnt, ext, seg, UIO_WRITE, countp);
}

/*
 * Common setup for call to filesystem dependent read/write routine.
 */
static
int
rdwr (
register int		fildes,
register struct iovec	*iov,
register int		iovcnt,
int			ext,
enum uio_rw		rw,
int *			countp)
{
	struct	file *	fp;
	int		rc;

	if (rc = getf(fildes, &fp))
		return rc;

	if (audit_flag)
	{
		static int svcnumR = 0;
		static int svcnumW = 0;
		int tlock = (U.U_procp->p_active > 1);

		if (tlock)
			U_FD_LOCK();
		if (rw == UIO_READ)
		{
			if ((U.U_ufd[fildes].flags & UF_AUD_READ) == 0)
			{
				U.U_ufd[fildes].flags |= UF_AUD_READ;
				if (tlock)
				{
					U_FD_UNLOCK();
					tlock = 0;
				}
				if (audit_svcstart("FILE_Read",
						&svcnumR, 1, fildes))
					audit_svcfinis();
			}
		}
		else if (rw == UIO_WRITE)
		{
			if ((U.U_ufd[fildes].flags & UF_AUD_WRITE) == 0)
			{
				U.U_ufd[fildes].flags |= UF_AUD_WRITE;
				if (tlock)
				{
					U_FD_UNLOCK();
					tlock = 0;
				}
				if (audit_svcstart("FILE_Write",
						&svcnumW, 1, fildes))
					audit_svcfinis();
			}
		}
		if (tlock)
			U_FD_UNLOCK();
	}

	/* trace interesting information if enabled */
	if (TRC_ISON(0))
		lfs_trace(fildes,fp,iov,iovcnt,ext,rw);

	/* Clear byte transfer count since we are dependent on
	 * having a valid count on return.
	 */
	*countp = 0;

	rc = rwuio(fp, iov, iovcnt, ext, UIO_USERSPACE, rw, countp, 0);

	/* Error processing */
	if (rc)
	{
		/* Fix up u_error for System V FNDELAY compatibility */
		if (rc == EAGAIN && fp->f_flag & FNDELAY
				&& fp->f_type == DTYPE_VNODE 
				&& (fp->f_vnode->v_type == VFIFO 
				|| fp->f_vnode->v_type == VCHR
				|| fp->f_vnode->v_type == VMPC)) {
			rc = 0;
			*countp = 0;
		}

		/* Allow streams devices to return EAGAIN even with system V
		 * FNDELAY.  The streams driver will return EAGAIN with the 
		 * high bit set.  We turn it off here.
		 */
		if (rc == (EAGAIN | (1 << 31)))
			rc = EAGAIN;
		
		/* For writes to a regular file where some data has been
		 * written, traditional semantics suggest we should return
		 * success when an error has occurred.  This is because we
		 * have changed persistent state.  So when the return code
		 * shows an error and the count is not zero, we clear the
		 * return code.
		 */
		if (*countp
		    && (rw == UIO_WRITE)
		    && (fp->f_type == DTYPE_VNODE)
		    && (fp->f_vnode->v_type == VREG))
		{
			rc = 0;
#ifdef notdef
			/* There is debate about whether or not there are 
			 * cases where the error should be returned regardless
			 * of data movement.  Someday we may use code that 
			 * looks like this.
			 */
			switch(rc)
			{
			case ENOSPC:
				/* POSIX indicates that as much data as possible
				 * should be written when we run out of space.
				 */
			case EDQUOT:
				/* It is reasonable that reaching quota should
				 * be treated like no space available.
				 */
				rc = 0;
			}
#endif
		}
	}

	U.U_ioch += *countp;

	ufdrele(fildes);
	return rc;
}


static
int
fp_rdwr (
struct file	*fp,		/* file pointer of file to read from	*/
struct iovec	*iov,		/* I/O vector of buffers for transfer	*/
int		iovcnt,		/* number of buffers in I/O vector	*/
int		ext,		/* device driver extension data		*/
int		seg,		/* segment containing I/O buffers	*/
enum uio_rw	rw,		/* direction:  UIO_READ or UIO_WRITE	*/
int		*countp)	/* return address of byte I/O count	*/
{
	extern		kernel_lock;	/* global kernel lock		*/
	register int	rc;		/* return value			*/
	int klock;	                /* save kernel_lock state       */

	if ((klock = IS_LOCKED(&kernel_lock)) != 0)
		unlockl(&kernel_lock);

	rc = rwuio(fp, iov, iovcnt, ext, seg, rw, countp, FKERNEL);

	/* Allow streams devices to return EAGAIN even with system V
	 * FNDELAY.  The streams driver will return EAGAIN with the 
	 * high bit set.  We turn it off here.
	 */
	if (rc == (EAGAIN | (1 << 31)))
		rc = EAGAIN;

	if (klock)
		lockl(&kernel_lock, LOCK_SHORT);

	return rc;
}

static
int
rwuio (fp, iov, iovcnt, ext, seg, rw, countp, fflag)
register struct file	*fp;		/* file pointer of file for I/O	*/
register struct iovec	*iov;		/* vector of I/O buffers	*/
register int		iovcnt;		/* number of I/O buffers	*/
int			ext;		/* device driver ext value	*/
int			seg;		/* segment with I/O buffers	*/
register enum uio_rw	rw;		/* dir:  UIO_READ or UIO_WRITE	*/
register int		*countp;	/* return of I/O byte count	*/
int			fflag;		/* extra file flags		*/
{
	struct uio	auio;	/* uio structure describing I/O		*/
	int		i;	/* loop counter over buffers		*/
	int		rc;	/* return code				*/
	int		count;	/* count of bytes read or written	*/
#ifdef OFFSET_LOCK
	int		offset_locked = 0; /* set if f_offset is locked */
#endif
	cpu_t		cpuid;

	/* it is assumed that ONLY the kernel (e.g. loader) can open a
	 * file for FEXEC - user opens never have this state.
	 * When a file is open for FEXEC, test below allows it to be read.
	 * Since FEXEC file table entrys are only available to kernel services
	 * this is safe.  N.B. when we replicate a file table entry for
	 * the debugger, this gets complicated.  See fp_ufalloc
	 */
	if ((fp->f_flag & (rw==UIO_READ ? (FREAD|FEXEC) : FWRITE)) == 0)
		return EBADF;

	auio.uio_iov	= iov;
	auio.uio_iovcnt	= iovcnt;
	auio.uio_segflg	= seg;
	auio.uio_fmode	= fp->f_flag | fflag;
	auio.uio_offset = fp->f_offset;
	auio.uio_resid	= 0;

	for (i = 0; i < auio.uio_iovcnt; i++) {
		if (iov->iov_len < 0)
			return EINVAL;
		auio.uio_resid += iov->iov_len;
		if (auio.uio_resid < 0)
			return EINVAL;
		iov++;
	}

	count = auio.uio_resid;
        
#ifdef OFFSET_LOCK
	/*
	 * Holding the offset lock across the VNOP can cause deadlock
	 * if the underlying filesystem supports enforced mode
	 * record locking.  Another thread can block this thread
	 * with a record lock and be blocked by this threads offset
	 * lock.  Version 3 did not attempt to protect the offset
	 * between processes which shared common file table entries, 
	 * so we will continue that behaviour.  Several suggestions
	 * such as performing common_reclock() in the LFS were
	 * suggested, but these medicines were deemed too bitter
	 * to solve this problem.  This code is not to be compiled
	 * into the LFS, and is here for historical purposes.
	 */
        if ( ((U.U_procp->p_threadcount != 1) || (fp->f_count != 1))
	    && (fp->f_type == DTYPE_VNODE)
	    && ((fp->f_vnode->v_type == VREG) || (fp->f_vnode->v_type == VDIR)))
        {
                offset_locked = 1;
                FP_OFFSET_LOCK(fp);
        }
#endif

	rc = (*fp->f_ops->fo_rw)(fp, rw, &auio, ext);
	
	fp->f_offset = auio.uio_offset;
#ifdef OFFSET_LOCK
	if (offset_locked)
		FP_OFFSET_UNLOCK(fp);
#endif

	/* check for interrupted system call */

	if (rc == EINTR)			/* interrupted?		*/
		if (count == auio.uio_resid)	/* no data transferred?	*/
			rc = ERESTART;		/* restart (perhaps)	*/
		else
			rc = 0;			/* partial transfer	*/

	count -= auio.uio_resid;

	cpuid = CPUID;
	if (rw == UIO_READ) {
		sysinfo.sysread++;
		sysinfo.readch += count;
		cpuinfo[cpuid].sysread++;
		cpuinfo[cpuid].readch += count;
	}
	else {
		sysinfo.syswrite++;
		sysinfo.writech += count;
		cpuinfo[cpuid].syswrite++;
		cpuinfo[cpuid].writech += count;
	}

	if (countp)
		*countp = count;

	return rc;
}

int
fp_rwuio(
register struct file	*fp,		/* file pointer of file for I/O	*/
register enum uio_rw	rw,		/* dir:  UIO_READ or UIO_WRITE	*/
struct uio		*uiop,		/* uio structure describing I/O	*/
int			ext)		/* device driver ext value	*/
{
	extern		kernel_lock;	/* global kernel lock		*/
	int		rc;		/* return code			*/
	int		count;		/* count of bytes transferred	*/
	int		move_rwptr = 1;	/* do we update the rw pointer? */
#ifdef OFFSET_LOCK
	int             offset_locked = 0; /* set if f_offset is locked */
#endif
	int klock;	                /* save kernel_lock state       */
	cpu_t		cpuid;

	if ((klock = IS_LOCKED(&kernel_lock)) != 0)
		unlockl(&kernel_lock);

	if (rw == UIO_READ_NO_MOVE) {
		move_rwptr = 0;
		rw = UIO_READ;
	} else if (rw == UIO_WRITE_NO_MOVE) {
		move_rwptr = 0;
		rw = UIO_WRITE;
	}

#ifdef OFFSET_LOCK
        if (move_rwptr
            && ((U.U_procp->p_threadcount != 1) || (fp->f_count != 1))
	    && (fp->f_type == DTYPE_VNODE)
	    && ((fp->f_vnode->v_type == VREG) || (fp->f_vnode->v_type == VDIR)))
        {
                offset_locked = 1;
                FP_OFFSET_LOCK(fp);
        }
#endif

	count = uiop->uio_resid;

	rc = (*fp->f_ops->fo_rw)(fp, rw, uiop, ext);

	/* Allow streams devices to return EAGAIN even with system V
	 * FNDELAY.  The streams driver will return EAGAIN with the 
	 * high bit set.  We turn it off here.
	 */
	if (rc == (EAGAIN | (1 << 31)))
		rc = EAGAIN;
		
	count -= uiop->uio_resid;

	if (move_rwptr)
	{
		fp->f_offset = uiop->uio_offset;
#ifdef OFFSET_LOCK
		if (offset_locked)
			FP_OFFSET_UNLOCK(fp);
#endif
	}

	cpuid = CPUID;
	if (rw == UIO_READ) {
		sysinfo.sysread++;
		sysinfo.readch += count;
		cpuinfo[cpuid].sysread++;
		cpuinfo[cpuid].readch += count;
	}
	else {
		sysinfo.syswrite++;
		sysinfo.writech += count;
		cpuinfo[cpuid].syswrite++;
		cpuinfo[cpuid].writech += count;
	}

	if (klock)
		lockl(&kernel_lock, LOCK_SHORT);

	return rc;
}

int
rdwrdir(fp, uiop, ext)
register struct file	*fp;
struct uio		*uiop;
int	ext;
{
	struct	vnode	*vp;
	caddr_t		dirbuf = NULL;
	int		bsize;
	int		rc;
	int 		resid;
	int		cur_count;
	int 		nbytes;
	int		loc;
	int		seendirent;
	struct dirent	*dp;
	struct uio	ruio;
	struct iovec	riov;
	struct sys5_direct {
		ushort	d_ino;
		char	d_name[14];
	} svd;

	if (uiop->uio_resid <= 0)
		return 0;

	if (ext != 1 && ext != 0)
		return EINVAL;

	vp = fp->f_vnode;

	if (ext == 1)		/* obsolete ? */
	{
		/*
		 * it is not clear that arbitrary buffer offsets and sizes
		 * need be supported;
		 * however, it is probably so (for PS/2 compatibility).
		 * in this case, an offset translation similar to the above
		 * will be required.
		 */
		rc = VNOP_READDIR(fp->f_vnode, uiop, fp->f_cred);
		return rc;
	}

	/* underlying format is System V, or */

	if ((vp->v_vfsp->vfs_gfs->gfs_flags & GFS_SYS5DIR)  ||
		 ((U.U_compatibility & PROC_RAWDIR)))
	{
		/*
		 * this is a copy of the "regular file case".
		 */
		rc = VNOP_RDWR (vp, UIO_READ, fp->f_flag,
				uiop, ext, fp->f_vinfo, NULL, fp->f_cred);
		return rc;
	}

	/*
	 * Directory format is Berkeley and caller expects system 5.
	 * In case the caller lseek()'ed before he got here, we need to
	 * correctly position ourself for the read.  Do this by recursively
	 * calling rdwrdir() and do an actual read of the directory.
	 * Note: On the recursive call the code inside the following if is
	 *       not executed.  Because of this, it is impossible to recurse
	 *       more than 1 level.
	 */

	if (fp->f_offset && !fp->f_dir_off) {
		/*
		 * Round the buffer sizes to a multiple of a sys5 direntry,
		 * in this case 16 bytes.  Also initialize local ruio struct,
		 * and set fp->f_offset to 0 so that we don't come back 
		 * through here.
		 */
		bsize = vp->v_vfsp->vfs_bsize & (~(sizeof(svd) - 1));
		nbytes = (fp->f_offset &= ~(sizeof(svd) - 1));
		resid = uiop->uio_iov->iov_len;

		ruio.uio_iov = &riov;
		ruio.uio_segflg = UIO_USERSPACE;
		ruio.uio_offset = 0;

		fp->f_offset = 0;
		/*
		 * use this while loop to position ourself for the read
		 */
		while (nbytes > 0) {
			riov.iov_base = uiop->uio_iov->iov_base;
			riov.iov_len = ruio.uio_resid = MIN(nbytes, resid);
			ruio.uio_iovcnt = 1;
			if (rc = rdwrdir(fp, &ruio, 0))
				goto fail;
			if ((resid - ruio.uio_resid) == 0)
				break;
			nbytes -= resid - ruio.uio_resid;
		}

		uiop->uio_offset = fp->f_offset;
	}

	/*
	 * do I really need to reset bsize?
	 */
	bsize = vp->v_vfsp->vfs_bsize;

	/* allocate buffer for reading BSD directory entries */
	if ((dirbuf = (char *)malloc(bsize)) == NULL)
		return EAGAIN;

	ruio.uio_iov = &riov;
	ruio.uio_iovcnt = 1;
	ruio.uio_segflg = UIO_SYSSPACE;

	ruio.uio_offset = fp->f_dir_off;	/* real offset in directory */

	seendirent = 0;
	loc = cur_count = 0;
	/* copy entries to the user buffer */
	while (uiop->uio_resid >= sizeof(svd))
	{
		/* at end of buffer ? */
		if (loc >= cur_count)
		{
			/*
			 * offset for this read() was set up
			 * by the previous read(), but
			 * we must remember the current offset.
			 */
			riov.iov_base = dirbuf;
			riov.iov_len = bsize;
			ruio.uio_resid = bsize;
			if (rc = VNOP_READDIR(vp, &ruio, fp->f_cred))
				goto fail;
			/* record the amount read */
			if ((cur_count = bsize - ruio.uio_resid) == 0)
				goto done;
			/* we are at the start of the buffer, again */
			loc = 0;
		}
		seendirent = 1;
		/* point to the current BSD directory entry */
		dp = (struct dirent *)(dirbuf + loc);

		/* copy into the System V structure */
		svd.d_ino = dp->d_ino;
		copyname(svd.d_name, dp->d_name, 14);
		
		/* move this to user space */
		if (rc = uiomove((caddr_t) &svd, sizeof(svd), UIO_READ, uiop))
			goto fail;

		/* advance to the next BSD entry */
		loc += dp->d_reclen;

	}
done:
	/* 
	 * only reset directory offset if we have seen some
	 * directory entries.
	 */
	if (seendirent)
		fp->f_dir_off = dp->d_offset;
fail:
	if (dirbuf)
		free(dirbuf);
	return rc;
}


static void
lfs_trace(
int		fd,
struct file *	fp,
struct iovec *	iov,
int		iovcnt,
long		ext,
enum uio_rw	rw)
{
	if (rw == UIO_READ)
	{
		if (ext)
			TRCHKL5T(HKWD_SYSC_LFS | hkwd_SYSC_READX,
			fd, fp->f_vnode, iov->iov_base,
			iov->iov_len, ext);
		else
			TRCHKL5T(HKWD_SYSC_READ, fd, fp->f_vnode,
			iov->iov_base, iov->iov_len, 0);
	}
	else
	{
		if (ext)
			TRCHKL5T(HKWD_SYSC_LFS | hkwd_SYSC_WRITEX,
			fd, fp->f_vnode, iov->iov_base,
			iov->iov_len, ext);
		else
			TRCHKL5T(HKWD_SYSC_WRITE, fd, fp->f_vnode,
			iov->iov_base, iov->iov_len, 0);
	}
}
