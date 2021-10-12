static char sccsid[] = "@(#)93  1.19.1.22  src/bos/usr/bin/w/w.c, cmdstat, bos41B, 9504A 12/21/94 13:41:52";
/*
 * COMPONENT_NAME: (CMDSTAT) status
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26, 27, 83
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
/*
 * LEVEL 1,5 Years Bull Confidential Information
 */

#define _ILS_MACROS
#include <time.h>
#include <string.h>
#include <sys/param.h>
#include <stdio.h>
#include <ctype.h>
#include <utmp.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <procinfo.h>
#include <errno.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/ioctl.h>
#include <mbstr.h>

#include        <nl_types.h>
#include        "w_msg.h"
static nl_catd catd;

#define RIGHT		  0	/* right justify for wcstring */
#define LEFT		  1	/*  left justify for wcstring */
#define TRUNC		0x1	/* Truncate flag for wcstring */
#define PAD		0x2	/*      Pad flag for wcstring */
#define	WBUFSIZE	256 	/* time/date length to allow for all locales */
#define MSGSTR(num,str)  catgets(catd,MS_W,num,str)  /*MSG*/

#define PROCSIZE	sizeof (struct procsinfo)
#define THRDSIZE	sizeof (struct thrdsinfo)

#define USER_ENTRY(UTMP)	((utmp.ut_type == USER_PROCESS) \
					&& (utmp.ut_user[0] != '\0'))

#define NMAX sizeof(utmp.ut_user)
#define LMAX sizeof(utmp.ut_line)

#define	MIN_S	(time_t)60		/* 1 min = MIN_S seconds... */
#define	HOUR_S	(60 * MIN_S)
#define	DAY_S	(24 * HOUR_S)

#define	HOUR_M	(time_t)60		/* 1 hour = HOUR_M minuets */
#define	DAY_M	(24 * HOUR_M)

#define	USERFMT		8
#define	TTYFMT		9
#define	LOGINFMT	9
#define	IDLEFMT		10
#define	JCPUFMT		9
#define	PCPUFMT		9
#define	WHATFMT		4

/*
 * Must agree with SBITS in sys/prod/sched.c
 */
#define FSHIFT	   16
#define FSCALE 	   (1<<FSHIFT)

struct procsinfo * getprocdata( long *nproc );
struct thrdsinfo *getthreaddata();
static void   readpr( void );
static void   prtat( time_t *, int );
static void   prttime( time_t, int );
static void   putline( void );
static void   gettty( void );
static time_t findidle( void );
static lastchild( pid_t );
wchar_t * wcstring( char *, size_t, int, int );

#define ARGWIDTH	33	/* # chars left on 80 col crt for args */

static struct pr {
	pid_t	w_pid;			/* proc.p_pid */
	pid_t	w_ppid;			/* proc.p_ppid */
	pid_t	w_pgrp;			/* proc.p_gid */
	char	w_flag;			/* proc.p_flag */
	short	w_size;			/* proc.p_size */
	long	w_seekaddr;		/* where to find args */
	long	w_lastpg;		/* disk address of stack */
	int	w_chan;			/* MPX channel */
	int	w_start;		/* start time of process */
	time_t	w_time;			/* CPU time used by this process */
	time_t	w_ctime;		/* CPU time used by children */
	dev_t	w_tty;			/* tty device of process */
	uid_t	w_uid;			/* uid of process */
	char	w_comm[15];		/* user.u_comm, null terminated */
	char	w_args[ARGWIDTH+1];	/* args if interesting process */
} *pr;
static long	nproc;
static int	runqueue=0;

static unsigned long avenrun[3];

static FILE	*ut;
static dev_t	tty;
static uid_t	uid;
static int		chan=-1;	/* for multiplexed tty's, -1 means not multiplexed */
static int 	t_pgrp = -1;	/* tty's current process group */
static char	doing[520];	/* process attached to terminal */
static char	tmpbuff[10];	/* debug proctime and jobtime info */
static time_t	proctime;	/* cpu time of process in doing */

#define	DIV60(t)	((t+30)/60)    /* x/60 rounded */ 

#define	TTYEQ		((tty == pr[i].w_tty) && \
			 (uid == pr[i].w_uid) && \
			 ((chan != -1) ? chan == pr[i].w_chan : 1))

#define max(a,b)	((a) > (b) ? (a) : (b))

