static char sccsid[] = "@(#)55	1.6  src/bos/usr/ccs/lib/libs/pw_rest.c, libs, bos41J, 9520A_all 5/14/95 18:44:43";
/*
 *   COMPONENT_NAME: LIBS
 *
 *   FUNCTIONS: DAYS2SECONDS
 *		TOLOWERCASE
 *		VALID_CHAR
 *		WEEKS2SECONDS
 *		_AgeInitialize
 *		_AnalyzeAges
 *		_CompositionInitialize
 *		_CompositionMessages
 *		_ConfigurablePasswordChecks
 *		_LoadConfigurationTable
 *		_PWDAge
 *		_PWDComposition
 *		_PWDDictionary
 *		_PWDMethods
 *		_PasswdRetrievalPolicy
 *		_WarnTime
 *		_ch_minother
 *		_local_passwdrestrictions
 *		_userpwdup
 *		_userpwfree
 *		_val_chars
 *		_val_maxrepeat
 *		_val_minalpha
 *		_val_mindiff
 *		_val_minlen
 *		_val_minother
 *		_valid_crypted
 *		passwdexpired
 *		passwdrequired
 *		passwdrestrictions
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>		/* free() */
#include <unistd.h>		/* getuid() */
#include <userpw.h>		/* for getuserpw() */
#include <usersec.h>		/* for S_READ and S_WRITE */
#include <pwd.h>		/* struct passwd */
#include <ctype.h>		
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <time.h>		/* for time() */
#include "libs.h"		/* for PWD_FILENAME */


/*
 * Time conversion macros.
 */
#define DAYS2SECONDS(x) 	(24L * 60L * 60L * (x))
#define	WEEKS2SECONDS(x)	(7L * DAYS2SECONDS(x))


/*
 * Cumbersome or frequently used structure names.
 */
typedef	struct	secmethod_table SMTable;
typedef	struct	userpw		UserPW;
typedef	struct	passwd		Passwd;
typedef	struct passattrs
{
	int	val;		/* attribute value */
	int 	low;		/* low value (also default values) */
	int 	high;		/* high value */
	void	(*ch_val) ();	/* sanity check composite values */
	int	(*validate) ();	/* validation routine */
	int	fail;		/* Failure flag */
	char	*attr;		/* attribute name */
	char	*dmsg;		/* default error message */
	char 	*dmsgs;		/* default error message for plural messages */
	int	emsg;		/* error message constant */
	int	emsgs;		/* error message constant for plural messages */
} PasswordAttr;


/*
 * Prototypes for subroutines in this file (that are not contained in libs.h).
 */
static	int	_ConfigurablePasswordChecks(UserPW *, char *, char *, MsgBuf *);
static	int	_LoadConfigurationTable(char *, PasswordAttr *, int,MsgBuf *);

static	int	_PWDMethods(UserPW *, char *, char *, MsgBuf *);

static	int	_PWDComposition(UserPW *, char *, char *, MsgBuf *);
static	int	_CompositionInitialize(char *, PasswordAttr *, MsgBuf *);
static	int	_CompositionMessages(PasswordAttr *, int, MsgBuf *);
static	int	_val_chars(    char *);
static	int	_val_minalpha( PasswordAttr *, char *, char *);
static	int	_val_minother( PasswordAttr *, char *, char *);
static	int	_val_mindiff(  PasswordAttr *, char *, char *);
static	int	_val_maxrepeat(PasswordAttr *, char *, char *);
static	int	_val_minlen(   PasswordAttr *, char *, char *);
static  void  	_ch_minother(  PasswordAttr *);

static	int	_PWDAge(UserPW *, MsgBuf *);
static	int	_AgeInitialize(char *, PasswordAttr *, MsgBuf *);
static	int	_AnalyzeAges(UserPW *, int, MsgBuf *);
static	void	_WarnTime(UserPW *, ulong, ulong, MsgBuf *);
static	int	_valid_crypted(char *);

static	int	_PWDDictionary(UserPW *, char *, MsgBuf *);

extern	int	_PWDHistory(   UserPW *, char *, MsgBuf *);

extern	int	(*load(char *, unsigned int, char *))();
extern	int	unload(int (*)());
extern	void	setpwent(void);
extern	void	endpwent(void);


/*
 * NAME:     passwdrestrictions
 *
 * FUNCTION: The high level password restrictions subroutine.  It determines
 *           whether to invoke the "local" password restrictions subroutine
 *           or a "remote" (external) method.  For the "local" case, it invokes
 *           _local_passwdrestrictions().  For a "remote" method, it loads
 *           and invokes the appropriate method.
 *
 * NOTES:    mb          - Messages are placed into mb for printing by higher
 *                         level subroutines.  For return codes of -1 and 2,
 *                         _MBReplace() is used.  For all other return codes,
 *                         _MBAppend() is used.  The caller is responsible
 *                         for initializing and freeing this storage.
 *
 * RETURNS:  (in order of precedence)
 *           -1 - Internal error with errno set.
 *            2 - Password does not meet requirements. Password change not
 *                allowed at this time.  (PW_ADMIN, minage, maxexpired)
 *            1 - Password does not meet requirement. Attempt with new password.
 *            0 - Password meets requirements.
 */
int
passwdrestrictions(	char	*registry,
			UserPW	*pw,
			char	*longname,
			char 	*password, 
			char 	*oldpassword,
			int	 adminchange,
			MsgBuf	*mb)
{
	SMTable	methodtable;
	char	*message;
	int	MB_rc;
	int	tmp_errno;
	int	rc = 0;

	if (ISLOCAL(registry) || ISNIS(registry))
	{
		/*
		 * Execute the local (files) password restrictions method.
		 */
		return(_local_passwdrestrictions
			(pw, password, oldpassword, adminchange, mb));
	}

	/*
	 * Load the remote password restrictions method.
	 */
	if (_load_secmethod(registry, &methodtable))
	{
		/*
		 * Can't find the remote password restrictions method.
		 * Preserve _load_secmethod() errno.
		 */
		tmp_errno = errno;
		(void) _MBReplace(mb, MSGSTR(M_FAILLOAD, DEF_FAILLOAD),
								registry);
		errno = tmp_errno;
		return(-1);		/* Internal error */
	}

	/*
	 * Execute the remote password restrictions method.
	 */
	if (methodtable.method_passwdrestrictions)
	{
		rc = (*methodtable.method_passwdrestrictions)
				(longname, password, oldpassword, &message);
		tmp_errno = errno;

		if (rc == 0 || rc == 1)
		{
			if (message)
			{
				MB_rc     = _MBAppend(mb, message);
				tmp_errno = errno;
				free((void *) message);
				if (MB_rc)
					rc = -1;	/* Internal error */
			}
			else
			if (rc && _MBAppend(mb, MSGSTR(M_PWRFAIL, DEF_PWRFAIL)))
			{
				tmp_errno = errno;
				rc = -1;	/* Internal error */
			}
		}
		else
		if (rc == 2)
		{
			if (message)
			{
				MB_rc     = _MBReplace(mb, message);
				tmp_errno = errno;
				free((void *) message);
				if (MB_rc)
					rc = -1;	/* Internal error */
			}
			else
			if (_MBReplace(mb, MSGSTR(M_PWRADMFAIL,DEF_PWRADMFAIL)))
			{
				tmp_errno = errno;
				rc = -1;	/* Internal error */
			}
		}
		else
		{
			if (message)
			{
				(void) _MBReplace(mb, message);
				free((void *) message);
			}
			else
			{
				(void) _MBReplace(mb,
					MSGSTR(M_PWRERR, DEF_PWRERR), registry);
			}
			rc = -1;
		}
	}

	if (rc == -1)
		errno = tmp_errno;
	return(rc);
}


