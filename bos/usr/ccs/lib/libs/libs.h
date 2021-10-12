/* @(#)60	1.16.1.15  src/bos/usr/ccs/lib/libs/libs.h, libs, bos41J, 9517A_all 4/24/95 11:47:09 */
/*
 *   COMPONENT_NAME: LIBS
 *
 *   FUNCTIONS: COPYFIELD
 *		CPYATTR
 *		CPYCOMMENT
 *		CPYNAME
 *		CPYSPACE
 *		MSGC
 *		MSGSTR
 *		NEXTLINE
 *		SKIPeol
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <ctype.h>
#include <usersec.h>

/****************************************/
/* message catalog stuff		*/
/****************************************/

#include "libs_msg.h"
#define	MSGSTR(Num, Str) getlibsmsg (Num, Str)
#define  MSGC(Num) Num

/* default newpass() messages */
#define	DEF_MINALPHA	"\tminimum of %d alphabetic character\n" 
#define	DEF_MINALPHAS	"\tminimum of %d alphabetic characters\n" 
#define	DEF_MINOTHER	"\tminimum of %d non-alphabetic character\n" 
#define	DEF_MINOTHERS	"\tminimum of %d non-alphabetic characters\n" 
#define	DEF_MINDIFF	"\tminimum of %d character not in old password\n" 
#define	DEF_MINDIFFS	"\tminimum of %d characters not in old password\n" 
#define	DEF_MAXREPEAT 	"\tmaximum of %d repeated character\n" 
#define	DEF_MAXREPEATS	"\tmaximum of %d repeated characters\n" 
#define	DEF_MINLENGTH	"\tminimum of %d character in length\n" 
#define	DEF_MINLENGTHS	"\tminimum of %d characters in length\n" 
#define	DEF_MINAGE	"New password requires a minimum of %d elapsed week between changes.\n"
#define	DEF_MINAGES	"New password requires a minimum of %d elapsed weeks between changes.\n"
#define	DEF_NOMATCH	"They do not match, try again.\n" 
#define DEF_INCHAR 	"\nInvalid character in password: '%s'\n"
#define DEF_ATTREQ	"\nYour new password must have:\n"
#define	DEF_ATTRFAIL	"\nYour password failed to meet:\n"
#define	DEF_MATCH	"password mismatch.\n" 

#define DEF_ENTPROMPT 	"Re-enter %s's new password:"
#define DEF_NEWPROMPT 	"New password:"
#define DEF_OLDPROMPT 	"Old password:"
#define DEF_PROMPT 	"Password:"
#define DEF_PWNEW	"%s's New password:"
#define DEF_PWEITHER	"Enter %s's password or %s's Old password:"
#define DEF_PWOLD	"%s's Old password:"
#define DEF_PWPROMPT	"%s's Password:"
#define DEF_SETPASS	"Setting \"%s's\" password to NULL.\n"


/* default ckuserID() messages */
#define DEF_PASREQ	"Passwords are required. Please choose a password.\n"
#define DEF_PASEXP	"Your password has expired. Please choose a new password.\n"
#define DEF_PASADM	"You are required to change your password. Please choose a new one.\n"

/* default putuattr() messages */
#define DEF_OPEN	"Error opening file \"%s\" for write"
#define DEF_LOCK	"Error setting lock on file \"%s\".\nPlease try again later.\n"
#define DEF_WRITE	"Error writing to \"%s\""

/* default setpenv() message */
#define DEF_CHDIR	"Unable to change directory to \"%s\".\n\tYou are in \"%s\" instead.\n"

/* default authenticate() messages */
#define DEF_USRNONEX   "User \"%s\" does not exist.\n"
#define DEF_GRAMMARERR "An authentication error occurred on the grammar \"%s\".\n"
#define DEF_FAILLOCAL "User \"%s\" failed local authentication.\n"
#define DEF_FAILLOAD  "Security method \"%s\" could not be loaded.\n"
#define DEF_NOAUTHPTR "Security method \"%s\" has no authentication function.\n"
#define DEF_NOPASSPTR "Security method \"%s\" has no password change function.\n"
#define DEF_PUTERR     "An error occurred updating the password database.\n"
#define DEF_NOAUTH     "You are not authorized to change \"%s's\" password.\n"

