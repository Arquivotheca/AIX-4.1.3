static char sccsid[] = "@(#)15	1.12  src/bos/usr/lib/sendmail/mailstats.c, cmdsend, bos411, 9428A410j 1/18/94 17:28:46";
/* 
 * COMPONENT_NAME: CMDSEND mailstats.c
 * 
 * FUNCTIONS: MSGSTR, Mmailstats, clear, report 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
**  Sendmail
**  Copyright (c) 1983  Eric P. Allman
**  Berkeley, California
**
**  Copyright (c) 1983 Regents of the University of California.
**  All rights reserved.  The Berkeley software License Agreement
**  specifies the terms and conditions for redistribution.
**
*/


#include <nl_types.h>
#include "mailstat_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILSTAT,n,s) 

#include <locale.h>
# include <stdio.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <netinet/in.h>
# include <time.h>
# include <sys/lockf.h>
# include "conf.h"
# include "sendmail.h"
# include "mailstats.h"
# include "sysexits.h"

#define STATSPATH	"/etc/sendmail.st"

static  int clear (int, char *);
static  int report(int, char *);

/*
**  MAILSTATS -- print mail statistics.
**
**	Exit Status:
**		Same stati that come from sendmail (sysexits.h).
*/

main (argc, argv)
int  argc;
char *argv[];
{
	register int fd;
	char  *spath;
	int  pi, fail, zflag;

        setlocale(LC_ALL,"");
	catd = catopen(MF_MAILSTAT,NL_CAT_LOCALE);

	/*
	 *  Get the flags (which start with -).
	 *  The stats file path may be given as argument to -S flag.
	 *  The -z flag means to clear the file.
	 */
	fail = 0;				/* clear fail mode */
	pi = 1;					/* init flag counter */
	spath = NULL;				/* set for default */
	zflag = 0;				/* don't zero file */
	while (pi < argc && argv[pi][0] == '-')	/* get flags */
	{
	    switch (argv[pi][1])
	    {
		/*
		 *  Set statistics file path.
		 */
		case 'S':
		    if (spath != NULL)		/* duplicate? */
		    {
			fprintf (stderr, MSGSTR(EXCESS, "mailstats: excess flag \"%s\"\n"), argv[pi]); /*MSG*/
			fail = 1;		/* set fail mode */
		    }
		    else			/* 1st one */
		    {
			if (argv[pi][2] != (char)NULL) /* immediately after -S? */
		            spath = &argv[pi][2];
			else			/* maybe there is white space */
			{
			    if (++pi < argc && argv[pi][0] != '-')
			        spath = argv[pi];
			    else		/* next parm isn't right */
			    {
				pi--;		/* back off */
				fprintf (stderr, MSGSTR(INVALID, "mailstats: invalid flag \"%s\"\n"), /*MSG*/
								argv[pi]);
				fail = 1;	/* set fail mode */
			    }
			}
		    }

		    break;

		/*
		 *  Clear the file instead of reporting.
		 */
		case 'z':
		    if (argv[pi][2] == '\0')	/* any unexpect stuff? */
			zflag = 1;		/* no, set the mode */
		    else
		    {
			fprintf (stderr, MSGSTR(INVALID, "mailstats: invalid flag \"%s\"\n"), /*MSG*/
								argv[pi]);
			fail = 1;		/* set fail mode */
		    }

		    break;

		default:
		    fprintf (stderr, MSGSTR(UNKNOWN, "mailstats: unknown flag \"%s\"\n"), argv[pi]); /*MSG*/
		    fail = 1;			/* set fail mode */
	    }

	    pi++;				/* move to next flag */
	}

	/*
	 *  When we are out of flags, check for nonflags.
	 */
	while (pi < argc)
	{
	    fprintf (stderr, MSGSTR(UNKPARM, "mailstats: unknown parameter \"%s\"\n"), argv[pi]); /*MSG*/
	    fail = 1;				/* set failure mode */
	    pi++;
	}

	/*
	 *  After processing flags and nonflags, set defaults.
	 */
	if (spath == NULL)
	    spath = STATSPATH;

	/*
	 *  If we are in failure mode, go out now with usage.
	 *  If we pass this, we are out of the user interface analysis
	 *  and ready to process.
	 */
	if (fail)
	{
	    fprintf (stderr, MSGSTR(USAGE,
		"usage: mailstats [-S<stats file path>] [-z]\n"));
	    exit (EX_USAGE);
	}

	/*
	 *  Open configuration file
	 */
	fd = open(spath, O_RDWR);
	if (fd < 0)
	{
		perror (spath);
		exit (EX_NOINPUT);
	}

	/*
	 *  Lock to prevent read or write by others.
	 */
	if (lockf (fd, F_LOCK, 0) < 0)
	{
		perror (spath);
		exit (EX_OSERR);
	}

	/*
	 *  Switch on operation mode
	 */
	if (zflag != 0)
	    exit (clear (fd, spath));
	else
	    exit (report (fd, spath));

/*NOTREACHED*/
}

static int clear (int fd, char *spath)
{
	if (ftruncate (fd, 0) < 0)
	{
		perror (spath);
		return (EX_IOERR);
	}

	return (EX_OK);
}

static int report (int fd, char *spath)
{
	struct statistics stat;
	register int i;
	char buf[BUFSIZ];

	/*
	 *  Read in the data structure.
	 */
	if ((i = read (fd, &stat, sizeof stat)) < 0)
	{
		perror (spath);
		return (EX_IOERR);
	}

	/*
	 *  Check for empty file.  The next execution of sendmail will
	 *  create the data structure.
	 */
	if (i == 0)
	{
		fprintf (stdout, MSGSTR(NODATA, "No statistics data in file \"%s\"\n"), spath); /*MSG*/
		return (EX_OK);
	}

	/*
	 *  Validate that all requested data was read, and then that
	 *  the size indicator in the structure is correct.
	 */
	if (i != sizeof stat || stat.stat_size != sizeof stat)
	{
		(void) fprintf (stderr, MSGSTR(SIZE, "mailstats: file size change; use previous mailstats version\n")); /*MSG*/
		return (EX_DB);
	}

	fprintf (stdout, MSGSTR(STATFILE, "Sendmail statistics from file \"%s\"\n"), spath); /*MSG*/
	if (strftime(buf, BUFSIZ, "%c", localtime(&stat.stat_itime)))
		fprintf (stdout, MSGSTR(COLLECT, 
			"    Collection started at %s\n"),buf); /*MSG*/
	else
		fprintf (stdout, MSGSTR(COLLECT, 
			"    Collection started at %s\n"), 
			ctime (&stat.stat_itime)); /*MSG*/
		
	fprintf (stdout, MSGSTR(HEADER, "Mailer          msgs_from     bytes_from      msgs_to       bytes_to\n")); /*MSG*/
	fprintf (stdout, MSGSTR(LINES, "------------    ---------     ----------    ---------     ----------\n")); /*MSG*/
	for (i = 0; i < MAXMAILERS; i++)
	{
	    if (*stat.stat_mname[i] != '\0')
	    {
	        fprintf (stdout, "%-12s " , stat.stat_mname[i]);
	        fprintf (stdout, "%12ld " , stat.msgsfrom[i]);
	        fprintf (stdout, "%14ld " , stat.bytesfrom[i]);
	        fprintf (stdout, "%12ld " , stat.msgsto[i]);
	        fprintf (stdout, "%14ld\n", stat.bytesto[i]);
	    }
	}

	return (EX_OK);
}










