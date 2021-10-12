static char sccsid[] = "@(#)87	1.14.1.4  src/bos/usr/ccs/lib/libs/getconfattr.c, libs, bos41J, 9512A_all 3/14/95 15:53:38";
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
 * (C) COPYRIGHT International Business Machines Corp. 1988,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <sys/types.h>
#include <sys/errno.h>
#include <usersec.h>
#include <userconf.h>
#include "libs.h"

/*
 * NAME: getconfattr
 *                                                                    
 * FUNCTION: get security configuration attributes
 *                                                                    
 * EXECUTION ENVIRONMENT: library for internal use by security routines
 *	  These routines serve as an interface to the security configuration
 *	  data which is located in different files.
 *                                                                   
 *	  The routines work like the getuserattr() routines. 
 *	  Most information pertaining to the attributes are stored in the
 *	  following table. However, file and stanza names are represented
 * 	  by modes and resolved in setconfattr().
 *
 * RETURNS:
 *	  errno is set and (-1) is returned on error.
 *	  0 indicates success.
 */  

/* get routines from configuration storage according to type */
static	void 	*getint ();	/* integer retrieval */
static  void	*getstr ();	/* string retrieval */
static  void	*getlist ();	/* list retrieval */

extern	int	errno;

struct	f
{
	AFILE_t	fhdle;		/* file handle */
	char	*fnam;		/* file name */
};

static	struct	f	ftab [] = 
{
	{ NULL,	NULL},
	{ NULL,	LOGIN_FILENAME},
	{ NULL,	USERDEF_FILENAME },
	{ NULL,	AUDIT_FILENAME},
	{ NULL,	SPWD_FILENAME}
};

/* file table indexes */
#define	JFILE	0	/* junk file not used */
#define	LFILE	1
#define	UFILE	2
#define	AFILE	3
#define	PFILE	4

/* flag definitions */
#define	CONF_AUD	0x1	/* the caller defines the attribute name */
#define	CONF_CACHED	0x2	/* attr value is in memory */
#define CONF_DYNAMIC 	0x4	/* attr value is dynamically allocated */
#define CONF_NOT_FOUND	0x8	/* attr value was not found */
#define	CONF_PROG	0x10	/* stanza name for authentication program */
#define	CONF_PWD	0x20	/* passwd attribute entrys */

struct	cf
{
	unsigned short	flgs;	/* flags for value */
	char	*val;		/* value for this attribute */
	int	spare;		/* spare */
	int	fidx;		/* file table index */
	ATTR_t  shdle;		/* stanza handle */
	char	*stnam;		/* stanza name */
	char	*atnam;		/* attribute name */
	void	*(*get)();	/* retrieval function */
};

static	struct	cf	confattrs [] =
{

	/* attributes associated with logins */
	{ 0, '\0', 0, LFILE, NULL, SC_SYS_LOGIN,SC_SHELLS,	getlist },
	{ 0, '\0', 0, LFILE, NULL, SC_SYS_LOGIN,SC_GECOS,	getlist },
	{ 0, '\0', 0, LFILE, NULL, SC_SYS_LOGIN,SC_MAXLOGINS,	getint 	},
	{ 0, '\0', 0, LFILE, NULL, SC_SYS_LOGIN,S_LOGTIMEOUT,	getint  },
	/* this attribute is for authentication methods */

