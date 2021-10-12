$ @(#)94	1.12  src/bos/usr/bin/csplit/csplit.msg, cmdfiles, bos411, 9428A410j 1/12/94 15:10:30
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$
$ $ Message numbers 100 - 129 reserved for this file.
$
$set MS_CSPLIT

BADLNUM "csplit: 0653-100 The specified line number %s is not valid.\n"
CANTCREAT "csplit: 0653-102 Cannot create %s.\n"
CANTOPEN "csplit: 0653-103 Cannot open %s.\n"
FILELIM "csplit: 0653-104 The maximum number of files that can be created has been exceeded.\n"
ILLOFF "csplit: 0653-106 %s: The offset is not valid.\n"
$ Do not move the %s from its current position. It is filled in with
$ the regular expression string.
ILLRPT "csplit: 0653-108 %s is not valid. Specify a numeric repeat count.\n"
$ %s is replaced with the specified repeat count.
INTSIG "csplit: 0653-109 The program was interrupted at parameter %s.\n"
MISSBRKT "csplit: 0653-110 %s: There is a missing } character.\n"
MISSDEL "csplit: 0653-111 %s: There is a missing delimiter.\n"
NOOP "csplit: 0653-112 Cannot determine what operation to repeat for %s.\n"
OUTRNG "csplit: 0653-113 %s is greater than the current line or the end of the file.\n"
PRELONG "csplit: 0653-114 The prefix %s is too long.\n"
RPT2LNG "csplit: 0653-115 The repeat count %s is larger than 11 bytes.\n"
USAGE "Usage: csplit [-ks] [-fPrefix] File Parameters ...\n"
ILLCHAR "csplit: 0653-116 Pattern has an illegal character.\n"