/*
 * NAME:     _local_passwdrestrictions 
 *
 * FUNCTION: Perform the password restrictions based on the user's attributes
 *           defined in /etc/security/user, the user's upw_flags, the new
 *           and current clear text passwords, and the real user ID.
 *
 * PARAMETERS:
 *           pw          - The user's userpw structure. In the user's upw_flags
 *                         field, the following flags are recognized:
 *
 *                         PW_ADMIN    - Only allow root to change the password.
 *                         PW_NOCHECK  - Do not check the password complexity
 *                                       rules.  Only check the required rules
 *                                       such as 7 bit characters.
 *                         PW_ADMCHG   - Do not check "minage" or "maxexpired"
 *                                       since the user's password was last
 *                                       changed by an administrator.
 *           password    - the user's new cleartext password to check.
 *           oldpassword - the user's old cleartext password 
 *           adminchange - true if an administrator is changing someone
 *                         else's password; otherwise, false.
 *           mb          - messages are placed into mb for printing by higher
 *                        level subroutines.  For return codes of -1 and 2,
 *                        _MBReplace() is used.  For all other return codes,
 *                        _MBAppend() is used.  The caller is responsible
 *                        for initializing and freeing this storage.
 *
 * NOTES:    Differences between 'adminchange' and PW_ADMCHG:
 *              adminchange - An administrator is currently changing
 *                            someone else's password. Thus, age and
 *                            composition rules do not apply.
 *              PW_ADMCHG   - A user is being administratively forced to
 *                            change his own password.  Thus, composition
 *                            rules apply, but age rules do not.
 *
 * RETURNS:  (in order of precedence)
 *           -1 - Internal error with errno set.
 *            2 - Password does not meet requirements. Password change not
 *                allowed at this time.  (minage, lifetime, PW_ADMIN && !root)
 *            1 - Password does not meet requirement. Attempt with new password.
 *            0 - Password meets requirements.
 */
int
_local_passwdrestrictions( UserPW	*pw,
			   char 	*password, 
			   char 	*oldpassword,
			   int		adminchange,
			   MsgBuf	*mb)
{
	char	short_password[PW_PASSLEN+1];
	char	short_oldpassword[PW_PASSLEN+1];
	char	crypted[PW_CRYPTLEN+1];
	UserPW	tpw;

	/*
	 * Duplicate pw in case it is a pointer from getuserpw().
	 */
	memcpy(&tpw, pw, sizeof(UserPW));
	tpw.upw_passwd = strncpy(crypted, pw->upw_passwd, sizeof(crypted));
	crypted[sizeof(crypted)-1] = '\0';
	pw = &tpw;

	/*
	 * Only root can change the password on an account that has
	 * the PW_ADMIN flag set.
	 */
	if (checkforadm(pw->upw_flags))
	{
		if (_MBReplace(mb, MSGSTR(M_ADMINONLY, DEF_ADMINONLY)))
			return(-1);	/* Internal error */
		return(2);		/* Can't change password */
	}

	/*
	 * Truncate password size to PW_PASSLEN.
	 */
	strncpy(short_password,    password,    sizeof(short_password)   -1);
	strncpy(short_oldpassword, oldpassword, sizeof(short_oldpassword)-1);
	short_password[   sizeof(short_password)   -1] =
	short_oldpassword[sizeof(short_oldpassword)-1] = '\0';

	/*
	 * Check that the characters are within the ASCII set.
	 * This restriction is not configurable!
	 */
	if (_val_chars(short_password))
	{
		if (_MBReplace(mb, MSGSTR(M_INCHAR, DEF_INCHAR), password))
			return(-1);	/* Internal error */
		errno = EINVAL;
		return(1);		/* Bad password */
	}

	/*
	 * If the PW_NOCHECK flag is set or an administrator is changing
	 * someone else's password, then the rest of the checks are not
	 * required and can be skipped.
	 */
	if ((pw->upw_flags & PW_NOCHECK) || adminchange)
		return(0);		/* Success */

	return(_ConfigurablePasswordChecks(pw,  short_password,
						short_oldpassword, mb));
}


/*
 * NAME:     _ConfigurablePasswordChecks
 *
 * FUNCTION: Performs the configurable password restrictions.
 *
 * RETURNS:  (in order of precedence)
 *           -1 - Internal error with errno set.
 *            2 - Password does not meet requirements. Password change not
 *                allowed at this time.  (minage, lifetime)
 *            1 - Password does not meet requirement. Attempt with new password.
 *            0 - Password meets requirements.
 */
static	int
_ConfigurablePasswordChecks(	UserPW	*pw,
				char	*password,
				char	*oldpassword,
				MsgBuf	*mb)
{
	int	Check = 0;
	int	Done  = FALSE;
	int	tmp_rc;
	int	rc    = 0;			/* Success. */

	while (!Done)
	{
		switch(Check)
		{
		    case 0:
			tmp_rc = _PWDAge(pw, mb);
			break;
		    case 1:
			tmp_rc = _PWDComposition(pw, password, oldpassword, mb);
			break;
		    case 2:
			tmp_rc = _PWDDictionary( pw, password, mb);
			break;
		    case 3:
			tmp_rc = _PWDHistory(    pw, password, mb);
			break;
		    case 4:
			tmp_rc = _PWDMethods(    pw, password, oldpassword, mb);
			break;
		    default:
			Done   = TRUE;
			continue;
		}

		/*
		 * tmp_rc:	2   Admin Failure.   Return  2.
		 *		1   Composition Failure.  Propagate a 1.
		 *		0   Success.  Continue processing.
		 *	     else   Internal Error.  Return -1.
		 */
		if (tmp_rc == 1)
		{
			rc =  1;	/* Composition Failure. Propagate. */
		}
		else
		if (tmp_rc)
		{
			rc = (tmp_rc == 2) ? 2 : -1;
			break;
		}
		
		Check++;
	}

	return(rc);
}


/* 
 * NAME:     _LoadConfigurationTable
 *
 * FUNCTION: Loads the password restriction attributes from the configuration
 *           file into the table.  If specific restrictions are not found,
 *           the defaults are used.
 *
 * RETURNS:  (in order of precedence)
 *           -1 - Internal error with errno set.
 *            0 - Success.
 */
static	int
_LoadConfigurationTable(char		*shortname,
			PasswordAttr	*Attr,
			int		AttrSize,
			MsgBuf		*mb)
{
	PasswordAttr	*a;
	int 		val;
	int		i;
	int		tmp_errno;

	setuserdb(S_READ);

	/*
	 * For each entry in Attr, read in the value defined for
	 * this user.
	 */
	for (i=0; i < AttrSize; i++)
	{
		a = &Attr[i];

		if (getuserattr(shortname, a->attr, (void *) &val, SEC_INT))
		{
			if (errno != ENOATTR && errno != ENOENT)
			{
				tmp_errno = errno;
				enduserdb();
				(void) _MBReplace(mb,
					MSGSTR(M_PWATTRIBUTE, DEF_PWATTRIBUTE),
								a->attr);
				errno = tmp_errno;
				return(-1);
			}
			/* Non-existing attributes are OK. */
		}
		else
		{
			/*
			 * Use 'val' if 'val' is inbetween the 'high' and 'low'
			 * values; otherwise, round down large values and
			 * round up small values.
			 */
			if ((val <= a->high) && (val >= a->low))
				a->val = val;
			else
				a->val = (val > a->high) ? a->high : a->low;

			/*
			 * Perform any defined checks on attribute values.
			 */
			if (a->ch_val)
				(*(a->ch_val)) (Attr);
		}
	}
	enduserdb();
	return(0);
}


/*****************************************************************************/
/**************  Password Restrictions External Methods Code  ****************/
/*****************************************************************************/


/*
 * NAME:     _PWDMethods
 *
 * FUNCTION: Loads and invokes external password restriction subroutines
 *           defined by the user's 'pwdchecks' attribute.
 *
 * RETURNS:  (in order of precedence)
 *           -1 - Internal error with errno set.
 *            2 - Password does not meet requirements. Password change not
 *                allowed at this time.
 *            1 - Password does not meet requirement. Attempt with new password.
 *            0 - Password meets requirements.
 */
