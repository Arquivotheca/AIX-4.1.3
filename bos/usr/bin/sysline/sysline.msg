$ @(#)26    1.6  src/bos/usr/bin/sysline/sysline.msg, cmdstat, bos411, 9428A410j 6/7/91 03:26:52
$
$ COMPONENT_NAME: (CMDSTAT) status
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for command sysline
$

$set MS_SYSLINE

$quote "

BADARG          "sysline: 0551-250 The argument %s is not valid.\n"

NOOPEN          "sysline: 0551-251 Cannot open /dev/kmem\n"
$ Do not translate /dev/kmem

NONAMLST        "sysline: 0551-252 There is no namelist.\n"

BADTIME         "sysline: 0551-253 Time is not valid. The namelist may be wrong.\n"

BADUTMP         "sysline: 0551-254 Cannot open the utmp file.\n"
$ Do not translate utmp

NOMEM           "sysline: 0551-255 There is not enough memory available now.\n"

RUNABLE         "run:%d"
$ This is the output from the sysline command. Do not translate.

M_ON            "on:"
$ This is the output from the sysline command. Do not translate.

M_OFF           "off:"
$ This is the output from the sysline command. Do not translate.

M_ARRIVAL       "Mail has just arrived"
$ This is the output from the sysline command. Do not translate.

M_MAIL          "Mail from %s "
$ This is the output from the sysline command. Do not translate.

MON             "on %s "
$ This is the output from the sysline command. Do not translate.

NO_STATUS       "sysline: 0551-256 There is no status capability for %s.\n"

M_DOWN          "%s down"
$ This is the output from the sysline command. Do not translate.

USAGE           "Usage: sysline [-b][-c][-d][-e][-h][-i][-j][-l][-m][-p][-q][-r][-s][-w] \n"

USAGE2          "               [-D] [-H Remote] [+N] \n"

BADOPT          "sysline: 0551-257 The %c option is not valid.\n"

