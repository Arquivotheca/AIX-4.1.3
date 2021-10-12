static char sccsid[] = "@(#)31	1.26.1.14  src/bos/usr/bin/who/who.c, cmdstat, bos41B, 9504A 12/21/94 13:41:55";
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

#define _ILS_MACROS
#ifdef _AIX
/* AIX security enhancement */
#include <sys/access.h>
/* TCSEC DAC mechanism */
#endif

#include <stdlib.h> 
#include <unistd.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/m_wait.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <utmp.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <signal.h>
#include <fcntl.h>
#include <IN/AFdefs.h>
#include <locale.h>

#include <time.h>
#include <ctype.h>

#include "who_msg.h"
static nl_catd catd;
#define MSGSTR(Num, Str)  catgets(catd, MS_WHO, Num, Str)

static struct utmp utmp_size;

#define SIZEUSER	(sizeof(utmp_size.ut_user))	
#define	SIZELINE	(sizeof(utmp_size.ut_line))

#define FAILURE -1
#define MAXLINE 100
#define FLAGS   UTMAXTYPE

	/* --- Width of fields (in the output/without spacing) --- */
#define	LOGF_SIZE	 9	/* user name 	: Name */
#define	TTYS_SIZE	 1	/* tty status	: ST */
#define	LINE_SIZE	11	/* tty name	: Line */
#define	TIME_SIZE	13	/* login time	: Time */
#define	IDLE_SIZE	 6	/* Idle time	: Activity */
#define	PID_SIZE	 9	/* user's PID	: PID */
#define	HOST_SIZE	17	/* workstation	: Hostname or Hostname/Exit */
#define	ID_SIZE 	 7	/* id of /Exit  : (id=xxxx) */

	/* --- Width of fields (in the Header) --- */
#define	TTYS_HEAD	 2	/* Due to the fact that the title "ST" and  */
#define	TIME_HEAD	11	/* "Activity" are so long that it is eating */
#define	IDLE_HEAD	 8	/* its previous field.			    */


int usage(char *format, char *arg1);
char *user(struct utmp *utmp);
char *host(struct utmp *utmp);
char *line(struct utmp *utmp);
static void copypad(char *to, char *from, size_t size, int width);
char *id(struct utmp *utmp);
char *ltime(struct utmp *utmp);
char *etime(struct utmp *utmp);
char *mode(struct utmp *utmp);
int alarmclk(void);
char *loc(struct utmp *utmp);

static char username[SIZEUSER+1];

		/* values for 'shortflag' */
#define	S_DFLT	 0	/* no options related to '-s' */
#define	S_SHORT	 1	/* short: suppress Activity and PID (and "/Exit") */
#define	S_LONG	-1	/* long:  display them */

static int shortflag;          /* Set when fast form of "who" is to run. */
static int ttystatus;          /* Set when write to tty status desired.  */
static int header=0;		/* Print a header                         */
static int quickly=0;		/* Just print the number of people and their names */
static int totaluser=0;	/* Count how many users logged in.        */

static char*	options;
static char *lc_time;

main(int argc, char **argv)
{
	static int entries[FLAGS+1];    /* Flag for each type entry */
	register struct utmp *utmp;
	register int i;
	char *ptr;
	struct passwd *passwd;
	char outbuf[BUFSIZ];
	extern int shortflag,ttystatus;
	int foundone = 0;               /* found a matching utmp entry */
	int	optlet;		/* an option letter found in getopt() */
	int	whoami, sflag;	/* D7660, This flag is used to indicate whether the sequence "who am i" is being used. */

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_WHO, NL_CAT_LOCALE);
	lc_time = setlocale(LC_TIME, 0); /* get what LC_TIME is set to */

/* Make "stdout" buffered to speed things up. */

	setbuf(stdout, outbuf);

/* Look up our own name for reference purposes. */

	if((passwd = getpwuid(getuid())) == (struct passwd *)NULL) {
		 username[0] = '\0';
	} else {
/* Copy the user's name to the array "username". */
		 strncpy(&username[0],passwd->pw_name, SIZEUSER);
		 username[SIZEUSER+1] = '\0';
	}


