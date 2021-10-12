$ @(#)91	1.4  src/bos/usr/bin/xsend/xsend.msg, cmdmailx, bos411, 9428A410j 4/7/93 19:07:52
$ 
$ COMPONENT_NAME: CMDMAILX xsend.msg
$ 
$ FUNCTIONS:  
$
$ ORIGINS: 10  26  27 
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for xsend
$

$quote "         define quote character.

$len 
$set MS_xsend        Define initial set#

M_SUSAGE             "usage: xsend user_name"
M_MSG_1              "enroll: output of makekey is invalid\n"
M_MSG_2              "you can only mail to one person"
M_MSG_3              "specified recipient is not enrolled (they must use the enroll command)"
M_MSG_4              "warning: recipient's key file may be subverted\n"
M_MSG_5              "error reading recipient's key file"
M_MSG_6              "Enter encryption key: "
M_MSG_7              "cannot create mail file"
M_MSG_8              "Enter encryption key: "
M_MSG_9              "zero length mail file\n"
M_MSG_10             "cannot pipe to /bin/bellmail"
M_MSG_11             "cannot get your username"
M_MSG_12             "Subject: secret mail from %s@%s\n"
M_MSG_13             "Your secret mail can be read on host %s using the \"xget\" command.\n"
M_MSG_17             "Commands are:\n"
M_MSG_18             "q	quit, leaving unread messages\n"
M_MSG_19             "n	delete current message and display next\n"
M_MSG_20             "d	same as above\n"
M_MSG_21             "<CR>	same as above\n"
M_MSG_22             "!	execute shell command\n"
M_MSG_23             "s	save message in the named file or mbox\n"
M_MSG_24             "w	same as above\n"
M_MSG_25             "?	print this list\n"
M_MSG_26             "No secret mail for %s\n"
M_MSG_28             "memory allocation error"
M_MSG_29             "you have entered the wrong encryption key for user %s\n"
M_MSG_30             "error reading key file"
M_MSG_31	     "Unknown user %s. Can't create $HOME/mbox.\n"
