$ @(#)65        1.17.1.1  src/bos/usr/bin/mv/mv.msg, cmdfiles, bos411, 9434A411a 8/19/94 13:18:59
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS: Messages 
$
$ ORIGINS: 27, 18, 71
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
$
$ Messages for the mv command.
$
$quote "
$
$ $ Message numbers 400 - 416 reserved for this file.
$
$set MS_MV
$       GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate mv  --- this is the command name.
$ Do not translate %s etc.
$

P2LNG	"mv: 0653-400 %s: Destination pathname too long.\n"
$
REMOVE	"overwrite %s? "
$
REMOVE2	"override mode %o on %s? "
$
RENAME	"%s: 0653-401 Cannot rename %s to %s:\n\t     %s\n"
$
NOMEM	"%s: 0653-402 %s: Out of memory.\n"
$
NOEXEC	"%s: 0653-403 Cannot exec %s.\n"
$
USAGE	"Usage: %s [-i | -f]  [--] src target\n   or: %s [-i | -f]  [--] src1 ... srcN directory\n"
$
CANTDUP	"%s: 0653-404 %s: Unable to duplicate owner and mode after move.\n"
$
IDENTICLE "%s: 0653-405 %s and %s are identical.\n"
$
CANTRM	"mv: 0653-406 Cannot remove source file %s\n"
