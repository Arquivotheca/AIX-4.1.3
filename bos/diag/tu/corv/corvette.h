/* @(#)35	1.3.1.1  src/bos/diag/tu/corv/corvette.h, tu_corv, bos41J, 9511A_all 2/28/95 16:40:42 */
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
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef CORVETTEBLD 

#define CORVETTEBLD 
#include "ScsiBld.h"
#include "CmdBld.h" 
#include "Adapters.h"
#include <sys/types.h>

#define _CORVETTE

#ifndef NULL
#define NULL 0
#endif

#define _DEVICE_STATUS_AVAILABLE 0x02
#define _RESERVED 0x00
#define _BUS_ID_0 0
#define _BUS_ID_1 1
#define _IMMEDIATE_SUCCESS 0xa
#define _SCB_SUCCESS 0x1
#define _COMMAND_FAILURE 0xc
#define _COMMAND_ERROR 0xe
#define SUCCESSFUL 0
#define FAILED -1
#define _ERROR_CODE 17
#define _INVALID_DEVICE 0x13

#define corvette_download_prepare() build_cmd("%32",4, 0x1e04aa55)

#define corvette_immediate_assign(a,b,c,d,e,f,g) build_cmd("%16%1%3%4%1%3%1%3", 4, 0x0e04, d, e, f, g, a, b, c) 
#define _INTERNAL_BUS 0x00
#define _EXTERNAL_BUS 0x01
#define _MIN_INVALID_2 0x02
#define _MAX_INVALID_7 0x07
#define _LOWER_SCSI_ADDRESS_MODE 0
#define _UPPER_SCSI_ADDRESS_MODE 1
#define _REMOVE 1
#define _ASSIGN 0
#define _DEVICE_ID_0 0
#define _DEVICE_ID_1 1
#define _DEVICE_ID_2 2
#define _DEVICE_ID_3 3
#define _DEVICE_ID_4 4
#define _DEVICE_ID_5 5
#define _DEVICE_ID_6 6
#define _DEVICE_ID_7 7
#define _DEVICE_ID_8 8
#define _DEVICE_ID_9 9
#define _DEVICE_ID_10 10
#define _DEVICE_ID_11 11
#define _DEVICE_ID_12 12
#define _DEVICE_ID_13 13
#define _DEVICE_ID_14 14
#define _DEVICE_ID_15 15
#define _INITIATOR_MODE_DEVICE 0
#define _TARGET_MODE_DEVICE 1

#define _AUTO_TSB 0
#define _TSB_ON_ERROR 1
#define _UNSUPPRESSED_DATA 0
#define _SUPPRESSED_DATA 1
#define _BUFFER_BYTE_COUNT_0 0
#define _BUFFER_BYTE_COUNT_N 5
#define _BUFFER_BYTE_COUNT_20 20
#define _BUFFER_BYTE_COUNT_40 40
#define _BUFFER_BYTE_COUNT_250 250
#define _BUFFER_BYTE_COUNT_255 255
#define _BUFFER_BYTE_COUNT_256 256 
#define _BUFFER_BYTE_COUNT_2560 2560
#define _BUFFER_BYTE_COUNT_MAX 16000000

#define _SCB_CHAIN_ADDR_0 0x00000000
#define _SCB_CHAIN_ADDR_N
#define _WRITE 0
#define _READ 1
#define _POINTER_TO_LIST_0 0
#define _POINTER_TO_LIST_1 1
#define _BYPASS_BUFFER_0 0
#define _BYPASS_BUFFER_1 1
#define _CACHE_PREFETCH_0 0
#define _CACHE_PREFETCH_1 1
#define _LOOP_SCATTER_GATHER_0 0
#define _LOOP_SCATTER_GATHER_1 1
 
#define _LBA_0 0
#define _LBA_0x000FFFFF 0x000FFFFF
#define _LBA_0x00000FFF 0x00000FFF
#define _LBA_MAX 0x1954FC

