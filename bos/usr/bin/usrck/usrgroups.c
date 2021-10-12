static char sccsid[] = "@(#)46	1.7  src/bos/usr/bin/usrck/usrgroups.c, cmdsadm, bos411, 9428A410j 6/15/90 23:44:04";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: rm_group, ck_group, ck_pgrp, ck_groups, ck_admgroups,
 *	ck_sugroups, fix_groups, fix_admgroups, fix_sugroups
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
#include <string.h>
#include <usersec.h>
#include <pwd.h>
#include "usrck_msg.h"
#include "usrck.h"

/*
 * Global data
 */

extern	int	verbose;
extern	int	fixit;

extern	struct	groups	*groups;
extern	int	ngroups;

extern	int	yp_grp_entries;

extern	int	grpcmp (const void *, const void *);

/*
 * NAME: rm_group
 *
 * FUNCTION: Remove a group from a double-null terminated list
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * NOTES:
 *	Removes a group from a double-null terminate list of groups
 *	by copying all of the groups afterwards forward to this
 *	point.
 *
 * RETURNS: NONE
 */

static void
rm_group (groups)
char	*groups;
{
	char	*old,
		*new;

	/*
	 * Point old at where the current group is and point
	 * new at the start of the next group in the list.
	 */

	new = old = groups;
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
 * NAME: ck_group
 *
 * FUNCTION: Check for the existence of a group by name
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * NOTES:
 *	Binary search of the group table for the named group
 *
 * RETURNS: Index of group entry, or -1 on failure
 */

static int
ck_group (group)
char	*group;
{
	int	high;
	int	low;
	int	test;
	int	i;

	low = 0;
	high = ngroups - 1;
	test = (low + high) / 2;

	while (low <= high) {
		if (i = strcmp (group, groups[test].grp_name)) {
			if (i > 0)
				low = test + 1;
			else
				high = test - 1;

			test = (low + high) / 2;
		} else {
			if (! groups[test].grp_valid)
				return -1;
			else
				return test;
		}
	}
	return -1;
}

/*
 * NAME: ck_pgrp
 *
 * FUNCTION: Check for existence of primary group
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Checks the list of groups for the existence of this
 *	users primary group.
 *
 * RETURNS: Zero if the group exists, non-zero otherwise
 */

int
ck_pgrp (struct users *user)
{
	char	gid[10];
	int	i;

	/*
	 * Locate the user's primary group in the groups table.
	 * Report an error if one doesn't exist.
	 */

	for (i = 0;i < ngroups;i++)
		if (groups[i].grp_gid == user->usr_gid)
			return 0;

	if (yp_grp_entries) {
		sprintf (gid, "%lu", user->usr_gid);
		msg2 (MSGSTR (M_ASSYPGRP, DEF_ASSYPGRP),
			user->usr_name, gid);
		return 0;
	} else {
		msg2 (MSGSTR (M_NOPGRP, DEF_NOPGRP),
			user->usr_name, user->usr_gid);
		return -1;
	}
}

/*
 * NAME: ck_groups
 *
 * FUNCTION: Check a users concurrent groupset for validity
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Each member in the user's concurrent group set is checked
 *	for existence.
 *
 * RETURNS: Zero if all groups exist, non-zero otherwise.
 */

int
ck_groups (char *name)
{
	char	*groups;
	int	errors = 0;

	/*
	 * See if the user has a concurrent group set
	 */

	if (getuserattr (name, S_GROUPS, (void *) &groups, 0) || ! groups)
		return 0;

	/*
	 * Check each member of the concurrent group set for existence
	 * and count how many don't exist.  This will be returned as
	 * the error code.
	 */

	while (*groups) {
		if (ck_group (groups) < 0) {
			if (yp_grp_entries) {
				msg2 (MSGSTR (M_ASSYPGRP, DEF_ASSYPGRP),
					name, groups);
			} else {
				errors++;

				msg2 (MSGSTR (M_NOGROUP, DEF_NOGROUP),
					name, groups);
			}
		}
		while (*groups++)
			;
	}
	return errors;
}

/*
 * NAME: ck_admgroups
 *
 * FUNCTION: Check a users administrative groupset for validity
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Each member in the user's administrative group set is checked
 *	for existence.
 *
 * RETURNS: Zero if all groups exist, non-zero otherwise.
 */

int
ck_admgroups (struct users *user)
{
	char	*groups;
	char	*name = user->usr_name;
	int	errors = 0;

	/*
	 * See if the user has an administrative group set
	 */

	if (getuserattr (name, S_ADMGROUPS, (void *) &groups, 0)
			|| ! groups)
		return 0;

	/*
	 * Check each member of the administrative group set for existence
	 * and count how many don't exist.  This will be returned as
	 * the error code.
	 */

	while (*groups) {
		if (ck_group (groups) < 0) {
			if (yp_grp_entries) {
				msg2 (MSGSTR (M_ASSYPGRP, DEF_ASSYPGRP),
					name, groups);
			} else {
				errors++;

				msg2 (MSGSTR (M_NOADMGRP, DEF_NOADMGRP),
					name, groups);
			}
		}
		while (*groups++)
			;
	}
	return errors;
}

/*
 * NAME: ck_sugroups
 *
 * FUNCTION: Check a users switch-user groupset for validity
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Each member in the user's switch-user group set is checked
 *	for existence.
 *
 * RETURNS: Zero if all groups exist, non-zero otherwise.
 */

int
ck_sugroups (struct users *user)
{
	char	*groups;
	char	*name = user->usr_name;
	int	errors = 0;

	/*
	 * See if the user has a switch-user group set
	 */

	if (getuserattr (name, S_SUGROUPS, (void *) &groups, SEC_LIST)
			|| ! groups)
		return 0;

	/*
	 * Check each member of the switch-user group set for existence
	 * and count how many don't exist.  This will be returned as
	 * the error code.
	 */

	while (*groups) {
		if (strcmp (groups, "ALL") != 0 && ck_group (groups) < 0) {
			if (yp_grp_entries) {
				msg2 (MSGSTR (M_ASSYPGRP, DEF_ASSYPGRP),
					name, groups);
			} else {
				errors++;

				msg2 (MSGSTR (M_NOSUGRP, DEF_NOSUGRP),
					name, groups);
			}
		}
		while (*groups++)
			;
	}
	return errors;
}

/*
 * NAME: fix_groups
 *
 * FUNCTION: Fix a users concurrent groupset
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Each member in the user's concurrent group set is checked
 *	for existence and non-existent groups are removed.
 *
 * RETURNS: NONE
 */

void
fix_groups (char *name)
{
	char	*groups;
	char	*cp;
	char	buf[MAXATTRSIZ];

	/*
	 * Get the users concurrent group set
	 */

	if (getuserattr (name, S_GROUPS, (void *) &groups, 0) || ! groups)
		return;

	for (cp = buf;groups[0] || groups[1];*cp++ = *groups++)
		;
	*cp = 0;

	groups = buf;

	/*
	 * Check each member of the concurrent group set for existence
	 * and remove those that don't exist.
	 */

	while (*groups) {
		if (ck_group (groups) < 0) {
			if (yp_grp_entries)
				msg2 (MSGSTR (M_ASSYPGRP, DEF_ASSYPGRP),
					name, groups);
			else
				rm_group (groups);
		} else
			while (*groups++)
				;
	}
	if (putuserattr (name, S_GROUPS, (void *) buf, 0))
		fprintf (stderr, MSGSTR (M_BADPUT, DEF_BADPUT), name, S_GROUPS);
}

/*
 * NAME: fix_admgroups
 *
 * FUNCTION: Fix a users administrative groupset
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Each member in the user's administrative group set is checked
 *	for existence and non-existent groups are removed.
 *
 * RETURNS: Zero if all groups exist, non-zero otherwise.
 */

void
fix_admgroups (char *name)
{
	char	*groups;
	char	*cp;
	char	buf[MAXATTRSIZ];

	/*
	 * Get the user's administrative groupset
	 */

	if (getuserattr (name, S_ADMGROUPS, (void *) &groups, 0) || ! groups)
		return;

	for (cp = buf;groups[0] || groups[1];*cp++ = *groups++)
		;
	*cp = 0;

	groups = buf;

	/*
	 * Check each member of the administrative group set for existence
	 * and remove those that don't exist.
	 */

	while (*groups) {
		if (ck_group (groups) < 0) {
			if (yp_grp_entries)
				msg2 (MSGSTR (M_ASSYPGRP, DEF_ASSYPGRP),
					name, groups);
			else
				rm_group (groups);
		} else
			while (*groups++)
				;
	}
	if (putuserattr (name, S_ADMGROUPS, (void *) buf, 0))
		fprintf (stderr, MSGSTR (M_BADPUT, DEF_BADPUT),
			name, S_ADMGROUPS);
}

/*
 * NAME: fix_sugroups
 *
 * FUNCTION: Fix a users switch-user groupset
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Each member in the user's switch-user group set is checked
 *	for existence and all non-existent groups are removed.
 *
 * RETURNS: Zero if all groups exist, non-zero otherwise.
 */

void
fix_sugroups (char *name)
{
	char	*groups;
	char	*cp;
	char	buf[MAXATTRSIZ];

	/*
	 * Get the users switch-user group set
	 */

	if (getuserattr (name, S_SUGROUPS, (void *) &groups, SEC_LIST)
			|| ! groups)
		return;

	for (cp = buf;groups[0] || groups[1];*cp++ = *groups++)
		;
	*cp = 0;

	groups = buf;

	/*
	 * Check each member of the switch-user group set for existence
	 * and remove those that don't exist.
	 */

	while (*groups) {
		if (strcmp (groups, "ALL") != 0 && ck_group (groups) < 0) {
			if (yp_grp_entries)
				msg2 (MSGSTR (M_ASSYPGRP, DEF_ASSYPGRP),
					name, groups);
			else
				rm_group (groups);
		} else
			while (*groups++)
				;
	}
	if (putuserattr (name, S_SUGROUPS, (void *) buf, SEC_LIST))
		fprintf (stderr, MSGSTR (M_BADPUT, DEF_BADPUT),
			name, S_SUGROUPS);
}
