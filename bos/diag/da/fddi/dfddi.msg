$ @(#)55	1.5.3.1  src/bos/diag/da/fddi/dfddi.msg, dafddi, bos411, 9434A411a 8/19/94 11:42:39
$ $ *
$ $ * COMPONENT_NAME: DAFDDI 
$ $ *
$ $ * FUNCTIONS: None ( Message File for Fiber Distributed Data Interface) 
$ $ *
$ $ * ORIGINS: 27 
$ $ *
$ $ * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ $ * combined with the aggregated modules for this product)
$ $ *                  SOURCE MATERIALS
$ $ * (C) COPYRIGHT International Business Machines Corp. 1991
$ $ * All Rights Reserved
$ $ *
$ $ * US Government Users Restricted Rights - Use, duplication or
$ $ * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ $ *
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

 
$set 1
$quote "

TESTING_REGULAR "\
TESTING FIBER DISTRIBUTED DATA INTERFACE (FDDI) ADAPTER %1$s\n\
IN LOCATION %2$s\n\n\
Please stand by."
$
$ This message is displayed when the FDDI  Diagnostic
$ Application begins execution.
$ %1$s is the adapter that is tested.
$ %2$s is the location code of the device as described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title line.


TESTING_LOOPMODE "\
TESTING FIBER DISTRIBUTED DATA INTERFACE (FDDI) ADAPTER %1$s\n\
IN LOCATION %2$s IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%3$6d\tpasses completed.\n\
%4$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ This message is displayed while testing is performed in 
$ loop mode.
$ %1$s is the adapter that is tested.
$ %2$s is the location code of the device as described in the general notes
$ %3$6d\t sets the number of diagnostics test passes completed.
$ %4$6d\t sets the number of errors logged during the tests. 
$ Leave line spacing as shown.  See general notes on length of title line.

TESTING_ADVANCED_MODE "\
TESTING FIBER DISTRIBUTED DATA INTERFACE (FDDI) ADAPTER %1$s\n\
IN LOCATION %2$s IN ADVANCED MODE\n\n\
Please stand by."
$
$ This message is displayed while testing is performed in 
$ advanced mode.
$ %1$s is the adapter that is tested.
$ %2$s is the location code of the device as described in the general notes
$ Leave line spacing as shown.  See general notes on length of title line.

TESTING_ADVANCED_MODE_NO_STANDBY "\
TESTING FIBER DISTRIBUTED DATA INTERFACE (FDDI) ADAPTER %1$s\n\
IN LOCATION %2$s IN ADVANCED MODE"

$
$ This message is displayed while testing is performed in 
$ advanced mode.
$ %1$s is the adapter that is tested.
$ %2$s is the location code of the device as described in the general notes
$ Leave line spacing as shown.  See general notes on length of title line.


FDDI_EXTENDER_ADAPTER_EXIST "\
Is the FDDI extender adapter installed in location %3$s?\n\n\
Move cursor to selection, then press Enter."
$
$ These line ask the user if he has a wrap plug part number 81F9016.
$ Display diagnostics menu title and ask the user for a wrap plug.
$ Leave line spacing as shown.  See general notes on length of title line.


FDDI_WRAP_PLUG_TITLE "\
The following test requires a wrap plug, Part Number 81F9016.\n\n\
Do you have this wrap plug?\n\n\
Move cursor to selection, then press Enter."
$
$ These line ask the user if he has a wrap plug part number 81F9016.
$ Display diagnostics menu title and ask the user for a wrap plug.
$ Leave line spacing as shown.  See general notes on length of title line.

FDDI_YES_OPTION "YES"
$ This option is shown when the user is asked to have a YES NO option.
$ Leave line spacing as shown.  See general notes on length of title line.

FDDI_NO_OPTION "NO"
$ This option is shown when the user is asked to have a YES NO option.
$ Leave line spacing as shown.  See general notes on length of title line.

FDDI_OPTION "Move cursor to selection, then press Enter "
$ Leave line spacing as shown.  See general notes on length of title line.

FDDI_FINISHED "\
When finished, press Enter."
$
$ Leave line spacing as shown.  See general notes on length of title line.

