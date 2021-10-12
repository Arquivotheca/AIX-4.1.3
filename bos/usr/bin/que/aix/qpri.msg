$ @(#)68	1.5.1.1  src/bos/usr/bin/que/aix/qpri.msg, cmdque, bos411, 9428A410j 9/24/93 14:23:32
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ NOTE: All messages are run through a message routine that requires the
$       messages not to have linefeeds (\n) at the end of the message string.

$quote "
$set MS_QPRI
MSGUSE1 "-#JobNumber -aPriority"
MSGUSE2 "Prioritizing a job in the print queue."
$ do not translate the command word qpri or the flags -a and -#
