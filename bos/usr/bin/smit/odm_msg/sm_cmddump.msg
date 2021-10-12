$ @(#)85 1.5  src/bos/usr/bin/smit/odm_msg/sm_cmddump.msg, cmddump, bos411, 9428A410j 3/8/93 10:28:46

$
$ COMPONENT_NAME:  (CMDDUMP) cmddump.10
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

$ $ dump messages
$set 10
$quote ~
1 Show Current Dump Devices
2 Change Primary Dump Device
3 Change Secondary Dump Device
4 Show Information About the Previous System Dump
5 Copy a System Dump from a Dump Device to a File
6 Format a System Dump
7 Start a Dump to the Primary Dump Device
8 Start a Dump to the Secondary Dump Device
9 Primary dump device
10 Secondary dump device
11 Copy dump image from:
12 Copy dump image to:
13 Size in bytes of dump image
14 Date of last dump
15 Dump file or dump device
16 Filename to send report to (default is stdout):
17 Printer to send report to:
18 Send the formatted dump to:
19 1 filename (defaults to stdout)
$ do not translate 1, stdout; leave a space after 1
20 2 printer
$ only translate printer, leave a space after 2
21 Input and output file blocksize for copy
22 No previous dump recorded.
23 Copy a System Dump from a Dump Device to Diskette
24 Show Estimated Dump Size
25 Change the Directory to which Dump is Copied on Boot
$ The latest dump will be copied to the specified directory when
$ the system boots or upon system reset.  This option allows you
$ to set that directory.
26 CONTINUE Boot if Copy Fails
27 DIRECTORY full pathname
