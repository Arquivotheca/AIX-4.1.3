/* @(#)08	1.45  src/bos/usr/bin/newgrp/tcbauth.h, cmdsuser, bos41B, 412_41B_sync 12/19/94 16:51:22 */
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: MSGSTR
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <userconf.h>
#include <usersec.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <ndbm.h>

/************************/
/* Language Support	*/
/************************/
#include  "tcbauth_msg.h"
nl_catd  catd;
#define	MSGSTR(Num, Str) catgets (catd, MS_TCBAUTH, Num, Str)

/************************/
/* Error Messages 	*/
/************************/

#define GRPNINST 	MSGSTR(M_GRPNINST, "\"%s\" not in current user groupset.\n")
#define GRPIN 		MSGSTR(M_GRPIN, "\"%s\" already in current process groupset")
#define ISREEL 		MSGSTR(M_ISREEL, "Group \"%s\" is already real group")
#define GETUSRGRPS	MSGSTR(M_GETUSRGRPS, "Error getting groups for \"%s\".\n")
#define GETPCRED	MSGSTR(M_GETPCRED, "Error getting process credentials")
#define SETPCRED	MSGSTR(M_SETPCRED, "Error setting process credentials")
#define SETPRIV		MSGSTR(M_SETPRIV, "Cannot set process privilege")
#define GETPENV		MSGSTR(M_GETPENV, "Error getting environment")

#define GRPTOLG 	MSGSTR(M_GRPTOLG, "Exceeds maximum number of groups \"%s\"")
#define LASTGRP         MSGSTR(M_LASTGRP, "Current process group set must contain at least one group.\n")
#define CURUSE 		MSGSTR(M_CURUSE, "\tuser groups = %s\n")
#define CURPROC		MSGSTR(M_CURPROC, "\tprocess groups = %s\n")

#define GRPNONEX	MSGSTR(M_GRPNONEX, "Group \"%s\" does not exist.\n")
#define USRNONEX	MSGSTR(M_USRNONEX, "User \"%s\" does not exist.\n")
#define GEXIST		MSGSTR(M_GEXIST, "Group \"%s\" exists.\n")
#define UEXIST		MSGSTR(M_UEXIST, "User \"%s\" exists.\n")
#define NODEFGRP	MSGSTR(M_NODEFGRP, "No default group.\n")
#define NODEFHOME	MSGSTR(M_NODEFHOME, "No default home directory.\n")
#define MKUSRSYS 	MSGSTR(M_MKUSRSYS, "Could not create user.\n")

#define CHGONERR	MSGSTR(M_CHGONERR, "Error changing \"%s\"")
#define CHGTOERR	MSGSTR(M_CHGTOERR, "Error changing \"%s\" to \"%s\"")
#define SETERR		MSGSTR(M_SETERR, "Cannot set \"%s\" to \"%s\"")
#define ERRADD		MSGSTR(M_ERRADD, "Error adding \"%s\"")
#define ERRLIST		MSGSTR(M_ERRLIST, "Error listing \"%s\"")
#define ERRADM		MSGSTR(M_ERRADM, "Error adding \"%s\" to \"%s\"")
#define RMERR		MSGSTR(M_RMERR, "Error removing \"%s\"")
#define LISTERR		MSGSTR(M_LISTERR, "Attribute \"%s\" is not valid.\n")

#define COMMIT		MSGSTR(M_COMMIT, "Error committing changes to \"%s\"")

#define UNKIVAL		MSGSTR(M_UNKIVAL, "\"%d\"")
#define UNKSVAL		MSGSTR(M_UNKSVAL, "\"%s\"")

#define DROP		MSGSTR(M_DROP, "Cannot drop \"%s\" from primary group \"%s\".\n")
#define RMPRIME		MSGSTR(M_RMPRIME, "Cannot remove \"%s's\" primary group \"%s\".\n")
#define USRINFO		MSGSTR(M_USRINFO, "Could not get user id.\n")
#define EXECL		MSGSTR(M_EXECL, "Error execing \"%s\".\n")

#define ISADMIN		MSGSTR(M_ISADMIN, "\"%s\" already administrator")
#define OPENERR		MSGSTR(M_OPENERR, "Cannot open \"%s\" ")
#define IDINFO		MSGSTR(M_IDINFO, "Couldn't get ids ")
#define GERRGET		MSGSTR(M_GERRGET, "Error getting gecos for \"%s\".\n")
#define CURRGEK		MSGSTR(M_CURRGEK, " %s's current gecos: \n\t\t\"%s\"")
#define CHANGE		MSGSTR(M_CHANGE, " Change (%s) or (%s)? > ")
#define TO		MSGSTR(M_TO, " To?>")

