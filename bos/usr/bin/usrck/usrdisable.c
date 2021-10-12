static char sccsid[] = "@(#)45	1.6.1.1  src/bos/usr/bin/usrck/usrdisable.c, cmdsadm, bos411, 9428A410j 10/4/93 11:23:10";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: ck_disabled, mk_disabled
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <usersec.h>
#include "usrck_msg.h"
#include "usrck.h"

/*
 * NAME: ck_disabled
 *
 * FUNCTION: Check if a user is locked
 *
 * EXECUTION ENVIRONMENT:
 *	User process.
 *
 * NOTES:
 *	Checks the account_locked attribute of the user.
 *
 * RETURNS: Non-zero if the account is disabled, zero otherwise
 */

int
ck_disabled (char *name)
{
	int	locked;

	if (getuserattr (name, S_LOCKED, &locked, SEC_BOOL))
		return 0;

	return locked;
}

/*
 * NAME: mk_disabled
 *
 * FUNCTION: Disable an account by setting account_locked to true.
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * NOTES:
 *	The account_locked attribute is set to true.
 *
 * RETURNS: Zero on success, non-zero otherwise.
 */

int
mk_disabled (char *name)
{
	return putuserattr (name, S_LOCKED, "true", SEC_BOOL);
}
