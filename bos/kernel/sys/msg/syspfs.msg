$ @(#)17	1.2  src/bos/kernel/sys/msg/syspfs.msg, sysproc, bos411, 9428A410j 6/11/91 10:30:05
$
$ COMPONENT_NAME:  (SYSPFS) syspfs.msg
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

$set 2
$quote "

$ 
$ MSG_SYSPFS_01 thru MSG_SYSPFS_16 must be maintained in order (the range
$ of the disk quota messages must remain intact).
$ 

MSG_SYSPFS_01 "\n%s: operation failed, user disk quota limit reached\n"
MSG_SYSPFS_02 "\n%s: operation failed, group disk quota limit reached\n"

$ MSG_SYSPFS_03 is an old message which is replaced by MSG_SYSPFS_13
MSG_SYSPFS_03 "\n%s: operation failed, user disk quota time lime reached\n"

$ MSG_SYSPFS_04 is an old message which is replaced by MSG_SYSPFS_14
MSG_SYSPFS_04 "\n%s: operation failed, group disk quota exceeded too long\n"

MSG_SYSPFS_05 "\n%s: warning, user disk quota exceeded\n"
MSG_SYSPFS_06 "\n%s: warning, group disk quota exceeded\n"
MSG_SYSPFS_07 "\n%s: operation failed, user inode quota limit reached\n"
MSG_SYSPFS_08 "\n%s: operation failed, group inode quota limit reached\n"

$ MSG_SYSPFS_09 is an old message which is replaced by MSG_SYSPFS_15
MSG_SYSPFS_09 "\n%s: operation failed, user inode quota time limit reached\n"

$ MSG_SYSPFS_10 is an old message which is replaced by MSG_SYSPFS_16
MSG_SYSPFS_10 "\n%s: operation failed, group inode quota time limit reached\n"

MSG_SYSPFS_11 "\n%s: warning, user inode quota exceeded\n"
MSG_SYSPFS_12 "\n%s: warning, group inode quota exceeded\n"

$ MSG_SYSPFS_13 replaces MSG_SYSPFS_03
MSG_SYSPFS_13 "\n%s: operation failed, user disk quota time limit exceeded\n"

$ MSG_SYSPFS_14 replaces MSG_SYSPFS_04
MSG_SYSPFS_14 "\n%s: operation failed, group disk quota time limit exceeded\n"

$ MSG_SYSPFS_15 replaces MSG_SYSPFS_09
MSG_SYSPFS_15 "\n%s: operation failed, user inode quota time limit exceeded\n"

$ MSG_SYSPFS_16 replaces MSG_SYSPFS_10
MSG_SYSPFS_16 "\n%s: operation failed, group inode quota time limit exceeded\n"


