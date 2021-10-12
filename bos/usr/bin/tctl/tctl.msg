$ @(#)06	1.7.1.4  src/bos/usr/bin/tctl/tctl.msg, cmdarch, bos411, 9428A410j 8/11/93 17:33:49
$
$ COMPONENT_NAME: (CMDARCH) archive files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
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

$ Error numbers 575-599 are reserved for this file.

$quote "
$set MS_TCTL

INVALID "%1$s: 0511-575 %2$s is not a recognized subcommand.\n"
$ %1$s is replaced by the command name - do not translate
$ %2$s is replaced by a string
$ do not translate the flag name

NEGA "%s: 0511-576 Specify a positive integer for the repeat count.\n"
$ %1$s is replaced by the command name - do not translate

TFAILED "failed"
$ This is part of a message of the form:
$ <tape> <command> <count> failed: <perror message>

MUSAGE "Usage:  %s [-f Device] Subcommand [Count]\n"
$ %1$s is replaced by the command name - do not translate
$ do not translate the flag name
$ This message is followed by the VALIDCMDS message

ERRWRI "tctl: 0511-577 Cannot write %d bytes: "
$ No trailing newline

ERRRD  "tctl: 0511-578 Cannot read %d bytes: "
$ No trailing newline

ERRMEM "tctl: 0511-579 There is not enough memory available now.\n"

ERRMULT "tctl: 0511-580 Pipe size must be a multiple of buffer size.\n"

INFORD "read %d bytes\n"

INFOWR "wrote %d bytes\n"

VALIDCMDS "\tValid subcommands are:\n"

NOTUNIQ "%1$s: 0511-581 %2$s is not a unique subcommand.\n"
$ %1$s is replaced by the command name - do not translate
$ %2$s is replaced by a string

TUSAGE "Usage:  %s [ -Bnv ] [ -b Blocks ] [ -p Num ] [-f Device] Subcommand [Count]\n"
$ %s is replaced by the command name - do not translate
$ do not translate the flag name
$ This message is followed by the VALIDCMDS message

CHGPARM "tctl: 0511-582 Changing parameters"
$ This message is followed by a colon and a perror message

