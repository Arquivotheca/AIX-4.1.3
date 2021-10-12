static char sccsid[] = "@(#)49	1.9.1.1  src/bos/usr/bin/usrck/usrinit.c, cmdsadm, bos411, 9428A410j 4/15/92 15:14:44";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: dup_string, expand_users, expand_groups, mark_users,
 *	grpcmp, init_users, init_groups
 *
 * ORIGINS: 26,27
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
#include <sys/limits.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <stdlib.h>
#include <usersec.h>
#include <userpw.h>
#include "usrck_msg.h"
#include "usrck.h"

/*
 * Global data
 */

struct	users	*users;		/* table of users and flags                 */
int	nusers;			/* number of users in users table           */

struct	groups	*groups;	/* list of groups in groups file            */
int	ngroups;		/* number of groups in the table            */

char	**checknames;		/* list of names to check                   */

extern	int	verbose;
extern	int	fixit;
extern	int	all;
extern	int	nflg;

extern	int	yp_grp_entries;	/* set if YP group entries found            */

extern	char	*strdup (char *);
extern	char	*afread (AFILE_t);
extern	

findname (char *name)
{
	int	i;

	for (i = 0;checknames[i];i++)
		if (strcmp (name, checknames[i]) == 0)
			return 0;

	return -1;
}

finduser (char *name)
{
	int	i;

	for (i = 0;i < nusers;i++)
		if (strcmp (name, users[i].usr_name) == 0 &&
				users[i].usr_valid)
			return i;

	return -1;
}

findgroup (char *group)
{
	int	i;

	for (i = 0;i < ngroups;i++)
		if (strcmp (group, groups[i].grp_name) == 0 &&
				groups[i].grp_valid)
			return i;

	return -1;
}

static void
rm_passwd (char *line)
{
	char	*begin;
	char	*end;

	begin = strchr (line, (int) ':') + 1;
	if (*begin == ':')
		return;

	*begin++ = '!';

	if (*begin == ':')
		return;

	for (end = strchr (begin, (int) ':');end && (*begin++ = *end++);)
		;
}

/*
 * NAME:	pwparse - parse a password file line
 *
 * FUNCTION:	Parse a single line of text from a password file
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURN VALUE:
 *	Pointer to structure if the line is valid, NULL otherwise.
 */

/*
 * BEGIN BSD CODE - from lib/c/adm/getpwent.c
 *
 * This code was take from the above module.  It appears to have its
 * origins in BSD, or possibly even old AT&T code.
 */

static char *
pwskip(char *p)
{
	while(*p && *p != ':' && *p != '\n')
		++p;
	if (*p == '\n')
		*p = '\0';
	else if (*p != '\0')
		*p++ = '\0';
	return(p);
}