static	int
_PWDMethods(	UserPW	*pw,
		char	*password,
		char	*oldpassword,
		MsgBuf	*mb)
{
       register	char	*Entry;
		char	*List;
		char	*message;
		char	local_user[PW_NAMELEN];
		char	local_password[PW_PASSLEN+1];
		char	local_oldpassword[PW_PASSLEN+1];
		int	(*method)();
		int	MB_rc;
		int	tmp_errno;
		int	tmp_rc;
		int	rc = 0;

	/*
	 * Get the 'pwdchecks' attribute for this user.
	 */
	setuserdb(S_READ);
	tmp_rc = getuserattr(pw->upw_name, S_PWDCHECKS, (void *)&List,SEC_LIST);
	tmp_errno = errno;

	/*
	 * If the attribute doesn't exist or is empty, then return success.
	 */
	if (tmp_rc)
	{
		enduserdb();
		if (tmp_errno == ENOATTR || tmp_errno == ENOENT)
			return(0);		/* Success */

		(void) _MBReplace(mb, MSGSTR(M_PWATTRIBUTE, DEF_PWATTRIBUTE),
								S_PWDCHECKS);
		errno = tmp_errno;	/* Return original errno. */
		return(-1);		/* Internal error */
	}

	if (!List || !(*List))
	{
		enduserdb();
		return(0);			/* Success */
	}

	/*
	 * For each entry in 'pwdchecks' load, invoke, and unload the
	 * method.  ('List' contains a DNL (double null list).)
	 */
	for (Entry = List; *Entry;)
	{
		if (!(method = load(Entry, 1, PWDCHECKS_LIBPATH)))
		{
			/* The load module didn't load. */
			tmp_errno = errno;
			(void) _MBReplace(mb, MSGSTR(M_BADDATA, DEF_BADDATA),
							Entry, S_PWDCHECKS);
			enduserdb();
			errno = tmp_errno;	/* Return original errno. */
			return(-1);		/* Internal error. */
		}

		/*
		 * Create copies of the parameters to pass to the method.
		 * We don't want this unknown method to corrupt our versions.
		 */
		strcpy(local_user,        pw->upw_name);
		strcpy(local_password,    password);
		strcpy(local_oldpassword, oldpassword);

		tmp_rc  = (*method)(local_user, local_password,
						local_oldpassword, &message);
		tmp_errno = errno;

		/* Erase the clear text passwords. */
		(void) memset(local_password,    0, sizeof(local_password));
		(void) memset(local_oldpassword, 0, sizeof(local_oldpassword));
		(void) unload(method);

		/*
		 * tmp_rc:	2   Admin Failure.   Return  2.
		 *		1   Composition Failure.  Propagate a 1.
		 *		0   Success.  Continue processing.
		 *	     else   Internal Error.  Return -1.
		 */
		if (tmp_rc == 0 || tmp_rc == 1)
		{
			/*
			 * Handle successes and composition failures.
			 */
			if (message)
			{
				MB_rc     = _MBAppend(mb, message);
				tmp_errno = errno;
				free((void *) message);
				if (MB_rc)
				{
					enduserdb();
					errno = tmp_errno;
					return(-1);	/* Internal error */
				}
			}
			else
			/*
			 * If 'method' had a composition failure, but didn't
			 * provide a message, then provide one for it.
			 */
			if (tmp_rc && _MBAppend(mb,
			    MSGSTR(M_CHKFAIL, DEF_CHKFAIL), S_PWDCHECKS, Entry))
			{
				tmp_errno = errno;
				enduserdb();
				errno = tmp_errno;
				return(-1);	/* Internal error */
			}

			if (tmp_rc)
				rc = 1;	/* Composition Failure. Propagate. */
		}
		else
		if (tmp_rc == 2)
		{
			/*
			 * Handle administrative failures.
			 */
			if (message)
			{
				MB_rc = _MBReplace(mb, message);
				tmp_errno = errno;
				free((void *) message);
				if (MB_rc)
				{
					enduserdb();
					errno = tmp_errno;
					return(-1);	/* Internal error */
				}
			}
			else
			/*
			 * If 'method' had an administration failure, but didn't
			 * provide a message, then provide one for it.
			 */
			if (_MBReplace(mb, MSGSTR(M_CHKADMFAIL, DEF_CHKADMFAIL),
						S_PWDCHECKS, Entry))
			{
				tmp_errno = errno;
				enduserdb();
				errno = tmp_errno;
				return(-1);		/* Internal error */
			}

			enduserdb();
			return(2);
		}
		else
		{
			/*
			 * Handle all other values as internal errors.
			 */
			if (message)
			{
				(void) _MBReplace(mb, message);
				free((void *) message);
			}
			else
			{
				/*
				 * If 'method' had an internal error, but didn't
				 * provide a message, then provide one for it.
				 */
				(void) _MBReplace(mb, MSGSTR(M_CHKERR,
					DEF_CHKERR), S_PWDCHECKS, Entry);
			}
			enduserdb();
			errno = tmp_errno;  /* Return original errno. */
			return(-1);
		}

		while(*Entry++);	/* Go to next DNL entry. */
	}

	enduserdb();
	return(rc);
}


/*****************************************************************************/
/****************  Password Composition Restriction Code  ********************/
/*****************************************************************************/


/*
 * These #defines are the indices to the CompositionAttrs[] table.
 */
#	define	CA_MINALPHA	0	/* minalpha   index  */
#	define	CA_MINOTHER	1	/* minother   index  */
#	define	CA_MINDIFF	2	/* mindiff    index  */
#	define	CA_MAXREPEAT	3	/* maxrepeats index  */
#	define	CA_MINLEN	4	/* minlen     index  */
#	define	CA_NATTRS 	5	/* number of entries */

/*
 * NAME:     _CompositionInitialize
 *
 * FUNCTION: Copies the CompositionAttrs[] table into a readable and
 *           writable location (i.e., into a new table) and invokes
 *           _LoadConfigurationTable() to update the default values in the
 *           new table with the values contained in /etc/security/user.
 *
 * RETURNS:  (in order of precedence)
 *           -1 - Internal error with errno set.
 *            0 - Success.
 */
static	int
_CompositionInitialize(	char		*shortname,
			PasswordAttr	*Attrs,
			MsgBuf		*mb)
{
	/*
	 * Note because _ch_minother() is associated with minother and depends
	 * upon minalpha, minalpha must always be evaluated before minother.
	 * Thus, minalpha must always come before minother in
	 * CompositionAttrs[].  In general, if a checking subroutine depends
	 * upon multiple attributes, those attributes must be evaluated before
	 * the checking subroutine is invoked.
	 */
	static PasswordAttr CompositionAttrs[] =
	{
  	/* value, low, high, sanity check, validate, flag, attr,  
	  default singular, default plural, message singular, message plural */

  	{ MIN_MINALPHA, MIN_MINALPHA, MAX_MINALPHA,
	  NULL,         _val_minalpha,  0, S_MINALPHA,
	  DEF_MINALPHA,  DEF_MINALPHAS,  MSGC(M_MINALPHA), MSGC(M_MINALPHAS)},

  	{ MIN_MINOTHER, MIN_MINOTHER, MAX_MINOTHER,
	  _ch_minother, _val_minother,  0, S_MINOTHER,
	  DEF_MINOTHER,  DEF_MINOTHERS,  MSGC(M_MINOTHER), MSGC(M_MINOTHERS)},

  	{ MIN_MINDIFF,  MIN_MINDIFF,  MAX_MINDIFF,
	  NULL,         _val_mindiff,   0, S_MINDIFF,
 	  DEF_MINDIFF,   DEF_MINDIFFS,   MSGC(M_MINDIFF), MSGC(M_MINDIFFS)},

  	{ MAX_MAXREP,   MIN_MAXREP,   MAX_MAXREP,
	  NULL,         _val_maxrepeat, 0, S_MAXREPEAT,
	  DEF_MAXREPEAT, DEF_MAXREPEATS, MSGC(M_MAXREPEAT), MSGC(M_MAXREPEATS)},

  	{ MIN_MINLEN,   MIN_MINLEN,   MAX_MINLEN,
	  NULL,         _val_minlen,    0, S_MINLEN,
 	  DEF_MINLENGTH, DEF_MINLENGTHS, MSGC(M_MINLENGTH), MSGC(M_MINLENGTHS)}
	};

	/*
	 * Copy data from the readonly CompositionAttrs into Attrs.
	 */
	(void) memcpy(Attrs, CompositionAttrs, sizeof(CompositionAttrs));

	/*
	 * Read and set up this user's composition configuration table.
	 */
	if (_LoadConfigurationTable(shortname, Attrs, CA_NATTRS, mb) < 0)
		return(-1);	/* Internal error */

	return(0);
}


