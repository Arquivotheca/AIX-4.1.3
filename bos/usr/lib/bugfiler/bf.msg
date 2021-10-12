$ @(#)08	1.1  src/bos/usr/lib/bugfiler/bf.msg, cmdmailx, bos411, 9428A410j 10/2/89 09:17:36
$ 
$ COMPONENT_NAME: CMDMH bf.msg
$ 
$ FUNCTIONS: 
$
$ ORIGINS: 10  26  27 
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$ scmc msg file created Mon Sep 18 14:51:22 1989
$ user:  danc

$quote "         define quote character.

$len 
$set MS_BF           Define initial set#

USAGE                "Usage: unixtomh name1 name2\n"
NOMEM                "unixtomh:  Ran out of memory\n"
USAGE2               "Usage: bugfiler [-d] [-m msg_mode] [-b bugperson] [maildir]\n"
BPUNKOWN             "%s: bugs person is unknown\n"
NOCHDIR              "can't chdir to %s\n"
NOCREATE             "cannot create %s\n"
NOTMPOPEN            "cannot fdopen temp file\n"
FLDTRUNC             "field truncated\n"
NOMEM2               "ran out of memory\n"
MISSING              "Missing %s\n"
NOOPEN               "Can't open %s\n"
FOLDEREQ             "folder = %s\n"
NOOPEN2              "Cannot open %s/%s\n"
NOOPEN3              "cannot open %s\n"
NOCREATE2            "cannot create %s\n"
NODIST               "redistribute(%s, %d), no distribution list\n"
NOLIST               "no redistribution list found\n"
NOCREATE3            "%s: couldn't create\n"
DISTLIST             "redistribution list %s"
COPYTO               "copy to %s\n"
UNTITBUG             "Untitled bug report\n"
DISAPPEAR            "%s: disappeared?\n"
EMPTYBUG             "empty bug report?\n"
DELFAIL              "dodeliver, \"%s\" failed\n"
BFEOF                "EOF\n"
NOCREATE4            "Can't create %s\n"
NOREPLY              "No one to reply to\n"
INACK                "In-Acknowledgement-Of: Your message of "
