/* @(#)26	1.26  src/bos/usr/bin/uucp/uucp.h, cmduucp, bos411, 9428A410j 3/10/94 11:15:41 */
/* 
 * COMPONENT_NAME: CMDUUCP uucp.h
 * 
 * FUNCTIONS: ASSERT, BASENAME, CDEBUG, DEBUG, DIRECTORY, EQUALS, 
 *            EQUALSN, LASTCHAR, MSGSTR, NOTEMPTY, PREFIX, READANY, 
 *            READSOME, VERBOSE, VERSION, WRITEANY 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*	uucp.h	1.18
*/
/*
#ifndef lint
static	char	h_uucp[] = "uucp.h	1.18";
#endif lint
*/

#define	DFS				/* silly -- for stat.h */

#include "parms.h"
#include "msg.h"

#include <nl_types.h>
#include <locale.h>
#include "uucp_msg.h"
#define MSGSTR(N,S)	catgets(catd, MS_UUCP, N, S)


#ifdef BSD4_2
#define V7
#undef NONAP
#undef FASTTIMER
#endif BSD4_2

#ifdef FASTTIMER
#undef NONAP
#endif

#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include <sys/param.h>

#if defined (ATTSV) && ! defined (CDLIMIT)
#include <sys/fmgr.h>
#endif

/*
 * param.h includes types.h and signal.h in 4bsd
 */
#ifdef V7
#include <sgtty.h>
#include <sys/timeb.h>
#else !V7
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#endif

#include <sys/stat.h>
#include <sys/fullstat.h>
#include <sys/dir.h>
#undef DIRSIZ
#define	DIRSIZ AIX_DIRSIZ

#ifdef BSD4_2
#include <sys/time.h>
#else !BSD4_2
#include <time.h>
#endif

#include <sys/times.h>
#include <errno.h>

#ifdef ATTSV
#include <sys/sysmacros.h>
#endif ATTSV

/*#ifdef	RT 			doesn't need to be here. */
/*
#include "rt/types.h"
#include "rt/unix/param.h"
#include "rt/stat.h"
*/
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ustat.h>
/*#endif RT*/

#include <sys/access.h>

/* what mode should D. files have upon creation? */
#define DFILEMODE 0600

/* what mode should C. files have upon creation? */
#define CFILEMODE 0644

/* define the value of DIRMASK, for umask call */
/* used for creating system subdirectories */
#define DIRMASK 0022
#define UMASK DIRMASK

#define MAXSTART	300	/* how long to wait on startup */

/* define the last characters for ACU  (used for 801/212 dialers) */
#define ACULAST "<"

/*  caution - the following names are also in Makefile 
 *    any changes here have to also be made there
 *
 * it's a good idea to make directories .foo, since this ensures
 * that they'll be ignored by processes that search subdirectories in SPOOL
 *
 *  XQTDIR=/usr/spool/uucp/.Xqtdir
 *  CORRUPT=/usr/spool/uucp/.Corrupt
 *  LOGDIR=/usr/spool/uucp/.Log
 *  SEQDIR=/usr/spool/uucp/.Sequence
 *  STATDIR=/usr/spool/uucp/.Status
 *  
 */

/* where to put the STST. files? */
#define STATDIR		"/usr/spool/uucp/.Status"

/* where should logfiles be kept? */
#define LOGUUX		"/usr/spool/uucp/.Log/uux"
#define LOGUUXQT	"/usr/spool/uucp/.Log/uuxqt"
#define LOGUUCP		"/usr/spool/uucp/.Log/uucp"
#define LOGCICO		"/usr/spool/uucp/.Log/uucico"
#define CORRUPTDIR	"/usr/spool/uucp/.Corrupt"

/* some sites use /usr/lib/uucp/.XQTDIR here */
/* use caution since things are linked into there */
#define XQTDIR		"/usr/spool/uucp/.Xqtdir"

/* how much of a system name can we print in a [CX]. file? */
/* MAXBASENAME - 1 (pre) - 1 ('.') - 1 (grade) - 4 (sequence number) */
#define SYSNSIZE (MAXBASENAME - 7)

#ifdef ETCLOCKS
#define DEVICE_LOCKPRE		"/etc/locks/"
#define LOCKPRE			"/etc/locks/LCK."
#else
#ifdef USRSPOOLLOCKS
#define LOCKPRE		"/usr/spool/locks/LCK."
#else
#define LOCKPRE		"/usr/spool/uucp/LCK."
#endif USRSPOOLLOCKS
#endif ETCLOCKS

