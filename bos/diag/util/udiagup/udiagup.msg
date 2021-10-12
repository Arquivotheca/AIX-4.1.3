$ @(#)11	1.8  src/bos/diag/util/udiagup/udiagup.msg, dsaudiagup, bos411, 9428A410j 4/8/94 14:44:18
$
$   COMPONENT_NAME: DSAUDIAGUP
$
$   FUNCTIONS: Diagnostic message file.
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1991,1994
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$
$ GENERAL NOTES
$
$ Do not translate %d, %s, %x, %07X, or \t in any messages.  They
$   are used for word or number substitution and are noted in the
$   comments for the individual messages.  The 1$, 2$, 3$, etc,
$   within the substitutions are used to denote the order of the
$   substitutions.
$
$ These comments concern the TITLE LINES at the top of the diagnostic screen.
$   The title must be in all capital letters. The first line
$   of the title cannot be longer than 70 characters starting from 
$   column 1.  If the line is greater than 70, it may be continued on
$   the next line.  Leave line spacing as shown:  one blank line after
$   the last title line.  For example:
$
$ *****
$ TESTING PORT 12 OF THE 16-PORT ASYNCHRONOUS ADAPTER IN PLANAR SLOT 2
$ IN ADVANCED MODE
$
$ Please stand by.
$ *****
$
$ These comments concern the user ACTIONS in all caps.
$   If translations require the creation of new lines, begin the
$   new lines in the column immediately following the row of periods. 
$   For example:
$
$ *****
$ ACTION.........one line of English might require several when translated, so
$                begin the next line at the same point of the previous line.
$ ACTION.........the next action follows with no blank line preceding it.
$ *****
$
$       This file contains the corresponding messages for the menus and
$	errors generated from the UDIAGUP Utility.
$
$ END OF GENERAL NOTES

$quote "
$set DIAGUP_SET

DIAGUP_TITLE "DIAGNOSTIC UPDATE SERVICE AID"
$
$ See GENERAL NOTES on titles

DIAGUP_END	"\
To continue, press Enter."
DIAGUP_INSERT	"REMOVE.........the %1$s, if any, from the drive.\n\
INSERT.........the %2$s containing the update into the drive.\n\n\
NOTE: While processing this update, the screen will be cleared\n\
and you may be prompted to insert additional updates."
$
$	menu Insert Diskette
$ Leave line spacing as shown.  See general notes on length of title.
$ The %1$s and %2$s will be replaced with the word DISKETTE or TAPE.
$
DIAGUP_ENTER	"When finished, press Enter."
$
$	Press enter prompt  
$ Leave line spacing as shown.  See general notes on length of title.
DIAGUP_MORE	"\
This diagnostic update is complete.\n\n\
Do you have any additional diagnostic updates?\n\n\
Move cursor to selection, then press Enter."
$
$	menu More Diskette?
$ Leave line spacing as shown.  See general notes on length of title.
DIAGUP_NO	"NO"
DIAGUP_YES	"YES"
$
DIAGUP_PROLOG_NEW	"\
This service aid is used to update the diagnostics on the disk drive.\n\
The update can be from either a diskette or tape containing a\n\
valid installp image.\n\n\
The updates may be new diagnostics or an update to the existing\n\
diagnostics."
$
$ display for the Top Menu
$ Leave line spacing as shown.  See general notes on length of title.
$
DIAGUP_NOUP	"\
This update did not complete because the update media is\n\
defective or all software updates have already been\n\
installed on the disk drive.\n\n\
NOTE:  You may use the Display Configuration Service Aid\n\
       to look at all the software updates that have been installed\n\
       on the disk drive.\n\n\
Do you have any additional updates?\n\n\
Move cursor to selection, then press Enter."
$
$	menu NoUpdate
$ Leave line spacing as shown.  See general notes on length of title.
DIAGUP_PLS_STANDBY	"\
Processing update \n\n\
Please stand by.\n\n"
$
$	Standby message.
DIAGUP_NONE	"\
A diskette or tape drive is required to run this service aid. This\n\
service aid could not find either of the two devices. If either of\n\
these two devices are present on this system, run diagnostics to\n\
determine the problem."
$
$	menu No Device.
$ Leave line spacing as shown.  See general notes on length of title.
DIAGUP_EXIT	"\
To exit, press Enter."
$
DIAGUP_SELECT	"\
Select the device to install from.\n\n\
Move cursor to selection, then press Enter."
$
$	menu Select Device.
$ Leave line spacing as shown.  See general notes on length of title.
DIAGUP_TAPE	"tape"
$
$ This message is substituted for %s in DIAGUP_INSERT
$ when the tape update is chosen.

DIAGUP_DISKETTE	"diskette"
$
$ This message is substituted for %s in DIAGUP_INSERT
$ when the diskette update is chosen.
