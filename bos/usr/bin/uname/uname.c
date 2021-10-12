static char sccsid[] = "@(#)30  1.12.1.4  src/bos/usr/bin/uname/uname.c, cmdstat, bos41B, 9504A 12/21/94 13:41:40";
/*
 * COMPONENT_NAME: (CMDSTAT) status
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 18, 26, 27
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
 * uname: Displays the name of the current operating system.
 *
 *      If "UNAMEX" is #defined, then "-l" reports the LAN nid value
 *      and "-x" reports all eXtended info.
 *      Otherwise -l and -x are illegal; they are not ignored.
 *
 */
#define UNAMEX		1
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<sys/utsname.h>
#include	<locale.h>
#include	<errno.h>

#include "uname_msg.h"

static nl_catd	catd;
#define MSGSTR(Num, Str)  catgets(catd, MS_UNAME, Num, Str)

extern int  getopt( int argc, char* argv[], const char* options );
extern char *optarg;
		/* until getopt() is declared in a (standard) header file... */
extern int unameu( struct setuname* , short swflg);
extern int unamex( struct xutsname* );
		/* until unameX() is declared in a (standard) header file... */

static int setname( char *newname ,short swflg);
static struct	utsname	unstr;

main(argc, argv)
char **argv;
int argc;
{
struct	utsname	*un = &unstr;
	int     sflg=1, nflg=0, lflg=0, rflg=0, vflg=0, mflg=0, errflg=0;
	int	optlet, setflg = 0;
	int	fields = 0;	/* Are there any previous output fields? */
	short	swflg = 0;

#ifdef UNAMEX
	static char *options = "snlrvmaxS:T:";
#else
	static char *options = "snrvmaS:T:";
#endif
	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_UNAME, NL_CAT_LOCALE);

	while((optlet=getopt(argc, argv, options)) != EOF) switch(optlet) {
		case 'a':
			sflg++; nflg++; rflg++; vflg++; mflg++;
			break;
		case 'x':
			sflg++; nflg++; lflg++; rflg++; vflg++; mflg++;
			break;
		case 's':
			sflg++;
			break;
		case 'n':
			nflg++;
			break;
		case 'r':
			rflg++;
			break;
		case 'l':
			lflg++;
			break;
		case 'v':
			vflg++;
			break;
		case 'm':
			mflg++;
			break;
		case 'S':
			setflg++;
			swflg++;
			break;
		case 'T':
			setflg++;
			break;
		case '?':
			errflg++;
	}
	if(errflg) {
		fprintf(stderr,MSGSTR(USAGE, "usage: uname [-%s]\n"), options);
		exit(1);
	}

	if(setflg) {
		if(sflg > 1 || nflg || lflg || vflg || rflg || mflg || (setflg > 1)) {
			fprintf(stderr,MSGSTR(STONLY,"S or T must be only flag \n"));
			exit(1);
		}
		exit( setname(optarg,swflg) );	/* just set and exit */
	}

	if (uname(un) < 0) {
		perror (MSGSTR(UNAMEFAILD,"uname system call failed"));
		exit (-1);
	}

	if(nflg | rflg | lflg | vflg | mflg) sflg--;
	if(sflg) {
		fields++;
		fprintf(stdout, "%.*s", SYS_NMLN, un->sysname);
	}
	if(nflg) {
		if ( fields++ ) putchar(' ');
		fprintf(stdout, "%.*s", SYS_NMLN, un->nodename);
	}
#ifdef UNAMEX
	/* after all other fields: for compilance for POSIX 1003.2 D10. */
	if(lflg) {
		struct xutsname xunstr;

		if (unamex(&xunstr) < 0) {
		       perror(MSGSTR(UNAMEXFAILD,"unamex system call failed"));
		       exit (-1);
		}
		if ( fields++ ) putchar(' ');
		fprintf(stdout, "%u", xunstr.nid);
	}
#endif
	if(rflg) {
		if ( fields++ ) putchar(' ');
		fprintf(stdout, "%.*s", SYS_NMLN, un->release);
	}
	if(vflg) {
		if ( fields++ ) putchar(' ');
		fprintf(stdout, "%.*s", SYS_NMLN, un->version);
	}
	if(mflg) {
		if ( fields++ ) putchar(' ');
		fprintf(stdout, "%.*s", SYS_NMLN, un->machine);
	}
	putchar('\n');
	exit(0);
}

/*
 *  NAME:  setname
 *
 *  FUNCTION:  Set the machine's node name as <newname>,
 *             or set's the machine's system name as <newname>.
 *		Only the root user can use this function.
 *	      
 *  RETURN VALUE: value for exit().
 *			0 ... successfully set
 *			1 ... any kind of error
 */

static	int
setname( char *newname, short swflg )
{
struct	setuname sets;
	unsigned int  len;

	if (geteuid()) {
		fprintf(stderr, MSGSTR(PERMST,"Permissions denied for -S or -T\n"));
		return	1;
	}
	sets.len = len = strlen(newname);
	sets.target = UUCPNODE;
	if(swflg){		/* change nodename */
		if (len > sizeof(unstr.nodename)) {
			fprintf(stderr,
			MSGSTR(TOOBIG,"node name too long:maximum = %d characters.\n"),
				sizeof(unstr.nodename));
			return	1;
		}
	} else {		/* change systemname */
		if (len > sizeof(unstr.sysname)) {
			fprintf(stderr,
			MSGSTR(TOOBIGS,"system name too long:maximum = %d characters.\n"),
				sizeof(unstr.sysname));
			return	1;
		}
	}
	if ( (sets.newstr = malloc(len + 1)) == NULL) {
		fprintf(stderr, MSGSTR(MALFAIL,"malloc failed\n"));
		return	1;
	}
	if (!strcpy(sets.newstr,newname)) {
		fprintf(stderr, MSGSTR(STRFAIL,"copy to setuname failed\n"));
		return	1;
	}
	errno = 0;
	if (unameu(&sets,swflg) < 0) {
		fprintf(stderr,
			MSGSTR(FAILU,"uname: failed error = %d\n"),errno);
		return	1;
	}
	return	0;
}
