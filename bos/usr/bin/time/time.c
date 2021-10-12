static char sccsid[] = "@(#)28	1.19  src/bos/usr/bin/time/time.c, cmdstat, bos41J, 9507A 2/3/95 16:48:23";
/*
 * COMPONENT_NAME: (CMDSTAT) status
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<signal.h>
#include	<errno.h>
#include	<locale.h>
#include	<sys/wait.h>
#include	<sys/times.h>	/* times() / clock_t / struct tms */
#include	<unistd.h>		/* sysconf() */

#include "time_msg.h"
static nl_catd	catd;
#define MSGSTR(n,s)	catgets(catd,MS_TIME,n,s)

static void usage() 
{
	fprintf(stderr, MSGSTR(USAGE,"usage: time [-p] utility [argument...]\n"));
}

static void	printt( char * str, clock_t ticks );
static char errbuffer[BUFSIZ];

main( int argc, char **argv )
{
	struct tms	buffer;
	register int p;
	int	status;
	clock_t	before, after;
	int	optlet;		/* an option letter found in getopt() */

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_TIME,NL_CAT_LOCALE);
	setbuf(stderr,errbuffer);


	while ( (optlet = getopt(argc,argv,"p")) != EOF ) {
		switch ( optlet ) {
			case 'p' :
				/* "POSIX" format, which is the same as default */
				argc--;
				break;
			default :
				usage();
				exit(1);
		}
	}

	if(argc<=1) {
		usage();
		exit(1);
		}

	before = times(&buffer);
	p = fork();
	if(p == -1) {
		fprintf(stderr,MSGSTR(NOFORK,"time: cannot fork -- try again.\n"));
		exit(2);
	}
	if(p == 0) {
		execvp(argv[optind], &argv[optind]);
	        fprintf(stderr, "%s: %s\n", strerror(errno), argv[optind]);
		if (errno == EACCES)
			exit(126);
		else if (errno == ENOENT)
			exit(127);
		else
			exit(2);
	}
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	while(wait(&status) != p)
		;
	after = times(&buffer);
	if((status & 0377) != '\0')
		fprintf(stderr,MSGSTR(ABTERM, 
				"time: command terminated abnormally.\n"));
	fprintf(stderr,"\n");
	printt(MSGSTR(REAL,"real  "), (after-before));
	printt(MSGSTR(USER,"user  "), buffer.tms_cutime);
	printt(MSGSTR(SYS, "sys   "), buffer.tms_cstime);

	exit(WIFEXITED(status) ? WEXITSTATUS(status) : status);
}

/*
 *  NAME:  printt
 *
 *  FUNCTION:	prints out the string followed by a number.
 *
 *  RETURN VALUE:  	 void
 */

#define	PREC	2	/* number of digits following the radix character */

static void 
printt( char * str, clock_t ticks )
{
	float	seconds = (float)ticks / sysconf( _SC_CLK_TCK );

	fprintf( stderr, "%s %.*f\n", str, PREC, seconds );
}
