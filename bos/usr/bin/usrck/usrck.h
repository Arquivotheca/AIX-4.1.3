/* @(#)43	1.14.1.4  src/bos/usr/bin/usrck/usrck.h, cmdsadm, bos411, 9428A410j 10/27/93 15:26:04 */
/*
 *   COMPONENT_NAME: CMDSADM
 *
 *   FUNCTIONS: MSGSTR
 *		msg
 *		msg1
 *		msg2
 *		msg3
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define	MSGSTR(Num, Str) usrckcatgets (Num, Str)

#define	DEF_USAGE \
     "Usage: usrck [ -p | -y | -n | -t ] [ user ... | ALL ]"
#define	DEF_DUPNAME \
     "The user name %s is not unique.\n"
#define	DEF_DUPUID \
     "The UID %d is duplicated for user %s.\n"
#define	DEF_BADGET \
     "Error getting user %s's attribute %s.\n\
         You must add this attribute with the chuser command.\n"
#define	DEF_BADPUT \
     "Error updating user %s's attribute %s.\n"
#define	DEF_NOMEM \
     "Unable to allocate memory.\n"
#define	DEF_NOPGRP \
     "User %s has a non-existent primary group %lu.\n"
#define	DEF_NOGROUP \
     "User %s has a non-existent concurrent group %s.\n"
#define	DEF_NOADMGRP \
     "User %s has a non-existent administrative group %s.\n"
#define	DEF_NOSUGRP \
     "User %s has a non-existent sugroups entry %s.\n"
#define	DEF_NOSHELL \
     "User %s has a non-existent\n\
         or nonexecutable login shell %s.\n"
#define	DEF_NOHOME \
     "User %s has a non-existent or inaccessible\n\
         home directory %s.\n"
#define	DEF_NOTTY \
     "User %s has a non-existent login port %s.\n"
#define	DEF_NOAUDIT \
     "User %s has a non-existent audit class %s.\n"
#define	DEF_NOTPATH \
     "User %s does not have a trusted login shell.\n"
#define	DEF_RESOURCE \
     "User %s has resource limit %s less than %d.\n"
#define DEF_BADPWD \
     "\"%s\"\n\
	 is an invalid password file entry.\n"
#define	DEF_NOUSER \
     "%s is a non-existent user.\n"
#define	DEF_NOPWDFILE \
     "Unable to open the system password file.\n"
#define	DEF_BADAUTH \
     "User %s has an invalid authentication\n\
         method %s.\n"
#define	DEF_BADPROG \
     "The authentication program %s\n\
         for user %s does not exist or is nonexecutable.\n"
#define	DEF_BADNAME \
     "User %s is an invalid name.\n"
#define	DEF_BADTPATH \
     "User %s has an invalid tpath value %s.\n"
#define	DEF_DISABLE \
     "Disable account for %s? "
#define	DEF_FIXGRPS \
     "Remove non-existent groups for %s? "
#define	DEF_FIXAGRPS \
     "Remove non-existent admgroups for %s? "
#define	DEF_FIXSGRPS \
     "Remove non-existent sugroups for %s? "
#define	DEF_FIXCLASS \
     "Remove non-existent audit classes for %s? "
#define	DEF_FIXTTYS \
     "Remove non-existent login ports for %s? "
#define	DEF_FIXLIMIT \
     "Reset %s to default value? "
#define DEF_NONAME \
     "Invalid or missing user name on password line\n\
         %s.\n"
#define DEF_BADUID \
     "Invalid or missing UID on password line\n\
	 %s.\n"
#define DEF_BADGID \
     "Invalid or missing GID on password line\n\
	 %s.\n"
#define DEF_BADHOME \
     "Invalid or missing HOME on password line\n\
 	 %s.\n"
#define DEF_DELETE \
     "Remove the line beginning with\n         %s? "
#define DEF_PWDREOPEN \
     "Error re-opening the file /etc/passwd.\n"
#define	DEF_OLDDBM \
     "The DBM files for /etc/passwd are out of date.\n\
         Use the mkpasswd command to update these files.\n"
#define DEF_UNKLIMIT \
     "The user name %s appears in /etc/security/limits\n\
         but not in /etc/passwd.\n"
#define	DEF_NOLIMFILE \
     "The file /etc/security/limits does not exist.\n"
#define	DEF_NOLIMIT \
     "The user %s has no stanza in /etc/security/limits.\n"
#define DEF_ADDLIMIT \
     "Add stanza for user %s to /etc/security/limits? "
#define DEF_UNKSECPWD \
     "The user name %s appears in /etc/security/passwd\n\
         but not in /etc/passwd.\n"
#define	DEF_NOSECPWDFILE \
     "The file /etc/security/passwd cannot be opened.\n"
#define	DEF_NOSECPWD \
     "The user %s has no stanza in /etc/security/passwd.\n"
#define DEF_ADDSECPWD \
     "Add stanza for user %s to /etc/security/passwd? "
#define DEF_UNKSECUSER \
     "The user name %s appears in /etc/security/user\n\
         but not in /etc/passwd.\n"
#define	DEF_NOSECUSERFILE \
     "The file /etc/security/user cannot be opened.\n"
#define	DEF_NOSECUSER \
     "The user %s has no stanza in /etc/security/user.\n"
#define DEF_ADDSECUSER \
     "Add stanza for user %s to /etc/security/user? "
#define DEF_NOGRPFILE \
     "Unable to open the system group file.\n"
#define DEF_NOGRPNAME \
     "Missing group name on group line\n\
         %s.\n"
#define DEF_BADGRPGID \
     "Invalid or missing GID on group line\n\
	 %s.\n"
#define	DEF_NOSECGROUPFILE \
     "The file /etc/security/group cannot be opened.\n"
#define	DEF_NOSECGROUP \
     "The group %s has no stanza in /etc/security/group.\n"
#define	DEF_ADDGROUP \
     "Add stanza for group %s to /etc/security/group? "
#define DEF_UNKGROUP \
     "The group name %s appears in /etc/security/group\n\
         but not in /etc/group.\n"
#define DEF_CREDFAIL \
     "Unable to set process credentials for user %s.\n"
#define DEF_ASSYPGRP \
     "User %s has a group %s that does not exist, assuming\n\
         the group to be a Network Information Services group name.\n"
#define DEF_BADGRAMMAR \
     "User %s has an invalid\n\
         authentication grammar \"%s\".\n"
#define DEF_BADREGISTRY \
     "User %s has an invalid\n\
         authentication registry \"%s\".\n"
#define DEF_BADLOGTIMES \
     "User %s has an invalid logintimes value.\n"
#define DEF_TOOMANYBAD \
     "There have been too many invalid login attempts by user %s.\n"
#define DEF_LOCKED \
     "User %s is locked.\n"
#define DEF_BADEXPIRES \
     "User %s has an invalid expires value of %s.\n"
#define DEF_EXPIRED \
     "The account for user %s has expired.\n"
#define DEF_WARNTIME \
     "User %s's pwdwarntime occurs before minage.\n"
#define DEF_FIXWARNTIME \
     "Reduce pwdwarntime to minage? "
#define	DEF_PWDLOW \
     "User %s's %s is less than %d.\n"
#define	DEF_FIXPWDLOW \
     "Reset %s to the lowest allowed value? "
#define	DEF_PWDHIGH \
     "User %s's %s is larger than %d.\n"
#define	DEF_FIXPWDHIGH \
     "Reset %s to the highest allowed value? "
#define	DEF_MINAGE \
     "User %s's minage is greater than maxage.\n"
#define	DEF_FIXMINAGE \
     "Reduce minage to maxage? "
#define	DEF_MINOTHER \
     "User %s's minother is greater than the\n\
         maximum password length minus minalpha.\n"
#define	DEF_FIXMINOTHER \
     "Reduce minother? "
#define	DEF_BADFILE \
     "User %s's %s attribute contains\n\
         a bad file name \"%s\".\n"
#define	DEF_NOACCESS \
     "User %s's %s attribute contains\n\
         an inaccessible file \"%s\".\n"
#define	DEF_ABSPATH \
     "User %s's %s attribute contains\n\
         \"%s\" which is not an absolute path name.\n"
#define	DEF_FIXFILE \
     "Remove the file name from %s? "


#define	MAXATTRSIZ	4096
#define	MIN_FSIZE	200
#define	MIN_CPU		120
#define	MIN_DATA	128
#define	MIN_STACK	128
#define	MIN_RSS		128
#define	MIN_CORE	200

struct	users {
	char	*usr_name;	/* id of user */
	uid_t	usr_uid;	/* uid of user */
	gid_t	usr_gid;	/* gid of user */
	char	*usr_pwdline;	/* pointer to line from /etc/passwd */
	char	usr_disable;	/* flag indicating account is to be disabled */
	char	usr_test;	/* flag indicating user is to be tested */
	char	usr_delete;	/* flag indicating line is to be deleted */
	char	usr_change;	/* flag indicating line has been changed */
	char	usr_valid;	/* flag indicating passwd line is valid */
	char	usr_limits;	/* flag indicating limits stanza exists */
	char	usr_passwd;	/* flag indicating passwd stanza exists */
	char	usr_user;	/* flag indicating user stanza exists */
};

