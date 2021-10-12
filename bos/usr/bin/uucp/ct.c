static char sccsid[] = "@(#)86  1.16  src/bos/usr/bin/uucp/ct.c, cmduucp, bos411, 9428A410j 11/11/93 15:07:23";
/* 
 * COMPONENT_NAME: CMDUUCP ct.c
 * 
 * FUNCTIONS: Mct, assert, cleanup, disconnect, exists, gdev, logent, 
 *            startat, stopat, uugetty, zero 
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

/* static char sccsid[] = "ct.c	5.2 -  - "; */
/*	ct.c	1.13
 *
 *	ct [-h] [-v] [-w max_wait] [-x debuglevel] [-s speed] phone_number
 *
 *	dials the given telephone number, waits for the
 *	modem to answer, and initiates a login process.
 *
 *	ct uses several routines from uucp:
 *	- getto(flds) takes a vector of fields needed to make
 *	  a connection and returns a file descriptor or -1
 *	- rddev( ... ) takes several arguments and returns lines
 *	  from the /etc/uucp/Devices that match the type
 *	  (in ct the type will be ACU)
 *	- fdig(string) takes a string that is zero or more
 *	  alphabetic characters follow by a number (baud rate)
 *	  and returns a pointer to the first digit in the string.
 *	- mlock(dev) takes the last part of a device (cul01 for
 *	  device /dev/cul01) and tries to create a lock file
 *	  LCK..cul01 in this example.  It returns FAIL if it fails.
 *	- rmlock(pointer) removes the lock file.  In ct pointer is
 *	  always CNULL (a null pointer) causing rmlock to remove
 *	  all LCK files associated with this execution of ct.
 */

#include "uucp.h"
#include <pwd.h>
#include <utmp.h>
#include <termios.h>

nl_catd catd;
#define ROOT	0
#define SYS	3
#define TTYMOD	0644
#define DEV	"/dev/"
#define TELNOSIZE	32		/* maximum phone # size is 31 */
#define LEGAL	"0123456789-*#="
#define USAGE	"Usage: ct [-h] [-v] [-w max_wait] [-x debuglevel] [-s speed] phone_number\n"
#define LOG	"/usr/adm/ctlog"
#define	GETTY	"/usr/sbin/getty"
#define TRUE	1
#define FALSE	0

static 
int	_Status,		/* exit status of child */
	_Pid = 0;		/* process id of child */

static
char	*_Num,			/* pointer to a phone number */
	_Tty[sizeof DEV+DIRSIZ] = "",  /* /dev/ttyxx for connection device */
	*_Dev[D_MAX + 1],	/* Filled in by rddev and used globally */
	_Devbuf[BUFSIZ],	/* buffer for rddev */
	*_Flds[7];		/* Filled in as if finds() in uucp did it */

static 
time_t	_Log_on,
	_Log_elpsd;

static 
FILE	*_Fdl;

extern int  optind;
extern char *optarg;
extern  void cleanup(int);
extern void devreset();
/* extern struct passwd  *getpwuid (); */
extern struct utmp *getutid (), *getutent(), *pututline();
extern void setutent(), endutent();

static int gdev(), uugetty(), exists();
static void startat(), stopat(), disconnect(int), zero();

/*
 * These two dummy routines are needed because the uucp routines
 * used by ct reference them, but they will never be
 * called when executing from ct
 */
void assert () { }		/* for ASSERT in gnamef.c */
void logent () { }		/* so we can load ulockf() */

char   *Myline = NULL;		/* referenced in conn.c; must be NULL for ct */
jmp_buf Sjbuf;			/* used by uucp routines */

