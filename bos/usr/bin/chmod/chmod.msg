$ @(#)20	1.10.2.2  src/bos/usr/bin/chmod/chmod.msg, cmdsdac, bos411, 9428A410j 10/27/93 16:47:28
$
$ COMPONENT_NAME:  (CMDSDAC) security: access control
$
$ FUNCTIONS:  acledit
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1993
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$
$ messages for command chmod
$
$set MS_CHMOD
$quote "

USAGE "Usage: chmod [-R] [-f] [-h] {u|g|o|a ...} {+|-|=} {r|w|x|X|s|t ...} File ...\n\
\tchmod [-R] [-f] [-h] OctalNumber File ...\n\
\tChanges the permission codes for files or directories.\n"
EMODE "0481-013 chmod: A flag or octal number is not correct.\n\
\tUsage: chmod [-R] [-f] [-h] {u|g|o|a ...} {+|-|=} {r|w|x|s|t ...} File ...\n\
\tchmod [-R] [-f] [-h] OctalNumber File ...\n"
