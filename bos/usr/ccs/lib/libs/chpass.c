static char sccsid[] = "@(#)14	1.8  src/bos/usr/ccs/lib/libs/chpass.c, libs, bos41J 5/12/95 13:57:24";
/*
 *   COMPONENT_NAME: LIBS
 *
 *   FUNCTIONS: chpass
 *		_method_chpass
 *		_system_chpass
 *		_passwd_moved
 *		_pw_encrypt
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/errno.h>
#include <usersec.h>	/* user attributes */
#include <userpw.h>	/* user passwd attributes */
#include <userconf.h>	/* security configuration attributes */
#include <login.h>
#include <string.h>
#include <pwd.h>
#include <ndbm.h>
#include <sys/signal.h>
#include "libs.h"

/*
 * Global routines.
 */
int 	chpass(char *, char *, int *, char **);
char *  _pw_encrypt(char *);

/* 
 * Local routines.
 */
static int  _passwd_moved(char *);
static int  _method_chpass(char *, char *, char *, char *, MsgBuf *);
static int  _system_chpass(char *, char *, struct userpw *, int, char **);

extern DBM  *pwdbm_open();
extern int  pwdbm_update(DBM *, char *, char *, int);

/*
 * Macro to reset internal static variables.
 */
#define RESET\
	{\
		state = INITIAL;\
		admin = 0;\
                if(fname)     { free(fname);     fname     = (char *)NULL; }\
		if(oldpasswd) { free(oldpasswd); oldpasswd = (char *)NULL; }\
		if(newpasswd) { free(newpasswd); newpasswd = (char *)NULL; }\
		if(registry)  { free(registry);  registry  = (char *)NULL; }\
		if(pwd)	      { _pwdfree(pwd);   pwd = (struct passwd *)NULL;}\
		if(upw)	      { _userpwfree(upw);upw = (struct userpw *)NULL;}\
	}
		
/*
 * Macro to end processing.  After this is called chpass() is ready to 
 * process a new user.
 */
#define END(a)\
        {\
                *reenter = 0;\
		RESET;\
                return(a);\
        }

/*
 * Macro to return to the caller with expectations of a recall
 * to chpass().  On reentry to this routine, chpass() will be 
 * in state "b".
 */
#define CONTINUE(b)\
        {\
                *reenter = 1;\
                state = b;\
                return(0);\
        }

/*
 * NAME: chpass
 * 
 * FUNCTION:	Entry point for changing passwords.
 * 
 * RETURNS: reenter variable is used to signal completion of this routine.
 *	    When reenter is false, the following return codes may be checked.
 *
 *		0 - Password change was successful
 *		1 - Password change unsuccessful.  The user should attempt 
 *			again.
 *		2 - Password change unsuccessful.  The user should not attempt 
 *			again.
 *
 *  ERRNO:
 *		ESAD	- User did not provide correct authentication
 *		ENOENT	- User does not exist 
 *		EPERM	- Password can't be changed by user due to permissions
 *		ENOMEM  - malloc failed
 *		EINVAL	- Invalid parameters given or not what was expected.
 */
