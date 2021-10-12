/* @(#)56	1.1  src/bos/kernel/sys/fs_hooks.h, syslfs, bos411, 9428A410j 5/28/91 01:32:02 */
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: fs_hooks.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_FSH_
#define _H_FSH_
/* hooks into the filesystem */

#ifdef _KERNEL

#include <sys/file.h>

/*
 * Values for cmd parameter to fs_hook_add() kernel service
 */
#define FS_EXECH_ADD 1		/* add hook at exec */
#define FS_CLOSEH_ADD 2		/* add hook at close */

struct fs_hook {
	void (*hook)(int fd, struct file *fp);
	struct fs_hook *next;
};

#ifdef _NO_PROTO
extern void fs_hookadd();
#else /* _NO_PROTO */
extern void fs_hookadd(struct fs_hook *fshp, int cmd);
#endif /* _NO_PROTO */

extern struct fs_hook *closeh_anchor;
extern struct fs_hook *fs_exech_anchor;

#endif /* _KERNEL */

#endif /* _H_FSH_ */