int
pwd_interpret (char *val, struct passwd **passwd)
{
	char	*p;
	char	*end;
	char	*name;
	ulong	x;
	static	struct	passwd	interppasswd;
	static	char	interpline[BUFSIZ];

	*passwd = &interppasswd;
	interppasswd.pw_name = (char *) 0;
	interppasswd.pw_uid = (uid_t) -1;
	interppasswd.pw_gid = (gid_t) -1;

	(void) strncpy(interpline, val, (size_t) BUFSIZ);
	p = interpline;

	if (strchr (interpline, ':') == 0 || *p == ':') {
		if (all)
			msg1 (MSGSTR (M_NONAME, DEF_NONAME), val);

		return ENOTRUST;
	}
	interppasswd.pw_name = name = p;
	if (interppasswd.pw_name[0] == '\0')
		return ENOTRUST;

	p = pwskip(p);

	interppasswd.pw_passwd = p;
	p = pwskip(p);

	if (*p == ':') {
		if (all || findname (name) == 0)
			msg1 (MSGSTR (M_BADUID, DEF_BADUID), val);

		return ENOTRUST;
	} else
		x = strtoul(p, &end, 10);       
	p = end;
 
	if (*p++ != ':') {
		if (all || findname (name) == 0)
			msg1 (MSGSTR (M_BADUID, DEF_BADUID), val);

		return ENOTRUST;
	} else
		interppasswd.pw_uid = x;

	if (*p == ':') {
		if (all || findname (name) == 0)
			msg1 (MSGSTR (M_BADGID, DEF_BADGID), val);

		return ENOTRUST;
	} else {
		errno = 0;
		if ((x = strtoul(p, &end, 10)), errno != 0) {
			if (all || findname (name) == 0)
				msg1 (MSGSTR (M_BADGID, DEF_BADGID), val);

			return ENOTRUST;
		}
	}
	p = end;
	if (*p++ != ':') {
		if (all || findname (name) == 0)
			msg1 (MSGSTR (M_BADGID, DEF_BADGID), val);

		return ENOTRUST;
	}
	interppasswd.pw_gid = x;

	interppasswd.pw_gecos = p;
	p = pwskip(p);

	interppasswd.pw_dir = p;
	if (*interppasswd.pw_dir == '\0') {
		if (all || findname (name) == 0)
			msg1 (MSGSTR (M_BADHOME, DEF_BADHOME), val);

		return ENOTRUST;
	}
	p = pwskip(p);

	interppasswd.pw_shell = p;
	while(*p && *p != '\n')
		p++;

	*p = '\0';
	return 0;
}

int
grp_interpret (char *val, struct group **group)
{
	char	*p;
	char	*end;
	char	*name;
	ulong	x;
	int	i;
	static	struct	group	interpgroup;
	static	char	interpline[BUFSIZ*8];
	static	char	*group_members[1024];

	*group = &interpgroup;
	interpgroup.gr_name = (char *) 0;
	interpgroup.gr_gid = (gid_t) -1;

	(void) strncpy(interpline, val, (size_t) sizeof interpline);
	p = interpline;

	if (strchr (interpline, ':') == 0 || *p == ':') {
		if (all)
			msg1 (MSGSTR (M_NOGRPNAME, DEF_NOGRPNAME), val);

		return ENOTRUST;
	}
	interpgroup.gr_name = name = p;
	if (interpgroup.gr_name[0] == '\0')
		return ENOTRUST;

	if ((p = pwskip(p)), *p == '\0')	/* Skip the name */
		return ENOTRUST;

	if ((p = pwskip(p)), *p == '\0')	/* Skip the password */
		return ENOTRUST;

	if (*p == ':') {
		if (all)
			msg1 (MSGSTR (M_BADGRPGID, DEF_BADGRPGID), val);

		return ENOTRUST;
	} else {
		errno = 0;
		if ((x = strtoul(p, &end, 10)), errno != 0) {
			if (all)
				msg1 (MSGSTR (M_BADGRPGID, DEF_BADGRPGID), val);

			return ENOTRUST;
		}
	}
	p = end;
	if (*p++ != ':') {
		if (all)
			msg1 (MSGSTR (M_BADGRPGID, DEF_BADGRPGID), val);

		return ENOTRUST;
	}
	interpgroup.gr_gid = x;

	for (i = 0;*p && i < 1023;i++) {
		group_members[i] = p;
		while (*p && *p != ',')
			p++;

		if (*p)
			*p++ = '\0';
	}
	group_members[i] = '\0';
	interpgroup.gr_mem = group_members;

	return 0;
}

/*
 * NAME: dup_string
 *
 * FUNCTION: Allocate memory for and copy a string
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * NOTES:
 *	Exits on memory allocation errors.
 *
 * RETURNS: Pointer to an allocate version of the string
 */

static char *
dup_string (char *s)
{
	char	*cp;

	cp = (char *) malloc ((size_t) strlen (s) + 1);

	if (cp)
		return (char *) strcpy (cp, s);

	fprintf (stderr, MSGSTR (M_NOMEM, DEF_NOMEM));
	exit (ENOMEM);

	/* NOTREACHED */
}

