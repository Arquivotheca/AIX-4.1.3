static char sccsid[] = "@(#)89	1.6  src/bos/usr/ccs/lib/libs/loginlog.c, libs, bos411, 9428A410j 4/3/94 19:20:50";
/*
 * COMPONENT_NAME: (LIBS) Security Library Functions 
 *
 * FUNCTIONS: loginsuccess, _lockedport, _portlocked, loginfailed
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <sys/errno.h>
#include <ctype.h>
#include <usersec.h>
#include <pwd.h>
#include <fcntl.h>
#include <userpw.h>
#include <sys/audit.h>
#include <utmp.h>
#include <syslog.h>
#include "libs.h"


/*
 * NAME: loginsuccess
 *
 * FUNCTION: records a successful login
 *
 * EXECUTION ENVIRONMENT:
 *	library routine
 *
 * RETURNS:
 *	zero for success, -1 for failure
 */
int
loginsuccess (char *username, char *hostname, char *tty, char **msg)
{
	int err = 0, audit_rc = 0;
	long ucount, stime, utime;
	uid_t uid;
	char *shost, *uhost, *stty, *utty;
	char localhost[MAXHOSTNAMELEN + 1], str[256];
	char line1[256], line2[256], line3[256];

	if (setuserdb(S_READ | S_WRITE)) {
		errno = EACCES;
		return(-1);
	}

        if (getuserattr(username, S_ID, (void *)&uid, SEC_INT))
        {
		enduserdb();
                errno = ENOENT;
                return(-1);
        }

	/*
	 * Fetch the old lastlog information for the message to be constructed.
	 */
	if(getuserattr(username, S_ULOGCNT, &ucount, SEC_INT) != 0)
		ucount = 0;
	if(getuserattr(username, S_LASTTIME, &stime, SEC_INT) != 0)
		stime = 0;
	if(getuserattr(username, S_LASTTTY, &stty, SEC_CHAR) != 0)
		stty = "";
	if(getuserattr(username, S_LASTHOST, &shost, SEC_CHAR) != 0)
		shost = "";
	if(getuserattr(username, S_ULASTTIME, &utime, SEC_INT) != 0)
		utime = 0;
	if(getuserattr(username, S_ULASTTTY, &utty, SEC_CHAR) != 0)
		utty = "";
	if(getuserattr(username, S_ULASTHOST, &uhost, SEC_CHAR) != 0)
		uhost = "";

	/*
	 * Get the local hostname.
	 */
	gethostname (localhost, MAXHOSTNAMELEN);

	/*
	 * Handle number of unsuccessful login attempts.
	 */
	if(ucount == 1)
		sprintf(line1, MSGSTR(M_FCOUNT, DEF_FCOUNT));
	else if(ucount > 1)
		sprintf(line1, MSGSTR(M_FCOUNTS, DEF_FCOUNTS), ucount);
	else
		line1[0] = '\0';

	/*
	 * Handle last unsuccessful login attempt.
	 */
	if(utime != 0)
	{
		/*
		 * Convert the unsuccessful login time to human readable form.
		 */
		strftime(str, 256, "%c", localtime(&utime));

		if(strcmp(localhost, uhost))
			sprintf(line2, MSGSTR(M_FLOGINHT, DEF_FLOGINHT), str,
				utty, uhost);
		else
			sprintf(line2, MSGSTR(M_FLOGIN, DEF_FLOGIN), str, utty);
	}
	else
		line2[0] = '\0';

	/*
	 * Handle last successful login attempt.
	 */
	if(stime != 0)
	{
		/*
		 * Convert the unsuccessful login time to human readable form.
		 */
		strftime(str, 256, "%c", localtime(&stime));

		if(strcmp(localhost, shost))
			sprintf(line3, MSGSTR(M_SLOGINHT, DEF_SLOGINHT), str,
				stty, shost);
		else
			sprintf(line3, MSGSTR(M_SLOGIN, DEF_SLOGIN), str, stty);
	}
	else
		line3[0] = '\0';

	/*
	 * Build the string to return to the user.
	 */
	*msg = (char *)malloc(strlen(line1) + strlen(line2) +
			      strlen(line3) + 1);
	if(*msg)
	{
		strcpy(*msg, line1);
		strcat(*msg, line2);
		strcat(*msg, line3);
	}

	/*
	 * Write out the tty, host, and time of this login, and zero the
	 * unsuccessful login count.
	 */
	if(putuserattr(username, S_LASTTIME, time(0), SEC_INT) != 0)
		err++;
	if(putuserattr(username, S_LASTTTY, tty, SEC_CHAR) != 0)
		err++;
	if (hostname == NULL) {
		if(putuserattr(username, S_LASTHOST, localhost, SEC_CHAR) != 0)
			err++;
	}
	else {
		if(putuserattr(username, S_LASTHOST, hostname, SEC_CHAR) != 0)
			err++;
	}
	if(putuserattr(username, S_ULOGCNT, 0, SEC_INT) != 0)
		err++;

	/*
	 * Commit the changes.
	 */
	if(putuserattr(username, NULL, NULL, SEC_COMMIT) != 0)
		err++;

	enduserdb();

	/*
	 * Write an audit record.
	 */
	if (auditwrite("USER_Login", AUDIT_OK, username, strlen(username) + 1, 
			NULL)) {
		if (errno != EINVAL) audit_rc = errno;
	}

	/*
	 * If err is zero and audit_rc is zero, then return zero (everything 
	 * was successful); otherwise, return -1 and appropriate errno.
	 */
	if (err != 0) {
		errno = EACCES;
		return(-1);
	}
	else if (audit_rc != 0) {
		errno = EPERM;
		return(-1);
	}
	else return(0);
}