#define IGINT		(1+3*1)		/* ignoring both SIGINT & SIGQUIT */
static int	debug = 0;	/* true if -d flag; debug info */
static int	ttywidth = 80;	/* width of tty (sigh... it is hard coded) */
static int	header = 1;	/* true if -h flag: don't print heading */
static int	lflag = 1;	/* true if -l flag: long style output */
static int	uflag = 0;	/* true if -u flag: equivalent to uptime info */
static int	login;		/* true if invoked as login shell */
static time_t	idle;		/* number of minutes user is idle */
static int	nusers;		/* number of users logged in now */
static char *	sel_user;	/* login of particular user selected */
static time_t	jobtime;	/* total cpu time visible */
static time_t	now;		/* the current time of day */
static time_t	boottime=0;
static time_t	uptime;		/* time of last reboot & elapsed time since */
static int	np;		/* number of processes currently active */
static struct	utmp utmp;
static struct	procsinfo *mproc;
static int	len_user,	/* Widths of translated title strings */
	len_tty,
	len_login,
	len_idle,
	len_jcpu,
	len_pcpu,
	len_what;

static char	*day_str, *days_str;

main(
	int argc,
	char **argv
	)
{
	int optlet, doit;
	int days, hrs, mins;
	register int i, j;
	char *cp, firstchar;
	register int empty;
	register pid_t curtime;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_W,NL_CAT_LOCALE);

	login = (argv[0][0] == '-');
	cp = rindex(argv[0], '/');
	firstchar = login ? argv[0][1] : (cp==NULL) ? argv[0][0] : cp[1];
	uflag = (firstchar == 'u');

		while ( (optlet = getopt(argc,argv,"hlsuwd")) != EOF )
		{
				switch(optlet) {

				case 'd':
					debug++;
					break;
				case 'h':
					header = 0;
					break;

				case 'l':
					lflag++;
					break;

				case 's':
					lflag = 0;
					break;

				case 'u':
					uflag = 1;
					break;

				case 'w':
					uflag = 0;
					break;

				default:
					fprintf(stderr, MSGSTR(USAGE,"Usage: %s [ -hlsuw ] [ user ]\n"), argv[0]);
					exit(1);
				}
		}
		sel_user = argv[optind]; 


