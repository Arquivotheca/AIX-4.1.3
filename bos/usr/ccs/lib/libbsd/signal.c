static char sccsid[] = "@(#)51	1.13  src/bos/usr/ccs/lib/libbsd/signal.c, libbsd, bos411, 9428A410j 7/5/90 16:26:09";
/*
 * COMPONENT_NAME: (LIBBSD)  Berkeley Compatibility Library
 *
 * FUNCTIONS: signal
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
 * NAME: signal()		(BSD)
 *
 * FUNCTION: sets action for a signal, compatibility interface to sigaction()
 *
 * NOTES: signal() allows only a subset of the function of sigaction().
 *	  It is provided for compatibility with old source code, and
 *	  for comformance to POSIX and ANSI "C" standards.
 *	  This (BSD) version of signal() "preserves" the existing signal
 *	  handler mask and action flags.
 *
 * RETURN VALUES:
 *	(int (*)())-1 => failed, errno is set to specify the cause of failure
 *      errno = EINVAL => invalid signal number
 *	      = EINVAL => attempt to ignore SIGKILL,SIGSTOP,SIGCONT
 *	      = EINVAL => attempt to catch SIGKILL,SIGSTOP
 *	Any other return value implies success, and the return value is
 *	the previous signal action handler value (see sigaction()).
 */

void  (*signal(int signo, void  (*fun)(int)))(int)
/* signo:    signal whose action is to be changed   */
/* fun  :    pointer to function or SIG_IGN/SIG_DFL */
{
	static sigset_t mask[NSIG];	/* remembered signal handler masks */
	static int 	flags[NSIG];	/* remembered signal action flags */

	struct sigaction act;	/* new signal action structure */
	struct sigaction oact;  /* returned signal action structure */
	void 	(*rc)(int);	/* return value */


	/* Signal bound checking is required to prevent an invalid    
	 * memory access while accessing the mask and flags arrays.
	 *
	 * SIGCONT handling is required here as BSD sigvec does not 
	 * allow this signal to be ignored, whereas POSIX sigaction
	 * does.
	 */
	if(signo <= 0 || signo >= NSIG || (signo==SIGCONT && fun==SIG_IGN))
	{
		errno=EINVAL;
		return(BADSIG);
	}

	act.sa_handler = (void (*)(int))fun;
	act.sa_mask = mask[signo];
	act.sa_flags = (flags[signo] & ~SA_OLDSTYLE) | SA_RESTART;


	/* use the sigaction() system call to set new and get old action */

	if ( sigaction(signo, &act, &oact) == 0 )
	{	
		rc = (void (*)(int))oact.sa_handler;


		/* if the "remembered" mask and flags were out of sync with
		 * real mask and flags, update "remembered" stuff and put
		 * mask and flags back to correct (unchanged) values */

		if ( (mask[signo].losigs != oact.sa_mask.losigs) ||
		     (mask[signo].hisigs != oact.sa_mask.hisigs) ||
	             (flags[signo] != oact.sa_flags) )
		{
			act.sa_mask = oact.sa_mask;
			mask[signo] = oact.sa_mask;

			/* Always forcing SA_RESTART in signal is incorrect
			 * however for release one we will let this pass.
			 * P36643.
			 */
			act.sa_flags = (oact.sa_flags & ~SA_OLDSTYLE) 
							| SA_RESTART;
			flags[signo] = act.sa_flags;
			sigaction(signo, &act,(struct sigaction *)NULL);
		}
	}
	else
		return(BADSIG);

	return (rc);
}

void  (*_signal(int signo, void  (*fun)(int)))(int)
{
return(signal(signo,fun));
}
