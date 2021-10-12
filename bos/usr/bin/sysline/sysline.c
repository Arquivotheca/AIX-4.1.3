static char sccsid[] = "@(#)25  1.20  src/bos/usr/bin/sysline/sysline.c, cmdstat, bos41B, 9504A 12/21/94 13:40:49";
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
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
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
 * LEVEL 1, 5 Years Bull Confidential Information
 */
/* sysline - system status display on 25th line of terminal
 *
 * Written:  J. K. Foderaro
 * Modified: A. E. Lang to run on AIX/386 and AIX/370.
 * Modified: J. P. Le Goaller for thread support
 *
 * Prints a variety of information on the special status line of
 * terminals that have a status display capability. 
 * By default, all information is printed, and flags are given on the
 * command line to disable the printing of information.
 * The information and disabling flags are:
 *
 *  flag	what
 *  -----	----
 *		Time of day
 *		Load average and change in load average in the last 5 mins
 *		Number of users logged on
 *   -p		Number of processes the users owns which are runnable and
 *     		the number which are suspended.
 *     		Processes whose parent is 1 or -1 are not counted.
 *   -l		Users who've logged on and off.
 *   -m		Summarize new mail which has arrived
 *   -H     	remote status
 *   -D         print Day/Date before time
 *
 *  <Other flags>
 *   -r		Use non reverse video
 *   -c		Turn off 25th line for 5 seconds before redisplaying.
 *   -b		Beep once one the half hour, twice on the hour
 *   +N		Refresh display every N seconds.
 *   -i		Print pid first thing
 *   -e		Do simple print designed for an emacs buffer line
 *   -w		Do the right things for a window
 *   -h		Print hostname between time and load average
 *   -D		Print day/date before time of day
 *   -d		Debug mode - print status line data in human readable format
 *   -q		Quiet mode - don't output diagnostic messages
 *   -s		Print Short (left-justified) line if escapes not allowed
 *   -j		Print left Justified line regardless
 */

/**********************************************************************/
/* Include File                                                       */
/**********************************************************************/

#define _ILS_MACROS
#include  <stdio.h>
#include  <string.h>
#include  <sys/param.h>
#include  <sys/signal.h>
#include  <utmp.h>
#include  <ctype.h>
#include  <sys/time.h>
#include  <sys/stat.h>
#include  <sys/vtimes.h>
#include  <sys/proc.h>
#include  <procinfo.h>
#include  <nlist.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <locale.h>
#include  <errno.h>
#include  <time.h>
#include  <pwd.h>
#include  <curses.h>
#include  <term.h>
#include  <sys/ioctl.h>

#include  <nl_types.h>
#include  "sysline_msg.h"

/**********************************************************************/
/* Constant Definition / Macro Function                               */
/**********************************************************************/

#define  HOSTNAME  1    /* 4.1a or greater, with hostname() */
#define  RWHO      0    /* 4.1a or greater, with rwho */

#define  DEFDELAY  60   /* Update status once per minute */

#define  NETPREFIX  "ucb"

#define  MSGSTR(Num,Str)  catgets(catd,MS_SYSLINE,Num,Str)  

#define  LINESIZE	sizeof(old->ut_line)
#define  NAMESIZE	sizeof(old->ut_name)
#define  PROCSIZE	sizeof(struct procsinfo)
#define  THRDSIZE	sizeof(struct thrdsinfo)
/*
 * Status codes to say what has happened to a particular entry in utmp.
 * NOCH means no change, ON means new person logged on,
 * OFF means person logged off.
 */
#define  NOCH  0x0    /* No change */
#define  ON    0x1    /* Logged on */
#define  OFF   0x2    /* Logged off */

/**********************************************************************/
/* Function Prototype Declaration                                     */
/**********************************************************************/

struct procsinfo  *getprocdata();
struct thrdsinfo *getthreaddata();
static void  ttyprint();
static void  whocheck();
static void  timeprint();
static void  printinfo();

char  *sysrup();
char  *strcpy1();
int   outc();
int   clearbotline(void);

/**********************************************************************/
/* Global / External Variables                                        */
/**********************************************************************/

static nl_catd  catd;    /* Catalog descriptor */

/*
 * Stuff for the kernel ...
 */
static int   kmem;       /* File descriptor for /dev/kmem */
static long  nproc;      /* Number of entries in proc table */

static struct procsinfo  *process;

/*
 * In order to determine how many people are logged on and who has
 * logged in or out, we read in the /etc/utmp file. We also keep track
 * of the previous utmp file.
 */
static int   ut = -1;		/* The file descriptor */
static char  *status;		/* Per tty status bits, see below */
static int   nentries;		/* Number of utmp entries */

static struct utmp  *new;		/* New utmp structures */
static struct utmp  *old;		/* Old utmp structures */

static char  whofilename[100];    /* Information to print on bottom line */
static char  whofilename2[100];   /* Information to print on bottom line */
static char  lockfilename[100];   /* If exists, will prevent us from running */

#ifdef HOSTNAME
/* One more for null termination. One for trailing space */
static char  hostname[MAXHOSTNAMELEN+2];
#endif

	/* Flags which determine which info is printed */
