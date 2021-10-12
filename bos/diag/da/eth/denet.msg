$ @(#)43	1.4.2.6  src/bos/diag/da/eth/denet.msg, daeth, bos411, 9428A410j 11/1/93 09:40:45
$
$   COMPONENT_NAME: DAETH
$
$   FUNCTIONS: None 
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

 
$set 1
$quote "

ETHERNET_DEVICE "ETHERNET ADAPTER"


ADVANCED_NO_STANDBY "\
TESTING ETHERNET ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE"
$
$ %1$s is the device that is tested.
$ %2$s is the location code of the device as described in the general notes
$ Leave line spacing as shown.  See general notes on length of title line.

ADVANCED_NO_STANDBY_WITH_WAIT "\
TESTING ETHERNET ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
The test may take up to 5 minutes.\n\n\
Please stand by."
$
$ %1$s is the device that is tested.
$ %2$s is the location code of the device as described in the general notes
$ Leave line spacing as shown.  See general notes on length of title line.


WHICH_TYPE_CONNECTOR "\
Which connector is being used on the Ethernet adapter?\n\n\
Move cursor to selection, then press Enter.\n"
$
$ This message is displayed to find out which wrap plug to have
$ the user attach to the adapter, which has two connectors on it.
$ BNC is a technical term.  See general notes.
$ Leave line spacing as shown.

DIX_OPTION "DIX  (Multipin receptor)"
$
$ This option is used to identify the type of connect being used.
$ DIX is a technical term.  See general notes.

BNC_OPTION "BNC  (Coaxial connector)"
$
$ This option is used to identify the type of connect being used.
$ BNC is a technical term.  See general notes.


$ Menu 887004

XCEIVER_EXIST "\
Is there a transceiver connected to the cable attached to the\n\
Ethernet adapter?\n\n\
Move cursor to selection, then press Enter."
$

$ Menu 887005

CONVERTER_EXIST "\
Is there a RJ-45 converter attached to the Ethernet adapter or \n\
a transceiver connected to the cable attached to the Ethernet adapter?\n\n\
Move cursor to selection, then press Enter."
$

TEST_XCEIVER "\
Do you want to test the Ethernet transceiver?\n\n\
Move cursor to selection, then press Enter."

$

TEST_CONVERTER "\
Do you want to test the Ethernet transceiver or converter?\n\n\
Move cursor to selection, then press Enter."

$

WHICH_TYPE_XCEIVER "\
What type of Ethernet transceiver is connected to Ethernet adapter?\n\n\
Move cursor to selection, then press Enter."

WHICH_TYPE_CONVERTER "\
What type of Ethernet transceiver or converter is connected to\n\
the Ethernet adapter?\n\n\
Move cursor to selection, then press Enter."

BASET "10BASE-T (Part Number 02G7429)"
BASE2 "10BASE2  (Part Number 02G7435)"
UNKNOWN "Type Not Known (NO TEST will be performed)"
RJ_45 "RJ-45 Converter (Part Number 51G8479)"


$ Menu 887010

WRAP_PLUG_TWISTED "\
The following test requires wrap plug Part Number 00G2380.\n\n\
Do you have this wrap plug?\n\n\
Move cursor to selection, then press Enter."
$

$ Menu 887008

WRAP_PLUG_BNC "\
The following test requires a wrap plug, one of the following Part Numbers:\n\
71F1168, 70F9626 or 02G7433.\n\n\
Do you have one of these wrap plugs?\n\n\
Move cursor to selection, then press Enter."

$ Menu 887009

WRAP_PLUG_DIX "\
The following test requires a wrap plug, either Part Number 71F1167\n\
or 70F9625.\n\n\
Do you have one of these wrap plugs?\n\n\
Move cursor to selection, then press Enter."
$

ISOLATE_XCEIVER "\
The following test is used for problem isolation between the Ethernet\n\
adapter and the Ethernet transceiver.\n\
The test requires a wrap plug, either Part Number 71F1167\n\
or 70F9625.\n\n\
Do you have one of these wrap plugs?\n\n\
Move cursor to selection, then press Enter."
$

ISOLATE_CONVERTER "\
The following test is used for problem isolation between the Ethernet\n\
adapter and the Ethernet RJ-45 converter.\n\
The test requires a wrap plug, either Part Number 71F1167\n\
or 70F9625.\n\n\
Do you have one of these wrap plugs?\n\n\
Move cursor to selection, then press Enter."
$

YES_OPTION "YES"
$ Leave line spacing as shown.  See general notes on length of title line.

NO_OPTION "NO"
$ Leave line spacing as shown.  See general notes on length of title line.

SELECTION_OPTION "Move cursor to selection, then press Enter "
$ Leave line spacing as shown.  See general notes on length of title line.

FINISHED "\
When finished, press Enter."
$
$ Leave line spacing as shown.  See general notes on length of title line.

PLUG_XCEIVER_T "\
UNPLUG.........the transceiver from the network.\n\
	       NOTE:  DO NOT UNPLUG the cable between the Ethernet adapter\n\
               and the transceiver.\n\
PLUG...........wrap plug (Part Number 00G2380) into the connector on\n\
               the Ethernet transceiver.\n\
ENSURE.........the link test switch on the transceiver is in the ON position."

$

XCEIVER_T_2_PHASE "\
UNPLUG.........the wrap plug from the connector on the Ethernet transceiver.\n\
MOVE...........the link test switch on the transceiver to the OFF position.\n\n\
When finished, press Enter."
$


PLUG_XCEIVER_10BASE_2 "\
UNPLUG.........the cable between the transceiver and the network.\n\
               NOTE:  if a T connector is used, unplug the T connector\n\
               from the transceiver. DO NOT UNPLUG any cable from the\n\
               T connector because this will bring the network down.\n\
ENSURE.........the cable between the transceiver and the Ethernet\n\
               adapter is attached.\n\
PLUG...........the wrap plug/terminator (Part Number 71F1168 or\n\
               70F9626 or 02G7433) into the BNC (Coaxial) connector on\n\
	       the transceiver.\n\n\
When finished, press Enter."
$
PLUG_CONVERTER "\
PLUG...........the Ethernet RJ-45 converter into the connector on the\n\
               Ethernet adapter, if not already attached.\n\
UNPLUG.........the network cable from the RJ-45 converter.\n\
PLUG...........the wrap plug (Part Number 02G2380) into the connector\n\
               on the Ethernet RJ-45 converter.\n\n\
When finished, press Enter."
$


DD_FILE_BAD "887029 \
ETHERNET DEVICE DRIVER FILE MISSING\n\
The diagnostics Ethernet device driver file is either missing or \n\n\
not accessible.  This device driver is necessary in order to run Diagnostics."

$
$

NETWORK_BAD "887031 \
ETHERNET NETWORK PROBLEM\n\n\
There was no problem found in the Ethernet adapter.\n\
However, the network caused intermittent error to the Ethernet adapter.\n\
Ask the network administrator to check the Ethernet network \n\
to make sure it is in normal working operation."

$
$

PLUG_DIX "\
UNPLUG.........any network cable(s), if attached, from the\n\
               connector on the Ethernet adapter.\n\
PLUG...........the wrap plug (Part Number 71F1167 or 70F9625) into\n\
               the connector on the Ethernet adapter.\n\n\
When finished, press Enter."
$

PLUG_BNC "\
UNPLUG.........the transceiver from the connector on the Ethernet adapter.\n\
PLUG...........the wrap plug (Part Number 71F1168 or 70F9626 or 02G7433)\n\
               into BNC (Coaxial) connector on the Ethernet adapter.\n\n\
When finished, press Enter.\n"
$ This message is displayed for plugging BNC wrap plug

STIL_PLUG_TWISTED "\
UNPLUG.........the network cable, if attached, from the connector on the\n\
               Ethernet adapter.\n\
PLUG...........the wrap plug (Part Number 00G2380) into the twisted\n\
               pair connector on the Ethernet adapter."
$ This message is displayed for plugging twisted pair wrap plug



UNPLUG_XCEIVER_AND_PLUG_DIX "\
UNPLUG.........the transceiver from the connector on the Ethernet adapter.\n\
PLUG...........the wrap plug (Part Number 71F1167 or 70F9625) into the\n\
               connector on the Ethernet adapter.\n\n\
When finished, press Enter."

UNPLUG_WRAP_GENERIC "\
UNPLUG.........the wrap plug from the connector on the Ethernet adapter.\n\
PLUG...........the network cable, if removed, into the connector on\n\
               the Ethernet adapter.\n\n\
When finished, press Enter."
$

UNPLUG_DIX_AND_PUT_BASE2 "\
UNPLUG.........the wrap plug or terminator from the connector on\n\
               the transceiver.\n\
PLUG...........the network cable(s), if removed, into the connector on\n\
               the transceiver.\n\
PLUG...........the transceiver, if removed, into the connector on\n\
               the Ethernet adapter.\n\n\
When finished, press Enter."
$
UNPLUG_T_WRAP_AND_PUT_CONVERTER_BACK "\
UNPLUG.........the wrap plug from the connector on the Ethernet RJ-45\n\
               converter.\n\
PLUG...........the network cable, if removed, into the connector on the\n\
               Ethernet RJ-45 converter.\n\
PLUG...........the Ethernet RJ-45 converter, if removed, into the connector\n\
               on the Ethernet adapter.\n\n\
When finished, press Enter."
$
PLUG_T_XCEIVER_BACK "\
PLUG...........the network cable back into the connector on the Ethernet\n\
               transceiver, if removed.\n\
ENSURE.........the link test switch on the transceiver is in the ON position.\n\
               NOTE: The link test switch must be left ON for normal\n\
               network operation.\n\
PLUG...........the Ethernet transceiver into the connector of the Ethernet\n\
               adapter, if removed.\n\n\
When finished, press Enter."
$

UNPLUG_CONVERTER_AND_PLUG_DIX "\
UNPLUG.........the RJ-45 converter from the connector on the Ethernet adapter.\n\
PLUG...........the wrap plug (Part Number 71F1167 or 70F9625) into the\n\
               connector on the Ethernet adapter.\n\n\
When finished, press Enter."

$ Menu 887026

UNPLUG_DIX_WRAP_AND_PUT_XCEIVER_BASE2 "\
UNPLUG.........the wrap plug from the connector on the Ethernet adapter.\n\
PLUG...........the network cable(s), if removed, into the connector on\n\
               the transceiver.\n\
PLUG...........the transceiver, if removed, into the connector\n\
               on the Ethernet adapter.\n\n\
When finished, press Enter."

$ Menu 887027

UNPLUG_DIX_WRAP_AND_PUT_XCEIVER_TWISTED "\
UNPLUG.........the wrap plug from the connector on the Ethernet adapter.\n\
PLUG...........the network cable, if removed, into the connector on\n\
               the transceiver.\n\
ENSURE.........the link test switch on the transceiver is in the ON position.\n\
               NOTE: The link test switch must be left ON for normal\n\
               network operation.\n\
PLUG...........the transceiver, if removed, into the connector\n\
               on the Ethernet adapter.\n\n\
When finished, press Enter."

$ Menu 887015

UNPLUG_WRAP_CABLE_FOR_ISOLATION "\
The following test is used for problem isolation between the Ethernet\n\
adapter (circuit breaker) and the Ethernet network.\n\n\
UNPLUG.........any wrap plug or network cable from the connector on\n\
               the Ethernet adapter.\n\n\
When finished, press Enter."
$
 
$ Menu 887028

UNPLUG_DIX_WRAP_AND_PUT_CONVERTER "\
UNPLUG.........the wrap plug from the connector on the Ethernet adapter.\n\
PLUG...........the network cable, if removed, into the connector on\n\
               the RJ-45 converter.\n\
PLUG...........the converter, if removed, into the connector\n\
               on the Ethernet adapter.\n\n\
When finished, press Enter."

$
$ Menu 887030
$

PLUG_NETWORK_CABLE_ONLY "\
PLUG...........the network cable back into the connector on\n\
               the Ethernet adapter, if removed.\n\n\
When finished, press Enter."
 
STIL_ETH_PLUG_BNC "\
UNPLUG.........any network cable(s) or wrap plug(s), if attached, from the\n\
               connector(s) on the Ethernet adapter.\n\
PLUG...........the wrap plug (Part Number 71F1168 or 70F9626 or 02G7433)\n\
               into BNC (Coaxial) connector on the Ethernet adapter."
$ This message is displayed for plugging BNC wrap plug


STIL_ETH_PLUG_TWISTED "\
UNPLUG.........the network cable, if attached, from the connector on the\n\
               Ethernet adapter.\n\
PLUG...........the wrap plug (Part Number 00G2380) into the twisted\n\
               pair connector on the Ethernet adapter."

$
JUMPER_SETTING_BNC "887140 \
JUMPER SETTING FOR THE ETHERNET RISERCARD\n\
The Diagnostics has detected that the jumper on the Ethernet\n\
risercard is set to :\n\n\
   - BNC (Coaxial connector)"

$
JUMPER_SETTING_DIX "887140 \
JUMPER SETTING FOR THE ETHERNET RISERCARD\n\
The Diagnostics has detected that the jumper on the Ethernet\n\
risercard is set to :\n\n\
   - DIX (Multipin receptor)"

STIL_UNPLUG_TRAN_T "\
UNPLUG.........the  transceiver from the network.\n\
               NOTE:  DO NOT UNPLUG the cable between the Ethernet adapter\n\
               and the transceiver.\n\
MOVE...........the link test switch on the transceiver to the OFF position.\n\
REMOVE.........any wrap plug from the transceiver if attached."


LPP_DEVICE_CANNOT_UNCONFIGURED "852300 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s         Ethernet High-Performance Adapter"


LPP_DEVICE_CANNOT_SET_TO_DIAGNOSE "852301 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE SET TO DIAGNOSE STATE\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be set to DIAGNOSE\n\
state.\n\n\
The resource is\n\
- %1$s             %2$s         Ethernet High-Performance Adapter"


LPP_DEVICE_CANNOT_SET_TO_DEFINE "852302 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE SET TO DIAGNOSE STATE\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be set from DIAGNOSE\n\
state to DEFINE state.\n\n\
The resource is\n\
- %1$s             %2$s         Ethernet High-Performance Adapter"

DIAG_DEVICE_CANNOT_CONFIGURED "852303 \
THE DIAGNOSTICS DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s             %2$s         Ethernet High-Performance Adapter"


DEVICE_DOES_NOT_EXIST "852304 \
THE SYSTEM INDICATES THAT THE DEVICE DOES NOT EXIST. \n\
No trouble was found.  However, the system indicated that \n\
the resource does not exist.\n\n\
The resource is\n\
- %1$s             %2$s         Ethernet High-Performance Adapter"

DIAG_DEVICE_CANNOT_UNCONFIGURED "852305 \
THE DIAGNOSTICS DEVICE CANNOT BE UNCONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s         Ethernet High-Performance Adapter"


LPP_DEVICE_CANNOT_CONFIGURED "852306 \
THE LPP(LICENSED PRODUCT PROGRAM) DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s             %2$s         Ethernet High-Performance Adapter"




$
$set 2
$quote "
POS_FAILED "POS Register Failed."
$ POS stands for Programmable Option Select.

IO_REG_FAILED "I/O Registers Test Failed."
$ I/O stands for Input/Output.

RAM_FAILED "Local RAM Test Failed."

VPD_FAILED "Vital Product Data Test Failed."

LAN_82596_FAILED "LAN Coprocessor 82596 Internal Test Failed."

INTERNAL_LB_FAILED "Internal Loopback Test Failed."

THICK_THIN_RISER_CARD "Thick And Thin Riser Card."

EXT_LB_GENERIC_FAILED "External Loopback Test Failed."
EXT_LB_FAILED "External Loopback (DIX Connector) Test Failed."

TWISTED_RISERCARD "Twisted Pair Riser Card."

EXT_LB_BNC_FAILED "External Loopback (BNC Connector) Test Failed."

EXT_LB_PARITY_FAILED "External Loopback Parity Test Failed."

EXT_LB_FAIRNESS_FAILED "External Loopback Fairness Test Failed."

EXT_LB_PARITY_FAIRNESS_FAILE "\
External Loopback Parity And Fairness Test Failed."

EXT_LB_TP_FAILED "External Loopback (Twisted Pair Connector) Test Failed."

EXT_LB_TP_PARITY_FAILED "\
External Loopback (Twisted Pair Connector) Parity Test Failed."

EXT_LB_TP_FAIRNESS_FAILED "\
External Loopback (Twisted Pair Connector) Fairness Test Failed ."

EXT_LB_TP_PARITY_FAIRNESS_FAILED "\
External Loopback (Twisted Pair Connector) Parity And Fairness Test Failed."

TWISTED_WRAP_FAILED "Twisted Pair Wrap Data Failed."

DEVICE_CONFIG_FAILED "Device Configuration Failed."

SOFTWARE_FAILED "Software Error."

HARD_PROBLEM "Device Driver Indicates A Hardware Problem."

RISERCARD_DETERMINATION "Riser Card Determination Failed."

XCEIVER_TP_10BASE_T "10BASE-T Transceiver Failed."

XCEIVER_10BASE_T "10BASE-T Transceiver"

XCEIVER_10BASE_2 "10BASE-2 Transceiver"

ETHERNET_CONVERTER "RJ-45 Ethernet Converter"

CONVERTER_TEST_FAILED "RJ-45 Ethernet Converter Test Failed"

XCEIVER_10BASE_2_FAILED "10BASE-2 Transceiver Test Failed."

XCEIVER_TWISTED_FAILED "10BASE-T Transceiver Test Failed."

INT_LB_TP_FAILED "Internal Loopback Test (Twisted Pair ) Failed."

ERROR_LOG_FAILED "Error Log Analysis Indicates A Hardware Problem."

FUSE_TEST_FAILED "Fuse Test Failed"

ETHERNET_BAD_FUSE "Bad Fuse"

CIRCUIT_BREAKER_TEST_FAILED "Ethernet Circuit Braker Test Failed"

ETHERNET_NETWORK "Ethernet Network"

RERUN_DIAGNOSTICS "Please Rerun Diagnostics in Advanced Mode and \n\
NOT LOOPMODE for Accurate Problem Determination"
