$ @(#)83 1.2.1.2 src/bos/usr/bin/smit/odm_msg/cmdcntl.msg, cmdcntl, bos411, 9428A410j 11/30/93 03:11:38

$
$ COMPONENT_NAME:  (CMDCNTL) cmdcntl.msg
$
$ FUNCTIONS: (none)
$
$ ORIGINS: 27, 83
$
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ LEVEL 1, 5 Years Bull Confidential Information
$

$set 20
$quote "

15 "Remove a Process"
20 "SIGNAL type"
25 "SIGTERM,SIGKILL"
30 "PROCESS ID"
35 "Show all Current Processes"

100 "Show Process Status"
105 "Set Initial Priority of a Process"
110 "PRIORITY number"
115 "COMMAND name"
120 "Alter the Priority of a Running Process"
125 "PRIORITY increment"
130 "PROCESS ID"

$ ps with threads information messages
135 "1 no"
$ do not translate 1 ; leave a space after 1

140 "2 yes"
$ do not translate 2 ; leave a space after 2

145 "Show THREADS information ?"
150 "Show all Current Processes and Threads"

$ Bind a Process to a Processor messages
160 "Bind a Process to a Processor"
165 "PROCESS ID"
170 "PROCESSOR ID"

$ Unbind a Process  messages
180 "Unbind a Process"
