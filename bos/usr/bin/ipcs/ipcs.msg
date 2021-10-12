$ @(#)50	1.2.1.1  src/bos/usr/bin/ipcs/ipcs.msg, cmdipc, bos411, 9428A410j 11/25/92 11:14:48
$
$ COMPONENT_NAME: (CMDIPC) ipc commands
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1991,1992
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "
$set MS_IPCS


USAGE    "usage:  ipcs [-abcmopqst] [-C corefile] [-N namelist]\n"

CHDR "T     ID     KEY        MODE       OWNER    GROUP"
CHDR2 "  CREATOR   CGROUP"
NAMELIST "ipcs: 0515-001 No valid name list found in %s.\n"
MEMORY "ipcs: 0515-002 Unable to open the memory file %s.\n"
STATUS "IPC status from %s as of %s"
QOMSG " CBYTES  QNUM"
QBMSG " QBYTES"
QPMSG " LSPID LRPID"
QTMSG "   STIME    RTIME    CTIME "
QMSG "Message Queues"
MQNIS "Message Queue facility not in system.\n"
CBARQ "Could not access remote queue\n"
MOMSG " NATTCH"
MBMSG "  SEGSZ"
MPMSG "  CPID  LPID"
MTMSG "   ATIME    DTIME    CTIME "
MMSG "Shared Memory:\n"
NOSHMEM "Shared Memory facility not in system.\n"
SBMSG " NSEMS"
STMSG "   OTIME    CTIME "
SMSG "Semaphores:\n"
SFNIS "Semaphore facility not in system.\n"
IPCSSE "ipcs: 0515-003 seek error"
$ This message is passed to the perror() function.
IPCSRE "ipcs: 0515-004 read error"
$ This message is passed to the perror() function.
NOENTRY " no-entry"
