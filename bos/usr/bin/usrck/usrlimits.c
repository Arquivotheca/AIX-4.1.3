static char sccsid[] = "@(#)74	1.7  src/bos/usr/bin/usrck/usrlimits.c, cmdsadm, bos411, 9428A410j 8/7/91 14:45:41";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: ck_limit, ck_resource
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
#include <sys/audit.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <usersec.h>
#include "usrck_msg.h"
#include "usrck.h"

/*
 * Global data
 */

extern	int	verbose;

/*
 * NAME: ck_limit
 *
 * FUNCTION: Check the limit for the requested resource against a lower limit
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * RETURNS: NONE
 */

static int
ck_limit (name, resource, min)
char	*name;
char	*resource;
long	min;
{
	long	l;

	/*
	 * Fetch the resource limit.
	 */

	if (getuserattr (name, resource, (void *) &l, 0)) {
		msg2 (MSGSTR (M_BADGET, DEF_BADGET), name, resource);

		l = 0;
	}

	/*
	 * Compare the value of the resource limit against the
	 * required minimum value.  Correct the value if it is
	 * less than the minimum allowed.
	 */


	if ((l > 0) && (l < min)) {
		msg3 (MSGSTR (M_RESOURCE, DEF_RESOURCE), name, resource, min);

		if (ck_query (MSGSTR (M_FIXLIMIT, DEF_FIXLIMIT), resource)) {
			l = min;
			if (putuserattr (name, resource, (void *) l, 0))
				fprintf (stderr, MSGSTR (M_BADPUT, DEF_BADPUT),
					name, resource);
			mk_audit_rec (AUDIT_OK, name, resource, Fixed);
		} else {
			mk_audit_rec (AUDIT_FAIL, name, resource, NotFixed);
		}
		return -1;
	}
	return 0;
}

/*
 * NAME: ck_resource
 *
 * FUNCTION: Check resource usage values for a user
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Checks the 6 resource usage limits for "sensible" values.
 *
 * RETURNS: Zero for success, non-zero otherwise.
 */

int
ck_resource (struct users *user)
{
	int	errors = 0;	/* Count of incorrect resource limits */
	char	*name = user->usr_name;

	/*
	 * See if the user has a limits file entry and if the invoker
	 * wants me to add a stanza if not.
	 */

	if (! user->usr_limits) {
		msg1 (MSGSTR (M_NOLIMIT, DEF_NOLIMIT), user->usr_name);

		if (ck_query (MSGSTR (M_ADDLIMIT, DEF_ADDLIMIT),
				user->usr_name)) {

			/*
			 * Add just the stanza name - this is a hack
			 * to do this and still keep the file locking
			 * code, etc. in use.
			 */

			putuserattr (name, S_UFSIZE, (void **) 0, SEC_DELETE);
			mk_audit_rec (AUDIT_OK, user->usr_name,
				"add limits file entry", Fixed);
			user->usr_limits = 1;
			return -1;
		} else {

			/*
			 * Invoker decided not to add the stanza,
			 * so I just audit the failure and say I
			 * didn't fix anything.  If they ask to
			 * fix any attributes it will be fixed 
			 * elsewhere.
			 */

			mk_audit_rec (AUDIT_FAIL, user->usr_name,
				"add limits file entry", NotFixed);
		}
		errors++;
	}

	/*
	 * The six resources limits to be checked are "fsize", "cpu",
	 * "data", "stack", "rss", and "core".
	 */

	if (ck_limit (name, S_UFSIZE, MIN_FSIZE))
		errors++;

	if (ck_limit (name, S_UCPU, MIN_CPU))
		errors++;

	if (ck_limit (name, S_UDATA, MIN_DATA))
		errors++;

	if (ck_limit (name, S_USTACK, MIN_STACK))
		errors++;

	if (ck_limit (name, S_URSS, MIN_RSS))
		errors++;

	if (ck_limit (name, S_UCORE, MIN_CORE))
		errors++;

	return errors;
}
