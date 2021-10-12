$ @(#)68	1.8  src/bos/usr/bin/env/env.msg, cmdsh, bos411, 9428A410j 3/17/94 17:19:23
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
$set    MS_ENV
$ Do not translate env and printenv.

OVERFLOW "env: 0402-044 Too many environment variables exist. Only 250 are displayed.\n"
PUSAGE "Usage: printenv [Name]\n"
EUSAGE "Usage: env [-i | -] [Name=Value]... [Command [Argument...]]\n\
       env [Name]\n"
