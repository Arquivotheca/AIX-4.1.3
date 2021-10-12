/* @(#)68	1.38.1.8  src/bos/kernel/sys/dir.h, syslfs, bos411, 9428A410j 7/8/94 10:50:18 */
#ifdef _POWER_PROLOG_
/*
 * COMPONENT_NAME:  (SYSLFS) logical filesystem
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3,24,26,27,71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#endif /* _POWER_PROLOG_ */

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
#ifdef	_SUN
/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */
#endif	/* _SUN */

#ifndef _H_DIR
#define _H_DIR

#include <standards.h>

#include <sys/types.h>

#ifdef _POSIX_SOURCE
/*
 * The POSIX standard way of returning directory entries is in directory entry
 * structures, which are of variable length.  Each directory entry has
 * a struct direct at the front of it, containing its inode number,
 * the length of the entry, and the length of the name contained in
 * the entry.  These are followed by the name padded to a 4 byte boundary
 * with null bytes.  All names are guaranteed null terminated.
 * The maximum length of a name in a directory is _D_NAME_MAX
 */
#define _D_NAME_MAX 255

struct	dirent {
	ulong_t		d_offset;	/* real off after this entry */
	ino_t		d_ino;		/* inode number of entry */
					/* make ino_t when it's ulong */
	ushort_t	d_reclen;	/* length of this record */
	ushort_t	d_namlen;	/* length of string in d_name */
	char	        d_name[_D_NAME_MAX+1];	/* name must be no longer than this */
					/* redefine w/#define when name decided */
};

#ifndef _KERNEL

/*
 * Definitions for library routines operating on directories.
 */
typedef struct _dirdesc {
#ifdef _ALL_SOURCE
	int	dd_fd;		/* file descriptor of directory */
	int	dd_blksize;	/* this filesystem's block size */
	char	*dd_buf;	/* malloc'd buffer depending of fs bsize */
	long	dd_size;	/* size of buffer */
	long	dd_flag;	/* private flags for readdir, unused */
	long	dd_loc;		/* logical(dirent) offset in  directory */
	long	dd_curoff;	/* real offset in directory corresponding
				 * to dd_loc */
#else
	int	__dd_fd;		/* file descriptor of directory */
	int	__dd_blksize;	/* this filesystem's block size */
	char	*__dd_buf;	/* malloc'd buffer depending of fs bsize */
	long	__dd_size;	/* size of buffer */
	long	__dd_flag;	/* private flags for readdir, unused */
	long	__dd_loc;	/* logical(dirent) offset in  directory */
	long	__dd_curoff;	/* real offset in directory corresponding
				 * to dd_loc */
#endif
#ifdef _THREAD_SAFE
	void	*dd_lock;	/* for inter-thread locking */
#endif

} DIR;


#ifdef _NO_PROTO
extern	DIR *opendir();
extern	struct dirent *readdir();
#ifdef	_THREAD_SAFE
extern int readdir_r();
#endif
extern	int closedir();
extern  void rewinddir();

#else /* _NO_PROTO */

extern  DIR *opendir(const char *);
extern  struct dirent *readdir(DIR *);

#ifdef	_THREAD_SAFE

/* See comments in stdlib.h on _AIX32_THREADS */
#ifdef	_AIX32_THREADS
extern int readdir_r(DIR *dirp, struct dirent *entry);
#else	/* POSIX 1003.4a Draft 7 prototype */
extern int readdir_r(DIR *, struct dirent *, struct dirent **);
#endif	/* _AIX32_THREADS */

#endif	/* _THREAD_SAFE */

extern  int closedir(DIR *);
extern  void rewinddir(DIR *);

#endif /* _NO_PROTO */
#endif /* _KERNEL */

#endif /* _POSIX_SOURCE */

#ifdef _XOPEN_SOURCE
#ifndef _KERNEL
#ifdef _NO_PROTO 

extern	long telldir();
extern	void seekdir();

#else /* _NO_PROTO */

