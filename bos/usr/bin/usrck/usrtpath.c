static char sccsid[] = "@(#)78	1.7.1.1  src/bos/usr/bin/usrck/usrtpath.c, cmdsadm, bos411, 9428A410j 3/24/93 13:56:33";
/*
 *   COMPONENT_NAME: CMDSADM
 *
 *   FUNCTIONS: ck_tpath
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
#include <sys/mode.h>
#include <sys/audit.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <usersec.h>
#include <userconf.h>
#include <stdlib.h>
#include "usrck_msg.h"
#include "usrck.h"

/*
 * Global data
 */

extern	int	verbose;
extern	int	fixit;

/*
 * NAME: ck_tpath
 *
 * FUNCTION: Check the validity of the user's "tpath" attribute
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	If the user's "tpath" attribute has the value "always" the
 *	login shell must exist and be tagged as a trusted process.
 *
 * RETURNS: Non-zero if "tpath" is "always" and the login shell is
 *	not tagged a trusted process.  Zero otherwise.
 */

ck_tpath (char *name)
{
	char	*shell;
	char	*tpath;
	int	length;
	int	trusted;
	struct	pcl	*pcl;

	/*
	 * Get the user's "tpath" attribute.  Return success if the
	 * attribute does not exist or is blank.  Also return success
	 * if the value of the attribute is not "always", but an
	 * otherwise valid value.  [ Return failure for invalid
	 * values of "tpath". ]
	 */

	if (getuserattr (name, S_TPATH, (void *) &tpath, 0) || ! tpath)
		return 0;

	if (strcmp (tpath, "always") && strcmp (tpath, "on") &&
			strcmp (tpath, "notsh") && strcmp (tpath, "nosak")) {
		msg2 (MSGSTR (M_BADTPATH, DEF_BADTPATH), name, tpath);

		return ENOTRUST;
	}
	if (strcmp (tpath, "always") != 0)
		return 0;

	/*
	 * Since the user is always required to be on the trusted
	 * path we check the login shell for trustedness.  First
	 * we find out what shell is being used ...
	 */

	if (getuserattr (name, S_SHELL, (void *) &shell, 0) ||
			! shell || ! *shell)
		shell = "/usr/bin/sh";

	/*
	 * The user is required to always be on the trusted path.  We
	 * do a stat_priv() to get the privilege bits on the user's
	 * shell and test for the S_ITCB bit to be set, indicating this
	 * is a trusted process.
	 */

	for (length = sizeof *pcl;;) {

		/*
		 * Allocate the space for the PCL using the
		 * length of either the base PCL or the
		 * size as told by statpriv.
		 */

		if (! (pcl = (struct pcl *) malloc ((size_t) length))) {
			fprintf (stderr, MSGSTR (M_NOMEM, DEF_NOMEM));
			exit (ENOMEM);
		}

		/*
		 * statpriv the file - it may succeed this time,
		 * in which case we just break out.  We have the
		 * entire PCL finally ...
		 */

		if (! statpriv (shell, 0, pcl, length))
			break;

		/*
		 * The only errno we expect is ENOSPC, which indicates
		 * the buffer used above was too small.  If we get a
		 * different error free the buffer and return failure.
		 */

		if (errno != ENOSPC) {
			free ((void *) pcl);
			return -1;
		}

		/*
		 * Set up the length the new PCL needs to be and free
		 * the old space.  Try getting the PCL on the next loop.
		 */

		length = pcl->pcl_len;
		free ((void *) pcl);
	}

	/*
	 * I have a PCL and I want to check for the S_ITCB bit.  Real
	 * straight forward, return failure if the S_ITCB bit is OFF.
	 */

	trusted = (pcl->pcl_mode & S_ITCB) != 0;

	if (! trusted)
		msg1 (MSGSTR (M_NOTPATH, DEF_NOTPATH), name);

	free ((void *) pcl);

	return trusted == 0;
}
