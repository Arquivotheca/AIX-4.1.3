static char sccsid[] = "@(#)87	1.36.1.5  src/bos/usr/ccs/lib/libs/setpenv.c, libs, bos411, 9428A410j 6/7/94 14:54:20";
/*
 * COMPONENT_NAME: (LIBS) Security Library Functions 
 *
 * FUNCTIONS: setpenv
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/priv.h>		/* for TRUSTED_PATH, setpriv() etc. */
#include <sys/id.h>		/* for ID_REAL */
#include <sys/tcb.h>		/* for tcb() */
#include <fcntl.h>		/* for O_RDWR etc. */
#include <sys/termio.h>		/* for TCTRUST etc. */
#include <uinfo.h>		/* for SETUINFO */
#include <ctype.h>		/* for is*() macros */
#include <stdio.h>		/* for BUFSIZ */
#include <usersec.h>		/* for getuserattr() stuff */
#include <userpw.h>		/* for PW_NAMELEN */
#include "libs.h"		/* for M_CHDIR */

/* just to avoid multiple string constants of the same value */
#define PWD_STR		"PWD="
#define TERM_STR	"TERM="	
#define SHELL_STR	"SHELL="
#define HOME_STR	"HOME="
#define LOGNAME_STR	"LOGNAME="
#define PATH_STR	"PATH="
#define USER_STR	"USER="
#define LOGIN_STR	"LOGIN="
#define AUTHSTATE_STR	"AUTHSTATE="
#define DCECRED_STR	"KRB5CCNAME="
#define TTY_STR		"TTY="
#define NAME_STR	"NAME="

#define TERM		"TERM"
#define AUTHSTATE	"AUTHSTATE"
#define DCECRED		"KRB5CCNAME"

#define  ENVSIZ	1000

/* externs */
extern	char	**environ;
extern	int	errno;
extern	char	*strstr ();
extern	char	*strchr ();

/* local functions */
static	char	*nexttok(char **to_p,char **from_p);
static	char	*subval (char *org);
static	char	*subenv (char *);
static	char  	*getenval (char *attr);
static	char  	**mkargs (char *user,char *cmd,int mode);
static	char	**search(char **env,char *str);
static	int	cktrust(char *path);
static	int	checkmode(int mode);
static	int	listlen(char *p);
static	int 	setpinit (char *user);
static	int 	setpargs (char **envp,char **usrp,int mode);
static	int  	mkenv (char *s1,char * s2);
static	int	getlogname(char *logname);
static	int	set_eff_priv();
static	int	tokcmp();
static	int 	setpsys ();
static	int 	setpexists ();
static	char	*newinfo(char *usrbuf,char *usrp);
static	int	gotohome (char *);
static	int	gotoguest(char *home);


/* the environment for the new proces */
char	**newenv;	/* the new environment */
int	envsiz;		/* size of new environment */
int	envcount;	/* count  of environment strings in newenv */

/*
 * NAME: setpenv
 *                                                                    
 * FUNCTION: set process environment
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine sets the process environment for the calling process.
 *	Then executes the 'cmd' specified by the caller. It does not return.
 *
 * RETURNS: if successful it does not return. If fail returns -1 and
 *		errno is set to:
 *
 *		EINVAL	the mode parameter is nonsensical or
 *		        environment string did not parse
 *		EACCES  calling process does not have read permission 
 *			for the specified user's usinfo
 *		ESAD	user cannot execute a non-trusted program.
 *
 */  