#define _BLOCK_COUNT_1 1
#define _BLOCK_COUNT_4 4
#define _BLOCK_COUNT_0 0
#define _BLOCK_COUNT_5 5
#define _BLOCK_COUNT_MAX 0x7A12
#define _BLOCK_LENGTH_0x200 0x200
#define _POINTER_TO_LIST_0 0
#define _POINTER_TO_LIST_1 1


#define corvette_read_adapter_local_RAM(a,b,c,d,e,f,g) build_cmd("%16%8%1%1%1%5%32s%32s%32s%32s%32s%32",28,0x1c1c,0x00,1,a,b,0x02,c,d,e,f,g,0x0000)
#define _RETRY_ENABLE 1
#define _RETRY_DISABLE 0
#define corvette_write_adapter_local_RAM(a,b,c,d,e,f,g) build_cmd("%16%8%1%1%1%5%32s%32s%32s%32s%32s%32",28,0x1d1c,0x00,0,a,b,0x02,c,d,e,f,g,0x0000)
#define corvette_download_microcode(a,b,c,d,e,f,g) build_cmd("%16%8%1%1%4%1%1%32s%32s%32s%32s%32s%32s", 28, 0x1b1c,0x00,0,a,0x00,1,0,b, c,d,e,f,g) 
#define _BYTE_COUNT_0 0
#define _BYTE_COUNT_9 9 
#define _BYTE_COUNT_10 10
#define _BYTE_COUNT_20 20
#define corvette_request_sense(a,b,c,d,e,f,g,h) build_cmd("%16%8%1%1%3%1%1%1%32s%32s%32s%32s%32s%32s",28,0x081c,0x00,1,a,0x00,b,1,0,c,d,e,f,g,h) 
#define corvette_device_inquiry(a,b,c,d,e,f,g,h) build_cmd("%16%8%1%1%3%1%1%1%32s%32s%32s32s%32s%32s",28,0x0b1c,0x00,1,a,0x00,b,1,0,c,d,e,f,g,h) 
#define _BUFFER_BYTE_COUNT_4 (long)0x0004
#define corvette_device_read(a,b,c,d,e) build_cmd("%16%16%32%32%32%32%32%16%16",28,0x811c,0x00c0,a,b,c,d,0x00,e,0x0002) 
#define corvette_write_data(a,b,c,d,e,f,g,h,i,j,k) build_cmd("%16%4%1%3%1%1%1%1%2%1%1%32s%32s%32s%32s%32s%16s%16s",28,0x021c,0x00,d,0x00,0,a,0,b,0x00,c,0,e,f,g,h,i,j,k) 
#define corvette_spin_up(a) build_cmd("%32%8%24%32%32%32s%32%32%32%32",36,0x9f240200,0x06,0x0000,0x0000,0x0000,a,0x0000,0x1b000000,0x01000000,0x0000) 
#define build_send_scsi() build_cmd("%32%32%24%8%32%16%16%32%32%32%32%32%32%32%32", 52, 50, 0x410000, 0x1, a, ++correlation_id, 0x0f, 0x8621, b, 0xff, 0x00, 0x00, 0x00, d, c, d)
#define corvette_immediate_abort(a) build_cmd("%16%16", 4, 0x0f04, a)
#define corvette_immediate_download_prepare() build_cmd("%16%16", 4, 0x1e04, 0xaa55)
#define corvette_immediate_run_selected_self_test() build_cmd("%16%16", 4, 0x1304, 0x012e)
#define corvette_clear_warm_start() build_cmd("%16%16", 4, 0x1304, 0x012d) /* deeren */
#define corvette_immediate_feature_control(a,b) build_cmd("%16%8%3%5", 4, 0x0c04, b&0x0ff, a, ((b&0x1f00)>>8))
#define _10_MHz 0
#define _8_MHz 1
#define _6_66_MHz 2
#define _5_MHz 3
#define _4_MHz 4
#define _3_1_MHz 5
#define _2_5_MHz 6
#define _2_MHz 7

