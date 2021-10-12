$ @(#)04	1.3  src/bos/diag/util/udiskenh/udiskmnt.msg, dsaudiskenh, bos411, 9435A411a 8/22/94 11:19:09
$
$   COMPONENT_NAME: DSAUDISKMNT
$
$   FUNCTIONS: Diagnostic message file.
$		
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
$ These comments concern the technical terms used in the messages.
$   SCSI:  is an acronym for "Small Computer Systems Interface."  SCSI is
$      a technical term.  Do not translate unless an equivalent term is
$      used in the translating country.  A SCSI Drawer is a drawer that
$      installs in a rack mounted system.
$
$ END OF GENERAL NOTES


$quote "

$set TITLES

SA_NAME "Disk Maintenance \n\
   Disk to disk copy, display/alter sectors." 

MAIN_TITLE "\
DISK MAINTENANCE SERVICE AID\n"

DISK_TO_DISK_COPY_TITLE "\
DISK MAINTENANCE SERVICE AID \n\n\
Disk to Disk Copy" 

DISP_ALTER_TITLE "\
DISK MAINTENANCE SERVICE AID \n\n\
Display/Alter Sector"

$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

$set SCROLLABLES
$ Main menu
DISK_TO_DISK_COPY "\
Disk to Disk Copy \n\
     Copies the entire contents of one disk (including the volume ID) to\n\
     another disk. Assumes the target disk is formatted.  All data on the \n\
     target disk is lost."

DISP_ALTER "Display/Alter Sector \n\
     Allows selected sectors of a selected disk to be displayed and altered."

$ Disk copy confirmation
DONT_COPY "Do not Copy"

DO_COPY "Copy"

$ Read error detected
CONTINUE_COPYING "Continue Copying"

ABORT_COPY "Abort Copy Process"

$ Target disk in LVM
DISREGARD "Disregard Warning"

TERMINATE "Terminate"

$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

$set INSTRUCTIONS

SELECT_INSTRUCTION "\
Move cursor to selection, then press Enter."

ENTER_TO_CONTINUE "\
Press Enter to continue."

SOURCE_DISK_INSTRUCTION "\
Move the cursor to the disk you want to copy from (source), then press Enter."

TARGET_DISK_INSTRUCTION "\
Move the cursor to the disk you want to copy to (target), then press Enter."

TARGET_DISK_IN_LVM "A valid LVM record exists on the target disk\n\
(%s), \n\
indicating that the disk is in an LVM Volume Group.  If you believe the \n\
target disk is NOT a member of an LVM Volume \n\
Group (for example, the disk was taken from another system and never added \n\
to a Volume Group on this system), you may disregard this warning.\n\
If you believe the disk IS a member of an LVM Volume Group, you should \n\
terminate this procedure and select a different target or delete the disk \n\
from its Volume Group before proceeding. \n\n\
Move cursor to selection and then press Enter."

DISK_TO_DISK_CONF "Source disk (%s) will be copied\n\
to target disk (%s).\n\n\
The size of the source disk is %d sectors.\n\
The size of the target disk is %d sectors.\n\
(If the target is larger than the source, the additional space may be\n\
inaccessible after the copy is complete.)\n\n\
* * * WARNING! * * * \n\
THE COPY OPERATION WILL DESTROY ALL OLD DATA ON THE TARGET DISK. \n\n\
Move cursor to selection and then press Enter."

$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

$set GENERAL

AT_LOCATION "%s at location %s"

PROGRESS "Copying %s to \n\
%s. \n\n\
%d%% complete.\n\n\
Please stand by."

UNHANDLED_DISK_READ_ERROR "An unrecoverable read error has occurred on the \n\
source disk at sector %s.  This procedure is terminating.  You may need to\n\
run diagnostics on the source disk (%s)."

DISK_WRITE_ERROR "The maximum number of write errors allowed on the target \n\
disk (%s) \n\
has been exceeded. This procedure is terminating.  \n\
You may need to run diagnostics and/or replace the disk."

TARGET_DISK_WONT_OPEN "Unable to open target disk \n\
(%s), \n\
possibly due to conflict with another user or program."

UNUSED_SOURCE_DISK_WONT_OPEN "Unable to open source disk \n\
(%s), \n\
possibly due to conflict with another user or program."
 
LESS_THAN_TWO_DISKS "Less than two disk drives were found in the Customized \n\
Device Database, or less than two disks could be opened to read their\n\
capacities.  At least two disk drives must be available to perform the \n\
Disk to Disk Copy operation."

SOURCE_DISK_WONT_OPEN_EACCESS "Unable to open source disk \n\
(%s), \n\
due to conflict with another user or program.  One solution is to run \n\
diagnostics from removeable media (that is, in standalone mode). \n\
\n\
If you want to perform the copy while in concurrent mode, you will need to \n\
prevent other access to the disk by unmounting any filesystems on the disk \n\
or, if the disk is in an LVM Volume Group, remove the disk from the Volume \n\
Group or vary the Volume Group off."

SOURCE_DISK_WONT_OPEN_OTHER "Unable to open source disk \n\
(%s), \n\
due to possible hardware error.  You may need to run diagnostics on the \n\
disk and SCSI adapter."

CANT_READ_BBDIR "Unable to copy the source disk \n\
(%s), \n\
because a read error occurred while reading the LVM record or the Bad \n\
Block Directory, or the Bad Block Directory is corrupt."


$set DISK_TO_DISK_SET

DISK_TO_DISK_INFO "This procedure is intended to support the replacement of an\n\
existing disk drive with a new disk drive of the same size and type.\n\
If you have not already done so, \n\
install the new disk drive by setting its SCSI address to an address that is \n\
not currently in use and installing it in an empty location.  Both the old \n\
(source) and new (target) disk drives must be installed and available for use \n\
in order to perform the Disk to Disk Copy operation.\n\n\
Once the copy is complete, only one of the disks may remain installed.\n\
Either remove the target disk to return to the original configuration OR \n\
perform the following procedure to complete the replacement of the old \n\
(source) disk with the new (target) disk: \n\
1. Remove both disks. \n\
2. Set the SCSI address of the new (target) disk to the SCSI address \n\
   of the old (source) disk. \n\
3. Install the new disk in the old disk's location."

DISK_TO_DISK_COMPLETION "The source disk (%s) \n\
has been copied to\n\
the target disk (%s). \n\n\
The system cannot operate normally with both drives.  You may return to the \n\
original configuration by removing and undefining the target disk, or complete\n\
the replacement of the source disk by physically removing both disks, setting \n\
the SCSI ID of the target disk to the that of the source disk, and physically\n\
installing the target disk at the source disk's original location. "

UNUSED_NOT_SAME_SIZE "There are no other disks to copy to that are the same size as \n\
the source disk (%s). \n\
Procedure terminating."

NOT_SAME_SIZE "There are no other disks to copy to that are not smaller than \n\
and not more than 10%% larger than the source disk\n\
(%s). \n\
Procedure terminating."

$set DISP_ALTER_SET

DISP_ALTER_INFO "*** WARNING *** \n\
Inappropriate modifications to some disk sectors may result in total loss of \n\
all data on the disk drive."

ENTER_SECTOR "Enter the sector number (in decimal) that you would like to \n\
display/alter on the selected disk \n\
(%s):"

SECTOR_NUMBER "Sector number (0-%d):"

DESC_STR "Sector %d on %s."

INST_STR "Use tab key to switch between hex and ASCII, F7 to save."

OFFSET_STR "Offset:"

KEYS_STR "F1=Help   F3=Cancel  F7=Commit  F10=Exit"

$ "Hex" is short for hexadecimal and probably should not be translated.
HEX_WINDOW "Hex Window"

$ "ASCII" is an acronym that should not be translated.
ASCII_WINDOW "ASCII Window"

WRITE_CONFIRMATION "Are you sure you want to save the changes to sector %d?\n\n\
Press Enter to save changes and return to the edit panel, \n\
Cancel to return to the edit panel without saving changes."

NO_DISKS "No disk drives were found in the Customized Device Database."

DISK_WONT_OPEN "Unable to open the selected disk\n\
(%s),\n\
possibly due to conflict with another user or program."

DISP_ALTER_READ_ERROR "Sector %s is unreadable."

DISP_ALTER_WRITE_ERROR "Unable to write sector %s."

