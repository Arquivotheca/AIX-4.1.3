static char sccsid[] = "@(#)79  1.23  src/bos/usr/ccs/lib/libc/getpass.c, libcs, bos411, 9428A410j 3/14/94 14:56:07";
/*
 * COMPONENT_NAME: (LIBCS) Standard C Library System Security Functions 
 *
 * FUNCTIONS: getpass 
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
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

/*LINTLIBRARY*/
#include <stdio.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>

static int intrupt;
static void catch(int);

char *
getpass(prompt)
const char	*prompt;
{
	struct termios ttyb;
	tcflag_t flags;
	register char *p;
	register int c;
	FILE	*fi, *outfp;
	static char pbuf[PASS_MAX+1];
	struct sigaction nintact, oact_sigint, oact_sigquit;

	nintact.sa_handler = catch;
	sigemptyset(&(nintact.sa_mask));
	nintact.sa_flags=0;

	/* if tty cannot be opened, use stderr and stdin so that */
	/* programs which have input piped via stdin, such as    */
	/* crypt, will work correctly.                           */

	if((fi = outfp = fopen("/dev/tty", "r+")) == NULL) {
		outfp = stderr;
		fi = stdin;
	}

	setbuf(fi, (char*)NULL);

	intrupt = 0;
	sigaction (SIGINT, &nintact, &oact_sigint);
	sigaction (SIGQUIT, &nintact, &oact_sigquit);

	/* get current settings */
	if (tcgetattr (fileno(fi), &ttyb) < 0) 
                goto out;

	flags = ttyb.c_lflag;
	ttyb.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);

	/* set new attributes */
	if (tcsetattr (fileno(fi), TCSANOW, &ttyb) < 0 ||
	    tcflush (fileno(fi), TCIFLUSH))
                goto out;

	(void) fputs (prompt, stderr);
	rewind (stderr);	/* implied flush */
	for(p=pbuf; !intrupt && (c = getc(fi)) != '\n' && c != EOF; ) {
		if(p < &pbuf[PASS_MAX])
			*p++ = c;
	}
	*p = '\0';
	(void) putc ('\n', stderr);

	ttyb.c_lflag = flags;
	(void) tcsetattr(fileno(fi), TCSANOW, &ttyb);

	sigaction (SIGINT, &oact_sigint, NULL);
	sigaction (SIGQUIT, &oact_sigquit, NULL);
	if (fi != stdin)
		(void) fclose (fi);
	if (intrupt) 
		(void) kill (getpid(), intrupt);
	return (pbuf);

out :
        sigaction (SIGINT, &oact_sigint, NULL);
        sigaction (SIGQUIT, &oact_sigquit, NULL);
        if (fi != stdin)
                (void) fclose (fi);
        return ((char *)NULL);
}

static void
catch(int sig)
{
	intrupt = sig;
}
