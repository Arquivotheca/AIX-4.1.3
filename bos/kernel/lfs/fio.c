static char sccsid[] = "@(#)42	1.39.1.10  src/bos/kernel/lfs/fio.c, syslfs, bos411, 9428A410j 5/16/94 13:11:41";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: getf, getfx, _getf, getft, finit, fpalloc, 
 *	      falloc, fpfree, fpgrow, ffree, fp_ufalloc, 
 *	      fp_getf, fp_hold
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

#include "sys/param.h"
#include "sys/limits.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/filsys.h"
#include "sys/fs_locks.h"
#include "sys/file.h"
#include "sys/conf.h"
#include "sys/inode.h"
#include "sys/var.h"
#include "sys/acct.h"
#include "sys/sysinfo.h"
#include "sys/syspest.h"
#include "sys/lock_alloc.h"
#include "sys/lockname.h"
struct file *ffreelist;

extern nfile_min, nfile_max;

static
int
_getf (		int		fd,
		struct file **	fpp,
		int		revokeflag);
static
int
_getf_1thread (	int		fd,
		struct file **	fpp,
		int		revokeflag);
static
int
fpgrow (	void);

/*
 * Convert a user supplied file descriptor into a pointer
 * to a file structure.  Kill the process if access to the
 * file has been revoked (ie. FREVOKED is set).
 */
int
getf(fd, fpp)
register int	fd;
struct file	**fpp;
{
	if (U.U_procp->p_active == 1)
		return _getf_1thread(fd, fpp, 1);
	else
		return _getf(fd, fpp, 1);
}

/*
 * Convert a user supplied file descriptor into a pointer
 * to a file structure.  It does not verify FREVOKED.
 * This is used by close() and exit().
 */
int
getfx(fd, fpp)
register int	fd;
struct file	**fpp;
{
	if (U.U_procp->p_active == 1)
		return _getf_1thread(fd, fpp, 0);
	else
		return _getf(fd, fpp, 0);
}

/* Common handler for getf and getfx -- single-threaded.
 * This routine exists to optimize performance in the
 * single threaded case.  It is a leaf routine, so it
 * does not require a stack frame. 
 */
static
int
_getf_1thread(
register int	fd,
struct file	**fpp,
int		revokeflag)
{
	register struct file	*fp;

	if (0 <= fd && fd < U.U_maxofile)
	{
		/* can't be CLOSING because process is single threaded */

		if ((fp = U.U_ufd[fd].fp) != NULL)
		{
			/* fd is now known to be valid */

			if ( !((fp->f_flag & FREVOKED) && revokeflag) )
			{
				*fpp = fp;
				U.U_ufd[fd].count++;
				return 0;
			}
		}
	}
	return EBADF;
}

/* Common handler for getf and getfx. */
static
int
_getf(
register int	fd,
struct file	**fpp,
int		revokeflag)
{
	register struct file	*fp;

	U_FD_LOCK();
	if (0 <= fd && fd < U.U_maxofile)
	{
		if (!(U.U_ufd[fd].flags & UF_CLOSING))
		{
			if ((fp = U.U_ufd[fd].fp) != NULL)
			{
				/* fd is now known to be valid */

				if ( !((fp->f_flag & FREVOKED) && revokeflag) )
				{
					*fpp = fp;
					U.U_ufd[fd].count++;
					U_FD_UNLOCK();
					return 0;
				}
			}
		}
	}
	U_FD_UNLOCK();
	return EBADF;
}

/* allocate and return the file pointer for a given file descriptor	*/
int
fp_getf(fd, fpp)
int		fd;		/* file descriptor of file pointer	*/
struct file	**fpp;		/* file pointer return address		*/
{
	if (U.U_procp->p_active == 1)
		return _getf_1thread(fd, fpp, 1);
	else
		return _getf(fd, fpp, 1);
}

/*
* getft - Get the file pointer referred to by a file descriptor and
* validate the type.
*/
int
getft(fd, fpp, want)
int		fd;
struct file	**fpp;
int		want;
{
	int	rc;
	struct file *fp;

	if (U.U_procp->p_active == 1)
		rc = _getf_1thread(fd, &fp, 1);
	else
		rc = _getf(fd, &fp, 1);

	if (rc == 0)
		if (fp->f_type != want)
		{
			ufdrele(fd);
			rc = EINVAL;
		}
		else
			*fpp = fp;

	return rc;
}

void
finit()
{
	register struct file	*fp;

	/* Initialize the file free list lock */
	lock_alloc(&ffree_lock,LOCK_ALLOC_PAGED,FFREE_LOCK_CLASS,-1);
	simple_lock_init(&ffree_lock);

	v.v_file = 0;
	v.ve_file = (caddr_t) &file[0];

	fpgrow();

	U.U_cmask = CMASK;			/* file creation mask */
	U.U_maxofile = 0;			/* initialize max open files */
	U.U_compatibility = PROC_RAWDIR;        /* default to raw directories */
}

/*
* fpalloc	Allocate a file table entry.  Return 0 if success,
*		else return the error number.  The allocated file
*		pointer is returned via fpp.
*/
int
fpalloc(vp, flag, type, ops, fpp)
struct vnode *		vp;	/* pointer to associated vnode	*/
int			flag;	/* new value for f_flag field	*/
int			type;	/* type of file struct		*/
struct fileops *	ops;	/* pointer to file op structure	*/
struct file **		fpp;	/* pointer to rtn'd file pointer*/
{
	register struct file *fp;