int
setpenv (char *user, int mode, char **env, char *cmd)
{
char		**usrp;		/* pointer to protected env string */
char		**envp;		/* pointer to user's env string */
static char	**args;		/* the command arguments */
char		*path;		/* the command to run */
char		**argvec;	/* the execve arguments  */
uid_t		uid;		/* the process' real user id */
int		userenv = 0;	/* indicates user specified environment */
register int	siz;		/* size to malloc */
register int	i;
char 		*p;
char		shortname[PW_NAMELEN];	/* Short user name (max 8 bytes) */


	/* set up the size of the environment array */
	envsiz = ENVSIZ;
	siz = sizeof (char *) * envsiz;
	if ((newenv = (char **)malloc(siz)) == NULL)
		return (-1);
	newenv[0] = NULL;
	envcount = 0;

	/* check the mode flags */
	if (checkmode(mode))
	{
		errno = EINVAL;
		return (-1);
	}

	/* find user name if not given */
	if (!user)
	{
		uid = getuid();
		if ((user = IDtouser (uid)) == NULL)
			return(-1);

		/* Save the user name so subsequent operations don't lose it */
		if ((user = strdup(user)) == (char *)NULL) 
			return(-1);
		strcpy(shortname, user);
	}
	else
		_normalize_username(shortname, user);

	/* parse optional environment variables */
	if (env && *env)
	{
		/* set flag that environment has new stuff */
		userenv = 1;

		/* look for user and environ strings in input */
		envp = search (env, PENV_USRSTR);
		usrp = search (env, PENV_SYSSTR);
		if (!( (usrp && *usrp) || (envp && *envp) ))
		{
			/* failed parse */
			errno = EINVAL;
			return (-1);
		}
		/* remove the keywords, we don't need them anymore */
		if (usrp && *usrp)
		{
			*usrp = NULL;
			usrp++;
		}
		if (envp && *envp)
		{
			*envp = NULL;
			envp++;
		}
	}

	/* change environment as requested */
	/* set initializing environment variables */
	if(mode & PENV_INIT)
	{
		if (setpinit (shortname) < 0)
		{
			return (-1);
		}
	}
	/* retain existing environment */
	else 
	{
		if (mode & PENV_DELTA)
		{
			if (setpexists () < 0)
			{
				return (-1);
			}
		}
	}

	/*
	 * Create LOGIN with user's full name.  This variable changes, 
 	 * whether the mode is DELTA or INITIAL, since this should contain 
	 * the user's last authenticated name.  The user still has the 
	 * capability of explicitly setting this in the userenv below.
	 */
	if (mkenv(LOGIN_STR, user) < 0) 
		return(-1);

	/* set environment variables specified by caller */
	if (userenv)
	{
		if (setpargs (envp, usrp, mode) < 0)
		{
			return (-1);
		}
	}

	/* exec the specified command */
	if (mode & PENV_ARGV)
	{
		if (cmd)
		{
			args = (char **) cmd;
			path = args[0];
			argvec = &args[0];
		}
		else
		{
			args = argvec = (char **) NULL;
			path = (char *) NULL;
		}
	}
	/* get the argument string */
	else
	{
		args = mkargs (shortname, cmd, mode);
		if (*args == NULL)
		{
			if (!errno)
				errno = EINVAL;
			return (-1);
		}
		path = args[0];
		argvec = &args[1];
	}

	/* cleanup the environment on KLEEN */
	if(mode & PENV_KLEEN)
	{
		kleenup (3, 0, 0);
	}

	/* check trusted path */
	if (cktrust(path))
	{
		return(-1);
	}

	/* drop effective privilege */
	if (set_eff_priv())
	{
		return(-1);
	}

	/* Don't perform the execve() */
	if (mode & PENV_NOEXEC)
	{
		return(0);
	}

	/* run the program */
	execve (path, argvec, newenv);

	/* 'execve' failed (assume errno is set) */
	return (-1);
}

/*
 * NAME: setpinit
 *                                                                    
 * FUNCTION: 	
 *	1. set initial environment variables:
 *		TERM SHELL HOME LOGNAME PATH USER AUTHSTATE
 * 	2. set variables from /etc/environment
 *	3. set environ and usrinfo variables from user database
 *                                                                    
 * RETURNS: fail: -1, successful: 0
 */  
