static char sccsid[] = "@(#)96	1.8.1.5  src/bos/usr/bin/logout/logout.c, cmdsauth, bos411, 9438A411a 9/7/94 16:35:24";
/*
 * COMPONENT_NAME: (CMDSAUTH) Command Authorization
 *
 * FUNCTIONS: logout.c
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <sys/termio.h>
#include <sys/signal.h>
#include <sys/audit.h>
#include <sys/id.h>
#include <sys/priv.h>
#include <limits.h>
#include <usersec.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <utmp.h>
#include <strings.h>
#include <locale.h>
#include "logout_msg.h"

char	*USER_Logout = "USER_Logout";

#ifndef	PENV_SYSTEM
#define	PENV_SYSTEM PENV_SYS
#endif

#define	DEF_GETPENV	"Error getting privileged environment.\n"
#define	DEF_TTYNAME	"Error getting terminal name.\n"
#define	DEF_TCQMGR	"Error getting SAK Manager Process ID.\n"
#define	DEF_LOGUSER	"You must be the login user.\n"
#define	DEF_LOGTTY	"You must be on the login terminal.\n"

#define	MSGSTR(Num, Str) logoutcatgets (Num, Str)
char	*logoutcatgets (int Num, char *Str);

#ifdef	_NO_PROTO
char	*ttyname();
char	*getuinfo();
struct	utmp	*getutline();
#else
char	*ttyname (int);
char	*getuinfo (char *);
struct	utmp	*getutline(struct utmp *);
#endif

/*
 * NAME:	main
 *
 * FUNCTION:	Perform logout processing on and off the trusted path.
 *
 * NOTES:
 *	This process performs auditing.
 *
 *	In all likelyhood this command will result in the called being logged
 *	out even if it fails since it is executed as a shell builtin.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User command.
 *
 * RETURN VALUES:
 *	Zero exit status on success, ESAD otherwise.
 */

