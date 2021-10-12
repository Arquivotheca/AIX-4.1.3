$ @(#)18	1.5  src/bos/diag/util/ufd/ufd.msg, dsaufd, bos411, 9428A410j 7/13/93 15:22:33
$ * COMPONENT_NAME: dsaufd
$ *
$ * FUNCTIONS: 
$ *
$ * ORIGINS: 27 
$ *
$ * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ * combined with the aggregated modules for this product)
$ *                  SOURCE MATERIALS
$ * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ * All Rights Reserved
$ * 
$ * US Government Users Restricted Rights - Use, duplication or
$ * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ *
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
$ Message catalog file for the diskette service aids 
$
$ $ The menus/messages are developed and displayed using 
$ $ file   /fdsamenu.c
$
$ END OF GENERAL NOTES


$quote "
$set DISKETTE_SA

DISKETTE_SA_TITLE "DISKETTE MEDIA SERVICE AID"
$
$ The top line of all of the Verify Utility displays
$ See general notes on length of title.


AID_DESCRIPTION "Diskette Media Service Aid Description:\n\n\
This service aid allows you to determine if a formatted\n\
diskette media has any defects.\n\n\
This is accomplished by reading the entire diskette\n\
to determine if there are bad sectors present.\n\n\
To continue, press Enter."
$
$ AID_DESCRITPTION is the first message displayed. It gives 
$ a general description of the Verify Utility.
$ Leave line spacing as shown.

RESULT_MESSAGE "The Diskette Verification Test has been completed.\n\n\
Diskette Location.........Drive %1$d\n\
Diskette Type.............%2$s\n\
Diskette Capacity.........%3$d\n\
Sectors Reported Bad......%4$d\n\
Sectors Reported Good.....%5$d\n\
%6$s\n\n\
To continue, press Enter."
$
$ If any bad sectors are found then this message will be displayed
$ %1$d is either (0) or (1) 
$ %2$s is the diskette type:  (3.5 Inch HI/LOW density or 5.25 Inch HI/LOW )
$ %3$d is the capacity of the diskette 1.0 or 2.0 for 3.5 inch, 
$      360 KB or 1.2 MB for 5.25 inch diskettes.
$ %4$d is the number of sectors that are good.
$ %5$d is the number of sectors that are bad.
$ %6$s is the status GOOD_DISKETT/BAD_DISKETTE 
$ Leave line spacing as shown.  Be sure to keep end of periods in same column.

GOOD_DISKETTE "The diskette verified on all sectors."
BAD_DISKETTE "The diskette did NOT verify on all sectors."


FD_SA_TESTING "\
Diskette Location.........Drive %1$d\n\
Diskette Type.............%2$s\n\n\
%3$s\n\n\
Diskette verification is in progress.\n\
Please stand by."
$
$ The above message is displayed while the tests are being run.
$ %1$d is the Drive location (0) or (1)
$ %2$s is the diskettte type:  (3.5 Inch HI/LOW density or 5.25 Inch HI/LOW )
$ %3$s is the ERROR_RECORD message or blanks.
$ Leave line spacing as shown.  Be sure to keep end of periods in same column.

ERROR_RECORD "\
Verification indicates bad sectors have been found on the diskette."

INSERT_DISK  "DISKETTE REQUIRED:  %1$s\n\n\
INSERT.........the required diskette into drive %2$d\n\
%3$s\n\n\
When finished, press Enter."
$
$ The above INSERT_DISK message is displayed after a user has selected
$ %1$s is the type of diskette that is to be verified.  
$ %2$d is the diskette drive location (0) or (1)
$ %3$s is 'CLOSE the door' (for 5.25 In) or blanks for (3.5 Inch) 
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

REMOVE_DISK  "\
REMOVE.........the diskette from drive %d.\n\n\
When finished, press Enter."
$
$ The %d is the drive location (0) or (1)
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

S_HEAD "Diskette Type And Drive Location Selection\n\n\
Move the cursor to the type of diskette that requires verification,\n\
then press Enter.\n"
$ 
$ The above message is used along with the four(4) selection listed
$ below for the DFD_SELECT_menu() function. SEE: fdsamenu.c 
$ Leave line spacing as shown.