static int  mailcheck = TRUE;      /* m - do biff like checking of mail */
static int  proccheck = TRUE;      /* p - give information on processes */
static int  logcheck  = TRUE;      /* l - tell who logs in and out */
static int  hostprint = FALSE;     /* h - print out hostname */
static int  dateprint = FALSE;     /* h - print out day/date */
static int  quiet     = FALSE;     /* q - hush diagnostic messages */

	/* Flags which determine how things are printed */
static int  clr_bet_ref = FALSE;   /* c - clear line between refeshes */
static int  reverse     = TRUE;    /* r - use reverse video */
static int  shortline   = FALSE;   /* s - short (left-justified) if escapes not allowed */
static int  leftline    = FALSE;   /* j - left-justified even if escapes allowed */

	/* Flags which have terminal do random things	*/
static int  bpflag    = FALSE;       /* b - beep every half hour and twice every hour */
static int  printid = FALSE;       /* i - print pid of this process at startup */
static int  synch   = TRUE;        /* synchronize with clock */

	/* Select output device (status display or straight output) */
static int  emacs  = FALSE;        /* e - assume status display */
static int  window = FALSE;        /* w - window mode */
static int  debug  = FALSE;        /* d - debug */

/*
 * Used to turn off reverse video every REVOFF times in an attempt
 * to not wear out the phospher.
 */
#define  REVOFF  5
static int  revtime = 1;

	/* Used by mail checker */
static off_t  mailsize = 0L;		/* The current size of the mailfile */
static off_t  linebeg = 0L;		/* Place where we last left off reading */

	/* Things used by the string routines */
static int   chars;			/* The number of printable characters */
static char  *sp;			/* Pointer to current posiotion in strarr */
static char  strarr[512];		/* Where data to print is stored */

	/* Flags to stringdump() */
static char  sawmail;		/* Remember mail was seen to print bells */
static char  mustclear;		/* Status line messed up */

	/* Strings which control status line display */
static char  *rev_out, *rev_end;
char  *tparm();

	/* Random globals */
static char   userhome[100];   /* The user's home directory */
static char   *ourtty;         /* Keep track of what tty we're on */
static uid_t  uid;             /* Uid of current user */
static int    users = 0;       /* Number of users logged on to current host */
static int    runable = 0;     /* number of procrun processes */

static struct stat  mstbuf;		/* Used to stat mail file */

static unsigned int  delay = DEFDELAY;	/* Current delay between writes */

static char  mailfile[25];
static struct passwd  *pw;

/**********************************************************************/

#ifdef  RWHO
#include  <protocols/rwhod.h>

#define  DOWN_THRESHOLD  (11 * 60)
#define  RWHOLEADER  "/var/spool/rwho/whod."

static struct remotehost {
	char  *rh_host;
	int   rh_file;
}  remotehost[10];

static int  nremotes = 0;
#endif  RWHO

/**********************************************************************/

	/* To deal with window size changes */

/**********************************************************************/
/* NAME:  do_nothing                                                  */
/* FUNCTION:  Just do nothing!                                        */
/* RETURN VALUE:  none                                                */
/**********************************************************************/

static do_nothing(){}

/**********************************************************************/
/* NAME:  main                                                        */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/

main( 
int  argc,
char *argv[]
)
{
	extern int optind;
	extern char * optarg;
	char *cp;
	int  optlet, x;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_SYSLINE,NL_CAT_LOCALE);

#ifdef  HOSTNAME
	gethostname(hostname, sizeof(hostname) - 1);
	if ( (cp = strchr(hostname, '.')) != NULL ) {
		*cp = '\0';
	}
