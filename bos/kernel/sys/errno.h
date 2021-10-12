/* @(#)49	1.27.1.10  src/bos/kernel/sys/errno.h, incstd, bos411, 9438C411c 9/23/94 19:32:38 */
/*
 * COMPONENT_NAME: (INCSTD) Standard Include Files
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27,71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */

#ifndef _H_ERRNO
#define _H_ERRNO
#include <standards.h>

/*
 *	Error codes
 *
 *      The ANSI, POSIX, and XOPEN standards require that certain values be
 *	in errno.h.  The standards allow additional macro definitions,
 *      beginning with an E and an uppercase letter.
 *
 */
 
#ifdef _ANSI_C_SOURCE

#if defined(_THREAD_SAFE) || defined(_THREAD_SAFE_ERRNO)
/*
 * Per thread errno is provided by the threads provider. Both the extern int
 * and the per thread value must be maintained by the threads library.
 */
#define errno	(*_Errno())

#endif	/* _THREAD_SAFE || _THREAD_SAFE_ERRNO */

extern int errno;

#define	EPERM	1	/* Operation not permitted		*/
#define	ENOENT	2	/* No such file or directory		*/
#define	ESRCH	3	/* No such process			*/
#define	EINTR	4	/* interrupted system call		*/
#define	EIO	5	/* I/O error				*/
#define	ENXIO	6	/* No such device or address		*/
#define	E2BIG	7	/* Arg list too long			*/
#define	ENOEXEC	8	/* Exec format error			*/
#define	EBADF	9	/* Bad file descriptor			*/
#define	ECHILD	10	/* No child processes			*/
#define	EAGAIN	11	/* Resource temporarily unavailable	*/
#define	ENOMEM	12	/* Not enough space			*/
#define	EACCES	13	/* Permission denied			*/
#define	EFAULT	14	/* Bad address				*/
#define	ENOTBLK	15	/* Block device required		*/
#define	EBUSY	16	/* Resource busy			*/
#define	EEXIST	17	/* File exists				*/
#define	EXDEV	18	/* Improper link			*/
#define	ENODEV	19	/* No such device			*/
#define	ENOTDIR	20	/* Not a directory			*/
#define	EISDIR	21	/* Is a directory			*/
#define	EINVAL	22	/* Invalid argument			*/
#define	ENFILE	23	/* Too many open files in system	*/
#define	EMFILE	24	/* Too many open files			*/
#define	ENOTTY	25	/* Inappropriate I/O control operation	*/
#define	ETXTBSY	26	/* Text file busy			*/
#define	EFBIG	27	/* File too large			*/
#define	ENOSPC	28	/* No space left on device		*/
#define	ESPIPE	29	/* Invalid seek				*/
#define	EROFS	30	/* Read only file system		*/
#define	EMLINK	31	/* Too many links			*/
#define	EPIPE	32	/* Broken pipe				*/
#define	EDOM	33	/* Domain error within math function	*/
#define	ERANGE	34	/* Result too large			*/
#define	ENOMSG	35	/* No message of desired type		*/
#define	EIDRM	36	/* Identifier removed			*/
#define	ECHRNG	37	/* Channel number out of range		*/
#define	EL2NSYNC 38	/* Level 2 not synchronized		*/
#define	EL3HLT	39	/* Level 3 halted			*/
#define	EL3RST	40	/* Level 3 reset			*/
#define	ELNRNG	41	/* Link number out of range		*/
#define	EUNATCH 42	/* Protocol driver not attached		*/
#define	ENOCSI	43	/* No CSI structure available		*/
#define	EL2HLT	44	/* Level 2 halted			*/
#define EDEADLK 45	/* Resource deadlock avoided		*/

#define ENOTREADY	46	/* Device not ready		*/
#define EWRPROTECT	47	/* Write-protected media 	*/
#define EFORMAT		48	/* Unformatted media 		*/

#define ENOLCK		49	/* No locks available 		*/

#define ENOCONNECT      50      /* no connection                */
#define ESTALE          52      /* no filesystem                */
#define	EDIST		53 	/* old, currently unused AIX errno*/ 

/* non-blocking and interrupt i/o */
/*
 * AIX returns EAGAIN where 4.3BSD used EWOULDBLOCK;
 * but, the standards insist on unique errno values for each errno.
 * A unique value is reserved for users that want to code case
 * statements for systems that return either EAGAIN or EWOULDBLOCK.
 */
#ifdef _ALL_SOURCE
#define EWOULDBLOCK     EAGAIN   /* Operation would block	*/
#else	/* not _ALL_SOURCE */
#define EWOULDBLOCK	54
#endif	/* _ALL_SOURCE */

