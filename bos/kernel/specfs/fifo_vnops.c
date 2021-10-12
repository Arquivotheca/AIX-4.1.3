static char sccsid[] = "@(#)98	1.35.1.17  src/bos/kernel/specfs/fifo_vnops.c, sysspecfs, bos41B, 412_41B_sync 12/8/94 12:45:24";
/*
 * COMPONENT_NAME: (SYSSPECFS) Special File System
 *
 * FUNCTIONS: fifo_close, fifo_open,
 *            fifo_read, fifo_rdwr, fifo_select,
 *            fifo_write, fifo_bufalloc, fifo_buffree
 *            fifo_badop, fifo_einval, fifo_noop
 *            fifo_pool_init
 *
 * ORIGINS: 3, 26, 24, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1986 by Sun Microsystems, Inc.
 */

#include "sys/param.h"
#include "sys/systm.h"
#include "sys/time.h"
#include "sys/proc.h"
#include "sys/dir.h"
#include "sys/user.h"
#include "sys/uio.h"
#include "sys/vnode.h"
#include "sys/vattr.h"
#include "sys/vfs.h"
#include "sys/file.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/malloc.h"
#include "sys/poll.h"
#include "sys/pri.h"
#include "sys/lockl.h"
#include "sys/syspest.h"
#include "sys/utsname.h"
#include "sys/sleep.h"
#include "sys/specnode.h"
#include "sys/vmuser.h"
#include "sys/fs_locks.h"

#include "sys/vmker.h"		 	/* vmker gives access to vmker
					 * structure which has mem size for
					 * fifo_init modification of
					 * pipe_maxfree.
					 */

extern int spec_elist[];	/* ok from special's support */
Simple_lock   fifobuf_lock;	/* Pipe buffer allocation lock */

#define SELDONE	0x80

#define PIPE_BSZ	PAGESIZE		/* # data bytes in buffer */
#define L2PIPE_BSZ	PGSHIFT
#define PIPE_BSZ_MASK	(PAGESIZE - 1)

#define PIPE_SIZE	(PIPE_BSZ * NFBUF)	/* maximum fifo capacity */
#define L2PIPE_SIZE	(L2PIPE_BSZ + L2NFBUF)
#define PIPE_SIZE_MASK	(PIPE_SIZE - 1)

#define PIPE_MAX	(~0)			/* largest write */

#define PIPE_ATOMIC	PIPE_SIZE		/* writes of this or fewer
						 * bytes will be atomic
						 */

int pipe_mnb;				/* max bytes used by fifos */

int pipe_maxfree;			/* Max free bytes kept default
					 * value. Will be revised up in
					 * fifo_init based on memory size.
					 */

#define PIPE_MAX_GROW	256		/* Maximum pages the dedicated pool
					 * will always hold once its grown
					 * to that point.  -  1 MB
					 */

#define PIPE_MNB_GROW	4096		/* Maximum pages the dedicated pool
					 * can ever grow to.  -  16 MB
					 */

#define PIPE_MAX_DIVIDER  128		/* When divided into total pages of
					 * mem produces min number of pages
					 * to hold in pipe mem pool.
					 */

#define PIPE_MNB_DIVIDER    2		/* When divided into total pages of
					 * mem produces max number of pages
					 * to hold in pipe mem pool.
					 */

/*
 * The pipes are implemented by keeping read and write pointers
 * (sn_[rw]ptr) and the number of bytes currently in the
 * pipe (sn_size). The read and write pointers give the address
 * of the next byte to read and (where to) write, respectively.
 *
 * The pipe pointers can be split into two values:
 *    a block number, which is used as an index into the sn_buf array; and
 *    an offset within the block
 *
 * Only the least significant L2PIPE_BSZ+L2NFBUF bits of
 * the pipe read and write pointers are used, the higher
 * bits are ignored thruout the code by using the following
 * macros:
 */

#define	BUFBLOCK(p)	(NFBUFMASK & ((p) >> L2PIPE_BSZ))
#define	BUFOFFSET(p)	((p) & PIPE_BSZ_MASK)
#define	BUFPTR(p)	((p) & (PIPE_BSZ_MASK | (NFBUFMASK << L2PIPE_BSZ)))
#define PIPE_FULL(vp)	((vp)->sn_size >= PIPE_SIZE)

#if (PIPE_ATOMIC != PIPE_BUF)		/* #ASSERT ? */
#error "PIPE_ATOMIC != PIPE_BUF"
#endif

union freepage {
	char buf[PIPE_BSZ];		/* as a buffer		*/
	union freepage *free;		/* as a freelist	*/
};

struct {
	int	count;		/* track total # fifo bytes		  */
	int	nfree;		/* # bytes on freelist			  */
	union freepage *free;	/* free page list			  */
	int	allocblk;	/* somebody waiting on count to decrease  */
	int	aevent;		/* event list for waiting on alloc blocks */
} fifo_alloc = { 0, 0, 0, 0, EVENT_NULL };

#ifdef	DEBUG

int	fifo_print;
#define	FIFO_DEBUG(a,b,c)	fifo_debug(a,b,c)
#define	DBGASSERT(a,b)	{					\
				if (!(a)) {			\
					fifo_print++;		\
					fifo_debug("stop",b,0);	\
					fifo_print--;		\
					printf("a");		\
					brkpoint(b);		\
				}				\
			}

#else

#define	FIFO_DEBUG(a,b,c)
#define	DBGASSERT(a,b)	ASSERT(a)

#endif

/* Declarations
 */
static int fifo_bufalloc();
static void fifo_buffree();
extern struct vnodeops fifo_vnops;

/*
 * NAME:	fifo_open (vp, flag, ext, vinfop)
 *
 * FUNCTION:	Open fifo.  On open for read or write if FNDELAY
 *		return immediately.  If not FNDELAY wait for counter-
 *		part to connect.
 *
 * PARAMETERS:	vp	- spec vnode to open
 *		flag	- open flags
 *		ext	- extended open info
 *		vinfop	- vfs specific info
 *
 * RETURN :	ENXIO	- write with "delay" and no readers
 *		EINTR	- interupted open
 *
 */

fifo_open (vp, flag, ext, vinfop, crp)
struct vnode *vp;		/* vnode to open */
int flag;			/* open mode */
int ext;			/* extended open info */
caddr_t *vinfop;		/* vfs specific info, not used */
struct ucred *crp;		/* credentials */
{
	struct specnode *sp;
	int rc;			/* return code */

	sp = VTOSP (vp);

	if (flag & FEXEC)
	{
		rc = EACCES;
		goto open_out;
	}

	SPECNODE_LOCK(sp);

