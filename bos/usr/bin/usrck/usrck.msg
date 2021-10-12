$
$   COMPONENT_NAME: CMDSADM
$
$   FUNCTIONS: none
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "

$set MS_SCCSID

$ This is the SCCSID string.  It needs to be a message so it will
$ show-up as the output of the what command.

M_SCCSID	"@(#)44	1.9.3.1  src/bos/usr/bin/usrck/usrck.msg, cmdsadm, bos411, 9435B411a 8/30/94 16:15:20"

$set MS_USRCK

$
$ Messages for the USRCK command
$

M_USAGE		"Usage: usrck [ -p | -n | -y | -t ] [ user ... | ALL ]"
M_DUPNAME	"3001-602 The user name %s is not unique.\n"
M_DUPUID	"3001-603 The UID %d is duplicated for user %s.\n"
M_BADGET	"3001-604 Error getting user \"%1$s\"'s attribute %2$s.\n\
         You must add this attribute with the chuser command.\n"
M_BADPUT	"3001-605 Error updating user %s's attribute %s.\n"
M_NOMEM		"3001-606 Unable to allocate memory.\n"
M_NOPGRP	"3001-607 User %s has a non-existent primary group %lu.\n"
M_NOGROUP	"3001-608 User %s has a non-existent concurrent group\n\
         %s.\n"
M_NOADMGRP	"3001-609 User %s has a non-existent administrative\n\
         group %s.\n"
M_NOSUGRP	"3001-610 User %s has a non-existent sugroups entry\n\
         %s.\n"
M_NOSHELL	"3001-611 User %s has a non-existent\n\
	 or nonexecutable login shell %s.\n"
M_NOHOME	"3001-612 User %s has a non-existent\n\
         or inaccessible home directory %s.\n"
M_NOTTY		"3001-613 User %s has a non-existent login port %s.\n"
M_NOAUDIT	"3001-614 User %s has a non-existent audit class %s.\n"
M_NOTPATH	"3001-615 User %s does not have a trusted login shell.\n"
M_RESOURCE	"3001-616 User %s has resource limit %s less than %d.\n"
M_BADPWD	"3001-617 \"%s\"\n\
         is an invalid password file entry.\n"
M_NOUSER	"3001-618 %s is a non-existent user.\n"
M_NOPWDFILE	"3001-619 Unable to open the system password file.\n"
M_BADAUTH	"3001-620 User %s has an invalid\n\
         authentication method %s.\n"
M_BADPROG	"3001-621 The authentication program %s\n\
         for user %s does not exist or is nonexecutable.\n"
M_BADNAME	"3001-622 User %s is an invalid name.\n"
M_BADTPATH	"3001-623 User %s has an invalid tpath value %s.\n"
M_DISABLE	"3001-624 Disable account for %s? "
M_FIXGRPS	"3001-625 Remove non-existent groups for %s? "
M_FIXAGRPS	"3001-626 Remove non-existent admgroups for %s? "
M_FIXSGRPS	"3001-627 Remove non-existent sugroups for %s? "
M_FIXCLASS	"3001-628 Remove non-existent audit classes for %s? "
M_FIXTTYS	"3001-629 Remove non-existent login ports for %s? "
M_FIXLIMIT	"3001-630 Reset %s to default value? "
M_NONAME	"3001-631 Invalid or missing user name on password line\n\
	%s.\n"
M_BADUID	"3001-632 Invalid or missing UID on password line\n\
         %s.\n"
M_BADGID	"3001-633 Invalid or missing GID on password line\n\
         %s.\n"
M_BADHOME	"3001-634 Invalid or missing HOME on password line\n\
         %s.\n"
M_DELETE	"3001-635 Remove the line beginning with\n         %s? "
M_PWDREOPEN	"3001-636 Error re-opening the file /etc/passwd.\n"
$ Do not translate DBM or mkpasswd
M_OLDDBM	"3001-637 The DBM files for /etc/passwd are out of date.\n\
         Use the mkpasswd command to update these files.\n"
M_UNKLIMIT      "3001-638 The user name %s appears in /etc/security/limits\n\
         but not in /etc/passwd.\n"
M_NOLIMFILE	"3001-639 The file /etc/security/limits does not exist.\n"
M_NOLIMIT       "3001-640 The user %s has no stanza in /etc/security/limits.\n"
M_ADDLIMIT	"3001-641 Add stanza for user %s to /etc/security/limits? "
M_UNKSECPWD	"3001-642 The user name %s appears in /etc/security/passwd\n\
         but not in /etc/passwd.\n"