#define corvette_immediate_format_prepare() build_cmd("%16%16", 4, 0x1704, 0xaa55)
#define corvette_immediate_pacing_control(a,b) build_cmd("%16%8%8", 4, 0x0d04, a, b)
#define _100_PERCENT 100
#define _25_PERCENT 25
#define _75_PERCENT 75
#define _24_PERCENT 24
#define _101_PERCENT 101
#define _15_PERCENT 15

#define corvette_immediate_reset(a) build_cmd("%16%16", 4, 0x0004, a)
#define corvette_immediate_run_diagnostic_test(a) build_cmd("%16%16s",4,0x1204,a) 
#define _PARITY_CHECK_BYTE_0 0X0009
#define _PARITY_CHECK_BYTE_1 0X000A
#define _PARITY_CHECK_BYTE_2 0X000B
#define _PARITY_CHECK_BYTE_3 0X000C
#define _PARITY_CHECK_DISABLE 0X0010
#define corvette_immediate_set_FIFO_threshold(a) build_cmd("%16%16", 4, 0x1504, a)
#define corvette_error_log_control(a,b,c,d,e,f,g,h,i) build_cmd("%16%8%1%1%3%1%1%1%32s%32s%32s%32s%32s%32s", 28, 0x2e1c, 0x00,a, b,0x00, c,1,0,d,e,f,g,h,i) 
#define corvette_format_unit(a,b,c,d,e,f,g,h,i) build_cmd("%16%8%1%1%4%1%1%3%1%1%3%8%16s%32s%32s%32s%32s%32s", 28, 0x161c, 0x00,0,a,0x00,1,0,0x00,b,c,0x00,0x00,d,e,f,g,h,i)
#define _NO_DEFECT_LIST 0
#define _DEFECT_LIST 1
#define _ADD 0
#define _REPLACE 1
#define _INTERLEAVE_FACTOR_0 0
#define _INTERLEAVE_FACTOR_1 1
#define _INTERLEAVE_FACTOR_FF 0xFF  
#define _NO_BACKGROUND_FORMAT 0
#define _BACKGROUND_FORMAT 1
#define _LIST_LENGTH_8 8
#define _DEFECT_BLOCK_ADDRESS_0 0
#define _DEFECT_ADDR_0x000000ff 0x000000ff
#define _DEFECT_ADDR_0x00000fff 0X00000FFF
#define _DEFECT_ADDR_0x0000ffff 0X0000FFFF
#define _DEFECT_ADDR_MAX 0XFFFFFFFF
#define _DEFECT_LIST_BYTE_COUNT_24 24
#define _DEFECT_LIST_BYTE_COUNT_8 8

#define corvette_get_command_complete_status(a,b,c,d,e,f,g) build_cmd("%16%8%1%1%4%1%1%32s%32s%32%32s%32s%32s", 28, 0x071c,0x00,1,a,0x00,1,0,b,c,d,e,f,g)
#define corvette_get_POS_and_adapter_information(a,b,c,d,e,f,g,h) build_cmd("%16%8%1%1%3%1%1%1%32s%32s%32s%32s%32s%32s", 28, 0x0a1c,0x00,1,a,0x00,b,1,0,c,d,e,f,g,h)
#define corvette_read_data(a,b,c,d,e,f,g,h,i,j,k,l,m) build_cmd("%16%3%1%1%3%1%1%1%1%1%1%1%1%32s%32s%32s%32s%32s%16s%16s", 28, 0x011c,0x00,e,f,0x00,1,a,0,b,0,c,d,0,g,h,i,j,k,l,m)
#define _BUFFER_BYTE_COUNT_2M 0X1F4000 
#define _BLOCK_COUNT_4000 4000
#define corvette_read_device_capacity(a,b,c,d,e,f,g) build_cmd("%16%8%1%1%4%1%1%32s%32s%32%32s%32s%32s", 28, 0x091c, 0x00,1,a,0x00,1,0,b,c,d,e,f,g)
#define corvette_read_logical_device_assignments(a, b,c,d,e,f,g) build_cmd("%16%8%1%1%4%1%1%32s%32s%32%32s%32s%32s", 28, 0x2a1c,0x00,1,a,0x00,1,0,b,c,d,e,f,g)
#define corvette_read_verify(a,b,c,d,e,f,g,h) build_cmd("%16%8%1%1%4%1%1%32s%32s%32s%32s%32s%16s%16s", 28, 0x031c,0x00,1,a,0x00,1,0,b,c,d,e,f,g,h)
#define corvette_reassign_block(a,b,c,d,e,f,g) build_cmd("%16%8%1%1%4%1%1%32s%32s%32s%32s%32s%32s", 28, 0x181c,0x00,0,a,0x00,1,0,b,c,d,e,f,g)
#define corvette_reassign_block_defect_list(a,b,c) build_cmd("%16s%16s%32s",8,a,b,c)
#define _DEFECT_LIST_LENGTH_8 8


