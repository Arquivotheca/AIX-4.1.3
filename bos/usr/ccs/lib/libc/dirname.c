static char sccsid[] = "@(#)06	1.10  src/bos/usr/ccs/lib/libc/dirname.c, libcadm, bos411, 9428A410j 3/4/94 10:24:49";
/*
 *   COMPONENT_NAME: (LIBCADM) Standard C Library System Admin Functions 
 *
 *   FUNCTIONS: dirname, dirname_r 
 *
 *   ORIGINS: 27,71
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */
/*
 * dirname.c,v $ $Revision: 1.4.2.4 $ (OSF) $Date: 92/01/03 17:40:19 $";
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include "ts_supp.h"
/*LINTLIBRARY*/

#ifdef _THREAD_SAFE
int
dirname_r(char *path, char *buf, int size)
#else
char *
dirname(char *path)
#endif	/* _THREAD_SAFE */
{
	static char dot_return[2] = {"."};
	register char	*cp;
	size_t		length;
#ifndef _THREAD_SAFE
	static char	*buf = NULL;
	static int	size = 0;
#endif	/* _THREAD_SAFE */

	TS_EINVAL((buf == NULL || size <= 0));

	/**********
	  all of the special cases
	**********/
	if (!path || !*path || (strcmp(path, "..") == 0) || (strcmp(path, ".") == 0)) {
#ifdef _THREAD_SAFE
		strncpy(buf, dot_return, size);
#endif /* _THREAD_SAFE */
		return (TS_FOUND(dot_return));
	}
	/*
	** find end of string
	*/
	for (cp = path; *cp; cp++)
		;
	cp--;
	/*
	** lop off trailing slashes
	*/
	while (cp > path && *cp == '/')
		cp--;
	/*
	** find the next last slash
	*/
	while (cp > path && *cp != '/')
		cp--;
	/*
	** if no directory, return "."
	*/
	if (cp == path && *cp != '/') {
#ifdef _THREAD_SAFE
		strncpy(buf, dot_return, size);
#endif /* _THREAD_SAFE */
		return (TS_FOUND(dot_return));
	}
	/*
	** skip slash sequence
	*/
	while (cp > path && *cp == '/')
		cp--;
	cp++;

	if ((length = cp - path) >= size) {
#ifdef _THREAD_SAFE
		errno = EINVAL;
		return (TS_FAILURE);
#else
		if (buf)
			free(buf);
		size = length + 1;
		if (!(buf = malloc(size)))
			return (TS_FAILURE);
#endif	/* _THREAD_SAFE */
	}

	buf[length] = '\0';
	strncpy(buf, path, length);
	return (TS_FOUND(buf));
}
