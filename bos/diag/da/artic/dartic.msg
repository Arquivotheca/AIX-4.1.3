$ @(#)62        1.2.1.2  src/bos/diag/da/artic/dartic.msg, daartic, bos41J, 9519A_all 5/6/95 14:42:15
$
$   COMPONENT_NAME: DAARTIC
$
$   FUNCTIONS: None  (Messages file)
$
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$


$set 1
$quote "
$

TESTING_REGULAR_GENERIC "\
TESTING %1$s %2$s\n\
IN LOCATION %3$s\n\n\
Please stand by."
$ Do not translate %1$s %2$s %3$s


TESTING_ADVANCED_GENERIC "\
TESTING %1$s %2$s\n\
IN LOCATION %3$s IN ADVANCED MODE\n\n\
Please stand by."
$ Do not translate %1$s %2$s %3$s

TESTING_LOOPMODE_GENERIC "\
TESTING %1$s %2$s\n\
IN LOCATION %3$s IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%4$6d\tpasses completed.\n\
%5$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"

$
$ This message is displayed while testing is performed in
$ loop mode.
$ %1$s is the device that is tested.
$ %2$s is the location code of the device as described in the general notes
$ %3$6d\t sets the number of diagnostics test passes completed.
$ %4$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title line.



TESTING_ADVANCED_MODE "\
TESTING T1/E1/J1 PORTMASTER ADAPTER /A %1$s\n\
IN LOCATION %2$s IN ADVANCED MODE\n\n\
Please stand by."
$
$ This message is displayed while testing is performed in
$ advanced mode.
$ %1$s is device that is tested.
$ %2$s is the location code of the device as described in the general notes
$ Leave line spacing as shown.  See general notes on length of title line.

TESTING_ADVANCED_MODE_WITH_WAIT "\
TESTING T1/E1/J1 PORTMASTER ADAPTER /A %1$s\n\
IN LOCATION %2$s IN ADVANCED MODE\n\n\
The test may take up to 5 minutes.\n\n\
Please stand by."
$
$ This message is displayed while testing is performed in
$ advanced mode.
$ %1$s is device that is tested.
$ %2$s is the location code of the device as described in the general notes
$ Leave line spacing as shown.  See general notes on length of title line.

TESTING_LOOPMODE "\
TESTING T1/E1/J1 PORTMASTER ADAPTER /A %1$s\n\
IN LOCATION %2$s IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%3$6d\tpasses completed.\n\
%4$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ This message is displayed while testing is performed in
$ loop mode.
$ %1$s is the device that is tested.
$ %2$s is the location code of the device as described in the general notes
$ %3$6d\t sets the number of diagnostics test passes completed.
$ %4$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title line.

ADVANCED_NO_STANDBY "\
TESTING T1/E1/J1 PORTMASTER ADAPTER /A %1$s\n\
IN LOCATION %2$s IN ADVANCED MODE"
$
$
$ This message is displayed while testing is performed in
$ advanced mode.
$ %1$s is the device that is tested.
$ %2$s is the location code of the device as described in the general notes
$ Leave line spacing as shown.  See general notes on length of title line.


HAVE_WRAP_PLUG "\
The following test requires a wrap plug, Part Number 91F9161.\n\n\
Do you have this wrap plug?\n\n\
Move cursor to selection, then press Enter."

$ Leave line spacing as shown.  See general notes on length of title line.
YES_OPTION "YES"
$ This option is shown when the user is asked to have a YES NO option.
$ Leave line spacing as shown.  See general notes on length of title line.

NO_OPTION "NO"
$ This option is shown when the user is asked to have a YES NO option.
$ Leave line spacing as shown.  See general notes on length of title line.

OPTION "Move cursor to selection, then press Enter "
$ Leave line spacing as shown.  See general notes on length of title line.

FINISHED "\
When finished, press Enter."
$
$ Leave line spacing as shown.  See general notes on length of title line.

T1_CABLE_REQUIRE "\
Do you want to test the T1/J1 cable?\n\
NOTE:  The following parts are required to run the test:\n\
       - The T1/J1 cable (Part Number 51G9431).\n\
       - Wrap plug (Part Number 91F9161).\n\n\
Move cursor to selection, then press Enter."

E1_CABLE_REQUIRE "\
Do you want to test the E1 cable?\n\
NOTE:  The following parts are required to run the test:\n\
       - The E1 cable (Part Number 51G9432).\n\
       - Wrap plug (Part Number 91F9161).\n\n\
Move cursor to selection, then press Enter."

PLUG_T1_CABLE "\
PLUG...........the T1/J1 cable to be tested (Part Number 51G9431)\n\
               into the connector on the adapter in location %1$s,\n\
               if it is not already plugged in.\n\
PLUG...........the wrap plug (Part Number 91F9161) into the other \n\
               end of the cable to be tested."

PLUG_E1_CABLE "\
PLUG...........the E1 cable to be tested (Part Number 51G9432)\n\
               into the connector on the adapter in location %1$s,\n\
               if it is not already plugged in.\n\
PLUG...........the wrap plug (Part Number 91F9161) into the other \n\
               end of the cable to be tested."
UNPLUG_WRAP_AND_CABLE "\
UNPLUG.........wrap plug from the cable.\n\
UNPLUG.........the cable from the connector on the adapter in\n\
               location %1$s.\n\
PLUG...........wrap plug (Part Number 91F9161) into the connector\n\
               on the adapter in location %2$s."


PLUG_WRAP "\
UNPLUG.........the cable, if attached, from the connector\n\
               on the adapter in location %1$s.\n\
PLUG...........the wrap plug (Part Number 91F9161) into the connector\n\
               on the adapter in location %2$s."

UNPLUG_WRAP "\
UNPLUG.........wrap plug from the connector on the adapter\n\
               in location %1$s.\n\
PLUG...........the network cable, if removed, into the connector\n\
               on the adapter in location %2$s."


UNPLUG_CABLE "\
UNPLUG.........the wrap plug from the cable.\n\
PLUG...........the network cable, if removed, into the connector\n\
               on the adapter in location %1$s."

PORTMASTER_GENERIC "PORTMASTER ADAPTER"
X25_OR_MP_2_GENERIC "Either X.25 Interface Coprocessor or Multiport/2"
MP2_4PORT_232 "4-Port EIA-232-C Multiport/2 Adapter"
MP2_8PORT_232 "8-Port EIA-232-C Multiport/2 Adapter"
MP2_6PORT_232 "6-Port Synchronous EIA-232-C Multiport/2 Adapter"
MP2_4PORT_232_C4 "4-Port EIA-232-C / 4-Port EIA-422-A Multiport/2 Adapter"
MP2_8PORT_422 "8-Port EIA-422-A Multiport/2 Adapter"
UNRECOGNISED_MP2_EIB "Mutiport/2 Adapter Without EIB Attached"

PM_8PORT_232 "8-Port EIA-232-D Portmaster Adapter/A"
PM_8PORT_422 "8-Port EIA-422-A Portmaster Adapter/A"
PM_6PORT_V35 "6-Port V.35 Portmaster Adapter/A"
PM_6PORT_X21 "6-Port X.21 Portmaster Adapter/A"
UNRECOGNISED_PM_EIB "Portmaster Adapter/A Without EIB Attached"

X25_ADAPTER "X.25 Interface Co-Processor/2 Adapter"

T1E1J1_ADAPTER "T1/E1/J1 PORTMASTER ADAPTER /A"
UNRECOGNISED_T1E1J1_EIB "T1E1J1 PORTMASTER ADAPTER/A Without EIB Attached"

MPQP_4PORT_ADAPTER "4-Port Multiprotocol Communications Controller"

FUNCTIONAL_MICROCODE_MISSING "851011 \
OPERATIONAL MICROCODE FILE MISSING\n\
The operational microcode file is either missing or not accessible.\n\n\
This operational microcode file is necessary in order to use \n\n\
adapter in normal system operation."


DIAG_MICROCODE_MISSING "855012 \
DIAGNOSTICS MICROCODE FILE MISSING\n\
The diagnostics microcode file is either missing or not accessible.\n\n\
This diagnostics microcode file is necessary in order to\n\
run Diagnostics."

DIAG_DEVICE_CANNOT_BE_DEFINED "855110 \
THE DIAGNOSTICS DEVICE DRIVER COULD NOT BE DEFINED\n\
No trouble was found.  However, the diagnostics device may not be \n\
operational because the system indicated that the resource could \n\
not be defined.\n\
No testing has been done.\n\n\
The resource is\n\
- %1$s      %2$s        %3$s"

LPP_DEVICE_CANNOT_UNCONFIGURED "855013 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE UNCONFIGURED\n\
No trouble was found.  However, the resource was not tested because\n\
the system indicated that the resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s      %2$s         %3$s \n\n\
To test this resource, you can:\n\
  Run Diagnostics from the Standalone Diagnostic Diskette or CDROM package."

DIAG_DEVICE_DRIVER_MISSING "855014 \
DIAGNOSTICS DEVICE DRIVER FILE MISSING\n\
The T1/E1/J1 Portmaster Diagnostics device driver file is\n\
either missing or not accessible.\n\n\
This T1/J1/E1 Diagnostics device driver file is necessary\n\
in order to run Diagnostics."

LPP_DEVICE_CANNOT_CONFIGURED "855015 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s        %2$s          %3$s"

DIAG_DEVICE_CANNOT_UNCONFIGURED "855110 \
THE DIAGNOSTICS DEVICE COULD NOT BE UNCONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s        %2$s         %3$s"

DIAG_DEVICE_CANNOT_CONFIGURED "855110 \
THE DIAGNOSTICS DEVICE COULD NOT BE CONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s        %2$s         %3$s"

LPP_DEVICE_CANNOT_SET_TO_DEFINE "855107 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE COULD NOT BE SET TO DEFINE STATE\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be set from DIAGNOSE\n\
state to DEFINE state.\n\n\
The resource is\n\
- %1$s        %2$s         %3$s"


DEVICE_DOES_NOT_EXIST_OLD ""
$ See below for updated message


LPP_DEVICE_CANNOT_SET_TO_DIAGNOSE "850106 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE COULD NOT BE SET TO DIAGNOSE STATE\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be set to DIAGNOSE\n\
state.\n\n\
The resource is\n\
- %1$s             %2$s         %3$s"


X25_OR_MP_GENERIC "Either X.25 Interface Coprocessor or Multiport"
UNRECOGNISED_MP_EIB "Mutiport Adapter Without EIB Attached"

C1X_ADAPTER "X.25 Interface Co-Processor Adapter"

DIAG_DEVICE_CANNOT_BE_UNDEFINED "855110 \
THE DIAGNOSTICS DEVICE DRIVER COULD NOT BE UNDEFINED\n\
No trouble was found.  However, the diagnostics device may not be \n\
operational because the system indicated that the resource could \n\
not be undefined.\n\n\
The resource is\n\
- %1$s      %2$s        %3$s"

DEVICE_DOES_NOT_EXIST "850109 \
THE SYSTEM INDICATES THAT THE DEVICE DOES NOT EXIST. \n\
No trouble was found.  However, no testing was performed because the\n\
ODM stanza for the device does not exist.\n\n\
The resource is\n\
- %1$s             %2$s         %3$s"





$ This for FRU message

$set 2

ARTIC_101  "Adapter power on self-test failure"
ARTIC_102  "Adapter Vital Product Data test failure"
ARTIC_103  "Adapter Interrupt Test failure"
ARTIC_104  "Adapter Central Processing Unit (C.P.U) test failure"
ARTIC_105  "Adapter Dynamic Random Access Memory (D.R.A.M) test failure"
ARTIC_107  "Adapter Channel Input/Output test failure"
ARTIC_108  "Adapter Test Failure"
ARTIC_118  "General Registers test failure"
ARTIC_119  "Adapter Download Diagnostics failure"
ARTIC_110  "Adapter Download Diagnostics test failure"
ARTIC_111  "Adapter Download Diagnostics test failure"
ARTIC_112  "Cable wrap test failure"
ARTIC_150  "Device Configuration failure"
ARTIC_151  "Device driver indicates a hardware failure"
ARTIC_152  "Adapter failure and error in determining which type of adapter"
ARTIC_SCC  "Adapter Serial Communication Controller (SCC) test failure"
ARTIC_GATE_ARRAY "Adapter logic test failure"
ARTIC_SOFTWARE_ERROR "Software"
T1_J1   "T1/J1 Portmaster Adapter /A "
E1      "E1 Portmaster Adapter /A "
ARTIC_UNKNOWN  "The test indicates a hardware failure"

$  This is for 4 Ports Adapter  and Portmaster

MPQP_INTERNAL_TESTS "Adapter Hardware Failure."
ELA_ERROR "Error log analysis indicates a software failure."
PORT_0_V35_WRAP_TEST_FAILURE "Port 0 - V.35 external wrap test failed."
PORT_0_232_WRAP_TEST_FAILURE "Port 0 - 232D external wrap test failed."
PORT_0_X21_WRAP_TEST_FAILURE "Port 0 - X.21 external wrap test failed."
PORT_0_422_WRAP_TEST_FAILURE "Port 0 - 422A external wrap test failed."
PORT_1_232_WRAP_TEST_FAILURE "Port 1 - 232D external wrap test failed."
PORT_1_V35_WRAP_TEST_FAILURE "Port 1 - V.35 external wrap test failed."
PORT_3_WRAP_TEST_FAILURE     "Port 3 - 232D external wrap test failed."
PORT_2_232_WRAP_TEST_FAILURE "Port 2 - 232D external wrap test failed."
PORT_2_422_WRAP_TEST_FAILURE "Port 2 - 422A external wrap test failed."

MPQP_TWIN_TAIL_TEST_FAILURE "The twin tail test has failed."
MPQP_CONNECTOR_0_WRAP_TEST_FAILURE "\
Interface cable Port 0 - V.35 connector wrap test failed."
MPQP_CONNECTOR_1_WRAP_TEST_FAILURE "\
Interface cable Port 0 - 232D connector wrap test failed."
MPQP_CONNECTOR_2_WRAP_TEST_FAILURE "\
Interface cable Port 0 - X.21 connector wrap test failed."
MPQP_CONNECTOR_3_WRAP_TEST_FAILURE "\
Interface cable Port 0 - 422A connector wrap test failed."
MPQP_CONNECTOR_4_WRAP_TEST_FAILURE "\
Interface cable Port 1 - 232D connector wrap test failed."
MPQP_CONNECTOR_5_WRAP_TEST_FAILURE "\
Interface cable Port 1 - V.35 connector wrap test failed."
MPQP_CONNECTOR_6_WRAP_TEST_FAILURE "\
Interface cable Port 3 - 232D connector wrap test failed."
MPQP_CONNECTOR_7_WRAP_TEST_FAILURE "\
Interface cable Port 2 - 232D connector wrap test failed."
MPQP_CONNECTOR_8_WRAP_TEST_FAILURE "\
Interface cable Port 2 - 422A connector wrap test failed."

CABLE_WRAP_TEST_FAILURE "Cable wrap test failed."
PORT_WRAP_TEST_FAILURE "Port wrap test failed."
MPQP_RS_232_CABLE_FAILURE "RS-232 cable wrap test failed."
MPQP_RS_422_CABLE_FAILURE "RS-422 cable wrap test failed."
MPQP_V_35_CABLE_FAILURE "V.35 cable wrap test failed."
MPQP_X_21_CABLE_FAILURE "X.21 cable wrap test failed."
V35_cable "V.35 Cable"
X21_cable "X.21 Cable"
cable_232 "RS-232  Cable"
cable_422 "RS-422  Cable"
MPQP_78PIN_CABLE "4-Port jumper cable wrap test failed."
CONFIG_FAILURE "Configurations fails."
MPQP_OPEN_DEVICE_ERROR "The device driver indicates a hardware failure."
MPQP_SOFTWARE_ERROR "Software"
MPQP_INTERFACE_CARD     "Multiprotocol Interface Card"
MPQP_JUMPER_CABLE       "4-port Multiprotocol (Rack) Jumper Cable"
MULTIPORT_INTERFACE_BOX "Multiport Interface Cable Assembly"

$ Reason code set used by device type "X-25"

R_X25_ADAPTER "An error was found on the adapter."
R_X25_DRAM "An error was found on the DRAM SIPs."
$ DRAM stands for Dynamic Random Access Memory.  SIP stands for
$ Single In-line Package.
R_X21_CABLE "An error was found with the X.21 interface adapter cable."
R_V24_CABLE "An error was found with the V.24 interface adapter cable."
R_V35_CABLE "An error was found with the V.35 interface adapter cable."
R_ELA "Error log analysis indicates a hardware error."
R_DD "Adapter hardware has caused a software failure."
F_X25_DRAM "DRAM SIPs on the adapter card"
$ DRAM stands for Dynamic Random Access Memory.  SIP stands for
$ Single In-line Package.
F_X21_CABLE "X.25 Attachment Cable - V.21"
F_V24_CABLE "X.25 Attachment Cable - V.24"
F_V35_CABLE "X.25 Attachment Cable - V.35"

$ This is for Multiport /2
MP4_232         "4-Port Multiport/2 Adapter RS-232"
MP6_SYN         "6-Port Multiport/2 Adapter SYNC"
MP8_232         "8-Port Multiport/2 Adapter RS-232"
MP8_422         "8-Port Multiport/2 Adapter RS-422"
MP4_4           "8-Port Multiport/2 Adapter RS-232/RS-422"
MP_4232EIB      "RS-232 Electronic Interface Board, Part Number 09F1957."
MP_6SYNEIB      "SYNC Electronic Interface Board, Part Number 15F8864."
MP_8232EIB      "RS-232 Electronic Interface Board, Part Number 09F1889."
MP_8422EIB      "RS-422 Electronic Interface Board, Part Number 15F8858."
MP_4_4_EIB      "RS-232/RS-422 Electronic Interface Board, Part Number 09F1890."
MP_GENKBMM      "256KB/512KB Memory module, Part Number 16F2267/16F2265."
MP_256KBMM      "256KB Memory module, Part Number 16F2267."
MP_512KBMM      "512KB Memory module, Part Number 16F2265."
MP_6SCABLE      "Synchronous Interface cable, Part Number 15F8868."
MP_48CABLE      "Multiport Interface cable, Part Number 00F5524."
MP_SWERR        "software error"
MP_ERRMSGS      "reserved"
MP_01           "Power on self test failed."
MP_02           "POST DRAM test failed, Low address."
MP_03           "POST DRAM test failed, High address."
MP_04           "Advanced DRAM test failed, Low address."
MP_05           "Advanced DRAM test failed, High address."
MP_06           "Base adapter test failed."
MP_07           "Electronic Interface Board/Base adapter test failed."
MP_08           "Multiport Interface Cable test failed."
MP_09           "Synchronous Interface Cable test failed."
MP_10           "reserved"
MP_11           "Error opening device driver."
MP_12           "Error configuring adapter."
MP_13           "Error configuring device driver."

EIB_6P_V35      "6-port V.35 Portmaster Interface Card"
EIB_6P_X21      "6-port X.21 Portmaster Interface Card"
EIB_8P_232      "8-port 232D Portmaster Interface Card"
EIB_8P_422      "8-port 422A Portmaster Interface Card"

MP2_EIB "Multiport/2 Interface Card (EIB)"

