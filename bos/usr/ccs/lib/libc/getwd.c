static char sccsid[] = "@(#)62  1.3  src/bos/usr/ccs/lib/libc/getwd.c, libcenv, bos411, 9428A410j 11/10/93 15:23:30";
/*
 *   COMPONENT_NAME: LIBCENV
 *
 *   FUNCTIONS: GETWDERR
 *		getwd
 *		prepend
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/fullstat.h>
#include <sys/dir.h>
#include <errno.h>

/*
 *
 * FUNCTION: returns the pathname of the current working directory. 
 *
 * RETURN VALUE DESCRIPTION: On error an error message is copied to pathname
 *	and a NULL pointer is returned.
 */
/*LINTLIBRARY*/

#include <nl_types.h>
#include "libc_msg.h"
#define GETWDERR(s,n)	cd=catopen(MF_LIBC, NL_CAT_LOCALE); \
			strcpy(pathname, catgets(cd,MS_LIBC,n,s));\
			catclose(cd);

static char *prepend();			/* prepend dirname to pathname */

char *
getwd(pathname)
	char *pathname;
{
	char pathbuf[PATH_MAX];		/* temporary pathname buffer */
	char *pnptr = &pathbuf[(sizeof pathbuf)-1]; /* pathname pointer */
	char curdir[PATH_MAX];		/* current directory buffer */
	char *dptr = curdir;		/* directory pointer */
	long cvfs, rvfs;		/* current & root vfs id */
	ino_t cino, rino;		/* current & root inode number */
	DIR *dirp;			/* directory stream */
	struct dirent *dir;		/* directory entry struct */
	struct fullstat d, dd;		/* file status struct */
	nl_catd cd;			/* message catalog descriptor */
	int pathsize;			/* pathname length */
#ifdef _THREAD_SAFE
	struct dirent dir_buf;		/* have to copy static buffer here */
	dir=&dir_buf;			/* make dir point to thread's copy */
#endif

	pathsize = 0;
	*pnptr = '\0';
	if (fullstat("/", FL_STAT, &d) < 0) {
		GETWDERR("getwd: can't fullstat /", M_RSTAT);
		return (NULL);
	}
	rvfs = d.fst_vfs;
	rino = d.st_ino;
	strcpy(dptr, "./");
	dptr += 2;
	if (fullstat(curdir, FL_STAT, &d) < 0) {
		GETWDERR("getwd: can't fullstat .", M_HSTAT);
		return (NULL);
	}
	for (;;) {
		if (d.st_ino == rino && d.fst_vfs == rvfs)
			break;		/* reached root directory */
		cino = d.st_ino;
		cvfs = d.fst_vfs;
		strcpy(dptr, "../");
		dptr += 3;
		if ((dirp = opendir(curdir)) == NULL) {
			GETWDERR("getwd: can't open ..", M_OPENPAR);
			return (NULL);
		}
		ffullstat(dirp->dd_fd, FL_STAT, &d);
		if (cvfs == d.fst_vfs) {
			if (cino == d.st_ino) {
				/* reached root directory */
				closedir(dirp);
				break;
			}
			do {
#ifdef _THREAD_SAFE
				if (readdir_r(dirp, &dir_buf, &dir) != 0) {
#else
				if ((dir = readdir(dirp)) == NULL) {
#endif
					closedir(dirp);
					GETWDERR("getwd: read error in ..",M_READPAR);
					/* readdir() needn't set errno for EOF 
					 */
					if (!errno)
						errno = ENOENT;
					return (NULL);
				}
			} while (dir->d_ino != cino);
		} else
			do {
#ifdef _THREAD_SAFE
				if (readdir_r(dirp, &dir_buf, &dir) != 0) {
#else
				if ((dir = readdir(dirp)) == NULL) {
#endif
					closedir(dirp);
					GETWDERR("getwd: read error in ..",M_READPAR);
					/* readdir() needn't set errno for EOF 
					 */
					if (!errno)
						errno = ENOENT;
					return (NULL);
				}
				strcpy(dptr, dir->d_name);
				fullstat(curdir, FL_STAT | FL_NOFOLLOW, &dd);
			} while(dd.st_ino != cino || dd.fst_vfs != cvfs);
		pnptr = prepend("/", prepend(dir->d_name, pnptr, &pathsize), &pathsize);
		closedir(dirp);
	}
	if (*pnptr == '\0')		/* current dir == root dir */
		strcpy(pathname, "/");
	else
		strcpy(pathname, pnptr);
	return (pathname);
}

/*
 * prepend() tacks a directory name onto the front of a pathname.
 */
static char *
prepend(dirname, pathname, pathsize)
	register char *dirname;
	register char *pathname;
	register int  *pathsize;
{
	register int i;			/* directory name size counter */

	for (i = 0; *dirname != '\0'; i++, dirname++)
		continue;
	if ((*pathsize += i) < PATH_MAX)
		while (i-- > 0)
			*--pathname = *--dirname;
	return (pathname);
}