int 
chpass(char *longname, 	/* user name who is incurring a password change    */
       char *passwd,    /* user's password (either old or new) 		   */ 
       int  *reenter,	/* whether chpass() has finished processing	   */ 
       char **message)	/* message to return to user			   */
{
	static enum {INITIAL, QUERY_NEW, QUERY_AGAIN, CHANGE} state;
	static struct userpw *upw;
	static struct passwd *pwd;
	static char *fname;
	static char *registry;
	static char *oldpasswd;
	static char *newpasswd;
	static char shortname[PW_NAMELEN];
	static int  opwrequired;
	static int  admin;
	struct userpw *tpw;
	MsgBuf mb;
	char   lpasswd[PW_PASSLEN+1];
	char   *newmsg = (char *)NULL;
	int rc;
	int ruid;

        /* The caller must supply a user name */
        if (!longname || (longname[0] == (char)NULL))
        {
                errno = EINVAL;
                END(1);
        }

        /*
         * If I currently have a user name from a previous call and am
         * now being given a different user name then I should reset.
         */
        if (fname && strcmp(fname, longname))
                RESET;

	_MBInitialize(&mb, message);	/* Prepare for return messages */

	switch (state)
	{
	   case INITIAL:
	        /*
                 * Malloc and store full user name from initial call.
                 * This will be rechecked upon reentry.
                 */
                if ((fname = strdup(longname)) == (char *)NULL)
                        END(2);

		/* Retrieve shortened AIX definition of user name */
                _normalize_username(shortname, longname);

                /*
                 * Retrieve "registry" line from the database.
                 * This will describe where the user is administered.
		 * This ensures that password changes always follow the
		 * master server, even if the server is down during login.
		 */
		if ((registry = _getregistry(longname)) == (char *)NULL)
			END(2);

                /*
                 * Check the name resolution to see if this user is defined.
                 * If the user is unknown then we fail.
                 */
		if (_PasswdRetrievalPolicy(shortname, &pwd, &upw, registry))
		{
			_MBReplace(&mb, MSGSTR(M_USRNONEX, DEF_USRNONEX),
				shortname);
			errno = ENOENT;
                        END(2);
		}

		ruid = getuid();
		/*
		 * If I am a normal user or I am the root user and my registry
		 * is non local, and I have a password, then I have to
		 * supply it.
		 */
		opwrequired = ((ruid || (!ruid && !ISLOCAL(registry)))
			      && (pwd->pw_passwd && pwd->pw_passwd[0] != '\0'));

		/*
		 * If I'm not the root user then I can't change an
		 * account with the ADMIN flag.   
		 */
		if ((upw->upw_flags & PW_ADMIN) && ruid)
		{
			_MBReplace(&mb, MSGSTR(M_NOAUTH,DEF_NOAUTH), shortname);
			errno = EPERM;
			END(2);
		}

		/*
	 	 * If user did not supply the old password during 
		 * the initial invocation, and I'm the root user, 
		 * and this user is administered locally, and I'm 
		 * not changing the root user's password, then this 
		 * is a local administrative change.
		 */
	        if (!passwd && !ruid && strcmp(shortname, IDtouser(0)))
			admin = TRUE;

		/*
		 * If user did not supply the old password during the
		 * initial invocation and one is required, then I have 
		 * to prompt for the old password.
		 *
		 * Otherwise there won't be any prompt returning from
		 * our initial state.
		 */
		if (!passwd && opwrequired)
		{
			if (_MBReplace(&mb, 
			    MSGSTR(M_PWOLD, DEF_PWOLD), shortname))
				END(2);
		}
		else
			_MBDiscard(&mb);

		/*
		 * At this point I may or may not have a message in my
		 * return buffer, however I'm about to return to my
		 * caller.  If I have a message, then I am expecting the
		 * old password.  The absence of a message will be a signal
		 * to the caller that the first state need not be complied
		 * with (example: root changing local user's password).
		 * Many callers (such as passwd) need to know the exact
		 * state we are in, so we shouldn't go directly into
		 * querying for new passwords. 
		*/
		CONTINUE(QUERY_NEW);

	   /*
	    * Check the old password and query for the new password.
	    */ 
	   case QUERY_NEW:

		/* If an old password was required, then check it. */
		if (opwrequired)
		{
			/*
			 * If I didn't supply a password, or the password
			 * was null then no match can occur.  Otherwise
			 * if I received a password, I should store it.
			 * If this is a password for a locally administered
			 * user then I can check it immediately for correctness.
			 */
			if (!passwd || (passwd && (*passwd == (char)NULL)))
			{
				_MBReplace(&mb, MSGSTR(M_MATCH, DEF_MATCH));
				errno = ESAD;
				END(2);
			}
			else
			{
				if ((oldpasswd=strdup(passwd)) == (char *)NULL)
					END(2);

				if (ISNIS(registry) || ISLOCAL(registry))
				{
					if (_crypt_authenticate(registry, 
							longname,  passwd))
					{
						_MBReplace(&mb, MSGSTR(M_MATCH, 
							DEF_MATCH));
						END(2);
					}
				}
			}
		}

		/* Query for the new password. */
		if (_MBReplace(&mb, MSGSTR(M_PWNEW, DEF_PWNEW), shortname))
			END(2);

		CONTINUE(QUERY_AGAIN);

	   /*
	    * Check the password restrictions and ask user to reenter
	    * password to ensure correctness.
	    */
	   case QUERY_AGAIN:

		if (passwd)
			if ((newpasswd = strdup(passwd)) == (char *)NULL)
				END(2);

		rc = passwdrestrictions(registry, upw, longname, newpasswd,
		    		oldpasswd, admin, &mb);

		/*
		 * If user is incapable of changing this password, then
		 * return 2, otherwise this is a standard restrictions problem
		 * that the user will have to meet by supplying another
		 * new password.
		 */
		if (rc == 2) { END(2); }
		else if (rc) { END(1); }

		/* Query for the second password. */
		if (_MBReplace(&mb, 
		    MSGSTR(M_ENTPROMPT, DEF_ENTPROMPT), shortname))
			END(2);

		CONTINUE(CHANGE);

	   /* 
  	    * Make sure new passwords match and make the change
	    */
	   case CHANGE:

		/*
		 * Both Local and NIS expect passwords of 8 or less bytes.
		 */
		if (ISLOCAL(registry) || ISNIS(registry))
		{
			if (passwd && (strlen(passwd) > PW_PASSLEN))
			{
				passwd = strncpy(lpasswd, passwd, PW_PASSLEN);
				passwd[PW_PASSLEN] = '\0';
			}

			if (oldpasswd && (strlen(oldpasswd) > PW_PASSLEN))
				oldpasswd[PW_PASSLEN] = '\0';
			if (newpasswd && (strlen(newpasswd) > PW_PASSLEN))
				newpasswd[PW_PASSLEN] = '\0';
		}

	        if (strcmp(passwd, newpasswd))
                {
			if (_MBReplace(&mb, MSGSTR(M_NOMATCH, DEF_NOMATCH)))
				END(2);

			errno = EINVAL;
			END(1);
		}

		/*
		 * Finally invoke the appropriate method or local routine
		 * to make the password change.  
		 */
		if (ISLOCAL(registry))
		{
			if ((rc = _system_chpass(shortname, newpasswd, upw, 
			    admin, &newmsg)) || (rc = _passwd_moved(shortname)))
				_MBReplace(&mb, MSGSTR(M_PUTERR, DEF_PUTERR));
			if (newmsg)
			{
				 _MBAppend(&mb, newmsg);
					free (newmsg);
			}

		}
		else
			rc = _method_chpass(registry, longname, oldpasswd,
					newpasswd, &mb);
	}
	END(rc);
}

