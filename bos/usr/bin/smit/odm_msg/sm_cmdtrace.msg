$ @(#)03 1.5.1.2  src/bos/usr/bin/smit/odm_msg/sm_cmdtrace.msg, cmdtrace, bos411, 9428A410j 2/4/94 23:34:41

$
$ COMPONENT_NAME:  (CMDTRACE) cmdtrace.08
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

$ Warning -- do not remove the "$quote ~" line below.
$            It is needed to keep gencat from failing when this
$            set is merged with other sets that use other quote characters.

$ $ trace messages
$set 9
$quote ~
1 START Trace
2 STOP Trace
3 Generate a Trace Report
4 Show exec PATHNAMES for each event?
5 Show PROCESS IDs for each event?
6 Show CURRENT SYSTEM CALL for each event?
7 Time CALCULATIONS for report
8 IDs of events to INCLUDE in report
9 ID's of events to EXCLUDE from report
10 STARTING time
11 ENDING time
12 FILE NAME for trace report (default is stdout)
13 PRINTER to send trace report to
14 EVENT GROUPS to trace (default is kernel trace)
15 ADDITIONAL event IDs to trace
16 Event IDs to EXCLUDE from trace
17 Trace MODE
18 STOP when log file full?
19 LOG FILE 
20 Omit DATE-SYSTEM HEADER to log file?
21 Trace BUFFER SIZE in bytes
22 LOG FILE SIZE in bytes
23 Run in INTERACTIVE mode?
24 yes,no
25 Send the Trace Report to:
26 1 filename (defaults to stdout)
$ do not translate 1, stdout; leave a space after 1
27 2 printer
$ only translate printer, leave a space after 2
28 alternate,single,circular
29 elapsed+delta,short_elapsed,microseconds,none
30 LOG FILE to create report from
31 no,yes
32 SAVE PREVIOUS log file?
33 Omit PS/NM/LOCK HEADER to log file?
34 Show THREAD IDs for each event?
