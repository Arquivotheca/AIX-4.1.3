static char sccsid[] = "@(#)43	1.6  src/bos/usr/bin/tip/tipout.c, cmdtip, bos411, 9428A410j 10/17/90 13:10:31";
/* 
 * COMPONENT_NAME: UUCP tipout.c
 * 
 * FUNCTIONS: intUSR1, intIOT, intSYS, intTERM, sigmask, tipout 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* static char sccsid[] = "tipout.c	5.1 (Berkeley) 4/30/85"; */

#include "tip.h"
/*
 * tip
 *
 * lower fork of tip -- handles passive side
 *  reading from the remote host
 */

static	jmp_buf sigbuf;

/*
 * TIPOUT wait state routine --
 *   sent by TIPIN when it wants to posses the remote host
 */
void
intIOT(int s)
{

	write(repdes[1],&ccc,1);
	read(fildes[0], &ccc,1);
	longjmp(sigbuf, 1);
}

/*
 * Scripting command interpreter --
 *  accepts script file name over the pipe and acts accordingly
 */
void
intUSR1(int s)
{
	char c, line[256];
	register char *pline = line;
	char reply;

	read(fildes[0], &c, 1);
	while (c != '\n') {
		*pline++ = c;
		read(fildes[0], &c, 1);
	}
	*pline = '\0';
	if (boolean(value(SCRIPT)) && fscript != NULL)
		fclose(fscript);
	if (pline == line) {
		boolean(value(SCRIPT)) = FALSE;
		reply = 'y';
	} else {
		if ((fscript = fopen(line, "a")) == NULL)
			reply = 'n';
		else {
			reply = 'y';
			boolean(value(SCRIPT)) = TRUE;
		}
	}
	write(repdes[1], &reply, 1);
	longjmp(sigbuf, 1);
}

void
intTERM(int s)
{

	if (boolean(value(SCRIPT)) && fscript != NULL)
		fclose(fscript);
	exit(0);
}

void
intSYS(int s)
{

	boolean(value(BEAUTIFY)) = !boolean(value(BEAUTIFY));
	longjmp(sigbuf, 1);
}

#ifdef _AIX
/*
 * Macro for converting signal number to a mask suitable for
 * sigblock().
 */
#define sigmask(m)	(1 << ((m)-1))
#endif

/*
 * ****TIPOUT   TIPOUT****
 */
tipout()
{
	char buf[BUFSIZ];
	register char *cp;
	register int cnt;
	extern int errno;
	int omask;

	(void) setjmp(sigbuf);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
#ifdef _AIX221
	signal(SIGDANGER, (void(*)(int)) intEMT); /* attention from TIPIN */
#else
	signal(SIGUSR1, (void(*)(int)) intUSR1);  /* attention from TIPIN */
#endif
	signal(SIGTERM, (void(*)(int)) intTERM); /* time to go signal */
	signal(SIGIOT, (void(*)(int)) intIOT);   /* scripting going on signal */
	signal(SIGHUP, (void(*)(int)) intTERM);	 /* for dial-ups */
	signal(SIGSYS, (void(*)(int)) intSYS);	 /* beautify toggle */
	for (omask = 0;; sigsetmask(omask)) {
		cnt = read(FD, buf, BUFSIZ);
		if (cnt <= 0) {
			/* lost carrier */
			if (cnt < 0 && errno == EIO) {
				sigblock(sigmask(SIGTERM));
				intTERM(0);
				/*NOTREACHED*/
			}
			continue;
		}
#ifndef _AIX221
#define	ALLSIGS	sigmask(SIGUSR1)|sigmask(SIGTERM)|sigmask(SIGIOT)|sigmask(SIGSYS)
#else
#define	ALLSIGS	sigmask(SIGDANGER)|sigmask(SIGTERM)|sigmask(SIGIOT)|sigmask(SIGSYS)
#endif
		omask = sigblock(ALLSIGS);
		for (cp = buf; cp < buf + cnt; cp++)
			*cp &= pmask;
		write(1, buf, cnt);
		if (boolean(value(SCRIPT)) && fscript != NULL) {
			if (!boolean(value(BEAUTIFY))) {
				fwrite(buf, 1, cnt, fscript);
				continue;
			}
			for (cp = buf; cp < buf + cnt; cp++)
				if ((*cp >= ' ' && *cp <= '~') ||
				    any(*cp, value(EXCEPTIONS)))
					putc(*cp, fscript);
		}
	}
}
