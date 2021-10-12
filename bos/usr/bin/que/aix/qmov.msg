$ @(#)62	1.1  src/bos/usr/bin/que/aix/qmov.msg, cmdque, bos411, 9428A410j 2/8/93 16:40:36
$
$ COMPONENT_NAME: (CMDQUE) spooling commands
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ NOTE: All messages are run through a message routine that requires the
$       messages not to have linefeeds (\n) at the end of the message string.

$quote "
$set MS_QMOV
MSGUSE1 "-m<queue moving to> {-#<job number> [-P<queue>] | -P<queue> |"
MSGUSE2 "-u<user> [-P<queue>]}"
MSGUSE3 "Move jobs from one queue to another."
$ do not translate the command word qhld or the flags -u, -#, -P