/*
 * NAME:     _PWDComposition
 *
 * FUNCTION: Performs all of the password composition restrictions defined
 *           in the CompositionAttrs[] table of _CompositionInitialize().
 *
 * RETURNS:  (in order of precedence)
 *           -1 - Internal error with errno set.
 *            1 - Password does not meet requirement. Attempt with new password.
 *            0 - Password meets requirements.
 */
static	int
_PWDComposition(UserPW	*pw,
		char	*password,
		char	*oldpassword,
		MsgBuf	*mb)
{
	PasswordAttr	Attrs[CA_NATTRS];
	PasswordAttr	*a;
	int		i;
	int		AttrFailed = 0;

	/*
	 * Read and set up this user's composition configuration table.
	 */
	if (_CompositionInitialize(pw->upw_name, Attrs, mb) < 0)
		return(-1);	/* Internal error */

	/*
	 * Traverse the list of composition checking subroutines, calling
	 * each and recording failures.
	 */
	for (i=0; i < CA_NATTRS; i++)
	{
		a = &Attrs[i];

		if (a->validate && (*(a->validate))(Attrs,password,oldpassword))
				AttrFailed = a->fail = 1;
	}

	if (AttrFailed)
	{
		if (_CompositionMessages(Attrs, CA_NATTRS, mb))
			return(-1);	/* Internal error */
		return(1);		/* Bad password   */
	}

	return(0);			/* Success */
}


/*
 * NAME:     _CompositionMessages
 *
 * FUNCTION: First generates the overall composition restriction messages.
 *           Then generates messages for the restrictions that the password
 *           failed to meet.
 *
 * RETURNS:  (in order of precedence)
 *           -1 - Internal error with errno set.
 *            0 - Success.
 */
static	int
_CompositionMessages(	PasswordAttr	*Attrs,
			int		AttrSize,
			MsgBuf		*mb)
{
	PasswordAttr	*a;
	char		*message;
	int		i;

	/* 
	 * Record the password constraints the user must meet.
	 */
	if (_MBAppend(mb, MSGSTR(M_ATTREQ, DEF_ATTREQ)))
		return(-1);


	/*
	 * For each constraint that is active, tell the user what the
	 * active value is.
	 */
	for (i=0; i < AttrSize; i++) 
	{ 
		a = &Attrs[i];

		if (a->val != a->low)
		{
			/*
			 * Create a singular or plural message.
			 */
			message = (a->val == 1) ? MSGSTR(a->emsg,  a->dmsg)
						: MSGSTR(a->emsgs, a->dmsgs);
			if (_MBAppend(mb, message, a->val))
					return(-1);
		}
	}

	/* 
	 * Record the password constraints the user failed to meet.
	 */
	if (_MBAppend(mb, MSGSTR(M_ATTRFAIL, DEF_ATTRFAIL)))
		return(-1);

	for (i=0; i < AttrSize; i++) 
	{
		a = &Attrs[i];

		if (a->fail)
		{
			/*
			 * Create a singular or plural message.
			 */
			message = (a->val == 1) ? MSGSTR(a->emsg,  a->dmsg)
						: MSGSTR(a->emsgs, a->dmsgs);
			if (_MBAppend(mb, message, a->val))
					return(-1);
		}
	}

	if (_MBAppend(mb, "\n"))
		return(-1);

  	return(0);
}


/* 
 * NAME:     _ch_minother
 *
 * FUNCTION: Sanity check the value of minother. 'minalpha + minother' should
 *           not be greater than PW_PASSLEN.  If they are, then reduce minother.
 *           _LoadConfigurationTable() ensures that minalpha and minother are
 *           each PW_PASSLEN bytes or less.
 * 
 * RETURNS:  NONE
 */
static void
_ch_minother(PasswordAttr *Attr)
{
	int	 minalpha =   Attr[CA_MINALPHA].val;
	int	*minother = &(Attr[CA_MINOTHER].val);

	if ((minalpha + *minother) > PW_PASSLEN)
		*minother = PW_PASSLEN - minalpha;
}


/*
 * NAME:     _val_chars
 *
 * FUNCTION: Checks the password string for 8 bit values.  Only 7 bit values
 *           are allowed because crypt() assumes 7 bit values.
 *
 * RETURNS:  (in order of precedence)
 *           1 - Failure. Contains 8 bit values.
 *           0 - Success. Conforms.
 */
static	int
_val_chars(char *password)
{
	char 	*p;

	if (password)
		for (p = password; *p; p++)
			/* No NLS characters allowed! */
			if (*p & 0x80)
	    			return(1);

	return (0);
}


/*
 * NAME:     _val_minalpha
 *
 * FUNCTION: Check to make sure the password has at least a minimum
 *           number of alphabetic characters.
 *
 * RETURNS:  (in order of precedence)
 *           1 - Failure.  Contains less than minalpha characters.
 *           0 - Success.
 */
static	int
_val_minalpha(PasswordAttr *Attr, char *password, char *oldpassword)
{
	int	minalpha = Attr[CA_MINALPHA].val;
	int 	alpha    = 0;

	if (!password)
		return(minalpha ? 1 : 0);

	while (*password)
		if (isalpha ((int)*password++))
			alpha++;

	return((alpha < minalpha) ? 1 : 0);
}


/*
 * NAME:     _val_minother
 *
 * FUNCTION: Checks that the password has a minimum of characters other
 *           than alphabetic characters.
 *
 * RETURNS:  (in order of precedence)
 *           1 - Failure.  Contains less than minother characters.
 *           0 - Success.
 */
static	int
_val_minother(PasswordAttr *Attr, char *password, char *oldpassword)
{
	int	minother = Attr[CA_MINOTHER].val;
	int 	other    = 0;

	if (!password)
		return(minother ? 1 : 0);

	while(*password)
		if (!isalpha((int)*password++))
			other++;

	return((other < minother) ? 1 : 0);
}


/*
 * NAME:     _val_mindiff 
 *
 * FUNCTION: Checks that the number of characters in the new password  
 *           differs from the old password by mindiff characters.
 *           Note that mindiff is bound by the size of the new password.
 *           If mindiff=8 and the new password contains only 6 characters,
 *           mindiff is reduced to 6.  Thus, if minlen=6 and mindiff=8,
 *           6 character passwords are allowed but all 6 characters must
 *           be different.  Passwords are not forced to be at least
 *           mindiff characters in length.  Instead minlen and 'minalpha
 *           + minother' are used to enforce the size of a password.
 *           This check is silent about new passwords that contain less
 *           than mindiff characters.
 *            
 * RETURNS:  (in order of precedence)
 *           1 - Failure.  Contains less than adjusted mindiff characters.
 *           0 - Success.
 */
static	int
_val_mindiff(PasswordAttr *Attr, char *password, char *oldpassword)
{
	int	mindiff;
	int	diff;

	if (password && oldpassword && oldpassword[0])
	{
		/* Adjust mindiff. */
		mindiff = strlen(password);
		if (Attr[CA_MINDIFF].val < mindiff)
			mindiff = Attr[CA_MINDIFF].val;

		if (mindiff)
		{
			/* Check the password for mindiff characters. */
			for (diff = 0; *password; password++)
				if (!strchr(oldpassword, *password)) 
					++diff;

			if (diff < mindiff)
				return 1;
		}
	}
	return (0);
}


