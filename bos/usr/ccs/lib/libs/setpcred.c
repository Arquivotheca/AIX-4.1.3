static char sccsid[] = "@(#)86	1.42.1.8  src/bos/usr/ccs/lib/libs/setpcred.c, libs, bos41J, 9509A_all 2/23/95 16:25:04";
/*
 * COMPONENT_NAME: (LIBS) security library functions
 *
 * FUNCTIONS: setpcred
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

#include <sys/types.h>	
#include <sys/errno.h>	
#include <sys/id.h>
#include <sys/priv.h>
#include <sys/audit.h>	
#include <sys/resource.h>	/* for RLIMIT_RSS etc. */
#include <sys/param.h>		/* for UBSIZE etc. */
#include <sys/time.h>		/* for timeval struct info etc. */
#include <userpw.h>		/* for PW_NAMELEN */
#include <usersec.h>
#include "libs.h"

/*
 *   Policy for setting User ids:
 *	ID_LOGIN requires setting ID_SAVED, ID_REAL and ID_EFFECTIVE
 *	ID_SAVED requires setting ID_REAL and ID_EFFECTIVE
 *	ID_REAL requires setting ID_EFFECTIVE
 *	ID_EFFECTIVE can be set independently
 */
#define	F_EFF_ID	(ID_EFFECTIVE) 
#define	F_REAL_ID	(ID_REAL | F_EFF_ID) 
#define	F_SAVED_ID	(ID_SAVED | F_REAL_ID)
#define	F_LOGIN_ID	(ID_LOGIN | F_SAVED_ID)

#define NO_PRIV		0	/* No priv required to set this attribute */

#define	ADD_PRIVILEGE(v,p)	(((p) <= 32) ? \
	(v.pv_priv[0] |= (1<<((p)-1))):\
	(v.pv_priv[1] |= (1<<((p)-33))))

/*
 * routines used to normalize values from 
 * caller strings to values in the database
 */
static  char	*normuid ();
static  char	*normrlim ();

/* set process credential routines */
static int	setprlim ();
static int	setpaudit ();
static int	setprocgid ();
static int	setpumask ();
static int	setpuid (); 
static int	setpgrps ();
static int	setpgrpsids ();
static char	*cnvtolist ();

/*
 * setpcred is driven from the following table.
 */
struct  cs
{
	unsigned short	flgs;		/* flags for value of this credential */
	void		*val;		/* value of credential from argument */
	char		*name;		/* credential name */
	char		*attr;		/* credential attribute name */
	int		type;		/* attribute type */
	int		size;		/* sizeof credential value */
	char		*(*norm) ();	/* normalize user's credentials */
	int		priv;		/* Privilege needed to set credential */
	int		(*setp) ();	/* set process credentials */
	int		flags;		/* flags to the setp() */
};

static struct cs cred [] =
{	
	{0,NULL,"GROUPSIDS",S_GROUPSIDS,SEC_LIST,S_NGROUPS,cnvtolist,
			SET_PROC_DAC,setpgrpsids,  0 },

	{0,NULL,"GROUPS",S_GROUPS,SEC_LIST,S_NGROUPS,cnvtolist,
			SET_PROC_DAC,setpgrps,  0 },

	{0,NULL,"REAL_GROUP",S_PGRP,0,1,'\0',
			SET_PROC_DAC,setprocgid,F_SAVED_ID},

	{0,NULL,"RLIMIT_FSIZE",S_UFSIZE,SEC_INT,1,normrlim,
			SET_PROC_RAC,setprlim,RLIMIT_FSIZE},

	{0,NULL,"RLIMIT_CPU",S_UCPU,SEC_INT,1,normrlim,
			SET_PROC_RAC,setprlim,RLIMIT_CPU},

	{0,NULL,"RLIMIT_DATA",S_UDATA,SEC_INT,1,normrlim,
			SET_PROC_RAC,setprlim,RLIMIT_DATA},

	{0,NULL,"RLIMIT_STACK",S_USTACK,SEC_INT,1,normrlim,
			SET_PROC_RAC,setprlim,RLIMIT_STACK},

	{0,NULL,"RLIMIT_CORE",S_UCORE,SEC_INT,1,normrlim,
			SET_PROC_RAC,setprlim,RLIMIT_CORE},

	{0,NULL,"RLIMIT_RSS",S_URSS,SEC_INT,1,normrlim,
			SET_PROC_RAC,setprlim,RLIMIT_RSS},

