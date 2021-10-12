static char sccsid[] = "@(#)25	1.37.2.5  src/bos/usr/lib/sendmail/main.c, cmdsend, bos41J, 9510A_all 2/24/95 10:45:34";
/* 
 * COMPONENT_NAME: CMDSEND main.c
 * 
 * FUNCTIONS: MSGSTR, Mmain, brksig, disconnect, finis, freeze, 
 *            initconf, initmacros, intsig, mkqdir, semsig, semwait, 
 *            t_delta, t_mark, wiz, wrcons 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
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

# define  _DEFINE

#include <locale.h>
# include <signal.h>
# include <stdio.h>
# include <ctype.h>
# include <errno.h>
# include <string.h>
# include <fcntl.h>
# include <unistd.h>
# include "conf.h"
# include "useful.h"
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/utsname.h>
# include <netinet/in.h>
# include <sys/ipc.h>
# include <sys/sem.h>
#include <sys/file.h>
#include <sgtty.h>
#include <arpa/nameser.h>
#include <resolv.h>
#ifdef _CSECURITY
#include <sys/audit.h>
#endif _CSECURITY
# include "sysexits.h"
# include "sendmail.h"
#include <sys/socket.h>

#include <langinfo.h>
#include <nl_types.h>
#include "sendmail_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SENDMAIL,n,s) 

#if ((defined LOG) || (defined _CSECURITY))
#include <sys/syslog.h>
#endif

key_t ftok ();
char  *getenv ();
void  exit ();
void  _exit ();
char  *crypt ();
long  convtime ();
ENVELOPE  *newenvelope ();
char  **myhostname ();
char  *arpadate ();
char  **prescan ();
int  putheader (), putbody ();
int finis();
void intsig(int);
void initconf(int);
int freeze();
int freezenl();
char  *xalloc ();
char  *safestr ();
extern int validdomain ();

#ifdef LOCAL_DEBUG
void brksig();				/* signal handler to enter debugger */
#endif LOCAL_DEBUG

int  Sid;				/* public semaphore id */
int  Scount;				/* max user count for cleanup */

int  InQueueDir;
uid_t  CurUid;
gid_t  CurGid;
int  Either, Both;
extern char  *Version;
extern char  BSDVersion[];
#ifdef DEBUG
extern char  Salt[];
#endif DEBUG
char  *DelimChar;
extern int  SyserrLog;

#ifdef PIDFILE 				/* file to store current named PID */
char	*PidFile = PIDFILE;
#else
char	*PidFile = "/etc/sendmail.pid";	
#endif

static char Logfile[] = "/tmp/slog";
static struct sigvec sigin = { 0, 0, 0 };

int local_nls=FALSE;

/*
**  SENDMAIL -- Post mail to a set of destinations.
**
**	This is the basic mail router.  All user mail programs should
**	call this routine to actually deliver mail.  Sendmail in
**	turn calls a bunch of mail servers that do the real work of
**	delivering the mail.
**
**	Sendmail is driven by tables read in from /usr/lib/sendmail.cf
**	(read by readcf.c).  Some more static configuration info,
**	including some code that you may want to tailor for your
**	installation, is in conf.c.  You may also want to touch
**	daemon.c (if you have some other IPC mechanism), acct.c
**	(to change your accounting), names.c (to adjust the name
**	server mechanism).  Certain other constants may be tailored
**	below, in the main program.
**
**	Usage:
**		/usr/lib/sendmail [flags] addr ...
**
**		See the associated documentation for details.
**
**	Author:
**		Eric Allman, UCB/INGRES (until 10/81)
**			     Britton-Lee, Inc., purveyors of fine
**				database computers (from 11/81)
**		The support of the INGRES Project and Britton-Lee is
**			gratefully acknowledged.  Britton-Lee in
**			particular had absolutely nothing to gain from
**			my involvement in this project.
**
**	Modified by: Boyd Murrah
**		     IBM Corp., Austin, TX
*/

int		NextMailer;	/* "free" index into Mailer struct */
char		*FullName;	/* sender's full name */
ENVELOPE	BlankEnvelope;	/* a "blank" envelope */
ENVELOPE	MainEnvelope;	/* the envelope around the basic letter */
ADDRESS		NullAddress =	/* a null address */
		{ "", "", "" };

/*
**  Pointers for setproctitle.
**	This allows "ps" listings to give more useful information.
*/
char		**Argv = NULL;		/* pointer to argument vector */
char		*LastArgv = NULL;	/* end of argv */

char *bmode = "user=root and group=system only";
char *emode = "user=root or group=system only";
static char mn_alias[] = "Alias data base \"%sDB\" created";
static char mn_ealias[] = "Alias data base \"%sDB\" creation failure";
static char mn_invdb[] = "NLS data base \"%sDB\" is invalid";

int Src_fd = 0;  /* file descriptor for src socket if we are invoked by src */
#define CONFFILE	"/etc/sendmail.cf"
char  *conffile;			/* config file name */

/*
 *  The following definitions are for semaphore usage.
 */
#define LOCKFILE	"Lockfile"	/* in a queue directory */
#define NOPS		10000		/* big number, prevent deadlock! */

static int do_readcf(char *), mkqdir();
static t_mark (char *), t_delta(char *);
static wiz(char *, char *);