main ()
{
	int	i;
	int	sak;
	int	sakmgr;
	int	fd;
	int	rc;
	int	uid;
	char	*tty = 0;
	char	*name = 0;
	char	*curtty = 0;
	char	**env;
	struct	utmp	*utent;
	struct	utmp	uttmp;
	sigset_t sigs;
	pid_t	login_pid;

	auditproc (0, AUDIT_STATUS, AUDIT_SUSPEND, 0);
	setlocale (LC_ALL, "");
	privilege (PRIV_LAPSE);

	/*
	 * Ignore a mess of signals to prevent being killed off by
	 * something or another.
	 */

	SIGFILLSET (sigs);
	sigblock (&sigs);

	/*
	 * Get the privileged environment.  The TTY name should have been saved
	 * away during login processing.
	 */

	if (! (tty = getuinfo ("TTY"))) {
		fputs (MSGSTR (M_GETPENV, DEF_GETPENV), stderr);
		tty = "???";
		goto failure;
	}

	/*
	 * Compare the real user ID against the login user ID.  They
	 * must match.  Also, the current tty must be the same as the
	 * login tty.
	 */

	if ((name = getuinfo ("NAME")) == (char *) 0) {
		fputs (MSGSTR (M_GETPENV, DEF_GETPENV), stderr);
		goto failure;
	}
	if (getuserattr (name, "id", (void *) &uid, 0) ||
			uid != getuidx (ID_REAL) || uid != getuidx (ID_LOGIN)) {
		fputs (MSGSTR (M_LOGUSER, DEF_LOGUSER), stderr);
		goto failure;
	}
	if ((curtty = ttyname (0)) == 0 || strcmp (curtty, tty) != 0) {
		fputs (MSGSTR (M_LOGTTY, DEF_LOGTTY), stderr);
		goto failure;
	}

	/*
	 * Get my login group process ID from the UTMP file.  This
	 * process group will be sent a SIGHUP before killing the
	 * processes.  This gives everyone a chance to clean up before
	 * being killed.
	 */

	memset (&uttmp, 0, sizeof uttmp);
	if (strncmp (tty, "/dev/", 5) == 0)
		strncpy (uttmp.ut_line, tty + 5, sizeof uttmp.ut_line);
	else
		strncpy (uttmp.ut_line, tty, sizeof uttmp.ut_line);

	/*
	 * Search for the utmp file entry for this line.  Skip over any
	 * entries that have invalid PIDs.  The process should still
	 * exist.
	 */

	for (login_pid = (pid_t) -1;;) {
		if ((utent = getutline (&uttmp)) &&
				utent->ut_type == USER_PROCESS &&
				! kill (utent->ut_pid, 0)) {
			login_pid = utent->ut_pid;
			break;
		} else if (! getutent ())
			break;
	}

	/*
	 * Open the TTY so I can get the process ID of the SAK manager.
	 * Also, find out if this is a multiplexed HFT - the entire HFT
	 * will be revoked later on if there is no SAK manager.
	 */

	if (tty == 0 || (fd = open (tty, O_RDONLY|O_NOCTTY|O_NDELAY)) < 0) {
		fputs (MSGSTR (M_TTYNAME, DEF_TTYNAME), stderr);
		goto failure;
	}

	/*
	 * If SAK processing is enabled and there is a SAK manager, then
	 * I can signal the SAK manager and have it perform the logout
	 * processing.
	 */

	if (ioctl (fd, TCQSAK, &sakmgr)) {
		fputs (MSGSTR (M_TCQMGR, DEF_TCQMGR), stderr);
		goto failure;
	}

	/*
	 * Close all open files to prevent being killed either by the
	 * SAK manager or by the revoke() call.  I want to exit on my
	 * own.
	 */

	kleenup (0, 0, 0);

	/*
	 * If SAK is turned on, AND there is a SAK manager, send the logout
	 * signal [ SIGMSG ] to the SAK manager and exit.  The SAK manager
	 * will kill off everything on this line.  The login process group
	 * will first be given a chance to cleanup by being sent a SIGHUP.
	 * Then the appropriate collection of kill()'s and revoke()'s is
	 * issued.
	 */

	if (sakmgr > 0) {
		privilege (PRIV_ACQUIRE);
		auditwrite (USER_Logout, AUDIT_OK, tty, strlen (tty) + 1, 0);
		privilege (PRIV_LAPSE);

		if (login_pid != (pid_t) -1) {
			signal (SIGHUP, SIG_IGN);
			kill (- login_pid, SIGHUP);
			sleep (3);
		}
		revoke (tty);
		privilege (PRIV_ACQUIRE);
		kill (sakmgr, SIGMSG);
		exit (0);
	}

	/*
	 * I have to kill my own login session because there is no SAK
	 * manager to do it for me.  To logout I revoke() access to the
	 * port, and then exit.  Since the user may currently be on an
	 * opened port, such as an HFT, revoke the current device name
	 * as well.  Any other ports not related to the login port or
	 * the current port will not be revoked, but their names can't
	 * be reliably found anyway.
	 */

	privilege (PRIV_ACQUIRE);
	auditwrite ("USER_Logout", AUDIT_OK, tty, strlen (tty) + 1, 0);
	privilege (PRIV_LAPSE);

	revoke (tty);

	if (curtty && strcmp (curtty, tty) != 0)
		revoke (curtty);

	if (login_pid != (pid_t) -1) {
		signal (SIGHUP, SIG_IGN);
		kill (- login_pid, SIGHUP);
		sleep (3);
	}

	exit (0);

	/*
	 * Common point for creating a failure audit record
	 */

failure:
	privilege (PRIV_ACQUIRE);
	auditwrite (USER_Logout, AUDIT_FAIL, tty, strlen (tty) + 1, 0);
	exit (ESAD);
}


/*
 * NAME: logoutcatgets
 *
 * FUNCTION: Get a string from the LOGOUT message catalog
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * NOTES:
 *	This routine returns a pointer to a message string.  If
 *	the catalog is unopened, an attempt is made to open it.
 *	If the open fails all future messages will be from the
 *	default built-in values.
 *
 * RETURNS: Pointer to message text or NULL on error.
 */

char *
#ifdef	_NO_PROTO
logoutcatgets (Num, Str)
int	Num;
char	*Str;
#else
logoutcatgets (int Num, char *Str)
#endif
{
	static	int	once;	/* counter for NLS initialization */
	static  nl_catd	catd;	/* catalogue file descriptor */

	if (! once) {
		catd = catopen (MF_LOGOUT, NL_CAT_LOCALE);
		once++;
	}
	if (catd != (nl_catd) -1)
		return catgets (catd, MS_LOGOUT, Num, Str);
	else
		return Str;
}
