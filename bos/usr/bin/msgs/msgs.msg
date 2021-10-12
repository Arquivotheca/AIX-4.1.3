$ @(#)56 1.7  src/bos/usr/bin/msgs/msgs.msg, cmdcomm, bos411, 9428A410j 2/1/93 10:08:32
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for msgs
$
$set MS_MSGS
$		GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate %d  %1$s %2$s etc.
$ These are output formats for file names, directory
$ names, permission codes, command names, and the like,
$ depending on their context within each message. 
$ The program replaces %d,%1$d,etc with a decimal number.
$ The program replaces %s,%1$s,etc with a string of 
$ characters.   
$ In this catalog, many messages are similar and mean the
$ same thing. You can view messages in different
$ formats, hence the headings come out different.
$quote "

SORRY "msgs: 0803-020 Sorry. Check permissions.\n"
USAGE "Usage: msgs [-fhlpq] [[-]Number]\n\
\tmsgs -s\n\
\tmsgs -c [-Days]\n"
$ Do not translate msgs -- it is the command name. 
MESSAGE "Message %1$d:\nFrom %2$s %3$s\nSubject: "
FROM "From %1$s %2$s\nSubject: %3$s\n"
NOTE "Notice: The message should have a subject field.\n"
RESET "Warning: Message bounds are reset to: %1$d through %2$d.\n"
$ Bounds are the upper and lower numbers in /usr/msgs/bounds.
$ The program uses these to determine if there are messages for the
$ user.
$ The warning is to make you aware that you might miss a message or
$ that you might see the message again.
LAST "Warning: The last message is number %d.\n"
$ %d is replaced by the number of the last received message. 
$ The warning is to make you aware that you might miss a message or
$ that you might see the message again.
NEW "There are new messages.\n"
MSGFROM "Message %1$d:\nFrom %2$s %3$s\n"
MESSAG2 "Message %1$d:\nFrom %2$s %3$s"
SUBJ "Subject: %s"
POSTPON "--Postponed--\n"
$ This means that the message has been skipped at
$ the present time. Reading of the message is postponed.
NONEW "There are no new messages.\n"
KILLED "msgs: 0803-023 --Killed--"
$ This means that you have erased your message
$ before sending it.
PROMPT "%s "
$ This is a placeholder.  The %s is replaced by
$ a string of characters relevant to what you are doing.
NOTFOUND "Message %d is not found.\n"
$ You have identified a message by number that
$ does not exist.
SAVED "Message %1$d is saved in %2$s.\n"
LINE1 "(%1$d%2$slines) "
$ This is a footer line showing 
$ word and line counts -- it is not an error.
LINE2 "(%1$d%2$slines) "
$ This is a footer line showing 
$ word and line counts -- it is not an error.
DASH "\n-----\n"
$ do not translate
SEP "--%s--\n"
$ do not translate
FROM1 "From"
SUB "Subj"
NOSUB "(A subject has not been specified)\n"
LINE_1 "(%1$d lines) "
$ This is a footer line showing 
$ word and line counts -- it is not an error.
$ It is replacing LINE1.
LINE_2 "(%1$d more lines) "
$ This is a footer line showing 
$ word and line counts -- it is not an error.
$ It is replacing LINE2.
GOTO "Go to %d"
NEXT "Next message? [yn] "
$ Do not translate [yn].  It is a partial set of available subcommands.
MORE "More? [ynq] "
$ Do not translate [ynq].  It is a partial set of available subcommands.
NOMORE "(No More) [q] ? "
$ Do not translate [q].  It is a partial set of available subcommands.
REPLAY "Replay"
$ Indicates the same message header has been displayed.
FLUSHED "Flushed"
$ Indicates the new message has been skipped without being read. 