	FFREE_LOCK();
	while ((fp = ffreelist) == NULL)
		if (!fpgrow())
		{
			FFREE_UNLOCK();
			return ENFILE;	/* exhausted system open file table */
		}
	ffreelist = fp->f_next;
	FFREE_UNLOCK();

	ASSERT(fp->f_count == 0);

	fp->f_count = 1;
	fp->f_vnode = vp;
	fp->f_flag = flag;
	fp->f_type = type;
	fp->f_ops = ops;
	fp->f_offset = 0;
	fp->f_dir_off = 0;
	fp->f_cred = NULL;
	fp->f_vinfo = NULL;

	*fpp = fp;
	return 0;
}

fpfree(fp)
struct file	*fp;
{
	ASSERT(fp->f_count == 0);

	FFREE_LOCK();
	fp->f_next = ffreelist;
	ffreelist = fp;
	FFREE_UNLOCK();
}

/*
 * NAME: fp_ufalloc
 *
 * FUNCTION: copy a file table entry.  Allocate
 *      a user file descriptor that refers to
 *	the new file pointer.  This is used by ptrace.
 *      New file table entry is seeked to zero and has
 *      FEXEC turned off.
 *
 * RETURNS:	newly allocated file descriptor OR
 *		-1 with u_error set if an error occurred.
 */
int
fp_ufalloc(fp)
register struct file	*fp;
{
	int		newfd = 0;
	struct file	*newfp = NULL;
	short		flag;
	int		rc;
	int             klock;

	/* Note: this routine assumes that the fp that is passed in
	 * is held and cannot be freed while this routine is executing.
	 */

	if ((klock = IS_LOCKED(&kernel_lock)) != 0)
		unlockl(&kernel_lock);

	ASSERT(fp != NULL);
	ASSERT(fp->f_count > 0);
	ASSERT(fp->f_type == DTYPE_VNODE);

	if (rc = ufdalloc(0, &newfd))
		goto out;

	/* if FEXEC is on we turn it off and turn on FREAD */
	flag = fp->f_flag;
	if (flag & FEXEC)
	{
		flag |= FREAD;
		flag &= ~FEXEC;
	}

	if (rc = fpalloc(fp->f_vnode, flag, fp->f_type, fp->f_ops, &newfp))
		goto out;

	rc = VNOP_OPEN(fp->f_vnode, flag, NULL, &(newfp->f_vinfo), fp->f_cred);

out:
	if (!rc)
	{
		/* newfp->f_vnode is the same as fp->f_vnode */
		VNOP_HOLD(newfp->f_vnode);	
		/* hold the cred struct and fill in the cred pointer */
		crhold(fp->f_cred);
		newfp->f_cred = fp->f_cred;
		/* 
		 * Don't need to take the u_fd_lock for the following
		 * assignment since no one can use this file pointer 
		 */
		U.U_ufd[newfd].fp = newfp;
	}
	else
	{
		if (newfd)
			ufdfree(newfd);
		if (newfp)
		{
			newfp->f_count = 0;
			fpfree(newfp);
		}
	}
	if (klock)
		lockl(&kernel_lock, LOCK_SHORT);

	return (rc ? -1 : newfd);
}

void
fp_hold(fp)
struct file	*fp;
{
	ASSERT(fp != NULL);
	ASSERT(fp->f_count > 0);

	FP_LOCK(fp);
	fp->f_count++;
	FP_UNLOCK(fp);

	return;
}

/*
* fpgrow - grow the system open file table by a page.  Return TRUE
* if the growth was successful, else return FALSE.
*/
static
int
fpgrow(void)
{
	struct file *fp;
	int i;
	int grow;
	static int slotno = 1;

	/* ffree_lock must be held on entry to this routine */

	/*
	 * check for full file table
	 * return
	 */
	if (v.v_file >= nfile_max)
		return 0;

	/*
	 * Try to grow the file table to the next page boundary.
	 * If there is not room to grow by a full page then calculate
	 * the growth to fill the file table.
	 */
	fp = ((int)v.ve_file + PAGESIZE) & ~(PAGESIZE-1);
	grow = (int)((char *)fp - v.ve_file) / sizeof(struct file);
	if (grow <= 1)
		grow += PAGESIZE / sizeof(struct file);
	if (v.v_file + grow > nfile_max)
	{
		grow = nfile_max - v.v_file;
		if (grow == 0)
			return 0;
	}

	/*
	 * link the newly allocated files
	 */
	fp = (struct file *)v.ve_file;
	for (i = 0; i < grow; i++)
	{
		fp[i].f_next = &fp[i+1];
		lock_alloc(&fp[i].f_lock,
			   LOCK_ALLOC_PAGED, FPTR_LOCK_CLASS, slotno);
		simple_lock_init(&fp[i].f_lock);
		lock_alloc(&fp[i].f_offset_lock,
			   LOCK_ALLOC_PAGED, FOFF_LOCK_CLASS, slotno);
		simple_lock_init(&fp[i].f_offset_lock);
		slotno++;
	}
	fp[grow-1].f_next = NULL;

	v.v_file += grow;
	v.ve_file = (caddr_t) &fp[grow];
	ffreelist = fp;

	return 1;
}

/*
 * free a file table entry
 */

void
ffree(fp)
struct file	*fp;
{
	/*
	 * Don't need a lock for this assignment since I am
	 * simply zero'ing the value out.
	 */
	fp->f_count = 0;
	fpfree(fp);
}


