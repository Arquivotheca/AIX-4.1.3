static char sccsid[] = "@(#)74	1.7.1.3  src/bos/usr/bin/hostname/hostname.c, cmdnet, bos411, 9428A410j 6/8/94 09:07:28";
/* 
 * COMPONENT_NAME: TCPIP hostname.c
 * 
 * FUNCTIONS: Mhostname, printit, usage 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
#ifndef lint
char copyright[] =
"Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "hostname.c	5.1 (Berkeley) 4/30/85";
#endif not lint
*/

/*
 * hostname -- get (or set hostname)
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <sys/stat.h>

#include <nl_types.h>
#include "hostname_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_HOSTNAME,n,s) 

#include <locale.h>

#define MAXDNAME	256  	/* maximum domain name	*/
#define PIODMGR_PATH	"/usr/lib/lpd/pio/etc/piodmgr"
#define PIODMGR_FLAGS	" -h >/dev/null 2>&1"

char *cmd_name;
static void usage();

main(argc,argv)
int  argc;
char *argv[];

{

extern int optind;
int ch, sflag = 0;
char *p, hostname[MAXDNAME];

	setlocale(LC_ALL,"");
	catd = catopen(MF_HOSTNAME,NL_CAT_LOCALE);
	cmd_name = argv[0];
	if (argc > 3)
		usage();

	while ((ch = getopt(argc, argv, "s")) != EOF)
		switch((char)ch) {
			case 's':
				sflag = 1;
				break;
			case '?':
			default:
				usage();
		}
	argv += optind;

	if (*argv) {  /* set it */
		struct stat fst;

		if (sethostname(*argv,strlen(*argv))) {  /* error */
			perror(MSGSTR(SETHOST,"sethostname"));
			exit(1);
		}

		/* Update the print queue database to reflect the new
		   hostname. */
		if (stat(PIODMGR_PATH,&fst) != -1)
			(void)system(PIODMGR_PATH PIODMGR_FLAGS);

	}

	/* print it in either case */

	if(gethostname(hostname,sizeof(hostname))){
		perror(MSGSTR(GETHOSTNAME, "gethostname"));
		exit(1);
	}
	if(sflag && (p = (char *)index(hostname, '.')))  /* trim domain name */
		*p = '\0';
	puts(hostname);

	exit(0);
}

static void usage()
{
        printf(MSGSTR(USAGE, "usage: %s [-s] [hostname]\n"), cmd_name ); /*MSG*/
	exit(1);
}