FDDI_PLUG_WRAP "\
UNPLUG.........network cable(s), if attached, from the connectors\n\
               of the FDDI adapter in location %1$s.\n\
PLUG...........the wrap plug (Part Number 81F9016) into the connectors\n\
               of the FDDI adapter in location %1$s."
$ 
$
$ This message is displayed while testing is performed in 
$ advanced mode.
$ %1$s is the adapter that is tested.
$ %2$s is the location code of the device as described in the general notes
$ Leave line spacing as shown.  See general notes on length of title line.

$ this message is replaced with the same name, further down
$
$ FDDI_PLUG_WRAP_EXTENDER "\
$ UNPLUG.........network cable(s), if attached, from the connectors\n\
$                of the FDDI extender adapter in location %1$s.\n\
$ PLUG...........the wrap plug (Part Number 81F9016) into the connectors\n\
$                of the FDDI extender adapter in location %1$s."

$ 
$
$ This message is displayed while testing is performed in 
$ advanced mode.
$ %1$s is the adapter that is tested.
$ %2$s is the location code of the parent device as described in the 
$ general notes
$ %3$s is the location code of the child device as described in the 
$ general notes
$ Leave line spacing as shown.  See general notes on length of title line.

FDDI_UNPLUG_WRAP "\
UNPLUG.........the wrap plug from the connectors on the FDDI adapter\n\
               in location %1$s.\n\
PLUG...........the network cable(s), if removed, into the connectors on\n\
               the FDDI adapter in location %1$s."
$
$ This message is displayed while testing is performed in 
$ advanced mode.
$ %1$s is the device that is tested.
$ %2$s is the location code of the device as described in the general notes
$ Leave line spacing as shown.  See general notes on length of title line.

$ This message is replace by further down

FDDI_DD_FILE_BAD "859010 \
FIBER DISTRIBUTED DATA INTERFACE (FDDI) DEVICE DRIVER FILE MISSING\n\
The FDDI device driver file is either missing or not accessible.\n\n\
This device driver is necessary in order to use the FDDI\n\
in normal system operation or to run Diagnostics."
$
$ This message is displayed while running diagnostics in console
$ Leave line spacing as shown.  See general notes on length of title line.
$

FDDI_OP_MICROCODE_FILE_BAD "859009 \
OPERATIONAL MICROCODE FILE MISSING\n\
The Fiber Distributed Data Interface (FDDI) operational microcode file\n\
is either missing or not accessible.\n\n\
This FDDI operational microcode  is necessary in order to use the \n\
FDDI in normal system operation or to run Diagnostics."
$
$ This message is displayed while running diagnostics in console
$ Leave line spacing as shown.  See general notes on length of title line.
$

FDDI_DIAG_MICROCODE_FILE_BAD "859010 \
DIAGNOSTICS MICROCODE FILE MISSING\n\
The Fiber Distributed Data Interface (FDDI) diagnostics microcode file\n\
is either missing or not accessible.\n\n\
This FDDI diagnostics microcode  is necessary in order to run Diagnostics."
$
$ This message is displayed while running diagnostics in console
$ Leave line spacing as shown.  See general notes on length of title line.
$



FDDI_UNPLUG_WRAP_EXTENDER "\
UNPLUG.........the wrap plug from the connectors on the FDDI Dual Ring\n\
               Upgrade Kit in location %1$s.\n\
PLUG...........the network cable(s), if removed, into the connectors on\n\
               the FDDI Dual Ring Upgrade Kit in location %1$s."
$
$
$ 

FDDI_UNPLUG_WRAP_ALL "\
UNPLUG.........the wrap plug from the connectors on the FDDI adapter\n\
               in location %1$s.\n\
UNPLUG.........the wrap plug from the connectors on the FDDI Dual \n\
               Ring Upgrade Kit in location %2$s.\n\
PLUG...........the network cable(s), if removed, into the connectors on\n\
               the FDDI adapter in location %1$s.\n\
PLUG...........the network cable(s), if removed, into the connectors on\n\
               the FDDI Dual Ring Upgrade Kit in location %2$s."

