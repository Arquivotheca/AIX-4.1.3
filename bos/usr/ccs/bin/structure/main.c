static char sccsid[] = "@(#)95	1.5  src/bos/usr/ccs/bin/structure/main.c, cmdprog, bos411, 9428A410j 3/9/94 13:21:58";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: main, dexit
 *
 * ORIGINS: 26 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <signal.h>
#include <stdio.h>
#include <locale.h>
#include "1.defs.h"
#include "def.h"

nl_catd catd;

char (*input)(), (*unput)();
FILE *outfd	= stdout;



main(argc,argv)
int argc;
char *argv[];
	{
	int anyoutput;
	int dexit(void);
	char *getargs();
	char input1(), unput1(), input2(), unput2();

	setlocale(LC_ALL, "");
#ifdef MSG
	catd = catopen(MF_STRUCT, NL_CAT_LOCALE);
#endif
	anyoutput = FALSE;
	getargs(argc,argv);
	if (debug == 2) debfd = stderr;
	else if (debug)
		debfd = fopen("debug1","w");

	if (signal(SIGINT, SIG_IGN) !=SIG_IGN)
		signal(SIGINT,(void(*)(int)) dexit);
	prog_init();

	for (;;)
		{
		++routnum;
		routerr = 0;

		input = input1;
		unput = unput1;
		if (!mkgraph()) break;
		if (debug) prgraph();
		if (routerr) continue;

		if (progress)fprintf(stderr,MSGSTR(BUILD, "build:\n")); /*MSG*/
		build();
		if (debug) prtree();
		if (routerr) continue;

		if (progress)fprintf(stderr,MSGSTR(STRUCTURE, "structure:\n")); /*MSG*/
		structure();
		if (debug) prtree();
		if (routerr) continue;
		input = input2;
		unput = unput2;

		if (progress)fprintf(stderr,MSGSTR(OUTPUT, "output:\n")); /*MSG*/
		output();
		if (routerr) continue;
		anyoutput = TRUE;
		freegraf();
		}
	if (anyoutput)
		exit(0);
	else
		exit(1);
	}


dexit(void)
	{
	exit(1);
	}
