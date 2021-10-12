/* @(#)72	1.8.1.9  src/bos/usr/ccs/lib/libs/tables.h, libs, bos41J, 9512A_all 3/14/95 15:50:50 */
/*
 * COMPONENT_NAME: (LIBS) security library functions
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

extern	char		*opst ();		/* open stanza file */
extern	int		clst ();		/* close stanza file */
extern	int		rdst ();		/* read stanza file */

extern	char		*oppwd();		/* open base passwd file */
extern	int		clpwd();		/* close base passwd file */
extern	int		rdpwd();		/* read base passwd file */
extern	char		*wrpass();		/* write passwd file */

extern	char		*opbase();	/* open base passwd and group file */
extern	int		clbase();	/* close base passwd and group file */
extern	int		rdbase();	/* read base passwd and group file */

extern	char		*opgroup();		/* open base group file */
extern	int		clgroup();		/* close base group file */
extern	int		rdgroup();		/* read base group file */
extern	int		rdgrps();		/* read base group file */
extern	int		rdgrpsids();		/* read base group file (ids)*/
extern	char		*wrgrps();		/* read base group file */

extern	int		rdaudit();		/* read audit file */
extern	char		*wraudit();		/* read audit file */

extern	char		*getstr ();		/* get a string from caller */
extern	char		*putstr ();		/* put a string from caller */

extern	char		*getint ();		/* get an int from caller */
extern	char		*putint ();		/* put an int from caller */

extern	char		*getlist ();		/* get a list from caller */
extern	char		*putlist ();		/* put a list into the cache */

extern	char		*getbool ();		/* get a boolean from caller */
extern	char		*putbool ();		/* put a boolean from caller */
extern	char		*cpybool ();		/* copy a boolean from caller */

extern	int		chgstanza ();		/* change a stanza file */
extern	int		chgcolon ();		/* change a colon file */

extern	int		rmrecord();	/* del an entry from a stanza file */
extern	int		rmaudit();	/* del an entry from the audit file */

struct	ehead		*getableinfo(); /* gets ehead struct for table */
extern	struct	ehead	*setattr();	/* set the session handle */
extern	char		*wrname ();	/* write to a file where  */
					/* the record starts with given name */

static int		chkgrp();	/* validates the group name */


/********************************/
/*	translation table	*/
/********************************/

struct	xlat	xtab [] = 
{
	{ 0,	 	 '\0',		'\0',		'\0'},
	{ 0,	 	 getstr,	putstr,		getstr},
	{ sizeof (int),  getint,	putint,		'\0'},
	{ 0,  	 	 getlist,	putlist,	getlist},
	{ sizeof (short),getbool,	putbool, 	cpybool} 
};

/********************************/
/* 	passwd file table 	*/
/********************************/

static	struct 	attr	pwdatab [] =
{
	{-1,		-1,		NULL,		-1,	  -1, 0, nchk},
	{IDX_PASSWD,	S_SECPWD,	SEC_PASSWD,	SEC_CHAR,  0, 0, nchk},
	{IDX_LASTUP,	S_SECPWD,	SEC_LASTUP,	SEC_INT,   0, 0, nchk},
	{IDX_FLAGS,	S_SECPWD,	SEC_FLAGS,	SEC_LIST,  0, 0, nchk},

	/* note: last entry is a wild card
	 * (i.e. the caller can specify attribute name and type
	 */
	{IDX_PLAST,	S_SECPWD,	"*",		0,	   0, 0, nchk},
};

static	struct	atfile	pwdftab [] =
{
	{ 
	  SPWD_FILENAME, "/etc/security/opasswd", NULL,0, 0, NULL,
	  opst, clst, rdst, wrname, chgstanza, rmrecord
	},
};

/********************************/
/* 	group file table 	*/
/********************************/

static	struct 	attr	groupatab [] =
{
	{ -1,		-1,		NULL,		-1,	  -1, 0, nchk},
	{ IDX_ID,	S_BASEGROUP,	S_ID,		SEC_INT,   2, 0, nchk},
	{ IDX_PUSERS,	S_GBASEPWD,	S_PUSERS,	SEC_LIST,  0, 0, nchk},
	{ IDX_USERS,	S_BASEGROUP,	S_USERS,	SEC_LIST,  3, 0, nchk},
	{ IDX_ADMS,	S_SECGROUP,	S_ADMS,		SEC_LIST,  0, 0, nchk},
	{ IDX_GADMIN,	S_SECGROUP,	S_ADMIN,	SEC_BOOL,  0, FAILBADVAL, nchk},
	{ IDX_GRPEXPORT,S_SECGROUP,	S_GRPEXPORT,	SEC_BOOL,  0, 0, nchk},

