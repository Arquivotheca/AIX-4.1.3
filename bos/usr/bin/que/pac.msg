$ @(#)21  1.10  src/bos/usr/bin/que/pac.msg, cmdque, bos411, 9428A410j 11/3/93 14:23:13
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

$set MS_PAC
$quote "
USAGE		"[-PPrinter] [-pPrice] [-qFileName] [-s] [-c] [-r] [-m] [User ...]"
QC_E		"0781-057 Error in processing qconfig file: %s\n"
HEADER		"  Login               pages/feet   runs               price\n"
TOTAL		"total"
NOOPEN		"0781-058 Can't open: %s\n"
BADQNAM		"0781-318 Invalid queue name: %s"
BADENV		"0781-319 Invalid value from PRINTER or LPDEST environment variable."
NOACCT		"0781-320 No accounting file specified for queue: %s"
MSGACCT		"0781-321 Problem opening account file. Errno = %d."
MSGSUMM		"0781-322 Problem opening summary file. Errno = %d"
MSGWACC		"0781-323 Problem writing to account file. Errno = %d"
MSGFORK         "0781-324 Cannot fork for %s."
MSGEXEC         "0781-325 Cannot exec digester %s."
MSGDIGE         "0781-326 Error from digester %1$s, status = %2$d, rv = %3$d."
MSGNOPN         "0781-327 Cannot open %s for reading."
MSGNADA         "0781-328 Nothing read from %1$s.  Errno = %2$d."
