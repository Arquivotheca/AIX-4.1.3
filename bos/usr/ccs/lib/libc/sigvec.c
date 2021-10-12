static char sccsid[] = "@(#)00	1.1  src/bos/usr/ccs/lib/libc/sigvec.c, libcproc, bos411, 9428A410j 2/26/91 17:48:10";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: sigvec
 *
 * ORIGINS: 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <signal.h>
#include <errno.h>


/*
 * NAME: sigvec()		(BSD/AIX)  
 *
 * FUNCTION: sets action for a signal, compatibility interface to sigaction()
 *
 * NOTES: sigvec() allows only a subset of the function of sigaction().
 *	  It is provided for compatibility with old source code.  
 *	  Only signals of value 1-31 can be blocked by the signal handler
 *	  mask set up using sigvec(); sigaction() must be used if signal
 *	  values 32-63 need to be blocked by a signal handler mask.
 *        The sigvec() subroutine uses the sigaction() system call.
 *
 *	  This LIBCGEN sigvec is for AIX 5A compatiblity and always
 *	  results in interruptable system calls, i.e it always unsets the
 *        SA_RESTART bit in the flags.
 *
 * WARNING: This source code is dependent on the type of sigset_t
 *	since it does not use the sig...set functions to form the
 *	sigaction sa_mask.
 *
 * RETURN VALUES: 
 *	0 => successful
 *	-1 => failed, errno is set to specify the cause of failure
 *      errno = EFAULT => invalid addresses specified for invec or outvec
 *            = EINVAL => invalid signal number
 *	      = EINVAL => attempt to ignaore or catch SIGKILL,SIGSTOP,SIGCONT
 *
 *	Upon return, if "outvec" is non-NULL, the sigvec structure pointed to
 *	by "outvec" will contain the previous signal action information.
 */
int sigvec(signo, invec, outvec)
int	signo;		/* signal whose action is to be changed */
struct	sigvec *invec;	/* structure containing desired action information */ 
struct	sigvec *outvec; /* structure in which to return previous action */
{
	struct sigaction act;	/* new sigaction info for sigaction() call */
	struct sigaction oact;	/* old sigaction info from sigaction() call*/
	struct sigaction *actp;	/* pointer to new sigaction, or NULL */
	int	rc;		/* save return value from sigaction() */

	/* if new action is requested, move action info to sigaction struct */
	if ( invec != NULL)
	{
		/* BSD does not allow SIGCONT,SIGKILL and SIGSTOP to be ignored */
		/* Handle SIGCONT here, sigaction will take care of the other two */
		/* P30158 */
		if((signo == SIGCONT) && (invec->sv_handler == SIG_IGN))
			{
			errno=EINVAL; 
			return(-1);
			}

		act.sa_handler = (void (*)(int))invec->sv_handler;

		act.sa_mask.losigs = (unsigned long int) invec->sv_mask;
		act.sa_mask.hisigs = 0;      
 		/* BSD does not allow SIGCONT,SIGKILL and SIGSTOP to be masked */
	        /* Handle SIGCONT here ,SIGKILL and SIGSTOP are handled in sigaction */
		/* P30158 */
		sigdelset(&act.sa_mask,SIGCONT);
	
		/* This forces usage of LIBCGEN sigvec to ignore SV_INTERRUPT bit 
		   and always result in  interruptable system calls */
		/* P30158 */
		act.sa_flags = invec->sv_flags & ~SA_RESTART;

		actp = &act;
	}
	/* don't change action if no new action was requested */
	else
		actp = NULL;

	/* use the sigaction system call to change signal action and/or
	   get current signal action information */
	rc = sigaction(signo, actp, &oact);

	/* if previous action info is requested, move it to sigvec struct */
	if ( outvec != NULL )
	{
		outvec->sv_handler = (void (*)(int))oact.sa_handler;
		outvec->sv_mask = (int) oact.sa_mask.losigs;
		outvec->sv_flags = oact.sa_flags;
	}
	
	/* return the value that was returned from the sigaction system call */
	return (rc);
}