	/* 
	 * note: last entry is a wild card
	 * (i.e. the caller can specify attribute name and type
	 */
	{ IDX_GLAST,	S_SECGROUP,	"*",		0,	   0, 0, nchk}
};

static	struct	atfile	groupftab [] =
{
	{
	  GROUP_FILENAME, "/etc/ogroup", NULL, 0, 0, NULL,
	  opgroup, clgroup, rdgroup, wrname, chgcolon, rmrecord
	},
	{
	  SGROUP_FILENAME, "/etc/security/ogroup", NULL,0, 0, NULL,
	  opst,   clst,   rdst, wrname, chgstanza, rmrecord
	}
};

/********************************/
/*	user file table		*/
/********************************/

static	struct 	attr	useratab [] =
{
	{ -1,		-1,		NULL,		-1,        -1, 0, nchk},
	{ IDX_ID,	S_BASEPWD,	S_ID,		SEC_INT,    2, 0, nchk},
	{ IDX_PGRP,	S_BASEPWD,	S_PGRP,		SEC_CHAR,   3, 0,
									chkgrp},
	{ IDX_GROUPS,	S_BASEGRP,	S_GROUPS,	SEC_LIST,   0, 0, nchk},
	{ IDX_ADMGROUPS,S_USERFILE,	S_ADMGROUPS,	SEC_LIST,   0, 0, nchk},

	{ IDX_PACCT,	S_ACCTSFILE,	S_PACCT,	NOT_IMPLEM, 0, 0, nchk},
	{ IDX_ACCTS,	S_ACCTSFILE,	S_ACCTS,	NOT_IMPLEM, 0, 0, nchk},
	{ IDX_ADMACCTS,	S_ACCTSFILE,	S_ADMACCTS,     NOT_IMPLEM, 0, 0, nchk},

	{ IDX_ULIMIT,	S_LIMITSFILE, 	S_ULIMIT,       NOT_IMPLEM, 0, 0, nchk},
	{ IDX_AUDIT,	S_AUDITFILE,	S_AUDITCLASSES,	SEC_LIST,   0, 0, nchk},

	{ IDX_HOME,	S_BASEPWD,	S_HOME,		SEC_CHAR,   5, 0, nchk},
	{ IDX_SHELL,	S_BASEPWD,	S_SHELL,	SEC_CHAR,   6, 0, nchk},
	{ IDX_GECOS,	S_BASEPWD,	S_GECOS,	SEC_CHAR,   4, 0, nchk},

	{ IDX_SYSENV,	S_ENVFILE,	S_SYSENV,  SEC_LIST,   0, QUOTES, nchk},

	{ IDX_LOGINCHK,	S_USERFILE,	S_LOGINCHK,	SEC_BOOL,   0, 0, nchk},
	{ IDX_DAEMONCHK,S_USERFILE,	S_DAEMONCHK,	SEC_BOOL,   0, 0, nchk},
	{ IDX_SUCHK,	S_USERFILE,	S_SUCHK,	SEC_BOOL,   0, 0, nchk},
	{ IDX_TPATH,	S_USERFILE,	S_TPATH,	SEC_CHAR,   0, 0, nchk},
	{ IDX_TTYS,	S_USERFILE,	S_TTYS,		SEC_LIST,   0, 0, nchk},
	{ IDX_SUGROUPS,	S_USERFILE,	S_SUGROUPS,	SEC_LIST,   0, 0, nchk},
	{ IDX_EXPIRATION,S_USERFILE,	S_EXPIRATION,	SEC_CHAR,   0, 0, nchk},
	{ IDX_AUTH1,	S_USERFILE,	S_AUTH1,	SEC_LIST,   0, 0, nchk},
	{ IDX_AUTH2,	S_USERFILE,	S_AUTH2,	SEC_LIST,   0, 0, nchk},

	{ IDX_UFSIZE,	S_LIMITSFILE, 	S_UFSIZE,	SEC_INT,    0, 0, nchk},
	{ IDX_UCPU,	S_LIMITSFILE, 	S_UCPU,  	SEC_INT,    0, 0, nchk},
	{ IDX_UDATA,	S_LIMITSFILE, 	S_UDATA,	SEC_INT,    0, 0, nchk},
	{ IDX_USTACK,	S_LIMITSFILE, 	S_USTACK,	SEC_INT,    0, 0, nchk},
	{ IDX_UCORE,	S_LIMITSFILE, 	S_UCORE,	SEC_INT,    0, 0, nchk},
	{ IDX_URSS,	S_LIMITSFILE, 	S_URSS,		SEC_INT,    0, 0, nchk},

