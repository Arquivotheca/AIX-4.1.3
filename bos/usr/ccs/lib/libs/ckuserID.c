static char sccsid[] = "@(#)85	1.23.1.5  src/bos/usr/ccs/lib/libs/ckuserID.c, libs, bos411, 9428A410j 8/24/93 13:13:47";
/*
 *   COMPONENT_NAME: LIBS	
 *
 *   FUNCTIONS: authmethods
 *		ckuserID
 *		sysauth
 *		getauthprog
 *		getnewyppasswd
 *		execyp
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/errno.h>
#include <usersec.h>	/* user attributes */
#include <userpw.h>	/* user passwd attributes */
#include <userconf.h>	/* security configuration attributes */
#include <login.h>
#include <pwd.h>
#include <sys/id.h>
#include <sys/audit.h>
#include "libs.h"

#define AUTH_AUDIT(rc,msg)\
			 auditwrite("USER_Login",rc,msg,strlen(msg)+1,NULL)
#define CHPASS_AUDIT(rc,msg)\
			 auditwrite("PASSWORD_Change",rc,msg,strlen(msg)+1,NULL)

int 	getnewyppasswd(struct userpw *pw);
int	execyp(char *);
int	ckuserID(char *, int);

static	int	authmethods (char *, char *, char *, int);
static  char 	**getauthprog(char *, char *);
static  int 	sysauth(char *, char *);
static	int	execute(char **);

/*
 * NAME: ckuserID		OBSOLETE 7/93
 *                                                                    
 * FUNCTION: authenticate the user specified according to 'mode'
 *
 * NOTES:    This routine requires an interactive session with the user who 
 *	     has a controlling tty.  The alternate routine authenticate()
 *	     is the mandated interface that new programs should code to.  This
 *	     routine is left for old style programs that need to support the
 *	     AIX 3.1 and 3.2 authentication methods.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * 	This is a library routine. The user is authenticated with
 *	each authentication method specified in the user database.
 *	The user must pass each primary authentication method to succeed.
 *	If the mode is S_LOGIN then secondary authentication methods 
 *	will be invoked.
 *
 *	Each user has two authentication attributes in the user database.
 *	One, auth1, for primary authentication, and ,auth2, for secondary
 *	authentication. The value of these attributes are as follows:
 *
 *		auth1 = "method:name,method:name, ..."
 *		auth2 = "method:name,method:name, ..."
 *
 *	Where, 'method' is either 'SYSTEM' or the name of a stanza in 
 *	LOGIN_CONFIG file.  If it is the latter then the
 *	'program' attribute will specify the executable program to
 *	be run for authentication. The 'name' is the user name.
 * 
 * RETURNS: ckuserID returns a status code:
 *
 *	if status code equals 0 then success
 *	if status code less than 0 then warning was encountered
 *	if status code greater than 0 then an error occured
 */  
int
ckuserID (char *user, int mode)
{
	char	*methods;		/* return from getuserattr           */
	char	shortname[PW_NAMELEN];	/* shortened (max 8 bytes) user name */

	/* check arguments */
	if (!user || !((mode & S_PRIMARY) || (mode & S_SECONDARY)))
	{
		errno = EINVAL;
		return (-1);
	}

	/* 
	 * Create shortened AIX name since the user may have logged in
	 * with a DCE cellname as part of the login sequence.
	 */
	_normalize_username(shortname, user);

	/*
	 * primary authentication: this authentication is based 
	 * on the auth1 value for this user in /etc/security/user file.
	 * the authentication method "SYSTEM" is the standard
	 * password authentication method.  if there is no value
	 * for auth1 the method defaults to "SYSTEM".
	 */
	if (mode & S_PRIMARY)
	{
		if (getuserattr(shortname, S_AUTH1,(void *)&methods, SEC_LIST))
			methods = "SYSTEM";

		/* indicate primary by setting flag to one */
		if (authmethods (user, shortname, methods, 1))
		{
			/* failed primary authentication */
			errno = ESAD;
			return (-1);
		}
	}

	/*
	 * secondary authentication: this authentication is based 
	 * on the auth2 value for this user in /etc/security/user file.
	 * it is not a requirement to pass this authentication.
	 */
	if (mode & S_SECONDARY)
	{
		if (getuserattr(shortname, S_AUTH2, (void *)&methods, SEC_LIST))
			return (0);
		else
		{
			if (methods && *methods)
				/* indicate secondary by setting flag to zero */
				(void) authmethods(user, shortname, methods, 0);
		}

	}

	return (0);
}

