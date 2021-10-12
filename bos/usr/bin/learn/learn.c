static char sccsid[] = "@(#)17 1.6  src/bos/usr/bin/learn/learn.c, cmdlearn, bos411, 9428A410j 11/15/93 09:31:52";
/*
 * COMPONENT_NAME: (CMDLEARN) provides computer-aided instruction courses
 *
 * FUNCTIONS: hangup, intrpt
 *
 * ORIGINS: 26, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
*/

#include "stdio.h"
#include "lrnref.h"
#include "signal.h"
#include <locale.h>
#include "learn_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LEARN,n,s) 

char	*direct	= "/usr/share/lib/learn";	/* CHANGE THIS ON YOUR SYSTEM */
int	more = 1;
char	*level;
int	speed = 0;
char	*sname;
char	*todo;
FILE	*incopy	= NULL;
int	didok;
int	sequence	= 1;
int	comfile	= -1;
int	status;
int	wrong;
char	*pwline;
char	*dir;
FILE	*scrin;
int	logging	= 0;	/* set to 0 to turn off logging */
int	ask;
int	again;
int	skip;
int	teed;
int	total;

main(argc,argv)
int argc;
char *argv[];
{
	extern hangup(void), intrpt(void);
	extern char * getlogin(), *malloc();
	(void) setlocale(LC_ALL, "");
	catd = catopen(MF_LEARN, NL_CAT_LOCALE);
	pwline = getlogin();
	selsub(argc, argv);
	chgenv();
	signal(SIGHUP, (void (*)(int))hangup);
	signal(SIGINT, (void (*)(int))intrpt);
	while (more) {
		selunit();
		dounit();
		whatnow();
	}
	wrapup(0);
}

hangup(void)
{
	wrapup(1);
}

intrpt(void)
{
	char response[LINE_MAX+1], *p;

	signal(SIGINT, (void (*)(int))hangup);
	write(2, MSGSTR(LINTERRUPT, "\nInterrupt.\nWant to go on?  "), strlen(MSGSTR(LINTERRUPT, "\nInterrupt.\nWant to go on?  ")));
	p = response;
	*p = 'n';
	while (read(0, p, 1) == 1 && *p != '\n')
		p++;
	*p = NULL;
	if (rpmatch(response) != 1)
		wrapup(0);
	ungetwc(L'\n', stdin);
	signal(SIGINT, (void (*)(int))intrpt);
}
