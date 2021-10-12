static char sccsid[] = "@(#)15        1.6  src/bos/kernext/pse/str_filesys.c, sysxpse, bos411, 9428A410j 11/10/93 11:05:58";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      fd_to_cookie
 *                 fd_alloc
 *                 cookie_destroy
 *                 sth_fd_to_sth
 *                 sth_fattach
 *                 sth_update_times 
 *                 
 * 
 * ORIGINS: 63, 71, 83
 * 
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
 */
/** Copyright (c) 1988  Mentat Inc.
 **/

/*
 *	File: str_filesys.c - interface between STREAMS and file system
 */

#include <sys/errno.h>
#include <sys/time.h>	/* vnode.h needs this ... */
#include <sys/stat.h>
#include <sys/user.h>

#include <pse/str_stream.h>
#include <pse/str_proto.h>
#include <pse/str_debug.h>

#include <sys/specnode.h>

/*
 * List of routines in this file
 *
 * fd_to_cookie   - translate a fd to a file cookie
 * fd_alloc       - allocate a file desciptor for given file cookie
 * cookie_destroy - deallocate a file cookie
 * sth_fd_to_sth  - find and return fd associated with fd
 *
 *
 * fd_to_cookie, fd_alloc, cookie_destroy
 *
 * are needed to implement the passing of file descriptors between
 * processes via STREAMS pipes using I_SENDFD / I_RECVFD operations.
 * For each call to fd_to_cookie(), there will be exactly one call
 * to fd_alloc() or cookie_destroy(). Should the fd_alloc() fail,
 * then there will be another call to cookie_destroy().
 *
 * However, whether and when the second call happens, depends on
 * the behavior (and correctness) of the application programs.
 * The STREAMS framework can only ensure that cookie_destroy() gets
 * called when the associated message gets discarded.
 */

/*
 *	fd_to_cookie
 *
 *	Given a fd in the current process context (unchecked user
 *	parameter!), translate it into a file cookie. Regard this
 *	as an additional reference to this file, so that it does not
 *	get closed while we hold this reference.
 *
 *	Parameters:
 *	fd		(in)  - file descriptor to translate
 *	cookie		(out) - translated file descriptor
 *
 *	Return value          - error condition
 *				EBADF - invalid file descriptor
 */

int
fd_to_cookie (fd, cookie)
	int			fd;
	struct file_cookie *	cookie;
{
	int error;

	ENTER_FUNC(fd_to_cookie, fd, cookie, 0, 0, 0, 0);

        error = fp_getf(fd, &cookie->fc_fp);
	if (error == 0)
	{
		cookie->fc_magic = FILE_COOKIE_MAGIC;	/* validate cookie */
		fp_hold(cookie->fc_fp);
		ufdrele(fd);
	}

	LEAVE_FUNC(fd_to_cookie, error);
	return error;
}

/*
 *	fd_alloc
 *
 *	Given a cookie obtained from a call to fd_to_cookie (possibly
 *	in another process context) allocate a file descriptor and associate
 *	it with the cookie. The cookie does already represent a reference
 *	to that file.
 *
 *	If the allocation of the file descriptor is impossible (ENFILE),
 *	then DON'T destroy the cookie, but leave this to the dispostion
 *	of the caller. This restriction is necessary, since the caller
 *	might not be prepared to close a stream which is currently
 *	manipulating.
 *
 *	Parameters:
 *	cookie		(in)	- pointer to file cookie
 *	fdp		(out)	- pointer to file descriptor
 *
 *	Return value		- error condition
 *				  ENFILE - no file descriptor available
 */

int
fd_alloc (cookie, fdp)
	struct file_cookie *	cookie;
	int *			fdp;
{
	int	error;

	ENTER_FUNC(fd_alloc, cookie, fdp, 0, 0, 0, 0);

	if (cookie->fc_magic != FILE_COOKIE_MAGIC) {
		STR_DEBUG(printf("fd_alloc: invalid cookie %x\n", cookie->fc_magic));
		LEAVE_FUNC(fd_alloc, EBADF);
		return EBADF;
	}

        if ((error = ufdalloc(0, fdp)) == 0) {
		ufdsetf(*fdp, cookie->fc_fp);
	}

	LEAVE_FUNC(fd_alloc, error);
	return error;
}

