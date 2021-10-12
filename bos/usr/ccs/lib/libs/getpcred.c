static char sccsid[] = "@(#)83	1.17.1.3  src/bos/usr/ccs/lib/libs/getpcred.c, libs, bos41J, 9516A_all 11/9/94 15:55:28";
/*
 * COMPONENT_NAME: (LIBS) Security Library Functions 
 *
 * FUNCTIONS: getpcred
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/audit.h>
#include <sys/id.h>
#include <sys/priv.h>
#include <sys/resource.h>	/* for getrlim() */
#include <usersec.h>
#include <sys/param.h>		/* for UBSIZE */

/*
 * NAME: getpcred
 *                                                                    
 * FUNCTION: Get the process credentials that are specified by the
 *		parmeter 'which'. If 'which' is NULL returns all the
 *		credentials.
 *                                                                    
 * EXECUTION ENVIRONMENT: Under a process.
 *
 * RETURNS: Credentials are returned in allocated memory
 *		pointed to by a static pointer (a static array of
 * 		static pointers.
 *		The caller can and should do a free on these pointers.
 *		If not a subsequent call will realloc the memory.
 *
 *	    errno is set to EINVAL if 'which' parameter is invalid.
 */  

#define	PCREDSIZ 1000 /*initial size of pcred array. We'll get more if needed */

static	char	**pcred = (char **)NULL;  /* string to return to caller      */
static	int	pcredsiz = -1;		  /* dynamic size of return string   */
static	int	argcount = 0;		  /* count of credentials for vector */

static	int	_getpuid  (/* char *, int */);	/* get process user ids */
static	int	_getpgid  (/* char *, int */);	/* get process group id */
static  int	_getpgrps (/* char *, int */);	/* get process group set */
static  int	_getpaud  (/* char *, int */);	/* get process audit classes */
static  int	_getpulim (/* char *, int */);	/* get process ulimit */
static  int	_getpumsk (/* char *, int */);	/* get process umask */
static  int	_buildvec (/* char *, ... */);	/* VARARGS */

struct	cs
{
	int	type;		/* type associated with this credential */
	char	*attr;		/* string attr associated with credential */
	int	(*getp)();	/* function to get this credential */
	int	flags;		/* flags used by get function */
	int	priv;		/* the priv needed to read this credential */
};

#define NO_PRIV_REQ	0xFFFF  /* no priv required to read this credential */

static	struct	cs	cred [] = 
{
	{ CRED_RUID,    "REAL_USER",	_getpuid,  ID_REAL, NO_PRIV_REQ },
	{ CRED_LUID,    "LOGIN_USER",	_getpuid,  ID_LOGIN,NO_PRIV_REQ },
	{ CRED_RGID,	"REAL_GROUP",	_getpgid,  ID_REAL, NO_PRIV_REQ },
	{ CRED_GROUPS,	"GROUPS",	_getpgrps, 0,	   NO_PRIV_REQ },
	{ CRED_AUDIT,	"AUDIT_CLASSES",_getpaud,  0,	   SET_PROC_AUDIT },
	{ CRED_RLIMITS,	"ULIMIT",	_getpulim, 0,	   NO_PRIV_REQ },
	{ CRED_UMASK,	"UMASK",	_getpumsk, 0,	   NO_PRIV_REQ }
};

struct	lm
{
	int	type;	/* type of limit */
	char	*attr;	/* string value of attribute */
};

static	struct lm	limits[] = 
{
	{ RLIMIT_CPU , "RLIMIT_CPU"} , 
	{ RLIMIT_FSIZE , "RLIMIT_FSIZE"} , 
	{ RLIMIT_DATA , "RLIMIT_DATA"} ,
	{ RLIMIT_STACK , "RLIMIT_STACK"} , 
	{ RLIMIT_CORE , "RLIMIT_CORE"} , 
	{ RLIMIT_RSS , "RLIMIT_RSS"}
};

extern	char	*malloc(), *realloc();
extern  int errno;

