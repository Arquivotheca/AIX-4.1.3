$ @(#)40    1.10.2.1 src/bos/usr/bin/who/who.msg, cmdstat, bos411, 9435C411a 9/1/94 17:49:40
$
$ COMPONENT_NAME: (CMDSTAT) status
$
$ FUNCTIONS:
$
$ ORIGINS: 18, 27
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

$quote "         define quote character.

$len
$set MS_WHO        Define initial set#

NOTERM		"who: 0551-012 The process is not attached to a terminal.\n\
\tDo not run who am i as a background process.\n"
BADSWTCH	"who: 0551-013 The flag is not recognized.\n"
NOEXST		"who: 0551-014 %s either does not exist or is not readable.\n"
$
$ %s is replaced with a name.
EMPTYSLT	"There is an empty slot in the accounting file.\n"
ID		" id=%-*.*s"
$ Do NOT translate
TERMID		" term=%d exit=%d"
$ Do Not translate

AMI		"am i"
$ MAY be translated, if there is any appropriate word as in "who 'am I' ?".

NAME		"Name"
$ yes translate....eight characters maximum
TTYSTATUS	"ST"
$ Do not translate
ACTIVITY	"Activity"
$ Yes translate....8 characters maximum
WHO_LINE	"Line "
$ Do not translate
WHO_TIME	"   Time   "
$ Yes translate....10 characters max
WHO_PID		"PID"
$ Do not translate
WHO_EXIT	"/Exit"
$ Do not translate

$ New or Modified messages for options for 3.2

WHO_HOST	"Hostname"

USAGE		"\nUsage: who [-%s] [am {i,I}] [utmp_like_file]\n"
$ Do not translate [-%s] and [am {i,I}]

ACC_ENT		"\t  -A  Accounting entries\n"
OPTIONS		"\t  -a  All (AbdHlprTtu) options\n"
$ Do not translate (AbdHlprTtu)

BOOTTM		"\t  -b  Boot time\n"
DEADPROC	"\t  -d  Dead processes\n"
HEADERS		"\t  -H  Display a Header (title)\n"
PROCCNT		"\t  -l  Login processes\n"
CURTERM		"\t  -m  Information about current terminal (same as 'am i')\n"
$ Do not translate 'am i'

PROCESS		"\t  -p  Processes other than getty or user process\n"
QUICK		"\t  -q  Quick (only user and host name)\n"
RUNLEVEL	"\t  -r  Run level\n"
SHRTFRM		"\t  -s  Short form (suppress Activity and PID)\n"
$ Do not translate PID

TIMECHG		"\t  -t  Time changes\n"
TTYMSG		"\t -T,w Status of tty \n\t\t(+ writable, - not writable, x exclusive open, ? no response)\n"
USEINFO		"\t -u,i Activity and PID of shell\n"

$ note: the last two messages (TTYMSG and USEINFO) are for POSIX P1003.2a D5.
$ In POSIX - D6, the option letters "-T,w" and "-u,i" are converged into "-T"
$ and "-u", respectively.