/*
 *	cookie_destroy
 *
 *	Given a cookie obtained from a call to fd_to_cookie (possible
 *	in another process context), destroy it. (That is, decrement the
 *	reference count on the file structure.)
 *
 *	As a result of this, a close operation might be necessary. It is
 *	the caller's responsibility to provide an appropriate context
 *	for this. The actual action is taken by cookie_destroy. (The second
 *	is guaranteed not to happen for the thread which "just" allocated
 *	the cookie, and finds that it can't use it.)
 *
 *	Parameters:
 *	cookie		(in)	- pointer to file cookie
 *
 *	Return value:		- none.
 */

void
cookie_destroy (cookie)
	struct file_cookie *	cookie;
{

	ENTER_FUNC(cookie_destroy, cookie, 0, 0, 0, 0, 0);
	if (cookie->fc_magic != FILE_COOKIE_MAGIC) {
		STR_DEBUG(printf("cookie_destroy: invalid cookie %x\n", cookie->fc_magic));
		LEAVE_FUNC(cookie_destroy, -1);
		return;
	}
	(void) fp_close(cookie->fc_fp);
	LEAVE_FUNC(cookie_destroy, 0);
}

/*
 *	sth_fd_to_sth
 *
 *	Given an fd, translate that into a stream head.
 *	Must verify that fd references another stream.
 *	Should, but doesn't, take reference on file.
 *	See usage and comments in str_osr.c...
 */

int
sth_fd_to_sth (fd, sthpp)
	int		fd;
	STHPP		sthpp;
{
	struct file *	fp;
	int		error = 0;

	ENTER_FUNC(sth_fd_to_sth, fd, sthpp, 0, 0, 0, 0);

	if (!( *sthpp = (STHP)fdtosth(fd))) error = EINVAL;

      	LEAVE_FUNC(sth_fd_to_sth, error);
	return error;
}

/*
 * Keep fake inode, fattach list, create, access, and modify times for pipes.
 */

#include <sys/unpcb.h>
#ifdef UNPMISC_LOCK
#undef UNPMISC_LOCK
#undef UNPMISC_UNLOCK
#define UNPMISC_LOCK()
#define UNPMISC_UNLOCK()
#endif

ino_t   unp_vno;        /* prototype for fake vnode numbers */

struct sth_pipestat {
	ino_t	ino;
	time_t	ctime, atime, mtime;
	STHP	sth1, sth2;
	struct sth_fattach {
		struct sth_fattach *next, *prev;
		STHP	sth;
		void *	vnode;
	} *fnext, *fprev;
	decl_simple_lock_data(EMPTY_FIELD,lock)
};

int
sth_fattach (sth, flag, p)
	STHP	sth;
	int	flag;
	void *	p;
{
	struct vnode *vp = (struct vnode *)p;
	struct sth_pipestat *stb;
	struct sth_fattach *stf;

	ENTER_FUNC(sth_fattach, sth, flag, p, 0, 0, 0);

	if ((sth->sth_flags & F_STH_PIPE) == 0) {
		LEAVE_FUNC(sth_fattach, 0);
		return 0;
	}
	if ((stb = (struct sth_pipestat *)sth->sth_next) == 0) {
		sth_update_times(sth, FCREAT, (struct stat *)0);
		stb = (struct sth_pipestat *)sth->sth_next;
	}
	SIMPLE_LOCK(&stb->lock);
	if (flag) {	/* fattach() */
		if (!stb->sth1 || !stb->sth2) {
			/* Cannot re-fattach after forcible fdetach */
			SIMPLE_UNLOCK(&stb->lock);
			LEAVE_FUNC(sth_fattach, EINVAL);
			return EINVAL;
		}
		SIMPLE_UNLOCK(&stb->lock);
		NET_MALLOC(stf, struct sth_fattach *, sizeof *stf,
			M_STREAMS, M_NOWAIT);
		if (stf == 0) {
			LEAVE_FUNC(sth_fattach, ENOMEM);
			return ENOMEM;
		}
		stf->sth = sth;
		stf->vnode = vp;
		SIMPLE_LOCK(&stb->lock);
		insque(stf, stb->fprev);
		SIMPLE_UNLOCK(&stb->lock);
	} else {	/* fdetach() */
		for (stf = stb->fnext;
		     stf != (struct sth_fattach *)&stb->fnext;
		     stf = stf->next)
			if (stf->sth == sth && stf->vnode == vp) {
				remque(stf);
				break;
			}
		SIMPLE_UNLOCK(&stb->lock);
		if (stf == (struct sth_fattach *)&stb->fnext) {
			LEAVE_FUNC(sth_fattach, EINVAL);
			return EINVAL;
		}
		NET_FREE(stf, M_STREAMS);
	}
	LEAVE_FUNC(sth_fattach, 0);
	return 0;
}

