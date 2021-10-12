/* @(#)93	1.20  src/bos/kernel/sys/file.h, syslfs, bos411, 9428A410j 1/26/94 16:08:15 */
#ifndef _H_FILE
#define _H_FILE

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 3, 26
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/access.h>
#include <sys/lock_def.h>
#include <sys/cred.h>
#include <fcntl.h>

#ifdef _KERNEL
/*
 * One file structure is allocated for each open/creat/pipe call.
 * Main use is to hold the read/write pointer associated with
 * each open file.
 */
struct	file {
	long	f_flag;		/* see fcntl.h for definitions */
	long	f_count;	/* reference count */
	short	f_msgcount;	/* references from message queue */
	short	f_type;		/* descriptor type */
	union {
		struct vnode	*f_uvnode;	/* pointer to vnode structure */
		struct file	*f_unext;	/* next entry in freelist */
	} f_up;
#ifdef _LONG_LONG
	offset_t f_offset;	/* read/write character pointer */
#else
	long    f_rsvd;         /* ANSI-C does not suppport long long   */ 
	off_t   f_offset;       /* off_t rd/wr pointer for ANSI-C mode  */
#endif

	off_t	f_dir_off;	/* BSD style directory offsets */
	struct ucred *f_cred;   /* process credentials at time of open */
	Simple_lock f_lock;	/* file structure fields lock */
	Simple_lock f_offset_lock; /* file structure offset field lock */
	caddr_t	f_vinfo;	/* any info vfs needs */ 
	struct fileops
	{
		int	(*fo_rw)();
		int	(*fo_ioctl)();
		int	(*fo_select)();
		int	(*fo_close)();
		int	(*fo_fstat)();
	} *f_ops;
};

#define	f_data		f_up.f_uvnode
#define	f_vnode		f_up.f_uvnode
#define	f_next		f_up.f_unext

/* f_type values */

#define	DTYPE_VNODE	1	/* file */
#define	DTYPE_SOCKET	2	/* communications endpoint */
#define	DTYPE_GNODE	3	/* device */
#define	DTYPE_OTHER    -1	/* unknown */

extern struct file file[];	/* The file table itself */
extern struct file *ffreelist;	/* Head of freelist pool */
extern Simple_lock ffree_lock;	/* File Table Free List Lock */
#endif

/*
 * Flock call.
 */
#define	LOCK_SH		1	/* shared lock */
#define	LOCK_EX		2	/* exclusive lock */
#define	LOCK_NB		4	/* don't block when locking */
#define	LOCK_UN		8	/* unlock */

/*
 * Lseek call.
 */
#define	L_SET		0	/* absolute offset */
#define	L_INCR		1	/* relative to current offset */
#define	L_XTND		2	/* relative to end of file */

#endif /* _H_FILE */