#endif

	pw = getpwuid(uid = getuid());
	sprintf(mailfile,"/var/spool/mail/%s",pw->pw_name);

	optlet = '\0';
	while ((optlet != EOF) && (optind < argc)) {
		optlet = argv[optind][0];
		if ((optlet == '+') && isdigit(argv[optind][1])) {
			delay = atoi(argv[optind] + 1);
			if (delay < 0) delay = 60;
			synch = FALSE;	/* No more sync */
			optind++;
		} else if ((optlet = getopt(argc,argv,"bcdehijlmpqrswDH:")) != EOF)  {
			switch (optlet) {
				case 'r':  /* Turn off reverse video */
					reverse = FALSE;
					break;
				case 'c':
					clr_bet_ref = TRUE;
					break;
				case 'h':
					hostprint = TRUE;
					break;
				case 'D':
					dateprint = TRUE;
					break;
#ifdef RWHO
				case 'H':
					if (strcmp(hostname, optarg) &&
						strcmp(&hostname[sizeof(NETPREFIX) - 1], optarg))
						remotehost[nremotes++].rh_host = optarg;
						break;
#endif RWHO
				case 'm':
					mailcheck = FALSE;
					break;
				case 'p':
					proccheck = FALSE;
					break;
				case 'l':
					logcheck = FALSE;
					break;
				case 'b':
					bpflag = TRUE;
					break;
				case 'i':
					printid = TRUE;
					break;
				case 'w':
					window = TRUE;
					break;
				case 'e':
					emacs = TRUE;
					break;
				case 'd':
					debug = TRUE;
					break;
				case 'q':
					quiet = TRUE;
					break;
				case 's':
					shortline = TRUE;
					break;
				case 'j':
					leftline = TRUE;
					break;
				default:
					usage();
					break;
			}
		}
	}

	/*
	 * If not emacs or window, initialize terminal dependent info.
	 */
	initterm();
	if (emacs || window) {
		if (emacs) {
			reverse = FALSE;
		}
		getwinsize();
		columns--;
	}


	/*
	 * Immediately fork and let the parent die
	 * if not emacs or window mode.
	 */
	if (!(emacs || window || debug)) {
		if (fork()) {
			exit(0);
		}
		/*
		 * Pgrp should take care of things, but ignore them anyway.
		 */
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		signal(SIGTTOU, SIG_IGN);
	}

	/*
	 * When we logoff, init will do a "vhangup()" on this
	 * tty which turns off I/O access and sends a SIGHUP
	 * signal. We catch this and thereby clear the status
	 * display. Note that a bug in 4.1bsd caused the SIGHUP
	 * signal to be sent to the wrong process, so you had to
	 * `kill -HUP' yourself in your .logout file.
	 * Do the same thing for SIGTERM, which is the default kill
	 * signal.
	 */
	signal(SIGHUP, (void (*)(int))clearbotline);
	signal(SIGTERM, (void (*)(int))clearbotline);
	signal(SIGUSR1, (void (*)(int))printinfo);
	signal(SIGALRM,(void (*)(int))do_nothing);

	ourtty = ttyname(2);	/* Remember what tty we are on */
	if (printid) {
		printf("%d\n", getpid());
		fflush(stdout);
	}
	dup2(2, 1);

	strcpy(userhome, getenv("HOME"));
	if (userhome == NULL) {
		mailcheck = FALSE;
	}
	else {
		strcpy(strcpy1(whofilename, userhome), "/.who");
		strcpy(strcpy1(whofilename2, userhome), "/.sysline");
		strcpy(strcpy1(lockfilename, userhome), "/.syslinelock");
	}

	initprocread();

	if (mailcheck) {
		chdir(userhome);
		if (stat(mailfile, &mstbuf) >= 0) {
			mailsize = mstbuf.st_size;
		}
		else {
			mailsize = 0;
		}
	}

	while (emacs || window || isloggedin()) {
		if (access(lockfilename, 0) >= 0) {
			signal(SIGALRM,(void (*)(int))do_nothing);
			alarm(60);
			pause();
		}
		else {
			printinfo();
			signal(SIGALRM,(void (*)(int))do_nothing);
			alarm(delay);
			pause();
			if (clr_bet_ref && !emacs) {
				if (window) {
					for (x=0; x<=columns; x++)
						strarr[x]=' ';
					strarr[x]='\0';
					printf("\r%s\r",strarr);
					fflush(stdout);
				}
				else {
					tputs(dis_status_line, 1, outc);
					fflush(stdout);
				}
				signal(SIGALRM,(void (*)(int))do_nothing);
				alarm(5);
				pause();
			}
			revtime = (1 + revtime) % REVOFF;
		}
	}

	if (!(window || emacs)) {
		clearbotline();
	}
}

/**********************************************************************/
/* NAME:  isloggedin                                                  */
/* FUNCTION:  Check if the user has logged out.                       */
/* RETURN VALUE:  0 - person is still logged in                       */
/*                1 - person has logged out                           */
/**********************************************************************/

static isloggedin()
{
	struct stat  statbuf;

	/*
	 * You can tell if a person has logged out if the owner of
	 * the tty has changed
	 */
	return(fstat(2, &statbuf) == 0 && statbuf.st_uid == uid);
}

/**********************************************************************/
/* NAME:  readutmp                                                    */
/* FUNCTION:  If nflag is 1 read the utmp into "new",                 */
/*            if it is zero, read it into "old".                      */
/* RETURN VALUE:  0 - read failed                                     */
/*                1 - read successful                                 */
/**********************************************************************/

static readutmp(nflag)
int  nflag;
{
	static time_t  lastmod;     /* Time of last modification */
	static off_t   utmpsize;    /* Size of umtp file */
	struct stat    st;

	if (ut < 0 && (ut = open("/etc/utmp", 0)) < 0) {
		fprintf(stderr,
		MSGSTR(BADUTMP,"sysline: can't open utmp file\n"));
		exit(1);
	}

	/* Utmp hasn't been modified since last check */
	if (fstat(ut, &st) < 0 || st.st_mtime == lastmod) {
		return(0);
	}

	lastmod = st.st_mtime;
	if (utmpsize != st.st_size) {
		utmpsize = st.st_size;
		nentries = utmpsize / sizeof(struct utmp);
		if (old == 0) {
			old = (struct utmp *)calloc((size_t)utmpsize,(size_t)1);
			new = (struct utmp *)calloc((size_t)utmpsize,(size_t)1);
		}
		else {
			old = (struct utmp *) realloc((void *) old, (size_t)utmpsize);
			new = (struct utmp *) realloc((void *) new, (size_t)utmpsize);
			free((void *)status);
		}
		status = malloc((size_t)(nentries * sizeof(*status)));
		if (old == 0 || new == 0 || status == 0) {
			fprintf(stderr, MSGSTR(NOMEM,"sysline: out of memory\n"));
			exit(1);
		}
	}

	lseek(ut, 0L, 0);
	(void) read(ut, (char *) (nflag ? new : old), (unsigned)utmpsize);
	return(1);
}