/*
 * NAME: _system_chpass
 *                                                                    
 * FUNCTION: Encrypts the cleartext password, stores it in the userpw
 *	     struct provided and commits the password to the database.
 *	     Callers of this routine should have already been authenticated
 *	     and ensure that all restrictions have been met.
 *                                                                    
 * RETURNS:  0 - successful
 *	     1 - failure
 *
 * ERRNO:
 *	     ENOENT 		- User doesn't exist
 *	     ENAMETOOLONG	- Over system limit 
 *	     EINTR		- signal caught
 */  
static int
_system_chpass(char *uname, 		/* User name 			    */
	       char *newpasswd, 	/* User's new cleartext password    */
	       struct userpw *upw,	/* User's userpw structure          */
	       int admin,		/* Is change administrative?        */	
	       char **msg )		/* return message from putuserpwhist*/
{
	int	rc = 1;
	char    *encrypted;		/* pointer to new encrypted passwd  */
	struct  sigaction oaction_intr, /* Historically ignored during      */
			  naction_intr, /* rewrite of the passwd file.      */
			  oaction_quit,
	  		  naction_quit,
	  		  oaction_tstp,
	  		  naction_tstp;

	/*
	 * Produce a new encrypted password and replace the old password 
	 * in the userpw struct with the new one.
	 */
	if (newpasswd && *newpasswd)
		encrypted = _pw_encrypt(newpasswd);
	else
		encrypted = "";

	if (upw->upw_passwd) free(upw->upw_passwd);

	if ((upw->upw_passwd = strdup(encrypted)) == (char *)NULL)
		return(1);

	/* If admin set administrative change flag or conversely clear it. */
	if (admin)
		upw->upw_flags |= PW_ADMCHG;
	else
		upw->upw_flags &= ~PW_ADMCHG;

	/* set new update time */
	upw->upw_lastupdate = time((long *) 0);

	/* Ignore SIGINT, SIGQUIT, SIGTSTP. */
        naction_intr.sa_handler = SIG_IGN;
        naction_intr.sa_flags   = 0;
        sigemptyset(&(naction_intr.sa_mask));
        sigaction(SIGINT, &naction_intr, &oaction_intr);

        naction_quit.sa_handler = SIG_IGN;
        naction_quit.sa_flags   = 0;
        sigemptyset(&(naction_quit.sa_mask));
        sigaction(SIGQUIT, &naction_quit, &oaction_quit);

        naction_tstp.sa_handler = SIG_IGN;
        naction_tstp.sa_flags   = 0;
        sigemptyset(&(naction_tstp.sa_mask));
        sigaction(SIGTSTP, &naction_tstp, &oaction_tstp);

	/* update /etc/security/passwd */
	setpwdb(S_READ | S_WRITE);
	rc = putuserpwhist(upw, msg);
	endpwdb();

	sigaction(SIGINT,  &oaction_intr, NULL);
	sigaction(SIGQUIT, &oaction_quit, NULL);
	sigaction(SIGTSTP, &oaction_tstp, NULL);
	
	return(rc ? 1 : 0);
}

