static char sccsid[] = "@(#)90	1.9  src/bos/usr/ccs/lib/libs/loginrestrictions.c, libs, bos41J, 9515A_all 3/29/95 16:20:41";
/*
 * COMPONENT_NAME: (LIBS) Security Library Functions 
 *
 * FUNCTIONS: loginrestrictions, OKtosu, OKtty, OKrcmd, checklmode,
 *	      chkexpires, _chklogintimes
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/id.h>
#include <sys/sem.h>
#include <sys/limits.h>
#include <sys/stat.h>
#include <time.h>
#include <usersec.h>
#include <userpw.h>
#include <userconf.h>
#include <utmp.h>
#include <values.h>
#include <login.h>
#include <fcntl.h>
#include <string.h>
#include "libs.h"

	int	OKtosu (char *destuser);
	int	OKtty (char *user,char *tty);
	int	OKrcmd (char *user, char *cmd);
	int	checklmode(int mode);
	int	chkexpires(char *exp);
static	int	_chklogintimes(char *ttylist, char *user, uid_t uid);

/*
 * NAME: loginrestrictions
 *                                                                    
 * FUNCTION: Determines if the user can login
 *                                                                    
 * EXECUTION ENVIRONMENT:
 * 	This is a library routine. It returns a message in the msg parameter
 *	which is a pointer to malloc()ed memory.  It is the responsibility
 *	of the caller to free() this memory.
 *
 * NOTES:    This code assumes that the first port in 'ttylist' is the
 *           port name that will be added to the utmp file.
 *
 * RETURNS: 
 *	If status code equals 0 then success.
 *	If status code greater than 0 then an error occured.
 */ 
