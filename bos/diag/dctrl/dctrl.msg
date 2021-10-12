$ @(#)08	1.20.4.3  src/bos/diag/dctrl/dctrl.msg, dctrl, bos41J, 9514A_all 3/29/95 17:38:45
$
$   COMPONENT_NAME: (CMDDIAG) Diagnostic Controller
$
$   FUNCTIONS: Diagnostic message file.
$
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989, 1995
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
$ Additional notes:
$
$       This file contains the corresponding messages for the menus and
$       errors generated from the Diagnostic Controller.
$
$ END OF GENERAL NOTES

$quote "
$set DISKETTE3
$
$ This message cannot exceed 56 columns in width

MSG_DISK_1     "Bad diskette. Insert another diskette and\n\
press Enter to continue."
MSG_DISK_2     "Processing Supplemental Diagnostic Diskette.\n\n\
Please stand by."
MSG_DISK_3     "Incorrect diskette. Insert another diskette\n\n\
and press Enter to continue."
MSG_DISK_4     "Insert the Supplemental Diagnostic Diskette.\n"
MSG_DISK_5     "Press Enter to continue."
MSG_DISK_6     "Restoring file(s) from tape device %s.\n\n\
Please stand by."
MSG_DISK_7	"\
An error was received during the restore operation.\n\n\
The following service procedures are recommended.\n\
CLEAN..........the tape drive read/write head and tape path.  For\n\
	       instructions on cleaning, refer to the Setup and \n\
	       Operator Guide for this tape drive.\n\
\n\
OBTAIN.........a new diagnostic tape.\n\
	       NOTE: An old or worn out diagnostic tape\n\
	             may cause media errors.\n\
\n\
RUN............Advanced Diagnostics, to ensure this drive is \n\
	       functioning properly.\n\n\
Press Enter to continue."

$set SET_DMODE
$

MSG_DMODE_0     "DIAGNOSTIC MODE SELECTION"
$
$ See general notes on length of title line.

MSG_DMODE_1     "Problem Determination\n\
  This selection tests the system and analyzes the error log\n\
  if one is available.  Use this option when a problem is\n\
  suspected on the machine."
$
$ displayed for the DIAGNOSTIC MODE SELECTION MENU
$ Leave two blank spaces at the beginning of every line after the first.

MSG_DMODE_2     "System Verification\n\
  This selection will not analyze the error log.  Use this option\n\
  to verify that the machine is functioning correctly after completing\n\
  a repair or an upgrade."
$
$ displayed for the DIAGNOSTIC MODE SELECTION MENU
$ Leave two blank spaces at the beginning of every line after the first.

MSG_DMODE_E     "Move cursor to selection, then press Enter."
$
$ displayed for the DIAGNOSTIC MODE SELECTION MENU in standalone package.
$

MSG_DMODE_S     "System Verification\n\
  Use this option to verify that the machine is functioning correctly\n\
  after completing a repair or an upgrade."
$
$ displayed for the DIAGNOSTIC MODE SELECTION MENU for standalone package
$
MSG_DMODE_PD    "Problem Determination\n\
  This selection tests the system.  Use this option when a\n\
  problem is suspected on the machine.\n\n\
  NOTE: Error log analysis is only performed when running from the\n\
        hard file."
$
$ displayed for the DIAGNOSTIC MODE SELECTION MENU for standalone package
$

$set SET_TMETHOD
$

MSG_TMETHOD_0   "TEST METHOD SELECTION"
$
$ See general notes on length of title line.

MSG_TMETHOD_1   "Run Test Once"
$
$ displayed for the TEST METHOD SELECTION MENU

MSG_TMETHOD_2   "Run Test Multiple Times\n\
  This selection should be used when a problem occurs intermittently.\n\
  This selection will continue testing until 'Cancel' is pressed.\n\
  Warning: This selection is normally used by the service representative.\n\
           Inconsistent use may result in false problems being reported.\n\
  NOTE:  After 'Cancel' is pressed, it may take some time before\n\
         the testing stops.  The testing goes through a final phase\n\
         to return the resources to their original state."
$
$ displayed for the TEST METHOD SELECTION MENU
$ Leave two blank spaces at the beginning of every line after the first.

MSG_TMETHOD_E   "Move cursor to selection, then press Enter."
$

$set SET_DSMENU
$

MSG_DSMENU_A    "ADVANCED DIAGNOSTIC SELECTION\n\n\
An * in front of the resource shows that the test has been selected."
$
$ displayed for the DIAGNOSTIC SELECTION MENU
$ Leave line spacing as shown.  See general notes on length of title.

MSG_DSMENU_C    "DIAGNOSTIC SELECTION\n\n\
An * in front of the resource shows that the test has been selected."
$
$ displayed for the DIAGNOSTIC SELECTION MENU
$ Leave line spacing as shown.  See general notes on length of title.

MSG_DSMENU_1    "System Checkout\n\
  System Checkout tests all functions that do not require manual\n\
  intervention.  The other options test only the specified resource."
$
$ displayed for the DIAGNOSTIC SELECTION MENU
$ Leave two blank spaces at the beginning of every line after the first.

MSG_DSMENU_2    "\
Base System      00-00            CPU, fpa, memory, I/O planar, op panel"
$
$ displayed for the DIAGNOSTIC SELECTION MENU
$ Leave this message aligned as shown  The first item, Base System, should
$ start in column 1.  The second item, 00-00, should start in column 18.
$ The third item, CPU, fpa, ..., should start in column 35.
$ If the translation for the third portion of the this line is greater
$ than 40 characters, be sure and start the second line in column 35 to
$ line up with the previous line.

MSG_DSMENU_E    "Choose the test that you want to run."
$ displayed when reading a supplemental diagnostic diskette
$

MSG_DSMENU_4    "Read supplemental diskette"

MSG_DSMENU_ISA  "\
Base System      00-00            CPU, fpa, memory"
$
$ displayed for the DIAGNOSTIC SELECTION MENU
$ Leave this message aligned as shown  The first item, Base System, should
$ start in column 1.  The second item, 00-00, should start in column 18.
$ The third item, CPU, fpa, ..., should start in column 35.
$ If the translation for the third portion of the this line is greater
$ than 40 characters, be sure and start the second line in column 35 to
$ line up with the previous line.

$set SET_MISSING
$

MSG_MISSING     "MISSING RESOURCE"
$
$ See general notes on length of title.

MSG_MISSING_1  "\
The resource has NOT been removed from the system, moved to\n\
  another location or address, or turned off.\n\
  This selection will determine why the resource was not detected."
$
$ displayed for the MISSING RESOURCE MENU
$ Leave two blank spaces at the beginning of every line after the first.

MSG_MISSING_2  "\
The resource has been removed from the system and should be\n\
  removed from the system configuration."
$
$ displayed for the MISSING RESOURCE MENU
$ Leave two blank spaces at the beginning of every line after the first.

MSG_MISSING_3  "\
The resource has been moved to another location and should be\n\
  removed from the system configuration."
$
$ displayed for the MISSING RESOURCE MENU
$ Leave two blank spaces at the beginning of every line after the first.

MSG_MISSING_4  "\
The resource has been turned off and should be removed from\n\
  the system configuration."
$
$ displayed for the MISSING RESOURCE MENU
$ Leave two blank spaces at the beginning of every line after the first.

MSG_MISSING_5  "\
The resource has been turned off but should remain in the\n\
  system configuration."
$
$ displayed for the MISSING RESOURCE MENU
$ Leave two blank spaces at the beginning of every line after the first.

MSG_MISSING_E   "\
The following resource was detected previously, but is not detected now:\n\n\
%s\n\n\
Has the resource been removed from the system, moved to another location\n\
or address, or turned off?"
$
$ displayed for the MISSING RESOURCE MENU
$ %s is the resource that is missing.
$ Leave line spacing as shown.

$set SET_COMP
$

MSG_NOMOREEND   "TESTING COMPLETE on %s\n\n\
No trouble was found.\n\n\
The resources tested were:"
$
$ displayed for the TESTING COMPLETE MENU
$ Leave line spacing as shown.  See general notes on length of title.

MSG_MOREEND     "TESTING COMPLETE on %s\n\n\
No trouble was found.\n\n\
Additional testing can be done if additional resources are freed.\n\n\
The resources tested were:"
$
$ In order to test additional resources, they must be freed, or made available
$ to the system.  All programs using the resource must be stopped and the
$ resource can then be used by diagnostics.
$ displayed for the TESTING COMPLETE MENU
$ Leave line spacing as shown.  See general notes on length of title.

MSG_COMPEND      "To continue, press Enter."

MSG_COMPMLC      "\
REMINDER: The Product Topology Service Aid should be performed\n\
whenever you have completed one of the following:\n\
         1. Device Installation (EC's, MES's, or RPQ)\n\
         2. Repair Action\n\
         3. System Installation\n\n\
To continue, press Enter."

$set SET_ALLNEW
$

MSG_ALLNEW      "NEW RESOURCE"
$
$ Leave line spacing as shown.  See general notes on length of title.

MSG_ALLNEW_0  "%s"
$
$ Leave this substitution as is.  It is used to list all new resources.

MSG_ALLNEW_1STANDALONE  ""

$ The above message is no longer used. It is replaced by MSG_ALLNEW_NOISA
$ Leave line spacing as shown.

MSG_ALLNEW_1HARDFILE  ""

$ The above message is no longer used. It is replaced by MSG_NEWRESOURCES_OPT1
$ Leave line spacing as shown.

MSG_ALLNEW_2  ""

$ The above message is no longer used. It is replaced by MSG_NEWRESOURCES_OPT2
$ Leave line spacing as shown.

MSG_ALLNEW_STANDALONE    ""

$ The above message is no longer used. It is replaced by MSG_ALLNEW_NOISA
$ Leave line spacing as shown.

MSG_ALLNEW_HARDFILE    ""

$ The above message is no longer used. It is replaced by MSG_NEWRESOURCES_OPT2
$ Leave line spacing as shown.


MSG_ALLNEW_ISA  "\
The following new resource(s) were detected.\n\
Some resources may require software installation or supplemental media\n\
processing to appear on the list.\n\
ISA resources require configuration before they will appear on the list.\n\n\
Select an option from the bottom of the list, then press Enter."

$ displayed for the NEW RESOURCE MENU, when booting up on system with
$ ISA Bus.
$ Leave line spacing as shown.

MSG_ALLNEW_NOISA  "\
The following new resource(s) were detected.\n\
Some resources may require software installation or supplemental media\n\
processing to appear on the list.\n\n\
Select an option from the bottom of the list, then press Enter."

$ displayed for the NEW RESOURCE MENU.
$ Leave line spacing as shown.

MSG_NEWRESOURCES_OPT1 "\
1. Continue. The list contains all resources that should appear."

MSG_NEWRESOURCES_OPT2 "\
2. A resource that should appear on the list is missing."

MSG_NEWRESOURCES_OPT3  "\
3. Invoke the ISA Device Configuration Service Aid."

$
$ displayed for the NEW RESOURCE MENU
$ Leave line spacing as shown.

$
$set SET_SIMNEW
$

MSG_SIMNEW      "NEW RESOURCES"
$
$ See general notes on length of title line.

MSG_SIMNEW_1    "Device is not listed."
MSG_SIMNEW_E    "\
The following new resource or resources are similar in type to\n\
the missing resource.  Please select the entry where the missing\n\
resource was moved.\n\n\
Move cursor to selection, then press Enter."
$
$ The above panel is displayed for the NEW RESOURCES MENU
$ Leave line spacing as shown.

$set SET_ADDPR
$

MSG_ADDPR_T     "ADDITIONAL RESOURCES ARE REQUIRED FOR TESTING\n\n\
A problem has been detected that can not be isolated without\n\
freeing additional resources.\n\n\
The resource needed is\n%s\n\
To test this resource, you can:\n\
  Free this resource and continue testing.\n\
  Shut down the system and run in maintenance mode.\n\
  Run Diagnostics from the Diagnostic Standalone package."
$
$ displayed for the ADDITIONAL RESOURCES NEEDED - PR
$ %s is a substitution for the name of the resource.
$ In order to test additional resources, they must be freed, or made available
$ to the system.  All programs using the resource must be stopped and the
$ resource can then be used by diagnostics.
$ vary the resource off is a UNIX term meaning to remove all programs that
$ are using the resource so that the resource can be tested fully.
$ Leave line spacing as shown.

MSG_ADDPR_2     "Testing should stop."
MSG_ADDPR_3     "The resource is now free and testing can continue."
$
$ In order to test additional resources, they must be freed, or made available
$ to the system.  All programs using the resource must be stopped and the
$ resource can then be used by diagnostics.
$ vary the resource off is a UNIX term meaning to remove all programs that
$ are using the resource so that the resource can be tested fully.

MSG_ADDPR_E "Move cursor to selection, then press Enter."

$set DIAG_PROB_SETID
$

DIAG_PROB_TITLE "A PROBLEM WAS DETECTED ON "
$
$ displayed for the PROBLEM REPORT MENU

DIAG_PROB_SRN "The Service Request Number or Numbers:"
DIAG_PROB_CUS "\nProbable Cause or Causes:"
DIAG_PROB_ADD "\n\
Additional isolation may be obtained by running more tests.\n\n\
To continue, press Enter."
$
$ displayed for the PROBLEM REPORT MENU

DIAG_PROB_RET "To continue, press Enter."
DIAG_PROB_SRN_ERRCODE "The Service Request Number(s)/Error Code(s)."
$set SET_ADDDD
$

MSG_ADDDD_T     "ADDITIONAL RESOURCES ARE REQUIRED FOR TESTING\n\n\
No trouble was found.  However, the resource was not tested because\n\
the device driver indicated that the resource was in use.\n\n\
The resource needed is\n%s\n\
To test this resource, you can:\n\
  Free this resource and continue testing.\n\
  Shut down the system and run in maintenance mode.\n\
  Run Diagnostics from the Diagnostic Standalone package."
$
$ The above panel is displayed for the ADDITIONAL RESOURCES NEEDED -
$                                       DEVICE DRIVER ERROR MENU
$ %s is the resource that should be freed for further testing.
$ vary the resource off is a UNIX term meaning to remove all programs that
$ are using the resource so that the resource can be tested fully.
$ In order to test additional resources, they must be freed, or made available
$ to the system.  All programs using the resource must be stopped and the
$ resource can then be used by diagnostics.
$ Leave line spacing as shown.  See general notes on length of title.

$set SET_ADDNTF
$

MSG_ADDNTF_T    "ADDITIONAL RESOURCES ARE REQUIRED FOR TESTING\n\n\
No trouble was found.  Additional testing can be done if additional\n\
resources are freed.\n\n\
The resource needed is\n%s\n\
To test this resource, you can:\n\
  Free this resource and continue testing.\n\
  Shut down the system and run in maintenance mode.\n\
  Run Diagnostics from the Diagnostic Standalone package."
$
$ The above panel is displayed for the ADDITIONAL RESOURCES NEEDED - NTF
$ %s is the resource that should be freed for further testing.
$ vary the resource off is a UNIX term meaning to remove all programs that
$ are using the resource so that the resource can be tested fully.
$ In order to test additional resources, they must be freed, or made available
$ to the system.  All programs using the resource must be stopped and the
$ resource can then be used by diagnostics.
$ Leave line spacing as shown.  See general notes on length of title.

$set SET_SYSCHECKOUT
$

MSG_SYSCK_T "SYSTEM CHECKOUT"
$
$ displayed for the SYSTEM CHECKOUT - diskette

MSG_SYSCK_1 "\
The diagnostics necessary to test the system are located on more than\n\
one diskette.  Select the diskette containing the devices that you want\n\
to test.\n\n\
Move cursor to selection, then press Enter."
$
$ displayed for the SYSTEM CHECKOUT - diskette

MSG_SYSCK_D "Diskette: Diagnostics"
$

$set SET_MISSREP
$

MSG_MISSREP_T   "MISSING RESOURCE"
$
$ displayed for the MISSING RESOURCE MENU

MSG_MISSREP_1  "None of the devices listed should be reconfigured."
$
$ displayed for the MISSING RESOURCE MENU

MSG_MISSREP_2  "All of the devices listed should be reconfigured."
$
$ displayed for the MISSING RESOURCE MENU

MSG_MISSREP_E   "Select the device or devices to be reconfigured.\n\n\
Move cursor to selection, then press Enter."
$
$ displayed for the MISSING RESOURCE MENU
$ Leave line spacing as shown.

$set SET_EMODE
$

MSG_ELAPD_0     "RESOURCE NOT SUPPORTED IN CONCURRENT MODE\n\n\
The resource %s is not supported by diagnostic tests in concurrent\n\
mode.  You may perform error log analysis on the resource or exit.\n\n\
NOTE: If you have replaced this resource, do not perform error log\n\
analysis."
$
$ displayed for the ADDITIONAL RESOURCES NEEDED - PR
$ %s is a substitution for the name of the resource.
$ Leave line spacing as shown.

MSG_ELAPD_1     "Perform error log analysis."
MSG_ELAPD_2     "Exit to Diagnostic Selection Menu."
$
MSG_ELAPD_E "Move cursor to selection, then press Enter."

$set INFOSET
$

TMSGS "Starting diagnostics\n"
TMSGT "Testing"
TMSGP "A problem was detected.  Run diagnostics.\n"
TMSGE "Ending diagnostics.\n"
TMSGU "Must specify a device to test.\n\
Usage:  diag -c -d resource-name\n"

$set ESET
$ various error messages
$

ERROR1 "The device pointer is not valid."
ERROR2 "Cannot find device."
ERROR3 "Error obtaining data for display."
ERROR4 "Abnormal stop of Diagnostic Application\n%s"
ERROR5 "Cannot start process %1$s\n%2$s"
ERROR6 "Object class %1$s error\n%2$s"
ERROR7 "The parent or child relationship in CuDv is not valid."
$ DO NOT TRANSLATE CuDv.  parent and child are technical terms used to
$ describe the relationship between two processes.

ERROR8 "\nError initializing Diagnostic Database.\n"
ERROR9 "No new resource was found similar in type to the missing resource\n%s"
ERROR10 "Internal error occured in %s.  Cannot initialize ODM."
ERROR11 "\
INFORMATION MESSAGE\n\n\
Move the cursor to the bottom of the list\n\
of resources and select option 1 or 2.\n\n\
To continue, press Enter."
ERROR12 "\nDIAG requires a minimum of 24 line by 80 column output display.\n\
Check terminal or window size and try again.\n\n"
ERROR13 "The current environment does not allow testing of the"
ERROR14 "The current environment does not allow testing of the %s device."
ERROR15 "The diagnostic lock file (/etc/lpp/diagnostics/data/dctrl.lck)\n\
indicates that another diagnostic session (pid %s) is currently active.\n\
If this pid is no longer active, the lock file can be deleted.\n\
Then restart diagnostics to continue.\n"
$ DO NOT TRANSLATE pid. Pid stands for Process id and is a technical term
$ used to describe the unique id number associated with each process
$ running on the system.

ERROR16 "Unable to create lock file:\n\
%s\n"
ERROR17 "Device does not have diagnostics support."
ERROR18 "Diagnostics is not supported on this system.\n"