	{0,NULL,"UMASK",S_UMASK,SEC_INT,1,normrlim,
			NO_PRIV,setpumask,0},

	{0,NULL,"LOGIN_USER",S_ID,SEC_INT,1,normuid,
			SET_PROC_AUDIT,setpuid,F_LOGIN_ID },

	{0,NULL,"REAL_USER",S_ID,SEC_INT,1,normuid,
			SET_PROC_DAC,setpuid,F_REAL_ID },

	{0,NULL,"AUDIT_CLASSES",S_AUDITCLASSES,SEC_LIST,1,cnvtolist,
			SET_PROC_AUDIT,setpaudit,0 },
};

/*
 * The following defines are the indices into the above table for each
 * of the named credentials.  This set of defines must be kept in sync
 * with the above table or chaos ensues.
 */

#define CI_GROUPSIDS	0
#define	CI_GROUPS	1
#define	CI_REAL_GROUP	2
#define	CI_RLIMIT_FSIZE	3
#define	CI_RLIMIT_CPU	4
#define	CI_RLIMIT_DATA	5
#define	CI_RLIMIT_STACK	6
#define	CI_RLIMIT_CORE	7
#define	CI_RLIMIT_RSS	8
#define	CI_UMASK	9
#define	CI_LOGIN_USER	10
#define	CI_REAL_USER	11
#define	CI_AUDIT_CLASSES 12

extern  int 	errno; 
/*
 * setpcred 
 *                                                                    
 * FUNCTION: Set the security credentials of this process 
 *                                                                    
 * EXECUTION ENVIRONMENT: Under a process
 *                                                                   
 * NOTES: Setpcred is given a credentials vector (array of strings)
 *	        of the same format output by getpcred ().
 *		This is parsed for the users credentials. Missing
 *		credentials are filled in from the user database.
 *		The resulting credentials are set if the process
 *		has privilege.
 *
 * RETURNS: if successful returns 0 else returns -1 and errno is set
 *
 *		EINVAL	Neither parameter was specified or
 *			'user' does not have valid credentials
 *			'cred' parameter is incorrectly specified
 *		EACCES  Process does not have read permission for the
 *			specified credentials
 *		EPERM	Set when the process does not have the kernel priviledge
 *			required for the requested change as follows:
 *			change accounting id without 	 SET_PROC_ACCT
 *			change group or user ids without SET_PROC_DAC
 *			change to login id without 	 SYS_AUDIT 
 */  