	if (rc = specaccess(vp, flag, crp))
		goto open_unlock;

	FIFO_DEBUG("begin-open", sp, 0);


	if (flag & FREAD)
	{	if (sp->sn_rcnt++ == 0)
		{
			/* if any writers waiting, wake them up */
			e_wakeupx(&sp->sn_revent, E_WKX_NO_PREEMPT);
		}
	}

	if (flag & FWRITE)
	{	if ((flag & (FNDELAY|FNONBLOCK)) && (sp->sn_rcnt == 0))
		{	rc = ENXIO;
			goto open_unlock;
		}

		if (sp->sn_wcnt++ == 0) {
			/* indicate an open for write occured */
			sp->sn_flag |= FIFO_WWRT;

			/* if any readers waiting, wake them up */
			e_wakeupx(&sp->sn_wevent, E_WKX_NO_PREEMPT);
		}
	}

	if (flag & FREAD)
	{	while (sp->sn_wcnt == 0)
		{	/* if no delay, or data in fifo, open is complete */
			if ((flag & (FNDELAY|FNONBLOCK)) || sp->sn_size)
				goto open_unlock;

			rc = e_sleep_thread(&sp->sn_wevent, &sp->sn_lock,
					       LOCK_SIMPLE|INTERRUPTIBLE);
			if (rc == THREAD_INTERRUPTED)
				break;
			else
				rc = 0;
		}
	}

	if (flag & FWRITE)
	{	while (sp->sn_rcnt == 0)
		{
			rc = e_sleep_thread(&sp->sn_revent, &sp->sn_lock,
						LOCK_SIMPLE|INTERRUPTIBLE);
			if (rc == THREAD_INTERRUPTED)
				break;
			else
				rc = 0;
		}
	}

open_unlock:
	SPECNODE_UNLOCK(sp);

	if (rc == THREAD_INTERRUPTED)
	{
		(void) fifo_close (vp, flag & FMASK, vinfop);
		rc = EINTR;
	}

open_out:
	FIFO_DEBUG("end-open", sp, 0);
	return rc;
}

/*
 * NAME:	fifo_close (vp, flag, vinfo)
 *
 * FUNCTION:	Close fifo. Wakeup any blocked process, including selects.
 *		Release all allocated buffers.
 *
 * PARAMETERS:	vp	- spec vnode to close
 *		flag	- open flags
 *		vinfop	- vfs specific info
 *
 * RETURN :	0
 *
 */


/* ARGSUSED */
fifo_close (vp, flag, vinfo, crp)
struct vnode *vp;		/* Vnode to close */
int flag;			/* open flags */
caddr_t vinfo;			/* vfs specific information */
struct ucred *crp;		/* credentials */
{
	struct specnode *sp;

	sp = VTOSP (vp);
	SPECNODE_LOCK(sp);

	FIFO_DEBUG("begin-close", sp, 0);
	if (flag & FREAD)
	{	if (--sp->sn_rcnt == 0)
		{	if (sp->sn_flag & FIFO_WBLK)
			{	sp->sn_flag &= ~FIFO_WBLK;
				e_wakeupx(&sp->sn_wevent, E_WKX_NO_PREEMPT);
			}
			/* wake up any sleeping write select()s */
			if (sp->sn_poll & POLLOUT)
			{
				sp->sn_flag |= SELDONE;
				selnotify (POLL_FIFO,sp,POLLOUT);
				sp->sn_poll &= ~POLLOUT;
			}
		}
	}

	if (flag & FWRITE)
	{	if (--sp->sn_wcnt == 0)
		{	if (sp->sn_flag & FIFO_RBLK)
			{	sp->sn_flag &= ~FIFO_RBLK;
				e_wakeupx(&sp->sn_revent, E_WKX_NO_PREEMPT);
			}
			/* wake up any sleeping read select()s */
			if (sp->sn_poll & POLLIN)
			{
				sp->sn_flag |= SELDONE;
				selnotify (POLL_FIFO,sp,POLLIN);
				sp->sn_poll &= ~POLLIN;
			}
		}
	}

	if ((sp->sn_rcnt == 0) && (sp->sn_wcnt == 0))
	{
		/* update times only if there were bytes flushed from fifo */
		if (sp->sn_size != 0)
			smark (sp, IUPD|ICHG, crp);

		sp->sn_rptr = 0;
		sp->sn_wptr = 0;
		sp->sn_size = 0;

		/* free all buffers associated with this fifo,
		 */
		fifo_buffree(sp);

	}
	FIFO_DEBUG("end-close", sp, 0);
	SPECNODE_UNLOCK(sp);
	return 0;
}

/*
 * NAME:	fifo_rdwr (vp, rwmode, flags, uiop, ext, vinfo, vattrp, crp)
 *
 * FUNCTION:	Read or write character device
 *
 * PARAMETERS:	vp	- spec vnode to r or w
 *		rwmode	- r or w
 *		flag	- open flags
 *		uiop	- locality info
 *		ext	- extended open info
 *		vinfo	- vfs specific info
 *              vattrp  - attributes to return
 *              crp     - credentials
 *
 * RETURN :	returns from devsw[] interface
 *		EINVAL	- negative r/w offset
 */

/* ARGSUSED */
fifo_rdwr (vp, rw, flags, uiop, ext, vinfo, vattrp, crp)
struct vnode 	*vp;			/* Vnode to r or w */
enum uio_rw 	rw;			/* r or w */
int 		flags;			/* open flags */
struct uio 	*uiop;			/* locality info */
int 		ext;			/* extended open flag */
caddr_t 	vinfo;			/* Vfs specific info */
struct vattr	*vattrp;		/* attrubutes */
struct ucred    *crp;			/* credentials */
{
	struct specnode *sp;
	int rc = 0;

	uiop->uio_offset = 0;

	sp = VTOSP (vp);
	if (rw == UIO_WRITE)
		rc = fifo_write (sp, flags, uiop, crp);
	else
		rc = fifo_read (sp, flags, uiop, crp);

	if (vattrp)
		vattrp->va_flags = VA_NOTAVAIL;

	/* guarantee that f_offset stays 0 */
	uiop->uio_offset = 0;
	return rc;
}

#ifdef FEFIFOFUM
/*
 * NAME:	fifo_rw (fp, rw, uiop, ext)
 *
 * FUNCTION:	Read or write character device
 *
 * PARAMETERS:	fp	- file pointer to read or write
 *		rw	- r or w
 *		uiop	- locality info
 *		ext	- extended open info
 *
 * RETURN :	returns from devsw[] interface
 *		EINVAL	- negative r/w offset
 */