#define SERRGET		MSGSTR(M_SERRGET, "Error getting shell for \"%s\".\n")
#define ERRSET		MSGSTR(M_ERRSET, "Error setting shell for \"%s\" to \"%s\"")
#define ACURRSHELL	MSGSTR(M_ACURRSHELL, " Current available shells:\n")
#define UCURRSHELL	MSGSTR(M_UCURRSHELL, " %s's current login shell:\n")

#define MEMBS		MSGSTR(M_MEMBS, "\tmembers = %s\n")
#define ADMIN		MSGSTR(M_ADMIN, "\tadms = %s\n")
#define GETAT		MSGSTR(M_GETAT, "Error getting attribute \"%s\" for \"%s\"")
#define USRNIN 		MSGSTR(M_USRNIN, "User \"%s\" not in current list.\n")

#define SETSENUSAGE	MSGSTR(M_SETSENUSAGE, "Usage: setsenv [-] [\"variable=value\" ... ]\n")
#define SETGRUSAGE	MSGSTR(M_SETGRUSAGE, "Usage: setgroups [-] [[[ -a | -d ] groupset ] | [ -r [group]]]\n")
#define NEWGRUSAGE	MSGSTR(M_NEWGRUSAGE, "Usage: newgrp [-] [-l] [group]\n")
#define CHUSRUSAGE	MSGSTR(M_CHUSRUSAGE, "Usage: chuser \"attr=value\" ... user\n")
#define CHMEMUSAGE	MSGSTR(M_CHMEMUSAGE, "Usage: chgrpmem {{ -a | -m } {+-=} members} group\n")
#define CHSHUSAGE	MSGSTR(M_CHSHUSAGE, "Usage: chsh [username [shell]]\n")
#define CHFNUSAGE	MSGSTR(M_CHFNUSAGE, "Usage: chfn [username]\n")

#define RMUSRUSAGE 	MSGSTR(M_RMUSRUSAGE, "Usage: rmuser [-p] user\n")
#define RMGRPUSAGE	MSGSTR(M_RMGRPUSAGE, "Usage: rmgroup group\n")

#define LSGRPUSAGE	MSGSTR(M_LSGRPUSAGE, "Usage: lsgroup [ -c | -f ] [ -a attr attr .. ] { \"ALL\" | group1,group2 ... }\n")
#define LSUSRUSAGE	MSGSTR(M_LSUSRUSAGE, "Usage: lsuser [ -c | -f ] [ -a attr attr ... ] { \"ALL\" | user1,user2 ... }\n")

#define CHGRPUSAGE	MSGSTR(M_CHGRPUSAGE, "Usage: chgroup \"attr=value\" ... group\n")

#define MKGRPUSAGE 	MSGSTR(M_MKGRPUSAGE, "Usage: mkgroup [-a] [-A] \"attr=value\" ... newgroup\n")
#define MKUSRUSAGE	MSGSTR(M_MKUSRUSAGE, "Usage: mkuser [-a] \"attr=value\" ... newuser\n")

#define NAMINFO		MSGSTR(M_NAMINFO, "Couldn't get user name from id \"%d\"")
#define CHGPASS		MSGSTR(M_CHGPASS, "Changing password for \"%s\"\n")
#define GOTINT		MSGSTR(M_GOTINT, "Terminating from signal\n")
#define KEYWD		MSGSTR(M_KEYWD, "Keyword \"%s\" ")
#define PWADMUSAGE	MSGSTR(M_PWADMUSAGE, "Usage: pwdadm [ -f flags | -q | -c ] user\n")
#define BADFLAG		MSGSTR(M_BADFLAG, "Flag \"%s\" ")
#define ERRGET		MSGSTR(M_ERRGET, "Error getting \"%s\" value")
#define LONGNAME	MSGSTR(M_LONGNAME, "Username \"%s\" is too long.\n")

