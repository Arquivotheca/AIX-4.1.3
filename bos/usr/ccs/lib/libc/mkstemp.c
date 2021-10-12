static char sccsid[] = "@(#)63	1.4  src/bos/usr/ccs/lib/libc/mkstemp.c, libcenv, bos411, 9428A410j 9/9/93 14:50:09";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: mkstemp
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
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

#include <stdio.h>		/* for NULL		*/
#include <sys/access.h>		/* for access(,F_OK)...	*/
#include <stdlib.h>
#include <sys/file.h>

extern int  access(), getpid();
extern char *__lucky();			/* see stdio/DStemp.c */

/*
 * NAME:	mkstemp
 *
 * FUNCTION:	mkstemp - construct a unique filename and return the open
 *			  file descriptor.
 *
 * NOTES:	Mktemp expects a string of length at least 6, with
 *		six trailing 'X's, and overwrites the X's with a
 *		(hopefully) unique encoding of the process' pid and
 *		a pseudo-random number.
 *
 * RETURN VALUE DESCRIPTION:
 *		Returns an open file descriptor upon success. Otherwise
 *		returns -1 if no suitable file could be created. 
 */

int mkstemp(template)		
char *template;
{
	int rv;
	char *tmp; 

	tmp = mktemp(template);
	if (tmp == NULL || *tmp == '\0') 
		return(-1);
	rv = open(template, O_CREAT|O_EXCL|O_RDWR, 0600);
	if (rv<0)
		*template = '\0';
	return (rv);
}