	/* administrator default attributes */
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_UID,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_GROUP,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_GROUPS,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_HOME,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_PROG,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_GECOS,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_AUDIT,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_LOGINCHK,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_SUCHK,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_RLOGINCHK,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_TELNETCHK,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_DAEMONCHK,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_ADMIN,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_SUGROUPS,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_ADMGROUPS,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_TPATH,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_TTYS,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_EXPIRATION,getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_AUTH1, 	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_AUTH2, 	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_FSIZE,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_CPU,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_DATA,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_STACK,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_CORE,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_RSS,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_SYSENV,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_USRENV,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_UMASK,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_REGISTRY,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_AUTHSYSTEM,getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,S_LOGTIMES,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,S_LOCKED,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,S_LOGRETRIES,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_MINALPHA,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_MINOTHER,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_MINDIFF,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_MAXREPEAT,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_MINLEN,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_MINAGE,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_MAXAGE,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_MAXEXPIRED,getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_HISTEXPIRE,getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_HISTSIZE,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_PWDCHECKS,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_DICTION,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_PWDWARNTIME,getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_ADMUSER,SC_USREXPORT,	getstr },

	/* user default attributes */
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_UID,		getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_GROUP,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_GROUPS,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_HOME,		getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_GECOS,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_PROG,		getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_AUDIT,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_LOGINCHK,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_SUCHK,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_RLOGINCHK, 	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_TELNETCHK,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_DAEMONCHK,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_ADMIN,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_SUGROUPS,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_ADMGROUPS,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_TPATH,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_TTYS,		getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_EXPIRATION,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_AUTH1,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_AUTH2, 	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_FSIZE,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_CPU,		getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_DATA,		getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_STACK,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_RSS,		getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_CORE,		getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_USRENV,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_SYSENV,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_UMASK,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_REGISTRY,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_AUTHSYSTEM,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,S_LOGTIMES,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,S_LOCKED,	getstr },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,S_LOGRETRIES,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_MINALPHA,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_MINOTHER,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_MINDIFF,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_MAXREPEAT,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_MINLEN,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_MINAGE,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_MAXAGE,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_MAXEXPIRED,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_HISTEXPIRE,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_HISTSIZE,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_PWDCHECKS,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_DICTION,	getlist },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_PWDWARNTIME,	getint },
	{ 0, '\0', 0, UFILE, NULL, SC_SYS_USER,SC_USREXPORT,	getstr },

	{ CONF_PWD,'\0', 0, PFILE, NULL, "*",SC_LASTUPDATE,	getint},
	{ CONF_PWD,'\0', 0, PFILE, NULL, "*",SC_FLAGS,		getlist},

	/* attributes associated with login information */
	{ CONF_PROG,'\0', 0, LFILE, NULL, "*",SC_AUTHPROGRAM,	getstr},
	{ CONF_PROG,'\0', 0, LFILE, NULL, "*",SC_AUTHRETRY,	getint},
	{ CONF_PROG,'\0', 0, LFILE, NULL, "*",SC_AUTHTIMEOUT,	getint},
	{ CONF_PROG,'\0', 0, LFILE, NULL, "*",SC_AUTHRETRYDELAY,getint},

	/* attributes associated with audit information */
	{ CONF_AUD,'\0', 0, AFILE, NULL, SC_SYS_AUDIT,  "*",	getlist },
	{ CONF_AUD,'\0', 0, AFILE, NULL, SC_SYS_AUSERS, "*",	getlist },
	{ CONF_AUD,'\0', 0, AFILE, NULL, SC_SYS_ASYS,   "*",	getlist },
	{ CONF_AUD,'\0', 0, AFILE, NULL, SC_SYS_ABIN,   "*",	getlist },
	{ CONF_AUD,'\0', 0, AFILE, NULL, SC_SYS_ASTREAM,"*",	getlist }

};

static	int	nattr = sizeof (confattrs) / sizeof (struct cf);

static int
setconfattr (register struct cf *cf)
{
register struct	f  *f;

	f = &(ftab[cf->fidx]);
	if (f->fhdle == NULL)
	{
		if (!(f->fhdle = afopen (f->fnam)))
			return (-1);
	}
	cf->shdle = afgetrec (f->fhdle, cf->stnam);
	return (0);
}

int
endconfattr (void)
{
struct	cf	*cf;
struct	f	*f;

	for (cf = &confattrs[0]; cf < &confattrs[nattr]; cf++)
	{
		f = &(ftab[cf->fidx]);
		if (f->fhdle)
		{
			afclose (f->fhdle);
			f->fhdle = NULL;
		}
		if ((cf->flgs & CONF_DYNAMIC) && cf->val)
		{
			free ((void *)cf->val);
		}
		cf->flgs &= CONF_AUD;
	}
	return;
}

