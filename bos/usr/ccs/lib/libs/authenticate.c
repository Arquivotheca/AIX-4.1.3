static char sccsid[] = "@(#)11	1.6  src/bos/usr/ccs/lib/libs/authenticate.c, libs, bos41J, 9519A_all 5/8/95 14:19:32";
/*
 *   COMPONENT_NAME: LIBS
 *
 *   FUNCTIONS: authenticate
 *		_method_authenticate
 *		_grammar_authenticate
 *		_crypt_authenticate
 *		_run_parse_tree
 *		_set_authenv
 *		_pwddup
 *		_pwdfree
 *		SET_AUTHSTATE
 *		RESET
 *		END
 *		CONTINUE
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
#include <userpw.h>
#include <userconf.h>	/* security configuration attributes */
#include <syslog.h>
#include <login.h>
#include <string.h>
#include <pwd.h>
#include "libs.h"

/*  
 * Prototypes, internal and external
 */
int    	       	_crypt_authenticate(char *, char *, char *);
struct passwd *	_pwddup(struct passwd *);
void   	       	_pwdfree(struct passwd *);
static void 	_set_authenv(char *);
static int  	_grammar_authenticate(char *, char *, MsgBuf *, struct passwd *,
				char *);
static int 	_run_parse_tree(char *, char *, MsgBuf *, struct passwd *, 
				struct secgrammar_tree *,struct methodstats **);
static char *	_method_authenticate(char *, char *, MsgBuf *, struct passwd *, 
				char *, int, struct methodstats **);
extern struct secgrammar_tree *	_create_parse_tree(char *);
extern void    		  	_release_parse_tree(struct secgrammar_tree *);


/*
 * The following structure definition is used to hold the name and
 * return code of each method checked.  Since method names may appear more
 * than once in a grammar list, there is no reason to check them again after
 * the first time.  I will keep a list of checked methods for referral.
 */
struct methodstats
{
        char *name;
        int  returned;
        struct methodstats *next;
};

/*
 * The authstate variable describes the primary authentication method
 * that passed during login (this is defined as the first method that passes).
 * The authstate variable is used to hold the method name of any authentication
 * routines that pass (either _method_authenticate() or _crypt_authenticate()).
 * The SET_AUTHSTATE() macro initializes the variable, and the _set_authenv()
 * routine which places authstate into the environment.
 */
static char   *authstate;
#define SET_AUTHSTATE(a)\
	{\
	 	if (!authstate) authstate = strdup(a);\
	}

/*
 * Macro to reset internal static variables.  
 */ 
#define RESET\
	{\
		state = INITIAL;\
		if (fname)     { free(fname);     fname     = (char *)NULL; }\
		if (authstate) { free(authstate); authstate = (char *)NULL; }\
		if (grammar)   { free(grammar);   grammar   = (char *)NULL; }\
		if (pwd)       { _pwdfree(pwd);   pwd       = \
						     (struct passwd *)NULL; }\
	}

/*
 * Macro to end processing and reset internal variables.  After
 * this is called authenticate() is ready to process a new user.
 * Set the authstate if the user passed authentication.
 */
#define END(a)\
	{\
		*reenter = 0;\
		if (!a) _set_authenv(authstate);\
		RESET;\
	  	return(a);\
	}

/*
 * Macro to return to the caller with expectations of a recall
 * to authenticate().  On reentry to this routine, authentication
 * will be in state "b" (QUERY). 
 */
#define CONTINUE(b)\
	{\
		*reenter = 1;\
		state = b;\
		return(0);\
	}


/*
 * NAME: authenticate
 * 
 * FUNCTION: Entry point for all system authentication.  This routine
 *	     implements a recallable interface that provides challenges
 *	     and validates responses.  The routine may return with a
 *	     challenge to the user in the "message" parameter.  If the
 *	     "reenter" variable is set, then the caller of this routine
 *	     should respond to the challenge with a recall and response
 *	     in the "passwd" parameter. 
 *
 * RETURNS:  0 && (reenter == 0)  - user has passed authentication.
 *	     0 && (reenter == 1)  - user must respond to challenge.
 *	     1 && (reenter == 0)  - user has failed authentication.
 *
 * ERRNO:
 *	     ENOENT	- User does not exist
 *	     ENOMEM	- Malloc failed
 *	     ESAD	- Authentication denial
 *	     EINVAL	- Invalid parameter, invalid grammar, or invalid
 *			  method authentication function
 */
