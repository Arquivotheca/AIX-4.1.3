/* @(#)94       1.5  src/bos/kernext/disp/ped/pedmacro/hw_dsp.h, pedmacro, bos411, 9428A410j 3/23/94 17:06:43 */

/*
 *   COMPONENT_NAME: PEDMACRO
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/****************************************************************************/
/*                           DSP Commo Return Codes                         */
/****************************************************************************/

#define    HARDWARE_ERROR         0x80000000
#define    MICROCODE_CRC_FAILED   0x80010000
#define    BLAST_ERROR            0x80020000
#define    BLAST_PROC_ERROR       0x80030000
#define    PIPE_PROC1_ERROR       0x80040000
#define    PIPE_PROC2_ERROR       0x80050000
#define    PIPE_PROC3_ERROR       0x80060000
#define    PIPE_PROC4_ERROR       0x80070000
	
#define    DATA_STREAM_ERROR      0x40000000
#define    INVALID_SE_LENGTH      0x40010000
#define    NON_RECOVERABLE_ER     0x40020000
#define    INVALID_CONTEXT_ID     0x40030000
#define    INVALID_REQUEST        0x40040000
#define    CONFIG_NOT_SUPPORTED   0x40050000
#define    DMA_COMPLETE_ERROR     0x40060000
	
#define    RESET_COMPLETE         0x00010000
#define    MICROCODE_CRC_PASSED   0x00020000
#define    SOFT_RESET_IN_PROGRESS 0x00030000
#define    INQUIRE_DATA_AVAIL     0x00040000
#define    PICK_COMPLETE          0x00050000
#define    DMA_COMPLETE           0x00060000
#define    CTX_SAVE_COMPLETE      0x00070000
#define    NEW_CTX_COMPLETE       0x00080000
#define    FONT_REQUEST           0x000A0000
#define    PIN_CTX_MEM_REQ        0x000B0000
#define    SYNC_RESPONSE          0x000C0000
#define    FIFO_STALLED           0x000D0000
#define    FRAME_BUF_SWAPPED      0x000E0000
#define    END_RENDERING_3DM1     0x000F0000
#define    HI_CTX_DMA_COMPLETE    0x00100000
#define    AI_CTX_DMA_COMPLETE    0x00110000
#define    CTX_DMA_PARM_ERROR     0x00120000
	