	{ IDX_UMASK,	S_USERFILE, 	S_UMASK,	SEC_INT,    0, 0, nchk},
  	{ IDX_USRENV,	S_ENVFILE,	S_USRENV,  SEC_LIST,   0, QUOTES, nchk},

	{ IDX_RLOGINCHK,S_USERFILE,	S_RLOGINCHK,	SEC_BOOL,   0, 0, nchk},
	{ IDX_TELNETCHK,S_USERFILE,	S_TELNETCHK,	SEC_BOOL,   0, 0, nchk},

	{ IDX_UADMIN,	S_USERFILE,	S_ADMIN,	SEC_BOOL,   0, 0, nchk},

	{ IDX_PWD,	S_BASEPWD,	S_PWD,		SEC_CHAR,   1, 0, nchk},

	/* attributes set by tsm */
	{ IDX_LASTTIME,	S_LASTLOG,	S_LASTTIME,	SEC_INT,    0, 0, nchk},
	{ IDX_ULASTTIME,S_LASTLOG,	S_ULASTTIME,	SEC_INT,    0, 0, nchk},
	{ IDX_LASTTTY  ,S_LASTLOG,	S_LASTTTY,	SEC_CHAR,   0, 0, nchk},
	{ IDX_ULASTTTY ,S_LASTLOG,	S_ULASTTTY,	SEC_CHAR,   0, 0, nchk},
	{ IDX_LASTHOST ,S_LASTLOG,	S_LASTHOST,	SEC_CHAR,   0, 0, nchk},
	{ IDX_ULASTHOST,S_LASTLOG,	S_ULASTHOST,	SEC_CHAR,   0, 0, nchk},
	{ IDX_ULOGCNT  ,S_LASTLOG,	S_ULOGCNT,	SEC_INT,    0, 0, nchk},

	/* this attribute is not set by tsm */
	{ IDX_GROUPSIDS,S_BASEGRPIDS,	S_GROUPSIDS,	SEC_LIST,   0, 0, nchk},

	{ IDX_AUTHSYSTEM,S_USERFILE,	S_AUTHSYSTEM,	SEC_CHAR,0,QUOTES,nchk},
	{ IDX_REGISTRY, S_USERFILE,	S_REGISTRY,	SEC_CHAR,   0, 0, nchk},
	{ IDX_LOGTIMES, S_USERFILE,	S_LOGTIMES,	SEC_LIST,   0, 0, nchk},
	{ IDX_LOCKED,	S_USERFILE,	S_LOCKED,	SEC_BOOL,   0, FAILTRUE, nchk},
	{ IDX_LOGRETRIES, S_USERFILE,	S_LOGRETRIES,	SEC_INT,    0, 0, nchk},

	/* password checking attributes */
	{ IDX_MINALPHA,   S_USERFILE,   S_MINALPHA,     SEC_INT,   0, 0, nchk},
	{ IDX_MINOTHER,   S_USERFILE,   S_MINOTHER,     SEC_INT,   0, 0, nchk},
	{ IDX_MINDIFF,    S_USERFILE,   S_MINDIFF,      SEC_INT,   0, 0, nchk},
	{ IDX_MAXREPEAT,  S_USERFILE,   S_MAXREPEAT,    SEC_INT,   0, 0, nchk},
	{ IDX_MINLEN,     S_USERFILE,   S_MINLEN,       SEC_INT,   0, 0, nchk},
	{ IDX_MINAGE,     S_USERFILE,   S_MINAGE,       SEC_INT,   0, 0, nchk},
	{ IDX_MAXAGE,     S_USERFILE,   S_MAXAGE,       SEC_INT,   0, 0, nchk},
	{ IDX_MAXEXPIRED, S_USERFILE,   S_MAXEXPIRED,   SEC_INT,   0, 0, nchk},
	{ IDX_HISTEXPIRE, S_USERFILE,   S_HISTEXPIRE,   SEC_INT,   0, 0, nchk},
	{ IDX_HISTSIZE,   S_USERFILE,   S_HISTSIZE,     SEC_INT,   0, 0, nchk},
	{ IDX_PWDCHECKS,  S_USERFILE,   S_PWDCHECKS,    SEC_LIST,  0, 0, nchk},
	{ IDX_DICTION,    S_USERFILE,   S_DICTION,      SEC_LIST,  0, 0, nchk},
	{ IDX_PWDWARNTIME,S_USERFILE,   S_PWDWARNTIME,  SEC_INT,   0, 0, nchk},


