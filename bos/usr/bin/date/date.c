static char sccsid[] = "@(#)25	1.34.2.11  src/bos/usr/bin/date/date.c, cmdstat, bos41B, 9504A 12/21/94 13:40:03";
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
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.0
 */

/**********************************************************************/
/* Include File                                                       */
/**********************************************************************/

#ifdef SECUREWARE
#include  <sys/secdefines.h>
#if SEC_BASE
#include  <sys/security.h>
#endif
#endif

#include  <sys/types.h>
#include  <sys/select.h>
#include  <fcntl.h>
#include  <utmp.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <time.h>
#include  <locale.h>
#include  <errno.h>
#include  <sys/syslog.h>

#include  <nl_types.h>
#include  <langinfo.h>
#include  "date_msg.h"
#include  <unistd.h>
#include  <netdb.h>

/**********************************************************************/
/* Constant Definition / Macro Function                               */
/**********************************************************************/

#define  NO_ERROR  0
#define  ERR_DISP  1
#define  ERR_SET   2

#define  NUM_SIZE    7  /* Size of num array */
#define  UNDEFINED  -1  /* Field not defined */

#define  FORMAT_SIZE  256  /* Size allowed to define output format. */	
#define  OUTPUT_SIZE  256  /* Size to display output string.        */

#define  MIN_mm  00  /* Minimum value of month - zero based for mktime()  */
#define  MAX_mm  11  /* Maximum value of month - 0 is Jan & 11 is Dec     */	
#define  MIN_dd  01  /* Minimum value of day    */
/* Maximum value of day for each month */
static char MAX_dd[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define  MIN_HH  00  /* Minimum value of hour   */
#define  MAX_HH  23  /* Maximum value of hour   */
#define  MIN_MM  00  /* Minimum value of minute */
#define  MAX_MM  59  /* Maximum value of minute */
#define  MIN_SS  00  /* Minimum value of second */
#define  MAX_SS  61  /* Maximum value of second - accounts for leap second */

#define  MIN_20yy  00  /* Minimum value of year 20XX */
#define  MAX_20yy  38  /* Maximum value of year 20XX */	
#define  MIN_19yy  70  /* Minimum value of year 19XX */
#define  MAX_19yy  99  /* Maximum value of year 19XX */	

#define  MSGSTR(num,str)  catgets(catd,MS_DATE,num,str)
#define  NUMBER(c1,c2)  ( ((c1)-'0')*10 + ((c2)-'0') )
#define  leapyr(A) (((A) % 4 == 0 && (A) % 100 != 0 || (A) % 400 == 0) ? 1 : 0)

/**********************************************************************/
/* Function Prototype Declaration                                     */
/**********************************************************************/

int   chk_all_num(char *);
void  set_date_time(char *);
int   parse_string(char *);
void  disp_date_time(char *);
int   settime(time_t);
void  usage(void);

/**********************************************************************/
/* Global / External Variables                                        */
/**********************************************************************/

static char  format[FORMAT_SIZE+1];  /* Format of output string  */
static char  output[OUTPUT_SIZE+1];  /* Output string to display */

static int  uflag = FALSE;  /* Flag indicating that -u was specified. */
static int  nflag = FALSE;  /* Flag indicating that -n was specified. */

static int date_specified = 0;	/* Date parameter has already been specified. */

static char    *cbp;    /* Character buffer pointer */
static time_t  timbuf;  /* Time in seconds since 00:00:00 GMT, Jan 1, 1970 */

static struct  utmp  wtmp[2] = { 
	{"","",OTIME_MSG,OLD_TIME,0,0,0,0,""},
	{"","",NTIME_MSG,NEW_TIME,0,0,0,0,""} 
};

static nl_catd  catd;    /* Catalog descriptor       */

static int  tfailed;

/***********************************************************************
 * NAME:  main                                                   
 *
 * FUNCTION:  date - with format capabilities   
 *
 * ARGUMENTS:  argc (int)     - Argument counter.
 *             argv (char **) - Argument string pointer.
 *
 * RETURN VALUE:  Execution status. (int)
 *                  0 - date written or set successfully.
 *                  1 - An error occured while writing date.
 *                  2 - An error occured while setting date.
 **********************************************************************/

int
main(int argc, char *argv[])
{
	int  i;
	int  opt;
	int  fd;     

	(void) setlocale (LC_ALL,"");  /* Set locale                 */ 
	catd = catopen(MF_DATE,NL_CAT_LOCALE);     /* Open message catalog       */

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
#endif

	tfailed = 0;
	format[0] = '\0';  /* Initialize format string */

	/*************************************
	 *** Get option from argument list ***
	 *************************************/
	while ( (opt = getopt(argc, argv, "nu")) != EOF ) {
		switch ( opt ) {
		  case 'n':
			nflag = TRUE;
			break;
		  case 'u':
			uflag = TRUE;
			putenv("TZ=GMT0");  /* Set TimeZone to GMT0 */
			break;
		  case '?':
			usage();
			exit(ERR_DISP);
		}
	}

	/******************************
	 *** Check for usage errors ***
	 ******************************/
	for (i=optind; i<argc; i++) {
		cbp = argv[i];
		if ((*cbp != '+') && (chk_all_num(cbp) == TRUE)) {
			if (date_specified) {
				(void) fprintf(stderr, MSGSTR(MULTDATE,
				"date: Do not specify more than one date/time parameter.\n"));
				usage();
				exit(ERR_DISP);
			}
			date_specified = 1;
		}
	}
	/******************************************
	 *** Parse all strings in argument list ***
	 ******************************************/
	for (i=optind; i<argc; i++) {
		cbp = argv[i];
		if (*cbp == '+') {		/* [+format] specified */
			cbp++;
			(void) strcat(format,cbp);       
		} else {			/* mmddhhmm[yy] specified */
			if ( chk_all_num(cbp) == TRUE ) {
				set_date_time(cbp);
			}
			else if ( strchr(cbp, '%') != NULL ) {
				(void) fprintf(stderr, MSGSTR(NEED_PLUS,
				"Field descriptor must begin with a + (plus sign).\n"));
				usage();
				exit(ERR_DISP);
			}
			else {
				(void) fprintf(stderr, MSGSTR(INV_CHAR,
				"Invalid character in date/time specification.\n"));
				usage();
				exit(ERR_SET);
			}
		}
	}

	disp_date_time(format);
	exit( tfailed ? ERR_SET : NO_ERROR );
}

/***********************************************************************
 *  NAME:  chk_all_num
 *
 *  FUNCTION:  Check if string only contains numeric characters '0'-'9', and
 *             checks that '.' only occurs as 8th character if present.
 *             Checks that string is 8, 10 or 13 characters in length
 *             to handle formats:  mmddHHMM[yy] and mmddHHMM[.SSyy]
 *
 *  ARGUMENTS:  string pointer. (char *)
 *
 *  RETURN VALUE:  TRUE  - '0'-'9' and '.' only.
 *                 FALSE - invalid character or format.
 **********************************************************************/

static int
chk_all_num(char *str)
{
	int  i, len;

	len = strlen(str);
	if (len != 8 && len != 10 && len != 13)
		return(FALSE);
	for ( i=0; str[i]!=0; i++ ) {
		if ( str[i] == '.') {
			if (i != 8)
				return(FALSE);
		}
		else if ( str[i] < '0' || str[i] > '9' )
			return(FALSE);
	}
	return(TRUE);
}

/***********************************************************************
 *  NAME:  set_date_time
 *
 *  FUNCTION:  Parse string and set date and time.
 *
 *  ARGUMENTS:  string pointer. (char *)
 *
 *  RETURN VALUE:  none.                           
 **********************************************************************/

static void
set_date_time(char *str)
{
	int  wf;

#if SEC_BASE
	int            privs_raised;
	privvec_t      saveprivs;
	extern priv_t  *privvec();
#endif

		if (parse_string(str) == FALSE) {
			(void) fprintf(stderr, MSGSTR(INV_RANGE,
			"date: Specified value out of range.\n"));
			usage();
			exit(ERR_SET);
		}

		(void) time(&wtmp[0].ut_time);

#if SEC_BASE
		/*
		 * If user is authorized to set the date, raise the
		 * necessary privileges.  SEC_SYSATTR is required to
		 * set it locally, and SEC_REMOTE is required to create
		 * a privileged socket to talk to the time daemon.
		 * The access control overrides allow us to make entries
		 * in the utmp and wtmp files.
		 */
		if (authorized_user("sysadmin")) {
			if (forceprivs(privvec(SEC_SYSATTR, SEC_REMOTE,
				SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), saveprivs)) {
				(void) fprintf(stderr, MSGSTR(EPRIV,
				"date: insufficient privileges\n"));
				exit(ERR_SET);
			}
			privs_raised = 1;
		}
		else {
			privs_raised = 0;
		}
#endif

		if (nflag || !settime(timbuf)) {
#if SEC_BASE
			disablepriv(SEC_SUSPEND_AUDIT);
#endif
			if (stime(&timbuf) < 0) {
#if SEC_BASE
				forcepriv(SEC_SUSPEND_AUDIT);
#endif
				tfailed++;
				(void) fprintf(stderr, MSGSTR(BAD_PERM,
						"date: no permission\n"));
			} 
			else {
#if SEC_BASE
				forcepriv(SEC_SUSPEND_AUDIT);
#endif
				(void) time(&wtmp[1].ut_time);

				/*  Attempt to write entries to the
				 *  utmp file and to the wtmp file. */

				pututline(&wtmp[0]);
				pututline(&wtmp[1]);

				if ((wf = open(WTMP_FILE, O_WRONLY|O_APPEND)) >= 0) {
					(void) write(wf, (char *)wtmp, sizeof(wtmp));
				}
			}
		}
#if SEC_BASE
		if (privs_raised) {
			seteffprivs(saveprivs, (priv_t *) 0);
		}
#endif

		if (!tfailed) {
			char *username;
			username = getlogin();
			/* single user or no tty */
			if (username == NULL || *username == '\0') {
				username = "root";
			}
			syslog(LOG_NOTICE, "set by %s", username);
		}
}

/***********************************************************************
 *  NAME:  parse_string
 *
 *  FUNCTION:  Convert the date/time format given on the command line
 *             to timbuf.
 *             The given string will be processed on assumption that
 *             it does not contain any invalid character.
 *
 *  ARGUMENTS:  str (char *) - date/time format string.
 *
 *  RETURN VALUE:  TRUE  - succeeded in conversion
 *                 FALSE - conversion failed
 **********************************************************************/

static int
parse_string(char *str)
{
	int  i, j;
	int  num[NUM_SIZE];
	int  dot = FALSE;
	int  yy;
	time_t  tbuf;
	struct tm  *tim, tmset;
	
	/*** Initialize num array ***/
	for ( i=0; i<NUM_SIZE; i++ ) 
		num[i] = UNDEFINED;

	/*** Store each value specified ***/
	for ( i=0, j=0; str[i]!=0; i++ ) {
		if ( str[i] == '.' ) { 
			dot = TRUE;
			continue;
		}

		if ( str[i+1] == 0 || j == NUM_SIZE ) {
			(void) fprintf(stderr, MSGSTR(INV_FORM,
			"date: Invalid date/time format.\n"));
			usage();
			exit(ERR_SET);
		} else {
			num[j] = NUMBER(str[i],str[i+1]);
			i++;
			j++;
		}
	}

	/*** Get current date and time ***/
	tzset();
	(void) time(&tbuf);
	tim = localtime(&tbuf);

	j = 0;
	tmset.tm_mon  = num[j++] - 1;
	tmset.tm_mday = num[j++];
	tmset.tm_hour = num[j++];
	tmset.tm_min  = num[j++];
	tmset.tm_sec  = tim->tm_sec;
	yy = tim->tm_year % 100;

	/*** Determine format type ***/

	if ( dot ) {
		/*** Format is mmddHHMM.SS[yy] ***/

		if ( num[j] == UNDEFINED )
			return(FALSE);

		tmset.tm_sec = num[j++];
	}

	if ( num[j] != UNDEFINED )
		yy = num[j++];

	if (num[j] != UNDEFINED) {
		(void) fprintf(stderr, MSGSTR(INV_FORM,
			"date: Invalid date/time format.\n"));
		usage();
		exit(ERR_SET);
	}

	/*** Check range of each value ***/
	if ( tmset.tm_mon  < MIN_mm || tmset.tm_mon  > MAX_mm ) return(FALSE);
	if ( tmset.tm_mday < MIN_dd || tmset.tm_mday > MAX_dd[tmset.tm_mon] )
		return(FALSE);
	if ( tmset.tm_hour < MIN_HH || tmset.tm_hour > MAX_HH ) return(FALSE);
	if ( tmset.tm_min  < MIN_MM || tmset.tm_min  > MAX_MM ) return(FALSE);
	if ( tmset.tm_sec  < MIN_SS || tmset.tm_sec  > MAX_SS ) return(FALSE);

	if ( yy >= MIN_20yy && yy <= MAX_20yy ) {
		tmset.tm_year = yy + 100;  /* offset from 1900 for time_t format */
		yy += 2000;
	} else if ( yy >= MIN_19yy || yy <= MAX_19yy ) {
		tmset.tm_year = yy;
		yy += 1900;
	} else
		return(FALSE);

	/* If month is February, days > 28 and not leap year, then return. */
	if (tmset.tm_mon == 1 && tmset.tm_mday > 28 && !leapyr(yy)) return(FALSE);

	tmset.tm_isdst = -1;

	if ((timbuf = mktime(&tmset)) == (time_t) -1)
		return(FALSE);

	return(TRUE);
}

/**********************************************************************/

#include <sys/param.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define TSPTYPES
#include <protocols/timed.h>

#define WAITACK		2	/* seconds */
#define WAITDATEACK	5	/* seconds */

extern	int errno;

/***********************************************************************
 *  NAME:  settime
 *
 *  FUNCTION:  Set the date in the machines controlled by timedaemons
 *             by communicating the new date to the local timedaemon. 
 *             If the timedaemon is in the master state, it performs the
 *             correction on all slaves.  If it is in the slave state,
 *             it notifies the master that a correction is needed.
 *
 *  RETURN VALUE:  0 - failure
 *                 1 - success
 **********************************************************************/

static int
settime(time_t timebuf)
{
	int  s, length, port, timed_ack, found, err;
	long  waittime;
	fd_set  ready;
	char  hostname[MAXHOSTNAMELEN];
	struct timeval  tout;
	struct timeval  tv;
	struct servent  *sp;
	struct tsp  msg;
	struct sockaddr_in  sin, dest, from;

	tv.tv_sec = timebuf;
	tv.tv_usec = 0;

	sp = getservbyname("timed", "udp");
	/* If the timed service is not on the machine then just return. */
	if (sp == NULL)
		return (0);
	dest.sin_port = sp->s_port;
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = htonl((u_long)INADDR_ANY);
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		if (errno != EPROTONOSUPPORT)
			perror("date: socket");
		goto bad;
	}
	bzero((char *)&sin, sizeof (sin));
	sin.sin_family = AF_INET;
	for (port = IPPORT_RESERVED - 1; port > IPPORT_RESERVED / 2; port--) {
		sin.sin_port = htons((u_short)port);
		if (bind(s, (struct sockaddr *)&sin, sizeof (sin)) >= 0)
			break;
		if (errno != EADDRINUSE) {
			if (errno != EADDRNOTAVAIL)
				perror("date: bind");
			goto bad;
		}
	}
	if (port == IPPORT_RESERVED / 2) {
		(void) fprintf(stderr, MSGSTR(NOPORTS,
		"date: all ports in use\n"));
		goto bad;
	}
	msg.tsp_type = TSP_SETDATE;
	msg.tsp_vers = TSPVERSION;
	(void) gethostname(hostname, sizeof (hostname));
	(void) strncpy(msg.tsp_name, hostname, sizeof (hostname));
	msg.tsp_seq = htons((u_short)0);
	msg.tsp_time.tv_sec = htonl((u_long)tv.tv_sec);
	msg.tsp_time.tv_usec = htonl((u_long)tv.tv_usec);
	length = sizeof (struct sockaddr_in);
	if (connect(s, &dest, length) < 0) {
		perror("date: connect");
		goto bad;
	}
	if (send(s, (char *)&msg, sizeof (struct tsp), 0) < 0) {
		if (errno != ECONNREFUSED)
			perror("date: send");
		goto bad;
	}
	timed_ack = -1;
	waittime = WAITACK;
loop:
	tout.tv_sec = waittime;
	tout.tv_usec = 0;
	FD_ZERO(&ready);
	FD_SET(s, &ready);
	found = select(FD_SETSIZE, &ready, (fd_set *)0, (fd_set *)0, &tout);
	length = sizeof(err);
	if (getsockopt(s, SOL_SOCKET, SO_ERROR, (char *)&err, &length) == 0
/*		&& err) {  */
		      ) {
		errno = err;
/*		if (errno != ECONNREFUSED)  */
		if (errno != ECONNREFUSED && errno != 0)
			perror("date: send (delayed error)");
		goto bad;
	}
	if (found > 0 && FD_ISSET(s, &ready)) {
		length = sizeof (struct sockaddr_in);
		if (recvfrom(s, (char *)&msg, sizeof (struct tsp), 0, &from,
			&length) < 0) {
			if (errno != ECONNREFUSED)
				perror("date: recvfrom");
			goto bad;
		}
		msg.tsp_seq = ntohs(msg.tsp_seq);
		msg.tsp_time.tv_sec = ntohl(msg.tsp_time.tv_sec);
		msg.tsp_time.tv_usec = ntohl(msg.tsp_time.tv_usec);

		switch (msg.tsp_type) {
		case TSP_ACK:
			timed_ack = TSP_ACK;
			waittime = WAITDATEACK;
			goto loop;

		case TSP_DATEACK:
			(void)close(s);
			return (1);

		default:
			(void) fprintf(stderr, MSGSTR(WRONG_ACK,
			"date: wrong ack received from timed: %s\n"), 
			tsptype[msg.tsp_type]);
			timed_ack = -1;
			break;
		}
	}
	if (timed_ack == -1) {
		(void) fprintf(stderr, MSGSTR(CANTREACH,
		"date: can't reach time daemon - time set locally\n"));
	}
bad:
	(void)close(s);
	return (0);
}

