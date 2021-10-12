static char sccsid[] = "@(#)18  1.9  src/bos/usr/ccs/lib/libs/util.c, libs, bos41J, 9511A_all 3/6/95 14:23:50";
/*
 *   COMPONENT_NAME: LIBS
 *
 *   FUNCTIONS: _load_secmethod
 *		_normalize_username
 *		_getregistry
 *		_is_user_in_NIS_database
 *		_is_user_in_DCE_database
 *		_is_user_in_local_database
 *		_chrncpy
 *		_hangup_chpass
 *		_alarm_chpass
 *		_passwdentry
 *		SYSTEM_is_NONE
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


#include <userpw.h>
#include <usersec.h>
#include <userconf.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "libs.h"

/*
 * Global routines
 */
int  	_passwdentry(char *, char *);
char *	_chrncpy(char *, char *, int);
int 	_is_user_in_local_database(char *);
int	_is_user_in_NIS_database(char *);
int	_is_user_in_DCE_database(char *);
int	SYSTEM_is_NONE(char *);

/*
 * Local routines
 */
static void 	_hangup_chpass(int);
static void 	_alarm_chpass(int);

/*
 * Local variables
 */
static int hangup_caught;
static int alarm_caught;

/*
 * Externs
 */
extern char *getpass(char *);


/*
 * NAME : _load_secmethod
 *
 * FUNCTION : This function loads a security method and calls the entry
 *	      point.  The entry point is expected to initialize the
 *	      function table with all services provided by the method.
 *
 * EXECUTION ENVIRONMENT: library routine
 *
 * RETURNS : -1    - _load_secmethod() failure with errno from load() routine
 *	     Other - method's initialization return code. 
 */
int 
_load_secmethod(char *method, 		/* name of security method */
		struct secmethod_table *meths)	/* function pointer table  */
{
	int    rc;
	int    (*init)();
	char   *path = (char *)NULL;
	int    freepath = 0;
	struct seclist			/* Linked list of loaded methods */
	{
		char   *name;			/* Method token name */
		struct secmethod_table table;	/* Function pointers */
		int    initrc;			/* Initial return    */
		struct seclist  *next;
	};
	static struct seclist *headmethod;	/* Beginning of list */
	       struct seclist *mptr;		/* Traversal pointer
	
	/* Search the list of previously loaded methods. */
	mptr = headmethod;
	while(mptr)
	{
		if (!strcmp(method, mptr->name))
		{
			memcpy(meths, &(mptr->table),
				sizeof(struct secmethod_table));
			return(mptr->initrc);
		}
		else
			mptr = mptr->next;
	}
		
        /*
         * Check the database for the pathname; malloc space for it; and
         * record it.  If the database does not define a pathname for the
         * method, we will look in the default method path of "/usr/lib"
         */
        rc = getconfattr(method, SC_AUTHPROGRAM, &path, SEC_CHAR);
        if ((rc != 0) || (path == (char *)NULL) || (path[0] == (char)NULL))
        {
                if ((path = (char *)malloc
                    (strlen(AUTH_DEFPATH)+strlen(method)+1)) == (char *)NULL)
                        return(-1);
                strcpy(path, AUTH_DEFPATH);
                strcat(path, method);
                freepath = TRUE;
        }

	if ((init = load(path, 1, "/usr/lib")) == 0)
		return(-1);

	if (freepath) free(path);

	/*
	 * Allocate storage for our list structure which will keep all
	 * loaded methods for quick lookup 
	 */
	if ((mptr = (struct seclist *)calloc(1, sizeof(struct seclist))) == 
	    (struct seclist *)NULL)
		return(-1);

	if ((mptr->name = (char *)strdup(method)) == (char *)NULL)
	{
		free(mptr);
		return(-1);
	}

	/*
	 * Call the initialization routine which sets up the pointers
	 * for our method functions.  Copy the function pointers to 
	 * the callers table and hook up this method to our loaded list.
	 */
	mptr->initrc = (*init)(&(mptr->table));
	memcpy(meths, &(mptr->table), sizeof(struct secmethod_table));
	mptr->next = headmethod;
	headmethod = mptr;

	return(mptr->initrc);
}


/*
 * NAME: _getregistry
 * 
 * FUNCTION: Returns malloc'd storage containing the user's administration
 * 	     domain.  The storage contains a string identifier signalling
 *	     where the user is administered, and the token name of any method
 *	     associated with the domain.  The caller of this routine
 *	     should deallocate this storage.
 *
 * RETURNS:  NULL     - error in malloc or user not found
 *	     registry - domain where user is administered
 */
