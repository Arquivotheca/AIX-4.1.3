static char sccsid[] = "@(#)80	1.6  src/bos/usr/ccs/lib/libc/errlst.c, libcproc, bos411, 9428A410j 3/4/94 10:25:37";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: sys_errlist, sys_nerr
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

char	*sys_errlist[] = {
	"Error 0",                             /* Corresponding errno */
	"Not owner",                           /*  EPERM          1 */
	"No such file or directory",           /*  ENOENT         2 */
	"No such process",                     /*  ESRCH          3 */
	"Interrupted system call",             /*  EINTR          4 */
	"I/O error",                           /*  EIO            5 */
	"No such device or address",           /*  ENXIO          6 */
	"Arg list too long",                   /*  E2BIG          7 */
	"Exec format error",                   /*  ENOEXEC        8 */
	"Bad file number",                     /*  EBADF          9 */
	"No child processes",                  /*  ECHILD         10 */
	"Resource temporarily unavailable",    /*  EAGAIN         11 */
	"Not enough space",                    /*  ENOMEM         12 */
	"Permission denied",                   /*  EACCES         13 */
	"Bad address",                         /*  EFAULT         14 */
	"Block device required",               /*  ENOTBLK        15 */
	"Device busy",                         /*  EBUSY          16 */
	"File exists",                         /*  EEXIST         17 */
	"Cross-device link",                   /*  EXDEV          18 */
	"No such device",                      /*  ENODEV         19 */
	"Not a directory",                     /*  ENOTDIR        20 */
	"Is a directory",                      /*  EISDIR         21 */
	"Invalid argument",                    /*  EINVAL         22 */
	"File table overflow",                 /*  ENFILE         23 */
	"Too many open files",                 /*  EMFILE         24 */
	"Not a typewriter",                    /*  ENOTTY         25 */
	"Text file busy",                      /*  ETXTBSY        26 */
	"File too large",                      /*  EFBIG          27 */
	"No space left on device",             /*  ENOSPC         28 */
	"Illegal seek",                        /*  ESPIPE         29 */
	"Read-only file system",               /*  EROFS          30 */
	"Too many links",                      /*  EMLINK         31 */
	"Broken pipe",                         /*  EPIPE          32 */
	"Argument out of domain",              /*  EDOM           33 */
	"Result too large",                    /*  ERANGE         34 */
	"No message of desired type",          /*  ENOMSG         35 */
	"Identifier removed",                  /*  EIDRM          36 */
	"Channel number out of range",         /*  ECHRNG         37 */
	"Level 2 not synchronized",            /*  EL2NSYNC       38 */
	"Level 3 halted",                      /*  EL3HLT         39 */
	"Level 3 reset",                       /*  EL3RST         40 */
	"Link number out of range",            /*  ELNRNG         41 */
	"Protocol driver not attached",        /*  EUNATCH        42 */
	"No CSI structure available",          /*  ENOCSI         43 */
	"Level 2 halted",                      /*  EL2HLT         44 */
	"Deadlock condition if locked",        /*  EDEADLK        45 */
	"Device not ready",                    /*  ENOTREADY      46 */
	"Write-protected media",               /*  EWRPROTECT     47 */
	"Unformatted or incompatible media",   /*  EFORMAT        48 */
	"No locks available",                  /*  ENOLCK         49 */
	"Cannot Establish Connection",         /*  ENOCONNECT     50 */
	"Connection Down",                     /*  EBADCONNECT    51 */
	"Missing file or filesystem",          /*  ESTALE         52 */
	"Requests blocked by Administrator",   /*  EDIST          53 */
	"Operation would block",               /*  EWOULDBLOCK    54 */
	"Operation now in progress",           /*  EINPROGRESS    55 */
	"Operation already in progress",       /*  EALREADY       56 */
	"Socket operation on non-socket",      /*  ENOTSOCK       57 */
	"Destination address required",        /*  EDESTADDRREQ   58 */
	"Message too long",                    /*  EMSGSIZE       59 */
	"Protocol wrong type for socket",      /*  EPROTOTYPE     60 */
	"Protocol not available",              /*  ENOPROTOOPT    61 */
	"Protocol not supported",              /*  EPROTONOSUPPORT 62 */
	"Socket type not supported",           /*  ESOCKTNOSUPPORT 63 */
	"Operation not supported on socket",   /*  EOPNOTSUPP     64 */
	"Protocol family not supported",       /*  EPFNOSUPPORT   65 */ 
	"Addr family not supported by protocol", /*  EAFNOSUPPORT 66 */ 
	"Address already in use",              /*  EADDRINUSE     67 */ 	
	"Can't assign requested address",      /*  EADDRNOTAVAIL  68 */ 
	"Network is down",                     /*  ENETDOWN       69 */ 
	"Network is unreachable",              /*  ENETUNREACH    70 */
	"Network dropped connection on reset", /*  ENETRESET      71 */
	"Software caused connection abort",    /*  ECONNABORTED   72 */
	"Connection reset by peer",            /*  ECONNRESET     73 */
	"No buffer space available",           /*  ENOBUFS        74 */
	"Socket is already connected",         /*  EISCONN        75 */
	"Socket is not connected",             /*  ENOTCONN       76 */
	"Can't send after socket shutdown",    /*  ESHUTDOWN      77 */
	"Connection timed out",                /*  ETIMEDOUT      78 */
	"Connection refused",                  /*  ECONNREFUSED   79 */
	"Host is down",                        /*  EHOSTDOWN      80 */
	"No route to host",                    /*  EHOSTUNREACH   81 */
	"Restart the system call",             /*  ERESTART       82 */
	"Too many processes",                  /*  EPROCLIM       83 */
	"Too many users",                      /*  EUSERS         84 */
	"Too many levels of symbolic links",   /*  ELOOP          85 */
	"File name too long",                  /*  ENAMETOOLONG   86 */
	"Directory not empty",                 /*  ENOTEMPTY      87 */
	"Disk quota exceeded",                 /*  EDQUOT         88 */
	"For future use ",                     /*                 89 */
	"For future use ",                     /*                 90 */
	"For future use ",                     /*                 91 */
	"For future use ",                     /*                 92 */
	"Item is not local to host",           /*  EREMOTE        93 */
	"For future use ",                     /*                 94 */
	"For future use ",                     /*                 95 */
	"For future use ",                     /*                 96 */
	"For future use ",                     /*                 97 */
	"For future use ",                     /*                 98 */
	"For future use ",                     /*                 99 */
	"For future use ",                     /*                 100 */
	"For future use ",                     /*                 101 */
	"For future use ",                     /*                 102 */
	"For future use ",                     /*                 103 */
	"For future use ",                     /*                 104 */
	"For future use ",                     /*                 105 */
	"For future use ",                     /*                 106 */
	"For future use ",                     /*                 107 */
	"For future use ",                     /*                 108 */
	"Function not implemented",            /*  ENOSYS         109 */
	"Media surface error",                 /*  EMEDIA         110 */
	"I/O completed, but needs relocation", /*  ESOFT          111 */
	"No attribute found", 		       /*  ENOATTR        112 */
	"Security Authentication Denied",      /*  ESAD           113 */
	"Not a Trusted Program",	       /*  ENOTRUST       114 */
	"Too many references: can't splice",   /*  ETOOMANYREFS   115 */
	"Invalid wide character",              /*  EILSEQ         116 */
	"Asynchronous I/O cancelled",          /*  ECANCELED      117 */
	"Out of STREAMS resources",            /*  ENOSR          118 */
	"System call timed out",               /*  ETIME          119 */
	"Next message has wrong type",         /*  EBADMSG        120 */
	"Error in protocol",                   /*  EPROTO         121 */
	"No message on stream head read q",    /*  ENODATA        122 */
	"fd not associated with a stream",     /*  ENOSTR         123 */
	"Unsupported attribute value",         /*  ENOTSUP        124 */
	"Multihop is not allowed",	       /*  EMULTIHOP      125 */
	"The server link has been severed",    /*  ENOLINK        126 */
	"Value too large to be stored in data type", /* EOVERFLOW 127 */
};

int	sys_nerr = { sizeof(sys_errlist)/sizeof(sys_errlist[0]) };