/* get the proc structure */


	readpr();
	day_str = MSGSTR(DAY, "day");
	days_str = MSGSTR(DAYS, "days");

	if ((ut = fopen(UTMP_FILE,"r")) == NULL) {
		fprintf(stderr,MSGSTR(BADOPEN,"Cannot open %s.\n"),UTMP_FILE);
		exit(1);
	}
	time(&now);
	if (header) {
		/* Print time of day */
		prtat(&now,0);

		/* get bootime from utmp.... */

		while (fread((void *)&utmp, (size_t)sizeof(utmp), (size_t)1, ut)) {
			if (utmp.ut_type == BOOT_TIME) {
				boottime = utmp.ut_time;
				break;
			}
		}
		rewind(ut);

		uptime = now - boottime;
		if ( uptime < 0 ) {
			fprintf(stderr, MSGSTR(INVTIME, 
				"(boot or current time is wrong)"));
		}else{
			uptime += 30;
			days = uptime / DAY_S;
			uptime %= DAY_S;
			hrs = uptime / HOUR_S;
			uptime %= HOUR_S;
			mins = uptime / MIN_S;

			printf("  %s",MSGSTR(UP,"up"));
			if ( days > 1 )
				printf(" %d %s,",days,days_str);
			 else if ( days == 1 ) 
				printf(" %d %s,",days,day_str); 

			if (hrs > 0 && mins > 0) {
				printf( "  %2d:%02d,", hrs, mins );
			}else{
			  if ( hrs > 1 )
				printf(" %d %s,",hrs, MSGSTR(HOURS, "hrs"));
			  else if ( hrs == 1 )
				printf(" %d %s,",hrs, MSGSTR(HOUR, "hr"));

			  if ( mins > 1 )
				printf(" %d %s,",mins,MSGSTR(MINUTES,"mins"));
			  else if ( mins == 1 )
				printf(" %d %s,",mins,MSGSTR(MINUTE, "min"));
			}
		}
		/* Print number of users logged in to system */
		while (fread((void *)&utmp, (size_t)sizeof(utmp), (size_t)1, ut)) {
			if (USER_ENTRY(utmp))
				nusers++;
		}
		rewind(ut);
		if (nusers == 1)
			printf("  %d %s", nusers, MSGSTR(USER, "user"));
		else
			printf("  %d %s", nusers, MSGSTR(USERS, "users"));

		get_avenrun(avenrun);
		/*
		 * Print 1, 5, and 15 minute load averages.
		 */
		printf(MSGSTR(LOAD,",  load average: %.2f, %.2f, %.2f\n"),
		       (double)avenrun[0]/FSCALE, (double)avenrun[1]/FSCALE,
		       (double)avenrun[2]/FSCALE);

		if (uflag)
			exit(0);
	}

	/* Headers for rest of output */

	len_user  = strlen(MSGSTR(H_USER,"User"));
	len_user  = max( len_user, USERFMT );
	len_tty   = strlen(MSGSTR(H_TTY,"tty"));
	len_tty   = max( len_tty, TTYFMT);
	len_login = strlen(MSGSTR(H_LOGIN,"login@"));
	len_login = max( len_login, LOGINFMT);
	len_idle  = strlen(MSGSTR(H_IDLE,"idle"));
	len_idle  = max( len_idle, IDLEFMT);
	len_jcpu  = strlen(MSGSTR(H_JCPU,"JCPU"));
	len_jcpu  = max( len_jcpu, JCPUFMT);
	len_pcpu  = strlen(MSGSTR(H_PCPU,"PCPU"));
	len_pcpu  = max( len_pcpu, PCPUFMT);
	len_what  = strlen(MSGSTR(H_WHAT,"what"));
	len_what  = max( len_what, WHATFMT);

	if (header) {
		if (lflag) {
			printf("%-*.*s %-*.*s %*.*s %*.*s %*.*s %*.*s %-*.*s\n",
				len_user,len_user,MSGSTR(H_USER,"User"),
				len_tty,len_tty,MSGSTR(H_TTY,"tty"),
				len_login,len_login,MSGSTR(H_LOGIN,"login@"),
				len_idle,len_idle,MSGSTR(H_IDLE,"idle"),
				len_jcpu,len_jcpu,MSGSTR(H_JCPU,"JCPU"),
				len_pcpu,len_pcpu,MSGSTR(H_PCPU,"PCPU"),
				len_what,len_what,MSGSTR(H_WHAT,"what"));
		} else {
			printf("%-*.*s %-*.*s %*.*s %-*.*s\n",
				len_user,len_user,MSGSTR(H_USER,"User"),
				len_tty,len_tty,MSGSTR(H_TTY,"tty"),
				len_idle,len_idle,MSGSTR(H_IDLE,"idle"),
				len_what,len_what,MSGSTR(H_WHAT,"what"));
		}
		fflush(stdout);
	}

	while (fread((void *)&utmp, (size_t)sizeof(utmp), (size_t)1, ut) != (size_t)NULL) {
		if (!USER_ENTRY(utmp))
			continue;	/* that tty is free */
		if (sel_user && strncmp(utmp.ut_name, sel_user, (size_t)NMAX) != 0)
			continue;	/* we wanted only somebody else */

		gettty();
		jobtime = 0;
		proctime = 0;
		strcpy(doing, "-");	/* default act: normally never prints */
		empty = 1;
		curtime = 0;
		idle = findidle();
		for (i=0; i<np; i++) {	/* for each process on this tty */
			if (!(TTYEQ))
				continue;
			jobtime += pr[i].w_time + pr[i].w_ctime;
			proctime += pr[i].w_time;
			doit = 0;
			if (t_pgrp == -1) {
				if (pr[i].w_start > curtime)
					doit = 1;
			} else {
				if ((pr[i].w_start >= curtime) &&
				    (pr[i].w_pgrp == t_pgrp) &&
				    (lastchild(pr[i].w_pid)))
					doit = 1;
			}
			if (doit) {
				curtime = pr[i].w_start;
				if (lflag)
				    strcpy(doing, pr[i].w_args);
				else
				    strcpy(doing, pr[i].w_comm);
				if ( (doing[0] == 0) ||
				    ((doing[0] == '-') && (doing[1] <= ' ')) ||
				     (doing[0] == '?')) {
					strcat(doing, " (");
					strcat(doing, pr[i].w_comm);
					strcat(doing, ")");
				}
			}
		}
		putline();
	}
	fclose(ut);
	exit(0);
}

