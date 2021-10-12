static char sccsid[] = "@(#)90	1.30.1.3  src/bos/usr/ccs/lib/libs/newpass.c, libs, bos411, 9428A410j 10/12/93 09:22:23";
/*
 *   COMPONENT_NAME: LIBS
 *
 *   FUNCTIONS: checkforadm
 *		checkold
 *		getadmpass
 *		getnewpass
 *		hangup_catch
 *		newpass
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <userpw.h>		/* for getuserpw () */
#include <usersec.h>		/* for S_READ and S_WRITE */
#include <pwd.h>
#include <ctype.h>		
#include <sys/types.h>
#include <sys/errno.h>
#include <stdlib.h>		/* for free() */
#include <string.h>		/* For strncpy() */
#include <unistd.h>		/* For getuid() */
#include <sys/signal.h>		/* For signal stuff */
#include <userconf.h>		/* to get password constraints */
#include "libs.h"		/* for PWD_FILENAME */

#define PROMPTSIZE	160	/* Maximum size a passwd prompt can be */

static	int	hangup_caught = 0;

/*
 * Prototypes.
 */
	int	getnewpass(char **, struct userpw *, char *);
	int	getadmpass(char **, struct userpw *);
	int	checkold(  char **, struct userpw *);
extern	char	*getpass();
extern	char	*crypt();	/* encryption */
extern  char	*_pw_encrypt(char *);
extern	void	setpwent(void);
extern	void	endpwent(void);


/*
 * NAME: hangup_catch
 *
 * FUNCTION: catch hangup signals and set a flag
 *
 * EXECUTION ENVIRONMENT: software signal processing
 *
 * RETURNS: NONE
 */
void
hangup_catch (int signal)
{
	hangup_caught = 1;
}


/*
 * NAME: newpass
 *                                                                    
 * NOTES:   This function is used for local only password administration,
 *	    via AIX password administrators.  The preferred interface is 
 *	    chpass() since it is cognizant of network passwords and 
 *	    is used by the vast majority of password changers.   This 
 *	    routine is only used by the pwdadm command.
 *
 * FUNCTION: change a user passwd
 *                                                                    
 *	This function changes the password of the user specified by the 
 *	user parameter.  If the caller is the root user no authentication
 *	is required.  If the caller is changing another user's password
 *	he/she is required to know the other user's old password or the
 *	root user's password.  If the caller is an administrative user
 *	he/she is required to give their password.
 *
 *	Only the root user can change an administrative user's password.
 *
 *	File descriptors 0 and 1 must be a terminal or newpass() will fail.
 *
 * RETURNS: new password on success
 *
 *	    NULL and errno on failure
 *
 *		EPERM   if stdin and stdout are not associated with 
 *			terminal devices
 *		EPERM   you are not allowed to change an administrators
 *			password.
 *		EINVAL  invalid parameters passed
 *		ESAD	user did not meet criteria to change password.
 *		ENOENT  user could not be found
 */  
char * 
newpass(struct userpw *pw)
{
	char	*newpassword;	/* the new password that is returned */
	struct	sigaction nintact,ointact;
	int	tmp_errno;
	int	rc;


	/* check for user name in passwd structure */
	if (!pw || !pw->upw_name)
	{
		errno = EINVAL;
		return(NULL);
	}

	/* check for administrative flag */
	if (checkforadm(pw->upw_flags))
		return(NULL);

	/*
	 * Set up the signal handler for a hangup signal.  This is
	 * so a line drop can be detected and the user's password not
	 * set to "".
	 */

	hangup_caught = 0;
	nintact.sa_handler = hangup_catch;
	sigemptyset(&(nintact.sa_mask));
	nintact.sa_flags=0;
	sigaction(SIGHUP,&nintact,&ointact);

	/*
	 * If the ADMCHG flag is set, authenticate the user with their
	 * own password, rather than the user being changed.  Otherwise,
	 * use the password of the user being changed.
	 */

	if (pw->upw_flags & PW_ADMCHG)
		rc = getadmpass (&newpassword, pw);
	else
		rc = checkold (&newpassword, pw);
	tmp_errno = errno;

	/*
	 * Undo the signal catching for hangup.  The hangup signal will
	 * have to be propagated back to this process after the old
	 * signal handler has been restored.
	 */

	sigaction (SIGHUP, &ointact, NULL);

	if (hangup_caught)
	{
		kill (getpid (), SIGHUP);
		return NULL;
	}

	if (rc)
	{
		errno = tmp_errno;
		return NULL;
	}
	return newpassword;
}


