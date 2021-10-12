static char sccsid[] = "@(#)11	1.15  src/bos/usr/ccs/lib/libc/readdir.c, libcio, bos411, 9438C411c 9/23/94 19:36:50";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: _readdir, readdir , readdir_r
 *
 * ORIGINS: 26,27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
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
#include <sys/errno.h>
#include <dirent.h>
#include "ts_supp.h"
#ifdef _THREAD_SAFE
#	include "rec_mutex.h"
#endif /* _THREAD_SAFE */

/*                                                                    
 * FUNCTION: get next entry in a directory.
 *
 * RETURN VALUE DESCRIPTION: Struct dirent * to next entry, else NULL.
 */  
/* The readdir() function includes all the POSIX requirements */

_readdir (dirp)
DIR	*dirp;
{
        extern int getdirent();

	dirp->dd_loc = 0;
	dirp->dd_size = getdirent (dirp->dd_fd, dirp->dd_buf, dirp->dd_blksize);
	return (dirp->dd_size);
}

/*
 * get next entry in a directory.
 */
struct dirent *readdir(DIR *dirp)
{
	register struct dirent *dp;

	if (dirp == NULL) { /* dirp does not refer to an open
			      directory stream */
		errno = EBADF;
		return (NULL);
	}
	TS_LOCK(dirp->dd_lock);
	while (1)
	{
		/* If at start of directory, reread it in case it has been
		   changed since last getdirent (BSD 4.3 behavior). */
		if (!dirp->dd_loc || dirp -> dd_loc >= dirp -> dd_size)
			if (_readdir (dirp) <= 0) {
				return (NULL);
			}

		dp = (struct dirent *) (dirp->dd_buf + dirp->dd_loc);
		dirp->dd_loc += dp->d_reclen;
		if (dp->d_ino == 0)
			continue;
		dirp->dd_curoff = dp->d_offset;
		return(dp);
	}
}

#ifdef _THREAD_SAFE
/*
 * get next entry in a directory. (THREAD_SAFE VERSION)
 */
int 
readdir_r(DIR *dirp, struct dirent *dirent, struct dirent **result)
{
	register struct dirent *dp;

	if (dirp == NULL) { /* dirp does not refer to an open
			      directory stream */
		errno = EBADF;
		return (EBADF);
	}
	TS_LOCK(dirp->dd_lock);
	while (1)
	{
		/* If at start of directory, reread it in case it has been
		   changed since last getdirent (BSD 4.3 behavior). */
		if (!dirp->dd_loc || dirp -> dd_loc >= dirp -> dd_size)
			if (_readdir (dirp) <= 0) {
				TS_UNLOCK(dirp->dd_lock);
				return (EBADF);
			}

		dp = (struct dirent *) (dirp->dd_buf + dirp->dd_loc);
		dirp->dd_loc += dp->d_reclen;
		if (dp->d_ino == 0)
			continue;
		dirp->dd_curoff = dp->d_offset;
		memcpy((void *)dirent, (void *)dp, sizeof(struct dirent));
		*result=dirent;
		TS_UNLOCK(dirp->dd_lock);
		return(0);
	}
}
#endif /* _THREAD_SAFE */