char *
_getregistry(char *longname)
{
	char *registry = (char *)NULL;
	char shortname[PW_NAMELEN];

	_normalize_username(shortname, longname);

	setuserdb(S_READ);
	/*
	 * Attempt to retrieve an explicit definition of registry from
	 * the user table.  If this fails then I will retrieve my
	 * primary authentication method.  If this fails then I will
	 * go to the local database.
	 * Note: "compat" is not a valid registry value.
	 */
	if (!getuserattr(shortname, S_REGISTRY, (void *)&registry, SEC_CHAR) && !ISCOMPAT(registry))
	{
		_grammar_keyword(registry, &registry);
		registry = strdup(registry);
	}
	else 
	{
		char	*env = getenv(AUTH_ENV);
		if ( (_is_user_in_local_database(shortname)) &&
		     ( !strcmp(env,AUTH_FILES) || !strcmp(env,AUTH_COMPAT) ) )
			registry = strdup(AUTH_FILES);
		else if ( (_is_user_in_DCE_database(longname)) &&
		     ( !strcmp(env,AUTH_DCE) ) )
			registry = strdup(AUTH_DCE);
		else if (_is_user_in_local_database(shortname))
			registry = strdup(AUTH_FILES);
		else if (_is_user_in_NIS_database(shortname))
			registry = strdup(AUTH_NIS);
		else if (_is_user_in_DCE_database(longname))
			registry = strdup(AUTH_DCE);
	}
	enduserdb();

	return(registry);
}

/*
 * NAME: _is_user_in_DCE_database
 *
 * FUNCTION: Determines through normal name resolution routines, whether
 *	     the user is defined in a remote DCE database.
 *
 * RETURNS:  1 - User is in DCE
 *	     0 - User not found in DCE
 */
int
_is_user_in_DCE_database(char *name)
{
	struct passwd *pw;

	set_getpwent_remote(3);		/* Restrict lookup to DCE only        */
	pw = getpwnam(name);		/* Does user resolve thru DCE	      */
	set_getpwent_remote(1);		/* Return to full domain lookups      */

	return(pw ? 1 : 0);		/* If user resolves then return TRUE  */
}

/*
 * NAME: _is_user_in_local_database
 *
 * FUNCTION: Determines through normal name resolution routines, whether
 *	     the user is defined in the local database.
 *
 * RETURNS:  1 - User is in local
 *	     0 - User not found in local
 */
int
_is_user_in_local_database(char *name)
{
	struct passwd *pw;
	
	set_getpwent_remote(0);		/* Restrict lookup to local only      */
	pw = getpwnam(name);		/* Does user resolve locally	      */
	set_getpwent_remote(1);		/* Return to full domain lookups      */

	return(pw ? 1 : 0);		/* If user resolves then return TRUE  */
}

/*
 * NAME: _is_user_in_NIS_database
 *
 * FUNCTION: Determines through normal name resolution routines, whether
 *	     the user is defined in a remote NIS database.
 *
 * RETURNS:  1 - User is in NIS
 *	     0 - User not found in NIS
 */
int
_is_user_in_NIS_database(char *name)
{
	int rc;

	set_getpwent_remote(2);		/* Restrict lookup to local plus NIS  */
	rc = isuserrmtnam(name);	/* Is user remote within this context */
	set_getpwent_remote(1);		/* Return to full domain lookups      */

	return((rc == 1) ? 1 : 0);
}


/*
 * NAME: _normalize_username
 * 
 * FUNCTION: Moves past possible DCE cell specifications in the login 
 * 	     user name and truncates the user name to a normal 8 character
 *	     AIX maximum.  
 * 
 *	     Users should call this routine with a "uname" of possible 
 *	     indefinite length and a "newname" array of at least
 *	     PW_NAMELEN bytes long.
 *
 * RETURNS:  The value of the "newname" parameter
 */
