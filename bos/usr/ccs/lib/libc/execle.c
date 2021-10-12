static char sccsid[] = "@(#)15	1.8  src/bos/usr/ccs/lib/libc/execle.c, libcsys, bos411, 9428A410j 1/12/93 11:13:37";
/*
 * COMPONENT_NAME: (LIBCSYS) Standard C Library System Functions 
 *
 * FUNCTIONS: execle 
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

#include <stdarg.h>

extern int execve(), exec_args();

/*
 * NAME:	execle
 *
 * FUNCTION:	Execle - exec a file using the arguments to
 *		construct an argv for the new program.  The
 *		environment is passed as the last argument.
 *
 * NOTES:	execle(path, arg0 [, arg1, ...], (char *) 0, envp)
 *		Use arg0, arg1... to construct the argv for the
 *		new program.  Use envp as the environment.  If this
 *		call succeeds, it will not return.
 *
 * RETURN VALUE DESCRIPTION:	-1 on a memory allocation error, or
 *		if too many arguments were passed, or if the exec
 *		failed.  Else this function does not return.
 */

int
execle(const char *path, ...)
{
	int error;		/* error status code	*/
	char **argv;		/* argv for `path'	*/
	char **envp;		/* envp for `path'	*/
	va_list args;		/* varargs args		*/

	/* set up the arg list...  */
	va_start(args, path);

	/* get the rest of the args into argv and envp ...  */
	error = exec_args(&argv, &envp, args);

	/* clean up the arg list...  */
	va_end(args);

	if (error == 0)
		/* do the exec...  */
		error = execve(path, argv, envp);

	return (error);
}
