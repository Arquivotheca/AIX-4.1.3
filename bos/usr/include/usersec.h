/* @(#)65	1.20.1.11  src/bos/usr/include/usersec.h, cmdsauth, bos41J, 9515B_all 4/13/95 08:46:00 */
/*
 * COMPONENT_NAME: CMDSAUTH
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#ifndef _H_USERSEC    
#define _H_USERSEC

#include <sys/limits.h>
#include <sys/types.h>
#include <stdio.h>

#ifdef  _NO_PROTO
	/* set and get user credentials */
	extern  int	setpcred ();
	extern  char 	**getpcred ();

	/* set and get process environment */
	extern  int	setpenv ();
	extern  char	**getpenv ();

	/* user database */
	extern	int	setuserdb ();
	extern	int	enduserdb ();

	/* set and  get user attributes */
	extern  int	setuserattr ();
	extern	int	getuserattr ();
	extern	int	putuserattr ();
	extern	int	enduserattr ();
	extern  char	*IDtouser ();

	/* set and  get group attributes */
	extern  int	setgroupattr ();
	extern	int	getgroupattr ();
	extern	int	putgroupattr ();
	extern  int	endgroupattr ();
	extern  char	*IDtogroup ();

	extern	int	getconfattr ();
	extern 	char	*newpass();

	/* user authentication */
	extern	int	ckuseracct();

	/* next user and group name */
	extern	char	*nextuser();
	extern	char	*nextgroup();
#else	/* _NO_PROTO */
	/* set and get user credentials */
	extern  int	setpcred (char *, char **);
	extern  char 	**getpcred (int);

	/* set and get process environment */
	extern  int	setpenv (char *, int, char **, char *);
	extern  char	**getpenv (int);

	/* user database */
	extern	int	setuserdb (int);
	extern	int	enduserdb (void);

	/* set and  get user attributes */
	extern  int	setuserattr (int);
	extern	int	getuserattr (char *, char *, void *, int);
	extern	int	putuserattr (char *, char *, void *, int);
	extern	int	enduserattr ();
	extern  char	*IDtouser (uid_t);

	/* set and  get group attributes */
	extern  int    setgroupattr (int);
	extern	int    getgroupattr(char *, char *, void *, int);
	extern	int    putgroupattr(char *, char *, void *, int);
	extern	int    endgroupattr ();
	extern  char   *IDtogroup(uid_t);

	extern	int getconfattr(char *, char *, void *, int);
	extern 	char	*newpass    (struct userpw *);

	/* user authentication */
	extern	int	ckuseracct (char *, int, char *);

	/* next user and group name */
	extern	char	*nextuser (int, int);
	extern	char	*nextgroup (int, int);
#endif	/* _NO_PROTO */

/* modes for set attribute routines */
#define S_READ		0x1
#define S_WRITE		0x2

/* attribute types (these are indexes into a table in secattr) */
#define	SEC_CHAR	1
#define SEC_INT		2
#define SEC_LIST	3
#define SEC_BOOL	4

/* delete the entry */
#define SEC_DELETE	5

/* Commit the changes to hard files */
#define SEC_COMMIT	6

/* create a new entry */
#define SEC_NEW		7

/* Type value of attribute that is not implemented */
#define NOT_IMPLEM	-1	/* Not implemented */

/*
 * The user attributes names 
 */
#define	S_ID		"id"		/* SEC_INT - user, group id */
#define	S_PWD		"password"	/* SEC_PWD - user, group id */
#define	S_PGRP		"pgrp"		/* SEC_CHAR - primary group id */
#define S_GROUPS	"groups"	/* SEC_LIST - concurrent group list */
#define S_GROUPSIDS	"groupsids"	/* SEC_LIST - concurrent group list by id */
#define S_ADMGROUPS	"admgroups"	/* SEC_LIST - groups for 
					which this user is an administrator */
#define S_PUSERS	"primary"	/* SEC_LIST - primary users of group */
#define	S_USERS		"users"		/* SEC_LIST - the members of a group */
#define S_ADMIN		"admin"		/* SEC_BOOL - administrative group */
#define S_ADMS		"adms"		/* SEC_LIST - group administrators */
#define S_PACCT		"pacct"		/* NOT_IMPLEM - primary account */
#define S_ACCTS		"accts"		/* NOT_IMPLEM - this users accounts */
#define S_ADMACCTS	"admacct"	/* NOT_IMPLEM - accounts for which 
					this user is an administrator */
