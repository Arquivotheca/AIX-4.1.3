static char sccsid[] = "@(#)96	1.4  src/bos/usr/bin/chsec/chsecatab.c, cmdsuser, bos41J, 9512A_all 3/14/95 15:56:14";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: uexist, uexists, gexists, chkusw, chkmkdef, chkflag, chkflags,
 *		chgint, chghrld
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/errno.h>
#include <userconf.h>
#include <usersec.h>
#include "tcbauth.h"
#include "chsec.h"

#define ENVIRON 0
#define GROUPF 1
#define LASTLOGF 2
#define LIMITS 3
#define LOGINCFG 4
#define MKUSERDEF 5
#define PASSWDF 6
#define PORTLOG 7
#define USER 8

/*
 * File table listing the file names that can be handled by chsec/lssec, and
 * the audit record to be written when those files are modified by chsec.
 */

struct fileattr filetable[] =
{
	{ ENVIRON,   ENVFILE,                 CHUSRAUD  },
	{ GROUPF,    SGROUP,                  CHGRPAUD  },
	{ LASTLOGF,  LASTLOG,                 CHUSRAUD  },
	{ LIMITS,    LIMFILE,                 CHUSRAUD  },
	{ LOGINCFG,  LOGINFILE,               CHPORTAUD },
	{ MKUSERDEF, MKDEF,                   NULL      },
	{ PASSWDF,   SPASSWD,                 CHUSRAUD  },
	{ PORTLOG,   "/etc/security/portlog", CHPORTAUD },
	{ USER,      USERFILE,                CHUSRAUD  },

	/*
	 * Marker for the end of the table.
	 */
	{ 0,         0,                       0         }
};

/*
 * A table of all the attributes that can be handled by chsec/lssec.
 */

#define nchk 0

