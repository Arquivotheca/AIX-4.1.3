static char sccsid[] = "@(#)84  1.12  src/bos/usr/bin/timex/timex.c, cmdstat, bos41B, 9504A 12/21/94 13:41:29";
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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <locale.h>
#include <time.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/param.h>
#include <sys/wait.h>

#include "sar_msg.h"
static nl_catd catd;
#define MSGSTR(Num, Str)  catgets(catd, MS_SAR, Num, Str)

extern int getopt( int argc, char* argv[], const char* options );
		/* until getopt() is declared in a (standard) header file... */
extern int optind;


static void	flag_o_p( int oflg, int pflg, char* cmd );
static void	printt( char* str, clock_t clocks );
static void	hmstime( char* stime );
static void	diag( const char *s );

static char	fname[20];

#define	TIMESTR	9

main(argc, argv)
int argc;
char **argv;
{
	struct	tms buffer, obuffer;
	clock_t	before, after;
	int	pflg = 0, sflg = 0, oflg = 0;
	int	c;
register pid_t	pid;
	int	status;
	char	stime[TIMESTR], etime[TIMESTR];
	char	cmd[80];
	char	aopt[25];

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_SAR, NL_CAT_LOCALE);

	/* check options; */
	while((c = getopt(argc, argv, "sopfhkmrt")) != EOF)
		switch(c)  {
		case 's':  sflg++;  break;
		case 'o':  oflg++;  break;
		case 'p':  pflg++;  break;

		case 'f':  strcat(aopt, "-f ");  break;
		case 'h':  strcat(aopt, "-h ");  break;
		case 'k':  strcat(aopt, "-k ");  break;
		case 'm':  strcat(aopt, "-m ");  break;
		case 'r':  strcat(aopt, "-r ");  break;
		case 't':  strcat(aopt, "-t ");  break;

		case '?':  diag(MSGSTR(TIMEXUSE, 
				"Usage: timex [-s][-o][-p[-fhkmrt]] cmd"));
				break;
		}
	if(optind >= argc)      diag(MSGSTR(MISSCOM, "Missing command"));

	if (sflg) {
		sprintf(fname,"/tmp/tmx%d",getpid());
		sprintf(cmd,"/usr/lib/sa/sadc 1 1 %s",fname);
		system(cmd);
	}
	if (pflg + oflg) hmstime(stime);
	before = times(&obuffer);
	if ((pid = fork()) == -1) diag(MSGSTR(TRYAGAIN, "Try again.\n"));
	if(pid == 0) {
		setgid(getgid());
		execvp(*(argv+optind),(argv+optind));
		fprintf(stderr, "%s: %s\n", *(argv+optind), strerror(errno));
		exit(1);
	}
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	while(wait(&status) != pid);
	if((status&0377) != 0)
		fprintf(stderr,MSGSTR(ABNORMALEXIT, "Command terminated abnormally.\n"));
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	after = times(&buffer);
	if (pflg + oflg) hmstime(etime);
	if (sflg) system(cmd);

	fprintf(stderr,"\n");
	printt(MSGSTR(REAL, "real"), (after-before));
	printt(MSGSTR(USER, "user"), buffer.tms_cutime - obuffer.tms_cutime);
	printt(MSGSTR(SYS, "sys "), buffer.tms_cstime - obuffer.tms_cstime);
	fprintf(stderr,"\n");

	if (oflg+pflg) {
		char	ttyid[12];

		if(isatty(0))
			sprintf(ttyid, "-l %s", ttyname(0)+5);
		sprintf(cmd, "/usr/sbin/acct/acctcom -S %s -E %s -u %s %s -i %s",
				stime, etime, cuserid((char *)0), ttyid, aopt);
		flag_o_p( oflg, pflg, cmd );
	}
	if (sflg)  {
		sprintf(cmd,"/usr/sbin/sar -ubrycwaqvm -f %s 1>&2",fname);
		system(cmd);
		unlink(fname);
	}
	exit(status>>8);
}

/*
 *  NAME:  flag_o_p
 *
 *  FUNCTION:  invoke the acctcom command when "-o" and/or "-p" is given
 */

#define	L_BUF 150

static void 
flag_o_p( int oflg, int pflg, char* cmd )
{
register FILE	*pipin;
	char	line[L_BUF];
	char	fld[20][12];
	int	iline = 0, nfld, i;
	int	ichar, iblok;
	long	chars = 0, bloks = 0;

	pipin = popen(cmd, "r");
	while (	fgets( line, L_BUF, pipin ) != NULL ) {
		if ( *line == '\n' )
			continue;	/* don't ++iline with blank lines */
		if(pflg)
			fprintf(stderr, "%s", line);
		if(oflg) {
			nfld=sscanf(line,
			"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
			fld[0], fld[1], fld[2], fld[3], fld[4],
			fld[5], fld[6], fld[7], fld[8], fld[9],
			fld[10], fld[11], fld[12], fld[13], fld[14],
			fld[15], fld[16], fld[17], fld[18], fld[19]);
			if(++iline == 3)
				for(i=0; i<nfld; i++)  {
					if(strcmp(fld[i], "CHARS") == 0)
						ichar = i+2;
					if(strcmp(fld[i],"BLOCKS") == 0)
						iblok = i+2;
				}
			if (iline > 4)  {
				chars += atol(fld[ichar]);
				bloks += atol(fld[iblok]);
			}
		}
	}
	pclose(pipin);

	if(oflg) {
	    if(iline > 4)
		fprintf(stderr,MSGSTR(CHARSBLOCKS,
		"\nCHARS TRNSFD = %1$ld\nBLOCKS READ  = %2$ld\n"),chars,bloks);
	    else
		fprintf(stderr,MSGSTR(NORECS,"\nNo process records found!\n"));
	}
}

/*
 *  NAME:  printt
 *
 *  FUNCTION:  convert the time in seconds to a string in clock time.
 *
 *  RETURN VALUE:  	 none
 */

#define	PREC	 2	/* number of digits following the radix character */

static void 
printt( char* str, clock_t clocks )
{
	float	seconds = (float)clocks / sysconf( _SC_CLK_TCK );

	fprintf( stderr, "%s %.*f\n", str, PREC, seconds );
}

/*
 *  NAME:  hmstime
 *
 *  FUNCTION:
 * 		hmstime() sets current time in hh:mm:ss string format in stime;
 *
 *  RETURN VALUE:	void
 */


static void
hmstime( char*	stime )
{
	time_t	tme;

	tme = time((time_t *)NULL);
	strftime( stime, TIMESTR, "%H:%M:%S", localtime(&tme) );
}

/*
 *  NAME:  diag
 *
 *  FUNCTION:  Print out an error diagnostic and unlink the working file.
 *
 *  RETURN VALUE:  	 exits
 */

static void
diag( const char *s )
{
	fprintf(stderr,"%s\n",s);
	unlink(fname);
	exit(1);
}