#define DEF_NOPASSREUSE "Password was recently used and is not valid for reuse.\n"
#define	DEF_BADUSER "Error obtaining the user's password information.\n"

#define	DEF_PWATTRIBUTE	"Error retrieving the \"%s\" attribute.\n"
#define	DEF_ADMINONLY	"Only the system administrator can change this password.\n"
#define	DEF_SEEADMIN	"Please see the system administrator to change your password.\n"
#define	DEF_BADDATA	"\"%s\" defined by your \"%s\" password\n\
attribute could not be executed.  Please see the system administrator.\n"
#define	DEF_CHKFAIL	"Password composition failure from \"%s=%s\".\n"
#define	DEF_CHKADMFAIL	"Password administration failure from \"%s=%s\".\n"
#define	DEF_CHKERR	"Password internal error from \"%s=%s\".\n"
#define	DEF_BADSTATE	\
"Your password has expired, but your password has not reached\n\
its minimum age requirement.\n"
#define	DEF_ADMINEXP	"Your password has been expired for too long.\n"
#define	DEF_EXPWARN	"Your password will expire: %s\n"
#define	DEF_BADCRY	"Your encrypted password is invalid. Please choose a new password.\n"
#define	DEF_BADCRYPT	"Your encrypted password is invalid.\n"
#define	DEF_PASREQED	"Passwords are required.\n"
#define	DEF_PASEXPED	"Your password has expired.\n"
#define	DEF_PASADMIN	"A password change is required.\n"
#define	DEF_DICTERR	"Error accessing password dictionary \"%s\".\n"
#define	DEF_DICTMATCH	"Passwords must not match words in the dictionary.\n"
#define	DEF_PWRFAIL	"Your password does not meet the password requirements.\n\
\tPlease try again.\n"
#define	DEF_PWRADMFAIL	"A password change is not allowed at this time.\n"
#define	DEF_PWRERR	"Security method \"%s\" detected an internal error.\n"
#define DEF_PUTHISTERR "The password database was successfully updated, but an \n\
\terror occurred while updating the password history database.\n"
#define DEF_HISTATTR   "Error retrieving the password history attributes.\n"
#define DEF_HISTDBOPEN "Error opening the password history database.\n"

/* default loginrestrictions() messages */
#define DEF_BADLOGIN	"You entered an invalid login name or password.\n"
#define DEF_USERLOCKED	"Your account has been locked; please see the system administrator.\n"
#define DEF_EXPIRED	"Your account has expired; please see the system administrator.\n"
#define DEF_TOOMANYBAD	"There have been too many unsuccessful login attempts; please see\n\tthe system administrator.\n"
#define DEF_USERTIME	"You are not allowed to login at this time.\n"
#define DEF_NOLOCAL	"Local logins are not allowed for this account.\n"
#define DEF_NOREMOTE	"Remote logins are not allowed for this account.\n"
#define DEF_NOSU	"You are not allowed to su to this account.\n"
#define DEF_NODAEMON	"You are not allowed to use SRC or cron.\n"
#define DEF_NOTTYS	"You are not allowed to access the system via this terminal.\n"
#define DEF_TTYLOCKED	"This terminal has been locked; please see the system administrator.\n"
#define DEF_TTYTIME	"This terminal can not be used at this time.\n"
#define DEF_LICENSE	"All available login sessions are in use.\n"
#define DEF_BADLOGTIMES	"User %s has an invalid logintimes value of \"%s\".\n"

/* default loginsuccess() messages */
#define DEF_SLOGIN	"Last login: %s on %s\n"
#define DEF_SLOGINHT	"Last login: %s on %s from %s\n"
#define DEF_FLOGIN	"Last unsuccessful login: %s on %s\n"
#define DEF_FLOGINHT	"Last unsuccessful login: %s on %s from %s\n"
#define DEF_FCOUNT	"1 unsuccessful login attempt since last login.\n"
#define DEF_FCOUNTS	"%d unsuccessful login attempts since last login.\n"

/****************************************/
/* function types used in libraries	*/
/****************************************/

extern		int	errno;
extern		void	*malloc();
extern		void	*realloc();
extern		char	*acl_fget();

/**********************************/
/* values for (struct atval).flgs */
/**********************************/

