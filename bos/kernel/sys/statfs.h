/* @(#)17	1.11  src/bos/kernel/sys/statfs.h, syslfs, bos411, 9428A410j 8/10/93 08:38:31 */
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: statfs header
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

#ifndef _H_STATFS
#define _H_STATFS

#include <sys/types.h>

/*
 * statfs.h - statfs system call return structure
 */

/*
 * file system statistics
 * NOTE: f_version is UNUSED NOW, but should be set to 0!
 * NOTE: all other reserved fields should be cleared by the vfs implementation
 */
struct statfs {
	long f_version;		/* version/type of statfs, 0 for now */
	long f_type;		/* type of info, zero for now */
	long f_bsize;		/* optimal file system block size */
	long f_blocks;		/* total data blocks in file system */
	long f_bfree;		/* free block in fs */
	long f_bavail;		/* free blocks avail to non-superuser */
	long f_files;		/* total file nodes in file system */
	long f_ffree;		/* free file nodes in fs */
	fsid_t f_fsid;		/* file system id */
	long f_vfstype;		/* what type of vfs this is */
	long f_fsize;		/* fundamental file system block size */
	long f_vfsnumber;	/* vfs indentifier number */
	long f_vfsoff;		/* reserved, for vfs specific data offset */
	long f_vfslen;		/* reserved, for len of vfs specific data */
	long f_vfsvers;		/* reserved, for vers of vfs specific data */
	char f_fname[32];	/* file system name (usually mount pt.) */
	char f_fpack[32];	/* file system pack name */
	long f_name_max;	/* maximum component name length for posix */
};

#endif /* _H_STATFS */