/*
 * NAME: expand_users
 *
 * FUNCTION: Increase the size of the user's information table.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * NOTES:
 *	This function exits on memory allocation failure.
 *
 * RETURNS: NONE
 */

static void
expand_users (void)
{

	/*
	 * If the table does not already exist we create it with
	 * space for 32 users, otherwise double its size.
	 */

	if (nusers == 0) {
		nusers = 32;
		users = (struct users *)
				malloc ((size_t) nusers * sizeof *users);
	} else {
		nusers = nusers * 2;
		users = (struct users *) realloc ((void *) users,
				(size_t) nusers * sizeof *users);
	}

	/*
	 * Panic if we have run out of core.
	 */

	if (users == 0) {
		fprintf (stderr, MSGSTR (M_NOMEM, DEF_NOMEM));
		exit (ENOMEM);
	}
}

/*
 * NAME: expand_groups
 *
 * FUNCTION: Increase the size of the groups table.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * NOTES:
 *	This function exits on memory allocation failure.
 *
 * RETURNS: NONE
 */

void
expand_groups (void)
{

	/*
	 * Increase the size of the groups table.  If the table
	 * does not already exist create it, otherwise double
	 * the size.
	 */

	if (ngroups == 0) {
		ngroups = 32;
		groups = (struct groups *)
			malloc ((size_t) ngroups * sizeof *groups);
	} else {
		ngroups = ngroups * 2;
		groups = (struct groups *) realloc ((void *) groups,
			(size_t) ngroups * sizeof *groups);
	}

	/*
	 * Panic if we have run out of core.
	 */

	if (groups == 0) {
		fprintf (stderr, MSGSTR (M_NOMEM, DEF_NOMEM));
		exit (ENOMEM);
	}
}

/*
 * NAME: mark_users
 *
 * FUNCTION: Tag user's to be tested using list of names.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * RETURNS: 0 for success, ENOENT if an unknown user is requested
 */

static int
mark_users (char **names)
{
	int	i;		/* index into list of names */
	int	errors = 0;	/* Count of nonexistent users */
	struct	users	*user;	/* index into table of users */

	/*
	 * If all is set, go ahead and mark the entire list of users.
	 * Don't return just yet.  Scan the entire list for nonexistent
	 * bodies that someone added for fun.
	 */

	if (all)
		for (user = users;user != &users[nusers];user++)
			user->usr_test = 1;

	/*
	 * Check each user in the table for a match against a name
	 * in the list.  Turn on the 'usr_test' flag if a match is
	 * found.  Don't try looking for "ALL" if it exists.
	 */

	for (i = 0;names[i];i++) {

		if (strcmp (names[i], "ALL") == 0)
			continue;

		/*
		 * Check each name in the list for a match and set
		 * its 'usr_test' flag.
		 */

		for (user = users;user != &users[nusers];user++) {
			if (strcmp (names[i], user->usr_name) == 0) {
				user->usr_test = 1;
				break;
			}
		}

		/*
		 * Now see if a match was made.  If not, report the error.
		 */

		if (user == &users[nusers]) {
			msg1 (MSGSTR (M_NOUSER, DEF_NOUSER), names[i]);

			errors++;
		}
	}

	/*
	 * See if any nonexistent users were requested.  All of them
	 * were found and reported to the user.
	 */

	if (errors)
		return ENOENT;
	else
		return 0;
}

/*
 * NAME: grpcmp
 *
 * FUNCTION: Wrapper for strcmp to be used by qsort
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Called indirectly by qsort
 *
 * RETURNS: The result of the strcmp
 */

int
grpcmp (const void *a, const void *b)
{
	struct	groups	*ap = (struct groups *) a;
	struct	groups	*bp = (struct groups *) b;

	if (ap->grp_name == 0) {
		if (bp->grp_name == 0)
			return 0;

		return -1;
	} else if (bp->grp_name == 0)
		return 1;

	return strcmp (ap->grp_name, bp->grp_name);
}