/**********************************************************************/
/* NAME:  initprocread                                                */
/* FUNCTION:  Read in the process table locations and sizes, and      */
/*            allocate space for storing the process table.           */
/*            This is done only once.                                 */
/* RETURN VALUE:  0 - succeeded in conversion                         */
/*                1 - conversion failed                               */
/**********************************************************************/

static initprocread()
{
	process = getprocdata (process, &nproc);
	if (process == NULL)  return (1);
	else                  return (0);
}

/**********************************************************************/
/* NAME:  getprocdata                                                 */
/* FUNCTION:  Read in the proc structure of interesting processes.    */
/*            Set parameter nproc to the total number of process.     */
/*            This routine taken and modified from ps.c               */
/* RETURN VALUE:  Pointer to the beginning of                         */
/*                the copied proc structure.                          */
/**********************************************************************/

static struct procsinfo  *getprocdata(proc_mem, nproc)
long  *nproc;
struct procsinfo  *proc_mem;
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
	if (proc_mem == NULL) {
		Proc = (struct procsinfo *)malloc((size_t)sizeof(unsigned long));
	}
	else {
		Proc = proc_mem;
	}
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

/**********************************************************************/
/* NAME:  readproctab                                                 */
/* FUNCTION: Read in the process table. This assumes that             */
/*           initprocread() has already been called to set up storage.*/
/* RETURN VALUE:                                                      */
/**********************************************************************/

static readproctab()
{
	process = getprocdata(process, &nproc);
	if (process == NULL)  return (0);
	else                  return (1);
}

/*********************************************************************/
/* NAME: getthreaddata						     */
/*								     */
/* FUNCTION: fetch information found in the structure thrdsinfo for  */
/*  the threads of a given process				     */
/*								     */
/* NOTES: N/A							     */
/*								     */
/* RETURNS: struct thrdsinfo					     */
/*********************************************************************/
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

/**********************************************************************/
/* NAME:  printinfo                                                   */
/* FUNCTION:  Print out specified information.                        */
/* RETURN VALUE:  none                                                */
/**********************************************************************/

static void  printinfo()
{
	register int  i;
	int   on, off;
	int   diff;
	char  tempbuf[150];
	long  loadavg = 0.0;
	int done;
	int process_pri;
	int nthread;
	int procrun;
	int procstop;
	struct thrdsinfo *thrd;

	stringinit();

/*
 * 	always check the window for resizes.
 */
	getwinsize();
	/* Check for file named .who in the home directory */
	whocheck();

	/* Print out time information */
	timeprint();
	stringspace();

	/*
	 * If mail is seen, don't print rest of info, just the mail.
	 */
	if (mailcheck && (sawmail = mailseen())) {
		goto bottom;
	}

#ifdef  HOSTNAME
#ifdef  RWHO
	for (i = 0; i < nremotes; i++) {
		char  *tmp;

		tmp = sysrup(remotehost + i);
		stringcat(tmp, strlen(tmp));
		stringspace();
	}
#endif

	/*
	 * Print hostname info if requested
	 */
	if (hostprint) {
		stringcat(hostname, -1);
		stringspace();
	}
#endif
	if (readproctab()) {
		register struct procsinfo *p;

		/*
		 * We are only interested in processes which have the same
		 * uid as us, and whose parent process id is not 1.
		 */
		procrun = procstop = runable = 0;
		for (p = process; p < &process[nproc]; p++) {
			if (p->pi_state == SACTIVE) {
				thrd = getthreaddata(p->pi_pid, &nthread);
				i=0;
				done = FALSE;
				do {
					if (thrd[i].ti_state == TSRUN) {
						runable ++;
						done = TRUE;
					}
					i++;
				} while ((done != TRUE) && (i != nthread));
			}
			if (p->pi_state == 0 || p->pi_uid != uid ||
			    p->pi_pgrp == 0 || p->pi_ppid == 1L ||
			    p->pi_ppid == -1) {
				continue;
			}

			switch (p->pi_state) {
			  case SSTOP:
				procstop++;
				break;
			  case SACTIVE:
				/*
				 * Sleep can mean waiting for a signal or just
				 * in a disk or page wait queue ready to run.
				 * We can tell if it is the latter if the 
				 * priority is negative.
				 */
				if (done == FALSE) {
					thrd = getthreaddata(p->pi_pid, &nthread);
					process_pri = PIDLE;
					for (i=0; i< nthread; i++) {
						if (thrd[i].ti_pri < process_pri)
							process_pri = thrd[i].ti_pri;
					}
					if (process_pri < PZERO) 
						procrun++;
				}
				else
					procrun++;
				break;
			  case SIDL:
				procrun++;
			}
		}
	}

	sprintf(tempbuf, MSGSTR(RUNABLE,"run:%d"), runable);
	stringcat(tempbuf, -1);
	loadavg = runable;

	stringspace();

	/*
	 * Read utmp file (logged in data) only if we are doing a full
	 * process, or if this is the first time and we are calculating
	 * the number of users.
	 */
	on = off = 0;
	if (users == 0) {    /* First time */
		if (readutmp(0)) {
			for (i = 0; i < nentries; i++) {
				if (old[i].ut_name[0] != '\0' &&
					old[i].ut_type == USER_PROCESS) {
					users++;
				}
			}
		}
	}
	else if (readutmp(1)) {
		struct utmp  *tmp;

		users = 0;
		for (i = 0; i < nentries; i++) {
			if (new[i].ut_type != USER_PROCESS && 
				old[i].ut_type != USER_PROCESS) {
				status[i] = NOCH;
			}
			else if (strncmp(old[i].ut_name, 
				new[i].ut_name, (size_t)NAMESIZE) == 0) {
				status[i] = NOCH;
			}
			else if (strncmp(new[i].ut_name, "LOGIN", (size_t)5) == 0 ||
				strncmp(new[i].ut_name, "getty", (size_t)5) == 0 ||
				new[i].ut_name[0] == '\0') {
				status[i] = OFF;
				off++;
			}
			else if (strncmp(old[i].ut_name,"LOGIN",(size_t)5) == 0 ||
				strncmp(old[i].ut_name, "getty", (size_t)5) == 0 ||
				old[i].ut_name[0] == '\0') {
				status[i] = ON;
				on++;
			}
			else {
				status[i] = ON | OFF;
				on++;
				off++;
			}

			if (new[i].ut_name[0] != '\0' &&
				new[i].ut_type == USER_PROCESS) {
				users++;
			}
		}

		/*
		 * Reverse new and old so that next time we run, we 
		 * won't lose log in and out information
		 */
		tmp = new;
		new = old;
		old = tmp;
	}

	/*
	 * Print:
	 *  1. Number of users
	 *  2. A * for unread mail
	 *  3. A - if load is too high
	 *  4. Number of processes running and stopped
	 */
	stringprint("%du", users);  /* Number of users */
	if (mailsize > 0L && mstbuf.st_mtime >= mstbuf.st_atime) {
		stringcat("*", -1);
	}

	if (proccheck && (procrun > 0 || procstop > 0)) {
		stringspace();

		if (procrun > 0 && procstop > 0) {
			/* Number of executable and suspended process */
			stringprint("%dr %ds", procrun, procstop);
		}
		else if (procrun > 0) {
			/* Number of executable process */
			stringprint("%dr", procrun);
		}
		else {
			/* Number of suspended process */
			stringprint("%ds", procstop);
		}
	}

	/*
	 * If anyone has logged on or off, and we are interested in it,
	 * print it out.
	 */
	if (logcheck) {
		/* Note that "old" and "new" have already been swapped */
		if (on) {
			stringspace();
			stringcat(MSGSTR(M_ON,"on:"), -1);
			for (i = 0; i < nentries; i++) {
				if (status[i] & ON) {
					stringprint(" %.8s", old[i].ut_name);
					ttyprint(old[i].ut_line);
				}
			}
		}
		if (off) {
			stringspace();
			stringcat(MSGSTR(M_OFF,"off:"), -1);
			for (i = 0; i < nentries; i++) {
				if (status[i] & OFF) {
					stringprint(" %.8s", new[i].ut_name);
					ttyprint(new[i].ut_line);
				}
			}
		}
	}
bottom:

	/* Dump out what we know */
	stringdump();
}