main (argc, argv)
char   *argv[];
{
    register int    c;
    register char  *dp,
                   *aptr;
    int		found = 0,
		errors = 0,
		first = TRUE,
		i;
    char    tbuf[TELNOSIZE];
    int     dl,
            count,
	    uugettyflag,	/* is there a uugetty on the line */
            hangup = 1,		/* hangup by default */
            minutes = 0;	/* number of minutes to wait for dialer */
    int     fdl;
    struct termios   termio;
    typedef int (*save_sig)(int);
    save_sig	save_hup,
		save_quit,
		save_int;
    save_hup = (save_sig) signal (SIGHUP, (void(*)(int))cleanup);
    save_quit = (save_sig) signal (SIGQUIT, (void(*)(int))cleanup);
    save_int = (save_sig) signal (SIGINT, (void(*)(int))cleanup);
    (void) signal (SIGTERM, (void(*)(int))cleanup);
    (void) strcpy (Progname, "ct");

    setlocale(LC_ALL,"");
    catd = catopen(MF_UUCP,NL_CAT_LOCALE);

    setservice("cu");
    if ( sysaccess(EACCESS_DEVICES) != 0 ) {
	(void) fprintf(stderr, MSGSTR(MSG_CT1,"ct: can't open %s\n"),DEVFILE);
        cleanup(101);
    }

    /* Set up the _Flds vector as if finds() [from uucico] built it */
    _Flds[F_NAME] = "dummy";		/* never used */
    _Flds[F_TIME] = "Any";		/* never used */
    _Flds[F_TYPE] = "ACU";
    _Flds[F_CLASS] = "1200";		/* default at 1200 */
    _Flds[F_PHONE] = "";			/* filled in by arguments */
    _Flds[F_LOGIN] = "";			/* never used */
    _Flds[6] = NULL;

    while ((c = getopt (argc, argv, "hvw:s:x:")) != EOF) {
	switch (c) {
	    case 'h': 
		hangup = 0;
		break;

	    case 'v': 
		Verbose = 1;
		break;

	    case 'w': 
		minutes = atoi (optarg);
		if (minutes < 1) {
		    (void) fprintf(stderr,MSGSTR(MSG_CT23, USAGE));
		    (void) fprintf(stderr, 
			MSGSTR(MSG_CT3, "(-w %s) Wait time must be > 0\n"),
			optarg);
		    cleanup(101);
		}
		break;

	    case 's': 
		_Flds[F_CLASS] = optarg;
		break;

	    case 'x':
		Debug = atoi(optarg);
		if (Debug < 0)
		    Debug = 0;
		else if (Debug > 9)
		    Debug = 9;
		break;

	    case '?': 
		(void) fprintf(stderr, MSGSTR(MSG_CT23, USAGE));
		cleanup(101);
		/* NOTREACHED */
	}
    }

    if (optind == argc) {
	(void) fprintf(stderr, MSGSTR(MSG_CT23, USAGE));
	(void) fprintf(stderr, MSGSTR(MSG_CT7,
		"No phone numbers specified!\n"));
	cleanup(101);
    }

    /* check for valid phone number(s) */
    for (count = argc - 1; count >= optind; --count) {
	_Num = argv[count];
	if (strlen (_Num) >= sizeof tbuf - 1) {
	    (void) fprintf(stderr, MSGSTR(MSG_CT20,
		"ct: phone number too long -- %s\n"), _Num);
	    ++errors;
	}
	if (strspn (_Num, LEGAL) < strlen (_Num)) {
	    (void) fprintf(stderr, MSGSTR(MSG_CT8,
		"ct: bad phone number -- %s\n"), _Num);
	    ++errors;
	}
    }
    if (errors)
	cleanup(101);

    /************************************************************/
    /*		Begin Loop:  Find an available Dialer		*/
    /************************************************************/
    for (count = 0;; count++) { /* count will be wait time after first
				 * time through the loop.
				 * break will be used exit loop.
				 */
	if ( (found = gdev (_Flds)) > 0) {  /* found a dialer */
	    (void) fprintf(stdout, MSGSTR(MSG_CT9,
			"Allocated dialer at %s baud\n"),
		_Flds[F_CLASS]);
		devreset();
	    break;
	}
	else if (found == 0) {	/* no dialers of that on system */
	    (void) fprintf(stdout, 
			MSGSTR(MSG_CT10, "No %s dialers on this system\n"),
		fdig(_Flds[F_CLASS]) );
    	    cleanup(101);
	}
	    
	if (!first) { /* not the first time in loop */
	    VERBOSE( (found == -1)  ?
		MSGSTR(MSG_CTV2,"Dialer is busy %d minute(s)\n")  :
		MSGSTR(MSG_CTV3,"Dialers are busy %d minute(s)\n"),  count);            if (count < minutes) {
	        sleep(60);
	        continue;
	    }
	    /* This is the end of the loop - no time left */
	    break;
	}

	/**************************************************************/
	/* First time through loop - get wait minutes if no -w option */
	/**************************************************************/
	first = FALSE;
	if(found == -1)
		(void) printf(MSGSTR(MSG_CT11,"The (%d) %s dialer is busy\n"),
				 -found, _Flds[F_CLASS]);
	else
		(void) printf(MSGSTR(MSG_CT12,
		  "The (%d) %s dialers are busy\n"), -found, _Flds[F_CLASS]);

	if (minutes) {	/* -w already set wait minutes */
	    	if(minutes > 1)
	    		(void) printf( MSGSTR(MSG_CT13,
			 "Waiting for %d minutes\n"), minutes);
		else
	    		(void) printf( MSGSTR(MSG_CT14,
			 "Waiting for %d minute\n"), minutes);
	    sleep(60);
	    continue;
	}

	if (!isatty(fileno(stdin)) )  {  /* not a terminal - get out */
	    cleanup(101);
	}

	/* Ask user if she/he wants to wait */
	(void) printf(MSGSTR(MSG_CT15,
		"Do you want to wait for dialer? (y for yes): "));
	if ((c = getchar ()) == EOF || tolower (c) != 'y')
	    cleanup(101);
	while ( (c = getchar()) != EOF && c != '\n')
	    ;

	(void) printf(MSGSTR(MSG_CT16,"Time, in minutes? "));
	(void) scanf ("%d", &minutes);
	while ( (c = getchar()) != EOF && c != '\n')
	    ;

	if (minutes <= 0)
	    cleanup(101);

	(void) printf(MSGSTR(MSG_CT17,"Waiting for dialer\n"));
	sleep(60);
	continue;

    }
    /************************************************************/
    /*		End Loop:  Find an available Dialer		*/
    /************************************************************/

    /* check why loop terminated */
    if (found < 0) {	/* no dialer found - get out */
        (void) printf(MSGSTR(MSG_CT18,"*** TIMEOUT ***\n"));
        cleanup(101);
    }

    (void) signal(SIGHUP, SIG_IGN);
    /* found a dialer. now try to call */
    if (!isatty (fileno (stdin)))
        hangup = 0;

    if (hangup) {  	/* 
			 * -h option not specified. Callback expected. Hang
			 * up so remote modem will re-answer when called.
			 */
        while(1) {

            (void) fputs (MSGSTR(MSG_CT25,"Confirm: Hang up your tty before dialing? (y/n): "), stdout);
            if ((c = getchar ()) == EOF || tolower (c) == 'n'){
		fputs(MSGSTR(MSG_CT26, 
			"To proceed without closing your terminal, use the -h flag.\n"),
			stdout);
                cleanup(101);
	    }
            else if( tolower(c) == 'y') {
                while ( (c = getchar()) != EOF && c != '\n')
                        ;
                break;
                }
            else
                while( c != EOF && c != '\n')
                      c = getchar();
        }

        /* close 1; if stderr is not redirected, close 2 also */
        (void) close(1);
        if (isatty (fileno (stderr))) {
            Verbose = 0;
            Debug = 0;
            (void) close (2);
        }
	
	/* Hang up the current line */
	(void) tcgetattr (0, &termio);
        termio.c_cflag |= HUPCL;	/* speed to zero for hangup */
        (void) tcsetattr (0, TCSANOW, &termio);  /* hang up terminal */
        (void) sleep (5);
    }

    /* Try each phone number until a connection is made, or none work */
    for (count = optind; count < argc; count++) {
	/* call getto routine to make connection */
	_Flds[F_PHONE] = argv[count];
	clrlock(CNULL);	/* remove temporary lock set by gdev */
	fdl = getto(_Flds);
	if (fdl > 0) {
	    /*
	     * If there is a uugetty on the line, get rid
	     * of the lock file quickly so that when the uugetty
	     * reads the first character, the lock file will be gone
	     * indicating that the uugetty should act as a getty
	     * rather than going into the sleep-check_lock loop
	     */
	    if ( (uugettyflag = uugetty(Dc)) ) /* really an assignment! */
		clrlock(CNULL);

		(void) sprintf(_Tty, "%s%s", DEV, Dc);
	    /* NOTE:  Dc is set in the caller routines */
	    break;
	}
    }

    /* check why the loop ended (connected or no more numbers to try) */
    if (count == argc)
	cleanup(101);

    /****** Successfully made connection ******/
    VERBOSE(MSGSTR(MSG_CTV5,"Connected\n"), "");

#ifdef  DATAKIT
        if (!strcmp(_Dev[D_CALLER], "DK")) {
                strcpy(_Tty, dtnamer(dkminor(fdl)));
                strcpy(Dc, (strrchr(_Tty, '/')+1));
                if ((_Fdl = fopen(_Tty, "r+")) == NULL) {
                        (void) fprintf(stderr, "ct: Cannot open %s, errno %d\n",
                                _Tty, errno);
                        cleanup(101);
                }
        }
#endif

    /* ignore some signals if they were ignored upon invocation of ct */
    /* or else, have them go to graceful disconnect */
    if (save_hup == (save_sig) SIG_IGN)
	(void) signal (SIGHUP, SIG_IGN);
    else
	(void) signal (SIGHUP, (void(*)(int))disconnect);

    if (save_quit == (save_sig) SIG_IGN)
	(void) signal (SIGQUIT, SIG_IGN);
    else
	(void) signal (SIGQUIT, (void(*)(int))disconnect);

    if (save_int == (save_sig) SIG_IGN)
	(void) signal (SIGINT, SIG_IGN);
    else
	(void) signal (SIGINT, (void(*)(int))disconnect);

    (void) signal (SIGTERM, (void(*)(int))disconnect);
    (void) signal (SIGALRM, (void(*)(int))disconnect);

    (void) sleep (2);		/* time for phone line/modem to settle */

    if (!hangup) {	/* close files - for neatness */
	(void) close(0);
	(void) close(1);
	if (isatty (2))
            (void) close (2);
    }

    _Log_on = time ((long *) 0);

    /*
     * If there was a uugetty on the port then sleep to maintain
     * the connection (via the open file descriptor fdl) until the
     * new uugetty has been spawned and opened the port.
     */
    if (uugettyflag) {	/* there is a uugetty on the line */
	CDEBUG(4,MSGSTR(MSG_CTCD1,"there is a shared or delayed getty; exit\n"), 0);
	sleep(10);
	exit(0);
    }

    CDEBUG(4, MSGSTR(MSG_CTCD2,"start getty %s "), Dc );
    CDEBUG(4, "%s\n", fdig(_Flds[F_CLASS]));
    for (;;) {
	switch(_Pid = fork()) {
	case -1:	/* fork failed */
	    if ((!hangup || Verbose))
		(void) fprintf(stderr,MSGSTR(MSG_CT19,
		       "ct: can't fork for getty\n"));
	    cleanup(101);
	    /*NOTREACHED*/

	case 0:		/* child process */
	    startat ();
	    (void) setpgrp ();	      /* getting /dev/tty right for children */
	    (void) signal(SIGHUP, SIG_DFL);  /* so child will exit on hangup */
	    (void) execl (GETTY, "getty", _Tty, NULL);
	    /* exec failed */
	    cleanup(101);
	    /*NOTREACHED*/

	default:	/* parent process */
	    break;
	}

	/* Parent process */

	while (wait (&_Status) != _Pid);
	if ((_Status & 0xff00) < 0) {
	    if (!hangup)
		VERBOSE(MSGSTR(MSG_CTV6, "ct: can't exec getty\n"), "");
	    cleanup(101);
	}

        (void) alarm (20);
	_Fdl = fopen(_Tty, "r+");
	if (!_Fdl) {
		fprintf(stderr, MSGSTR(MSG_CT24, 
			"Cannot open tty stream. errno=%d\n"), errno);
		cleanup(errno);
	}
        rewind (_Fdl);	/* flush line */

        (void) fprintf(_Fdl, MSGSTR(MSG_CT21,"\nReconnect? "));
        rewind (_Fdl);
        c = getc (_Fdl);

        if (c == EOF || tolower (c) == 'n')
	    disconnect (0);	/* normal disconnect */
        while ( (c = getc(_Fdl)) != EOF && c != '\n')
    	    ;
        (void) alarm (0);
    }
}