char	**
getpcred (int which)
{
int	gotit = 0;	/* flag for valid which */
int	i;
int	ncreds;

	/* Initialize */
	pcred = (char **)NULL;		/* the pcred array 	*/
	pcredsiz = -1;			/* size of the array 	*/
	argcount = 0;			/* count of pcred args	*/
	

	ncreds = sizeof (cred)/sizeof (struct cs);

	/* Get all the credentials */
	if (which == (int)NULL)
	{
		which = 0xFFFF;
	}

	/* Check privileges for all requests */
	for (i = 0; i < ncreds; i++)
	{
		if (which & cred[i].type)
		{
			gotit = 1;
			if (cred[i].priv != NO_PRIV_REQ)
			{
				if (privcheck(cred[i].priv))
				{
					errno = EPERM;
					return (NULL);
				}
			}
		}
	}

	/* if not found in above for loop "which" parameter must be bad */
	if (!gotit)
	{
		errno = EINVAL;
		return (NULL);
	}

	/* Get each id requested */
	for (i = 0; i < ncreds; i++)
	{
		if (which & cred[i].type)
		{
			if ((*(cred[i].getp)) (cred[i].attr, cred[i].flags) < 0)
			{
				return (NULL);
			}
		}
	}

	/* terminate the pcred array with a null pointer */
	pcred[argcount] = NULL;

	return (pcred);
}
/*
 * _getpuid
 *                                                                    
 * FUNCTION: Get the process user id as specified
 *
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: -1 if error, 0 if successful
 */  
static	int
_getpuid (ap, flgs)
char	*ap;		/* attribute string */
int	flgs;		/* for getuidx func */
{
uid_t	id, getuidx();
char	*name;
		
	/*
	 * no check on return code because of lint
	 * this system call is supposed to return an unsigned long
	 * which can never be -1
	 */

	id = getuidx (flgs);

	if ((name = (char *)IDtouser((uid_t) id)) == NULL)
		return(-1);

	if (_buildvec ("%s=%s", ap, name) < 0)	
		return (-1);

	return (0);
}
/*
 * _getpgid
 *                                                                    
 * FUNCTION: Get the process user group id
 *
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: -1 if error, 0 if successful
 */  
static	int
_getpgid (ap, flgs)
char	*ap;		/* attribute string */
int	flgs;		/* for getgidx func */
{
gid_t	gid, getgidx ();
char	*name;
		
	/*
	 * no check on return code because of lint
	 * this system call is supposed to return an unsigned long
	 * which can never be -1
	 */

	gid = getgidx (flgs);

	if ((name = (char *)IDtogroup((gid_t) gid)) == NULL)
		return(-1);

	if (_buildvec ("%s=%s", ap, name) < 0)	
		return (-1);

	return (0);
}
/*
 * _getpaud
 *                                                                    
 * FUNCTION: Get the processes audit classes 
 *
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: -1 if error, 0 if successful
 */  
static	int
_getpaud (ap, flgs)
char	*ap;		/* attribute string */
int	flgs;		/* not used */
{
int	siz = BUFSIZ;	/* siz of buffer to pass to auditproc()*/
char	*buf = NULL;	/* pointer to buffer to pass ... */

	/* get a buffer */
	if ((buf = malloc (siz)) == NULL)
	{
		return (-1);
	}
	/* get audit events from kernel */
	while (auditproc (0, AUDIT_QEVENTS, buf, siz) < 0)
	{
		if (errno == ENOMEM)
		{
			/* get bigger buffer */
			siz = siz + BUFSIZ;
			if ((buf = realloc (buf, siz)) == NULL)
			{
				return (-1);
			}
		}
		else
			return (-1);
	}
	/* if any audit classes ... */
	if (strlen (buf))
	{
	char	*p;

		/* 
		 * replace NULLs with commas and 
		 * first of the ending-double-NULL with a space 
		 */
		p = buf;
		while (1)
		{
			while (*p++) ;
			if (*p)
				/*
				 * Previous character was a null, change
				 * it to a comma and continue
				 */
				*(p-1) = ',';
			else
			{
				*(p-1) = ' ';
				break;
			}
		}
		/* add to the credentials */
		if (_buildvec ("%s=%s", ap, buf) < 0)
			return (-1);
	}
	else
	{
		if (_buildvec ("%s=",ap) < 0)
			return (-1);
	}

	/* free and return */
	free (buf);
	return (0);
}
/*
 * _getpulim
 *                                                                    
 * FUNCTION: Get the process ulimit 
 *
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: -1 if error, 0 if successful
 */  