/***********************************************************************
 *  NAME:  disp_date_time
 *
 *  FUNCTION:  Display output string according to format.
 *
 *  ARGUMENTS:  format string (char *)
 *
 *  RETURN VALUE:  none
 **********************************************************************/

static void
disp_date_time(char *format)
{
	time_t  tbuf;
	struct tm  *tim, timCp ;
	char *loc;

	(void) time(&tbuf);
	tim = localtime(&tbuf);
	timCp = *tim ;

	loc = setlocale(LC_TIME, NULL);

	if (format[0] == '\0') {  /* Set default format */
		if (!strcmp(loc, "C") || !strcmp(loc, "POSIX"))
			format = "%a %b %e %H:%M:%S %Z %Y";
		else
			format = "%c";
	}

	(void)strftime(output, OUTPUT_SIZE, format, &timCp);
	printf("%s\n", output);
}

/***********************************************************************
 *  NAME:  usage
 *
 *  FUNCTION:  Print out the command options.
 *             They are different for the super user.
 *
 *  ARGUMENTS:  none.
 *
 *  RETURN VALUE:  none.
 **********************************************************************/

static void
usage(void)
{
	/****************************/
	/*** Usage for super user ***/
	/****************************/
	if (getuid() == 0){ 
		(void) fprintf(stderr, MSGSTR(USAGE_ROOT1,
		"Usage: date [-n][-u] [mmddHHMM[yy]] [+Field Descriptors]\n")); 
		(void) fprintf(stderr, MSGSTR(USAGE_ROOT2,
		"Usage: date [-n][-u] [mmddHHMM[.SSyy]] [+Field Descriptors]\n")); 
		}

	/*****************************/
	/*** Usage for other users ***/
	/*****************************/
	else {
		(void) fprintf(stderr, MSGSTR(USAGE,
		"Usage: date [-u] [+Field Descriptors]\n"));
	}
}