fifo_rw(fp, rw, uiop, ext)
struct file	*fp;			/* ^ to file struct	*/
enum uio_rw 	rw;			/* r or w		*/
struct uio 	*uiop;			/* I/O vector		*/
{
	register struct specnode *sp;
	register int rc = 0;
	register flags;

	uiop->uio_offset = 0;		/* f_offset stays zero	*/
	flags = fp->f_flag;

	sp = VTOSP(fp->f_vnode);

	if (rw == UIO_WRITE)
		rc = fifo_write(sp, flags, uiop);
	else
		rc = fifo_read(sp, flags, uiop);

	uiop->uio_offset = 0;		/* f_offset stays zero	*/

	return rc;
}
#endif

/*
 * NAME:	fifo_getattr (vp, vap, cmd, flag)
 *
 * FUNCTION:	stat(2) fifo. Set times in specnode. Set "real" fifo size and
 *		block size in vattr.
 *
 * PARMETERS:	vp	- fifo of interest
 *		vap	- vattr struct
 *		cmd	- level of stat
 *		flag	- obsolete
 *
 * RETURN:	returns from real xix_getattr()
 */

/* ARGSUSED */
int
fifo_getattr (
	struct vnode *	vp,		/* pipe or fifo vnode to stat	*/
	struct vattr *	vap,		/* attribute struct to fill in	*/
	int		cmd,		/* command (ignored)		*/
	int		flag,		/* flags (ignored)		*/
	struct ucred *	crp)		/* credentials                  */
{
	int		rc = 0;		/* return code			*/
	struct specnode *snp;		/* specnode for fifo or pipe	*/

	snp = VTOSP(vp);

	/* get a fifo's attributes from the PFS */
	if (vp->v_pfsvnode)
	{
		rc = VNOP_GETATTR(vp->v_pfsvnode, vap, crp);
		vap->va_size = snp->sn_size;
	}
	else
	{
		/* For a pipe, the device is NODEVICE, the inode number is the
		 * specnode address, and the rest of the fields are kept in the
		 * specnode.
		 */
		bzero(vap, sizeof *vap);
		SPECNODE_LOCK(snp);
		vap->va_serialno =      (long)snp;
		vap->va_type =		VFIFO;
		vap->va_nlink =		1;
		vap->va_mode =		snp->sn_mode;
		vap->va_uid =		snp->sn_uid;
		vap->va_gid =		snp->sn_gid;
		vap->va_dev =		NODEVICE;
		vap->va_atime =		snp->sn_atime;
		vap->va_mtime =		snp->sn_mtime;
		vap->va_ctime =		snp->sn_ctime;
		vap->va_nid =		xutsname.nid;
		vap->va_size =		snp->sn_size;
		SPECNODE_UNLOCK(snp);
	}
	vap->va_blocksize = PIPE_BSZ;

	return rc;
}

/*
 * NAME:	fifo_setattr(vp, cmd, arg1, arg2, arg3)
 *
 * FUNCTION:	This function updates the owner, group, or times
 *		for the file based upon the specified command.
 *
 * PARAMETERS:	vp	- fifo vnode to set attributes of
 *		cmd	- command indicating what portion of the attributes
 *				we wish to set and how the arguments
 *				should be interpretted
 *		arg1	-  various arguments whose meaning is determined
 *		arg2		by the specified command
 *		arg3
 *
 * RETURNS:	error returns from the PFS setattr vnode op
 */

int
fifo_setattr (
	struct vnode *	vp,
	int		cmd,
	int		arg1,
	int		arg2,
	int		arg3,
	struct ucred *	crp)
{
	struct specnode *snp;		/* specnode for fifo or pipe	*/
	int		rc;		/* return code 			*/

	if (vp->v_pfsvnode)
		rc = VNOP_SETATTR(vp->v_pfsvnode, cmd, arg1, arg2, arg3, crp);
	else
	{
		snp = VTOSP(vp);
		SPECNODE_LOCK(snp);

		switch (cmd)
		{
			case V_MODE:
				rc = dochmod(snp->sn_uid,
					     snp->sn_gid,
					     arg1,		/* new mode */
					     &snp->sn_mode,
					     crp);
				break;
			case V_OWN:
				rc = dochown(arg1,		/* flags   */
					     arg2,		/* new uid */
					     arg3,		/* new gid */
					     &snp->sn_uid,
					     &snp->sn_gid,
					     &snp->sn_mode,
					     crp);
				break;
			case V_UTIME:
				rc = doutime(snp->sn_uid, /* objects uid */
					     arg1,	/* flags	*/
					     arg2,	/* new atime	*/
					     arg3,	/* new mtime	*/
					     &snp->sn_atime,
					     &snp->sn_mtime,
					     crp);
				break;
			default:
				rc = EINVAL;
				break;
		}
		SPECNODE_UNLOCK(snp);
	}

	return rc;

}

/*
 * NAME:	fifo_select (vp, corl, reqevents, rtneventsp, notify, vinfo)
 *
 * FUNCTION:	fifo specific select / poll routine.
 *
 * PARMETERS:	Vp 	   - pointer to a vnode
 *		corl       - correlator for this request
 *		reqevents  - requested events
 *		rtneventsp - pointer to returned events
 *		vinfo      - vfs specific information
 *
 * RETURN:	0
 */

/* ARGSUSED */
fifo_select (vp, corl, reqevents, rtneventsp, notify, vinfo, crp)
struct vnode *vp;
int corl;
ushort reqevents;
ushort *rtneventsp;
void (*notify)();
caddr_t vinfo;
struct ucred *crp;
{
	struct specnode *sp;
	int	was_locked;

	was_locked = FS_KLOCK(&kernel_lock, LOCK_SHORT);
	sp = VTOSP (vp);
	FIFO_DEBUG("begin-select", sp, reqevents);

	SPECNODE_LOCK(sp);
	if (reqevents & POLLIN) {
		/* NOTE check for "if this file readable" done in selscan now
		 * does pipe has data in it, or have all writers gone away
		 * (ref cnt must be zero and there must have been a previous
		 * writer).
		 */
		if ( (sp->sn_size > 0) || ((sp->sn_wcnt == 0) && 
		   ( sp->sn_flag & FIFO_WWRT ) ) )
			*rtneventsp |= POLLIN;	/* yes there is data here */
	}

	if (reqevents & POLLOUT)
		/* NOTE check for "if this file writable" done in selscan now
		 * does pipe have space in it, or have all readers gone away
		 */
		if ( !(PIPE_FULL(sp)) || (sp->sn_rcnt == 0) )
			*rtneventsp |= POLLOUT;	/* yes pipe has room */