/* Set "ptr" to null pointer so that in normal case of "who" */
/* the "ut_line" field will not be looked at.  "ptr" is used */
/* only for "who am i" or "who -m" command. */

	ptr = NULL;
	whoami = FALSE;
	sflag = 0;
	shortflag = S_DFLT;	/* not set yet */
	ttystatus = FALSE;

/* Analyze the switches and set the flags for each type of entry */
/* that the user requests to be printed out. */

	options = "AabdHilmpqrsTtuw";


	while ( (optlet = getopt(argc,argv,options)) != EOF ) {
		switch ( optlet ) {
			case 'A' :
				entries[ACCOUNTING] = TRUE;
				break;
			case 'a' :
				entries[BOOT_TIME] = TRUE;
				entries[DEAD_PROCESS] = TRUE;
				entries[LOGIN_PROCESS] = TRUE;
				entries[INIT_PROCESS] = TRUE;
				entries[RUN_LVL] = TRUE;
				entries[OLD_TIME] = TRUE;
				entries[NEW_TIME] = TRUE;
				entries[USER_PROCESS] = TRUE;
				ttystatus = TRUE;
				shortflag = S_LONG;
				break;
			case 'b' :
				entries[BOOT_TIME] = TRUE;
				break;
			case 'd' :
				entries[DEAD_PROCESS] = TRUE;
				shortflag = S_LONG;
				break;
			case 'H' :
				header++;
				break;
			case '?' :
				usage("", "");
				break;
			case 'l' :
				entries[LOGIN_PROCESS] = TRUE;
				shortflag = S_LONG;
				break;
			case 'm' :	
				whoami = TRUE;
				break;
			case 'p' :
				entries[INIT_PROCESS] = TRUE;
				shortflag = S_LONG;
				break;
			case 'q' :
				quickly++;
				break;
			case 'r' :
				entries[RUN_LVL] = TRUE;
				break;
			case 's' :
				sflag++;
				break;
			case 't' :
				entries[OLD_TIME] = TRUE;
				entries[NEW_TIME] = TRUE;
				break;
			case 'w' :	
			case 'T' :
				ttystatus = TRUE;
				break;
			case 'i' :	
			case 'u' :
				entries[USER_PROCESS] = TRUE;
				shortflag = S_LONG;
				break;
		}
	}
	argc -= optind;	  /* skip the command name and processed options */
	argv = &argv[optind];
	for ( ; argc > 0 ; ++argv, --argc) {

/* Is this the "who am i" sequence? */
/* In the MSG version, both traditional "who am i" and local version of
   the "am i" are supported. */


		if ( strcmp(*argv,MSGSTR(AMI,"am i")) == 0 ) {
			whoami = TRUE;
			continue;
		}
		if ( argc >= 2 && strcmp(*argv,"am") == 0 ){ /* who am what? */
			if ( strcmp(*(argv+1),"i")==0 
			  || strcmp(*(argv+1),"I")==0 ) {
				whoami = TRUE;
				argc --;
				argv ++;
				continue;
			}
		}

/* Is there an argument left?  If so, assume that it is a utmp */
/* like file.  If there isn't one, use the default file. */

		if ( access(*argv,0) != FAILURE ) {
			utmpname(*argv);
		}else{
			usage(MSGSTR(NOEXST, 
				"%s doesn't exist or isn't readable"), *argv);
		}
	}

	if ( whoami ) {
	/* Which tty am I at?  Get the name and set "ptr" to just past */
	/* the "/dev/" part of the pathname. */

		if ((ptr = ttyname((int)fileno(stdin))) == NULL
		 && (ptr = ttyname((int)fileno(stdout))) == NULL
		 && (ptr = ttyname((int)fileno(stderr))) == NULL) {
		   usage(MSGSTR(NOTERM,"process not attached to terminal"),"");
		}
		ptr += sizeof("/dev/") - 1;

		entries[USER_PROCESS] = TRUE;
	}

