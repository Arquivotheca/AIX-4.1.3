$ static char sccsid[] = "@(#)18  1.7  src/bos/usr/bin/sccs/rmchg.msg, cmdsccs, bos411, 9428A410j 7/9/93 12:51:12";
$
$ COMPONENT_NAME: CMDSCCS       source code control system
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

$quote "         define quote character.

$ Do not translate 
$ Do not translate the parenthesis (rc#) at the end of each message.
$len 
$set 10        Define initial set#

MISSINGR   "1255-121 Use the -r flag with this command. (rc1)\n"
$ Do not translate -r
NOTOWNER   "1255-122 Only the owner of the file or %s\n\
\tcan use this command. (rc4)\n"
$ %s is the creator of the delta. 
NOTLEAF    "1255-123 Only the most recently created delta on a branch,\n\
\tor the latest trunk delta if it has no branches can be removed. (rc5)\n"
DELTSPEC   "1255-124 Cannot remove this delta or change its commentary. (rc9)\n"
BADINVOKE  "1255-125 Cannot invoke rmdel or cdc except by the commands\n\
\trmdel or cdc. (rc10)\n"
RNOSID     "1255-126 Specify an SID with the -r flag. (rc11)\n"
$ Do not translate -r
BEINGEDIT  "1255-127 Another user is working on the specified delta. (rc12)\n"
CHANGED    "** CHANGED ** "