/*
 *  NAME:  gettty
 *
 *  FUNCTION:  
 * 	Figure out the major/minor device # pair for this tty
 *
 *	set global variables tty, uid and chan...
 *	      
 *  RETURN VALUE: none
 *		
 */

static void
gettty( void )
{
	char ttybuf[20];
	char *ptr;
	struct stat statbuf;
	int fd;
	int j;

	ttybuf[0] = 0;
	strcpy(ttybuf, "/dev/");
	strcat(ttybuf, utmp.ut_line);
	if ((fd = open(ttybuf, O_RDONLY|O_NONBLOCK)) >= 0) {
		fstat(fd,&statbuf);
		if (ioctl(fd, TIOCGPGRP, &t_pgrp))
			t_pgrp = -1;
		else if (0 == t_pgrp)
			t_pgrp = -1;
		close(fd);
	} else {
		stat(ttybuf,&statbuf);
		t_pgrp = -1;
	}
	tty = statbuf.st_rdev;
	uid = statbuf.st_uid;

	/*
	 * Now check to see whether the process group we got for this tty
	 * has the same uid as the uid of this tty.
	 * If YES  - then continue
	 *    ELSE - assign it -1 and use algorithm before defect 76011
	 */
	if ( t_pgrp != -1 ) {
		for (j=0; j<np; j++) { /* for each process on this tty */
			if (pr[j].w_pgrp == t_pgrp && pr[j].w_uid != uid) {
				t_pgrp = -1;
				break;
			}
		}
	}

	/*
	 * if this looks like a /dev/<whatever>/<channel> (ie a multiplexed
	 * device) whack off the last component and stat it instead.  we
	 * cannot assume that the channel device's ISVTX bit is on (since
	 * the owner could chmod it off).  however, it's a lot tougher to
	 * chmod the ISVTX bit off the base multiplexed device (eg /dev/hft).
	 * so we'll look there to determine multiplexed or not.  this still
	 * isn't fool-proof but until something better comes along.....
	 */
	if (((ptr = strrchr(ttybuf,'/')) != NULL) && (isdigit(ptr[1]))) {
		*ptr = '\0';		/* whack off last component */
		stat(ttybuf, &statbuf);
		*ptr = '/';		/* restore last component */
		}
	if (statbuf.st_mode & S_ISVTX) {
		if (((ptr = strrchr(ttybuf,'/')) != NULL) && (isdigit(ptr[1])))
			chan = atoi(ptr+1);
		else
			fprintf(stderr,"\nw: gettty: Unexpected tty buffer.\n");
	} else {
		chan = -1;
	}
}

/*
 *  NAME:  putline
 *
 *  FUNCTION:  
 *				Print out the accumulated line of info about one user.
 *	      
 *  RETURN VALUE:  	 none
 */

static void
putline( void )
{

int len_daystr=0;		
int len;

	/* print the login name of the user */

	printf("%S ", wcstring(utmp.ut_user, len_user, LEFT, TRUNC|PAD));

	/* print the tty user is on */
	if (lflag)
		/* long form: all (up to) field len max chars */
		printf("%S ", wcstring(utmp.ut_line, len_tty, LEFT, TRUNC|PAD));
	else 
		printf("%S ", wcstring(utmp.ut_line, len_tty, LEFT, TRUNC|PAD));

	if (lflag)
		/* print when the user logged in */
		prtat( &utmp.ut_time, len_login);

	/* print idle time */
	if ( idle >= DAY_M ) {
		int	dlen;
		int	idays = (idle + (DAY_M / 2)) / DAY_M;
		char	*dstr;
		wchar_t	*wc_dstr;

		dstr = (idays > 1) ? days_str : day_str;
		dlen = strlen(dstr);
		if (MB_CUR_MAX > 1) {
			wc_dstr = wcstring(dstr, dlen, RIGHT, 0);
			printf("%*d%S",
				(len_idle - ((len=wcswidth(wc_dstr,WBUFSIZE)) == -1 ? 1 : len)),
				idays, wc_dstr);
		} else {
			printf("%*d%s",(len_idle - dlen), idays, dstr);
		}
	}else
		prttime(idle, len_idle);

	if (lflag) {
		/* print CPU time for all processes & children */
		prttime(jobtime,len_jcpu+1);
		/* print cpu time for interesting process */
		prttime(proctime,len_pcpu+1);
	}

	/* what user is doing, either command tail or args */
	if ( lflag )
		ttywidth -= (len_user+len_tty+len_login+
				len_idle+len_jcpu+len_pcpu+6+1);
	else
		ttywidth -= (len_user+len_tty+len_idle+3+1);
					/* 3 terms, 1 heading-space */

	printf(" %S\n", wcstring(doing, ttywidth, LEFT, TRUNC));

	if (debug) 
		printf( "jobtime: %-8d proctime: %-8d\n\n",jobtime,proctime);  

	ttywidth = 80;
}