struct stanzaattr attrtable[] = {
/*
 * Attributes in the /etc/security/environ file.
 */
{ ENVIRON,   S_USRENV,       SEC_LIST, uexists,  strtolist,  chglist, QUOTE },
{ ENVIRON,   S_SYSENV,       SEC_LIST, uexists,  strtolist,  chglist, QUOTE },

/*
 * Attributes in the /etc/security/group file.
 */
{ GROUPF,    S_ADMIN,        SEC_BOOL, gexists,  chkbool,    chgbool, NONE  },
{ GROUPF,    S_ADMS,         SEC_LIST, gexists,  chknames,   chglist, NONE  },
{ GROUPF,    S_GRPEXPORT,    SEC_BOOL, gexists,  chkbool,    chgbool, NONE  },

/*
 * Attributes in the /etc/security/lastlog file.
 */
{ LASTLOGF,  S_LASTTIME,     SEC_INT,  uexists,  chkint,     chgint,  NONE  },
{ LASTLOGF,  S_LASTTTY,      SEC_CHAR, uexists,  nchk,       nchk,    NONE  },
{ LASTLOGF,  S_LASTHOST,     SEC_CHAR, uexists,  nchk,       nchk,    NONE  },
{ LASTLOGF,  S_ULASTTIME,    SEC_INT,  uexists,  chkint,     chgint,  NONE  },
{ LASTLOGF,  S_ULASTTTY,     SEC_CHAR, uexists,  nchk,       nchk,    NONE  },
{ LASTLOGF,  S_ULASTHOST,    SEC_CHAR, uexists,  nchk,       nchk,    NONE  },
{ LASTLOGF,  S_ULOGCNT,      SEC_INT,  uexists,  chkint,     chgint,  NONE  },

/*
 * Attributes in the /etc/security/limits file.
 */
{ LIMITS,    S_UFSIZE,       SEC_INT,  uexists,  chkulimit,  chgint,  NONE  },
{ LIMITS,    S_UCPU,         SEC_INT,  uexists,  chkint,     chgint,  NONE  },
{ LIMITS,    S_UDATA,        SEC_INT,  uexists,  chkdata,    chgint,  NONE  },
{ LIMITS,    S_USTACK,       SEC_INT,  uexists,  chkstack,   chgint,  NONE  },
{ LIMITS,    S_UCORE,        SEC_INT,  uexists,  chkint,     chgint,  NONE  },
{ LIMITS,    S_URSS,         SEC_INT,  uexists,  chkint,     chgint,  NONE  },

/*
 * Attributes in the /etc/security/login.cfg file.
 */
{ LOGINCFG,  S_HERALD,       SEC_CHAR, nchk,     nchk,       chghrld, QUOTE },
{ LOGINCFG,  S_SAKENABLED,   SEC_BOOL, nchk,     chkbool,    chgbool, NONE  },
{ LOGINCFG,  S_SYNONYM,      SEC_LIST, nchk,     strtolist,  chglist, NONE  },
{ LOGINCFG,  S_LOGTIMES,     SEC_LIST, nchk,     _usertodb, _dbtouser,NONE  },
{ LOGINCFG,  S_LOGDISABLE,   SEC_INT,  nchk,     chkint,     chgint,  NONE  },
{ LOGINCFG,  S_LOGINTERVAL,  SEC_INT,  nchk,     chkint,     chgint,  NONE  },
{ LOGINCFG,  S_LOGREENABLE,  SEC_INT,  nchk,     chkint,     chgint,  NONE  },
{ LOGINCFG,  S_LOGDELAY,     SEC_INT,  nchk,     chkint,     chgint,  NONE  },
{ LOGINCFG,  "port_locked",  SEC_BOOL, nchk,     chklock,    chglock, FAKE  },
{ LOGINCFG,  SC_AUTHPROGRAM, SEC_CHAR, nchk,     nchk,       nchk,    NONE  },
{ LOGINCFG,  SC_AUTHRETRY,   SEC_INT,  nchk,     chkint,     chgint,  NONE  },
{ LOGINCFG,  SC_AUTHTIMEOUT, SEC_INT,  nchk,     chkint,     chgint,  NONE  },
{ LOGINCFG,  SC_AUTHRETRYDELAY,SEC_INT,nchk,     chkint,     chgint,  NONE  },
{ LOGINCFG,  SC_SHELLS,      SEC_LIST, chkusw,   strtolist,  chglist, NONE  },
{ LOGINCFG,  SC_MAXLOGINS,   SEC_INT,  chkusw,   chkint,     chgint,  NONE  },
{ LOGINCFG,  S_LOGTIMEOUT,   SEC_INT,  chkusw,   chkint,     chgint,  NONE  },

/*
 * Attributes of the /usr/lib/security/mkuser.default file.
 */
{ MKUSERDEF, S_PGRP,         SEC_CHAR, chkmkdef, chkgrp,     nchk,    NONE  },
{ MKUSERDEF, S_GROUPS,       SEC_LIST, chkmkdef, chkgrps,    chglist, NONE  },
{ MKUSERDEF, S_AUDITCLASSES, SEC_LIST, chkmkdef, chkaudit,   chglist, NONE  },
{ MKUSERDEF, S_AUTH1,        SEC_LIST, chkmkdef, strtolist,  chglist, NONE  },
{ MKUSERDEF, S_AUTH2,        SEC_LIST, chkmkdef, strtolist,  chglist, NONE  },
{ MKUSERDEF, S_SHELL,        SEC_CHAR, chkmkdef, chkprog,    nchk,    NONE  },
{ MKUSERDEF, S_GECOS,        SEC_CHAR, chkmkdef, chkgek,     nchk,    NONE  },
{ MKUSERDEF, S_USRENV,       SEC_LIST, chkmkdef, strtolist,  chglist, QUOTE },
{ MKUSERDEF, S_SYSENV,       SEC_LIST, chkmkdef, strtolist,  chglist, QUOTE },
{ MKUSERDEF, S_LOGINCHK,     SEC_BOOL, chkmkdef, chkbool,    chgbool, NONE  },
{ MKUSERDEF, S_SUCHK,        SEC_BOOL, chkmkdef, chkbool,    chgbool, NONE  },
{ MKUSERDEF, S_RLOGINCHK,    SEC_BOOL, chkmkdef, chkbool,    chgbool, NONE  },
{ MKUSERDEF, S_DAEMONCHK,    SEC_BOOL, chkmkdef, chkbool,    chgbool, NONE  },
{ MKUSERDEF, S_TPATH,        SEC_CHAR, chkmkdef, chktpath,   nchk,    NONE  },
{ MKUSERDEF, S_TTYS,         SEC_LIST, chkmkdef, chkttys,    chglist, NONE  },
{ MKUSERDEF, S_SUGROUPS,     SEC_LIST, chkmkdef, chkgrps,    chglist, NONE  },
{ MKUSERDEF, S_ADMGROUPS,    SEC_LIST, chkmkdef, chkgrps,    chglist, NONE  },
{ MKUSERDEF, S_EXPIRATION,   SEC_CHAR, chkmkdef, chkexpires, nchk,    NONE  },
{ MKUSERDEF, S_UFSIZE,       SEC_INT,  chkmkdef, chkulimit,  chgint,  NONE  },
{ MKUSERDEF, S_UCPU,         SEC_INT,  chkmkdef, chkint,     chgint,  NONE  },
{ MKUSERDEF, S_UDATA,        SEC_INT,  chkmkdef, chkdata,    chgint,  NONE  },
{ MKUSERDEF, S_USTACK,       SEC_INT,  chkmkdef, chkstack,   chgint,  NONE  },
{ MKUSERDEF, S_UCORE,        SEC_INT,  chkmkdef, chkint,     chgint,  NONE  },
{ MKUSERDEF, S_URSS,         SEC_INT,  chkmkdef, chkint,     chgint,  NONE  },
{ MKUSERDEF, S_UMASK,        SEC_INT,  chkmkdef, chkumask,   chgint,  NONE  },
{ MKUSERDEF, S_LOGTIMES,     SEC_LIST, chkmkdef, _usertodb, _dbtouser,NONE  },
{ MKUSERDEF, S_LOCKED,       SEC_BOOL, chkmkdef, chkbool,    chgbool, NONE  },
{ MKUSERDEF, S_LOGRETRIES,   SEC_INT,  chkmkdef, chkint,     chgint,  NONE  },
{ MKUSERDEF, S_PWDWARNTIME,  SEC_INT,  chkmkdef, chkint,     chgint,  NONE  },
{ MKUSERDEF, S_AUTHSYSTEM,   SEC_CHAR, chkmkdef, chkauthsystem,nchk,  QUOTE },
{ MKUSERDEF, S_REGISTRY,     SEC_CHAR, chkmkdef, chkregistry,nchk,    NONE  },
{ MKUSERDEF, S_MINAGE,       SEC_INT,  chkmkdef, chkminage,  chgint,  NONE  },
{ MKUSERDEF, S_MAXAGE,       SEC_INT,  chkmkdef, chkmaxage,  chgint,  NONE  },
{ MKUSERDEF, S_MAXEXPIRED,   SEC_INT,  chkmkdef, chkmaxexpired,chgint,NONE  },
{ MKUSERDEF, S_MINALPHA,     SEC_INT,  chkmkdef, chkminalpha, chgint,  NONE  },
{ MKUSERDEF, S_MINOTHER,     SEC_INT,  chkmkdef, chkminother,chgint,  NONE  },
{ MKUSERDEF, S_MINDIFF,      SEC_INT,  chkmkdef, chkmindiff, chgint,  NONE  },
{ MKUSERDEF, S_MAXREPEAT,    SEC_INT,  chkmkdef, chkmaxrepeats,chgint,NONE  },
{ MKUSERDEF, S_MINLEN,       SEC_INT,  chkmkdef, chkminlen,  chgint,  NONE  },
{ MKUSERDEF, S_HISTEXPIRE,   SEC_INT,  chkmkdef, chkhistexpire,chgint,NONE  },
{ MKUSERDEF, S_HISTSIZE,     SEC_INT,  chkmkdef, chkhistsize,chgint,  NONE  },
{ MKUSERDEF, S_PWDCHECKS,    SEC_LIST, chkmkdef, chkpwdchecks,chglist,NONE  },
{ MKUSERDEF, S_DICTION,      SEC_LIST, chkmkdef, chkdictionlist,chglist,NONE  },
{ MKUSERDEF, S_USREXPORT,    SEC_BOOL, chkmkdef, chkbool,    chgbool, NONE  },

/*
 * Attributes in the /etc/security/passwd file.
 */
{ PASSWDF,   SEC_LASTUP,     SEC_INT,  uexist,   chkint,     chgint,  NONE  },
{ PASSWDF,   SEC_FLAGS,      SEC_LIST, uexist,   chkflags,   chglist, NONE  },

/*
 * Attributes in the /etc/security/portlog file.
 */
{ PORTLOG,   S_LOCKTIME,     SEC_INT,  nchk,     chkint,     chgint,  NONE  },
{ PORTLOG,   S_ULOGTIMES,    SEC_LIST, nchk,     strtolist,  chglist, NONE  },

/*
 * Attributes in the /etc/security/user file.
 */
{ USER,      S_LOGINCHK,     SEC_BOOL, uexists,  chkbool,    chgbool, NONE  },
{ USER,      S_SUCHK,        SEC_BOOL, uexists,  chkbool,    chgbool, NONE  },
{ USER,      S_RLOGINCHK,    SEC_BOOL, uexists,  chkbool,    chgbool, NONE  },
{ USER,      S_DAEMONCHK,    SEC_BOOL, uexists,  chkbool,    chgbool, NONE  },
{ USER,      S_ADMIN,        SEC_BOOL, uexists,  chkbool,    chgbool, NONE  },
{ USER,      S_SUGROUPS,     SEC_LIST, uexists,  chkgrps,    chglist, NONE  },
{ USER,      S_ADMGROUPS,    SEC_LIST, uexists,  chkgrps,    chglist, NONE  },
{ USER,      S_TPATH,        SEC_CHAR, uexists,  chktpath,   nchk,    NONE  },
{ USER,      S_TTYS,         SEC_LIST, uexists,  chkttys,    chglist, NONE  },
{ USER,      S_EXPIRATION,   SEC_CHAR, uexists,  chkexpires, nchk,    NONE  },
{ USER,      S_AUTH1,        SEC_LIST, uexists,  strtolist,  chglist, NONE  },
{ USER,      S_AUTH2,        SEC_LIST, uexists,  strtolist,  chglist, NONE  },
{ USER,      S_UMASK,        SEC_INT,  uexists,  chkumask,   chgint,  NONE  },
{ USER,      S_LOGTIMES,     SEC_LIST, uexists,  _usertodb, _dbtouser,NONE  },
{ USER,      S_LOGRETRIES,   SEC_INT,  uexists,  chkint,     chgint,  NONE  },
{ USER,      S_PWDWARNTIME,  SEC_INT,  uexists,  chkint,     chgint,  NONE  },
{ USER,      S_LOCKED,       SEC_BOOL, uexists,  chkbool,    chgbool, NONE  },
{ USER,      S_AUTHSYSTEM,   SEC_CHAR, uexists,  chkauthsystem,nchk,  QUOTE },
{ USER,      S_REGISTRY,     SEC_CHAR, uexists,  chkregistry,nchk,    NONE  },
{ USER,      S_MINAGE,       SEC_INT,  uexists,  chkminage,  chgint,  NONE  },
{ USER,      S_MAXAGE,       SEC_INT,  uexists,  chkmaxage,  chgint,  NONE  },
{ USER,      S_MAXEXPIRED,   SEC_INT,  uexists,  chkmaxexpired,chgint,NONE  },
{ USER,      S_MINALPHA,     SEC_INT,  uexists,  chkminalpha, chgint,  NONE  },
{ USER,      S_MINOTHER,     SEC_INT,  uexists,  chkminother,chgint,  NONE  },
{ USER,      S_MINDIFF,      SEC_INT,  uexists,  chkmindiff, chgint,  NONE  },
{ USER,      S_MAXREPEAT,    SEC_INT,  uexists,  chkmaxrepeats,chgint,NONE  },
{ USER,      S_MINLEN,       SEC_INT,  uexists,  chkminlen,  chgint,  NONE  },
{ USER,      S_HISTEXPIRE,   SEC_INT,  uexists,  chkhistexpire,chgint,NONE  },
{ USER,      S_HISTSIZE,     SEC_INT,  uexists,  chkhistsize,chgint,  NONE  },
{ USER,      S_PWDCHECKS,    SEC_LIST, uexists,  chkpwdchecks,chglist,NONE  },
{ USER,      S_DICTION,      SEC_LIST, uexists,  chkdictionlist,chglist,NONE  },
{ USER,      S_USREXPORT,    SEC_BOOL, uexists,  chkbool,    chgbool, NONE  },

/*
 * Marker for the end of the table.
 */
{ 0,         0,              0,        0,        0,          0,       0 }
};

