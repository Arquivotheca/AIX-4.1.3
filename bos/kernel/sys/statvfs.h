/* @(#)10	1.1  src/bos/kernel/sys/statvfs.h, libcfs, bos411, 9428A410j 12/7/93 20:17:40 */
/*
 *   COMPONENT_NAME: LIBCFS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 4,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_STATVFS
#define _H_STATVFS

#include <sys/vmount.h>

#define FSTYPSIZ 16

/*
 * statvfs.h - statvfs system call return structure
 */

struct statvfs {
	ulong  f_bsize;		/* preferred file system block size          */
	ulong  f_frsize;	/* fundamental file system block size        */
	ulong  f_blocks;	/* total # of blocks of f_frsize on file     */ 
				/* 	system 				     */
	ulong  f_bfree;		/* total # of free blocks 		     */
	ulong  f_bavail;	/* # of blocks available to non super user   */
	ulong  f_files;		/* total # of file nodes (inode in JFS)      */
	ulong  f_ffree;		/* total # of free file nodes		     */
	ulong  f_favail;	/* # of nodes available to non super user    */
	fsid_t f_fsid;		/* file system id			     */
	char   f_basetype[FSTYPSIZ]; /* Filesystem type name (eg. jfs)       */
	ulong  f_flag;		/* bit mask of flags			     */
	ulong  f_namemax;	/* maximum filename length	  	     */
	char   f_fstr[32];	/* filesystem-specific string */
	ulong  f_filler[16];	/* reserved for future use		     */
};

#define ST_NOSUID	MNT_NOSUID	/* don't maintain SUID capability    */
#define ST_RDONLY	MNT_READONLY	/* file system mounted read only     */
#define ST_NODEV	MNT_NODEV	/* don't allow device access across  */
					/* this mount		 	     */	

/*
 * Prototypes
 */
#ifdef _NO_PROTO
extern int statvfs();
extern int fstatvfs();
#else
extern int statvfs(const char *, struct statvfs *);
extern int fstatvfs(int, struct statvfs *);
#endif

#endif /* _H_STATVFS */