/*
 *  NAME:  findidle
 *
 *  FUNCTION:  
 * 	find & return the number of minutes the current tty has been idle
 *	      
 *  RETURN VALUE:  	 idle time
 */

static time_t
findidle( void )
{
	struct stat stbuf;
	time_t lastaction, diff;
	char ttyname[20];

	strcpy(ttyname, "/dev/");
	strncat(ttyname, utmp.ut_line, (size_t)LMAX);
	stat(ttyname, &stbuf);
	time(&now);
	lastaction = stbuf.st_atime;
	diff = now - lastaction;
	diff = DIV60(diff);
	if (diff < 0) diff = 0;
	return((time_t)diff);
}

static void
prttime( time_t tim, int len )
{
	char str[80];

	if ( tim >= 0 ) {
		if (tim >= MIN_S)
			sprintf(str,"%3d:%02d", tim / HOUR_M, tim % HOUR_M );
		else
			sprintf(str,"%d", tim );

		printf("%S", wcstring(str,len, RIGHT, TRUNC));
	}
}

/*
 *  NAME:  prtat
 *
 *  FUNCTION:  
 * 	prtat prints a 12 hour time given a pointer to a time of day 
 *	      
 *  RETURN VALUE:	none
 */

static void
prtat( time_t* time, int len )
{
register time_t past;
	 char	*fmt, str[WBUFSIZE];

	past = (now > *time) ? now - *time : 0;	/* 0 - time changed/invalid */
	if ( past <= 24 * HOUR_S )
		fmt = "%I:%M" "%p";	/* 12:59pm */
	else if ( past <= 7 * DAY_S )
		fmt = "%a%I"  "%p";	/* Mon11am */
	else
		fmt = "%d%b%y";		/* 31Mar91 */
 
	/*	the format specifier for strftime() has sometimes to be
		divided. e.g. '%-M-%-p' is recognized as '%-M-%' and 'p' 
		by SCCS, and will be expanded such as "93p". 
	*/
	strftime( str, WBUFSIZE, fmt, localtime(time) );
	if ( len == 0 )
		printf("%S ", wcstring(str, LOGINFMT, RIGHT, 0));
	else
		printf("%S ", wcstring(str, len, RIGHT, TRUNC));
}

/*
 *  NAME:  readpr
 *
 *  FUNCTION:  
 * 	readpr finds and reads in the array pr, containing the interesting
 * 	parts of the proc and user tables for each live process.  readpr
 *	also counts the number of runable processes and stores them in
 *	the runqueue variable.
 *	      
 *  RETURN VALUE:  	none 
 */

