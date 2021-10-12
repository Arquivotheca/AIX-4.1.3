$ @(#)33        1.21.9.6  src/bos/usr/ccs/lib/libdiag/dcda.msg, libdiag, bos41J, 9519A_all 4/20/95 14:38:05
$
$   COMPONENT_NAME: LIBDIAG
$
$   FUNCTIONS: Diagnostic message file.
$
$
$   ORIGINS: 27, 83
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1995
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$   LEVEL 1, 5 Years Bull Confidential Information
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
$ These comments concern the techincal terms used in the messages.
$    There are many technical terms and acronyms used in this file.
$    DO NOT TRANSLATE these unless an equivalent term exists in
$    the translating language.  Verify that all translated terms
$    are correct and used properly.
$
$ For official product names, be sure to check with the official publications
$    in the translating languages for the official product name as
$    translated.  All official product names used in the diagnoatics programs
$    should match the offical product names as printed in the publications.
$
$       This file contains the corresponding messages for the menus and
$       errors generated from the Diagnostic Controller.
$
$ Be sure to chack that all part numbers are correct for the items in
$ in the translating countries.
$
$ END OF GENERAL NOTES


$quote "
$set DC_MSG_SET_1
$ ---------------------------------------------------------------
$ REASON CODE TEXT MESSAGES
$ Reason code set 1 used by Diagnostic Controller
$ $ The following messages are for MORPS

DC_NEW_MSG  "System did not detect all of the new resources."
DC_MIS_DEV  "A resource was not detected that was previously installed."
DC_SOFT_MSG  "\
A software error was encountered while running the Diagnostic Application."
DC_PROCESSING_MSG "Processing data ..."

P_SUPPLY "Device power supply"
FANS "Drawer cooling fans"
P_SUPPLY_FAILURE "Power supply malfunction"
DEV_ADAP_FAILURE "The resource does not respond."
HARR1_FAN_FAILURE "Drawer fan malfunction"
DRIVE "External Diskette Drive"
MEDIA_DRW_FAN_FAILURE "Media Drawer fan malfunction"
HDA "Drive head disk assembly"
DE "857 MB Drive Electronics Card"
RACK_PSUPPLY "Rack power supply"
DRW_PSUPPLY "Drawer power supply"
HARR2_SRN_RMSG "Use the 9333 Installation and Service Guide."
HARR2_PS_RMSG "Use the Rack Service Procedures."
DEC "Drive Electronics Card"

fMsgFanAssembly "Fan assembly"
fMsgAcFanAssembly "AC fan assembly"
fMsgDcFanAssembly "DC fan assembly"
fMsgScsiBus "SCSI bus"
fMsgExpPowerSupply "Expansion unit power supply"
fMsgRelay "Relay unit"

$set DSYS_MSG_SET_2
$ ---------------------------------------------------------------
$ Reason code set used by device type "Processor"

R_IO "An error was found with the I/O Planar during IPL."
$ I/O stands for Input/Output.  IPL stands for Initial Program Load.

R_NIO "An error was found with the Standard I/O Planar during IPL."
$ I/O stands for Input/Output.  IPL stands for Initial Program Load.

R_EPOW  "Power Status register indicates the following problem:"
R_MCHK  "Service Request Number is based on machine check log out data."

R_ECHK  "Service Request Number is based on external check log out data."


F_SOFT  "software error"
F_UNMEM "memory - size and location not known"
F_HIGH  "thermal high limit"
F_INT   "internal power supply failure"
F_OVER  "power supply overload"
F_PRIM  "loss of primary power"
F_FAN1  "power supply fan"
F_FAN2  "memory fan"
F_FAN3  "I/O fan"
F_MEM   "memory card"
F_MSIMMS "multiple(8) simms"
F_BLANK  "   "
R_CHECKSTOP "Service Request Number is based on error log analysis"
R_18 "One memory card and eight simms were found to be defective."
R_216 "Two memory cards and sixteen simms were found to be defective."
R_BLANK "   "
R_TCW "Translation control word simm test failed during IPL"
F_TCW "Translation control word simm"
R_L2 "Error Log Analysis detected machine check"

R_IOD "An error was found with the IOD during IPL."
$ IOD stands for Input/Output Daughter Board.
$ IPL stands for Initial Program Load.
R_CPU "CPU Module test failed during IPL."
$ CPU stands for Central Processor Unit
$ IPL stands for Initial Program Load.
R_CPUT "Interprocessor test unit failed."
F_MSIMM4 "multiple(4) simms"
$ simm stands for Single Inline Memory Module

$ I/O stands for Input/Output.

$set DFPA_MSG_SET_3
$ ---------------------------------------------------------------
$ Reason code set used by device type "Floating Point"

R_MSG_01 "A floating point instruction gives a result that is not correct."

F_FPP "Floating Point Processor"

$set DMEMORY_MSG_SET_4
$ ---------------------------------------------------------------
$ Reason code set used by device type "Memory"

R_SIMM_ERROR "A memory simm was found to be defective during IPL."
$ IPL stands for Initial Program Load.  Simm stands for Single In-line
$ Memory Module.

R_CARD_ERROR "A memory card was found to be defective during IPL."
$ IPL stands for Initial Program Load.

$ $ R_CONFIG_CARD "A memory card cannot be configured during IPL."
$ $ IPL stands for Initial Program Load.

F_SIMM_1   "1 MB SD1 Memory SIMM"
$ MB stands for megabyte and means 1024x1024 bytes.  SIMM stands for
$ Single Inline Memory Module.  This is the official name of the product.

F_SIMM_2   "2 MB SD1 Memory SIMM"
$ MB stands for megabyte and means 1024x1024 bytes.  SIMM stands for
$ Single Inline Memory Module.  This is the official name of the product.

F_SIMM_4   "4 MB HD2 Memory SIMM"
$ MB stands for megabyte and means 1024x1024 bytes.  SIMM stands for
$ Single Inline Memory Module.  This is the official name of the product.

F_CARD_8   "8 MB SD1 Memory Card"
$ MB stands for megabyte and means 1024x1024 bytes.

F_CARD_16 "16 MB SD1 Memory Card"
$ MB stands for megabyte and means 1024x1024 bytes.

F_CARD_32 "32 MB HD2 Memory Card"
$ MB stands for megabyte and means 1024x1024 bytes.

R_CARD2SIMMS "A memory card and 2 simms were found to be defective during IPL."
$ IPL stands for Initial Program Load.

R_CARD3SIMMS "A memory card and 3 simms were found to be defective during IPL."
$ IPL stands for Initial Program Load.

F_CARD_32A   "32 or 64 MB, S1 or S3 Memory Card"
$ MB stands for megabyte and means 1024x1024 bytes.

F_CARD_64   "32 or 64 MB, S1 or S3 Memory Card"
$ MB stands for megabyte and means 1024x1024 bytes.

F_SIMM_4S15   "4 MB S1.5 Memory SIMM"
$ MB stands for megabyte and means 1024x1024 bytes.  SIMM stands for
$ Single Inline Memory Module.  This is the official name of the product.

F_SIMM_4S3   "4 MB S3 Memory SIMM"
$ MB stands for megabyte and means 1024x1024 bytes.  SIMM stands for
$ Single Inline Memory Module.  This is the official name of the product.

F_SIMM_8S15   "8 MB S1.5 Memory SIMM"
$ MB stands for megabyte and means 1024x1024 bytes.  SIMM stands for
$ Single Inline Memory Module.  This is the official name of the product.

F_SIMM_8S3   "8 MB S3 Memory SIMM"
$ MB stands for megabyte and means 1024x1024 bytes.  SIMM stands for
$ Single Inline Memory Module.  This is the official name of the product.

F_CARD_32S25   "32 MB S2.5 Memory Card"
$ MB stands for megabyte and means 1024x1024 bytes.

F_SIMM_4S25   "4 MB S2.5 Memory SIMM"
$ MB stands for megabyte and means 1024x1024 bytes.  SIMM stands for
$ Single Inline Memory Module.  This is the official name of the product.

F_SIMM_1S3   "1 MB S3 Memory SIMM"
$ MB stands for megabyte and means 1024x1024 bytes.  SIMM stands for
$ Single Inline Memory Module.  This is the official name of the product.

F_SIMM_2S3   "2 MB S3 Memory SIMM"
$ MB stands for megabyte and means 1024x1024 bytes.  SIMM stands for
$ Single Inline Memory Module.  This is the official name of the product.

F_CARD_8S3   "8 MB S3 Memory Card"
$ MB stands for megabyte and means 1024x1024 bytes.

F_CARD_16S3   "16 MB S3 Memory Card"
$ MB stands for megabyte and means 1024x1024 bytes.

F_CARD_32S15   "32 MB S1.5 Memory Card"
$ MB stands for megabyte and means 1024x1024 bytes.

F_CARD_32S3   "32 MB S3 Memory Card"
$ MB stands for megabyte and means 1024x1024 bytes.

F_CARD_64S15   "64 MB S1.5 Memory Card"
$ MB stands for megabyte and means 1024x1024 bytes.

F_CARD_64S3   "64 MB S3 Memory Card"
$ MB stands for megabyte and means 1024x1024 bytes.

F_SIMM   "A Memory Simm"
$ MB stands for megabyte and means 1024x1024 bytes.  SIMM stands for
$ Single Inline Memory Module.  This is the official name of the product.

F_CARD   "Memory Card(s)"

FF_BLANK "   "

RR_BLANK "    "

RR_PLANAR "Memory tests indicate System Planar/CPU card failure"

R_L2CACHE "L2 cache tests fail"

F_L2CACHE "L2 cache"

F_RISER "MCA Riser card"

F_SIMM_MD2 "8 MB MD2 Memory SIMM"
$ MB stands for megabyte and means 1024x1024 bytes.  SIMM stands for
$ Single Inline Memory Module.  This is the official name of the product.

F_SIMM_MD4 "32 MB MD4 Memory SIMM"
$ MB stands for megabyte and means 1024x1024 bytes.  SIMM stands for
$ Single Inline Memory Module.  This is the official name of the product.

F_SIMM_8MB "8 MB Memory SIMM"
$ MB stands for megabyte and means 1024x1024 bytes.  SIMM stands for
$ Single Inline Memory Module.  This is the official name of the product.

F_SIMM_16MB "16 MB Memory SIMM"
$ MB stands for megabyte and means 1024x1024 bytes.  SIMM stands for
$ Single Inline Memory Module.  This is the official name of the product.

F_SIMM_32MB "32 MB Memory SIMM"
$ MB stands for megabyte and means 1024x1024 bytes.  SIMM stands for
$ Single Inline Memory Module.  This is the official name of the product.

$set DPP_MSG_SET_5
$ ---------------------------------------------------------------
$ Reason code set used by device type "SIO Parallel"
$ $ Do not add any messages above DC_SPP.
$ $ Do not add any messages above SPP_4.

DC_SPP  "Standard Parallel Port P"
SPP_2   "Standard Parallel Port P register test failed."
SPP_3   "Standard Parallel Port P wrap test failed."
SPP_4   "Standard Parallel Port P interrupt test failed."

$set DASYNC_NIO_MSG_SET_6
$ ---------------------------------------------------------------
$ Reason code set used by device type "SIO Serial"
$ $ Do not add any messages before or between existing messages.
$ $ Blank messages are place holders.

DC_SP   "Serial Port"
SSP_2           "Message number is not valid."
SSP_ACPN_3      "Asynchronus Cable EIA-232, Part Number 6323741"
SSP_CIPN_4      "Printer/Terminal interposer, Part Number 58F2861"
$ An interposer is a small connector that fits between a cable and a device,
$ converting a modem cable to a cable that will interface with some other
$ serial device.

SSP_5           ""
SSP_6           ""
SSP_7           "10-pin to 25-pin cable, Part Number 59F3740"
SSP_8           ""
SSP_9           ""
SSP_10          "Standard Serial Port register test failed."
SSP_20          "Standard Serial Port local chip wrap test failed."
SSP_30          "Standard Serial Port adapter driver wrap test failed."
SSP_40          ""
SSP_50          ""
SSP_60          "Standard Serial Port 10-pin to 25-pin cable wrap test failed."
SSP_70          "Standard Serial Port asynchronous cable wrap test failed."
SSP_80          "Standard Serial Port interposer wrap test failed."
SSP_LM          "Standard Serial Port adapter test failed."
SSP_7C          "Standard Serial Port converter cable, Part Number 31F4590"
SSP_8C          "10-pin to 25-pin cable, Part Number 59F4533"
SSP_60C         "Standard Serial Port converter cable wrap test failed."
SSP_9C          "Standard Serial Port converter cable, Part Number 31F4126"
SSP_10C         "Standard Serial Port converter cable, Part Number 6450242"
SSP_PN24        ""
SSP_PTCPN_25    "Printer/Terminal Cable, Part Number 12H1204"

$set DASYNC_8_MSG_SET_7
$ ---------------------------------------------------------------
$ Reason code set used by device type "Rs232 8 port"
$ $ Do not add any messages before or between existing messages.
$ $ Blank messages are place holders.

DC_232_8        "8-Port Async Adapter EIA-232"
$ Async stands for asynchronous communications.
DM8_ICPN_2      "Multiport Interface Cable, Part Number 00F5524"
DM8_ACPN_3      "Asynchronous Cable EIA-232, Part Number 6323741"
DM8_CIPN_4      "Printer/Terminal interposer, Part Number 58F2861"
DM8_5           ""
DM8_ECPN_6      ""
$ ECPN_6 moved to CPN24 for name change.
DM8_7           ""
DM8_8           ""
DM8_9           ""
DM8_TU10        "8-port Async adapter EIA-232 register test failed."
$ Async stands for asynchronous communications.
DM8_TU20        "8-port Async adapter EIA-232 local chip wrap test failed."
$ Async stands for asynchronous communications.
DM8_TU30        "8-port Async adapter EIA-232 driver wrap test failed."
$ Async stands for asynchronous communications.
DM8_TU40        "8-port Async adapter extension cable wrap test failed."
$ Async stands for asynchronous communications.
DM8_TU50        "Multiport interface cable wrap test failed."
DM8_TU60        ""
DM8_TU70        "Asynchronous cable wrap test failed."
DM8_TU80        "Interposer wrap test failed."
DM8_LM          "8-port EIA-232 adapter test failed."
DM8_PN19        ""
DM8_PN20        ""
DM8_TU60C       ""
DM8_PN22        ""
DM8_PN23        ""
DM8_CPN24       "8/16-Port Rack Extension Cable, Part Number 53F3048"
DM8_PTCPN_25    "Printer/Terminal Cable, Part Number 12H1204"

$set DASYNC_16_MSG_SET_8
$ ---------------------------------------------------------------
$ Reason code set used by device type "Rs232 16 port"
$ $ Do not add any messages before or between existing messages.
$ $ Blank messages are place holders.

DC_232_16       "16-Port Async Adapter EIA-232"
$ Async stands for asynchronous communications.
DM14_ICPN_2     "16-Port Interface Cable EIA-232, Part Number 53F3311"
DM14_ACPN_3     "Asynchronous Cable EIA-232, Part Number 6323741"
DM14_CIPN_4     "Printer/Terminal interposer, Part Number 58F2861"
DM14_5          ""
DM14_ECPN_6     ""
$ ECPN_6 moved to CPN24 for name change.
DM14_7          ""
DM14_8          ""
DM14_9          ""
DM14_TU10       "16-port Async adapter EIA-232 register test failed."
$ Async stands for asynchronous communications.
DM14_TU20       "16-port Async adapter EIA-232 local chip wrap test failed."
$ Async stands for asynchronous communications.
DM14_TU30       "16-port Async adapter EIA-232 driver wrap test failed."
$ Async stands for asynchronous communications.
DM14_TU40       "16-port Async adapter extension cable wrap test failed."
$ Async stands for asynchronous communications.
DM14_TU50       "16-port interface cable EIA-232 wrap test failed."
DM14_TU60       ""
DM14_TU70       "Asynchronous cable wrap test failed."
DM14_TU80       "Interposer wrap test failed."
DM14_LM         "16-port EIA-232 adapter test failed."
DM14_PN19       ""
DM14_PN20       ""
DM14_TU60C      ""
DM14_PN22       ""
DM14_PN23       ""
DM14_CPN24      "8/16-Port Rack Extension Cable, Part Number 53F3048"
DM14_PTCPN_25   "Printer/Terminal Cable, Part Number 12H1204"

$set DASYNC_64_MSG_SET_9
$ ---------------------------------------------------------------
$ Reason code set used by device type "Rs232 64 port"
$ $ Do not add any messages before or between existing messages.
$ $ Blank messages are place holders

DC_232_64       "64-Port Async Controller Port"
$ Async stands for asynchronous communications.
DM15_CBPN_2     "16-Port Async Concentrator, Part Number 53F3371"
$ Async stands for asynchronous communications.
DM15_ACPN_3     "Asynchronous Cable EIA-232, Part Number 6323741"
$ Async stands for asynchronous communications.
DM15_CIPN_4     "Printer/Terminal interposer, Part Number 58F2861"
DM15_5          ""
DM15_CCPN_6     "64-Port Controller Cable, Part Number 53F3368"
DM15_CCPN_7     "RJ-45 to DB-25 converter cable, Part Number 53F3367"
DM15_8          ""
DM15_9          ""
DM15_TU10       "64-port Async Controller VPD test failed."
$ Async stands for asynchronous communications.
$ VPD stands for Vital Product Data.
DM15_TU20       "64-port Async Controller memory test failed."
$ Async stands for asynchronous communications.
DM15_TU30       "64-port Async Controller wrap test failed."
$ Async stands for asynchronous communications.
DM15_TU40       "64-port Controller cable wrap test failed."
DM15_TU50       "16-Port Concentrator wrap test failed."
DM15_TU60       "RJ-45 to DB-25 converter cable wrap test failed."
DM15_TU70       "Asynchronous cable wrap test failed."
DM15_TU80       "Interposer wrap test failed."
DM15_LM         "64-port Async Controller test failed."
DM15_19         ""
DM15_20         ""
DM15_21         ""
DM15_22         ""
DM15_23         ""
DM15_24         ""
DM15_PTCPN_25   "Printer/Terminal Cable, Part Number 12H1204"
$ Async stands for asynchronous communications.

$set DENET_MSG_SET_10
$ ---------------------------------------------------------------
$ Reason code set used by device type "Ethernet"

ETH_RMSG_1 "The POS register test failed."
$ POS stands for Programmable Option Select.

ETH_RMSG_2 "The hard reset test failed."
ETH_RMSG_3 "The shared memory test failed."
ETH_RMSG_4 "The I/O registers test failed."
$ I/O stands for Input/Output.

ETH_RMSG_5 "The Vital Product Data test failed."
ETH_RMSG_6 "The internal loopback test failed."
ETH_RMSG_7 "The external loopback (DIX connector) test failed."
$ DIX is a standard type of connector.

ETH_RMSG_8 "The external loopback (BNC connector) test failed."
$ BNC is a standard type of connector.

ETH_RMSG_9 "Error log analysis found a problem with the adapter."
ETH_RMSG_10 "Configuration fails."
ETH_RMSG_11 "Software"
ETH_RMSG_12 "The device driver indicates a hardware failure."
$ new message
ETH_RMSG_13 "The Transceiver Test Failed"
$ new message
ETH_RMSG_14 "The Transceiver"

$set DMULTP_MSG_SET_11
$ ---------------------------------------------------------------
$ Reason code set used by device type "Multiprotocol Adapter"
$ $ Do not insert any messages above DC_MPQP_0.
$ General test return codes

DC_MPQP_0 "Multiprotocol Adapter Port"
MPQP_SOTWARE_HARDWARE_FAILURE "Adapter hardware has caused a software failure."
MPQP_ELA_SOFT  "Error log analysis indicates a software failure."
MPQP_ELA_DEVICE "Error log analysis indicates a device failure."
MPQP_ELA_ADAP   "Error log analysis indicates an adapter failure."

MPQP_SOFTWARE_INIT_ERROR "\
An adapter software initialization error occurred during the ROS POST test."
$ ROS stands fot Read Only Storage.  POST stands for Power On Self Test.

MPQP_EXTENDED_DRAM_SIP_TEST_FAILURE "Extended DRAM SIP test failed."
$ DRAM stands for Dynamic Random Access Memory.
$ SIP stands for Single In-line Package.

MPQP_ROS_CHECKSUM_TEST_FAILURE "ROS checksum test failed."
$ ROS stands for Read Only Storage.

MPQP_BUS_MASTER_DMA_TEST_FAILURE "Internal test failed."
MPQP_RETURN_MEM_SIZE_TEST_FAILURE "Memory size test failed."
MPQP_GET_INTERFACE_ID_TEST_FAILURE "Interface ID test failed."
$ ID stands for Identification.

MPQP_GET_EIB_ID_TEST_FAILURE  "EIB ID test failed."
$ EIB stands for Electriacal Interface Board.  ID stands for Identification.

MPQP_GET_ROS_VERSION_TEST_FAILURE  "ROS version test failed."
$ ROS stands for Read Only Storage.

MPQP_DUSCC_REG_QUERY_TEST_FAILURE "DUSCC Register test failed."
$ DUSCC stands for Dual Universal Serial Communication.

MPQP_CIO_REG_QUERY_TEST_FAILURE "CIO register test failed."
$ CIO stands for Counter/Timer Input/Output.

MPQP_DMA_REG_QUERY_TEST_FAILURE "DMA tegister test failed."
$ DMA stands for Direct Memory Access.

MPQP_X21_PAL_TEST_FAILURE "X.21 PAL test failed."
$ PAL stands for Programmable Array Logic.

MPQP_PORT_0_0_WRAP_TEST_FAILURE "Port 0 - V.35 external wrap test failed."
MPQP_PORT_0_1_WRAP_TEST_FAILURE "Port 0 - 232D external wrap test failed."
MPQP_PORT_0_2_WRAP_TEST_FAILURE "Port 0 - X.21 external wrap test failed."
MPQP_PORT_0_3_WRAP_TEST_FAILURE "Port 0 - 422A external wrap test failed."
MPQP_PORT_1_4_WRAP_TEST_FAILURE "Port 1 - 232D external wrap test failed."
MPQP_PORT_1_5_WRAP_TEST_FAILURE "Port 1 - V.35 external wrap test failed."
MPQP_PORT_3_6_WRAP_TEST_FAILURE "Port 3 - 232D external wrap test failed."
MPQP_PORT_2_7_WRAP_TEST_FAILURE "Port 2 - 232D external wrap test failed."
MPQP_PORT_2_8_WRAP_TEST_FAILURE "Port 2 - 422A external wrap test failed."
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
MPQP_RS_232_CABLE_FAILURE "RS-232 cable wrap test failed."
MPQP_RS_422_CABLE_FAILURE "RS_422 cable wrap test failed."
MPQP_V_35_CABLE_FAILURE "V.35 cable wrap test failed."
MPQP_X_21_CABLE_FAILURE "X.21 cable wrap test failed."

MPQP_DRAM "Multiprotocol Base Card"
MPQP_EIB  "Multiprotocol Interface Card"
MPQP_FOB  "interface cable"
MPQP_CABL "cable"

MPQP_78PIN_CABLE "4-Port jumper cable wrap test failed."
MPQP_CONFIGURATION_FAIL "Configurations fails."
MPQP_OPEN_DEVICE_ERROR "The device driver indicates a hardware failure."
MPQP_SOFTWARE_ERROR "Software"

$ $ Do not insert any messages above PM6_V35.
$ $ Do not add any messages between PM_ERRMSGS and PM_01.

PM6_V35         "6-Port Portmaster Adapter/A V.35"
PM6_X21         "6-Port Portmaster Adapter/A X.21"
PM8_232         "8-Port Portmaster Adapter/A RS-232"
PM8_422         "8-Port Portmaster Adapter/A RS-422"
PM_EIBS         "reserved"
$ PM_EIBS is the message number all FRU bucket fmsg's are referenced to.

PM6_V35EIB      "V.35 Electronic Interface Board, Part Number 72F0164."
PM6_X21EIB      "X.21 Electronic Interface Board, Part Number 04G5500."
PM8_232EIB      "RS-232 Electronic Interface Board, Part Number 53F2612."
PM8_422EIB      "RS-422 Electronic Interface Board, Part Number 53F2615."
PM_RES          "reserved"
PM_1MB2MB       "1MB/2MB Memory module, Part Number 53F2662/53F2666."
PM_1MBSIMM      "1MB Memory module, Part Number 53F2662."
PM_2MBSIMM      "2MB Memory module, Part Number 53F2666."
PM_6VCABLE      "Six-port V.35 Interface cable, Part Number 72F1966."
PM_6XCABLE      "Six-port X.21 Interface cable, Part Number 04G5501."
PM_8PCABLE      "Eight-port Interface cable, Part Number 53F2621."
PM_SWERR        "software error"
PM_ERRMSGS      "Reserved"
$ PM_ERRMSGS is the message number all FRU bucket rmsg's are referenced to.

PM_01           "Power on self test failed."
PM_02           "POST DRAM test failed."
PM_03           "reserved"
PM_04           "Advanced DRAM test failed."
PM_05           "reserved"
PM_06           "Base adapter test failed."
PM_07           "Electronic Interface Board/Base adapter test failed."
PM_08           "Eight port Interface Cable test failed."
PM_09           "Six port V.35 Interface Cable test failed."
PM_10           "Six port X.21 Interface Cable test failed."
PM_11           "Error opening device driver."
PM_12           "Error configuring adapter."
PM_13           "Error configuring device driver."

$set DTOKEN_MSG_SET_12
$ ---------------------------------------------------------------
$ Reason code set used by device type "Token Ring"

TOKNETMSG "Token-Ring Network"
CABLEMSG  "Token-Ring Cable"
TOK_RMSG_1 "The Token-Ring adapter open test failed at ring speed 4MB."
TOK_RMSG_2 "The Token-Ring wrap data test failed at ring speed 4MB."
TOK_RMSG_3 "Not able to set Token-Ring speed to 4 MB."
TOK_RMSG_4 "The Token-Ring adapter open test failed at ring speed 16MB."
TOK_RMSG_5 "The Token-Ring wrap data test failed at ring speed 16MB."
TOK_RMSG_6 "Not able to set Token-Ring speed to 16 MB."
TOK_RMSG_7 "An error occurred that could not be identified."
TOK_RMSG_8 "Error log analysis indicates hardware failure."
TOK_RMSG_9 "Tokenring configuration fails."
TOK_RMSG_10 "The device driver indicates a hardware failure."
TOK_RMSG_11 "Software."


$set DDCA_MSG_SET_13
$ ---------------------------------------------------------------
$ Reason code set used by device type "DCA"

DCANETMSG "3270 Network"
DCA_RMSG_1 "The 3270 Connection adapter register test failed."
DCA_RMSG_2 "The 3270 Connection adapter RAM test failed."
$ RAM stands for Random Access Memory.
DCA_RMSG_3 "The 3270 Connection adapter interrupt test failed."
DCA_RMSG_4 "The 3270 Connection adapter mode test failed."
DCA_RMSG_5 "The 3270 Connection adapter connection test failed."
DCA_RMSG_6 "Adapter hardware has caused a software failure."
DCA_RMSG_7 "Configuration fails."
DCA_RMSG_8 "Device driver indicates a hardware failure."
DCA_RMSG_9 "Software."

$set D6193A_MSG_SET_14
$ ---------------------------------------------------------------
$ Reason code set used by device type "6193 Adapter"

DC_EU   "Expansion adapter"
EU_2    "Asynchronous Expansion unit adapter interface cable"
EU_3    "Asynchronous Expansion unit planar"
EU_4    "Reserved"
EU_5    "Reserved"
EU_6    "Reserved"
EU_7    "Reserved"
EU_8    "Reserved"
EU_9    "Reserved"
EU_10   "Asynchronous Expansion unit adapter register test failed."
EU_20   "Asynchronous Expansion unit adapter VPD test failed."
$ VPD stands for Vital Product Data.
EU_30   "Asynchronous Expansion unit adapter internal wrap test failed."
EU_40   "Asynchronous Expansion unit adapter address verification test failed."
EU_50   "Asynchronous Expansion unit planar register test failed."
EU_60   "Asynchronous Expansion unit planar VPD test failed."
$ VPD stands for Vital Product Data.
EU_70   "Asynchronous Expansion unit planar internal wrap test failed."

$set DAMSLA_MSG_SET_15
$ ---------------------------------------------------------------
$ Reason code set used by device type "MSLA"

DMSLA_DA_MSG_100  "POS test:  the memory did not load correctly."
DMSLA_DA_MSG_101  "POS test:  the microcode did not start."
DMSLA_DA_MSG_102  "POS test failed."
DMSLA_DA_MSG_103  "POS test:  driver IOCTL failed."
$ IOCTL stands for I/O control.
DMSLA_DA_MSG_104  "POS test:  a level 2 microcode file open error occurred."
DMSLA_DA_MSG_105  "POS test:  a level 3 microcode file open error occurred."
$
DMSLA_DA_MSG_200  "Memory test failed (byte transfer)."
DMSLA_DA_MSG_201  "Memory test failed (word transfer)."
DMSLA_DA_MSG_202  "Memory test failed (longword transfer)."
DMSLA_DA_MSG_203  "There is not enough memory available now."
$
DMSLA_DA_MSG_300  "Register test failed."
DMSLA_DA_MSG_301  "\
Register test failed (CPU to MSLA interrupt not detected)."
$ MSLA stands for MicroChannel Serial Link Adapter.
DMSLA_DA_MSG_400  "Hardware test:  the microcode started, but never progressed."
DMSLA_DA_MSG_401  "68k instuction set test failed."
$ k stands for kilo and means 1024.
DMSLA_DA_MSG_402  "RAM test failed."
$ RAM stands for Random Access Memory.
DMSLA_DA_MSG_403  "Trap test failed."
DMSLA_DA_MSG_404  "Timer test failed."
DMSLA_DA_MSG_405  "MTOS interrupt failed."
$ MTOS stands for
DMSLA_DA_MSG_406  "Hardware test failed."
DMSLA_DA_MSG_407  "Hardware test:  the microcode did not load correctly."
DMSLA_DA_MSG_408  "Hardware test:  the microcode did not load or did not start."
DMSLA_DA_MSG_409  "Hardware test:  a microcode file open error occurred."
$
DMSLA_DA_MSG_600  "SDLC wrap test:  the memory did not load correctly."
$ SDLC stands for
DMSLA_DA_MSG_601  "SDLC wrap test:  the microcode did not start."
DMSLA_DA_MSG_602  "SDLC wrap test failed."
DMSLA_DA_MSG_603  "Modem wrap test:  the memory did not load correctly."
DMSLA_DA_MSG_604  "Modem wrap test:  the microcode did not start."
DMSLA_DA_MSG_605  "A modem wrap test error occurred."
DMSLA_DA_MSG_606  "SDLC wrap test:  a microcode file open error occurred."
DMSLA_DA_MSG_607  "Modem wrap test:  a microcode file open error occurred."
$
DMSLA_DA_MSG_700  "Interrupt test:  the microcode did not start."
DMSLA_DA_MSG_701  "Interrupt test failed."
DMSLA_DA_MSG_702  "Interrupt test:  the microcode did not finish."
DMSLA_DA_MSG_703  "Interrupt test:  driver IOCTL failed."
$ IOCTL stands for I/O control.
DMSLA_DA_MSG_704  "Interrupt test:  IOCTL interrupt count failed."
$ IOCTL stands for I/O control.
DMSLA_DA_MSG_705  "Interrupt test:  a microcode file open error occurred."
$
DMSLA_DA_MSG_800  "DMA test (CPU to MSLA):  the microcode did not start."
$ MSLA stands for MicroChannel Serial Link Adapter.
DMSLA_DA_MSG_801  "DMA test (CPU to MSLA):  the transfer failed."
$ MSLA stands for MicroChannel Serial Link Adapter.
DMSLA_DA_MSG_802  "DMA test (CPU to MSLA):  driver IOCTL failed"
$ MSLA stands for MicroChannel Serial Link Adapter.
$ IOCTL stands for I/O control.
DMSLA_DA_MSG_803  "\
DMA test (CPU to MSLA):  a microcode file open error occurred."
$ MSLA stands for MicroChannel Serial Link Adapter.
DMSLA_DA_MSG_804  "DMA test (MSLA TO CPU):  the microcode did not start."
$ MSLA stands for MicroChannel Serial Link Adapter.
DMSLA_DA_MSG_805  "DMA test (MSLA to CPU):  the transfer failed."
$ MSLA stands for MicroChannel Serial Link Adapter.
DMSLA_DA_MSG_806  "DMA test (MSLA to CPU):  driver IOCTL failed."
$ MSLA stands for MicroChannel Serial Link Adapter.
$ IOCTL stands for I/O control.
DMSLA_DA_MSG_807  "\
DMA test (MSLA to CPU):  a microcode file open error occurred."
$
DMSLA_DA_MSG_ELA1 "A hardware error was logged by the PSLA device driver."
$ PSLA stands for
DMSLA_DA_MSG_ELA2 "A hardware error was logged by the SSLA device driver."
$ SSLA stands for

$set DKBDA_MSG_SET_16
$ ---------------------------------------------------------------
$ Reason code set used by device type "NIO Keyboard Adapter"

DKBDA_FAILURES "Keyboard adapter failed."
DKBDA_WRAP_TEST "Keyboard adapter control logic failed."
DKBDA_UNEXPECTED "\
Results that were not expected were received when the test ran."
DKBDA_REG_TEST "Register test failed."
DKBDA_LOGIC_ERR "Keyboard adapter logic error."
DKBDA_XMIT_ERR "Keyboard adapter transmit error."
DKBDA_REC_ERR "Keyboard adapter receive error."
DKBDA_FUSE_ERR "Fuse test failed."
DKBDA_SPKR_FAILURES "Speaker controller error."
DKBDA_CONF "Device cannot be configured."
DKBDA_OPEN "Unable to open the device."
DKBDA_SW "Software"
DKBDA_FUSE "Fuse"
DKBDA_KBD "Keyboard"
DKBDA_WRAP_PLG "Tablet wrap plug not removed."
DKBDA_XWRAP "Tablet adapter wrap test failed."
DKBDA_RESET "Keyboard adapter reset failed."
DKBDA_TAB "Tablet"
DKBDA_A_K_CON "Keyboard cannot be configured."
DKBDA_A_K_UN "Keyboard cannot be unconfigured."
DKBDA_A_K_OP "Keyboard device cannot be opened."
DKBDA_A_T_CON "Tablet cannot be configured."
DKBDA_A_T_UN "Tablet cannot be unconfigured."
DKBDA_A_T_OP "Tablet device cannot be opened."
DKBDA_A_KTA "Keyboard/Tablet adapter"

$set DMOUSEA_MSG_SET_17
$ ---------------------------------------------------------------
$ Reason code set used by device type "NIO Mouse Adapter"

MSE_A_UNEXPECTED "\
Results that were not expected were received when the test ran."
MSE_A_REGISTER "Status register test failed."
MSE_A_IWRAP "Internal wrap test failed."
MSE_A_NONBLOCK_FAILED "Unable to put mouse adapter into non-block mode."
MSE_A_BLOCK_FAILED "Unable to put mouse adapter into block mode."
MSE_A_B_NONB "Mouse."
MSE_A_FUSE_ERR "Fuse test failed."
MSE_A_CONF "Device cannot be configured."
MSE_A_OPEN "Unable to open the device."
MSE_A_SW "Software"
MSE_A_FUSE "Fuse"
MSE_A_M_UN "Mouse cannot be unconfigured."
MSE_A_M_CON "Mouse cannot be configured."

$set DTABLETA_MSG_SET_18
$ ---------------------------------------------------------------
$ Reason code set used by device type "NIO Tablet Adapter"

TBL_A_UNEXPECTED "\
Results that were not expected were received when the test ran."
TBL_A_ENB_DISABLE "Enable device and disable device test failed."
TBL_A_IWRAP "Internal wrap test failed."
TBL_A_XWRAP "Wrap test failed."
TBL_A_RESET "Reset test failed."
TBL_A_FUSE_ERR "Fuse test failed."
TBL_A_CONF "Device cannot be configured."
TBL_A_OPEN "Unable to open the device."
TBL_A_SW "Software"
TBL_A_FUSE "Fuse"
TBL_A_WRAP_PLG "Tablet wrap plug not removed."
TBL_A_T_CON "Tablet cannot be configured."
TBL_A_T_UN "Tablet cannot be unconfigured."
TBL_A_T_OP "Tablet device cannot be opened."

$set DKBD_MSG_SET_19
$ ---------------------------------------------------------------
$ Reason code set used by device type "Keyboard"

DKBD_UA_ERR     "An adapter error occurred that was not expected."
DKBD_UAD_ERR    "An device or adapter error occurred that was not expected."
DKBD_RESET_ERR  "Keyboard reset failed."
DKBD_UKEYBOARD  "Keyboard is not known."
DKBD_KBID_ERR   "Read keyboard ID test failed."
$ ID stands for identification.
DKBD_LAYOUT_ERR "Keyboard layout ID test failed."
$ ID stands for identification.
DKBD_ECHO_ERR   "Keyboard echo test failed."
DKBD_SCAN_ERR   "Select scan code set test failed."
DKBD_LON_ERR    "Error in turning lamps on."
$ The lamps are the three LED's (Light Emitting Diodes) in the upper right-hand
$ corner of the keyboard.
DKBD_LOFF_ERR   "Error in turning lamps off."
$ The lamps are the three LED's (Light Emitting Diodes) in the upper right-hand
$ corner of the keyboard.
DKBD_KOOO_ERR   "Keyboard does not work."
DKBD_DCLICK_ERR "Auto-click cannot be disabled."
DKBD_ECLICK_ERR "Auto-click cannot be enabled."
DKBD_SPKR_ERR   "Speaker test failed."
DKBD_ELA_DEVICE "Error log analysis indicates a device failure."
DKBD_ELA_ADAP   "Error log analysis indicates an adapter failure."
DKBD_ELA_AORD   "Error log analysis indicates an unknown failure."
DKBD_CLICK_ERR  "Keyboard click test failed."
DKBD_CONF       "Device cannot be configured."
DKBD_OPEN       "Unable to open the device."
DKBD_TAB_OP     "Tablet device cannot be opened."
DKBD_TAB_CON    "Tablet cannot be configured."
DKBD_TAB_UN     "Tablet cannot be unconfigured."
DKBD_TAB        "Tablet"
DKBD_SW         "Software"

$set DMOUSE_MSG_SET_20
$ ---------------------------------------------------------------
$ Reason code set used by device type "Mouse"

MSE_UNEXPECTED "A device or adapter error occurred that was not expected."
MSE_DISABLE "Mouse disable test failed."
MSE_RESET "Mouse reset failed."
MSE_STATUS "Mouse read status failed."
MSE_GENERAL "Mouse device failed."
MSE_UNKNOWN "Unknown mouse."
MSE_WRAP "Mouse wrap mode failed."
MSE_PARAM "Error setting mouse parameters."
MSE_CONF "Device cannot be configured."
MSE_OPEN "Unable to open the device."
MSE_UNCONF "Device cannot be unconfigured."
MSE_SW "Software."
MSE_UNAT "Mouse not attached."

$set DTABLET_MSG_SET_21
$ ---------------------------------------------------------------
$ Reason code set used by device type "Tablet"
$ $ Do not add any messages above DC_TBLT.
$ $ Do not add any messages between DC_TBLT and TM_FM2.

DC_TBLT "Reserved for diagnostic controller"
TM_FM2  "Tablet"
TM_FM3  "Stylus"
TM_FM4  "4-button cursor"
TM_FM5  "Reserved"
TM_FM6  "Reserved"
TM_FM7  "Reserved"
TM_FM8  "Reserved"
TM_FM9  "Reserved"
TM_TU10 "Tablet reset and read tests failed."
TM_TU20 "Tablet internal wrap test failed."
TM_TU30 "Tablet conversion and resolution test failed."
TM_TU40 "Tablet indicator light test failed."
TM_TU50 "Tablet origin and sense test failed."
TM_TU60 "Input device indicator light test failed."
TM_TU70 "Input device switch test failed."
TM_TU80 "Tablet incremental data mode test failed."
TM_TU90 "Tablet point mode test failed."
TM_TUA0 "Tablet switched incremental data mode test failed."
TM_TUB0 "Tablet remote mode test failed."
TM_TUC0 "Disable tablet and enable tablet test failed."
TM_CNFG_ERR "Device cannot be configured."
TM_OPEN_ERR "Unable to open the device."
TM_SW "Software"
TM_FM10 "Cursor"
TM_UNCNFG_ERR "Tablet cannot be unconfigured."
TM_RSP "Tablet did not respond."
TM_ATT "Tablet not attached."
TM_STY "Tablet input device not attached."

$set DGIO_MSG_SET_22
$ ---------------------------------------------------------------
$ Reason code set used by device type "5080_GIO"

DGIO_DS_MSG "Graphics application input adapter"
DGIO_REGISTER  "Register test failed."
DGIO_FUSE  "Testing indicates fuse has failed."
DGIO_VPD  "VPD verification test failed."
$ VPD stands for Vital Product Data.
DGIO__COM  "Wrap test failed."
DGIO__MODEM  "Control line wrap test failed."
DGIO__BAUD  "Baud rate test failed."
DGIO__PARITY  "Parity test failed."
DGIO__FIFO  "FIFO test failed."
$ FIFO stands for First In, First Out.
DGIO__COM_DRIVER  "The communication drivers failed."
DGIO__RESET_LINE  "Selective reset line driver failed."
DGIO_ELA  "Error log analysis indicates an adapter failure."
DGIO_CONFIG_FAIL "Device configuration fails."
DGIO_SOFTWARE_ERROR "Software."
DGIO_OPEN_ERROR "Device driver indicates a hardware failure."
DGIO_IOPLANAR "I/O Planar."


$set DDIALS_MSG_SET_23
$ ---------------------------------------------------------------
$ Reason code set used by device type "Dials"

DIAL_CBL "Dials cable needs replacement."
DIAL_COM_FAILED "Dials communication line failed."
DIAL_RESET_LINE "Dials selective reset line failed."
DIAL_RESET_FAILED "Dials reset failed."
DIAL_BAT_CODE "Dials basic assurance test failed."
DIAL_LCL_WRAP "Dials local wrap test failed."
DIAL_FAILED "Dials unit failed."
DIAL_POW "Device external power supply failure."
ASYNC_DIAL_CBL "Cable from Serial Port to Dial Unit Failed."
SER_PORT_FAILED "Serial port that Dial is connected to failed."
DIAL_CONFIG_FAIL "Device configuration fails."
DIAL_SOFTWARE_ERROR "Software."
DIAL_OPEN_ERROR "Device driver indicates a hardware failure."
DIAL_IOPLANAR "I/O Planar."


$set DLPFK_MSG_SET_24
$ ---------------------------------------------------------------
$ Reason code set used by device type "LPF Keys"
$ LPFK stands for Lighted Program Function Keyboard.

LPF_CBL "LPFK cable needs replacement."
LPFK_COM_FAILED "LPFK communication line failed."
LPFK_RESET_LINE "LPFK selective reset line failed."
LPFK_RESET_FAILED "LPFK reset failed."
LPFK_BAT_CODE "LPFK basic assurance test failed."
LPFK_LCL_WRAP "LPFK local wrap test failed."
LPFK_FAILED "LPFK unit failed."
LPFK_POW "Device external power supply failure."
ASYNC_LPFK_CBL "Cable from Serial Port to LPFK Unit Failed."
SERIAL_PORT_FAILED "Serial port that LPFK is connected to failed."
LPFK_CONFIG_FAIL "Device configuration fails."
LPFK_SOFTWARE_ERROR "Software."
LPFK_OPEN_ERROR "Device driver indicates a hardware failure."
LPFK_IOPLANAR "I/O Planar."

$set D6157_MSG_SET_25
$ ---------------------------------------------------------------
$ Reason code set used by device type "Tape 6157"
$ $ FRU prefix 971

D6157_DEVICE  "150 MB 1/4-Inch Tape Drive"
$ MB stands for megabyte and means 1024x1024 bytes.
D6157_SCSI_CABLE "Cable, SCSI generic"
$ SCSI stands for Small Computer System Interface.
D6157_MSG_ID_101  "\
Timeout while attempting communication with SCSI device."
$ SCSI stands for Small Computer System Interface.
D6157_MSG_ID_102  "SCSI device is indicating BUSY."
$ SCSI stands for Small Computer System Interface.
D6157_MSG_ID_103  "SCSI device is indicating RESERVATION CONFLICT."
$ SCSI stands for Small Computer System Interface.
D6157_MSG_ID_104  "SCSI device is indicating CHECK CONDITION."
$ SCSI stands for Small Computer System Interface.
D6157_MSG_ID_105  "Error detected in request sense data."
D6157_MSG_ID_107  "Drive returned sense data that was not expected."
D6157_MSG_ID_108  "Vital Product Data mismatch."
D6157_MSG_ID_110  "\
Drive returned non-extended sense data when extended sense data was expected."
D6157_MSG_ID_111  "Condition from drive after a RESERVE is not valid."
D6157_MSG_ID_112  "Write protect sensor failed."
D6157_MSG_ID_113  "Condition from drive after a REQUEST SENSE is not valid."
D6157_MSG_ID_114  "Timeout while attempting communication with SCSI device."
$ SCSI stands for Small Computer System Interface.
D6157_ERROR_ELA    "An error is diagnosed from the error log analysis."
D6157_MSG_ID_115 "The write, read, compare test failed."
D6157_MSG_ID_116 "Unable to configure the device."
D6157_MSG_ID_117 "An unexpected SCSI error occurred."
D6157_MSG_SWERR "Software Error."

$set D6159x_MSG_SET_26
$ ---------------------------------------------------------------
$ Reason code set used by device type "Tape 6159x"
$ $ FRU prefix 972        Device VDAT

D6159x_DEVICE  "8mm Tape Drive"

D6159x_SCSI_CABLE "Cable, SCSI generic"
$ SCSI stands for Small Computer System Interface.
D6159x_MSG_ID_101  "Timeout while attempting communication with SCSI device."
$ SCSI stands for Small Computer System Interface.
D6159x_MSG_ID_102  "SCSI device is indicating BUSY."
$ SCSI stands for Small Computer System Interface.
D6159x_MSG_ID_103  "SCSI device is indicating RESERVATION CONFLICT."
$ SCSI stands for Small Computer System Interface.
D6159x_MSG_ID_104  "SCSI device is indicating CHECK CONDITION."
$ SCSI stands for Small Computer System Interface.
D6159x_MSG_ID_105  "Error detected in request sense data."
D6159x_MSG_ID_107  "Drive returned sense data that was not expected."
D6159x_MSG_ID_108  "Vital Product Data mismatch."
D6159x_MSG_ID_110  "\
Drive returned non-extended sense data when extended sense data was expected."
D6159x_MSG_ID_111  "Condition from drive after a RESERVE is not valid."
D6159x_MSG_ID_112  "Write protect sensor failed."
D6159x_MSG_ID_113  "Condition from drive after a REQUEST SENSE is not valid."
D6159x_MSG_ID_114  "Timeout while attempting communication with SCSI device."
$ SCSI stands for Small Computer System Interface.
D6159_ERROR_ELA    "An error is diagnosed from the error log analysis."
D6159x_MSG_ID_115 "The write, read, compare test failed."
D6159x_MSG_ID_116 "Unable to configure the device."
D6159x_MSG_ID_117 "An unexpected SCSI error occurred."
D6159x_MSG_SWERR "Software Error."

$set D6158x_MSG_SET_27
$ ---------------------------------------------------------------
$ Reason code set used by device type "Tape 6158x"
$ $ FRU prefix 970        Device 1/2 Inch 9-Track Tape Drive

D6158x_DEVICE  "1/2 Inch 9-Track Tape Drive"
D6158x_SCSI_CABLE "Cable, SCSI generic"
$ SCSI stands for Small Computer System Interface.
D6158x_MSG_ID_101  "Timeout while attempting communication with SCSI device."
$ SCSI stands for Small Computer System Interface.
D6158x_MSG_ID_102  "SCSI device is indicating BUSY."
$ SCSI stands for Small Computer System Interface.
D6158x_MSG_ID_103  "SCSI device is indicating RESERVATION CONFLICT."
$ SCSI stands for Small Computer System Interface.
D6158x_MSG_ID_104  "SCSI device is indicating CHECK CONDITION."
$ SCSI stands for Small Computer System Interface.
D6158x_MSG_ID_105  "Error detected in request sense data."
D6158x_MSG_ID_107  "Drive returned sense data that was not expected."
D6158x_MSG_ID_108  "Vital Product Data mismatch."
D6158x_MSG_ID_110  "\
Drive returned non-extended sense data when extended sense data was expected."
D6158x_MSG_ID_111  "Condition from drive after a RESERVE is not valid."
D6158x_MSG_ID_112  "Write protect sensor failed."
D6158x_MSG_ID_113  "Condition from drive after a REQUEST SENSE is not valid."
D6158x_MSG_ID_114  "Timeout while attempting communication with SCSI device."
$ SCSI stands for Small Computer System Interface.
D6158x_MSG_ID_EXT  "\
Refer to the Unit Reference Codes in the 1/2 9-Track Tape Drive documentation."
D6158_ERROR_ELA    "An error is diagnosed from the error log analysis."
D6158x_MSG_ID_115 "The write, read, compare test failed."
D6158x_MSG_ID_116 "Unable to configure the device."
D6158x_MSG_ID_117 "An unexpected SCSI error occurred."
D6158x_MSG_SWERR "Software Error."

$set DSKY116_MSG_SET_28
$ ---------------------------------------------------------------
$ Reason code set used by device type "Skyway"

DSKY_DA_DCR     "\
The display control registers test using direct access failed."
DSKY_IA_DCR     "\
The display control registers test using indexed access failed."
DSKY_CRTC_HDR   "The horizontal display registers test failed."
DSKY_CRTC_VDR   "The vertical display registers test failed."
DSKY_CRTC_SCR   "The sprite control registers test failed."
$ A "sprite" is a form of animation where only one element, such as a person,
$ is drawn on a background that does not change.
DSKY_CRTC_SCRDS "The sprite control registers and disable sprite test failed."
$ A "sprite" is a form of animation where only one element, such as a person,
$ is drawn on a background that does not change.
DSKY_MRT        "The miscellaneous registers test failed."
DSKY_CRTC_SPCR  "The sprite and palette control registers test failed."
$ A "sprite" is a form of animation where only one element, such as a person,
$ is drawn on a background that does not change.
DSKY_VPD        "The test of the Vital Product Data failed."
DSKY_VRAM_MC    "The VRAM memory clear test failed."
$ VRAM stands for Volatile Random Access Memory.
DSKY_VRAM_MF    "The VRAM memory write test with 0xFF failed."
$ VRAM stands for Volatile Random Access Memory.
$ 0xFF is a hex pattern written to memory.  DO NOT TRANSLATE.
DSKY_VRAM_MF3   "The VRAM memory write test with 0x33 failed."
$ VRAM stands for Volatile Random Access Memory.
$ 0xFF is a hex pattern written to memory.  DO NOT TRANSLATE.
DSKY_VRAM_MFC   "The VRAM memory write test with 0xCC failed."
$ VRAM stands for Volatile Random Access Memory.
$ 0xFF is a hex pattern written to memory.  DO NOT TRANSLATE.
DSKY_VRAM_MF5   "The VRAM memory write test with 0x55 failed."
$ VRAM stands for Volatile Random Access Memory.
$ 0xFF is a hex pattern written to memory.  DO NOT TRANSLATE.
DSKY_VRAM_MFA   "The VRAM memory write test with 0xAA failed."
$ VRAM stands for Volatile Random Access Memory.
$ 0xFF is a hex pattern written to memory.  DO NOT TRANSLATE.
DSKY_VRAM_MFP   "The VRAM memory pattern write test failed."
$ VRAM stands for Volatile Random Access Memory.
DSKY_VRAM_MF16  "The VRAM memory 16 bit write test failed."
$ VRAM stands for Volatile Random Access Memory.
DSKY_VRAM_MF32  "The VRAM memory 32 bit write test failed."
$ VRAM stands for Volatile Random Access Memory.
DSKY_COPR_ILT   "The interrupt level test failed."
DSKY_COPR_BLDT  "The Bressenham line draw test failed."
DSKY_COPR_MLDT  "The multiple line draw test failed."
DSKY_COPR_SLLDT "The screen length line draw test failed."
DSKY_COPR_SDLDT "The step and draw line test failed."
DSKY_COPR_SDMDT "The step and draw multiple line test failed."
DSKY_COPR_SDSDT "The step and draw screen length line test failed."
DSKY_COPR_AF    "The area fill test failed."
DSKY_COPR_CC    "The color compare test failed."
DSKY_COPR_CP    "The color plane test failed."
DSKY_COPR_PXBLT "The PxBLT octant test failed."
$ DO NOT TRANSLATE PxBLT.
DSKY_COPR_4BPP  "The 4 bit per pel test failed."
$ A pel is a picture element, or one single dot on the display.
DSKY_COPR_MT    "The mask test failed."
DSKY_COPR_MET   "The mask enabled test failed."
DSKY_COPR_VHVNN "\
A VRAM to Host to VRAM DMA transfer failed."
$ VRAM stands for Volatile Random Access Memory.
$ DMA stands for Direct Memory Access.
DSKY_COPR_VHVNM "\
A VRAM to Host to VRAM DMA transfer using 4 bits per pel failed."
$ VRAM stands for Volatile Random Access Memory.
$ DMA stands for Direct Memory Access.
$ A pel is a picture element, or one single dot on the display.
DSKY_RGB   "The color display test failed."
DSKY_CLEAR   "The clear display test failed."
DSKY_AMON   "attached monitor"
DSKY_UAE    "An adapter error occurred that could not be identified."
DSKY_UDE    "An display error occurred that could not be identified."

$set MSG_SET_29
$ ---------------------------------------------------------------
$ Reason code set used by device type reserved


$set DGEM_MSG_SET_30
$ ---------------------------------------------------------------
$ Reason code set used by device type "Gemini"

DGEM_MSG_300 "Global memory test failed."
DGEM_MSG_400 "Register test failed."
DGEM_MSG_500 "MBC register test failed."
$ MBC stands for
DGEM_MSG_700 "Not able to load graphics control processor."
DGEM_MSG_701 "Graphics control processor test failed."
DGEM_MSG_702 "Graphics control processor not found."
DGEM_MSG_703 "Not able to load drawing processor."
DGEM_MSG_704 "Drawing processor test failed."
DGEM_MSG_705 "Drawing processor not found."
DGEM_MSG_706 "Not able to load shading processor."
DGEM_MSG_707 "Shading processor test failed."
DGEM_MSG_708 "Shading processor not found."
DGEM_MSG_709 "Pixel memory test failed."
DGEM_MSG_710 "Not able to load multiple cards."
DGEM_MSG_711 "EPM card failed."
$ EPM stands for
DGEM_MSG_712 "Multiple cards not found."
DGEM_MSG_ELA "Hardware error logged by device driver."
DGEM_MSG_VIS "Visual test failed."
DGEM_MSG_CAB "Graphics subsystem cable"
DGEM_MSG_MAG "CVME interface card"
DGEM_MSG_GCP "Graphics Control Processor"
DGEM_MSG_SHP "Shading Processor"
DGEM_MSG_DRP "Drawing Processor"
DGEM_MSG_EPM "Pixel memory card"
$
DGEM_MSG_MON "The attached monitor"
DGEM_MSG_DMA "Direct memory access (DMA) test failed."
DGEM_MSG_FIF "FIFO test failed."
DGEM_VPD_GCP "VPD checksum test failed in Graphics control processor"
DGEM_VPD_DrP "VPD checksum test failed in Drawing processor"
DGEM_VPD_ShP "VPD checksum test failed in Shading processor"
DGEM_VPD_FBB "VPD checksum test failed in Pixel Memory card"
DGEM_VPD_MAG "VPD checksum test failed in CVME interface card"
DGEM_VPD_MCA "VPD checksum test failed in Microchannel interface card"
DGEM_LOAD    "Microcode load failed"
DGEM_MSG_BUS     "Shared bus test failed"
$

$set DSCSIA_MSG_SET_31
$ ---------------------------------------------------------------
$ Reason code set used by device type "dscsia"

DSCSIA_MSG_1 "Adapter diagnostic subcommand failed."
DSCSIA_MSG_2 "Adapter fuse failed."
DSCSIA_MSG_3 "Wrap test failed."
DSCSIA_MSG_4 "BCR register write and read test failed."
$ BCR stands for
DSCSIA_MSG_5 "POS register write and read test failed."
$ POS stands for Programmable Option Select.
DSCSIA_MSG_6 "Internal and external reset test failed."
DSCSIA_MSG_7 "Adapter command timed out."
DSCSIA_MSG_8 "Adapter hardware has caused a software failure."
DSCSIA_MSG_9 "Error log analysis indicates hardware failure."
DSCSIA_MSG_10 "ROM CRC error."
$ ROM stands for Read Only Memory.  CRC stands for Cyclic Redundancy Check.
DSCSIA_MSG_11 "Adapter RAM error."
$ RAM stands for Random Access Memory.
DSCSIA_MSG_12 "Control logic failed."
DSCSIA_MSG_13 "Diagnostics completed with previous error."
FUSE_SCSI "Fuse, SCSI Adapter"
$ SCSI stands for Small Computer System Interface.
DSCSIA_MSG_14 "VPD Test failed."
$ VPD stands for Vital Product Data.
DSCSIA_MSG_15 "Thermal fuse has tripped."
$ SCSI Thermal fuse failed.
DSCSIA_MSG_16 "Device failed to configure."
SOFTWARE_ERROR "Software error."
SCSI_BUS_ERROR "SCSI Bus problem"
FUSE_PTC_OPEN "Fuse/Thermal device open."


$set DHF_MSG_SET_32
$ ---------------------------------------------------------------
$ Reason code set used by device type "dhf"

DHF_DA_MSG_255  "A medium error occurred that was not recoverable."
DHF_DA_MSG_256  "The drive motor failed to restart."
DHF_DA_MSG_257  "The drive electronics card failed."
DHF_DA_MSG_258  "A bus failure occurred."
DHF_DA_MSG_259  "Medium format is corrupted."
DHF_DA_MSG_260  "Diagnostic test failed."
DHF_DA_MSG_261  "A hardware error occurred that was not recoverable."
DHF_DA_MSG_262  "Protocol error."
DHF_DA_MSG_263  "A SCSI command timeout error occurred."
DHF_DA_MSG_264  "A SCSI BUSY command error occurred."
DHF_DA_MSG_265  "A SCSI RESERVATION CONFLICT error occurred."
DHF_DA_MSG_266  "A SCSI CHECK CONDITION error occurred."
DHF_DA_MSG_267  "Disk Drive has caused a software failure."
DHF_DA_MSG_268  "Error log analysis indicates a hardware failure."
DHF_DA_MSG_269  "The drive cannot be started."
DHF_DA_MSG_270  "A Disk Drive hardware error occurred."
DHF_DA_MSG_272  "A write protect error occured."
DHF_ELECTRONICS_CARD "857 MB Drive Electronics Card"
DHF_DISK_ENCLOSURE "Drive head disk assembly"
DHF_DRIVE_ELEC_CARD "Drive Electronics Card"
DHF_DA_MSG_273 "Error Log Analysis has detected error(s) that may require your attention."
DHF_DA_MSG_274  "Use the Subsystem Service Guide to complete Diagnosis."
DHF_SOFTWARE_ERROR "Software"
DHF_ADAPTER_CONFIG_FAIL "Adapter Configuration Failure"
DHF_DEVICE_CONFIG_FAIL "Device Configuration Failure"
DHF_DEVICE_CERTIFY_FAIL "The certify operation failed."
DHF_DA_MSG_DISK_ERR3  "Error log analysis indicates a SCSI bus problem."
DHF_SCSI_BUS "SCSI bus problem: cables, terminators or other SCSI devices"


$set DFDA_MSG_SET_33
$ ---------------------------------------------------------------
$ Reason code set used by device type "dfda"

DFDA_DISKETTE_ADAPTER_ERR "Diskette Adapter failed."


$set DFD_MSG_SET_34
$ ---------------------------------------------------------------
$ Reason code set used by device type "dfd"
$ MB stands for megabyte and means 1024x1024 bytes.

DFD144_DISKETTE_ADAPTER_ERR "Diskette adapter failure"
DFD144_SELECT_ERR "\
Diskette Drive (3.5 inch 1.44MB) select test failed."

DFD144_DESLECT_ERR "\
Diskette Drive (3.5 inch 1.44MB) deselect test failed."

DFD144_RECALIBRATE_ERR "\
Diskette Drive (3.5 inch 1.44MB) recalibration test failed."

DFD144_DISK_CHANGE_ERR "\
Diskette Drive (3.5 inch 1.44MB) disk change test failed."

DFD144_WRITE_PROT_ERR "\
Diskette Drive (3.5 inch 1.44MB) write protect failed."

DFD144_INDEX_ERR "\
Diskette Drive (3.5 inch 1.44MB) index test failed."

DFD144_STEP_ERR "\
Diskette Drive (3.5 inch 1.44MB) step test failed."

DFD144_READ_ERR "\
Diskette Drive (3.5 inch 1.44MB) read test failed."

DFD144_WRITE_ERR "\
Diskette Drive (3.5 inch 1.44MB) write test failed."

DFD144_DATA_COMPARE_ERR "\
Diskette Drive (3.5 inch 1.44MB) write and read test failed."

DFD144_DENSITY_FORMAT_ERR "\
Diskette Drive (3.5 inch 1.44MB) low density test failed."

DFD144_VERIFY_ERR "\
Diskette Drive (3.5 inch 1.44MB) verify test failed."

DFD144_MOTOR_SPEED_ERR "\
Diskette Drive (3.5 inch 1.44MB) motor speed failed."

DFD144_HEAD_SETTLE_ERR "\
Diskette Drive (3.5 inch 1.44MB) head settle test failed."

DFD144_REDRIVE "Redrive Card "
DFD12_DISKETTE_ADAPTER_ERR "Diskette adapter failed."
DFD12_SELECT_ERR "\
Diskette Drive (5.25 inch 1.2MB) select test failed."

DFD12_DESLECT_ERR "\
Diskette Drive (5.25 inch 1.2MB) deselect test failed."

DFD12_RECALIBRATE_ERR "\
Diskette Drive (5.25 inch 1.2MB) recalibration test failed."

DFD12_DISK_CHANGE_ERR "\
Diskette Drive (5.25 inch 1.2MB) disk change test failed."

DFD12_WRITE_PROT_ERR "\
Diskette Drive (5.25 inch 1.2MB) write protect failed."

DFD12_INDEX_ERR "\
Diskette Drive (5.25 inch 1.2MB) index test failed."

DFD12_STEP_ERR "\
Diskette Drive (5.25 inch 1.2MB) step test failed."

DFD12_READ_ERR "\
Diskette Drive (5.25 inch 1.2MB) read test failed."

DFD12_WRITE_ERR "\
Diskette Drive (5.25 inch 1.2MB) write test failed."

DFD12_DATA_COMPARE_ERR "\
Diskette Drive (5.25 inch 1.2MB) write and read test failed."

DFD12_DENSITY_FORMAT_ERR "\
Diskette Drive (5.25 inch 1.2MB) low density test failed."

DFD12_VERIFY_ERR "\
Diskette Drive (5.25 inch 1.2MB) verify test failed."

DFD12_MOTOR_SPEED_ERR "\
Diskette Drive (5.25 inch 1.2MB) motor speed test failed."

DFD12_HEAD_SETTLE_ERR "\
Diskette Drive (5.25 inch 1.2MB) head settle test failed."

DFD12_REDRIVE "Redrive Card "
DFD_ELA_12M "\
Error log analysis indicates a diskette drive (5.25 inch 1.2MB) failure."

DFD_ELA_144M "\
Error log analysis indicates a diskette drive (3.5 inch 1.44MB) failure."

DFD_CONFIG_ERR "Unable to configure the device."

DFD_SELECT_ERR "Diskette Drive select test failed."
DFD_DESLECT_ERR "Diskette Drive deselect test failed."
DFD_RECALIBRATE_ERR "Diskette Drive recalibration test failed."
DFD_DISK_CHANGE_ERR "Diskette Drive disk change test failed."
DFD_WRITE_PROT_ERR "Diskette Drive write protect test failed."
DFD_INDEX_ERR "Diskette Drive index test failed."
DFD_STEP_ERR "Diskette Drive step test failed."
DFD_READ_ERR "Diskette Drive read test failed."
DFD_WRITE_ERR "Diskette Drive write test failed."
DFD_DATA_COMPARE_ERR "Diskette Drive write and read test failed."
DFD_DENSITY_FORMAT_ERR "Diskette Drive low density test failed."
DFD_VERIFY_ERR "Diskette Drive verify test failed."
DFD_MOTOR_SPEED_ERR "Diskette Drive motor speed test failed."
DFD_HEAD_SETTLE_ERR "Diskette Drive head settle test failed."
DFD_ELA "Error log analysis indicates a hardware failure."

$set DLEGA_MSG_SET_35
$ ---------------------------------------------------------------
$ Reason code set used by device type "lega"

POS_TEST_FAILED "POS register test failed."
VPD_CRC_FAILED "VPD CRC check failed."
HOST_BIM_FAILED "Host Bus Interface Module failed."
HOST_MEM "Adapter Memory test failed."
MASTER_C30_MEM "Processor on the base card failed."
BLAST_C30_MEM "Processor daughter card failed."
BIM_FAILED "Bus Interface Module failed."
HOST_INTERRUPT "Bus Interface Module failed to interrupt the host."
BIM_DMA "DMA Bus Interface Module failed."
BIM_DYNAMIC "Dynamic Bus Interface Module failed."
M_B_DYNAMIC "Handshake Bus to Interface Module failed."
BLAST "Read/Write register failed."
RAMDAC "Read/Write test failed."
FRAME_BUFFER "VRAM test failed."
Z_BUFFER1 "Z-Buffer DRAM failed."
Z_BUFFER2 "Z-Buffer DRAMs failed."
Z_BUFFER3 "Z-Buffer DRAMs failed."
Z_BUFFERS "Z-Buffer DRAMs failed."
WINDOW_BUFFER "Window memory test failed."
RED_SCREEN "Red Screen verification failed."
GREEN_SCREEN "Green Screen verification failed."
BLUE_SCREEN "Blue Screen verification failed."
LEGA_SOFTWARE "Software Error."
LEGA_FAILED "Error Log Analysis indicate hardware failure."
LEGA_GRAPHIC "Graphics Processor Card."
LEGA_ZBUFFER "Graphics Z-Buffer Card."

R_PROC "Processor Card Test failed."
R_GRAPHIC "Graphic Card Test failed."
R_OPTION "Option Card Test failed."
R_PIPE "Pipe Card Test failed."
R_OPEN "Device failed to open."
R_CFG "Device failed to configure."
R_MON "Monitor test failed."
R_GEN "This test failed."
F_MON "Attached Monitor"
R_BASE "POWER Gt family graphics adapter base card test failed."
R_PLANAR "Graphics card test indicates I/O planar failure."
F_GT3PC "POWER Gt3 Processor Card"

$set DPED_MSG_SET_36
$ ---------------------------------------------------------------
$ Reason code set used by device type "reserved"


$set DCDROM_MSG_SET_37
$ ---------------------------------------------------------------
$ Reason code set used by device type "dcdrom"

DCDROM_DEVICE  "CD-ROM Drive"
DCDROM_SCSI_CABLE "Cable, SCSI generic"
DCDROM_MSG_ID_111 "Not able to reserve device."
DCDROM_MSG_ID_112 "Not able to do configuration."
DCDROM_MSG_ID_113 "Not able to open device driver."
DCDROM_MSG_ID_121_164 "CD ROM indicates error."
DCDROM_MSG_ID_171 "Error during audio test."
DCDROM_MSG_ID_172 "Error during audio test."
DCDROM_MSG_ID_181 "Error during audio test."
DCDROM_MSG_ID_182 "Error during audio test."
DCDROM_MSG_ID_199 "An error occured that could not be identified."
DCDROM_MSG_ID_211 "After a response, an error occured."
DCDROM_MSG_ID_281 "After a response, an error occured."
DCDROM_MSG_ID_301 "Error log analysis indicates medium failure."
$ Medium refers to the disc used by the CD-ROM.
DCDROM_MSG_ID_302 "Error log analysis indicates hardware failure."
DCDROM_MSG_ID_183 "Error reading data on media."
DCDROM_SOFTWARE  "Software"

$set DPCA_MSG_SET_38
$ ---------------------------------------------------------------
$ Reason code set used by device type "370 Parallel Channel Attach"
$ $ Do not add any messages above DC_370PCA.
$ $ Do not add any messages between D370_BMN and D370_1.
$ $ Do not add any messages above D370_26.

DC_370PCA       "370 Parallel Channel Attach Adapter"
D370_CCIB       "Channel Cable and Interface Box, Part Number 00F0000."
D370_OPEN       "Error opening device driver."
D370_ECFG       "Error configuring device."
D370_BMN        "Reserved"
$ D370_BMN is the message number all FRU bucket rmsg's are referenced to.

D370_1          "Power on self test failed."
D370_2          "POS register test failed."
D370_3          "370PCA adapter to system unit interrupt test failed."
D370_4          "370PCA 80186 microprocessor test failed."
D370_5          "DRAM test failed."
D370_6          "SRAM test from 80186 failed."
D370_7          "VPD verification test failed."
D370_8          "370PCA interrupt level test failed."
D370_9          "370PCA internal wrap test failed."
D370_10         "370PCA adapter wrap test failed."
D370_11         "Reserved."
D370_12         "370PCA hardware register test failed."
D370_13         "Reserved."
D370_14         "Diagnostic microcode download failed."
D370_15         "370PCA Channel Cable Interface box wrap test failed."
D370_16         "Reserved."
D370_17         "Hardware FIFO test from 80186 failed."
D370_18         "Hardware FIFO test from Micro Channel Interface failed."
D370_19         "Reserved."
D370_20         "Reserved."
D370_21         "Base address switching test failed."
D370_22         "Micro Channel parity test failed."
D370_23         "SRAM test from Micro Channel Interface failed."
D370_24         "DMA test failed."
D370_25         "370PCA interrupt test failed."
D370_26         "370PCA data loopback test failed."

$set DHFD_MSG_SET_39
$ ---------------------------------------------------------------
$ Reason code set used by device type "dhfd"

DHFD_TU0_1  "Diagnostic self-test failed."
DHFD_TU0_2  "Diagnostic self-test stop command sent."
DHFD_TU1_3  "Buffer write, read, and compare test failed."
DHFD_TU1_4  "Buffer write, read, and compare test stop command sent."
DHFD_TU2_5  "Read and write test failed."
DHFD_TU2_6  "Read and write test stop command sent."
DHFD_TU3_7  "Seek test failed."
DHFD_TU3_8  "Seek test stop command sent."
DHFD_TU4_9  "Read and verify test failed."
DHFD_TU4_10  "Read and verify test stop command sent."
DHFD_TU5_11  "IOCINFO test failed."
$ IOCINFO stands for
DHFD_TU1_12  "Write and read buffer failed on read and compare."
DHFD_MSG_13 "Command timeout."
DHFD_MSG_14 "Operating System failure."
$ AIX stands for Advanced Interactive Executive and is the operating system
$ for use on RISC System/6000 machines.
DHFD_MSG_15 "Disk Drive has caused a software failure."
DHFD_MSG_16 "Error log analysis indicates hardware failure."
DHFD_MSG_17 "The disk drive has failed. "
DHFD_MSG_18 "The disk drive has failed. "
DHFD_MSG_19 "The disk drive has failed. "
DHFD_RISER_CARD "Riser Card "

$set DASYNC_422_8_MSG_SET_40
$ ---------------------------------------------------------------
$ Reason code set used by device type "Rs422 8 port"
$ $ Do not insert any messages above DC_422_8.
$ $ Do not add any messages between DC_422_8 and DM9_ICPN_2.

DC_422_8 "8-Port Asynchronous Adapter EIA-422A"
DM9_ICPN_2      "Multiport Interface Cable, Part Number 00F5524"
DM9_TCPN_3      "Terminal Cable EIA-422A, Part Number 30F8966"
DM9_4           "Reserved"
DM9_5           "Reserved"
DM9_ECPN_6      ""
$ ECPN_6 moved to CPN24 for name change
DM9_7           "Reserved"
DM9_8           "Reserved"
DM9_9           "Reserved"
DM9_TU10        "8 port Async adapter EIA-422A register test failed."
$ Async stands for asynchronous communications.
DM9_TU20        "8 port Async adapter EIA-422A local chip wrap test failed."
$ Async stands for asynchronous communications.
DM9_TU30        "8 port Async adapter EIA-422A driver wrap test failed."
$ Async stands for asynchronous communications.
DM9_TU40        "8-port Async adapter extension cable wrap test failed."
$ Async stands for asynchronous communications.
DM9_TU50        "Multiport interface cable wrap test failed."
DM9_TU60        "Reserved"
DM9_TU70        "Terminal cable wrap test failed."
DM9_TU80        "Reserved"
DM9_LM          "8 port EIA422 adapter test failed."
DM9_PN19        "Reserved"
DM9_PN20        "Reserved"
DM9_TU60C       "Reserved"
DM9_PN22        "Reserved"
DM9_PN23        "Reserved"
DM9_CPN24       "8/16-Port Rack Extension Cable, Part Number 53F3048"

$set DASYNC_422_16_MSG_SET_41
$ ---------------------------------------------------------------
$ Reason code set used by device type "Rs422 16 port"
$ $ Do not insert any messages above DC_422_16.
$ $ Do not add any messages between DC_422_16 and DM11_ICPN_2.

DC_422_16 "16-Port Asynchronous Adapter EIA-422A"
DM11_ICPN_2     "16 Port Interface Cable EIA-422A, Part Number 53F3381"
DM11_TCPN_3     "Terminal Cable EIA-422A, Part Number 30F8966"
DM11_4          "Reserved"
DM11_5          "Reserved"
DM11_ECPN_6     ""
$ ECPN_6 moved to CPN24 for name change.
DM11_7          "Reserved"
DM11_8          "Reserved"
DM11_9          "Reserved"
DM11_TU10       "16 port Async adapter EIA-422A register test failed."
$ Async stands for asynchronous communications.
DM11_TU20       "16 port Async adapter EIA-422A local chip wrap test failed."
$ Async stands for asynchronous communications.
DM11_TU30       "16 port Async adapter EIA-422A driver wrap test failed."
$ Async stands for asynchronous communications.
DM11_TU40       "16-port Async adapter extension cable wrap test failed."
$ Async stands for asynchronous communications.
DM11_TU50       "16 port interface cable EIA-422A wrap test failed."
DM11_TU60       "Reserved"
DM11_TU70       "Terminal cable wrap test failed."
DM11_TU80       "Reserved"
DM11_LM         "16 port EIA422 adapter test failed."
DM11_PN19       "Reserved"
DM11_PN20       "Reserved"
DM11_TU60C      "Reserved"
DM11_PN22       "Reserved"
DM11_PN23       "Reserved"
DM11_CPN24      "8/16-Port Rack Extension Cable, Part Number 53F3048"

$set DASYNC_mil_MSG_SET_42
$ ---------------------------------------------------------------
$ Reason code set used by device type "Mil 188"
$ $ Do not add any messages before or between existing messages.
$ $ Blank messages are place holders.

DC_MIL_188 "8-Port Asynchronous Adapter MIL_STD 188"
DM10_ICPN_2     "Multiport Interface Cable Part, Number 00F5524"
DM10_ACPN_3     "Asynchronous Cable EIA-232, Part Number 6323741"
DM10_CIPN_4     "Printer/Terminal Interposer, Part Number 58F2861"
DM10_5          ""
DM10_ECPN_6     ""
$ ECPN_6 moved to CPN24 for name change.
DM10_7          ""
DM10_8          ""
DM10_9          ""
DM10_TU10       "8 port Async adapter MIL-STD 188 register test failed."
$ Async stands for asynchronous communications.
DM10_TU20       "8 port Async adapter MIL-STD 188 local chip wrap test failed."
$ Async stands for asynchronous communications.
DM10_TU30       "8 port Async adapter MIL-STD 188 wrap test failed."
$ Async stands for asynchronous communications.
DM10_TU40       "8-port Async adapter extension cable wrap test failed."
$ Async stands for asynchronous communications.
DM10_TU50       "Multiport interface cable wrap test failed."
DM10_TU60       ""
DM10_TU70       "Asynchronous cable wrap test failed."
DM10_TU80       "Interposer wrap test failed."
DM10_LM         "8 port MIL-STD188 adapter test failed."
DM10_PN19       ""
DM10_PN20       ""
DM10_TU60C      ""
DM10_PN22       ""
DM10_PN23       ""
DM10_CPN24      "8/16-Port Rack Extension Cable, Part Number 53F3048"
DM10_PTCPN_25   "Printer/Terminal Cable, Part Number 12H1204"

$set DX25_MSG_SET_43
$ ---------------------------------------------------------------
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

$ $ Do not insert any messages above MP6_232.
$ $ Do not add any messages between MP_ERRMSGS and MP_01.

MP4_232         "4-Port Multiport/2 Adapter RS-232"
MP6_SYN         "6-Port Multiport/2 Adapter SYNC"
MP8_232         "8-Port Multiport/2 Adapter RS-232"
MP8_422         "8-Port Multiport/2 Adapter RS-422"
MP4_4           "8-Port Multiport/2 Adapter RS-232/RS-422"
MP_EIBS         "reserved"
$ MP_EIBS is the message number all FRU bucket fmsg's are referenced to.

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
$ MP_ERRMSGS is the message number all FRU bucket rmsg's are referenced to.

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

$set DIOP_MSG_SET_44
$ ---------------------------------------------------------------
$ Reason code set used by device type "IoPlanar"

DIOP_ERR_NVRAM   "I/O Planar - NVRAM test failed."
$ I/O stands for Input/Output.
$ NVRAM stands for Non-Volatile Random Access Memory.
DIOP_ERR_TOD     "I/O Planar - Time of Day RAM test failed."
$ I/O stands for Input/Output.
$ RAM stands for Random Access Memory.
DIOP_ERR_CRRA    "I/O Planar - CRRA register test failed."
$ I/O stands for Input/Output.
$ CRRA stands for
DIOP_ERR_OP      "I/O Planar - operator test panel LED failed."
$ I/O stands for Input/Output.
$ LED stands for Light Emitting Diode.
DIOP_ERR_BAT     "I/O Planar - battery test failed."
$ I/O stands for Input/Output.
DIOP_ERR_TOD_P   "I/O Planar - Time of Day a POR"
$ I/O stands for Input/Output.
$ POR stands for
DIOP_ERR_TOD_N   "I/O Planar - Time of Day clock is not running."
$ I/O stands for Input/Output.
DIOP_ERR_KEY     "I/O Planar - key lock failed."
$ I/O stands for Input/Output.
OP_PANEL         "Operator panel"
$ I/O stands for Input/Output.
BATT             "I/O Planar - battery"
$ I/O stands for Input/Output.
KEYLOCK          "Operator panel key lock"
IOP_LED          "Three-Digit Display"
DIOP_EPOW "The EPOW connector test failed."
$ EPOW stands for Early Power Off Warning.
DIOP_ERR_VPD     "I/O Planar - VPD test failed."
$ I/O stands for Input/Output.
DIOP_ERR_ECREG   "The register test on the second I/O planar failed."
$ I/O stands for Input/Output.
DIOP_ERR_MODE_SWITCH   "I/O Planar - mode switch failed."
$ I/O stands for Input/Output.
DIOP_DMA_ERR   "Error log indicates that a DMA error occurred"
$
DIOD_ERR_NVRAM   "IOD Board - NVRAM test failed."
$ IOD Board stands for Input/Output Daughter Board.
$ NVRAM stands for Non-Volatile Random Access Memory.
DIOD_ERR_LCD     "IOD Board - operator panel LCD test failed."
$ IOD Board stands for Input/Output Daughter Board.
$ LCD stands for Liquid Crystal Display.
DIOD_ERR_KEY     "IOD Board - key lock failed."
$ IOD Board stands for Input/Output Daughter Board.
DIOD_ERR_TOD_P   "IOD Board - Time of Day at POR"
$ IOD Board stands for Input/Output Daughter Board.
$ POR stands for Power On Reset.
DIOD_ERR_TOD_N   "IOD Board - Time of Day RAM test failed."
$ IOD Board stands for Input/Output Daughter Board.
DIOD_ERR_TOD     "IOD Board - Time of Day clock is not running."
$ IOD Board stands for Input/Output Daughter Board.
$ RAM stands for Random Access Memory.
DIOD_ERR_BAT     "IOD Board - battery test failed."
$ IOD Board stands for Input/Output Daughter Board.


$set DASD_MSG_SET_45
$ ---------------------------------------------------------------
$ Reason code set used by device type "Serial disk"

DSDASD_MSG1 "Use the 9333 Installation and Service Guide."

$set OPTICS_MSG_SET_46
$ ---------------------------------------------------------------
$ Reason code set used by device type "2-port optics"


OPTICS_MSG_501 "Configuration For Device Fails "
OPTICS_MSG_502 "Software"
OPTICS_MSG_701 "Device driver indicates a hardware failure"
OPTICS_MSG_801 "Error log indicates a hardware failure"
OPTICS_MSG_901 "CPU Planar"
OPTICS_MSG_902 "Serial Optic Channel Converter"


NEW_OPTICS_MSG_101 "Buffer Access Mode Test Fails"
NEW_OPTICS_MSG_201 "Internal Wrap Tests Fails"
NEW_OPTICS_MSG_301 "Internal Wrap tests Fails"
NEW_OPTICS_MSG_401 "Data Wrap Tests Fails"
NEW_OPTICS_MSG_903 "Optical Cable"



$set DA_COMMON_MSG_SET_47
$ -----------------------------------------------------------------
$ Common messages used by Diagnostic Applications

CUSTOMER_TESTING_MENU "TESTING %1$s %2$s IN LOCATION %3$s\n\n\
Please stand by."
$
$ This panel is displayed when the test starts.
$ %1$s is the type of resource.
$ %2$s is the name of the resource being tested.
$ %3$s is the location of the resource.
$ Leave line spacing as shown.  See general notes on length of title line.

ADVANCED_TESTING_MENU "TESTING %1$s %2$s IN LOCATION %3$s\n\
IN ADVANCED MODE\n\n\
Please stand by."
$
$ This panel is displayed when the test starts in advanced mode.
$ %1$s is the type of resource.
$ %2$s is the name of the resource being tested.
$ %3$s is the location of the resource.
$ Leave line spacing as shown.  See general notes on length of title line.

LOOPMODE_TESTING_MENU "TESTING %1$s %2$s IN LOCATION %3$s\n\
IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%4$6d\tpasses completed.\n\
%5$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ This panel is displayed when in the loop test mode.
$ %1$s is the type of resource.
$ %2$s is the name of the resource being tested.
$ %3$s is the location of the resource.
$ %4$6d\t sets the number of diagnostic test passes completed.
$ %5$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title line.

NO_MICROCODE_MENU "%1$6X \
%2$s OPERATIONAL MICROCODE IS MISSING\n\n\
The %3$s operational microcode is either missing\n\
or not accessible.\n\n\
This microcode is necessary in order to use the %4$s\n\
in normal system operations."
$
$ %1$6X is the 6 hexadecimal digits representing the menu goal number.
$ %2$s is the description of the resource in uppercase (SCSI ADAPTER).
$ %3$s is the description of the resource in lowercase (SCSI adapter).
$ %4$s is similar %3s
$ Leave line spacing as shown.  See general notes on length of title.
$ Do not translate the 6 digits number at the beginning of the message.
$ Leave it exactly as shown.

NO_DIAGMICROCODE_MENU "%1$6X \
%2$s DIAGNOSTIC MICROCODE IS MISSING\n\n\
The %3$s diagnostic microcode is either missing\n\
or not accessible.\n\n\
This microcode is necessary in order to run diagnostics \n\
on the %4$s."
$
$ %1$6X is the 6 hexadecimal digits representing the menu goal number.
$ %2$s is the description of the resource in uppercase (SCSI ADAPTER).
$ %3$s is the description of the resource in lowercase (SCSI adapter).
$ %4$s is similar %3s
$ Leave line spacing as shown.  See general notes on length of title.
$ Do not translate the 6 digits number at the beginning of the message.
$ Leave it exactly as shown.

NO_DDFILE_MENU "%1$6X \
%2$s DEVICE DRIVER IS MISSING\n\n\
The %3$s device driver is either missing\n\
or not accessible.\n\n\
This device driver is necessary in order to use the %4$s\n\
in normal system operations."
$
$ %1$6X is the 6 hexadecimal digits representing the menu goal number.
$ %2$s is the description of the resource in uppercase (SCSI ADAPTER).
$ %3$s is the description of the resource in lowercase (SCSI adapter).
$ Leave line spacing as shown.  See general notes on length of title.
$ Do not translate the 6 digits number at the beginning of the message.
$ Leave it exactly as shown.

NO_HOT_KEY "* * * WARNING! * * *\n\n\
DO NOT ATTEMPT TO CHANGE TO ANOTHER WINDOW \n\
UNTIL THIS TEST IS COMPLETED.\n\n\
Changing windows while this test is running may cause\n\
unpredictable results to occur.\n\n\
Press Enter to continue or `Cancel' to exit."

DEVICE_INITIAL_STATE_FAILURE     "%1$6X \
RESOURCE COULD NOT BE RESTORED TO INITIAL STATE\n\n\
No trouble was found.  However, resource %2$s could not \n\
be restored to its initial configured state. \n\n\
To restore the resource to its initial state, run the\n\
following command from the command line:\n\
    rmdev -l %3$s -d\n\
Then run cfgmgr to reconfigure the system:\n\
    cfgmgr\n\n\
If you are experiencing problems with your system after\n\
executing the commands listed above, it may be necessarey\n\
to shut down and reboot the system to properly configure\n\
all resources."

$
$ %1$6X is the 6 hexadecimal digits representing the menu goal number.
$ %2$s and %3$s is the resource name.
$ This menu goal is displayed when the resource status cannot
$ be restored after testing it.
$

$set MSG_SET_48
$ ---------------------------------------------------------------
$ Reason code set used by device type reserved

$set RESRV_MSG_SET_49
$ ---------------------------------------------------------------
$ Reason code set used by device type "reserved"

$set SGA_MSG_SET_50
$ ---------------------------------------------------------------
$ Reason code set used by device type "salmon graphics adapter"

R_SGA "Adapter card test failed."

$set STILWELL_ETH_MSG_SET_51
$ ---------------------------------------------------------------
$ Reason code set used by device type "stilwell ethernet"

STILETH_MSG_101 "POS Register Failed."
$ POS stands for Programmable Option Select.
STILETH_MSG_102 "I/O Registers Test Failed."
$ I/O stands for Input/Output.
STILETH_MSG_103 "Local RAM Test Failed."
STILETH_MSG_104 "Vital Product Data Test Failed."
STILETH_MSG_105 "LAN Coprocessor 82596 Internal Test Failed."
STILETH_MSG_106 "Internal Loopback Test Failed."
STILETH_MSG_TT "Thick And Thin Riser Card."
STILETH_MSG_107 "External Loopback (DIX Connector) Test Failed."
STILETH_MSG_TP "Twisted Pair Riser Card."
STILETH_MSG_108 "External Loopback (BNC Connector) Test Failed."
STILETH_MSG_109 "External Loopback Parity Test Failed."
STILETH_MSG_110 "External Loopback Fairness Test Failed."
STILETH_MSG_111 "External Loopback Parity And Fairness Test Failed."
STILETH_MSG_112 "External Loopback (Twisted Pair Connector) Test Failed."
STILETH_MSG_113 "External Loopback (Twisted Pair Connector) Parity Test Failed."
STILETH_MSG_114 "External Loopback (Twisted Pair Connector) Fairness Test Failed."
STILETH_MSG_115 "External Loopback (Twisted Pair Connector) Parity And Fairness Test Failed."
STILETH_MSG_116 "Twisted Pair Wrap Data Failed."
STILETH_MSG_117 "Device Configuration Failed."
STILETH_MSG_SF "Software Error."
STILETH_MSG_118 "Device Driver Indicates A Hardware Problem."
STILETH_MSG_120 "Riser Card Determination Failed."
STILETH_MSG_121 "Ethernet Transceiver (Twisted Pair)  Test Failed."
STILETH_MSG_TX_T "10BASE-T Transceiver (Twisted Pair) Failed."
STILETH_MSG_122 "Ethernet Transceiver (Thick And Thin) Test Failed."
STILETH_MSG_TX_2 "10BASE-2 Transceiver (Thick And Thin) Failed."
STILETH_MSG_123 "Internal Loopback Test (Twisted Pair ) Failed."
STILETH_MSG_124 "Error Log Analysis Indicates A Hardware Problem."
STILETH_MSG_BAD_FUSE "Fuse Test Failed"
BAD_FUSE "Bad Fuse"


$set FDDI_MSG_SET_52
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
FDDI_EXTENDER_CARD "Extender Adapter"
FDDI_CROSSOVER_CABLE "Crossover Cable"
FDDI_SOFTWARE_ERROR "Software"
FDDI_CONFIG_FAIL "Device Configuration Failure"
FDDI_OPEN_FAIL "Device Driver Indicates A Hardware Failure"
FDDI_ERROR_LOG "Error Log Indicates A Fardware Failure"
FDDI_MSG_999 "CPU Planar"

$
$set MSG_SET_53
$ ----------------------------------------------------------------------------
$ Reason code set used by device type "reserved"
$

$set MSG_SET_54
$ ----------------------------------------------------------------------------
$ Reason code set used by device type "reserved"
$

$set MSG_SET_55
$ ----------------------------------------------------------------------------
$ Reason code set used by device type "reserved"
$

$set MSG_SET_56
$ ----------------------------------------------------------------------------
$ Reason code set used by device type "reserved"
$

$set SCSITAPES_MSG_SET_57
$quote "
$ ----------------------------------------------------------------------------
$ Reason code set used by device type "TAPE"
$ Name for tile line.
TAPE_DRIVE "TAPE DRIVE"
$ $
$ $ Tape DA FRU bucket reason messages (rmsg). */
$ $
$ SRN FFC-110
DTAPE_RESERVE "The Reserve Unit command failed."
$
$ SRN FFC-120
DTAPE_INQUIRY "The Inquiry command failed."
$
$  SRN FFC-130/180
DTAPE_LOAD "The Load Unit command failed."
$
$  SRN FFC-135/185
DTAPE_UNLOAD "The Unload Unit command failed."
$
$ SRN FFC-140/190
DTAPE_MODE_SELECT "The Mode Select command failed."
$
$ SRN FFC-150/200
DTAPE_TEST_UNIT_READY "The Test Unit Ready command failed."
$
$ SRN FFC-160
DTAPE_SEND_DIAGNOSTICS "The Send Diagnostics command failed."
$
$ SRN FFC-170
DTAPE_TAR_W_R_COMPARE "The Read, Write and Compare test failed."
$
$ SRN FFC-210
DTAPE_WRITE_PROTECT "The Write Protect test failed."
$
$ SRN FFC-220
DTAPE_RELEASE "The Release Unit command failed."
$
$ SRN FFC-230
DTAPE_REQUEST_SENSE "The Request Sense command failed."
$
$ SRN FFC-240
DTAPE_OPENX_FAILED "The Diagnostic Open command failed."
$
$ SRN FFC-300
DTAPE_CFG_DEVICE "The Device Configuration test failed."
$
$ SRN FFC-310
DTAPE_CFG_PARENT "The SCSI Adapter Configuration test failed."
$
$ SRN FFC-320
DTAPE_ELA_SRN "Error Log Analysis indicates a permanent hardware failure."
$
$ SRN FFC-4XX (XX == 11 - 22)
DTAPE_RESERVATION "A Reservation conflict occurred."
$
$ SRN FFC-5XX (XX == 11 - 22)
DTAPE_NONEXTENDED "The drive returned bad or non-extended sense data."
$
$ SRN FFC-6XX (XX == 11 - 22)
DTAPE_BUS_IO "An Adapter or Bus I/O error occurred."
$
$ SRN FFC-7XX (XX == 11 - 22)
DTAPE_TIMEOUT "A device timeout occurred."
$ $
$ $ Tape DA failing FRU messages (fmsg).
$ $
DTAPE_MEDIA "Test media."
$
DTAPE_SW_ERROR "Software error."
$
$ $
$ $ Tape DA failing FRU names (fname).
$ $
$ Test media name.
DTAPE_TAPE "Tape"
$
$ Software error name.
DTAPE_SW "SW"
$
$

$set NSCA_MSG_SET_58
$quote "
$ ----------------------------------------------------------------------------
NSCA_100     "Unexpected results from test"
NSCA_101     "Memory failure"
NSCA_103     "Processor test failure"
NSCA_104     "Microchannel interface test failure"
NSCA_105     "DMA test failure"
NSCA_106     "Controller test failure"
NSCA_108     "Processor storage test failure"
NSCA_109     "Memory controller test failure"
NSCA_110     "Microchannel interface controller wrap test failure"
NSCA_111     "On chip monitor testing of Microchannel interface\n\
controller failed"
NSCA_112     "Microchannel DMA test failed"
NSCA_113     "VPD CRC test failed"
NSCA_117     "Error log analysis indicated a hardware failure"
NSCA_118     "Device could not be configured"
NSCA_119     "A software error occurred"
OP_WRAP      "Optical wrap"
NSCA_FOSA    "Fibre Optic Sub-Assembly"
NSCA_SOFTWARE "Software"

$set MSG_SET_59
$ ----------------------------------------------------------------------------
$ Reason code set used by device reserved
$

$set MSG_SET_60
$ ----------------------------------------------------------------------------
$ Reason code set used by 3rd party vendors
$

$set MSG_SET_61
$ ----------------------------------------------------------------------------
$ Reason code set used by device type "Rs232 128 port"
$ $ Do not add any messages before or between existing messages.
$ $ Blank messages are place holders.
$
DC_232_128      "128-Port Async Controller Port"
DM16_CBPN_2     "16-Port Remote Async Node"
DM16_ACPN_3     "Asynchronous Cable EIA-232, Part Number 6323741"
DM16_CIPN_4     "Printer/Terminal Interposer, Part Number 58F2861"
DM16_5          ""
DM16_CCPN_6     "128-Port Controller Cable, Part Number 43G0937 (15 foot), Part Number 43G0936 (9 inch)"
DM16_CCPN_7     "RJ-45 to DB-25 Converter Cable, Part Number 51G8610"
DM16_8          "128-Port Async Controller test failed."
DM16_9          "16-Port Remote Async Node test failed."
DM16_TU10       "128-port Async Controller VPD test failed."
DM16_TU20       "128-port Async Controller register verification test failed."
DM16_TU30       "16-port Remote Async Node VPD test failed."
DM16_TU40       "128-port Controller wrap test failed."
DM16_TU50       "16-Port Remote Async Node wrap test failed."
DM16_TU60       "RJ-45 to DB-25 converter cable wrap test failed."
DM16_TU70       "Asynchronous cable wrap test failed."
DM16_TU80       "Interposer wrap test failed."
DM16_LM         "128-port Async Controller line test failed."
DM16_7C         ""
DM16_8C         ""
DM16_TU60C      "Pinout converter wrap test failed"
DM16_9C         "64-port to 128-port Pinout Converter, Part Number 88G3651"
DM16_23         ""
DM16_24         ""
DM16_PTCPN_25   "Printer/Terminal Cable, Part Number 12H1204"

$set DASYNC_8_MSG_SET_62
$ ---------------------------------------------------------------
$ Reason code set used by device type "IBM 8-Port Async, EIA-232 (ISA)"
$ $ Do not add any messages before or between existing messages.
$ $ Blank messages are place holders.

DC_232_8ISA	"8-Port Async, EIA-232 (ISA)"
$ Async stands for asynchronous communications.
DM8ISA_ICPN_2	"Multiport Interface Cable, Part Number 11H5967"
DM8ISA_ACPN_3	"Asynchronous Cable EIA-232, Part Number 6323741"
DM8ISA_CIPN_4	"Printer/Terminal interposer, Part Number 58F2861"
DM8ISA_5	""
DM8ISA_ECPN_6	""
DM8ISA_7	""
DM8ISA_8	""
DM8ISA_9	""
DM8ISA_TU10	"8-port Async, EIA-232 (ISA) register test failed."
$ Async stands for asynchronous communications.
DM8ISA_TU20	"8-port Async, EIA-232 (ISA) local chip wrap test failed."
$ Async stands for asynchronous communications.
DM8ISA_TU30	"8-port Async, EIA-232 (ISA) driver wrap test failed."
$ Async stands for asynchronous communications.
DM8ISA_TU40	""
DM8ISA_TU50	"Multiport interface cable wrap test failed."
DM8ISA_TU60	""
DM8ISA_TU70	"Asynchronous cable wrap test failed."
DM8ISA_TU80	"Interposer wrap test failed."
DM8ISA_LM	"8-port Async, EIA-232 (ISA) adapter test failed."
DM8ISA_PN19	""
DM8ISA_PN20	""
DM8ISA_TU60C	""
DM8ISA_PN22	""
DM8ISA_PN23	""
DM8ISA_CPN24	""
DM8ISA_PTCPN_25	"Printer/Terminal Cable, Part Number 12H1204"
