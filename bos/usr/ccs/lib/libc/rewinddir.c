static char sccsid[] = "@(#)46	1.3  src/bos/usr/ccs/lib/libc/rewinddir.c, libcio, bos411, 9428A410j 10/20/93 14:31:25";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: rewinddir
 *
 *   ORIGINS: 26,27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */
/* rewinddir.c,v $ $Revision: 1.9 $ (OSF) */

/*
 * FUNCTION:
 *	resets the positiion of the directory stream to which 'dirp' refers
 *	to the beginning of the directory.
 *
 * RETURNS:
 *	rewinddir() does not return a value.
 *
 */

#include <sys/param.h>
#include <dirent.h>
#include <sys/file.h>
#include <unistd.h>
#include "ts_supp.h"
#ifdef _THREAD_SAFE
#include "rec_mutex.h"
#endif

#ifdef rewinddir
#undef rewinddir
#endif

/*
 * Since seekdir is unsafe, we need an explicit routine for rewinddir
 */
void
rewinddir(DIR *dirp)

{
	long curloc, base, offset;
	struct direct *dp;
	extern long lseek();

	if (dirp == NULL || dirp->dd_buf == NULL)
		return;
	TS_LOCK(dirp->dd_lock);
	dirp->dd_loc = 0;
	(void) lseek(dirp->dd_fd, (off_t)0, SEEK_SET);
	TS_UNLOCK(dirp->dd_lock);
}