#define PASUSAGE	MSGSTR(M_PASUSAGE, "Usage: passwd [ -f | -s ] [username]\nWhere:\n\t-f\tchanges your finger information\n\t-s\tchanges your login shell.\n")
#define NOAUTH		MSGSTR(M_NOAUTH, "You are not authorized to change \"%s's\" password.\n")
#define IDNOEXIST	MSGSTR(M_IDNOEXIST, "User \"%lu\" does not exist.\n")
#define ERRLOCK		MSGSTR(M_ERRLOCK, "Error setting lock on file \"%s\"")
#define SETPENV		MSGSTR(M_SETPENV, "Error setting process environment")
#define MALLOC		MSGSTR(M_MALLOC, "Error getting memory for process")
#define LONGGROUP	MSGSTR(M_LONGGROUP, "Groupname \"%s\" is too long.\n")

#define CHECK		MSGSTR(M_CHECK, "Check \"%s\" file.\n")
#define NOTVALID	MSGSTR(M_NOTVALID, " : Not an allowable value.\n")
#define TOOLONG		MSGSTR(M_TOOLONG, " : Name is too long.\n")
#define PERMISSION	MSGSTR(M_PERMISSION, " : You do not have permission.\n")
#define ERRADDADM	MSGSTR(M_ERRADDADM, "Error adding administrator \"%s\"")
#define NOMEM		MSGSTR(M_NOMEM, " : Not enough memory.\n")
#define USRTOGRPER	MSGSTR(M_USRTOGRPER, "Error adding \"%s\" to group \"%s\".\n")
#define ERBADVAL	MSGSTR(M_ERBADVAL, " : Value is invalid.\n")
#define ERBADATTR	MSGSTR(M_ERBADATTR, " : Attribute is invalid.\n")
#define ERNOTERM	MSGSTR(M_ERNOTERM, " : Must run from tty.\n")
#define RMKEY		MSGSTR(M_RMKEY, "Cannot remove keyword \"%s\".\n")
#define RMADMERR	MSGSTR(M_RMADMERR, "Error removing administrator \"%s\"")
#define RMADMUSR	MSGSTR(M_RMADMUSR, "Cannot remove \"%s\" from \"%s\"")
#define ERCHGPASS	MSGSTR(M_ERCHGPASS, "Error changing password for \"%s\"")
#define TERMINAL	MSGSTR(M_TERMINAL, "\"%s\" must be run from a valid tty.\n")
#define SETPASS		MSGSTR(M_SETPASS, "Setting \"%s's\" password to NULL.\n")
#define SETGECOS	MSGSTR(M_GECOS, " Gecos information not changed.\n")
#define SETSHELL	MSGSTR(M_SHELL, " Login shell not changed.\n")
#define ACCEXIST	MSGSTR(M_ACCEXIST, " : Account exists.\n")

#define DBM_ADD_FAIL	MSGSTR(M_DBM_ADD_FAIL, \
"Cannot update the DBM files.  Run \"mkpasswd\" to correct the DBM files.\n")

#define	MUST_USE_YPPASSWD	MSGSTR(M_YPPASSWD,\
	"You must use yppasswd on a NIS server.\n")
#define	MUST_USE_MASTER		MSGSTR(M_MASTER,\
	"You can only change account attributes on the NIS master.\n")
#define WARN_NO_UPDATE	MSGSTR(M_WARN,\
	"Warning: %s does not update %s with the new gid.\n")
#define DEF_CANT_ID\
	"You can only change the User ID on the name server.\n"
#define ID_SERVONLY	MSGSTR(CANT_ID, DEF_CANT_ID)
#define DEF_CANT_PGRP\
	"You can only change the PRIMARY group on the name server.\n"
#define PGRP_SERVONLY	MSGSTR(CANT_PGRP, DEF_CANT_PGRP)
#define DEF_CANT_GROUPS\
	"You can only change the Group SET on the name server.\n"
#define GROUPS_SERVONLY	MSGSTR(CANT_GROUPS, DEF_CANT_GROUPS)
#define DEF_CANT_HOME\
	"You can only change the HOME directory on the name server.\n"
#define HOME_SERVONLY	MSGSTR(CANT_HOME, DEF_CANT_HOME)
#define DEF_CANT_SHELL\
	"You can only change the Initial PROGRAM on the name server.\n"
#define SHELL_SERVONLY	MSGSTR(CANT_SHELL, DEF_CANT_SHELL)
#define DEF_CANT_GECOS\
       "You can only change the User INFORMATION on the name server.\n"
#define GECOS_SERVONLY	MSGSTR(CANT_GECOS, DEF_CANT_GECOS)

#define FLAGS		MSGSTR(M_FLAGS, "flags")
#define ADMINFLAGS	MSGSTR(M_ADMINFLAGS, "ADMIN")