#define SQFILE		"/etc/uucp/SQFILE"
#define SQTMP		"/etc/uucp/SQTMP"
#define SLCKTIME	5400	/* system/device timeout (LCK.. files) */
#define SYSFILE		"/etc/uucp/Systems"
#define SYSFILES	"/etc/uucp/Sysfiles"
#define DEVFILE		"/etc/uucp/Devices"
#define DIALERFILE	"/etc/uucp/Dialers"
#define DIALFILE	"/etc/uucp/Dialcodes"
#define PFILE		"/etc/uucp/Permissions"
#define SYSDIR		"/etc/uucp"
#define	DEVCONFIG	"/etc/uucp/Devconfig"


#define SPOOL		"/usr/spool/uucp"
#define SEQDIR		"/usr/spool/uucp/.Sequence"

#define X_LOCKTIME	3600
#ifdef ETCLOCKS
#define SEQLOCK		"/etc/locks/LCK.SQ."
#define SQLOCK		"/etc/locks/LCK.SQ"
#define X_LOCK		"/etc/locks/LCK.X"
#define S_LOCK		"/etc/locks/LCK.S"
#define X_LOCKDIR	"/etc/locks"	/* must be dir part of above */
#else
#ifdef USRSPOOLLOCKS
#define SEQLOCK		"/usr/spool/locks/LCK.SQ."
#define SQLOCK		"/usr/spool/locks/LCK.SQ"
#define X_LOCK		"/usr/spool/locks/LCK.X"
#define S_LOCK		"/usr/spool/locks/LCK.S"
#define X_LOCKDIR	"/usr/spool/locks"	/* must be dir part of above */
#else
#define SEQLOCK		"/usr/spool/uucp/LCK.SQ."
#define SQLOCK		"/usr/spool/uucp/LCK.SQ"
#define X_LOCK		"/usr/spool/uucp/LCK.X"
#define S_LOCK		"/usr/spool/uucp/LCK.S"
#define X_LOCKDIR	"/usr/spool/uucp"	/* must be dir part of above */
#endif USRSPOOLLOCKS
#endif ETCLOCKS
#define X_LOCKPRE	"LCK.X"		/* must be last part of above */

#define PUBDIR		"/usr/spool/uucppublic"
#define ADMIN		"/usr/spool/uucp/.Admin"
#define ERRLOG		"/usr/spool/uucp/.Admin/errors"
#define SYSLOG		"/usr/spool/uucp/.Admin/xferstats"
#define RMTDEBUG	"/usr/spool/uucp/.Admin/audit"
#define CLEANUPLOGFILE	"/usr/spool/uucp/.Admin/uucleanup"

#define	WORKSPACE	"/usr/spool/uucp/.Workspace"

#define SQTIME		60
#define TRYCALLS	2	/* number of tries to dial call */
#define MINULIMIT	(1L<<11)	/* minimum reasonable ulimit */
#define MAX_LOCKTRY	5	/* minimum reasonable ulimit */

/*
 * CDEBUG is for communication line debugging 
 * DEBUG is for program debugging 
 * #define SMALL to compile without the DEBUG code
 *
 * Note: CDEBUG and DEBUG expect f and s to be strings 
 * 	 that have been retreived by or prepared for the Message Facility.
 *	 
 */

#define CDEBUG(l, f, s) if (Debug >= l) fprintf(stderr, f, s)

#define DEBUG(l, f, s) if (Debug >= l) fprintf(stderr, f, s)

/*
 * VERBOSE is used by cu and ct to inform the user
 * about the progress of connection attempts.
 * In uucp, this will be NULL.
 *
 * Note: VERBOSE expects f and s to be strings 
 * 	 that have been retreived by or prepared for the Message Facility.
 */

#ifdef STANDALONE
#define VERBOSE(f, s) if (Verbose > 0) fprintf(stderr, f, s); else
#else
#define VERBOSE(f, s)
#endif

#define PREFIX(pre, str)	(strncmp((pre), (str), strlen(pre)) == SAME)
#define BASENAME(str, c) ((Bnptr = strrchr((str), c)) ? (Bnptr + 1) : (str))
#define EQUALS(a,b)	((a) && (b) && (strcmp((a),(b))==SAME))
#define EQUALSN(a,b,n)	((a) && (b) && (strncmp((a),(b),(n))==SAME))
#define LASTCHAR(s)	(s+strlen(s)-1)