/**********************************************************************/
/* NAME:  timeprint                                                   */
/* FUNCTION:  Print out the current time and date (if specified).     */
/*            Set up beeps if appropriate.                            */
/* RETURN VALUE:  none                                                */
/**********************************************************************/

static void  timeprint()
{
	long        curtime;
	struct tm   *tp;
	static int  lasthbeep = -1; /* last beep on the half hour */
	static int  lastfbeep = -1; /* last beep on the full hour */

	/* Always print time */
	time(&curtime);
	tp = localtime(&curtime);
	if (lasthbeep == -1) {
		lasthbeep = lastfbeep = tp->tm_hour;
	}
	if (dateprint) {
		/* Current date */
		stringprint("%.11s", ctime(&curtime));
	}
	/* Current time */
	stringprint("%d:%02d", tp->tm_hour > 12 ? tp->tm_hour - 12 :
		(tp->tm_hour == 0 ? 12 : tp->tm_hour), tp->tm_min);
	if (synch) {   /* sync with clock */
		delay = 60 - tp->tm_sec;
	}

	if (bpflag) {
		if ((tp->tm_min >= 30) && (lasthbeep != tp->tm_hour)) {
			lasthbeep = tp->tm_hour;
			tputs(bell, 1, outc);
			fflush(stdout);
		}
		else {
			if ((tp->tm_min >= 0) && (lastfbeep != tp->tm_hour)) {
				lastfbeep = tp->tm_hour;
				tputs(bell, 1, outc);
				fflush(stdout);
				tputs(bell, 1, outc);
				fflush(stdout);
			}
		}
	}
}

/**********************************************************************/
/* NAME:  whocheck                                                    */
/* FUNCTION:  Check for file named .who and print it out first.       */
/* RETURN VALUE:  none                                                */
/**********************************************************************/

static void  whocheck()
{
	int   chss;
	register char  *p;
	char  buff[81];
	int   whofile;

	if ((whofile = open(whofilename, 0)) < 0 &&
	    (whofile = open(whofilename2, 0)) < 0) {
		return;
	}
	chss = read(whofile, buff, sizeof(buff) - 1);
	close(whofile);
	if (chss <= 0) {
		return;
	}
	buff[chss] = '\0';

	/*
	 * Remove all line feeds, and replace by spaces if they are within
	 * the message, else replace them by nulls.
	 */
	for (p = buff; *p;) {
		if (*p == '\n') {
			if (p[1])  *p++ = ' ';
			else       *p = '\0';
		}
		else {
			p++;
		}
	}
	stringcat(buff, p - buff);
	stringspace();
}

