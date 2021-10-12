$ @(#)27	1.4  src/bos/usr/bin/script/script.msg, cmdsh, bos411, 9428A410j 7/6/94 17:01:45
$
$ COMPONENT_NAME: (CMDSH) Bourne shell and related commands
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for script
$

$set MS_SCRIPT
$ Do not translate script

$quote "

USAGE           "Usage: script [ -a ] [ Typescript ]\n"
SCRIPTSTART     "Script command is started. The file is %s.\n"
FORK            "fork"
$ Do not translate.
STARTSCR        "Script command is started on %s."
ENDSCR          "\nScript command is complete on %s."
DONESCRIPT      "Script command is complete. The file is %s.\n"
NOPTY           "Out of pty's.\n"
$ This is an obsolete message. Do not translate.
CHMOD_ERR	"chmod failed to change permissions of tty. Errno = %d.\n"
CHOWN_ERR	"chown failed to change ownership of tty. Errno = %d.\n"
ACLSET_ERR	"acl_set failed to change permissions of tty. Errno = %d.\n"