/*
 * NAME:     _val_maxrepeat
 *
 * FUNCTION: Checks that the password characters do not repeat more
 *           times than allowed by the maxrepeats password restriction. 
 *
 * RETURNS:  (in order of precedence)
 *           1 - Failure.  Contains too many repeating values.
 *           0 - Success.
 */
static	int
_val_maxrepeat(PasswordAttr *Attr, char *password, char *oldpassword)
{
	char 	*p;
	int	repeat;
	int	maxrepeat = Attr[CA_MAXREPEAT].val;

	if (password && maxrepeat)
	{
		for (; *password; password++)
		{
			repeat = 0;
			for (p = password; *p; p++)
				if ((*password == *p) && (++repeat > maxrepeat))
					return (1);
		}
	}
	return (0);
}


/*
 * NAME:     _val_minlen  (Feature 57080)
 *
 * FUNCTION: Checks that the given password conforms to the minimum length
 *           restriction.
 *
 * RETURNS:  (in order of precedence)
 *           1 - Failure.  Password is too small.
 *           0 - Success.
 */
static	int
_val_minlen(PasswordAttr *Attr, char *password, char *oldpassword)
{
	int	minlen = Attr[CA_MINLEN].val;

	if (!password)
		return(minlen ? 1 : 0);

	return((strlen(password) < minlen) ? 1 : 0);
}


/*
 * NAME:     passwdrequired
 *
 * FUNCTION: Checks to see if the user is required to have a password.
 *           Passwords are required if the password restrictions specify
 *           a minimum length for the password.
 *
 * NOTES:    passwdrequired() and passwdexpired() are both written with
 *           a different philosophy for handling messages and internal errors.
 *           Unlike the other subroutines in this file, these 2 subroutines
 *           are more insistent on returning the proper return code than on
 *           returning an error message (i.e., if allocating memory for error
 *           messages fails, these 2 subroutines ignore the failure).  This
 *           philosophical difference is due to where these 2 subroutines
 *           are used.
 *
 * RETURNS:  (in order of precedence)
 *           -1 - Internal error with errno set.
 *            1 - Password required.
 *            0 - Password not required.
 */
int
passwdrequired(char *shortname, char **message)
{
	PasswordAttr	Attrs[CA_NATTRS];
	MsgBuf		mb;
	int		rc;

	_MBInitialize(&mb, message);

	if (_CompositionInitialize(shortname, Attrs, &mb))
		return(-1);

	rc = (Attrs[CA_MINALPHA].val > 0 ||
	      Attrs[CA_MINOTHER].val > 0 ||
	      Attrs[CA_MINLEN  ].val > 0) ? 1 : 0;

	if (rc)
		(void) _MBReplace(&mb, MSGSTR(M_PASREQ, DEF_PASREQ));
	return(rc);
}

/*
 * To avoid the accidental usage of these defines by the age related 
 * subroutines, undefine them.  This also provides a sense of modularity.
 */
#undef	CA_MINALPHA
#undef	CA_MINOTHER
#undef	CA_MINDIFF
#undef	CA_MAXREPEAT
#undef	CA_MINLEN
#undef	CA_NATTRS



/*****************************************************************************/
/********************  Password Age Restriction Code  ************************/
/*****************************************************************************/


#define	R_CHANGING	0	/* Responses for a user changing his password.*/
#define	R_EXPIRED	1	/* Responses for the system checking the      */
				/* expired state of a password.               */

/*
 * These #defines are the indices to the AgeAttrs[] table.
 */
#define	AA_MINAGE	0	/* minage     index  */
#define	AA_MAXAGE	1	/* maxage     index  */
#define	AA_MAXEXPIRED	2	/* maxexpired index  */
#define	AA_NATTRS 	3	/* number of entries */

/*
 * NAME:     _AgeInitialize
 *
 * FUNCTION: Copies the AgeAttrs[] table into a readable and
 *           writable location (i.e., into a new table) and invokes
 *           _LoadConfigurationTable() to update the default values in the
 *           new table with the values contained in /etc/security/user.
 *
 * RETURNS:  (in order of precedence)
 *           -1 - Internal error with errno set.
 *            0 - Success.
 */
static	int
_AgeInitialize(	char	 	*shortname,
		PasswordAttr	*Attrs,
		MsgBuf		*mb)
{
	static PasswordAttr AgeAttrs[AA_NATTRS] =
	{
	/* value, low, high, sanity check, validate, flag, attr,  
	  default singular, default plural, message singular, message plural */
	  { MIN_MINAGE, MIN_MINAGE, MAX_MINAGE, NULL, NULL, 0, S_MINAGE,
	    DEF_MINAGE,    DEF_MINAGES,    MSGC(M_MINAGE),    MSGC(M_MINAGES)},
	  { MIN_MAXAGE, MIN_MAXAGE, MAX_MAXAGE, NULL, NULL, 0, S_MAXAGE,
	    (char *)NULL,(char *)NULL,     NULL,              NULL           },
	  { MIN_MAXEXP, MIN_MAXEXP, MAX_MAXEXP, NULL, NULL, 0, S_MAXEXPIRED,
	    (char *)NULL,(char *)NULL,     NULL,              NULL           }
	};

	/*
	 * Copy data from the readonly AgeAttrs into Attrs.
	 */
	(void) memcpy(Attrs, AgeAttrs, sizeof(AgeAttrs));

	/*
	 * Read and set up this user's age configuration table.
	 */
	if (_LoadConfigurationTable(shortname, Attrs, AA_NATTRS, mb) < 0)
		return(-1);	/* Internal error */

	return(0);
}


/*
 * NAME:     _PWDAge
 *
 * FUNCTION: Performs all of the password age restrictions defined
 *           in the AgeAttrs[] table of _AgeInitialize() for a user
 *           changing a password.
 *
 * RETURNS:  (in order of precedence)
 *           -1 - Internal error with errno set.
 *            2 - Password does not meet requirements. Password change not
 *                allowed at this time.  (minage, maxexpired)
 *            0 - Password meets requirements.
 */
static	int
_PWDAge(UserPW	 *pw, MsgBuf	 *mb)
{
	/*
	 * The user is free to change his password if ADMCHG is set,
	 * regardless of elapsed time.
	 */
	if (pw->upw_flags & PW_ADMCHG)
		return(0);

	return(_AnalyzeAges(pw, R_CHANGING, mb));
}


/*
 * Below is the password age restriction state machine.  Note that if maxage
 * is not active, then maxexpired is ignored (i.e., maxexpired is dependent
 * on maxage).
 *
 *     MaxExp  MaxAge  MinAge   Result       Action                Change  Exprd
 *       0       0       0      OK.          None.                   0       0*
 *       0       0       1      MinAge.      User can't change yet.  2       3
 *       0       1       0      MaxAge.      User must change.       0       1
 *       0       1       1      Bad state.   Admin change required.  2       2
 *       1       0       0      OK.          None.                   0       0*
 *       1       0       1      MinAge.      User can't change yet.  2       3
 *       1       1       0      MaxExpired.  Admin change required.  2       2
 *       1       1       1      MaxExpired.  Admin change required.  2       2
 *
 * Legend:  MaxExp  -  maxexpired
 *          MaxAge  -  maxage
 *          MinAge  -  minage
 *          Change  -  Can the user change it?    PWDAge() return codes.
 *          Exprd   -  Has the password expired?  passwdexpired() return codes.
 *
 *   *For states 000 and 100, pwdwarntime will be evaluated for passwdexpired().
 *
 * For Change and Exprd:
 *     3 - MinAge.
 *     2 - User can't change.
 *     1 - Password expired.
 *     0 - Password OK as per the request.
 *
 * This state machine is based on the following precedences:
 *      1) MaxExpired takes precedence over MinAge when changing a password.
 *      2) Return code of 2 has a greater precedence over a return code of 1.
 *
 * Note that I use '>=' for maxage and '<' for minage so that there isn't
 * an over lap or gap when both minage and maxage have the same value.
 * I also use '>=' for maxexpired so that both maxage and maxexpired take
 * affect simultaneously when maxexpired == 0, thus, there isn't a gap where
 * the user can change his password.
 *
 * Note that maxexpired is not enforced on root.  This is to keep root
 * from being locked out of the system.
 *
 * Note that if 'now < lastupdate' (i.e., a chronometer problem), the
 * result will be the state 000 (password OK).
 */

