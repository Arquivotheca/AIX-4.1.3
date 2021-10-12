$ @(#)89	1.3  src/bos/usr/bin/syscall/syscall.msg, cmdsh, bos411, 9428A410j 6/15/90 23:28:34
$
$ COMPONENT_NAME: (CMDSH) Bourne shell and related commands
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for syscall
$

$set MS_SYSCALL
$ Do not translate syscall

$quote "

USAGE           "Usage: syscall [-n] name [parameter ...] [; name [parameter ...] ] ...\n"
BADSYS          "syscall: 0402-140 SIGSYS received: System call not valid."
NONSYS          ": 0402-141 This is not a system call."
NOSUPP          "syscall: 0402-142 System call is not supported by this command.\n"
TOOMANY         "syscall: 0402-143 Cannot specify more than 20 system calls."
BADOP           "syscall: 0402-144 The following operator is not valid: "
RHS             "syscall: 0402-145 A size value of 1, 2, or 4 must follow the ! operator."
