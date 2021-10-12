static char sccsid[] = "@(#)94	1.1  src/bos/usr/bin/usrck/usrlogtimes.c, cmdsadm, bos411, 9428A410j 10/4/93 11:25:22";
/*
 *   COMPONENT_NAME: CMDSADM
 *
 *   FUNCTIONS: ck_logintimes
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <usersec.h>
#include "usrck_msg.h"
#include "usrck.h"

/*
 * Global data
 */

extern	int	verbose;

/*
 * NAME: ck_logintimes
 *
 * FUNCTION: Check the validity of the user's "logintimes" attribute
 *
 * EXECUTION ENVIRONMENT:
 *	User process.
 *
 * NOTES:
 *	Checks to see if the login time restriction specified by the logintimes
 *	attribute can be parsed correctly.
 *
 * RETURNS:
 *	Non-zero if logintimes is invalid, zero otherwise.
 */

ck_logintimes (char *name)
{
	char	*logintimes, *userlogtimes;

	/*
	 * Get the user's "logintimes" attribute.  Return success if the
	 * attribute does not exist or is blank.
	 */

	if (getuserattr (name, S_LOGTIMES, (void *) &logintimes, SEC_LIST) ||
            ! logintimes || (strlen (logintimes) == 0))
		return 0;

	/*
	 * Parse the logintimes attribute; return 0 if it parses correctly and
	 * 1 otherwise.
	 */
	userlogtimes = _dbtouser (name, logintimes);
	if (userlogtimes)
	{
		free (userlogtimes);
		return 0;
	}
	else
	{
		msg1 (MSGSTR (M_BADLOGTIMES, DEF_BADLOGTIMES), name);
		return 1;
	}
}

