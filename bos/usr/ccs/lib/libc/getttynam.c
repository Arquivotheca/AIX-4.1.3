static char sccsid[] = "@(#)13	1.9  src/bos/usr/ccs/lib/libc/getttynam.c, libcio, bos411, 9428A410j 10/20/93 14:29:43";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: ENDTTYENT
 *		GETTTYENT
 *		SETTTYENT
 *		getttynam_r
 *
 *   ORIGINS: 26,27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/* getttynam.c,v $ $Revision: 2.7.2.2 $ (OSF) */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef _THREAD_SAFE
#include <stdio.h>
#endif	/* _THREAD_SAFE */

#include <ttyent.h>
#ifndef NULL
#define NULL 0
#endif /* NULL */

#include "ts_supp.h"


#ifdef _THREAD_SAFE

#define	SETTTYENT()	setttyent_r(&cur_tty)
#define	GETTTYENT(t)	(getttyent_r(line, &cur_tty, t) != TS_FAILURE)
#define	ENDTTYENT()	endttyent_r(&cur_tty)
#define TTY		(*tty)

#else

#define	SETTTYENT()	setttyent()
#define	GETTTYENT(t)	((t = getttyent()) != TS_FAILURE)
#define	ENDTTYENT()	endttyent()
#define TTY		tty

#endif	/* _THREAD_SAFE */


/*
 * NAME: getttynam
 *
 * FUNCTION: searches the tty file for the name passed to it.
 *
 * RETURN VALUE DESCRIPTION: 
 *	returns a pointer to an object with ttyent structure that contains
 *	tty in the name field or a null pointer if EOF is reached or error
 */

#ifdef _THREAD_SAFE
int
getttynam_r(const char *ttyname, struct ttyent *tty, char *line)
{
	FILE	*tty_fp = 0;
#else
struct ttyent *      
getttynam(const char *ttyname)
{
	static struct ttyent tty;
#endif	/* _THREAD_SAFE */

	register struct ttyent	*ttyent;
	int found = 0;

#ifdef _THREAD_SAFE
	struct ttyent temp;
	struct ttyent *cur_tty = NULL;
	ttyent = &temp;
#endif

	/**********
	  free up any space malloced last time
	**********/
	if (TTY.ty_name) {
	    free(TTY.ty_name);
	    TTY.ty_name = NULL;
	}
	if (TTY.ty_getty) {
	    free(TTY.ty_getty);
	    TTY.ty_getty = NULL;
	}
	if (TTY.ty_type) {
	    free(TTY.ty_type);
	    TTY.ty_type = NULL;
	}

	SETTTYENT();
	while (GETTTYENT(ttyent)) {
		if (strcmp(ttyname, ttyent->ty_name) == 0) {
			found = 1;
			break;
		}
	}

	if (found) {
	    /* malloc the space, if any mallocs fail, getttynam fails */
	    if ((TTY.ty_name = malloc(strlen(ttyent->ty_name) + 1)) == NULL) {
		ENDTTYENT();
		return(TS_NOTFOUND);
	    }
	    if ((TTY.ty_getty = malloc(strlen(ttyent->ty_getty) + 1)) == NULL) {
		ENDTTYENT();
		free(TTY.ty_name);
		TTY.ty_name = NULL;
		return(TS_NOTFOUND);
	    }
	    if ((TTY.ty_type = malloc(strlen(ttyent->ty_type) + 1)) == NULL) {
		ENDTTYENT();
		free(TTY.ty_name);
		TTY.ty_name = NULL;
		free(TTY.ty_getty);
		TTY.ty_getty = NULL;
		return(TS_NOTFOUND);
	    }

	    /*********
	      copy the information from the ttyent structure
	    *********/
	    strcpy(TTY.ty_name, ttyent->ty_name);
	    strcpy(TTY.ty_getty, ttyent->ty_getty);
	    strcpy(TTY.ty_type, ttyent->ty_type);
	    TTY.ty_status = ttyent->ty_status;
	}

	/**********
	  free up the space malloced by getttyent()
	**********/
	ENDTTYENT();

	if (found)
	    return(TS_FOUND(&tty));
	
	return (TS_NOTFOUND);
}