int
loginrestrictions (char *user,int mode,char *ttylist, char **msg)
{
	int	err, locked, now = time(0), count, max, fd, invalid_attr;
	char	*expiration, *logtimes;
	char	shortname[PW_NAMELEN];
	struct stat stat_buf;
	uid_t	uid;

	*msg = NULL;

	/*
	 * Check to see if the mode is valid.
	 */
	if (checklmode(mode))
	{
		errno = EINVAL;
		return (-1);
	}

	/*
	 * Check to see that the account exists.
	 */
	if (getuserattr (user, S_ID, (void *) &uid, SEC_INT))
	{
		*msg = strdup(MSGSTR(M_BADLOGIN, DEF_BADLOGIN));
		errno = ENOENT;
		return (-1);
	}

	_normalize_username(shortname, user);

	/*
	 * Check to see if the account is locked.
	 */
	if(getuserattr(shortname, S_LOCKED, &locked, SEC_BOOL))
	{
		/*
		 * If getuserattr() failed because of ENOENT or ENOATTR, then
		 * we assume the account is not locked.  If it fails for any
		 * other reason (i.e. corrupted database), we assume the
		 * account is locked (with a root override on the failure case
		 * only).
		 */
		if((errno == ENOENT) || (errno == ENOATTR))
			locked = FALSE;
		else if(uid)
			locked = TRUE;
		else
			locked = FALSE;
	}

	if(locked)
	{
		*msg = strdup(MSGSTR(M_USERLOCKED, DEF_USERLOCKED));
		errno = EPERM;
		return (-1);
	}

	/*
	 * Check to see if the account has expired.
	 */
	err = getuserattr (shortname, S_EXPIRATION, (void *) &expiration, SEC_CHAR);
	if (!err && expiration)
	{
		/*
		 * Expiration MMDDHHMMYY must be converted to unix time.
		 */
		if (chkexpires(expiration))
		{
			*msg = strdup(MSGSTR(M_EXPIRED, DEF_EXPIRED));
			errno = ESTALE;
			return (-1);
		}
	}

	/*
	 * Check to see if the user can access the system at this time
	 * (logtimes attribute for the user).
	 */
	if (!getuserattr(shortname, S_LOGTIMES, &logtimes, SEC_LIST))
	{
		err = _checktime(logtimes);
		if (err == -1)
		{
			char msg[256];

			sprintf(msg, MSGSTR(M_BADLOGTIMES, DEF_BADLOGTIMES),
				user, logtimes);
			auditwrite("USER_Login", 1, msg, strlen(msg));
		}
		if ((err == 1) || ((err == -1) && (uid != 0)))
		{
			*msg = strdup(MSGSTR(M_USERTIME, DEF_USERTIME));
			errno = EACCES;
			return (-1);
		}
	}

	/*
	 * Check to see if there have been too many unsuccessful login attempts
	 * by this user.
	 */
	invalid_attr = 0;/* distinguish between loginretries not specified, and
			  * loginretries specified but not valid.  The former
			  * allows the user to login, the latter does not.
			  */
	if (getuserattr (shortname, S_LOGRETRIES, (void *) &max, SEC_INT)) {
		if ((errno == ENOENT) || (errno == ENOATTR)) 
			max = 0;
		else
			invalid_attr = 1;
	}
	if (getuserattr (shortname, S_ULOGCNT, (void *) &count, SEC_INT))
		count = 0;
	if ( (uid) && ( (invalid_attr) || ((max > 0) && (count >= max)) ))
	{
		*msg = strdup(MSGSTR(M_TOOMANYBAD, DEF_TOOMANYBAD));
		errno = EPERM;
		return (-1);
	}

	/*
	 * Check the requested access mode.
	 */
	if (mode & S_LOGIN)
	{
		int	login;

		err = getuserattr (shortname, S_LOGINCHK, (void *) &login, SEC_BOOL);
		if (!err && !login)
		{
			*msg = strdup(MSGSTR(M_NOLOCAL, DEF_NOLOCAL));
			errno =  EACCES;
			return (-1);
		}
	}

	if (mode & S_RLOGIN)
	{
		int	rlogin;

		err = getuserattr (shortname, S_RLOGINCHK, (void *) &rlogin,
				   SEC_BOOL);
		if (!err && !rlogin)
		{
			*msg = strdup(MSGSTR(M_NOREMOTE, DEF_NOREMOTE));
			errno =  EACCES;
			return (-1);
		}
	}

	if ((mode & S_SU) && !OKtosu (shortname))
	{
		*msg = strdup(MSGSTR(M_NOSU, DEF_NOSU));
		errno =  EACCES;
		return (-1);
	}

	if (mode & S_DAEMON)
	{
		int	daemon;

		err = getuserattr (shortname, S_DAEMONCHK, (void *) &daemon,
				   SEC_BOOL);
		if (!err && !daemon)
		{
			*msg = strdup(MSGSTR(M_NODAEMON, DEF_NODAEMON));
			errno =  EACCES;
			return (-1);
		}
	}

	/*
	 * Perform tty checks if a tty name was given.
	 */
	if (ttylist && *ttylist)
	{
		char	tty[PATH_MAX];
		int	i;

		/*
		 * Check to see if the user is allowed to use these ttys.
		 */
		if (ttylist[0] != 'R' && !OKtty(shortname, ttylist))
		{
			*msg = strdup(MSGSTR(M_NOTTYS, DEF_NOTTYS));
			errno =  EACCES;
			return (-1);
		}

		/*
		 * Check to see if the user is allowed to use this network
		 * access mechanism.
		 */
		if (ttylist[0] == 'R' && !OKrcmd(shortname, ttylist))
		{
			*msg = strdup(MSGSTR(M_NOREMOTE, DEF_NOREMOTE));
			errno =  EACCES;
			return (-1);
		}

		/*
		 * Check to see if these ttys have been disabled (only performed
		 * if the user is not root).
		 */
		if (uid && _portlocked(ttylist, now))
		{
			*msg = strdup(MSGSTR(M_TTYLOCKED, DEF_TTYLOCKED));
			errno = EPERM;
			return (-1);
		}

		/*
		 * Check to see if the user can access these ports at
		 * this time (check logintimes attribute for the ports).
		 */
		if (_chklogintimes(ttylist, shortname, uid))
		{
			*msg  = strdup(MSGSTR(M_TTYTIME, DEF_TTYTIME));
			errno = EACCES;
			return (-1);
		}

		/*
		 * If the mode is S_LOGIN or S_RLOGIN then perform the
		 * licensing check.  (Assume the first port in the list
		 * is the same port used in the utmp file.)
		 */
		for (i=0; ttylist[i] && ttylist[i] != ','; i++)
			tty[i] = ttylist[i];
		tty[i] = '\0';

		if (((mode & S_LOGIN) || (mode & S_RLOGIN)) &&
		    !_GetLicense(tty, shortname))
		{
			*msg = strdup(MSGSTR(M_LICENSE, DEF_LICENSE));
			errno = EAGAIN;
			return (-1);
		}
	}

	/*
	 * If we are not root, then check for the existence of /etc/nologin.
	 * If it exists, then we return it to the user and disallow logins.
	 */
	if(uid != 0)
	{
		if(stat("/etc/nologin", &stat_buf) == 0)
		{
			*msg = malloc(stat_buf.st_size + 1);
			if(*msg)
			{
				if(fd = open("/etc/nologin", O_RDONLY))
				{
					read(fd, *msg, stat_buf.st_size);
					(*msg)[stat_buf.st_size] = '\0';
					close(fd);
				}
				else
				{
					free(*msg);
					*msg = NULL;
				}
			}
			errno = EPERM;
			return(-1);
		}
	}

	return (0);
}