static int
setpinit (char *user) 	/* User name (maximum 8 byte) standard AIX name       */
{
	char	*shell;		/* User's shell				      */
	char	*tty;		/* User's current tty		  	      */
	char	*db;		/* User environment from database	      */
	char	*var;		/* value portion of attribute=value pairs     */
	char	*path;		/* User's path from database		      */
	int	len = 0;	/* byte length of usrinfo buffer	      */
	int	i;		/* Loop variable		  	      */
	int 	notfound = 1;	/* Signal variable to create default path     */
	char 	logname
		   [PW_NAMELEN];/* LOGNAME from protected env                 */
	char	ubuf[UINFOSIZ];	/* usrinfo buffer			      */

	/* get TERM out of 'environ' */
	if (mkenv (TERM_STR, (char *)getenv (TERM)) < 0)
	{
		/* use default term */
		if (mkenv (TERM_STR, "dumb") < 0)
			return (-1);
	}

	/*
	 * Get AUTHSTATE out of environment.  AUTHSTATE describes the primary
	 * method that authenticated this user (default is local plus NIS
	 * authentication).
	 */
	if (mkenv(AUTHSTATE_STR, (char *)getenv(AUTHSTATE)) < 0)
		if (mkenv(AUTHSTATE_STR, AUTH_COMPAT) < 0)
			return(-1);

	/*
	 * Get DCE credential environment variable.  This is set by 
	 * the DCE load module if we authenticated in this fashion.
	 */
	mkenv(DCECRED_STR, (char *)getenv(DCECRED));

	/* Get SHELL out of the database.  If error default to /bin/sh */
	if (getuserattr(user, S_SHELL, (void *)&shell, SEC_CHAR) || !shell || 
	    (*shell == '\0'))
	{
		if (mkenv (SHELL_STR, "/bin/sh") < 0)
			return (-1);
	}
	else
	{
		if ((var = subval(shell)) == NULL)
		{
			if (mkenv (SHELL_STR, shell) < 0)
				return (-1);
		}
		else
		{
			if (mkenv (SHELL_STR, var) < 0)
				return (-1);
		}
	}

	/* change directory to the user's home */
	if (gotohome(user))
		return (-1);


	/* make USER */
	if (mkenv (USER_STR, user) < 0)
	{
		return (-1);
	}

	/* set the variables specified in /etc/environment */
	if (setpsys () < 0)
	{
		return (-1);
	}

	/*
	 * see if PATH has been set
	 */
	for (i = 0; i < envcount; i++)
	{
		if (tokcmp (newenv[i],PATH_STR, '='))
		{
			notfound = 0;
			break;
		}
	}

	if (notfound)
	{
		/* make PATH */
		path = "/bin:/usr/bin:$HOME:.";

		if ((var = subval(path)) == NULL)
		{
			if (mkenv (PATH_STR, path) < 0)
				return (-1);
		}
		else
		{
			if (mkenv (PATH_STR, var) < 0)
				return (-1);
		}
	}

	/* get environ out of user database */
	if (!getuserattr (user, S_USRENV, (void *) &db, SEC_LIST))
	{
	char	*p = db;

		while (*p)
		{
			if ((var = subval(p)) == NULL)
			{
				if (mkenv ("",p) < 0)
					return (-1);
			}
			else
			{
				if (mkenv ("",var) < 0)
					return (-1);
			}
			while (*p++);
		}
	}

	logname[0] = (char)NULL;
	if (getlogname(logname))
		return (-1);

	if (!logname[0])
	{
		strncpy(logname, user, PW_NAMELEN - 1);
		logname[PW_NAMELEN - 1] = (char)NULL;
	}

	if (mkenv(LOGNAME_STR, logname) < 0)
		return (-1);

	tty = ttyname(0); 	/* Get current tty name */

        /* make default usrinfo variables */
        memset (ubuf, NULL, UINFOSIZ);

	if ((sizeof(LOGNAME_STR) + sizeof(NAME_STR) + sizeof(TTY_STR) +
	     strlen(logname) + strlen(user) + strlen(tty) + 1) > UINFOSIZ)
		return(-1);
			
        len = sprintf(ubuf,"%s%s%c%s%s%c%s%s", LOGNAME_STR,logname, 0,
        	NAME_STR,user, 0, TTY_STR,tty);
	
        len++;  /* include last NULL */

	/* get usrinfo from user database */
	if (!getuserattr(user, S_SYSENV, (void *) &db, SEC_LIST) && db && *db)
        {
        char    *bp = &ubuf[len];
        int     dblen = listlen (db);
        char    *lenv;

                if (dblen > UINFOSIZ - len)
		{
                /* leave room for the double-null ending (hence '- 2')*/
                        dblen = UINFOSIZ - len - 2;
		}

                /* substitute all the $PATH type env variables */
                if ((lenv = subenv(db)) != (char *)NULL)
                {
                        memcpy (bp, lenv, dblen);
                        len += dblen;
                }
        }

	/* finally, set usrinfo variables in kernel */
	if (usrinfo (SETUINFO, ubuf, len) < 0)
		return (-1);

	return (0);
}

/*
 * NAME: cktrust()
 *                                                                    
 * FUNCTION: checks for process trustedness.
 *		if terminal is trusted (process is on the trusted path)
 *			check for trusted program
 *				if program is not trusted
 *					fail
 *				else
 *					return ok
 *		else
			return ok
 *                                                                    
 * RETURNS: fail: -1, successful: 0
 */  

static int
cktrust(char *path)
{
	int the_rc = 0;
	int trusted_state = 0;

	/* see if process is on trusted path */
	ioctl(0, TCQTRUST, &trusted_state);
	
	if (trusted_state == TCTRUSTED)
	{
		/* check tp bit from mode of program to be executed */
		if (tcb(path,TCB_QUERY) != TCB_ON)
		{
			errno = ESAD;
			return (-1);
		}
	}
	return (0);
}

/*
 * NAME: set_eff_priv()
 *                                                                    
 * FUNCTION: drops the effective privilege.
 *                                                                    
 * RETURNS: fail: -1, successful: 0
 */  