$
$ 

FDDI_PLUG_WRAP_EXTENDER "\
UNPLUG.........network cable(s), if attached, from the connectors\n\
               of the FDDI Dual Ring Upgrade Kit in location %1$s.\n\
PLUG...........the wrap plug (Part Number 81F9016) into the connectors\n\
               of the FDDI Dual Ring Upgrade Kit in location %1$s.\n\
               NOTE:  Do not unplug the wrap plug from the connectors on\n\
               the FDDI adapter in location %2$s."

$
$
FDDI_FOXCROFT_WRAP_PLUG_TITLE "\
The following test requires a wrap plug, Part Number 33G2759.\n\n\
Do you have this wrap plug?\n\n\
Move cursor to selection, then press Enter."
$
$

FDDI_FOXCROFT_PLUG_WRAP_EXTENDER "\
UNPLUG.........network cable(s), if attached, from the connectors\n\
               of the FDDI Dual Ring Upgrade Kit in location %1$s.\n\
PLUG...........the wrap plug (Part Number 33G2759) into the connectors\n\
               of the FDDI Dual Ring Upgrade Kit in location %1$s.\n\
               NOTE:  Do not unplug the wrap plug from the connectors on\n\
               the FDDI adapter in location %2$s."
$
$

FDDI_FOXCROFT_PLUG_WRAP "\
UNPLUG.........network cable(s), if attached, from the connectors\n\
               of the FDDI adapter in location %1$s.\n\
PLUG...........the wrap plug (Part Number 33G2759) into the connectors\n\
               of the FDDI adapter in location %1$s."
$
$ This message for putting wrap plug in both in main and extender
$

FDDI_FOXCROFT_PLUG_WRAP_ALL "\
UNPLUG.........network cable(s), if attached, from the connectors\n\
               of the FDDI adapter in location %1$s.\n\
UNPLUG.........network cable(s), if attached, from the connectors\n\
               of the FDDI Dual Ring Upgrade Kit in location %2$s.\n\
PLUG...........the wrap plug (Part Number 33G2759) into the connectors\n\
               of the FDDI adapter in location %1$s.\n\
PLUG...........the wrap plug (Part Number 33G2759) into the connectors\n\
               of the FDDI Upgrade Kit in location %2$s."
$
$
$
$

NEW_FOXCROFT_PLUG_WRAP_ALL "\
PLUG...........FDDI cable (Part Number 33G2761) into the connector\n\
               of the FDDI adapter in location %1$s. If it is not \n\
               already plugged in.\n\
PLUG...........Wrap plug (Part Number 33G2759) into the other end\n\
               of the FDDI cable that is plugged into\n\
               the adapter (location %1$s).\n\
PLUG...........FDDI cable (Part Number 33G2761) into the connector\n\
               of the FDDI adapter in location %2$s. If it is not \n\
               already plugged in.\n\
PLUG...........Wrap plug (Part Number 33G2759) into the other end\n\
               of the FDDI cable that is plugged into\n\
               the adapter (location %2$s)."

NEW_FOXCROFT_PLUG_WRAP "\
PLUG...........FDDI cable (Part Number 33G2761) into the connector\n\
               of the FDDI adapter in location %1$s. If it is not \n\
               already plugged in.\n\
PLUG...........Wrap plug (Part Number 33G2759) into the other end\n\
               of the FDDI cable that is plugged into\n\
               the adapter (location %1$s)."

NEW_FOXCROFT_WRAP_PLUG_TITLE "\
The following test requires a wrap plug (Part Number 33G2759)\n\
and a FDDI cable (Part Number 33G2761).\n\n\
Do you want to run this test?\n\n\
Move cursor to selection, then press Enter."

NEW_FOXCROFT_TWO_WRAP_PLUG_TITLE "\
The following test requires two wrap plugs (Part Number 33G2759)\n\
and two FDDI cables (Part Number 33G2761).\n\n\
Do you want to run this test?\n\n\
Move cursor to selection, then press Enter."

