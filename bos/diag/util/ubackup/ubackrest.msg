$
$   COMPONENT_NAME: dsaubackup
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
$ END OF GENERAL NOTES


$quote "
$set BR_SET
$

BK_RST "Backup/Restore Media Service Aid\n\
  This selection provides a tool for testing tape and diskette media and\n\
  devices."

MENU_BR "BACKUP/RESTORE MEDIA SERVICE AID\n\n\
This service aid is used to test tape and diskette media and devices.  It\n\
will backup the file '/usr/lpp/diagnostics/CEREADME' to media, restore \n\
the file to /tmp, and compare the restored file to the original file.  It\n\
provides a means of showing that a backup device works."
MENU_BR_ACTION "To continue, press Enter."

MENU_DEV_SELECT "BACKUP/RESTORE MEDIA SERVICE AID\n\n\
Make sure media is installed before selecting device."
MENU_DEV_SELECT_ACTION "Move cursor to the device you want to test,\n\
then press Enter."

$ example format for disks
$  hdisk0            00-01-00-00       320 MB SCSI Disk Drive
$  hdisk1            00-01-00-10       320 MB SCSI Disk Drive

MENU_FMT_SELECT "BACKUP/RESTORE MEDIA SERVICE AID"
MENU_FMT_SELECT_TAR "tar"
MENU_FMT_SELECT_BACKUP "backup"
MENU_FMT_SELECT_CPIO "cpio"
MENU_FMT_SELECT_ACTION "Move cursor to the backup format you want to use,\n\
then press Enter."

$ When the restore device is selected, a confirmation menu will be
$ displayed. This menu will display the restore device and
$ ask if this is correct.  Yes will be the default selection.

MENU_CONFIRM    "BACKUP/RESTORE MEDIA SERVICE AID\n\n\
Test file will be backed-up using '%s' to device \n\
  %s.\n\
Is this the test you want to execute?"
MENU_CONFIRM_YES    "YES"
MENU_CONFIRM_NO     "NO"
MENU_CONFIRM_ACTION   "Move cursor to selection, then press Enter." 

MENU_BACKUP1 "BACKUP/RESTORE MEDIA SERVICE AID\n\n\
Test file is being backed up.\n\n"

MENU_BACKUP2 "BACKUP/RESTORE MEDIA SERVICE AID\n\n\
Test file has been backed-up to device %s."
MENU_BACKUP2_ACTION "To continue and restore file to /tmp, press Enter."

MENU_RESTORE1 "BACKUP/RESTORE MEDIA SERVICE AID\n\n\
Test file is being restored.\n\n"

MENU_RESTORE2 "BACKUP/RESTORE MEDIA SERVICE AID\n\n\
The restore has completed."
MENU_RESTORE2_ACTION "To continue and compare restored file to original, press Enter."

MENU_COMPARE1 "BACKUP/RESTORE MEDIA SERVICE AID\n\n\
Comparing original test file with restored test file.\n\n"

MENU_COMPARE2 "BACKUP/RESTORE MEDIA SERVICE AID\n\n\
Files compared successfully.  File sizes and checksums are:\n\n\
Original file  : %d bytes, CRC-16= 0x%04x\n\
Backed-up file : %d bytes, CRC-16= 0x%04x"
MENU_RETURN_ACTION "To return to device selection menu, press Enter."

$ If CEREADME already exists in /tmp, a confirmation menu will be
$ displayed. 

MENU_TMP_CONFIRM    "BACKUP/RESTORE MEDIA SERVICE AID\n\n\
File 'CEREADME' already exists in /tmp.  Overwrite it?"

MENU_NOFILE "BACKUP/RESTORE MEDIA SERVICE AID\n\n\
The file '/usr/lpp/diagnostics/CEREADME' does not exist."
MENU_EXIT_ACTION "To exit the service aid, press Enter."

MENU_NODEV "BACKUP/RESTORE MEDIA SERVICE AID\n\n\
There are no diskette or tape devices on this machine."

MENU_CFG_ERROR "BACKUP/RESTORE MEDIA SERVICE AID\n\n\
ERROR.  Unable to configure device %s."

MENU_BACKUP_ERR "BACKUP/RESTORE MEDIA SERVICE AID\n\n\
ERROR.  Unable to backup test file to device %s.  The system\n\
reported the following error:\n\n\
%s"

MENU_RESTORE_ERR "BACKUP/RESTORE MEDIA SERVICE AID\n\n\
ERROR.  Unable to restore test file from device %s.  The system\n\
reported the following error:\n\n\
%s"

MENU_FILE_ERR "BACKUP/RESTORE MEDIA SERVICE AID\n\n\
ERROR.  Test file did not exist on device %s."

MENU_COMPARE_ERR "BACKUP/RESTORE MEDIA SERVICE AID\n\n\
ERROR.  Files did not compare properly.  File sizes and checksums are:\n\n\
Original file  : %d bytes, CRC-16= 0x%04x\n\
Backed-up file : %d bytes, CRC-16= 0x%04x"

USAGE_MESSAGE "Usage: ubackrest [-c -f Device -F tar|backup|cpio]"
ERROR_MESSAGE "ERROR: backup, restore, or compare failed.  Run ubackrest\n\
without any command-line options to isolate the cause of\n\
the failure."
SUCCESS_MESSAGE "Backup, Restore, and CRC-16 Compare were successful.  Tested\n\
device works."
