$ @(#)72      1.12.3.2  src/bos/usr/bin/vmstat/vmstat.msg, cmdstat, bos411, 9428A410j 10/13/93 05:30:41
$
$ COMPONENT_NAME: (CMDSTAT) vmstat
$
$ FUNCTIONS:
$
$ ORIGINS: 27, 83
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989,1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ LEVEL 1, 5 Years Bull Confidential Information
$ 
$set MS_VMSTAT
$quote "
$ Do not translate vmstat

MSGIOP "vmstat: 0551-117 The -i flag is not supported.\n"
MSGUSG "Usage: vmstat [ -fsi ] [Drives] [Interval] [Count]\n"

MSGS01 "%9d total address trans. faults\n"
MSGS02 "%9d page ins\n"
MSGS03 "%9d page outs\n"
MSGS04 "%9d paging space page ins\n"
MSGS05 "%9d paging space page outs\n"
MSGS06 "%9d total reclaims\n"
MSGS07 "%9d zero filled pages faults\n"
MSGS08 "%9d executable filled pages faults\n"
MSGS09 "%9d pages examined by clock\n"
MSGS10 "%9d revolutions of the clock hand\n"
MSGS11 "%9d pages freed by the clock\n"
MSGS12 "%9d backtracks\n"
MSGS13 "%9d lock misses\n"
MSGS14 "%9d free frame waits\n"
MSGS15 "%9d extend XPT waits\n"
MSGS16 "%9d pending I/O waits\n"
MSGS17 "%9d start I/Os\n"
MSGS18 "%9d iodones\n"
MSGS19 "%9d cpu context switches\n"
MSGS20 "%9lu device interrupts\n"
MSGS21 "%9lu software interrupts\n"
MSGS22 "%9d traps\n"
MSGS23 "%9d syscalls\n"
MSGF01 "%d forks\n"

$ WARNING: Be careful in translating the following as they are
$          used as headers for column outpus from vmstat as follows:
$ kthr     memory             page              faults        cpu      disk xfer
$ ----- ----------- ------------------------ ------------ ----------- -----------
$  r  b   avm   fre  re  pi  po  fr   sr  cy  in   sy  cs us sy id wa  1  2  3  4
$  0  0  6805    29   0   2   2  11   72   0 172  203  80  8  8 72 12  7

MSGH01 "kthr     memory             page           "
MSGH02 "   faults        cpu     "
MSGH03 " r  b   avm   fre  re  pi  po  fr   sr  cy "
MSGH04 " in   sy  cs us sy id wa "
MSGH05 " disk xfer"
MSGH06 " 1  2  3  4"
MSGH07 "priority level    type   count module(handler)\n"


$ WARNING: Be careful in translating MSGH07 as it heads MSGH08 as follows:

$ priority level    type   count module(handler)
$     0       0   hardware     0 i_misc(18b5c)
$     0      24   hardware     0 i_epow(18b68)
$     0      24   hardware     0 /etc/drivers/hscsidd(1391d54)

$          The fields width specifiers must be carefuly changed to if
$          translation affects the format of the output between 
$          messages MSGH07 MSGH08.

MSGH08 "%5d %7d   %s %5d %s(%x)\n"
$ Do not translate

MSGE01 "vmstat: 0551-118 The interval value is not valid.\n"
MSGE02 "vmstat: 0551-119 The count value is not valid.\n"
MSGE03 "vmstat: 0551-120 Read error on vmminfo structure."
$ Do not translate vmminfo
MSGE04 "vmstat: 0551-121 Read error on sysinfo structure."
$ Do not translate sysinfo
MSGE05 "vmstat: 0551-122 Read error on lbolt."
$ Do not translate lbolt
MSGE06 "vmstat: 0551-123 Read error on vmmrate structure."
$ Do not translate vmmrate
MSGE07 "vmstat: 0551-124 Read error on sysrate structure."
$ Do not translate sysrate
MSGE08 "vmstat: 0551-125 Read error on vmker structure."
$ Do not translate vmker
MSGE09 "vmstat: 0551-126 Cannot open name list."
MSGE11 "vmstat: 0551-127 Read error on iostat structure."
$ Do not translate iostat
MSGE12 "vmstat: 0551-128 Drive %s is not found.\n"
MSGE13 "vmstat: 0551-129 knlist failed."
$ Do not translate knlist
MSGE14 "vmstat: 0551-130 Permission Denied.\n"