/*
 * NAME: uexist
 *
 * FUNCTION: Determine if the user name exists.
 *
 * EXECUTION ENVIRONMENT:
 *	User process.
 *
 * RETURNS:
 *	1 if the user name is valid, 0 otherwise.
 */

int
uexist(char *user)
{
        int x;

        if(!getuserattr(user, S_ID, &x, SEC_INT))
                return(1);
        return(0);
}

/*
 * NAME: uexists
 *
 * FUNCTION: Determine if the user name exists or is "default".
 *
 * EXECUTION ENVIRONMENT:
 *	User process.
 *
 * RETURNS:
 *	1 if the user name is valid, 0 otherwise.
 */

int
uexists(char *user)
{
        int x;

        if(!strcmp(user, "default"))
                return(1);
        if(!getuserattr(user, S_ID, &x, SEC_INT))
                return(1);
        return(0);
}

/*
 * NAME: gexists
 *
 * FUNCTION: Determine if the group name exists or is "default".
 *
 * EXECUTION ENVIRONMENT:
 *	User process.
 *
 * RETURNS:
 *	1 if the group name is valid, 0 otherwise.
 */

int
gexists(char *group)
{
        int x;

        if(!strcmp(group, "default"))
                return(1);
        if(!getgroupattr(group, S_ID, &x, SEC_INT))
                return(1);
        return(0);
}

