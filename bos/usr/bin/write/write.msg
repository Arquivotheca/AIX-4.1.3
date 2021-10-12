$ @(#)11	1.9.1.2  src/bos/usr/bin/write/write.msg, cmdwrite, bos411, 9428A410j 3/1/94 10:08:57
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ user:  guest

$quote "         define quote character.

$set MS_write        Define initial set#
M_MSG_1 "Message from %1$s on %2$s (%3$s) [%4$s] ...\n"
$ first %s will be replaced by user id
$ second %s will be replaced by tty
$ third %s will be replaced with hostname
$ fourth %s will be replaced with the current date and time.
M_MSG_2 "write: 0803-024 Cannot find your user name or user number.\n"
M_MSG_3 "write: Cannot determine your tty line.\n"
$ The tty is the identification of the user's terminal.
M_MSG_4 "write: 0803-026 Cannot determine your host name."
M_MSG_5 "write: 0803-027 Your reply must be %1$s, %2$s, or %3$s.\n"
M_MSG_8 "write: 0803-028 Cannot open %s.\n"
M_MSG_9 "write: 0803-029 Cannot continue.\n"
M_MSG_10 "write: 0803-025 %s is not logged on.\n"
M_MSG_11 "%s is logged on at more than one place.\n"
M_MSG_12 "You are connected to %s.\n"
M_MSG_13 "Other locations are:\n"
M_MSG_15 "%1$s on %2$s is logged on at more that one place.\n"
M_MSG_16 "You are connected to %s.\n"
M_MSG_17 "Other locations are:\n"
M_MSG_19 "write: 0803-030 There is no such tty.\n"
M_MSG_21 "write: 0803-031 Permission denied.\n"
M_MSG_22 "write: Cannot create another process."
$ will be followed by a perror message
M_MSG_24 "write: 0803-032 This is not a known service.\n"
E_MSG_25 "write: Cannot make the correct connections."
$ will be followed by a perror message
M_MSG_26 "write: 0803-034 This is not a known host.\n"
E_MSG_27 "write: Cannot make the correct connection."
$ will be followed by a perror message
M_MSG_28 "write: 0803-036 %1$s is not logged on at %2$s.\n"
M_MSG_29 "write: 0803-037 There is no such tty on the remote host.\n"
M_MSG_30 "write: 0803-038 Permission denied.\n"
M_MSG_31 "write: 0803-039 Cannot open %1$s on %2$s.\n"
$ utmp file not found
M_MSG_32 "write: 0803-040 There is not enough memory available now.\n"
M_MSG_33 "write: 0803-041 The -h value on %s is not correct.\n"
M_MSG_34 "write: 0803-042 Could not send a reply.\n"
M_MSG_35 "Usage: write [-r|-q|-h Handle,%s|%s|%s] [-n Node] UserName[@Host] [Line]\n"
$ The three %s will be replaced with the three possible replies.
M_MSG_36 "write: 0803-044 Cannot get a reply.\n"
M_MSG_37 "write: 0803-045 Cannot communicate with the writesrv.\n"
$ do not translate writsrv it is the name of the daemon that has to be running
M_MSG_38 "write: malloc"
$ will be followed by a perror message
M_MSG_39 "<EOT>\n"
$ EOT stands for end of transmission.
M_MSG_40 "write: 0803-047 %s is not running.\n"
M_MSG_41 "write: 0803-048 The writesrv cannot provide this service now.\n"
M_MSG_42 "write: 0803-049 The writesrv is shutting down.\n"
UNKNOWN "UNKNOWN"
$ unknown tty, will in the place of the second %s in M_MSG_1
M_MSG_43 "write: 0803-050  Error in multibyte character conversion.\n"
M_MSG_44 "write: 0803-051 %1$s is not logged in on %2$s.\n"
