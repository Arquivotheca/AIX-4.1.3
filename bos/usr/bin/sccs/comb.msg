$ @(#)11        1.7 1/26/94 14:03:49
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "         define quote character.

$ Do not translate comb
$len 
$set 5        Define initial set#

PFILEXSTS            "\n\
comb: 1255-082 Cannot use the comb command while there is an outstanding\n\
\tdelta to the file. (cb1)\n"
$ Do not translate (cb1)
NOPANDC              "\n\
comb: 1255-083 Cannot use both the -c and -p flags together. (cb2)\n"
$ Note to translator: The user can only choose one of the above flags on the 
$ command line. 
$ Do not translate (cb2). 
NOTHTODO             "\n\
comb: 1255-084 The flags specified on the command line would create no changes\n\
\tto the SCCS file. (cb4)\n"
$ Do not translate (cb4). 
DELTAMSG             "*** DELTA TABLE PRIOR TO COMBINE ***\n"
$
COMB_USAGE		     "Usage: comb [-o] [-s] [-p SID] [-c List] File\n"
$ Do not translate comb.