/*
 * NAME: chkusw
 *
 * FUNCTION: Determine if the stanza name is "usw".
 *
 * EXECUTION ENVIRONMENT:
 *	User process.
 *
 * RETURNS:
 *	1 if the stanza name is "usw", 0 otherwise.
 */

int
chkusw(char *stanza)
{
	return(!strcmp(stanza, SC_SYS_LOGIN));
}

/*
 * NAME: chkmkdef
 *
 * FUNCTION: Determine if the stanza name is "user" or "admin".
 *
 * EXECUTION ENVIRONMENT:
 *	User process.
 *
 * RETURNS:
 *	1 if the stanza name is valid, 0 otherwise.
 */

int
chkmkdef(char *stanza)
{
	if(!strcmp(stanza, SC_SYS_USER))
		return(1);
	if(!strcmp(stanza, SC_SYS_ADMUSER))
		return(1);
	return(0);
}

/*
 * NAME: chkflag
 *
 * FUNCTION: Determine if the password flag is "ADMIN", "ADMCHG", or "NOCHECK".
 *
 * EXECUTION ENVIRONMENT:
 *	User process.
 *
 * RETURNS:
 *	0 if the password flag is valid, non-zero otherwise.
 */

int
chkflag(char *flg)
{
	return(strcmp(flg, "ADMIN") && strcmp(flg, "ADMCHG") &&
	       strcmp(flg, "NOCHECK"));
}