	if (!(reqevents & POLLSYNC) &&
	    !((reqevents & *rtneventsp) && (reqevents & (POLLIN | POLLOUT)))) 
	{
		/* register for asynchronous event notification */
		sp->sn_poll |= reqevents;
		selreg(corl,POLL_FIFO,sp,reqevents,notify);
	}	
	sp->sn_flag &= ~SELDONE;
	SPECNODE_UNLOCK(sp);
	FIFO_DEBUG("end-select", sp, rtneventsp);
	if ( was_locked != LOCK_NEST)
		FS_KUNLOCK(&kernel_lock);
        return (0);
}

fifo_write (
	struct specnode	*sp,		/* Fifo's specnode to w */
	int 		flags,		/* open flags */
	struct uio 	*uiop,		/* locality info */
	struct ucred    *crp)		/* credentials */
{
	u_int blk;
	u_int cnt;
	int off;
	unsigned i;
	int rc = 0;
	int ocnt = uiop->uio_resid;	/* save original request size */

	FIFO_DEBUG("begin-write", sp, uiop);
	SPECNODE_LOCK(sp);

	/* PIPE_SIZE: max number of bytes buffered per open pipe
	 * PIPE_MAX: max size of single write to a pipe
	 *
	 * If the cnt is less than PIPE_ATOMIC, it must occur
	 * atomically.  If it does not currently fit in the
	 * kernel pipe buffer, either sleep or return EAGAIN (POSIX),
	 * depending on FNDELAY  (library routine translates).
	 *
	 * If the cnt is greater than PIPE_ATOMIC, it will be
	 * non-atomic (FNDELAY clear).  If FNDELAY is set,
	 * write as much as will fit into the kernel pipe buffer
	 * and return the number of bytes written.
	 *
	 * If the cnt is greater than PIPE_MAX, return EINVAL.
	 */

	if ((uint)uiop->uio_resid > PIPE_MAX)
	{	rc = EINVAL;
		goto wrdone;
	}

	while (cnt = uiop->uio_resid)
	{	if (sp->sn_rcnt == 0)
		{	/* no readers anymore! */
			pidsig(u.u_procp->p_pid, SIGPIPE);
			rc = EPIPE;
			goto wrdone;
		}

		if ((cnt + sp->sn_size) > PIPE_SIZE)
		{	if (flags & (FNDELAY|FNONBLOCK)) /* NO DELAY */
			{
				/* Only check for atomic writes the first time
				 * (cnt == ocnt?) because cnt is a residual
				 * count of what remains to be written.
				 */
				if (cnt == ocnt && cnt <= PIPE_ATOMIC)
				{
					/* Write will be satisfied
					 * atomically, later.
					 */
					rc = (flags & FNONBLOCK) ? EAGAIN : 0;
					goto wrdone;

				}
				else
				{
					if (PIPE_FULL(sp))
					{
						/* Write will never be atomic.
						 * At this point it cant even be
						 * partial. But some portion
						 * of the write may already have
						 * succeeded.  If so, cnt
						 * reflects this.
						 */
						if (cnt == ocnt)
							rc = EAGAIN;
						goto wrdone;
					}
				}
			}
			else			/* DELAY */
			{
				if (cnt <= PIPE_ATOMIC || PIPE_FULL(sp))
				{
					/* Sleep until room for this request.
					 * On wakeup, go top of the loop.
					 */
					sp->sn_flag |= FIFO_WBLK;
					rc = e_sleep_thread(&sp->sn_wevent, 
						&sp->sn_lock,
						LOCK_SIMPLE|INTERRUPTIBLE);
					switch (rc)
					{
						case THREAD_AWAKENED:
							rc = 0;
							break;

						case THREAD_INTERRUPTED:
							rc = EINTR;
							goto wrdone;

						default:
							/* we aren't sure how we got here */
							rc = 0;
					}
					goto wrloop;
				}
			}

			/* at this point, can do a partial write */

			cnt = PIPE_SIZE - sp->sn_size;
		}

		/* Can write 'cnt' bytes to pipe now.   Make sure
		 * there is enough space in the allocated buffers.
		 * If allocation does not succeed immediately, go back
		 * to the  top of the loop to make sure everything is
		 * still cool.
		 */

		ASSERT (((sp->sn_wptr - sp->sn_rptr) & PIPE_SIZE_MASK) ==
			(sp->sn_size & PIPE_SIZE_MASK));

		if (! fifo_bufalloc(sp, cnt, flags))
		{
			if (flags & (FNDELAY|FNONBLOCK)) /* NO DELAY */
			{
				if (cnt == ocnt)
					rc = (flags & FNONBLOCK) ? EAGAIN : 0;
				goto wrdone;
			}
			goto wrloop;
		}

		/* There is now enough space to write 'cnt' bytes.
		 * Find append point and copy new data.
		 */

		off = BUFOFFSET(sp->sn_wptr);
		while (cnt)
		{	i = PIPE_BSZ - off;
			i = MIN(cnt, i);
			blk = BUFBLOCK(sp->sn_wptr);

			rc = uiomove (sp->sn_buf[blk]+off, i, UIO_WRITE, uiop);

			if (rc)
			{
				/* release extra buffers that might 
				 * have ben allocated at the end
				 */
				fifo_buffree(sp);
				goto wrdone;
			}

			sp->sn_size += i;
			sp->sn_wptr += i;
			cnt -= i;
			off = 0;
		}

		smark(sp, IUPD|ICHG, crp);	/* update mod times */

		/* wake up any sleeping readers */
		if (sp->sn_flag & FIFO_RBLK)
		{	sp->sn_flag &= ~FIFO_RBLK;
			e_wakeupx(&sp->sn_revent, E_WKX_NO_PREEMPT);
		}

		/* wake up any sleeping read selectors */
		if (sp->sn_poll & POLLIN)
		{
			sp->sn_flag |= SELDONE;
			selnotify (POLL_FIFO, sp, POLLIN);
			sp->sn_poll &= ~POLLIN;
		}
wrloop:
		continue;		/* write loop */
	}

wrdone:
	FIFO_DEBUG("end-write", sp, uiop);
	SPECNODE_UNLOCK(sp);
	return rc;
}

fifo_read (
	struct specnode *sp,		/* Fifo's specnode to read */
	int 		flags,		/* open flags */
	struct uio 	*uiop,		/* locality info */
	struct ucred    *crp)		/* credentials */
{
	u_int blk;
	u_int cnt;
	int off;
	unsigned i;
	int rc = 0;

	FIFO_DEBUG("begin-read", sp, uiop);
	SPECNODE_LOCK(sp);