/*
 * NAME:     _lockedport
 *
 * FUNCTION: Determine if the given port is locked.
 *
 * EXECUTION ENVIRONMENT:
 *	     library
 *
 * RETURNS:  True if the port is locked, false otherwise.
 */
static	int
_lockedport (char *tty, time_t logtime)
{
	long interval, locktime;
	int rc;

	setuserdb(S_READ);
	if(getportattr(tty, S_LOCKTIME, &locktime, SEC_INT))
                locktime = ((errno == ENOENT) || (errno == ENOATTR)) ? 0 : -1;
	enduserdb();

	if(locktime == 0)
		return(FALSE);
	else if(locktime == -1)
		return(TRUE);
	else
	{
		/*
		 * The port is locked; see if it is time to reenable the port.
		 */
		setuserdb(S_READ);
		if(getportattr(tty, S_LOGREENABLE, &interval, SEC_INT))
			interval = 0;
		enduserdb();

		if(interval != 0)
		{
			if(locktime > (logtime - interval * 60))
			{
				return(TRUE);
			}
			else
			{
				/*
				 * It's time to reenable the port, so unlock it.
				 */
				setuserdb(S_READ | S_WRITE);
				putportattr(tty, S_LOCKTIME, 0, SEC_INT);
				putportattr(tty, NULL, NULL, SEC_COMMIT);
				enduserdb();
				return(FALSE);
			}
		}
		else
		{
			return(TRUE);
		}
	}
}

/*
 * NAME:     _portlocked
 *
 * FUNCTION: Determine if any of the given ports are locked.
 *
 * EXECUTION ENVIRONMENT:
 *	     library
 *
 * RETURNS:  True if one or more of the ports are locked, false otherwise.
 */
int
_portlocked (char *ttylist, time_t logtime)
{
	char	tty[PATH_MAX];
	int	i;

	while (*ttylist)
	{
		for (i=0; *ttylist; i++)
			if ((tty[i] = *ttylist++) == ',')
				break;
		tty[i] = '\0';
		if (_lockedport(tty, logtime))
			return(TRUE);
	}
	return(FALSE);
}

/*
 * NAME: loginfailed
 *
 * FUNCTION: records a failed login
 *
 * EXECUTION ENVIRONMENT:
 *	library
 *
 * RETURNS:
 *	zero for success, -1 for failure
 */
