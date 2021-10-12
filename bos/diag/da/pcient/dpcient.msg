$ @(#)45        1.4  src/bos/diag/da/pcient/dpcient.msg, dapcient, bos41J, 9523B_all 6/6/95 17:24:44
$
$   COMPONENT_NAME: dapcient
$
$   FUNCTIONS: None 
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1995
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

 
$ =============================================================================
$set MSG_MAIN_SET
$quote "

$ -----------------------------------------------------------------------------
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

$ -----------------------------------------------------------------------------
ETHERNET_DEVICE "ETHERNET ADAPTER"

ADVANCED_NO_STANDBY "\
TESTING ETHERNET ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE"
$
$ %1$s is the device that is tested.
$ %2$s is the location code of the device as described in the general notes
$ Leave line spacing as shown.  See general notes on length of title line.

WHAT_TYPE_ADAPTER "\
Which type of Ethernet adapter is to be tested?\n\n\
Move cursor to selection, then press Enter."
$

T2_OPTION "\
T2 Adapter\n\
   This adapter supports BNC (Coax) and RJ-45 (Twisted Pair) connectors."
$ BNC, Coax, RJ-45, Twisted Pair are technical terms.  See general notes.

TP_OPTION "\
TP Adapter\n\
   This adapter supports DIX (Multipin receptor) and RJ-45 (Twisted Pair)\n\
   connectors."
$ DIX, Coax, RJ-45, Twisted Pair are technical terms.  See general notes.

ADAPTER_UNKNOWN_OPTION "\
Unknown Adapter\n\
   This selection exercises the adapter's internal tests. Wrap tests will\n\
   not be performed."

TP_WHAT_TYPE_CONNECTOR "\
Which connector is being used on the Ethernet adapter?\n\n\
Move cursor to selection, then press Enter."
$

T2_WHAT_TYPE_CONNECTOR "\
Which connector is being used on the Ethernet adapter?\n\n\
Move cursor to selection, then press Enter."
$

RJ_45_OPTION "RJ-45 (Twisted pair)"
$
$ This option is used to identify the type of connection being used.
$ RJ-45 is a technical term.  See general notes.

DIX_OPTION "DIX (Multipin receptor)"
$
$ This option is used to identify the type of connection being used.
$ DIX is a technical term.  See general notes.

BNC_OPTION "BNC (Coaxial)"
$
$ This option is used to identify the type of connection being used.
$ BNC is a technical term.  See general notes.

$ -----------------------------------------------------------------------------
HAVE_WRAP_PLUG_BNC "\
The following test requires a wrap plug with one of the following\n\
Part Numbers: %1$s.\n\n\
Do you have one of these wrap plugs?\n\n\
Move cursor to selection, then press Enter."
$ %1 contains a list of Part Numbers, ie. 71F1168, 70F9626, 02G7433

PLUG_BNC_CONNECTOR "\
UNPLUG.........any network cable(s), if attached, from the\n\
               connector on the Ethernet adapter.\n\
PLUG...........the wrap plug into the BNC (Coaxial) connector on the\n\
	       Ethernet adapter (use a wrap plug with one of the following\n\
               Part Numbers: %1$s)."
$ This message is displayed for plugging BNC wrap plug
$ %1 contains a list of Part Numbers, ie. 71F1168, 70F9626, 02G7433

UNPLUG_WRAP_GENERIC "\
UNPLUG.........the wrap plug from the connector on the Ethernet adapter.\n\
PLUG...........the network cable, if removed, into the connector on\n\
               the Ethernet adapter."
$

$ -----------------------------------------------------------------------------
HAVE_WRAP_PLUG_DIX "\
The following test requires a wrap plug with one of the following\n\
Part Numbers: %1$s.\n\n\
Do you have one of these wrap plugs?\n\n\
Move cursor to selection, then press Enter."
$ %1 contains a list of Part Numbers, ie. 71F1167, 70F9625

PLUG_DIX_CONNECTOR "\
UNPLUG.........any network cable(s), if attached, from the\n\
               connector on the Ethernet adapter.\n\
PLUG...........the wrap plug into the DIX (Multipin receptor) connector on\n\
	       the Ethernet adapter (use a wrap plug with one of the following\n\
	       Part Numbers: %1$s)."
$ %1 contains a list of Part Numbers, ie. 71F1167, 70F9625

$ -----------------------------------------------------------------------------
HAVE_WRAP_PLUG_TWISTED "\
The following test requires a wrap plug with one of the following\n\
Part Numbers: %1$s.\n\n\
Do you have this wrap plug?\n\n\
Move cursor to selection, then press Enter."
$ %1 contains a list of Part Numbers, ie. 00G2380

PLUG_TP_CONNECTOR "\
UNPLUG.........the network cable, if attached, from the connector on the\n\
               Ethernet adapter.\n\
PLUG...........the wrap plug into the RJ-45 (Twisted Pair) connector on the\n\
	       Ethernet adapter (use a wrap plug with one of the following\n\
	       Part Numbers: %1$s)."
$ %1 contains a list of Part Numbers, ie. 00G2380

$ -----------------------------------------------------------------------------
XCEIVER_EXIST "\
Is there a transceiver connected to the cable attached to the\n\
Ethernet adapter?\n\n\
Move cursor to selection, then press Enter."
$

TEST_XCEIVER "\
Do you want to test the Ethernet transceiver?\n\n\
Move cursor to selection, then press Enter."
$

WHAT_TYPE_XCEIVER "\
What type of Ethernet transceiver is connected to Ethernet adapter?\n\n\
Move cursor to selection, then press Enter."

BASET "10BASE-T (Part Number %1$s)"
BASE2 "10BASE-2 (Part Number %1$s)"
XCEIVER_UNKNOWN "Unkown Type (Transceiver testing will not be performed)"

$ -----------------------------------------------------------------------------
PLUG_XCEIVER_T "\
UNPLUG.........the transceiver from the network.\n\
               NOTE:  DO NOT UNPLUG the cable between the Ethernet adapter\n\
               and the transceiver.\n\
PLUG...........the wrap plug into the connector on the Ethernet transceiver\n\
               (use a wrap plug with one of the following Part Numbers:\n\
	       %1$s).\n\
ENSURE.........the link test switch on the transceiver is in the ON position."
$ %1$s is a string like "00G2380"

PLUG_T_XCEIVER_BACK "\
PLUG...........the network cable(s), if removed, into the connector on\n\
               the transceiver.\n\
ENSURE.........the link test switch on the transceiver is in the ON position.\n\
               NOTE: The link test switch must be left ON for normal\n\
               network operation.\n\
PLUG...........the transceiver, if removed, into the connector of the Ethernet\n\
               adapter."
$ 

ISOLATE_XCEIVER "\
The following test is used for problem isolation between the Ethernet\n\
adapter and the Ethernet transceiver.\n\
The test requires a wrap plug with one of the following Part Numbers:\n\
%1$s.\n\n\
Do you have one of these wrap plugs?\n\n\
Move cursor to selection, then press Enter."
$ %1$s is a string like "71F1167, 70F9625"

UNPLUG_XCEIVER_AND_PLUG_DIX "\
UNPLUG.........the transceiver from the connector on the Ethernet adapter.\n\
PLUG...........the wrap plug into the connector on the Ethernet adapter\n\
               (use a wrap plug with one of the following Part Numbers:\n\
	       %1$s)."

UNPLUG_DIX_AND_PUT_TP_XCEIVER_BACK "\
UNPLUG.........the wrap plug from the connector on the Ethernet adapter.\n\
PLUG...........the network cable(s), if removed, into the connector on\n\
               the transceiver.\n\
ENSURE.........the link test switch on the transceiver is in the ON position.\n\
               NOTE: The link test switch must be left ON for normal\n\
               network operation.\n\
PLUG...........the transceiver into the connector on the Ethernet adapter."
$ 

$ -----------------------------------------------------------------------------
PLUG_XCEIVER_10BASE_2 "\
UNPLUG.........the cable between the transceiver and the network.\n\
               NOTE:  if a T connector is used, unplug the T connector\n\
               from the transceiver. DO NOT UNPLUG any cable from the\n\
               T connector because this will bring the network down.\n\
ENSURE.........the cable between the transceiver and the Ethernet\n\
               adapter is attached.\n\
PLUG...........the wrap plug into the BNC (Coaxial) connector on the\n\
	       transceiver (use a wrap plug with one of the following\n\
	       Part Numbers: %1$s)."
$ %1$s is a string like "71F1168, 70F9626, 02G7433"

PLUG_GENERIC_XCEIVER_BACK "\
PLUG...........the network cable(s), if removed, into the connector on\n\
               the transceiver.\n\
PLUG...........the transceiver, if removed, into the connector of the Ethernet\n\
               adapter."
$

UNPLUG_DIX_AND_PUT_GENERIC_XCEIVER_BACK "\
UNPLUG.........the wrap plug from the connector on the Ethernet adapter.\n\
PLUG...........the network cable(s), if removed, into the connector on\n\
               the transceiver.\n\
PLUG...........the transceiver into the connector on the Ethernet adapter."
$

$ ===============================================================================
$set MSG_SRN_SET
$quote "

CONFIG_REG_FAILED "Configuration Registers Test Failed."

IO_REG_FAILED "I/O Registers Test Failed."
$ I/O stands for Input/Output.

INTERNAL_LB_FAILED "Internal Loopback Test Failed."

EXT_LB_GENERIC_FAILED "External Loopback Test Failed."

XCEIVER_10BASE_2 "10BASE-2 Transceiver"

XCEIVER_10BASE_2_FAILED "10BASE-2 Transceiver Test Failed."

XCEIVER_10BASE_T "10BASE-T Transceiver"
XCEIVER_10BASE_T_FAILED "10BASE-T Transceiver Test Failed."

DEVICE_CONFIG_FAILED "Device Configuration Failed."

ELA_FAILED "Error Log Analysis Indicates A Hardware Problem."

SOFTWARE_FAILED "Software Error."

$ ===============================================================================
$set MSG_MENUGOAL_SET
$quote "

ELA_WARNING "%1$06.6lx \
ADDITIONAL INFORMATION FOR (%2$s) IN LOCATION %3$s\n\n\
A device error log entry has been detected. The error log entry does not\n\
imply a failure of the adapter. Look at the error log entries in the\n\
error log for more information."
$ 1 = Screen number
$ 2 = Device name (ent0)
$ 3 = Device location (00-00-00)

TU_CLOSE_FAILED " "
$ This is an empty message used as a place holder
