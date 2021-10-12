static char sccsid[] = "@(#)91	1.3  src/bos/usr/ccs/lib/libc/psignal.c, libcproc, bos411, 9428A410j 11/10/93 15:26:41";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: psignal
 *
 * ORIGINS: 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993 
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

/*
 * FUNCTION: Print the name of the signal indicated along with the supplied message.
 *
 */
/*LINTLIBRARY*/


extern	char *sys_siglist[];

#include "libc_msg.h"
nl_catd catd;

psignal(sig, s)
	unsigned sig;
	char *s;
{
	register char *c;
	register n;

	catd = catopen(MF_LIBC, NL_CAT_LOCALE);

	if (sig < SIGMAX+1)
		c = catgets(catd, MS_LIBC, (int)(M_PSIGNAL + sig),
			sys_siglist[sig] );
	else
		c = catgets(catd, MS_LIBC, M_PSIGNAL + NSIG,
		 	"Unknown signal" );
	n = strlen(s);
	if (n) {
		write(2, s, n);
		write(2, ": ", 2);
	}
	write(2, c, strlen(c));
	write(2, "\n", 1);
}