	/* Handle zero-length reads specially here
	 */

	if ((cnt = uiop->uio_resid) == 0)
		goto rdexit;

	while ((i = sp->sn_size) == 0)
	{	if (sp->sn_wcnt == 0)	/* no data and no writers...(EOF) */
			goto rdexit;

		/* No data in pipe, but writer is there;
		 * if No-Delay, return EAGAIN (POSIX)
		 */
		if (flags & (FNONBLOCK|FNDELAY))
		{	rc = (flags & FNONBLOCK) ? EAGAIN : 0;
			goto rdexit;
		}

		sp->sn_flag |= FIFO_RBLK;
		rc = e_sleep_thread(&sp->sn_revent, &sp->sn_lock, 
					LOCK_SIMPLE|INTERRUPTIBLE);
		switch (rc)
		{
			case THREAD_AWAKENED:
				rc = 0;
				break;

			case THREAD_INTERRUPTED:
				rc = EINTR;
				goto rdexit;

			default:
				/* we aren't sure how we got here */
				rc = 0;
		}

		/* loop to make sure there is still a writer */
	}

	ASSERT (((sp->sn_wptr - sp->sn_rptr) & PIPE_SIZE_MASK) ==
		(sp->sn_size & PIPE_SIZE_MASK));

	/* Get offset into first buffer at which to start getting data.
	 * Truncate read, if necessary, to amount of data available.
	 */
	cnt = MIN (cnt, i);	/* smaller of pipe size and read size */
	off = BUFOFFSET(sp->sn_rptr);

	while (cnt)
	{	i = PIPE_BSZ - off;
		i = MIN(cnt, i);
		blk = BUFBLOCK(sp->sn_rptr);

		if (rc = uiomove(sp->sn_buf[blk]+off, (int)i, UIO_READ, uiop))
			goto rddone;

		sp->sn_size -= i;
		sp->sn_rptr += i;
		cnt -= i;
		off = 0;
	}

	smark(sp, IACC, crp);	/* update the access times */

	/* wake up any sleeping writers */

	if (sp->sn_flag & FIFO_WBLK)
	{	sp->sn_flag &= ~FIFO_WBLK;
		e_wakeupx(&sp->sn_wevent, E_WKX_NO_PREEMPT);
	}

	/* wake up any sleeping write selectors */
	if (sp->sn_poll & POLLOUT)
	{
		sp->sn_flag |= SELDONE;
		selnotify(POLL_FIFO, sp, POLLOUT);
		sp->sn_poll &= ~POLLOUT;
	}

rddone:
	fifo_buffree (sp);

rdexit:
	FIFO_DEBUG("end-read", sp, uiop);
	SPECNODE_UNLOCK(sp);
	return (rc);
}

void
fifo_pool_init()
{
        int curmempages;

        curmempages = vmker.nrpages - vmker.badpages;
	
	/*
	 * pipe_maxfree is smallest size fifo pool will drop to.  It scales
	 * at 16 pages per 8 MB of main memory.  Until reaching a max of
	 * 1 MB. Units are bytes.
	 */
	
	pipe_maxfree = curmempages / PIPE_MAX_DIVIDER;
	
		if( pipe_maxfree > PIPE_MAX_GROW )
			pipe_maxfree = PIPE_MAX_GROW;
	
		pipe_maxfree = pipe_maxfree * PAGESIZE;
	
	/*
	 * pipe_mnb is max size pool can grow too. Previously it was nailed
	 * at 16 MB.  Now pipe_mnb = max of half of curmempages up until it
	 * hits 16 MB which is its max.
	 */
	
	pipe_mnb = curmempages / PIPE_MNB_DIVIDER;

	if( pipe_mnb > PIPE_MNB_GROW )
		pipe_mnb = PIPE_MNB_GROW;

	pipe_mnb = pipe_mnb * PAGESIZE;
}

/*
 * Allocate enough buffers so that the fifo can hold cnt more bytes.
 * This code assumes that there is at least cnt bytes of room in the pipe.
 * Return:
 *	1 if could allocate the buffers
 *	0 if would exceed pipe_mnb or malloced failed and FNDELAY|FNONBLOCK
 *	  are set.
 */
static int
fifo_bufalloc (sp, cnt, flags)
struct specnode *sp;
int cnt;
int flags;
{
	u_int blk;
	u_int off;
	int malloced = 0;	/* did we malloc data buffers */
	int rc;			/* return code from e_sleep_thread */

	FIFO_DEBUG("begin-bufalloc", sp, 0);

	ASSERT(sp->sn_size + cnt <= PIPE_SIZE);

	off = BUFOFFSET(sp->sn_wptr);
	blk = BUFBLOCK(sp->sn_wptr);

	/* if there is a partial buffer at the write end
	 */
	if (off != 0)
	{
		/* ... and the request fits, return ok
		 */
 		if (off + cnt <= PIPE_BSZ)
		{
			FIFO_DEBUG("end-bufalloc", sp, 0);
			return 1;
		}

		/* start allocating at the next block address
		 */
		blk++;
		blk &= NFBUFMASK;
		cnt -= PIPE_BSZ - off;
	}

	/* Take the pipe buffer allocation lock to serialize allocation */
	FIFOBUF_LOCK();

	while (cnt > 0) {
		/* Impose a system-wide maximum on buffered data in pipes.
		 */
		if (fifo_alloc.count + cnt - fifo_alloc.nfree <= pipe_mnb) {

			/* If there is not a buffer already there allocate one.
			 * The only way this can happen is when there is a
			 * partially unread buffer and the write is going to 
			 * wrap around into it.
			 */
			if (!sp->sn_buf[blk]) {
				if (sp->sn_buf[blk] = fifo_alloc.free->buf) {
					/*
					 * got one from the free list.
					 * pop free list, and reduce nfree.
					 */
					fifo_alloc.free = fifo_alloc.free->free;
					fifo_alloc.nfree -= PIPE_BSZ;
				} else if (sp->sn_buf[blk] = malloc(PIPE_BSZ)) {
					/*
					 * mallocated another one.
					 */
					fifo_alloc.count += PIPE_BSZ;
					assert(fifo_alloc.count <= pipe_mnb);
				} else {
					/*
					 * malloc failed
					 */
					malloced = 1;
					goto sleep;
				}
			} 
#ifdef DEBUG
			else {
				DBGASSERT(cnt <= PIPE_BSZ, sp);
			}
#endif
			cnt -= PIPE_BSZ;
			blk++;
			blk &= NFBUFMASK;
			continue;
		}

sleep:
		/* 
		 * if no delay, return 0.
		 * Make sure to free any malloc'ed data buffers
		 */
		if (flags & (FNDELAY|FNONBLOCK)) {
			FIFOBUF_UNLOCK();
			if (malloced)
				/* free the malloc'ed data buffers */
				fifo_buffree(sp);
			FIFO_DEBUG("end-bufalloc", sp, 0);
			return 0;
		} else {
			fifo_alloc.allocblk = 1;
			(void)e_assert_wait(&fifo_alloc.aevent, INTERRUPTIBLE);
			FIFOBUF_UNLOCK();
			SPECNODE_UNLOCK(sp);

			rc = e_block_thread();
			
			SPECNODE_LOCK(sp);

			if (rc != THREAD_AWAKENED)
				return 0;

			FIFOBUF_LOCK();
		}
	}

	/* unlock buffer lock before return */
	FIFOBUF_UNLOCK();

	FIFO_DEBUG("end-bufalloc", sp, 0);

	return 1;
}

