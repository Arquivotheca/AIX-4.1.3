static char sccsid[] = "@(#)59	1.21  src/bos/usr/bin/touch/touch.c, cmdscan, bos41J, 9507A 1/31/95 09:42:33";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
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
 * 
 */
/*
 *	Updates the access and modification times of each file or
 *	directory named.  Options include:
 *
 *		-a	 	Access time only
 *		-c		Do NOT create the file if it doesn't exist
 *		-m	 	Modification time only
 *		-f		force (the default case).
 *		-r ref_file     Use time from reference file
 *		-t time	        Use the specified time instead of current time
 *
 *   		touch format time with no flag is [mmddhhmm[yy]] 
 *   		touch format time with -t flag is [[CC]YY]MMDDhhmm[.SS]] 
 *
 */                                                                   

#define _ILS_MACROS

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <time.h>
#include <errno.h>
#include <locale.h>
#include <fcntl.h>
#include <utime.h>
#include <string.h>
#include <sys/mode.h>
#include <langinfo.h>
#include "touch_msg.h"

static nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd,MS_TOUCH,Num,Str)

static struct	stat	stbuf;
static int	status;

static char	*cbp;
static time_t	timbuf;
static void usage();

/*
 * NAME: gtime
 *                                                                    
 * FUNCTION: 	Convert ascii time value on command line into a
 *		the number of seconds since 1970.  This value is
 *		returned in global value:
 *
 * GLOBAL VALUE:	timebuf
 *                                                                    
 * RETURN VALUE:	0 success
 * 			1 failure
 */  

static int
gtime(tflg)
int tflg;
{
	register int y, cc;
	long nt;
	int minarg = 0;
	char *s1;
	struct tm tmset;

	minarg = strlen(cbp);
	tzset();
	(void) time(&nt);
	tmset.tm_sec  = 0;
	tmset.tm_year = localtime(&nt)->tm_year;
	if (tflg) {
		/*
		 * get seconds
		 */
		if ( (s1 = strrchr(cbp, '.')) != NULL) {
			s1++;
			if ((minarg < 11) || (strlen(s1) != 2))
				return(1);

			tmset.tm_sec = atoi(s1);		/* Seconds */
			minarg-=3;
		}
		if (minarg != 8) {
			cc = -1;
			if (minarg == 12) {
				if ((cc = gpair()) < 0)		/* Century */
					return(1);
				cc = ((cc - 19) * 100);
				if (cc<0)
					return(1);
				minarg -= 2;
			}
			if (minarg != 10)
				return(1);
			if ((y = gpair()) < 0)			/* Year    */
				return(1);
			minarg -= 2;

			if (y>=0 && y<69 && cc<0)
				cc = 100;
			if (cc<0)
				cc = 0;
			
			y += cc;
			tmset.tm_year = y;
		}
	}

	if ((minarg != 8) && (minarg != 10))
		return(1);
		
	if ((tmset.tm_mon  = gpair()-1) < 0)			/* Month   */
		return(1);
	if ((tmset.tm_mday = gpair()) < 0)			/* Day     */
		return(1);
	if ((tmset.tm_hour = gpair()) < 0)			/* Hour    */
		return(1);
	if ((tmset.tm_min  = gpair()) < 0)			/* Minute  */
		return(1);

	if (!tflg && (minarg == 10)) {
		if ((y = gpair()) < 0)				/* Year    */
			return(1);
		if ( y >= 00 && y <=68 )
			y += 100;
		tmset.tm_year = y;
	}

	tmset.tm_isdst = -1;
	if ((timbuf = mktime(&tmset)) == (time_t) -1)
		return(1);
	return(0);
}


/*
 * NAME: gpair
 *                                                                    
 * FUNCTION: 		Converts the first two characters in a given parameter
 *			string into an integer.
 *                                                                   
 * RETURN VALUE:	success:  integer corresponding to the first two digits
 *			failure:  -1
 */  
static int
gpair()
{
	register int c, d;
	register char *cp;

	cp = cbp;
	if(*cp == 0)
		return(-1);
	c = (*cp++ - '0') * 10;
	if (c<0 || c>100)
		return(-1);
	if(*cp == 0)
		return(-1);
	if ((d = *cp++ - '0') < 0 || d > 9)
		return(-1);
	cbp = cp;
	return (c+d);
}