FOXCROFT_UNPLUG_WRAP_ALL "\
UNPLUG.........the wrap plug from the FDDI cable that is plugged\n\
               into  the FDDI adapter (location %1$s).\n\
UNPLUG.........the wrap plug from the FDDI cable that is plugged into\n\
               the FDDI Dual Ring Upgrade Kit (location %2$s).\n\
PLUG...........the network cable(s), if removed, back into the network." 

FDDI_PLUG_WRAP_ALL "\
UNPLUG.........network cable(s), if attached, from the connectors\n\
               of the FDDI adapter in location %1$s.\n\
PLUG...........the wrap plug (Part Number 81F9016) into the connectors\n\
               of the FDDI adapter in location %1$s.\n\
UNPLUG.........network cable(s), if attached, from the connectors\n\
               of the FDDI Dual Ring Upgrade Kit in location %2$s.\n\
PLUG...........the wrap plug (Part Number 81F9016) into the connectors\n\
               of the FDDI Dual Ring Upgrade Kit in location %2$s."

FDDI_FOXCROFT_UNPLUG_WRAP "\
UNPLUG.........the wrap plug from the FDDI cable that is plugged\n\
               into  the FDDI adapter (location %1$s).\n\
PLUG...........the network cable(s), if removed, back into the network." 

FDDI_TWO_WRAP_PLUG_TITLE "\
The following test requires two wrap plugs (Part Number 81F9016).\n\n\
Do you have these wrap plugs?\n\n\
Move cursor to selection, then press Enter."

TESTING_REGULAR_WITH_EXTENDER "\
TESTING FIBER DISTRIBUTED DATA INTERFACE (FDDI) ADAPTER %1$s\n\
IN LOCATION %2$s AND %3$s IN LOCATION %4$s \n\n\
Please stand by."
$
$ This message is displayed when the FDDI  Diagnostic
$ Application begins execution.
$ Leave line spacing as shown.  See general notes on length of title line.


TESTING_LOOPMODE_WITH_EXTENDER "\
TESTING FIBER DISTRIBUTED DATA INTERFACE (FDDI) ADAPTER %1$s\n\
IN LOCATION %2$s AND %3$s IN LOCATION %4$s IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%5$6d\tpasses completed.\n\
%6$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ This message is displayed while testing is performed in 
$ loop mode.
$ Leave line spacing as shown.  See general notes on length of title line.

TESTING_ADVANCED_MODE_WITH_EXTENDER "\
TESTING FIBER DISTRIBUTED DATA INTERFACE (FDDI) ADAPTER %1$s\n\
IN LOCATION %2$s AND %3$s IN LOCATION %4$s IN ADVANCED MODE\n\n\
Please stand by."
$
$ This message is displayed while testing is performed in 
$ advanced mode.
$ Leave line spacing as shown.  See general notes on length of title line.

TESTING_ADVANCED_MODE_NO_STANDBY_WITH_EXTENDER "\
TESTING FIBER DISTRIBUTED DATA INTERFACE (FDDI) ADAPTER %1$s\n\
IN LOCATION %2$s AND %3$s IN LOCATION %4$s IN ADVANCED MODE"

$
$ This message is displayed while testing is performed in 
$ advanced mode.
$ Leave line spacing as shown.  See general notes on length of title line.


LPP_DEVICE_CANNOT_UNCONFIGURED "859020 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE COULD NOT BE UNCONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s         Fiber Distributed Data Interface (FDDI) Adapter"


LPP_DEVICE_CANNOT_SET_TO_DIAGNOSE "859021 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE COULD NOT BE SET TO DIAGNOSE STATE\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be set to DIAGNOSE\n\
state.\n\n\
The resource is\n\
- %1$s             %2$s         Fiber Distributed Data Interface (FDDI) Adapter"


LPP_DEVICE_CANNOT_SET_TO_DEFINE "859022 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE COULD NOT BE SET TO DEFINE STATE\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be set from DIAGNOSE\n\
state to DEFINE state.\n\n\
The resource is\n\
- %1$s             %2$s         Fiber Distributed Data Interface (FDDI) Adapter"

DIAG_DEVICE_CANNOT_CONFIGURED "859023 \
THE DIAGNOSTICS DEVICE COULD NOT BE CONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s             %2$s         Fiber Distributed Data Interface (FDDI) Adapter"


