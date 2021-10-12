$ @(#)14	1.5  src/bos/usr/bin/ln/ln.msg, cmdfiles, bos411, 9428A410j 8/2/93 15:01:16
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS: Messages 
$
$ ORIGINS: 27, 18
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
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ OSF/1 Release 1.0
$
$ RCSfile: ln.msg,v  Revision: 1.3  (OSF) Date: 90/10/07 16:41:38 
$
$ Messages for the ln command.
$
$quote "
$
$ $ Message numbers 417 - 432 reserved for this file.
$
$set MS_LN
$       GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate ln  --- this is the command name.
$ Do not translate %s etc.
$

$
ISDIR	   "ln: 0653-417 %s is a directory. (cannot unlink)\n"
$
IDENTICAL  "ln: 0653-419 %s and %s are identical.\n"
$
CANTUNLINK "ln: 0653-420 Cannot unlink %s.\n"
$
FOPT       "ln: 0653-421 %s exists.\n    Specify -f to remove %s before linking.\n"	
$
DIFFFS     "ln: 0653-422 %s and %s are located on different file systems.\n"
$
SISDIR     "ln: 0653-423 %s is a directory.  Cannot hard link.\n"
$       
NOPERM     "ln: 0653-424 Insufficient permission for %s.\n"
$
USAGE  	   "Usage: ln [-fs] src [target]\n   or: ln [-fs] src1 ... srcN directory\n"