/*
 * NAME: authmethods		Obsolete (7/93)
 *                                                                    
 * FUNCTION:
 *	The methods parameter is a string of the form 'method;name'.
 *	Each method is 'invoked' with the associated user.
 *		"method;user\0method;user\0 ... \0\0"
 *
 *	If the 'primary' parameter is set then the first failed
 *	method will fail the authentication. If 'primary' is not
 *	set (secondary) then each method will be 'invoked' regardless
 *	of the value of the previous method.
 *                                                                   
 *	Methods are one of:
 *		SYSTEM   - the typical UNIX authentication method with 
 *		           passwords.  This method has been expanded to
 *			   include load modules and login rules.  Reference
 *			   the authenticate routine.
 *		NONE     - no authentication specified
 *		token	 - an administrator defined token which is described
 *			   in login.cfg 
 *
 * NOTES:	Helper routine for ckuserID() which has been marked as
 *		obsolete.
 *
 * EXECUTION ENVIRONMENT: static 
 *				 
 * RETURNS: 
 *		0 if authentication successful and 1 if not 
 *
 * PASSED:
 *	char	*authuser;	 the user being authenticated
 *	char    *shortname;	 User's shortened (max 8 bytes) name
 *	char	*methods;	 authentication methods for above user
 *	int	primary; 	 indicating primary or secondary auth method
 */  
static int
authmethods (char *authuser, char *shortname, char *methods, int primary)
{
	int	rc;		/* return code */
 	char	*user;		/* user whose method is being used */
	char	*meth;		/* a method of authentication from 'methods'*/
	char	*mp;		/* temporary pointer for auth methods */
	int	donesysauth = 0;/* Signal variable for system authentication */

	/*
	 * parse the methods list
	 * method;user\0method2;user2...\0\0
	 */

	while (*methods)
	{
		/* get some space and copy in the methods string */
		if ((meth = malloc (strlen (methods) + 1)) == NULL)
		{
			rc = 1;
			break;
		}
		strcpy (meth, methods);
		mp = meth;

		/* look for the end of the string or a ";" */
		while (*mp && (*mp != ';'))
			mp++;

		/*
		 * Get the user out of the methods list.  The string 
		 * should be: method;user.  Otherwise set the user to
		 * the name being authenticated.
		 */
		if (*mp == ';')
		{
			*mp++ = '\0';
			user = mp;
		}
		else
			user = authuser;

		/*
		 * increment methods pointer to catch up to where 
		 * we are now in the meth string.
		 */
		while (*methods++);

		/* 
		 * If the method is "SYSTEM" do password authentication.
		 * If the method is "NONE" then no authentication required.
		 * Otherwise the administrator has his/her own method and
		 * we have to exec it.
		 */
		if (strcmp (meth, "SYSTEM") == 0)
		{
			rc = sysauth(user, shortname);
			donesysauth = 1;	/* sysauth() complete */
		}
		else if (strcmp (meth, "NONE") == 0)
			rc = 0;
		else
		{
			char	**authprog;

			authprog = getauthprog (meth, user);
			if (authprog)
			{
			int	child;
			int	pid;
			int	status;

				if ((child = kfork()) == 0)
				{
					execvp (authprog[0], &authprog[0]);
					_exit (-1);
				}
				while ((pid = wait (&status)))	
				{
					if (child == pid)
					{
						if (status & 0x00FF)
							/* due to signal */
							rc = status & 0x00FF;
						else
							/* return exit code */
							rc = status & 0xFF00;
						break;
					}
				}
			}
			else
			{
				/* methods not found but expected */
				rc = EINVAL;
			}
		}

		/* free up space from earlier malloc */
		free ((void *)meth);

		/* if we are checking primary and have failed return */
		if (primary && rc)
			break;

	}

	/*
	 * sysauth() implements the system password authentication.  This
	 * implementation is based on the new SYSTEM grammar used by 
	 * authenticate().  If we have arrived here and have yet to process
	 * sysauth(), then the user has explicity removed "SYSTEM" from the
	 * auth1 line or stated "NONE".  Since the SYSTEM grammar is the 
	 * preferred authentication means (and obsoletes auth1/auth2),
	 * the user must explicitly cut off the SYSTEM grammar as well
	 * in order to bypass sysauth().  DON'T CHANGE THIS CODE POLICY.
	 */
	if (!donesysauth && primary)
		rc = sysauth(authuser, shortname);
		
	return (rc);
}

