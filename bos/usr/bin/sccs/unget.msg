$ @(#)14        1.7 3/16/93 18:25:01
$
$ COMPONENT_NAME: CMDSCCS       source code control system
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

$quote "         define quote character.

$len 
$set 11        Define initial set#

SIDSPEC1             "\n\
1255-152 There is more than one outstanding SID in the p-file. (un1)\n"
$ Do not translate p-file, SID (un1).
LOGNAME              "\n\
1255-153 User name is not in the p-file. (un2)\n"
$ Do not translate p-file.
$ Do not translate (un2). 
SIDSPEC3             "\n\
1255-154 The specified SID does not exist. (un3)\n"
$ Do not translate SID, (un3). 
NOOUTDELTA           "There are no outstanding deltas for %s.\n"
$
USAGESACT "Usage: sact File...\n"
$ Do not translate sact.
USAGEUNGET "Usage: unget [ -r SID ] [ -s ] [ -n ] File...\n"
$ Do not translate unget.