	/* DCE attributes */
	{ IDX_USREXPORT,  S_USERFILE,	S_USREXPORT,	SEC_BOOL,   0, 0, nchk},

	/* 
	 * note: last entry is a wild card 
	 * (i.e. the caller can specify attribute name and type
	 */
	{ IDX_ULAST,	S_USERFILE,	"*",		0,	    0, 0, nchk}
};

static	struct	atfile	userftab [] =
{
	{ 
	  PWD_FILENAME, "/etc/opasswd", NULL,0, 0, NULL,
	  opbase, clbase, rdbase, wrpass, chgcolon, rmrecord
	},
	{
	  GROUP_FILENAME, "/etc/ogroup", NULL, 0, 0, NULL,
	  opbase, clbase, rdgrps, wrgrps, NULL, rmrecord
	},
	{
	  USER_FILENAME, "/etc/security/ouser", NULL,0, 0, NULL,
	  opst,   clst,   rdst, wrname, chgstanza, rmrecord
	},
	{
	  LIMIT_FILENAME, "/etc/security/olimits", NULL,0, 0, NULL,
	  opst,   clst,   rdst, wrname, chgstanza, rmrecord
	},
	{
	  SGROUP_FILENAME, "/etc/security/ogroups", NULL,0, 0, NULL,
	  opst,   clst,   rdst, wrname, chgstanza, rmrecord
	},
	{
	  ENV_FILENAME,"/etc/security/oenviron",NULL,0, 0, NULL,
	  opst,   clst,   rdst, wrname, chgstanza, rmrecord
	},
	{
	  AUDIT_FILENAME,  "/etc/security/audit/oaudit",  NULL,0, 0, NULL,
	  opst,   clst,   rdaudit, wraudit, NULL, rmaudit
	},
	{ 
	  SPWD_FILENAME, "/etc/security/opasswd", NULL,0, 0, NULL,
	  opst,   clst,   rdst, wrname, chgstanza, rmrecord
	},
	{
	  LASTLOG_FILENAME, "/etc/security/olastlog", NULL,0, 0, NULL,
	  opst,   clst,   rdst, wrname, chgstanza, rmrecord
	},
	{
	  GROUP_FILENAME, "/etc/ogroup", NULL, 0, 0, NULL,
	  opbase, clbase, rdgrpsids, NULL, NULL, rmrecord
	}
};

/********************************/
/* 	sysck file table	*/
/********************************/


struct 	attr	sysckatab [] =
{
	{ -1,		-1,		NULL,	    -1,      -1,  0,      nchk},
	{ IDX_CLASS,	S_SYSCK,	S_CLASS,    SEC_LIST, 0 , 0,      nchk},
	{ IDX_OWNER,	S_SYSCK,	S_OWNER,    SEC_CHAR, 0 , 0,      nchk},
	{ IDX_GROUP,	S_SYSCK,	S_GROUP,    SEC_CHAR, 0 , 0,      nchk},
	{ IDX_MODE,	S_SYSCK,	S_MODE,	    SEC_LIST, 0 , 0,      nchk},
	{ IDX_PROGRAM,	S_SYSCK,	S_PROGRAM,  SEC_LIST, 0 , 0,      nchk},
	{ IDX_TCB,	S_SYSCK,	S_TCB,	    SEC_BOOL, 0 , 0,      nchk},
	{ IDX_LINKS,	S_SYSCK,	S_LINKS,    SEC_LIST, 0 , 0,      nchk},
	{ IDX_SYMLINKS,	S_SYSCK,	S_SYMLINKS, SEC_LIST, 0 , 0,      nchk},
	{ IDX_TYPE,	S_SYSCK,	S_TYPE,	    SEC_CHAR, 0 , 0,      nchk},
	{ IDX_ACL,	S_SYSCK,	S_ACL,	    SEC_CHAR, 0 , QUOTES, nchk},
	{ IDX_PCL,	S_SYSCK,	S_PCL,	    SEC_CHAR, 0 , QUOTES, nchk},
	{ IDX_CHECKSUM,	S_SYSCK,	S_CHECKSUM, SEC_CHAR, 0 , QUOTES, nchk},
	{ IDX_SOURCE,	S_SYSCK,	S_SOURCE,   SEC_CHAR, 0 , QUOTES, nchk},
	{ IDX_SIZE,	S_SYSCK,	S_SIZE,	    SEC_INT,  0 , 0,      nchk},
	{ IDX_TARGET,	S_SYSCK,	S_TARGET,   SEC_CHAR, 0 , 0, 	  nchk},
	{ IDX_SLAST,	S_SYSCK,	"*",	    0,	      0 , 0,      nchk}
};

