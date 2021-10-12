static char sccsid[] = "@(#)54	1.5  src/bos/usr/ccs/lib/libc/execlp.c, libcsys, bos411, 9428A410j 10/20/93 14:28:03";
/*
 * COMPONENT_NAME: (LIBCSYS) Standard C Library System Functions 
 *
 * FUNCTIONS: execlp 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985,1993 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdarg.h>

#include <errno.h>
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include <rec_mutex.h>
extern struct rec_mutex _exec_rmutex;
#endif /* _THREAD_SAFE */

extern int execvp(), exec_args();

/*
 * NAME:	execlp
 *
 * FUNCTION:	exec a file, using the arguments to construct an
 *		argv for the new file, and PATH to find the file
 *		to exec.
 *
 * NOTES:	execlp(file, arg0 [, arg1, ...], (char *) 0)
 *		execs a file like 'execl', except that the
 *		PATH is searched for the file to execute.
 *
 * RETURN VALUE DESCRIPTION:	-1 if a memory allocation occurs,
 *		if too many arguments were given, or if the exec
 *		fails.  Else execlp does not return.
 */

int
execlp(const char *file, const char *arg0,  ...)
{
	int error;		/* error status code	*/
	char **argv;		/* argv for `program'	*/
	va_list args;		/* varargs args		*/

	if (!TS_TRYLOCK(&_exec_rmutex) ) {
		errno = EAGAIN;
		return(-1);
	}

	/* set up the arg list...  */
	va_start(args, file);

	/* get the rest of the args into argv...  */
	error = exec_args(&argv, (char ***) NULL, args);

	/* clean up the arg list...  */
	va_end(args);

	if (error == 0)
		/* do the exec...  */
		error = execvp(file, argv);

	TS_UNLOCK(&_exec_rmutex);

	return (error);
}