#define SAME 0
#define ANYREAD 04
#define ANYWRITE 02
#define FAIL -1
#define SUCCESS 0
#define NULLCHAR	'\0'
#define CNULL (char *) 0
#define STBNULL (struct sgttyb *) 0
#define MASTER 1
#define SLAVE 0

/* Changing MAXNASENAME and NAMEASIZE is required for initial PDA 
   support. That is updating the sources to use directory access
   routines instead of doing so directly.
   In this first pass of adding PDA support, NAME_MAX will be
   kept to 14.  However, before NAME_MAX is set higher than the 
   current 14, the BNU sources should be reviewed for space efficiency.
   Other defines might also need to be changed.
*/
#ifdef PDA 
#define MAXBASENAME 	NAME_MAX 
#define NAMESIZE 	MAXBASENAME /* includes ending null char */
#else
#define MAXBASENAME 14 /* should be DIRSIZ but 4.2bsd prohibits that */
#define NAMESIZE MAXBASENAME+1
#endif

#define MAXFULLNAME BUFSIZ
#define MAXNAMESIZE	64	/* /usr/spool/uucp/<14 chars>/<14 chars>+slop */
#define MAXMSGTIME 33
#define DEFMAXEXPECTTIME 45
#define MAXCHARTIME 15
#define	SIZEOFPID	10		/* maximum number of digits in a pid */
#define EOTMSG "\004\n\004\n"
#define CALLBACK 1

/* manifests for sysfiles.c's sysaccess()       */
/* check file access for REAL user id */
#define ACCESS_SYSTEMS  1
#define ACCESS_DEVICES  2
#define ACCESS_DIALERS  3
/* check file access for EFFECTIVE user id */
#define EACCESS_SYSTEMS 4
#define EACCESS_DEVICES 5
#define EACCESS_DIALERS 6


/* manifest for chkpth flag */
#define CK_READ		0
#define CK_WRITE	1

/*
 * commands
 */
#define SHELL		"/bin/bsh"
#define MAIL		"mail"
#define UUCICO		"/usr/sbin/uucp/uucico"
#define UUCICO_ALT	"/usr/lib/uucp/uucico"
#define UUXQT		"/usr/sbin/uucp/uuxqt"
#define UUCP		"uucp"


/* system status stuff */
#define SS_OK			0
#define SS_NO_DEVICE		1
#define SS_TIME_WRONG		2
#define SS_INPROGRESS		3
#define SS_CONVERSATION		4
#define SS_SEQBAD		5
#define SS_LOGIN_FAILED		6
#define SS_DIAL_FAILED		7
#define SS_BAD_LOG_MCH		8
#define SS_LOCKED_DEVICE	9
#define SS_ASSERT_ERROR		10
#define SS_BADSYSTEM		11
#define SS_CANT_ACCESS_DEVICE	12
#define SS_DEVICE_FAILED	13	/* No longer used */
#define SS_WRONG_MCH		14
#define SS_CALLBACK		15
#define SS_RLOCKED		16
#define SS_RUNKNOWN		17
#define SS_RLOGIN		18
#define SS_UNKNOWN_RESPONSE	19
#define SS_STARTUP		20
#define SS_CHAT_FAILED		21
#define SS_NOSUCH_HOST		22

#define MAXPH	60	/* maximum phone string size */
#define	MAXC	BUFSIZ

#define	TRUE	1
#define	FALSE	0
#define NAMEBUF	32

/* structure of an Systems file line */
#define F_MAX	50	/* max number of fields in Systems file line */
#define F_NAME 0
#define F_TIME 1
#define F_TYPE 2
#define F_CLASS 3	/* an optional prefix and the speed */
#define F_PHONE 4
#define F_LOGIN 5

/* structure of an Devices file line */
#define D_TYPE 0
#define D_LINE 1
#define D_CALLDEV 2
#define D_CLASS 3
#define D_CALLER 4
#define D_ARG 5
#define D_MAX	50	/* max number of fields in Devices file line */

#define D_ACU 1
#define D_DIRECT 2
#define D_PROT 4

/* past here, local changes are not recommended */
#define CMDPRE		'C'
#define DATAPRE		'D'
#define XQTPRE		'X'

/*
 * stuff for command execution
 */
