$
$   COMPONENT_NAME: dsaugmcode
$
$   FUNCTIONS: messages
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1994
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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
$
$ END OF GENERAL NOTES


$quote "
$set GENERIC_SET
$

$ This is the text displayed by the Service Aid Menu for this service aid

GEN_DL "Generic Microcode Download Service Aid\n\
  This selection provides a tool for restoring and executing any\n\
  application that downloads microcode to a device."

$
$ This panel will be displayed to show which backup device contains the
$ genucode file to restore.

MENU_GEN "GENERIC MICROCODE DOWNLOAD SERVICE AID\n\n\
This service aid is used to restore a program from diskette or tape and\n\
execute it.  The restored program will do whatever is required to download\n\
microcode onto a specific adapter or device.\n\n\
The service aid restores a program named 'genucode' that has been written\n\
with the 'tar' command.  The program will be restored to /tmp."
MENU_GEN_ACTION "To continue, press Enter."

MENU_SELECT "GENERIC MICROCODE DOWNLOAD SERVICE AID\n\n\
Make sure media is installed before selecting device."
MENU_SELECT_ACTION "Move cursor to the device from which you wish to restore,\n\
then press Enter."

$ example format for disks
$  hdisk0            00-01-00-00       320 MB SCSI Disk Drive
$  hdisk1            00-01-00-10       320 MB SCSI Disk Drive

$ When the restore device is selected, a confirmation menu will be
$ displayed. This menu will display the restore device and
$ ask if this is correct.  Yes will be the default selection.

MENU_DEV_CONFIRM    "GENERIC MICROCODE DOWNLOAD SERVICE AID\n\n\
File 'genucode' will be restored and executed from device \n\
  %s.\n\
Do you want to restore from this device?"
MENU_CONFIRM_YES    "YES"
MENU_CONFIRM_NO     "NO"
MENU_CONFIRM_ACTION   "Move cursor to selection, then press Enter." 

$ If no is selected, then the service aid will return to the selection
$ menu. If yes is selected, then the restore command will be run, and the
$ restored program will be executed.
$ When the restore is complete, a completion menu will be displayed.

MENU_COMPLETE   "GENERIC MICROCODE DOWNLOAD SERVICE AID\n\n\
File 'genucode' has been restored and executed from device %s."
MENU_RETURN_ACTION "To return to device selection menu, press Enter."

$ If genucode already exists in /tmp, a confirmation menu will be
$ displayed.  No will be the default selection.

MENU_TMP_CONFIRM    "GENERIC MICROCODE DOWNLOAD SERVICE AID\n\n\
File 'genucode' already exists in /tmp.  Overwrite it?"

MENU_NODEV "GENERIC MICROCODE DOWNLOAD SERVICE AID\n\n\
ERROR: There are no diskette or tape devices on this machine."
MENU_NODEV_ACTION "To exit the service aid, press Enter."

MENU_CFG_ERROR "GENERIC MICROCODE DOWNLOAD SERVICE AID\n\n\
ERROR: Unable to configure device %s."

$ Problems occurred with the restore

MENU_RES_ERROR "GENERIC MICROCODE DOWNLOAD SERVICE AID\n\n\
ERROR: Unable to restore 'genucode' from device %s.  Check\n\
the restore media."

$ Problems occurred executing genucode

MENU_EXC_ERROR "GENERIC MICROCODE DOWNLOAD SERVICE AID\n\n\
ERROR: Unable to execute 'genucode'."

$ Problems occurred in genucode

MENU_GEN_ERROR "GENERIC MICROCODE DOWNLOAD SERVICE AID\n\n\
ERROR: 'Genucode' detected a problem while executing.\n\
        Its return code was %d."