#define	CACHED		0x1	/* indicates attribute value is in memory */
#define DYNAMIC 	0x2	/* attr value in dynamically allocated memory */
#define NOT_FOUND	0x4	/* indicates attribute value was not found */
#define QUOTES		0x8	/* indicates attribute needs quotes */
#define UPDATED		0x10	/* indicates attribute was updated */
#define NUKED		0x20	/* indicates attribute was deleted */
#define LOCKED		0x40	/* indicates file is locked */
#define COMMIT		0x80	/* indicates file is to be written out */
#define	NOT_VALID	0x100	/* indicates attribute not a valid number */
#define REUSED		0x200	/* wild card attribute entry has been re-used */
#define FAILTRUE        0x400   /* invalid boolean attribute defaults to true */
#define FAILBADVAL      0x800   /* invalid bool attr returns out of range val */

/* indexes into the passwd attribute table below */
#define IDX_PASSWD	1
#define IDX_LASTUP	2
#define IDX_FLAGS	3
#define IDX_PLAST	4

/* indexes into passwd file table */
#define	S_SECPWD	0	/* base password file */

/* indexes into group attribute table */
#define IDX_ID 		1
#define IDX_PUSERS 	2
#define IDX_USERS 	3
#define IDX_ADMS 	4
#define IDX_GADMIN	5
#define IDX_GRPEXPORT	6
#define IDX_GLAST	7

/* indexes into group file table */
#define	S_BASEGROUP	0	/* base group file */
#define S_SECGROUP	1	/* security group file */
#define S_GBASEPWD	2	/* primary user's are in the /etc/passwd file */

/* indexes into user attribute table */
#define IDX_ID 		1
#define IDX_PGRP 	2
#define IDX_GROUPS 	3
#define IDX_ADMGROUPS 	4
#define IDX_PACCT 	5
#define IDX_ACCTS 	6
#define IDX_ADMACCTS 	7
#define IDX_ULIMIT 	8
#define IDX_AUDIT 	9
#define IDX_HOME 	10
#define IDX_SHELL 	11
#define IDX_GECOS 	12
#define IDX_SYSENV	13
#define IDX_LOGINCHK 	14
#define IDX_DAEMONCHK 	15
#define IDX_SUCHK 	16
#define IDX_TPATH 	17
#define IDX_TTYS 	18
#define IDX_SUGROUPS 	19
#define IDX_EXPIRATION 	20
#define IDX_AUTH1 	21
#define IDX_AUTH2 	22
#define IDX_UFSIZE 	23
#define IDX_UCPU 	24
#define IDX_UDATA 	25
#define IDX_USTACK 	26
#define IDX_UCORE 	27
#define IDX_URSS 	28
#define IDX_UMASK 	29
#define IDX_USRENV 	30
#define IDX_RLOGINCHK	31
#define IDX_TELNETCHK	32
#define IDX_UADMIN	33
#define IDX_PWD		34
#define IDX_LASTTIME	35
#define IDX_ULASTTIME	36
#define IDX_LASTTTY	37
#define IDX_ULASTTTY	38
#define IDX_LASTHOST	39
#define IDX_ULASTHOST	40
#define IDX_ULOGCNT	41
#define IDX_GROUPSIDS	42
#define IDX_AUTHSYSTEM	43
#define IDX_REGISTRY	44
#define IDX_LOGTIMES	45
#define IDX_LOCKED	46
#define IDX_LOGRETRIES	47
#define IDX_MINALPHA    48
#define IDX_MINOTHER    49
#define IDX_MINDIFF     50
#define IDX_MAXREPEAT   51
#define IDX_MINLEN      52
#define IDX_MINAGE      53
#define IDX_MAXAGE      54
#define IDX_MAXEXPIRED  55
#define IDX_HISTEXPIRE  56
#define IDX_HISTSIZE    57
#define IDX_PWDCHECKS   58
#define IDX_DICTION     59
#define IDX_PWDWARNTIME 60
#define IDX_USREXPORT	61
#define IDX_ULAST       62

