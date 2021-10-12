$ @(#)82	1.8.1.2  src/bos/usr/bin/smit/odm_msg/cmdarch.msg, cmdarch, bos411, 9428A410j 5/5/93 17:14:59
$
$ COMPONENT_NAME:  (CMDARCH) cmdarch.msg
$
$ FUNCTIONS: (none)
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$set 21
$quote "

5 "yes,no"
10 "This option will perform a backup by inode."
15 "This option will perform a backup by name."
20 "FILESYSTEM to backup"
25 "Backup DEVICE"
30 "Restore DEVICE"
35 "Backup LEVEL (0 for a full backup)"
40 "RECORD backup in /etc/dumpdates?"
45 "FILE or DIRECTORY to backup"
50 "Current working DIRECTORY"
55 "Backup LOCAL files only?"
60 "VERBOSE output?"
65 "PACK files?"
70 "Target DIRECTORY"
75 "FILE or DIRECTORY to restore"
80 "(Leave blank to restore entire archive.)"
85 "Number of BLOCKS to read in a single input\n  operation"