#define S_AUDITCLASSES	"auditclasses"	/* SEC_LIST - the users audit classes */
#define S_HOME		"home"		/* SEC_CHAR - home directory */
#define S_SHELL		"shell"		/* SEC_CHAR - the users login shell */
#define S_GECOS		"gecos"		/* SEC_CHAR - user information */
#define S_SYSENV	"sysenv"	/* SEC_LIST - protected environment */
#define S_USRENV	"usrenv"	/* SEC_LIST - public environment */
#define S_LOGINCHK	"login"		/* SEC_BOOL - login permitted or not */
#define S_SUCHK		"su"		/* SEC_BOOL - su permitted or not */
#define S_DAEMONCHK	"daemon"	/* SEC_BOOL - cron or src permitted */
#define S_RLOGINCHK	"rlogin"	/* SEC_BOOL - rlogin or telnet allowed*/
#define S_TELNETCHK	"telnet"	/* SEC_BOOL - rlogin or telnet allowed*/
#define S_ADMCHK	"admchk"	/* SEC_BOOL - force passwd renewal */
#define S_TPATH		"tpath"		/* SEC_CHAR - can be 
					: "nosak", "always", "notsh", or "on" */
#define S_TTYS		"ttys"		/* SEC_LIST - allowed login ttys */
#define S_SUGROUPS	"sugroups"	/* SEC_LIST - groups that can 
						su to this account */
#define S_EXPIRATION	"expires"	/* SEC_CHAR - account expiration */
#define S_AUTH1		"auth1"		/* SEC_CHAR - primary authentication */
#define S_AUTH2		"auth2"		/* SEC_CHAR - secondary authentication*/
#define	S_UFSIZE 	"fsize"		/* SEC_INT  - file size */
#define S_ULIMIT	"limits"	/* SEC_INT  - ulimit */
#define	S_UCPU   	"cpu"		/* SEC_INT  - cpu usage limit */
#define	S_UDATA 	"data"		/* SEC_INT  - data memory limit */
#define	S_USTACK 	"stack"		/* SEC_INT  - stack memory limit */
#define	S_UCORE 	"core"		/* SEC_INT  - core memory limit */
#define	S_URSS		"rss"		/* SEC_INT  - rss memory limit */
#define	S_UMASK		"umask"		/* SEC_INT  - file creation mask */
#define S_AUTHSYSTEM	"SYSTEM"	/* SEC_CHAR - authentication grammar */
#define S_REGISTRY	"registry"	/* SEC_CHAR - administration domain */
#define S_LOGTIMES	"logintimes"	/* SEC_LIST - valid login times */
#define S_LOCKED	"account_locked"/* SEC_BOOL - is the account locked */
#define S_LOGRETRIES	"loginretries"	/* SEC_INT  - invalid login attempts
					before the account is locked */
#define	S_MINALPHA	"minalpha"	/* SEC_INT - passwd minalpha   */
#define	S_MINOTHER	"minother"	/* SEC_INT - passwd minother   */
#define	S_MINDIFF	"mindiff"	/* SEC_INT - passwd mindiff    */
#define	S_MAXREPEAT	"maxrepeats"	/* SEC_INT - passwd maxrepeats */
#define	S_MINLEN	"minlen"	/* SEC_INT - passwd minlen     */
#define	S_MINAGE	"minage"	/* SEC_INT - passwd minage     */
#define	S_MAXAGE	"maxage"	/* SEC_INT - passwd maxage     */
#define	S_MAXEXPIRED	"maxexpired"	/* SEC_INT - passwd maxexpired */
#define S_HISTEXPIRE    "histexpire"    /* SEC_INT - passwd reuse interval  */
#define S_HISTSIZE      "histsize"      /* SEC_INT - passwd reuse list size */
#define	S_PWDCHECKS	"pwdchecks"	/* SEC_LIST - passwd pwdchecks   */
#define	S_DICTION	"dictionlist"	/* SEC_LIST - passwd dictionlist */
#define	S_PWDWARNTIME	"pwdwarntime"	/* SEC_INT - passwd pwdwarntime */