main(argc, argv, envp)
	int argc;
	char **argv;
	char **envp;
{
	register char *p;
	char *from;
	FILE *fp;
	typedef int (*fnptr)();
	register int i;
	int  pi;
	int queuemode = FALSE;		/* process queue requests */
	static int reenter = FALSE;
	int  err;
	int  uerrors, perrors;
	int  hopcount;
	struct sigvec  sigout;
	int kji = FALSE;
	char hlds[NL_TEXTMAX];
	struct sockaddr_in saddr;	/* used to check src socket */
#ifdef DEBUG
#define	PWLEN	25			/* max size of password entry	*/
	char  pw[PWLEN+1];		/* read buffer for password */
	char  *pwfile;			/* password file name	*/
	int  debugok;
	int  firstdb = TRUE;		/* set for first debug flag	*/
	int  pwlen;
	char  *opword;
#endif DEBUG


#ifdef LOG
#ifdef LOG_MAIL
	openlog("sendmail", LOG_PID, LOG_MAIL);
#else LOG_MAIL
	openlog("sendmail", LOG_PID);
#endif LOG_MAIL
#endif LOG

#ifdef _CSECURITY
	if ((auditproc((char *)0,AUDIT_STATUS,AUDIT_SUSPEND, (char *)0)) < 0) {
		syslog(LOG_ALERT, "%s : auditproc: %m", argv[0]);
		exit(1);
	}
#endif _CSECURITY

	/*
	 *  Check to see if we reentered.  This would normally happen if 
	 *  e_putheader or e_putbody were NULL when invoked.
	 *
	 *  Question: Under what circumstances does this occur?
	 *  Note: The syserr is assumed properly executable even though
	 *  it depends on a lot of global variables.  They have supposedly
	 *  already been initialized if we are reentered.
	 */
	if (reenter)
	{
	    syserr(MSGSTR(MN_EREENTER, "main: reentered!")); /*MSG*/
	    exit (EX_SOFTWARE);
	}
	reenter = TRUE;
	Ined = FALSE;

	/*
	 *  Set a bunch of global variables so that important things like
	 *  syserr can work.  Are there others?
	 */

        setlocale(LC_ALL,"");
	catd = catopen(MF_SENDMAIL,NL_CAT_LOCALE);

	/*
	 *  Set up envelopes.
	 */
	BlankEnvelope.e_puthdr = putheader;
	BlankEnvelope.e_putbody = putbody;
	BlankEnvelope.e_xfp = NULL;
	BlankEnvelope.e_from = NullAddress;
	BlankEnvelope.e_btype = BT_UNK;	
	CurEnv = &BlankEnvelope;
	MainEnvelope.e_from = NullAddress;

	/*
	 *  Miscellaneous
	 */
	InChannel = stdin;
	OutChannel = stdout;
	MotherPid = getpid();

	/* NLS stuff */
	NlFile = NULL;		/* no sendmail.nl file defined by default */
	IsoIn = FALSE;		/* treat incomming 8bit mail as ISO */
	NlEsc = TRUE;		/* encode all outgoing mail */
	Btype = BT_UNK;		/* default envelope body type */

	/*
	 *  File creation mask doesn't allow other.
	 */
	OldUmask = umask(S_IRWXO);

	/*
	 *  Sendmail MUST run setuid/setgid to root/system (via file mode).
	 */
        if (geteuid () || getegid ())	/* must run as superuser	*/
	{
	    strcpy (hlds, MSGSTR(MN_SET, bmode)); /*MSG*/
	    syserr (MSGSTR(MN_ESETUID, "Sendmail must run setuid/setgid as %s"), hlds); /*MSG*/
	    exit (EX_SOFTWARE);
	}

	/*
	 *  Gather logical info about REAL user/group id's for 
	 *  convenience in subsequent processing.
	 */
	CurUid = getuid ();
	CurGid = getgid ();
	Either = !(CurUid * CurGid);	/* TRUE if root OR system */
	Both   = !(CurUid + CurGid);	/* TRUE if root AND system */

	/*
	**  Be sure we have enough file descriptors.
	**	But also be sure that 0, 1, & 2 are open.
	*/

	i = open("/dev/null", O_RDWR);
	while (i >= 0 && i < 2)
		i = dup(i);
	for (i = 3; i < MAXFDESC; i++)	/* takes < 1/10 sec for 200	*/
		(void) close(i);
	errno = 0;

	/*
	 *  Set default values for variables.
	 *  Other values for these factors can be loaded from config files.
	 */
	initcf();

	/*
	 *  Allocate environment into new strings.  Is this necessary
	 *  since thawing has been removed?
	 */
	for (i=0; i<MAXUSERENVIRON && envp[i]!=NULL; i++)
		UserEnviron[i] = newstr(envp[i]);
	UserEnviron[i] = NULL;

	Argv = argv;			/* set up for proctitle		*/
	if (i > 0)
		LastArgv = envp[i - 1] + strlen(envp[i - 1]);
	else
		LastArgv = argv[argc - 1] + strlen(argv[argc - 1]);

	/*
	 *  When sendmail is exec'd, signal handling is either SIG_DFL 
	 *  or SIG_IGN for each signal (depending on parent).
	 *  There is a whole set of signals, any of which can affect
	 *  sendmail (though there are certain system restrictions
	 *  on who can generate them and how it is done).  Security
	 *  should always be kept in mind.  For instance, a core dump
	 *  breaks security, since the user can possibly obtain the
	 *  text of mail message by analyzing the core file.  Therefore,
	 *  it is a security item that sendmail "chdir" to a protected
	 *  directory before handling mail text in the mail queues.
	 *  This is now done, probably for convenience, but most likely
	 *  not for security reasons.
	 */

	/*
	 *  If SIGINT or SIGHUP are not ignored, attach them to a handler 
	 *  to unlock the queue file.  Do this without changing the setting
	 *  of the signal handling mode, in case it is already ignored.
	 */
	if (sigvec (SIGINT, (struct sigvec *) 0, &sigout))
	{
	    syserr (MSGSTR(MN_ESIGINT, "SIGINT sigvec failure")); /*MSG*/
	    exit (EX_OSERR);
	}

	if (sigout.sv_handler == (void (*) (int)) SIG_DFL)
	{
	    sigin.sv_handler = (void(*)(int))intsig;
	    if (sigvec (SIGINT, &sigin, (struct sigvec *) 0))
		syserr (MSGSTR(MN_ESIGINT2, "SIGINT: sigvec handler intsig failure"));
	}

	/*
	 *  Always attach SIGTERM to the unlock handler.
	 *  Always ignore SIGPIPE.
	 *  Always make SIGCLD default handling (with zombies).
	 */
	sigin.sv_handler = (void(*)(int))intsig;
	if (sigvec(SIGTERM, &sigin, (struct sigvec *) 0))
	    syserr (MSGSTR(MN_ESIGTERM, "SIGTERM: sigvec failure"));
	sigin.sv_handler = (void (*) (int)) SIG_IGN;
	if (sigvec (SIGPIPE, &sigin, (struct sigvec *)0))
	    syserr (MSGSTR(MN_ESIGPIPE, "SIGPIPE: sigvec failure"));
	sigin.sv_handler = (void (*) (int)) SIG_DFL;
	if (sigvec (SIGCLD, &sigin, (struct sigvec *)0))
	    syserr (MSGSTR(MN_ESIGCLD, "SIGCLD: sigvec failure"));

#ifdef LOCAL_DEBUG
	sigin.sv_handler = brksig;  /* set up handler to enter debugger */
	if (sigvec (SIGUSR1, &sigin, (struct sigvec *)0))
	    syserr ("SIGUSR1: sigvec failure");
#endif LOCAL_DEBUG

	errno = 0;			/* clear out any junk for err handlers*/
	from = NULL;

	/* initialize some macros, etc. */
	initmacros();

	conffile = NULL;
#ifdef DEBUG
	pwfile   = "/usr/lib/sendmail.pw";
#endif DEBUG

	/*
	 *  Scan and interpret argv flags.
	 */
	p = strrchr (argv[0], '/');	/* look for program name	*/
	if (p == NULL)  p = argv[0];
	else		p++;

	OpMode = 0;			/* init to "nothing"		*/
	if (strcmp(p, "mailq") == 0)
	    OpMode = MD_PRINT;
	if (strcmp(p, "newaliases") == 0)
	    OpMode = MD_INITALIAS;

#ifdef DEBUG
	debugok = FALSE;		/* init to "not ok"		*/
	if (Either)			/* if root or system		*/
	    debugok = TRUE;		/* then debug always allowed	*/
#endif DEBUG
	uerrors = perrors = 0;		/* clear error counters		*/
	hopcount = FALSE;		/* not seen yet			*/
        for (pi=1; pi<argc; pi++)
	{
	    errno = 0;		/* clear out any junk		*/

	    p = argv[pi];
	    if (p[0] != '-')	/* end of flags?		*/
		    break;

	    switch (p[1])
	    {
		/*
		 *  Specify operational mode.
		 */
		case 'b':
		    if (OpMode)
		    {
			syserr (MSGSTR(MN_EMODE, "Operation mode already set, or more than one '-b' flag")); /*MSG*/
			uerrors++;
			break;
		    }

		    if (!p[2])		/* specified nothing?		*/
		    {
			syserr (MSGSTR(MN_EMODE2, "No operation mode specified")); /*MSG*/
			uerrors++;
			break;
		    }

		    OpMode = p[2];	/* set mode to nonnull		*/
#ifdef DEBUG
		    opword = &p[3];	/* following string (can be empty) */
#endif DEBUG
		    switch (p[2])	/* validate the mode		*/
		    {
#ifdef DEBUG
			case MD_WIZ:
#endif DEBUG
			case MD_DAEMON:
			case MD_INITALIAS:
			case MD_FREEZE:
			case MD_NLFREEZE:
			case MD_NLTEST:
			case MD_ARPAFTP:
			    if (!Either)	/* root || system */
			    {
				strcpy (hlds, MSGSTR(MN_SET2, emode)); /*MSG*/
			        syserr (MSGSTR(MN_EMODE3, "Operation mode for %s"), hlds); /*MSG*/
			        perrors++;

			    } else if (p[2] == MD_DAEMON)
				(void) unsetenv("HOSTALIASES");

#ifdef DEBUG
			    if (p[2] == MD_WIZ)
			    {
				if (opword[0] == '\0')
				{
		        	    if (++pi >= argc || *(p = argv[pi]) == '-')
		        	    {
		    	    		pi--;
		    	    		syserr (MSGSTR(MN_EMODE4, "Op mode flag has no associated password")); /*MSG*/
			    		uerrors++;
				    }
				    else
					opword = p;
				}
			    }
#endif DEBUG
			    break;

			case MD_DELIVER:
			case MD_VERIFY:
			case MD_TEST:
			case MD_PRINT:
			case MD_SMTP:
			    break;

			default:
			    syserr (MSGSTR(MN_EMODE5, "Invalid operation mode \"%c\""), p[2]); /*MSG*/
			    uerrors++;
		    }
		    break;

		/*
		 *  Specify nondefault configuration file path.  This
		 *  may be specified with or without white space between
		 *  -C and the path.
		 */
		case 'C':
		    /*
		     *  First, always parse the correct number of fields for
		     *  the flag.
		     */
		    p += 2;			/* point to string */
		    if (*p == '\0')		/* name in next field? */
		    {
		        if (++pi >= argc || *(p = argv[pi]) == '-')
		        {
		    	    pi--;
		    	    syserr (MSGSTR(MN_ECFPATH, "'-C' flag has no associated path")); /*MSG*/
			    uerrors++;
			    break;
			}
		    }

		    /*
		     *  Next, reject duplicate flag.  This msg would normally
		     *  be the highest priority, except that we want to process
		     *  a multifield -C spec as a unit.
		     */
		    if (conffile != NULL)
		    {
			syserr (MSGSTR(MN_ECF, "More than one '-C' flag")); /*MSG*/
			uerrors++;
			break;
		    }

		    /*
		     *  Now save ptr to name and enable duplicate detection.
		     */
		    conffile = p;

		    /*
		     *  Next, require the path to be absolute.
		     */
		    if (p[0] != '/')	/* must be absolute path	*/
		    {			/* also tests for nonnull string */
			syserr (MSGSTR(MN_ECFABS, "Configuration file path must be absolute")); /*MSG*/
			uerrors++;
			break;
		    }

		    if (!Either)	/* if not root || system */
		    {
			strcpy (hlds, MSGSTR(MN_SET2, emode)); /*MSG*/
		        syserr (MSGSTR(MN_ECONF, "Configuration file specification for %s"), hlds); /*MSG*/
			perrors++;
			break;
		    }

		    break;

		/*
		 *  Perform debug printouts.
		 */
		case 'd':
		    /*
		     *  Always de-buffer stdout.
		     *  Always print internal version number.
		     *  Do all this whether debug flags can be set or not.
		     */
		    setbuf(stdout, (char *) NULL);
		    (void) printf (MSGSTR(MN_VERSION, "Version %s\n"), Version); /*MSG*/

		    /*
		     *  Base -d flag is finished at this point.
		     */
		    if (p[2] == '\0')	/* want's level only? */
		        break;

# ifdef DEBUG
		    /*
		     *  If not already verified or ok, check debug password
		     *  file for match with string after first -d flag.
		     */
		    if ( firstdb  && 
		         !debugok && 
		         (i = open (pwfile, O_RDONLY)) >= 0
		       )
		    {
		        pwlen = read (i, pw, PWLEN);
		        if (pwlen >= 5 && pwlen <= PWLEN)
		        {
		    	    char  seed[3];
			    char *pwp;

			    pw[pwlen] = '\0'; /* terminate the string */

			    (void) strncpy (seed, pw, 2);
			    seed[2] = '\0';

			    pwp = crypt (&p[2], seed);

			    if (!strcmp (pw, pwp))
			        debugok = TRUE;
		        }
		        (void) close (i);
		    }
		    errno = 0;			/* don't leave any junk */
		    if (firstdb)		/* don't do above again */
		        firstdb = FALSE;

		    if (debugok)		/* if passed for debug	*/
		    {
			/*
			 *  We might be "passed" via uid/gid without password.
			 *  (Passwords don't start with digits).
			 */
		        if (isdigit (p[2])) /* skip uneeded password */
		        {
		            tTsetup(tTdvect, sizeof tTdvect, "21.1");
		            tTflag(&p[2]);
		        }
			_res.options |= RES_DEBUG;
		    }
		    else
		    {
		        syserr ("Debug flags have restricted usage");
		        perrors++;
		    }
		    break;
# else DEBUG
		    syserr (MSGSTR(MN_EDEBUG, "Debug mode not available")); /*MSG*/
		    perrors++;
		    break;
# endif DEBUG

		/*
		 *  Set "from" address
		 */
		case 'f':
		case 'r':	/* obsolete -f flag */
		    /*
		     *  First, always parse the correct number of fields for
		     *  the flag.
		     */
		    p += 2;
		    if (*p == '\0')
		    {
		        if (++pi >= argc || *(p = argv[pi]) == '-')
		        {
		    	    pi--;
		    	    syserr (MSGSTR(MN_ENOADDR, "'-f' flag has no associated address")); /*MSG*/
			    uerrors++;
			    break;
			}
		    }

		    /*
		     *  Next, prevent duplicates.  This would normally be the
		     *  highest priority msg except for the requirement to keep
		     *  any fields together to process the flag as a unit.
		     */
		    if (from != NULL)
		    {
		    	syserr (MSGSTR(MN_EFFLAG, "More than one '-f' flag")); /*MSG*/
		    	uerrors++;
		    	break;
		    }

		    p = safestr(p);
		    from = newstr(p);
		    break;

		/*
		 *  Set full name.
		 */
		case 'F':
		    /*
		     *  First, always parse the correct number of fields for
		     *  the flag.
		     */
		    p += 2;
		    if (*p == '\0')
		    {
		        if (++pi >= argc || *(p = argv[pi]) == '-')
		        {
		    	    pi--;
		    	    syserr (MSGSTR(MN_EFFLAG2, "'-F' flag has no associated name")); /*MSG*/
		    	    uerrors++;
		    	    break;
		        }
		    }

		    /*
		     *  Next, prevent duplicates.  This would normally be the
		     *  highest priority msg except for the requirement to keep
		     *  any fields together to process the flag as a unit.
		     */
		    if (FullName != NULL)
		    {
		        syserr (MSGSTR(MN_EFFLAG3, "More than one '-F' flag")); /*MSG*/
		        uerrors++;
		        break;
		    }

		    p = safestr(p);
		    FullName = newstr(p);
		    break;

		/*
		 *  Set hop count
		 */
		case 'h':
		    /*
		     *  First, always parse the correct number of fields for
		     *  the flag.
		     */
		    p += 2;
		    if (*p == '\0')
		    {
		        if (++pi >= argc || !isdigit (*(p = argv[pi])))
		        {
		    	    pi--;
		    	    syserr (MSGSTR(MN_EHFLAG, "'-h' flag has no associated hop count")); /*MSG*/
		    	    uerrors++;
		    	    break;
		        }
		    }

		    /*
		     *  Next, prevent duplicates.  This would normally be the
		     *  highest priority msg except for the requirement to keep
		     *  any fields together to process the flag as a unit.
		     */
		    if (hopcount)
		    {
			syserr (MSGSTR(MN_EHFLAG2, "More than one '-h' flag")); /*MSG*/
			uerrors++;
			break;
		    }

		    hopcount = TRUE;
		    CurEnv->e_hopcount = atoi (p);
		    break;
		
		/*
		 *  Defeat aliasing.
		 */
		case 'n':
		    NoAlias = TRUE;
		    break;

		/*
		 *  Set a configuration option.
		 */
		case 'o':
		    /*
		     *  "safe" flag is FALSE because setoption is being
		     *  called from data on user invocation line.
		     *  Options from command line are sticky.  They won't
		     *  be overridden by subsequent setoption calls for
		     *  those options.
		     */
		    if (err = setoption(p[2], &p[3], FALSE, TRUE))
		    {
		        if (err == EX_NOPERM)
		            perrors++;
		        else
		            uerrors++;
		    }
		    break;

		/*
		 *  Specify queue handling.
		 */
		case 'q':
		    /*
		     *  See duplicates
		     */
		    if (queuemode)
		    {
			syserr (MSGSTR(MN_EQUEUE, "Queue mode already specified")); /*MSG*/
			uerrors++;
			break;
		    }

		    queuemode = TRUE;
		    QueueIntvl = convtime(&p[2]);

		    if (!Either)
		    {
			strcpy (hlds, MSGSTR(MN_SET2, emode)); /*MSG*/
		        syserr (MSGSTR(MN_QUEUE, "Queue processing for %s"), hlds); /*MSG*/
		    	perrors++;

		    } else
			(void) unsetenv("HOSTALIASES");
		    break;

		/*
		 *  Read recipients from messages.
		 */
		case 't':
		    GrabTo = TRUE;
		    break;

		/*
		 *  Mail from a local mail program which may have NLS
		 *  extended characters in the body of the mail item.
		 *  Since do not know if running KANJI until having read 
		 *  configuration file, set some value to remind us later.
		 */
		case 'x':
		    Btype = BT_NLS;
		    local_nls = TRUE;
		    kji	= TRUE;
		    break;

		/*
		 *  Compatibility flags.  The real way to specify these is with
		 *  the -o flag.  These are preserved for compatibility with
		 *  certain old standards (delivermail program?).
		 */
		case 'c':	/* connect to non-local mailers */
		case 'e':	/* error message disposition */
		case 'i':	/* don't let dot stop me */
		case 'm':	/* send to me too */
		case 'T':	/* set timeout interval */
		case 'v':	/* give blow-by-blow description */
		    /*
		     *  "safe" flag is FALSE because setoption is being
		     *  called from data on user invocation line.
		     *  Options from command line are sticky.  They won't
		     *  be overridden by subsequent setoption calls for
		     *  those options.
		     */
		    if (err = setoption(p[1], &p[2], FALSE, TRUE))
		    {
		        if (err == EX_NOPERM)
		    	    perrors++;
		        else
		    	    uerrors++;
		    }
		    break;

		/*
		 *  Save "From" lines in headers.
		 */
		case 's':
		    /*
		     *  "safe" flag is FALSE because setoption is being
		     *  called from data on user invocation line.
		     *  Options from command line are sticky.  They won't
		     *  be overridden by subsequent setoption calls for
		     *  those options.
		     */
		    if (err = setoption('f', &p[2], FALSE, TRUE))
		    {
		        if (err == EX_NOPERM)
		    	    perrors++;
		        else
		    	    uerrors++;
		    }
		    break;

		/*
		 *  default: unknown flag
		 */
		default:
		    syserr (MSGSTR(MN_EFLAG, "Unknown flag '%s'"), p); /*MSG*/
		    uerrors++;
	    }
	}
	if (!OpMode)
	    OpMode = MD_DELIVER;
	if (conffile == NULL)
	    conffile = CONFFILE;
	if (uerrors)
	    exit (EX_USAGE);
	if (perrors)
	    exit (EX_NOPERM);

	/*
	 *  set full name from environment variable; moved here to maintain
	 *  BSD precedence of -F flag over environment var
	 */

	if (! FullName && (p = getenv("NAME")) && *p)
	{
	    p = safestr(p);
	    FullName = newstr(p);
	}

	/*
	 *  check to see if our stdin is a socket; if so, it means
	 *  that we were invoked by src.  This is a bad assumption,
       	 *  I therefore added an AND statement in the check to see if
 	 *  we used domain name sockets or we issued the command from 
 	 *  the command line. If by chance the way the structure of the 
 	 *  sockets should change, this would have to be revisited.
         */

	err = sizeof(saddr);
	if ((! getsockname(0, &saddr, &err)) && (saddr.sin_family == AF_UNIX)) {
        /* yup, it's src */

	    /* we can only be invoked by src in daemon mode */

	    if (OpMode != MD_DAEMON) {
		    syserr(MSGSTR(MN_SRCMODE,
"Invalid operation mode: use the \"-bd\" flag when invoking sendmail via SRC"));
		    exit (EX_USAGE);
	    }

	    /* dup the stdin socket to the src file descriptor */

	    if ((Src_fd = dup(0)) == -1) {
		    syserr(MSGSTR(DM_EDUP, "daemon: dup"));
		    exit(EX_OSERR);
	    }
	}

	/*
	 *  Freeze config file if that is the idea.
	 */
	if (OpMode == MD_FREEZE)
	    exit (freeze(conffile)); 

	/*
	 *  Load frozen configuration database.
	 */
	if (err = do_readcf(conffile))
	    exit (err);
	
	/*
	 *  Use MotherPid as the seed to srand(), so we can get nice 
	 *  irregular patterns of 0 and 1 for MX records.		
	 */
	if (bitnset(T_MX, NameServOpt))
		srand(MotherPid);

	/*
	 *  Ensure that both Netcode and MailCode are both set if used. 
	 *  If one is set and the other isn't, send a syslog message and   
	 *  set them according to the locale. 
	 */
	if ((*NetCode && (MailCode == NULL)) || ((NetCode == NULL) && *MailCode)) {
#ifdef LOG
		syslog(LOG_ERR, MSGSTR(MN_CODESET, "Both the ObNetCode and OOMailCode options must be set."));
#endif LOG
		MailCode = nl_langinfo(CODESET);
		NetCode = MailCode;
	}

	/*
	 *  In AIX V3.1, when running KANJI, Btype was set in beginning of
	 *  main with BT_UNK. BT_UNK was set to BT_JIS if in KJI. Since we
	 *  don't know if we are running KJI until we read the configuration
	 *  file, we wait and set the Btype here.
	 *  Set NlEsc to False if NetCode == MailCode, when in Kanji .
	 *  Need to set it, because it is checked later on.
	 */
	if (*NetCode && *MailCode) {
		if (kji)
			Btype = BT_MC;
		else    
			Btype = BT_NC;
		BlankEnvelope.e_btype = BT_NC;
		if (strcmp(NetCode, MailCode) == 0)
			NlEsc = FALSE;
	}
	else 
		BlankEnvelope.e_btype = Btype;

	/*
	 *  Freeze sendmail.nl file if that is the idea.
	 */
	if (OpMode == MD_NLFREEZE)  {
	    Verbose = TRUE;			/* get info msg at end */
	    exit (freezenl(NlFile)); /* freezenl will post errors */
	}


	/*
	 *  Load frozen sendmail.nl database.
	 */
	if (err = readnl(NlFile))
	{
# ifdef LOG
	    syslog (LOG_ERR, MSGSTR(MN_INVDB, mn_invdb), NlFile);
# endif LOG
	    exit (err);
	}

#ifdef DEBUG
	/*
	 *  Encrypt new debug password, if requested.
	 */
	if (OpMode == MD_WIZ)
	{
	    err = wiz (pwfile, opword);
#ifdef LOG
	    if (err)
		syslog (LOG_WARNING, "Debug password installation failure");
	    else
		syslog (LOG_NOTICE, "Debug password installed");

	    exit (err);
	}
#endif LOG
#endif DEBUG

	/*
	 *  Perform alias database update, if requested.
	 */
	if (OpMode == MD_INITALIAS)
	{
	    Verbose = TRUE;			/* get info msg at end */
	    err = readaliases (AliasFile);
#ifdef _CSECURITY
	    p = MSGSTR(MN_ALUPDT, "Alias database update");
	    auditwrite(ConfigEvent, (err ? AUDIT_FAIL : AUDIT_OK), p,
		strlen(p) + 1, NULL);
#endif _CSECURITY
#ifdef LOG
	    if (err)
		syslog (LOG_ERR, MSGSTR(MN_EALIAS, mn_ealias), AliasFile);
	    else
		syslog (LOG_NOTICE, MSGSTR(MN_ALIAS, mn_alias), AliasFile);
#endif LOG
	    exit (err);
	}

	/*
	 *  Handle print queue function.
	 */
	if (OpMode == MD_PRINT)
	{
		dropenvelope(CurEnv);
		printqueue();
		exit(EX_OK);
	}

	/*
	 *  Open alias database unless daemon.
	 *  Daemon opens it in srvrsmtp.
	 */
	if (OpMode != MD_DAEMON)
	{
	    if (err = openaliases(AliasFile))
	    {
#ifdef LOG
	        syslog (LOG_ERR, MSGSTR(MN_EALIAS2, "Alias data base \"%sDB\" is invalid"), AliasFile); /*MSG*/
#endif LOG
		exit (err);
	    }
	}

# ifdef DEBUG
	if (tTd(0, 15)) {
	    dumpcf(CurEnv, "main: after config");
	    dumpnl();
	    dumpal();
	}
# endif DEBUG

	/*
	 *  Switch to the main envelope.
	 */
	CurEnv = newenvelope(&MainEnvelope);
	MainEnvelope.e_flags = BlankEnvelope.e_flags;

	/*
	 *  If test mode, read addresses from stdin and process.
	 */
	if (OpMode == MD_TEST)
	{
	    char buf[MAXLINE];

	    (void) printf(MSGSTR(MN_ADDR, "ADDRESS TEST MODE\nEnter <ruleset> <address>\n")); /*MSG*/

	    while (1)
	    {
		register char **pvp;
		char *q;

		(void) printf("> ");
		(void) fflush(stdout);
		if (fgets(buf, sizeof buf, stdin) == NULL)
			finis();
		/*
		 *  Scan for non whitespace
		 */
		for (p = buf; isspace(*p); p++)
			continue;
		/*
		 *  Save start of set numbers
		 */
		q = p;
		/*
		 *  Scan for end or whitespace
		 */
		while (*p != '\0' && !isspace(*p))
			p++;
		/*
		 *  If end, ignore this line
		 */
		if (*p == '\0')
			continue;
		/*
		 *  Mark end of set numbers.
		 */
		*p = '\0';
		/*
		 *  Process the set
		 */
		do
		{
			char pvpbuf[PSBUFSIZE];

			/*
			 *  Prescan the line, using comma as delim.
			 */
			pvp = prescan(++p, ',', pvpbuf);
			/*
			 *  If no satisfactory result, continue
			 */
			if (pvp == NULL)
				continue;
			/*
			 *  Scan list of sets and apply them.
			 */
			p = q;
			while (*p != '\0')
			{
				rewrite(pvp, atoi(p));
				/*
				 *  Scan for next comma
				 */
				while (*p != '\0' && *p++ != ',')
					continue;
			}
		} while (*(p = DelimChar) != '\0');
	    }
	}

	/*
	 *  If nls test mode, read addresses from stdin and process.
	 */

	if (OpMode == MD_NLTEST)
	{
	    char buf[MAXLINE];

	    (void) printf(MSGSTR(MN_TESTMODE, "NLS AND ISO-8859 ADDRESS TEST MODE\nEnter <address>\n")); /*MSG*/

	    while (1)
	    {
		register char **pvp;
		char *q;
		char pvpbuf[PSBUFSIZE];

		(void) printf("> ");
		(void) fflush(stdout);
		if (fgets(buf, sizeof buf, stdin) == NULL)
			finis();

		/* Scan for non whitespace */
		for (p = buf; isspace(*p); p++)
			continue;

		/* If end, ignore this line */
		if (*p == '\0')
			continue;

		/*
		 *  Prescan the line, to remove comments
		 */
		pvp = prescan(p, '\0', pvpbuf);
		/*
		 *  If no satisfactory result, continue
		 */
		if (pvp == NULL)
			continue;
		/*
		 *  do a rewrite from ruleset 7 and find body type
		 */
		rewrite(pvp, 7);
		cataddr(pvp, buf, MAXLINE);
		printf(MSGSTR(MN_R7OUTPUT, "Output of ruleset 7 = \"%s\"\n"), buf); /*MSG*/
		switch (get_btype(buf))  {
		case BT_ESC:
			printf(MSGSTR(MN_BODYCODE, "Body encoding = NLS escape\n")); /*MSG*/
			break;
		case BT_ISO:
			printf(MSGSTR(MN_BODYCODE2, "Body encoding = ISO-8858/1\n")); /*MSG*/
			break;
		case BT_FLAT:
			printf(MSGSTR(MN_BODYCODE3, "Body encoding = Flat ASCII\n")); /*MSG*/
			break;
		}
	    }
	}


	/*
	 *  If the queue is simply to be processed once ...
	 */
	if (queuemode && OpMode != MD_DAEMON && QueueIntvl == 0)
	{
	    runqueue ();
	    finis();
	}

	/*
	 *  Turn into daemon if daemon mode or interval queue processing.
	 *
	 *	If we should also be processing the queue, start
	 *		doing it in background.
	 *	We check for any errors that might have happened
	 *		during startup.
	 */
	if (OpMode == MD_DAEMON || QueueIntvl != 0)
	{
	    /*
	     *  We release the count we have pulled from the queue clean
	     *  semaphore.  This semaphore will be handled specially
	     *  while daemon.  This adjusts the semadj value so that
	     *  this process will not touch the semaphore during exit.
	     *
	     *  All exceptions cause a syserr, but don't otherwise interfere
	     *  with sendmail operation.  We assume that a failure here means
	     *  that all subsequent semops in orderq and elsewhere will also
	     *  fail.  No queue cleaning will be attempted.
	     */
	    (void) semsig (Sid, 1, 0);

#ifdef DEBUG
	    if (tTd (0, 111))
		exit (99);
#endif DEBUG

	    /*
	     *  From here on out, ignore zombies.
	     */
	    sigin.sv_handler = (void (*) (int)) SIG_IGN;
	    if (sigvec(SIGCLD, &sigin, (struct sigvec *) 0))
		syserr(MSGSTR(MN_ESIGCLD2, "SIGCLD: sigvec handler SIG_IGN"));

	    /*
	     *  If not debug flag or src-invoked, then fork to become the
	     *  daemon process; else this process remains as the daemon
	     */

	    if (! Src_fd  /* zero if we are not invoked by src */
#ifdef DEBUG
			  && !tTd(3, 1)
#endif DEBUG
				       )
	    {
	    	/* put us in background */
	    	i = fork ();
	    	if (i < 0)
	    	{
	    	    syserr (MSGSTR(MN_EFORK, "Cannot fork"));
	    	    exit (EX_OSERR);
	    	}
	    	if (i)  /* parent just exits */
	    	    exit(EX_OK);
		MotherPid = getpid ();	/* child: get our new pid */
    	    }

	    /* tuck daemon's process id away */
	    fp = fopen(PidFile, "w");
	    if (fp != NULL) {
		fprintf(fp, "%d\n", MotherPid);
		(void) fclose(fp);
	    }

#ifdef DEBUG
	    if (!tTd(3, 1))
#endif DEBUG
		disconnect (TRUE);	/* disconnect from terminal */

# ifdef LOG
	    syslog (LOG_INFO, MSGSTR(MN_DAEMON,
		"Daemon/queue proc started%s, pid %d"),
		    (Src_fd ? MSGSTR(MN_SRC, " by SRC") : ""), MotherPid);
# endif LOG

    	    /*
    	     *  Everything from this point on is done in a child of this
	     *  process.  These children are timed queue runs and/or
	     *  daemon requests.
    	     */
    	    if (queuemode)
    	    {
		/*
		 *  Do queue run (in a child) and always schedule 
		 *  the next one automatically.
		 */
		trunqueue ();		/* special for clock */

		/*
		 *  If queue runs are all we do, then the base
		 *  level just pauses for signals, thus releasing
		 *  the cpu for other work.
		 */
		if (OpMode != MD_DAEMON)
		    while (1) (void) pause();  /* just handle sigs */
	    }

	    /*
	     *  Go handle requests from the net in daemon mode
	     */
	    if(!*MyHostName) /* for SMTP code, local host must be defined */
	    {
        	   syserr(MSGSTR(MN_UNDHOST, "Local host name is not defined"));
 	           exit(EX_NOLHOST);
	    }
	    getrequests();			/* never returns */

	/*NOTREACHED*/
	}
	
#ifdef DEBUG
	if (tTd (0, 111))
	    exit (99);
#endif DEBUG

	/*
	**  If running SMTP protocol, start collecting and executing
	**  commands.  This will never return.
	*/
	if (OpMode == MD_SMTP)
	    smtp();

	/*
	 *  Just send mail
	 */
	initsys();
	setsender(from);

	if (OpMode != MD_ARPAFTP && pi >= argc && !GrabTo)
	{
	    usrerr(MSGSTR(MN_ERECIP, "Recipient names must be specified")); /*MSG*/

	    /* collect body for UUCP return */
	    if (OpMode != MD_VERIFY)
	    	collect(FALSE);
	    finis();
	}
	if (OpMode == MD_VERIFY)
	    SendMode = SM_VERIFY;

	/*
	**  Scan argv and deliver the message to everyone.
	*/

	sendtoargv(&argv[pi]);

	/* if we have had errors sofar, arrange a meaningful exit stat */
	if (Errors > 0 && ExitStat == EX_OK)
		ExitStat = EX_USAGE;

	/*
	**  Read the input mail.
	*/

	CurEnv->e_to = NULL;
	if (OpMode != MD_VERIFY || GrabTo)
		collect(FALSE);
	errno = 0;

	/*
	 *  Save statistics for the "from" mailer.
	 */
	if (OpMode != MD_VERIFY)
	    markstats (FALSE, CurEnv->e_from.q_mailer->m_mno,CurEnv->e_msgsize);

# ifdef DEBUG
	if (tTd(1, 1))
	    (void) printf("From person = \"%s\"\n", CurEnv->e_from.q_paddr);
# endif DEBUG

	/*
	**  Actually send everything.
	**	If verifying, just ack.
	*/

	CurEnv->e_from.q_flags |= QDONTSEND;
	CurEnv->e_to = NULL;
	sendall(CurEnv, SM_DEFAULT);

	/*
	** All done.
	*/

	finis();
/*NOTREACHED*/
}
/*
 *  do_readcf - read the config file and set up globals based on config values
 */