int 
authenticate(char *uname, char *passwd, int *reenter, char **message)
{
	static enum {INITIAL, QUERY} state;
	static struct passwd *pwd;	/* User's passwd structure    */
	static char   *fname;		/* User name upon first entry */
	static char   *grammar;		/* Grammar to authenticate by */
	int    rc;			/* Return code		      */
	char   shortname[PW_NAMELEN];	/* Short name (max 8 bytes)   */
	MsgBuf mb;
	

	/* The caller must supply a valid user name */
	if (!uname || (uname[0] == (char)NULL))
	{
		errno = EINVAL;
		END(1);
	}

	/*
	 * If I currently have a user name from a previous call and am
	 * now being given a different user name then I should reset.
	 */
	if (fname && strcmp(fname, uname))
		RESET;


	_MBInitialize(&mb, message);	/* Set up for return messages */

	switch (state)
	{
	   case INITIAL:
		/* 
		 * Malloc and store full user name from initial call.
		 * This will be rechecked upon reentry.
		 */
		if ((fname = strdup(uname)) == (char *)NULL)
			END(1);
			
		/* Retrieve possibly shortened AIX specification for name */
		_normalize_username(shortname, uname);

		setpwent();
                /*
                 * Check the name resolution to see if this user is defined.
                 * If the user is unknown then we fail authentication without
                 * ever checking the password.
                 */
                if ((pwd = getpwnam(uname)) == (struct passwd *)NULL)
		{
			_MBReplace(&mb, MSGSTR(M_USRNONEX, DEF_USRNONEX), 
				uname);
			errno = ENOENT;
			END(1);
		}

		/* Get local copy so any subsequent operations don't lose it. */
		if ((pwd = _pwddup(pwd)) == (struct passwd *)NULL)
			END(1);
		endpwent();

		setuserdb(S_READ);
		/*
	 	 * Retrieve "SYSTEM" line from the database.  This will 
		 * describe the authentication domains for this user.  If 
		 * the SYSTEM line does not appear then this implicitly means 
		 * local password authentication (includes NIS).
		 */
		getuserattr(shortname,S_AUTHSYSTEM, (void *)&grammar, SEC_CHAR);
		
		/* Assign default or convert to correct moniker */
		if ((grammar == (char *)NULL) || (grammar[0] == (char)NULL))
			grammar = AUTH_COMPAT;
		else if (_grammar_keyword(grammar, &grammar) == -1)
		{
			grammar = (char *)NULL;
			END(1);
		}

		/* Get local copy so any subsequent operations don't lose it. */
		if ((grammar = strdup(grammar)) == (char *)NULL)
			END(1);
		enduserdb();

		/*
		 * If the grammar explicitly states no authentication required
		 * then we exit successfully.
		 */
		if (ISNONE(grammar))
		{
			SET_AUTHSTATE(AUTH_COMPAT);
			END(0);
		}

                /*
                 * If the user has no password and supplied no password
                 * during the initial invocation, then authentication passes.
                 */
                if (    (pwd->pw_passwd == (char *)NULL || 
		         pwd->pw_passwd[0] == (char)NULL)
		     && (!passwd || passwd[0] == (char)NULL))
		{
			SET_AUTHSTATE(AUTH_COMPAT);
			END(0);
		}

                /*
                 * If the user supplied no passwd argument during the
                 * initial call then move to the query phase.
                 */
                if (!passwd)
                {
			if (_MBReplace(&mb, 
			    MSGSTR(M_PWPROMPT, DEF_PWPROMPT), uname))
				END(1);

			CONTINUE(QUERY); /* Next phase is query */
		}

                /* ... Otherwise continue into the query phase */


	   /* 
	    * If I have just queried for the password I will call
	    * the authentication routines.
	    */
	   case QUERY:
		/*
		 * If the grammar is a simple local or compat authentication 
		 * (normal case which uses crypt()/compare authentication)
		 * then I will call it directly.  Otherwise I'll move into 
		 * the longer approach and parse the grammar.
		 */
		if (ISLOCAL(grammar) || ISCOMPAT(grammar))
		{
			rc = _crypt_authenticate(grammar, uname, passwd);
			if (rc == AUTH_SUCCESS)
			{
				SET_AUTHSTATE(grammar);
			}
			else
				_MBReplace(&mb, MSGSTR(M_FAILLOCAL, 
					DEF_FAILLOCAL), pwd->pw_name);
		}		
		else
			rc = _grammar_authenticate(uname, passwd, &mb, pwd, 
				grammar);

		END(rc);
	}
}

