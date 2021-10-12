static char sccsid[] = "@(#)64	1.15  src/bos/kernel/lfs/fd.c, syslfs, bos41B, 412_41B_sync 12/6/94 12:15:24";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: ufdgetf, ufdsetf, ufdhold, ufdrele, getufdflags, setufdflags,
 *	      ufdalloc, ufdfree, ufdcreate
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
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
#include "sys/vfs.h"
#include "sys/acct.h"
#include "sys/sysinfo.h"
#include "sys/syspest.h"
#include "sys/user.h"
#include "sys/lock_def.h"

/*
 * NAME: ufdgetf
 *
 * FUNCTION: Returns the pointer to the file structure associated
 *	     with the file descriptor fd in the location specified
 *	     by fpp.
 *
 * RETURNS:  Possible ERRNO values:
 *              EBADF   The fd argument is not a file descriptor for an
 *                      open file.
 *
 * SERIALIZATION: The caller of this routine must have called ufdhold()
 *		  to place a reference count on the file descriptor
 *		  so the file could not be closed, and to check if the
 *		  the file descriptor is not in the process of being
 *		  closed.
 *
 */

int
ufdgetf(int fd,				/* file descriptor */
	struct file **fpp)		/* file pointer returned */
{
	register struct file	*fp;


	if (0 <= fd && fd < U.U_maxofile)
	{
		/* Make sure that the user application did a ufdhold */
		assert(U.U_ufd[fd].count > 0);

		if ((fp = U.U_ufd[fd].fp) != NULL)
		{
			/* fd is now known to be valid */

			if (!(fp->f_flag & FREVOKED))
			{
				*fpp = fp;
				return 0;
			}
		}
	}
	return EBADF;
}

/*
 * NAME: ufdsetf
 *
 * FUNCTION: Stores the file pointer into the file descriptor passed in.
 *
 * RETURNS: NONE
 *
 * SERIALIZATION: The caller of this routine must have called ufdalloc().
 *		  
 *
 */

void
ufdsetf(int fd,				/* file descriptor */
	struct file *fp)		/* file pointer to store */
{
	int tlock;	/* is multi-thread locking required? */

	if (tlock = (U.U_procp->p_active > 1))
		U_FD_LOCK();
	ASSERT(0 <= fd && fd < U.U_maxofile);
	ASSERT(U.U_ufd[fd].count == 0);
	ASSERT(U.U_ufd[fd].fp == NULL);
	ASSERT(U.U_ufd[fd].flags & UF_ALLOCATED);
	U.U_ufd[fd].fp = fp;
	U.U_ufd[fd].flags = UF_ALLOCATED;
	if (tlock)
		U_FD_UNLOCK();
}

/*
 * NAME: ufdhold
 *
 * FUNCTION: Increments the file descriptor reference count.
 *
 * RETURNS:  Possible ERRNO values:
 *              EBADF   The fd argument is not a file descriptor for
 *			an opened file.
 *
 * SERIALIZATION: Take the U_FD_LOCK when incrementing the count
 *		  on the file descriptor.
 *
 */

int
ufdhold(int fd)				/* file descriptor */
{
	int tlock;	/* is multi-thread locking required? */
	int rc;

	if (tlock = (U.U_procp->p_active > 1))
		U_FD_LOCK();

	rc = EBADF;
	if (0 <= fd && fd < U.U_maxofile)
	{
		if (!(U.U_ufd[fd].flags & UF_CLOSING))
		{
			U.U_ufd[fd].count++;
			rc = 0;
		}
	}

	if (tlock)
		U_FD_UNLOCK();
	return rc;
}	

/*
 * NAME: ufdrele
 *
 * FUNCTION: Decrements the file descriptor reference count.
 *
 * RETURNS:  Possible ERRNO values:
 *              EBADF   The fd argument is not a file descriptor for
 *			an opened file.
 *
 * SERIALIZATION: Take the U_FD_LOCK when decrementing the count
 *		  on the file descriptor.
 */