#define corvette_specify_maximum_LBA(a,b,c,d,e,f,g) build_cmd("%16%8%1%1%4%1%1%32s%32s%32s%32s%32s%32s", 28, 0x1a1c,0x00,0,a,0x00,1,0,b,c,d,e,f,g)
#define _MAX_LBA_0 0
#define _MAX_LBA_0x000FFFFF 0x000FFFFF
#define _MAX_LBA_MAX 0xFFFFFFFF
#define corvette_send_other_SCSI(a,b,c,d,e,f,g,h,i,j,k) build_cmd("%16%8%1%1%1%1%1%1%1%1%4%4%8%16%32s%32s%32s%32s%16s%16s%16s%16s%16s%16s",36,0x1f24,0x00,a,b,0,c,0,d,1,0,0x0,e,0x00,f,g,h,i,j,k,0x00,0x00,0x00,0x00,0x00)
#define _TEST_UNIT_READY 0x00
#define _6_BYTE_SCSI_CMD 0x06
#define corvette_write_with_verify(a,b,c,d,e,f,g,h,i,j,k) build_cmd("%16%4%1%3%1%1%1%1%2%1%1%32s%32s%32s%32s%32s%16s%16s", 28, 0x041c,0x00,d,0x00,0,a,0,b,0x00,c,0,e,f,g,h,i,j,k)
#define _BLOCK_COUNT_3920 3920

#define corvette_invalid_op_code_test(a) build_cmd("%16%16",4,a,0x0000)

