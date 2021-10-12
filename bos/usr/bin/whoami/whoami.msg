$ @(#)52    1.5 src/bos/usr/bin/whoami/whoami.msg, cmdstat, bos411, 9428A410j 4/25/91 17:47:11
$
$ COMPONENT_NAME: (CMDSTAT) status
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "         define quote character.

$len
$set MS_WHOAMI        Define initial set#

INTRUDER             "whoami: 0551-300 The user name is not recognized.\n"
$ Do not translate whoami
$ This warning notifies you that a user who is not in the /etc/passwd file
$ has gained access to your system.
