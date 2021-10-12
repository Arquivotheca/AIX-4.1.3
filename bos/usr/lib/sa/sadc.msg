$ @(#)76        1.5.1.2  src/bos/usr/lib/sa/sadc.msg, cmdstat, bos411, 9428A410j 1/27/94 09:10:34
$
$ COMPONENT_NAME: (CMDSTAT) status
$
$ FUNCTIONS:
$
$ ORIGINS: 27 83
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ LEVEL 1, 5 Years Bull Confidential Information
$
$ messages for command sadc
$

$quote "
$set MS_SADC
$ Do not translate kmem


KMEM                    "0551-104 Cannot open kmem."
$ Do not translate kmem
NO_TAPE                 "0551-105 Tape location is not found."
DISK_READ               "0551-106 diskcnt read failed."
$ Do not translate diskcnt
NODISK                  "0551-107 Disk location is not found."
LSEEK_VAR               "0551-108 lseek to var location failed."
$ Do not translate lseek, var
BADKER                  "sadc: 0551-109 Warning: The kernel data structures are not consistent.\n"
BADREADSI               "0551-110 Cannot read sysinfo."
$ Do not translate sysinfo
BADREADCPU              "0551-114 Cannot read cpuinfo."
$ Do not translate cpuinfo 
BADREADKER              "0551-111 Cannot read vmkerdata."
$ Do not translate vmkerdata
VMDERDATA               "0551-112 Cannot read vmkerdata."
$ Do not translate vmkerdata
SYSERRREAD              "0551-113 The syserr read failed."
$ Do not translate syserr
