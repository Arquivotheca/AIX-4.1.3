#ifndef __SIGNAL
static char sccsid[] = "@(#)32	1.1  src/bos/usr/ccs/lib/libc/signal.c, libcsig, bos411, 9428A410j 2/27/91 14:56:50";
#endif
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: signal, sigset, sigignore
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

#include <signal.h>
#include <errno.h>

/*
 * NOTES: This file is included by the _signal.c file with __SIGNAL defined
 *	to create _signal().  See _signal.c for more details.
 */


/*
 *
 * FUNCTION: sets action for a signal, compatibility interface to sigaction()
 *
 * NOTES: signal() allows only a subset of the function of sigaction().
 *	  It is provided for compatibility with old source code, and
 *	  for comformance to POSIX and ANSI "C" standards.  
 *
 * RETURN VALUES: 
 *	SIGERR         => failed, errno is set to specify the cause of failure
 *      errno = EINVAL => invalid signal number
 *	      = EINVAL => attempt to ignore or catch SIGKILL,SIGSTOP,SIGCONT
 *	Any other return value implies success, and the return value is
 *	a the previous signal action handler value (see sigaction()).
 */

void
#ifdef __SIGNAL
(*_signal(int signo, void (*fun)(int)))(int)
#else
(*signal(int signo, void (*fun)(int)))(int)
#endif
{
	struct sigaction act;	/* new signal action structure */
	struct sigaction oact;  /* returned signal action structure */ 

	/*   Setup a sigaction struct */

 	act.sa_handler = fun;        /* Handler is function passed */
	sigemptyset(&(act.sa_mask)); /* No signal to mask while in handler */
	act.sa_flags = SA_OLDSTYLE;  /* Set flags to indicate old System V
				      * type signal */
	
	/* use the sigaction() system call to set new and get old action */

	if(sigaction(signo, &act, &oact))
		/* If sigaction failed return SIG_ERR */
	    return(SIG_ERR);
	else
        	/* use the previous signal handler as a return value */
	    return(oact.sa_handler);
}
#ifndef __SIGNAL

void
(*sigset(int sig, void (*func)(int)))(int)
{
	struct sigaction nact;	/* new signal action structure */
	struct sigaction oact;  /* returned signal action structure */ 
	struct sigaction *nactp;	/* pointer to new action, or NULL */
	sigset_t set,oset;	/* new and old signal masks */
	int	rc;	/* return value from sigaction */


	/* Error handling put in here to trap sigset(SIGKILL,SIG_DFL) 
	 * and also to avoid return value checking for each call to
         * sigaddset,sigprocmask,sigismember .. */

	if( sig <= 0 || sig > SIGMAX || sig == SIGKILL )
		{
		errno=EINVAL;
		return(SIG_ERR);
		}
		
	sigprocmask(SIG_BLOCK,(sigset_t *) NULL,&oset);

	/* If function is SIG_HOLD mask this signal using sigprocmask
         * and set the SA_SIGSETSTYLE flag using sigaction to let the
	 * kernel know we are using the ATT sigset semantics        */

	if(func==SIG_HOLD)	
		{
		sigemptyset(&set);
		sigaddset(&set,sig);
		sigprocmask(SIG_BLOCK,&set,(sigset_t *) NULL);
		if(( rc=sigaction(sig,(struct sigaction *) NULL,&oact)) != -1)
			{
			if( ! (oact.sa_flags & SA_SIGSETSTYLE) )
				{
				oact.sa_flags = SA_SIGSETSTYLE;
				sigaction(sig,&oact,(struct sigaction *) NULL);
				}
			}
		}
	else
        	/*  Set new handler,set mask to ignore this signal when 
                 *  handler is executing and set flags to SA_SIGSETSTYLE */
		{
		nact.sa_handler = func;
		sigemptyset(&(nact.sa_mask));
		sigaddset(&(nact.sa_mask), sig);
		nact.sa_flags = SA_SIGSETSTYLE;
		rc=sigaction(sig,&nact,&oact);
		}

	if(rc == -1)
		return(SIG_ERR);

 	/* If previous action was SIG_HOLD return SIG_HOLD */
	if(sigismember(&oset,sig) == 1)  
		{
	        /* If new function installed clear the previous HOLD */
		if(func != SIG_HOLD)     
			{
			sigemptyset(&set);
			sigaddset(&set,sig);
			sigprocmask(SIG_UNBLOCK,&set,(sigset_t *) NULL);
  			}
		return(SIG_HOLD);
		}

	return (oact.sa_handler);
}


int
sigignore(int sig)
{
	struct sigaction nact;	/* new signal action structure */
	sigset_t set;		/* new signal mask */

	/* If the signal was set to SIG_HOLD release it */
	sigemptyset(&set);
	if( sigaddset(&set,sig) == -1)
		return(-1);
	sigprocmask(SIG_UNBLOCK,&set,(sigset_t *) NULL);
		
	nact.sa_handler = SIG_IGN;
	sigemptyset(&(nact.sa_mask));
	nact.sa_flags = 0;
	return (  sigaction(sig, &nact, (struct sigaction *)NULL)  ) ;

}
#endif /* __SIGNAL */
