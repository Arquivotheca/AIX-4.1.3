$ @(#)92        1.10.1.2  src/bos/usr/bin/chown/chown.msg, cmdsdac, bos411, 9428A410j 8/26/93 14:47:19
$
$ COMPONENT_NAME:  (CMDSDAC) security: access control
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

$ messages for chown command

$ 3002-130 thru 3002-139

$set MS_CHOWN
$quote "

USAGE	  "Usage: chown -fhR {username|uid}[:{groupname|gid}] file ...\n"
$ <<<<<<NOTE>>>>>>> colon is new user/group separator character
$ also, chown is the command name - do not translate.
NOBACK    "chown: 3002-130  Cannot change directory back to %s.\n"
$ chown is the command name - do not translate.
BADUID    "chown: 3002-131 %s is an unknown username.\n"
$ chown is the command name - do not translate.
BADGID    "chown: 3002-132 %s is an unknown groupname.\n"
$ chown is the command name - do not translate.
