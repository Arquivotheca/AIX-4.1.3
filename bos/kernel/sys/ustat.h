/* @(#)04	1.7  src/bos/kernel/sys/ustat.h, syslfs, bos411, 9428A410j 12/7/93 18:49:48 */

#ifndef _H_USTAT
#define _H_USTAT
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
#ifdef DFS

struct  ustat
{
	daddr_t	f_tfree;	/* total free */
	ino_t	f_tinode;	/* total inodes free */
	char	f_fname[6];	/* filsys name */
	char	f_fpack[6];	/* filsys pack name */
};

#ifndef _KERNEL
#ifndef _NO_PROTO
extern int ustat(dev_t, struct ustat *);
#else
extern int ustat();
#endif /* _NO_PROTO */
#endif /* _KERNEL   */

#endif
#endif	/* _H_USTAT */
