$ @(#)44        1.8.2.1  src/bos/diag/da/disks/dhf.msg, dadisks, bos41J, 9522A_all 5/29/95 23:37:29
$
$   COMPONENT_NAME: DADISKS
$
$   FUNCTIONS: message
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1992
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

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
$set DHF_SET1
$
WE_R_TESTING " "
$
$ This panel is displayed when the test starts.
$ SCSI is a technical term.  See general notes.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ Leave line spacing as shown.  See general notes on length of title line.

WE_R_ADVANCED " "
$
$ This panel is displayed when the test starts in advanced mode.
$ SCSI is a technical term.  See general notes.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ Leave line spacing as shown.  See general notes on length of title line.

LOOPMODE_STATUS " "
$
$ This panel is displayed when in the loop test mode.
$ SCSI is a technical term.  See general notes.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ %3$6d\t sets the number of diagnostic test passes completed.
$ %4$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title line.

START_MOTOR "TESTING SCSI DISK DRIVE %1$s IN LOCATION %2$s\n\n\
The test is attempting to start the disk drive motor. This will\n\
take approximately 1 minute to complete.\n\n\
Please stand by."
$
$ This panel will be displayed if the disk drive motor has to be spun up.
$ SCSI is a technical term.  See general notes.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ Leave line spacing as shown.  See general notes on length of title line.

BAD_FORMAT "TESTING SCSI DISK DRIVE %1$s IN LOCATION %2$s\n\n\
The disk drive format is damaged.\n\
Use the Disk Media service aid to format the disk drive."
$
$ This panel will be displayed if the diagnostics detect that a problem
$ occured while the device was being formatted.
$ SCSI is a technical term.  See general notes.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ Leave line spacing as shown.  See general notes on length of title line.

WRITE_PROTECT "TESTING SCSI DISK DRIVE %1$s IN LOCATION %2$s\n\n"
PROTECT_INFO "\
The disk drive indicates that it is write protected. The disk drive can\n\
be set in the write protect mode by the program or by a jumper.\n\n\
Should the disk drive be write protected at this time?\n"
$
$ This panel will be displayed if diagnostics detects that the
$ disk drive is write protected.
$ SCSI is a technical term.  See general notes.
$ A jumper is a small connector that joins two pins on device.  The purpose
$ of the jumper is to complete a circuit that will specify a particular state.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ Leave line spacing as shown.  See general notes on length of title line.

DO_WE_CERTIFY "TESTING SCSI DISK DRIVE %1$s IN LOCATION %2$s\n\n\
No trouble was found. Media verification can be done by running\n\
the certify procedure. This will allow you to determine\n\
the number of non-recoverable and soft data errors.\n\
The certify procedure may take up to %3$d minutes to complete.\n\
Do you want to certify the disk?\n "
$
$ This panel is displayed when the 'certify' option is chosen.
$ SCSI is a technical term.  See general notes.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ Leave line spacing as shown.  See general notes on length of title line.

REPAIR_DEC "TESTING SCSI DISK DRIVE %1$s IN LOCATION %2$s\n\n\
If the Drive Electronics Card was replaced for this disk drive:\n\n\
RETURN.........to the Function Selection Menu.\n\
SELECT.........the Service Aid option.\n\
SELECT.........the Certify Disk option from the Disk Media service aid.\n"
$
$ This panel will be displayed if the Repair Verfication option was
$ chosen from the Diagnostic Mode Selection menu and the diagnostics test
$ verified that the disk drive was OK.
$ SCSI is a technical term.  See general notes.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ See general notes to on how to expand ACTION lines if necessary.
$ Leave line spacing as shown.  See general notes on length of title line.

REPAIR_HDA "TESTING SCSI DISK DRIVE %1$s IN LOCATION %2$s\n\n\
If you replaced the Drive Assembly or the Head/Disk Assembly (HDA):\n\n\
RETURN.........to the Function Selection Menu.\n\
SELECT.........the Service Aid option.\n\
SELECT.........the Format Disk option from the Disk Media service aid.\n"
$
$ This panel will be displayed if the Repair Verfication option was
$ chosen from the Diagnostic Mode Selection menu and the diagnostics test
$ verified that the disk drive was OK.
$ SCSI is a technical term.  See general notes.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ See general notes to on how to expand ACTION lines if necessary.
$ Leave line spacing as shown.  See general notes on length of title line.

BACKUP_DRIVE "TESTING SCSI DISK DRIVE %1$s IN LOCATION %2$s\n\n\
* * * WARNING! * * *\n\n\
Error Log Analysis has detected a number of non recoverable\n\
data errors. Please back up the SCSI Disk Drive %3$s and perform\n\
media maintenance on the disk drive.\n"
$
$ This panel will be displayed if error log analysis finds the
$ error data is indicating that the disk drive is going bad.
$ SCSI is a technical term.  See general notes.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ %3$s is the same as %1$s.
$ Leave line spacing as shown.  See general notes on length of title line.

PRESENTER "To continue, press Enter."
$ This line is displayed on the REPAIR_HDA and REPAIR_DEC menus.

DM_YES "YES"
$ This option is shown when a YES answer is possible.

DM_NO "NO"
$ This option is shown when a NO answer is possible.

CHOOSE_AN_OPTION "Move cursor to selection, then press Enter.\n"
$ This message is shown when a multiple selection list is presented.

NR_MEDIUM_ERROR "TESTING SCSI DISK DRIVE %1$s IN LOCATION %2$s\n\n\
A medium error was detected which is not recoverable.\n\
Perform media maintenance on the disk drive.  If the media maintenance\n\
procedure fails, replace the disk enclosure or the disk drive.\n"
$
$ The 'medium' is the disk drive magnetic recording material.
$ This panel is displayed when encountering a medium error
$ which cannot be recovered while running the CERTIFY_TU.
$ SCSI is a technical term.  See general notes.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ Leave line spacing as shown.  See general notes on length of title line.

MICROCODE_MISCMP "TESTING SCSI DISK DRIVE %1$s IN LOCATION %2$s\n\n\
The disk drive indicates that the disk microcode level is not correct.\n\n\
Use the Microcode Download service aid to load the microcode\n\
on the disk drive.\n"
$
$ These lines are displayed when encountering a microcode miscompare.
$ SCSI is a technical term.  See general notes.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ Leave line spacing as shown.  See general notes on length of title line.

PLEASE_STAND_BY "TESTING SCSI DISK DRIVE %1$s IN LOCATION %2$s\n\n\
The certify operation is in progress.\n\n\
Please stand by.\n\
\n\
%3$d%% completed.\
"
$
$ These lines are displayed when the Certify option is selected.
$ SCSI is a technical term.  See general notes.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ %3$d is the percentage completion of the certify procedure.
$ BE SURE THAT THERE ARE TWO %% AFTER THE d IN SUBSTITUTION 3$.
$ Leave line spacing as shown.  See general notes on length of title line.
 
CERTIFY_TERMINATED "TESTING SCSI DISK DRIVE %1$s IN LOCATION %2$s\n\n\
The certify operation has stopped.\n\n\
It is recommended that the data on the disk drive be backed up\n\
and that the disk drive be replaced.\n"
$
$ These lines are displayed when the Certify option is selected.
$ SCSI is a technical term.  See general notes.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.

CERTIFY_COMPLETED "TESTING SCSI DISK DRIVE %1$s IN LOCATION %2$s\n\n\
Fixed Disk Capacity.................... %3$d MB\n\
Data Errors Recovered.................. %4$d\n\
Data Errors Not Recovered.............. %5$d\n\
Equipment Check Errors Recovered....... %6$d\n\
Equipment Check Errors Not Recovered... %7$d\n\
\n\
"
$
$ Keep the substitution lined up in the same column as shown.
$ MB stands for megabyte and means 1024x1024 bytes.
$ Leave line spacing as shown.

CERTIFY_COMPLETED_BAD "TESTING SCSI DISK DRIVE %1$s IN LOCATION %2$s\n\n\
The certify operation has stopped before completion because of\n\
excessive errors that are not recoverable.\n\n\
It is recommended that the data on the disk drive be backed up\n\
and that the disk drive be replaced.\n"
$
$ Leave line spacing as shown.

DO_WE_CERTIFY2 "TESTING SCSI DISK DRIVE %1$s IN LOCATION %2$s\n\n\
No trouble was found. Media verification can be done by running\n\
the certify procedure. This will allow you to determine\n\
the number of non-recoverable and soft data errors.\n\
The certify procedure may take up to %3$d minutes to complete.\n\
Do you want to certify the disk?\n "
$
$ This panel is displayed when the 'certify' option is chosen.
$ SCSI is a technical term.  See general notes.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ Leave line spacing as shown.  See general notes on length of title line.

SCSI_DISK_DRIVE "SCSI DISK DRIVE"
$
$ This option is used in title line of menu.
$ Leave line spacing as shown.  See general notes on length of title line.

MISSING_ATTRIBUTES "%1$6X \
TESTING SCSI DISK DRIVE %2$s IN LOCATION %3$s\n\n\
\n\
A number of attributes are found to be missing from the Data Base.\n\
Unexpected results may occur due to this problem."
$
$ This menu goal is used when ODM attributes cannot be 
$ obtained.

RUN_FORMAT_SA "TESTING SCSI DISK DRIVE %1$s IN LOCATION %2$s\n\
\n\
The block size of this disk drive in not compatible with the\n\
operating system. The disk drive needs to be formatted to the\n\
correct block size.\n\
\n\
Use the Disk Media service aid to format the disk drive."

CERTIFY_COMPLETED_WARNING "\
DISK MEDIA SERVICE AID\n\n\
The certify operation has detected excessive errors that are not recoverable.\n\n\
It is recommended that the data on the disk drive be backed up\n\
and that the media maintenance procedure be performed on the drive."
$
$ Leave line spacing as shown.