static int
set_eff_priv()
{
priv_t	priv;

	priv.pv_priv[0] = 0;
	priv.pv_priv[1] = 0;

	return(setpriv(PRIV_SET|PRIV_EFFECTIVE,&priv,sizeof(priv_t)));

}


/*
 * NAME: setpargs
 *                                                                    
 * FUNCTION: set the environment with those specified by caller
 *
 * NOTES:
 *	The environment consists of privileged and non-privileged variables.
 * 	Privileged variables are accessed by usrinfo() and non-privileged
 *	are accessed by the 'extern char **environ' variable.
 *
 *	Basically, the given strings are put into their respective 
 *	environment places (ie privileged into the kernel with usrinfo() )
 *	and non-privileged into 'environ'.
 *
 * RETURNS: 
 *	fail: -1, successful: 0
 *
 * PASSED:
 *	char	**envp;		 environ (non-privileged) pointer
 *	char	**usrp;		 usrinfo (privileged) pointer
 *	int	mode;		 user supplied mode
 */  
static int
setpargs (char **envp,char **usrp,int mode)
{
register int	siz;

	/* environment */
	if (envp && *envp)
	{
		while(*envp)
		{
			mkenv(*envp,"");
			envp++;
		}
	}

	/* user info */
	if (usrp && *usrp)
	{
	char	*usrbuf;		/* used to set usrinfo */
	char	buf [UINFOSIZ];		/* used to set usrinfo */
	char	*ubp;			/* ptr to usrbuf */
	int	ulen;			/* uinfo len */
	int 	i;
	char	*new;
	char	*newptr;
	char	*oldptr;

		/* initialize */
		memset (buf, NULL, UINFOSIZ); /* clear buffer */
		ulen = 0;

		/* get existing usrinfo variables if requested */
		if ( (mode & PENV_DELTA) || (mode & PENV_INIT) )
		{
			if ((ulen = usrinfo (GETUINFO, buf, UINFOSIZ)) < 0)
				ulen = 0;
		}

		/* for each new variable either replace or add it */
		usrbuf = &buf[0];
		for (i=0;usrp[i];i++)
		{
			if ((new = newinfo(usrbuf,usrp[i])) == NULL)
			{
				return (0);
			}

			/* copy the new string back to the original buffer */
			if (listlen (new) > UINFOSIZ)
				continue;

			newptr = new;
			oldptr = &buf[0];
			while (*newptr)
			{
				while (*newptr)
					*oldptr++ = *newptr++;
				*oldptr++ = *newptr++;
			}
		}

		/* get length of new list */
		ulen = listlen (&buf[0]);

		if (usrinfo (SETUINFO, buf, ulen) < 0)
		{
			return (-1);
		}
	}
	return (0);
}

static char	newbuf[UINFOSIZ];

/*
 * NAME: newinfo()
 *                                                                    
 * FUNCTION: sets or replaces variables in the usrinfo buffer.
 *                                                                    
 * RETURNS: NULL if fail new buffer if ok.
 */  
static char *
newinfo(char *usrbuf,char *usrp)
{
char	*ptr;			/* pointer to ubuf */
char	*val;			/* pointer to ubuf */
char	*newptr;			/* pointer to ubuf */
char	*newbufptr;			/* pointer to ubuf */
char	*attr;
char	*new;
char 	newattr[UINFOSIZ];
char 	newval[UINFOSIZ];
char	newstring[UINFOSIZ];

	/* clear new buffer */
	memset (newbuf,0,UINFOSIZ);

	/* save the original string */
	strcpy (newstring,usrp);

	/* get the VARIABLE part of VARIABLE=val */
	ptr = &newattr[0];
	while (*usrp && *usrp != '=')
		*ptr++ = *usrp++;
	*ptr = '\0';
	if (!*usrp)
		return (0);
	usrp++;

	/* get the var part of VARIABLE=val */
	newptr = &newval[0];
	ptr = &newval[0];
	while (*usrp)
		*ptr++ = *usrp++;
	*ptr++ = '\0';

	/* parse the usrinfo string looking for this VARIABLE */
	newbufptr = &newbuf[0];
	new = &newbuf[0];
	ptr = usrbuf;

	while (*ptr)
	{
		/*
		 * get the attribute from the original
		 * buffer and copy into new buffer
		 */
		attr = ptr;
		while (*ptr && (*ptr != '='))
			*newbufptr++ = *ptr++;

		if (*ptr == '=')
		{
			*ptr = '\0';

			/*
			 * see if the new attribute is equal
			 * to any in the old string
			 */
			if (strcmp (attr,newattr) == 0)
			{
				/*
				 * the attributes are equal
				 * so increment past the old value
				 */
				ptr++;
				while (*ptr)
					ptr++;

				/* copy in the new value */
				*newbufptr++ = '=';
				while (*newptr)
					*newbufptr++ = *newptr++;

				/* copy in the NULL */
				*newbufptr++ = *ptr++;

				/* copy in the rest of the buffer if any */
				while (*ptr)
				{
					while (*ptr)
						*newbufptr++ = *ptr++;
					*newbufptr++ = *ptr++;
				}

				/* copy in the NULL */
				*newbufptr = *ptr;
				return (&newbuf[0]);
			}
			*ptr = '=';

			/*
		 	 * get the value from the original
		 	 * buffer and copy into new buffer
		 	 */
			while (*ptr)
				*newbufptr++ = *ptr++;
			*newbufptr++ = *ptr++;
		}
	}

	/* add the new string to the usrinfo buffer */
	ptr = &newstring[0];
	while (*ptr)
		*newbufptr++ = *ptr++;
	
	/* add the two NULLS */
	*newbufptr++ = '\0';
	*newbufptr = '\0';
	
	return (&newbuf[0]);
}

