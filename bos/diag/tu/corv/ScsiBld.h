/* @(#)31       1.1  R2/cmd/diag/tu/corv/ScsiBld.h, tu_corv, bos325 7/22/93 18:57:16 */
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

#include "CmdBld.h"

#define _CHANGE_DEFINITION 0x40
#define _COMPARE 0x39
#define _COPY 0x18
#define _COPY_AND_VERIFY 0x3a
#define _FORMAT_UNIT 0x04
#define _INQUIRY 0x12
#define _LOCK_UNLOCK_CACHE 0x36
#define _LOG_SELECT 0x4c
#define _LOG_SENSE 0x4d
#define _MODE_SELECT_6 0x15
#define _MODE_SELECT_10 0x55
#define _MODE_SENSE_6 0x1a
#define _MODE_SENSE_10 0x5a
#define _PRE_FETCH 0x34
#define _PREVENT_ALLOW_MEDIUM_REMOVAL 0x1e
#define _READ_6 0x08
#define _READ_10 0x28
#define _READ_BUFFER 0x3c
#define _READ_CAPACITY 0x25
#define _READ_DEFECT_DATA 0x37
#define _READ_LONG 0x3e
#define _REASSIGN_BLOCKS 0x07
#define _RECEIVE_DIAGNOSTIC_RESULTS 0x1c
#define _RELEASE 0x17
#define _REQUEST_SENSE 0x03
#define _RESERVE 0x16
#define _REZERO_UNIT 0x01
#define _SEARCH_DATA_EQUAL 0x31
#define _SEARCH_DATA_HIGH 0x30
#define _SEARCH_DATA_LOW 0x32
#define _SEEK_6 0x0b
#define _SEEK_10 0x2b
#define _SEND_DIAGNOSTIC 0x1d
#define _SET_LIMITS 0x33
#define _START_STOP_UNIT 0x1b
#define _SYNCHRONIZE_CACHE 0x35
#define _TEST_UNIT_READY 0x00
#define _VERIFY 0x2f
#define _WRITE_6 0x0a
#define _WRITE_10 0x2a
#define _WRITE_AND_VERIFY 0x2e
#define _WRITE_BUFFER 0x38
#define _WRITE_LONG 0x3f
#define _WRITE_SAME 0x41

#define ONE_BLOCK 512
#define _LUN_0 0
#define _LUN_1 1
#define _LUN_2 2
#define _LUN_3 3
#define _LUN_4 4
#define _LUN_5 5
#define _LUN_6 6
#define _LUN_7 7

#define _SCSI_ID_0 0
#define _SCSI_ID_1 1
#define _SCSI_ID_2 2
#define _SCSI_ID_3 3
#define _SCSI_ID_4 4
#define _SCSI_ID_5 5
#define _SCSI_ID_6 6
#define _SCSI_ID_7 7

#define scsi_format_unit_command(b,c,d,e,f,g,h,i) build_cmd("%8%3%1%1%3%8%8%8%8%",6,_FORMAT_UNIT,b,c,d,e,f,g,h,i)
#define scsi_inquiry_command(b,c,d,e,f,g,h) build_cmd("%8%3%4%1%8%8%8%8%",6,_INQUIRY,b,c,d,e,f,g,h)
#define scsi_mode_select_6(b,c,d,e,f,g,h,i) build_cmd("%8%3%1%3%1%8%8%8%8%",6,_MODE_SELECT_6,b,c,d,e,f,g,h,i)
#define scsi_read_6(b,c,d,e) build_cmd("%8%3%21%8%8%",6,_READ_6,b,c,d,e)
#define scsi_request_sense(b,c) build_cmd("%8%3%5%8%8%8%",6,_REQUEST_SENSE,b,0,0,0,c);
#define scsi_test_unit_ready(b) build_cmd("%8%3%5%",6,_TEST_UNIT_READY,b,0);
#define scsi_write_6(b,c,d,e) build_cmd("%8%3%21%8%8%",6,_WRITE_6,b,c,d,e)


#define SCSI_COMMAND COMMAND

#endif
