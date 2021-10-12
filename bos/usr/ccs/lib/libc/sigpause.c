static char sccsid[] = "@(#)99	1.1  src/bos/usr/ccs/lib/libc/sigpause.c, libcproc, bos411, 9428A410j 2/26/91 17:48:06";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: sigpause
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <signal.h>


/*
 * NAME: sigpause()		(BSD/AIX)  
 *
 * FUNCTION: change the set of blocked signals, and wait for any unblocked sig
 *
 * NOTES: sigpause() allows only a subset of the function of sigsuspend().
 *	  It is provided for compatibility with old source code.  
 *	  Only signals of value 1-32 can be blocked using sigpause();
 *	  siguspend() must be used if signal values 33-64 need to be
 *	  blocked during the wait for a signal.
 *        The sigpause() subroutine uses the sigsuspend() system call.
 *
 * WARNING: This source code is dependent on the type of sigset_t
 *	since it does not use the sig...set functions to form the
 *	set of signals to be blocked.
 *
 * RETURN VALUES: 
 *	-1 with errno = EINTR => this is a successful return, there are
 *	no other return values.  
 */ 

int sigpause(mask)
int	mask;	/* mask whose bits correspond to signals 0-32 */
{
	int	rc;		/* return code from call to sigsuspend() */

	sigset_t set;		/* new signal set for sigprocmask() */

	set.losigs = (ulong)mask;
	set.hisigs = 0;
	sigdelset(&set,SIGCONT);	/* BSD cantmask includes SIGCONT */
	rc = sigsuspend(&set);
	return (rc);
}
