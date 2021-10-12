$ @(#)83	1.2  src/bos/usr/bin/cp/cp.msg, cmdfiles, bos411, 9428A410j 11/5/92 17:18:20
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS: Messages 
$
$ ORIGINS: 18, 27
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
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ OSF/1 Release 1.0
$
$ RCSfile: cp.msg,v Revision: 1.3  (OSF) Date: 90/10/07 16:18:52 
$
$ Messages for the cp command.
$
$quote "
$
$ $ Message numbers 433 - 449 reserved for this file.
$
$set MS_CP
$       GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate cp  --- this is the command name.
$ Do not translate %s etc.
$

RANDR		"%s: 0653-433 the -R and -r options are mutually exclusive.\n"
$
SPACE		"%s: 0653-434  out of space.\n"
$
SAME		"%s: 0653-435  %s and %s are identical (not copied).\n"
$
ISDIR		"%s: 0653-436  %s is a directory.\n\tSpecify -r or -R to copy.\n"
$
NOTDIR		"%s: 0653-437  %s is not a directory.\n"
$
OVER		"overwrite %s? "
$
CANTREAD	"%s: 0653-438 cannot read directory %s.\n"
$
TOLNG		"%s: 0653-439 %s: name too long.\n"
$
TOLNG2		"%s: 0653-440 %s/%s: name too long.\n"
$
USAGE	"Usage: %s [-fhip] [-r|-R] [--] src target\n\
   or: %s [-fhip] [-r|-R] [--] src1 ... srcN directory\n"
$
