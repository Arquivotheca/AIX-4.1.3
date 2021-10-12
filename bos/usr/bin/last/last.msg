$ @(#)01	1.5  src/bos/usr/bin/last/last.msg, cmdstat, bos411, 9428A410j 4/25/91 17:45:59
$
$ COMPONENT_NAME: (CMDSTAT) last
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
$quote "
$set MS_LAST

M_LOGGEDIN      "  still logged in.\n"
M_DOWN          "System is halted by system administrator. "
$ Leave a space at the end, and no new line.
M_CRASH         "System halted abnormally. "
$ Leave a space at the end, and no new line.
M_BEGIN         "\nwtmp begins %10.10s %5.5s \n"
$ Do not translate wtmp, %10.10s, %5.5s
M_INTR          "\ninterrupted %10.10s %5.5s \n"
$ Do not translate  %10.10s, %5.5s
