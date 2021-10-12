static char sccsid[] = "@(#)00	1.13  src/bos/usr/bin/last/last.c, cmdstat, bos41B, 9504A 12/21/94 13:40:16";
/*
 * COMPONENT_NAME: (CMDSTAT) Displays Information about Previous logins
 *
 * FUNCTIONS: last
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
 */

#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <utmp.h>
#include <time.h>
#include <locale.h>
#include <limits.h>


#define NMAX    sizeof(buf[0].ut_user)
#define LMAX	sizeof(buf[0].ut_line)
#define HMAX    sizeof(buf[0].ut_host)
#define	SECDAY	(24*60*60)
#define MAXTTYS 256

#define	lineq(a,b)	(!strncmp(a,b,LMAX))
#define	nameq(a,b)	(!strncmp(a,b,NMAX))
#define	hosteq(a,b)	(!strncmp(a,b,HMAX))

#include "last_msg.h"
static nl_catd	catd;
#define MSGSTR(n,s)     ( catgets(catd,MS_LAST,n,s) )

static char	**argv;
static int	argc;
static int	nameargs;

static struct	utmp buf[128];
static char	ttnames[MAXTTYS][LMAX+1];
static long	logouts[MAXTTYS];

char	*strspl();
int	onintr(int);
static ushort  fflag = 0;
static char 	filename[PATH_MAX];

main(ac, av)
	int ac;
	char **av;
{
	register int i, k;
	int bl, wtmp;
	char dt[13];
	char tt[6];
	register struct utmp *bp;
	long otime;
	struct stat stb;
	int print;
	char * crmsg = (char *)0;
	long crtime;
	long outrec = 0;
	long maxrec = 0x7fffffffL;
 
	(void) setlocale (LC_ALL,"");

	catd = catopen( MF_LAST,NL_CAT_LOCALE );

	time(&buf[0].ut_time);
	ac--, av++;
	nameargs = argc = ac;
	argv = av;
	strcpy(filename, "/var/adm/wtmp");
	for (i = 0; i < argc; i++) {
		if (argv[i][0] == '-' &&
		    argv[i][1] >= '0' && argv[i][1] <= '9') {
			maxrec = atoi(argv[i]+1);
			nameargs--;
			continue;
		}
		if (argv[i][0] == '-' && argv[i][1] == 'f')  {
			if (argv[i+1] == NULL)
				continue;
			fflag = 1;
			strcpy(filename, argv[++i]);
			nameargs = nameargs - 2;
			continue;
		}
		if (strlen(argv[i])>2)
			continue;
		if (!strcmp(argv[i], "~"))
			continue;
		if (!strcmp(argv[i], "ftp"))
			continue;
		if (!strcmp(argv[i], "uucp"))
			continue;
		if (getpwnam(argv[i]))
			continue;
		argv[i] = strspl("tty", argv[i]);
	}
	wtmp = open(filename, 0);
	if (wtmp < 0) {
		perror(filename);
		exit(1);
	}
	fstat(wtmp, &stb);
	bl = (stb.st_size + sizeof (buf)-1) / sizeof (buf);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN) {
		signal(SIGINT, (void (*)(int))onintr);
		signal(SIGQUIT, (void (*)(int))onintr);
	}
	for (bl--; bl >= 0; bl--) {
		lseek(wtmp, bl * sizeof (buf), 0);
		bp = &buf[read(wtmp, buf, sizeof (buf)) / sizeof(buf[0]) - 1];
		for ( ; bp >= buf; bp--) {
			print = want(bp);
			if (print) {
				strftime( dt, 13, "%sD",
					localtime(&bp->ut_time) );
				strftime( tt, 6, "%sT",
					localtime(&bp->ut_time) );
				printf("%-*.*s  %-*.*s %-*.*s %12.12s %5.5s ",
				    NMAX, NMAX, bp->ut_user,
				    LMAX, LMAX, bp->ut_line,
				    HMAX, HMAX, bp->ut_host,
				    dt, tt );
			}
			for (i = 0; i < MAXTTYS; i++) {
				if (ttnames[i][0] == 0) {
					strncpy(ttnames[i], bp->ut_line,
					    sizeof(bp->ut_line));
					otime = logouts[i];
					logouts[i] = bp->ut_time;
					break;
				}
				if (lineq(ttnames[i], bp->ut_line)) {
					otime = logouts[i];
					logouts[i] = bp->ut_time;
					break;
				}
			}
			if (print) {
				if ( (nameq(bp->ut_name, "shutdown"))
					|| (nameq(bp->ut_name, "reboot")) )
					printf("\n");
				else if (otime == 0)
					printf(MSGSTR(M_LOGGEDIN,
						"  still logged in\n" ));
				else {
					long delta;
					if (otime < 0) {
						otime = -otime;
						printf("- %s", crmsg);
					} else
						{
						strftime( tt, 6, "%sT",
						    localtime(&otime) );
						printf("- %5.5s", tt );
						}
					delta = otime - bp->ut_time;
					if (delta < SECDAY) {
					strftime(tt,6,"%X",gmtime(&delta));
					    printf("  (%s)\n",tt);
					}
					else {
					strftime(tt,6,"%sT",gmtime(&delta));
					    printf(" (%ld+%5.5s)\n",
						delta / SECDAY,tt);
					}
				}
				fflush(stdout);
				if (++outrec >= maxrec)
					exit(0);
			}
			if (nameq(bp->ut_user, "shutdown")) {
				crmsg = MSGSTR( M_DOWN, "down " );
				for (i = 0; i < MAXTTYS; i++)
					logouts[i] = -bp->ut_time;
				}
			else if (nameq(bp->ut_user, "reboot")) {
				crmsg = MSGSTR( M_CRASH, "crash" );
				for (i = 0; i < MAXTTYS; i++)
					logouts[i] = -bp->ut_time;
				}
		}
	}
	strftime( dt, 13, "%sD", localtime(&buf[0].ut_time) );
	strftime( tt, 6, "%sT", localtime(&buf[0].ut_time) );
	printf(MSGSTR(M_BEGIN,"\nwtmp begins %12.12s %5.5s \n"),dt,tt);
	exit(0);
}