/* Release the buffers that are not needed to hold
 * the data queued in the pipe. 
 * The number of buffer pointers is small so it is
 * faster to do all the work here than to pass information
 * about what is being released from the callers.
 * The implementation is simple,
 *	- count the number of buffers that should be there
 *	  to cover the (wrapping) interval:
 *		[sn_rptr, sn_rptr + sn_size - 1]
 *	  (call that nbufs)
 *	- release (NFBUF - nbufs) buffers starting from
 *	  buffer number: (BUFBLOCK(sn_rptr) + nbufs)
 */

static void
fifo_buffree (sp)
struct specnode *sp;
{
	int	nbufs;
	int	size;
	int	nfree;
	u_int	blk;

	FIFO_DEBUG("begin-buffree", sp, 0);

	/* Take the pipe buffer allocation lock to serialize allocation */
	FIFOBUF_LOCK();

	/* Compute nbufs, nfree and size.
	 * Nfree is the number of buffers to release.
	 * Size is the number of bytes in the pipe plus the
	 * number of bytes free in the first and last buffers.
	 */
	if (sp->sn_size == 0)
	{
		sp->sn_rptr = 0;
		sp->sn_wptr = 0;
		nfree = NFBUF;
		blk = 0;
	}
	else
	{
		size = sp->sn_size + BUFOFFSET(sp->sn_rptr) +
			((PIPE_BSZ - BUFOFFSET(sp->sn_wptr)) & PIPE_BSZ_MASK);
		nbufs = size >> L2PIPE_BSZ;
		blk = BUFBLOCK(sp->sn_rptr);
		blk += nbufs;
		blk &= NFBUFMASK;
		nfree = NFBUF - nbufs;
	}

#ifdef	DEBUG
	/*
	 * The previous code took care of the fallowing boundary cases:
	 *
	 *	0 Empty pipe:
	 *		special case
	 */

	/* 1 Full pipe:
	 *	{sn_size == PIPE_SIZE}
	 *	==>>
	 *	size >= PIPE_SIZE, nbufs >= NFBUF, nfree <= 0
	 *   in this case the release loop below will not be entered
	 */
/*1*/	if (sp->sn_size == PIPE_SIZE)
		DBGASSERT(size >= PIPE_SIZE && nbufs >= NFBUF && nfree <= 0,sp);

	/* 2 Pipe not empty, pipe not full, all buffers needed,
	 *   i.e. no free buffer slots between the buffer for the
	 *   last byte written and the next byte to be read:
	 */
/*2*/	if (sp->sn_size != 0 && sp->sn_size != PIPE_SIZE &&
	    freeslots(sp) == 0)
	{

		/* 2.1 Last byte written and next byte to be read
		 *     are in the same buffer:
		 *	  {"pipe not empty"},
		 *	  {"no free buffer slots"},
		 *	  {"last byte written(w) and next data to be read(r)
		 *	    are in the same buffer"}
		 *		..|xx..xx|..wf..fr..|xx..xx|..
		 *			(x)in-use, (f)available
		 *			sn_size == count of [rwx] bytes
		 *	  ==>>
		 *	  {PIPE_SIZE - sn_size <= PIPE_BSZ-2}
		 *	  ==>>
		 *	  size >= PIPE_SIZE, nbufs >= NFBUF, nfree <= 0
		 *     in this case the release loop below will not be entered
		 */
/*2.1*/		if (BUFBLOCK(sp->sn_rptr) == BUFBLOCK(sp->sn_wptr - 1))
		{
			DBGASSERT(PIPE_SIZE - sp->sn_size <= PIPE_BSZ-2, sp);
			DBGASSERT(size >= PIPE_SIZE && nbufs >= NFBUF &&
				nfree <= 0, sp);
		}

		/* 2.2 Last byte written and next byte to be read
		 *     are not in the same buffer:
		 *	  {"last byte written(w) and next data to be read(r)
		 *     	    are not in the same buffer"}
		 *	  {"no free buffer slots"},
		 *	  ==>>
		 *	  {"last byte written(w) and next data to be read(r)
		 *	    are in contiguous buffers"},
		 *		-o-
		 *	  {"pipe not empty"},
		 *	  {"no free buffer slots"},
		 *	  {"last byte written(w) and next data to be read(r)
		 *	    are in contiguous buffers"},
		 *		..|xx..xx|..xwf..|..frx..|xx..xx|.. 
		 *			(x)in-use, (f)available
		 *			sn_size == count of [rwx] bytes
		 *	  ==>>
		 *	  {PIPE_SIZE - sn_size <= 2*PIPE_BSZ - 2}
		 *	  ==>>
		 *	  size == PIPE_SIZE, nbufs == NFBUF, nfree == 0
		 *     in this case the release loop below will not be entered
		 */
/*2.2*/		else
		{
			DBGASSERT(PIPE_SIZE - sp->sn_size <= 2*PIPE_BSZ - 2,	
				sp);
			DBGASSERT(size == PIPE_SIZE && nbufs == NFBUF &&
				nfree == 0, sp);
		}
	}

