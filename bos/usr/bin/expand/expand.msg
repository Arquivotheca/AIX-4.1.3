$ @(#)52	1.7  src/bos/usr/bin/expand/expand.msg, cmdfiles, bos411, 9428A410j 10/30/92 09:06:51
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1992
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for expand and unexpand
$
$quote "
$
$ $ Message numbers 190 - 196 reserved for this file.
$
$set MS_EXPAND


USAGE	"Usage: unexpand [-a | -t tablist] [File ...]\n"
$ do not translate "unexpand", "-a", or "-t"

EXPUSAGE "Usage: expand [-tabstop][-tab1,tab2,...,tabn] [File ...]\n"
$ do not translate "expand"

TABORDER "0653-190 Specify tabs in increasing order.\n"

TABOVERFLOW "0653-191 Too many tabs.\n"

TABSEP	"expand: 0653-192 Tabs must be separated with a comma.\n"
$ do not translate "expand"

NEWEXPUSAGE "Usage: expand [-t tablist] [File ...]\n"
$ do not translate "expand" or "-t"

NEWTABSEP "0653-193 Tabs must be separated with a comma or a blank.\n"
