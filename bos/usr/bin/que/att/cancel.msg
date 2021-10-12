$ @(#)74	1.6  src/bos/usr/bin/que/att/cancel.msg, cmdque, bos411, 9428A410j 11/1/93 16:06:46
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
$set MS_CANCEL
MSGUSE1	"{JobNumber ... | PrinterName}"
MSGUSE2 "Cancels print jobs specified with the lp command."
$ Do not translate the command word cancel or the command word lp.
