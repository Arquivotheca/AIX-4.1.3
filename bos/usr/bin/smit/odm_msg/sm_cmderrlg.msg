$ @(#)86 1.6  src/bos/usr/bin/smit/odm_msg/sm_cmderrlg.msg, cmderrlg, bos411, 9428A410j 8/11/93 14:58:35

$ @(#)cmderrlg.08 1.1  com/cmd/smit/odm_msg,3.1,9005A 1/10/90 20:16:31

$
$ COMPONENT_NAME:  (CMDERRLG) cmderrlg.08
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

$ Warning -- do not remove the '$quote "' line below.
$            It is needed to keep gencat from failing when this
$            set is merged with other sets that use other quote characters.

$ $ errlog messages
$set 8
$quote "
1 "Generate an Error Report"
2 "Schedule automatic cleaning of the error log"
3 "Clean the error log"
4 "CONCURRENT error reporting?"
5 "SUMMARY or DETAILED error report"
6 "Error CLASSES (default is all)"
7 "Error TYPES   (default is all)"
8 "Resource CLASSES (default is all)"
9 "Resource TYPES   (default is all)"
10 "Resource NAMES  (default is all)"
11 "Error ID's      (default is all)"
12 "STARTING time interval"
13 "ENDING time interval"
14 "FILENAME to send report to (default is stdout)"
15 "PRINTER to send report to"
16 "Remove entries older than this NUMBER OF DAYS"
17 "This command will run until interrupted."
18 "Press Enter to continue."
19 "1 no"
$ only translate no, leave space after 1
20 "2 yes"
$ only translate yes, leave space after 2
21 "no"
22 "yes"
23 "summary,detailed"
24 "FILENAME for error report"
25 "1 filename (defaults to stdout)"
$ do not translate 1, stdout; leave a space after 1
26 "2 printer"
$ only translate printer, leave space after 2
27 "Change / Show Characteristics of the Error Log"
28 "Maximum LOGSIZE" 
29 "LOGFILE"
30 "TEMPLATE file"
31 "MESSAGE file"
32 "Error LABELS (default is all)"
33 "SEQUENCE numbers (default is all)"
34 "Memory BUFFER SIZE"
