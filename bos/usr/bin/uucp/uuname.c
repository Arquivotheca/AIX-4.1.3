static char sccsid[] = "@(#)36	1.11  src/bos/usr/bin/uucp/uuname.c, cmduucp, bos411, 9428A410j 1/6/94 10:11:06";
/* 
 * COMPONENT_NAME: CMDUUCP uuname.c
 * 
 * FUNCTIONS: Muuname 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	/sccs/src/cmd/uucp/s.uuname.c
	uuname.c	1.1	7/29/85 16:34:11
*/
#include "uucp.h"
/* VERSION( uuname.c	5.2 -  -  ); */
 
nl_catd catd;        
/*
 * returns a list of all remote systems.
 * option:
 *	-l	-> returns only the local system name.
 *	-c	-> returns remote systems accessible to cu
 */
main(argc,argv, envp)
int argc;
char **argv, **envp;
{
	register short lflg = 0;
	int c, cflg = 0;
	char s[BUFSIZ], prev[BUFSIZ], name[BUFSIZ];
	extern void setservice();

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP,NL_CAT_LOCALE);


	while ( (c = getopt(argc, argv, "lc")) != EOF )
		switch(c) {
		case 'c':
			cflg++;
			break;
		case 'l':
			lflg++;
			break;
		default:
			(void) fprintf(stderr, MSGSTR(MSG_UUNAME1,
			       "usage: uuname [-l | -c]\n"));
			exit(1);
		}
 
	if (lflg) {
		if ( cflg )
			(void) fprintf(stderr, MSGSTR(MSG_UUNAME4,
			"uuname: -l overrides -c ... -c option ignored\n"));
		uucpname(name);

		/* initialize to null string */
		(void) printf("%s",name);
		(void) printf("\n");
		exit(0);
	}
	if ( cflg )
		setservice("cu");
	else
		setservice("uucico");
	
	if ( sysaccess(EACCESS_SYSTEMS) != 0 ) {
		(void)fprintf(stderr, MSGSTR(MSG_UUNAME3,
		"uuname: cannot access protectd Systems files.\n"), argv[0]);
		exit(1);
	}

	while ( getsysline (s, sizeof(s)) ) {
		if((s[0] == '#') || (s[0] == ' ') || (s[0] == '\t') || 
		    (s[0] == '\n'))
			continue;
		(void) sscanf(s, "%s", name);
		if (EQUALS(name, prev))
		    continue;
		(void) printf("%s", name);
		(void) printf("\n");
		(void) strcpy(prev, name);
	}
	catclose(catd);

	exit(0);
}
/* small, private copies of assert(), logent(), */
/* cleanup() so we can use routines in sysfiles.c */

void assert(s1, s2, i1, file, line)
char *s1, *s2, *file; int i1, line;
{
	(void)fprintf(stderr, "uuname: %s %s\n", s2, s1);
}

void logent() {}

void cleanup(code)	{ exit(code);   }

