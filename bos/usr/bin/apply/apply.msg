$ @(#)13        1.5  src/bos/usr/bin/apply/apply.msg, cmdsh, bos411, 9428A410j 2/22/93 14:56:06
$
$ COMPONENT_NAME: (CMDSH) bourne shell and related commands
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

$
$ messages for apply
$
$quote "
$
$set MS_APPLY
$ Do not translate apply

USAGE "Usage: apply [-aCharacter] [-Number] Command Parameter ...\n"
TOOFEWARGS "apply: 0402-041 Specify parameter(s) after the %s argument.\n"
$ %s will be some unix command that the user enters
NOSHELL "apply: 0402-042 Cannot run the shell specified by $SHELL or /usr/bin/sh.\n"
$ Do not translate /usr/bin/sh or $SHELL (it is an environment variable)
NOFORK "apply: 0402-043 The fork function failed.\n"
$ Do not translate fork
BADNUM "apply: 0402-200 Specify a positive integer.\n"
TOOLONG "apply: 0402-201 Command line exceeds implementation limit of ARG_MAX bytes.\n"
$ Do not translate ARG_MAX. 
INVMB "apply: 0402-202 Incomplete or invalid multibyte character string encountered.\n"