#define X_RQDFILE	'F'
#define X_STDIN		'I'
#define X_STDOUT	'O'
#define X_CMD		'C'
#define X_USER		'U'
#define X_BRINGBACK	'B'
#define X_MAILF		'M'
#define X_RETADDR	'R'
#define X_COMMENT	'#'
#define X_NONZERO	'Z'
#define X_SENDNOTHING	'N'
#define X_SENDZERO	'n'


/* This structure describes call routines */
struct caller {
	char	*CA_type;
	int	(*CA_caller)();
};

/* This structure describes dialing routines */
struct dialer {
	char	*DI_type;
	int	(*DI_dialer)();
};

struct nstat {
	int	t_pid;		/* process id				*/
	long	t_start;	/* process id				*/
	time_t	t_beg;		/* start  time				*/
	time_t	t_scall;	/* start call to system			*/
	time_t	t_ecall;	/* end call to system			*/
	time_t	t_tacu;		/* acu time				*/
	time_t	t_tlog;		/* login time				*/
	time_t	t_sftp;		/* start file transfer protocol		*/
	time_t	t_sxf;		/* start xfer 				*/
	time_t	t_exf;		/* end xfer 				*/
	time_t	t_eftp;		/* end file transfer protocol		*/
	time_t	t_qtime;	/* time file queued			*/
	int	t_ndial;	/* # of dials				*/
	int	t_nlogs;	/* # of login trys			*/
	struct tms t_tbb;	/* start execution times		*/
	struct tms t_txfs;	/* xfer start times			*/
	struct tms t_txfe;	/* xfer end times 			*/
	struct tms t_tga;	/* garbage execution times		*/
};

/* external declarations */

extern int (*Read)(), (*Write)(), (*Ioctl)();
extern char *LineType;
extern int Ifn, Ofn;
extern int Debug, Verbose;
extern ushort Dev_mode;		/* save device mode here */
extern int Bspeed;
extern int Uid, Euid;		/* user-id and effective-uid */
extern int Ulimit;
extern char Wrkdir[];
extern long Retrytime;
extern char **Env;
extern char Uucp[];
extern char Pchar;
extern struct nstat Nstat;
extern char Dc[];			/* line name			*/
extern char Fwdname[];		/* foward name			*/
extern int Seqn;			/* sequence #			*/
extern int Role;
extern char Logfile[];
extern int linebaudrate;	/* adjust sleep time on read in pk driver */
extern char Rmtname[];
extern char User[];
extern char Loginuser[];
extern char *Thisdir;
extern char *Spool;
extern char *Pubdir;
extern char Myname[];
extern char Progname[];
extern char RemSpool[];
extern char *Bnptr;		/* used when BASENAME macro is expanded */

extern char Jobid[];		/* Jobid of current C. file */
extern int Uerror;		/* global error code */
extern char *UerrorText();	/* text function for error code */

/*	Some global I need for section 2 and section 3 routines */
extern errno;
extern char *optarg;	/* for getopt() */
extern int optind;	/* for getopt() */

/* things get kind of gross beyond this point -- please stay out */

#ifdef ATTSV
#define index strchr
#define rindex strrchr 
#define vfork fork
#define ATTSVKILL
#define UNAME
#else
#define strchr index
#define strrchr rindex
#endif

#ifdef lint
#define VERSION(x)	;
#define ASSERT(e, s1, s2, i1)	;

#else NOT lint

#define VERSION(x)	static	char	sccsid[] = "x";
#define ASSERT(e, s1, s2, i1) if (!(e)) {\
	assert(s1, s2, i1, sccsid, __FILE__, __LINE__);\
	cleanup(FAIL);};
#endif

extern struct stat __s_;
#define READANY(f)	((stat((f),&__s_)==0) && ((__s_.st_mode&(0004))!=0) )
#define READSOME(f)	((stat((f),&__s_)==0) && ((__s_.st_mode&(0444))!=0) )

#define WRITEANY(f)	((stat((f),&__s_)==0) && ((__s_.st_mode&(0002))!=0) )
#define DIRECTORY(f)	((stat((f),&__s_)==0) && ((__s_.st_mode&(S_IFMT))==S_IFDIR) )
#define NOTEMPTY(f)	((stat((f),&__s_)==0) && (__s_.st_size!=0) )

/*
**	This was commented out because of problems created when 
**	readdir was attempting to handle files that really weren't
**	files ...
*/

/*
#ifndef BSD4_2
#define DIR FILE
#define opendir(x) fopen((x), "r")
#define closedir(x) fclose((x))
#endif BSD4_2
*/