static
void
disconnect (int code) {
register int    pid,
                hrs,
                mins,
                secs;
register char  *aptr;
extern char *getenv ();
struct termios  termio;

    (void) alarm(0);
    (void) signal (SIGALRM, SIG_IGN);
    (void) signal (SIGINT, SIG_IGN);
    (void) signal (SIGTERM, SIG_IGN);

    _Log_elpsd = time ((long *) 0) - _Log_on;

    (void) tcgetattr (fileno(_Fdl), &termio);
    termio.c_cflag |= HUPCL;		/* speed to zero for hangup */
    (void) tcsetattr(fileno(_Fdl), TCSANOW, &termio);  /* hang up terminal */
    (void) fclose (_Fdl);

    DEBUG(5, "Disconnect(%d)\n", code);
    VERBOSE(MSGSTR(MSG_CTV7,"Disconnected\n"), "");

    stopat (_Flds[F_PHONE]);

    cleanup(code);
}

/*
 * clean and exit with "code" status
 */
void
cleanup (register int    code)
{
    CDEBUG(5, MSGSTR(MSG_CTCD3, "cleanup(%d)\n"), code);
    clrlock (CNULL);
    if (*_Tty != '\0') {
	CDEBUG(5, "chmod/chown %s\n", _Tty);
	if (chown (_Tty , UUCPUID, UUCPGID) < 0 || chmod (_Tty , TTYMOD) < 0) {
	    CDEBUG(5, MSGSTR(MSG_CTCD4, "Can't chown/chmod on %s "), _Tty);
	    CDEBUG(5, MSGSTR(MSG_CTCD5, "to %d\n"), UUCPUID);
	}
    }
    if (_Pid) { /* kill the child process */
	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);
	(void) kill (_Pid, SIGKILL);
    }
    exit (code);
}

