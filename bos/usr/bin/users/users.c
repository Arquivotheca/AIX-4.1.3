static char sccsid[] = "@(#)79  1.6  src/bos/usr/bin/users/users.c, cmdstat, bos41B, 9504A 12/21/94 13:41:44";
/*
 * COMPONENT_NAME: (CMDSTAT) status
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26, 27
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
 * users - List the login names of the users currently on
 *         the system in a compact, one-line format.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <utmp.h>
#include <locale.h>
#include <sys/types.h>

#include        "users_msg.h"
static nl_catd catd;
#define MSGSTR(num,str)  catgets(catd,MS_USERS,num,str)


#define NMAX sizeof(utmp.ut_name)
#define LMAX sizeof(utmp.ut_line)

#define MAXUSERS	200

static struct utmp utmp;
static char	*names[MAXUSERS];
static char	**namp = names;
static size_t	ncnt;			/* count of names */

static void putline( void );
static int  scmp(char **p, char **q);
static void summary( void );


main(argc, argv)
char **argv;
int argc;
{
	register char *s;
	register FILE *fi;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_USERS,NL_CAT_LOCALE);

	s = "/etc/utmp";
	if (argc == 2)
		s = argv[1];
	if ((fi = fopen(s, "r")) == NULL) {
		perror(s);
		exit(1);
	}
	ncnt = 0;
	while (fread((void *)&utmp, (size_t)sizeof(utmp), (size_t)1, fi) == 1) {
		if (utmp.ut_name[0] == '\0' || utmp.ut_type != USER_PROCESS)
			continue;
		if (++ncnt > MAXUSERS) {
			ncnt = MAXUSERS;
			fputs(MSGSTR(TOO_MANY, "users: too many users.\n"),
			      stderr);
			break;
		}
		putline();
	}
	summary();
	exit(0);
}

/*
 *  NAME:  putline
 *
 *  FUNCTION:  A valid user name was identified from the /etc/utmp file.
 *		Store the users name in memory to be sorted later.
 *	      
 *  RETURN VALUE:  	 none
 */

static void
putline( void )
{
	char temp[NMAX+1];

	strncpy(temp, utmp.ut_name, NMAX);
	temp[NMAX] = 0;
	*namp = malloc((size_t)(strlen(temp) + 1));
	if (namp == NULL) {
		perror("malloc");
		exit(1);
	}
	strcpy(*namp++, temp);
}

/*
 *  NAME:  scmp
 *
 *  FUNCTION:  Compare the first elements in two arrays of strings.
 *            (cannot be replaced by a macro, because this is for qsort().)
 *	      
 *  RETURN VALUE:  	 0   - equal
 *			 1   - not
 */

static
int scmp(char **p, char **q)
{
	return(strcmp(*p, *q));
}

/*
 *  NAME:  summary
 *
 *  FUNCTION:  Sort all the users alphabetacally and print them out.
 *	      
 *  RETURN VALUE:  	 none
 */

static void
summary( void )
{
	register char	**p;
	register size_t users = ncnt;

	if ( users < 1 )	/* at least one user should be there */
		return;

	qsort( names, users, sizeof(names[0]), scmp );

	fputs(*(p = names), stdout);
	while ( --users ) {
		putchar(' ');
		fputs(*++p, stdout);
	}
	putchar('\n');
}
