$ @(#)31\t1.5  src/bos/usr/bin/look/look.msg, cmdscan, bos411, 9428A410j 1/21/91 15:32:22
$
$ COMPONENT_NAME: (CMDSCAN) commands that scan files
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
$ messages for look
$
$quote "         define quote character.
$len
$set MS_LOOK        Define initial set#
$ Do not translate look, %s.

CANTOPEN             "look: 0652-037 Cannot open %s.\n"
$ %s is replaced by the file or directory that cannot be opened.
TOOLONG              "look: 0652-038 The line has been shortened to the 250 character limit.\n"
WORDFILE             "look: 0652-039 The wordfile line is too long.\n"
USAGE		     "Usage: look [-d -f] String [File ...] \n"
