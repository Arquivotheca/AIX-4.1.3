static char sccsid[] = "@(#)27	1.8  src/bos/usr/ccs/lib/libc/execv.c, libcsys, bos411, 9428A410j 1/12/93 11:13:46";
/*
 * COMPONENT_NAME: (LIBCSYS) Standard C Library System Functions 
 *
 * FUNCTIONS: execv 
 *
 * ORIGINS: 3, 27 
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
 */

extern int execve(char *path, char *argv[], char *envp[]);

extern	char	**environ;		/* environment	*/

/*
 * NAME:	execv
 *
 * FUNCTION:	execv(char *path, char *argv[])
 *
 * NOTES:	execv executes 'path' using 'argv' as the argv for
 *		'path'.  The environment is passed automatically.
 *		If this calls succeeds, it will not return, since
 *		the new program overlays the current program.
 *
 * RETURN VALUE DESCRIPTION:	the return value from execve if
 *		that call fails, else execv does not return
 */

int
execv(const char *path, char *const argv[])
{
	return (execve(path, argv, environ));
}