/* indexes into user file table */
#define	S_BASEPWD	0	/* base password file */
#define	S_BASEGRP	1	/* base group file */
#define S_USERFILE	2	/* user file */
#define S_LIMITSFILE	3	/* limits file */
#define S_GROUPFILE	4	/* security/group file */
#define S_ENVFILE	5	/* basic information file */
#define S_AUDITFILE	6	/* audit password file */
#define S_SPWDFILE	7	/* security password file */
#define S_LASTLOG	8	/* security password file */
#define S_BASEGRPIDS	9	/* base group file by gids */
#define S_ACCTSFILE	NOT_IMPLEM	

/* indexes into sysck attribute table */
#define IDX_CLASS 	1
#define IDX_OWNER 	2
#define IDX_GROUP 	3
#define IDX_MODE 	4
#define IDX_PROGRAM	5
#define IDX_TCB		6
#define IDX_LINKS	7
#define	IDX_SYMLINKS	8
#define	IDX_TYPE	9
#define	IDX_ACL		10
#define	IDX_PCL		11
#define	IDX_CHECKSUM	12
#define	IDX_SOURCE	13
#define	IDX_SIZE	14
#define	IDX_TARGET	15
#define	IDX_SLAST	16

/* indexes into sysck file table */
#define	S_SYSCK		0	/* base tcb file */

/* indices into port attribute table */
#define IDX_HERALD	1
#define IDX_SAKENABLED	2
#define IDX_SYNONYM	3
#define IDX_PLOGTIMES	4
#define IDX_LOGDISABLE	5
#define IDX_LOGINTERVAL	6
#define IDX_LOGREENABLE 7
#define IDX_LOGDELAY	8
#define IDX_LOCKTIME	9
#define IDX_ULOGTIMES	10
#define IDX_POLAST	11

/* indices into port file table */
#define S_LOGINCFG	0
#define S_PORTLOG	1

/* indices into generic attribute table */
#define IDX_GENERIC	1
#define IDX_GENLAST	2

/* indices into generic file table */
#define S_GENERIC	0

/********************************************************/
/*	structures in used in user attribute calls	*/
/********************************************************/

/* the administrative login file structure */
struct	adm
{
	char	*name;		/* user name */
	long	lastlogin;	/* time of last login */
	long	lastbad;	/* time of last bad login */
	int	numbad;		/* number of bad logins allowed */
};

/* per attribute structure */
struct  attr
{
	int	ai;		/* attribute index */
	int	fi;		/* index into file table */
	char	*atnam;		/* name of attribute stanza files only */
	int	xi;		/* index into translate table */
	int	field;		/* field number in the record */
				/*   stanza files are always 0 and */
				/*   colon files are always greater than 0 */
	int	qwerks;		/* special handling */
	int	(*chk)();	/* validate attribute function, returns 0 if ok */
};
#define nchk	((int (*)()) 0)	/* null validate attribute function */


/* structure holding all the attributes */
struct  atval
{
	short		siz;	/* where used this means the # of elements */
	unsigned short	flgs;	/* flags */
	char		*val;	/* the attribute value */
};

/* per entity (user, group, acct) structure */
struct	eval
{
	char		*name;	
	struct	atval 	*atvals;	
	struct	eval	*next;	
};

/* per file structure */
struct  atfile
{
	char	*org;		/* file name */
	char	*old;		/* old file name */
	char	*fhdle;		/* file handle */
	int	fd;		/* file descriptor */
	int	flags;		/* file is locked or cached etc. */
	char	*buf;		/* place to hold file string pointer */
	char	*(*open)();	/* setup the session for this attribute */
	int	(*close)();	/* end the session for this attribute */
	int	(*read)();	/* read in next record */
	char	*(*write)();	/* file specific write */
	int	(*new)();	/* routine to generate a new record */
	int	(*del)();	/* file specific delete */
};

/* per data type structure */
struct  xlat
{
	int	siz;		/* size of data type in bytes */
	char	*(*get)();	/* cache to name */
	char	*(*put)();	/* name to cache */
	char	*(*cpy)();	/* copy caller supplied info into our cache */
};

/* per session structures */
struct 	ehead
{
	int		mode;		/* type of underlying storage */
	struct eval	*eval;		/* name name and attribute values */
	int		lockfd;		/* lock file for this session */
};

/*
 * Message buffer control structure.
 */
typedef struct  msgbuf {
        char    **buf;          /* The message buffer. */
        int     used;           /* The current amount of data in the buffer. */
                                /* It does not include the null character.   */
        int     size;           /* The current size of the buffer. */
        int     size_increment; /* The dynamic size increments.    */
} MsgBuf;