/*
 * NAME: setpsys
 *                                                                    
 * FUNCTION: set the environment from those in the /etc/environment file
 *                                                                    
 * RETURNS: fail: -1, successful: 0
 */  
static int
setpsys ()
{
FILE	*fp;

	if ((fp = fopen ("/etc/environment", "r")) != NULL)
	{
	char	*buf, *p;

		if ((buf = malloc (BUFSIZ)) == NULL)
		{
			fclose(fp);
			return (-1);
		}
		while (1)
		{
			/* get environment variable */
			if (fgets (buf, BUFSIZ, fp) == 0)
			{
				break;
			}
			/* ignore comments and lines without '=' */
			if ((*buf == '#') || (strchr (buf, '=') == 0))
			{
				continue;
			}
			/* replace newline character with NULL */
			for (p = buf; *p; p++)
			{
				if (*p == '\n')
				{
					*p = '\0';
					break;
				}
			}
			/* add to new environment */
			mkenv (buf, "");
		}
		free ((void *)buf);
		fclose (fp);
	}
	return (0);
}
/*
 * NAME: setpexists
 *                                                                    
 * FUNCTION: set the new environment from existing environment
 *                                                                    
 * RETURNS: fail: -1, successful: 0
 */  

static int
setpexists ()
{
extern  char **environ;
char	**e;

	for (e = environ; *e ; e++)
	{
		if (mkenv (*e, "") < 0)
		{
			return (-1);
		}
	}
	return (0);
}

/*
 * NAME: mkenv - make an environment construct (i.e. 'name=value') 
 *
 * FUNCTION: This static function makes an environment variable from two
 *		strings and stores it in the new environment.
 */

static	int
mkenv (register char *s1,register char * s2)
{
int	siz;
int	i=0;
char	*buf;

	/* if needed, enlarge new environment pointer array */
	if (envcount == (envsiz -1))
	{
		envsiz += ENVSIZ;
		siz = sizeof(char *) * envsiz;
		if((newenv =(char **)realloc(newenv,siz)) == NULL)
		{
			return (-1);
		}
	}

	/* sanity check pointers (pointing to NULL is ok) */
	if (!s1 || !s2)
	{
		return (-1);
	}

	siz = strlen (s1) + strlen (s2) + 2;
	if ((buf = (char *) malloc (siz)) == NULL)
	{
		return (-1);
	}

	/* assuming s1 or s2 has a '=' */
	sprintf (buf, "%s%s", s1, s2);

	/* replace existing environment variable */
	for (i = 0; i < envcount; i++)
	{
		if (tokcmp (newenv[i], buf, '='))
		{
			newenv[i] = buf;
			return (0);
		}
	}

	/* add new environment variable */
	newenv[envcount++] = buf;
	newenv[envcount] = (char *)NULL;
	return (0);
}
/*
 * NAME subval  substitute value 
 *
 * EXECUTION:
 *		Substitute $token for value of token as it is defined
 *		in the new environment
 *
 * RETURNS:	
 *		on success returns a pointer to the new substituted value 
 *		on fail returns NULL
 */
