/* @(#)06	1.29  src/bos/kernel/sys/stat.h, syslfs, bos411, 9428A410j 12/7/93 18:44:53 */
/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_STAT
#define _H_STAT

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifndef _H_TYPES
#include <sys/types.h>
#endif

#ifndef _H_MODE
#include <sys/mode.h>
#endif

/*
 * POSIX requires that certain values be included in stat.h.  It also
 * requires that when _POSIX_SOURCE is defined only those standard
 * specific values are present.  This header includes all the POSIX
 * required entries.
 */

#ifdef _POSIX_SOURCE 

/*
 *	Stat structure
 *
 *	The complete structure is returned only for statx() & fstatx().  All
 *	other flavors of stat() return subsets of this structure.  To be exact,
 *	only the fields up to the related comments below are filled in for
 *	each of the specified compatibility interfaces.
 */

struct  stat
{
	dev_t	st_dev;			/* ID of device containing a directory*/
					/*   entry for this file.  File serial*/
					/*   no + device ID uniquely identify */
					/*   the file within the system */
	ino_t	st_ino;			/* File serial number */
#ifdef	_NONSTD_TYPES
	ushort	st_mode_ext;
	ushort  st_mode;
#else
	mode_t	st_mode;		/* File mode; see #define's below */
#endif	/* _NONSTD_TYPES */
	short	st_nlink;		/* Number of links */
#ifdef	_NONSTD_TYPES
	ushort  st_pad_to_word;
	ushort	st_uid_ext;
	ushort	st_uid;
	ushort	st_gid_ext;
	ushort	st_gid;
#else
	uid_t	st_uid;			/* User ID of the file's owner */
	gid_t	st_gid;			/* Group ID of the file's group */
#endif	/* _NONSTD_TYPES */
	dev_t	st_rdev;		/* ID of device */
					/*   This entry is defined only for */
					/*   character or block special files */
	off_t	st_size;		/* File size in bytes */
	time_t	st_atime;		/* Time of last access */
	int	st_spare1;
	time_t	st_mtime;		/* Time of last data modification */
	int	st_spare2;
	time_t	st_ctime;		/* Time of last file status change */
	int	st_spare3;
					/* Time measured in seconds since */
					/*   00:00:00 GMT, Jan. 1, 1970 */
	ulong_t	st_blksize;		/* Optimal blocksize for file system
					   i/o ops */
	ulong_t	st_blocks;		/* Actual number of blocks allocated
					   in DEV_BSIZE blocks */
	int	st_vfstype;		/* Type of fs (see vnode.h) */
	ulong_t	st_vfs;			/* Vfs number */
	ulong_t	st_type;		/* Vnode type */

	/********************************************************************/
	/****  End of initialized data for stat(), fstat(), and lstat()  ****/
	/********************************************************************/

	ulong_t	st_gen;			/* Inode generation number */
	ulong_t	st_flag;		/* Flag word */

#ifdef _ALL_SOURCE
	uid_t	Reserved1;		/* Reserved */
	gid_t	Reserved2;		/* Reserved */
#else
	uid_t	st_Reserved1;		/* Reserved */
	gid_t	st_Reserved2;		/* Reserved */
#endif

	/*****************************************************************/
	/****  End of initialized data for fullstat() and ffullstat() ****/
	/*****************************************************************/

	ushort_t st_access;		/* Process' access to file */
	ulong_t	st_spare4[5];		/* Reserved */
};
			/* End of the stat structure */


#ifndef _KERNEL
#ifndef	_NONSTD_TYPES
#ifdef	_NO_PROTO
	extern mode_t	umask(); 
#else	/* _NO_PROTO */
	extern mode_t	umask(mode_t); 
#endif	/* _NO_PROTO */
#endif	/* _NONSTD_TYPES */
#endif	/* _KERNEL */

#ifdef _NO_PROTO
#ifndef _KERNEL
	extern int	mkdir(); 
	extern int	stat();
	extern int	fstat();
	extern int	chmod();
#endif /* _KERNEL */
	extern int	mkfifo();
#else				/* use POSIX required prototypes */
#ifndef _KERNEL
	extern int	mkdir(const char *, mode_t); 
	extern int	stat(const char *, struct stat *);
	extern int	fstat(int, struct stat *);
	extern int	chmod(const char *, mode_t);
#endif /* _KERNEL */
	extern int	mkfifo(const char *, mode_t);
#endif /* _NO_PROTO */

#endif /* _POSIX_SOURCE */

#ifdef _ALL_SOURCE

#ifdef _NO_PROTO
#ifndef _KERNEL
        extern int      lstat();
        extern int      fchmod();
	extern int 	mknod();
#endif /* _KERNEL */
#else                           /* use POSIX required prototypes */
#ifndef _KERNEL
        extern int      lstat(const char *, struct stat *);
        extern int      fchmod(int, mode_t);
	extern int	mknod(const char *, mode_t, dev_t);
#endif /* _KERNEL */
#endif /* _NO_PROTO */

/*
 *	st_flag values
 */

#define	FS_VMP		01		/* file is vfs root or mounted over */
#define	FS_MOUNT	FS_VMP		/*   (vfs mount point) */
#define FS_REMOTE	02		/* file is remote */

/*
 * st_dev bit definitions
 */
#define SDEV_REMOTE	0x80000000	/* remote `device' if on	*/

/*
 *	statx() cmd arguments
 */

#define	STX_NORMAL	0x00	/* normal statx processing */
#define	STX_LINK	0x01	/* do not traverse final symbolic link */
#define	STX_MOUNT	0x02	/* do not traverse final mount point */
#define	STX_HIDDEN	0x04	/* do not traverse final hidden directory */


/*
 *	stat structure subset sizes for statx() compatibility routines
 */

#define STATXSIZE	(sizeof(struct stat))
#define STATSIZE	((int)&((struct stat *)0)->st_gen)
#define FULLSTATSIZE	((int)&((struct stat *)0)->st_access)

#endif /* _ALL_SOURCE */
#endif /* _H_STAT */