#define corvette_entity_ID_management_request(a,b,c,d,e,f,g,h,i,j,k,l) build_cmd("%16s%16%16%1%7%3%2%3%8%8%16%32s%16%16s%8%8%4%4%4%4",24,a,0x0000,0x0000,c,0x10,0x00,b,0x00,0x00,d,e,f,0x0000,g,l,0x00,j,k,h,i)
#define _RELEASE_ENTITY_ID 0xA001
#define _ASSIGN_ENTITY_ID 0XA000
#define _RELEASE_ALL_ID 0xA002
#define _AUTO_TARGET 8
#define _INITIATOR_MODE 0x0000
#define _SCSI_0 0
#define _SCSI_3 3
#define _SCSI_5 5
#define _LUN_0 0
#define _ENTITY_ID_1 1
#define _ENTITY_ID_5 5
#define _ENTITY_ID_3 3
#define _ENTITY_ID_6 6
#define corvette_abort_SCSI_request(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) build_cmd("%16s%16%16%1%1%6%2%1%2%3%8%8%16s%32s%6%1%1%1%1%2%1%1%1%1%16s%32s",24,a,0x0000,0x0000,d,1,0x00,0x00,b,c,0x00,f,e,0x0000,g,0x00,n,0,h,i,0x00,j,k,l,m,o,p)
#define _LENGTH_0 0
#define _LENGTH_24 24
#define _REPLY_SENT 0
#define _SUPPRESS_REPLY 1
#define _NO_CHAINING 0x00
#define _START_OF_CHAINING 0x01
#define _MIDDLE_OF_CHAINING 0x03
#define _END_OF_CHAINING 0x02
#define _NO_EXPEDITE 0
#define _EXPEDITE 1
#define _ENTITY_ID_0 0x00
#define _MINUTES 1
#define _SECONDS 0
#define _ERROR_ELEMENT_RETURN 0
#define _NO_ERROR_ELEMENT 1
#define _NO_CLEAR_QUEUE 0
#define _CLEAR_QUEUE 1
#define _NO_ABORT_TAG 0
#define _ABORT_TAG 1
#define _NO_BUS_DEVICE_RESET 0
#define _BUS_DEVICE_RESET 1
#define _NO_REACTIVATE_QUEUE 0
#define _REACTIVATE_QUEUE 1
#define _TIME_0 0
#define _TIME_60 60
#define _TIME_MAX 65535 	
#define _UNIT_ID_0 0
#define _UNIT_ID_3 3
#define _SCSI_MSG_0 0
#define _SCSI_MSG_1 1 
#define _ABORT_ID_0 0
#define _ABORT_ID1
#define _ABORT_ID2
#define _ABORT_ID3
#define _ABORT_ID4
#define corvette_cancel_request(a,b,c,d,e,f,g,h) build_cmd("%16s%16%16%1%3%1%1%2%2%1%2%3%8%8%16%32s",16,a,0x0000,0x0000,d,0x00,1,1,0x00,0x00,b,c,0x00,f,e,g,h)
#define _SOURCE_ID_0 0
#define _LENGTH_16 16
#define corvette_diagnose_request(a,b,c,d,e,f,g,h,i) build_cmd("%16s%16%16%1%3%1%1%1%1%2%1%2%3%8%8%16%32s%8%8%16",20,a,0x0000,0x0000,d,0x00,1,1,1,1,0x00,b,c,0x00,0x00,e,f,g,i,h,0x0000)
#define _LENGTH_20 20
#define _NUMBER_TEST_1 1
#define _NUMBER_TEST_0 0
#define _NUMBER_TEST_3 3
#define _NUMBER_TEST_MAX 255
#define _TEST_ID_2 0X02
#define corvette_establish_buffer_pool_request(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) build_cmd("%16s%16%16%1%7%2%1%2%3%8%8%16s%32s%16s%4%4%8%8%1%7%16s%32s%32%32s%8%1%7%16s%32s%32%32s",52,a,0x0000,0x0000,d,0x49,0x00,b,c,0x00,0x00,e,f,g,i,0x00,h,0x00,0x00,k,0x01,j,l,0x0000,m,0x00,o,0x01,n,p,0x00,q)
#define _0_BUFFERS_MAX 0
#define _2_BUFFERS_MAX 2
#define _10_BUFFERS_MAX 10
#define _0_THRESHOLD 0
#define _0x00ff_THRESHOLD 0X00FF
#define _0xffff_THRESHOLD 0xffff
#define _ID1_0 0
#define _ID1_1 1
#define _C1_0 0
#define _C1_1 1
#define _BUFFER1_SIZE_0 0
#define _BUFFER1_SIZE_50 50
#define _BUFFER1_SIZE_64 64
#define _BUFFER2_SIZE_128 128
#define _LSW1_0 0
#define _ID2_0 0
#define _C2_0 0
#define _BUFFER2_SIZE_0 0
#define _LSW2_0 0
#define _LENGTH_52 52

#define corvette_execute_locate_mode_SCB_request(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) build_cmd("%16s%16%16%1%7%2%1%2%3%8%8%16s%32s%4%1%1%1%1%1%1%2%1%3%16s%32s%8%8%16%32s",32,a,0x0000,0x0000,d,0x70,0x00,b,c,0x00,0x00,e,0x0000,f,0x03,j,k,l,0,g,h,0x00,i,0x00,m,n,o,0x00,0x0000,p)
#define _NO_EMBEDDED_CMD 0