static	int
_getpulim (ap, flgs)
char	*ap;		/* attribute string */
int	flgs;		/* not used */
{
struct rlimit	lm;
int		i;
int		lim;
		
	for (i = 0; i < RLIM_NLIMITS; i++)
	{
		if (getrlimit(limits[i].type,&lm) < 0)
		{
			return (-1);
		}

		/* the size is returned in bytes we want blocks */
		if (limits[i].type != RLIMIT_CPU)
			lim = (lm.rlim_cur / UBSIZE);
		else
			lim = lm.rlim_cur;

		if (_buildvec ("%s=%d",limits[i].attr,lim) < 0)	
		{
			return (-1);
		}
	}
		
	return (0);
}
/*
 * _getpumsk
 *                                                                    
 * FUNCTION: Get the process umask 
 *
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: -1 if error, 0 if successful
 */  
static	int
_getpumsk (ap, flgs)
char	*ap;		/* attribute string */
int	flgs;		/* not used */
{
int	mask;
		
	mask = (int) umask ((mode_t) 0);/* get previous mask */
	(void) umask ((mode_t) mask);	/* reset to previous mask */
	if (_buildvec ("%s=%o", ap, mask) < 0)	
	{
		return (-1);
	}
	return (0);
}
/*
 * _getpgrps
 *                                                                    
 * FUNCTION: Get the process group set
 *
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: -1 if error, 0 if successful
 */  
static	int
_getpgrps (ap, flgs)
char	*ap;		/* attribute string */
int	flgs;		/* not used */
{
gid_t	groupset[S_NGROUPS];
int	i, n;
char	buf[BUFSIZ]; /* to build the string */

	if ((n = getgroups (S_NGROUPS, groupset)) < 0)
		return (-1);
	
	if (n)
		sprintf (buf,"%s=%s", ap, IDtogroup (groupset[0]));
	else
		return (0);

	for (i = 1; i < n; i++)
	{
		strcat(buf,",");
		strcat(buf,IDtogroup(groupset[i]));
	}

	if (_buildvec ("%s", buf) < 0)
		return (-1);

	return (0);
}
/*
 * _buildvec
 *                                                                    
 * FUNCTION: Build the credentials vector.
 *
 *	The credentials vector is an array of pointers to alloc'd memory.
 *	The given string is added to the credentials vector.
 *	Memory is realloc'd PCREDSIZ at a time. A static counter 
 *	of credentials is kept.
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: -1 if error, 0 if successful
 */  
static	int
_buildvec (args)
char	*args;
{
char	**argv;
char	buf[BUFSIZ];

	argv = &args;
	vsprintf (buf, argv[0], (va_list) &argv[1]);

	/* initial allocation of pcred memory */
	if (pcredsiz == -1)
	{
		pcredsiz = PCREDSIZ;
		if ((pcred=(char **)malloc(sizeof(char *) * pcredsiz)) == NULL)
		{
			return (-1);
		}
	}

	/* get more memory if needed */
	if (argcount == (pcredsiz -1))
	{
		pcredsiz += PCREDSIZ;
		if ((pcred = (char **)realloc (pcred,(sizeof(char *) * pcredsiz))) == NULL)
		{
			return (-1);
		}
	}

	/* allocate space to hold the given string */
	if ((pcred[argcount] = (char *)malloc (strlen(buf)+1)) == NULL)
	{
		return (-1);
	}

	/* add the credentials string to the pcred array */
	strcpy(pcred[argcount],buf);

	/* adjust array size (to keep the count) */
	argcount++;

	return (0);
}

