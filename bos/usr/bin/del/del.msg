$ @(#)81	1.4  src/bos/usr/bin/del/del.msg, cmdfiles, bos411, 9428A410j 1/12/94 17:55:52
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$
$ $ Message numbers 600 - 624 reserved for this file.
$
$set MS_DEL
$               GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate del
$ Do not translate %o %s  %1$s %2$s etc.

USAGE "Usage: del [-] File...\n"
DELETE_MSG1 "del: Remove %s? Enter y or the ENTER key for yes.  "
$ %s is a file. The del command waits for a response on the same line.
$ Do not put a newline at the end of this message.
$ Your translation of "y" and "yes" should match the translation of
$ YES_MSG below.
DELETE_MSG2 "del: Remove these files? Enter y or the ENTER key for yes. "
$ The del command displays a list of files. Then it displays this message
$ and waits for a response on the same line. Do not put a newline at the
$ end of this message.
$ Your translation of "y" and "yes" should match the translation of
$ YES_MSG below.
YES_MSG "yes\n"
NO_MSG  "no\n"
DIRECTORY_MSG "del: 0653-606 Cannot delete directory %s.\n"
$ Do not translate %s. %s is a directory name.
PERM_MSG "del: 0653-607 Cannot remove %s.\n"
NOTREMOVED "%1$s: 0653-612 Cannot remove %2$s.\n"
$ %1$s is a command that attempted to remove %2$s. The attempt to remove %2$s
$ failed. A system error message about %2$s will be displayed on the next line.