int
setpcred (char *user,char **credentials)
{
	register int	i;
	register int	ncreds;
	char shortname[PW_NAMELEN];	/* Short user name (max 8 bytes) */

	/* at least one of the args has to be supplied */
	if (!(credentials || user))
	{
		errno = EINVAL;
		return (-1);
	}

	/* Initialize */
	ncreds = sizeof (cred)/sizeof (struct cs);
	for (i = 0; i < ncreds; i++)
	{
		cred[i].val = NULL;
		cred[i].flgs = NOT_FOUND;
	}

	/* parse specified credentials */
	if (credentials)
	{
	char	**credptr;		/* points to input credentials vector */
	char	*strstr (), *strchr ();
	char	*val;
	char	*ptr;
	int	gotit = 0;

		/* set temporary pointer */
		credptr = credentials;

		while (*credptr)
		{
			gotit = 0;
			i = 0;
			/* parse NAME=value (this assumes no spaces) */
			val = *credptr;
			ptr = val;
			while (*ptr && (*ptr != '='))
				ptr++;
			if (*ptr != '=')
			{
				errno = EINVAL;
				return(-1);
			}
			*ptr++ = '\0';
			while (i < ncreds)
			{
				if (!strcmp(val,cred[i].name))
				{
					gotit = 1;
					val = ptr;
					break;
				}
				i++;
			}

			if (!gotit) 	/* couldn't find that token */
			{
				credptr++;
				continue;  	/*  try another */
			}

			/* normalize user supplied credentials */
			if (cred[i].norm)
			{
				cred[i].val = (*(cred[i].norm))(val,&(cred[i]));
			}
			else
			{
				/* value is in expected form so just copy it */
				if((cred[i].val=malloc(strlen(val) + 1))==NULL)
				{
					return (-1);
				}
				strcpy (cred[i].val, val);
				cred[i].flgs |= DYNAMIC;
			}
			cred[i].flgs &= ~NOT_FOUND;
			credptr++;
		}
	}

	/* If user name is given then merge in values from the user db */
	if (user)
	{
		int	err;

		_normalize_username(shortname, user);

		/* get existing user credentials from user db */
		for (i = 0; i < ncreds; i++)
		{
			if (i != CI_GROUPS && cred[i].flgs & NOT_FOUND)
			{
				err = getuserattr(user,cred[i].attr,
					(void *) &(cred[i].val), cred[i].type);

				if (!err)
					cred[i].flgs &= ~NOT_FOUND;
				else
				if (i == CI_AUDIT_CLASSES)
				{
					cred[i].val = (void *) NULL;
					cred[i].flgs &= ~NOT_FOUND;
				}
			}
		}
	}

	/* Check privileges. If we cannot set them all don't set any */
	for (i = 0; i < ncreds; i++)
	{
		if ( !(cred[i].flgs & NOT_FOUND) &&
		      (cred[i].priv != NO_PRIV) )
		{
			if (privcheck (cred[i].priv))
			{
				errno = EPERM;
				return (-1);
			}
		}
	}

	/*
	 * Caveats:
	 *	Some of the fields have default values which must be
	 *	used if the value is not in the database.
	 */

	if (user)
	{
		/*
		 * umask is always set to something.  the value
		 * is the DECIMAL representation of the OCTAL value.
		 * thus, 022 is just plain 22.
		 */

		if (cred[CI_UMASK].flgs & NOT_FOUND) {
			cred[CI_UMASK].val = (void *) 22;	/* rwxr-xr-x */
			cred[CI_UMASK].flgs &= ~(NOT_FOUND|DYNAMIC);
		}

		/*
		 * concurrent group set must be set to something
		 * if there is no concurrent group set.
		 */

		if (cred[CI_GROUPSIDS].flgs & NOT_FOUND) {
			char	*pgrp;

			if (getuserattr (user, S_PGRP, (void **) &pgrp, 0))
				return -1;

			if ( ! (cred[CI_GROUPS].val =
			        (void *) malloc(strlen(pgrp) + 2) ) )
				return -1;
			memset (cred[CI_GROUPS].val, '\0', strlen (pgrp) + 2);
			strcpy ((char *) cred[CI_GROUPS].val, pgrp);
			cred[CI_GROUPS].flgs &= ~NOT_FOUND;
			cred[CI_GROUPS].flgs |= DYNAMIC;
		}
	}

	/* set process credentials */
	for (i = 0; i < ncreds; i++)
	{
		/* if no value or no set function then get next */
		if ((cred[i].flgs & NOT_FOUND) || !cred[i].setp)
		{
			continue;
		}
		/* call the set function */
		if ((*(cred[i].setp))(cred[i].val, cred[i].flags, cred[i].size)
				< 0)
		{
			return (-1);
		}
	}
	/* free all the 'cred[i].val' memory */
	for (i = 0; i < ncreds; i++)
	{
		if (cred[i].val && (cred[i].flgs & DYNAMIC))
		{
			free (cred[i].val);
		}
	}

	endpwent();

	/*
	 * Now I must reset as many of the privileges as are not
	 * needed by this process.  setpenv() will finish up with
	 * what was started here.  Only root is left with all
	 * privilege intact.
	 *
	 * The return status is not checked because I can always
	 * set the privilege sets to {}, and if SET_PROC_ENV isn't
	 * currently in the maximum set, it can't be in the effective
	 * set and this isn't an error - setpenv would fail anyhow.
	 */

	if (geteuid () != 0) {
		priv_t	priv;

		memset (&priv, 0, sizeof priv);
		(void) setpriv (PRIV_SET|PRIV_INHERITED|PRIV_BEQUEATH,
			&priv, sizeof priv);

		ADD_PRIVILEGE(priv, SET_PROC_ENV);
		ADD_PRIVILEGE(priv, TPATH_CONFIG);
		ADD_PRIVILEGE(priv, SET_PROC_AUDIT);

			/* need to get to /etc/security/environ later */
		ADD_PRIVILEGE(priv, BYPASS_DAC_READ);
		ADD_PRIVILEGE(priv, BYPASS_DAC_EXEC);

		(void) setpriv (PRIV_SET|PRIV_MAXIMUM|PRIV_EFFECTIVE,
			&priv, sizeof priv);
	}
	return (0);
}
/*
 * NAME: normuid
 *                                                                    
 * FUNCTION: convert the user's name to the user's id 
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: if successful a pointer to a uid, if fail a NULL pointer
 */  
