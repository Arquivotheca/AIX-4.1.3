$ @(#)36        1.2  src/bos/diag/da/d7135/d7135.msg, da7135, bos41J, 9511A_all 3/13/95 08:51:33
$
$   COMPONENT_NAME: DA7135
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
$  $$$$$$$$$$$$$$$$$$$$$$  FRU SET  $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$set D7135_FRU_SET
$

CNTRL_CARD	       "Controller card"
DIFF_CABLE	       "SCSI DIFF cable"
SCSI_CABLES	       "SCSI cable assm"
SCSI_TERMINATOR        "SCSI terminator"
INT_DIFF_CABLE	       "Int. DIFF cable"
PWR_DIST_CARD	       "Power Dist Card"
DC_SENSE_CABLE_LOOM    "P.D. cable assm"
PWR_CABLE_3WAY	       "3-way P. cable"
LOWER_PSU_FAN1	       "D. Unit F/PS 1 "
LOWER_PSU_FAN2	       "D. Unit F/PS 2 "
LOWER_PSU_FAN3	       "D. Unit F/PS 3 "
UPPER_PSU_FAN1	       "E. Unit F/PS 1 "
UPPER_PSU_FAN2	       "E. Unit F/PS 2 "
UPPER_PSU_FAN3	       "E. Unit F/PS 3 "
CNTRL_3_FAN_ASSEMBLY   "3-fan assembly "
CNTRL_FRONT_FAN        "One-fan assm.  "
CABLE		       "One-fan cable  "
LOWER_MULT_FAN	       "Mult Lower fans"
UPPER_MULT_FAN	       "Mult Upper fans"
FRONT_FAN	       "Front fan      "
FRONT_FAN_CABLE        "Front fan cable"
DC_CABLES	       "DC cables      "
PDC_CARDS	       "PDC cards      "
MOTHER_BOARD	       "Mother board   "
DRIVE_CARRIER	       "Drive carrier  "
AWAITING_FORMAT        "Awaiting format"
CONNECTOR_CARD	       "Connector card "
DIFF_SE_CONV_CARD      "Converter card "
DC_POWER_CABLE	       "DC power cable "
DRIVE_ASSEM_2GB        "2.0GB disk assm"
DRIVE_ASSEM_1_3GB      "1.3GB disk assm"
UNKNOWN_PDISK	       "Disk module    "
SUBSYSTEM	       "Array Subsystem"
ADAPTER_CARD	       "SCSI Adapter   "
BACKPLANE	       "Backplane assm."
DISK_MOD	       "Disk module    "
MORE_ISOLATION	       "More isolation "

$
$ The above messages are the SRN fname messages used in the FRU structs.
$ The message numbers, from d7135_msg.h, are used in the FRU struct.
$ NOTE: fname is 16 characters maximum.
$

DIFF_PROBLEM	 "Differential SCSI problem."
CHECK_CAB_CONN	 "Ensure that all cables and connectors are connected correctly."
REPL_IN_ORDER	 "Replace FRU's in order."
BLANK		 "    "
ONE_OF_2_CNTRL	 "One of two controller cards is failing or powering down."
CHECK_PDC_LIGHT  "Ensure the power distribution light is on."
CRIT_POWER_FAULT "Critical power fault."
AC_INPUT_POWER	 "AC input power to more than one power unit is on, either the upper or lower drawer is missing."

FAN_3_ASSEMBLY	 "Replace the controller 3-fan assembly."
REPLACE_FAN	 "Replace the fan."
REPLACE_CABLE	 "Replace the cable."
CRIT_FAN_FAULT	 "Critical fan fault."
CHECK_FAN_GOOD	 "Check fan good lights."
R_POWER_MISSING  "Redundant power missing."
SECOND_FAILURE	 "Most likely a secondary failure to another SRN."
PWR_DIST_CARD_B  "Power distribution card A in controller drawer failed."
PWR_DIST_CARD_A  "Power distribution card B in controller drawer failed."
FORMAT_LUN	 "Use RDAM to format the logical unit."
INT_SCSI_BUS	 "Internal SCSI Bus failure."
MISSING_OPTIONS  "Missing options SRN."
RDAC_ACNTRL_FAIL "Controller failure."
RDAC_PCNTRL_FAIL "Passive controller failure."
REPL_DRIVE	 "Replace the drive."
TUR_FAILURE	 "Test Unit Ready failure."
M_SEL_FAILURE	 "User failed by RDAM."
S_OF_DAY_FAILURE "Start of Day failure."

PARAM_MM_MSG "Parameters do not match."
$
WRONG_SECTOR "Incorrect sector size."
$
WRONG_CAPACITY_F "Incorrect capacity. The disk drive might belong to a different logical unit."
$
INC_MODE_PARAMS  "Incorrect mode parameters."
$
WRONG_CNTRL_SN_F "Incorrect array controller number."
$
CH_MISMATCH_F	 "Channel does not match. The disk drive might belong to a different logical unit."
$
ID_MISMATCH_F	 "ID does not match. The disk drive might belong to a different logical unit."
$
DR_PEND_FORMAT_F "The disk drive module is waiting to be formatted."
$
WRONG_DRIVE	 "Incorrect disk drive module."
$
PFA_REPL_DRIVE	 "Predictive Failure Analysis."
$
$ The above messages are the SRN fmsg's used in the FRU structs (see array.h).
$

RMSG_FRUC "Sense data indicates a failing FRU."
RMSG_SUBSYS "Mode sense data indicates a subsystem failure."
RMSG_F_LUN "The logical unit is waiting to be formatted."
RMSG_SCSI "Host unable to communicate with the controller."
RMSG_MORPS "Missing Options has detected a problem."
RMSG_ELA "Error Log Analysis indicates a hardware failure."
RMSG_DRIVE "Mode sense data indicates a drive failure."
RMSG_CNTRL "Mode sense data indicates a controller failure."
RMSG_DEV_CFG "The controller configuration failed."
RMSG_PAR_CFG "The SCSI adapter configuration failed."
RMSG_OPENX "The device failed to open in Diagnostic mode."
$
$ The above messages are the SRN rmsg messages used in the FRU structs.
$

SW "SW"
$ SW stands for SoftWare.
RMSG_CNTRL_SW  "The passive to active controller switch failed."
$
RMSG_CNTRL_ELA "Error Log Analysis indicates the passive controller has failed."
$
DR_WARNING "Drive is in the warning state."
$
RMSG_101 "Differential SCSI problem."
$
RMSG_102 "One of two array controller cards fails or powers down."
$
RMSG_200 "Critical power fault. No AC input power is present at more than one fan-and-power-supply assembly in either the disk drive unit or the expansion unit."
$
RMSG_FAN "The fan and power supply assembly is failing."
$ Used for SRN's 201 - 206.
RMSG_207 "One of the fans in the three-fan cooling assembly has failed."
$
RMSG_208 "The one-fan cooling assembly has failed."
$
RMSG_209 "Critical fan fault. Two or more fan-and-power-supply assemblies have failed in the disk drive unit."
$ Also used for SRN 211
RMSG_210 "Critical fan fault."
$
RMSG_212 "Redundant power is missing."
$
RMSG_213 "Power distribution card B in the control unit has failed."
$
RMSG_214 "Power distribution card A in the control unit has failed."
$
RMSG_302 "Internal SCSI bus failure."
$
RMSG_401 "Error Log Analysis indicates an active array controller failure."
$
RMSG_402 "Error Log Analysis indicates a passive array controller failure."
$
RMSG_PDISK "A physical disk failed to respond."
$
RMSG_PFA "Predictive Failure Analysis."
$ 102
RMSG_ELA_PFA "Error Log Analysis indicates Predictive Failure Analysis."
$ 103
RMSG_ELA_SUBSYS "Error Log Analysis indicates subsystem failure. Run Diagnostics to isolate the problem."
$ 104
COMP_F "Component failure."
$
EXCH_FRU "Exchange the FRU for a new FRU."
$
FORMAT_F "Format failure."
$
WRITE_F "Write failure."
$
MS_FAILURE "Mode Select failure."
$
USE_READI "If the problem can not be corrected with the\n\
Disk Array Manager, then replace the drive."
$
FORMAT_DISK "Format the disk drive module."
$
PROB_REMAINS "If the problem remains, exchange the FRUs for new FRUs".
$
REFER_TO_PUBS "Refer to the 7135 Installation and Service Guide."
$
EXCH_FRUS "Exchange the FRUs for new FRUs."
$
PWR_DIST_CARD_AB "Power distribution card A or B."
$
USE_MAP_2010 "For further isolation, go to 7135 START MAP 2010."
$

AC_DISK_MOD_1_3GB "67G5069"
AC_DISK_MOD_2_0GB "67G5070"
SF_DISK_MOD_1_1GB "SF 1.1"
SF_DISK_MOD_2_2GB "SF 2.2"
SF_DISK_MOD_4_4GB "SF 4.4"
OEM_DISK_MOD      "OEM Disk"
$ Disk module P/N's

RMSG_TTD_CIOP "The physical disk is not support in the dual-active configuration."
$

RMSG_404 "The controller is reporting an ICON chip failure."
$
RMSG_405 "The configuration of the controller has changed."
$
RMSG_406 "Array controller switch failure."
$
RMSG_311 "The controller failed to become active due to an ICON chip failure."
$
RMSG_312 "The controller failed to become active due to a non-supported drive."


$  $$$$$$$$$$$$$$$$$$$$$$  MESSAGE SET	$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$set D7135_MSG_SET
$

TITLE "TESTING ARRAY DEVICE (%1$s) IN LOCATION %2$s\n\
%3$s"
$ 1

ADVANCED_MODE "IN ADVANCED MODE."
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

STAND_BY "This test may take up to %1$d minute(s) to complete.\n\
\n\
Please stand by."
$ %1$d is the time the test may take in minutes.
$ 8

LOOPMODE "\
Testing multiple times.\n\
\n\
This test may take up to %1$d minute(s) to complete.\n\
\n\
%2$6d\tpasses completed.\n\
%3$6d\terrors logged.\n\
\n\
To stop the test, use 'Cancel.'"
$
$ %1$d is the time the test may take in minutes.
$ %2$6d\t is the number of diagnostic test passes completed.
$ %3$6d\t is the number of errors logged during the tests.
$ 9

USE_UP_DOWN_KEYS "\
It may be necessary to use the cursor up/down keys to view\n\
all of the information. When finished, press Enter."
$ 10

DO_YOU_WISH "Do you wish to run this test?\n"
$ 11

ARRAY_STATUS "\
This screen shows the status of the logical and physical\n\
devices in the array. It may be necessary to use the cursor\n\
up/down keys to view all of the information.\n\
\n\
Refer to the 7135 RAIDiant Array Installation and Service\n\
documentation for more information.\n\
\n\
Note: Physical disk location codes\n\
      U=Upper L=Lower F=Front R=Rear\n\
      Example: UF-1 = Upper Front drawer, position number 1.\n\
\n\
				    RAID\n\
     Device Name  Location	    Level   Capacity  Status\n\
----------------  ----------------  ------  --------  ------------------\n"
$ 12

ARRAY_STATUS_NOTE "\n\
Note: Physical disk location codes\n\
      U=Upper L=Lower F=Front R=Rear\n\
      Example: UF-1 = Upper Front drawer, position number 1.\n"
$ 13

ARRAY_STATUS_ITEM "%1$16.16s  %2$-16.16s  %3$6.6s  %4$8.8s  %5$-19.19s\n"
$
$  %1$-16.16s	Logical Name	 (16 chars max)
$  %2$16.16s	Location	 (16 chars max)
$  %3$6.6s	RAID level	 (6 chars max)
$  %4$-8.8s	Capacity	 (8 chars max including 2 for GB or MB)
$  %5$18.18s	Status		 (18 chars max)
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
$ 18
CNTRL_STATUS_ACTIVE	"Active            "
$ 19
CNTRL_STATUS_FAILED	"Passive Failed    "
$ Controller status messages (18 chars max).
$ 20

CNTRL_SWITCH " "
$ 21

OPTIMAL_DRIVE	       "Optimal Drive     "
NON_EXISTENT	       "Non-Existant Drive"
NON_SUPPORT_ID	       "Non-Supported ID  "
SPARE_DRIVE	       "Spare Drive       "
FAILED_DRIVE	       "Failed Drive      "
TUR_FAILURE_S	       "TUR Failure       "
WRITE_FAILURE	       "Write Failure     "
S_OF_DAY_FAIL	       "SOD Failure       "
REPLACED_DR	       "Replaced Drive    "
RECONSTR_INIT	       "Reconstruct. Init."
DRIVE_WARNING	       "Dr. Warning State "
PARAM_MISMATCH	       "Parameter Mismatch"
WRONG_CAPACITY	       "Wrong Capacity    "
WRONG_CNTRL_SN	       "Wrong Cntrl. SN   "
ID_MISMATCH	       "ID Mismatch       "
THIS_CNTRL	       "This Cntrl. OK    "
DR_FORMAT_INIT	       "Drive Format Init."
WRONG_DR_REPL	       "Wrong Dr. Replaced"
NON_SUPPORT_CH	       "Non-Supported CH  "
NON_SUPP_CH_ID	       "Non-Supported ID  "
HOT_SPARE_DRIV	       "Hot Spare Drive   "
COMP_FAILURE	       "Component Failure "
FORMAT_FAILURE	       "Format Failure    "
USER_FAILED_MS	       "Failed by User    "
FORMAT_INIT	       "Format Init.      "
WRONG_SECTOR_S	       "Wrong Sector Size "
INC_MODE_PARAM	       "Invalid Mode Param"
CH_MISMATCH	       "Channel Mismatch  "
DR_PEND_FORMAT	       "Dr. Pending Format"
$ ----- array physical status msg's above, array logical status msg's below.
OPTIMAL_LUN	       "Optimal LUN       "
R0_DR_FORMAT	       "RAID 0 Dr. Format "
AWAIT_FORMAT	       "Awaiting Format   "
BAD_LUN 	       "LUN open failed   "
UNKNOWN 	       "Unknown Status    "
DR_FAILURE	       "Drive Failure     "
REPL_DR_FORMAT	       "Dr. is Formatting "
MULT_DR_FAILED	       "Mult. Dr. Failure!"
FORMAT_IN_PROG	       "Format in Progress"
REPL_WRONG_DR	       "Replaced Wrong Dr."
OPTIMAL_SUB_LUN        "Optimal Sub LUN   "
$
$ The above messages are the status messages used in the array status.
$ NOTE: The status field is 18 chars maximum (see the ARRAY_STATUS_ITEM msg).
$

DR_2GB		"2.0GB"
$ 62

DR_1_3GB	"1.3GB"
$ 63

TEST_OTHER_CNTRL "\
It is recommended to test the other subsystem controller.\n\
Have you tested the other subsystem controller?"
$ 64

RETURN_CNTRL_STATE "\
The state of the subsystem has been changed by running Diagnostics.\n\
You may change the state of the subsystem back to the previous state.\n\
\n\
CAUTION: Changing back to the previous state will cause one of the\n\
	 controllers to go into the passive or held-in-reset state.\n\
\n\
Do you wish to return the subsystem to the previous state?"
$ 65

CNTRL_STATUS_ALT_PATH	"Alternate Path    "
$ 66

ACCESS_ARRAY_STATUS "Accessing the array status information."
$ 67

DETECT_SPARES "Testing physical disk in location %1$s."
$ 68

PERFORM_CNTRL_SWITCH "Performing a controller switch."
$ 69

GET_LUN_STATUS "Accessing the array status information for logical\n\
\n\
device %1$s in location %2$s."
$ 70

PSCAN  "Parity Scan"
$ 71

PSCAN_MISMATCH "Parity Mismatch"
$ 72

PSCAN_ERROR "Parity Error"
$ 73

MORPS_SEL_MENU_1 " "
$ 74

MORPS_SEL_MENU_2 " "
$ 75

MORPS_SEL_MENU_3 " "
$ 76

MORPS_SEL_MENU_4 " "
$ 77

ACCESS_SUBSYS_STATUS "Accessing the array subsystem status information."
$ 78

SPARE_DISK_NAME "Spare Drive"
$ 79

HOT_SPARE_DISK_NAME "Hot Spare Drive"
$ 80

CNTRL_SW_BACK " "
$ 81

CNTRL_STATUS_HIR	"Held in Reset     "
$ 82

$  $$$$$$$$$$$$$$$$$$$$$$$$$  MNG SET  $$$$$$$$$$$$$$$$$$$$$$$$$$
$set D7135_MNG_SET
$

UNEXPECTED_ERROR "%1$06.6lx \
SERVICE INFORMATION FOR ARRAY DEVICE (%2$s) IN LOCATION %3$s\n\
\n\
An unexpected error caused testing to stop."
$

RUN_RAID_MANAGER "%1$06.6lx \
SERVICE INFORMATION FOR ARRAY DEVICE (%2$s) IN LOCATION %3$s\n\
When using the Disk Array Manager, certain activity (such as\n\
failing drives, etc.) will cause certain SRN's to be reported.\n\
\n\
Please run the Disk Array Manager to resolve as many SRN's\n\
as possible before calling for service or replacing any parts.\n\
\n\
Refer to the 7135 RAIDiant Array for AIX: Installation and\n\
Reference for more information."

NO_uCODE "%1$06.6lx \
SERVICE INFORMATION FOR ARRAY DEVICE (%2$s) IN LOCATION %3$s\n\
The 7135 Array is reporting that the microcode has not been loaded.\n\
\n\
Exit this test and run the Microcode Download Service Aid.\n\
\n\
Refer to the 7135 RAIDiant Array Installation and Service\n\
documentation for more information."
$

UNKNOWN_LUN "%1$06.6lx \
SERVICE INFORMATION FOR ARRAY DEVICE (%2$s) IN LOCATION %3$s\n\
Diagnostics has detected one or more LUN's with unknown status.\n\
\n\
A LUN either failed to configure or would not respond to commands.\n\
\n\
Use the Disk Array Manager to correct this problem.\n\
\n\
Refer to the 7135 RAIDiant Array for AIX: Installation and\n\
Reference for more information."
$

CHECK_CABLES "%1$06.6lx \
SERVICE INFORMATION FOR ARRAY DEVICE (%2$s) IN LOCATION %3$s\n\
A problem has been detected that may be caused by a loose or\n\
incorrect cable connection.\n\
\n\
Before replacing a physical disk drive, ensure all cables and\n\
terminators are connected correctly.\n\
\n\
Refer to the 7135 RAIDiant Array Installation and Service\n\
documentation for more information."
$

DAC_IN_RESET "%1$06.6lx \
SERVICE INFORMATION FOR ARRAY DEVICE (%2$s) IN LOCATION %3$s\n\
The controller configuration failed. In dual active configurations,\n\
if a controller is in the held-in-reset state, all commands sent\n\
to the controller will fail.\n\
\n\
To remove a controller from the held-in-reset state, run Diagnostics\n\
on the paired controller."
$

PDISK_FORMAT_PENDING "%1$06.6lx \
SERVICE INFORMATION FOR ARRAY DEVICE (%2$s) IN LOCATION %3$s\n\
The %4$s physical disk in location %5$s is reporting corrupted\n\
format.\n\
\n\
Use the 7135 Service Aids to format the physical disk drive."
$ 