/*
 * NAME: intcmp
 *
 * FUNCTION: compares the line numbers of two group structures
 *
 * EXECUTION ENVIRONMENT:
 *	Called indirectly by qsort().
 *
 * NOTES:
 *	INPUT:
 *		(const void *) a - first group line
 *		(const void *) b - second group line
 *	OUTPUT:
 *		(int) result of comparision between a->grp_lineno and
 *		    b->grp_lineno.
 */

int
intcmp (const void *a, const void *b)
{
	return ((struct groups *) a)->grp_lineno -
			((struct groups *) b)->grp_lineno;
}

/*
 * NAME: init_users
 *
 * FUNCTION: Read in the entire password file saving user names and UIDs.
 *	     Read in the entire security/limits file checking user names.
 *	     Read in the entire security/passwd file checking user names.
 *	     Read in the entire security/user file checking user names.
 *
 * NOTES:
 *	This is handled in three steps.  The first part involves reading
 *	the entire password file into a table of user names and UIDs.
 *	This is required to test for unique names and UIDs later.
 *
 *	The second step involves flagging the users to actually be
 *	tested.
 *
 *	The third step involves reading the security files and flagging each
 *	user as not having a stanza.
 *
 *	The table of users is grown by doubling the size each time it
 *	is expanded.  After all users have been entered the table is
 *	truncated to its correct size.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Called once at command startup.
 *
 * RETURNS: NONE
 *	This function exits if it is unable to open the system password file.
 */

