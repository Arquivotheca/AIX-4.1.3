static char sccsid[] = "@(#)45	1.13.1.3  src/bos/usr/ccs/lib/libc/opendir.c, libcio, bos411, 9428A410j 10/20/93 14:30:24";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: opendir 
 *
 * ORIGINS: 26,27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/vfs.h>
#include <sys/statfs.h>
#include <sys/errno.h>
#include <malloc.h>
#include <sys/types.h>			/* added for POSIX (see below) */
#include <unistd.h>
#include <fcntl.h>
#ifdef _THREAD_SAFE
#	include "rec_mutex.h"
#endif /* _THREAD_SAFE */

/*
 * open a directory.
 */

DIR *
opendir(const char *name)
{
	register 	DIR 	*dirp;
	register 	int 	fd;
	int		blksize;
	struct		statfs	fsbuf;
	int		fdflags;	/* the file descriptor flags */
	struct 		stat 	sb;

	if (stat(name, &sb) == -1)
		return (NULL);
	if ((sb.st_mode & S_IFMT) != S_IFDIR) {
		errno = ENOTDIR;
		return (NULL);
	}
	if ((fd = open(name, 0)) == -1)
		return (NULL);

	if (fstatfs(fd, &fsbuf) == -1) {
		(void) close (fd);
		return(NULL);
	}
	blksize = fsbuf.f_bsize;

        if (
		((dirp = (DIR *)malloc((size_t)sizeof(DIR))) == NULL) ||
		((dirp->dd_buf = malloc((size_t)blksize+1)) == NULL)
	   )
	{
                if (dirp)
                        free((void *)dirp);
		close (fd);
		return NULL;
	}


#ifdef _THREAD_SAFE
	/**********
	  get the space for the lock
	**********/
	if (_rec_mutex_alloc((rec_mutex_t *)&dirp->dd_lock) < 0) {
		(void)free((char *)dirp->dd_buf);
		(void)free((char *)dirp);
		close(fd);
		return(NULL);
	}
#endif /* _THREAD_SAFE */

	dirp->dd_fd = fd;
        dirp->dd_flag = 0;
        dirp->dd_blksize = blksize;
        dirp->dd_curoff = 0;
	dirp->dd_loc = 0;

	/*
	 * since the NFS opendir() doesn't actually access the remote dir,
	 * we need to try a readdir() to detect permission errors etc.
	 * NB that we can get a NULL from readdir() on an empty dir
	 * (from a non-unix system) but still have no error (errno == 0).
	 */

	if (readdir(dirp) == NULL && errno)
	{
		int	err;

		err = errno;
		closedir (dirp);
		errno = err;
		return (NULL);
	}
	rewinddir(dirp);

	/*
	** POSIX 1003.3 5.1.2.2 04(C) defines that if opendir() is
	** implemented using a file descriptor, then the FD_CLOEXEC
	** flag shall be set for that file descriptor.
	** in english that means turn on the "close on exec" flag
	** via an fcntl()
	*/

	fdflags = fcntl( fd, F_GETFD );
	fdflags |= FD_CLOEXEC;
	fcntl( fd, F_SETFD, fdflags );

	return (dirp);
}
