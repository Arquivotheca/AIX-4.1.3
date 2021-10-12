/* @(#)77	1.7  src/bos/usr/ccs/lib/libc/libvfs.h, libcadm, bos411, 9428A410j 6/16/90 01:03:33 */
/*
 * COMPONENT_NAME: (LIBCADM) Standard C Library System Admin Functions 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_LIBFS
#define _H_LIBFS

#include <stdio.h>

extern int            VfsFd;
extern unsigned int   VfsOpen;
extern char           VfsBuf[BUFSIZ];
extern FILE          *VfsStream;

extern char           *default_local_vfs;
extern char           *default_remote_vfs;

#endif /* _H_LIBFS */