/*	gdev()
 * Find an available line with a dialer on it.
 * Set a temporary lock file for the line.
 * Return:
 *	>0 - got a dialer
 *	<0 - failed - return the number of possible dialers
 *	0 - not dialers of requested class on the system.
 */

static
int
gdev (flds)
char   *flds[];
{
    int	count = 0,
	status;

    devreset();
    while ((status = rddev ("ACU", _Dev, _Devbuf, D_MAX)) != FAIL) {
	/* check caller type */
	if (!EQUALS (flds[F_TYPE] /* "ACU" */, _Dev[D_TYPE]))
	    continue;
	/* check class, check (and possibly set) speed */
	if (!EQUALS (flds[F_CLASS] /* speed */, _Dev[D_CLASS]))
	    continue;
	count++;

	if (ttylock(_Dev[D_LINE]) == FAIL)
	    continue;

	/* found available dialer and set temporary lock */
	return (count);

    }
    return (- count);
}

/*
 * Check if there is a uugetty active on this line.
 * Return:
 *	0 - there is no getty on this line
 *	1 - found a uugetty on this line
 */

static
int
uugetty(line)
char *line;
{
    register struct utmp   *u;

    while ((u = getutent()) != NULL) {
	if (u->ut_type == LOGIN_PROCESS
	    && EQUALS(u->ut_line, line)) {
		CDEBUG(7, "ut_line %s, ", u->ut_line);
		CDEBUG(7, "ut_user %s, ", u->ut_user);
		CDEBUG(7, "ut_id %.4s, ", u->ut_id);
		CDEBUG(7, "ut_pid %d\n", u->ut_pid);

		/* see if the process is still active */
		if (kill(u->ut_pid, 0) == 0 || errno == EPERM) {
		    CDEBUG(4, MSGSTR(MSG_CTCD6, "process still active\n"), 0);
		    return(1);
		}
	}
    }
    return(0);
}

