static char sccsid[] = "@(#)26	1.18  src/bos/usr/ccs/lib/libc/cuserid.c, libcs, bos411, 9428A410j 4/20/94 17:39:05";
/*
 * COMPONENT_NAME: (LIBCS) Standard C Library System Security Functions
 *
 * FUNCTIONS: cuserid 
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

#include <stdio.h>
#include <usersec.h>

#include "ts_supp.h"
#include "push_pop.h"

#ifndef _THREAD_SAFE
static	char	name[L_cuserid];
#else
#include "rec_mutex.h"
extern	struct	rec_mutex _libs_rmutex;
#endif	/* _THREAD_SAFE */

char	*cuserid(char *s)
{
	char	*p;

	/* get user name associated with calling process */
	TS_LOCK(&_libs_rmutex);
	TS_PUSH_CLNUP(&_libs_rmutex);

	p = IDtouser(geteuid());

	TS_POP_CLNUP(0);

	if ( s == NULL)	/* buffer not provided */
	{	if (p == NULL)
			; /* user name not found, NULL will be returned. */
		else
		{
#ifdef _THREAD_SAFE
			/* Return user name in allocated buffer. */
			if ((s = (char *) malloc(L_cuserid)) != NULL)
				strcpy(s,p);
			/* else NULL will be returned */
#else
			/* Return user name in static buffer. */
			s = name;
                        strcpy(s,p);
#endif /* _THREAD_SAFE */
		}
	}
	else
	{	if (p == NULL)
			/* user name not found, store a NUL char */
			*s = '\0';
		else
			/* return user name in provided buffer */
			strcpy(s,p);
		/* always return pointer to provided buffer */
	}
	TS_UNLOCK(&_libs_rmutex);
	return s;
}
