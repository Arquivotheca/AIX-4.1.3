static char sccsid[] = "@(#)31	1.8.1.6  src/bos/usr/bin/nice/nice.c, cmdcntl, bos41B, 9504A 1/4/95 10:12:58";
/*
 * COMPONENT_NAME: (CMDCNTL) system control commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 */

/*
 *   The nice command lets you  run the specified command at a
 *   lower priority (or higher if you are the superuser).
 */                                                                   

#include	<stdio.h>
#include	<ctype.h>
#include 	<nl_types.h>
#include	<locale.h>
#include 	"nice_msg.h"
#include	<errno.h>
static nl_catd	catd;
#define	MSGSTR(Num,Str) catgets(catd,MS_NICE,Num,Str)

main(argc, argv)
int argc;
char *argv[];
{
	int	nicarg = 10;
	char	*p, *nicarg_str;
	int	nflg = 0;
	char    *end_ptr;


	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_NICE,NL_CAT_LOCALE);
	if (argc < 2)
		usage();
	if (strcmp(argv[1],"--") == 0) {
		argv++;
		argc--;
		if (argc < 2) 
			usage();
	}
	else {
  	     if (argv[1][0] == '-') {
		p = argv[1];
		if (*++p == 'n') {
			if (p[1] == '\0') {
			    argc--;
			    argv++;
			    nicarg_str = argv[1];
			    p = argv[1];
			} else {
			    p++;
			    nicarg_str = p;
			}
			if(*p == '-')
			    p++;
			nflg++;
		}
		else {
			nicarg_str = &argv[1][1];
			if (*p == '-')
		       	    p++;
		};
		nicarg = strtol(nicarg_str, &end_ptr, 10);
 		if ((*end_ptr != '\0') || (strlen(nicarg_str) == 0)) {
    			fprintf(stderr,MSGSTR(MSGBNUM, "nice: argument must be numeric.\n")); 
			usage();
		}
		argc--;
		argv++;
            }

	    if (argc < 2)
		usage();
	    if (strcmp(argv[1],"--") == 0) {
		argv++;
		argc--;
		if (argc < 2)
			usage();
	     }	
	}

	(void) nice(nicarg);	/* Set priority of this process. */
				/* We don't care if it failed.  */
	execvp(argv[1], &argv[1]);
	perror (strcat("nice: ", argv[1]));
	/*
   	 * exit status: 
   	 * 127 if utility is not found.
   	 * 126 if utility cannot be invoked 
	 */
        exit(errno == ENOENT ? 127 : 126);

}

static usage()
{
        fprintf(stderr, MSGSTR(USAGE1,
                "usage: nice [-n increment] command\n"));
        fprintf(stderr, MSGSTR(USAGE2,
                "usage: nice [-increment] command\n"));
        exit(2);
}

