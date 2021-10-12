/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: free_disk_space
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * HISTORY
 * $Log: free_disk_space.c,v $
 * Revision 1.1.4.3  1993/05/05  14:28:53  marty
 * 	Include sys/socket.h for type caddr_t and vfs_data.
 * 	[1993/05/05  14:27:55  marty]
 *
 * Revision 1.1.4.2  1993/04/27  21:01:21  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  21:01:12  damon]
 * 
 * Revision 1.1.2.8  1992/12/03  17:21:37  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:40  damon]
 * 
 * Revision 1.1.2.7  1992/11/13  15:20:29  root
 * 	Include sys/statfs.h if INC_STATFS is defined
 * 	[1992/11/13  15:05:48  root]
 * 
 * Revision 1.1.2.6  1992/11/12  18:28:03  damon
 * 	CR 329. Added INC_VFS de-ansified decl of free_disk_space
 * 	[1992/11/12  18:26:48  damon]
 * 
 * Revision 1.1.2.5  1992/11/06  16:21:08  damon
 * 	CR 329. Use mount.h if not using statvfs
 * 	[1992/11/06  16:20:54  damon]
 * 
 * Revision 1.1.2.4  1992/11/06  16:15:22  damon
 * 	CR 329. Changed NO_STATFS to NO_FSTATFS
 * 	[1992/11/06  16:10:59  damon]
 * 
 * Revision 1.1.2.3  1992/11/05  19:45:08  damon
 * 	CR 329. Added include of param.h
 * 	[1992/11/05  19:44:51  damon]
 * 
 * Revision 1.1.2.2  1992/09/24  19:02:15  gm
 * 	CR282: Made more portable to non-BSD systems.
 * 	[1992/09/24  13:55:25  gm]
 * 
 * $EndLog$
 */
/*
 * how much space is free on filesystem containing "file"
 */

#ifndef lint
static char sccsid[] = "@(#)11  1.1  src/bldenv/sbtools/libode/porting/free_disk_space.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:10";
#endif /* not lint */

#include <ode/util.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#ifndef NO_STATVFS
#include <sys/statvfs.h>
#else
#include <sys/mount.h>
#endif
#ifdef INC_VFS
#include <sys/vfs.h>
#endif
#ifdef INC_STATFS
#include <sys/statfs.h>
#endif

int
free_disk_space( int fd, char *fname )
{
#ifndef NO_STATVFS
	struct statvfs sfs;
#else
#ifndef NO_FSTATFS
	struct statfs sfs;
#else
	struct fs_data sfs;
#endif
#endif

#ifndef NO_STATVFS
	if (fstatvfs(fd, &sfs) < 0)
#else
#ifndef NO_FSTATFS
	if (fstatfs(fd, &sfs) < 0)
#else
	if (statfs(fname, &sfs) <= 0)
#endif
#endif
		return(-1);
#if !defined(NO_STATVFS) || !defined(NO_FSTATFS)
#ifdef USE_FRSIZE
	return(sfs.f_bavail * sfs.f_frsize);
#else
#ifdef USE_BSIZE
	return(sfs.f_bavail * sfs.f_bsize);
#else
	return(sfs.f_bavail * sfs.f_fsize);
#endif
#endif
#else
	return(sfs.fd_req.bfreen * sfs.fd_req.bsize);
#endif
}