/* standard functions used */

extern char	*strcat(), *strcpy(), *strncpy(), *strrchr();
extern char	*strchr(), *strpbrk();
extern char	*index(), *rindex(), *getlogin(), *ttyname();
extern clock_t	lseek(), atol();
extern int	strcmp(), strncmp();
extern unsigned long strlen();
extern int	fork(), pipe(), close(), getopt();

/*
#ifdef BSD4_2
extern char *sprintf();
#else
extern int sprintf();
#endif BSD4_2
*/

/* uucp functions and subroutine */
extern          (*genbrk)();
extern int	anlwrk(), iswrk(), gtwvec();		/* anlwrk.c */
extern void	chremdir(), mkremdir();			/* chremdir.c */
extern		void toCorrupt();			/* cpmv.c  */
extern		int xcp(), xmv();			/* cpmv.c  */

extern int	get_args();				/* getargs.c */
extern void	bsfix();				/* getargs.c */
extern char	*getprm();				/* getprm.c */

extern void	logent(), uusyslog(), close_log();	/* logent.c */
extern time_t	millitick();				/* logent.c */

extern char	*protoString();				/* permission.c */
extern		logFind(), mchFind();			/* permission.c */
extern		chkperm(), chkpth();			/* permission.c */
extern		cmdOK(), switchRole();			/* permission.c */
extern		callBack(), requestOK();		/* permission.c */
extern void	myName();				/* permission.c */

extern void	systat();				/* systat.c */
extern int	ttylocked(), clrlock();			/* ttylock.c */
extern int	ttylock(), ttyunlock();			/* ttylock.c */
extern int	ttywait(), ttytouchlock();		/* ttylock.c */
extern int	ulockf(), checkLock(), delock();	/* ulockf.c */
extern void	rmlock(), ultouch();			/* ulockf.c */
extern char	*timeStamp();				/* utility.c */
extern void	assert(), errent();			/* utility.c */
extern void	uucpname();				/* uucpname.c */
extern int	versys();				/* versys.c */
extern void	xuuxqt(), xuucico();			/* xqt.c */

#ifdef ATTSV
unsigned	sleep();
void	exit();
long	ulimit();
#else !ATTSV
int	sleep(), exit(), setbuf(), ftime();
#endif

#ifndef NOUSTAT
#ifdef V7USTAT
struct  ustat {
	daddr_t	f_tfree;	/* total free */
	ino_t	f_tinode;	/* total inodes free */
};
#else !NOUSTAT && !V7USTAT
#include <ustat.h>
#endif V7USTAT
#endif NOUSTAT

#ifdef UNAME
#include <sys/utsname.h>
#endif UNAME

#if defined(BSD4_2)
char *gethostname();
#endif

/* messages */

/*
**	These were commented out because they were causing problems
**	with the NLS Macro expansions.
*/

/*
extern char *Ct_OPEN;
extern char *Ct_WRITE;
extern char *Ct_READ;
extern char *Ct_CREATE;
extern char *Ct_ALLOCATE;
extern char *Ct_LOCK;
extern char *Ct_STAT;
extern char *Ct_CHOWN;
extern char *Ct_CHMOD;
extern char *Ct_LINK;
extern char *Ct_CHDIR;
extern char *Ct_UNLINK;
extern char *Wr_ROLE;
extern char *Ct_CORRUPT;
extern char *Ct_FORK;
extern char *Ct_CLOSE;
extern char *Fl_EXISTS;
extern char *Ue_BADSYSTEM;
extern char *Ue_TIME_WRONG;
extern char *Ue_SYSTEM_LOCKED;
extern char *Ue_NO_DEVICE;
extern char *Ue_DIAL_FAILED;
extern char *Ue_LOGIN_FAILED;
extern char *Ue_SEQBAD;
extern char *Ue_BAD_LOG_MCH;
extern char *Ue_WRONG_MCH;
extern char *Ue_LOCKED_DEVICE;
extern char *Ue_ASSERT_ERROR;
extern char *Ue_CANT_ACCESS_DEVICE;
extern char *Ue_DEVICE_FAILED;
*/

/*
 * omask is a global that should be initalized to umask() if you want to
 * change umask; immediately afterward, do an atexit(oldmask) to restore
 * the original umask at exit() time
 */
/*
mode_t omask = -1;
*/
extern void oldmask();

