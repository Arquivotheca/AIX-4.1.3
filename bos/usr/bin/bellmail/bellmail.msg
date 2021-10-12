$ @(#)06	1.9  src/bos/usr/bin/bellmail/bellmail.msg, cmdmailx, bos411, 9428A410j 10/20/93 16:36:48
$ 
$ COMPONENT_NAME: CMDMAILX bellmail.msg
$ 
$ FUNCTIONS:
$
$ ORIGINS: 3  10  27 
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "         define quote character.

$set MS_bellmail        Define initial set#

M_MSG_0              "%s: can't open %s for writing\n"
M_MSG_1              "usage: %s [-erpqt] [-d directory | -f file] [persons]\n"
M_MSG_5              "%s: cannot open %s: "
M_MSG_6              "No mail.\n"
M_MSG_7              "Your mail is being forwarded to "
M_MSG_9              "and your mailbox contains extra stuff\n"
M_MSG_10             "%s: can't open %s\n"
M_MSG_12             "usage\n"
M_MSG_14             "invalid command\n"
M_MSG_15             "%s: cannot append to %s\n"
M_MSG_16             "%s: cannot save mail\n"
M_MSG_17             "invalid command\n"
M_MSG_19             "%s: can't re-read %s\n"
M_MSG_20             "%s: can't re-write %s\n"
M_MSG_21             "%s: can't re-read %s\n"
M_MSG_22             "copyback:Too many letters\n"
M_MSG_23             "%s: can't rewrite %s\n"
M_MSG_24             "%s: cannot copy mail back\n"
M_MSG_25             "new mail arrived\n"
M_MSG_26             "copymt:Too many letters\n"
M_MSG_32             "%s: cannot reopen %s for reading\n"
M_MSG_33             "%s: can't malloc\n"
M_MSG_34             "%s: cannot create %s\n"
M_MSG_35             "Mail saved in %s\n"
M_MSG_36             "Mail saved in %s\n"
M_MSG_37             "%s: cannot save in dead letter\n"
M_MSG_38             "null name\n"
M_MSG_42             "%s: cannot pipe to mail command\n"
M_MSG_43             "unbounded forwarding\n"
M_MSG_44             "%s: can't send to %s\n"
M_MSG_45             "%s: cannot append to %s\n"
M_MSG_46             "%s: cannot append to %s\n"
M_MSG_47             "%s: error signal %d\n"
M_MSG_49             "%s: failed to create lock file %s after %d tries\n"
M_MSG_52             " remote from %s\n"
M_MSG_53             " forwarded by %s\n"
M_MSG_55             "%s: no space for temp file\n"
M_MSG_56             "q\t\tquit\n"
M_MSG_57             "x\t\texit without changing mail\n"
M_MSG_58             "p\t\tprint\n"
M_MSG_59             "s [file]\tsave (default mbox)\n"
M_MSG_60             "w [file]\tsame without header\n"
M_MSG_61             "-\t\tprint previous\n"
M_MSG_62             "d\t\tdelete\n"
M_MSG_63             "+\t\tnext (no delete)\n"
M_MSG_64             "m [user]\tmail to user\n"
M_MSG_65             "! cmd\t\texecute cmd\n"
M_MSG_66             "localhost name is not defined\n"
M_MSG_67             "biff service is not defined in /etc/services\n"
M_MSG_68             "error getting biff socket: %m\n"
M_MSG_69             "error sending message to biff: %m\n"
BL_CHOWN	"%s: error changing ownership of file"
BL_ELOCK	"%s: cannot create lock file %s: "
BL_QUOTA             "%s: disk quota exceeded\n"
