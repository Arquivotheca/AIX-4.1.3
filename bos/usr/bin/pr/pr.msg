$ @(#)27	1.9  src/bos/usr/bin/pr/pr.msg, cmdfiles, bos411, 9428A410j 8/6/93 14:59:50
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
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$
$ $ Message numbers 550 - 564 reserved for this file.
$
$set MS_PR

PAGE            "Page"
EMPTYF          "0653-550 The file %s is empty."
NOCORE          "0653-551 There is not enough memory available now."
PBOVRFL         "0653-552 Internal error. A page-buffer overflow has occurred."
TOOMANYF        "0653-553 Too many files are specified."
WDTHLARGE       "0653-554 The width specified with the -n flag is too large."
CANTOPEN	"0653-555 File %s cannot be opened."
$ The file cannot be opened for reading. Either the file does not exist or
$ the user does not have read permission for the file.
USAGE           "Usage: pr [+Page][[-Column [-a]] | -m][-dFrt]\n\
\t  [-e[Character][Gap]][-h Header][-i[Character][Gap]][-l Lines]\n\
\t  [-n[Character][Width]][-o Offset][-s[Character]][-w Width]\n\
\t  [-x[Character][Width]][-fp][File ...]\n"));
BADNUM		"pr: 0653-556 Specify an integer value with the -%c option.\n"
BADVAL		"pr: 0653-557 Value %d can not be greater than %d for -%c\n"