/*
 * Create an entry in utmp file if one does not already exist.
 */
static
void
startat () {
	
    struct utmp utmpbuf;
    register struct utmp   *u,
                           *oldu;
    FILE * fp;



/*	Set up the prototype for the utmp structure we want to write.	*/

    u = &utmpbuf;
    zero (&u -> ut_user[0], sizeof (u -> ut_user));
    zero (&u -> ut_line[0], sizeof (u -> ut_line));

/*	Fill in the various fields of the utmp structure.		*/

    u -> ut_id[0] = _Tty[strlen(_Tty)-2];
    u -> ut_id[1] = _Tty[strlen(_Tty)-1];
    u -> ut_id[2] = '\0';
    u -> ut_id[3] = '\0';
    u -> ut_pid = getpid ();

    u -> ut_exit.e_termination = 0;
    u -> ut_exit.e_exit = 0;
    u -> ut_type = INIT_PROCESS;
    time (&u -> ut_time);
    setutent ();		/* Start at beginning of utmp file. */

/*	For INIT_PROCESSes put in the name of the program in the	*/
/*	"ut_user" field.						*/

    strncpy (&u -> ut_user[0], "getty", sizeof (u -> ut_user));
    strncpy (&u -> ut_line[0], Dc, sizeof (u -> ut_line));

/*	Write out the updated entry to utmp file.			*/
    pututline (u);

/*	Now attempt to add to the end of the wtmp file.  Do not create	*/
/*	if it doesn't already exist.  **  Note  ** This is the reason	*/
/*	"r+" is used instead of "a+".  "r+" won't create a file, while	*/
/*	"a+" will.							*/

    if ((fp = fopen (WTMP_FILE, "r+")) != NULL) {
	fseek (fp, 0L, 2);	/* Seek to end of file */
	fwrite (u, sizeof (*u), 1, fp);
	fclose (fp);
    }
    endutent ();
}

