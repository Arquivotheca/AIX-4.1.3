static char sccsid[] = "@(#)40	1.4  src/bos/usr/bin/usrck/usraudit.c, cmdsadm, bos411, 9428A410j 6/15/90 23:43:34";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: rm_class, ck_class, ck_audit, fix_audit
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
#include <userconf.h>
#include "usrck_msg.h"
#include "usrck.h"

/*
 * Global data
 */

extern	int	verbose;
extern	int	fixit;

/*
 * NAME: rm_class
 *
 * FUNCTION: Remove a single audit class
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * NOTES:
 *	Removes an audit class from a double-null terminated list
 *	of audit classes by copying all of the audit classes
 *	forward to this point.
 *
 * RETURNS: NONE
 */

static void
rm_class (classes)
char	*classes;
{
	char	*old,
		*new;

	/*
	 * Point old at where the current audit class is
	 * and point new at the start of the next audit
	 * class in the list.
	 */

	new = old = classes;
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
 * NAME: ck_class
 *
 * FUNCTION: Check for the existence of an audit class
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * RETURNS: Zero if the class exists, non-zero otherwise.
 */

static int
ck_class (class)
char	*class;
{
	char	*dummy;

	return getconfattr (SC_SYS_AUDIT, class, (void *) &dummy, 0);
}

/*
 * NAME: ck_audit
 *
 * FUNCTION: Check a user's audit classes for validity
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Each audit class in the user's set is checked for existence.
 *
 * RETURNS: Zero if all classes exist, non-zero otherwise.
 */

int
ck_audit (char *name)
{
	char	*classes;
	int	errors = 0;

	/*
	 * Get the user's audit classes.  If the value is "ALL",
	 * just return.
	 */

	if (getuserattr (name, S_AUDITCLASSES, (void *) &classes, 0)
			|| ! classes)
		return 0;

	if (strcmp (classes, "ALL") == 0)
		return 0;

	/*
	 * Check each audit class for existence and count those that
	 * don't exist.  We pass the count back as the return code.
	 */

	while (*classes) {
		if (ck_class (classes)) {
			errors++;
			msg2 (MSGSTR (M_NOAUDIT, DEF_NOAUDIT), name, classes);
		}
		while (*classes++)
			;
	}
	return errors;
}

/*
 * NAME: fix_audit
 *
 * FUNCTION: Fix a user's audit classes.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Each audit class in the user's list of audit classes is
 *	checked for existence and the non-existent classes are
 *	removed.
 *
 * RETURNS: NONE
 */

void
fix_audit (char *name)
{
	char	*classes;
	char	*cp;
	char	buf[MAXATTRSIZ];

	/*
	 * Get the user's audit classes and copy the list to a
	 * buffer where it can be worked on.
	 */

	if (getuserattr (name, S_AUDITCLASSES, (void *) &classes, 0)
			|| ! classes)
		return;

	for (cp = buf;classes[0] || classes[1];*cp++ = *classes++)
		;
	*cp = '\0';

	classes = buf;

	/*
	 * Check each audit class and remove those that don't exist
	 */

	while (*classes) {
		if (ck_class (classes))
			rm_class (classes);
		else
			while (*classes++)
				;
	}

	/*
	 * Output the new list.
	 */

	if (putuserattr (name, S_AUDITCLASSES, (void *) buf, 0))
		msg2 (MSGSTR (M_BADPUT, DEF_BADPUT), name, S_AUDITCLASSES);
}

/*
 * NAME: mk_audit_rec
 *
 * FUNCTION: create an audit record given the provided information
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: NONE
 */

void
mk_audit_rec (int status, char *user, char *error, char *result)
{
	privilege (PRIV_ACQUIRE);

	auditwrite (USER_Check, status,
			user, strlen (user) + 1,
			error, strlen (error) + 1,
			result, strlen (result) + 1,
			0);

	privilege (PRIV_LAPSE);
}
