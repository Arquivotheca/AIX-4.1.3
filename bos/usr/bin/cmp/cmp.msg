$ @(#)78	1.11  src/bos/usr/bin/cmp/cmp.msg, cmdfiles, bos411, 9428A410j 12/4/92 09:58:52
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
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1991
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$
$ $ Message numbers 023 - 029 reserved for this file.
$
$set MS_CMP
$ Do not translate cmp
DIFF "%s %s differ: char %d, line %d\n"
$ DO NOT TRANSLATE
$ This is part of the output of the cmp command. The standards dictate
$ that it be in precisely this format. Customers will expect it this way.
USG "Usage: cmp [-l | -s] File1 File2\n"
NOTOPEN "cmp: 0653-023 Cannot find or open %s.\n"
EOFMSG "cmp: EOF on %s\n"
BADFILES "cmp: 0653-024 File1 and File2 can not both refer to stdin.\n"