#define	DEF_MKPASSWD \
	"Usage: mkpasswd [ -v ] file\n"
#define	DEF_NOPASSWD \
	"\"%s\" does not exist or could not be opened for reading.\n"
#define	DEF_NOPWDDBM \
	"The DBM files for \"%s\" could not be created.\n"
#define	DEF_PWDBMFAIL \
	"The DBM write to \"%s\" failed.\n"
#define	DEF_DSTORE \
	"Storing user \"%s\" as UID %u.\n"
#define	DEF_MKPWDFINI \
	"%d password entries, maximum length %d\n"
#define DEF_PWDBADFORM \
	"Error reading file \"%s\", line %d is not in the correct format.\n"

#define CHSECUSAGE	MSGSTR(M_CHSECUSAGE, \
	"Usage: chsec -f file -s stanza -a \"attr=value\" ...\n")
#define LSSECUSAGE	MSGSTR(M_LSSECUSAGE, \
	"Usage: lssec [-c] -f file -s stanza -a attr ...\n")
#define FILENAME	MSGSTR(M_FILENAME, \
	"Only one file name can be specified.\n")
#define BADFILENAME	MSGSTR(M_BADFILENAME, "Invalid file name \"%s\".\n")
#define STANZA		MSGSTR(M_STANZA, \
	 "Only one stanza name can be specified.\n")
#define BADSTANZA	MSGSTR(M_BADSTANZA, "Invalid stanza name \"%s\".\n")

/************************/
/* Hard wired pathnames */
/************************/

#define	PASSWD		"/etc/passwd"
#define SPASSWD		"/etc/security/passwd"
#define	GROUP		"/etc/group"
#define	SGROUP		"/etc/security/group"
#define USERFILE	"/etc/security/user"
#define ENVFILE		"/etc/security/environ"
#define LIMFILE		"/etc/security/limits"
#define MKSYS		"/usr/lib/security/mkuser.sys"
#define	AUDFILE		"/etc/security/audit/config"
#define MKDEF		"/usr/lib/security/mkuser.default"
#define	DOTIDFILE	"/etc/security/.ids"
#define	LOGINFILE	"/etc/security/login.cfg"
#define	LASTLOG		"/etc/security/lastlog"

#define RMUSER		"/usr/sbin/rmuser"
#define CHUSER		"/usr/bin/chuser"
#define CHGROUP		"/usr/bin/chgroup"
#define CHGRPMEM	"/usr/bin/chgrpmem"
#define SETGROUPS	"/usr/bin/setgroups"
#define NEWGRP		"/usr/bin/newgrp"
#define SETSENV		"/usr/bin/setsenv"
#define PWDADMIN	"/usr/bin/pwdadm"
#define CHSH		"/usr/bin/chsh"
#define CHFN		"/usr/bin/chfn"
#define BINSH		"/usr/bin/sh"

/************************/
/* Variables & Maximums	*/
/************************/

#define MAXID		100	    /* largest id			      */
#define MAXATTRS	64	    /* maximum number of attributes 	      */
#define MIN_FSIZE	8192	    /* smallest fsize 			      */
#define MIN_STACK	49	    /* smallest stack 			      */
#define MIN_DATA	1272	    /* smallest data 			      */
#define	MAX_FSIZE	4194303	    /* largest fsize,(0x0..0x7FFFFFFF)-512    */
#define	MAX_STACK	523264	    /* max stack,0x20000000..0x2FF7FFFC(errno)*/
#define	MAX_DATA	4194304	    /* largest data,0x30000000..0xAFFFFFFF    */
#define NOPRINT		0	    /* flag to exitax 			      */
#define PRINT		1	    /* flag to exitax 			      */
#define BADATTR		3000	    /* flag to exitax 			      */
#define BADVALUE	3001	    /* flag to exitax 			      */
#define EXISTS		3002	    /* flag to exitax 			      */
#define NOEXISTS	3003	    /* flag to exitax 			      */
#define MAXBOOL		16	    /* Max boolean length in English 	      */
#define SECURITY	"security"  /* Group "security" 		      */


/************************/
/* Headers for commands	*/
/************************/

#define HEADER		"GROUPS="
#define SHEAD		"shell="
#define GHEAD		"gecos="
#define MHEAD		"users="
#define AHEAD		"adms="
#define REAL_GROUP	"REAL_GROUP="

/************************/
/* Audit Events		*/
/************************/