DEVICE_DOES_NOT_EXIST "859024 \
THE SYSTEM INDICATES THAT THE DEVICE DOES NOT EXIST. \n\
No trouble was found.  However, the because the system indicated that \n\
the resource does not exist.\n\n\
The resource is\n\
- %1$s             %2$s         Fiber Distributed Data Interface (FDDI) Adapter"

DIAG_DEVICE_CANNOT_UNCONFIGURED "859025 \
THE DIAGNOSTICS DEVICE COULD NOT BE UNCONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s         Fiber Distributed Data Interface (FDDI) Adapter"


LPP_DEVICE_CANNOT_CONFIGURED "859026 \
THE LPP(LICENSED PRODUCT PROGRAM) DEVICE COULD NOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s             %2$s         Fiber Distributed Data Interface (FDDI) Adapter"

$set SRN 


$ FDDI stands for Fiber Distributed Data Interface
$ ---------------------------------------------------------------
$ Reason code set used by device type "Fiber Distributed Data Interface"

FDDI_MSG_101 "Placing Adapter In Diagnostics Mode Failure"
FDDI_MSG_102 "Placing Adapter In Normal Mode Failure"
FDDI_MSG_103 "Placing Adapter in On Line Diagnostics Mode Failure"
FDDI_MSG_104 "Releasing Adapter From On Line Diagnostics Mode Failure"
FDDI_MSG_106 "System Interface Registers Test Failuer"
FDDI_MSG_107 "Adapter Shared RAM Test Failure"
FDDI_MSG_108 "One Transfer Adapter Ram Buffer Failure"
FDDI_MSG_109 "Two Transfers Adapter Ram Buffer Failure"
FDDI_MSG_110 "Three Transfers Adapter Ram Buffer Failure"
FDDI_MSG_111 "One Transfer Adapter Node Processor Bus Data Store Failure"
FDDI_MSG_112 "Two Transfers Adapter Node Processor Bus Data Store Failure"
FDDI_MSG_113 "Three Transfers Adapter Node Processor Bus Data Store Failure"
FDDI_MSG_114 "One Transfer Adapter Node Processor Bus Program Store Failure"
FDDI_MSG_115 "Two Transfers Adapter Node Processor Bus Program Store Failure"
FDDI_MSG_116 "Three Transfers Adapter Node Processor Bus Program Store Failure"
FDDI_MSG_117 "Main Adapter Data Wrap Test Failure"
FDDI_MSG_118 "Download Operational Microcode Failure"
FDDI_MSG_121 "Download Diagnostics Microcode Failure"
FDDI_MSG_122 "Node Processor Instruction Test Failure"
FDDI_MSG_123 "Interface Support Array Chip Test Failure"
FDDI_MSG_124 "Vital Product Data CRC Test Failure"
FDDI_MSG_125 "Node Processor Data Memory Test Failure "
FDDI_MSG_126 "Supplemental Logic Interface Test Failure"
FDDI_MSG_127 "Fiber Distributed Data Interface Chip Set Test Failure"
FDDI_MSG_128 "Data Path Between Interface Support Array and RAM Buffer Failure"
FDDI_MSG_129 "Class B Data Path Failure"
FDDI_MSG_130 "Class A Data Path Failure"
FDDI_MSG_131 "Operational Microcode CRC Test Failure"
FDDI_MSG_132 "Extender Vital Product Data CRC Test Failure"
FDDI_MSG_133 "Extender Adapter Data Wrap Test Failure"
FDDI_MSG_134 " "
FDDI_MSG_135 " "
FDDI_EXTENDER_CARD "Extender Adapter"
FDDI_CROSSOVER_CABLE "Crossover Cable"
FDDI_SOFTWARE_ERROR "Software"
FDDI_CONFIG_FAIL "Device Configuration Failure"
FDDI_OPEN_FAIL "Device Driver Indicates A Hardware Failure"
FDDI_ERROR_LOG "Error Log Indicates A Hardware Failure"
FDDI_MSG_999 "CPU Planar"