/* security library routines */
extern void    _MBInitialize(MsgBuf *, char **);
extern void    _MBDiscard(MsgBuf *);
extern int     _MBAppend();
extern int     _MBReplace();
extern int	passwdexpired( char *, char **);
extern int	passwdrequired(char *, char **);
extern int	passwdrestrictions(char *, struct userpw *, char *, char *,
							char *, int, MsgBuf *);
extern int     _local_passwdrestrictions(struct userpw *, char *, char *,
								int, MsgBuf *);
extern int     _PasswdRetrievalPolicy(char *, struct passwd **,
					struct userpw **, char *);
extern int	checkforadm(ulong);
extern int     _load_secmethod(char *, struct secmethod_table *);
extern char    *_getregistry(char *);
extern char    *_normalize_username(char *, char *);
extern char    *_truncate_username(char *);
extern char    *getlibsmsg(int, char *);
extern struct	passwd	*_pwddup( struct passwd *);
extern void		 _pwdfree(struct passwd *);
extern struct	userpw	*_userpwdup( struct userpw *);
extern void		 _userpwfree(struct userpw *);
extern int	_GetLicense(char *, char *);
extern void	_ReleaseLicense(void);
extern int	_delete_PWDHistory(char *);
extern int	_writelock(int);
extern void	_writeunlock(int);


/****************/
/* macros	*/
/****************/


/* copy up to and including the next colon or newline */
#define COPYFIELD(np, cp)\
{\
	while (*cp && (*cp != ':') && (*cp != '\n'))\
	{\
		*np++ = *cp++;\
	}\
	if (*cp)\
		*np++ = *cp++;\
}


#define CPYCOMMENT(dest, src)\
{\
	while (*src == '*')\
	{\
		while (*src != '\n')\
		{\
			*dest++ = *src++;\
		}\
	}\
}
/* copy the stanza name or attribute value (both delimited by newline) */
/* comments always come before or after a stanza name or after an attribute value */
#define CPYNAME(dest, src)\
{\
	CPYCOMMENT(dest, src);\
	while (*src != '\n')\
	{\
		*dest++ = *src++;\
	}\
	*dest++ = *src++;\
	CPYCOMMENT(dest, src);\
}
#define CPYSPACE(dest, src)\
	{\
		while (isspace ((int)*src))\
		{\
			*dest++ = *src++;\
		}\
	}
#define CPYATTR(dest, src)\
	{\
		while (*src != '=')\
		{\
			*dest++ = *src++;\
		}\
		*dest++ = *src++;\
		while ((*src == ' ') || (*src == '\t'))\
		{\
			*dest++ = *src++;\
		}\
	}
/* skip to end of line */
#define SKIPeol(mp)\
	{\
		while (*mp && (*mp++ != '\n')) ;\
	}
/* skip to next line */
#define NEXTLINE(mp)\
	{\
		while (*mp && (*mp++ != '\n')) ;\
		while (*mp == '*')\
			while (*mp && *mp++ != '\n') ;\
	}

/************************************/
/* file names for the user database */
/************************************/

#define PWD_FILENAME	"/etc/passwd"
#define GROUP_FILENAME	"/etc/group"
#define	SPWD_FILENAME	"/etc/security/passwd"
#define USER_FILENAME	"/etc/security/user"
#define LIMIT_FILENAME	"/etc/security/limits"
#define	SGROUP_FILENAME	"/etc/security/group"
#define	ENV_FILENAME	"/etc/security/environ"
#define	AUDIT_FILENAME	"/etc/security/audit/config"
#define	LOGIN_FILENAME	"/etc/security/login.cfg"
#define USERDEF_FILENAME "/usr/lib/security/mkuser.default"
#define LASTLOG_FILENAME "/etc/security/lastlog"
#define	SYSCK_FILENAME	"/etc/security/sysck.cfg"
#define PORTLOG_FILENAME "/etc/security/portlog"
#define FAILEDLOG_FILENAME "/etc/security/failedlogin"


/*******************************/
/* Password History File Names */
/*******************************/

#define	HISTFILE	"/etc/security/pwdhist"
#define	HISTPGFILE	"/etc/security/pwdhist.pag"
