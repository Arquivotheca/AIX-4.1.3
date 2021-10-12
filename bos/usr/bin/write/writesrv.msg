$ @(#)13	1.7  src/bos/usr/bin/write/writesrv.msg, cmdwrite, bos411, 9428A410j 4/23/91 13:57:18
$
$ COMPONENT_NAME: (CMDCOMM) user to user communication
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

$quote "         define quote character.
$set MS_writesrv        Define initial set#

M_MSG_0 "0803-012 %s cannot create another process.\n"
$ %s is replaced by the system that cannot create the process.
E_MSG_1 "Accept"
$ will be followed by a perror message.
M_MSG_2 "writesrv: 0803-020 Reading the stream message.\n"
M_MSG_3 "writesrv: 0803-013 The service specified is not known.\n"
$ Do not translate "writesrv".
M_MSG_4 "Opening the stream socket"
$ will be followed by a perror message.
E_MSG_5 "Binding the stream socket"
$ will be followed by a perror message.
M_MSG_6 "Getting the socket name"
$ will be followed by a perror message.
M_MSG_12 "writesrv: 0803-014 Cannot open %s.\n"
$ %s is replaced by a file name.
M_MSG_13 "writesrv: 0803-015 Cannot continue.\n"
$ Do not translate "writesrv".
M_MSG_14 "writesrv: 0803-016 The specified tty line does not exist.\n"
$ tty is a unix term for a terminal
M_MSG_15 "writesrv: 0803-017 Write permission has been denied on the target tty.\n"
M_MSG_16 "writesrv: 0803-018 Cannot open the target tty line for writing.\n"
M_MSG_17 "Node     Service  Handle      Time\n"
$ These words are column heads for a table. Do not change spacing.
M_MSG_18 "\tTo reply, enter:  write -h %d,%s|%s|%s\n"
$ %d is replaced by a number string.
$ the three %s are replaced by the three possible replies.
M_MSG_19 "writesrv: 0803-019 Cannot communicate with the write command.\n"
E_MSG_20 "writesrv: accept"
$ will be followed by a perror message.
M_MSG_44 "writesrv: 0803-051  Error in multibyte character conversion.\n"
