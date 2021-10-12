$ @(#)18        1.7  src/bos/diag/da/scsitapes/datape.msg, datape, bos411, 9428A410j 5/31/94 17:53:29
$
$   COMPONENT_NAME: DATAPE
$
$   FUNCTIONS: ncartridge
$
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
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
$set DTAPE_SET

$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$ $$ Selection menu messages.

TEST_REQS " "
$ (not used)
$

SYSX_REQS " "
$ (not used)
$

$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$ $$ Menu Selections.

MOVE_CURSOR "Move cursor to selection, then press Enter.\n"
$

TC_PRESS_ENTER "To continue, press Enter."
$

WF_PRESS_ENTER "When finished, press Enter."
$

YES "YES"
$

NO "NO"
$

MEDIA "\
Run System Exerciser with Media Installed."
$

NO_MEDIA "\
Run System Exerciser with No Media Installed"
$

$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$ $$ Misc messages.

TITLE " "
$ (not used)
$

ADV_MODE "IN ADVANCED MODE"
$

CUST_MODE " "
$ 1 blank space.

STAND_BY "\
Please stand by."
$

ENABLE "enabled"
$

PROTECT "protected"
$

COMPLETE "Testing is complete.\n"
$

LOOPMODE "\
  %1$ld passes completed.\n\
  %2$ld errors logged.\n"
$
$ %1$s = Loop counter (not translated).
$ %2$s = Loop errors (not translated).
$

LONG_TEST "\
This test may take up to %1$s minutes to complete.\n"
$
$ %1$s = Test Time (not translated).
$


REMOVE_TAPE "\
REMOVE.........the tape, if any, from the drive.\n"
$

ENSURE_TAPE "\
ENSURE.........the diagnostic test tape is write %1$s\n\
	       %2$s.\n"
$
$ %1$s = ENABLE or PROTECT message.
$ %1$s = one of the TXXX_WEWP messages.
$

INSERT_TAPE "\
INSERT.........the diagnostic test tape into the drive.\n\
	       Close the drive door, if applicable."
$

CHECK_TAPE "\
This tape drive is reporting that the diagnostic test tape is write %1$s.\n\
\n\
This test requires the diagnostic test tape to be write %2$s."
$
$ %1$s = Catalog message ENABLE or PROTECT.
$ %2$s = Catalog message PROTECT or ENABLE.
$

NO_TAPE "\
This tape drive indicates the test tape is not in the drive.\n"
$

$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$ $$ Device specific messages.

OST_PN   "."
T3600_PN "\ncartridge (Part Number 92X7510)."
T3800_PN "\ncartridge (Part Number 21F8586)."
T4100_PN "\ncartridge (Part Number 21F8734)."
T8200_PN "\ncartridge (Part Number 21F8577)."
T8505_PN "\ncartridge (Part Number 21F8577)."
T4MM_PN  "\ncartridge (Part Number 21F8762)."
T9348_PN "."
$ Test Tape Part Numbers.
$   NOTE: Messages may start with a new line (\n) character
$         and will end with a period.

OST_WE   "(set to write enable)"
T3600_WE "(indicator should point away from safe)"
T3800_WE "(indicator should point away from safe)"
T4100_WE "(indicator should point away from safe)"
T8200_WE "(square tab should be in the open position)"
T8505_WE "(square tab should be in the open position)"
T4MM_WE  "(square tab should be in the closed position)"
T9348_WE "(place ring on bottom of reel)"
$ Write Enable help messages.
$
OST_WP   "(set to write protect)"
T3600_WP "(indicator should point toward safe)"
T3800_WP "(indicator should point toward safe)"
T4100_WP "(indicator should point toward safe)"
T8200_WP "(square tab should be in the closed position)"
T8505_WP "(square tab should be in the closed position)"
T4MM_WP  "(square tab should be in the open position)"
T9348_WP "(remove ring from bottom of reel)"
$ Write Protect help messages.
$

$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$ $$ Menu Goals.

MNG_ELA_MEDIA "%1$06.6lx \
SERVICE PROCEDURE FOR TAPE DRIVE (%2$s) IN LOCATION %3$s\n\
\n\
No problems were found, however, Error Log Analysis\n\
indicates this tape drive has one or more temporary media\n\
error entries in the System Error Log.\n\
\n\
The following service procedures are recommended.\n\
\n\
CLEAN..........the tape drive read/write head and tape path.\n\
	       For instructions on cleaning, refer to the\n\
	       Setup and Operator Guide for this tape drive.\n\
\n\
TRANSFER.......data on old media to new media.\n\
	       NOTE: Old or worn out media may continue to cause\n\
	             media errors.\n\
\n\
RUN............Advanced Diagnostics, in System Verification mode,\n\
	       to ensure this tape drive is functioning properly."
$
$ %1$l = Menu Goal screen number (not translated).
$ %2$s = Device name.
$ %3$s = Device location.
$ NOTE TO TRANSLATOR: The space at the end of the first line is
$                     required. Do not remove.

MNG_TAPE_SERVICE_HINTS "%1$06.6lx \
ADDITIONAL INFORMATION FOR TAPE DRIVE (%2$s) IN LOCATION %3$s\n\
\n\
This tape drive was not able to complete the tests sucessfully.\n\
\n\
This problem may be caused by something on the SCSI bus other\n\
than this tape drive.\n\
\n\
Please refer to the Diagnostic Service Hints for more information.\n\
\n\
The Service Hints can be viewed by running the Service Hints\n\
Service Aid."
$
$ %1$l = Menu Goal screen number (not translated).
$ %2$s = Device name.
$ %3$s = Device location.
$ NOTE TO TRANSLATOR: The space at the end of the first line is
$                     required. Do not remove.
$

MNG_DISP_SENSE_DATA "%1$06.6lx \
ADDITIONAL INFORMATION FOR TAPE DRIVE (%2$s) IN LOCATION %3$s\n\
\n\
This tape drive returned a check condition with the following sense data:\n\
\n\
\t\tSense Key = %4$02.2X\tASC/ASCQ = %5$04.4X\n\
\n\
%6$s\n\
This data may be requested by your support personnel for additional analysis."
$
$ %1$l = Menu Goal screen number (not translated).
$ %2$s = Device name.
$ %3$s = Device location.
$ %4$s = Sense Key (not translated)
$ %5$s = Sense Code (not translated)
$ %6$s = Sense Data bytes (not translated)
$ NOTE TO TRANSLATOR: The space at the end of the first line is
$                     required. Do not remove.
$

MNG_CLEAN_NEW_TTAPE "%1$06.6lx \
SERVICE PROCEDURE FOR TAPE DRIVE (%2$s) IN LOCATION %3$s\n\
\n\
This tape drive is reporting a media error.\n\
\n\
The following service procedures are recommended.\n\
\n\
CLEAN..........the tape drive read/write head and tape path.  For\n\
	       instructions on cleaning, refer to the Setup and \n\
	       Operator Guide for this tape drive.\n\
\n\
OBTAIN.........a new diagnostic test tape.\n\
	       NOTE: An old or worn out diagnostic test tape\n\
	             may cause media errors.\n\
\n\
RUN............Advanced Diagnostics, in System Verification mode,\n\
	       to ensure this drive is functioning properly."
$
$ %1$l = Menu Goal screen number (not translated).
$ %2$s = Device name.
$ %3$s = Device location.
$ NOTE TO TRANSLATOR: The space at the end of the first line is
$                     required. Do not remove.
$

MNG_CLEAN_DRIVE "%1$06.6lx \
ADDITIONAL INFORMATION FOR TAPE DRIVE (%2$s) IN LOCATION %3$s\n\
\n\
This tape drive indicates that it needs to be cleaned.\n\
\n\
The following service procedures are recommended.\n\
\n\
CLEAN..........the tape drive read/write head and tape path.\n\
	       For instructions on cleaning, refer to the\n\
	       Setup and Operator Guide for this tape drive.\n\
\n\
RUN............Advanced Diagnostics, in System Verification mode,\n\
	       to ensure this tape drive was cleaned properly."
$
$ %1$l = Menu Goal screen number (not translated).
$ %2$s = Device name.
$ %3$s = Device location.
$ NOTE TO TRANSLATOR: The space at the end of the first line is
$                     required. Do not remove.
$

MNG_NOT_SUPPORTED "%1$06.6lx \
INFORMATION FOR TAPE DRIVE (%2$s) IN LOCATION %3$s\n\
\n\
This tape drive is not currently supported by diagnostics."
$
$ %1$l = Menu Goal screen number (not translated).
$ %2$s = the device name (i.e. rmt0).
$ %3$s = the device location (i.e. 00-00-00).
$ NOTE TO TRANSLATOR: The space at the end of the first line is
$                     required. Do not remove.
$

MNG_UNKNOWN_DEVICE "%1$06.6lx \
ADDITIONAL INFORMATION FOR TAPE DRIVE (%2$s) IN LOCATION %3$s\n\
\n\
Diagnostics does not recognize this tape drive.\n\
\n\
For this reason, the SRN may be invalid."
$
$ %1$l = Menu Goal screen number (not translated).
$ %2$s = the device name (i.e. rmt0).
$ %3$s = the device location (i.e. 00-00-00).
$ NOTE TO TRANSLATOR: The space at the end of the first line is
$                     required. Do not remove.
$

$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$ $$ End of original set, add NEW catalog messages below.

$ $$$$$$$$$$$$$$$$$$$$$$ 3490 messages $$$$$$$$$$$$$$$$$$$$$
$

SUPPORT_3490 "\
The Diagnostic support for this device consists of displaying 32 bytes\n\
of sense data for each error logged against this device in the system \n\
error log.\n"
$
$
$

INTERPRET_3490 "\
In order to interpret the following sense data, refer to the\n\
START section of the 3490E Maintenance Information documentation.\n"

INFO_3490 "\
Each screen displayed will contain information for up to three error\n\
log entries. Use the cursor keys to scroll through the information.\n"
$
$
$

SENSE_3490 "\
ERROR LABEL....TAPE_ERR%1$d\n\
ERROR ID.......%2$X\n\
TIMESTAMP......%3$s\n\
\n\
BYTE NUMBER....00 01 02 03 04 05 06 07  08 09 10 11 12 13 14 15\n\
SENSE DATA.....%4$s\n\
\n\
BYTE NUMBER....16 17 18 19 20 21 22 23  24 25 26 27 28 29 30 31\n\
SENSE DATA.....%5$s\n\
===============================================================\n"
$
$ This message displays 32 bytes of sense data from the error log
$ detail data. The substitutions will come from an error log entry.
$
$ %1$s = Error log entry lable (i.e. TAPE_ERR1).
$ %2$s = Error log ID.
$ %3$s = Error log entry date and time.
$ %4$s = Error log entry detail data (16 bytes of sense data).
$ %5$s = Error log entry detail data (16 bytes of sense data).
$

MORE_3490 "\
To search the system error log for more entries logged against this\n\
device, press Enter.\n"
$
$
$

$ $$$$$$$$$$$$$$$$$$$$$$ 3490 menu goals $$$$$$$$$$$$$$$$$$$
$

MNG_3490_RUN_PD "%1$06.6lx \
ADDITIONAL INFORMATION FOR TAPE DRIVE (%2$s) \
IN LOCATION %3$s\n\
\n\
The Diagnostic support for this device consists of displaying\n\
32 bytes of sense data for each error logged against this device\n\
in the system error log.\n\
\n\
The system error log indicates there are one or more errors\n\
logged against this device. To examine the sense data, test\n\
this device using disk based Diagnostics in Problem Determination\n\
mode.\n\
\n\
For additional service information, refer to the START section\n\
of the 3490E Maintenance Information documentation.\n"
$
$ %1$l = Menu Goal screen number (not translated).
$ %2$s = the device name (i.e. rmt0).
$ %3$s = the device location (i.e. 00-00-00).
$ NOTE TO TRANSLATOR: The space at the end of the first line is
$                     required. Do not remove.
$

MNG_3490_IPL "%1$06.6lx \
ADDITIONAL INFORMATION FOR TAPE DRIVE (%2$s) \
IN LOCATION %3$s\n\
\n\
The Diagnostic support for this device consists of displaying\n\
32 bytes of sense data for each error logged against this device\n\
in the system error log.\n\
\n\
This mode of Diagnostics does not support Error Log Analysis. To\n\
search the system error log for errors logged against this device,\n\
test this device using disk based Diagnostics in Problem Determination\n\
mode.\n\
\n\
For additional service information, refer to the START section\n\
of the 3490E Maintenance Information documentation.\n"
$
$ %1$l = Menu Goal screen number (not translated).
$ %2$s = the device name (i.e. rmt0).
$ %3$s = the device location (i.e. 00-00-00).
$ NOTE TO TRANSLATOR: The space at the end of the first line is
$                     required. Do not remove.
$

$ $$$$$$$$$$$$$$$$$$$$$$ New messages $$$$$$$$$$$$$$$$$$$
$

RETURN_TO_SEL_MENU "\
To return to the Diagnostic Selection menu, press Cancel."

CANCEL_LOOPMODE "\
To stop the test, use 'Cancel.'"
$

MSG_TITLE "TESTING %1$s TAPE DRIVE (%2$s) IN LOCATION %3$s\n\
%4$s\n"
$ %1$s = the device PdDv unique type (i.e. 8mm)
$ %2$s = the device name (i.e. rmt0).
$ %3$s = the device location (i.e. 00-00-00-00).
$ %4$s = ADV_MODE or CUST_MODE message.
$

MSG_TEST_REQS "\
TESTING %1$s TAPE DRIVE (%2$s) IN LOCATION %3$s\n\
%4$s\n\
\n\
The following tests require a diagnostic test tape%5$s\n\
\n\
ANY DATA ON THE DIAGNOSTIC TEST TAPE WILL BE ERASED.\n\
\n\
This test will take approximately %6$s minutes to complete.\n\
\n\
Do you have a diagnostic test tape?\n"
$
$ %1$s = the device PdDv name (i.e. 8mm).
$ %2$s = the device logical name (i.e. rmt0).
$ %3$s = the device location (i.e. 00-00-00).
$ %4$s = ADV_MODE or CUST_MODE message.
$ %5$s = one of the TXXXX_PN messages.
$ %6$s = Test Time (not translated).
$

MSG_SYSX_REQS "\
TESTING %1$s TAPE DRIVE (%2$s) IN LOCATION %3$s\n\
%4$s\n\
\n\
You may decide to run the System Exerciser with media\n\
installed in a device or with no media installed in a\n\
device. Running the test with media installed in a device\n\
is a more thorough test.\n\
\n\
If you chose to run the System Exerciser with media\n\
installed, the test will require a diagnostic test tape%5$s\n\
\n\
This test will take approximately %6$s minutes to complete."
$
$ %1$s = the device PdDv name (i.e. 8mm).
$ %2$s = the device logical name (i.e. rmt0).
$ %3$s = the device location (i.e. 00-00-00).
$ %4$s = ADV_MODE or CUST_MODE message.
$ %5$s = one of the TXXXX_PN messages.
$ %6$s = Test Time (not translated).
$

MNG_SCSI_BUS_RESET "%1$06.6lx \
ADDITIONAL INFORMATION FOR TAPE DRIVE (%2$s) \
IN LOCATION %3$s\n\
\n\
A recoverable error occurred while testing this device. A SCSI Bus\n\
reset may cause this type of error. A SCSI Bus reset may be caused\n\
by one or more of the following:\n\
\n\
  - SCSI Cables and Terminators\n\
      A loose, deffective or incompatable SCSI Cable or Terminator\n\
      can cause this type of error.\n\
\n\
  - SCSI Devices\n\
      An intermittent problem with any device on this SCSI Bus\n\
      can also cause a SCSI Bus reset to occur.\n\
\n\
If this type of error continues to occur, contact your support\n\
personel for more information."
$
$ %1$s = the device logical name (i.e. rmt0).
$ %2$s = the device location (i.e. 00-00-00).
$

ENSURE_TAPE_WP "\
ENSURE.........the diagnostic test tape is write protected.\n\
	       %1$s.\n"
$
$ %1$s = one of the TXXX_WEWP messages.
$

ENSURE_TAPE_WE "\
ENSURE.........the diagnostic test tape is write enabled.\n\
	       %1$s.\n"
$
$ %1$s = one of the TXXX_WEWP messages.
$

CHECK_TAPE_WE "\
This tape drive is reporting that the diagnostic test tape is\n\
write protected.\n\
\n\
This test requires the diagnostic test tape to be write enabled."
$

CHECK_TAPE_WP "\
This tape drive is reporting that the diagnostic test tape is\n\
write enabled.\n\
\n\
This test requires the diagnostic test tape to be write protected."
$

$ $$$$$$$$$$$$$$$$$$$$$$ New messages for bos325 $$$$$$$$$$$$$$$$$$$
$

T4326_PN "\ncartridge (Part Number 8191146)."
$ Test Tape Part Numbers.
$   NOTE: Messages may start with a new line (\n) character
$         and will end with a period.

SUPPORT_3490_N "\
The Diagnostic support for this device consists of displaying\n\
sense data for each error logged against this device in the\n\
system error log.\n"
$
$
$

SENSE_3490_N "\
ERROR LABEL....TAPE_ERR%1$d\n\
ERROR ID.......%2$X\n\
TIMESTAMP......%3$s\n\
\n\
BYTE NUMBER..........00 01 02 03 04 05 06 07  08 09 10 11 12 13 14 15\n\
3490E SENSE DATA.....%4$s\n\
\n\
BYTE NUMBER..........16 17 18 19 20 21 22 23  24 25 26 27 28 29 30 31\n\
3490E SENSE DATA.....%5$s\n\
\n\
BYTE NUMBER..........00 01 02 03 04 05 06 07  08 09 10 11 12 13 14 15\n\
SCSI SENSE DATA......%6$s\n\
\n\
BYTE NUMBER..........16 17 18 19 20 21\n\
SCSI SENSE DATA......%7$s\n\
===============================================================\n"
$
$ This message displays 32 bytes of sense data from the error log
$ detail data. The substitutions will come from an error log entry.
$
$ %1$s = Error log entry lable (i.e. TAPE_ERR1).
$ %2$s = Error log ID.
$ %3$s = Error log entry date and time.
$ %4$s = Error log entry detail data (16 bytes of sense data).
$ %5$s = Error log entry detail data (16 bytes of sense data).
$ %6$s = Error log entry detail data (16 bytes of sense data).
$ %7$s = Error log entry detail data ( 6 bytes of sense data).
$

MNG_3490_RUN_PD_N "%1$06.6lx \
ADDITIONAL INFORMATION FOR TAPE DRIVE (%2$s) \
IN LOCATION %3$s\n\
\n\
The Diagnostic support for this device consists of displaying\n\
sense data for each error logged against this device in the\n\
system error log.\n\
\n\
The system error log indicates there are one or more errors\n\
logged against this device. To examine the sense data, test\n\
this device using disk based Diagnostics in Problem Determination\n\
mode.\n\
\n\
For additional service information, refer to the START section\n\
of the 3490E Maintenance Information documentation.\n"
$
$ %1$l = Menu Goal screen number (not translated).
$ %2$s = the device name (i.e. rmt0).
$ %3$s = the device location (i.e. 00-00-00).
$ NOTE TO TRANSLATOR: The space at the end of the first line is
$                     required. Do not remove.
$

MNG_3490_IPL_N "%1$06.6lx \
ADDITIONAL INFORMATION FOR TAPE DRIVE (%2$s) \
IN LOCATION %3$s\n\
\n\
The Diagnostic support for this device consists of displaying\n\
sense data for each error logged against this device in the\n\
system error log.\n\
\n\
This mode of Diagnostics does not support Error Log Analysis. To\n\
search the system error log for errors logged against this device,\n\
test this device using disk based Diagnostics in Problem Determination\n\
mode.\n\
\n\
For additional service information, refer to the START section\n\
of the 3490E Maintenance Information documentation.\n"
$
$ %1$l = Menu Goal screen number (not translated).
$ %2$s = the device name (i.e. rmt0).
$ %3$s = the device location (i.e. 00-00-00).
$ NOTE TO TRANSLATOR: The space at the end of the first line is
$                     required. Do not remove.
$

DIAG_BOOT_COMPLETE "\
INSERT.........the tape containing the diagnostic programs\n\
               back into the tape drive. Close the drive door,\n\
               if applicable."
$

DID_YOU_CLEAN_DRIVE "\
The tape drive is reporting a media error. Has this tape drive\n\
been cleaned recently (within the last 24 hours)?\n\
\n\
Move cursor to selection, then press Enter.\n"
$

DID_YOU_TRY_NEW_TTAPE "\
The tape drive is reporting a media error. Have you replaced the\n\
test tape recently (within the last 24 hours)?\n\
\n\
Move cursor to selection, then press Enter.\n"
$

CLEAN_DRIVE_NOW "\
CLEAN..........the tape drive at this time. For instructions on\n\
               cleaning, refer to the Setup and Operator Guide\n\
               for this tape drive.\n\
\n\
AFTER..........the tape drive has been cleaned,\n\
               press Enter to continue.\n\
\n\
Note: This test will be run again after the tape drive has been cleaned.\n\
      If you do not wish to run this test again, press Cancel or Exit."
$

GET_NEW_TTAPE_NOW "\
OBTAIN.........a new test tape at this time.\n\
\n\
AFTER..........a new test tape has been obtained,\n\
               press Enter to continue.\n\
\n\
Note: This test will be run again using the new test tape.\n\
      If you do not wish to run this test again, press Cancel or Exit."
$
