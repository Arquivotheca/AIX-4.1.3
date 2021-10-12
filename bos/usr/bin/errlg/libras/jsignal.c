static char sccsid[] = "@(#)91	1.1  src/bos/usr/bin/errlg/libras/jsignal.c, cmderrlg, bos411, 9428A410j 3/2/93 09:00:56";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: jsignal
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Default signal handling routine.
 * Usually, main.c for each program will supply its own
 *  routine in order to clean up.
 * This routine is called by the kernel signal handler for signals
 *  set up by the signal() system call, usually in main.c .
 * The input is the number of the signal.
 * If the signal is not in the predefined list (in switch statement)
 *  the routime exits.
 * This routine is mainly for SIGINT for the parsers in
 *  errclear and errupdate in interactive mode. These routines will
 *  set the global flag 'setjmpinitflg' to tell jsignal that the
 *  global jmpbuf 'eofjmp' has been initialized. Note: This could probably
 *  be checked by looking at the contents of 'eofjmp' to see if
 *  non-zero.
 */

#include <signal.h>
#include <errlg.h>

int setjmpinitflg;

void jsignal(signo,arg)
{

	switch(signo) {
	case SIGSEGV:
		Debug("segment violation. address %x\n",arg);
		break;
	case SIGINT:
		signal(SIGINT,(void(*)(int)) jsignal);
		if(setjmpinitflg)
			mainreturn(EXCP_INT);
		break;
	default:
		Debug("Signal %d\n",signo);
	}
	genexit(signo);
}

