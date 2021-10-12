static char sccsid[] = "@(#)95	1.3  src/bos/usr/bin/usrck/usrpwdwarn.c, cmdsadm, bos411, 9428A410j 11/3/93 15:11:45";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: ck_pwdwarntime
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

#include <sys/audit.h>
#include <usersec.h>
#include "usrck_msg.h"
#include "usrck.h"

/*
 * Global data
 */

extern	int	verbose;

/*
 * NAME: ck_pwdwarntime
 *
 * FUNCTION: Check the pwdwarntime user attribute.
 *
 * EXECUTION ENVIRONMENT:
 *	User process.
 *
 * NOTES:
 *	Consistency checks pwdwarntime against minage/maxage.
 *
 * RETURNS: Zero for success, non-zero otherwise.
 */

int
ck_pwdwarntime (char *name)
{
	long	minage, maxage, warntime;

	/*
	 * Get the minage, maxage, and warntime attributes.
	 */

	if (getuserattr (name, S_PWDWARNTIME, (void *) &warntime, SEC_INT))
	{
		msg2 (MSGSTR (M_BADGET, DEF_BADGET), name, S_PWDWARNTIME);
		return(0);
	}

	if (getuserattr (name, S_MINAGE, (void *) &minage, SEC_INT))
	{
		msg2 (MSGSTR (M_BADGET, DEF_BADGET), name, S_MINAGE);
		return(0);
	}

	if (getuserattr (name, S_MAXAGE, (void *) &maxage, SEC_INT))
	{
		msg2 (MSGSTR (M_BADGET, DEF_BADGET), name, S_MAXAGE);
		return(0);
	}

	/*
	 * Convert minage and maxage to days.
	 */

	minage *= 7;
	maxage *= 7;

	/*
	 * See if pwdwarntime is OK.
	 */

	if ((warntime > 0) && maxage && ((maxage - warntime) < minage))
	{
		msg1 (MSGSTR (M_WARNTIME, DEF_WARNTIME), name);
		if (ck_query (MSGSTR (M_FIXWARNTIME, DEF_FIXWARNTIME), name))
		{
			/*
			 * The user requested that pwdwarntime be fixed; set
			 * it to be the number of days between minage and
			 * maxage.
			 */
			if (putuserattr (name, S_PWDWARNTIME, maxage - minage,
					 SEC_INT))
				msg2 (MSGSTR (M_BADPUT, DEF_BADPUT), name,
				      S_PWDWARNTIME);
			mk_audit_rec (AUDIT_OK, name, S_PWDWARNTIME, Fixed);
		} else {
			mk_audit_rec (AUDIT_FAIL, name, S_PWDWARNTIME,
				      NotFixed);
		}
		return -1;
	}
	return 0;
}

