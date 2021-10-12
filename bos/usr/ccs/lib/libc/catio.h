/* @(#)87	1.18  src/bos/usr/ccs/lib/libc/catio.h, libcmsg, bos411, 9428A410j 5/13/94 14:08:03 */
#ifdef _POWER_PROLOG_
/*
 * COMPONENT_NAME: (LIBCMSG) LIBC Message Catalog Functions
 *
 * FUNCTIONS: catio.h 
 *
 * ORIGINS: 27,71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#endif /* _POWER_PROLOG_ */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */



#include <mesg.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifdef _THREAD_SAFE
#define RETURN(s) return(TS_UNLOCK(&_catalog_rmutex), errno=errno_save, (s))
#else /* _THREAD_SAFE */
#define RETURN(s) return(errno=errno_save, (s))
#endif /* _THREAD_SAFE */

extern	int	_cat_do_open(nl_catd);
extern	void	_cat_hard_close(nl_catd);
extern	FILE	*_cat_openfile(char *);

#define PATH_FORMAT	"/usr/lib/nls/msg/%L/%N:/usr/lib/nls/msg/%L/%N.cat"

#define FILE_UNUSED	((FILE *)-1)
#define FILE_DUMMY	((FILE *)-2)
