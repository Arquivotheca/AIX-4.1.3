/* @(#)23       1.2  src/bos/diag/tu/corv/CorvetteLib.h, tu_corv, bos411, 9428A410j 9/1/93 20:18:45 */
/*
 *   COMPONENT_NAME: TU_CORV
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _CORVETTE
#define _CORVETTE


typedef char CORVETTE_STRUCT;


/* The following reserved words describe the values used within
   the CONFIGURE_DELIVER_PIPES data structure                   */

#define _MANAGEMENT_REQUEST 0x9
#define _MOVE_MODE_SIGNAL 0xd

#define _RESERVED 0
#define CEN 1
#define _SET_A 0
#define _SET_B 1
#define _SELECT_ROM_PAGE 0

/* Defined ranges for Basic Control Register */
#define _ENABLE_SYSTEM_INTERRUPT 1
#define _DISABLE_SYSTEM_INTERRUPT 0
#define _ENABLE_BUS_MASTER_DMA 1
#define _DISABLE_BUS_MASTER_DMA 0
#define _RESET_EXCEPTION_CONDITION 1
#define _ENABLE_CLEAR_ON_READ 1
#define _DISABLE_CLEAR_ON_READ 0
#define _START_SUBSYSTEM_RESET 1
#define _DISABLE_SUBSYSTEM_RESET 0

/* Defined ranges for POS Register 2: Access Control */
#define _BIOS_ADDRESS_NOT_USED 0

/* Defined ranges for POS Register 4: I/O Control */
#define _ENABLE_BIOS_ROM 1
#define _DISABLE_BIOS_ROM 0
#define _ENABLE_MOVE_MODE 1
#define _DISABLE_MOVE_MODE 0
#define _SCSI_ID_6 6

/* Defined ranges for POS Register 5: Master Control */
#define _ENABLE_STREAMING 1
#define _DISABLE_STREAMING 0
#define _ENABLE_RETURN_CHECKING 1
#define _DISABLE_RETURN_CHECKING 0
#define _ENABLE_DATA_PARITY 1
#define _DISABLE_DATA_PARITY 0
#define _ENABLE_WAIT_STATE 1
#define _DISABLE_WAIT_STATE 0
#define _CONCAT_IO_ADDRESS 1
#define _SELECT_IO_RANGE 0

/* Defined ranges for POS Register 3B: DMA Control */
#define _NO_ADDRESS_BOUNDARY 0
#define _32_BYTE_ADDRESS_BOUNDARY 1
#define _64_BYTE_ADDRESS_BOUNDARY 2
#define _128_BYTE_ADDRESS_BOUNDARY 3
#define _RELEASE_AT_NEXT_TRANSFER 0
#define _2_MICROSECOND_RELEASE_DELAY 1
#define _4_MICROSECOND_RELEASE_DELAY 2
#define _6_MICROSECOND_RELEASE_DELAY 3

/* Defined ranges for POS Register 4B: SCSI Bus Control */
#define _ENABLE_EXTERNAL_FAST_SCSI 1
#define _DISABLE_EXTERNAL_FAST_SCSI 0
#define _ENABLE_COMPATIBILITY_MODE 0
#define _DISABLE_COMPATIBILITY_MODE 1
#define _ENABLE_SCSI_POSTED_WRITES 1
#define _DISABLE_SCSI_POSTED_WRITES 0
#define _ENABLE_EXTERNAL_TERMINATOR 1
#define _DISABLE_EXTERNAL_TERMINATOR 0
#define _ENABLE_SCSI_DISCONNECT 0
#define _DISABLE_SCSI_DISCONNECT 1
#define _ENABLE_TARGET_MODE 0
#define _DISABLE_TARGET_MODE 1

/* Defined ranges for IO Register 7: Basic Status Register */
#define _CIR_EMPTY 0x04
#define _CIR_FULL 0x08

/* Character constants for error message formatting */
char TSB_SCSI_Status[][100] = {  "Good Status (no error)",
                                "Check Condition(error)",
                                "Condition Met/Good (no error)",
                                "reserved",
                                "Busy (command rejected)",
                                "reserved",
                                "reserved",
                                "reserved",
                                "Intermediate/Good (no error)",
                                "reserved",
                                "Intermediate/Condition Met/Good (no error)",
                                "reserved",
                                "Reservation Conflict",
                                "reserved",
                                "reserved",
                                "reserved" };
char TSB_Cmd_Status[][100] = {  "Reserved",
                                "SCB Command Completed With Success",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "SCB Command Completed with Success With Retries",
                                "Reserved",
                                "Adapter Hardware Failure",
                                "Reserved",
                                "Reserved",
                                "Immediate Command Complete",
                                "Reserved",
                                "Command Completed With Failure",
                                "Reserved",
                                "Command Error (invalid command or parameter)",
                                "Software Sequency Error",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved" };

char TSB_Device_Code[][100] = { "No Error",
                                "SCSI Bus Reset Occured",
                                "SCSI Interface Fault",
                                "SCSI Device Posted Write Error (deferred error)",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "SCSI Selection Timeout (device not available)",
                                "Unexpected SCSI Bus Free",
                                "Mandatory SCSI Message Rejected",
                                "Invalid SCSI Phase Sequence",
                                "Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved",
                                "Short Length Record Error",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Bus Reset Msg Received (Target Mode)",
                                "Abort Msg Receive (Target Mode)",
                                "Target Cmd Overrun (Target Mode)",
                                "A data allocation error occurred (Target Mode)",
                                "A SCSI initiator error was detected (Target Mode)",
                                "A SCSI interface fault occurred (Target Mode)" };

char TSB_Cmd_Code[][100] = {    "No error",
                                "Invalid Parameter in SCB",
                                "Reserved",
                                "Command Not Supported",
                                "Command Aborted (by system)",
                                "Reserved",
                                "Cmd Rejected - Adapter Diagnostic Failure",
                                "Format Rejected - Sequence Error (requires pre-formate cmd)",
                                "Assign Rejected - Command in Progress on Device",
                                "Assign Rejected - SCSI Device Already Assigned",
                                "Cmd Rejected - SCSI device not assigned",
                                "Cmd Rejected - Maximum LBA Exceeded",
                                "Cmd Rejected - 16-bit Card Slot Address Range Exceeded",
                                "Reserved","Reserved",
                                "Cmd Rejected - Suspended SCSI Queue(s) and Adapter Queue Full",
                                "Reserved","Reserved",
                                "Assign Rejected - SCSI Bus Number Not Supported",
                                "Invalid Device for Command",
                                "Checksum/Header Error in EPROM Data",
                                "Memory Erasure Error - Even byte",
                                "Memory Erasure Error - Odd byte",
                                "Memory Verify Error - Even byte",
                                "Memory Verify Error - Odd byte",
                                "Download Microcode Rejected - Sequence Error (requires prepare)",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Adapter Hardware Error",
                                "Adapter Detected Global Command Timeout",
                                "DMA error",
                                "Reserved",
                                "Command Aborted By Adapter",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved",
                                "Target Assign Error (Target Mode)",
                                "Wrong Cmd Sent by Host for Current Mode (Target Mode)",
                                "Data Enable SCB Error (Target Mode)",
                                "Enable/End SCB Error (Target Mode)",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Internal Microcode Detected Error",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
                                "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved" };


#endif
