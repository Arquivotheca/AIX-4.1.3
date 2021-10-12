/* @(#)37	1.2  src/bos/usr/include/jfs/commit.h, syspfs, bos411, 9428A410j 4/29/91 11:55:41 */
/*
 * COMPONENT_NAME: (SYSPFS) commit header
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_JFS_COMMIT
#define _H_JFS_COMMIT

#include "sys/types.h"
#include "jfs/inode.h"
#include "jfs/log.h"

#define  FORCE 1
#define  NOFORCE 0

#define  KEEP  1
#define  NOKEEP 0

/* Handy if expression
 */
#define ISPECIAL(ip)	\
	((ip)->i_number == INODES_I || \
	(ip)->i_number == 0 || \
	(ip)->i_number == INOMAP_I || \
	(ip)->i_number == DISKMAP_I || \
	(ip)->i_number == SUPER_I || \
	(ip)->i_number == INDIR_I || \
	(ip)->i_number == INODEX_I)

/* Static interface to commit. Not externalized
 */
struct commit {
	int  number;		/* number of segments to commit */
	struct inode *iptr[6];	/* corresponding inodes */
};


/* This structure is used by the commit manager as an argument
 * to all commit support routines
 */
struct comdata {
	int  tid;      		/* tidreg value = index of tblk */
	int  current;  		/* current index in iptr */
	int  * indptr; 		/* pointer to current indirect block */
	int  number;   		/* number of entries in iptr */
	struct inode * iptr[8];	/* pointers to inodes */
	struct inode * ipi;	/* inode of .inodes */
	struct inode * ipind;	/* inode of .indirect */
	struct inode * ilog;	/* inode of log */
	struct inode * ipimap;  /* inode of inode map */
	struct inode * ipdmap;  /* inode of disk map */
	struct vmdlist * freep; /* head of free perm map list */
	struct vmdlist * freepw;/* head of free perm and work map list */
	struct vmdlist * alloc; /*  head of allocate perm map list */
	struct logrdesc lr;    	/* log record descriptor */
};

/* recognizable error conditions from setjmpx */
extern int reg_elist[];		/* ok from regular file and dir */

/* return macro used by subroutines that may have
 * experienced an exception of some sort, either directly or
 * in a called subroutine.
 */
# define	PFS_EXCEPTION	0x80000000
# define	RETURNX(rc, el)	\
	return((rc & PFS_EXCEPTION) ?pfs_exception(rc, el) :rc)

#endif /* _H_JFS_COMMIT */