/*
 * NAME: OKtosu
 *                                                                    
 * FUNCTION: checks to see if the caller can su to the specified user
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *                                                                   
 * RETURNS: 1 if successful and 0 if fail
 */  
int
OKtosu(char *destuser)
{
	gid_t	gidset[S_NGROUPS];	/* process's concurrent group set */
	int	ngroups;		/* number of groups */
	char	*sugroups;		/* groups allowed to su to this user */
	int	err;			/* return from getuserattr */
	int	suchk;			/* whether we can su or not */
	int	rc = 0;			/* return code (default = no-su) */
	int	i;			/* index for gidset */
	char	*gname;			/* group name */
	char	*su;			/* ptr that runs through 'sugroups' */
	
	/* see if su=true (no privilege over-ride allowed) */
	err = getuserattr (destuser, S_SUCHK, (void *) &suchk, SEC_BOOL);
	if (!err && !suchk)
	{
		errno =  EACCES;
		return 0;
	}

	/* if invoker is privileged then skip the rest... */
	if (!getuid())
		return 1;

	/* get the destination user's sugroups */
	err = getuserattr(destuser, S_SUGROUPS, (void *)&sugroups, SEC_LIST);
	if (err || !sugroups || !*sugroups)
		return 1;	/* attribute not found so default OK */

	/* see if one of callers groups is in sugroups of destination user */

	/* first check process's primary group */
	gname = IDtogroup(getgidx(ID_EFFECTIVE));
	su = sugroups;
	while (*su)
	{
		if ((strcmp(su, "*") == 0) ||
		    (strcmp(su, "ALL") == 0))
			rc = 1;
		else
		{
			/* allow for NOT operator */
			if (*su == '!')
			{
				su++;
				if (strcmp(su, gname) == 0)
					return 0;	/* game's over */
			}
			else
				if (strcmp(su, gname) == 0)
					rc = 1;
		}
		while (*su++);
	}

	/* now go through the process's concurrent group set */
	ngroups = getgroups(S_NGROUPS, &gidset[0]);
	for (i=0; i<ngroups; i++)
	{
		gname = IDtogroup(gidset[i]);
		su = sugroups;
		while (*su)
		{
			/* allow for NOT operator */
			if (*su == '!')
			{
				su++;
				if (strcmp(su, gname) == 0)
					return 0;	/* game's over */
			}
			else
				if (strcmp(su, gname) == 0)
					rc = 1;
			while (*su++);
		}
	}

	return rc;
}


/*
 * NAME: OKtty
 *                                                                    
 * FUNCTION: check to see if the requested usage is allowed on this terminal
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 *	S_TTYS is a comma separated list of terminal names or one of:
 *		'*' 	 which means all
 *		'ALL' 	 which also means all
 *                                                                   
 * RETURNS: 1 if ok and 0 if not ok
 */  
int
OKtty (char *user,char *tty)
{
char	*ttylist;
char 	*t;
char  	*s;
char	temptty[PATH_MAX]; 
char	*moveptr;
char	*beginptr;
int	rc = 0;
int	allrc = 0;
int	err = 0;
int 	ttylength = 0;


	err = getuserattr (user, S_TTYS, (void *) &ttylist, SEC_LIST);
	if (err || !ttylist || !*ttylist)
	{
		/* tty attribute not found: default OK */
		return (1);
	}

	beginptr = moveptr = tty;

	while (*moveptr) 
	{
		ttylength = 0;
		beginptr = moveptr;
		while (*moveptr && *moveptr++ != ',') 
			ttylength++;
	
		strncpy(temptty,beginptr,ttylength);
	 	*(temptty+ttylength) = '\0';

		t = ttylist;

		while (*t)
		{
			/* allow for NOT operator */
			if (*t == '!')
			{
				t++;
				rc = 0;
			}
			else
				rc = 1;

			if (!strcmp(t,temptty))
			{
				return(rc);
			}

			/* check for the logical name  */
			/* ie. /dev/tty3/0 = /dev/tty3 */
			/* assumes tty name always     */
			/* begins with "/dev/" and     */
			/* exactly three "/"           */
			
			for (s=temptty+5; *s && *s != '/'; s++)
				;
			
			if (*s) 
			{
				*s = '\0';
				if (!strcmp(t,temptty))
				{
					return(rc);
				}
			}

			if (!strcmp(t,"ALL"))
			{
				/* all ttys allowed */
				allrc = 1;
			}
			
			while (*t++)
				;
		}
		
	}
	return (allrc);
}

/*
 * NAME: OKrcmd
 *                                                                    
 * FUNCTION: check to see if the remote command method is permitted.
 *                                                                    
 * EXECUTION ENVIRONMENT: static 
 *
 *	S_TTYS is a comma separated list of terminal names or one of:
 *		'*' 	 which means all
 *		'ALL' 	 which also means all
 *		'RSH'    which means to allow "rsh" command
 *		'REXEC'  which means to allow "rexec" command
 *
 *	This routine only concerns itself with R-cmd strings.
 *                                                                   
 * RETURNS: 1 if ok and 0 if not ok
 */  
