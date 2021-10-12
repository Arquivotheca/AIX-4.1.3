static char sccsid[] = "@(#)55	1.4  src/bos/usr/ccs/lib/libc/exect.c, libcsys, bos411, 9428A410j 6/16/90 01:33:14";
/*
 * COMPONENT_NAME: (LIBCSYS) Standard C Library System Functions 
 *
 * FUNCTIONS: exect 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/reg.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <sys/types.h>

extern int ptrace(int request, pid_t pid, int *addr, int data, int *buff);

/*
 * NAME:	exect
 *
 * FUNCTION:	exect - execute a file in trace mode
 *
 * NOTES:	Exect executes a file in trace mode.  Ptrace() is called
 *		to put the current process into trace mode.
 *
 * RETURN VALUE DESCRIPTION:	-1 if either ptrace() or execve() fails,
 *		else this routine does not return
 *
 */

int
exect(char *name, char *argv[], char *envp[])
{
	int status;

	/*
	 * note that ptrace() ignores the other parameters
	 * when called with PT_TRACE_ME...
	 */
	if ((status = ptrace(PT_TRACE_ME, (pid_t)0, (int *)0,0, (int *)0)) == 0)
		status = execve(name, argv, envp);

	return (status);
}
