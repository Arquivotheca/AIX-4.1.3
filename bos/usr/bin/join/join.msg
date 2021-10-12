$ @(#)14	1.11  src/bos/usr/bin/join/join.msg, cmdfiles, bos411, 9428A410j 10/30/92 09:53:38
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
$quote "
$
$ $ Message numbers 285 - 292 reserved for this file.
$
$set MS_JOIN

BADFLDNUM "0653-285 The %s parameter is not a valid field number\n"
$ do not translate "join:" or "%s"

CANTOPEN "0653-286 Cannot open file %s.\n"
$ do not translate "join:" or "%s"

USAGE "Usage: join [-a Filenum] [-v Filenum] [-[j] Filenum Fieldnum]\n\
\t[-t Character] [-o list [-e String] ] File1 File2\n"

BADSEP    "join: 0653-287 Invalid -t separator character\n"
$ do not translate "join:" or "-t"

BOTHSTDIN "join: 0653-288 Both files cannot be \"-\"\n"

MALLOC "join: 0653-290 Memory allocation failure.\n"
$ do not translate "join:"

BADDSHE "join: 0653-291 The -e flag is only valid with the -o flag.\n\
Usage: join [-a Filenum] [-v Filenum] [-[j] Filenum Fieldnum]\n\
\t[-t Character] [-o list [-e String] ] File1 File2\n"
$ do not translate "join:", "-e", or "-o"
