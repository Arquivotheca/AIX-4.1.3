$ @(#)98        1.12  src/bos/usr/bin/kill/kill.msg, cmdcntl, bos411, 9428A410j 6/7/91 07:25:05
$
$ COMPONENT_NAME: (CMDCNTL) system control commands
$
$ FUNCTIONS:
$
$ ORIGINS: 27, 18
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1990, 1991
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ @OSF_COPYRIGHT@
$
$ RCSfile: kill.msg,v Revision: 1.5 (OSF) Date: 90/10/07 16:38:54 
$
$
$ messages for command kill
$

$set MS_KILL
$ Do not translate kill

$quote "

EPGROUP "0509-012 Cannot kill the specified process group.\n"
EPDENIED "0509-013 Permission denied.\n"
ESIGNAL "0509-014 Specify a signal number less than 64.\n"
ENOPROC "0509-015 The specified process does not exist.\n"
USAGE "Usage: kill [ -SignalNumber|-SignalName|-s [SignalName|SignalNumber]] {ProcessID}...\n"
$ The default signal number is -15 (software terminate).
USAGE2 "Usage: kill -l [exit_status]\n"
EKILL "kill: %d: %s\n"
BADNO   "0509-050 The specified signal number or name is not valid.\n"
PRIV    "%s: insufficient privileges\n"
BADEXIT "0509-051 invalid exit status value\n"
