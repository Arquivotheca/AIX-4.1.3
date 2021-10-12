static char sccsid[] = "@(#)84	1.10  src/bos/usr/ccs/lib/libs/getpenv.c, libs, bos411, 9428A410j 6/16/90 02:31:08";
/*
 * COMPONENT_NAME: (LIBS) Security Library Functions 
 *
 * FUNCTIONS: getpenv
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <usersec.h>
#include <uinfo.h>
#include <sys/errno.h>

extern	char 	**environ;	/* public environment variables */
extern	char	*malloc ();	/* memory allocator */
extern	char	*realloc ();	/* memory allocator */
static  int	addstr(register char *str);
static	int	checkmode(int mode);

static	char	**penv = (char **)NULL;	/* the return string of env variables */
static  int	penvsiz = -1;	/* dynamic size of return string */
static	int	argcount = 0;	/* count of penv arguments */

#define	PENVSIZ	1000  /* initial size of penv array. We'll get more if needed */

/* 
 * current working directory shell variable 
 * (korn shell defines it as PWD so we will also)
 */
#define PWD_STR	"PWD="
#define PWDLEN	4

/*
 * NAME: setcwd
 *                                                                    
 * FUNCTION: set the current working dir shell variable: PWD
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 *		This static function gets the current working directory and
 *		sets the PWD shell variable to it.
 *                                                                   
 * RETURNS: 	NONE
 */  
static void
setcwd ()
{
char *cwd, *getcwd();

	/* pre-allocate memory for cwd */
	if ((cwd = malloc (PATH_MAX + PWDLEN + 1)))
	{
		/* copy in the variable name and get the cwd */
		strcpy (cwd, PWD_STR);
		if (getcwd (cwd + PWDLEN, PATH_MAX))
		{
			/* if all is well add it to the return string */
			addstr (cwd);
		}
		free (cwd);
	}
	return;
}
/*
 * NAME: addstr
 *                                                                    
 * FUNCTION: add the environment string to static pointer
 *                                                                    
 * EXECUTION ENVIRONMENT: This static function manages the allocated memory
 *		used to store the environment variables. It makes sure there
 *		is enough memory and reallocates memory if not.
 *                                                                   
 * RETURNS: 0 if successful, -1 if failure (errno set to ENOMEM by malloc() )
 */  
static  int
addstr (register char *str)
{
register int siz;

	/* initial allocation of penv memory */
	if (penvsiz == -1)
	{
		penvsiz = PENVSIZ;
		siz = sizeof(char *) * penvsiz;
		if ((penv = (char **)malloc(siz)) == NULL)
			return (-1);
	}

	/* get more memory if needed */
	if (argcount == (penvsiz -1))
	{
		penvsiz += PENVSIZ;
		siz = sizeof(char *) * penvsiz;
		if ((penv = (char **)realloc (penv,siz)) == NULL)
			return (-1);
	}

	/* allocate space to hold the given string */
	siz = strlen(str) + 1;
	if ((penv[argcount] = (char *)malloc(siz)) == NULL)
		return (-1);

	/* add the environment string to the penv array */
	strcpy(penv[argcount],str);

	/* adjust array size (to keep the count) */
	argcount++;

	return (0);
}
/*
 * NAME: getpenv
 *                                                                    
 * FUNCTION: get the process environment
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This is a library call. It returns an allocated buffer containing
 *	the process environment variables. This buffer can be freed by caller.
 *	The environment variables are retrieved from the user info area in the 
 *	kernel via the usrinfo system call and from the **environ char pointer.
 *
 *	The 'which' argument specifies which environment variable set the caller
 *	wants. PENV_USR will return the environment out of 'environ' and 
 *	PENV_SYS will return the variables from the usrinfo (). Specifying
 *	(PENV_USR || PENV_SYS) will return both the 'environ' and
 * 	the usrinfo() variables.
 *
 *	The current working dir is always included in the environment as:
 *	'PWD=[current working dir]'. This is the way korn shell defines it so
 *	we will adopt the same convention.
 *	
 * RETURNS: string of environment variables;
 *	   if failure return NULL and errno will be:
 *
 *		ENOMEM - no memory left to allocate
 *		EPERM  - caller must be root (setusrinfo (GETUINFO) requires it)
 */  

char **
getpenv (int which)
{
int	cwd_defined;

	/* initialize */
	penv = (char **)NULL;		/* the environment string */
	penvsiz = -1;		/* size of environment string */
	argcount = 0;		/* count of penv arguments */
	cwd_defined = 0;	/* indicates whether working dir is defined */

	/* check the mode flags */
	if (checkmode(which))
	{
		errno = EINVAL;
		return ((char **)NULL);
	}

	/* add the public environ variables */
	if (which & PENV_USR)
	{
		addstr (PENV_USRSTR);
		{
		register char	**e;

			for (e = environ; *e; e++)
			{
				/* check for current dir shell variable */
				if (strncmp (*e, PWD_STR, PWDLEN) == 0)
				{
					/* set the current working directory */
					setcwd ();
					cwd_defined = 1;
					continue;
				}
				/* add each 'attr=value' string */
				if (addstr (*e) < 0)
				{
					/* no memory */
					return (NULL);
				}
			}
		}
		/* get working directory add it as PWD='curdir' to env */
		if (!cwd_defined)
			setcwd ();
	}
	/* add the private usrinfo variables */
	if (which & PENV_SYS)
	{
		addstr (PENV_SYSSTR);
		{
		register char	*buf, *u;

			/* get usr info from kernel */
			if ((buf = malloc (UINFOSIZ)) == NULL)
			{
				return (NULL);
			}
			if (usrinfo (GETUINFO, buf, UINFOSIZ) < 0)
			{
				/* usrinfo set errno so just return */
				return (NULL);
			}
			u = buf;
			/* buf is double null terminated */
			while (*u)
			{
				/* add each 'attr=value' string */
				if (addstr (u) < 0)
				{
					/* no memory */
					return (NULL);
				}
				while (*u++) ;
			}
			free (buf);
		}
	}

	/* terminate the penv array with a null pointer */
	penv[argcount] = NULL;

	return (penv);
}

/*
 * NAME checkmode - check the mode for valid values
 *
 * RETURN  - ok or not.
 */
static int
checkmode(int mode)
{
	if (!mode)
		return (-1);

	if (mode & PENV_USR)
		mode &= ~PENV_USR;

	if (mode & PENV_SYS)
		mode &= ~PENV_SYS;

	if (mode)
		return (-1);

	else 
		return (0);
}