M_NOSECPWDFILE	"3001-643 The file /etc/security/passwd cannot be opened.\n"
M_NOSECPWD	"3001-644 The user %s has no stanza in /etc/security/passwd.\n"
M_ADDSECPWD	"3001-645 Add stanza for user %s to /etc/security/passwd? "
M_UNKSECUSER	"3001-646 The user name %s appears in /etc/security/user\n\
         but not in /etc/passwd.\n"
M_NOSECUSERFILE	"3001-647 The file /etc/security/user cannot be opened.\n"
M_NOSECUSER	"3001-648 The user %s has no stanza in /etc/security/user.\n"
M_ADDSECUSER	"3001-649 Add stanza for user %s to /etc/security/user? "
M_NOGRPFILE	"3001-650 Unable to open the system group file.\n"
M_NOGRPNAME	"3001-651 Missing group name on group line\n\
         %s.\n"
M_BADGRPGID	"3001-652 Invalid or missing GID on group line\n\
         %s.\n"
M_NOSECGROUPFILE "3001-653 The file /etc/security/group cannot be opened.\n"
M_NOSECGROUP	"3001-654 The group %s has no stanza in /etc/security/group.\n"
M_ADDGROUP	"3001-655 Add stanza for group %s to /etc/security/group? "
M_UNKGROUP	"3001-656 The group name %s appears in /etc/security/group\n\
         but not in /etc/group.\n"
M_CREDFAIL	"3001-657 Unable to set process credentials for user %s.\n"
M_ASSYPGRP "3001-658 User %s has a group %s that does not exist, assuming\n\
         the group to be a Network Information Services group name.\n"
M_BADGRAMMAR    "3001-658 User %s has an invalid\n\
         authentication grammar \"%s\".\n"
M_BADREGISTRY   "3001-659 User %s has an invalid\n\
         authentication registry \"%s\".\n"
M_BADLOGTIMES	"3001-660 User %s has an invalid logintimes value.\n"
M_TOOMANYBAD	"3001-661 There have been too many invalid login attempts by user %s.\n"
M_LOCKED	"3001-662 User %s is locked.\n"
M_BADEXPIRES	"3001-663 User %s has an invalid expires value of %s.\n"
M_EXPIRED	"3001-664 The account for user %s has expired.\n"
M_WARNTIME	"3001-665 User %s's pwdwarntime occurs before minage.\n"
$ The %s is filled in with the user name, which is made possessive by the
$ "'s".
M_FIXWARNTIME	"3001-666 Reduce pwdwarntime to minage? "

M_PWDLOW	"3001-667 User %s's %s is less than %d.\n"
$ The first  %s is a user name, which is made possessive by the "'s".
$ The second %s is a password attribute name (e.g., minage).
$
M_FIXPWDLOW	"3001-668 Reset %s to the lowest allowed value? "
$ The %s is a password attribute name (e.g., minage).
$
M_PWDHIGH	"3001-669 User %s's %s is larger than %d.\n"
$ The first  %s is a user name, which is made possessive by the "'s".
$ The second %s is a password attribute name (e.g., minage).
$
M_FIXPWDHIGH	"3001-670 Reset %s to the highest allowed value? "
$ The %s is a password attribute name (e.g., minage).
$
M_MINAGE	"3001-671 User %s's minage is greater than maxage.\n"
$ The %s is a user name, which is made possessive by the "'s".
$
M_FIXMINAGE	"3001-672 Reduce minage to maxage? "
M_MINOTHER	"3001-673 User %s's minother is greater than the\n\
         maximum password length minus minalpha.\n"
$ The %s is a user name, which is made possessive by the "'s".
$
M_FIXMINOTHER	"3001-674 Reduce minother? "
M_BADFILE	"3001-675 User %s's %s attribute contains\n\
         a bad file name \"%s\".\n"
$ The first  %s is a user name, which is made possessive by the "'s".
$ The second %s is a password attribute name (e.g., dictionlist).
$ The third  %s is a file name.
$
M_NOACCESS	"3001-676 User %s's %s attribute contains\n\
         an inaccessible file \"%s\".\n"
$ The first  %s is a user name, which is made possessive by the "'s".
$ The second %s is a password attribute name (e.g., dictionlist).
$ The third  %s is a file name.
$
M_ABSPATH	"3001-677 User %s's %s attribute contains\n\
         \"%s\" which is not an absolute path name.\n"
$ The first  %s is a user name, which is made possessive by the "'s".
$ The second %s is a password attribute name (e.g., dictionlist).
$ The third  %s is a file name.
$
M_FIXFILE	"3001-678 Remove the file name from %s? "
$ The %s is a password attribute name (e.g., dictionlist).
$

