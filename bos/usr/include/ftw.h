/* @(#)08	1.13  src/bos/usr/include/ftw.h, libcgen, bos411, 9428A410j 5/27/94 13:26:38 */

/*
 * COMPONENT_NAME: LIBCGEN
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	Codes for the third argument to the user-supplied function
 *	which is passed as the second argument to ftw
 */
#ifndef _H_FTW
#define _H_FTW

#ifndef _H_STANDARDS
#include <standards.h>
#endif /* _H_STANDARDS */

#ifdef _XOPEN_SOURCE

#ifndef _H_STAT
#include <sys/stat.h>
#endif /* _H_STAT */

#define	FTW_F	0	/* file */
#define	FTW_D	1	/* directory */
#define	FTW_DNR	2	/* directory without read permission */
#define	FTW_NS	3	/* unknown type, stat failed */

#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE
#define FTW_SL	4	/* symlink points to existing file */
#define FTW_SLN	5	/* symlink points to non-existing file */
#define FTW_DP	6	/* directory with subdirs already visited */

/* For 4th Arg of nftw */
#define FTW_PHYS        0x00000001	/* Physical Walk, does not follow symlinks */
#define FTW_MOUNT       0x00000002	/* Do not cross mount points */
#define FTW_DEPTH       0x00000004	/* Visit all sub-dirs B4 the directory itself */
#define FTW_CHDIR       0x00000008	/* Change to each dir before reading it */

struct FTW  {
	int base;
	int level;
};

#endif /* _ALL_SOURCE */

#ifdef _XOPEN_SOURCE
#ifdef _NO_PROTO
extern int ftw();
#else /* _NO_PROTO */
extern int ftw(const char *, int (*)(const char *,const struct stat *, int),
	       int);
#endif /* _NO_PROTO */
#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE
#ifdef _NO_PROTO
extern int nftw();
#else /* _NO_PROTO */
extern int nftw(const char *, int (*)(const char *, const struct stat *, int, 
		struct FTW*), int, int);
#endif /* _NO_PROTO */
#endif /* _ALL_SOURCE */

#endif /* _H_FTW */