/*
 * NAME: _crypt_authenticate
 *                                                                    
 * FUNCTION: This function determines if the user passes normal AIX style
 *	     authentication.  This authentication is based on encrypted
 *	     passwords.  
 *
 *	     Both NIS and local system users authenticate in this fashion.
 *
 *                                                                    
 * RETURNS: 
 *	    AUTH_SUCCESS - authentication passed
 *	    AUTH_FAILURE - authentication failed.
 *
 * ERRNO:    ESAD - Authentication failed
 */  
int
_crypt_authenticate(char *method,
		    char *uname,	/* User's name			    */
		    char   *passwd)	/* User's cleartext password	    */
{
	char	lpasswd[PW_PASSLEN+1];
	char	*tp;			/* temporary pointer */
	int	rc = AUTH_FAILURE;	/* return code	     */	
	struct	passwd *pwd = NULL;	/* for getpwnam()    */
	struct  passwd *tmp_pwd = NULL;	/* for getpwnam()    */
		
	/*
	 * Call getpwnam to get the user's passwd struct, restricting
	 * getpwnam lookups to the local AIX files (including NIS).
         * This routine is performing a traditional authentication 
	 * because SYSTEM is set to "compat" or "files".  So the user's
	 * password information must be found traditionally as well (either
	 * local or NIS, but not DCE).  AUTHSTATE cannot be relied on here.
	 */

	set_getpwent_remote(2);	/* local lookups only	*/
	tmp_pwd = getpwnam(uname);
	set_getpwent_remote(1);	/* resume full lookups	*/
	
	if (tmp_pwd)
		pwd = _pwddup(tmp_pwd);

	if ( !tmp_pwd || !pwd )
	{
		rc = AUTH_FAILURE;
		errno = ESAD;
		return(rc);
	}
		
	/*
	 * If the user possesses a password then crypt() and compare.
	 * Otherwise if the password is NULL and the user supplied
	 * some form of password, consider that failing as well.
	 */ 
	if (pwd->pw_passwd && pwd->pw_passwd[0])
	{
		if (passwd && (strlen(passwd) > PW_PASSLEN))
		{
			/*
			 * Truncate password to 8 characters.
			 */
			passwd = strncpy(lpasswd, passwd, PW_PASSLEN);
			passwd[PW_PASSLEN] = '\0';
		}

		tp = crypt(passwd, pwd->pw_passwd);

		/* if passwd doesn't match then authentication fails */
		if (!strcmp(pwd->pw_passwd, tp))
			rc = AUTH_SUCCESS;
	}
	else
	{
		if (passwd == NULL || passwd[0] == (char)NULL)
			rc = AUTH_SUCCESS;
	}

	/*
	 * If I pass the crypt and compare, and am trying to pass
	 * local authentication only, then I should not be an NIS user.
	 * This case should be very rare since the default grammar is 
	 * AUTH_COMPAT.  AUTH_FILES would be kind of a silly way to cut off
	 * NIS users when you have the "+/-" syntax in the /etc/passwd file.
	 */
	if ((rc == AUTH_SUCCESS) && ISLOCAL(method))
	{
		/* If user is not explicitly in local database then we fail */
		if (!_is_user_in_local_database(pwd->pw_name))
			rc = AUTH_FAILURE;
	}

	if (rc == AUTH_FAILURE)	errno = ESAD;

	_pwdfree(pwd);
	return(rc);
}

/*
 * NAME:     _grammar_authenticate
 *
 * FUNCTION: This function accepts a user name, a password, and a grammar with 
 *	     which to authenticate the user.  This grammar is used to create
 *	     a parse tree which is then evaluated bottom up and left to right.
 *
 * RETURNS:  0 - User may login
 *	     1 - User may not login
 * 
 * ERRNO:    EINVAL - Invalid parse tree
 */
