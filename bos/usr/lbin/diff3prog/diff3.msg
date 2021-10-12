$ @(#)42	1.8  src/bos/usr/lbin/diff3prog/diff3.msg, cmdfiles, bos411, 9428A410j 4/28/91 20:43:34
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ ORIGINS: 18, 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ @OSF_COPYRIGHT@
$ No OSF rcsid string found in usr/lbin/diff3prog/diff3.msg
$
$quote "
$
$ $ Message numbers 860 - 879 reserved for this file.
$

$ Do not translate diff3.
$ messages for the diff3prog.c source file
$ messages for the diff3.sh shell script

$set 2

1 "Usage: diff3 [-e|-x|-E|-X|-3] File1 File2 File3\n"
2 "diff3: 0653-865 A diff command failed in diff3.\n"
$ The diff3 command calls the diff command. One of those calls to diff failed.
$ The standard error message from the diff command that failed is displayed
$ below this error message (2).

$set MS_DIFF3

ARGCNT "diff3: 0653-861 Specify three existing files.\n"
CANTOPEN "diff3: 0653-862 Cannot open %s.\n"
$ %s is a file.
CHNGCNT "diff3: 0653-863 There are more than 200 different lines between the 3 files.\n"
LOGERR "diff3: 0653-864 The diff3 program logic cannot handle the specified files.\n"

UNKFLG "diff3: 0653-866 Unknown flag: '%c'\n"
USAGE "Usage: diff3 [-e|-x|-E|-X|-3] File1 File2 File3\n"