/*
 * NAME: sysauth() 		OBSOLETE (7/93)
 *
 * FUNCTION: This is ckuserID()'s front end to authenticate() and chpass().
 *	     Previously this routine took care of password authentication
 *	     and password changes.  The authenticate() and chpass()
 *	     routines now handle this function and the new sysauth()
 *	     function is a wrapper call.  ckuserID() in general is 
 *	     a poor interface since it requires a tty for it's 
 *	     functionality.  Users' should migrate to the new 
 *	     authenticate()/chpass() functions.   The login/su commands 
 *	     will continue with ckuserID() (for at least one more release)
 *	     in order to provide the AUTH1/AUTH2 methods.
 *
 * NOTES:    Helper routine for ckuserID() which has been marked as obsolete.
 *
 * EXECUTION ENVIRONMENT: static
 *
 * RETURNS:   1 - authentication not successful
 *            0 - authentication successful
 */
static int
sysauth(char *username, char *shortname)
{
	int	rc;			 /* return code 		      */
	int 	reenter;		 /* recall authenticate()             */
	uid_t	uid;		 	 /* User id of target user	      */
	uid_t	curuid;			 /* Current user id		      */
	char 	*op	 = (char *)NULL; /* Pointer to user's old password    */
	char    *tp      = (char *)NULL; /* temporary pointer 		      */
	char	*message = (char *)NULL; /* return message from authenticate()*/
	char	passwd[MAX_PASS+1];	 /* user's cleartext password         */
	char    npasswd[MAX_PASS+1];	 /* user's new cleartext password     */
	
	/*
	 * Call authenticate.  If reenter is not set then this user
	 * is not required to enter a password.  Either the user is not
	 * a valid user, has no password, or has no authentication 
	 * requirements.  Otherwise display the challenge and gather
	 * the password.
	 */ 
	do
	{
		rc = authenticate(username, tp, &reenter, &message);
		if (message)
		{
			if (reenter)
			{
				if (_passwdentry(message, passwd))
				{
					reenter = 0;
					rc = 1;
				}
				tp = passwd;
			}
			else
			{
				/*
				 * If user passed authentication then it is
				 * ok to give him informative messages.  
				 * Otherwise I better just log the error
				 * since I don't want an attacker to gain 
				 * any knowledge.
				 */
				if (rc)
					AUTH_AUDIT(rc,message);
				else
					fprintf(stderr, message);
			}

			free(message);
			message = (char *)NULL;
		}
	} while (reenter);

	if (rc)	return(1);	/* Authentication failed */

	op = tp;	/* Point to password authenticated with */

	/*
	 * User's real user id should be correct for password administration
	 * setuidx() only accepts real and effective flags together. 
	 */
	curuid = getuid();
	if (!getuserattr(shortname, S_ID, &uid, SEC_INT))
	{
        	setuidx(ID_REAL | ID_EFFECTIVE, uid);
		setuidx(ID_EFFECTIVE, curuid);
	}

	/*
	 * Check password change requirements and call the chpass()
	 * interface if necessary.
	 */
	rc = passwdexpired(username, &message);
	
	if (message)
	{
		fputs(message, stderr);
		free(message);
		message = (char *)NULL;
	}

	/*
	 * If a normal password change is required, or an administrative
	 * password change is required and root is logging in, then
	 * invoke the chpass() interface.
	 */
	if ((rc == 1) || (rc == 2 && uid == 0))
	{
		/*
		 * chpass() will stay in this loop until the user
	 	 * enters a valid new password.
	 	 */
		do
		{
			/* 
			 * Set initial entry parameter to old password,
			 * so we do not have to supply it again.
			 */
			tp = op;	
			do
			{
				rc = chpass(username, tp, &reenter, &message);
				if (message)
				{
					if (reenter)
				     	{
					       if(_passwdentry(message,npasswd))
					       {
							reenter = 0;
							rc = 1;
					       }
					       tp = npasswd;
				     	}
				 	else
					       fputs(message, stderr);

					free(message);
					message = (char *)NULL;
				}
			} while (reenter);
		} while (rc == 1);

		if (rc)
			CHPASS_AUDIT(AUDIT_FAIL, shortname);
		else
			CHPASS_AUDIT(AUDIT_OK, shortname);
	}
	else if (rc == -1 && uid == 0) /* Allow root on internal error */
		rc = 0;		
		
        setuidx(ID_REAL | ID_EFFECTIVE, curuid);

	memset(passwd, 0, sizeof(passwd));
	memset(npasswd, 0, sizeof(npasswd));

	return(rc);
}

/*
 * NAME: execyp			OBSOLETE (7/93)
 *
 * FUNCTION: This function changes the yppassword of the user specified 
 *	     by the user parameter by calling yppasswd.  This routine is 
 *	     only sufficient for programs that are tty based (such as pwdadm).
 *	     The real API that should be used is chpass().  
 *
 * NOTES:    This routine is obsolete and commands that use it should either 
 *	     transition to chpass() or be phased out.
 *
 * RETURNS: -1 : error 
 *	     0 : success
 */