/**********************************************************************/
/* NAME:  ttyprint                                                    */
/* FUNCTION:  Given the name of a tty, print in the string buffer     */
/*            its short name surrounded by parenthesis.               */
/*            ttyxx is printed as (xx) and console is printed as (cty)*/
/* ARGUMENTS:  name - Name of tty.                                    */
/* RETURN VALUE:  none                                                */
/**********************************************************************/

static void  ttyprint(name)
char  *name;
{
	char  buff[11];

	if (strncmp(name, "tty", (size_t)3) == 0) {
		stringprint("(%.*s)", LINESIZE - 3, name + 3);
	}
	else if (strcmp(name, "console") == 0) {
		stringcat("(cty)", -1);
	}
	else {
		stringprint("(%.*s)", LINESIZE, name);
	}
}

/**********************************************************************/
/* NAME:  mailseen                                                    */
/* FUNCTION:  Check if mail is present.                               */
/* RETURN VALUE:  Returns 0 if no mail seen.                          */
/**********************************************************************/

static mailseen()
{
	FILE  *mfd;
	register int   n;
	register char  *cp;
	char  lbuf[100];
	char  sendbuf[100];
	char  *bufend;
	char  seenspace;
	int   retval = 0;
	struct stat  sbuf;

	if (stat(mailfile, &mstbuf) < 0) {
		mailsize = 0;
		return(0);
	}

	if ((mstbuf.st_size <= mailsize) || 
	    (mfd = fopen(mailfile, "r")) == NULL) {
		mailsize = mstbuf.st_size;
		return(0);
	}

	fseek(mfd, mailsize, 0);
	while ((n = readline(mfd, lbuf, sizeof(lbuf))) >= 0) {
		if (strncmp(lbuf, "From ", (size_t)5) == 0) {
			break;
		}
	}

	if (n < 0) {
		stringcat(MSGSTR(M_ARRIVAL,"Mail has just arrived"), -1);
		goto out;
	}
	retval = 1;

	/*
	 * Found a From line, get second word, which is the sender,
	 * and print it.
	 */
	for (cp = lbuf + 5; *cp && *cp != ' '; cp++)	/* Skip to blank */
		;
	*cp = '\0';    /* Terminate name */
	stringspace();
	stringprint(MSGSTR(M_MAIL,"Mail from %s "), lbuf + 5);

	/*
	 * Print subject, and skip over header.
	 */
	while ((n = readline(mfd, lbuf, sizeof(lbuf))) > 0) {
		if (strncmp(lbuf, "Subject:", (size_t)8) == 0) {
			stringprint(MSGSTR(MON,"on %s "), lbuf + 9);
		}
	}

	if (!emacs)  stringcat("->", 2);
	else         stringcat(": ", 2);

	if (n < 0) {    /* Already at eof */
		goto out;
	}

	/*
	 * Print as much of the letter as we can.
	 */
	cp = sendbuf;
	if ((n = columns - chars) > sizeof(sendbuf) - 1) {
		n = sizeof(sendbuf) - 1;
	}
	bufend = cp + n;
	seenspace = 0;
	while ((n = readline(mfd, lbuf, sizeof(lbuf))) >= 0) {
		register char *rp;

		if (strncmp(lbuf, "From ", (size_t)5) == 0) {
			break;
		}

		if (cp >= bufend) {
			continue;
		}

		if (!seenspace) {
			*cp++ = ' ';		/* space before lines */
			seenspace = 1;
		}

		rp = lbuf;
		while (*rp && cp < bufend) {
			if (isspace((int)*rp)) {
				if (!seenspace) {
					*cp++ = ' ';
					seenspace = 1;
				}
				rp++;
			}
			else {
				*cp++ = *rp++;
				seenspace = 0;
			}
		}
	}
	*cp = 0;
	stringcat(sendbuf, -1);

	/*
	 * Want to update write time so a star will
	 * appear after the number of users until the
	 * user reads his mail.
	 */
out:
	mailsize = linebeg;
	fclose(mfd);
	touch(mailfile); 
	return(retval);
}

/**********************************************************************/
/* NAME:  readline                                                    */
/* FUNCTION:  Read a line from fp and store it in buf.                */
/* ARGUMENTS:  fp  - File pointer.                                    */
/*             buf - Buffer in which to read into.                    */
/*             n   - Number of bytes to read.                         */
/* RETURN VALUE:  Return the number of character read.                */
/**********************************************************************/

static readline(fp, buf, n)
register FILE  *fp;
char  *buf;
register  n;
{
	register  c;
	register char  *cp = buf;

	/* Remember location where line begins */
	linebeg = ftell(fp);
	cp = buf;
	while (--n > 0 && (c = getc(fp)) != EOF && c != '\n') {
		*cp++ = c;
	}
	*cp = 0;
	if (c == EOF && cp - buf == 0) {
		return(-1);
	}
	return(cp - buf);
}

