$ @(#)19        1.1.4.1  src/bos/diag/util/uformat/serialdisk/ufmt.msg, dsauformat, bos41J, 9522A_all 5/29/95 22:49:29
$
$   COMPONENT_NAME: DSAUFORMAT
$
$   FUNCTIONS: Messages
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1994
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
$ The location of a resource is in the form of xx-xx-xx where x is an
$   alpha-numeric character.  The location is not translatable.  It is
$   an alpha-numeric descriptor of where the resource can be found.
$
$ These messages are used during the service aid to format scsi disks.
$
$ END OF GENERAL NOTES


$quote "
$set DUFORMAT_SET
$ 

OPTION_YES "YES"
$ This option is shown when a YES answer is possible.

OPTION_NO "NO"
$ This option is shown when a NO answer is possible.

OPTION_ENTER "To continue, press Enter."
OPTION_CHOOSE "Move cursor to selection, then press Enter."
OPTION_FORMAT_DRIVE "Format Disk"
OPTION_CERTIFY_DRIVE_MEDIA "Certify Disk"
OPTION_F_CERTIFY "Format and Certify Disk"
OPTION_F "Format"
OPTION_F_A "Format and Surface Analysis (approximate run time is three hours)"
OPTION_F_NO_CERTIFY "Format without Certify"
OPTION_DRIVE_NAME "Disk Drive in location %1$s."

$
$ %1$s is the location of the disk drive as described in the general notes.

SUB_FORMATTED "formatted"
$
$ This message is used with MENU_SELECT_DRIVE.  Translate this message
$ in the context of MENU_SELECT_DRIVE.

SUB_CERTIFIED "certified"
$
$ This message is used with MENU_SELECT_DRIVE.  Translate this message
$ in the context of MENU_SELECT_DRIVE.

SUB_FORMAT "format"
$
$ This message is used in context with other messages.  Translate this
$ message accordingly.

SUB_CERTIFY "certify"
$
$ This message is used in context with other messages.  Translate this
$ message accordingly.

MENU_SELECT_DRIVE "\
DISK MEDIA SERVICE AID\n\
\n\
Select the disk drive to be %1$s."
$ 
$ %1$s is a substitution for "formatted" or "certified."  Translate them in
$ context with this message.
$ Leave line spacing as shown. 

MENU_FORMAT_OR_CERTIFY "\
DISK MEDIA SERVICE AID\n\
\n\
Select the action desired."
$ 
$ Leave line spacing as shown. 

MENU_F_DRIVE_WARMED_UP "\
DISK MEDIA SERVICE AID\n\
\n\
NOTE:  The disk drive should be powered on a minimum of 30 minutes\n\
       prior to attempting a format operation."
$ 
$ Leave line spacing as shown. 

MENU_F_CERTIFY_AFTER "\
DISK MEDIA SERVICE AID\n\
\n\
Select the type of format desired."
$ 
$ Leave line spacing as shown. 

MENU_F_WARNING "\
DISK MEDIA SERVICE AID\n\
\n\
* * * WARNING! * * *\n\
\n\
THE FORMAT OPERATION WILL DESTROY ALL DATA ON THE DISK DRIVE.\n\n\
NOTE:  The disk drive should be powered on a minimum of 30 minutes\n\
       prior to attempting a format operation.\n\n\
Do you want to format this disk drive?"
$ 
$ Leave line spacing as shown. 

MENU_F_A_CERTIFY_AFTER "\
DISK MEDIA SERVICE AID\n\
\n\
Select the type of format desired."
$ 
$ Leave line spacing as shown. 

MENU_PLEASE_STAND_BY "\
DISK MEDIA SERVICE AID\n\
\n\
The %1$s operation is in progress.\n\n\
Please stand by.\n\
\n\
%2$d%% completed.\
"
$
$ %1$s can either be "format" or "certify."  Translate in context.
$ %2$d%% is the percent of the disk drive that has been formatted.
$  BE SURE THAT THERE ARE TWO %% AFTER THE d IN SUBSTITUTION 2$.
$ Leave line spacing as shown. 

MENU_F_COMPLETE "\
DISK MEDIA SERVICE AID\n\
\n\
The format operation has completed successfully.\n\
\n\
"
$ 
$ Leave line spacing as shown. 

MENU_C_COMPLETE "\
DISK MEDIA SERVICE AID\n\
\n\
Disk Drive Capacity................... %1$d MB\n\
Data Errors Recovered................. %2$d\n\
Data Errors Not Recovered............. %3$d\n\
Equipment Check Errors Recovered...... %4$d\n\
Equipment Check Errors Not Recovered.. %5$d\n\
\n\
"
$ 
$ Keep the substitutions lined up in the same column as shown.
$ MB stands for megabyte and means 1024x1024 bytes.
$ Leave line spacing as shown. 

MENU_C_COMPLETE_BAD "\
DISK MEDIA SERVICE AID\n\
\n\
The service aid has stopped before completion because of\n\
excessive errors that are not recoverable.\n\
\n\
It is recommended that the data on the disk drive be backed\n\
up and that the disk drive be replaced."
$ 
$ Leave line spacing as shown. 

MENU_TERMINATED "\
DISK MEDIA SERVICE AID\n\
\n\
The %1$s operation has stopped.\n\
\n\
It is recommended that diagnostics be run on this disk drive."
$ 
$ %1$s is either 'format' or 'certify.'  Translate in context.
$ Leave line spacing as shown. 

MENU_SHOULD_FORMAT "\
DISK MEDIA SERVICE AID\n\
\n\
This disk drive has been formatted with a block size that\n\
is incompatible with this operating system.\n\
\n\
To use this disk drive on this operating system, it must be reformatted."
$ 
$ Leave line spacing as shown. 

MENU_ESC_F_WARNING "\
DISK MEDIA SERVICE AID\n\
\n\
* * * WARNING! * * *\n\
\n\
The format operation is not completed.\n\n\
Stopping the format operation at this time will leave the\n\
disk drive in a state that is not usable by the system.\n\
In order to use the disk drive after stopping the format operation,\n\
the disk drive would need to be formatted again.\n\n\
Do you want to stop this format operation?"
$ 
$ Leave line spacing as shown. 

MENU_ESC_C_WARNING "\
DISK MEDIA SERVICE AID\n\
\n\
The certify operation is not completed.\n\n\
Do you want to stop this certify operation?"
$ 
$ Leave line spacing as shown. 

MENU_F_USE_FLOPPY "\
DISK MEDIA SERVICE AID\n\
\n\
The format operation cannot be performed.\n\
\n\
The disk drive you chose is not available to be formatted.\n\
To perform a format operation on this disk drive, you must use this\n\
service aid from the removable media package."
$ 
$ Leave line spacing as shown. 

MENU_C_USE_FLOPPY "\
DISK MEDIA SERVICE AID\n\
\n\
The certify operation cannot be performed.\n\
\n\
The disk drive you chose is not available to be certified.\n\
To perform a certify operation on this disk drive, you must use this\n\
service aid from the removable media package."
$ 
$ Leave line spacing as shown. 
 
CONFIG_FAIL_TITLE "DISK MEDIA SERVICE AID\n\n\
A software error has occurred.\n\n\
The disk drive cannot be configured.\n\n\
To return to the Selection Menu, press Enter."
$ 
$ Leave line spacing as shown.  See general notes on length of title.

OPEN_FAIL_TITLE "DISK MEDIA SERVICE AID\n\n\
A software error has occurred.\n\n\
The device driver cannot be opened.\n\n\
To return to the Selection Menu, press Enter."
$ 
$ Leave line spacing as shown.  See general notes on length of title.

FORMAT_IN_PROGRESS "DISK MEDIA SERVICE AID\n\n\
The Format operation is in progress.\n\n\
Please stand by.\n"
$ 
$ Leave line spacing as shown.  See general notes on length of title.

NO_HARD_DISKS "DISK MEDIA SERVICE AID\n\n\
The Disk Media Service Aid cannot find any hard disk.\n\
To return to the Selection Menu, press Enter."
$ 
$ Leave line spacing as shown.  See general notes on length of title.

MENU_NO_ATTR "DISK MEDIA SERVICE AID\n\n\
A software error has occurred.\n\n\
Unable to determine the position of the slide switch on the front panel."
$ 
$ Leave line spacing as shown.  See general notes on length of title.

OS_WARN_CONTINUE "DISK MEDIA SERVICE AID\n\n\
This service aid cannot be run on any disk currently in use by the\n\
operating system on this machine." 
$ 
$ Leave line spacing as shown.  See general notes on length of title.

OS_WARN_RETURN "DISK MEDIA SERVICE AID\n\n\
The format and certify operations cannot be performed on the disk drive\n\
in location %1$s.\n\n\
This disk drive may be in use by the operating system."
$
$ %1$s is the location of the disk drive as described in the general notes.

OS_AND_DATA_WARN "DISK MEDIA SERVICE AID\n\n\
This service aid cannot be run on any disk currently in use by the\n\
operating system on this machine.\n\n\
To perform format or certify operations on other disk drives, reboot\n\
the system in normal mode and run diagnostics."
$ 
$ Leave line spacing as shown.  See general notes on length of title.

OPTION_RETURN "To return to the Selection Menu, press Enter."
$ 
$ Leave line spacing as shown.  See general notes on length of title.

ADAPTER_CONFIGURE "\
DISK MEDIA SERVICE AID\n\
\n\
Unable to configure the SCSI device adapter.\n\
\n\
It is recommended that diagnostics be run on this adapter."
$
$ Leave line spacing as shown.  See general notes on length of title.

DEVICE_CONFIGURE "\
DISK MEDIA SERVICE AID\n\
\n\
Unable to configure the disk drive.\n\
\n\
It is recommended that diagnostics be run on this disk drive."
$
$ Leave line spacing as shown.  See general notes on length of title.

D_CLASS_DISK "DISK MEDIA SERVICE AID\n\n\
* * * WARNING! * * *\n\
\n\
THE ERASURE OPERATION WILL OVERWRITE ALL DATA ON THE DISK DRIVE\n\
AND MAY TAKE UP TO 4 HOURS TO COMPLETE.\n\n\
NOTE:  The disk drive should be powered on a minimum of 30 minutes\n\
       prior to attempting a disk erasure operation.\n\n\
If you do NOT wish to erase the disk, hit cancel."
$ 
$ Leave line spacing as shown. 


MENU_DCLASS_COMPLETE "\
DISK MEDIA SERVICE AID\n\
\n\
The disk erasure operation has completed successfully.\n\
\n\
"
$ 
$ Leave line spacing as shown. 

OPTION_DCLASS_DRIVE "Erase Disk"

SUB_D_CLASSIFIED "erasure"
$
$ This message is used with MENU_SELECT_DRIVE.  Translate this message
$ in the context of MENU_SELECT_DRIVE.

SUB_D_CLASSIFY "erasure"
$
$ This message is used in context with other messages.  Translate this
$ message accordingly.

MENU_ESC_DCLASS_WARNING "\
DISK MEDIA SERVICE AID\n\
\n\
The disk erasure operation has not completed.\n\n\
Do you want to stop the erasure of this disk?"
$ 
$ Leave line spacing as shown. 

MENU_DCLASS_SELECT "\
DISK MEDIA SERVICE AID\n\
\n\
Modify the disk erasure options.\n"
$ 
$ Leave line spacing as shown. 

OPTION_MODIFY "To modify, move cursor to selection and tab\n\
to desired value or type in new value.\n\n"
$ 
$ Leave line spacing as shown. 

NUM_OF_PATTERNS "Number of different patterns to write"
$ 
$ Leave line spacing as shown. 

PATTERN1 "Pattern #1"
$ 
$ Leave line spacing as shown. 

PATTERN2 "Pattern #2"
$ 
$ Leave line spacing as shown. 

PATTERN3 "Pattern #3"
$ 
$ Leave line spacing as shown. 

RANDOM "%1$d (Random Pattern)"
$ 
$ %1$d is the operation number which is being performed.
$ Translate in context. 
$ Leave line spacing as shown. 

PATTERN "%1$d (Pattern %2$d)"
$ 
$ %1$d is the operation number which is being performed.
$ %2$d is the pattern(1,2,or 3 ) which is being written. 
$ Translate in context. 
$ Leave line spacing as shown. 

RAND_PATTERN "Write random pattern?"
$ 
$ Leave line spacing as shown. 

READ_BLOCK "Read data from drive?"
$ 
$ Leave line spacing as shown. 

BLK_TO_READ "Input starting logical block"
$ This is the sector at which the read operation should start 
$ Leave line spacing as shown. 

NUM_OF_BLKS_TO_READ "How many blocks should be read?"
$ A block is equivalent to a sector.
$

MENU_ESC_UD_WARNING "\
DISK MEDIA SERVICE AID\n\
\n\
* * * WARNING! * * *\n\
\n\
The disk erasure procedure has not completed.\n\n\
This disk drive should be re-run through the complete\n\
disk erasure procedure if the operation is stopped\n\
at this time.\n\n\
Do you want to stop the erasure procedure?"
$ 
$ Leave line spacing as shown. 

CDEP_STAND_BY "\
DISK MEDIA SERVICE AID\n\
\n\
Pass %1$s of the disk erasure procedure is in progress.\n\n\
Please stand by.\n\
"
$
$ %1$s is the operation of the erasure.  Translate in context. 
$ Leave line spacing as shown.

CDEP_PERCENT_DONE "\
%1$d%% completed.\
"
$
$ %1$d%% is the percent of the disk drive that has been written.
$ BE SURE THAT THERE ARE TWO %% AFTER THE d IN SUBSTITUTION 2$.
$ Leave line spacing as shown.

MENU_E_USE_FLOPPY "\
DISK MEDIA SERVICE AID\n\
\n\
The disk erasure procedure cannot be performed.\n\
\n\
The disk drive selected is in use.\n\
To perform an erasure operation on this disk drive, you must\n\
first free up the device and assure no other processes are\n\
attempting to access the drive."
$ 
$ Leave line spacing as shown. 

MENU_READ_BLK "\
DISK MEDIA SERVICE AID\n\
\n\
The following is the data which was read from the drive.\n"
$ 
$ Leave line spacing as shown. 

MENU_ASK_READ_BLK "\
DISK MEDIA SERVICE AID\n\
\n\
Do you want to read a block of data?\n"
$ 
$ Leave line spacing as shown. 

NUM_OF_BLOCKS_TO_READ "\
DISK MEDIA SERVICE AID\n\
\n\
Enter the number of logical blocks that you want to view.\n"
$ A logical block is equivalent to a sector.
$ Leave line spacing as shown. 

NO_BAD_BLOCKS "\
DISK MEDIA SERVICE AID\n\n\
There were no bad blocks reallocated"
$ 
$ Leave line spacing as shown. 

BAD_BLOCKS_FOUND "\
DISK MEDIA SERVICE AID\n\n\
The following bad blocks were reallocated."
$ 
$ Leave line spacing as shown. 

BAD_BLOCKS_START "\
DISK MEDIA SERVICE AID\n\n\
The bad blocks listed can not be accessed.\n\
The disk erasure procedure will not write to the following blocks.\n"
$ 
$ Leave line spacing as shown. 

CMD_NOT_SUP "\
DISK MEDIA SERVICE AID\n\n\
The grown defect map could not be accessed. The\n\
erasure procedure may not be able to write to all areas\n\
on the disk drive.  Since all of the blocks are not accessible,\n\
this procedure should not be used to erase the drive.\n\n\
Would you like to continue with this procedure?"
$ 
$ The grown defect map is the list of bad sectors on the drive which is 
$ stored internally on the drive itself.
$ Leave line spacing as shown. 

OPTION_CONTINUE "To continue, press Enter" 
$ 
$ Leave line spacing as shown. 

UNKNOWN_ERR "\
DISK MEDIA SERVICE AID\n\
\n\
Error occurred while trying to erase the drive.\n\
\n\
It is recommended that diagnostics be run on this disk drive."
$
$ Leave line spacing as shown.  See general notes on length of title.

MENU_WRITE_FAILED "\
DISK MEDIA SERVICE AID\n\
\n\
The disk erasure operation has failed,\n\
and the disk could not be fully overwritten.\n\
\n\
It is recommended that diagnostics be run on this disk drive."
$
$ Leave line spacing as shown.

MENU_SUGGEST_FORMAT "\
DISK MEDIA SERVICE AID\n\n\
This disk erasure operation has completed.\n\n\
It is recommended that the disk drive now be formatted."
$
$ Leave line spacing as shown.


BAD_BLOCK_HEADER "cylinder / head / sector\n"
$
$ The cylinder,head, and sector are used for the location on the disk drive.
$ Leave line spacing as shown.

NO_SUCH_BLOCK "\
DISK MEDIA SERVICE AID\n\n\
Data could not be read. The block specified does not exist."
$
$ Message that is displayed when the requested block does not exist on the drive

MENU_ESC_F_WARNING_2 "\
DISK MEDIA SERVICE AID\n\
\n\
* * * WARNING! * * *\n\
\n\
The format operation has not completed.\n\n\
Stopping the format operation at this time will leave the\n\
disk drive in a state that is not usable by the system.\n\
In order to use the disk drive after stopping the format operation,\n\
the disk drive would need to be formatted again.\n\n\
Do you want to stop this format operation?"
$ 
$ Leave line spacing as shown. 

MENU_ESC_C_WARNING_2 "\
DISK MEDIA SERVICE AID\n\
\n\
The certify operation has not completed.\n\n\
Do you want to stop this certify operation?"
$ 
$ Leave line spacing as shown. 

MENU_F_USE_FLOPPY_2 "\
DISK MEDIA SERVICE AID\n\
\n\
The format operation cannot be performed.\n\
\n\
The disk drive you chose is not available to be formatted.\n\
To perform a format operation on this disk drive, you must use this\n\
service aid from the removable media package."
$ 
$ Leave line spacing as shown. 

MENU_C_USE_FLOPPY_2 "\
DISK MEDIA SERVICE AID\n\
\n\
The certify operation cannot be performed.\n\
\n\
The disk drive you chose is not available to be certified.\n\
To perform a certify operation on this disk drive, you must use this\n\
service aid from the removable media package."
$ 
$ Leave line spacing as shown. 

MENU_NO_ATTR_2 "DISK MEDIA SERVICE AID\n\n\
A software error has occurred.\n\n\
Unable to determine the position of the mode switch on the front panel."
$ 
$ Leave line spacing as shown.  See general notes on length of title.

MENU_F_SR_CERTIFY_AFTER "\
DISK MEDIA SERVICE AID\n\
\n\
Select the type of format desired."
$ 
$ Leave line spacing as shown. 

DATA_ERR "\
DISK MEDIA SERVICE AID\n\
\n\
The disk erasure operation has failed due to excessive\n\
data errors, and the disk could not be fully overwritten.\n\
\n\
It is recommended that format and certify be run on this disk drive."
$
$ Leave line spacing as shown.

EQUIP_ERR "\
DISK MEDIA SERVICE AID\n\
\n\
The disk erasure operation has failed due to equipment\n\
errors, and the disk could not be fully overwritten.\n\
\n\
It is recommended that diagnostics be run on this disk drive."
$
$ Leave line spacing as shown.

HELP "The data required for this field is a hex number"

NOHELP "No help is available for this field"

OPTION_READ_DRIVE "Read data from drive"

OPTION_WRITE_DRIVE "Write patterns to drive"

MENU_READ_OR_WRITE "\
DISK MEDIA SERVICE AID\n\
\n\
Select the action desired.\n\
\n\
NOTE:  Part of data from drive will be read when read option is selected.\n\
       Entire disk will be overwritten when write option is selected.\n"
$ 
$ Leave line spacing as shown. 

MENU_C_COMPLETE_WARNING "\
DISK MEDIA SERVICE AID\n\n\
The service aid has detected excessive errors that are not recoverable.\n\n\
It is recommended that the data on the disk drive be backed up\n\
and that the media maintenance procedure be performed on the drive."
$ 
$ Leave line spacing as shown. 