/*
 * NAME: chkflags
 *
 * FUNCTION: Determine if the user password attribute "flags" is valid.
 *
 * EXECUTION ENVIRONMENT:
 *	User process.
 *
 * RETURNS:
 *	0 if the attribute is valid, non-zero otherwise.
 */

int
chkflags(char *val, char **ret)
{
	int rc = 0;
	char *tmp;

	*ret = malloc(strlen(val) + 2);
	if (!*ret)
		return ENOMEM;
	strcpy(*ret, val);
	(*ret)[strlen(val) + 1] = '\0';
	while(val && *val)
	{
		tmp = strchr(val, ',');
		if(tmp)
		{
			*tmp = '\0';
			rc |= chkflag(val);
			*tmp = ',';
			val = tmp + 1;
		}
		else
		{
			rc |= chkflag(val);
			val += strlen(val);
		}
	}
        return(rc);
}

/*
 * NAME: chgint
 *
 * FUNCTION: Converts an integer for output to the user.
 *
 * EXECUTION ENVIRONMENT:
 *	User process.
 *
 * RETURNS:
 *	A string containing the ascii version of the integer.
 */

char *
chgint(char *name, char *val)
{
	long num;
	char *ret;

	num = (long)val;

	ret = (char *)malloc(16);
	if(!ret)
		return "";

	sprintf(ret, "%ld", num);

	return(ret);
}

