$ @(#)46        1.10  src/bos/usr/bin/unpack/unpack.msg, cmdfiles, bos411, 9428A410j 4/11/94 15:42:12
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
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$
$ $ Message numbers 730 - 754 reserved for this file.
$
$set MS_UNPACK
$ Do not translate unpack -- this is the command name.
$ The program replaces %o,%1$o,etc with an octal number.
$
READERR ".z: 0653-730 Cannot read this .z file."
$ This message is preceded by unpack: filename
$
NOTPCKD ".z: This file is not in packed format."
$ This message is preceded by unpack: filename
$
RDERR1 ".z: 0653-731 Cannot read this .z file."
$ This message is preceded by unpack: filename
$
UNPCKERR ".z: 0653-732 Error occurred while unpacking this file."
$ This message is preceded by unpack: filename
$
WRITERR ": 0653-733 Cannot write to this file."
$ This message is preceded by pack: filename
$
NMLENERR ": 0653-734 This file name is too long."
$ This message is preceded by pack: filename
$
CANTOPNZ ".z: 0653-735 Cannot open this file."
$ This message is preceded by pack: filename
$
FILEXST ": A file by this name already exists."
$ This message is preceded by pack: filename
$
LINKERR ".z: Warning: This file has links."
$ This message is preceded by pack: filename
$
CREATERR ": 0653-736 This file cannot be created."
$ This message is preceded by pack: filename
$
CHMODERR "unpack: 0653-737 Cannot change the mode to %o."
$ Obsolete message...do not translate.
$
UNPCKD "%s: %s: This file is now unpacked.\n"
$ This message is preceded by pack: filename
$
ZRDERR ".z: 0653-738 Cannot read this file."
$ This message is preceded by pack: filename
$ Do not translate read
$
WRITERR1 ": 0653-739 Cannot write to this file."
$ This message is preceded by pack: filename
$ Do not translate write
$
ACLGETMSG "0653-740 Cannot get access control list information."
$ This message is preceded by unpack:
$
ACLPUTMSG "0653-741 Cannot put access control list information."
$ This message is preceded by unpack:
$
USAGE "Usage: unpack file ...\n"
