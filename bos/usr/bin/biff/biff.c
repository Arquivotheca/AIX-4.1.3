static char sccsid[] = "@(#)26	1.9  src/bos/usr/bin/biff/biff.c, cmdmisc, bos411, 9428A410j 11/23/93 09:10:32";
/*
 * COMPONENT_NAME: (CMDMISC) miscellaneous commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26, 27
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
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <nl_types.h>	/* needed by message catalogue routines */
#include <string.h>	/* needed by strchr() */
#include <locale.h>	/* needed by setlocal() */
#include <regex.h> 	/* needed by rpmatch() */
#include "biff_msg.h"

#define MSGSTR(Num, Str) catgets(catd, MS_BIFF, Num, Str) 

char	*ttyname();
extern int errno;

main(argc, argv)
	int argc;
	char **argv;
{
	char *progname = argv[0];
	char *cp = ttyname(2);
	struct stat stb;
	nl_catd catd;
	int ret;

	(void)setlocale(LC_ALL, "");
	catd = catopen(MF_BIFF, NL_CAT_LOCALE);
	argc--, argv++;
	if (cp == 0) {
		fprintf(stderr, "%s\n",
		   MSGSTR(WHERE, "Where are you?"));
		exit(1);
	}
	if (stat(cp, &stb) < 0)
		strerror(errno), exit(1);

	if (argc == 0) {
		printf("%s %s\n",  MSGSTR(IS, "is"),
		stb.st_mode&0100 ? MSGSTR(YES, "y") : MSGSTR(NO, "n"));
		exit((stb.st_mode&0100) ? 0 : 1);
	}

	/* obtain response to yes/no query */
	ret = rpmatch(argv[0]);
	if(ret == 1) {
		/* positive response */
		if (chmod(cp, stb.st_mode|0100) < 0)
			strerror(errno);
	} else if(ret == 0) {
		/* negative response */
		if (chmod(cp, stb.st_mode&~0100) < 0)
			strerror(errno);
	} else if(ret <0) {
		/* no match response */
		fprintf(stderr,
		 MSGSTR(USAGE, "usage: %s [y] [n]\n"),
		 progname);
	}

	exit((stb.st_mode&0100) ? 0 : 1);
}
