$ @(#)01 1.3.1.2 src/bos/usr/bin/smit/odm_msg/cmdstat.msg, cmdstat, bos411, 9428A410j 11/30/93 03:11:28

$
$ COMPONENT_NAME:  (CMDSTAT) cmdstat.msg
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
$set 17
$quote "

5 "yes,no"
15 "Change / Show Day and Time"
25 "Report System Activity"
30 "Report ALL activities listed below?\n\n\
 - OR any combination of the following -\n"
35 "Report use of FILE ACCESS system routines?"
40 "Report BUFFER ACTIVITY for transfers, accesses,\n\
  and cache hit ratios?"
45 "Report SYSTEM CALLS?"
50 "Report KERNEL activity?"
55 "Report MESSAGE and SEMAPHORE activities?"
60 "Report AVERAGE QUEUE LENGTH while occupied, and\n\
 percentage of time occupied?"
65 "Report PAGING statistics?"
70 "Report CPU activity?"
75 "Report status of TEXT, PROCESS, i-node, and FILE\n\
 TABLES?"
80 "Report SYSTEM SWITCHING activity?"
85 "Report TTY DEVICE activity?"
90 "Show System Level"
95 "Display ALL the options?\n\n\
  - OR any combination of the following -\n"
100 "Display the LAN NETWORK number?"
105 "Display the type of HARDWARE running the system?"
110 "Display the name of the NODE?"
115 "Display the RELEASE number of the operating\n\
 system?"
120 "Display the SYSTEM name?"
125 "Display the operating system VERSION?"

135 "YEAR (00-99)"
140 "MONTH (01-12)"
145 "DAY (1-31)"
150 "HOUR (00-23)"
155 "MINUTES (00-59)"
160 "SECONDS (00-59)"
200 "Interval LENGTH of each system activity sampling"
205 "NUMBER of sampling intervals\n"
210 "*********************************************"

300 "Show Virtual Memory Statistics"
305 "SECONDS between samples"
310 "NUMBER of samples"
315 "Show Input/Output Statistics"
320 "SECONDS between samples"
325 "NUMBER of samples"
330 "Show System Activity While Running a Command"
335 "COMMAND name"

400 "Report System Activity per Processor ?"
405 "1 no"
$ do not translate 1 ; leave a space after 1

410 "2 yes"
$ do not translate 2 ; leave a space after 2

415 "Report System Activity per Processor"
420 "PROCESSOR IDs"

$ The following messages are for menus and dialogues
$ associated with Show Lock Statistics (lockstat command)

500 "Report TYPE"
505 "1 DEFAULT Report"
$ do not translate 1 ; leave a space after 1

510 "2 USER-DEFINED Report"
$ do not translate 2 ; leave a space after 2

515 "Show default Report"
520 "Show user-defined Report"
525 "SECONDS between samples"
530 "NUMBER of samples"
535 "Print the most ACTIVE locks ?"
540 "yes,no"
545 "MAX. NUMBER of locks to analyse"
550 "MAX. NUMBER of displayed locks"
555 "REFERENCE Rate in percent"
560 "Block RATIO in percent"
565 "MIN. NUMBER of times the lock has been taken"