static char *
subval (char *org)
{
static	char	*sub;	/* new substituted value */
static	char	*start;	/* start of token to replace */
static	char	*new;	/* the new token to substitute */
static  char	*sbuf;	/* buffer to hold the token */
static  char	*rem;	/* the remainder of the org token after substitution */
register int	siz;	/* space to allocate */

	/* get token after the '$' */
	if ((start = strchr (org, '$')) == NULL)
	{
		/* nothing to replace */
		return (NULL);	
	}
	/* sbuf holds the token to substitute plus '=' plus the NULL */
	siz = strlen (start) + 4;
	if ((sbuf = malloc (siz)) == NULL)
	{
		return (NULL);
	}
	/* copy token into buffer */
	{
	char	*p, *q;
		
		p = sbuf;
		q = start+1;
		while (isalpha ((int)*q))
		{
			*p++ = *q++;
		}
		*p = '\0';
		/* save remainder of string */
		rem = NULL;
		if (*q)
		{
			siz = strlen (q) + 1;
			if ((rem = malloc (siz)) == NULL)
			{
				free ((void *)sbuf);
				return (NULL);
			}
			strcpy (rem, q);
		}
	}
	/* get substitute value */
	strcat (sbuf, "=");
	if (((new = getenval (sbuf)) == NULL) || (*new == '\0'))
	{
		/* nothing to substitute */
		free ((void *)sbuf);
		if (rem)
			free ((void *)rem);
		return (NULL);	
	}
	/* get memory for new value */
	siz = strlen (org) + strlen(new) + 1;
	if ((sub = malloc (siz)) == NULL)
	{
		free ((void *)sbuf);
		if (rem)
			free ((void *)rem);
		return (NULL);
	}
	strcpy (sub, org);

	/* find start of the token again */
	if ((start = strchr (sub, '$')) == NULL)
	{
		free ((void *)sbuf);
		if (rem)
			free ((void *)rem);
		return (NULL);	
	}
	/* piece together new value */
	*start = '\0';
	strcat (sub, new);	
	if (rem)
		strcat (sub, rem);
	free ((void *)sbuf);
	return (sub);
}

/*
 * NAME: getenval - get environment value assigned to the specified attribute
 *
 * RETURNS: pointer to the value if successful; NULL if not found 
 */
static char *
getenval (char *attr)
{
int	i;

	for (i = 0; i < envcount; i++)
	{
		if (strncmp (attr, newenv[i], strlen (attr)) == 0)
		{
			/* return the value */
			return (newenv[i] + strlen (attr));
		}
	}
	return (NULL);
}
/*
 * NAME mkargs - from the given string build a command argument vector
 *
 * RETURN pointer to an array of argument strings
 *
 */

#define MAXARGS		128		
#define DEFAULT_SHELL	"/bin/sh"    /* user login shell when not specified */

static char **
mkargs (char *user,char *cmd,int mode)
{
char		*tok;		/* next token */
static	char	*av[MAXARGS]; 	/* arg vector to return */
int		ac;		/* arg counter */
static	char	*cbuf;		/* to hold the arguments */
char		*cmdp;
char		*cp;
char		*temp;
register int	siz;
register int	err;
char		*shell;


	/* initialize */
	ac = 0;

	/* perform substitution on command */
	if (cmd) 
	{
		/* If '$SHELL' is present in the cmd then try to expand it */
		if(temp = strchr(cmd,'$'))
			if(strncmp(temp,"$SHELL",6) ==0)
				cmd = subval(cmd);
	}

	/* 
	 * If shell not specified use :
	 *	1. the login shell in the user database (if it exists)
	 *	2. the default shell (it always exists)
	 */

	/* must have full path name */
	if (*cmd != '/')
	{
		err = getuserattr(user,S_SHELL,(void *)&shell,SEC_CHAR);
		cmd = (err || !shell) ? DEFAULT_SHELL : shell;
		for (; *cmd == ' ' || *cmd == '\t'; cmd++);
		if (*cmd != '/')
		{
			errno = EINVAL;
			return((char **) NULL);
		}
	}

	/* 
	 * get enough memory for the arguments 
	 * (the first argument is the program name to worst case
	 * is that we need double the memory of the command itself)
	 */

	siz = strlen (cmd) * 2;
	if ((cbuf = (char *) malloc (siz)) == NULL)
		return (NULL);

	/* first argument is special case */
	/* make sure the first arg is last component of the command or '-' */
	{
		tok = nexttok (&cbuf, &cmd);
		siz = strlen(tok) + 1;
		if ((av[ac] = (char *) malloc(siz)) == NULL)
			return (NULL);
		
		strcpy(av[ac++],tok);
		cp = cbuf;

		/* 
		 * if caller did not specify first arg 
		 * then add one according to mode
		 * here we are adding "-sh" to the arg list
		 */

		if ((mode & PENV_INIT) && !(mode & PENV_NOPROF))
		{
		char	*tp;
			
			if (tp = strchr (tok, ' '))
				sprintf (cp,"%s%s","-",strrchr(tp, '/') + 1);
			else
				sprintf (cp,"%s%s","-",strrchr(tok, '/') + 1);
		}
		else
		{
		char	*tp;
			
			if (tp = strchr (tok, ' '))
				strcpy (cp, strrchr(tp, '/') + 1);
			else
				strcpy (cp, strrchr(tok, '/') + 1);
		}
		siz = strlen(cp) + 1;
		if ((av[ac] = (char *) malloc(siz)) == NULL)
			return (NULL);
		strcpy(av[ac++],cp);
		cbuf += strlen (cp) + 1;
	}
	/* build args from cmd string */
	while (*cmd)
	{
		/* nexttok advances cbuf and cmd */
		if (!(tok = nexttok (&cbuf, &cmd)))
			break;

		siz = strlen(tok) + 1;
		if ((av[ac] = (char *) malloc(siz)) == NULL)
			return (NULL);

		strcpy(av[ac++],tok);
		if (strcmp (tok, "-c") == 0)
		{
			/* treat remaining tokens as one command */
			while (isspace((int)*cmd)) cmd++
				;
			siz = strlen(cmd) + 1;
			if ((av[ac] = (char *) malloc(siz)) == NULL)
				return (NULL);

			strcpy(av[ac++],cmd);
			break;
		}
	} 
	av[ac] = NULL;
	return (av);
}

