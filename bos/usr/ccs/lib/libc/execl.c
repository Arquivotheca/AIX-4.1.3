static char sccsid[] = "@(#)04	1.9  src/bos/usr/ccs/lib/libc/execl.c, libcsys, bos411, 9428A410j 12/13/93 14:06:20";
/*
 * COMPONENT_NAME: (LIBCSYS) Standard C Library System Functions 
 *
 * FUNCTIONS: execl 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1992
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

extern int exec_args();
extern int execv();

/*
 * NAME:	execl
 *
 * FUNCTION:	execute a file, using the arguments to execl
 *		as a list of arguments to pass to the exec'ed program
 *
 * NOTES:	execl(path, arg0 [, arg1, ...], (char *) 0)
 *		executes a file.  Argv to the exec'ed program is
 *		constructed from the arguments passed to execl.
 *		The environment is automatically passed.  If the
 *		call to execv succeeds, it will not return, since
 *		the calling program overlays the current program.
 *
 * RETURN VALUE DESCRIPTION:	return -1 on a malloc error or
 *		if execve() returns.  else it does not return.
 */

int
execl(const char *path, ...)
{
	int error;		/* error status code	*/
	char **argv;		/* argv for `path'	*/
	va_list args;		/* varargs args		*/

	if (!TS_TRYLOCK(&_exec_rmutex) ) {
		errno = EAGAIN;
		return(-1);
	}

	/* set up the arg list...  */
	va_start(args, path);

	/* get the rest of the args into argv...  */
	error = exec_args(&argv, (char ***) NULL, args);

	/* clean up the arg list...  */
	va_end(args);

	if (error == 0)
		/* do the exec...  */
		error = execv(path, argv);

	TS_UNLOCK(&_exec_rmutex);

	return (error);
}
