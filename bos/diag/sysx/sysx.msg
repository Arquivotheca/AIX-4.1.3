$ @(#)20	1.5.1.3  src/bos/diag/sysx/sysx.msg, dsysx, bos411, 9428A410j 5/5/94 16:53:26
$
$   COMPONENT_NAME: DSYSX
$
$   FUNCTIONS: Diagnostic message file.
$		
$
$   ORIGINS: 27, 83
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$   LEVEL 1, 5 Years Bull Confidential Information
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
$ Additional notes:
$
$       This file contains the corresponding messages for the menus and
$	errors generated from the Diagnostic Controller.
$
$ END OF GENERAL NOTES

$quote "
$set SET_INVALID
$

MSG_0	"DIAGNOSTIC SYSTEM EXERCISER"
$
$ See general notes on length of title line.

MSG_1	"The Diagnostic System Exerciser is only supported from\n\
disk drive in standalone or maintenance mode."
$
$ displayed if exerciser is run in concurrent mode

MSG_E	"To continue, press Enter."
$


$set SET_SYSXL
$

MSG_S1	"DIAGNOSTIC SYSTEM EXERCISER\n\n\
The following resources will be tested."
$
$ displayed for the DIAGNOSTIC SYSTEM EXERCISER 
$ Leave line spacing as shown.  See general notes on length of title.

MSG_S2	"To start the exerciser, press Enter."
$

MSG_S3	"DIAGNOSTIC SYSTEM EXERCISER %s\n\n\
One or more diagnostics are currently running.  Cancelling the System\n\
Exerciser at this time will stop any diagnostics currently running.\n\n\
The System Exerciser cannot be cancelled if the Keyboard diagnostic is\n\
running at the time you press the 'Cancel' key.  Wait a few seconds after\n\
you hear the keyboard speaker test before pressing the 'Cancel' key.\n\n\
Depending on the test list, the exerciser may run for several minutes\n\
or longer before the results screen is displayed.\n\n\
Please stand by."
$

MSG_S4	"DIAGNOSTIC SYSTEM EXERCISER\n\n\
Cancel in process. Please stand by."
$

MSG_S5 "DIAGNOSTIC SYSTEM EXERCISER %s\n\n\
Diagnostics are currently running\n\
If needed, scroll through results using KEY_UP & KEY_DOWN\n\n\
Device\t\tLoop_Count\tError_count"
$

MSG_S6 "Press Cancel or Exit To Stop Diagnostics"
$

$set SET_RESULTS
$

MSG_RES_0	"DIAGNOSTIC SYSTEM EXERCISER RESULTS\n\n\
No trouble found. Press Enter to restart the exerciser.\n\n\
List of tested resources:"

$
$ displayed for the DIAGNOSTIC SYSTEM EXERCISER 
$ Leave line spacing as shown.  See general notes on length of title.

MSG_RES_1	"DIAGNOSTIC SYSTEM EXERCISER RESULTS\n\n\
The following resource or resources have been tested and errors have\n\
been detected. Press Enter to restart the exerciser." 
$
$ displayed for the DIAGNOSTIC SYSTEM EXERCISER 
$ Leave line spacing as shown.  See general notes on length of title.

MSG_RES_2	"DIAGNOSTIC SYSTEM EXERCISER RESULTS\n\n\
An * in front of the resource indicates that the resource had an error."
$
$ displayed for the DIAGNOSTIC SYSTEM EXERCISER 
$ Leave line spacing as shown.  See general notes on length of title.

MSG_RES_3	"Choose the resource you would like to see results for."
$

MSG_RES_E	"To stop the exerciser, use 'Cancel'."
$

$set DIAG_PROB_SETID
$

DIAG_PROB_TITLE "A PROBLEM WAS DETECTED ON "
$
$ displayed for the PROBLEM REPORT MENU

DIAG_PROB_SRN "The Service Request Number or Numbers:"
DIAG_PROB_CUS "\nProbable Cause or Causes:"
DIAG_PROB_ERR "Number of passes = %d, Number of errors = %d\n"

$set ESET
$ various messages
$
INFO1  "There were no errors detected for this device."
ERROR6 "Object class %1$s error\n%2$s"

$set EXIT_SYSX
EXIT_TITLE    "DIAGNOSTIC SYSTEM EXERCISER MEDIA CHECK"
$
$ See general notes on length of title line.
EXIT_INSTRUCT "\
Remove all media (diskettes, tapes, cd-roms, etc)\n\
that were inserted for the System Exerciser Test."

EXIT_LAST     "When finished, press Enter."

$set SET_TSMENU
$

MSG_TSMENU_A	"DIAGNOSTIC SYSTEM EXERCISER\n\n\
An * in front of the resource shows the test has been selected."
$
$ displayed for the SIMULTANEOUS DIAGNOSTIC TEST SELECTION MENU
$ Leave line spacing as shown.  See general notes on length of title.

MSG_TSMENU_B	"Press Enter to Choose the device(s) that you want to \
Select \ or Unselect.\n\n\
Press Commit to start the system exerciser."

TSNORMAL	"Mode Selection		Exercise Device without test tools.\n\
     Press Help for Explanations.\n"

TSADVANCED	"Mode Selection		Exercise Device with test tools.\n\
     Press Help for Explanations.\n"

TSALLDEV	"All Devices"

$set HELP_SET

HELP_SELECT	"\
		MODE SELECTION\n\
Each Selection of this item allows the user to\n\
toggle test mode between:\n\
Test without test tools\n\
Test with test tools (wrap plugs, media).\n\n\
		DEVICE SELECTION\n\
Press Enter in front of one item to select or\n\
unselect the corresponding device\n\
When selected, an * will be displayed in front\n\
of the selected device.\n\n\
		All Devices\n\
When selected, an * will be displayed in front\n\
of the item, and any device selection will be\n\
disregarded: all tests will be run if Commit is\n\
pressed.\n\
		START TEST\n\
Press Commit to start test of selected device(s)\n"

$set SET_TMETHOD
$

MSG_TMETHOD_0	"TEST METHOD SELECTION"
$
$ See general notes on length of title line.

MSG_TMETHOD_1	"Run Test One Time"
$
$ displayed for the TEST METHOD SELECTION MENU

MSG_TMETHOD_2	"Run Test Multiple Times\n\
  This selection should be used when a problem occurs intermittently.\n\
  This will test the resource or resources until 'Cancel' is pressed.\n\
  NOTE:  After 'Cancel' is pressed, it may take some time before\n\
         the testing stops.  The test will go through a final phase\n\
         to return the resource or resources to their original state."
$
$ displayed for the TEST METHOD SELECTION MENU
$ Leave two blank spaces at the beginning of every line after the first.

MSG_TMETHOD_E	"Move cursor to selection, then press Enter."
$

