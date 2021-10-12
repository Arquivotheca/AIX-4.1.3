$ @(#)58	1.12  src/bos/usr/bin/tail/tail.msg, cmdscan, bos411, 9428A410j 1/17/94 13:53:40
$
$ COMPONENT_NAME: (CMDSCAN) commands that scan files
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
$quote "         define quote character.
$len
$set MS_TAIL        Define initial set#
$ tail is the command name. Do not translate tail anywhere in this file.

USAGE           "Usage: tail [+|-[Number]][l|b|c|k|m][f] [File]\n"
TOOMUCH         "tail: 0652-042 There is not enough memory available now.\n"
FSHRUNK         "tail: 0652-043 The size of the file has changed during the run of tail.\n"
USAGE1		"Usage: tail [-r] [-n Number] [File]\n"
USAGE2		"Usage: tail [-f] [-c Number|-n Number|-m Number|-b Number|-k Number] [File]\n"
PRSFILE		"tail: 0652-044 Can only process one file at a time.\n"
EXCLOPT		"tail: 0652-045 Options are mutually exclusive.\n"
