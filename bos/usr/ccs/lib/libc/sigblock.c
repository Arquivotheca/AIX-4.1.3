static char sccsid[] = "@(#)94	1.2  src/bos/usr/ccs/lib/libc/sigblock.c, libcproc, bos411, 9428A410j 5/22/91 09:59:31";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: sigblock, sigsetmask
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
#include <errno.h>


/*
 * NAME: sigblock()		(BSD/AIX)  
 *
 * FUNCTION: add a set of signals to the process's currently blocked signals
 *
 * NOTES: sigblock() allows only a subset of the function of sigprocmask().
 *	  It is provided for compatibility with old source code.  
 *	  Only signals of value 1-31 can be added to the already blocked
 *	  signals using sigblock(); sigprocmask() must be used if signal
 *	  values 32-63 need to be added to the set of blocked signals.
 *        The sigblock() subroutine uses the sigprocmask() system call.
 *
 * WARNING: This source code is dependent on the type of sigset_t
 *	since it does not use the sig...set functions to form the
 *	sigprocmask signal sets.
 *
 * RETURN VALUES: 
 *	-1 => failed, errno is set to specify the cause of failure
 *      errno = EPERM => calling process tryed to block SIGSAK w/o privilege
 *
 *	If a -1 is not returned, then sigblock() was successful, and the
 *	return value is a mask indicating which of the signals 1-31 were
 *	previously blocked.
 */ 

int sigblock(mask)
int	mask;	/* mask whose bits correspond to signals 1-31 */
{
	int	rc;		/* save return value from sigprocmask() */

	sigset_t nset;		/* new signal set for sigprocmask() */
	sigset_t oset;		/* structure to return old signal set */

	nset.losigs = (ulong)mask;
	nset.hisigs = 0;
	sigdelset(&nset,SIGCONT);	/* BSD does not allow SIGCONT masked */
	rc = sigprocmask(SIG_BLOCK, &nset, &oset);
	if ( rc == 0 )
		rc = oset.losigs;
	return (rc);
}
/*
 * NAME: sigsetmask()		(BSD/AIX)  
 *
 * FUNCTION: set the process's currently blocked signals
 *
 * NOTES: sigsetmask() allows only a subset of the function of sigprocmask().
 *	  It is provided for compatibility with old source code.  
 *	  Only signals of value 1-31 can be blocked using sigsetmask();
 *	  sigprocmask() must be used to block signal values 32-63.
 *        The sigsetmask() subroutine uses the sigprocmask() system call.
 *
 * WARNING: This source code is dependent on the type of sigset_t
 *	since it does not use the sig...set functions to form the
 *	sigprocmask signal sets.
 *
 * RETURN VALUES: 
 *	-1 => failed, errno is set to specify the cause of failure
 *      errno = EPERM => calling process tryed to block SIGSAK w/o privilege
 *
 *	If a -1 is not returned, then sigsetmask() was successful, and the
 *	return value is a mask indicating which of the signals 1-31 were
 *	previously blocked.  
 */ 
int sigsetmask(mask)
int	mask;	/* mask whose bits correspond to signals 1-31 */
{
	int	rc;		/* save return value from sigprocmask() */

	sigset_t nset;		/* new signal set for sigprocmask() */
	sigset_t oset;		/* structure to return old signal set */

	nset.losigs = (ulong)mask;
	nset.hisigs = 0;
	sigdelset(&nset,SIGCONT);	/* BSD does not allow SIGCONT masked */
	rc = sigprocmask(SIG_SETMASK, &nset, &oset);
	if ( rc == 0 )
		rc = oset.losigs;
	return (rc);
}
/*
 * NAME: sighold()		(ATT/AIX)  
 *
 * FUNCTION: add a signal to the process's currently blocked signals
 *
 * NOTES: sighold() allows only a subset of the function of sigset().
 *        The sighold() subroutine uses the sigset() subroutine.
 *
 * RETURN VALUES: 
 *	-1 => failed, errno is set to specify the cause of failure
 *       0 => success.
 *
 */ 
int sighold(sig)
int	sig;	/* sig whose bit correspond to signals 1-31 */
{
	if(sigset(sig,SIG_HOLD) == SIG_ERR)
		return(-1);
	else
		return(0);
}
/*
 * NAME: sigrelse()		(ATT/AIX)  
 *
 * FUNCTION: remove a signal to the process's currently blocked signals
 *
 * NOTES: sigrelse() allows only a subset of the function of sigprocmask().
 *
 *
 * RETURN VALUES: 
 *	-1 => failed, errno is set to specify the cause of failure
 *       0 => success.
 *
 */ 
int sigrelse(sig)
int	sig;	/* sig whose bit correspond to signals 1-31 */
{
    sigset_t nset;		/* new signal set for sigprocmask() */
    sigset_t oset;		/* structure to return old signal set */


    /* Return error if invalid signal */
    if (sig < SIGHUP || sig > SIGUSR2 || sig == SIGKILL)
    {
	errno=EINVAL;
	return(-1);
    }
    sigemptyset(&nset);
    sigaddset(&nset, sig);
    return (sigprocmask(SIG_UNBLOCK, &nset, &oset));
}