int
ufdrele(int fd)				/* file descriptor */
{
	int tlock;	/* is multi-thread locking required? */
	int rc;

	if (tlock = (U.U_procp->p_active > 1))
		U_FD_LOCK();

	if (0 <= fd && fd < U.U_maxofile)
	{
		assert(U.U_ufd[fd].count > 0);

		U.U_ufd[fd].count--;
		if (U.U_ufd[fd].count == 0 && (U.U_ufd[fd].flags & UF_CLOSING))
		{
			/* wakeup any sleepers in the list */
			e_wakeup(&U.U_fdevent);
		}
		rc = 0;
	}
	else
		rc = EBADF;

	if (tlock)
		U_FD_UNLOCK();
	return rc;
}

/*
 * NAME: getufdflags
 *
 * FUNCTION: Gets the file descriptor flags for the specified file descriptor.
 *
 * RETURNS:  Possible ERRNO values:
 *              EBADF	The fd argument is not a file descriptor for an open
 *			file.
 *
 */

int 
getufdflags(int fd,			/* file descriptor */
	    int * flagsp)		/* pointer to flags for fd */
{

	if (0 <= fd && fd < U.U_maxofile)
	{
		/* assert that our file descriptor is valid */
		assert(U.U_ufd[fd].count != 0);

		if (U.U_ufd[fd].fp)
		{
			*flagsp = U.U_ufd[fd].flags;
			return 0;
		}
	}
	return EBADF;
}
	
/*
 * NAME: setufdflags
 *
 * FUNCTION: Sets the file descriptor flags for the specified file descriptor.
 *	     The only valid flag to set is the FD_CLOEXEC. This function
 *	     will mask off this flag and return.
 *
 * RETURNS:  Possible ERRNO values:
 *              EBADF	The fd argument is not a file descriptor for an open
 *			file.
 *
 * SERIALIZATION: Take the U_FD_LOCK when setting the flags field in
 *		  the file descriptor.
 */

int 
setufdflags(int fd,			/* file descriptor */
	    int flags)			/* pointer to flags for fd */
{
	unsigned short *flagp;

	if (0 <= fd && fd < U.U_maxofile)
	{
		/* assert that our file descriptor is valid */
		assert(U.U_ufd[fd].count != 0);

		if (U.U_ufd[fd].fp)
		{
			int tlock;  /* is multi-thread locking required? */

			if (tlock = (U.U_procp->p_active > 1))
				U_FD_LOCK();
			flagp = &U.U_ufd[fd].flags;
			*flagp = (*flagp & ~FD_CLOEXEC) | (flags & FD_CLOEXEC);
			if (tlock)
				U_FD_UNLOCK();
			return 0;
		}
	}
	return EBADF;
}

/*
 * NAME: ufdalloc()
 *
 * FUNCTION: Allocates a user file descriptor. Allocated file descriptor
 * 	     returned in *fdp.
 *
 * PARAMETERS:	i	first file descriptor to look at.
 *		fdp	pointer to returned file descriptor.
 *
 * RETURN VALUES: 0 if successful.
 *
 * SERIALIZATION: The file descriptor table is serialized by the U_FD_LOCK.
 *		  This lock must be held for the duration of the table search
 *		  and the flags of the file descriptor checked to see if
 *		  the file descriptor is already opened.
 */
