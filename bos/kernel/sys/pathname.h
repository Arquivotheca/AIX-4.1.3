/* @(#)56	1.7  src/bos/kernel/sys/pathname.h, syslfs, bos411, 9428A410j 3/23/94 10:12:12 */

#ifndef _H_PATHNAME
#define _H_PATHNAME

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 24
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* (#)pathname.h	1.3 86/12/18 NFSSRC */

#include <sys/limits.h>

/*
 * Pathname structure.
 * System calls which operate on path names gather the
 * pathname from system call into this structure and reduce
 * it by peeling off translated components.  If a symbolic
 * link is encountered the new pathname to be translated
 * is also assembled in this structure.
 */
struct pathname {
	char	*pn_path;		/* remaining pathname */
	uint	pn_pathlen;		/* remaining length */
	char	pn_buf[PATH_MAX+1];	/* pathname storage */
};

#define	pn_peekchar(PNP)	((PNP)->pn_pathlen!=0?*((PNP)->pn_path):0)
#define pn_pathleft(PNP)	((PNP)->pn_pathlen)
#define pn_free(PNP)

#ifdef notneeded
extern int	pn_getchar();		/* get next pathname char */
extern int	pn_alloc();		/* allocat buffer for pathname */
extern void	pn_free();		/* free pathname buffer */
#endif
extern int	pn_get();		/* allocate buf and copy path into it */
extern int	pn_set();		/* set pathname to string */
extern int	pn_combine();		/* combine to pathnames (for symlink) */
extern int	pn_getcomponent();	/* get next component of pathname */
extern void	pn_skipslash();		/* skip over slashes */
extern caddr_t	pnload();		/* get path from user area */

#endif /* _H_PATHNAME */
