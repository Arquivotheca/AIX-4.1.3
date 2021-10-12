static char sccsid[] = "@(#)92	1.1  src/bos/usr/bin/usrck/usrexpires.c, cmdsadm, bos411, 9428A410j 10/4/93 11:24:47";
/*
 *   COMPONENT_NAME: CMDSADM
 *
 *   FUNCTIONS: ck_expires
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

#include <time.h>
#include <usersec.h>
#include "usrck_msg.h"
#include "usrck.h"

/*
 * Global data
 */

extern	int	verbose;

/*
 * NAME: ck_expires
 *
 * FUNCTION: Check the validity of the user's "expires" attribute
 *
 * EXECUTION ENVIRONMENT:
 *	User process.
 *
 * NOTES:
 *	Checks to see if a user's account has expired.
 *
 * RETURNS:
 *	Non-zero if expires is invalid or the user's account has expired,
 *	zero otherwise.
 */

ck_expires (char *name)
{
	char	*expires;
	int	year, month, day, hour, minute;
	time_t	tim;
	struct tm *curtime;

	/*
	 * Get the user's "expires" attribute.  Return success if the
	 * attribute does not exist or is blank.
	 */

	if (getuserattr (name, S_EXPIRATION, (void *) &expires, SEC_CHAR) ||
            ! expires || ! strcmp (expires, "0"))
		return 0;

	/*
	 * See if the expires attribute is valid.
	 */

	if (strlen(expires) != 10)
	{
		msg2 (MSGSTR (M_BADEXPIRES, DEF_BADEXPIRES), name, expires);
		return 1;
	}

	/*
	 * Parse the expires string.
	 */

	if (sscanf (expires, "%2d%2d%2d%2d%2d", &month, &day, &hour, &minute,
		    &year) != 5)
	{
		msg2 (MSGSTR (M_BADEXPIRES, DEF_BADEXPIRES), name, expires);
		return 1;
	}

	/*
	 * Get the current time.
	 */

	tim = time (0);
	curtime = localtime (&tim);

	/*
	 * Compare the expiration time against the current time.
	 */

	/*
	 * Check the year.
	 */

	if (year > curtime->tm_year)
		return 0;
	if (year < curtime->tm_year)
	{
		msg1 (MSGSTR (M_EXPIRED, DEF_EXPIRED), name);
		return 1;
	}

	/*
	 * Check the month.
	 */

	month--;
	if (month > curtime->tm_mon)
		return 0;
	if (month < curtime->tm_mon)
	{
		msg1 (MSGSTR (M_EXPIRED, DEF_EXPIRED), name);
		return 1;
	}

	/*
	 * Check the day.
	 */

	if (day > curtime->tm_mday)
		return 0;
	if (day < curtime->tm_mday)
	{
		msg1 (MSGSTR (M_EXPIRED, DEF_EXPIRED), name);
		return 1;
	}

	/*
	 * Check the hour.
	 */

	if (hour > curtime->tm_hour)
		return 0;
	if (hour < curtime->tm_hour)
	{
		msg1 (MSGSTR (M_EXPIRED, DEF_EXPIRED), name);
		return 1;
	}

	/*
	 * Check the minute.
	 */

	if (minute <= curtime->tm_min)
	{
		msg1 (MSGSTR (M_EXPIRED, DEF_EXPIRED), name);
		return 1;
	}

	return 0;
}