/*
 * Change utmp file entry to "dead".
 * Make entry in ct log.
 */

static
void
stopat (num)
char   *num;
{
    register long   stopt;
    struct utmp utmpbuf;
    register struct utmp   *u,
                           *oldu;
    struct utmp *getutid ();
    FILE * fp;

    stopt = time ((long *) 0);

/*	Set up the prototype for the utmp structure we want to write.	*/

    setutent();
    u = &utmpbuf;
    zero (&u -> ut_user[0], sizeof (u -> ut_user));
    zero (&u -> ut_line[0], sizeof (u -> ut_line));

/*	Fill in the various fields of the utmp structure.		*/

    u -> ut_id[0] = _Tty[strlen(_Tty)-2];
    u -> ut_id[1] = _Tty[strlen(_Tty)-1];
    u -> ut_id[2] = '\0';
    u -> ut_id[3] = '\0';
    u -> ut_pid = _Pid;
    strncpy (&u -> ut_line[0], Dc, sizeof (u -> ut_line));
    u -> ut_type = USER_PROCESS;

/*	Find the old entry in the utmp file with the user name and	*/
/*	copy it back.							*/

    if (u = getutid (u)) {
	utmpbuf = *u;
	u = &utmpbuf;
    }

    u -> ut_exit.e_termination = _Status & 0xff;
    u -> ut_exit.e_exit = (_Status >> 8) & 0xff;
    u -> ut_type = DEAD_PROCESS;
    time (&u -> ut_time);

/*	Write out the updated entry to utmp file.			*/

    pututline (u);

/*	Now attempt to add to the end of the wtmp file.  Do not create	*/
/*	if it doesn't already exist.  **  Note  ** This is the reason	*/
/*	"r+" is used instead of "a+".  "r+" won't create a file, while	*/
/*	"a+" will.							*/

    if ((fp = fopen (WTMP_FILE, "r+")) != NULL) {
	fseek (fp, 0L, 2);	/* Seek to end of file */
	fwrite (u, sizeof (*u), 1, fp);
	fclose (fp);
    }
    endutent ();

/*	Do the log accounting 					*/

    if (exists (LOG) && (fp = fopen (LOG, "a")) != NULL) {
	char   *aptr;
	int     hrs,
	        mins,
	        secs;
	extern char *getenv ();

 	/* ignore user set TZ for logfile purposes */
	if ( (aptr = getenv ("TZ")) != NULL )
		*aptr = '\0';

	(aptr = ctime (&_Log_on))[16] = '\0';
	hrs = _Log_elpsd / 3600;
	mins = (_Log_elpsd %= 3600) / 60;
	secs = _Log_elpsd % 60;
	(void) fprintf(fp, "%-8s ", getpwuid (getuid ()) -> pw_name);
	(void) fprintf(fp, "(%4s)  %s ", fdig(_Flds[F_CLASS]), aptr);
	if (hrs)
	    (void) fprintf(fp, "%2d:%.2d", hrs, mins);
	else
	    (void) fprintf(fp, "   %2d", mins);
	(void) fprintf(fp, ":%.2d  %s\n", secs, num);
	(void) fclose (fp);
    }
}
static
int
exists (file)
char   *file;
{
    struct stat statb;
    extern  errno;

    if (stat (file, &statb) == -1 && errno == ENOENT)
	return (0);
    return (1);
}

static
void
zero (adr, size)
register char  *adr;
register int    size;
{
    while (size--)
	*adr++ = '\0';
}
