static char sccsid[] = "@(#)75	1.5.1.1  src/bos/usr/bin/usrck/usrshell.c, cmdsadm, bos411, 9428A410j 3/24/93 13:55:27";
/*
 *   COMPONENT_NAME: CMDSADM
 *
 *   FUNCTIONS: ck_shell
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/access.h>
#include <sys/priv.h>
#include <sys/audit.h>
#include <sys/signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <usersec.h>
#include "usrck_msg.h"
#include "usrck.h"

#define	PRIV_ALL (PRIV_BEQUEATH|PRIV_EFFECTIVE|PRIV_INHERITED|PRIV_MAXIMUM)

/*
 * Global data
 */

int	fixit;			/* indicates fixes are to be made always    */
int	verbose;		/* indicates error messages are displayed   */

/*
 * NAME: ck_shell
 *
 * FUNCTION: Check user's login shell for execute permission.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  This function forks.
 *
 * NOTES:
 *	This function tests a user's login shell for execute
 *	permission.  The shell is tested using the user's credentials.
 *
 * RETURNS: Zero if the user's shell exists and can be executed by
 *	the user.
 */

int
ck_shell (char *name)
{
	char	*shell;
	int	status;
	pid_t	pid;
	pid_t	waitpid;
	priv_t	priv;

	/*
	 * Get the user's login shell
	 */

	if (getuserattr (name, S_SHELL, (void *) &shell, 0)
			|| shell == 0 || ! *shell)
		shell = "/usr/bin/sh";

	/*
	 * Fork off a child and wait.  The child will set its process
	 * credentials to those of the user and test for execute
	 * permission on the login shell.
	 */

	signal (SIGCLD, SIG_DFL);

	fflush (stderr);

	if ((pid = fork ()) == 0) {

		/*
		 * Set the credentials to be the same as those for
		 * the user being checked.  This shouldn't fail ...
		 */

		privilege (PRIV_ACQUIRE);
		if (setpcred (name, (char **) 0)) {
			msg1 (MSGSTR (M_CREDFAIL, DEF_CREDFAIL), name);
			fflush (stderr);
			_exit (EPERM);
		}

		/*
		 * Must turn off ALL privileges prior to testing
		 * for access.  Test for access then exit with the
		 * success or failure of the access call.  PRIV_DROP
		 * can't be used since the inherited privilege set
		 * may be non-empty.
		 */

		memset (&priv, 0, sizeof priv);
		if (setpriv (PRIV_SET|PRIV_ALL, &priv, sizeof priv)) {
			perror ("usrck: setpriv");
			_exit (errno);
		}
		if (accessx (shell, ACC_INVOKER, X_ACC))
			_exit (errno);

		_exit (0);
	} else if (pid != -1) {

		/*
		 * Spin waiting for the child to exit.  Catch the
		 * process with the same process ID as the child, or
		 * stop when no children remain [ there really must
		 * be at least one ... ]
		 */

		while ((waitpid = wait (&status)) != pid && waitpid != -1)
			;
	} else {

		/*
		 * This shouldn't really happen, but I want to at
		 * least mention that it did.  Really shouldn't return
		 * an error since that would cause the account to be
		 * disabled when it might be valid!
		 */

		perror ("usrck: fork");
		return 0;
	}

	/*
	 * Examine the return status and see what's up.  The high
	 * eight bits give the exit code, the low eight bits give the
	 * signal number.  If the entire thing is zero then it exited
	 * OK.
	 */

	if (status)
		msg2 (MSGSTR (M_NOSHELL, DEF_NOSHELL), name, shell);

	return status != 0;
}