#define	MaxExp	0x4	/* 3rd bit */
#define	MaxAge	0x2	/* 2nd bit */
#define	MinAge	0x1	/* 1st bit */

/*
 * NAME:     _AnalyzeAges
 *
 * FUNCTION: Performs all of the password age restrictions defined
 *           in the AgeAttrs[] table of _AgeInitialize() and produces
 *           different return values and messages based on 'response'.
 *
 * NOTES:    R_CHANGING - Responses for a user changing his password.
 *           R_EXPIRED  - Responses for the system checking the expired state
 *                        of a password.
 *
 * RETURNS:  (in order of precedence)
 *           -1 - Internal error with errno set.
 *            2 - Password does not meet requirements. Password change not
 *                allowed at this time.  (minage, maxexpired)
 *            1 - Password does not meet requirements.
 *            3 - Password can't be changed yet.  (minage)
 *            0 - Password meets requirements.
 */
static	int
_AnalyzeAges(	UserPW	*pw,
		int	response,
		MsgBuf	*mb)
{
	PasswordAttr	Attrs[AA_NATTRS];
	char		*message;
	ulong		maxage_start = 0;
	ulong		time_remaining = 0;
	ulong		now;
	int		minage;
	int		maxage;
	int		maxexpired;
	int		state = 0;
	int		rc;

	if (_AgeInitialize(pw->upw_name, Attrs, mb))
		return(-1);

	minage     = Attrs[AA_MINAGE    ].val;
	maxage     = Attrs[AA_MAXAGE    ].val;
	maxexpired = Attrs[AA_MAXEXPIRED].val;


	/*
	 * If upw_lastupdate is non-zero, then consider it valid.
	 * (getuserpw() and _PasswdRetrievalPolicy() set upw_lastupdate to
	 * zero if they can't find the lastupdate attribute.)
	 */
	if (pw->upw_lastupdate)
	{
		now = (ulong) time((time_t *) 0);

		maxage_start = pw->upw_lastupdate + WEEKS2SECONDS(maxage);

		/* Check maxexpired.  root is exempt from maxexpired. */
		if (maxexpired >= 0 && getuid() &&
			   now >= (maxage_start + WEEKS2SECONDS(maxexpired)))
			state |= MaxExp;

		/* Check maxage. */
		if (maxage > 0)
			if (now >= maxage_start)
				state |= MaxAge;
			else
				time_remaining = maxage_start - now;

		/*
		 * Check minage. Only perform this check if 'now >= lastupdate'.
		 */
		if (minage > 0 && now >= pw->upw_lastupdate && 
			   now < (pw->upw_lastupdate + WEEKS2SECONDS(minage)))
			state |= MinAge;
	}
	else
	{
		/*
		 * The lastupdate field is invalid.  If maxage is being
		 * enforced, then force the user to change his password.
		 */
		if (maxexpired >= 0)
			state |= MaxExp;
		if (maxage > 0)
			state |= MaxAge;
	}

	/*
	 * Generate the proper message and return code value.
	 */
	switch (state)
	{
	    /* Password OK */
	    case      0|     0|     0:
	    case MaxExp|     0|     0:
		if (response == R_EXPIRED && time_remaining)
			_WarnTime(pw, time_remaining, maxage_start, mb);
		rc = 0;
		break;

	    /* MinAge */
	    case      0|     0|MinAge:
	    case MaxExp|     0|MinAge:
		if (response == R_EXPIRED)
			rc = 3;		/* Within MinAge */
		else
		{
			message = (minage == 1) ? MSGSTR(M_MINAGE, DEF_MINAGE)
						: MSGSTR(M_MINAGES,DEF_MINAGES);
			(void) _MBReplace(mb, message, minage);
			rc = 2;		/* Can't change yet */
		}
		break;

	    /* MaxAge */
	    case      0|MaxAge|     0:
		if (response == R_EXPIRED)
		{
			(void) _MBAppend(mb, MSGSTR(M_PASEXP, DEF_PASEXP));
			rc = 1;		/* Expired */
		}
		else
			rc = 0;		/* OK to change */
		break;

	    /* Bad State */
	    case      0|MaxAge|MinAge:
		(void) _MBReplace(mb, MSGSTR(M_BADSTATE, DEF_BADSTATE));
		(void) _MBAppend( mb, MSGSTR(M_SEEADMIN, DEF_SEEADMIN));
		rc = 2;
		break;

	    /* MaxExpired */
	    case MaxExp|MaxAge|     0:
	    case MaxExp|MaxAge|MinAge:
		(void) _MBReplace(mb, MSGSTR(M_ADMINEXP, DEF_ADMINEXP));
		(void) _MBAppend( mb, MSGSTR(M_SEEADMIN, DEF_SEEADMIN));
		rc = 2;
		break;
	}
	return(rc);
}

#undef	MaxExp
#undef	MaxAge
#undef	MinAge


/*
 * NAME:     _WarnTime
 *
 * FUNCTION: Generates a password expiration warning message if the
 *           password will expire in pwdwarntime days or less.
 *
 * NOTES:    This code is written as if it were an exported return.
 *
 * RETURNS:  NONE
 */
static	void
_WarnTime(	UserPW	*pw,
		ulong	time_remaining,
		ulong	maxage_start,
		MsgBuf	*mb)
{
	char		buf[256];
	struct	tm	local_time;
	int		pwdwarntime;
	int		rc;

	setuserdb(S_READ);
	rc = getuserattr(pw->upw_name, S_PWDWARNTIME, (void *) &pwdwarntime,
								SEC_INT);

	/*
	 * If the warning period is greater than or equal to the time remaining,
	 * then print a warning message.
	 */
	if (!rc && pwdwarntime > 0 && time_remaining &&
			((ulong) DAYS2SECONDS(pwdwarntime)) >= time_remaining)
	{
		local_time = *localtime((time_t *) &maxage_start);
		if (strftime(buf, sizeof(buf), "%c", &local_time))
			(void) _MBReplace(mb, MSGSTR(M_EXPWARN, DEF_EXPWARN),
									buf);
	}

	enduserdb();
}


/*
 * NAME:     passwdexpired
 *
 * FUNCTION: Checks to see if user's passwd must be changed.  This 
 *           routine checks local restrictions or remote restrictions
 *           depending on where the user is defined.
 *
 *           For local restrictions, the password is considered expired if:
 *              1) An administrator made the last change.
 *              2) The password's age is greater than age allowed.
 *              3) The password is not a valid crypted string.
 *              4) The password is null and passwords are required.
 *
 *           The order of precedence of the 'upw_flags' values is (high to low):
 *              PW_ADMIN   (Only root can change this password.)
 *              PW_ADMCHG  (Force user to change password.)
 *              PW_NOCHECK (All configurable password restrictions are ignored.)
 *
 *           For remote restrictions, the appropriate method is loaded.
 *
 * NOTES:    passwdrequired() and passwdexpired() are both written with
 *           a different philosophy for handling messages and internal errors.
 *           Unlike the other subroutines in this file, these 2 subroutines
 *           are more insistent on returning the proper return code than on
 *           returning an error message (i.e., if allocating memory for error
 *           messages fails, these 2 subroutines ignore the failure).  This
 *           philosophical difference is due to where these 2 subroutines
 *           are used.
 *
 * RETURNS:  (in order of precedence)
 *           -1 - Internal error with errno set.
 *            2 - Password expired. Requires administrative intervention.
 *            1 - Password expired.
 *            0 - Password has not expired.
 */