/*
 *  NAME:  onintr
 *
 *  FUNCTION:  if last command is interrupted print the date out then
 *		exit if signal is SIGKILL.
 *  RETURN VALUE:  	 none
 */

static onintr(int signo)
{
	char dt[13];
	char tt[6];

	if (signo == SIGQUIT)
		signal(SIGQUIT, (void (*)(int))onintr);
	strftime( dt, 13, "%sD", localtime(&buf[0].ut_time) );
	strftime( tt, 6, "%sT", localtime(&buf[0].ut_time) );
	printf(MSGSTR(M_INTR,"\ninterrupted %12.12s %5.5s \n"), dt, tt);
	fflush(stdout);
	if (signo == SIGINT)
		exit(1);
}

/*
 *  NAME:  want
 *
 *  FUNCTION:	Determine whether the given utmp structure is one
 *		the user is requesting.
 *	      
 *  RETURN VALUE:  	1 if one is found
 *			0 otherwise
 *			
 */

static want(bp)
	struct utmp *bp;
{
	register char **av;
	register int ac;

 	/* don't want process types RUN_LVL, OLD_TIME, NEW_TIME,	*/
	/*		INIT_PROCESS, LOGIN_PROCESS. ACCOUNTING		*/

 	/* do want types BOOT_TIME(reboot), USER_PROCESS,		*/
	/*	DEAD_PROCESS, EMPTY(shutdown)				*/
	/*	ftp and uucp (these have negative process types)	*/
 
 	if ( (bp->ut_type == RUN_LVL) || 
		(bp->ut_type == ACCOUNTING) || 
		(bp->ut_type == OLD_TIME) || 
		(bp->ut_type == NEW_TIME) || 
		(bp->ut_type == INIT_PROCESS) || 
		(bp->ut_type == LOGIN_PROCESS) ) 
 		 return (0); 

 	if  (bp->ut_type == BOOT_TIME) {
 		if (strncmp(bp->ut_line, BOOT_MSG, 11) == 0) {
 		    /* make output conform to BSD */
 			strcpy(bp->ut_user, "reboot");  
 			strcpy(bp->ut_line, "~");      
 		}
 		else
 			 return (0); 
 	}

	if (strncmp(bp->ut_line, "ftp", 3) == 0)
		bp->ut_line[3] = '\0';
	if (strncmp(bp->ut_line, "uucp", 4) == 0)
		bp->ut_line[4] = '\0';
	if (bp->ut_user[0] == 0)
		return (0);
	if (nameargs == 0)
		return (1);
	av = argv;
	for (ac = 0; ac < argc; ac++, av++) {
		if (av[0][0] == '-')
			continue;
		if (nameq(*av, bp->ut_user) || lineq(*av, bp->ut_line))
			return (1);
	}
	return (0);
}

/*
 *  NAME:  strspl
 *
 *  FUNCTION:  		Concatenate two strings.
 *	      
 *  RETURN VALUE:   	I pointer to the new concatenated string.
 */

static char *
strspl(left, right)
	char *left, *right;
{
	char *res = (char *)malloc(strlen(left)+strlen(right)+1);
	if (res == NULL)
	{
		perror("malloc failed");
		exit (-1);
	}
	strcpy(res, left);
	strcat(res, right);
	return (res);
}
