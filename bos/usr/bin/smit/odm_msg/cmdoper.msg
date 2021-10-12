
$ @(#)95 1.1  src/bos/usr/bin/smit/odm_msg/cmdoper.msg, cmdoper, bos411, 9428A410j 3/27/90 00:57:01

$
$ COMPONENT_NAME:  (CMDOPER) cmdoper.msg
$
$ FUNCTIONS: (none)
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$set 22
$quote "

5 "yes,no"
15 "Schedule a Job"
50 "Stop the System"
55 "TIME the sytem goes down (in minutes from now)"
60 "MESSAGE to all users on the system"
65 "Set System Run Level"
70 "System RUN LEVEL (M = Maintenance mode)"
75 "0,1,2,3,4,5,6,7,8,9,M"
85 "INTERACTIVE shutdown"
90 "RESTART the system after shutdown"
95 "COMMAND or SHELL SCRIPT (full pathname)"
100 "YEAR"
105 "MONTH"
110 "Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec"
115 "DAY (1-31)"
120 "HOUR (0-23)"
125 "MINUTES (0-59)"
130 "SHELL to use for job execution"
135 "Korn (ksh),Bourne (sh),C (csh)"
140 "List All Scheduled Jobs"
145 "Remove a Job from the Schedule"
150 "JOB NUMBER to remove"
