static char sccsid[] = "@(#)01	1.2  src/bos/usr/ccs/lib/libc/ssignal.c, libcproc, bos411, 9428A410j 10/20/93 14:31:43";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: ssignal, gsignal
 *
 * ORIGINS: 3,27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985,1993 
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

#include <signal.h>		/* for SIG_DFL */

/* Highest allowable user signal number */
#ifndef MAXSIG
#define MAXSIG 16
#endif /* MAXSIG */

/* Lowest allowable signal number (lowest user number is always 1) */
#ifndef MINSIG
#define MINSIG (-4)
#endif /* MINSIG */

#ifndef _THREAD_SAFE
/* Table of signal values */
static void (*sigs[MAXSIG-MINSIG+1])(int);
#endif

/*
 * NAME:	ssignal_r
 *                                                                    
 * FUNCTION:	set software signal
 *                                                                    
 * NOTES:	Ssignal_r sets a software signal.  The signal can
 *		either be set to SIG_DFL, SIG_IGN or can be set
 *		to call a function when the signal is raised.
 *
 * DATA STRUCTURES:	'sigs' is modified
 *
 * RETURN VALUE DESCRIPTION:	The previous value of the signal
 *		is returned.
 */  

void
#ifdef _THREAD_SAFE
(*ssignal_r(int sig, void (*fn)(int), void (*sigs[])(int)))(int)
#else
(*ssignal(int sig, void (*fn)(int)))(int)
#endif
{
	void (*savefn)(int);	/* previous value of 'sig'	*/

	if (sig >= MINSIG && sig <= MAXSIG) {
		savefn = sigs[sig-MINSIG];
		sigs[sig-MINSIG] = fn;
	} else
		savefn = SIG_DFL;

	return(savefn);
}

/*
 * NAME:	gsignal
 *                                                                    
 * FUNCTION:	gsignal - raise a software signal.
 *                                                                    
 * NOTES:	Gsignal 'raises' a software signal.  In effect, the
 *		software signal is then processed.  If the value of
 *		signal is currently a function, the function is called.
 *
 * RETURN VALUE DESCRIPTION:	0 if the value of the function is SIG_DFL,
 *		1 if the value is SIG_IGN, else call the function and
 *		return the value returned by the function.
 */  

int
#ifdef _THREAD_SAFE
gsignal_r(int sig, void (*sigs[])(int))
#else
gsignal(sig)
int sig;
#endif
{
	int (*sigfn)(int);

	if (sig < MINSIG || sig > MAXSIG
	 || (sigfn = (int (*)(int))sigs[sig-MINSIG]) == (int (*)(int))SIG_DFL)
		return(0);
	else if (sigfn == (int (*)(int))SIG_IGN)
		return(1);
	else {
		sigs[sig-MINSIG] = SIG_DFL;
		return( (*sigfn)(sig) );
	}
}
