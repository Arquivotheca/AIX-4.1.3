static char sccsid[] = "@(#)06	1.5  src/bos/usr/bin/mkfifo/mkfifo.c, cmdposix, bos41B, 9504A 12/19/94 12:25:21";
/*
 * COMPONENT_NAME: (CMDPOSIX) new commands required by Posix 1003.2
 *
 * FUNCTIONS: mkfifo
 *
 * ORIGINS: 27
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/mode.h>

#include <nl_types.h>
#include "mkfifo_msg.h"

#define MODE 	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH

static nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_MKFIFO,num,str) 

int usage(void);

main(int argc, char **argv)
{
	int ch;			 /* option holder */
	int errflg = 0;		 /* error indicator */
	char *modestring = NULL; /* user supplied mode of fifo */
	char s[PATH_MAX+30];

	(void) setlocale(LC_ALL,"");

	catd = catopen(MF_MKFIFO, NL_CAT_LOCALE);

	while((ch = getopt(argc, argv, "m:")) != EOF)
		switch(ch) {
		case 'm':		/* Specify mode */
			modestring = optarg;
			break;
		default:
			usage();
		}

	if (!argv[optind]) {
		fprintf(stderr, MSGSTR(NOFILE, "mkfifo: must specify file\n"));
		usage();
	}

        /*
	 * if a mode is supplied on the command line, convert
         * For each file argument, create a fifo.  If a failure 
	 * occurs, print error message and continue processing
         * files.
         */

	for (; argv[optind]; optind++) {

		if (mkfifo(argv[optind], MODE)) {
			fprintf(stderr,"mkfifo: ");
			perror(argv[optind]);
			errflg++;
		} else if (modestring) {
			sprintf(s, "%s %s %s", "/usr/bin/chmod", modestring, argv[optind]);
			system(s);
		}
	}

	exit(errflg);
}

static usage(void)
{
	fprintf(stderr, MSGSTR(USAGE, "Usage: mkfifo [-m mode] file ...\n"));
	exit(2);
}