static int
OKrcmd (char *user,char *rshell)
{
	char	*ttylist;
	char	*entry;
	int	not = 0;
	int	result = 1;

	/*
	 * Get the TTYs attribute.  The remote commands are overloaded on
	 * this attribute.  The default is to allow access.
	 */

	if (getuserattr (user, S_TTYS, (void *) &ttylist, SEC_LIST))
		return 1;

	/*
	 * Check each attribute to see if it is the "rsh" or "rexec"
	 * cookie.  The usual rules for negation apply.
	 */

	for (entry = ttylist;entry && *entry;entry += strlen (entry) + 1) {
		if (not = (entry[0] == '!'))
			entry++;

		if (entry[0] == 'R' && (result = (strcmp (entry, rshell) == 0)))
			goto found;
	}

	/*
	 * I fell off the end of the list without finding the entry.  The
	 * default is to permit R-cmds, so return `OK'.
	 */

	return 1;

found:

	/* 
	 * I got here because I found the daemon in the access list.  Do
	 * the logic to see if access was permitted.
	 */

	return result ^ not;
}

/*
 * NAME: checklmode
 *                                                                    
 * FUNCTION: check to see if the proper mode was passed
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 * RETURNS: 1 if ok and 0 if not ok
 */  

int
checklmode(int mode)
{
	if (!mode)
		return(0);
	if (mode & S_LOGIN)
		return(0);
	if (mode & S_SU)
		return(0);
	if (mode & S_DAEMON)
		return(0);
	if (mode & S_RLOGIN)
		return(0);
	return(1);
}

/*
 * NAME: chkexpires
 *                                                                    
 * FUNCTION: check to see if the account has expired
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 * RETURNS: 0 if ok and 1 if not ok
 */  

int
chkexpires(char *exp)
{
struct tm *t;
int	month;
int	day;
int	hour;
int	min;
int	year = 0;
time_t	tim;
	
	/* if expires = "0" account doesn't expire */
	if (!strcmp(exp,"0"))
		return (0);

		/*   MM DD HH MM YY */
	sscanf(exp,"%2d%2d%2d%2d%2d", &month,&day,&hour,&min,&year);
		
	tim = time ((long *) 0);
	t = localtime(&tim);

	/*
	 * if expiration year is greater than current year ok
	 * if expiration year is less than current year fail
	*/
	if (year > t->tm_year)
		return (0);
	if (year < t->tm_year)
		return(1);

	/* 
	 * if expiration month is greater than current month ok
	 * if expiration month is less than current month fail
	 * time is returned 0-11 but date uses 1-12.
	*/
	month--;
	if (month > t->tm_mon)
		return (0);
	if (month < t->tm_mon)
		return (1);

	/* 
	 * if expiration day is greater than current day ok
	 * if expiration day is less than current day fail
	 */
	if (day > t->tm_mday)
		return (0);
	if (day < t->tm_mday)
		return (1);

	/* 
	 * if expiration hour is greater than current hour ok
	 * if expiration hour is less than current hour fail
	 */
	if (hour > t->tm_hour)
		return (0);
	if (hour < t->tm_hour)
		return (1);

	/*
	 * if expiration min is less than current min fail
	 */
	if (min <= t->tm_min)
		return (1);

	return(0);
}


/*
 * NAME:     _chklogintimes
 *
 * FUNCTION: Checks the port logintimes for a list of ports.
 *
 * RETURNS:   0 on success.
 *           -1 on failure.
 */
static	int
_chklogintimes(char *ttylist, char *user, uid_t uid)
{
	char	tty[PATH_MAX];
	char	*logtimes;
	int	err;
	int	i;

	/*
	 * For each tty in ttylist check the logintimes attribute.
	 */
	while (*ttylist)
	{
		for (i=0; *ttylist; i++)
			if ((tty[i] = *ttylist++) == ',')
				break;
		tty[i] = '\0';

		/*
		 * Check to see if the user can access this terminal
		 * at this time (logintimes attribute for the port).
		 */
		if (!getportattr(tty, S_LOGTIMES, &logtimes, SEC_LIST))
		{
			err = _checktime(logtimes);
			if (err == -1)
			{
				char	msg[256];

				sprintf(msg, MSGSTR(M_BADLOGTIMES,
				      DEF_BADLOGTIMES), user, logtimes);
				auditwrite("USER_Login", 1, msg, strlen(msg));
			}
			if ((err == 1) || ((err == -1) && (uid != 0)))
			{
				return (-1);
			}
		}
	}
	return(0);
}
