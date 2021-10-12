/* @(#)69	1.32  src/bos/usr/include/fcntl.h, syslfs, bos411, 9428A410j 6/2/94 12:14:18 */

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 3, 26
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_FCNTL
#define _H_FCNTL

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifndef _H_TYPES
#include <sys/types.h>
#endif

/*
 * POSIX requires that certain values be included in fcntl.h and that only
 * these values be present when _POSIX_SOURCE is defined.  This header
 * includes all the POSIX required entries.
 */

#ifdef _POSIX_SOURCE

#ifndef _H_FLOCK
#include <sys/flock.h>     /* for flock structure */
#endif

#ifdef _NO_PROTO
extern int open();
extern int creat();
extern int fcntl();

#else 		/* use POSIX required prototypes */
#ifndef _KERNEL
extern int open(const char *, int, ...);
extern int creat(const char *, mode_t);
extern int fcntl(int, int,...);
#endif /* _KERNEL */
#endif /* _NO_PROTO */

/* File access modes for open */
#define	O_RDONLY	00000000
#define	O_WRONLY	00000001
#define	O_RDWR		00000002

/* Mask for use with file access modes */
#define O_ACCMODE	3		

/* File flags accessible to open and fcntl */
#define O_NONBLOCK	00000004	/* POSIX non-blocking I/O	*/
#define	O_APPEND	00000010  	/* all writes append to the end	*/
 
/* Flag flags accessible only to open */
#define	O_CREAT		00000400	/* open with creation		*/
#define	O_TRUNC		00001000	/* open with truncation		*/
#define	O_EXCL		00002000	/* exclusive open		*/
#define O_NOCTTY	00004000	/* do not assign control tty	*/

/* fcntl requests */
#define	 F_DUPFD	0		/* Duplicate file descriptor	*/
#define	 F_GETFD	1		/* Get file descriptor flags	*/
#define	 F_SETFD	2		/* Set file descriptor flags	*/
#define	 F_GETFL	3		/* Get file flags		*/
#define	 F_SETFL	4		/* Set file flags		*/
#define	 F_GETLK	5		/* Get file lock		*/
#define	 F_SETLK	6		/* Set file lock		*/
#define	 F_SETLKW	7		/* Set file lock and wait	*/

/* File descriptor flags used for fcntl F_GETFD and F_SETFD */
#define FD_CLOEXEC      1		/* Close this file during exec	*/

#endif /* _POSIX_SOURCE */

#ifdef _XOPEN_SOURCE

#ifndef _H_UNISTD
#include <unistd.h>
#endif

#ifndef _H_STAT
#include <sys/stat.h>
#endif

#define O_SYNC		00000020	/* synchronous write option	*/
#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE

#ifdef _NO_PROTO
extern int openx();
#else
#ifndef _KERNEL
extern int openx(char *path, int oflag, ...);
#endif /* !_KERNEL */
#endif /* _NO_PROTO */

/* Additional open modes */

#define	O_NONE		00000003	/* open without read or write	*/
#define O_EXEC		00000040	/* open for exec		*/
#define O_RSHARE	00010000	/* read shared open		*/
#define O_DEFER		00020000	/* defered update		*/
#define O_DELAY		00040000	/* open with delay		*/
#define O_NDELAY	00100000	/* Non-blocking I/O		*/
#define O_NSHARE	00200000	/* read shared open		*/

/* Additional fcntl requests */

#define	 F_GETOWN	8		/* Get async I/O owner		*/
#define	 F_SETOWN	9		/* Set async I/O owner		*/
#define  F_CLOSEM	10		/* Close multiple files		*/

/* Additional fcntl F_SETFL flags */

#ifndef _KERNEL
#ifdef	_BSD
/* Provide appropriate definition of FNDELAY for BSD */
#define	FNDELAY		O_NONBLOCK
#undef	O_NDELAY
#define	O_NDELAY	O_NONBLOCK
#else	/* System V */
#define	FNDELAY		O_NDELAY
#endif	/* _BSD */
#endif	/* _KERNEL */

#define	FNONBLOCK 	O_NONBLOCK
#define	FAPPEND		O_APPEND
#define	FSYNC		O_SYNC
#define FASYNC		00400000
#define FSYNCALL	02000000

/* File flag values corresponding to file access modes */

#define	FOPEN		(-1)
#define	FREAD		(O_RDONLY-FOPEN)
#define	FWRITE		(O_WRONLY-FOPEN)

#ifdef _KERNEL

#define	FMPX		00000200	/* multiplexed open, obsolete	*/

/* FMASK defines the bits that remain in the file flags past the open	*/
#define	FMASK		00374377

/* FFCNTL is all the bits that may be set via fcntl. */
#define	FFCNTL		(FNONBLOCK|FNDELAY|FAPPEND|FSYNC|FASYNC)

/* open only modes */
#define	FCREAT		O_CREAT
#define	FTRUNC		O_TRUNC
#define	FEXCL		O_EXCL

#define FRSHARE 	O_RSHARE
#define FDEFER		O_DEFER
#define FDELAY		O_DELAY
#define FNDELAY		O_NDELAY
#define FNSHARE		O_NSHARE

/* additional modes */
#define	FEXEC		O_EXEC		/* open/close for exec		*/
#define	FNOCTTY		O_NOCTTY	/* do not assign control tty	*/
#define	FMOUNT		01000000	/* open/close for device mounts */
#define	FREVOKED	0x20000000	/* file access has been revoked	*/
#define	FKERNEL		0x40000000	/* caller is in kernel mode	*/
#define FAIO		00000100	/* aio on this fp		*/
#define FDOCLONE	0x10000000	/* reserve DDOCLONE for devices	*/
#define GCFMARK		0x00100000	/* mark during unp_gc() */
#define GCFDEFER	0x00200000	/* defer during unp_gc() */


#endif	/* _KERNEL	*/
#endif	/* _ALL_SOURCE	*/
#endif	/* _H_FCNTL	*/