/*
 * NAME:     getnewpass
 *
 * FUNCTION: Prompt the user for a new password.  The user must 
 * 	     supply two consecutive passwords the exact same.  The
 *  	     password will also be checked to see if it conforms to 
 * 	     the password restrictions no matter who invokes it unless
 *	     pw->upw_flags contains PW_NOCHECK.
 *
 * RETURNS:  0 for success
 *	    -1 for failure
 */
int
getnewpass(char **new, struct userpw *pw, char *opw)
{
	char	*tp;			/* temporary ptr */
	char	*message = (char *) NULL;
	char	prompt[PROMPTSIZE];	/* password prompt */
	char	passwd1[MAX_PASS+1];	/* passwd from first prompt  */
	char	passwd2[MAX_PASS+1];	/* passwd from second prompts */
	MsgBuf	mb;
	int	tmp_errno;
	int	rc;

	_MBInitialize(&mb, &message);
	while(1)	
	{
		sprintf(prompt, MSGSTR(M_PWNEW, DEF_PWNEW), pw->upw_name);

		/*	
		 * Get the first password.  Passwords are only significant
		 * to eight characters.
		 */
		if ((tp = getpass(prompt)) == NULL)
			return -1;


		strncpy (passwd1, tp, MAX_PASS);
		passwd1[PW_PASSLEN] = '\0';

		rc = _local_passwdrestrictions(pw, passwd1, opw, FALSE, &mb);
		tmp_errno = errno;

		if (message)
			fprintf(stderr, "%s", message);
		_MBDiscard(&mb);

		if (rc)
		{
			memset(passwd1, 0, sizeof(passwd1));
			if (rc == 1)
				continue;
			errno = (rc == 2) ? EPERM : tmp_errno;
			return -1;
		}

		/* 
		 * Check for Null passwords which are accepted 
		 */
		if (strlen (passwd1) == 0)
		{
			*new = "";
			break;
		}

		sprintf(prompt, MSGSTR(M_ENTPROMPT, DEF_ENTPROMPT), 
			pw->upw_name);

		/* 
		 * try getting password again
		 */
		if ((tp = getpass(prompt)) == NULL) 
		{
			memset(passwd1, 0, sizeof passwd1);
			return -1;
		}


		strncpy(passwd2, tp, MAX_PASS);
		passwd2[PW_PASSLEN] = '\0';

		if (strcmp(passwd1, passwd2)) 
		{
			fprintf(stderr, MSGSTR (M_NOMATCH, DEF_NOMATCH));
			memset(passwd1, 0, sizeof(passwd1));
			memset(passwd2, 0, sizeof(passwd1));
			continue;
		}

		/* New password successful */
		*new = _pw_encrypt(passwd1);
		break;
	}


	memset(passwd1, 0, sizeof(passwd1));
	memset(passwd2, 0, sizeof(passwd2));
	return (0);
} 


/*
 * Name : getadmpass
 *
 * Function : This routine is called when an administrative change is
 *	      occurring on the password.  
 *          
 *	      Administrative users (other than root) will have to supply 
 *	      their own password before they may change another user's 
 * 	      password (in case someone left a terminal unattended).
 *
 *	      The root user is not required to supply his/her password 
 *	      because the root user has discretionary access to all the 
 *	      security databases (ie. if you left a terminal unattended as 
 *	      the root user, you can't be stopped).
 *
 * Returns:   0 success
 *	     -1 and errno = ESAD || ENOENT on failure
 */
