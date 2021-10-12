static char sccsid[] = "@(#)38	1.13  src/bos/usr/ccs/lib/libc/execvp.c, libcsys, bos411, 9428A410j 10/20/93 14:28:07";
#ifdef _POWER_PROLOG_
/*
 * COMPONENT_NAME: (LIBCSYS) Standard C Library System Functions 
 *
 * FUNCTIONS: execvp 
 *
 * ORIGINS: 3,26,27,71 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985,1993 
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

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */


#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>
#include "ts_supp.h"
#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex _exec_rmutex;
#endif

static char *execat(), shell[] = "/usr/bin/sh";
extern char **exec_argv();

/*
 * times to try when we get a busy text error...
 */
#define	TXTBUSY_TRIES	5

/*
 * NAME:	execvp
 *
 * FUNCTION:	execvp - exec a file, using argv, but search PATH
 *		for the file
 *
 * NOTES:	execvp(file, argv) execs a file, using argv as the argv
 *		for the new program.  PATH is searched for the file.
 *
 * RETURN VALUE DESCRIPTION:
 *		-1 is returned if the exec fails, if too many arguments
 *		were given, or on a memory allocation error.  Else this
 *		call does not return.
 */

int
execvp(const char *file, char *const argv[])
/* char	*file;				   file to exec	*/
/* char	**argv;				   its argv	*/
{
	char    fullname[PATH_MAX+1];	/* full filename to exec	*/
	char	**newargv;		/* new argv to use		*/
	int	argvct;			/* 'newargv' pointers allocated	*/
	int	argc;			/* counter for newargv		*/
	char	*cp;			/* Current Path component	*/
	int eacces = 0;			/* seen errno == EACCES yet?	*/
	unsigned int etxtbsy = 1;	/* errno == ETXTBSY counter	*/
        int     rc;

	/* P27191 */
	/* If PATH or the file argument is an empty string return with  *
	 * errno set to ENOENT.  For execlp and execvp need to handle   *
	 * empty file here.						*/
	if( *file == '\0' ) {
		errno = ENOENT;
		return (-1);
	}

	/* don't search the path if the file has a '/' in it ...	*/
	if (strchr(file, (int ) '/') != NULL)
		cp = "";
	else if ((cp = getenv("PATH")) == NULL)
		cp = ":/bin:/usr/bin";		/* default path */

        if (!TS_TRYLOCK(&_exec_rmutex)) {
                errno = EAGAIN;
                return(-1);
        }

	do {
		/* construct an absolute pathname to exec... */
		cp = execat(cp, file, fullname);
	retry:
		/* try to exec it */
		(void) execv(fullname, argv);

		/* we had an error.  figure out what it was ... */
		switch(errno) {

		case ENOEXEC:		/* exists, but not executable	*/
			/* maybe it's a shell script.  try a shell	*/

			/* count number of pointers in argv */
			for (argc = 0; argv[argc] != NULL; argc++);
			argc++;		/* include the NULL	*/

			/* get some memory to work with */
			for (argvct = 0; argvct < argc + 1; ) {
				if ((newargv = exec_argv(&argvct)) == NULL) {
					TS_UNLOCK(&_exec_rmutex);
					return (-1);
				}
			}

			/*
			 * copy the old argv in.  leave space for the shell
			 * and program name
			 */
			(void) memmove((void *)&newargv[2], (void *)&argv[1],
				(size_t)((argc - 1) * sizeof(argv[1])));
			if ((newargv[0] = strrchr(shell,(int ) '/')) == NULL)
				newargv[0] = shell;
			else
				newargv[0]++;
			newargv[1] = fullname;

			/* exec it */
                        rc = execv(shell, newargv);

			TS_UNLOCK(&_exec_rmutex);

                        return (rc);

		case ETXTBSY:		/* text was busy */
			/* try again? */
			if(++etxtbsy > TXTBUSY_TRIES) {
				TS_UNLOCK(&_exec_rmutex);
				return(-1);
				}
			(void) sleep(etxtbsy);
			goto retry;

		case EACCES:		/* permission denied... */
			/* set the flag, and try another path element */
			++eacces;
			break;

		case ENOMEM:		/* problems with the arguments */
		case E2BIG:
			TS_UNLOCK(&_exec_rmutex);
			/* don't try again */	
			return(-1);

		}
	/* loop while we still have path components left... */
	} while(cp != NULL);

	if(eacces)
		/* set errno back to EACCES in case we lost it */
		errno = EACCES;

	TS_UNLOCK(&_exec_rmutex);
	return(-1);
}

/*
 * NAME:	execat
 *
 * FUNCTION:	construct a full pathname to exec, using the filename
 *		and the next component of the path
 *
 * NOTES:	execat takes a path, a filename to exec and builds
 *		the full filename to exec using the next component
 *		of the path.  A pointer to the following path
 *		component is returned.
 *
 * RETURN VALUE DESCRIPTION:	NULL if we've reached the end of
 *		the path, else a pointer to the next path component
 */

static char *
execat(path, file, fullname)
char	*path;			/* path to use		*/
char	*file;			/* file to exec		*/
char	*fullname;		/* destination filename	*/
{
	char	*fp;		/* fullname pointer	*/

	fp = fullname;

	/* copy in the current component of path */
	while (*path && *path != ':')
		*fp++ = *path++;

	/* append a '/' if there was a path name to copy in */
	if (fullname != fp)
		*fp++ = '/';

	/* append the filename */
	while (*file)
		*fp++ = *file++;

	/* NULL terminate the fullname */
	*fp = '\0';

	/*
	 * return NULL if we've hit the end of the path,
	 * else return a pointer to the next component
	 */
	return (*path == '\0' ? NULL : ++path);
}
