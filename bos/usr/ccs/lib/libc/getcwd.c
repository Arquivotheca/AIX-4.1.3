static char sccsid[] = "@(#)59	1.2  src/bos/usr/ccs/lib/libc/getcwd.c, libcenv, bos411, 9428A410j 1/12/93 11:15:00";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: getcwd
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1992
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
#include <errno.h>
#include <limits.h>
#include <sys/types.h>		/* for size_t */

#define	TRUE	1
#define	FALSE	0

extern int errno;
extern char *getwd();
extern char *strcpy();
extern char *malloc();		/* until ansi C		*/

/*
 * NAME:	getcwd
 *
 * FUNCTION:	getcwd - get current working directory
 *
 * NOTES:	Getcwd gets the current working directory.
 *
 *		`buf' is a pointer to a character buffer into which the
 *		path name of the current directory is placed by the
 *		subroutine.  `buf' may be NULL, in which case the 
 *		subroutine will call malloc to get the required space.
 *		`size 'is the length of the buffer space for the path-name.
 *		If the actual path-name is longer than (size-2), or if
 *		the value of size is not at least 3, the subroutine will
 *		return a value of NULL, with errno set as appropriate.
 *
 * RETURN VALUE DESCRIPTION:
 *		NULL if anything fails, else
 *		`buf' if it is non-null, else
 *		a pointer to the malloc'ed memory containing the path
 */

/* The function getcwd() contains all the POSIX requirements */

char *
getcwd(char *buf, size_t size)
{
	int malloced = FALSE;		/* did we malloc space?	*/
	char tmpbuf[PATH_MAX + 1];

/* check lowest size possible */
/* POSIX states that if the size argument is less than or equal to 0, then
 * EINVAL is returned.
 */

	if(size <= 0) {
		errno = EINVAL;
		return(NULL);
	}

	/* null buffer? */
	if(buf == NULL) {
		/* yep, allocate some space using 'malloc' */
		malloced = TRUE;
		/*
		 * we cast malloc's return value to (char *) here
		 * because ANSI C says malloc returns (void *)
		 * (look in string.h)
		 */
		if((buf = (char *) malloc((size_t) size)) == NULL) {
			errno = ENOMEM;
			return(NULL);
		}
	}

	/* get the current directory... */
	if (getwd(tmpbuf) != NULL) {
		/*
		 * check length...  the manual says the size of the
		 * buffer passed must be 2 greater than the size we
		 * need...
		 */

/* POSIX states that if the size arg is greater than 0, but is smaller than 
 * the length of the pathname + 1, ERANGE is returned
 */

		if (size < (strlen(tmpbuf) + 1) && size > 0)
			{
			if (malloced == TRUE)
				free(buf);
			errno = ERANGE;
			return(NULL);
			}
		return(strcpy(buf, tmpbuf));
	}
	return(NULL);
}