/**********************************************************************/
/* NAME:  stringinit                                                  */
/* FUNCTION:  Perform string related initializations.                 */
/* RETURN VALUE:                                                      */
/**********************************************************************/

static stringinit()
{
	sp = strarr;
	chars = 0;
}

/**********************************************************************/
/* NAME:  stringprint                                                 */
/* FUNCTION:  printf() style stringcat routine.                       */
/* RETURN VALUE:                                                      */
/**********************************************************************/

static stringprint(format, a, b, c)
char  *format;
{
	char  tempbuf[150];

	sprintf(tempbuf, format, a, b, c);
	stringcat(tempbuf, -1);
}

/**********************************************************************/
/* NAME:  stringdump                                                  */
/* FUNCTION:  Dump the string we have been building.                  */
/* RETURN VALUE:                                                      */
/**********************************************************************/

static stringdump()
{
    char            bigbuf[sizeof(strarr) + 200];
    register char  *bp = bigbuf;
    register int    i;
    char           *p;


    bp = strcpy1(bp, dis_status_line);
    if (sawmail)
	bp = strcpy1(bp, bell);

    /*
     * determine if to_status_line will take a parameter, if it will the do
     * the tparm stuff, else we need to left justify it ouselves. 
     */
    p = to_status_line;
    while (*p) {
	if (*p == '%') {
	    if (*++p == '%') {
		p++;
	    }
	    else
		break;
	}
	p++;
    }

    if (*p && !(window || emacs))
	bp = strcpy1(bp, tparm(to_status_line,
			       leftline ? 0 : columns - chars));
    else {
	if (!(window || emacs))
	    bp = strcpy1(bp, to_status_line);
	if (!(shortline || leftline || emacs))
	    for (i = columns - chars; --i >= 0;)
		*bp++ = ' ';
    }
    if (reverse && revtime != 0)
	bp = strcpy1(bp, rev_out);

    *sp = 0;
    bp = strcpy1(bp, strarr);
    if (!emacs) {
	if (reverse)
	    bp = strcpy1(bp, rev_end);
	bp = strcpy1(bp, from_status_line);
	if (sawmail)
	    bp = strcpy1(strcpy1(bp, bell), bell);
	*bp = 0;
	tputs(bigbuf, 1, outc);
	if (mustclear) {
	    mustclear = 0;
	    tputs(clr_eol, 1, outc);
	}
	if (debug)
	    putchar('\n');
	fflush(stdout);
    }
    else
	write(2, bigbuf, (unsigned) (bp - bigbuf));
    if (window)
	putchar('\r');
}
/**********************************************************************/
/* NAME:  stringspace                                                 */
/* FUNCTION:  Append a space to the list we are building to send out. */
/* RETURN VALUE:                                                      */
/**********************************************************************/

static stringspace()
{
	if (reverse && revtime != 0) {
		stringcat(rev_end,
			magic_cookie_glitch <= 0 ? 0 : magic_cookie_glitch);
		stringcat(" ", 1);
		stringcat(rev_out,
			magic_cookie_glitch <= 0 ? 0 : magic_cookie_glitch);
	}
	else {
		stringcat(" ", 1);
	}
}

/**********************************************************************/
/* NAME:  stringcat                                                   */
/* FUNCTION:  Concatenate characters in string "str"                  */
/*            to list we are building to send out.                    */
/* ARGUMENTS:  str - The string to print.                             */
/*                   May contain funny (terminal control) chars.      */
/*             n   -   -1 : str is all printable - truncate if needed.*/
/*                otherwise: Number of printable characters in string.*/
/*                           - don't truncate.                        */
/* RETURN VALUE:                                                      */
/**********************************************************************/

static stringcat(str, n)
register char  *str;
register int   n;
{
	register char  *p = sp;

	if (n < 0) {    /* Truncate */
		n = columns - chars;
		while ((*p++ = *str++) && --n >= 0)
			;
		p--;
		chars += p - sp;
		sp = p;
	}
	else {
		if (chars + n <= columns) {    /* Don't truncate */
			while (*p++ = *str++)
				;
			chars += n;
			sp = p - 1;
		}
	}
}

/**********************************************************************/
/* NAME:  touch                                                       */
/* FUNCTION:  Update the modify time of the specified file.           */
/* ARGUMENTS:  name - Name of file.                                   */
/* RETURN VALUE:                                                      */
/**********************************************************************/

static touch(name)
char  *name;
{
	time_t       timep[2];
	struct stat  sbuf;

	if (stat(name, &sbuf) >= 0) {
		timep[0] = sbuf.st_atime;
		timep[1] = time((time_t *) 0);
		utime(name, timep);
	}
}

/**********************************************************************/
/* NAME:  clearbotline                                                */
/* FUNCTION:  Clear bottom line of the terminal.                      */
/*            Called when process quits or is killed.                 */
/* RETURN VALUE:                                                      */
/**********************************************************************/

static clearbotline(void)
{
	int  fd;

	signal(SIGALRM, (void (*)(int))exit);
	alarm((unsigned)30);	/* If can't open in 30 secs, just die */
	if ((fd = open(ourtty, 1)) >= 0) {
		write(fd, dis_status_line, (unsigned)strlen(dis_status_line));
		close(fd);
	}
	exit(0);
}

