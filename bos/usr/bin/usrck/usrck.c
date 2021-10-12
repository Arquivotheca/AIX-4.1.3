static char sccsid[] = "@(#)42	1.14.1.5  src/bos/usr/bin/usrck/usrck.c, cmdsadm, bos411, 9428A410j 2/23/94 09:44:48";
/*
 *   COMPONENT_NAME: CMDSADM
 *
 *   FUNCTIONS: main
 *		usage
 *		usrckcatgets
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/audit.h>
#include <sys/acl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <usersec.h>
#include <locale.h>
#include "usrck_msg.h"
#include "usrck.h"

/*
 * Global data
 */

int	fixit;			/* indicates fixes are to be made always    */
int	verbose;		/* indicates error messages are displayed   */
int	query;			/* indicates fixes are prompted for         */
int	all;			/* all users are to be tested               */

int	pflg;			/* '-p' was seen on command line            */
int	yflg;			/* '-y' was seen on command line            */
int	nflg;			/* '-n' was seen on command line            */
int	tflg;			/* '-t' was seen on command line            */

int	error;			/* global error counter                     */

struct	users	*users;		/* table of users and flags                 */
int	nusers;			/* number of users in users table           */

struct	groups	*groups;	/* list of groups in groups file            */
int	ngroups;		/* number of groups in the table            */

int	yp_grp_entries;		/* true if any YP entries in /etc/group     */
nl_catd	catd;

/*
 * Strings for audit messages
 */

char	*USER_Check = "USER_Check";
char	*Fixed = "fixed";
char	*NotFixed = "not fixed";
char	*CantFix = "could not fix";
char	*PartiallyFixed = "partially fixed";

char	*usrckcatgets (int, char *);
int	intcmp (const void *, const void *);

/*
 * NAME: main
 *
 * FUNCTION: Main for usrck
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User level command.
 *
 * RETURNS: Exits with 0 if everything was OK, various error codes otherwise
 */

