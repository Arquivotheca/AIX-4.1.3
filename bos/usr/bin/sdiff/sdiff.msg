$ @(#)60	1.6.1.2  src/bos/usr/bin/sdiff/sdiff.msg, cmdfiles, bos411, 9428A410j 12/10/92 16:58:48
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
$ $ Message numbers 625 - 644 reserved for this file.
$
$set MS_SDIFF
LLERR "0653-625 Line length %s on the -w flag is too small."
$ %s is replaced by a string
USAGE "Usage: sdiff [-l | -s] [-o Output] [-w Number] File1 File2\n"
$ do not translate sdiff or the command flags
OPERR "0653-627 Cannot open %s."
$ %s is replaced by a string
OPERRT "0653-628 Cannot open the temporary file %s."
$ %s is replaced by a file name
OPERRO "0653-629 Cannot open the output file %s."
$ %s is replaced by a file name
NOFNDC "sdiff: 0653-630 %c is not a recognized subcommand for the -o flag.\n"
$ %c is replaced by a character
ILLCMD "sdiff: 0653-632 %s is not a recognized subcommand for the -o flag.\n"
$ %s is replaced by a flag name
LONGLINE "0653-635 Line in input file is too long"
