$ @(#)52        1.9  src/bos/diag/util/u7135/u7135.msg, dsau7135, bos41J, 9520B_all 5/18/95 09:47:42
$
$   COMPONENT_NAME: DSAU7135
$
$   FUNCTIONS:
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1993,1995
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
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
$		 begin the next line at the same point of the previous line.
$ ACTION.........the next action follows with no blank line preceding it.
$ *****
$
$ END OF GENERAL NOTES

$quote "
$  $$$$$$$$$$$$$$$$$$$$$$  MESSAGE SET	$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$set U7135_MSG_SET
$

TITLE "7135 RAIDiant ARRAY SERVICE AIDS"
$ 1

ADVANCED_MODE " "
$ 2

MOVE_CURSOR "Move cursor to selection, then press Enter.\n"
$ 3

WF_PRESS_ENTER "When finished, press Enter."
$ 4

TC_PRESS_ENTER "To continue, press Enter."
$ 5

YES "YES"
$ 6

NO "NO"
$ 7

STAND_BY "This operation may take up to %1$d minute(s) to complete.\n\
\n\
Please stand by."
$ %1$d is the time the test may take in minutes.
$ 8

BLOCK_SIZE_NOT_512 "\
The block size of this physical disk is not compatible with the\n\
operating system.\n\
\n\
Use the 7135 RAIDiant Array Format Service Aid to format the\n\
physical disk in location %1$s."
$ 9

USE_UP_DOWN_KEYS "\
It may be necessary to use the cursor up/down keys to view\n\
all of the information. When finished, press Enter."
$ 10

DO_YOU_WISH "Do you wish to run this test?\n"
$ 11

FORMAT_IN_PROGRESS "\
The physical disk in location %1$s is reporting format in progress.\n\
\n\
The physical disk will not be usable until the format operation\n\
has completed.\n\
\n\
To return to the Service Aid selection menu, press Enter."
$ 12

PSCAN  "Parity Scan"
$ 13

PSCAN_MISMATCH "Parity Mismatch"
$ 14

MB "MB"
$ MB stands for Mega-Byte
$ 15

GB "GB"
$ GB stands for Giga-Byte
$ 16

RAID "RAID"
$ 17

CNTRL_STATUS_PASSIVE	"Passive           "
CNTRL_STATUS_ACTIVE	"Active            "
CNTRL_STATUS_FAILED	"Passive Failed    "
$ Controller status messages (18 chars max).

PSCAN_ERROR "Parity Error"
$ 21

OPTIMAL_DRIVE	       "Optimal Drive     "
NON_EXISTENT	       "Non-Existant Drive"
NON_SUPPORT_ID	       "Non-Supported ID  "
SPARE_DRIVE	       "Spare Drive       "
FAILED_DRIVE	       "Failed Drive      "
TUR_FAILURE_S	       "TUR Failure       "
WRITE_FAILURE	       "Write Failure     "
S_OF_DAY_FAIL	       "S.O.D. Failure    "
NO_ACTION	       "No Action         "
RECONSTR_INIT	       "Reconstructing    "
DRIVE_WARNING	       "Drive in Warning  "
PARAM_MISMATCH	       "Parameter Mismatch"
WRONG_CAPACITY	       "Wrong Capacity    "
WRONG_CNTRL_SN	       "Wrong Cntrl. SN   "
ID_MISMATCH	       "ID Mismatch       "
THIS_CNTRL	       "This Cntrl. OK    "
DR_FORMAT_INIT	       "Drive Formatting  "
WRONG_DR_REPL	       "Wrong Dr. Replaced"
NON_SUPPORT_CH	       "Non-Supported CH  "
NON_SUPP_CH_ID	       "Non-Supported ID  "
HOT_SPARE_DRIV	       "Hot Spare Drive   "
COMP_FAILURE	       "Component Failure "
FORMAT_FAILURE	       "Format Failure    "
USER_FAILED_MS	       "Failed by User    "
FORMAT_INIT	       "Formatting        "
WRONG_SECTOR_S	       "Wrong Sector Size "
INC_MODE_PARAM	       "Invalid Mode Param"
CH_MISMATCH	       "Channel Mismatch  "
DR_PEND_FORMAT	       "Pending Format    "
$ ----- array physical status msg's above, array logical status msg's below.
OPTIMAL_LUN	       "Optimal LUN       "
R0_DR_FORMAT	       "RAID 0 Format     "
AWAIT_FORMAT	       "Awaiting Format   "
LUN_IS_BUSY	       "LUN is Busy       "
UNKNOWN 	       "Unknown Status    "
DR_FAILURE	       "Drive Failure     "
REPL_DR_FORMAT	       "Formatting        "
MULT_DR_FAILED	       "Multiple Failures "
FORMAT_IN_PROG	       "Format in Progress"
REPL_WRONG_DR	       "Replaced Wrong Dr."
OPTIMAL_SUB_LUN        "Optimal LUN       "
$
$ The above messages are the status messages used in the array status.
$ NOTE: The status field is 18 chars maximum (see the ARRAY_STATUS_ITEM msg).
$

DR_2GB		"2.0GB"
DR_1_3GB	"1.3GB"

FAILED 		"Failed!"
$ 64
FILE_NOT_FOUND 	"File not found!"
$ 65

EXIT_FORMAT "\
The format operation is still in progress. If you exit this Service\n\
Aid, the disk will continue to format and will not be usable until\n\
the format operation is complete.\n\
\n\
Do you wish to exit the Format Service Aid?"
$ 66

ACCESS_ARRAY_STATUS "Accessing the array logical device status information."
$ 67

DETECT_SPARES "Accessing the array physical disk status information\n\
\n\
for physical disk device in location %1$s."
$ 68

FORMAT_WARNING "\
*** WARNING ***\n\
\n\
All data on the physical disk will be lost.\n\
\n\
Note: Before formatting a physical disk, the disk should be powered on\n\
      for at least 30 minutes.\n\
\n\
Do you wish to continue with the format operation?"
$ 69

SA_SELECTION_MENU_TITLE "7135 RAIDiant Array Service Aids\n\
  This selection provides the format, certify and microcode download\n\
  Service Aids for the 7135 RAIDiant Array."
$  70

DOWNLOAD_LATEST "Download the latest level of microcode."
$ 71

SELECT_DAC "\
The following is a list of all the array controllers that\n\
were found in the system.\n\
\n\
Select an array controller device from the list below.\n\
Move the cursor to the selection, then press Enter."
$ 72

SELECT_LUN "\
The following is a list of all the LUN's that were found\n\
in the array. Select a LUN from the list below.\n\
Move the cursor to the selection, then press Enter."
$ 73

SA_SELECTION_ITEM "%1$16.16s  %2$-16.16s  %3$6.6s  %4$8.8s  %5$-19.19s\n"
$
$  %1$-16.16s	Logical Name	 (16 chars max)
$  %2$16.16s	Location	 (16 chars max)
$  %3$6.6s	RAID level	 (6 chars max)
$  %4$-8.8s	Capacity	 (8 chars max including 2 for GB or MB)
$  %5$18.18s	Status		 (18 chars max)
$ 74

SELECT_SA "Select the Service Aid you would like to perform.\n\
Move the cursor to the selection, then press Enter."
$ 75

FORMAT_LUN "Format LUN"
$ 76

CERTIFY_LUN "Certify LUN"
$ 77

CERTIFY_PDISK "Certify Spare Physical Disk"
$ 78

FORMAT_PDISK "Format Physical Disk"
$ 79

CNTRL_uCODE "Array Controller Microcode Download"
$ 80

PDISK_uCODE "Physical Disk Microcode Download"
$ 81

ARRAY_EPROM "Array EPROM Download"
$ 82

TITLE_FORMAT "7135 RAIDiant Array FORMAT SERVICE AID"
$ 83

TITLE_CERTIFY "7135 RAIDiant Array CERTIFY SERVICE AID"
$ 84

TITLE_uCODE "7135 RAIDiant Array MICROCODE DOWNLOAD SERVICE AID"
$ 85

IPL_NAME_CNTRL "array controller"
$ 86

IPL_NAME_LUN "Logical Unit"
$ 87

CNTRL_SELECTION_ITEM "Disk Array Controller (%1$s) in location %2$s"
$ 88

PDISK_SELECTION_ITEM "%1$16.16s %3$8.8s %2$18.18s %4$-16.16s %5$7.7s"
$ 89

LUN_SELECTION_ITEM "%1$-16.16s  %2$-16.16s  %3$6.6s  %4$8.8s  %5$-18.18s"
$
$  %1$-16.16s	Logical Name	 (16 chars max)
$  %2$16.16s	Location	 (16 chars max)
$  %3$6.6s	RAID level	 (6 chars max)
$  %4$-8.8s	Capacity	 (8 chars max including 2 for GB or MB)
$  %5$18.18s	Status		 (18 chars max)
$
$ 90

DAC_BUSY "\
In order to perform Service Aids on the 7135 RAIDiant Array,\n\
the array controller must not be in use by other applications.\n\
\n\
The RAIDiant Disk Array Manager uses the the array controller\n\
device for the configuration editor. Quit the Disk Array\n\
Manager (if it is running), and run this Service Aid again."
$ 91

UNEXPECTED_ERROR "\
An unexpected error caused this Service Aid to stop."
$ 92

SELECT_PDISK "\
The following is a list of the physical disks that were\n\
found in the array. Select a physical disk from the list below.\n\
Move the cursor to the selection, then press Enter."
$ 93

LUN_CERTIFY_P_COMPLETE "\
Certifying %1$s %2$s in location %3$s.\n\
\n\
\t%4$d%% Percent Complete\n\
\n\
If you wish to cancel the certify operation, press Cancel or Exit.\n\
\n\
Please stand by."
$ 94

CERTIFY_COMPLETE "\
The certify operation has completed successfully.\n\
\n\
After running a Service Aid, it is recommended to run Diagnostics\n\
on the array subsystem in system verification mode.\n\
\n\
To return to the Service Aid selection menu, press Enter."
$ 95

SOURCE_FOR_uCODE "\
You may download microcode from a diskette or from the hard disk.\n\
If you wish to download microcode from the hard disk, please ensure\n\
the microcode file is in backup format and is located in the\n\
/etc/array/fw directory on the hard disk.\n\
\n\
Select the method of downloading microcode. Move the cursor to the\n\
selection and press Enter."
$ 96

LOAD_uCODE_DISKETTE "\
Download microcode from a diskette."
$ 97

LOAD_uCODE_HARD_DISK "\
Download microcode from the hard disk."
$ 98

INSERT_uCODE_DISKETTE "\
Insert the microcode diskette in the diskette drive (fd0).\n\
\n\
When finished, press Enter."
$ 99

PREV_OR_LATEST "\
You may download the latest level of microcode or a previous level\n\
of microcode.\n\
\n\
Select the level of microcode you wish to download. Move the cursor\n\
to the selection and press Enter."
$ 100

uCODE_FILE_ERROR "\
An error occurred while reading the microcode file. The microcode\n\
file may be corrupted.\n\
\n\
Contact your service representative if this problem continues."
$ 101

DISKETTE_ERROR "\
An error occurred while reading the microcode file from the\n\
diskette.\n\
\n\
Contact your service representative if this problem continues.\n\
\n\
To return to the Service Aid selection menu, press Enter."
$ 102

uCODE_IN_PROGRESS "\
The microcode download operation is in progress.\n\
\n\
Microcode file name: %1$s\n\
\n\
Device location: %2$s\n\
\n\
This operation may take up to 3 minutes to complete.\n\
\n\
Please stand by."
$ 103

uCODE_COMPLETE "\
The microcode download operation completed successfully.\n\
\n\
After running a Service Aid, it is recommended to run Diagnostics\n\
on the array subsystem in system verification mode.\n\
\n\
To continue, press Enter."
$ 104

DOWNLOAD_PREVIOUS "Download a previous level of microcode."
$ 105

PDISK_FORMAT_P_COMPLETE "\
Formatting %1$s physical disk in location %2$s.\n\
\n\
\t%3$d%% Percent Complete\n\
\n\
Please stand by."
$ 106

FORMAT_COMPLETE "\
The format operation completed successfully.\n\
\n\
After running a Service Aid, it is recommended to run Diagnostics\n\
on the array subsystem in system verification mode.\n\
\n\
To return to the Service Aid selection menu, press Enter."
$ 107

PROBLEM_DETECTED "\
A problem has been detected, run Diagnostics to isolate the problem."
$ 108

GET_LUN_STATUS "Accessing the array status information for logical\n\
\n\
device %1$s in location %2$s."
$ 109

uCODE_FILE_NAME "%1$s"
$ 110

READING_uCODE_FILES "\
Searching for valid microcode files, please stand by."
$ 111

AT_CURRENT_LEVEL "\
Current microcode file name: %1$s\n\
\n\
This device is already at the current microcode level.\n\
\n\
Do you wish to download the current microcode level anyway?"
$ 112

NO_DACS_FOUND "No disk array controllers were found in the system."
$ 113

NO_SPARES_FOUND "\
There are no physical disks in the correct state for this service aid.\n\
\n\
Note: You can only format a failed physical disk.\n\
\n\
Refer to the 7135 RAIDiant Array Installation and Service Guide\n\
for more information."
$ 114

NO_LUNS_FOUND "No configured LUN's were found in the array."
$ 115

DEVICE_BUSY "\
The device selected is in use by another application.\n\
\n\
Free the resource and run this Service Aid again."
$ 116

NO_CURRENT_uCODE_FILE "\
*** WARNING ***\n\
\n\
The current level of microcode was not found. If you download a\n\
different level of microcode, you will not be able to return to\n\
to this level of microcode.\n\
\n\
Do you wish to continue with the microcode download?"
$ 117

PASSIVE_CNTRL_SELECTED "\
The array controller you have selected is not in the active state.\n\
\n\
The Service Aids for the 7135 RAIDiant Array require an active\n\
array controller."
$ 118

PDISK_CERTIFY_P_COMPLETE "\
Certifying %1$s physical disk in location %2$s.\n\
\n\
\t%3$d%% Percent Complete\n\
\n\
If you wish to cancel the certify operation, press Cancel or Exit.\n\
\n\
Please stand by."
$ 119

NO_uCODE_FILES "\
Current microcode file name: %1$s\n\
\n\
No microcode files were found to download."
$ 120

CERTIFY_PFA "\
Predictive Failure Analysis has been reported.\n\
\n\
The %1$s physical disk in location %2$s should be replaced.\n\
\n\
Refer to the 7135 RAIDiant Array Installation and Service Guide\n\
for more information."
$ 121

CERTIFY_BAD "\
The Certify operation has stopped due to excessive errors.\n\
\n\
The %1$s physical disk in location %2$s should be replaced.\n\
\n\
Refer to the 7135 RAIDiant Array Installation and Service Guide\n\
for more information."
$ 122

PASSED "Passed"
$ 123

DEGRADED_PDISK "\
This physical disk is awaiting format.\n\
\n\
Use the 7135 RAIDiant Array Format Service Aid to format the\n\
physical disk in location %1$s.\n\
\n\
Refer to the 7135 RAIDiant Array Installation and Service Guide\n\
for more information."
$ 124

CNTRL_uCODE_COMPLETE "\
The microcode download operation completed successfully.\n\
\n\
Current microcode file name: %1$s\n\
\n\
Note: The controller download must be run on each controller\n\
      in the array subsystem.\n\
\n\
After running a Service Aid, it is recommended to run Diagnostics\n\
on the array subsystem in system verification mode.\n\
\n\
To return to the Service Aid selection menu, press Enter."
$ 125

PDISK_uCODE_COMPLETE "\
The microcode download operation completed successfully.\n\
\n\
Current microcode level: %1$s\n\
\n\
After running a Service Aid, it is recommended to run Diagnostics\n\
on the array subsystem in system verification mode.\n\
\n\
To return to the Service Aid selection menu, press Enter."
$ 126

TITLE_EEPROM "7135 RAIDiant Array EEPROM UPDATE SERVICE AID"
$ 127

EEPROM_FILE_REQ "\
Ensure the EEPROM file is located in the /etc/array/fw directory.\n\
\n\
File name:  7135eeprom.nn  (Where nn is the EEPROM level, 00-99)\n\
\n\
The controller (%1$s) in location %2$s will be updated\n\
with the latest level EEPROM file.\n\
\n\
Do you wish to update the controller EEPROM?"
$ 128

INSERT_EEPROM_DISKETTE "\
Insert the EEPROM diskette in the diskette drive (fd0)."
$ 129

EEPROM_UPDATE_IN_PROGRESS "\
The EEPROM update operation is in progress."
$ 130

EEPROM_UPDATE_COMPLETE "\
The EEPROM update operation completed successfully.\n\
\n\
Current EEPROM level: %1$s\n\
\n\
Note: The EEPROM update must be run on each controller\n\
      in the array subsystem.\n\
\n\
In order for all EEPROM changes to take effect, the array subsystem\n\
must be powered off and back on again.\n\
\n\
To return to the Service Aid selection menu, press Enter."
$ 131

NO_EEPROM_FILES "No EEPROM files were found in the system."
$ 132

EEPROM_DISKETTE "\
Update the EEPROM from a diskette"
$ 133

EEPROM_HARD_DISK "\
Update the EEPROM from the hard disk"
$ 134

UPDATE_EEPROM "Update EEPROM"
$ 135

TITLE_REP_CNTRL "7135 RAIDiant Array REPLACE CONTROLLER SERVICE AID"
$ 136

CNTRL_STATE_NOT_HIR "\
The controller (%1$s) in location %2$s is not\n\
in the correct state to be removed from the subsystem.\n\
\n\
A controller must be in the held-in-reset state before\n\
it can be removed from the subsystem.\n\
\n\
Do you wish to change this controller to the held-in-reset\n\
state so it can be removed?"
$ 137

CNTRL_STATE_OK "\
The controller (%1$s) in location %2$s is in\n\
the correct state to be removed from the subsystem.\n\
\n\
You may replace the controller at this time.\n\
\n\
When finished, it is recommended to run Diagnostics in\n\
System Verification mode to test the new controller."
$ 138

REPLACE_CONTROLLER "Replace Controller"
$ 139

PAIRED_DAC_HIR "\
The state of this controller (%1$s) in location %2$s can't\n\
be changed because the paired controller is in the held-in-reset state.\n\
\n\
Before attempting to change the state of this controller to held-in-reset,\n\
you must run Diagnostics on this controller.\n\
\n\
This will change the state of both controllers to active and will\n\
help determine why the paired controller is not in the active state."
$ 140

EEPROM_FILE_ERROR "\
An error occurred when trying to open the EEPROM file. The file\n\
file may be corrupted.\n\
\n\
EEPROM file name: %1$s\n\
\n\
Contact your service representative if this problem continues."
$ 141

REMOVE_OLD_DAC "\
The controller (%1$s) in location %2$s has been\n\
changed to the held-in-reset state.\n\
\n\
Remove the controller from the subsystem.\n\
\n\
After the controller has been removed, press Enter."
$ 142

INSERT_NEW_DAC "\
Insert the new controller (%1$s) in location %2$s.\n\
\n\
After the new controller has been inserted, press Enter."
$ 143

REPLACE_CONTROLLER_DONE "\
The Replace Controller Service Aid has completed.\n\
\n\
The new controller (%1$s) in location %2$s is in the\n\
held-in-reset state. In order for the new controller to become active,\n\
you must run Diagnostics in System Verification mode and verify the\n\
new controller is functioning properly.\n\
\n\
To return to the Service Aid selection menu, press Enter."
$ 144

STOP_IO_FOR_uCODE "\
*** WARNING ***\n\
\n\
The physical disk in location %1$s requires all I/O activity to be\n\
stopped before downloading microcode.\n\
\n\
Ensure that I/O activity from all LUN's and all hosts to this disk\n\
drive has stopped before continuing with the microcode download.\n\
\n\
Do you wish to continue with the microcode download?"
$ 145

DL_TO_ALL_PDISKS "Download microcode to ALL physical disks."
$ 146

DL_TO_ONE_PDISK  "Download microcode to one physical disk."
$ 147

DL_ALL_SELECTION_MENU "\
You may download microcode to all physical disks in the array subsystem\n\
or a single physical disk. Select the type of physical disk download to\n\
perform.\n"
$ 148

STOP_IO_FOR_ALL_uCODE "\
*** WARNING ***\n\
\n\
This operation requires all I/O activity to be stopped before\n\
downloading microcode.\n\
\n\
Ensure that I/O activity from all LUN's and all hosts to the array\n\
subsystem has stopped before continuing with the microcode download.\n\
\n\
Do you wish to download microcode to all physical disks in the subsystem?"
$ 149

CNTRL_NOT_RESPOND "\
The controller (%1$s) in location %2$s is not responding\n\
to commands. The controller may be in the held-in-reset state.\n\
\n\
Run Diagnostics in System Verification mode to ensure the controller\n\
is in the active state before attempting to run this Service Aid again."
$ 150

SINGLE_CTRL_SUBSYSTEM "\
This array subsystem only has one controller.\n\
\n\
Ensure that I/O activity from all LUN's and all hosts to the array\n\
subsystem has stopped before replacing the controller.\n\
\n\
After the controller has been replaced, run Diagnostics in System\n\
Verification mode and verify the new controller is functioning properly."
$ 151