FD_144 "Drive 0 - 2.0 MB Diskette - 2 sides / 80 tracks / 18 sectors per track"
$ $ used with S_HEAD for the DFD_SELECT_menu() function. SEE: fdsamenu.c

FD_720 "Drive 0 - 1.0 MB Diskette - 2 sides / 80 tracks /  9 sectors per track"
$ $ used with S_HEAD for the DFD_SELECT_menu() function. SEE: fdsamenu.c

FD_12  "Drive 1 - 1.2 Mb Diskette - 2 sides / 80 tracks / 15 sectors per track"
$ $ used with S_HEAD for the DFD_SELECT_menu() function. SEE: fdsamenu.c

FD_360 "Drive 1 - 360 Kb Diskette - 2 sides / 40 tracks /  9 sectors per track"
$ $ used with S_HEAD for the DFD_SELECT_menu() function. SEE: fdsamenu.c

DISK_144 "Formatted High Capacity (2.0 MB) 3.5 Inch "
$ used as the Diskette Types

DISK_720 "Formatted Low Capacity  (1.0 MB) 3.5 Inch  "
$ used as the Diskette Types

DISK_12  "Formatted High Capacity (1.2 MB) 5.25 Inch "
$ used as the Diskette Types

DISK_360 "Formatted Low Capacity  (360 KB) 5.25 Inch "
$ used as the Diskette Types

TESTING_DISK_144 "High Capacity 2.0 MB 3.5 Inch Diskette"
$ used as the Diskette Types

TESTING_DISK_720 "Low Capacity  1.0 MB 3.5 Inch Diskette"
$ used as the Diskette Types

TESTING_DISK_12  "High Capacity 1.2 MB 5.25 Inch Diskette"
$ used as the Diskette Types

TESTING_DISK_360 "Low Capacity  360 KB 5.25 Inch Diskette"
$ used as the Diskette Types

ERROR_MESSAGE "An ERROR was detected during the verification test.\n\n\
%1$s\n\n\
To continue, press Enter."
$
$ %s is the error message
$ Leave line spacing as shown.

NO_DISK "The error indicates that a diskette was NOT inserted in the drive." 

NOT_USABLE " "
$ This is a blank message.

FORMAT_ERROR "The error indicates that the diskette is NOT formatted correctly."

USER_STOPPED "Verification has been halted.\n\
Diskette status is not known."

CLOSE_DOOR  "CLOSE..........the drive."
$
$ See general notes on expanding ACTION lines.

ZIP " "
$ This is a blank message.

DEFAULT_QUERY "\
Diskette Location.........Drive %1$d\n\
Diskette Type.............%2$s\n\
Diskette Capacity.........%3$d\n\
Sectors Checked ..........%4$d\n\
Sectors Reported Bad......%5$d\n\
Sectors Reported Good.....%6$d\n\
Sectors Remaining.........%7$d\n\n\
Do you wish to continue verification of this diskette?\n\n\
Move cursor to selection, then press Enter."
$
$ %1$d is either (0) or (1) 
$ %2$s is the diskette type:  (3.5 Inch HI/LOW density or 5.25 Inch HI/LOW )
$ %3$d is the capacity of the diskette 1.0 or 2.0 for 3.5 inch, 
$      360 KB or 1.2 MB for 5.25 inch diskettes.
$ %4$d is the number of sectors that have been checked so far.
$ %5$d is the number of sectors that are bad.
$ %6$d is the number of sectors that are good.
$ %7$d is the number of sectors remaining to test.
$ Leave line spacing as shown.
$ Be sure that all variables line up at the end of the periods.

YES_OPTION "YES - Continue verification"
NO_OPTION  "NO  - Exit"

FD_288 "Drive 0 - 4.0 MB Diskette - 2 sides / 80 tracks / 36 sectors per track"
$ $ used with S_HEAD for the DFD_SELECT_menu() function. SEE: fdsamenu.c

DISK_288 "Formatted High Capacity (4.0 MB) 3.5 Inch "
$ used as the Diskette Types

TESTING_DISK_288 "High Capacity 4.0 MB 3.5 Inch Diskette"
$ used as the Diskette Types

NOT_ACCESSIBLE "The diskette drive which was selected is not accessible."
$  Used to tell the user that there isn't a disk drive of this type connected

NO_DRIVES "There are no diskette drives attached to this system."
$  Used to tell the user that there isn't a disk drive of this type connected
