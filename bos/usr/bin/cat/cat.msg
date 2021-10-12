$ @(#)34	1.9  src/bos/usr/bin/cat/cat.msg, cmdscan, bos411, 9428A410j 12/3/93 09:21:23
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1992
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "         define quote character.
$len
$set MS_CAT        Define initial set#

$ messages for command cat
$   GLOBAL COMMENT FOR TRANSLATORS
$       Do not translate cat -- this is the command name.
$       Do not translate %o %s %1$s %2$s etc.
$       These are output formats for file names, directory
$       names, permission codes, command names, and the like,
$       depending on their context within each message.
ESTATOUT             "cat: 0652-049 Cannot get status on standard output.\n"
EOPEN                "cat: 0652-050 Cannot open %s.\n"
ESTAT                "cat: 0652-051 Cannot get status on %s.\n"
$
$ %s is a file name.
$
ECLOSE               "cat: 0652-053 Cannot close the file.\n"
$
EOUTPUT              "cat: 0652-054 Cannot write to output.\n"
$
EUSAGE               "Usage: cat [-qrSsu] [-n[b]] [-v[et]] [-|File ...]\n"
$
$       Do not translate any of -qsu, -n, b, -v, et.
