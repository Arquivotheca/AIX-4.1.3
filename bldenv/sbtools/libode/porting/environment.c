/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: _findenv
 *		defined
 *		getenv
 *		setenv
 *		unsetenv
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
 * $Log: environment.c,v $
 * Revision 1.2.2.3  1992/12/03  17:21:30  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:36  damon]
 *
 * Revision 1.2.2.2  1992/09/24  19:02:09  gm
 * 	CR282: Made more portable to non-BSD systems.
 * 	[1992/09/23  18:23:02  gm]
 * 
 * Revision 1.2  1991/12/05  20:44:50  devrcs
 * 	Added _FREE_ to copyright marker
 * 	[91/08/01  08:04:48  mckeen]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  16:08:28  mckeen]
 * 
 * 	Initial revision for project ODE
 * 	[91/01/14  11:40:02  mckeen]
 * 
 * 	Kill multiple occurrence of Endlog, HISTORY, or copyright header
 * 	[91/02/04  18:27:03  robert]
 * 
 * 	Initial revision for project ODE
 * 	[91/01/14  11:40:02  mckeen]
 * 
 * Revision 1.3  90/07/27  10:22:46  devrcs
 * 	More portability changes.
 * 	[90/07/23  12:32:17  gm]
 * 
 * 	More gcc cleanup.
 * 	[90/07/12  13:55:13  gm]
 * 
 * Revision 1.2  90/01/03  11:48:22  gm
 * 	Fixes for first snapshot.
 * 	[90/01/03  09:24:12  gm]
 * 
 * Revision 1.2  89/12/26  10:32:02  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * $EndLog$
 */
/*
 * Copyright (c) 1987 Regents of the University of California.
 * This file may be freely redistributed provided that this
 * notice remains attached.
 */

#ifndef lint
static char sccsid[] = "@(#)03  1.1  src/bldenv/sbtools/libode/porting/environment.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:53";
#endif /* not lint */

#if defined(LIBC_SCCS) && !defined(lint)
static char rcsid[] = "@(#)environment.c	1.1 (OSF) 9/12/89";
#endif /* LIBC_SCCS and not lint */

#if defined(LIBC_SCCS) && !defined(lint)
static char rcsid[] = "@(#)getenv.c	5.4 (Berkeley) 3/13/87";
#endif /* LIBC_SCCS and not lint */

/*
 * Copyright (c) 1987 Regents of the University of California.
 * This file may be freely redistributed provided that this
 * notice remains attached.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char rcsid[] = "@(#)setenv.c	1.2 (Berkeley) 3/13/87";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <stdio.h>

extern char **environ;
extern char *malloc(), *realloc();
static char *_findenv();

/*
 * setenv(name,value,rewrite)
 *	Set the value of the environmental variable "name" to be
 *	"value".  If rewrite is set, replace any current value.
 */
setenv(name,value,rewrite)
	register char	*name,
			*value;
	int	rewrite;
{
	static int	alloced;		/* if allocated space before */
	register char	*C;
	int	l_value,
		offset;

	if (*value == '=')			/* no `=' in value */
		++value;
	l_value = strlen(value);
	if ((C = _findenv(name,&offset))) {	/* find if already exists */
		if (!rewrite)
			return(0);
		if (strlen(C) >= l_value) {	/* old larger; copy over */
			while (*C++ = *value++);
			return(0);
		}
	}
	else {					/* create new slot */
		register int	cnt;
		register char	**P;

		for (P = environ,cnt = 0;*P;++P,++cnt);
		if (alloced) {			/* just increase size */
			environ = (char **)realloc((char *)environ,
			    (u_int)(sizeof(char *) * (cnt + 2)));
			if (!environ)
				return(-1);
		}
		else {				/* get new space */
			alloced = 1;		/* copy old entries into it */
			P = (char **)malloc((u_int)(sizeof(char *) *
			    (cnt + 2)));
			if (!P)
				return(-1);
			memcpy(P,environ,cnt * sizeof(char *));
			environ = P;
		}
		environ[cnt + 1] = NULL;
		offset = cnt;
	}
	for (C = name;*C && *C != '=';++C);	/* no `=' in name */
	if (!(environ[offset] =			/* name + `=' + value */
	    malloc((u_int)((int)(C - name) + l_value + 2))))
		return(-1);
	for (C = environ[offset];(*C = *name++) && *C != '=';++C);
	for (*C++ = '=';*C++ = *value++;);
	return(0);
}

/*
 * unsetenv(name) --
 *	Delete environmental variable "name".
 */
void
unsetenv(name)
	char	*name;
{
	register char	**P;
	int	offset;

	while (_findenv(name,&offset))		/* if set multiple times */
		for (P = &environ[offset];;++P)
			if (!(*P = *(P + 1)))
				break;
}

/*
 * getenv(name) --
 *	Returns ptr to value associated with name, if any, else NULL.
 */
char *
getenv(name)
	char *name;
{
	int	offset;

	return(_findenv(name,&offset));
}

/*
 * _findenv(name,offset) --
 *	Returns pointer to value associated with name, if any, else NULL.
 *	Sets offset to be the offset of the name/value combination in the
 *	environmental array, for use by setenv(3) and unsetenv(3).
 *	Explicitly removes '=' in argument name.
 *
 *	This routine *should* be a static; don't use it.
 */
static
char *
_findenv(name,offset)
	register char *name;
	int	*offset;
{
	register int	len;
	register char	**P,
			*C;

	for (C = name,len = 0;*C && *C != '=';++C,++len);
	for (P = environ;*P;++P)
		if (!strncmp(*P,name,len))
			if (*(C = *P + len) == '=') {
				*offset = P - environ;
				return(++C);
			}
	return(NULL);
}