static  char *
normuid (user,cs)
char	*user;
struct cs *cs;
{
	if (getuserattr (user, cs->attr,(void *) &(cs->val), SEC_INT))
	{
		cs->val = (char *) -2;
	}
	return (cs->val);
}
/*
 * NAME: setpuid
 *                                                                    
 * FUNCTION: set user's id 
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *			  flags specify the id to set
 *                                                                   
 * RETURNS: if successful 0, if fail -1 
 */  
static  int
setpuid (val,flags,siz)
uid_t	*val;
int	flags;
int	siz;
{
	if (setuidx (flags, val) < 0)
	{
		return (-1);
	}
	return (0);
}
/*
 * NAME: setprocgid
 *                                                                    
 * FUNCTION: set user's group id 
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *			  flags specify the id to set
 *                                                                   
 * RETURNS: if successful 0, if fail -1 
 */  
static  int
setprocgid (val,flags,siz)
char	*val;
int	flags;
int	siz;
{
gid_t	gid;
int	rc = 0;

	if (grouptoID(val, &gid) == 1)
	{
		if (setgidx (flags, gid) < 0)
		{
			rc = -1;
		}
	}
	else
		rc = -1;

	return (rc);
}
/*
 * NAME: setpgrps
 *                                                                    
 * FUNCTION: set user's concurrent group list
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * NOTES:
 *	There must be at least one group in the concurrent group set
 *	list.  If there isn't, the setgroups() call will fail to change
 *	the group set because it thinks you want to know how many groups
 *	are in the set.
 *
 * RETURNS: if successful 0, if fail -1 
 */  

/*ARGSUSED*/
static  int
setpgrps (val,flags,siz)
char	*val;
int	flags;
int	siz;
{
char	*p;
char	*ptr;
char	*group;
int	groups = 0;
gid_t	groupset[S_NGROUPS];
gid_t	gid;
int	i;

	/*
	 * Scan through the list of concurrent groups and add each
	 * to the group set.  Each numerical group ID is added at
	 * most once.  The resultant list is then made the active
	 * concurrent group set.
	 */

	while (*val && groups < S_NGROUPS) {

		/*
		 * Point to the name of the next group member to add
		 * to the group set.  Optionally check the name for
		 * conformance with the PW_NAMELEN restriction.
		 */

		group = val;
		while(*val++)
			;

#ifdef	_STRICT_NAMES
		if (strlen (group) > (PW_NAMELEN-1))
			continue;
#endif

		/*
		 * Convert the supplied name to a numerical GID and
		 * scan the list for the presense of this GID.  Each
		 * GID value appears exactly once in the group set
		 * to prevent strange behavior, notably with NIS.
		 */

		if (grouptoID (group, &gid) == 1) {
			for (i = 0;i < groups;i++)
				if (groupset[i] == gid)
					break;

			if (i == groups)
				groupset[groups++] = gid;
		}
	}

	/*
	 * Attempt to change the concurrent group set to the new values.
	 */

	if (setgroups (groups, groupset) < 0)
		return(-1);

	return (0);
}
/*
 * NAME: setpgrpsids
 *                                                                    
 * FUNCTION: set user's concurrent group list (by gid list)
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * NOTES:
 *	There must be at least one group in the concurrent group set
 *	list.  If there isn't, the setgroups() call will fail to change
 *	the group set because it thinks you want to know how many groups
 *	are in the set.
 *
 * RETURNS: if successful 0, if fail -1 
 */  

/*ARGSUSED*/
static  int
setpgrpsids (val,flags,siz)
char	*val;
int	flags;
int	siz;
{
char	*p;
char	*ptr;
char	*group;
int	groups = 0;
gid_t	groupset[S_NGROUPS];
gid_t	gid;
int	i;

	/*
	 * Scan through the list of concurrent groups and add each
	 * to the group set.  Each numerical group ID is added at
	 * most once.  The resultant list is then made the active
	 * concurrent group set.
	 */

	while (*val && groups < S_NGROUPS) {

		/*
		 * Point to the name of the next group member to add
		 * to the group set.  Optionally check the name for
		 * conformance with the PW_NAMELEN restriction.
		 */

		group = val;
		while(*val++)
			;

		/*
		 * Convert the string version of the gid to something
		 * numeric.  Each GID value appears exactly once in the
		 * group set to prevent strange behavior.
		 */

		gid = atol(group);
		for (i = 0;i < groups;i++)
			if (groupset[i] == gid)
				break;

		if (i == groups)
			groupset[groups++] = gid;
	}

	/*
	 * Attempt to change the concurrent group set to the new values.
	 */

	if (setgroups (groups, groupset) < 0)
		return(-1);

	return (0);
}