static void
readpr()
{
	int pn;
	struct thrdsinfo *thrd, *th;
	struct procsinfo *proc_pt;
	int nthread;
	int i;
	int done;

	mproc = getprocdata (&nproc);
	proc_pt = mproc;
	pr = (struct pr *)calloc((size_t)nproc, (size_t)sizeof(struct pr));
	if ( pr == NULL) {
		perror ("malloc: ");
		exit (1);
	}
	np = 0;

	for (pn=0; pn<nproc; pn++,mproc++){
		if (mproc->pi_state == SACTIVE) {
			thrd = getthreaddata(mproc->pi_pid, &nthread);
			i=0;
			done = FALSE;
			do {
				th = &thrd[i];
				if (th->ti_state == TSRUN) {
					runqueue ++;
					done = TRUE;
				}
				i++;
			} while ((done != TRUE) && (i < nthread));
		}
		free(thrd);

		/* decide if it's an interesting process */

		if (mproc->pi_state==0 || mproc->pi_state==SZOMB || mproc->pi_pgrp==0)
			continue;

		if (mproc->pi_ttyd == (long)NULL)
			continue;
		/* save the interesting parts */
		pr[np].w_pid = mproc->pi_pid;
		pr[np].w_ppid = mproc->pi_ppid;
		pr[np].w_pgrp = mproc->pi_pgrp;
		pr[np].w_flag = mproc->pi_flags;
		pr[np].w_size = mproc->pi_size;
		pr[np].w_time = mproc->pi_ru.ru_utime.tv_sec + mproc->pi_ru.ru_stime.tv_sec;
		pr[np].w_ctime = mproc->pi_cru.ru_utime.tv_sec + mproc->pi_cru.ru_stime.tv_sec;
		pr[np].w_tty = mproc->pi_ttyd;
		pr[np].w_chan = mproc->pi_ttympx;
		pr[np].w_start = mproc->pi_start;
		pr[np].w_uid = mproc->pi_uid;
		mproc->pi_comm[14] = 0;	/* Bug: This bombs next field. */
		strcpy(pr[np].w_comm, mproc->pi_comm);
		/*
		 * Get args if there's a chance we'll print it.
		 * Cant just save pointer: getargs returns static place.
		 * Cant use strcpyn: that crock blank pads.
		 */
		pr[np].w_args[0] = 0;
		if (getargs(mproc,PROCSIZE, pr[np].w_args, ARGWIDTH) != 0)
		{
			strcat(pr[np].w_args, " (");
			strcat(pr[np].w_args, pr[np].w_comm);
			strcat(pr[np].w_args, ")");
		}
		np++;
	}

	if (runqueue > 0)      /* Subtract the "Wait" process from the total */
		runqueue--;

	free(proc_pt);
}

/***********************************************************************
 * NAME: getprocdata( nproc )
 *
 * FUNCTION: Routine stolen from ps.  Grabs enough memory to read in
 *		     the proc structure, set nprocs and returns a pointer the
 *		     the interesting procs returned from the getproc systm call.
 *
 * EXECUTION ENVIRONMENT:
 *           
 * (NOTES):
 * (RECOVERY OPERATION:)
 * (DATA STRUCTURES:)
 * RETURNS: pointer to the procsinfo structure.
**************************************************************************/
static struct procsinfo *
getprocdata (nproc)
long *nproc;
{
        long temp;
        long multiplier;
        struct procsinfo *Proc;
	struct procsinfo *P;
	pid_t	index;
	int	count;
	int	ret;
	short	again;

	*nproc = 0;
	again = 1;
        multiplier = 5;
	index = 0;
	count = 1000000;

	Proc = NULL;
	P = NULL;
	do {
		if ((ret = getprocs(P, PROCSIZE, NULL, 0,&index, count)) != -1) {
			if (P == NULL) {				/* first call */
				count = ret + (multiplier <<= 1); 	/* Get extra in case busy system*/
                		Proc = (struct procsinfo *) malloc ((size_t)(PROCSIZE * count));
				P = Proc;
                		if ( Proc == NULL) {
        				/* We ran out of space before we could read */
                               		/* in the entire proc structure.            */
                       			 perror ("malloc: ");
                       			 exit (1);
				}
				index = 0;
			}
			else {
				*nproc += ret;
				if (ret >=  count) {			/* Not all entries were retrieved */ 
					count = (multiplier <<= 1);
                			Proc = (struct procsinfo *) realloc ((void *)Proc, (size_t)(PROCSIZE * (*nproc + count)));
                			if ( Proc == NULL) {
        			                         /* We ran out of space before we could read */
                               				 /* in the entire proc structure.            */
                       				 perror ("realloc: ");
                       				 exit (1);
                			}
					else 
						P = Proc + (*nproc) * PROCSIZE;
				}
				else				/* All entries were retrieved */
					again = 0;
			}
		}
	} while (again && (ret != -1));
	return (Proc);
}

/******************************************************************************
 * NAME: getthreaddata
 *
 * FUNCTION: fetch information found in the structure thrdsinfo for the threads of a 
 * given process
 *
 * NOTES: N/A
 *
 * RETURNS: struct thrdsinfo
 *****************************************************************************/
