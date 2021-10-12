$ @(#)96	1.3  src/bos/usr/bin/tset/tset.msg, cmdtty, bos411, 9428A410j 10/21/93 02:45:56

$ COMPONENT_NAME: CMDTTY terminal control commands
$
$ FUNCTIONS: .msg file for tset
$
$ ORIGINS: 27, 83
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 
$ LEVEL 1, 5 Years Bull Confidential Information
$ 

$set MS_TSET

$quote "

USE "usage: tset [-] [-rsIQS] [-eC] [-kC] [-iC] [-m [ident][test speed]:type] [type]\n"
NOTTERM "Not a terminal\n"
BADFLAG "Bad flag -"
FIND "Cannot find termcap\n"
TYPE "Type "
UNKNOWN " unknown\n"
TERMTYPE "Terminal type is "
IS " is "
SETTO " set to "
CANTOPEN "Cannot open "
TOOFEW "Too few args: "
UNEXPECT "Unexpected arg: "
BADEXE "Could not find executable for iprog: "
BADFORK "Error forking process\n"