#define _LENGTH_32 32
#define _NO_TAGGED_QUEUE 1
#define _TAGGED_QUEUE 0
#define _SCB_COMMAND 1
#define _IMMED_COMMAND 0
#define _NORMAL_TIMEOUT 1
#define corvette_initialize_SCSI_request(a,b,c,d,e,f,g,h,i,j) build_cmd("%16s%16%16%1%7%2%1%2%3%8%8%16s%32s%8%4%1%1%1%1%16",20,a,0x0000,0x0000,d,0x52,0x00,b,c,0x00,f,e,0x0000,g,0x00,0x00,h,i,0,j,0x0000)
#define _EXTERN_SCSI_BUS_NOT_SET 0
#define _EXTERN_SCSI_BUS_SET 1
#define _INTERN_SCSI_BUS_NOT_SET 0
#define _INTERN_SCSI_BUS_SET 1
#define _NO_HARD_RESET 0
#define _HARD_RESET 1
#define corvette_loop_scatter_gather_request(a,b,c,d,e,f,g,h) build_cmd("%16s%16%16%1%7%2%1%2%3%8%8%16s%32s",16,a,0x0000,0x0000,d,0x60,0x00,b,c,0x00,f,e,g,h)
#define corvette_send_SCSI_request(a,b,c,d,e,f,g,h,i,k,l,m,n,o,p,q) build_cmd("%16s%16%16%1%7%2%1%2%3%8%8%16s%32s%1%1%1%1%1%1%1%1%1%1%2%4%16s%32s%8%8%16%8%8%8%8%8%8%8%8%32s%32s%32s%32s",52,a,0x00,0x00,d,0x41,0x00,b,c,0x00,0x03,e,0x00,f,0,h,i,0,k,l,m,n,g,0,0x00,6,0x00,o,0xff,0x00,0x0000,0x12,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x0000,0x00ff,p,q)
#define _BUFFER_SIZE_0xffff 0xffff
#define corvette_read_list_request(a,b,c,d,e,f,g,h,i,k,l,m,o,q,t,u) build_cmd("%16s%32%1%7%2%1%2%3%8%8%16%32s%1%1%1%1%1%1%1%1%1%7%16s%32s%8%8%16%32s%16s%16s%32s%32s",44,a,0x0000,d,0x05,0x00,b,c,0x00,0x03,e,0x0000,f,0,h,i,0,k,l,m,1,g,0x00,0x00,o,0xff,0x00,0x0000,q,1,512,t,u)
#define _BUFFER_SIZE_512 512
#define _BUFFER_SIZE_1024 1024
/*
#define corvette_read_list_request(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u) build_cmd("%16s%16%16%1%7%2%1%2%3%8%8%16%32s%1%1%1%1%1%1%1%1%1%1%1%1%4%16s%32s%8%8%16%32s%16s%16s%32s%32s",44,a,0x0000,0x0000,d,0x05,0x00,b,c,0x00,0x03,e,0x0000,f,0,h,i,j,k,l,m,1,g,0,0,0,0x00,n,o,p,0x00,0x0000,q,r,s,t,u)
*/
#define corvette_write_list_request(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r) build_cmd("%16s%16%16%1%7%2%1%2%3%8%8%16%32s%1%1%1%1%1%1%1%1%1%1%1%1%4%16s%32s%8%8%16%32s%16s%16s%32s%32s",44,a,0x0000,0x0000,d,0x08,0x00,b,c,0x00,0x03,3,0x0000,f,0,h,0,i,j,k,l,0,g,0,0,0,0x00,0x00,m,n,0x00,0x0000,o,p,512,q,r)
#define _BUFFER_SIZE_2048 2048
#define corvette_write_list_request_with_loop_SG(b,d,f,g,h,i,j,k,l,m,n,o,p,q,r) build_cmd("%16s%16%16%1%7%2%1%2%3%8%8%16%32s%1%1%1%1%1%1%1%1%1%1%1%1%4%16s%32s%8%8%16%32s%16s%16s%32s%32s",44,44,0x0000,0x0000,d,0x08,0x00,b,0x00,0x00,0x03,0x03,0x0000,f,0,h,0,i,j,k,l,0,g,0,0,0,0x00,0x00,m,n,0x00,0x0000,o,p,512,q,r)
#define corvette_write_with_verify_list_request(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) build_cmd("%16s%16%16%1%7%2%1%2%3%8%8%16%32s%1%1%1%1%1%1%1%1%1%1%1%1%4%16s%32s%8%8%16%32s%16s%16s%32s%32s",44,a,0x0000,0x0000,d,0x48,0x00,b,c,0x00,0x03,e,0x0000,f,0,h,0,0,i,j,k,0,g,0,0,0,0x00,0x00,l,m,0x00,0x0000,n,o,512,p,q)
#define _BUFFER_SIZE_1024 1024
#define _BUFFER_SIZE_2048 2048
#define _LENGTH_44 44
#define _SUPPRESSED 1
#define _UNSUPPRESSED 0
#define _LOOP_SG_DISABLE 0
#define _LBA_1 1
#define _BLOCK_COUNT_1 1
/* #define _BLOCK_LENGTH_512 512
*/ 
#define _BUFFER1_SIZE_1024 1024
#define corvette_read_verify_request(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) build_cmd("%16s%16%16%1%7%2%1%2%3%8%8%16s%32s%1%1%2%1%1%1%1%1%7%16s%32s%8%8%16%32s%16s%16",36,a,0x0000,0x0000,d,0x44,0x00,b,c,0x00,f,e,0x00,g,0,i,0x00,j,k,l,1,h,0x00,m,n,o,0x00,0x0000,p,q,0x0000)
#define _LENGTH_36 36
#define _BLOCK_COUNT_2 2
#define corvette_read_device_capacity_request(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r) build_cmd("%16s%16%16%1%7%2%1%2%3%8%8%16%32s%1%1%2%1%1%1%1%1%1%1%1%4%16s%32s%8%8%16%32s%32s",36,0x0024,0x0000,0x0000,d,0x42,0x00,b,c,0x00,0x03,e,0x00,f,0,j,0x00,k,l,m,1,g,0,h,i,0x00,n,o,p,0x00,0x0000,q,r)
#define _DISCONNECT 0
#define _NO_DISCONNECT 1
#define _AUTO_REQUEST_SENSE_DISABLE 0
#define _AUTO_REQUEST_SENSE_ENABLE 1
#define _U_BIT_0 0
#define _U_BIT_1 1
#define _I_BIT_0 0
#define _I_BIT_1 1
#define _C_BIT_0 0
#define _C_BIT_1 1
#define _BUFFER_LENGTH_0xff 0xff
#define _BUFFER_ADDRESS_0xffff 0xffff
#define _BUFFER_SIZE_0X08 0x08
#define _RS_ADDR_0xff 0xff
#define corvette_read_immediate_request(a,b,c,d,e,f,g) build_cmd("%16s%16%16%1%7%2%1%2%3%8%8%16s%32s",16,a,0x0000,0x0000,d,0x06,0x00,b,c,0x00,f,e,0x0000,g)
#define corvette_reactivate_SCSI_queue_request(a,b,c,d,e,f,g,h) build_cmd("%16s%16%16%1%7%2%1%2%3%8%8%16s%32s",16,a,0x0000,0x0000,d,0x7E,0x00,b,c,0x00,f,e,g,h) 
/* POS Register 4: I/O Control */
#define corvette_release_buffer_pool_request(a,b,c,d,e,f,g,h) build_cmd("%16s%16%16%1%7%2%1%2%3%8%8%16s%32s",16,a,0x0000,0x0000,d,0x4c,0x00,b,c,0x00,f,e,g,h) 
#define _ENABLE_ROM_BIOS 1
#define _DISABLE_ROM_BIOS 0
#define _ENABLE_MOVE_MODE 1
#define _DISABLE_MOVE_MOVE 0
#define _ENABLE_STREAMING 1
#define _DISABLE_STREAMING 0
#define _ENABLE_SLAVE_RETURN_CHECKING 1
#define _DISABLE_SLAVE_RETURN_CHECKING 0

#endif
