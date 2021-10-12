$ @(#)08	1.7.1.2  src/bos/usr/bin/cut/cut.msg, cmdfiles, bos411, 9428A410j 12/9/92 16:57:52
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS: Messages
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
$ $ Message numbers 130 - 144 reserved for this file.
$
$set MS_CUT
$               GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate cut -- this is the command name.
$ Do not translate %s.
$ The program replaces %s with a string of
$ characters.

NODELIMITER "cut: 0653-130 Specify a valid character with the -d flag.\n"
$ This message is preceded with cut :  on the same line.
$
USAGE "Usage: cut -b List [-n] [File...]\n\
   or: cut -c List [File...]\n\
   or: cut -f List [-d Character] [-s] [File...]\n"
$
CANTOPEN "cut: 0653-132 Cannot open %s.\n"
$
BADLIST "cut: 0653-135 The list arguments following the %c option are not correct.\n"
$
MAXARGS "cut: 0653-136 Too many list arguments for the %c option.\n"
$
NOMEMORY "cut: 0653-137 Out of memory.\n"
$