	/* 3 Pipe not empty, pipe not full, not all buffers needed:
	 */
/*3*/	if (sp->sn_size != 0 && sp->sn_size != PIPE_SIZE &&
	    freeslots(sp) != 0)
	{

		/* 3.1 Last byte written and next byte to be read
		 *     are in the same buffer:
		 *	  {"pipe not empty"},
		 *	  {"free buffer slots"},
		 *	  {"last byte written(w) and next data to be read(r)
		 *     	    are in the same buffer"}
		 *		..|ff..ff|..frx..xwf..|ff..ff|..
		 *			(x)in-use, (f)available
		 *			sn_size == count of [rwx] bytes
		 *	  ==>>
		 *	  size == PIPE_BSZ, nbufs == 1, nfree == NFBUF-1
		 *     in this case the release loop below will be entered
		 */
/*3.1*/		if (BUFBLOCK(sp->sn_rptr) == BUFBLOCK(sp->sn_wptr - 1))
		{
			DBGASSERT(size == PIPE_BSZ && nbufs == 1 &&
				nfree == NFBUF-1, sp);
		}

		/* 3.2 Last byte written and next byte to be read
		 *     are not in the same buffer:
		 *        Non boundary case ...
		 */
	}

	DBGASSERT(nfree < 0 || freeslots(sp) <= nfree, sp);
#endif

	while (nfree > 0) {
		if (sp->sn_buf[blk]) {
			if (fifo_alloc.nfree>=pipe_maxfree || ps_not_defined) {
				/*
				 * already holding enuf free space, or
				 * we are not yet paging, give them back.
				 */
				fifo_alloc.count -= PIPE_BSZ;
				free(sp->sn_buf[blk]);
			} else {
				/*
				 * room to breathe.  hold on to up to
				 * pipe_maxfree bytes of VM.
				 *
				 * Once we are able to efficiently clear the
				 * mod bit in the pft, it would be wise to
				 * do so here.
				 */
				union freepage *fp;
				
				fp = (void *) sp->sn_buf[blk];
				fp->free = fifo_alloc.free;
				fifo_alloc.free = fp;

				fifo_alloc.nfree += PIPE_BSZ;
			}
			sp->sn_buf[blk] = NULL;
		}
		blk++;
		blk &= NFBUFMASK;
		--nfree;
	}

	if (fifo_alloc.allocblk) {
		fifo_alloc.allocblk = 0;
		e_wakeupx(&fifo_alloc.aevent, E_WKX_NO_PREEMPT);
	}

	FIFOBUF_UNLOCK();

	FIFO_DEBUG("end-buffree", sp, 0);
}

int
fifo_badop()
{
	panic("fifo_badop\n");
}


int
fifo_einval(void)
{
	return EINVAL;
}

int
fifo_noop(void)
{
	return 0;
}

/*
 * NAME:	fifo_link(vp, dvp, name)
 *
 * FUNCTION:	This function creates a link to vp in directory dvp by
 *		the name "name".
 *
 * PARAMETERS:	vp 	- vnode that represents the file to link to
 *		dvp	- directory in which name is to be created
 *		name	- name of new link
 *
 * RETURN:	errors from the PFS link vnode operations
 */
int
fifo_link (
	struct vnode *	vp,
	struct vnode *	dvp,
	char *		name,
	struct ucred *	crp)
{
	/* The LFS won't call us with a pipe. */
	return VNOP_LINK(vp->v_pfsvnode, dvp, name, crp);
}

/*
 * NAME:	fifo_remove(vp, dvp, name)
 *
 * FUNCTION:	This function removes the object vp (named by "name")
 *		from directory dvp.
 *
 * PARAMETERS:	vp 	- vnode of the object we want to remove
 *		dvp	- parent directory of vp
 *		name 	- name for vp
 *
 * RETURN:	errors from the PFS link vnode operation
 */
int
fifo_remove (
	struct vnode *	vp,
	struct vnode *	dvp,
	char *		name,
	struct ucred *	crp)
{
	/* The LFS won't call us with a pipe. */
	return VNOP_REMOVE(vp->v_pfsvnode, dvp, name, crp);
}

/*
 * NAME:	fifo_rename(svp, spvp, sname, dvp, dpvp, dname)
 *
 * FUNCTION:	Atomic rename of svp(name "sname") to dvp(name "dname").
 *
 * PARAMETERS:	svp	- vnode for sname, the source
 *		spvp	- svp's parent vnode
 *		sname	- name refering to svp, the source
 *		dvp	- destination vnode
 *		dpvp	- parent of dvp
 *		dname	- destination name
 *
 * RETURN: 	errors from the PFS rename vnode operation
 */

int
fifo_rename (
	struct vnode  *	svp,
	struct vnode  *	spvp,
	caddr_t 	sname,
	struct vnode  *	dvp,
	struct vnode  *	dpvp,
	caddr_t 	dname,
	struct ucred  * crp)
{
	/* The LFS won't call us with a pipe. */
	return VNOP_RENAME(svp->v_pfsvnode, spvp, sname, dvp, dpvp, dname, crp);
}

/*
 * NAME:	fifo_fid(vp, fidp)
 *
 * FUNCTION: 	This function modifies the file identifier pointed to by
 *		the fhp.
 *
 * PARAMETERS:	vp	- Pointer to the vnode that represents the object
 *			  for which a file handle is to be constructed.
 *		fidp	- Returned file handle
 *
 * RETURN:	errors from the PFS fid vnode operation
 */

int
fifo_fid (
	struct vnode *	vp,
	struct fileid *	fidp,
	struct ucred *  crp)
{
	/* The LFS won't call us with a pipe. */
	return VNOP_FID(vp->v_pfsvnode, fidp, crp);
}

/*
 * NAME:	fifo_hold(vp)
 *
 * FUNCTION:	This function increments the hold count on a vnode.
 *
 * PARAMETERS:	vp	- vnode to hold.
 *
 * RETURNS:	errors from PFS hold vnode operation
 */
int
fifo_hold (struct vnode *	vp)
{
	struct	specnode *snp;		/* specnode for this fifonode */
	int		rc = 0;		/* return code	*/
	int		snlocked;   	/* specnode locked on entry */

	snp = VTOSP(vp);
	if (!(snlocked = lock_mine(&snp->sn_lock)))
		SPECNODE_LOCK(snp);

	/* increment hold count on the spec vnode */
	++vp->v_count;

	/* For fifos, increment the hold count of the PFS vnode. */
	if (vp->v_pfsvnode)
		rc = VNOP_HOLD(vp->v_pfsvnode);
	
	if (!snlocked)
		SPECNODE_UNLOCK(snp);
	return rc;
}

/* 
 * NAME:	fifo_rele(vp)
 *
 * FUNCTION:	This function decrements the count on a vnode.  If it was
 *		the last reference to the vnode, the memory associated
 *		with the vnode is released.
 *
 * PARAMETERS:	vp	- the doomed vnode pointer
 *
 * RETURNS:	errors from the PFS rele vnode operation
 */
