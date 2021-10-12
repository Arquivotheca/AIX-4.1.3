/* @(#)23	1.6  src/bos/usr/include/doserrno.h, cmdpcdos, bos411, 9428A410j 6/16/90 00:09:36 */
/*
 * COMPONENT_NAME: CMDPCDOS  routines to read dos floppies
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 10,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* doserrno.h	5.1 - 86/12/09 - 06:04:14 */
#ifndef _H_DOSERRNO
#define _H_DOSERRNO

/*
 * Error codes: positive codes are from UNIX
 *              negative error are DOS-generated errors
 */

#define DE_FNAME  -2      /* Filename syntax error                */
#define DE_NOMNT  -3      /* UNIX filesystem not mounted          */
#define DE_UNOPEN -4      /* File unopened & not reopened         */
#define DE_EXDOS  -5      /* Exec on DOS filesystem file          */
#define DE_RFULL  -6      /* DOS root directory full              */
#define DE_ROOT   -7      /* Can't modify DOS root directory      */
#define DE_BADMNT -8      /* Bad header or FAT for DOS filesystem */
#define DE_NEMPTY -9      /* Directory is not empty               */
#define DE_INIT   -10     /* Dosinit configuration error          */
#define DE_ENVT   -11     /* Environment file problem             */

#define DE_PERM   1       /* Not super-user                       */
#define DE_NOENT  2       /* No such file or directory            */
#define DE_SRCH   3       /* No such process                      */
#define DE_INTR   4       /* interrupted system call              */
#define DE_IO     5       /* I/O error                            */
#define DE_NXIO   6       /* No such device or address            */
#define DE_2BIG   7       /* Arg list too long                    */
#define DE_NOEXEC 8       /* Exec format error                    */
#define DE_BADF   9       /* Bad file number                      */
#define DE_CHILD  10      /* No children                          */
#define DE_AGAIN  11      /* No more processes                    */
#define DE_NOMEM  12      /* Not enough core                      */
#define DE_ACCES  13      /* Permission denied                    */
#define DE_FAULT  14      /* Bad address                          */
#define DE_NOTBLK 15      /* Block device required                */
#define DE_BUSY   16      /* Mount device busy                    */
#define DE_EXIST  17      /* File exists                          */
#define DE_XDEV   18      /* Cross-device link                    */
#define DE_NODEV  19      /* No such device                       */
#define DE_NOTDIR 20      /* Not a directory                      */
#define DE_ISDIR  21      /* Is a directory                       */
#define DE_INVAL  22      /* Invalid argument                     */
#define DE_NFILE  23      /* File table overflow                  */
#define DE_MFILE  24      /* Too many open files                  */
#define DE_NOTTY  25      /* Not a typewriter                     */
#define DE_TXTBSY 26      /* Text file busy                       */
#define DE_FBIG   27      /* File too large                       */
#define DE_NOSPC  28      /* No space left on device              */
#define DE_SPIPE  29      /* Illegal seek                         */
#define DE_ROFS   30      /* Read only file system                */
#define DE_MLINK  31      /* Too many links                       */
#define DE_PIPE   32      /* Broken pipe                          */
#define DE_ROFS   30      /* Read only file system                */
#define DE_MLINK  31      /* Too many links                       */
#define DE_PIPE   32      /* Broken pipe                          */
#define DE_DOM    33      /* Math arg out of domain of func       */
#define DE_RANGE  34      /* Math result not representable        */
#define DE_NOMSG  35      /* No message of desired type           */
#define DE_IDRM   36      /* Identifier removed                   */
#define DE_CHRNG  37      /* Channel number out of range          */
#define DE_L2NSYNC 38     /* Level 2 not synchronized             */
#define DE_L3HLT  39      /* Level 3 halted                       */
#define DE_L3RST  40      /* Level 3 reset                        */
#define DE_LNRNG  41      /* Link number out of range             */
#define DE_UNATCH 42      /* Protocol driver not attached         */
#define DE_NOCSI  43      /* No CSI structure available           */
#define DE_L2HLT  44      /* Level 2 halted                       */
#define DE_DEADLK 45      /* Record locking deadlock              */

#define DE_NOTREADY  46   /* Device not ready			  */
#define DE_WRPROTECT 47   /* Write-protected media 		  */
#define DE_FORMAT    48   /* Unformatted media 			  */

#endif /* _H_DOSERRNO */