struct	atfile	sysckftab [] =
{
	{
	  SYSCK_FILENAME, "/etc/security/osysck.cfg", NULL, 0, 0, NULL,
	  opst, clst, rdst, wrname, chgstanza, rmrecord
	}
};

/********************************/
/*	port file table		*/
/********************************/

static	struct 	attr	portatab [] =
{
	{ -1,		-1,		NULL,		-1,        -1, 0, nchk},
	{ IDX_HERALD,	S_LOGINCFG,	S_HERALD,    SEC_CHAR, 0, QUOTES, nchk},
	{ IDX_SAKENABLED, S_LOGINCFG,	S_SAKENABLED,	SEC_BOOL,   0, 0, nchk},
	{ IDX_SYNONYM,	S_LOGINCFG,	S_SYNONYM,	SEC_LIST,   0, 0, nchk},
	{ IDX_PLOGTIMES, S_LOGINCFG,	S_LOGTIMES,	SEC_LIST,   0, 0, nchk},
	{ IDX_LOGDISABLE, S_LOGINCFG,	S_LOGDISABLE,	SEC_INT,    0, 0, nchk},
	{ IDX_LOGINTERVAL, S_LOGINCFG,	S_LOGINTERVAL,	SEC_INT,    0, 0, nchk},
	{ IDX_LOGREENABLE, S_LOGINCFG,	S_LOGREENABLE,	SEC_INT,    0, 0, nchk},
	{ IDX_LOGDELAY,	S_LOGINCFG,	S_LOGDELAY,	SEC_INT,    0, 0, nchk},
	{ IDX_LOCKTIME,	S_PORTLOG,	S_LOCKTIME,	SEC_INT,    0, 0, nchk},
	{ IDX_ULOGTIMES,S_PORTLOG,	S_ULOGTIMES,	SEC_LIST,   0, 0, nchk},
	/* 
	 * note: last entry is a wild card 
	 * (i.e. the caller can specify attribute name and type
	 */
	{ IDX_POLAST,	S_LOGINCFG,	"*",		0,	    0, 0, nchk}
};

static	struct	atfile	portftab [] =
{
	{ 
	  LOGIN_FILENAME, "/etc/security/ologin.cfg", NULL, 0, 0, NULL,
	  opst,   clst,   rdst, wrname, chgstanza, rmrecord
	},
	{
	  PORTLOG_FILENAME, "/etc/security/oportlog", NULL,0, 0, NULL,
	  opst,   clst,   rdst, wrname, chgstanza, rmrecord
	}
};

/********************************/
/* 	generic file table 	*/
/********************************/

static	struct 	attr	genatab [] =
{
	{-1,		-1,		NULL,		-1,	  -1, 0, nchk},
	{IDX_GENERIC,	S_GENERIC,	"",		0,	   0, 0, nchk},

	/* note: last entry is a wild card
	 * (i.e. the caller can specify attribute name and type
	 */
	{IDX_GENLAST,	S_GENERIC,	"*",		0,	   0, 0, nchk},
};

static	struct	atfile	genftab [] =
{
	{ 
	  "", "", NULL,0, 0, NULL,
	  opst, clst, rdst, wrname, chgstanza, rmrecord
	},
};

/****************************************/
/*	static pointers			*/
/****************************************/

static	struct ehead 	*userhead = NULL;
static	struct ehead 	*grouphead = NULL;
static	struct ehead 	*pwdhead = NULL;
static	struct ehead 	*sysckhead = NULL;
static	struct ehead	*porthead = NULL;
static	struct ehead	*generichead = NULL;
static	int		sessions = 0;
static	int		psessions = 0;

static	int	pwdnftab = sizeof (pwdftab) / sizeof (struct atfile);
static	int	pwdnattr = sizeof (pwdatab) / sizeof (struct attr);

static	int	groupnftab = sizeof (groupftab) / sizeof (struct atfile);
static	int	groupnattr = sizeof (groupatab) / sizeof (struct attr);

static	int	usernattr = sizeof (useratab) / sizeof (struct attr);
static	int	usernftab = sizeof (userftab) / sizeof (struct atfile);

static	int	syscknattr = sizeof (sysckatab) / sizeof (struct attr);
static	int	syscknftab = sizeof (sysckftab) / sizeof (struct atfile);

static	int	portnattr = sizeof (portatab) / sizeof (struct attr);
static	int	portnftab = sizeof (portftab) / sizeof (struct atfile);

static	int	gennattr = sizeof (genatab) / sizeof (struct attr);
static	int	gennftab = sizeof (genftab) / sizeof (struct atfile);

/*****************************************************************************/
