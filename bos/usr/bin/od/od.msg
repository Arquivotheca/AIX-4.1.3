$ @(#)43	1.8.2.3  src/bos/usr/bin/od/od.msg, cmdscan, bos411, 9428A410j 1/28/94 14:46:01
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 1.1
$

$quote "         define quote character.
$len
$set MS_OD        Define initial set#
$ Do not translate od, %s, %c.

CANTOPEN             "od: 0652-040 Cannot open %s.\n"
$       Do not translate the command name od and %s.
BADFLAG              "%s: 0652-041 -%c is not a recognized flag.\n"
$       %s MUST stay at the start of the message.
$       Do not translate %s and -%c.
USAGE	"usage: od [-abBcCdDefFhHiIlLoOpPsvxX] [-S [n]] [-w [n]] [file] [[+]offset[.|b|B]] [[+]label[.|b|B]]\n"
USAGE1	"       od [-v] [-A address_base] [-N count] [-j skip] [-t type_string] [file...]\n"
$ $ Check the usage statement.
$       Do not translate %s and all the flags.
AOPT  "-A option only accepts the following:  d, o, n, and x\n"
INVNUM "Invalid number of bytes to interpret\n"
DOPT  "d may only be followed with C, S, I, L, 1, 2, or 4\n"
FOPT  "f may only be followed with F, D, L, 4, or 8\n"
OOPT  "o may only be followed with C, S, I, L, 1, 2, or 4\n"
UOPT  "u may only be followed with C, S, I, L, 1, 2, or 4\n"
XOPT  "x may only be followed with C, S, I, L, 1, 2, or 4\\n"
INVOFF "Invalid offset\n"
