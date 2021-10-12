static char sccsid[] = "@(#)54	1.13  src/bos/usr/bin/nohup/nohup.c, cmdcntl, bos41J, 9514A_all 4/3/95 16:23:10";
/*
 * COMPONENT_NAME: (CMDCNTL) system control commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 27, 18
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 *
 */

/*
 * Nohup runs commands, ignoring all hangups and quit signals.  If
 * no output is specified, output is redirected to nohup.out.
 */                                                                   

#include <stdio.h>
#include <nl_types.h>
#include "nohup_msg.h"
static nl_catd catd;
#define MSGSTR(n,s)	catgets(catd,MS_NOHUP,n,s)
#include <locale.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static char	nout[PATH_MAX] = "nohup.out";

main(argc, argv)
char **argv;
{
	char	*home;
	FILE *temp;
	mode_t  mask;           /* mode mask */

	(void) setlocale (LC_ALL,"");
        catd = catopen(MF_NOHUP,NL_CAT_LOCALE);
	if(argc < 2) {
		fprintf(stderr,MSGSTR(USAGE,"usage: nohup command [arg ...]\n"));
		exit(127);
	}
	if(strcmp(argv[1],"--") == 0) {
		if (argc == 2) {
			fprintf(stderr,MSGSTR(USAGE,"usage: nohup command [arg ...]\n"));
			exit(127);
		} else {
			argv++;
			argc--;
		}
	}
	argv[argc] = 0;
	signal(SIGHUP, SIG_IGN); 
	if(isatty(1)) {
		mask = umask(0);
		(void)umask(066);
		if( (temp = fopen(nout, "a")) == NULL) {
			if((home=getenv("HOME")) == NULL) {
				fprintf(stderr,MSGSTR(NOCREAT,"nohup: cannot open/create nohup.out\n"));
				exit(127);
			}
			strcpy(nout,home);
			strcat(nout,"/nohup.out");
			if(freopen(nout, "a", stdout) == NULL) {
				fprintf(stderr,MSGSTR(NOCREAT,"nohup: cannot open/create nohup.out\n"));
				exit(127);
			}
		}
		else {
			fclose(temp);
			freopen(nout, "a", stdout);
		}
		(void)umask(mask);
		fprintf(stderr,MSGSTR(SENDOUT,"Sending output to %s\n"), nout);
	}
	if(isatty(2)) {
		close(2);
		dup(1);
	}
	execvp(argv[1],&argv[1]);

	/* It failed, so print an error */
	freopen("/dev/tty", "w", stderr);
	fprintf(stderr,"%s: %s: %s\n", argv[0], argv[1], strerror(errno));
	/*
	 * exit status:
	 * 127 if utility is not found.
	 * 126 if utility cannot be invoked 
	 */
        exit(errno == ENOENT ? 127 : 126);
}
