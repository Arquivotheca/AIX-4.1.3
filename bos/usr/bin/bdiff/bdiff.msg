$ @(#)62	1.8  src/bos/usr/bin/bdiff/bdiff.msg, cmdfiles, bos411, 9428A410j 12/10/92 14:32:02
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ ORIGINS: 18, 27
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
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ OSF/1 Release 1.0
$
$ Message number 001 - 014 reserved for this file.
$
$set MS_BDIFF
$quote "
ARGCNT2 "Usage: bdiff File1 File2 [Number] [-s]\n"
BOTHSTDIN2 "bdiff: 0653-001 Specify only one file as standard input.\n"
NONNUMLMT2 "bdiff: 0653-002 Specify file segment size as a line number.\n"
$ This message refers to the  [Number]  option in the  ARGCNT2  message.
$ Number must be a positive, nonzero decimal number.
CANTFORK2 "bdiff: 0653-003 Cannot create another process now.\n"
DIFFFAIL2 "bdiff: 0653-005 The diff command failed.\n"
$ Do not translate  diff . It is a command name.
CANTEXEC2 "bdiff: 0653-004 Cannot run the diff command.\n"
$ Do not translate  diff . It is a command name.
$
$ The next five messages below are NEW for AIX 3.2.
TMPWRTERR "bdiff: 0653-006 Error writing temporary file.\n"
PIPEFAIL "bdiff: 0653-007 The pipe command failed.\n"
UNLNKERR "bdiff: 0653-008 Cannot unlink temporary file.\n"
TMPCREERR "bdiff: 0653-009 Cannot create temporary file.\n"
CANTOPEN "bdiff: 0653-010 Cannot open file %s .\n"