struct	groups {
	char	*grp_name;	/* id of group */
	gid_t	grp_gid;	/* gid of group */
	char	*grp_grpline;	/* pointer to line from /etc/group */
	char	*grp_lineno;	/* line number in /etc/group file */
	char	grp_group;	/* flag indicating group stanza exists */
	char	grp_valid;	/* flag indicating group line is valid */
	char	grp_delete;	/* flag indicating line is to be deleted */
};

char	*usrckcatgets (int, char *);
int	init_users (char **);
int	init_groups (void);
int	ck_name (char *);
int	ck_uid (uid_t);
int	ck_pgrp (struct users *);
int	ck_groups (char *);
void	fix_groups (char *);
int	ck_admgroups (struct users *);
void	fix_admgroups (char *);
int	ck_audit (char *);
void	fix_audit (char *);
int	ck_home (char *);
int	ck_shell (char *);
int	ck_tpath (char *);
int	ck_ttys (char *);
void	fix_ttys (char *);
int	ck_sugroups (struct users *);
void	fix_sugroups (char *);
int	ck_auth (struct users *, char *);
int	ck_resource (struct users *);
int	ck_disabled (char *);
int	mk_disabled (char *);
int	strpcmp (void *, void *);
int	ck_query (char *, char *);
void	mk_audit_rec (int, char *, char *, char *);
int	ck_authsystem(struct users *);
int	ck_registry(struct users *);
int	ck_logintimes (char *);
int	ck_logretries (char *);
int	ck_expires (char *);
int	ck_pwdwarntime (char *);
int	ck_pwdrestrictions (char *);

/*
 * Macros for message output
 */

#define	msg(t)		(void) (verbose && fprintf (stderr, t))
#define	msg1(t,a)	(void) (verbose && fprintf (stderr, t, a))
#define	msg2(t,a,b)	(void) (verbose && fprintf (stderr, t, a, b))
#define	msg3(t,a,b,c)	(void) (verbose && fprintf (stderr, t, a, b, c))

/*
 * Character strings for audit messages
 */

extern	char	*USER_Check;
extern	char	*Fixed;
extern	char	*NotFixed;
extern	char	*CantFix;
extern	char	*PartiallyFixed;