int 
execyp(char *nam)
{
        struct  passwd *rpw;
        int rc;
        static char *str = "yppasswd";
        char    *args[3];

        args[0] = "/usr/bin/yppasswd";
        args[1] = nam;
        args[2] = NULL;
        rc = execute(args);

        if ((rc == -1) || (rc == 127))  /* could not fork or exec failed */
        {
                perror("execyp()");
                return(-1);
        }
        if (rc) return(-1);             /* yppasswd must have failed, probably
                                        because the user mistyped his passwd.*/
        return(0);
}

/*
 * NAME: getnewyppasswd		OBSOLETE (7/93)
 *
 * FUNCTION: The yppasswd program is exec'd and /etc/security/passwd is 
 *	     altered.  This routine is only sufficient for programs that
 * 	     are tty based.   The real API that should be used is chpass().  
 *
 * NOTES:    This routine is obsolete and commands that use it should 
 *	     either transition to chpass() or be phased out.
 *
 * RETURNS: -1 : error 
 *	     0 : success
 */
int 
getnewyppasswd(struct userpw *pw)
{
	int rc;

	endpwdb();
	rc = execyp(pw->upw_name);      /* run yppasswd instead */
        if (rc<0) return(rc);

	/* always clear the ADMCHG flag when setting a new password */
	pw->upw_flags &= ~PW_ADMCHG;

	/* set new update time */
	pw->upw_lastupdate = time ((long *) 0);

	/* update /etc/security/passwd */
	setpwdb (S_READ | S_WRITE );
	if (putuserpw (pw))
	{
		endpwdb();
		free((void *)pw->upw_passwd);
		return (-1);
	}
	endpwdb();
	return(rc);
}

/*
 * NAME: getauthprog		OBSOLETE (7/93)
 *                                                                    
 * FUNCTION: gets the authentication program from /etc/security/login.cfg
 *	     and converts it into a form which execvp will take.
 *                                                                    
 * NOTES:    Helper routine for ckuserID() which has been marked as obsolete
 *
 * EXECUTION ENVIRONMENT: static 
 *
 * RETURNS: the arguments or NULL.
 *
 * PASSED:
 *	char 	*methname; stanza name of the authentication method to run
 *	char	*username; user whom we are authenticating
 */  
static char **
getauthprog(char *methname, char *username)
{
	char	*tp;
	int	argc = 3;
	int	siz;
	char	**args = NULL;
	char	*prog;
	char	*pp;
	register int	i;

	/*
	 * get the attribute from /etc/security/login.cfg
	 * methname:
	 *	program = "something"
	 */

	if (getconfattr (methname,SC_AUTHPROGRAM, (void *) &tp, SEC_CHAR))
		return ((char **)NULL);

	if (*tp)
	{
		if ((prog = malloc(strlen(tp) + 2)) != NULL)
		{
			pp = prog;
		
			/* convert string into double-null terminated buffer */
			while (*tp)
			{
				/* skip the string indicators */
				if (*tp == '"')
					tp++;

				/* this is another argument */
				if (*tp == ' ')
				{
					*pp++ = '\0';
					argc++;
				}
				else
					*pp++ = *tp;
				tp++;
			}
			*pp++ = '\0';
			*pp = '\0';
			siz = argc * sizeof(char *);
			if ((args = (char **)malloc(siz)) != NULL)
			{
				/*
				 * build arguments for program
				 * username is last argument
				 */
				for (i = 0;; i++)
				{
					if (!(*prog))
					{
						args[i++] = username;
						args[i] = NULL;
						break;
					}
					args[i] = prog;
					while (*prog++)
						;
				}
			}
		}
	}
	return (args);
}


/*
 * NAME: execute		OBSOLETE (7/93)
 *
 * FUNCTION: Forks and execs the program and then catches the return code.
 *	     Helper routine for getnewyppasswd() which is obsolete.
 *
 * NOTES:    This routine is obsolete and commands that use it should either 
 *	     transition to chpass() or be phased out.
 *
 * RETURNS: Return code from program.
 */
static int
execute(char **args)
{
        int     pid;
        int     status;

        if ((pid = fork()) == 0)        /* child */
        {
                execv(args[0], args);
                _exit(127);
        }

        if (pid == -1)
                return -1;

        while (waitpid(pid, &status, 0) == -1)
                if (errno != EINTR)
                        break;

        return(status);
}