/*
 * NAME: _passwd_moved
 *
 * FUNCTION: This routine ensures that the globally readable /etc/passwd
 *	     file does not contain an encrypted password.  The encrypted
 *	     password should exist in the /etc/security/passwd file.
 * 	     This routine should be called after previous routines either
 *	     place the password in the /etc/security/passwd file or update
 *	     the server from whence passwords will now come.
 * 
 * RETURNS:  0 - successful update
 *           1 - failure 
 *
 * ERRNO:
 *	    ENOENT	- User doesn't exist
 */
static int
_passwd_moved(char *uname)
{
	DBM *pwdbm;

	pwdbm = pwdbm_open();

	setuserdb(S_READ | S_WRITE);

	/* Make changes on the /etc/passwd file */
	if (putuserattr(uname,S_PWD,"!",SEC_CHAR))
	{
		enduserdb();
     		return (1);
	}

	if (putuserattr(uname,NULL,NULL,SEC_COMMIT))
	{
		enduserdb();
		return (1);
	}

	enduserdb();

	/* update the dbm copies */
	if (pwdbm != NULL)
	{
		(void) pwdbm_update(pwdbm, uname,"!",PWDBM_PASSWD);
		(void) dbm_close(pwdbm);
	}
	return (0);
}

/*
 * NAME: _method_chpass 
 * 
 * FUNCTION: Loads and invokes the chpass() routine contained in the 
 *	     corresponding method's load module.
 *
 * RETURNS:  0 - password change successful.
 *	     1 - password change unsuccessful but user should attempt again.
 *	     2 - password change unsuccessful and user should not attempt again.
 *
 * ERRNO:
 *	     EINVAL 	- Invalid method or no password change function 
 */ 
static int
_method_chpass(char *method,
	      char *uname,
	      char *oldpasswd,
	      char *newpasswd,
	      MsgBuf *mb)
{
	struct secmethod_table methodtable;
	int  rc;
	char *msg = (char *)NULL;

        if (_load_secmethod(method, &methodtable))
	{
		_MBReplace(mb, MSGSTR(M_FAILLOAD, DEF_FAILLOAD), method);
                return(2);
	}

	/* Ensure that they indeed are providing a chpass() interface */
	if (methodtable.method_chpass == NULL)
	{
                _MBAppend(mb, MSGSTR(M_NOPASSPTR, DEF_NOPASSPTR), method);
                errno = EINVAL;
		return(2);
	}

        rc = (*methodtable.method_chpass)(uname, oldpasswd, newpasswd, &msg);
	if (msg)
	{
		if(_MBAppend(mb, msg))
		{
			free(msg);
			return(2);
		}
		free(msg);
	}
	return(rc);
}

/*
 * NAME: _pw_encrypt
 *
 * FUNCTION: Encrypt cleartext password with salt
 *
 * RETURNS: char pointer to encrypted passwd
 */ 
char *
_pw_encrypt(char *pw)
{
	register int i, c;
	long    salt;
	char    saltc[4];

	/* 'salt' the specified password */
	time(&salt);
	salt += getpid();

	saltc[0] = salt & 077;
	saltc[1] = (salt>>6) & 077;
	for(i=0;i<2;i++)
	{
		c = saltc[i] + '.';
		if(c>'9') c += 7;
		if(c>'Z') c += 6;
		saltc[i] = c;
	}
	return (crypt (pw, saltc));
}

