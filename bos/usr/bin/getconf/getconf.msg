$ @(#)79        1.2  src/bos/usr/bin/getconf/getconf.msg, cmdposix, bos411, 9428A410j 1/14/93 16:27:32
$
$ COMPONENT_NAME: (CMDPOSIX) new commands required by Posix 1003.2
$
$ FUNCTIONS: 
$
$ ORIGINS: 27, 71
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ OSF/1 1.1
$
$quote "
$set MS_GETCONF
$
$ Message numbers 021 - 040 reserved for this file.
$ Do not translate getconf.

USAGE1	"Usage: getconf System_var\n"
USAGE2	"       getconf Path_var Pathname\n"
NOTVAL  "getconf: 3016-021 Specified variable is not valid on this system.\n"
NOTDEF  "undefined\n"
NOPATH  "getconf: 3016-022 Specify pathname with path_var.\n\
Usage: getconf Path_var Pathname.\n" 