char *
_normalize_username(char *newname, 	/* New name max eight bytes 	    */
		    char *uname)	/* Name with possible DCE cell name */
{
	char *ptr;
	register i;	/* length of ptr */

	/*
	 * Special handling of the DCE cell specification in
	 * the login user name.  The cell name appears as
	 * ".../cellname/username", therefore I will check
	 * if I have one of these names.  If I do then I will
	 * need the user's real name so I will move past all
	 * cell specifications.
	 */
	if ( ((ptr = strstr(uname, "/.../")) != (char *)NULL) ||
	     ((ptr = strstr(uname, "/.:/")) != (char*)NULL) )
	{
		ptr = strrchr(uname, '/');    /* find last slash */
		ptr++;                        /* move past slash */
	}
 	else
		ptr = uname;

	i = strlen(ptr);
	i = i > PW_NAMELEN - 1 ? PW_NAMELEN - 1 : i;
	_chrncpy(newname, ptr, i);
	newname[i] = '\0';
	return(newname);
}


/*
 * NAME: _truncate_username
 * 
 * FUNCTION: truncates the username at the AIX maximum, which is currently
 *	     8 characters.  Any DCE cell specification remains as part of
 *	     the username string, unlike the _normalize_username routine
 *	     which returns just the username itself.
 * 
 *	     This routine should be called with a username character string,
 *	     which will be truncated at the end by inserting a null after
 *	     the 8th character of the username.
 *
 * RETURNS:  Pointer to the truncated user name
 *	     (the same pointer which the caller passed in).
 */
char *
_truncate_username(char *uname)
{
	char *ptr;
	int	i;

	if ( (!uname) || (!(*uname)) )
		return(uname);

	if ( ((ptr = strstr(uname, "/.../")) != (char *)NULL) ||
	     ((ptr = strstr(uname, "/.:/")) != (char*)NULL) )
	{
		ptr = strrchr(uname, '/');    /* find last slash */
		ptr++;                        /* move past slash */
	}
 	else
		ptr = uname;

	i = strlen(ptr);
	i = i > PW_NAMELEN - 1 ? PW_NAMELEN - 1 : i;
	_chrncpy(ptr, ptr, i);
	ptr[i] = '\0';
	return(uname);
}


/*
 * NAME: _chrncpy
 *
 * FUNCTION: The _chrncpy() subroutine copies at most "number" bytes
 *	     from the string pointed to by the "origin" parameter to the
 *	     array pointed to by the "target" parameter.  This routine 
 *	     pays special attention to multibyte characters.  If "number"
 *	     bytes would split an NLS character, then the number of whole
 *	     NLS characters that can be contained within "number" bytes
 *	     is copied.   
 *
 *	     If "origin" is less than "number" bytes long, then the 
 * 	     _chrncpy() subroutine pads "target" with trailing null 
 *	     characters to fill "number" bytes.  If "origin" is "number"
 *	     or more bytes long, then at most "number" bytes are
 *	     copied and the result may not be terminated with a null 
 *	     character.
 *
 * Returns:  The value of the "target" parameter.
 */
char *
_chrncpy(char *target, char *origin, int number)
{
	/*
	 * If the name is too long and I have a codeset that contains 
	 * characters greater than one byte long, then I want to ensure
	 * that I get the most whole characters possible, but don't split
	 * a character in the last bytes.
	 */
	if ((strlen(origin) > number) && (MB_CUR_MAX > 1))
	{
		int  tbytes = 0,   	/* total bytes copied	*/
		     nbytes = 0,	/* bytes in next string */
		     cbytes,		/* bytes in character	*/
		     i;	   		/* loop counter         */
		char *optr = origin,	/* pointer to origin    */
		     *tptr = target;	/* pointer to target    */

		/*
		 * While I'm still within the byte limit, find
		 * out how many bytes are within the next character.
		 * If appending this character will not go over my
		 * limit, then append it.
		 */
		do
		{
			nbytes += cbytes = mblen(optr, MB_CUR_MAX);
			if (nbytes <= number)
			{
				for (i = 0; i < cbytes; i++)
					*tptr++ = *optr++;
				tbytes += cbytes;
			}
		} while (nbytes < number);

		/* Append final Nulls */
		for (i = tbytes; i < number ; i++)
			*tptr++ = (char)NULL;
	}
	else
		strncpy(target, origin, number);

	return(target);
}


/*
 * FUNCTION: catch hangup signals and set a flag
 *
 * EXECUTION ENVIRONMENT: software signal processing
 *
 * RETURNS: NONE
 */
static void
_hangup_chpass(int signal)
{
        hangup_caught = 1;
}
/*
 * FUNCTION: catch alarm signals and set a flag
 *
 * EXECUTION ENVIRONMENT: software signal processing
 *
 * RETURNS: NONE
 */
static void
_alarm_chpass(int signal)
{
	alarm_caught = 1;
}