static int do_readcf(char *conffile)
{
char buf[128];
STAB *st;
int err;


# ifdef DEBUG
	if (tTd(0, 1))
	    t_mark ("Start readcf");
# endif DEBUG
	if (err = readcf(conffile)) {
# ifdef LOG
	    syslog (LOG_ERR, MSGSTR(MN_ECF2,
		"Configuration data base \"%sDB\" is invalid"), conffile);
# endif LOG
	    return(err);
	}
# ifdef DEBUG
	if (tTd(0, 1))
	    t_delta ("End readcf");
# endif DEBUG

	/*
	 *  Local and Prog mailers MUST be present.
	 *  LocalMailer must be known for alias data base update.
	 */
	st = stab("local", ST_MAILER, ST_FIND);
	if (st == NULL)
	{
		syserr (MSGSTR(MN_ELOCAL,
		    "No local mailer defined in configuration file"));
		return(EX_DB);
	}
	else
		LocalMailer = st->s_mailer;

	st = stab("prog", ST_MAILER, ST_FIND);
	if (st == NULL)
	{
		syserr (MSGSTR(MN_EPROG,
		    "No prog mailer defined in configuration file"));
		return(EX_DB);
	}
	else
		ProgMailer = st->s_mailer;

	/* do heuristic mode adjustment */
	if (Verbose)
	{
		/*
		 *  "safe" flag is TRUE because we are coming from a
		 *  configuration file.  Access to config files is
		 *  protected elsewhere.
		 */

		/* turn off noconnect option */
		if (err = setoption('c', "F", TRUE, FALSE))
		    return(err);

		/* turn on interactive delivery */
		if (err = setoption('d', "i", TRUE, FALSE))
		    return(err);
	}

	/*
	 *  SMTP code requires our host name
	 */
	expand("\001j", buf, &buf[sizeof buf - 1], CurEnv);
	MyHostName = newstr(buf);

	/*
	 *  Make sure queue directory exists.  If not create it.  Change to it.
	 *  Status indicates whether all this worked out or not.
	 */
	if ((err = mkqdir ()) != EX_OK)
	    return(err);

	/*
	 *  Wait on the queue directory semaphore.  This counts for all
	 *  functions, except the daemon.  That uses extra semaphore 
	 *  manipulations.  We may delay here if a clean is in progress.
	 *  Any program exit will adjust the semaphore.
	 *
	 *  All exceptions cause a syserr, but don't otherwise interfere
	 *  with sendmail operation.  We assume that a failure here means
	 *  all subsequent semops will fail.  Therefore, no queue cleaning
	 *  will be attempted.
	 */
	(void) semwait (Sid, 1, 0, 0);

	return(EX_OK);
}
/*
 *  mkqdir - Assure that queue directory exists.  Log any problems and return
 *           status.
 */