extern void seekdir(DIR *, long);
extern long telldir(DIR *);

#endif /* _NO_PROTO */
#endif /* _KERNEL */
#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE

#ifdef _NO_PROTO 

extern int scandir(); 
extern int alphasort();

#else /* _NO_PROTO */

extern int scandir(const char *, struct dirent ***, int (*)(), int (*)()); 
extern int alphasort(struct dirent **, struct dirent **);

#endif /* _NO_PROTO */
 

#include <sys/lkup.h>			/* for lookup flags */
/* <limits.h> has historically been included in <sys/dir.h>.  However,
   it is not allowed by Posix.1.  The include must remain in _ALL_SOURCE.
 */
#include <limits.h>

#define rewinddir(__dirp)	seekdir((__dirp), (long)0)

#define NO_STAT		0
#define STAT		1

/* For BSD compatibility */
#define MAXNAMLEN	255

/*
 * A directory consists of some number of blocks of DIRBLKSIZ
 * bytes, where DIRBLKSIZ is chosen such that it can be transferred
 * to disk in a single atomic operation (e.g. 512 bytes on most machines).
 *
 * Each DIRBLKSIZ byte block contains some number of directory entry
 * structures, which are of variable length.  Each directory entry has
 * a struct direct at the front of it, containing its inode number,
 * the length of the entry, and the length of the name contained in
 * the entry.  These are followed by the name padded to a 4 byte boundary
 * with null bytes.  All names are guaranteed null terminated.
 * The maximum length of a name in a directory is _D_NAME_MAX.
 *
 * The macro DIRSIZ(dp) gives the amount of space required to represent
 * a directory entry.  Free space in a directory is represented by
 * entries which have dp->d_reclen > DIRSIZ(dp).  All DIRBLKSIZ bytes
 * in a directory block are claimed by the directory entries.  This
 * usually results in the last entry in a directory having a large
 * dp->d_reclen.  When entries are deleted from a directory, the
 * space is returned to the previous entry in the same directory
 * block by increasing its dp->d_reclen.  If the first entry of
 * a directory block is free, then its dp->d_ino is set to 0.
 * Entries other than the first in a directory do not normally have
 * dp->d_ino set to 0.
 */
#if !defined(DEV_BSIZE)
#define	DEV_BSIZE	512
#endif

#define	DIRBLKSIZ	512

/*
 * The DIRSIZ macro gives the minimum record length which will hold
 * the directory entry.  This requires the amount of space in struct dirent
 * without the d_name field, plus enough space for the name with a terminating
 * null byte (dp->d_namlen+1), rounded up to a 4 byte boundary.
 */
#undef DIRSIZ
#define DIRSIZ(__dp) \
    ((sizeof (struct dirent) - (_D_NAME_MAX+1)) + (((__dp)->d_namlen+1 + 3) &~ 3))

#ifdef _KERNEL
# include	"jfs/dir.h"
#else /* _KERNEL */

#ifdef _BSD
#define direct dirent
#else /* _BSD */

/* This structure is defined ONLY to ease porting of System V applications.
   It is NOT advisable to use it and the read() system call to read 
   directories.  Rather the readdir() system call should be used since it
   will work with all supported file systems and file name lengths.
   To maintain compatiblilty with Sys V, the d_ino field is hardcoded to 
   a short (not an ino_t).  The d_name field must be AIX_DIRSIZ (14) bytes long.
*/

/* to compensate for the sys V DIRSIZ */
#define AIX_DIRSIZ 14
struct	direct {
	ushort_t d_ino;			/* inode number of entry */
	char	d_name[AIX_DIRSIZ];	/* name must be no longer than this */
};
#endif /* _BSD */

#ifdef	_SUN
#define dd_bsize	dd_blksize	/* ala sun */
#define d_fileno	d_ino		/* ala sun */
#endif	/* _SUN */

#ifndef NULL
#define NULL 0
#endif
#endif /* _KERNEL */

#endif /* _ALL_SOURCE */

#endif /* _H_DIR */