static int
_grammar_authenticate(char   *uname, 	  /* User name 			    */
		      char   *passwd, 	  /* User's cleartext password	    */
		      MsgBuf *mb, 	  /* Message to return to user	    */
		      struct passwd *pwd, /* User's passwd structure	    */
		      char   *grammar)	  /* Authentication grammar	    */
{
	struct methodstats  *headmethod = (struct methodstats *)NULL;  
					  /* Head element in a linked list  */	
					  /* of currently evaluated methods */
 	struct methodstats  *front;	  /* Used during list release       */
	struct secgrammar_tree *tptr;	  /* Parse tree root pointer        */
        int rc;				  /* return code                    */

	/*
	 * Create a parse tree for the grammar.  If the parse tree has 
	 * a syntax error and cannot be produced, then log the error and
	 * perform local authentication.  Otherwise run the parse tree
	 * and release it. 
	 */
	if((tptr=_create_parse_tree(grammar)) == (struct secgrammar_tree *)NULL)
	{
		if (_MBReplace(mb, 
		    MSGSTR(M_GRAMMARERR, DEF_GRAMMARERR), grammar))
			return(AUTH_FAILURE);	
	
		rc = _crypt_authenticate(AUTH_COMPAT, uname, passwd);
		if (rc == AUTH_SUCCESS)
		{
			SET_AUTHSTATE(AUTH_COMPAT);
		}
		else
			_MBAppend(mb, MSGSTR(M_FAILLOCAL, DEF_FAILLOCAL), 
				pwd->pw_name);
	}
	else
	{
		rc = _run_parse_tree(uname,passwd, mb, pwd, tptr, &headmethod);
		_release_parse_tree(tptr);

		/* 
		 * Release the linked list possibly created during the 
	 	 * parse tree run 
		 */
		while (headmethod)
		{
			front = headmethod->next;
			if (headmethod->name)
				free(headmethod->name);
			free(headmethod);
			headmethod = front;
		
		}	
	}
	return(rc);
}


/*
 * NAME: _run_parse_tree
 *
 * FUNCTION: This function implements a recursive, left, right, evaluation 
 * 	     of the grammar parse tree. 
 *
 * RETURNS:  0 - User may login
 *	     1 - User may not login
 */
static int
_run_parse_tree(char  *uname, 			/* User name		     */
		char  *passwd, 			/* User's cleartext password */
		MsgBuf *mb, 			/* Message to return to user */
		struct passwd *pwd,		/* User's passwd structure   */
		struct secgrammar_tree *tptr,	/* Node of parse tree        */ 
		struct methodstats **headmethod)/* List of evaluated methods */
{
	int rc  = AUTH_FAILURE,	/* return code from _run_parse_tree()	*/
	    lrc = AUTH_FAILURE,	/* left branch return code		*/
	    rrc = AUTH_FAILURE;	/* right branch return code		*/

	/* 
	 * Evaluation specific cases for interior nodes of the tree
 	 * and for leaves of the tree.
	 */
        switch (tptr->type)
        {
                case LEAF:
                        rc = _method_authenticate(uname, passwd, mb, pwd,
					tptr->name, tptr->result, headmethod);
                        break;

                case NODE:
                        if (tptr->left)  /* Visit left branch first */
                                lrc = _run_parse_tree(uname, passwd, mb,
					pwd, tptr->left, headmethod);

			/*
			 * Once the left branch is visited, we will use
			 * short circuit evaluation to determine if the 
			 * right branch need be visited.  If the operator
			 * is "AND" and the left branch evaluated false
			 * then no need to visit right branch.  If the
			 * operator is "OR" and the left branch evaluated
			 * true then no need to visit right branch.
			 */
			if (!strcmp(tptr->operator, AUTH_AND) && 
			    (lrc == AUTH_FAILURE))
				return(AUTH_FAILURE);
			if (!strcmp(tptr->operator, AUTH_OR) && 
			    (lrc == AUTH_SUCCESS))
				return(AUTH_SUCCESS);

                        if (tptr->right)  /* Otherwise visit right branch */
                                rrc = _run_parse_tree(uname, passwd, mb,
					pwd, tptr->right, headmethod);

			/* Finally evaluate return codes.  */
                        if (!strcmp(tptr->operator, AUTH_AND))
			{
				if(rrc == AUTH_SUCCESS)
					rc = AUTH_SUCCESS;
			}
                        else if (!strcmp(tptr->operator, AUTH_OR))
			{
				if(rrc == AUTH_SUCCESS)
					rc = AUTH_SUCCESS;
			}
                        break;
        }
        return(rc);
}