main(argc, argv)
int argc;
char *argv[];
{
	register int c;
	struct utimbuf times, newtimes;
        int default_time = 1;

	int mflg=0, aflg=0, cflg=0, rflg=0, optc, fd;
	int tflg = 0;
	char *reffile;
	int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_TOUCH, NL_CAT_LOCALE);

	if (argc==1) usage();

	while ((optc=getopt(argc, argv, "amcfr:t:")) != EOF) {
		switch(optc) {
		case 'm':
			mflg++;		/* Modification times only    */
                        default_time = 0;
			break;
		case 'a':		/* Access  times only         */
			aflg++;
                        default_time = 0;
			break;
		case 'c':		/* Don't creat if isn't there */
			cflg++;
			break;
		case 'f':		/* silent compatibility case  */
			break;
		case 'r':
			rflg++;
			reffile = optarg;
                        default_time = 0;
			break;
		case 't':
			tflg++;
			cbp = optarg;
			break;
		case '?':
			usage();
		}
	}
	if ((argc-optind) < 1)
		usage();
	if (!mflg && !aflg) {
		mflg++;
		aflg++;
	}

/* Defect 33939 added OR to if test 					   */ 
/* POSIX says that if no -t or -r was specified and at least two operands  */
/* are used then the first is assumed to be a date otherwise the first     */
/* operand shall be assumed to be a file operand.			   */

/* actually, according to XCU draft 6, the first operand should also be    */
/* treated as a file if it is not an 8 or 10 character long number.        */

	if(!tflg && (!isdatenumber(argv[optind]) || ((argc-optind) < 2))) {
		timbuf = time((long *) 0);
	} else {
                default_time = 0;
		if (!tflg)
			cbp = (char *)argv[optind++];
		if(gtime(tflg)) {
			if (!tflg)
				(void) fprintf(stderr,MSGSTR(TCONV, "touch: bad conversion\n"));
			else
				(void) fprintf(stderr,MSGSTR(SPECTIME, "touch: bad conversion\n"));
			exit(2);
		}
	}
	for(c=optind; c<argc; c++) {
		if(stat(argv[c], &stbuf)) {
			if (cflg) {
				/* status++; */ /* posix wants ret val of 0 */
				continue;
			}
			else if (errno == ENOENT) {
				if ((fd = creat (argv[c], mode)) < 0) {
					(void) fprintf(stderr, MSGSTR(TCREATE, "touch: %s cannot create\n"), argv[c]);
					status++;
				 	continue;
				}
				(void) close(fd);
				if(stat(argv[c], &stbuf)) {
					(void) fprintf(stderr,MSGSTR(TSTAT, "touch: %s cannot stat\n"),argv[c]);
					status++;
					continue;
				}
			}
			else {
				fprintf(stderr, "touch %s:", argv[c]);
				perror((char *)NULL);
				status++;
			}
		}

		times.modtime = stbuf.st_mtime;	/* Current times of file */
		times.actime = stbuf.st_atime;
		if (rflg) {
			if (stat(reffile, &stbuf) == -1) {
				fprintf(stderr, "%s:", reffile);
				perror("stat");
				exit(2);
			}
			newtimes.modtime = stbuf.st_mtime;
			newtimes.actime = stbuf.st_atime;
		} else
			newtimes.modtime = newtimes.actime = timbuf;
		if (mflg)
			times.modtime = newtimes.modtime;
		if (aflg)
			times.actime = newtimes.actime;
                if (utime(argv[c], (struct utimbuf *)
                                ((default_time==0)?&times:NULL))) {
			(void) fprintf(stderr,MSGSTR(TCHTIME, "touch: cannot change times on %s\n"),argv[c]); 
			status++;
			continue;
		}
	}
	exit(status);
}


/*
 * NAME: isdatenumber
 *                                                                    
 * FUNCTION: 		See if the value passed in is a string of 8 or
 *			10 numeric digits.  If so we have a candidate for
 *			a date.
 *                                                                    
 * RETURN VALUE:	1 it is.
 *			0 it is not.
 */  
static int
isdatenumber(s)
char *s;
{
	register int c;

	if ((strlen(s) != 8) && (strlen(s) != 10))
		return(0);
	while(c = *s++)
		if(!isdigit(c))
			return(0);

	return(1);
}

static void
usage()
{
	(void) fprintf(stderr, MSGSTR(TUSAGE, "usage: touch [-amcf] [MMDDhhmm[YY]] [-t [[CC]YY]MMDDhhmm[.SS]] [-r ref_file] file ...\n"));
	exit(2);
}