int
passwdexpired(char *longname, char **message)
{
	SMTable	methodtable;
	Passwd	*pwd;
	UserPW 	*pw;
	UserPW 	tpw;
	MsgBuf	mb;
	char 	shortname[PW_NAMELEN];
	char	cryptpwd[PW_CRYPTLEN+1];
	char	*registry;
	char	*msg;
	int	minage;
	int	disallowed;
	int	tmp_errno;
	int	rc;

	_MBInitialize(&mb, message);

	_normalize_username(shortname, longname);
	
	if ((registry = _getregistry(longname)) == (char *)NULL)
		return(0);

	if (ISLOCAL(registry))
	{
		if (_PasswdRetrievalPolicy(shortname, &pwd, &pw, registry))
		{
			(void) _MBReplace(&mb, MSGSTR(M_BADUSER, DEF_BADUSER));
			return(-1);
		}

		free((void *) registry);

		/*
		 * Place pw onto the stack so that we don't have to have
		 * a _userpwfree(pw) statement for every 'return' statement.
		 */
		memcpy(&tpw, pw, sizeof(UserPW));
		if (pw->upw_passwd)
			tpw.upw_passwd = strncpy(cryptpwd,
					pw->upw_passwd, sizeof(cryptpwd));
		cryptpwd[sizeof(cryptpwd)-1] = '\0';
		_userpwfree(pw);
		pw = &tpw;

		_pwdfree(pwd);	/* Not used */

		/*
		 * The passwords of users who authenticated should be valid,
		 * but a user that gained access via a .rhosts file may have
		 * an asterisk for an encrypted password or a corrupt encrypted
		 * password.  Under these circumstances, no user will able to
		 * change his own password, including root.
		 */
		if (pw->upw_passwd && pw->upw_passwd[0] &&
						!_valid_crypted(pw->upw_passwd))
		{
			/* fix for rlogind - after the message buffer is sent to
			 * rlogin, '\r' is not inserted following '\n'.
			 */
			(void) _MBReplace(&mb, MSGSTR(M_BADCRYPT,DEF_BADCRYPT));
			(void) _MBAppend(&mb,"\r");
			(void) _MBAppend(&mb,MSGSTR(M_ADMINONLY,DEF_ADMINONLY));
			(void) _MBAppend(&mb,"\r");
			return(2);
		}

		/*
		 * Determine if user can change this password.
		 */
		disallowed = checkforadm(pw->upw_flags);

		/*
		 * If the PW_ADMCHG flag is set, then consider the password
		 * as expired.
		 */
		if (pw->upw_flags & PW_ADMCHG)
		{
			/*
			 * A change is required.  Check if the user can change
			 * this password.  (Handle the anomalous condition of
			 * both PW_ADMIN and PW_ADMCHG being set.)
			 */
			if (disallowed)
			{
				(void) _MBReplace(&mb,
					MSGSTR(M_PASADMIN, DEF_PASADMIN));
				(void) _MBAppend(&mb,
					MSGSTR(M_ADMINONLY, DEF_ADMINONLY));
				return(2);
			}

			(void) _MBReplace(&mb, MSGSTR(M_PASADM,DEF_PASADM));
			return(1);
		}

		/*
		 * If PW_NOCHECK is set, then ignore age and composition
		 * restrictions.
		 */
		if (pw->upw_flags & PW_NOCHECK)
			return (0);

		/*
		 * Check if the password has expired.
		 */
		if ((rc = _AnalyzeAges(pw, R_EXPIRED, &mb)) != 0 && rc != 3)
		{
			/*
			 * A change is required or _AnalyzeAges()
			 * detected an internal error.
			 */
			if (rc != -1 && disallowed)
			{
				(void) _MBReplace(&mb,MSGSTR(M_PASEXPED,
								DEF_PASEXPED));
				(void) _MBAppend(&mb,MSGSTR(M_ADMINONLY,
					      			DEF_ADMINONLY));
				return(2);
			}
			return(rc);
		}

		/*
		 * Check if _AnalyzeAges() signaled that we are inside minage.
		 */
		minage = (rc == 3);

		/*
		 * If there is no password and not within the minage period,
		 * then check if there should be a password.
		 */
		msg = (char *) NULL;
		if ((!pw->upw_passwd || !pw->upw_passwd[0]) && !minage &&
				(rc = passwdrequired(pw->upw_name, &msg)))
		{
			tmp_errno = errno;
			/*
			 * A change is required or passwdrequired()
			 * detected an internal error.
			 */

			if (rc != -1 && disallowed)
			{
				if (msg) free((void *) msg);
				(void) _MBReplace(&mb,
					      MSGSTR(M_PASREQED, DEF_PASREQED));
				(void) _MBAppend(&mb,
					    MSGSTR(M_ADMINONLY, DEF_ADMINONLY));
				return(2);
			}
			if (msg)
			{
				(void) _MBReplace(&mb, msg);
				free((void *) msg);
			}

			errno = tmp_errno;
			return(rc);
		}
		/* Ignore message if no password is required */
		if (msg) free((void *) msg);
	}
	else
	{
		/*
		 * Load the security method and call the method's
		 * passwdexpired() interface if defined.
		 */
		if (_load_secmethod(registry, &methodtable))
		{
			tmp_errno = errno;
			(void) _MBReplace(mb, MSGSTR(M_FAILLOAD, DEF_FAILLOAD),
								registry);
			free((void *) registry);
			errno = tmp_errno;
			return(-1);		/* Internal error */
		}
		free((void *) registry);

		if (methodtable.method_passwdexpired == NULL)
			return(0);

		return((*methodtable.method_passwdexpired)(longname, message));
	}
	return (0);
}


/*
 * Name:     _valid_crypted
 *
 * FUNCTION: Checks a crypted passwd for validity (e.g., what crypt() 
 *           would return).  The VALID_CHAR() set defines the range
 *           of characters which are acceptable.  The password must
 *           also be thirteen characters in length.
 *
 * RETURNS:  (in order of precedence)
 *           1 - password is a valid crypted password
 *           0 - password is invalid
 */
#define	VALID_CHAR(x)	\
	(((x) >= 'a' && (x) <= 'z') || \
	 ((x) >= 'A' && (x) <= 'Z') || \
	 ((x) >= '0' && (x) <= '9') || \
	  (x) == '.' || (x) == '/')
static	int
_valid_crypted(char *crypted)
{
	char *c;

	/*
	 * Walk thru the password as long as we're seeing valid characters.
	 */
	for (c = crypted; *c != '\0' && VALID_CHAR(*c); c++);

	return (*c == '\0' && c - crypted == 13);
}


/*****************************************************************************/
/*****************  Password Dictionary Restriction Code  ********************/
/*****************************************************************************/

#define	TOLOWERCASE(b,p)			\
	for (p = b; *p; p++)			\
		if (*p >= 'A' && *p <= 'Z')	\
			*p += 'a' - 'A';

/*
 * NAME:     _PWDDictionary
 *
 * FUNCTION: Performs the dictionary restrictions on new passwords.
 *           It scans each file contained in the user's dictionlist
 *           attribute until it finds a match or until all dictionary
 *           files have been read.  The comparisons are case insensitive.
 *
 * NOTES:    Since passwords can only contain 7 bit ASCII characters, 
 *           a non-NLS, brute force case conversion technique is used.
 *
 * RETURNS:  (in order of precedence)
 *           -1 - Internal error with errno set.
 *            1 - Password matches a word in the dictionary. Try again.
 *            0 - Password does not match any words in the dictionary.
 */