main (argc, argv)
int	argc;
char	**argv;
{
	char	*name;		/* name of the user currently being checked */
	int	flag;		/* next argument to be processed            */
	int	i;		/* counter to step through users table      */
	int	user_errors;	/* count of errors on this user             */
	int	changed = 0;	/* putuserattr was called for any user      */
	extern	int	optind;	/* index of argument after flags            */
	struct	acl	*acl;	/* acl from old /etc/passwd                 */
	struct	acl	*acl_get();
	struct	acl	*acl_put();
	struct	stat	pwd1;
	struct	stat	pwd2;
	FILE	*ofp;		/* fp for old /etc/passwd                   */
	FILE	*nfp;		/* fp for new /etc/passwd                   */
	int	c;		/* character to copy                        */
	int	rc;		/* exit code                                */

	setlocale (LC_ALL, "");

	auditproc (0, AUDIT_STATUS, AUDIT_SUSPEND, 0);
	privilege (PRIV_LAPSE);

	/*
	 * Begin by checking the arguments and setting up a pointer to the
	 * list of user names to check.
	 */

	while ((flag = getopt (argc, argv, "pynt")) != EOF) {
		switch (flag) {
			case 'p':
				fixit = 1;
				pflg++;
				break;
			case 'y':
				verbose = 1;
				fixit = 1;
				yflg++;
				break;
			case 'n':
				verbose = 1;
				nflg++;
				break;
			case 't':
				verbose = 1;
				query = 1;
				tflg++;
				break;
			default:
				usage ();
		}
	}

	/* 
	 * Validate the values for pflg, yflg, and nflg.  Also check
	 * that at least one user argument is supplied.
	 */

	if (pflg + yflg + nflg + tflg != 1 || optind >= argc)
		usage ();

	/*
	 * Open the user attribute file.  If we might change something
	 * we have to open it for writing as well.
	 */

	if (setuserdb ((fixit || query) ? (S_READ|S_WRITE):(S_READ)))
		exit (EACCES);

	/*
	 * Read the entire password file worth of users into a table
	 * for testing.  The users to check are flagged for later
	 * reference.  Then read in the entire groups file.  It will
	 * be used to validate the existence of groups which are being
	 * tested for.
	 *
	 * If any of the named users were not found, give up early.
	 */

	if (rc = init_users (&argv[optind])) {
		if (rc == ENOENT)
			goto out;

		error++;
	}
	if (rc = init_groups ())
		error++;

	/*
	 * Every user to be tested has been flagged.  Loop through the
	 * entire table and perform all of the required tests on the
	 * user.
	 */

	for (i = 0;i < nusers;i++) {

		if (! users[i].usr_valid || users[i].usr_delete)
			continue;

		user_errors = 0;
		name = users[i].usr_name;

		/*
		 * See if we are testing this user.  If not, then just
		 * ignore them and go on to the next one.
		 */

		if (! users[i].usr_test)
			continue;

		/*
		 * See if this user is locked.  If so, tell the admin that
		 * the user is locked and go on to the next user.
		 */

		if (ck_disabled (name))
		{
			msg1 (MSGSTR (M_LOCKED, DEF_LOCKED), name);
			continue;
		}

		/*
		 * Check the validity and uniqueness of a user name.
		 * Disable the account if it is either invalid or
		 * duplicated.  If the login name itself is invalid,
		 * abandon all further testing.
		 */

		if (ck_name (name)) {
			user_errors++;

			if ((users[i].usr_uid != 0) && (users[i].usr_disable ||
					ck_query (MSGSTR (M_DISABLE,
						DEF_DISABLE), name))) {
				mk_audit_rec (AUDIT_OK,
					name, "name", Fixed);
				users[i].usr_disable = 1;
			} else
				mk_audit_rec (AUDIT_FAIL,
					name, "name", NotFixed);

			error++;

			if (valid_name (name))
				continue;
		}

		/*
		 * Check the uniqueness of a UID.  Print a diagnostic
		 * message if a duplicate is found, but nothing more.
		 */

		if (ck_uid (users[i].usr_uid)) {
			msg2 (MSGSTR (M_DUPUID, DEF_DUPUID),
				users[i].usr_uid, name);

			mk_audit_rec (AUDIT_OK, name, S_ID, CantFix);
			error++;
		}

		/*
		 * Check the existence of this user's primary group
		 * ID.  Disable the account if it does not exist.
		 */

		if (ck_pgrp (&users[i])) {
			user_errors++;
			if ((users[i].usr_uid != 0) && (users[i].usr_disable ||
					ck_query (MSGSTR (M_DISABLE,
						DEF_DISABLE), name))) {
				mk_audit_rec (AUDIT_OK, name, S_PGRP, Fixed);
				users[i].usr_disable = 1;
			} else {
				mk_audit_rec (AUDIT_FAIL,
					name, S_PGRP, NotFixed);
			}
			error++;
		}

		/*
		 * Check regular groups for existence.  Delete any
		 * non-existent groups.
		 */

		if (ck_groups (name)) {
			user_errors++;
			if (ck_query (MSGSTR (M_FIXGRPS, DEF_FIXGRPS), name)) {
				fix_groups (name);
				mk_audit_rec (AUDIT_OK, name, S_GROUPS, Fixed);
			} else {
				mk_audit_rec (AUDIT_FAIL,
					name, S_GROUPS, NotFixed);
			}
			error++;
		}

		/*
		 * Check administrative groups for existence.  Delete
		 * any non-existent groups.
		 */

		if (ck_admgroups (&users[i])) {
			user_errors++;
			if (ck_query (MSGSTR (M_FIXAGRPS,DEF_FIXAGRPS), name)) {
				fix_admgroups (name);
				mk_audit_rec (AUDIT_OK,
					name, S_ADMGROUPS, Fixed);
			} else {
				mk_audit_rec (AUDIT_FAIL,
					name, S_ADMGROUPS, NotFixed);
			}
			error++;
		}

		/*
		 * Check audit classes for existence.  Delete any
		 * non-existent audit classes.
		 */

		if (ck_audit (name)) {
			user_errors++;
			if (ck_query (MSGSTR (M_FIXCLASS,DEF_FIXCLASS), name)) {
				fix_audit (name);
				mk_audit_rec (AUDIT_OK,
					name, S_AUDITCLASSES, Fixed);
			} else {
				mk_audit_rec (AUDIT_FAIL,
					name, S_AUDITCLASSES, NotFixed);
			}
			error++;
		}

		/*
		 * The user must have a home directory which is
		 * accessible.  It must at least be read and
		 * executable, but not necessarily writable ...
		 */

		if (ck_home (name)) {
			user_errors++;
			if ((users[i].usr_uid != 0) && (users[i].usr_disable ||
				ck_query (MSGSTR (M_DISABLE,
					DEF_DISABLE), name))) {
				mk_audit_rec (AUDIT_OK, name, S_HOME, Fixed);
				users[i].usr_disable = 1;
			} else {
				mk_audit_rec (AUDIT_FAIL,
					name, S_HOME, NotFixed);
			}
			error++;
		}

		/*
		 * The user must have a valid login shell.  Check
		 * the shell for existence and executability by the
		 * user.
		 */

		if (ck_shell (name)) {
			user_errors++;
			if ((users[i].usr_uid != 0) && (users[i].usr_disable ||
					ck_query (MSGSTR (M_DISABLE,
						DEF_DISABLE), name))) {
				mk_audit_rec (AUDIT_OK, name, S_SHELL, Fixed);
				users[i].usr_disable = 1;
			} else {
				mk_audit_rec (AUDIT_FAIL,
					name, S_SHELL, NotFixed);
			}
			error++;
		}

		/*
		 * If the user is on the trusted path their login
		 * shell must be marked as a trusted process.
		 */

		if (ck_tpath (name)) {
			user_errors++;

			if ((users[i].usr_uid != 0) && (users[i].usr_disable ||
				ck_query (MSGSTR (M_DISABLE,
					DEF_DISABLE), name))) {
				mk_audit_rec (AUDIT_OK, name, S_TPATH, Fixed);
				users[i].usr_disable = 1;
			} else {
				mk_audit_rec (AUDIT_FAIL,
					name, S_TPATH, NotFixed);
			}
			error++;
		}

		/*
		 * Check the users valid login terminals for existence
		 * and remove the entries for the ones which don't exist
		 */

		if (ck_ttys (name)) {
			user_errors++;

			if (ck_query (MSGSTR (M_FIXTTYS, DEF_FIXTTYS), name)) {
				fix_ttys (name);
				mk_audit_rec (AUDIT_OK, name, S_TTYS, Fixed);
			} else {
				mk_audit_rec (AUDIT_FAIL,
					name, S_TTYS, NotFixed);
			}
			error++;
		}

		/*
		 * Check the groups which are permitted to switch user
		 * to this account.  Any groups which don't exist are
		 * removed.
		 */

		if (ck_sugroups (&users[i])) {
			user_errors++;

			if (ck_query (MSGSTR (M_FIXSGRPS, DEF_FIXSGRPS), name)){
				fix_sugroups (name);
				mk_audit_rec (AUDIT_OK,
					name, S_SUGROUPS, Fixed);
			} else {
				mk_audit_rec (AUDIT_FAIL,
					name, S_SUGROUPS, NotFixed);
			}
			error++;
		}

		/*
		 * Check the primary authentication method.  If the
		 * test fails the account is to be disabled.
		 */

		if ((rc = ck_auth (&users[i], S_AUTH1)) == -1) {
			user_errors++;

			if ((users[i].usr_uid != 0) && (users[i].usr_disable ||
				ck_query (MSGSTR (M_DISABLE,
					DEF_DISABLE), name))) {
				mk_audit_rec (AUDIT_OK, name, S_AUTH1, Fixed);
				users[i].usr_disable = 1;
			} else {
				mk_audit_rec (AUDIT_FAIL,
					name, S_AUTH1, NotFixed);
			}
			error++;
		} else if (rc == ENOTRUST) {
			user_errors++;
			error++;
		}

		/*
		 * Check the secondary authentication method.  It
		 * must also be valid [ if it exist ], but the account
		 * is not disabled if an error is found.
		 */

		if (rc = ck_auth (&users[i], S_AUTH2)) {
			if (rc != ENOTRUST)
				mk_audit_rec (AUDIT_FAIL, name, S_AUTH2,
					CantFix);

			error++;
		}

		/*
		 * Check the system authentication grammar.  If the
		 * test fails the account is to be disabled.
		 */
		if ((rc = ck_authsystem(&users[i])) == -1) 
		{
			user_errors++;

			if ((users[i].usr_uid != 0) && (users[i].usr_disable ||
				ck_query(MSGSTR(M_DISABLE, DEF_DISABLE), name)))
			{	
				mk_audit_rec(AUDIT_OK,name, S_AUTHSYSTEM,Fixed);
				users[i].usr_disable = 1;
			} 
			else 
				mk_audit_rec(AUDIT_FAIL, name, S_AUTHSYSTEM, 
					NotFixed);

			error++;
		}

		/*
		 * Check the authentication registry.  If the
		 * test fails the account is to be disabled.
		 */
		if ((rc = ck_registry(&users[i])) == -1) 
		{
			user_errors++;

			if ((users[i].usr_uid != 0) && (users[i].usr_disable ||
				ck_query(MSGSTR(M_DISABLE, DEF_DISABLE), name)))
			{	
				mk_audit_rec(AUDIT_OK,name, S_REGISTRY,Fixed);
				users[i].usr_disable = 1;
			} 
			else 
				mk_audit_rec(AUDIT_FAIL, name, S_REGISTRY, 
					NotFixed);

			error++;
		}

		/*
		 * Check the resource limits for minimum reasonable
		 * values.
		 */

		if (ck_resource (&users[i])) {
			user_errors++;
			error++;
		}

		/*
		 * Check the logintimes attribute.  Disable the user if the
		 * logintimes attribute is invalid.
		 */

		if (ck_logintimes (name)) {
			user_errors++;

			if ((users[i].usr_uid != 0) && (users[i].usr_disable ||
				ck_query (MSGSTR (M_DISABLE,
					DEF_DISABLE), name))) {
				mk_audit_rec (AUDIT_OK, name, S_LOGTIMES,
					Fixed);
				users[i].usr_disable = 1;
			} else {
				mk_audit_rec (AUDIT_FAIL, name, S_LOGTIMES,
					NotFixed);
			}
			error++;
		}

		/*
		 * Check the number of unsuccessful login attempts.  Disable
		 * the account if there have been too many.
		 */

		if (ck_logretries (name)) {
			user_errors++;

			if ((users[i].usr_uid != 0) && (users[i].usr_disable ||
				ck_query (MSGSTR (M_DISABLE,
					DEF_DISABLE), name))) {
				mk_audit_rec (AUDIT_OK, name, S_LOGRETRIES,
					Fixed);
				users[i].usr_disable = 1;
			} else {
				mk_audit_rec (AUDIT_FAIL, name, S_LOGRETRIES,
					NotFixed);
			}
			error++;
		}

		/*
		 * Check to see if the user's account has expired.  Disable the
		 * account if it has expired.
		 */

		if (ck_expires (name)) {
			user_errors++;

			if ((users[i].usr_uid != 0) && (users[i].usr_disable ||
				ck_query (MSGSTR (M_DISABLE,
					DEF_DISABLE), name))) {
				mk_audit_rec (AUDIT_OK, name, S_EXPIRATION,
					Fixed);
				users[i].usr_disable = 1;
			} else {
				mk_audit_rec (AUDIT_FAIL, name, S_EXPIRATION,
					NotFixed);
			}
  			error++;
  		}

		/*
		 * Check all of the password restrictions for this user.
		 */
		if (ck_pwdrestrictions (name)) {
			user_errors++;
			/* ck_pwdrestrictions() updates "error" automatically */
		}

		/*
		 * Check the password expiration warning value.
		 */

		if (ck_pwdwarntime (name))
		{
			user_errors++;
			error++;
		}

  
		/*
		 * Update the user database for this user.  If the
		 * account is going to be disabled do it now ...
		 */

		if ((fixit || query) && user_errors) {
			if (users[i].usr_disable)
				mk_disabled (name);

			changed = 1;
		}
	}

	/*
	 * See if the password file has to be replaced because any of the
	 * lines are to be deleted, or any of the lines have changed.
	 */

	for (i = 0;i < nusers;i++)
		if (users[i].usr_delete || users[i].usr_change)
			changed = 1;

	/*
	 * Update the user database files that may have changed.
	 */

	if (changed)
		putuserattr ((char *) 0, (char *) 0, (void *) 0, SEC_COMMIT);

	/*
	 * Say an error occured
	 */

	if (error || changed)
		rc = ENOTRUST;

	if (changed) {

	/*
	 * One or more of the lines in the password file need to be deleted
	 * so I'll make a copy, and output only the lines that =don't= get
	 * deleted.
	 *
	 * I can get here either by checking for lines to have been deleted
	 * after successfully checking one or more users, or after the
	 * initialization of the user table [ init_users() ] failed.
	 */

out:
		umask ((mode_t) 077);

		if ((ofp = fopen ("/etc/passwd", "r+")) == 0) {
			perror ("/etc/passwd");
			exit (errno);
		}
		if ((nfp = fopen ("/etc/opasswd", "w")) == 0) {
			perror ("/etc/opasswd");
			exit (errno);
		}
		acl = acl_get ("/etc/passwd");
		acl_put ("/etc/opasswd", acl, 0);

		while ((c = getc (ofp)) != EOF)
			putc (c, nfp);

		if (ferror (ofp) || ferror (nfp)) {
			perror ("copy");
			exit (-1);
		}
		fclose (ofp);
		if ((ofp = fopen ("/etc/passwd", "w")) == 0) {
			rename ("/etc/opasswd", "/etc/passwd");
			fprintf (stderr, MSGSTR (M_PWDREOPEN, DEF_PWDREOPEN));
			exit (errno);
		}
		acl_put ("/etc/passwd", acl, 0);

		for (i = 0;i < nusers;i++)
			if (users[i].usr_delete == 0)
				fputs (users[i].usr_pwdline, ofp);
	}

	/*
	 * See if the password file has to be replaced because any of the
	 * lines are to be deleted, or any of the lines have changed.
	 */

	for (i = 0;i < ngroups;i++)
		if (groups[i].grp_delete)
			changed = 1;

	if (changed) {
	
	/*
	 * Put the group entries back in their original order.
	 */

		qsort ((void *) groups, (size_t) ngroups,
			(size_t) sizeof *groups,
			(int (*) (const void *, const void *)) intcmp);

	/*
	 * One or more of the lines in the group file need to be deleted
	 * so I'll make a copy, and output only the lines that =don't= get
	 * deleted.
	 */

		umask ((mode_t) 077);

		if ((ofp = fopen ("/etc/group", "r+")) == 0) {
			perror ("/etc/group");
			exit (errno);
		}
		if ((nfp = fopen ("/etc/ogroup", "w")) == 0) {
			perror ("/etc/ogroup");
			exit (errno);
		}
		acl = acl_get ("/etc/group");
		acl_put ("/etc/ogroup", acl, 0);

		while ((c = getc (ofp)) != EOF)
			putc (c, nfp);

		if (ferror (ofp) || ferror (nfp)) {
			perror ("copy");
			exit (-1);
		}
		fclose (ofp);
		if ((ofp = fopen ("/etc/group", "w")) == 0) {
			rename ("/etc/ogroup", "/etc/group");
			fprintf (stderr, MSGSTR (M_NOGRPFILE, DEF_NOGRPFILE));
			exit (errno);
		}
		acl_put ("/etc/group", acl, 0);

		for (i = 0;i < ngroups;i++)
			if (groups[i].grp_delete == 0)
				fprintf (ofp, "%s\n", groups[i].grp_grpline);
	}

	/*
	 * Now see if the password file is newer than the look-aside
	 * files.  This will cause a warning message with no action
	 * taken.
	 */

	if (stat ("/etc/passwd", &pwd1))
		exit (rc);

	if (stat ("/etc/security/passwd", &pwd2) == 0) {
		if (pwd1.st_mtime < pwd2.st_mtime)
			pwd1 = pwd2;
	}
	if (stat ("/etc/passwd.pag", &pwd2) == 0 &&
			pwd1.st_mtime > pwd2.st_mtime)
		msg (MSGSTR (M_OLDDBM, DEF_OLDDBM));

	/*
	 * There had to have been some error, so I exit with an error code
	 * automatically.
	 */

	exit (rc);
}

/*
 * NAME: usage
 *
 * FUNCTION: Print usage message and exit with EINVAL
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * RETURNS: This routine does not return.
 */

usage ()
{
	fprintf (stderr, "%s\n", MSGSTR (M_USAGE, DEF_USAGE));
	exit (EINVAL);
}


/*
 * NAME: usrckcatgets
 *
 * FUNCTION: Get a string from the USRCK message catalog
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * NOTES:
 *	This routine returns a pointer to a message string.  If
 *	the catalog is unopened, an attempt is made to open it.
 *	If the open fails all future messages will be from the
 *	default built-in values.
 *
 * RETURNS: Pointer to message text or NULL on error.
 */

char *
usrckcatgets (int Num, char *Str)
{
	static	int	once;	/* counter for NLS initialization */
/*
	static  nl_catd	catd;
i*/
	char	*catgets(nl_catd, int, int, char *);

	if (! once) {
		catd =catopen (MF_USRCK, NL_CAT_LOCALE);
		once++;
	}
	if (catd != (nl_catd) -1)
		return catgets (catd, MS_USRCK, Num, Str);
	else
		return Str;
}