/**********************************************************************/
/* NAME:  initterm                                                    */
/* FUNCTION:  Initialize terminfo-related info.                       */
/* RETURN VALUE:                                                      */
/**********************************************************************/

static initterm()
{
	static char  standbuf[40];

	if (!emacs) {
		setupterm(0, 1, 0);
		if (!has_status_line && !window) {
			/* Not an appropriate terminal */
			if (!quiet) {
				fprintf(stderr, MSGSTR(NO_STATUS,
				"sysline: no status capability for %s\n"),
				getenv("TERM"));
			}
			exit(1);
		}
		columns = width_status_line;
	}

	if (status_line_esc_ok || window) {
		if (enter_standout_mode && exit_standout_mode) {
			rev_out = enter_standout_mode;
			rev_end = exit_standout_mode;
		}
		else {
			rev_out = rev_end = "";
		}
	}
	else {
		rev_out = rev_end = "";
	}
}

/**********************************************************************/
/* NAME:  sysrup                                                      */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/

#ifdef RWHO
static char  *sysrup(hp)
register struct remotehost  *hp;
{
	char  filename[100];
	struct whod  wd;
	static char  buffer[50];
	time_t  now;

#define  WHOD_HDR_SIZE  (sizeof(wd) - sizeof(wd.wd_we))

	/*
	 * rh_file is initially 0.
	 * This is ok since standard input is assumed to exist.
	 */
	if (hp->rh_file == 0) {
		/*
		 * Try rwho hostname file,
		 * and if that fails try ucbhostname.
		 */
		(void)strcpy1(strcpy1(filename,RWHOLEADER),hp->rh_host);
		if ((hp->rh_file = open(filename, 0)) < 0) {
			(void) strcpy1(strcpy1(strcpy1(filename,
				RWHOLEADER), NETPREFIX), hp->rh_host);
			hp->rh_file = open(filename, 0);
		}
	}
	if (hp->rh_file < 0) {
		sprintf(buffer, "%s?", hp->rh_host);
		return(buffer);
	}
	(void) lseek(hp->rh_file, (off_t) 0, 0);
	if (read(hp->rh_file, (char *)&wd, WHOD_HDR_SIZE) != WHOD_HDR_SIZE) {
		sprintf(buffer, "%s ?", hp->rh_host);
		return(buffer);
	}
	(void) time(&now);
	if (now - wd.wd_recvtime > DOWN_THRESHOLD) {
		long interval;
		long days, hours, minutes;

		interval = now - wd.wd_recvtime;
		minutes = (interval + 59) / 60;	/* Round to minutes */
		hours = minutes / 60;		/* Extract hours from minutes */
		minutes %= 60;			/* Remove hours from minutes */
		days = hours / 24;		/* Extract days from hours */
		hours %= 24;			/* Remove days from hours */

		if (days > 7 || days < 0) {
			sprintf(buffer, MSGSTR(M_DOWN,"%s down"), hp->rh_host);
		}
		else if (days > 0) {
			sprintf(buffer, "%s %d+%d:%02d",
			hp->rh_host, days, hours, minutes);
		}
		else {
			sprintf(buffer, "%s %d:%02d",
			hp->rh_host, hours, minutes);
		}
	}
	else {
		sprintf(buffer, "%s %.1f",
		hp->rh_host, wd.wd_loadav[0]/100.0);
	}
	return(buffer);
}
#endif RWHO

/**********************************************************************/
/* NAME:  getwinsize                                                  */
/* FUNCTION:  Get the current window size using the TIOCGWINSZ ioctl. */
/* RETURN VALUE:                                                      */
/**********************************************************************/

static getwinsize()
{
    struct winsize  winsize;
    static struct winsize owinsize;

    /* The "-1" below is to avoid cursor wraparound problems */
    if (ioctl(2, TIOCGWINSZ, (char *) &winsize) >= 0 && winsize.ws_col != 0)
	columns = winsize.ws_col - 1;
    if (owinsize.ws_col) {
	if (owinsize.ws_col != winsize.ws_col)
	    mustclear = 1;
    }
    owinsize = winsize;
}

/**********************************************************************/
/* NAME:  strcpy1                                                     */
/* FUNCTION:  Copy string "q" to "p"                                  */
/* ARGUMENTS:  p - Destination string                                 */
/*             q - Source string.                                     */
/* RETURN VALUE:  Return pointer to end of string.                    */
/**********************************************************************/

static char  *strcpy1(p, q)
register char *p, *q;
{
	while (*p++ = *q++)
		;
	return(p - 1);
}

/**********************************************************************/
/* NAME:  outc                                                        */
/* FUNCTION:                                                          */
/* RETURN VALUE:  none                                                */
/**********************************************************************/

static outc(c)
char  c;
{
	if (debug)  printf("%s", unctrl(c));
	else        putchar(c);
}

/**********************************************************************/
/* NAME:  usage                                                       */
/* FUNCTION:  Displays usage of function.                             */
/* RETURN VALUE:  none                                                */
/**********************************************************************/

static usage()
{
	fprintf( stderr, MSGSTR(USAGE,
	"usage: sysline [-b][-c][-d][-e][-h][-i][-j][-l][-m][-p][-q][-r][-s][-w] \n") );
	fprintf( stderr, MSGSTR(USAGE2,
	"               [-D] [-H remote] [+N] \n") );
	exit(1);
}