#define	S_USREXPORT	"dce_export"	/* SEC_BOOL - passwd export protection */
#define	S_GRPEXPORT	"dce_export"	/* SEC_BOOL - group export protection  */

/* attributes set by tsm */
#define	S_LASTTIME	"time_last_login"	
			/* SEC_INT  - time of last successful login */
#define	S_ULASTTIME	"time_last_unsuccessful_login"	
			/* SEC_INT  - time of last unsuccessful login */
#define	S_LASTTTY	"tty_last_login"	
			/* SEC_CHAR  - tty of last successful login */
#define	S_ULASTTTY	"tty_last_unsuccessful_login"	
			/* SEC_CHAR  - tty of last unsuccessful login */
#define	S_LASTHOST	"host_last_login"	
			/* SEC_CHAR  - host name of last successful login */
#define	S_ULASTHOST	"host_last_unsuccessful_login"	
			/* SEC_CHAR  - host name of last unsuccessful login */
#define	S_ULOGCNT	"unsuccessful_login_count"	
			/* SEC_INT  - number of unsuccessful logins */

/*
 * The port attribute names
 */
#define S_HERALD	"herald"	/* SEC_CHAR - login herald */
#define S_SAKENABLED	"sak_enabled"	/* SEC_BOOL - sak enabled or not */
#define S_SYNONYM	"synonym"	/* SEC_LIST - synonym ports */
#define S_LOGDISABLE	"logindisable"	/* SEC_INT - invalid login attempts
					before the port is disabled */
#define S_LOGINTERVAL	"logininterval"	/* SEC_INT - time period for port
					disabling */
#define S_LOGREENABLE	"loginreenable"	/* SEC_INT - time period after which
					the port is reenabled */
#define S_LOGDELAY	"logindelay"	/* SEC_INT - delay between invalid
					login attempts */
#define S_LOCKTIME	"locktime"	/* SEC_INT - time the port was locked */
#define S_ULOGTIMES	"unsuccessful_login_times"
					/* SEC_LIST - times when invalid login
					attempts occurred */

/*
 * System attribute names
 */
#define S_LOGTIMEOUT	"logintimeout"	/* SEC_INT - time given to enter a
					password */

/*
 * for process ENVIRONMENT manipulation: setpenv() and getpenv()
 */
/* setpenv() */
#define	PENV_INIT	0x1		/* initialize (used by tsm) */
#define	PENV_DELTA	0x2		/* modify by adding to process env */
#define	PENV_RESET	0x4		/* reset process environment */
#define PENV_ARGV	0x8		/* command will be in argv format */
#define PENV_KLEEN	0x10		/* indicates running kleenup() */
#define PENV_NOPROF	0x20		/* execute command without profiles */
#define PENV_NOEXEC	0x40		/* initialize environment only */

/* getpenv() */
#define PENV_SYS	0x1		/* indicates environment wanted */
#define PENV_USR	0x2		/* indicates environment wanted */

/* for setpenv() */
#define PENV_USRSTR	"USRENVIRON:"	/* start of environ from environ area */
#define PENV_SYSSTR	"SYSENVIRON:"	/* start of environ from usrinfo area */

/* defines for nextuser() and nextgroup()	*/
#define S_LOCAL		0x01		/* bit mask for 'local' database  */
#define	S_SYSTEM	0x02		/* bit mask for 'system' database */

/* bits for getpcred argument */
#define CRED_RUID	0x2    	/* user's real id */
#define CRED_LUID	0x4 	/* user's login id */
#define CRED_RGID	0x8	/* group id */
#define CRED_ACCT	0x10	/* account id */
#define CRED_AUDIT	0x20	/* audit classes */
#define CRED_RLIMITS	0x40	/* ulimit credentials */
#define CRED_UMASK    	0x80	/* umask credentials */
#define CRED_GROUPS	0x100	/* ids of all groups */

/* misc */
#define S_NAMELEN 	L_cuserid	/* for compatablity */
#define S_NGROUPS	32		/* max # of concurrent groups set */