void
sth_update_times (sth, flag, sb)
	STHP	sth;
	int	flag;
	struct stat *sb;
{
	STHP	sth2;
	queue_t *q;
	struct sth_pipestat *stb;
	struct sth_fattach *stf;
	struct timeval now;

	ENTER_FUNC(sth_update_times, sth, flag, sb, 0, 0, 0);
	if ((sth->sth_flags & F_STH_PIPE) == 0) {
		LEAVE_FUNC(sth_update_times, 0);
		return;
	}
	stb = (struct sth_pipestat *)sth->sth_next;
#if	MACH_ASSERT
	if ((stb && flag == FCREAT) || (!stb && flag != FCREAT))
		panic("sth_update_times");
#endif
	switch (flag) {
	default:
		SIMPLE_LOCK(&stb->lock);
		if (stb->ino == 0) {
			/* Use the same phony ino_t for streams and sockets */
			UNPMISC_LOCK();
			stb->ino = unp_vno++;
			UNPMISC_UNLOCK();
		}
		sb->st_ino = stb->ino;
		sb->st_atime = stb->atime;
		sb->st_mtime = stb->mtime;
		sb->st_ctime = stb->ctime;
		SIMPLE_UNLOCK(&stb->lock);
		break;
	case FREAD:
		microtime(&now);
		SIMPLE_LOCK(&stb->lock);
		stb->atime = now.tv_sec;
		SIMPLE_UNLOCK(&stb->lock);
		break;
	case FWRITE:
		microtime(&now);
		SIMPLE_LOCK(&stb->lock);
		stb->mtime = stb->ctime = now.tv_sec;
		SIMPLE_UNLOCK(&stb->lock);
		break;
	case FCREAT:
		for (q = sth->sth_wq; q->q_next; q = q->q_next)
			;
		sth2 = (STHP)q->q_ptr;
#if	MACH_ASSERT
		if (!sth2 || sth2->sth_next
		||  sth2->sth_rq->q_qinfo->qi_putp != sth_rput )
			panic("sth_update_times fattach");
#endif
		NET_MALLOC(stb, struct sth_pipestat *, sizeof *stb, \
				M_STREAMS, M_WAITOK);
		microtime(&now);
		simple_lock_init(&stb->lock);
		stb->atime = stb->mtime = stb->ctime = now.tv_sec;
		(stb->sth1 = sth)->sth_next = (STHP)stb;
		(stb->sth2 = (STHP)q->q_ptr)->sth_next = (STHP)stb;
		stb->fnext = stb->fprev = (struct sth_fattach *)&stb->fnext;
		break;
	case FSYNC:
		SIMPLE_LOCK(&stb->lock);
		if (stb->sth1 == sth)
			stb->sth1 = 0;
		if (stb->sth2 == sth)
			stb->sth2 = 0;
		sth->sth_next = 0;
		if (!stb->sth1 && !stb->sth2) {
			SIMPLE_UNLOCK(&stb->lock);
			NET_FREE(stb, M_STREAMS);
		} else
			SIMPLE_UNLOCK(&stb->lock);
		break;
	}
	LEAVE_FUNC(sth_update_times, 0);
}