/* Make sure at least one flag is set. */
	for ( i = 0 ; i <= FLAGS ; i++ ) {
		if ( entries[i] == TRUE )
			break;
	}
	if ( i > FLAGS )	/* No entries specified */
		entries[USER_PROCESS] = TRUE;

/* Some options overrides others */
	if ( quickly ) {
		ttystatus = FALSE;
		shortflag = S_SHORT;
	}
	if ( sflag ) {
		shortflag = S_SHORT;
	}

/* Now scan through the entries in the utmp type file and list */
/* those matching the requested types. */
	if (header) {
		fprintf(stdout,"%-*.*s%*.*s",
			LOGF_SIZE, LOGF_SIZE,
				MSGSTR(NAME,"Name    "),
			TTYS_HEAD, TTYS_HEAD,
				ttystatus ?  MSGSTR(TTYSTATUS,"ST") : "" );
		if ( !quickly ) {
			fprintf(stdout," %-*.*s %-*.*s",
				LINE_SIZE, LINE_SIZE,
					MSGSTR(WHO_LINE," Line"),
				TIME_HEAD, TIME_HEAD,
					MSGSTR(WHO_TIME,"   Time   ") );
			if (shortflag == S_LONG){
				fprintf(stdout," %*.*s",
					IDLE_HEAD, IDLE_HEAD,
						MSGSTR(ACTIVITY,"Activity"));
				if (!ttystatus)
					fprintf(stdout," %*.*s", 
						PID_SIZE,  PID_SIZE,
						MSGSTR(WHO_PID,"PID") );
			}else{
				fprintf(stdout," %*.*s",
					IDLE_HEAD, IDLE_HEAD, "" );
				/* the space for PID is skipped */
			}
		}
		if (!ttystatus) {
			fprintf(stdout," %-.*s", 
			HOST_SIZE, MSGSTR(WHO_HOST,"Hostname") );
			if ( shortflag != S_SHORT && entries[DEAD_PROCESS] == TRUE )
				fprintf(stdout,"%s", MSGSTR(WHO_EXIT,"/Exit"));
		}
		fprintf(stdout,"\n");
	}


	while((utmp = getutent()) != NULL) {
		short	utype = utmp->ut_type;

		/* Are we looking for this type of entry? */
		if ( utype < 0 || utype > FLAGS || entries[utype] == FALSE )
			continue;

		if ( utype == EMPTY ) {
			fprintf(stdout, MSGSTR(EMPTYSLT,"Empty slot.\n"));
			continue;
		}
		if (whoami&&strncmp(utmp->ut_line,ptr,sizeof(utmp->ut_line))!=0)
			continue;
		else{
			foundone = 1;	/* remember that we saw at least one*/
			totaluser++;
			if (quickly) {
				fprintf(stdout, "%s %-*s\n",user(utmp), HOST_SIZE, host(utmp));
				continue;
			}
		}

		fprintf(stdout, "%s %*.*s %s %-*.*s ",
				user(utmp), /* this is padded correctly */
				TTYS_SIZE, TTYS_SIZE, mode(utmp),
				line(utmp),
				TIME_SIZE, TIME_SIZE, ltime(utmp) );
		if (shortflag == S_LONG)
			fprintf(stdout, "%*.*s", IDLE_SIZE, IDLE_SIZE, etime(utmp) );
		if (!ttystatus) {
			if(utype == RUN_LVL) {
				fprintf(stdout, "    %c    %d    %c",
					utmp->ut_exit.e_termination,
					utmp->ut_pid, utmp->ut_exit.e_exit);

			}
			if(shortflag == S_LONG &&
			   (utype == LOGIN_PROCESS
			    || utype == USER_PROCESS
			    || utype == INIT_PROCESS
			    || utype == DEAD_PROCESS)) {
				fprintf(stdout, " %*d", 
					PID_SIZE, utmp->ut_pid );
				if(utype == INIT_PROCESS
				   || utype == DEAD_PROCESS) {
					fprintf(stdout, MSGSTR(ID, " id=%-*.*s"),
						ID_SIZE, ID_SIZE, id(utmp));
					if(utype == DEAD_PROCESS) {
						fprintf(stdout, MSGSTR(TERMID, " term=%d exit=%d"),
							utmp->ut_exit.e_termination, utmp->ut_exit.e_exit);
					}
				}
				else
					fprintf(stdout, " %-*s", HOST_SIZE,host(utmp));
			} else {
				fprintf(stdout, " %-*s", HOST_SIZE,host(utmp));
			}
		}
		fprintf(stdout,"\n"); 
		if ((totaluser % 25 ) == 0 )
		    fflush(stdout);
	}               /* End of "while ((utmp = getutent()" */
	if (quickly) 
		fprintf(stdout,"Total users: %d\n", totaluser);
	else if (!foundone && (entries[USER_PROCESS] == TRUE) && whoami )
		/* if no match for who am i, print something */
		if (ptr == NULL)
			fprintf(stdout, "%-*s \n",LOGF_SIZE, username);
		else
			fprintf(stdout, "%-*s %s \n",LOGF_SIZE, username,ptr);

	if (fclose(stdout) != 0) {
		perror("stdout");
		exit(1);
	}
	exit(0);
}


