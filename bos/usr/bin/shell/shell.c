static char sccsid[] = "@(#)97	1.10.1.4  src/bos/usr/bin/shell/shell.c, cmdsauth, bos411, 9428A410j 2/28/94 16:47:57";
/*
 * COMPONENT_NAME: (CMDSAUTH) Command Authorization
 *
 * FUNCTIONS: shell.c
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

#include <sys/types.h>
#include <sys/id.h>
#include <sys/termio.h>
#include <sys/audit.h>
#include <errno.h>
#include <usersec.h>
#include <stdio.h>
#include <locale.h>
#include "shell_msg.h"

char	*USER_Shell = "USER_Shell";	/* Audit Event name to use */

#define	DEF_GETUSER \
	"Cannot get user information for user name %s.\n"
#define	DEF_SETPCRED \
	"Cannot set credentials for %s.\n"
#define	DEF_SETPENV \
	"Cannot execute login shell for %s.\n"
#define	DEF_GETTPATH \
	"Cannot get trusted path information for user name %s.\n"
#define	DEF_NOTTY \
	"Cannot get terminal port name.\n"

#define	MSGSTR(Num, Str) shellcatgets (Num, Str)
char	*shellcatgets (int Num, char *Str);

main (argc, argv)
int	argc;
char	**argv;
{
	int	trust = TCUNTRUSTED;	/* Trusted state of current terminal */
	char	*tpath;			/* Trusted path of user              */
	char	*user;			/* Login name of user                */
	char	*tty;			/* TTY name of current process       */
	uid_t	userid;			/* Real UID of current process       */
	uid_t	u;			/* UID of login name                 */

	auditproc (0, AUDIT_STATUS, AUDIT_SUSPEND, 0);
	setlocale (LC_ALL, "");

	/*
	 * Get the current port name.  If you can't get a port, you
	 * can't run shell ...
	 */

	if (! (tty = ttyname (0))) {
		fprintf (stderr, MSGSTR (M_NOTTY, DEF_NOTTY));
		if (! (tty = getuinfo ("TTY")))
			tty = "???";

		auditwrite (USER_Shell, AUDIT_FAIL, tty, strlen (tty) + 1, 0);
		exit (errno);
	}

	/*
	 * Get the login name, and the real UID.  The goal is to
	 * try for a match between the real UID and the UID of
	 * the login name.  This is used to handle ambiguity when
	 * the same UID is shared by one or more login names.
	 */

	user = (char *) getlogin ();
	userid = getuidx (ID_REAL);

	if (user == 0 || getuserattr (user, "id", (void *) &u, 0)) {
		fprintf (stderr, MSGSTR (M_GETUSER, DEF_GETUSER), user);
		auditwrite (USER_Shell, AUDIT_FAIL, tty, strlen (tty) + 1, 0);
		exit (errno);
	}
	if (userid != u)
		user = IDtouser (userid);

	/*
	 * Get the user's trusted path information.  If "tpath" equals
	 * "always", the new login session will remain trusted.
	 */

	if (getuserattr (user, "tpath", (void *) &tpath, 0))
		tpath = "off";

	if (tpath && strcmp (tpath, "always") == 0)
		trust = TCTRUSTED;

	/*
	 * Some user name has been determined.  The initial
	 * credentials values for that user are set.
	 */

	if (setpcred (user, (char **) NULL)) {
		fprintf (stderr, MSGSTR (M_SETPCRED, DEF_SETPCRED), user);
		auditwrite (USER_Shell, AUDIT_FAIL, tty, strlen (tty) + 1, 0);
		exit (errno);
	}

	/*
	 * Close all of the files and perform an frevoke() on this device.
	 * Then dup() the appropriate file descriptors for the new login
	 * session.
	 */

	enduserdb ();

	/* in order to become a session leader for the controlling
	 * terminal, we must relinquish ourselves as a process group
	 * leader. To do this, we simply become a part of the
	 * parent's process group. Then we can become a session leader.
	 * Note: the ksh makes its child it's own process group
	 * leader. That is why we are doing this. bsh is ok. 
	 */

	(void) setpgid(0, getppid());
	(void) setsid();

	kleenup (1, 0, 0);
	frevoke (0);
	dup (0);
	dup (0);

	/*
	 * Now the login environment has to be re-created with an
	 * appropriately trusted terminal.  "trust" will be TCUNTRUSTED
	 * when "tpath" does not equal "always".  If "trust" is
	 * TCTRUSTED, setpenv() will execute the trusted shell.
	 */

	ioctl (0, TCTRUST, &trust);
	auditwrite (USER_Shell, AUDIT_OK, tty, strlen (tty) + 1, 0);

	setpenv (user, PENV_INIT, (char **) 0, (char *) 0);

	/*
	 * setpenv() does not return here unless the environment
	 * cannot be established successfully.
	 */

	fprintf (stderr, MSGSTR (M_SETPENV, DEF_SETPENV), user);
	auditwrite (USER_Shell, AUDIT_FAIL, tty, strlen (tty) + 1, 0);
	exit (errno);
}


/*
 * NAME: shellcatgets
 *
 * FUNCTION: Get a string from the SHELL message catalog
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
shellcatgets (Num, Str)
int	Num;
char	*Str;
{
	static	int	once;	/* counter for NLS initialization */
	static  nl_catd	catd;	/* catalogue file descriptor */

	if (! once) {
		catd = catopen (MF_SHELL, NL_CAT_LOCALE);
		once++;
	}
	if (catd != (nl_catd) -1)
		return catgets (catd, MS_SHELL, Num, Str);
	else
		return Str;
}
