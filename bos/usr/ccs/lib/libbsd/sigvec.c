static char sccsid[] = "@(#)62	1.11  src/bos/usr/ccs/lib/libbsd/sigvec.c, libbsd, bos411, 9428A410j 6/23/90 11:53:54";
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
 *        This is the LIBBSD version of sigvec and is BSD 4.3 compatible.
 *        It recognizes the SV_INTERRUPT bit in flags and sets the      
 *        SA_RESTART bit appropriatly. 
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
 *	      = EINVAL => attempt to ignore or catch SIGKILL,SIGSTOP,SIGCONT
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
		/* BSD doesn't allow SIGCONT,SIGKILL, SIGSTOP to be ignored */
		/* Handle SIGCONT here, let sigaction take care of the others */
		if((signo == SIGCONT) && (invec->sv_handler == SIG_IGN))
			{
			errno=EINVAL; 
			return(-1);
			}
		act.sa_handler = (void (*)(int))invec->sv_handler;

		act.sa_mask.losigs = (unsigned long int) invec->sv_mask;
		act.sa_mask.hisigs = 0;      
 		/* BSD does not allow SIGCONT,SIGKILL, SIGSTOP to be masked */
	        /* Handle SIGCONT here, let sigaction take care of the others */
		sigdelset(&act.sa_mask,SIGCONT);
	
		/* SV_INTERRUPT and SA_RESTART are the same bit with 
		 * complementary meanings so if SV_INTERRUPT bit is set
		 * turn off SA_RESTART else turn it on. 
		 * Since this is BSD sigvec unconditionally turn off
		 * the OLDSTYLE flag.
		 */

		act.sa_flags = ( invec->sv_flags ^ SA_RESTART ) & ~SA_OLDSTYLE ;

		/* Actually all bits not used by BSD should have been
		 * masked out here, SA_SIGSETSTYLE ... .
		 */

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
		outvec->sv_flags = oact.sa_flags ^ SA_RESTART;
	}
	
	/* return the value that was returned from the sigaction system call */
	return (rc);
}