int
init_users (char **names)
{
	char	buf[BUFSIZ];	/* Place to put read in password lines */
	char	name[BUFSIZ];	/* Name of the user in the passwd file */
	char	*cp;		/* Cursor into input line */
	int	current = 0;	/* Current number of users in the table */
	int	*found;		/* Flag for each username being found */
	int	i;		/* Count number of requested users */
	int	errors = 0;	/* Count of errors reading password file */
	struct	passwd	*pw;	/* A password file entry */
	struct	users	*usr;	/* A user structure pointer */
	struct	userpw	upw;	/* A user password file structure */
	FILE	*fp;		/* handle for reading password file */
	AFILE_t	af;		/* handle for reading limits file */

	checknames = names;

	/*
	 * See if "ALL" exists in the list of users.  If it does it
	 * must be the only user name.
	 */

	for (i = 0;names[i];i++)
		if (strcmp (names[i], "ALL") == 0)
			all = 1;

	if (all && i != 1)
		usage ();

	/*
	 * Read the entire password file, building our table as we go.
	 */

	if (! (fp = fopen ("/etc/passwd", "r"))) {
		fprintf (stderr, MSGSTR (M_NOPWDFILE, DEF_NOPWDFILE));
		exit (ENOENT);
	}

	while (1) {

		/*
		 * See if there is any room in the table for the
		 * new entry.  Expand the table as needed.
		 */

		if (current == nusers)
			expand_users ();

		/*
		 * Read lines from /etc/passwd, counting colons and
		 * reporting errors.  Read a line, remove the '\n',
		 * and count the colons.  The answer must be 5.  Then
		 * convert to a struct passwd for further testing.
		 */

		if (fgets (buf, (int) sizeof buf, fp) == 0)
			break;

		users[current].usr_pwdline = dup_string (buf);
		users[current].usr_delete = 0;
		users[current].usr_valid = 0;
		users[current].usr_uid = (uid_t) -1;
		users[current].usr_gid = (gid_t) -1;
		users[current].usr_test = 0;
		users[current].usr_limits = 0;
		users[current].usr_passwd = 0;
		users[current].usr_user = 0;

		/*
		 * Look for Yellow Pages hieroglyphics.  The characters
		 * '-', '+' and '@' start YP entries.  Skip the entire
		 * line, it isn't worth getting upset about ...
		 */

		if (buf[0] == '-' || buf[0] == '+' || buf[0] == '@') {
			current++;
			continue;
		}

		/*
		 * NUL terminate the line and parse it into pieces-parts.
		 */

		buf[BUFSIZ-1] = '\0';
		buf[strlen (buf) - 1] = '\0';

		for (cp = buf, i = 0;*cp;cp++)
			if (*cp == ':')
				i++;

		/*
		 * Insure the password line is 7 colon separated fields.
		 * pwd_interpret() will find any errors with lines having
		 * empty or mis-formed values.
		 */

		if (all && i != 6) {
			errors++;
			auditwrite ("USRCK_Error", AUDIT_FAIL,
				buf, strlen (buf), "passwd", 6, 0);

			msg1 (MSGSTR (M_BADPWD, DEF_BADPWD), buf);

			if (ck_query (MSGSTR (M_DELETE, DEF_DELETE), buf)) {
				users[current].usr_delete = 1;
				mk_audit_rec (AUDIT_OK,
					buf, "/etc/passwd", Fixed);
			} else {
				mk_audit_rec (AUDIT_FAIL,
					buf, "/etc/passwd", NotFixed);
			}
			current++;
			continue;
		}

		/*
		 * Parse the password line.  Make certain numbers
		 * are numbers and so on.  The UID, GID, and HOME
		 * must all be present [ as obviously must also be
		 * the name ... ]
		 */

		if (pwd_interpret (buf, &pw)) {
			if (! all) {
				if (! pw->pw_name || findname (pw->pw_name)) {
					current++;
					continue;
				}
			}
			errors++;
			if (ck_query (MSGSTR (M_DELETE, DEF_DELETE), buf)) {
				users[current].usr_delete = 1;
				mk_audit_rec (AUDIT_OK,
					buf, "/etc/passwd", Fixed);
			} else {
				mk_audit_rec (AUDIT_FAIL,
					buf, "/etc/passwd", NotFixed);
			}
			if (pw->pw_name == (char *) 0
					|| pw->pw_uid == (uid_t) -1
					|| pw->pw_gid == (gid_t) -1) {
				current++;
				continue;
			}
		}

		/*
		 * Now the name is checked for validity.  The name must
		 * match something like [a-z][a-z0-9]{0,7} as a regular
		 * expression.
		 */

		strcpy (name, pw->pw_name);

		if ((all || findname (name) == 0) && valid_name (name)) {
			errors++;
			msg1 (MSGSTR (M_BADNAME, DEF_BADNAME), name);
			if (ck_query (MSGSTR (M_DELETE, DEF_DELETE), buf)) {
				users[current].usr_delete = 1;
				mk_audit_rec (AUDIT_OK,
					buf, "/etc/passwd", Fixed);
			} else {
				mk_audit_rec (AUDIT_FAIL,
					buf, "/etc/passwd", NotFixed);
			}
			current++;
			continue;
		}

		/*
		 * Add the new entry, incrementing the pointer into
		 * the table.  current is always the first free space
		 * in the table.
		 */

		users[current].usr_name = dup_string (pw->pw_name);

		if (pw->pw_uid != (uid_t) -1) {
			users[current].usr_uid = pw->pw_uid;
			users[current].usr_gid = pw->pw_gid;
			users[current].usr_valid = 1;
		}
		current++;
	}
	fclose (fp);

	/*
	 * Trim the list down to size, saving the current size as
	 * 'nusers'.
	 */

	users = (struct users *) realloc ((void *) users,
			(size_t) sizeof *users * current);
	nusers = current;

	/*
	 * Now get the stanzas from /etc/security/limits and mark
	 * those users.
	 */

	if ((af = afopen ("/etc/security/limits")) == (AFILE_t) 0) {
		fprintf (stderr, MSGSTR (M_NOLIMFILE, DEF_NOLIMFILE));
		exit (ENOENT);
	}

	/*
	 * Read each stanza, reporting errors and marking users
	 * as we go.
	 */

	while (1) {
		errno = 0;
		if (afread (af) == (char *) 0) {
			if (errno == 0)
				break;

			continue;
		}

		/*
		 * First things first - ignore the default: stanza
		 */

		cp = af->AF_catr->AT_value;
		if (! strcmp (cp, "default"))
			continue;

		/*
		 * See if this is a user.  If so, mark the entry,
		 * otherwise, report the error.
		 */

		if ((i = finduser (cp)) >= 0) {
			if (users[i].usr_valid)
				users[i].usr_limits = 1;
		} else if (all) {
			msg1 (MSGSTR (M_UNKLIMIT, DEF_UNKLIMIT), cp);
			mk_audit_rec (AUDIT_FAIL, cp,
				"bad limits file entry", CantFix);
			errors++;
		}
	}
	afclose (af);

	/*
	 * Now get the stanzas from /etc/security/passwd and mark
	 * those users.
	 */

	if ((af = afopen ("/etc/security/passwd")) == (AFILE_t) 0) {
		fprintf (stderr, MSGSTR (M_NOSECPWDFILE, DEF_NOSECPWDFILE));
		exit (ENOENT);
	}

	/*
	 * Read each stanza, reporting errors and marking users
	 * as we go.
	 */

	while (1) {
		errno = 0;
		if (afread (af) == (char *) 0) {
			if (errno == 0)
				break;

			continue;
		}

		/*
		 * First things first - ignore the default: stanza
		 */

		cp = af->AF_catr->AT_value;
		if (! strcmp (cp, "default"))
			continue;

		/*
		 * See if this is a user.  If so, mark the entry,
		 * otherwise, report the error.
		 */

		if ((i = finduser (cp)) >= 0) {
			if (users[i].usr_valid)
				users[i].usr_passwd = 1;
		} else if (all) {
			msg1 (MSGSTR (M_UNKSECPWD, DEF_UNKSECPWD), cp);
			mk_audit_rec (AUDIT_FAIL, cp,
				"bad passwd file entry", CantFix);
			errors++;
		}
	}
	afclose (af);

	/*
	 * Now get the stanzas from /etc/security/user and mark
	 * those users.
	 */

	if ((af = afopen ("/etc/security/user")) == (AFILE_t) 0) {
		fprintf (stderr, MSGSTR (M_NOSECUSERFILE, DEF_NOSECUSERFILE));
		exit (ENOENT);
	}

	/*
	 * Read each stanza, reporting errors and marking users
	 * as we go.
	 */

	while (1) {
		errno = 0;
		if (afread (af) == (char *) 0) {
			if (errno == 0)
				break;

			continue;
		}

		/*
		 * Skip the default stanza.
		 */

		cp = af->AF_catr->AT_value;
		if (! strcmp (cp, "default"))
			continue;

		/*
		 * See if this is a user.  If so, mark the entry,
		 * otherwise, report the error.
		 */

		if ((i = finduser (cp)) >= 0) {
			if (users[i].usr_valid)
				users[i].usr_user = 1;
		} else if (all) {
			msg1 (MSGSTR (M_UNKSECUSER, DEF_UNKSECUSER), cp);
			mk_audit_rec (AUDIT_FAIL, cp,
				"bad user file entry", CantFix);
			errors++;
		}
	}
	afclose (af);

	/*
	 * Mark all users who are going to be tested and return
	 * error status.
	 */

	if (mark_users (names))
		return ENOENT;

	/*
	 * Now go back through and add stanzas for all the users
	 * who are going to be tested that don't have have stanzas
	 * in the limits or passwd file.
	 */

	for (usr = users;usr != &users[nusers];usr++) {

		/*
		 * Test for existence of a security/passwd file entry
		 */

		if (! usr->usr_passwd && usr->usr_valid && usr->usr_test) {

			/*
			 * This user has no passwd stanza.  Report this
			 * fact and see if the invoker wants to add a
			 * blank stanza for the user.
			 */

			msg1 (MSGSTR (M_NOSECPWD, DEF_NOSECPWD), usr->usr_name);
			if (! ck_query (MSGSTR (M_ADDSECPWD, DEF_ADDSECPWD),
					usr->usr_name)) {
				mk_audit_rec (AUDIT_FAIL, usr->usr_name,
					"add passwd file entry", NotFixed);
				errors++;
				continue;
			}
			memset ((void *) &upw, 0, sizeof upw);
			strcpy (upw.upw_name, usr->usr_name);

			/* 
			 * Move the passwd to the new stanza from the old
			 * passwd file entry.
			 */

			pwd_interpret (usr->usr_pwdline, &pw);
			if (strlen (pw->pw_passwd) != 1)
				upw.upw_passwd = pw->pw_passwd;
			else
				upw.upw_passwd = "*";

			/* 
			 * Write out the new record.
			 */

			setpwdb (S_READ|S_WRITE);
			if (putuserpw (&upw)) {
				mk_audit_rec (AUDIT_FAIL, usr->usr_name,
					"add passwd file entry", CantFix);
				perror ("usrck: putuserpw");
			} else {
				mk_audit_rec (AUDIT_OK, usr->usr_name,
					"add passwd file entry", Fixed);
				rm_passwd (usr->usr_pwdline);
				usr->usr_passwd = 1;
				usr->usr_change = 1;
			}
			endpwdb ();
			errors++;
		}
	}
	if (errors)
		return ENOTRUST;

	return 0;
}