/*
 * NAME: _method_authenticate()
 *
 * FUNCTION: This method either calls a method authentication function,
 *           or discerns what a previous call to the method returned.
 *           Once the return code from the method is ascertained, it
 *           is compared against the expected result.  If the return
 *           code matches the result specified in the grammar, then
 *           the function returns success.
 *
 * RETURNS:  0 - Method return code matches expected result
 *           1 - Method return code does not match expected result.
 *
 * ERRNO:    EINVAL - Invalid method or method function
 *	     ESAD   - Authentication failed
 */
static char *
_method_authenticate
	       (char *uname,			/* User name		     */
	        char *passwd,			/* User's cleartext password */
	        MsgBuf *mb,			/* Message to return to user */
	   	struct passwd *pwd,		/* User's passwd structure   */
	        char *method, 			/* Method name to evaluate   */	
	        int result,			/* Expected result of method */
	    	struct methodstats **headmethod)/* List of evaluated methods */
{
        struct secmethod_table 	methodtable;
        struct methodstats 	*newptr, 
				*ptr = *headmethod;
	char *msg = (char *)NULL;/* Return message from load method	    */
	int  rc;
	int  dummy;		/* unused field in case we want to export   */	
				/* challenges and continue into the methods */
				/* at a future date.			    */

	/*
	 * Performance objective here is to check the return codes
	 * from previous method invocations since method names may 
	 * appear in a grammar more than once, and there is no reason
	 * to recall them.
 	 */
        while (ptr)
        {
                if (!strcmp(method, ptr->name))
		{
			if ((result == AUTH_WILDCARD) ||
		            (ptr->returned == result))
			{	
				if (result == AUTH_SUCCESS)
					SET_AUTHSTATE(method);
				return(AUTH_SUCCESS);
			}
			else
				return(AUTH_FAILURE);
		}
                ptr = ptr->next;
        }

        /*
         * If we have gotten here, we can assume that we have not called
         * this method before.  We must malloc space to record this call
         * so that it need not be loaded if we see this method again.
         */
        if ((newptr = (struct methodstats *)
            malloc(sizeof(struct methodstats))) == (struct methodstats *)NULL)
                return(AUTH_FAILURE);
	/*
	 * Initialize the structure, set the returned field to failure and
	 * place at front of list.
	 */
	memset(newptr, 0, sizeof(struct methodstats));
	newptr->returned = AUTH_FAILURE;

        /*
         * malloc space for the name and record it
         */
        if ((newptr->name = strdup(method)) == (char *)NULL)
                return(AUTH_FAILURE);

        newptr->next = *headmethod;
        *headmethod = newptr;

	/* 
	 * If the method is local or compat then use normal crypt()/compare
	 * authentication. Call the local routines and connect the method 
	 * up to the list of checked methods.
	 */
        if (ISLOCAL(method) || ISCOMPAT(method))
	{
                newptr->returned = _crypt_authenticate(method, uname, passwd);
	        if ((result == AUTH_WILDCARD) || (newptr->returned == result))
		{
			if (result == AUTH_SUCCESS)
				SET_AUTHSTATE(method);
			return(AUTH_SUCCESS);
		}
		else
		{
			_MBAppend(mb, MSGSTR(M_FAILLOCAL, DEF_FAILLOCAL), 
				pwd->pw_name);
			return(AUTH_FAILURE);
		}
	}

	/*
	 * Otherwise load the security method and if it has defined an 
	 * authenticate entry point then call it.
	 */
        if (_load_secmethod(newptr->name, &methodtable))
	{
		_MBAppend(mb, MSGSTR(M_FAILLOAD, DEF_FAILLOAD), newptr->name);
                return(AUTH_FAILURE);
	}

