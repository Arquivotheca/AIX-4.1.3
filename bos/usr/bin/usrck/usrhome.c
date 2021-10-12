static char sccsid[] = "@(#)47	1.6.1.1  src/bos/usr/bin/usrck/usrhome.c, cmdsadm, bos411, 9428A410j 8/24/93 14:56:14";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: ck_home
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/access.h>
#include <sys/priv.h>
#include <sys/id.h>
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
 * NAME: ck_home
 *
 * FUNCTION: Check user's login directory for access
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  This function forks.
 *
 * NOTES:
 *	This function tests a user's login directory for access.  The
 *	directory is tested using the user's credentials for read and
 *	execute access.  Write access is not tested for so that
 *	restricted logins will work.
 *
 * RETURNS: Zero if the user's directory exists and can be accessed by
 *	the user.
 */

int
ck_home (char *name)
{
	char	*home;
	char 	*registry;	/* Administration domain of the user 	*/
	int	status;
	pid_t	pid;
	pid_t	waitpid;
	priv_t	priv;

	/*
	 * Get the user's login directory
	 */

	if (getuserattr (name, S_HOME, (void *) &home, 0)) {
		msg2 (MSGSTR (M_BADGET, DEF_BADGET), name, S_HOME);

		return -1;
	}

	/*
	 * Get the user's registry value.  This will describe the domain
	 * where the user is administered.  If the user is non locally
	 * administered then their home directory often resides on other
	 * systems and other filesystem types.  For this reason we exit
	 * without a check since our privilege model does not work in
	 * other environments (like DFS).
	 */
	if (!getuserattr(name, S_REGISTRY, (void *)&registry, SEC_CHAR))
		if (!ISLOCAL(registry))
			return(0);
		

	/*
	 * Fork off a child and wait.  The child will set its process
	 * credentials to those of the user and test for execute
	 * permission on the login directory.
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
		 * won't work since the inherited set may be non-empty.
		 */

		memset (&priv, 0, sizeof priv);
		if (setpriv (PRIV_SET|PRIV_ALL, &priv, sizeof priv)) { 
			perror ("usrck: setpriv");
			_exit (errno);
		}
		if (access (home, R_ACC|X_ACC)) {
			_exit (errno);
		}
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
		msg2 (MSGSTR (M_NOHOME, DEF_NOHOME), name, home);

	return status != 0;
}