/*
 * NAME: _passwdentry
 * 
 * FUNCTION: Retrieves a password from the user.  This routine handles
 * 	     the interaction with a user logged in on a tty.  The routine
 *	     requires the user to enter a password within the number of
 *	     seconds specified by the logintimeout attribute in the usw
 *	     stanza of login.cfg.  It also handles the possibility of
 *	     receiving a SIGHUP.
 *
 * RETURNS:  0 ok
 * 	    -1 failure
 */ 
int
_passwdentry(char *message,  	/* Message to display to user		*/
	     char *passwd)	/* Passwd array filled in on return  	*/
{
	char *tp;
	int  rc = 0;			 /* return code 	        */
	int  otime,	  		 /* old timeout period          */
	     ntime,			 /* new timeout period 		*/
	     delay;			 /* time the user is given to	*
					  * enter their password	*/
	struct sigaction oaction_alarm,	 /* Old SIGALRM signal handler  */
			 naction_alarm,	 /* New SIGALRM signal handler  */
			 oaction_sighup, /* Old SIGHUP signal handler   */
			 naction_sighup; /* New SIGHUP signal handler   */

	/* 
	 * Save old signal handlers, and setup new ones for SIGHUP
	 * and SIGALRM.  The SIGHUP one is of particular interest since
	 * we could potentially end up setting passwords to null if we
	 * don't catch it.
	 */
	hangup_caught = 0;
	alarm_caught = 0;

       	naction_sighup.sa_handler = _hangup_chpass;
       	naction_sighup.sa_flags   = 0;
       	sigemptyset(&(naction_sighup.sa_mask));
        sigaction(SIGHUP, &naction_sighup, &oaction_sighup);

       	naction_alarm.sa_handler = _alarm_chpass;
       	naction_alarm.sa_flags   = 0;
       	sigemptyset(&(naction_alarm.sa_mask));
        sigaction(SIGALRM, &naction_alarm, &oaction_alarm);

	/*
	 * Find out how long to wait for the user to enter their password.
	 * The default is 60 seconds.
	 */
	if (getconfattr(SC_SYS_LOGIN, S_LOGTIMEOUT, &delay, SEC_INT) ||
	    (delay <= 0))
		delay = 60;

	/*
	 * Set an alarm timeout the password read.  If an alarm is set
	 * already, then save the timeout period in otime.
	 */ 
	otime = alarm(delay);
	if((tp = getpass(message)) != (char *)NULL)
	{
		strncpy(passwd, tp, MAX_PASS);
		passwd[MAX_PASS] = '\0';
	}
	else
		rc = -1;
	
	/*
 	 * Turn off alarm and record how many seconds before the user's 
	 * 60 seconds were used up.  Then restore the old signal handler.
	 */
	ntime = alarm(0);
	sigaction(SIGALRM, &oaction_alarm, NULL);
	sigaction(SIGHUP,  &oaction_sighup, NULL);

	/*
	 * If there was a timer going before we set our 60 second timer,
	 * then we are going to have to calculate how many seconds should
	 * be left in the old timer.  We do this by                    
	 *
	 *	1) calculate amount of time we used up 
  	 *		(60 seconds minus the time left before timer expired)
	 * 	2) Subtract this time we used up from the old timer.
	 * 	3) If this turns out to be less than zero, then the timer
	 *	   	should be expired, so set it to one.
	 */
	if (otime)
	{
		if ((ntime = otime - (60 - ntime)) < 0)
			ntime = 1;
		alarm(ntime);
	}

	if (hangup_caught)
	{
		kill(getpid(), SIGHUP);
		return(-1); 
	}
	if (alarm_caught)
		return(-1);

	return(rc);
}




/*
 * NAME:     SYSTEM_is_NONE
 *
 * FUNCTION: Checks for SYSTEM = NONE.
 *
 * RETURNS:  1  when SYSTEM  = NONE
 *           0  when SYSTEM != NONE
 */
int
SYSTEM_is_NONE(char *uname)
{
	char	*start, *end;
	int	rc = 0;

	if (!getuserattr(uname, S_AUTHSYSTEM, &start, SEC_CHAR))
	{
		if (start = strdup(start))
		{
			/*
			 * Remove leading and trailing spaces.
			 */
			while (*start == ' ' || *start == '\t')
				start++;

			end = start;
			while (*end != ' ' && *end != '\t' && *end)
				end++;
			
			*end = NULL;

			if (ISNONE(start))
				rc = 1;
		}
	}
	return(rc);
}