/*
 * NAME nexttok - get the next token
 *
 * RETURN pointer to the argument or NULL 
 */
static char *
nexttok (char **to_p,char **from_p)
{
static	char	*to;
static	char	*from;
static	char	*ret;
char	c;

	from = *from_p;

	/* skip to start of next token */
	while (isspace ((int)*from))
		from++;

	/* if no more tokens, return failure */
	if (*from == '\0')
		return (NULL);

	/* remember start of this token */
	ret = to = *to_p;

	/* copy token from <from> to <to> */
	while (1)
	{
		c = *from;
		if ((c == '\0') || isspace((int)c))
			/*
			 * don't bump pointer to consume NULL;
			 * we'll want to see it next time around.
			 */
			break;
		from++;
		if (c != '\\')
		{
			*to++ = c;
			continue;
		}
		/* interpret the escape character */
		c = *from++;
		switch (c)
		{
			case 'n' :
				c = '\n';
				break;
			case 'r' :
				c = '\r';
				break;
			case 'v' :
				c = '\v';
				break;
			case 'b' :
				c = '\b';
				break;
			case 't' :
				c = '\t';
				break;
			case 'f' :
				c = '\f';
				break;
			case '0' : case '1' : case '2' : case '3' :
			case '4' : case '5' : case '6' : case '7' :
				c -= '0';
				if (isdigit ((int)*from))
				{
					c <<= 3;
					c += *from++ - '0';
					if (isdigit ((int)*from))
					{
						c <<= 3;
						c += *from++ - '0';
					}
				}
				c &= 0377;
				break;
			default :
				break;
		}
		*to++ = c;
	}
	*to++ = '\0';

	/* advance source and destination pointers */
	*to_p = to;
	*from_p = from;

	return (ret);
}
/*
 * NAME tokcmp - compares two strings up to the token c
 *
 * RETURN 1 for equal 0 for not
 */
static int
tokcmp (s1,s2,c)
char *s1;
char *s2;
char c;
{
	while (*s1 == *s2)
	{
		if (*s1 == c)
			return (1);
		s1++;
		s2++;
	}
	return (0);
}
/*
 * NAME listlen - returns length of list
 *
 * RETURN  - the length.
 */
static int
listlen (char *p)
{
int	len = 0;

	while (*p)
	{
		while (*p++)
			len++;
		len++;
	}
	len++;
	return (len);
}
/*
 * NAME checkmode - check the mode for valid values
 *
 * RETURN  - ok or not.
 */
static int
checkmode(int mode)
{

	if (mode & PENV_ARGV)
		mode &= ~PENV_ARGV;
		
	if (mode & PENV_KLEEN)
		mode &= ~PENV_KLEEN;
		
	if (mode & PENV_NOPROF)
		mode &= ~PENV_NOPROF;

	if (mode & PENV_NOEXEC)
		mode &= ~PENV_NOEXEC;

	switch(mode)
	{
		case PENV_INIT  : return(0);
		case PENV_DELTA : return(0);
		case PENV_RESET : return(0);
		default	        : return(-1);
	}
}

/*
 * NAME search - searches for str in env
 *
 * RETURN  - the string or NULL
 */