/*
 * NAME: init_groups
 *
 * FUNCTION: Read in the entire group file saving group names.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Called once at command startup.
 *
 * NOTES:
 *	This is fairly straight forward.  The list of groups is
 *	read from the groups file.  A pointer to each name is
 *	saved and the resulting list sorted.
 *
 * RETURNS: NONE
 */

int
init_groups (void)
{
	int	current = 0;	/* Current number of groups in the table */
	int	errors = 0;	/* Count of errors parsing /etc/group */
	int	i;		/* Index into groups array. */
	int	false = 0;	/* Place to keep a zero */
	struct	group	*gr;	/* A group file entry pointer */
	FILE	*fp;		/* Pointer to group file */
	AFILE_t	af;		/* POinter to group stanza file */
	char	buf[BUFSIZ*8];	/* A BIG buffer for group file lines */
	char	*cp;

	/*
	 * read the entire group file, building our table as we go.
	 */

	if (! (fp = fopen ("/etc/group", "r"))) {
		fprintf (stderr, MSGSTR (M_NOGRPFILE, DEF_NOGRPFILE));
		exit (ENOENT);
	}
	while (1) {

		/*
		 * See if there is any room in the table for the
		 * new entry.  Double the amount of space if we
		 * just ran out.
		 */

		if (current == ngroups)
			expand_groups ();

		/*
		 * Get a line and parse it
		 */

		if (fgets (buf, (int) sizeof buf, fp) == (char *) 0)
			break;

		/*
		 * NUL terminate the line and initialize the groups
		 * structure for this line.
		 */

		buf[sizeof buf - 1] = '\0';
		if (cp = strrchr (buf, (int) '\n'))
			*cp = '\0';

		groups[current].grp_grpline = dup_string (buf);
		groups[current].grp_lineno = current;
		groups[current].grp_name = (char *) 0;
		groups[current].grp_gid = (gid_t) -1;
		groups[current].grp_group = 0;
		groups[current].grp_valid = 0;
		groups[current].grp_delete = 0;

		/*
		 * Look for Yellow Pages hieroglyphics.  The characters
		 * '-', '+' and '@' start YP entries.  Skip the entire
		 * line, it isn't worth getting upset about ...
		 */

		if (buf[0] == '-' || buf[0] == '+' || buf[0] == '@') {
			current++;
			yp_grp_entries = 1;
			continue;
		}

		/*
		 * Parse the line into group file fields.
		 */

		if (grp_interpret (buf, &gr)) {
			if (! all) {
				current++;
				continue;
			}
			errors++;
			if (ck_query (MSGSTR (M_DELETE, DEF_DELETE), buf)) {
				groups[current].grp_delete = 1;
				mk_audit_rec (AUDIT_OK,
					buf, "/etc/group", Fixed);
			} else {
				mk_audit_rec (AUDIT_FAIL,
					buf, "/etc/group", NotFixed);
			}
			if (gr->gr_name == (char *) 0) {
				current++;
				continue;
			}
		}

		/*
		 * Add the new group entry into the table
		 */

		groups[current].grp_name = strdup (gr->gr_name);

		if (gr->gr_gid != (gid_t) -1) {
			groups[current].grp_gid = gr->gr_gid;
			groups[current].grp_valid = 1;
		}
		current++;
	}

	/*
	 * Trim the list down to size, saving the current size as
	 * 'ngroups'.
	 */

	groups = (struct groups *) realloc ((void *) groups,
			(size_t) (sizeof (struct groups)) * current);
	ngroups = current;

	/*
	 * Now sort the list to make it easier to search.
	 */

	qsort ((void *) groups, (size_t) ngroups, (size_t) sizeof *groups,
		(int (*) (const void *, const void *)) grpcmp);

	/*
	 * If this isn't ALL mode, I'm done - just return the number
	 * of errors I've counted up.
	 */

	if (! all)
		return errors ? ENOTRUST:0;

	/*
	 * This is ALL mode, all of the groups must have entries in
	 * /etc/security/group, otherwise, entries will be added.
	 */

	if (! (af = afopen ("/etc/security/group"))) {
		msg (MSGSTR (M_NOSECGROUPFILE, DEF_NOSECGROUPFILE));
		errors++;
	} else {
		while (1) {
			errno = 0;
			if (! afread (af)) {
				if (errno == 0)
					break;

				continue;
			}

			/*
			 * Skip the default stanza
			 */

			cp = af->AF_catr->AT_value;
			if (! strcmp (cp, "default"))
				continue;

			/*
			 * Locate the group element and flag it as having
			 * group stanza, or report it as an error.
			 */

			if ((i = findgroup (cp)) >= 0) {
				groups[i].grp_group = 1;
				continue;
			}
			msg1 (MSGSTR (M_UNKGROUP, DEF_UNKGROUP), cp);
			mk_audit_rec (AUDIT_FAIL, cp,
				"bad group file entry", CantFix);
			errors++;
		}
		afclose (af);
	}

	/* 
	 * Now go back and make certain every group in /etc/group has a
	 * matching stanza in /etc/security/group
	 */

	if (! nflg)
		setuserdb (S_READ|S_WRITE);
	else
		setuserdb (S_READ);

	for (i = 0;i < ngroups;i++) {
		if (! groups[i].grp_valid || groups[i].grp_group)
			continue;

		/*
		 * This user has no group stanza.  See if I should add one.
		 */

		errors++;
		msg1 (MSGSTR (M_NOSECGROUP, DEF_NOSECGROUP),
			groups[i].grp_name);

		if (! ck_query (MSGSTR (M_ADDGROUP, DEF_ADDGROUP),
				groups[i].grp_name)) {
			mk_audit_rec (AUDIT_FAIL, groups[i].grp_name,
				"add group file entry", NotFixed);
			continue;
		}

		/*
		 * Add the new group stanza and audit the update.
		 */

		putgroupattr (groups[i].grp_name,
			S_ADMIN, (void *) 0, SEC_DELETE);
		putgroupattr (groups[i].grp_name,
			S_ADMIN, (void *) "false", SEC_BOOL);
		mk_audit_rec (AUDIT_OK, groups[i].grp_name,
			"add group file entry", Fixed);
	}
	if (errors)
		putgroupattr ((char *) 0, (char *) 0, (void **) 0, SEC_COMMIT);

	return errors ? ENOTRUST:0;
}
