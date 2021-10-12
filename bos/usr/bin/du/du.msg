$               "@(#)91 1.9  src/bos/usr/bin/du/du.msg, cmdfiles, bos411, 9428A410j 8/3/93 14:02:55"
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
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
$quote "
$
$ $ Message numbers 175 - 189 reserved for this file.
$
$               GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate du -- this is the command name.
$ Do not translate %s.
$set MS_DU
$
USAGE "Usage: du [-a | -s] [-rlkx] [Name ...]\n"
$
CANTFIND "du: 0653-175 Cannot find the current directory.\n"
$
NAMETOOLONG "du: 0653-176 %s\nThe specified path name exceeded 255 bytes.\n"
$ DO NOT move the %s from where it is. It will be a very long name, and
$ will "wrap".
$
TOODEEP "du: 0653-177 %s\nThe generated path name exceeded 255 bytes.\n"
$ DO NOT move the %s from where it is. It will be a very long name, and
$ will "wrap".
$
NODOTDOT "du: 0653-178 %s: Cannot change directory to .. \n"
$ Do not translate "..". This is a part of the path.
$
