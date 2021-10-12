$ @(#)78	1.3  src/bos/diag/da/sdisk/dh2.msg, dasdisk, bos411, 9428A410j 8/6/93 16:59:28
$
$   COMPONENT_NAME: DASDISK
$
$   FUNCTIONS: Messages
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1991,1993
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
$ These comments concern the TITLE LINES at the top the diagnostic screen.
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
$ These comments concern the technical terms used in the messages.
$   SCSI:  is an acronym for "Small Computer Systems Interface."  SCSI is
$      a technical term.  Do not translate unless an equivalent term is
$      used in the translating country.  A SCSI Drawer is a drawer that
$      installs in a rack mounted system.
$
$ The location of a resource is in the form of xx-xx-xx where x is an
$   alpha-numeric character.  The location is not translatable.  It is
$   an alpha-numeric descriptor of where the resource can be found.
$
$ END OF GENERAL NOTES
$quote "
$set DH2_SET1
$
TESTING_ADAPTER " "
$
$ This panel is displayed when the test starts.
$ %1$s is the name of the adapter.
$ %2$s is the location of the adapter.
$ Leave line spacing as shown.  See general notes on length of title line.

ADVANCED_TESTING_ADAPTER " "
$
$ This panel is displayed when the test starts in advanced mode.
$ %1$s is the name of the adapter.
$ %2$s is the location of the adapter.
$ Leave line spacing as shown.  See general notes on length of title line.

ADAPTER_LOOPMODE_STATUS " "
$
$ This panel is displayed when in the loop test mode.
$ %1$s is the name of the adapter.
$ %2$s is the location of the adapter.
$ %3$6d\t sets the number of diagnostic test passes completed.
$ %4$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title line.

NO_RESET "TESTING SERIAL DISK ADAPTER %1$s IN LOCATION %2$s\n\n\
* * * WARNING! * * *\n\n\
The full test will not be run because the resource(s) attached to\n\
the adapter are currently busy."
$
$ This panel is displayed when the device cannot be reserved.
$ %1$s is the name of the disk.
$ %2$s is the location of the disk.
$ Leave line spacing as shown.  See general notes on length of title line.

TESTING_CONTROLLER " "
$
$ This panel is displayed when the test starts.
$ %1$s is the name of the controller.
$ %2$s is the location of the controller.
$ Leave line spacing as shown.  See general notes on length of title line.

ADVANCED_TESTING_CONTROLLER " "
$
$ This panel is displayed when the test starts in advanced mode.
$ %1$s is the name of the controller.
$ %2$s is the location of the controller.
$ Leave line spacing as shown.  See general notes on length of title line.

CONTROLLER_LOOPMODE_STATUS " "
$
$ This panel is displayed when in the loop test mode.
$ %1$s is the name of the controller.
$ %2$s is the location of the controller.
$ %3$6d\t sets the number of diagnostic test passes completed.
$ %4$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title line.

TESTING_DISK " "
$
$ This panel is displayed when the test starts.
$ %1$s is the name of the disk.
$ %2$s is the location of the disk.
$ Leave line spacing as shown.  See general notes on length of title line.

ADVANCED_TESTING_DISK " "
$
$ This panel is displayed when the test starts in advanced mode.
$ %1$s is the name of the disk.
$ %2$s is the location of the disk.
$ Leave line spacing as shown.  See general notes on length of title line.

DISK_LOOPMODE_STATUS " "
$
$ This panel is displayed when in the loop test mode.
$ %1$s is the name of the disk.
$ %2$s is the location of the disk.
$ %3$6d\t sets the number of diagnostic test passes completed.
$ %4$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title line.

DO_WE_CONTINUE "TESTING SERIAL DISK DRIVE %1$s IN LOCATION %2$s\n\n\
* * * WARNING! * * *\n\n\
THE FOLLOWING TEST MAY DESTROY DATA ON THE DISK DRIVE!\n\n\
The device may be reserved by another system through the\n\
alternate adapter port. The result of continuing this test\n\
is unpredictable and may cause a loss of data.\n\n\
Do you want to continue the test?"
$
$ This panel is displayed when the device cannot be reserved.
$ %1$s is the name of the disk.
$ %2$s is the location of the disk.
$ Leave line spacing as shown.  See general notes on length of title line.

NO_MICROCODE_TITLE " "
$ 
$ Leave line spacing as shown.  See general notes on length of title.
$ Do not translate the number 870015 at the beginning of the message.
$ Leave it exactly as shown.

CHOOSE_AN_OPTION "Move cursor to selection, then press Enter.\n"
$ This message is shown when a multiple selection list is presented.

DM_YES "YES"
$ This option is shown when a YES answer is possible.

DM_NO "NO"
$ This option is shown when a NO answer is possible.

DM_PRESS_ENTER "To continue, press Enter."
$ This option is shown when an Enter is required.

SERIAL_DISK_ADAPTER "SERIAL DISK ADAPTER"
$ This message is used in the menu title line.

SERIAL_DISK_CONTROLLER "SERIAL DISK CONTROLLER"
$ This message is used in the menu title line.

SERIAL_DISK "SERIAL DISK DRIVE"
$ This message is used in the menu title line.

SERIAL_DISK_SUBSYSTEM "SERIAL DISK SUBSYSTEM"
$ This message is used in the menu goal.

SERIAL_DISK_SUBSYSTEM1 "Serial Disk Subsystem"
$ This message is used in the menu goal.

DO_WE_CONTINUE2 "TESTING SERIAL DISK DRIVE %1$s IN LOCATION %2$s\n\n\
* * * WARNING! * * *\n\n\
   THE FOLLOWING TEST MAY DESTROY DATA ON THE DISK DRIVE!\n\n\
   The device may be reserved by another system or the host system is being\n\
   locked out by another host system ( Fenced-Out ).\n\
   Select NO to abort the test or YES to continue testing.\n\n\
If this device is attached to another system, the result of continuing this \n\
test is unpredictable and may cause a loss of data.\n\n\
Do you want to continue the test?"
$
$ This panel is displayed when the device cannot be reserved.
$ %1$s is the name of the disk.
$ %2$s is the location of the disk.
$ Do Not translate the word Fenced-Out this is a terminology word use in Serial
$ Disks.
$ Leave line spacing as shown.  See general notes on length of title line.