/*
 *  NAME:  usage
 *
 *  FUNCTION:  print out the usage statement
 *	      
 *  RETURN VALUE:  	 exit 1
 */

static struct id_n_msg {
	int	id;	/* message ID for the message catalog */
	char*	msg;	/* in case no catalogs available */
} options_usage[] = {
	ACC_ENT, "A\tAccounting entries\n",
	OPTIONS, "a\tAll (AbdHlprTtu) options\n",
	BOOTTM,  "b\tBoot time\n",
	DEADPROC,"d\tDead processes\n",
	HEADERS, "H\tDisplay a header (title)\n",
	PROCCNT, "l\tLogin processes\n",
	CURTERM, "m\tInformation about current terminal (same as 'am i')\n",
	PROCESS, "p\tProcesses other than getty or user process\n",
	QUICK,   "q\tQuick (only user and host name)\n",
	RUNLEVEL,"r\tRun level\n",
	SHRTFRM, "s\tShort form (suppress Activity and PID)\n",
	TTYMSG,  "T,w\tStatus of tty (+ writable, - not writable, x exclusive open, ? hung)\n",
	TIMECHG, "t\tTime changes\n",
	USEINFO, "u,i\tActivity and PID of shell\n",
	EOF, ""
};

static usage(char *format, char *arg1)
{
struct id_n_msg*  p = options_usage;

	fprintf(stderr,format,arg1);
	fprintf(stderr,MSGSTR(USAGE, 
		"\nUsage: who [-%s] [am {i,I}] [file]\n"), options );

	for ( p = options_usage ; p->id != EOF ; p++ )
		fprintf( stderr, MSGSTR(p->id, p->msg) );

	exit(1);
}

/*
 *  NAME:  user
 *
 *  FUNCTION:  	return a space padded buffer of the user
 *		name in a utmp structure.
 *	      
 *  RETURN VALUE: a pointer to the array is returned.
 */


static char *user(struct utmp *utmp)
{
	static char uuser[SIZEUSER*MB_LEN_MAX];

	if (utmp->ut_type == LOGIN_PROCESS)
		copypad(uuser,"LOGIN",SIZEUSER,LOGF_SIZE);
	else
		copypad(uuser,utmp->ut_user,SIZEUSER,LOGF_SIZE);
	return(uuser);
}

/*
 *  NAME: host
 *
 *  FUNCTION: return a space padded buffer of the host
 *             in a utmp structure.
 *
 *  RETURN VALUE: a pointer to the array is returned
*/
#define UTHSZ  	(sizeof(utmp->ut_host))
static char *host(struct utmp *utmp)
{
	static char uhost[UTHSZ + 4];

	if (*utmp->ut_host != '\0') {
		sprintf( uhost," (%s)", utmp->ut_host);
		return(uhost);
	}else
		return ("");
}

/*
 *  NAME:  line
 *
 *  FUNCTION:  	return a space padded buffer of the tty line
 *		name in a utmp structure.
 *	      
 *  RETURN VALUE: a pointer to the array is returned.
 */