#define CHGRPAUD	"GROUP_Change"
#define	MKGRPAUD	"GROUP_Create"
#define	RMGRPAUD	"GROUP_Remove"

#define CHUSRAUD	"USER_Change"
#define	MKUSRAUD	"USER_Create"
#define	RMUSRAUD	"USER_Remove"

#define CHPORTAUD	"PORT_Change"

#define	SETGRAUD	"USER_SetGroups"
#define	SETUAUD		"USER_SetEnv"
#define	PWDCHGAUD	"PASSWORD_Change"
#define	PWDFLGAUD	"PASSWORD_Flags"

/************************/
/* Configuration Files	*/
/************************/

#define	OPASSWD			"/etc/opasswd"
#define	OSPASSWD		"/etc/security/opasswd"
#define	OGROUP			"/etc/ogroup"
#define	OSGROUP			"/etc/security/ogroup"
#define OUSERFILE		"/etc/security/ouser"
#define OENVFILE		"/etc/security/oenviron"
#define OLIMFILE		"/etc/security/olimits"
#define	OAUDFILE		"/etc/security/audit/oconfig"

/************************/
/* Flags to setpenv()	*/
/************************/

#define RESET	0x1	/* reset env to passed in env string 	*/
#define	INIT	0x2	/* reset env to original login env 	*/
#define DELTA	0x4	/* keep env to current env 		*/

#define MEM		1
#define ADM		2


/********************************/
/* Flags to lsuser and lsgroup	*/
/********************************/

#define AFLAG	0x1
#define CFLAG	0x2
#define FFLAG	0x4
#define	COMMA	","
#define	CARAT	"^"
#define	COLON	":"
#define	SPACE	" "
#define	STAR	"*"
#define QUOTES	0x1	/* indicates attribute needs to be quoted */


/********************************/
/* Generic defines for CMDSUSER */
/********************************/

#define ALL	"ALL"
#define DEFAULT	"default"
#define YES	"yes"
#define NO	"no"
#define ALWAYS	"always"
#define NEVER	"never"
#define NOSAK	"nosak"
#define NOTSH	"notsh"
#define NOCHECK	"NOCHECK"
#define ADMCHG	"ADMCHG"
#define _ADMIN	"ADMIN"
#define ON	"on"
#define _TRUE	"true"
#define _FALSE	"false"

/********************************/
/*  defines for pwdbm_update()  */
/********************************/

#define PWDBM_PASSWD 1 
#define PWDBM_SHELL  4
#define PWDBM_GECOS  6