int
loginfailed (char *username, char *hostname, char *tty)
{
	static int logfailcount = 0;
	int fd, count, max, interval, err = 0, audit_rc = 0;
	time_t logtime = time(0);
	struct utmp ut;
	char badlogtimes[BUFSIZ], *ptr1, *ptr2;
	char localhost[MAXHOSTNAMELEN + 1];

	/*
	 * Free the user license (if one was obtained).
	 */
	_ReleaseLicense();

	setuserdb(S_READ | S_WRITE);

	/*
	 * Get the local hostname.
	 */
	gethostname (localhost, MAXHOSTNAMELEN);

	/*
	 * Is this a valid username?
	 */
	if(getuserattr(username, S_ID, &count, SEC_INT) == 0)
	{
		/*
		 * Get the unsuccessful login count for this user.
		 */
		if(getuserattr(username, S_ULOGCNT, &count, SEC_INT) != 0)
			count = 0;

		/*
		 * Increment the unsuccessful login count.
		 */
		count++;

		/*
		 * Write out the tty, host, and time of this unsuccessful login
		 * attempt, and the updates unsuccessful login count.
		 */
		if(putuserattr(username, S_ULASTTIME, logtime, SEC_INT) != 0)
			err++;
		if(putuserattr(username, S_ULASTTTY, tty, SEC_CHAR) != 0)
			err++;
		if (hostname == NULL) {
		  if(putuserattr(username,S_ULASTHOST, localhost,SEC_CHAR) != 0)
		    err++;
		}
		else {
		  if(putuserattr(username,S_ULASTHOST, hostname,SEC_CHAR) != 0)
			err++;
		}
		if(putuserattr(username, S_ULOGCNT, count, SEC_INT) != 0)
			err++;

		/*
		 * Commit the changes.
		 */
		if(putuserattr(username, NULL, NULL, SEC_COMMIT) != 0)
			err++;
	}
	else
	{
		/*
		 * Use "UNKNOWN_USER" as the username so that an accidentally
		 * entered password doesn't show up as clear text.
		 */
		username = "UNKNOWN_USER";
	}

	/*
	 * Write an audit record.
	 */
	if (auditwrite("USER_Login", AUDIT_FAIL, username, strlen(username) + 1,
		   NULL)) {
		if (errno != EINVAL) audit_rc = errno;
	}

	/*
	 * Get the maximum number of unsuccessful login attempts allowed for
	 * this port (logindisable attribute).
	 */
	if(getportattr(tty, S_LOGDISABLE, &max, SEC_INT) != 0)
		max = 0;

	/*
	 * See if port disabling is enabled.
	 */
	if((max > 0) && !_portlocked(tty, logtime) &&
	   strncmp(tty, "/dev/pts/", 9))
	{
		/*
		 * Get the list of unsuccessful login times.
		 */
		if(getportattr(tty, S_ULOGTIMES, &ptr1, SEC_LIST) != 0)
			ptr1 = NULL;

		/*
		 * Add the current failure to the end of the list of failed
		 * login attempts.
		 */
		ptr2 = badlogtimes;
		while(ptr1 && (*ptr1 != '\0'))
		{
			strcpy(ptr2, ptr1);
			ptr1 += strlen(ptr1) + 1;
			ptr2 += strlen(ptr2) + 1;
		}
		sprintf(ptr2, "%ld", logtime);
		ptr2[strlen(ptr2) + 1] = '\0';

		/*
		 * Get the time interval over which to consider failed logins.
		 */
		if((getportattr(tty, S_LOGINTERVAL, &interval, SEC_INT) != 0) ||
		   (interval < 0))
			interval = 0;
		interval = logtime - interval;

		/*
		 * Remove all failed logins which are older than our time
		 * interval.  The list of times is ordered oldest to newest
		 * from left to right, so we start removing from the beginning
		 * of the list and stop once a failed login within the time
		 * interval is found (the remaining times are newer and
		 * therefore within the time interval).
		 */
		ptr1 = badlogtimes;
		while(*ptr1 != '\0')
		{
			if(atol(ptr1) < interval)
				ptr1 += strlen(ptr1) + 1;
			else
				break;
		}
		ptr2 = ptr1;

		/*
		 * Determine how many failed login attempts there have been in
		 * the requested time interval.
		 */
		count = 0;
		while(*ptr1 != '\0')
		{
			ptr1 += strlen(ptr1) + 1;
			count++;
		}

		/*
		 * Remove any remaining failed login attempts above and beyond
		 * the maximum allowable.
		 */
		while(count > max)
		{
			ptr2 += strlen(ptr2) + 1;
			count--;
		}

		/*
		 * Save the updated list of failed login times.
		 */
		if(putportattr(tty, S_ULOGTIMES, ptr2, SEC_LIST) != 0)
			err++;

		/*
		 * See if there were too many failed login attempts.
		 */
		if(count == max)
		{
			if(putportattr(tty, S_LOCKTIME, logtime, SEC_INT) != 0)
				err++;
			if (auditwrite("PORT_Locked", 0, tty, strlen(tty), 0)) {
				if (errno != EINVAL) audit_rc = errno;
			}
		}

		/*
		 * Commit the changes.
		 */
		if(putportattr(tty, NULL, NULL, SEC_COMMIT) != 0)
			err++;
	}

	/*
	 * If the login retry delay is enabled, then sleep.
	 */
	if((getportattr(tty, S_LOGDELAY, &count, SEC_INT) == 0) &&
	   (count > 0))
	{
		logfailcount++;
		sleep(logfailcount * count);
	}

	enduserdb();

	/*
	 * Fill out the various fields of the utmp structure.
	 */
	bzero(&ut, sizeof(ut));
	strncpy(ut.ut_user, username, sizeof(ut.ut_user));
	if(!strncmp(tty, "/dev/", 5))
		tty += 5;
	strncpy(ut.ut_line, tty, sizeof(ut.ut_line));
	strncpy(ut.ut_host, hostname, sizeof(ut.ut_host));
	ut.ut_type = USER_PROCESS;
	ut.ut_pid = getpid();
	ut.ut_time = logtime;

	/* syslog the failed login attempt */
	if (hostname)
		syslog(LOG_INFO, "%s: failed login attempt for %s from %s\n", 
			tty, username, hostname);
	else
		syslog(LOG_INFO, "%s: failed login attempt for %s\n", 
			tty, username);
		
	/*
	 * Create the /etc/security/failedlogin file if it does not exist.
	 */
	if(fd = open(FAILEDLOG_FILENAME, O_CREAT|O_RDWR, 0644))
		close(fd);

	/*
	 * Append the utmp entry to the end of /etc/security/failedlogin.
	 */
	if(append_wtmp(FAILEDLOG_FILENAME, &ut) != 0)
		err++;

	/*
	 * If err is zero, then return zero (everything was successful);
	 * otherwise, return -1.
	 */
	if (err != 0) {
		errno = EACCES;
		return(-1);
	}
	else if (audit_rc != 0) {
		errno = EPERM;
		return(-1);
	}
	else return(0);
}