#define EINPROGRESS     55      /* Operation now in progress */
#define EALREADY        56      /* Operation already in progress */

/* ipc/network software */

	/* argument errors */
#define ENOTSOCK        57      /* Socket operation on non-socket */
#define EDESTADDRREQ    58      /* Destination address required */
#define EDESTADDREQ     EDESTADDRREQ /* Destination address required */
#define EMSGSIZE        59      /* Message too long */
#define EPROTOTYPE      60      /* Protocol wrong type for socket */
#define ENOPROTOOPT     61      /* Protocol not available */
#define EPROTONOSUPPORT 62      /* Protocol not supported */
#define ESOCKTNOSUPPORT 63      /* Socket type not supported */
#define EOPNOTSUPP      64      /* Operation not supported on socket */
#define EPFNOSUPPORT    65      /* Protocol family not supported */
#define EAFNOSUPPORT    66      /* Address family not supported by protocol family */
#define EADDRINUSE      67      /* Address already in use */
#define EADDRNOTAVAIL   68      /* Can't assign requested address */

	/* operational errors */
#define ENETDOWN        69      /* Network is down */
#define ENETUNREACH     70      /* Network is unreachable */
#define ENETRESET       71      /* Network dropped connection on reset */
#define ECONNABORTED    72      /* Software caused connection abort */
#define ECONNRESET      73      /* Connection reset by peer */
#define ENOBUFS         74      /* No buffer space available */
#define EISCONN         75      /* Socket is already connected */
#define ENOTCONN        76      /* Socket is not connected */
#define ESHUTDOWN       77      /* Can't send after socket shutdown */

#define ETIMEDOUT       78      /* Connection timed out */
#define ECONNREFUSED    79      /* Connection refused */

#define EHOSTDOWN       80      /* Host is down */
#define EHOSTUNREACH    81      /* No route to host */

/* ERESTART is used to determine if the system call is restartable */
#define ERESTART	82	/* restart the system call */

/* quotas and limits */
#define EPROCLIM	83	/* Too many processes */
#define	EUSERS		84	/* Too many users */
#define	ELOOP		85	/* Too many levels of symbolic links      */
#define	ENAMETOOLONG	86	/* File name too long			  */

/*
 * AIX returns EEXIST where 4.3BSD used ENOTEMPTY;
 * but, the standards insist on unique errno values for each errno.
 * A unique value is reserved for users that want to code case
 * statements for systems that return either EEXIST or ENOTEMPTY.
 */
#ifdef _ALL_SOURCE
#define ENOTEMPTY	EEXIST	/* Directory not empty */
#else	/* not _ALL_SOURCE */
#define ENOTEMPTY	87
#endif	/* _ALL_SOURCE */

/* disk quotas */
#define EDQUOT		88	/* Disc quota exceeded */

/* errnos 89-92 reserved for future use compatible with AIX PS/2 */

/* network file system */
#define EREMOTE		93	/* Item is not local to host */

/* errnos 94-108 reserved for future use compatible with AIX PS/2 */

#define ENOSYS		109	/* Function not implemented  POSIX */

/* disk device driver */
#define EMEDIA		110 	/* media surface error */
#define ESOFT           111     /* I/O completed, but needs relocation */

/* security */
#define ENOATTR		112 	/* no attribute found */
#define ESAD		113	/* security authentication denied */
#define ENOTRUST	114	/* not a trusted program */ 

/* BSD 4.3 RENO */
#define ETOOMANYREFS    115     /* Too many references: can't splice */

#define EILSEQ		116     /* Invalid wide character */
#define ECANCELED 	117     /* asynchronous i/o cancelled */

/* SVR4 STREAMS */
#define	ENOSR		118	/* temp out of streams resources */
#define	ETIME		119	/* I_STR ioctl timed out */
#define	EBADMSG		120	/* wrong message type at stream head */
#define	EPROTO		121	/* STREAMS protocol error */
#define	ENODATA		122	/* no message ready at stream head */
#define	ENOSTR		123	/* fd is not a stream */

#define	ECLONEME	ERESTART /* this is the way we clone a stream ... */

#define ENOTSUP		124	/* POSIX threads unsupported value */

#define EMULTIHOP       125     /* multihop is not allowed */
#define ENOLINK         126     /* the link has been severed */
#define EOVERFLOW       127     /* value too large to be stored in data type */

#endif /* _ANSI_C_SOURCE */
#endif /* _H_ERRNO */
