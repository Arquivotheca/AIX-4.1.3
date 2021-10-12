$ @(#)77	1.5  src/bos/usr/bin/que/att/lp.msg, cmdque, bos411, 9428A410j 7/11/90 16:29:21
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
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ NOTE: All messages are run through a message routine that requires the
$       messages not to have linefeeds (\n) at the end of the message string.

$quote "
$set MS_LP
MSGUSE1	"[-cmsw] [-dDestination] [-tTitle]"
MSGUSE2 "[-nNumber] [-oOption] [-] File ..."
MSGUSE3 "Prints a file in a format suitable for sending to a line printer."
$ do not translate the command word lp or the flag -cmswdnot