int
getadmpass(char **pass, struct userpw *opw)
{
	int	realuid;		/* real user id */
	char	*realname;		/* pointer to name of the real user */
	struct	passwd	*p;	        /* pointer to passwd struct */
	char	*tp;			/* pointer to password */
	char	prompt[PROMPTSIZE];	/* place to put prompt string */
	char	opasswd[MAX_PASS+1];	/* old password */
	struct  userpw	pw;		/* structure for getnewpass */
	char	crypted[PW_CRYPTLEN+1];	/* tmp crypted passwd  */

	/*
	 * The old password is used to check for compliance with the 
	 * password restrictions and I don't want to have an invalid 
	 * string to compare against. 
	 */
	opasswd[0] = '\0';

	/*
	 * Obtain our own copy of the user's userpw struct.  We aren't 
	 * sure that the user hasn't passed us a struct obtained from 
	 * the getuserpw() routine.
	 */
	strcpy(pw.upw_name, opw->upw_name);
	pw.upw_passwd = strncpy(crypted, opw->upw_passwd, sizeof(crypted));
	crypted[sizeof(crypted)-1] = '\0';
	pw.upw_flags = opw->upw_flags;
	pw.upw_lastupdate = opw->upw_lastupdate;
	
	/*
	 *  get the real user name
	 */
	setuserdb(S_READ);
	realuid = getuid();
	if ((realname = IDtouser((uid_t)realuid)) == NULL)
	{
		enduserdb();
		errno = ENOENT;
		return(-1);
	}
	if((realname = strdup(realname)) == (char *)NULL)
	{
		enduserdb();
		return(-1);
	}
	enduserdb();

	/* 
	 *  get the real user's passwd struct 
	 */
	setpwent();
	if ((p = getpwnam(realname)) == NULL)
	{
		endpwent();
		free(realname);
		errno = ENOENT;
		return(-1);
	}
	endpwent();

	/* 
	 * If I have a password and I am not the root user
	 * then I will have to supply my passwd. 
	 */
	if (p->pw_passwd && p->pw_passwd[0] && realuid)
	{
		/* prompt for and check administrator's passwd */
		sprintf(prompt,
			(char *)MSGSTR(M_PWPROMPT, DEF_PWPROMPT), p->pw_name);

		if ((tp = getpass (prompt)) == NULL)
		{
			free(realname);
			errno = ESAD;
			return -1;
		}

		strncpy (opasswd, tp, MAX_PASS);
		opasswd[PW_PASSLEN] = '\0';
		tp = crypt (opasswd, p->pw_passwd);

		if (strcmp (p->pw_passwd, tp) != 0) 
		{
			free(realname);
			fprintf(stderr, (char *)MSGSTR(M_MATCH,DEF_MATCH));
			errno = ESAD;
			return (-1);
		}
		/*  
		 * Now I need to clear the old password that
		 * I have just given.  I need to do this because
		 * it makes no sense to force the users password
		 * to meet the password restrictions when I am
		 * comparing against my password.
		 */ 
		memset(opasswd, 0, sizeof(opasswd));
	}

	/* get user's new password */
	free(realname);

	if (getnewpass(pass, &pw, opasswd))
	{
		errno = ESAD;
		return(-1);
	}
	return(0);

}


/*
 * NAME: checkold
 *
 * FUNCTION: This routine is called when the password change is not
 *	     an administrative change.  Therefore, if you are a normal
 *	     user changing your password, you will be required to
 *	     give your old password.  If you are a user changing another
 *	     user's password, you will be required to give either the
 *	     root's or the other user's old password.  If you are the
 *	     root user then no authentication will be required.
 *	    
 * RETURNS:  0 success
 *	    -1 failure
 * 
 * ERRNOS:   ESAD   - Authentication failure
 *	     ENOMEM - malloc failure
 */