static	char **
search(char **env,char *str)
{
	while(*env)
	{
		if (strcmp(*env,str) == 0)
			return(env);
		env++;
	}
	return(NULL);
}

/*
 * NAME: getlogname 
 * 
 * FUNCTION: gets LOGNAME from current user environment.
 *
 * RETURNS:  0 - success
 *	    -1 - failure
 */
static int
getlogname(char *logname)
{
	char	usrbuf [UINFOSIZ];	/* used to get usrinfo */
	char	*ubp;			/* pointer to ubuf */
	register char	*val;
	register char	*attr;

	/* initialize */
	memset (usrbuf, NULL, UINFOSIZ); /* clear buffer */

	/* get current usrinfo */
	if (usrinfo (GETUINFO, usrbuf, UINFOSIZ) < 0)
		return (-1);

	/* Go through buffer looking for the LOGNAME variable */
	ubp = &usrbuf[0];

	while (*ubp)
	{
		attr = ubp;
		while (*ubp && (*ubp != '='))
			ubp++;

		if (*ubp == '=')
		{
			*ubp++ = '\0';
			val = ubp;

			if (!strcmp(LOGNAME_STR,attr))
			{
				strncpy(logname, val, PW_NAMELEN - 1);
				logname[PW_NAMELEN - 1] = '\0';
				break;
			}
			while (*ubp)
				ubp++;
		}
		ubp++;
	}
	return(0);
}

/*
 * NAME subenv - substitutes all $VARIABLES with their
 *		respective environment variables.
 *
 * RETURN  - 0 or (-1) for fail.
 */

char *
subenv(char *db)
{
int	i=0;
char	*newenv;
char	*buf;
char	buffer[UINFOSIZ];
int	siz = 0;
	
	/* clear buffer */
	memset (buffer, NULL, UINFOSIZ);

	buf = &buffer[0];

	while (db && *db)
	{
		if ((newenv = subval (db)) == NULL)
		{
			/* copy in current string */
			while (*db)
				*buf++ = *db++;
			*buf++ = '\0';
			db++;
		}
		else
		{
			/* copy in new string */
			while (*newenv)
				*buf++ = *newenv++;
			*buf++ = '\0';

			/* get to the next entry in db */
			while (*db)
				db++;
			db++;
		}
	}
	*buf++ = 0;
	return(buffer);
}

/*
 * NAME gotohome 
 *
 * DESCRIPTION : changes the directory to the user's home directory,
 *		if this doesn't work it 
 *		changes to the home directory of the guest user.
 *		if this doesn't work it goes to "/".
 *		it also changes the HOME= env variable.
 *
 * RETURN  - 0 or (-1) for fail.
 */

static int
gotohome(char *user)
{
char *home;
char *dir;
register int	err;

	/*
	 * get HOME out of the database 
	 * if error default to guest's home first
	 * if error change to "/"
	 */

	err = getuserattr (user, S_HOME, (void *) &home, SEC_CHAR);
	if (err || !home || (home && (*home != '/')))
	{
		if (gotoguest((char *)NULL))
			return (-1);
		return (0);
	}
	else
	{
		if (chdir (home))
		{
			if (gotoguest(home))
				return (-1);
			return (0);
		}
	}

	/* add to environment */
	if (mkenv (HOME_STR, home))
		return (-1);

	return (0);
}

/*
 * NAME gotoguest 
 *
 * DESCRIPTION :  changes the home directory to guest's and
 *		sets the env variable HOME=
 *
 * RETURN  - 0 or (-1) for fail.
 */

static int
gotoguest(char *home)
{
char	*dir;
register int	err;

	err = getuserattr("guest",S_HOME,(void *) &dir,SEC_CHAR);
	if (err || !dir || (dir && (*dir != '/')))
	{
		dir = "/";
		fprintf (stderr,(char *) MSGSTR (M_CHDIR, DEF_CHDIR),home,dir);
		chdir (dir);
		/* add to environment */
		if (mkenv (HOME_STR, dir))
			return (-1);
		return (0);
	}
	
	if (chdir (dir) < 0)
	{
		dir = "/";
		fprintf (stderr,(char *) MSGSTR (M_CHDIR, DEF_CHDIR),home,dir);
		chdir (dir);
		/* add to environment */
		if (mkenv (HOME_STR, dir))
			return (-1);
		return (0);
	}

	fprintf (stderr,(char *) MSGSTR (M_CHDIR, DEF_CHDIR),home,dir);
	/* add to environment */
	if (mkenv (HOME_STR, dir))
		return (-1);

	return (0);
}