/*
 * NAME: chghrld
 *
 * FUNCTION: Converts the database version of the herald to human readable
 *	     form.  New line is converted to "\n" and tab is converted to "\t".
 *
 * EXECUTION ENVIRONMENT:
 *	User process.
 *
 * RETURNS:
 *	A string containing the login herald.
 */

char *
chghrld(char *name, char *val)
{
	char *ret, *ptr1, *ptr2;

	ptr1 = val;
	ptr2 = ret = (char *)malloc((strlen(val) << 1) + 1);
	if(!ret)
		return "";

	while(*ptr1)
	{
		if(*ptr1 == '\n')
		{
			*ptr2++ = '\\';
			*ptr2++ = 'n';
		}
		else if(*ptr1 == '\r')
		{
			*ptr2++ = '\\';
			*ptr2++ = 'r';
		}
		else if (*ptr1 == '\t')
		{
			*ptr2++ = '\\';
			*ptr2++ = 't';
		}
		else
			*ptr2++ = *ptr1;
		ptr1++;
	}
	*ptr2++ = '\0';

	return(ret);
}

/*
 * NAME: chklock
 *
 * FUNCTION: Converts the value of the pseudo-attribute "port_locked" to either
 *	     0 or the current time and places it in the database as the
 *	     "locktime" attribute.
 *
 * EXECUTION ENVIRONMENT:
 *	User process.
 *
 * RETURNS:
 *	0 for success and 1 for failure.
 */

int
chklock(char *name, char *val)
{
	int i, rc;
	char buf[16];

	/*
	 * See if the value matches based on the current language.
	 */
	rc = rpmatch(val);

	/*
	 * If the value doesn't match, then try the default English values.
	 */
	if(rc == -1)
	{
		/*
		 * Convert the given value to lower case.
		 */
		for(i = 0; val[i] && (i < 15); i++)
			buf[i] = tolower(val[i]);
		buf[i] = '\0';

		/*
		 * Check against the positive values.
		 */
		if(!strcmp(buf, "true") || !strcmp(buf, "yes") ||
		   !strcmp(buf, "always"))
			rc = 1;
		else if(!strcmp(buf, "false") || !strcmp(buf, "no") ||
			!strcmp(buf, "never"))
			rc = 0;
		else
			rc = -1;
	}

	/*
	 * If the value is affirmative, then set the lock time to the current
	 * time.
	 */
	if(rc == 1)
	{
		setuserdb(S_READ | S_WRITE);
		if(putportattr(name, S_LOCKTIME, -1, SEC_INT) ||
		   putportattr(name, 0, 0, SEC_COMMIT))
		{
			enduserdb();
			return(1);
		}
		else
		{
			enduserdb();
			return(0);
		}
	}

	/*
	 * If the value is negative, then set the lock time to zero.
	 */
	if(rc == 0)
	{
		setuserdb(S_READ | S_WRITE);
		if(putportattr(name, S_LOCKTIME, 0, SEC_INT) ||
		   putportattr(name, 0, 0, SEC_COMMIT))
		{
			enduserdb();
			return(1);
		}
		else
		{
			enduserdb();
			return(0);
		}
	}

	/*
	 * Value is invalid; return an error.
	 */
	return(1);
}

/*
 * NAME: chglock
 *
 * FUNCTION: Computes the value of the pseudo-attribute "port_locked" based on
 *	     the information in the port database.
 *
 * EXECUTION ENVIRONMENT:
 *	User process.
 *
 * RETURNS:
 *	0 for success and 1 for failure.
 */

char *chglock(char *port, char *val)
{
	long locktime, interval;

	setuserdb(S_READ);
	if(getportattr(port, S_LOCKTIME, &locktime, SEC_INT))
		locktime = 0;
	enduserdb();

	if(locktime == 0)
		return("false");
	else if(locktime == -1)
		return("true");
	else
	{
		setuserdb(S_READ);
		if(getportattr(port, S_LOGREENABLE, &interval, SEC_INT))
			interval = 0;
		enduserdb();

		if(interval != 0)
		{
			if(locktime > (time(0) - interval * 60))
				return("true");
			else
				return("false");
		}
		else
			return("true");
	}
}

