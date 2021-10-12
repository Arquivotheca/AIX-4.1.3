$ @(#)72	1.8  src/bos/usr/bin/que/bsd/lpr.msg, cmdque, bos411, 9428A410j 2/8/94 16:14:33
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ NOTE: All messages are run through a message routine that requires the
$       messages not to have linefeeds (\n) at the end of the message string.

$quote "
$set MS_LPR
MSGUSE1	"[-fghjlmnprs] [-PPrinter] [-#NumberCopies] [-CClass]"
MSGUSE2	"[-JJob] [-TTitle] [-iNumberColumns] [-wWidth] [Filename ...]"
MSGUSE3 "Prints the specified files when facilities become available."
$ do not translate the command word lpr or the flags -cdfghlmnprstvP#CJTiw
