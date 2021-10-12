static char sccsid[] = "@(#)93	1.1  src/bos/usr/bin/usrck/usrlogretries.c, cmdsadm, bos411, 9428A410j 10/4/93 11:25:04";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: ck_logretries
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <usersec.h>
#include "usrck_msg.h"
#include "usrck.h"

/*
 * Global data
 */

extern	int	verbose;

/*
 * NAME: ck_logretries
 *
 * FUNCTION: Check logretries attribute of a user
 *
 * EXECUTION ENVIRONMENT:
 *	User process.
 *
 * NOTES:
 *	Checks the logretries against the number of unsuccessful logins.
 *
 * RETURNS: Zero for success, non-zero otherwise.
 */

int
ck_logretries (char *name)
{
	long	max, count;

	/*
	 * Fetch the loginretries attribute.
	 */

	if (getuserattr (name, S_LOGRETRIES, (void *) &max, SEC_INT)) {
		msg2 (MSGSTR (M_BADGET, DEF_BADGET), name, S_LOGRETRIES);
		max = 0;
	}

	/*
	 * Fetch the number of unsuccessful login attempts.
	 */

	if (getuserattr (name, S_ULOGCNT, (void *) &count, SEC_INT))
		count = 0;

	/*
	 * Check to see if there have been too many invalid login attempts.
	 */

	if ((max > 0) && (count >= max)) {
		msg1 (MSGSTR (M_TOOMANYBAD, DEF_TOOMANYBAD), name);
		return 1;
	}

	/*
	 * The user is ok, so return success.
	 */

	return 0;
}
