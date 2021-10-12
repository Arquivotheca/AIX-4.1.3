$ @(#)80	1.9.1.1  src/bos/usr/bin/confer/confer.msg, cmdcomm, bos411, 9431A411a 8/2/94 15:57:39
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
$quote "         define quote character.

$ In the following messages where the expression %s%s%s occurs, it will 
$ usually be true that the first %s will be replaced by the name of the user,
$ the second %s will be replaced by the '@' character, and the third %s will be
$ replaced by the ttyname.

$set MS_confer        Define initial set#

M_MSG_0              "usage: %s [[-]~] [-v] [-n<name>] {user | @ttyname | user@ttyname} ...\n"
M_MSG_1              "usage: %s conference\n"

M_MSG_4              "Invalid flag (%s) ignored\n"
M_MSG_5              "User name (%s) too long. Ignored.\n"
M_MSG_6              "joinconf: 0803-055 Sorry, but you weren't invited.\n"
M_MSG_7              "Can't write to %s on %s\n"
M_MSG_8              "0803-057 Can't create %s\n"
M_MSG_9              "Type RETURN to take the floor and a blank line to relinquish it.\n\n"
M_MSG_10             "Do you want a transcript mailed to you? (n) "
M_MSG_11             "confer: 0803-058 Can't make %s\n"
M_MSG_12             "confer: 0803-059 Can't read %s\n"
M_MSG_13             "%s on %s\n"
M_MSG_14             "%s is not logged in\n"
M_MSG_15             "Nobody logged in on %s\n"
M_MSG_16             "0803-062 Can't create conference (%s)\n"

$ By *current*, we mean *created within the past 60 minutes*.
M_MSG_18             "0803-063 No current `%s' conference\n"

M_MSG_19             "\n%s%s%s is gone\n"
M_MSG_20             "Subject: Conference (%s) of %s\nTo: "
M_MSG_24             "** No transcript for %s\n"
M_MSG_26             "joinconf"
$ Do not translate joinconf.  The subcommand should not be translated.
M_MSG_31             "** %s%s%s has joined\n"
M_MSG_33             "BYE\n"
M_MSG_34             "\n\007Conference %s%srequested with %s%s%s"

$ the first %s will be replaced by the command joinconf and the second
$ %s will be replaced by the name of the confer.
M_MSG_35             "\n\007Please %s %s\n"

M_MSG_36             "excuse"
$ do not translate excuse.  The subcommand should not be translated.

M_MSG_38             "recording"
M_MSG_39             "off the record"
M_MSG_40             "0803-070 Not enough files for '!'\n"
M_MSG_41             "0803-071 No shell\n"
M_MSG_42             "** %s has excused %s%s%s\n"
M_MSG_43             "** %s not found\n"
M_MSG_44             "(off the record) "
M_MSG_45             "(verbose) ") : nullstr)

$ Do not translate the y in the following message.
M_MSG_46             "0803-073 cannot run mesg -y\n"

$ The following are new messages for AIXv320.
M_MSG_47             "0803-074 Illegal redirection of stdin or stdout.\n"
M_MSG_48             "0803-075 Illegal space after -n flag.\n"
M_MSG_49             "0803-076 Too many participants.\n"
M_MSG_50             "%s is not logged in on %s\n"