/* attribute names */
#define SEC_PASSWD	"password"
#define SEC_LASTUP	"lastupdate"
#define SEC_FLAGS	"flags"

/********************************/
/* table types for data base	*/
/********************************/

#define USER_TABLE	0
#define GROUP_TABLE	1
#define PASSWD_TABLE	2
#define	SYSCK_TABLE	3
#define PORT_TABLE	4
#define GENERIC_TABLE	5

/**************************************/
/* Defines for authentication grammar */
/**************************************/
#define AUTH_ENV	"AUTHSTATE"	/* Environment variable name         */
#define AUTH_COMPAT	"compat"	/* local files plus NIS              */
#define AUTH_DCE	"DCE"		/* Distributed Computing Environment */
#define AUTH_FILES	"files"		/* local files only		     */
#define AUTH_NONE_SEC	"NONE"		/* No authentication required	     */
#define AUTH_AND	"AND"		/* Boolean "and" within grammar	     */
#define AUTH_OR		"OR"		/* Boolean "or" within grammar       */
#define AUTH_DEFPATH	"/usr/lib/security/"	/* Default method path	     */
#define AUTH_NIS	"NIS"		/* Not used in grammar		     */

#define AUTH_SUCCESS	0
#define AUTH_FAILURE	1
#define AUTH_UNAVAIL	2
#define AUTH_NOTFOUND	3
#define AUTH_WILDCARD   4

#define ISLOCAL(str)    !strcmp(str, AUTH_FILES)
#define ISCOMPAT(str)   !strcmp(str, AUTH_COMPAT)
#define ISNIS(str)	!strcmp(str, AUTH_NIS)
#define ISNONE(str)	!strcmp(str, AUTH_NONE_SEC)
#define ISDCE(str)	!strcmp(str, AUTH_DCE)

/********************************/
/*    Method function pointers  */
/********************************/
struct secmethod_table {
	int (*method_authenticate)();
	int (*method_chpass)();
	int (*method_getpwnam)();
	int (*method_getpwuid)();
	int (*method_getgrgid)();
	int (*method_getgrnam)();
	int (*method_getgrset)();
	int (*method_passwdexpired)();
	int (*method_passwdrestrictions)();
	int *reserved[20];
};

/* Parse tree for authentication grammar */
struct secgrammar_tree
{
	enum {NODE, LEAF} type;         /* Interior branch or leaf      */
	char *name;                     /* Method name                  */
	int  result;                    /* Expected result from method  */
#ifndef __cplusplus
	char *operator;                 /* Binary operator "AND"/"OR"   */
#else
	char *binary_operator;		/* different name if C++ 	*/
#endif
	struct secgrammar_tree *left;   /* Left branch of tree          */
	struct secgrammar_tree *right;  /* Right branch of tree         */
};

/* defines for pwdbm_update() */
#define PWDBM_PASSWD 1
#define PWDBM_SHELL  4
#define PWDBM_GECOS  6

/********************************/
/*	AF routines defines	*/
/********************************/

/* structures for lowest level routine for attribute file manipulation */
struct ATTR
{       char *  AT_name;
	char *  AT_value;
};

typedef struct ATTR * ATTR_t;

struct AFILE
{       FILE *  AF_iop;
	int     AF_rsiz;
	int     AF_natr;
	char *  AF_cbuf;
	char *  AF_dbuf;
	ATTR_t  AF_catr;
	ATTR_t  AF_datr;
};

typedef struct AFILE * AFILE_t;

/* security library routines */
#ifdef  _NO_PROTO

extern AFILE_t 	afopen();
extern int	afclose();
extern int	afrewind();
extern ATTR_t 	afnxtrec();
extern ATTR_t 	afgetrec();
extern ATTR_t 	affndrec();
extern char * 	afgetatr();

#else /* _NO_PROTO */

extern AFILE_t 	afopen(char *);
extern int	afclose(AFILE_t);
extern int	afrewind(AFILE_t);
extern ATTR_t 	afnxtrec(AFILE_t);
extern ATTR_t 	afgetrec(AFILE_t, char *);
extern ATTR_t 	affndrec();
extern char * 	afgetatr(ATTR_t, char *);

#endif /* _NO_PROTO */

#endif /* _H_USERSEC */
