$ @(#)95    1.10.1.2 src/bos/usr/bin/w/w.msg, cmdstat, bos411, 9428A410j 7/8/93 14:18:59
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$ messages for command w/uptime

$quote "

$set MS_W

BADOPEN		"Cannot open %s.\n"
USAGE		"Usage: %s [ -hlsuw ] [ user ]\n"

$ Do not add or delete leading or trailing spaces to the following fields.

UP		"up"

H_USER		"User"
H_TTY		"tty"
H_LOGIN		"login@"
H_IDLE		"idle"
H_JCPU		"JCPU"
H_PCPU		"PCPU"
H_WHAT		"what"

$ singular and plural nouns;  each pairs can be in same word if the language 
$ into which they are translated  does not have singular/plural forms.

DAY		"day"
DAYS		"days"
HOUR		"hr"
HOURS		"hrs"
MINUTE		"min"
MINUTES		"mins"
USER		"user"
USERS		"users"

$ Do not add or delete leading or trailing spaces to the above  fields.

INVTIME		"(boot or current time is wrong)"

LOAD		",  load average: %.2f, %.2f, %.2f\n"

$