	if (methodtable.method_authenticate == NULL)
	{
		errno = EINVAL;
		_MBAppend(mb, MSGSTR(M_NOAUTHPTR, DEF_NOAUTHPTR),newptr->name);
		return(AUTH_FAILURE);
	}  

        newptr->returned = (*methodtable.method_authenticate)
				(uname,passwd,&dummy,&msg);
	/*
	 * Concatenate this message (if present) onto our return messages
	 * and free the storage allocated in the method.
	 */
	if (msg)
	{
		if (_MBAppend(mb, msg))
		{
			free(msg);
			return(AUTH_FAILURE);
		}
		free(msg);
	}

	if ((result == AUTH_WILDCARD) || (newptr->returned == result))
	{
		if (result == AUTH_SUCCESS)
			SET_AUTHSTATE(method);
		return(AUTH_SUCCESS);
	}
	errno = ESAD;
	return(AUTH_FAILURE);
}

/*
 * NAME: _set_authenv
 *
 * FUNCTION:    The AUTHSTATE environment variable is set during
 *              authentication and describes the primary authentication 
 *	        mechanism.
 *
 * RETURNS:     void
 */
static void
_set_authenv(char *state)
{
	char *envbuf;

	if ((envbuf = (char *)malloc(sizeof("AUTHSTATE=") + 
	     strlen(state))) == (char *)NULL)
		return;

        strcpy(envbuf, "AUTHSTATE=");
        strcat(envbuf, state);

        putenv(envbuf);

	reset_grjunk_authstate();
	reset_pwjunk_authstate();
}

/*
 * NAME: _pwddup
 *
 * FUNCTION: Malloc and store a local copy of the passwd structure.  
 *	     This is important since the getpw* routines have static
 *	     storage that is rewritten ever time a new lookup is done.
 *	     If subsequent operations are possible using these routines
 *	     and you are dependent on your copy of the passwd structure,
 *	     then your copy should be saved in local storage.
 *
 *	     The free routine for this storage is _pwdfree()
 *
 * RETURNS:  NULL 		- malloc failure
 *	     struct passwd *	- newly allocated copy of passwd structure
 */
struct passwd *
_pwddup(struct passwd *pwd)
{
	struct passwd *newpw;

	if ((newpw = (struct passwd *)malloc(sizeof(struct passwd))) == 
	    (struct passwd *)NULL)
		return((struct passwd *)NULL);

	memset(newpw, 0, sizeof(struct passwd));

	if (((newpw->pw_name   = strdup(pwd->pw_name))   == (char *)NULL) ||
	    ((newpw->pw_passwd = strdup(pwd->pw_passwd)) == (char *)NULL) ||
	    ((newpw->pw_gecos  = strdup(pwd->pw_gecos))  == (char *)NULL) ||
	    ((newpw->pw_dir    = strdup(pwd->pw_dir))    == (char *)NULL) ||
	    ((newpw->pw_shell  = strdup(pwd->pw_shell))  == (char *)NULL))
	{
		if (newpw->pw_name) 	free(newpw->pw_name);
		if (newpw->pw_passwd)	free(newpw->pw_passwd);
		if (newpw->pw_gecos)	free(newpw->pw_gecos);
		if (newpw->pw_dir)	free(newpw->pw_dir);
		if (newpw->pw_shell)	free(newpw->pw_shell);
		free(newpw);

		return((struct passwd *)NULL);
	}
	newpw->pw_uid = pwd->pw_uid;
	newpw->pw_gid = pwd->pw_gid;

	return(newpw);
}
	
/*
 * NAME: _pwdfree
 *
 * FUNCTION: Frees malloc'd storage allocated when making a local copy
 *	     of the passwd structure.
 *
 * RETURNS:  Nothing
 */
void
_pwdfree(struct passwd *pwd)
{
	if (!pwd)		return;
	if (pwd->pw_name)	free(pwd->pw_name);
	if (pwd->pw_passwd)	free(pwd->pw_passwd);
	if (pwd->pw_gecos)	free(pwd->pw_gecos);
	if (pwd->pw_dir)	free(pwd->pw_dir);
	if (pwd->pw_shell)	free(pwd->pw_shell);

	free(pwd);
}