static char *line(struct utmp *utmp)
{
	static char uline[SIZELINE*MB_LEN_MAX];

	copypad(uline, utmp->ut_line, SIZELINE,LINE_SIZE);
	return(uline);
}

/*
 *  NAME:  copypad
 *
 *  FUNCTION:  	Basically do a strcpy and pad the end with spaces.
 *
 *  RETURN VALUE: void
 *  OUTPUT:  *to - printable string from 'from', or "  .  "
 */

static void copypad(char *to, char *from, size_t size, int width)
{
	register int i, j, strwidth = 0, curwidth;
	short printable;
	wchar_t wc;

/* Scan for something textual in the field.  If there is */
/* nothing except spaces, tabs, and nulls, then substitute */
/* '.' for the contents. */

	printable = FALSE;

/* incrementor is length of current multi-byte char or 1 for single-byte */

	j=1;
	for (i=0; ((from[i] != '\0') && (i < size)); i+=j) {
		if (MB_CUR_MAX > 1) {
			j=mbtowc(&wc, from+i, MB_CUR_MAX);
			if ((j < 0) || (i+j > size)) {
				printable = FALSE;
				break;
			}
			curwidth=wcwidth(wc);
			if (curwidth == -1)
				curwidth = 1;
			if (strwidth + curwidth > width)
				break;
			strwidth += curwidth;
			if (iswgraph(wc))
				printable = TRUE;
		} else {
			if (i >= width)
				break;
			if (isgraph(from[i]))
				printable = TRUE;
		}
	}
	if (MB_CUR_MAX == 1)
		strwidth = i;
	if ( printable ) {
		strncpy(to, from, i);
		while (strwidth++ < width)
			to[i++] = ' ';		/* spaces are 1 column wide */
		to[i] = '\0';
	}else{
		int	halfway = width/2 - 1;       /* Where to put '.' */

/* Add pad at end of string consisting of spaces and a '\0'. */
/* Put an period at the halfway position.  (This only happens */
/* when padding out a null field.) */

		memset( to, ' ', width );
		to[ halfway ] = '.';
		to[ width ] = '\0';
	}
}


/*
 *  NAME:  id
 *
 *  FUNCTION:  return the uid string of the utmp structure.
 *
 */

static char *id(struct utmp *utmp)
{
	static char uid[sizeof(utmp->ut_id)+1];
	register char *ptr;
	register int i;

	for(ptr= &uid[0],i=0; i < sizeof(utmp->ut_id);i++) {
		if(isprint((int)utmp->ut_id[i]) || utmp->ut_id[i] == '\0') {
			*ptr++ = utmp->ut_id[i];
		} else {
			*ptr++ = '^';
			*ptr = (utmp->ut_id[i] & 0x17) | 0100;
		}
	}
	*ptr = '\0';
	return(&uid[0]);
}

/*
 *  NAME:  ltime
 *
 *  FUNCTION:  	Return a string pointer to login time.
 */

static char *ltime(struct utmp *utmp)
{
static	char	login_time[TIME_SIZE+1];
	struct tm *timestr;

	timestr = localtime(&utmp->ut_time);
	if (strcmp(lc_time, "C") && strcmp(lc_time,"POSIX"))
		strftime( login_time, TIME_SIZE+1, "%sD %sT",timestr);
	else
		strftime( login_time, TIME_SIZE+1, "%b %e %sT", timestr);
	login_time[ TIME_SIZE ] = '\0';

	return(login_time);
}

/*
 *  NAME:  etime
 *
 *  FUNCTION:  	Figure out the elapsed time since last user activity
 *		on a users tty line.
 *
 *  RETURN VALUE:	Return a string pointer to this information.
 */

#define	MIN_S	(time_t)60		/* minutes/hours/days in second */
#define	HOUR_S	(60 * MIN_S)
#define	DAY_S	(24 * HOUR_S)