int
ufdalloc(i, fdp)
int	i;		/* first file descriptor to look at	*/
int *	fdp;		/* ptr to returned descriptor		*/
{
	int tlock;	/* is multi-thread locking required?    */

	if (tlock = (U.U_procp->p_active > 1))
		U_FD_LOCK();
	for ( ; i < OPEN_MAX; i++)
	{
		if (i >= U.U_maxofile || !(U.U_ufd[i].flags & UF_ALLOCATED))
		{
			U.U_ufd[i].fp = NULL;
			U.U_ufd[i].count = 0;
			U.U_ufd[i].flags = UF_ALLOCATED;
			if (i >= U.U_maxofile)
			{
				int j;
				for (j = U.U_maxofile; j < i; j++)
				{
					U.U_ufd[j].fp = NULL;
					U.U_ufd[j].count = 0;
					U.U_ufd[j].flags = 0;
				}
				U.U_maxofile = i + 1;
			}
			*fdp = i;
			break;
		}
	}
	if (tlock)
		U_FD_UNLOCK();
	return (i < OPEN_MAX) ? 0 : EMFILE;
}

/*
 * NAME: ufdfree()
 *
 * FUNCTION: free a user file discriptor, no close action
 *
 * PARAMETERS: fd	file descriptor to close
 *
 * RETURN VALUES: Returns any error codes that may occur.
 *
 * SERIALIZATION:  Search of the file descriptor table is serialized
 *		   by the U_FD_LOCK.
 *
 */
void
ufdfree(fd)
int	fd;
{
	int tlock;		/* is multi-thread locking required? */
	register int i;

	if (tlock = (U.U_procp->p_active > 1))
		U_FD_LOCK();
	U.U_ufd[fd].fp = NULL;
	U.U_ufd[fd].flags = 0;
	if (fd == U.U_maxofile - 1)
	{
		/* search backward for last used file descriptor slot */
		i = U.U_maxofile - 2;
		while (i >= 0)
		{
			if (U.U_ufd[i].flags & UF_ALLOCATED)
				break;
			i--;
		}
		U.U_maxofile = i + 1;
	}
	if (tlock)
		U_FD_UNLOCK();
}

/*
 * NAME: ufdcreate
 *
 * FUNCTION: Creates a file descriptor and file struct pair
 * 	     without requiring the extension to understand or
 *	     conform to the synchronization of the LFS. This service
 *	     provides a file descriptor to the caller and creates
 *	     the underlying file struct without exposing the internal
 *	     ordering and locking that is needed.
 *
 * RETURNS:  Possible ERRNO values:
 *	     EINVAL  ops argument is null or the struct pointed to
 *		     does not have entries for every op.
 *	     EMFILE  All file descriptors for the process have already
 *		     been allocated.
 *	     ENFILE  The system file table is full.
 *	     ZERO    Upon success with the address to the File
 *		     Descriptor in fdp.
 *
 */

int
ufdcreate( flags, ops, datap, type, fdp, crp )
int			flags;		/* flags to be saved in file struct */
struct fileops *	ops;		/* list of file op routines         */
void *			datap;		/* address of dependent structures  */
short			type;		/* unique type value for file struc */
int *			fdp;		/* address of file descriptor       */
struct ucred *		crp;		/* creds to store in file struct    */
{ 
	struct file	*newfp = NULL;	/* newly created file pointer       */ 
	int		newfd = 0;	/* newly created file descriptor    */
	int             rc = 0;         /* return code                      */

	/* Check the fileops pointer */
	if (!ops)
		return EINVAL;

	/* Make sure all fileops are present */
	if ( !ops->fo_rw || !ops->fo_ioctl || !ops->fo_select ||
	     !ops->fo_close || !ops->fo_fstat )
		return EINVAL;

	/* Allocate a file descriptor */
	if (rc = ufdalloc(0, &newfd))
		return rc;

	/* Allocate a file pointer */
	if (!(rc = fpalloc(datap, flags, type, ops, &newfp))) {
		/* Hold the creds and save in the file struct */
		crhold(crp);
		newfp->f_cred = crp;

		/* 
		 * Attach newly created file pointer to file descriptor
		 */
		U.U_ufd[newfd].fp = newfp;

		/* Set the contents of fdp to the new file descriptor */
		*fdp = newfd;
	}
	else {
		/* Free the new file descriptor */
		ufdfree(newfd);
	}

	return rc;
}