static	struct cf *
findconf (char *stanza, char *atnam)
{
struct	cf	*cf = NULL;

	for (cf = &confattrs[0]; cf < &confattrs[nattr]; cf++)
	{
		/* check for audit file entry */
		if (cf->flgs & CONF_AUD)
		{
			if (!strcmp(stanza,cf->stnam))
			{
				if((cf->flgs & CONF_DYNAMIC) && cf->val)
					free ((void *)cf->val);
				cf->atnam = atnam;
				cf->flgs = CONF_AUD;
				break;
			}
			else
				continue;
		}

		/* check for login file entry */
		if (cf->flgs & CONF_PROG)
		{
			if (!strcmp(atnam,cf->atnam))
			{
				if((cf->flgs & CONF_DYNAMIC) && cf->val)
					free ((void *)cf->val);
				cf->stnam = stanza;
				cf->flgs = CONF_PROG;
				break;
			}
			else
				continue;
		}
	
		/* check for passwd file entry */
		if (cf->flgs & CONF_PWD)
		{
			if (!strcmp(atnam,cf->atnam))
			{
				if((cf->flgs & CONF_DYNAMIC) && cf->val)
					free ((void *)cf->val);
				cf->stnam = stanza;
				cf->flgs = CONF_PWD;
				break;
			}
			else
				continue;
		}
		
		if (!strcmp(stanza,cf->stnam))
		{
			if (strcmp (atnam, cf->atnam) == 0)
				break;
		}
	}
	return ((cf == &confattrs[nattr]) ? NULL : cf);
}
/*
 * getconfattr
 *                                                                    
 * FUNCTION: get a configuration attribute
 *
 * EXECUTION ENVIRONMENT: 
 *	    
 * RETURNS:
 *		0 if successful 
 *		-1 if not found or an error occured 
 */  

int
getconfattr (char *sys, char *atnam, void *val,int type)
{
register struct	cf	*cf;

	if (cf = findconf (sys, atnam))
	{
		if (setconfattr (cf))
			return (-1);

		*(void **) val =  (*(cf->get))(cf);
		return (!(CONF_CACHED & cf->flgs));
	}
	errno = ENOENT;
	return (-1);
}
/*
 * getint
 *                                                                    
 * FUNCTION: Get an int from a configuration file
 *
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS:
 *	     the integer
 */  
static void *
getint (cf)
register struct	cf	*cf;
{
char	*a = afgetatr (cf->shdle, cf->atnam);

	if (a && *a)
	{
		cf->val = (char *) strtol (a, (char **) NULL, 0);
		cf->flgs |= CONF_CACHED;
	}
	return ((void *) cf->val);
}
/*
 * getstr
 *                                                                    
 * FUNCTION: Get a string from a configuration file
 *
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: NULL is not found else pointer to 
 *	malloc'd memory containing the string
 */  
static void *
getstr (cf)
register struct	cf	*cf;
{
char	*a = afgetatr (cf->shdle, cf->atnam);

	if (a && *a)
	{
		if ((cf->val = malloc (strlen (a) + 2)) != NULL)
		{
			strcpy (cf->val, a);
			*(cf->val + strlen (a) + 1) = '\0';
			cf->flgs |= CONF_DYNAMIC | CONF_CACHED;
		}
	}
	return ((void *) cf->val);
}
/*
 * getlist
 *                                                                    
 * FUNCTION: Get a list from a configuration file
 *
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: NULL is not found else pointer to 
 *	malloc'd memory containing the list
 */  
static void *
getlist (cf)
register struct	cf	*cf;
{
char	*a = afgetatr (cf->shdle, cf->atnam);

	if (a && *a)
	{
	int	len = 0;
	char	*p = a;

		while (*p)
		{
			while (*p)
			{
				p++; len++;
			}
			p++; len++;
		}
		len++;
		if ((cf->val = malloc (len)) != NULL)
		{
			memcpy (cf->val, a, len);
			cf->flgs |= CONF_DYNAMIC | CONF_CACHED;
		}
	}
	return ((void *) cf->val);
}