static char *etime(struct utmp *utmp)
{
static	char	eetime[IDLE_SIZE+1];
	time_t lastactivity;
	char device[sizeof( "/dev/" ) + SIZELINE];
	struct stat statbuf;
	extern int shortflag;

	if( shortflag != S_LONG
	   || (utmp->ut_type != INIT_PROCESS 
	    && utmp->ut_type != LOGIN_PROCESS	/* no idle time necessary */
	    && utmp->ut_type != USER_PROCESS
	    && utmp->ut_type != DEAD_PROCESS) )	
	    	return "";	/* empty */

	sprintf(device,"/dev/%.*s", SIZELINE, utmp->ut_line);

/* If the device can't be accessed, put a period insted of time. */

	if ( stat(device,&statbuf) == FAILURE ) {
		strcpy(eetime,"   ?  ");
	}else{
/* Compute the amount of time since the last character was sent to */
/* the device.  If it is older than a day, just exclaim, otherwise */
/* if it is less than a minute, put in an '.', otherwise put in */
/* the hours and the minutes. */

		lastactivity = time((time_t *)NULL) - statbuf.st_mtime;
		if ( lastactivity > DAY_S ) {
			strcpy(eetime,"  old ");
		}else if ( lastactivity < MIN_S ) {
			strcpy(eetime,"   .  ");
		}else{
			sprintf(eetime,"%3u:%02.2u",
			     (unsigned)( lastactivity/HOUR_S),
			     (unsigned)((lastactivity/MIN_S)%MIN_S));
		}
	}
	return(eetime);
}

static int badsys;      /* Set by alarmclk() if open or close times out. */


/*
 *  NAME:  mode
 *
 *  FUNCTION:  
 * 		Check "access" for writing to the line.  To avoid
 *		getting hung, set up any alarm around the open.  If
 * 		the alarm goes off, print a question mark.
 *	      
 *  RETURN VALUE: 	"+" 	writable
 *			"-"	no write permission
 *			"?"	unknown
 */

static char *mode(struct utmp *utmp)
{
	char device[20];
	int fd;
	struct stat statbuf;
	register char *answer;
	extern ttystatus;

	if(ttystatus == FALSE
	   || utmp->ut_type == RUN_LVL
	   || utmp->ut_type == BOOT_TIME
	   || utmp->ut_type == OLD_TIME
	   || utmp->ut_type == NEW_TIME
	   || utmp->ut_type == ACCOUNTING
	   || utmp->ut_type == DEAD_PROCESS) return(" ");

	sprintf(&device[0],"/dev/%s",&utmp->ut_line[0]);


#ifdef DO_OPEN
	badsys = FALSE;
	(void) signal(SIGALRM,(void (*)(int))alarmclk);
	alarm((unsigned int)3);
#ifdef  CBUNIX
	fd = open(&device[0],O_WRONLY);
#else
	fd = open(&device[0],O_WRONLY|O_NDELAY);
#endif
	alarm((unsigned int)0);
	if(badsys) return("?");

	if(fd == FAILURE) {
/* If our effective id is "root", then send back "x", since it */
/* must be exclusive use. */

		if(geteuid() == 0) return("x");
		else return("-");
	}
#endif

/* If we are effectively root or this is a login we own then we */
/* will have been able to open this line except when the exclusive */
/* use bit is set.  In these cases, we want to report the state as */
/* other people will experience it. */

#ifdef DO_OPEN
	if(fstat(fd,&statbuf) == FAILURE) answer = "-";
	else if(faccessx(fd,W_ACC,ACC_ALL) == 0) answer = "+";
#else
	if(stat(device,&statbuf) == FAILURE) answer = "-";
	else if(accessx(device,W_ACC,ACC_ALL) == 0) answer = "+";
#endif
/* TCSEC DAC mechanism */
	else answer = "-";

/* To avoid getting hung set up any alarm around the close.  If */
/* the alarm goes off, print a question mark. */

#ifdef DO_OPEN
	badsys = FALSE;
	(void) signal(SIGALRM,(void (*)(int))alarmclk);
	alarm((unsigned int)3);
	close(fd);
	alarm((unsigned int)0);
	if(badsys) answer = "?";
#endif
	return(answer);
}

static alarmclk(void)
{
/* Set flag saying that "close" timed out. */

	badsys = TRUE;
}

