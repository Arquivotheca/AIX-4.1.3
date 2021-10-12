$ @(#)41	1.12.1.5  src/bos/usr/bin/find/find.msg, cmdscan, bos411, 9428A410j 7/15/93 10:59:03
$
$ COMPONENT_NAME: (CMDSCAN) commands that scan files
$
$ FUNCTIONS:
$
$ ORIGINS: 27, 71
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ OSF/1 1.1
$
$quote "         define quote character.
$len
$set MS_FIND        Define initial set#
$  Do not translate find.
$  Do not translate %s, %d, etc. These will be replaced by files, numbers etc.

BADPWD               "find: 0652-007 Cannot get the name of the current directory.\n"
$       Do not translate pwd.
USAGE                "Usage: find Path-list [Expression-list]\n"
$ $ Check usage statement.
PARSERR              "find: 0652-008 There is a command usage error.\n"
CONJ                 "find: 0652-009 There is a missing conjunction\n"
BADSTART             "find: 0652-010 The starting directory is not valid.\n"
OPFOP                "find: 0652-011 An operand follows another operand.\n"
$ NOTE
$ NOTE: the OPFOP message will never happen so don't bother translating
$ NOTE
NOUSER               "find: 0652-012 The user name does not exist.\n"
NOGROUP              "find: 0652-013 The group name does not exist.\n"
NOCREATE             "find: 0652-014 Cannot create file %s.\n"
NOACCES              "find: 0652-015 Cannot access file %s.\n"
BADNODE              "find: 0652-016 %s is not a valid node id.\n"
BADOPTION            "find: 0652-017 %s is not a valid option.\n"
INCSTATE             "find: 0652-018 An expression term lacks a required parameter.\n"
BADSTAT              "find: 0652-019 The status on %s is not valid.\n"
NOCOPY               "find: 0652-020 Cannot copy %s.\n"
OKPROMT              "%1$s %2$s (%3$s) "
$ Do not translate this message.
NOCHDIR              "find: 0652-022 Cannot change directory to %s.\n"
NOOPEN               "find: 0652-023 Cannot open file %s.\n"
NOREAD               "find: 0652-024 Cannot read file %s.\n"
BADPATHLEN           "find: 0652-025 The pathname cannot be longer than 512 bytes.\n"
BADTREE              "find: 0652-026 Cannot change to the parent directory.\n"
NOSTAT               "find: 0652-027 Cannot get status on %s.\n"
WRTOUT               "find: 0652-028 Cannot write output.\n"
READIN               "find: 0652-029 Cannot read input.\n"
ASKDEV               "To continue, enter the device or file name.\n"
NOGOOD               "find: 0652-030 Cannot open the specified device.\n"
BADDIR               "find: 0652-031 %s is not a valid directory.\n"
NOCD		     "find: 0652-081 cannot change directory to <%s/%s>:\n "
NOCDROOT	     "find: 0652-081 cannot change directory to </%s>:\n "
$ NOCD and NOCDROOT can have the same message number, there needed to be
$ a separate message when the filename was root or /.
YES  		      "yes"
TOOMANYOPTS	      "find: 0652-082 Too many options\n"
CANTEXEC	      "find: 0652-083 Cannot execute %s"
CANTOPEN	      "find: 0652-084 Cannot open %s\n"
BADPERM               "find: 0652-085 Specify a valid -perm argument.\n"
BADNUM                "find: 0652-086 Specify a decimal integer for %s\n"
$	Error numbers still available for this command: 087 
