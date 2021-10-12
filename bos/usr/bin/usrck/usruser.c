static char sccsid[] = "@(#)80  1.7  src/bos/usr/bin/usrck/usruser.c, cmdsadm, bos411, 9428A410j 5/10/91 13:24:03";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: ck_name, ck_uid
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

extern	struct	users	*users;
extern	int	nusers;
extern	int	verbose;

/*
 * NAME:	valid_name
 *
 * FUNCTION:	check user name for validity.
 *
 * RETURN VALUE:
 *	Zero for valid user names, non-zero otherwise.
 */

int
valid_name (char *name)
{
	char *namep;         /* prt to the name */
	static char illegal1stch[] = {'-', '+', ':', '~',0};
	int	i;

	/*
	 * A quick sanity check on the user name
	 */

	if (name == 0 || *name == '\0')
		return -1;

	/*
	 * Validate the user name.  It can't contain one of the illegalchar
	 * listed above as the first character and then cannot contain a (:) colon 
	 * in any of the following characters. 
	 */
	if (name && (strchr (illegal1stch, *name)))
        	return -1;

	/* name cannot contain a colon (:)  */

	for (i = 1; name[i]; i++)
		if (name[i] == ':')
			break;
 
	if (name[i])
		return -1;

	/*
	 * The user names "ALL" and "default" can't be used as user
	 * names since they have special meaning elsewhere.
	 */

	if ( (strcmp (name, "ALL") == 0) || (strcmp (name, "default") == 0) )
		return -1;

	return 0;
}

/*
 * NAME: ck_name
 *
 * FUNCTION: Scan for duplicate instances of 'name'
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Called once for each user selected for testing.
 *
 * RETURNS: Zero if name occurs exactly once, non-zero otherwise.
 */

int
ck_name (char *name)
{
	int	i;
	int	count = 0;

	/*
	 * Validate the user name.
	 */

	if (valid_name (name)) {
		msg1 (MSGSTR (M_BADNAME, DEF_BADNAME), name);

		return -1;
	}

	/*
	 * Scan for the name, incrementing a counter as you go.
	 * If the counter reaches two bail out and return an
	 * error.
	 */

	for (i = 0;i < nusers;i++) {
		if (strcmp (name, users[i].usr_name) == 0) {
			if (++count > 1)
				break;
		}
	}
	if (count > 1) {
		msg1 (MSGSTR (M_DUPNAME, DEF_DUPNAME), name);

		return -1;
	}
	return 0;
}

/*
 * NAME: ck_uid
 *
 * FUNCTION: Scan for duplicate instances of 'uid'
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Called once for each user selected for testing.
 *
 * RETURNS: Zero if uid occurs exactly once, non-zero otherwise.
 */

int
ck_uid (uid_t uid)
{
	int	i;
	int	count = 0;

	/*
	 * Scan for the UID, incrementing a counter as you go.
	 * If the counter reaches two, bail out and return an
	 * error.
	 */

	for (i = 0;i < nusers;i++)
		if (uid == users[i].usr_uid)
			if (++count > 1)
				return -1;

	return 0;
}
