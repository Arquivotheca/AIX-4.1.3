static char sccsid[] = "@(#)93	1.18  src/bos/usr/ccs/lib/libc/closedir.c, libcio, bos411, 9428A410j 4/20/94 17:38:49";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: closedir 
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

#include <sys/limits.h>
#include <sys/types.h>
#include <dirent.h>	
#include <errno.h>	
#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#	include "rec_mutex.h"
#endif /* _THREAD_SAFE */

/*
 * close a directory.
 */
int
closedir(DIR *dirp)
{

#ifdef _THREAD_SAFE
	rec_mutex_t	lock;
#endif /* _THREAD_SAFE */
	/**********
	  if dirp is null, or already been closed, then
	  get out now, do not try to lock it.
	**********/
#ifdef _THREAD_SAFE
	if (dirp == NULL || dirp->dd_fd <0 || dirp->dd_lock==NULL) {
#else /* _THREAD_SAFE */
	if (dirp == NULL || dirp->dd_fd <0) {
#endif /* _THREAD_SAFE */
		errno = EBADF;
		return(-1);
	}
	TS_LOCK(dirp->dd_lock);

	if (dirp->dd_fd >= 0) {	/* dirp may be valid */
		int rc;
		TS_PUSH_CLNUP(dirp->dd_lock);
		rc = close(dirp->dd_fd);
		TS_POP_CLNUP(0);
		if (rc == -1 && errno != 0) {
			TS_UNLOCK(dirp->dd_lock);	
			/**********
			  errno is set by close
			**********/
			return(-1);
		}
		dirp->dd_fd = -1;
		dirp->dd_loc = 0;
		dirp->dd_size = 0; /* so readdir will fail */

#ifdef _THREAD_SAFE
		/**********
		  Save the lock before freeing the space
		**********/
		lock = (rec_mutex_t)dirp->dd_lock;
#endif /* _THREAD_SAFE */

		free(dirp->dd_buf);
		(void)free(dirp);
#ifdef _THREAD_SAFE
		/**********
		  Free up the lock
		**********/
		TS_UNLOCK(lock);
		_rec_mutex_free(lock);
#endif /* _THREAD_SAFE */
		return(0);
	}
	else {	/* dirp does not refer to an open directory stream */
		TS_UNLOCK(dirp->dd_lock);
		errno = EBADF;
		return(-1);
	}
}
