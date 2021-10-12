static char sccsid[] = "@(#)79	1.5  src/bos/usr/bin/usrck/usrttys.c, cmdsadm, bos411, 9428A410j 8/26/91 16:45:36";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: rm_tty, ck_tty, ck_ttys, fix_ttys
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/access.h>
#include <sys/audit.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <usersec.h>
#include "usrck_msg.h"
#include "usrck.h"

/*
 * Global data
 */

extern	int	verbose;
extern	int	fixit;

/*
 * NAME: rm_tty
 *
 * FUNCTION: Remove a tty name from a double-null terminated list
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * NOTES:
 *	Removes a tty from a double-null terminate list of ttys
 *	by copying all of the ttys afterwards forward to this
 *	point.
 *
 * RETURNS: NONE
 */

static void
rm_tty (ttys)
char	*ttys;
{
	char	*old,
		*new;

	/*
	 * Point old at where the current tty is and point
	 * new at the start of the next tty in the list.
	 */

	new = old = ttys;
	while (*new++)
		;

	/*
	 * Copy from the new location backwards until a double
	 * null is seen.  Then paste an extra null on to double
	 * null terminate the new string.
	 */

	while (*new)
		while (*old++ = *new++)
			;

	*old = '\0';
}

/*
 * NAME: ck_tty
 *
 * FUNCTION: Check for the existence of a tty device
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * RETURNS: Zero if the tty exists, non-zero otherwise.
 */

static int
ck_tty (tty)
char	*tty;
{
        /* D33467, ! in front of tty is allowed, ALL and * are also valid */
        if (!strcmp(tty,"ALL") || !strcmp(tty,"*"))
           return(0);

        if (*tty == '!')
           tty++;

	return access (tty, E_ACC);
}

/*
 * NAME: ck_ttys
 *
 * FUNCTION: Check a users login tty's for existence
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Each tty in the user's set is checked for existence.
 *
 * RETURNS: Zero if all ttys exist, non-zero otherwise.
 */

int
ck_ttys (char *name)
{
	char	*ttys;
	int	errors = 0;

	 /* Get the user's login ttys. */

	if (getuserattr (name, S_TTYS, (void *) &ttys, 0) || ! ttys)
		return 0;

	/*
	 * Check each member of the login devices list for existence
	 * and count how many don't exist.  This will be returned as
	 * the error code.
	 */

	while (*ttys) {
		if (ck_tty (ttys)) {
			errors++;

			msg2 (MSGSTR (M_NOTTY, DEF_NOTTY), name, ttys);
		}
		while (*ttys++)
			;
	}
	return errors;
}

/*
 * NAME: fix_ttys
 *
 * FUNCTION: Fix a users login terminal list
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Each device in the user's login terminal list is checked
 *	for existence and non-existent ttys are removed.
 *
 * RETURNS: NONE
 */

void
fix_ttys (char *name)
{
	char	*ttys;
	char	*cp;
	char	buf[MAXATTRSIZ];

	/*
	 * Get the user's login tty list and copy it to a buffer
	 * where it can be worked on.
	 */

	if (getuserattr (name, S_TTYS, (void *) &ttys, 0) || ! ttys)
		return;

	for (cp = buf;ttys[0] || ttys[1];*cp++ = *ttys++)
		;

	*cp = '\0';

	ttys = buf;

	/*
	 * Check each login device and remove those that don't exist.
	 */

	while (*ttys) {
		if (ck_tty (ttys))
			rm_tty (ttys);
		else
			while (*ttys++)
				;
	}

	/*
	 * Output the new list.
	 */

	if (putuserattr (name, S_TTYS, (void *) buf, 0))
		fprintf (stderr, MSGSTR (M_BADPUT, DEF_BADPUT), name, S_TTYS);
}
