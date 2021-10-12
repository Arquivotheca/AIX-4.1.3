$ @(#)82	1.9.1.1  src/bos/usr/bin/xargs/xargs.msg, cmdsh, bos411, 9431A411a 8/3/94 23:14:42
$
$ COMPONENT_NAME: (CMDSH) Bourne shell and related commands
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
$quote "
$set MS_XARGS

CNT "0402-051 Specify a positive integer after the -%c option.\n"
LINESIZ "0402-052 -s value not within system limits; %s is not valid.\n"
$ Do not translate -s
$ The %s will be a command line argument
TTYREAD "0402-054 Cannot find or open /dev/tty for prompting.\n"
$ "/dev/tty" is a file name and shouldn't be translated
UNKWNOPT "The -%c flag is not valid.\n\
Usage: xargs [-p][-t] [-e[EndOfFileString]] [-E EndOfFileString]\n\
       [-I ReplacementString] [-i[ReplacementString]] [-L Number]\n\
       [-l[Number]] [-n Number [-x]] [-s Size] [Command [Argument ...]]\n"
ARGSIZ "0402-056 Cannot replace more than 5 instances of %s.\n"
$ The %s will be {} or whatever follows the -i flag on the command line
NOEXEC "0402-057 The %s command was not found or could not be run.\n"
ARGLIST "0402-058 A command line size exceeds the specified system limit.\n"
SNGLARG "0402-059 The size of a single parameter exceeded the command line limit.\n"
MSNGQUOT "0402-060 An end quote is missing from the string: %s.\n"
$ %s is replaced by a string of varying length. You may wish to put
$ it on a separate line.
MAXSARGSIZ "0402-061 A parameter constructed by replacing %s exceeded 255 bytes.\n"
$ The %s will be {} or whatever follows the -i flag on the command line

OVRFLOW "0402-062 A command line constructed by replacement exceeded 255 bytes.\n"
QUESTION        "?..."
MISSARG "0402-083 The option -%c requires an argument.\n\
Usage: xargs [-p][-t] [-e[EndOfFileString]] [-E EndOfFileString]\n\
       [-I ReplacementString] [-i[ReplacementString]] [-L Number]\n\
       [-l[Number]] [-n Number [-x]] [-s Size] [Command [Argument ...]]\n"