static	int
_PWDDictionary(	UserPW	*pw,
		char	*password,
		MsgBuf	*mb)
{
       register	char	*p;
       register	char	*Entry;
		char	*List;
        	char	buffer[100];
		char	lc_password[PW_PASSLEN+1];
        	FILE	*file;
		int	tmp_errno;
		int	rc;

	/*
	 * Get the 'dictionlist' attribute for this user.
	 */
	setuserdb(S_READ);
	rc = getuserattr(pw->upw_name, S_DICTION, (void *) &List, SEC_LIST);
	tmp_errno = errno;

	/*
	 * If the attribute doesn't exist or is empty, then return success.
	 */
	if (rc)
	{
		enduserdb();

		if (tmp_errno == ENOATTR || tmp_errno == ENOENT)
			return(0);		/* Success */

		(void) _MBReplace(mb, MSGSTR(M_PWATTRIBUTE, DEF_PWATTRIBUTE),
								S_DICTION);
		errno = tmp_errno;	/* Return original errno. */
		return(-1);		/* Internal error */
	}

	if (!List || !(*List))
	{
		enduserdb();
		return(0);			/* Success */
	}

	/* Convert the password to lower case. */
	strcpy(lc_password, password);
	TOLOWERCASE(lc_password, p);

	/*
	 * Scan through each dictionary in 'dictionlist'.
	 * ('List' contains a DNL (double null list) of dictionary files.)
	 */
	for (Entry = List; *Entry && !rc;)
	{
		if (!(file = fopen(Entry, "r")))
		{
			tmp_errno = errno;
			(void) _MBReplace(mb, MSGSTR(M_DICTERR, DEF_DICTERR),
									Entry);
			rc = -1;
			break;
		}
		while (fgets(buffer, sizeof(buffer), file))
		{
			if (p=strchr(buffer, '\n'))
				*p = '\0';

			TOLOWERCASE(buffer, p);

			if (!strcmp(lc_password, buffer))
			{
				rc = _MBAppend(mb, MSGSTR(M_DICTMATCH,
					DEF_DICTMATCH)) ? -1 : 1;
				tmp_errno = errno;
				break;
			}
		}
		fclose(file);

		while(*Entry++);	/* Go to next DNL entry. */
	}
	/* Erase the clear text password. */
	memset(lc_password, 0, sizeof(lc_password));
	enduserdb();

	if (rc == -1)
		errno = tmp_errno;
	return(rc);				/* Success */
}


/*****************************************************************************/
/**************************  Password Policy Code  ***************************/
/*****************************************************************************/


/*
 * NAME:     _PasswdRetrievalPolicy
 *
 * FUNCTION: Returns Passwd and UserPW structures that are consistent.
 *           Login uses what getpwnam() returns for a password, which
 *           may not be consistent with what getuserpw() returns.
 *
 * RETURNS:  (in order of precedence)
 *           -1 - Failure.  *pwd = *pw = NULL.
 *            0 - Success.  *pwd and *pw are valid.
 */
int
_PasswdRetrievalPolicy(	char	*shortname,
			Passwd	**pwd,
			UserPW	**pw,
			char	*registry)
{
	Passwd	*tpwd;
	UserPW	*tpw;
	int	tmp_errno;
	int	lookup_domain = 0;

	*pwd = (Passwd *) NULL;
	*pw  = (UserPW *) NULL;
	
	/*
	 * Determine where to lookup passwd information based on the
	 * user's registry attribute.  Normally this lookup is controlled
	 * the AUTHSTATE variable.  But for password changes, the lookups
	 * should go right to the user's registry.
	 *
	 */
	if ( !strcmp(registry,AUTH_FILES) || !strcmp(registry,AUTH_NIS) )
		lookup_domain = 2;
	else if (!strcmp(registry,AUTH_DCE))
		lookup_domain = 3;
	else lookup_domain = 1;  /* caller should always set registry */
	
	setpwent();
	set_getpwent_remote(lookup_domain);
	if (!(tpwd = getpwnam(shortname)) || !(*pwd = _pwddup(tpwd)))
	{
		set_getpwent_remote(1);	/* resume full lookups */
		tmp_errno = errno;
		endpwent();
		errno = tmp_errno;
		return(-1);
	}
	set_getpwent_remote(1);	/* resume full lookups */
	endpwent();

	/*
	 * If the user doesn't have an entry in /etc/security/passwd,
	 * then dummy up one.
	 */
	setpwdb(S_READ);
	if (!(tpw = getuserpw(shortname)))
	{
		tmp_errno = errno;
		endpwdb();
		if (tmp_errno != ENOATTR && tmp_errno != ENOENT)
		{
			_pwdfree(*pwd);
			*pwd = (Passwd *) NULL;
			errno = tmp_errno;
			return(-1);
		}
		if (!(*pw = (UserPW *) malloc(sizeof(UserPW))))
		{
			tmp_errno = errno;
			_pwdfree(*pwd);
			*pwd = (Passwd *) NULL;
			errno = tmp_errno;
			return(-1);
		}

		(*pw)->upw_passwd = (char *) NULL;

		if ((*pwd)->pw_passwd &&
			!((*pw)->upw_passwd = strdup((*pwd)->pw_passwd)))
		{
			tmp_errno = errno;
			free((void *) *pw);
			_pwdfree(*pwd);
			*pw  = (UserPW *) NULL;
			*pwd = (Passwd *) NULL;
			errno = tmp_errno;
			return(-1);
		}
		strcpy((*pw)->upw_name, shortname);
		(*pw)->upw_lastupdate = 0L;
		(*pw)->upw_flags      = 0;
		return(0);
	}

	*pw = _userpwdup(tpw);
	tmp_errno = errno;
	endpwdb();

	if (!(*pw))
	{
		_pwdfree(*pwd);
		*pwd = (Passwd *) NULL;
		errno = tmp_errno;
		return(-1);
	}

	/*
	 * Make sure that '*pw' contains the same password as '*pwd'.
	 * '*pwd' takes precedence since login uses getpwnam() and
	 * getpwnam() checks /etc/passwd first.
	 * It is possible for the encrypted passwords from getpwnam() and
	 * getuserpw() to be different (if someone hand edited /etc/passwd
	 * to contain an encrypted password and didn't remove the encrypted
	 * password from /etc/security/passwd, for instance).
	 */
	if (!((*pwd)->pw_passwd))
	{
		if ((*pw)->upw_passwd)
		{
			free((void *) (*pw)->upw_passwd);
			(*pw)->upw_passwd = (char *) NULL;
		}
		return(0);
	}
	if ((*pw)->upw_passwd)
	{
		if (!strcmp((*pwd)->pw_passwd, (*pw)->upw_passwd))
			return(0);
		free((void *) (*pw)->upw_passwd);
	}
	if (!((*pw)->upw_passwd = strdup((*pwd)->pw_passwd)))
	{
		tmp_errno = errno;
		free((void *) *pw);
		_pwdfree(*pwd);
		*pw  = (UserPW *) NULL;
		*pwd = (Passwd *) NULL;
		errno = tmp_errno;
		return(-1);
	}
	
	return(0);
}


/*
 * NAME:     _userpwdup
 *
 * FUNCTION: Duplicates a UserPW structure into malloc'ed memory.
 *
 * RETURNS:  (in order of precedence)
 *            NULL - Couldn't duplicate the structure.
 *           !NULL - Pointer to duplicate structure.
 */
UserPW	*
_userpwdup(UserPW *pw)
{
	UserPW	*newpw;
	int	tmp_errno;

	if (!(newpw = (UserPW *) malloc(sizeof(UserPW))))
		return((UserPW *) NULL);

	memcpy(newpw, pw, sizeof(UserPW));

	if (pw->upw_passwd && !(newpw->upw_passwd = strdup(pw->upw_passwd)))
	{
		tmp_errno = errno;
		free((void *) newpw);
		errno = tmp_errno;
		return((UserPW *) NULL);
	}
	return(newpw);
}


/*
 * NAME:     _userpwfree
 *
 * FUNCTION: Deallocates a malloc'ed UserPW structure.
 *
 * RETURNS:  NONE
 */
void
_userpwfree(UserPW *pw)
{
	if (pw)
	{
		if (pw->upw_passwd)
			free((void *) pw->upw_passwd);
		free((void *) pw);
	}
}
