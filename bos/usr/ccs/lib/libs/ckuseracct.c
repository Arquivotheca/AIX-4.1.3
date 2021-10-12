static char sccsid[] = "@(#)86	1.14.1.5  src/bos/usr/ccs/lib/libs/ckuseracct.c, libs, bos411, 9435C411a 9/1/94 14:54:29";
/*
 * COMPONENT_NAME: (LIBS) Security Library Functions 
 *
 * FUNCTIONS: ckuseracct, OKdaemon, OKrlogin
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/id.h>
#include <time.h>
#include <usersec.h>
#include <userconf.h>
#include <login.h>

extern	int	OKtosu(char *);
extern	int	OKtty(char *, char *);
extern	int	checklmode(int);
extern	int	chkexpires(char *);

static	int	OKdaemon (char *destuser);
static	int	OKrlogin (char *destuser);

/*
 * NAME: ckuseracct
 *                                                                    
 * FUNCTION: check the validity of the user's account 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * 	This is a library routine. It checks to see:
 *		- if the user account exists
 *		- if the user account has expired
 *		- if logins are permitted (if mode = S_LOGIN)
 *		- if current process can 'su' to this account (if mode = S_SU)
 *		- if this account can be used from the specified tty
 *
 * RETURNS: 
 *
 *	If status code equals 0 then success.
 *	If status code greater than 0 then an error occured.
 *	On an error 'errno' is set and returned as follows:
 *		EINVAL  - the user account does not exist
 *		ENXIO   - the user account expired 
 *		ENOENT  - user account is disabled
 *		EACCES  - permission denied: 
 *		          mode is S_LOGIN and no logins are allowed
 *		          mode is S_SU and caller not member of SU list of user
 *		EAGAIN  - maximum number of login attempts have been exceeded
 *		EBADF   - the requested usage is not permitted on this tty
 */ 
int
ckuseracct (char *user,int mode,char *tty)
{
int	err;		/* error returned by getuserattr */
char	*expiration;	/* return from S_EXPIRATION */
uid_t	uid;

	/* check if input mode is a valid flag */
	if (checklmode(mode))
	{
		errno = EINVAL;
		return (-1);
	}

	/* varify account existance */
	if (getuserattr (user, S_ID, (void *) &uid, SEC_INT))
		return (-1);

	/* if there is a specified expiration date check it */
	err = getuserattr (user, S_EXPIRATION, (void *) &expiration, SEC_CHAR);
	if (!err && expiration)
	{
		/* expiration MMDDHHMMYY must be converted to unix time */
		if (chkexpires(expiration))
		{
			errno = ESTALE;
			return (-1);
		}
	}
	/* is usage permitted? */
	if (mode & S_LOGIN)
	{
	int	login;

		err = getuserattr (user, S_LOGINCHK, (void *) &login, SEC_BOOL);
		if (!err && !login)
		{
			errno =  EACCES;
			return (-1);
		}
	}
	/* is 'su' permitted? */
	if ((mode & S_SU) && !OKtosu (user))
	{
		errno =  EACCES;
		return (-1);
	}
	/* If tty is given see if it can be used */
	if (tty && *tty && !OKtty (user, tty))
	{
		errno =  EACCES;
		return (-1);
	}
	/* is 'daemon' permitted? */
	if ((mode & S_DAEMON) && !OKdaemon (user))
	{
		errno =  EACCES;
		return (-1);
	}
	/* is 'login' permitted? */
	if ((mode & S_RLOGIN) && !OKrlogin (user))
	{
		errno =  EACCES;
		return (-1);
	}

	return (0);
}

/*
 * NAME: OKdaemon
 *                                                                    
 * FUNCTION: checks to see if the caller can invoke daemon or batch
 *		programs via the src or cron subsystems.
 *                                                                    
 * EXECUTION ENVIRONMENT: static 
 *                                                                   
 * RETURNS: 1 if ok and 0 if not
 */  
static	int
OKdaemon (char *destuser)
{
int	err;
int	daemon;

	/* if no attribute or daemon = true return ok */

	err = getuserattr (destuser,S_DAEMONCHK,(void *) &daemon,SEC_BOOL);
	if (err || daemon)
		return(1);

	return(0);
}

/*
 * NAME: OKrlogin
 *                                                                    
 * FUNCTION: checks to see if the account can be used for remote logins
 *		via the rlogin or telnetd programs
 *                                                                    
 * EXECUTION ENVIRONMENT: static 
 *                                                                   
 * RETURNS: 1 if ok and 0 if not
 */  
static	int
OKrlogin (char *destuser)
{
int	err;
int	rlogin;

	/* if no attribute or rlogin = true return ok */

	err = getuserattr (destuser,S_RLOGINCHK,(void *) &rlogin,SEC_BOOL);
	if (err || rlogin)
		return(1);

	return(0);
}
