static char sccsid[] = "@(#)60	1.3  src/bos/usr/ccs/lib/libc/getenv.c, libcenv, bos411, 9428A410j 4/7/94 14:40:38";
/*
 *   COMPONENT_NAME: LIBCENV
 *
 *   FUNCTIONS: _getenv
 *		getenv
 *
 *   ORIGINS: 3,26,27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/*
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <stdio.h>			/* for NULL */

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex	_environ_rmutex;
#endif	/* _THREAD_SAFE */

extern char **environ;			/* environment list		*/

/*
 * NAME:	getenv
 *                                                                    
 * FUNCTION:	return the value of an environment variable
 *                                                                    
 * NOTES:	Getenv searches the environment list for 'name', which
 *		is a string of the form "name=value".
 *
 * RETURN VALUE DESCRIPTION:	NULL if the enviroment variable is not
 *		found, else a pointer to the value.
 */  
/* The getenv() function includes all the POSIX requirements */

char *
getenv(const char *name)
{
	char **p;
	char *p_ptr, *n_ptr;
	TS_LOCK(&_environ_rmutex);

	if ((p = environ) != NULL)
		while (*p != NULL) {
			p_ptr=*p++;
			n_ptr=name;

			while (*n_ptr == *p_ptr++)
				if (*n_ptr++ == '=') {
					TS_UNLOCK(&_environ_rmutex);
					return(p_ptr);
					}

			if (*n_ptr == '\0' && *(p_ptr-1) == '=') {
			        TS_UNLOCK(&_environ_rmutex);
				return(p_ptr);
				}
			}

	TS_UNLOCK(&_environ_rmutex);
	return(NULL);
}

char *
_getenv(const char *name)
{
    return(getenv(name));
}

