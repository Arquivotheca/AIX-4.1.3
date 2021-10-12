$ @(#)72        1.13  src/bos/diag/da/fd/dfd.msg, dafd, bos411, 9428A410j 2/5/93 14:43:31
$
$   COMPONENT_NAME: dafd
$
$   FUNCTIONS: message file
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1993
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
$ END OF GENERAL NOTES


$quote "
$set Diskette_Diagnostics
$ Message catalog file for the diskette

Diskette_Adapter "TESTING STANDARD I/O DISKETTE ADAPTER %1$s"
$
$ This message is the only one used for the diskette adapter test
$ I/O is a technical term for Input/Output.  DO NOT TRANSLATE unless
$ an equivalent term is used in the translating language.
$ %1$s can be either "IN ADVANCED MODE" or blanks.
$ See general notes on length of title.

Testing_msg "TESTING %1$s DISKETTE DRIVE (%2$s) %3$s"
$
$ The Testing message
$ %1$s is the size of the drive  Three5 or Five25
$ %2$s is the drive location     DRIVE 1 or DRIVE 0
$ %3$s can be either             " IN ADVANCED MODE" or blanks.
$ See general notes on length of title.

Choose_an_item "Move cursor to selection, then press Enter."
$

Close_door "CLOSE..........the drive.\n"
$
$ See general notes on expanding ACTION lines.

Warning_msg "\
* * * WARNING! * * *\n\n\
THE FOLLOWING TESTS MAY DESTROY ALL DATA ON THE DISKETTE!\n\n"
Stand_by "Please stand by."
$

Advanced_Mode "IN ADVANCED MODE"
$
$ This message can be appended to either of the TESTING DISKETTE messages.
$ Translate this message with that consideration.  The total length of
$ the first line of the title can be no longer than 70 characters.

No_mode " "
$
$ This is a blank message.

Choose_not_fully_tested "Move cursor to selection, then press Enter.\n\n\
NOTE:  Skipping these tests means the diskette drive\n\
       will not be tested completely."
$
$ Leave line spacing as shown.

Yes_option "YES"
$ This option is shown when a YES answer is possible.

No_option  "NO"
$ This option is shown when a NO answer is possible.

Drive_zero  "DRIVE 0"
$
Drive_one   "DRIVE 1"
$

HC_drive "HIGH CAPACITY"
$
LC_drive "LOW CAPACITY"
$

Three5 "3.5 INCH"
$
Five25 "5.25 INCH"
$

Density_check_msg "\
REMOVE.........the diskette, if any, from the diskette drive (%1$s).\n\
ENSURE.........that the diskette is a High Capacity diskette.\n\n\
Is the diskette a High Capacity diskette?\n\n\
Move cursor to selection, then press Enter."
$ %1$s will be either "drive 0" or "drive 1".

Disk_1200 "\
High Capacity (1.2M byte)\n\
5.25 inch diskette"
$ The "M" in "M byte" means mega (million).

Disk_360  "\
Low Capacity (360K byte)\n\
5.25 inch diskette"
$ The "K" in "K byte" means thousand.


Disk_1200_insert "\
High Capacity (1.2M byte) 5.25 inch diskette\n"
$

Disk_360_insert "\
Low Capacity (360K byte) 5.25 inch diskette\n"
$

RWC_test "Diskette Read, Write, and Compare Tests - "
$
$ This message is followed by either the "High Density Compatibility"
$ or "Low Density Compatibility" message.

Disk_Change_Test "Diskette Change/Write Protect Test\n\n"
Write_protect "Diskette Write Protect Test\n\n"
$
Not_Protect_note_Three5 "\
NOTE:  The diskette must be write enabled (the write protect\n\
       tab should cover the hole).\n\n"
$
Protect_note_Three5 "\
NOTE:  The diskette must be write protected (the write protect\n\
       tab should not cover the hole).\n\n"
$

Format_note_high  "\
NOTE:  The diskette must be formatted as HIGH DENSITY.\n"
$
Format_note_low  "\
NOTE:  The diskette must be formatted as LOW DENSITY.\n"
$

HiCompatability "High Density Compatibility\n\n"
$
LowCompatability "Low Density Compatibility\n\n"
$

Is_not_protected " that is write enabled"
$
$ This line becomes a part of the Doyouhave message.  Translate in context.

Is_protected  " that is write protected"
$
$ This line becomes a part of the Doyouhave message.  Translate in context.

Is_formatted "formatted "
$
$ This line becomes a part of the Doyouhave message.  Translate in context.

Recalibrate_msg "Running Recalibration Test"
$

Select_Test "Diskette Select and Deselect Test\n\n\
The following tests are used to ensure that\n\
the selected diskette drive (%1$s) is working.\n\n\
When the test is started, the diskette drive (%2$s)\n\
in-use light will illuminate for approximately 5 seconds.\n\n\
OBSERVE........the in-use light while pressing Enter to begin the test.\n\n\
To continue, press enter."
$
$ This message is for testing the LEDs on the drives
$ %1$s is the drive location  (Drive 0 \ Drive 1)
$ %2$s is the same as %1$s
$ Leave line spacing as shown.  See General notes on expaning ACTION lines.

Diditdoit "Diskette Select and Deselect Test\n\n\
OBSERVE........the in-use light on the diskette drive (%1$s).\n\n\
Was the in-use light on for approximately 5 seconds and\n\
then did it turn off?\n\n\
Move cursor to selection, then press Enter."
$
$ This message is for testing the LEDs on the drives
$ %1$s is the drive location  (Drive 0 \ Drive 1)
$ Leave line spacing as shown.  See general notes on expaning ACTION lines.

Doyouhave "%1$s\
The following tests require the %2$s or an equivalent,\n\
formatted, scratch diskette%3$s.\n\n\
NOTE:  Skipping these tests means the diskette drive\n\
       will not be tested completely.\n\n\
Do you have the required diskette?\n\n\
Move cursor to selection, then press Enter."
$
$ This query message is used to tell the user that a diskette is required
$ %1$s is either NULL or a Test label
$ %2$s is either Disk_1440 , Disk_1200 , Disk_720 or Disk_360
$ %3$s is either NULL or Is_protected or Is_not_protected
$ The word substitutions used on this message need to be translated
$ in context.
$ Leave line spacing as shown.

Insert_Disk_Message "%1$s%2$s\
%3$s\
REMOVE.........the diskette, if any, from the diskette drive (%4$s).\n\
INSERT.........the %5$s or an equivalent, formatted,\n\
               scratch diskette into the diskette drive (%6$s).\n\
%7$s\n\
%8$s\
To continue, press Enter."
$
$ %1$s is the Test
$ %2$s is either NULL or High\Low Compatibility
$ %3$s is either NULL or WARNING
$ %4$s is the location (Drive 1 or 0)
$ %5$s is the disk type
$ %6$s is the location
$ %7$s is either NULL or "CLOSE..........the drive."
$ %8$s either NULL, Protect_note.
$ Leave line spacing as shown.  See general notes on expaning ACTION lines.

Remove_Diskette "\
REMOVE.........the diskette, if any, from the diskette drive (%1$s).\n\n\
To continue, press Enter."
$
$ Exit menu
$ %1$s is the drive location
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

Loop_msg "Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ This message is displayed during loop mode
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.

Drive_Loc_Query "Is the diskette drive that is being tested an\n\
internal drive or external drive?\n\n\
Move cursor to the selection, then press Enter."
$
$ Leave line spacing as shown.

Internal_Option "Internal Location"
$ This is a menu selection item.

External_Option "External Location"
$ This is a menu selection item.

Not_Protect_note_Five25 "\
NOTE:  The diskette must be write enabled (the write protect\n\
       tab should not cover the hole).\n\n"
$

Protect_note_Five25 "\
NOTE:  The diskette must be write protected (the write protect\n\
       tab should cover the hole).\n\n"
$

EXER_MENU "\
DIAGNOSTIC SYSTEM EXERCISER SETUP MENU\n\n\
You may decide to run the System Exerciser with media installed in\n\
a device or with no media installed in a device.  Running the test\n\
with media installed in a device is a more thorough test.\n\n\
The test for the %1$s Diskette Drive can be run with a test\n\
diskette installed.  The test will require a %2$s."
$ Used for the Sytem Exerciser set-up menu.
$ the %1$s will be either 3.5-Inch or 5.25-Inch
$ the %2$s will be one of the messages from above.

OPTION_EXER_1 "\
Run System Exerciser with Media Installed"
$ Used for the Sytem Exerciser set-up menu.

OPTION_EXER_2 "\
Run System Exerciser with No Media Installed"
$ Used for the Sytem Exerciser set-up menu.

Low_Density_check_msg "\
REMOVE.........the diskette, if any, from the diskette drive (%1$s).\n\
ENSURE.........that the diskette is a Low Capacity diskette.\n\n\
Is the diskette a Low Capacity diskette?\n\n\
Move cursor to selection, then press Enter."
$ %1$s will be either fd0 or fd1.

Disk_2880_NOPN "\
High Capacity (4M byte)\n\
Diagnostic Test Diskette"
$ The "M" in "M byte" means mega (million).

Disk_1440_NOPN "\
High Capacity (2M byte)\n\
Diagnostic Test Diskette"
$ The "M" in "M byte" means mega (million).

Disk_720_NOPN  "\
Low Capacity (1M byte)\n\
Diagnostic Test Diskette"
$ The "M" in "M byte" means mega (million).

Disk_2880_insert_NOPN "\
High Capacity (4M byte) Diagnostic Test\n\
               Diskette"
$

Disk_1440_insert_NOPN "\
High Capacity (2M byte) Diagnostic Test\n\
               Diskette"
$

Disk_720_insert_NOPN "\
Low Capacity (1M byte) Diagnostic Test\n\
               Diskette"
$
