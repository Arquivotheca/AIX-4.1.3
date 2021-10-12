$ @(#)91        1.2.1.2  src/bos/usr/bin/chlang/chlang.msg, cmdnls, bos411, 9428A410j 5/16/94 12:03:44
$  COMPONENT_NAME: CMDNLS
$
$  FUNCTIONS:
$
$  ORIGIN: 27
$
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$  All Rights Reserved
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$       This file contains the corresponding messages for the errors generated
$       during the chlang shell script.  These messages will only be used
$       by this shell script.
$
$set 1
$quote "

$
$ The chlang command uses message numbers 1311-101 through 1311-108.
$

1 "1311-101 %s: Could not write to %s.\n\tCheck permissions on the file.\n"
$ The first %s refers to the program name.
$ The second %s refers to a file name.

2 "Usage:\t%s [ -u UID|Uname ] [ -m MsgTransLst | -M ] Language\n\
\t%s [ -u UID|Uname ] -m MsgTransLst | -M\n\
\t%s -d [ -u UID|Uname ]\n\
\tSets the default language for the system or a user.\n"
$ %s refers to the program name.

3 "Warning: NLSPATH not present in %s.\n"
$ Do not translate NLSPATH.
$ %s refers to a file name.

4 "1311-104 %s: Language must be specified.\n"
$ %s refers to the program name.

5 "1311-105 %s: Cannot modify another user.\n"
$ %s refers to the program name.

6 "1311-106 %s: User \"%s\" not found in /etc/passwd file.\n"
$ Do not translate /etc/passwd.
$ The first %s refers to the program name.
$ The second %s refers to a user name.

7 "1311-107 %s: Cannot use -m and -M flags together.\n"
$ %s refers to the program name.

8 "1311-108 %s: Cannot use -d flag with Language, -m flag, or -M flag.\n"
$ %s refers to the program name.