int
fifo_rele (struct vnode *	vp)
{
	int		rc = 0;		/* return code			*/
	struct specnode *snp;		/* specnode for fifo		*/
	struct vnode	*pvp;		/* PFS vnode, if any		*/
	int		snlocked;   	/* specnode locked on entry */

	snp = VTOSP(vp);
	if (!(snlocked = lock_mine(&snp->sn_lock)))
		SPECNODE_LOCK(snp);

	/* decrement the hold count on the spec vnode */
	--vp->v_count;

	/* Hang onto the PFS vnode so we can release it later. */
	pvp = vp->v_pfsvnode;

	/* If this is the last reference to the vnode, release the
	 * memory associated with the vnode.
	 */
	if (vp->v_count == 0)
	{

		vn_free(vp);
		(void)snput(snp);
	}
	else
		if (!snlocked)
			SPECNODE_UNLOCK(snp);

	/* For fifos, decrement the hold count of the PFS vnode.
	 * We do this after freeing the spec vnode so that the spec
	 * vnode won't be in the PFS vfs at the time the PFS vnode is
	 * released.  Forced unmount expects to find no vnodes when the
	 * last PFS vnode in the vfs is released.
	 */
	if (pvp)
		rc = VNOP_RELE(pvp);

	return rc;
}

int
fifo_access (
	struct vnode *	vp,		/* fifo vnode to check access	*/
	int		mode,		/* access mode to check		*/
	int		who,		/* whom to check access for	*/
	struct ucred *  crp)		/* credentials */
{
	struct specnode	*snp;
	int rc = 0;

	snp = VTOSP(vp);
	/* The access information for pipes is in memory */
	if (!vp->v_pfsvnode)
	{
		SPECNODE_LOCK(snp);
		rc =  ic_specaccess(snp, mode, who, crp);
		SPECNODE_UNLOCK(snp);
		return rc;
	}

	/* let the PFS check for access to fifos */
	return VNOP_ACCESS(vp->v_pfsvnode, mode, who, crp);
}

fifo_lockctl (
	struct vnode *	vp,
	offset_t	offset,
	struct eflock *	lckdat,
	int		cmd,
	int		(* retry_fcn)(),
	ulong *		retry_id,
	struct ucred *	crp)
{
	int rc;

	return common_reclock(VTOGP(vp),
			      0,		/* size */
			      offset,
			      lckdat,
			      cmd,
			      retry_fcn,
			      retry_id,
			      NULL,
			      NULL);
}

int
fifo_getacl (
	struct vnode *	vp,
	struct uio   *	uiop,
	struct ucred *  crp)
{
	struct specnode *snp;
	int	rc = 0;			/* return code */

	/* let the PFS handle ACL's on fifos */
	if (vp->v_pfsvnode)
		return VNOP_GETACL(vp->v_pfsvnode, uiop, crp);

	/* ACL's are in memory */
	snp = VTOSP(vp);
	SPECNODE_LOCK(snp);
	rc = ic_getacl(snp->sn_acl, snp->sn_mode, uiop);
	SPECNODE_UNLOCK(snp);
	return rc;
}

int
fifo_setacl (
	struct vnode *	vp,
	struct uio   *	uiop,
	struct ucred *	crp)
{
	struct specnode *snp;
	int 	rc = 0;

	/* let the PFS handle ACL's on fifos */
	if (vp->v_pfsvnode)
		return VNOP_SETACL(vp->v_pfsvnode, uiop, crp);

	/* ACL's are in memory */
	snp = VTOSP(vp);
	if ((crp->cr_uid != snp->sn_uid) && privcheck_cr(SET_OBJ_DAC, crp))
		return EPERM;
	else
	{
		SPECNODE_LOCK(snp);
		rc = ic_setacl(&snp->sn_acl, &snp->sn_mode, snp->sn_gid,
				uiop, crp);
		SPECNODE_UNLOCK(snp);
		return rc;
	}
}

int
fifo_getpcl (
	struct vnode *	vp,
	struct uio   *	uiop,
	struct ucred *	crp)
{
	/* pcl's are not supported on pipes */
	if (vp->v_pfsvnode)
		return VNOP_GETPCL(vp->v_pfsvnode, uiop, crp);
	else
		return EINVAL;
}

int
fifo_setpcl (
	struct vnode *	vp,
	struct uio   *	uiop,
	struct ucred *	crp)
{
	/* pcl's are not supported on pipes */
	if (vp->v_pfsvnode)
		return VNOP_SETPCL(vp->v_pfsvnode, uiop, crp);
	else
		return EINVAL;
}

#ifdef	DEBUG
static int
freeslots(sp)
struct specnode *sp;
{
	int		bump;
	int		slots;
	int		cnt;
	int		i;
	off_t		off;
	int		first;

	/* simulate a write that could fill up the pipe,
	 * each time around the loop a block would be allocated
	 * except on:
	 *	a partially written block on the first iteration
	 *	a partially read block at the last iteration
	 */
	cnt = PIPE_SIZE - sp->sn_size;
	off = BUFOFFSET(sp->sn_wptr);
	first = 1;
	slots = 0;
	while (cnt > 0)
	{
		bump = 0;
		slots++;
		if (first && off)			/* first iteration */
			bump = 1;
		i = PIPE_BSZ - off;
		i = MIN(cnt, i);
		cnt -= i;
		off = 0;
		if (cnt <= 0 && BUFOFFSET(sp->sn_rptr))
			bump = 1;			/* last iteration */
		first = 0;
		slots -= bump;
	}
	DBGASSERT(slots >= 0 && slots <= NFBUF, sp);

	return slots;
}

fifo_debug (name, sp, uiop)
char		*name;
struct specnode	*sp;
struct uio 	*uiop;
{
	int	i;
	if (fifo_print)
	{
		printf ("\n\n\n");
		if (name)
			printf ("%s: ", name);
		printf ("-%x- ", curproc->p_pid);
		if (sp)
		{
			printf ("wcnt=%d rcnt=%d flag=0x%x\n"
				"rptr=0x%x wptr=0x%x size=%d\n",
				sp->sn_wcnt,
				sp->sn_rcnt,
				sp->sn_flag,
				sp->sn_rptr,
				sp->sn_wptr,
				sp->sn_size);
			printf ("freeslots=%d\n", freeslots (sp));
		}
		if (sp)
		{
			for (i = 0; i < NFBUF/2; i++)
				printf ("[%d]=0x%x ", i, sp->sn_buf[i]);
			printf ("\n");
			for (; i < NFBUF; i++)
				printf ("[%d]=0x%x ", i, sp->sn_buf[i]);
			printf ("\n");
		}
		if (uiop)
			printf ("offset=0x%x resid=%d\n",
				uiop->uio_offset,
				uiop->uio_resid);
	}
}

#endif