extern	int	chknames(char *,char **);
extern	int	chkbool(char *,char **);
extern	int	chkgid(char *,gid_t *);
extern	char	*listocom(char *);
extern	int	listlen(char *);
extern	int	strtolist(char *,char **);
extern	char	**listoarray(char *,int *);
extern	char	*chglist(char *,char *);
extern	char	*chgbool(char *,char *);
extern	char	*getusername(int,char *);
extern	char	*gets (char *);
extern	int	changeattr(char *,char *,char *);
extern	int	chkuid(char *,uid_t *);
extern	int	chkint(char *,unsigned long *);
extern	int	chkumask(char *,unsigned long *);
extern	int	chkmkumask(char *,unsigned long *);
extern	int	chkpgrp(char *,char **);
extern	int	chkgrp(char *,char **);
extern	int	chkadmgroups(char *,char **);
extern	int	chkadmgroup(char *,char **);
extern	int	chkmkadmgroups(char *,char **);
extern	int	chkgrps(char *,char **);
extern	int	chkmgrps(char *,char **);
extern	int	chkprog(char *,char **);
extern	int	chkaudit(char *,char **);
extern	int	chkaud(char *,char **);
extern	int	chktpath(char *,char **);
extern	int	chkttys(char *, char **);
extern	int	chkhome (char *, char **);
extern	int	chkgek(char *,char **);
extern	int	chkexpires(char *,char **);
extern	int	chkstack(char *,unsigned long *);
extern	int	chkmkstack(char *,unsigned long *);
extern	int	chkulimit(char *,unsigned long *);
extern	int	chkmkulimit(char *,unsigned long *);
extern	int	chkdata(char *,unsigned long *);
extern	int	chkmkdata(char *,unsigned long *);
extern	char	**comtoarray(char *,int *);
extern	void	inkids(int,int,uid_t,uid_t,gid_t,gid_t,int);
extern	int	getids(char *,uid_t *,uid_t *,gid_t *,gid_t *,char *);
extern	char	*getnewstring(char *,char *,char *,int,void *);
extern	void	addusers(char *,char *,char *,char **);
extern	int	delmems(char *,char *,char *,char **);
extern	int	addtogroup(char *,char *);
extern	int	addtopgrp(char *,char *);
extern	int	addtoadms(char *, char *);
extern	int	addtogrouplist(char *, char *);
extern	int	addtoadmgroups(char *, char *);
extern	int	chkname(int,char *);
extern	int	getvals(char *,char **,char **);
extern	int	putvalue(char *,char *,char *,char *,char *);
extern	int	chkeyword(char *);
extern	void	printstanza(char *,char *);
extern	void	printcolon(char *,char *);
extern	DBM	*pwdbm_open();
extern	int	pwdbm_update(DBM *, char *, char *, int);
extern	int	pwdbm_add(DBM *, char *);
extern	int	pwdbm_delete(DBM *, char *);
extern	int	chkaccess(char *,char *,char *);
extern	char	*dropblanks(char *);
extern	char	*getreal(int,char *,char *,char *,char **);
extern	void	getrealgroup(char *,char *);
extern	void	exitx(int);
extern	int	beqpriv();
extern	void	exitex(char *, int, char *, char *, int);
extern	void	xaudit(char *, int, char *, char *);
extern	int	gotaccess(void);
extern	void	usage(char *);
extern	void	xusage(char *,char *,char *);
extern	int	checkfortty(void);
extern	int	gotgaccess(char *);
extern	int	gotuaccess(char *);
extern	int	gotiaccess(char *);
extern	int	updpasswd(char *);
extern	int 	chkregistry(char *, char **);
extern	int	chkauthsystem(char *, char **);
extern	int	chkauthmethod(char *, char **);
extern	int	chkparsetree(struct secgrammar_tree *);
extern	int	addvalue(char *,char *,char *,char *);
extern	void	onint(void);
extern	int	_usertodb(char *, char **);
extern	char	*_dbtouser(char *, char *);
extern  int     _dbtomkuser(char *, char **);
extern	int	chkminage(char *, int *);
extern	int	chkmkminage(char *, int *);
extern	int	chkmaxage(char *, int *);
extern	int	chkmkmaxage(char *, int *);
extern	int	chkmaxexpired(char *, int *);
extern	int	chkmkmaxexpired(char *, int *);
extern	int	chkminalpha(char *, int *);
extern	int	chkmkminalpha(char *, int *);
extern	int	chkminother(char *, int *);
extern	int	chkmkminother(char *, int *);
extern	int	chkmindiff(char *, int *);
extern	int	chkmkmindiff(char *, int *);
extern	int	chkmaxrepeats(char *, int *);
extern	int	chkmkmaxrepeats(char *, int *);
extern	int	chkminlen(char *, int *);
extern	int	chkmkminlen(char *, int *);
extern	int 	chkhistexpire(char *, int *);
extern	int 	chkmkhistexpire(char *, int *);
extern	int 	chkhistsize(char *, int *);
extern	int 	chkmkhistsize(char *, int *);
extern	int	chkpwdchecks(char *, char **);
extern	int	chkdictionlist(char *, char **);


struct ids
	{
		uid_t	id;
		struct ids *next;
	};
		
struct chusattr 
	{
	char	*gattr;
	char	*gval;
	int	type;
	int	(*check)(char *val, char **ret);
	};

extern struct chusattr chusatab[ ];
extern int chusatabsiz;

struct chgrattr 
	{
	char	*gattr;
	char	*gval;
	int	type;
	int	(*check)(char *val, char **ret);
	};

extern struct chgrattr chgratab[ ];
extern int chgratabsiz;

struct lsgrattr 
	{
	char	*gattr;
	int	type;
	char	*(*format)(char *name,char *val);
	};

extern struct lsgrattr lsgratab [ ];
extern int lsgratabsiz;

struct lsusattr 
	{
	char	*gattr;
	int	type;
	char	*(*format)(char *name,char *val);
	unsigned short qwerks;
	};

extern struct lsusattr lsusatab [ ];
extern int lsusatabsiz;

struct mkusattr 
	{
	char	*cattr;
	void	*cval;
	char	*gattr;
	char	*gval;
	int	(*check)(char *val, char **ret);
	};

extern struct mkusattr mkusatab [ ];
extern int mkusatabsiz;