int
checkold(char **newpassword, struct userpw *opw)
{
	int	realuid;		/* real user's id       */
	char 	*realname;		/* real user's name     */
	char 	*rootname;		/* root user's name	*/
	char	*tp;			/* temporary pointer    */
	struct  userpw	p;		/* user's passwd struct */
	struct  userpw	*pw = &p;	/* user's passwd struct */
 	struct	passwd	*rootpwd;	/* root user's password */
	char	prompt[PROMPTSIZE];	/* place to put prompt  */
	char	opasswd[MAX_PASS+1];	/* old password		*/
	char	encrypt[PW_CRYPTLEN+1];	/* encrypted passwd copy*/

	/*
	 * The old password is used to check for compliance with the 
         * password restrictions and I don't want to have an invalid 
	 * string to compare against. 
	 */
	opasswd[0] = '\0';

	/*
	 * Obtain our own copy of the user's userpw struct.  We aren't 
	 * sure that the user hasn't passed us a struct obtained from 
	 * the getuserpw() routine.
	 */
	strcpy(pw->upw_name, opw->upw_name);
	if (opw->upw_passwd && opw->upw_passwd[0])
	{
		pw->upw_passwd =
			strncpy(encrypt, opw->upw_passwd, sizeof(encrypt));
		encrypt[sizeof(encrypt)-1] = '\0';
	}
	else
		pw->upw_passwd = (char *)NULL;

	pw->upw_flags = opw->upw_flags;
	pw->upw_lastupdate = opw->upw_lastupdate;


	setuserdb(S_READ);
	/* 
	 * get the real user id and the real user name
	 */
	realuid = getuid();
 	if ((realname = IDtouser(realuid)) == NULL)
 	{
		enduserdb();
 		errno = ESAD;
 		return -1;
 	}
	/* malloc storage to hold the name */
	if((realname = strdup(realname)) == (char *)NULL) 
	{
		enduserdb();
		return (-1);
	}

	/* 
	 * If the user has a password and I am not the root user
	 * then I will have to supply either the user's old password
	 * or the root users password.
	 */
	if (pw->upw_passwd && pw->upw_passwd[0] && realuid)
	{
		/*
		 * get the root user's name (normally "root")
		 */
		if ((rootname = IDtouser(0)) == NULL)
		{
			free(realname);
			enduserdb();
			errno = ESAD;
			return -1;
		}
		enduserdb();
		if((rootname = strdup(rootname)) == (char *)NULL)
			return(-1);

		/*
		 * If I am changing my own passwd or I am another person     
	         * changing the root user's password then my prompt will
		 * be to enter the old password.  Otherwise I am changing
		 * another user and my prompt should be to supply either
		 * that user's old password or the root user's password.
		 */
		if (!strcmp(realname, pw->upw_name) || 
		    !strcmp(rootname, pw->upw_name)) 
			sprintf(prompt,(char *)MSGSTR(M_PWOLD, DEF_PWOLD),
				pw->upw_name);
		else
			sprintf(prompt,(char *)MSGSTR(M_PWEITHER, DEF_PWEITHER),
                                rootname, pw->upw_name);

		if ((tp = getpass(prompt)) == NULL)
		{
			free(realname);
			free(rootname);
			errno = ESAD;
			return -1;
		}

		strncpy(opasswd, tp, MAX_PASS);
		opasswd[PW_PASSLEN] = '\0';
	
		tp = crypt(opasswd, pw->upw_passwd);

		/*
	 	 * Authentication is successful if:
	 	 *  1- the given password matches the 'user' password 
	 	 *  2- the given password matches the password of the root user
	 	 */
		if (strcmp(pw->upw_passwd, tp) != 0) 
		{
			/* 
			 * User did not supply the correct old password, 
			 * therefore lets check to see if we got root's
			 * password.
			 */
			setpwent();
			if ((rootpwd = getpwnam(rootname)) == NULL)
			{
				free(realname);
				free(rootname);
				endpwent();
				fprintf(stderr, MSGSTR(M_MATCH,DEF_MATCH));
				errno = ESAD;
				return (-1);
			}
			endpwent();

			/*
			 * if root's password is NULL and the
			 * user typed in garbage then we fail
			 */
			if (!rootpwd->pw_passwd || !rootpwd->pw_passwd[0])
			{
				if (opasswd && opasswd[0])
				{
					free(realname);
					free(rootname);
					fprintf(stderr,  
						MSGSTR(M_MATCH,DEF_MATCH));
					errno = ESAD;
					return (-1);
				}
			}

			tp = crypt(opasswd, rootpwd->pw_passwd);

			/* 
			 * The final straw breaks when the user has not
			 * supplied root's passwd either.
			 */
			if (strcmp(rootpwd->pw_passwd, tp))
			{
				free(realname);
				free(rootname);
				fprintf(stderr, MSGSTR(M_MATCH,DEF_MATCH));
				errno = ESAD;
				return (-1);
			}
			/*  
			 * Now I need to clear the old password that
			 * I have just given.  I need to do this because
			 * it makes no sense to force the users password
			 * to meet the password restrictions when I am
			 * comparing against the root user's password.
			 */ 
			memset(opasswd, 0, sizeof(opasswd));
		} 
		free(rootname);  /* No longer need root users name */
	}
	free(realname); /* No longer need real users name */
	enduserdb();    /* No longer will access database */


	/*
	 * Either I have supplied the user's old password, or I have
	 * supplied the root user's password, or I am the root user
	 * and I did not have to supply a password.
	 */
	if (getnewpass(newpassword, pw, opasswd))
	{
		memset(opasswd, 0, sizeof(opasswd));
		errno = ESAD;
		return(-1);
	}

	memset(opasswd, 0, sizeof(opasswd));
	return(0);
}


/*
 * NAME:     checkforadm
 *
 * FUNCTION: If the user is an ADMIN user, then only the root
 *	     user can change this person's password.
 *
 * RETURNS:  -1 and errno = EPERM   User cannot change password
 *	      0  		    User is allowed to change password
 */
int
checkforadm(ulong flags)
{
	/* if this flag is set only root can change the passwd */
	if ((flags & PW_ADMIN) && getuid())
	{
		errno = EPERM;
		return(-1);
	}
	return(0);
}
