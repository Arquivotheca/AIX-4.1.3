static char sccsid[] = "@(#)92	1.2  src/bos/usr/ccs/lib/libc/raise.c, libcproc, bos411, 9437C411a 9/14/94 15:29:36";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: raise
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<sys/types.h>

/*
 *
 * FUNCTION: sends the signal sig to the executing program
 *
 * PARAMETERS: 
 *	     int    sig - signal to be sent
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     - returns 0 if successful
 *	     - returns nonzero if unsuccessful
 *
 */


#ifdef _THREAD_SAFE
	/* Get raise from libpthreads.a */
#else /* _THREAD_SAFE */
int
raise(int sig)
{
	pid_t pid;
	pid = getpid();

	if (kill(pid,sig))
		return(-1);
	else
		return(0);
}
#endif /* _THREAD_SAFE */