static int
mkqdir ()
{
	int  err;
	key_t key;
#define SPRSTR "mkdir -p %s"
#define SPRSTRSIZ 6
	char  sysbuf[MAXNAME + SPRSTRSIZ];

	/*
	 *  "cd" to queue directory
	 */
	err = 0;			/* set after gone around once	*/
	while (chdir (QueueDir) < 0)
	{
	    if (err || errno != ENOENT)
	    {
		syserr (MSGSTR(MN_EQDIR, "Cannot change to or create queue directory \"%s\""),  QueueDir); /*MSG*/
		return (EX_SOFTWARE);
	    }
	    err = 1;			/* exit above if chdir fails again */

	    /*
	     * make subdirectory
	     */
	    if (strlen (QueueDir) >= MAXNAME)
	    {
		syserr (MSGSTR(MN_EQDIR2, "Queue directory path \"%s\" is too long"), QueueDir); /*MSG*/
		return (EX_DB);
	    }

	    (void) sprintf (sysbuf, SPRSTR, QueueDir);/* mkdir operation      */

	    /*
	     *  Don't check status on mkdir because someone else might have
	     *  made the subdirectory since our chdir above.  Just try
	     *  to chdir again.
	     */
	    (void) system (sysbuf);		/* do it, check w/ chdir */
	}
	errno = 0;

	InQueueDir = 1;			/* tell intsig that unlockqueue is ok */

	/*
	 *  Public semaphore for queue clean operation.
	 *
	 *  This semaphore is created and initialized to a certain very large
	 *  maximum value.  Any instance of sendmail which desires to use the 
	 *  queue pulls one or more values from the semaphore using semwait ().
	 *  When finished with the queue, semsig () is used to replace the
	 *  count.  This is a public semaphore, so that all instances
	 *  of sendmail operating in the same queue directory use the same
	 *  semaphore.
	 *
	 *  The orderq routine in the daemon (and queue print) function
	 *  tries to remove all counts from the semaphore which would be
	 *  available for removal if no one else were in the queue.  If this
	 *  operation is successful, then orderq cleans trash from the queue.
	 *  Others who try to gain access to the queue are pended until the
	 *  cleaning operation finishes.  When orderq is through cleaning, it 
	 *  replaces the counts that it removed, so that others can proceed.
	 *
	 *  Be thoroughly familiar with the semaphore documentation in the
	 *  AIX system calls guide before you tamper with this.  This es-
	 *  pecially includes the discussion of the "semadj" values for
	 *  each process.  These allow for automatic adjustment of the
	 *  semaphore when the process exits.  This is especially important
	 *  when a process dies for some uncontrollable reason.
	 *
	 *  Note: in most cases semsig is not explicitly used.  Rather,
	 *  process exit in combination with the "semadj" value resets
	 *  the semaphore properly.  However, in the vicinity of "fork"
	 *  operations and in the daemons, special processing occurs.
	 *
	 *  Certain exceptional conditions described at certain places in the
	 *  code indicate the manner in which the permanent public semaphore
	 *  can get out of adjustment.  The effect of this is that queue
	 *  cleaning operations will no longer take place until the 
	 *  semaphore is refreshed to its proper value.  This normally
	 *  takes place on the first sendmail operation on this queue after
	 *  the next system reboot.  In extreme cases, sendmail will stall.
	 *  The only answer in this case is to remove the semaphore (if
	 *  you don't want to reboot).  This is dangerous, however, to
	 *  running mail.
	 */

	/*
	 *  Exception processing here is rigorous, to prevent sendmail
	 *  from starting if there are problems.  Exceptions later are
	 *  treated more leniently.
	 */

	/*
	 *  Check for semaphore lock file.  Create if necessary.
	 */
	if ((err = open (LOCKFILE, O_RDWR | O_CREAT, 
				S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0)
	{
	    wrcons (MSGSTR(MN_ELOCK, "Cannot open lock file \"%s/%s\""), QueueDir, LOCKFILE); /*MSG*/

	    SyserrLog = LOG_EMERG;
	    syserr (MSGSTR(MN_ELOCK, "Cannot open lock file \"%s/%s\""), QueueDir, LOCKFILE); /*MSG*/

	    return (EX_SOFTWARE);
	}
	(void) close (err);

	/*
	 *  Make key for semaphore addressing.
	 */
	if ((key = ftok (LOCKFILE, 'M')) == (key_t) -1)
	{
	    wrcons (MSGSTR(MN_ESEM, "Error in ftok creating semaphore key, lock file \"%s/%s\""), QueueDir, LOCKFILE); /*MSG*/
	    SyserrLog = LOG_EMERG;
	    syserr (MSGSTR(MN_ESEM, "Error in ftok creating semaphore key, lock file \"%s/%s\""), QueueDir, LOCKFILE); /*MSG*/
	    return (EX_SOFTWARE);
	}

# ifdef DEBUG
	if (tTd (2, 1))
		(void) printf ("ftok: key = 0x%lx\n", key);
# endif DEBUG

	/*
	 *  Set up globals for semaphore control.
	 */
	Scount = NOPS;

	/*
	 *  Exclusively create semaphore.
	 *
	 *  The semaphore is for queue facility access for cleaning.
	 */
	Sid = semget (key, 1,
	    	IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

	/*
	 *  If we succeeded, the semaphore did not previously exist and we
	 *  are the first to create it.  All others get Sid < 0.
	 */
	if (Sid >= 0)
	{
# ifdef DEBUG
	    if (tTd (2, 1))
	        (void) printf ("semget create: Sid = %d\n", Sid);
# endif DEBUG

	    /*
	     *  Signal the facility access semaphore with the max count.
	     *  Any others waiting on it will now proceed.
	     */
	    SyserrLog = LOG_EMERG;
	    if ((err = semsig (Sid, Scount, 1)) != EX_OK)
		return (err);
	    SyserrLog = -1;		/* we didn't syserr */
	}
	else if (errno == EEXIST)		/* already there */
	{
	    /*
	     *  The semaphore already existed (but may not yet be fully
	     *  initialized by the above code in another sendmail process).
	     *  Try to attach to it.
	     */
	    if ((Sid = semget (key, 1, 0)) < 0)
	    {
	        wrcons (MSGSTR(MN_EATTACH, "Cannot attach to queue semaphore for \"%s\""),  QueueDir); /*MSG*/
	        SyserrLog = LOG_EMERG;
	        syserr (MSGSTR(MN_EATTACH, "Cannot attach to queue semaphore for \"%s\""),  QueueDir); /*MSG*/
		return (EX_SOFTWARE);
	    }

# ifdef DEBUG
	    if (tTd (2, 1))
	        (void) printf ("semget attach: Sid = %d\n", Sid);
# endif DEBUG

	    /*
	     *  Check to see if the semaphore is getting to be too small
	     *  in value.  This can happen if previous sendmail invocations
	     *  failed or were interrupted at certain sensitive points.  Those
	     *  interruptions or failures do not jeopardize the correct
	     *  operation of sendmail (except that queue cleaning operations
	     *  will cease being performed).  However, if the large value
	     *  that this semaphore is originally initialized to becomes
	     *  too small (near zero) it is possible that certain sendmail
	     *  invocations could begin to stall.  Therefore, check the
	     *  value.  If it is too small, just keep sendmail from starting.
	     *  This should not occur unless the system is up a very long time.
	     */
	    err = semctl (Sid, 0, GETVAL, 0);
	    if (err < 0 || (err < Scount/2 && err > 0))
	    {
	        wrcons (MSGSTR(MN_EQSMALL, "Queue semaphore value too small, directory \"%s\""),  QueueDir); /*MSG*/
	        SyserrLog = LOG_EMERG;
	        syserr (MSGSTR(MN_EQSMALL, "Queue semaphore value too small, directory \"%s\""),  QueueDir); /*MSG*/
		wrcons (MSGSTR(MN_EMGR, "Call SYSTEM MANAGER")); /*MSG*/
	        SyserrLog = LOG_EMERG;
		syserr (MSGSTR(MN_EMGR, "Call SYSTEM MANAGER")); /*MSG*/

		return (EX_SOFTWARE);
	    }
	}
	else
	{
	    wrcons (MSGSTR(MN_ESEMGET, "Error in semget for queue semaphore, directory \"%s\""),  QueueDir); /*MSG*/
	    SyserrLog = LOG_EMERG;
	    syserr (MSGSTR(MN_ESEMGET, "Error in semget for queue semaphore, directory \"%s\""),  QueueDir); /*MSG*/
	    return (EX_SOFTWARE);
	}

	/*
	 *  We have now created and initialized the semaphore, or have attached
	 *  to a preexisting (possibly incompletely initialized) semaphore.
	 *  The latter condition is OK.
	 */

	return (EX_OK);
}

/*
 *  semwait - wait on a semaphore.
 */
int
semwait (sid, k, nw, na)
int  sid;
int  k;					/* count to draw */
int  nw;				/* set to NOT wait */
int  na;				/* set to prevent semadj change */
{
	int  err;
	struct sembuf sop;

# ifdef DEBUG
	if (tTd (2, 1))
	{
	    (void) printf ("semwait (%d, %d, %d, %d): ", sid, k, nw, na);
	    (void) fflush (stdout);
	}
# endif DEBUG

	/*
	 *  Set up arg pkt.
	 */
	sop.sem_num = 0;		/* semaphore number */
	sop.sem_op  = -abs (k);		/* count to decrement */
	sop.sem_flg = (!na ? SEM_UNDO : 0) | (nw ? IPC_NOWAIT : 0);

	/*
	 *  Repeat the performance when signals interrupt.
	 */
	while ((err = semop (sid, &sop, 1)) < 0 && errno == EINTR)
	    ;

	/*
	 *  If IPC_NOWAIT and not enough counts were available...
	 */
	if (err < 0 && nw && errno == EAGAIN)
	{
# ifdef DEBUG
	    if (tTd (2, 1))
	        (void) printf ("tempfail\n");
# endif DEBUG
	    return (EX_TEMPFAIL);
	}

	if (err < 0)
	{
	    wrcons (MSGSTR(MN_SEMOP, "semwait: semop (%d) failure code %d, directory \"%s\""), sid, errno, QueueDir); /*MSG*/
	    if (SyserrLog < 0)  SyserrLog = LOG_ALERT;
	    syserr (MSGSTR(MN_SEMOP, "semwait: semop (%d) failure code %d, directory \"%s\""),  sid, errno, QueueDir); /*MSG*/
	    return (EX_SOFTWARE);
	}

# ifdef DEBUG
	if (tTd (2, 1))
	    (void) printf ("proceed\n");
# endif DEBUG

	return (EX_OK);
}

/*
 *  semsig - signal a semaphore.
 */
int
semsig (sid, k, na)
int  sid;
int  k;					/* count to put */
int  na;				/* set for no semadj adjustment */
{
	struct sembuf  sop;
	int  err;

# ifdef DEBUG
	if (tTd (2, 1))
	{
	    (void) printf ("semsig (%d, %d, %d): ", sid, k, na);
	    (void) fflush (stdout);
	}
# endif DEBUG

	/*
	 *  Now replace our user count.
	 *  Always adjust the semadj for the semaphore in this process.
	 */
	sop.sem_num = 0;
	sop.sem_op  = abs (k);
	sop.sem_flg = !na ? SEM_UNDO : 0;

	err = semop (sid, &sop, 1);

	if (err < 0)
	{
	    wrcons (MSGSTR(MN_ESEMSIG, "semsig: semop (%d) failure code %d, directory \"%s\""),  sid, errno, QueueDir); /*MSG*/
	    if (SyserrLog < 0)  SyserrLog = LOG_ALERT;
	    syserr (MSGSTR(MN_ESEMSIG, "semsig: semop (%d) failure code %d, directory \"%s\""),  sid, errno, QueueDir); /*MSG*/
	    return (EX_SOFTWARE);
	}

# ifdef DEBUG
	if (tTd (2, 1))
	    (void) printf ("ok\n");
# endif DEBUG

	return (EX_OK);
}

/*
 *  wrcons - write string to console.
 */
/*VARARGS1*/
wrcons (fmt, a, b, c, d, e)
char  *fmt;
{
	char hlds[NL_TEXTMAX], *hldp;
	FILE *fp;

	/* preserve the string since we make more calls to MSGSTR */
	strcpy(hlds, fmt);
	hldp = hlds;

	/*
	 *  Write msg to console, if possible.
	 */
	fp = fopen ("/dev/console", "a");
	if (fp != NULL)
	{
	    (void) fprintf (fp, MSGSTR(MN_SMNAME, "SENDMAIL: ")); /*MSG*/
	    (void) fprintf (fp, fmt, a, b, c, d, e);
	    (void) fprintf (fp, "\r\n");
	    (void) fclose (fp);
	}
}
/*
**  FINIS -- Clean up and exit.
**
**	Parameters:
**		none
**
**	Returns:
**		never
**
**	Side Effects:
**		exits sendmail
*/

finis()
{
# ifdef DEBUG
	if (tTd(2, 1))
	    (void) printf("\n====finis: stat %d e_flags %o\n",
		    ExitStat, CurEnv->e_flags);
# endif DEBUG

	/* clean up temp files */
	CurEnv->e_to = NULL;
	dropenvelope(CurEnv);

	/* and exit */
# ifdef LOG
	if (LogLevel > 11)
		syslog (LOG_DEBUG, MSGSTR(MN_FINI, "Finis, pid=%d"), getpid()); /*MSG*/
# endif LOG

	/* pretend everything's ok if we're in berknet error-disposal mode */
	exit (ErrorMode == EM_BERKNET ? 0 : ExitStat);
}

/*
**  INTSIG -- clean up on interrupt
**
**	This just arranges to exit.  It pessimises in that it
**	may resend a message.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Unlocks the current job.
*/

void intsig(int sig)
{
	FileName = NULL;
	SyserrLog = -1;

	/*
	 *  Try to avoid creating queue type files in user's current
	 *  directory.
	 */
	if (InQueueDir)
	    unlockqueue(CurEnv);

	exit(EX_OK);
}
/*
**  INITMACROS -- initialize the macro system
**
**	This just involves defining some macros that are actually
**	used internally as metasymbols to be themselves.
**	It also defines the hostname and version macros.
**
**	Parameters:
**		none.
**
**	Returns:
**		none (it will exit on fatal errors).
**
**	Side Effects:
**		initializes several macros to be themselves.
*/

struct metamac
{
	char	metaname;
	char	metaval;
};

struct metamac	MetaMacros[] =
{
	/* LHS pattern matching characters */
	'*', MATCHZANY,	'+', MATCHANY,	'-', MATCHONE,	'=', MATCHCLASS,
	'~', MATCHNCLASS,

	/* these are RHS metasymbols */
	'#', CANONNET,	'@', CANONHOST,	':', CANONUSER,	'>', CALLSUBR,

	/* the conditional operations */
	'?', CONDIF,	'|', CONDELSE,	'.', CONDFI,

	/* and finally the hostname lookup characters */
	'[', HOSTBEGIN,	']', HOSTEND,

	'\0'
};

initmacros()
{
	register struct metamac *m;
	register char **av, *q, *d, *p;
	char buf[5], jbuf[128];
	/* if BSDVersion changes then make sure version[] is big enough */
	static char version[(SYS_NMLN * 4) + 3];
	register int c;	
	struct utsname name;			

	buf[1] = '\0';
	for (m = MetaMacros; m->metaname != '\0'; m++)
	{
		buf[0] = m->metaval;
		define(m->metaname, newstr(buf), &BlankEnvelope);
	}
	buf[0] = MATCHREPL;
	buf[2] = '\0';
	for (c = '0'; c <= '9'; c++)
	{
		buf[1] = c;
		define(c, newstr(buf), &BlankEnvelope);
	}

	/*
	 *  define local host and domain name macros
	 */
	av = myhostname(jbuf, sizeof jbuf);
	if (jbuf[0] != '\0')
	{
		p = newstr(jbuf);
		setclass('w', p);  /* fully qualified form */
		if (q = strchr(p,'.'))
		    *q++ = '\0';  /* terminate at domain name */
		if (!validdomain(p, FALSE)) {
		    syserr(MSGSTR(MN_EHOST,
			"Illegal character in local host name \"%s\""), p);
		    exit(EX_DB);
		}
		define('w', p, &BlankEnvelope);  /* set hostname */
		setclass('w', p);  /* non-fully qualified form */
		
		/* define default local domain name */
		if (q) {
		    d = newstr(q);
		    if (!validdomain(d, TRUE)) {
			syserr(MSGSTR(MN_EDOM,
			    "Illegal character in local domain name \"%s\""),
			    d);
			exit(EX_DB);
		    }
		    define('D', d, &BlankEnvelope);
		    setclass('d', d);
		}
	}
	while (av != NULL && *av != NULL)
		setclass('w', *av++);

	/*
	 *  Set up 'v' macro.  Use internal program version if defined.
	 *  Else, use system name and version.
	 */
	if (*Version == '\0')
	{
	    if (uname (&name) < 0)
	    {
		syserr (MSGSTR(MN_EUNAME, "uname call failed")); /*MSG*/
		exit (EX_OSERR);
	    }

	    /*
	     *  Manufacture the string
	     */
	    (void) sprintf(version, "%s %s.%s/%s", name.sysname, name.version,
		name.release, BSDVersion);
	    Version = version;
	}

	define('v', newstr(Version), &BlankEnvelope);

	/* current time */
	define('b', arpadate((char *) NULL), &BlankEnvelope);
}
/*
**  DISCONNECT -- remove our connection with any foreground process
**
**	Parameters:
**		fulldrop -- if set, we should also drop the controlling
**			TTY if possible -- this should only be done when
**			setting up the daemon since otherwise UUCP can
**			leave us trying to open a dialin, and we will
**			wait for the carrier.
**
**	Returns:
**		none
**
**	Side Effects:
**		Trys to insure that we are immune to vagaries of
**		the controlling tty.
*/

disconnect(fulldrop)
	int fulldrop;
{
	int fd;
	struct sigvec in, out;

#ifdef DEBUG
	if (tTd(52, 1))
		(void) printf ("disconnect: In %d Out %d\n", fileno(InChannel),
						fileno(OutChannel));
	if (tTd(52, 5))
	{
		(void) printf ("don't\n");
		return;
	}
#endif DEBUG
	/* be sure we don't get nasty signals */
	sigin.sv_handler = (void (*) (int)) SIG_IGN;
	if (sigvec(SIGHUP, &sigin, (struct sigvec *) 0))
	   syserr(MSGSTR(MN_DSIGHUP, "disconnect: SIGHUP: handler SIG_IGN failer"));
	if (sigvec(SIGINT, &sigin, (struct sigvec *) 0))
	   syserr(MSGSTR(MN_DSIGINT, "disconnect: SIGINT: handler SIG_IGN failer"));
	if (sigvec(SIGQUIT, &sigin, (struct sigvec *) 0))
	   syserr(MSGSTR(MN_DSIGQUIT, "disconnect: SIGQUIT: handler SIG_IGN failer"));

	/* we can't communicate with our caller, so.... */
	HoldErrs = TRUE;
	ErrorMode = EM_MAIL;
	Verbose = FALSE;

	/* all input from /dev/null */
	if (InChannel != stdin)
	{
		(void) fclose(InChannel);
		InChannel = stdin;
	}
	(void) freopen("/dev/null", "r", stdin);

	/* output to the transcript */
	if (OutChannel != stdout)
	{
		(void) fclose(OutChannel);
		OutChannel = stdout;
	}
	if (CurEnv->e_xfp == NULL)
		CurEnv->e_xfp = fopen("/dev/null", "w");
	(void) fflush(stdout);
	(void) close(1);
	(void) close(2);
	while ((fd = dup(fileno(CurEnv->e_xfp))) < 2 && fd > 0)
		continue;

	/*
	 *  drop controlling TTY if we were not invoked by src
	 */
	if (fulldrop && ! Src_fd)
	    (void) setsid ();	/* set pgroup to current pid	*/

# ifdef LOG
	if (LogLevel > 11)
		syslog (LOG_DEBUG, MSGSTR(MN_BACK, "In background, pid=%d"),
		    getpid());
# endif LOG

	errno = 0;		/* clean up any leftovers		*/
	in.sv_handler = (void(*)(int))initconf;
	in.sv_mask = 0;
	in.sv_onstack = 0;
	sigvec (SIGHUP, &in, &out);
}

# ifdef DEBUG

# include <sys/times.h>

static long  t_start;
static struct tms  t_buffer;
/*long times ();*/

/*
 *  t_mark - Call times to mark current location in time.
 */
static t_mark (char *s)
{
    (void) printf ("t_mark: %s\n", s);

    t_start = times (&t_buffer);
}

/*
 *  t_delta - Calculate time delta and print it out if arg is true.
 */
static t_delta (char *s)
{
    long  delta;

    delta = times (&t_buffer) - t_start;

    (void) printf (MSGSTR(MN_TDELTA, "t_delta: %s: delta = %d\n"), s, (int) delta); /*MSG*/
}

# endif DEBUG

#ifdef DEBUG
/*
 *  wiz - create new debug password file
 *
 *  file - name of password file to create.
 *  word - name of user password.
 */
static wiz (char *file, char *word)
{
	int  i, pwlen;
	char  *p;

	/*
	 *  Remove file name from any attachment to unknown things.
	 */
	if (unlink (file))
	{
	    if (errno != ENOENT)
	    {
	        syserr ("Can't remove password file \"%s\"", file);
	        return (EX_DB);
	    }
	}
	errno = 0;			/* don't leave junk around	*/

	/*
	 *  Create and write the new password file.
	 */
	if ((i = creat (file, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0)
	{
	    syserr ("Error creating debug password file \"%s\"", file);
	    return (EX_DB);
	}

	if (strlen (word) < 6)
	{
	    syserr ("Debug password is too short");
	    return (EX_USAGE);
	}

	if (isdigit (*word))
	{
	    syserr ("Debug password must not begin with digit");
	    return (EX_USAGE);
	}

	p = crypt (word, Salt);
	pwlen = strlen (p);
	if (write (i, p, (unsigned) pwlen) != pwlen || close (i))
	{
	    (void) close (i);	/* in case write failed */
	    (void) unlink (file);	/* bad file might be easy password */
	    syserr ("Error writing password file \"%s\"", file);
	    return (EX_DB);
	}

	return (EX_OK);
}

#endif DEBUG

#ifdef DEBUG

/*
 *  logopen - open logf file onto fd, return fd pointing to old fd
 */
static int logopen(fd, logf)
int fd;
char *logf;
{
int oldout, logfd;
    
    oldout = dup(fd);  /* save fd */
    /* dup fd to log file */
    if ((logfd = open(logf, O_WRONLY | O_APPEND | O_CREAT, 0777)) == -1) {
	perror("open");
	return(-1);
    }
    if (dup2(logfd, fd) == -1) {
	perror("logopen: dup2");
	return(-1);
    }
    close(logfd);
    return(oldout);  /* so that caller can reset it later */
}

/*
 *  logclose - re-set fd to oldout fd and close logfile
 */
static int logclose(fd, oldout)
int fd;
int oldout;
{
    /* dup fd back to original one */
    if (dup2(oldout, fd) == -1) {
	perror("logclose: dup2");
	return(-1);
    }
    close(oldout);
    return(0);
}

#endif DEBUG

/*
 *  initconf - re-read the configuration data
 */
void initconf(int s)
{

ENVELOPE *oldenv;
# ifdef DEBUG
int oldout;
# endif DEBUG

	/* only re-configure if we are the parent and we are in daemon
	   mode, since we don't want to trash the config data in the
	   middle of processing a message */
	
	if (OpMode == MD_DAEMON && getpid() == MotherPid) {
	
# ifdef DEBUG
	    if (tTd(0, 15)) {
		/* redirect stdout to the log file */
		oldout = logopen(1, Logfile);
		dumpcf(&BlankEnvelope, "Blank before freeze");
		dumpnl();
		dumpal();
	    }
# endif DEBUG

	    /* we set the CurEnv to the BlankEnvelope before re-reading the
	       config file since we want the new data to affect the template
	       envelope, not the current one; the next envelope generated by
	       newenvelope() will then grab this data from BlankEnvelope */

	    oldenv = CurEnv;  /* save it */
	    CurEnv = &BlankEnvelope;
	    (void) freecf();  /* free and clear old config data */
	    (void) initcf();  /* init default values */
	    (void) initmacros();  /* and default macros */

	    /* if the config filename has been initialized, use it; otherwise
	       use the default config filename */

	    (void) freeze(conffile ? conffile : CONFFILE);

	    /* re-read the alias file; there are no globals to worry about
	       since it is all in the dbm file; note that we don't re-open
	       it since we are in daemon mode, and it will be attempted on
	       the next connection */
	    
	    if (readaliases (AliasFile))
		syslog (LOG_ERR, MSGSTR(MN_EALIAS, mn_ealias), AliasFile);
	    else
		syslog (LOG_NOTICE, MSGSTR(MN_ALIAS, mn_alias), AliasFile);
	    
	    /* free, re-freeze, and re-read the nls config file */

	    if (NlFile) {
		(void) freenl();
		if (! freezenl(NlFile))  /* it posts any error messages */
		    if (readnl(NlFile))  /* error */
			syslog (LOG_ERR, MSGSTR(MN_INVDB, mn_invdb), NlFile);
	    }

	    CurEnv = oldenv;  /* restore it */

# ifdef DEBUG
	    if (tTd(0, 15)) {
		dumpcf(&BlankEnvelope, "Blank after freeze");
		dumpnl();
		dumpal();
		/* close log file and reset stdout */
		logclose(1, oldout);
	    }
# endif DEBUG
	}
}

/*
** FREEZE -- freeze config file if it is requested
*/

freeze(conffile)
char *conffile;
{
    int err;
    register char *p;

    /*
     *  If the freeze fails, the syserr's show on the console.  
     *  A failure can't affect the sendmail environment.
     *  No syslog's are generated.  (These can't go to the log
     *  file since QueueDir isn't defined;  they need not be logged
     *  to the console since the syserr's already are.)
     */
    err = freezecf(conffile);

#ifdef _CSECURITY
    p = MSGSTR(MN_FRCONF, "Freeze configuration file");
    auditwrite(ConfigEvent, (err ? AUDIT_FAIL : AUDIT_OK), p,
	strlen(p) + 1, NULL);
#endif _CSECURITY

    if (err)
	exit (err);

    /*
     *  Reload frozen file mainly so that we know the location
     *  of the queue directory for logging purposes.  Also, this
     *  gives a run-through of loading to verify any syntax errors.
     *  Readcf generates no syslogs.  Any errors appear as syserr's
     *  on the console.  The syslogs shown below will appear in 
     *  the log file (provided the config file has defined it, and
     *  provided all necessary directories in the path exist).
     */
    err = do_readcf (conffile);
#ifdef LOG
    /*
     *  Assure queue directory exists.  If not, create it, if possible.
     *  Change to it, if possible.  Return status indicates state.
     */
    (void) mkqdir ();			/* assure q dir exists */

    /*
     *  If change to queue dir didn't work (essentially because it
     *  didn't exist, couldn't be created, or wasn't defined) then
     *  syslog's will automatically appear on the console.
     */
    if (err)
	syslog (LOG_ERR, MSGSTR(MN_ECF2, "Configuration data base \"%sDB\" is invalid"),  conffile); /*MSG*/
    else
	syslog (LOG_NOTICE, MSGSTR(MN_CF, "Configuration data base \"%sDB\" created"),  conffile); /*MSG*/
#endif LOG
    return (err);
}

#ifdef LOCAL_DEBUG
/*
** signal handler for SIGUSR1 in LOCAL_DEBUG mode; just call brkpoint() to
** enter the debugger
*/

void brksig()
{
	brkpoint();
}
#endif LOCAL_DEBUG
