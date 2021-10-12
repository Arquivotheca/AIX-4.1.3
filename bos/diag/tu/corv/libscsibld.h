/* @(#)33       1.1  src/bos/diag/tu/corv/libscsibld.h, tu_corv, bos411, 9428A410j 7/22/93 18:58:31 */
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
#ifndef SCSIBLD
#define SCSIBLD

#include "libcmdbld.h"

#define SCSI_COMMAND COMMAND


/* Constants for text error messages */

/* Total number of sense descriptions */
#define ASC_ENTRIES 120

typedef struct {
     int additional_sense_code;
     int additional_sense_qualifier;
     char description[100];
} ASC_and_ASCQ;
                            /*    ASC   ASCQ  DESCRIPTION                       */
ASC_and_ASCQ sense_errors[] = { { 0x00, 0x00, "No Additional Sense Information" },
                                { 0x00, 0x06, "I/O Process Terminated" },
                                { 0x01, 0x00, "No Index/Sector Signal" },
                                { 0x02, 0x00, "No Seek Complete" },
                                { 0x03, 0x00, "Peripheral Device Write Fault" },
                                { 0x04, 0x00, "Logical Unit Not Ready, Cause Not Reportable" },
                                { 0x04, 0x01, "Logical Unit is in Process of Becoming Ready" },
                                { 0x04, 0x02, "Logical Unit Not Ready, Initializing Command Required" },
                                { 0x04, 0x03, "Logical Unit Not Ready, Manual Intervention Required" },
                                { 0x04, 0x04, "Logical Unit Not Ready, Format In Progress" },
                                { 0x05, 0x00, "Logical Unit does not Respone to Selection" },
                                { 0x06, 0x00, "No Reference Position Found" },
                                { 0x07, 0x00, "Multiple Peripheral devices selected" },
                                { 0x08, 0x00, "Logical Unit Communication Failure" },
                                { 0x08, 0x01, "Logical Unit Communication Time-Out" },
                                { 0x08, 0x02, "Logical Unit Communication Parity Error" },
                                { 0x0a, 0x00, "Error Log Overflow" },
                                { 0x0c, 0x01, "Write Error Recorvered with Auto Reallocation" },
                                { 0x0c, 0x02, "Write Error - Auto Reallocation Failed" },
                                { 0x10, 0x00, "ID CRC or ECC Error" },
                                { 0x11, 0x00, "Unrecovered Read Error" },
                                { 0x11, 0x01, "Read Retries Exhausted" },
                                { 0x11, 0x02, "Error Too Long to Correct" },
                                { 0x11, 0x03, "Multiple Read Errors" },
                                { 0x11, 0x04, "Unrecovered Read Error - Auto Reallocate Failed" },
                                { 0x11, 0x0a, "Miscorrected Error" },
                                { 0x11, 0x0b, "Unrecovered Read Error - Recommend Reassignment" },
                                { 0x11, 0x0c, "Unrecovered Read Error - Recommend Rewrite the Data" },
                                { 0x12, 0x00, "Address Mark Not Found for ID Field" },
                                { 0x13, 0x00, "Address Mark Not Found for Data Field" },
                                { 0x14, 0x00, "Recorded Entity Not Found" },
                                { 0x14, 0x01, "Record Not Found" },
                                { 0x15, 0x00, "Random Positioning Error" },
                                { 0x15, 0x01, "Mechanical Positioning Error" },
                                { 0x15, 0x02, "Positioning Error Detected by Read of Medium" },
                                { 0x16, 0x00, "Data Synchronization Mark Error" },
                                { 0x17, 0x00, "Recovered Data with No Error Correction Applied" },
                                { 0x17, 0x01, "Recovered Data with Retries" },
                                { 0x17, 0x02, "Recovered Data with Positive Head Offset" },
                                { 0x17, 0x03, "Recovered Data with Negative Head Offset" },
                                { 0x17, 0x04, "Recovered Data with Retries and/or CIRC Applied" },
                                { 0x17, 0x05, "Recovered Data Using Previous Sector ID" },
                                { 0x17, 0x06, "Recovered Data without ECC - Data Auto-Reallocated" },
                                { 0x17, 0x07, "Recovered Data Without ECC - Recommend Reassignment" },
                                { 0x18, 0x00, "Recovered Data with Error Correction Applied" },
                                { 0x18, 0x01, "Recovered Data with Error Correction & Retries Applied" },
                                { 0x18, 0x02, "Recovered Data - Data Auto-Reallocated" },
                                { 0x18, 0x05, "Recovered Data - Recommend Reassignment" },
                                { 0x19, 0x00, "Defect List Error" },
                                { 0x19, 0x01, "Defect List Not Available" },
                                { 0x19, 0x02, "Defect List Error in Primary List" },
                                { 0x19, 0x03, "Defect List Error in Grown List" },
                                { 0x1a, 0x00, "Parameter List length Error" },
                                { 0x1b, 0x00, "Synchronous Data Transfer Error" },
                                { 0x1c, 0x00, "Defect List Not Found" },
                                { 0x1c, 0x01, "Primary Defect List Not Found" },
                                { 0x1c, 0x02, "Grown Defect List Not Found" },
                                { 0x1d, 0x00, "Miscompare During Verify Operation" },
                                { 0x1e, 0x00, "Recovered ID with ECC Correction" },
                                { 0x20, 0x00, "Invalid Command Operation Code" },
                                { 0x21, 0x00, "Logical Block Address Out of Range" },
                                { 0x22, 0x00, "Illegal Function (should use 20 00, 24 00, or 26 00)" },
                                { 0x24, 0x00, "Invalid Field in CDB" },
                                { 0x25, 0x00, "Logical Unit Not Supported" },
                                { 0x26, 0x00, "Invalid Field in Paramter List" },
                                { 0x26, 0x01, "Parameter Not Supported" },
                                { 0x26, 0x02, "Parameter Value Invalid" },
                                { 0x26, 0x03, "Threshold Parameters Not Supported" },
                                { 0x27, 0x00, "Write Protected" },
                                { 0x28, 0x00, "Not Ready to Ready Transition (Medium may have changed)" },
                                { 0x29, 0x00, "Power On, Reset, or Bus Device Reset Occurred" },
                                { 0x2a, 0x00, "Parameters Changed" },
                                { 0x2a, 0x01, "Mode Parameters Changed" },
                                { 0x2a, 0x02, "Log Parameters Changed" },
                                { 0x2b, 0x00, "Copy Cannot Execute since Host cannot Disconnect" },
                                { 0x2c, 0x00, "Command Sequence Error" },
                                { 0x2f, 0x00, "Commands Cleared by another Initiator" },
                                { 0x30, 0x00, "Incompatible Medium Installed" },
                                { 0x30, 0x01, "Cannot Read Medium - Unknown Formation" },
                                { 0x30, 0x02, "Cannot Read Medium - Incompatible Format" },
                                { 0x30, 0x03, "Cleaning Cartridge Installed" },
                                { 0x31, 0x00, "Medium Format Corrupted" },
                                { 0x31, 0x01, "Format Command Failed" },
                                { 0x32, 0x00, "No Defect Spare Location Available" },
                                { 0x32, 0x01, "Defect List Update Failure" },
                                { 0x37, 0x00, "Rounded Paramter" },
                                { 0x39, 0x00, "Saving Parameters Not Supported" },
                                { 0x3d, 0x00, "Invalid Bits in Identify Message" },
                                { 0x3e, 0x00, "Logical Unit has Not Self-Configured Yet" },
                                { 0x3f, 0x00, "Target Operating Conditions have Changed" },
                                { 0x3f, 0x01, "Microcode has been Changed" },
                                { 0x3f, 0x02, "Changed Operating Definition" },
                                { 0x3f, 0x03, "Inquiry Data has Changed" },
                                { 0x40, 0x00, "Diagnostic Failure in Component" },
                                { 0x41, 0x00, "Data Path Failure" },
                                { 0x42, 0x00, "Power-On or Self-Test Failure" },
                                { 0x43, 0x00, "Message Error" },
                                { 0x44, 0x00, "Internal Target Failure" },
                                { 0x45, 0x00, "Select or Reslect Failure" },
                                { 0x46, 0x00, "Unsuccessful Soft Reset" },
                                { 0x47, 0x00, "SCSI Parity Error" },
                                { 0x48, 0x00, "Initiator Detected Error Message Received" },
                                { 0x49, 0x00, "Invalid Message Error" },
                                { 0x4a, 0x00, "Command Phase Error" },
                                { 0x4b, 0x00, "Data Phase Error" },
                                { 0x4c, 0x00, "Logical Unit Failed Self-configuration" },
                                { 0x4e, 0x00, "Overlapped Commands Attempted" },
                                { 0x53, 0x00, "Media Load or Eject Failed" },
                                { 0x53, 0x02, "Medium Removal Prevented" },
                                { 0x5a, 0x00, "Operator Request or State Change Input (Unspecified)" },
                                { 0x5a, 0x01, "Operator Medium Removal Request" },
                                { 0x5a, 0x02, "Operator Selected Write Protect" },
                                { 0x5a, 0x03, "Operator Selected Write Permit" },
                                { 0x5b, 0x00, "Log Exception" },
                                { 0x5b, 0x01, "Threshold Condition Met" },
                                { 0x5b, 0x02, "Log Counter at Maximum" },
                                { 0x5b, 0x03, "Log List Codes Exhausted" },
                                { 0x5c, 0x00, "RPL Status Change" },
                                { 0x5c, 0x01, "Spindles Synchronized" },
                                { 0x5c, 0x02, "Spindles Not Synchronized" } };

char Sense_Key[][30] = {        "No Sense",
                                "Recovered Error",
                                "Not Ready",
                                "Medium Error",
                                "Hardware Error",
                                "Illegal Request",
                                "Unit Attention",
                                "Data Protect",
                                "Blank Check",
                                "Vendor Specific",
                                "Copy Aborted",
                                "Aborted Command",
                                "Equal",
                                "Volumne Overflow",
                                "Miscompare",
                                "Reserved" };


#endif
