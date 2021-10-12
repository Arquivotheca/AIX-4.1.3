$ @(#)08        1.6  src/bos/usr/bin/sccs/sccsdiff.msg, cmdsccs, bos411, 9428A410j 4/25/91 19:26:45
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
$ (C) COPYRIGHT International Business Machines Corp. 1991
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "         define quote character.

$len 
$set 1        Define initial set#

$ This is the sccsdiff message catalog. 
1                    "%1$s: 1255-210 %2$s is not an SCCS file.\n"
$ %1$s is the command path name 
$ %2$s is the file name. 
2                    "Usage: %s -rSID1 -rSID2 [-p] [-sNumber] SCCSFile ...\n"
$ %s is the command name 
3                    "There are no differences between the specified versions of the file %s.\n"
$ %s is the file name. 
4                    "%1$s: %2$s versus %3$s\n"
$ %1$s is the file name. 
$ %2$s and %3$s are SID numbers. 