static struct thrdsinfo *
getthreaddata (pid,nthread)
pid_t pid;
long *nthread;
{
        long multiplier;
        struct thrdsinfo *Thread;
	struct thrdsinfo *T;
	tid_t	index;
	int	count;
	int	ret;
	short	again;

	*nthread = 0;
	again = 1;
        multiplier = 5;
	index = 0;
	count = 1000000;

	Thread = NULL;
	T = NULL;
	do {
		if ((ret = getthrds(pid, T, THRDSIZE, &index, count)) != -1) {
			if (T == NULL) {				/* first call */
				count = ret + (multiplier <<= 1); 	/* Get extra in case busy system*/
                		Thread = (struct thrdsinfo *) malloc ((size_t)(THRDSIZE * count));
				T = Thread;
                		if ( Thread == NULL) {
        				/* We ran out of space before we could read */
                               		/* in the entire thread structure.          */
                       			 perror ("malloc: ");
                       			 exit (1);
				}
				index = 0;
			}
			else {
				*nthread += ret;
				if (ret >=  count) {			/* Not all entries were retrieved */ 
					count = (multiplier <<= 1);
                			Thread = (struct thrdsinfo *) realloc ((void *)Thread, (size_t)(THRDSIZE * (*nthread + count)));
                			if ( Thread == NULL) {
        			                         /* We ran out of space before we could read */
                               				 /* in the entire thread structure.            */
                       				 perror ("realloc: ");
                       				 exit (1);
                			}
					else 
						T = Thread + (*nthread) * THRDSIZE;
				}
				else				/* All entries were retrieved */
					again = 0;
			}
        	}
	} while (again && (ret != -1));
        return (Thread);
}

static get_avenrun(ulong *avenrun)
{
    int kmem = open("/dev/kmem", 0);
    static struct nlist nl[] = {
	{ "avenrun" },
    };

    if (kmem < 0)
	return;
    knlist(nl, 1, sizeof(struct nlist));
    if (lseek(kmem, nl[0].n_value, 0) != -1)
	read(kmem, avenrun, sizeof(avenrun) * 3);
    (void) close(kmem);
}

/*******************************************************************/
/* NAME:	wcstring                                           */	
/* FUNCTION:	If the string is printable then convert into a     */
/*		wide char string which, when handled by the printf */ 
/*		conversion specifiers, will be formated correctly. */
/* ARGUMENTS:	fromstr         string to be converted             */
/*		size            size of field                      */
/*		justify         0 right; 1 left                    */
/*		flags           0x1 Truncate; 0x2 Pad              */
/* RETURN:	pointer to the converted string.                   */
/*******************************************************************/
			

static wchar_t *
wcstring(const char *fromstr, size_t size, int justify, int flags)
{
	size_t curwidth, curlen, i;
	static char frombuf[WBUFSIZE];
	static wchar_t wcstr[WBUFSIZE], tmpstr[WBUFSIZE];

	/* some buffers are not null terminated */
	strncpy(frombuf, fromstr, size);
	frombuf[size] = '\0';

    	if ((curlen = mbstowcs(tmpstr, frombuf, WBUFSIZE)) == (size_t) -1)
		curlen=0;

	for(i=0; i<curlen; i++) {
		if (!iswprint(tmpstr[i]))
			tmpstr[i] = L' ';
	}

	tmpstr[curlen] = L'\0';

	if (MB_CUR_MAX == 1)
	    curwidth=curlen;
	else {
	    curwidth=wcswidth(tmpstr, WBUFSIZE);
		if (curwidth == -1)
			curwidth = 1;
	}

	i = 0;
	wcstr[0] = L'\0';
	if (justify == LEFT) {
		wcscat(wcstr, tmpstr);	
		i += curlen;
	}
	if ((justify == RIGHT) || (flags & PAD)) {
		while (curwidth < size) {
			curwidth++;
			wcstr[i++] = L' '; /* wide space */
		}
		wcstr[i] = L'\0';
	}
	if (justify == RIGHT) {
		wcscat(wcstr, tmpstr);	
		i += curlen;
	}

	if (flags & TRUNC) {
		while (curwidth > size)
			curwidth--;
		wcstr[curwidth] = L'\0';
	}

	return(wcstr);
}

static int lastchild( pid_t pid )
{
	struct pr *ptmp;

	for (ptmp = pr + np; --ptmp >=pr; )
		if ((ptmp->w_ppid == pid) &&
		    (ptmp->w_uid  == uid) &&
		    (ptmp->w_pgrp == t_pgrp))
			return(0);
	return(1);
}