/*
 * NAME: cnvtolist
 *                                                                    
 * FUNCTION: convert a comma separated list to a null terminated buffer
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: if successful a pointer to the audit classes, if fail a NULL pointer
 */  
static char *
cnvtolist (val,cs)
char	*val;
struct cs *cs;
{
register char	*nbuf;	/* new buffer */
register char	*np;	/* new pointer */
register char	*vp;	/* temp pointer */
register int	siz;

	siz = strlen (val) + 2;
	if ((nbuf = malloc (siz)) == NULL)
		return (NULL);

	/* clear out buffer */
	memset (nbuf,0,siz);

	np = nbuf;
	for (vp = val; *vp; vp++)
	{
		if (*vp == ',')
		{
			*np++ = '\0';
			continue;
		}
		if (*vp == ' ')
		{
			*np++ = '\0';
			*np = '\0';
			break;
		}
		*np++ = *vp;
	}
	cs->flgs |= DYNAMIC;
	return (nbuf);
}
/*
 * NAME: setpaudit
 *                                                                    
 * FUNCTION: set user's audit classes 
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: if successful 0, if fail -1 
 */  
static 	int
setpaudit (val,flags,siz)
char	*val;
int	flags;
int	siz;
{
register char 	*p;	/* temp pointer */
register int	len;	/* len of double null terminated string */

	len = 0;
	p = val;
	if (*p != '\0')
	{
		while (*p)
		{
			while (*p++) 
				len++;
			len++;
		}
		len++;
	}
	if (auditproc (0, AUDIT_EVENTS, val, len) != 0)
	{
		return (-1);
	}
	/* alloc'd in cnvtolist() */
	return (0);
}
/*
 * NAME: setprlim
 *                                                                    
 * FUNCTION: set user's rlimits
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: if successful 0, if fail -1 
 */  
static int
setprlim (val,flags,siz)
int	val;
int	flags;
int	siz;
{
struct	rlimit	rlp;

	if (val <= 0)
		rlp.rlim_cur = rlp.rlim_max = RLIM_INFINITY;
	else
	{
		if ((flags == RLIMIT_STACK) && (val <= 128))
			return (-1);

		/* cpu time is in seconds so don't modify */
		if (flags != RLIMIT_CPU)
			rlp.rlim_cur = (UBSIZE * val); 
		else
			rlp.rlim_cur = val; 
	
		if (flags == RLIMIT_FSIZE || flags == RLIMIT_CPU)
			rlp.rlim_max = rlp.rlim_cur;
		else
			rlp.rlim_max = RLIM_INFINITY;
	}

	if (setrlimit(flags,&rlp))
		return (-1);

	return (0);
}

/*
 * NAME: normrlim
 *                                                                    
 * FUNCTION: convert the string limit to an int
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: if successful a pointer to the limit, if fail a NULL pointer
 */  
static  char *
normrlim (limit,cs)
char	*limit;
struct cs *cs;
{
char	*ptr;

	/* check for valid ints */
	ptr = limit;
	while (ptr && *ptr)
	{
		if (!isdigit(*ptr))
		{
			errno = EINVAL;
			return(NULL);
		}
		ptr++;
	}
	
	return ((char *)atoi(limit));
}

/*
 * NAME: setpumask
 *                                                                    
 * FUNCTION: set user's umask 
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: if successful 0, if fail -1 
 */  
static int
setpumask (val,flags,siz)
void	*val;
int	flags;
int	siz;
{
unsigned long	flag = 0;
register int	c;
char		*ptr;
char		buf[BUFSIZ];

	sprintf (buf,"%d",val);

	ptr = &buf[0];

	if (isdigit((int)*ptr))
	{
		while ( c = *ptr++ )
		{
			if (c >= '0' && c <= '7')
			{
				flag = (flag<<3) + (c-'0');
			}
			else
				return (0);
		}
	}
		
	/* this is usually set in .profile so we won't fail */
	umask ((mode_t) flag);

	return (0);
}
