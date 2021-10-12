/* @(#)31  1.3  src/bos/kernext/disp/ped/inc/midctx.h, peddd, bos411, 9428A410j 3/19/93 18:53:47 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_MID_CTX
#define _H_MID_CTX




/****************************************************************************/
/*                       Normal DSP Commo Return Codes                      */
/****************************************************************************/

#define    MID_DSPC_RESET_COMPLETE       0x00010000
#define    MID_DSPC_UCODE_CRC_OK         0x00020000
#define    MID_DSPC_CTX_SW_COMPLETE      0x00030000
#define    MID_DSPC_INPUT_AVAILABLE      0x00040000
#define    MID_DSPC_PICK_COMPLETE        0x00050000
#define    MID_DSPC_DMA_COMPLETE         0x00060000
#define    MID_DSPC_CTX_SAVE_COMPLETE    0x00070000
#define    MID_DSPC_NEW_CTX_COMPLETE     0x00080000
#define    MID_DSPC_DIAGNOSTIC_COMPLETE  0x00090000
#define    MID_DSPC_NEED_FONT_REQ        0x000A0000
#define    MID_DSPC_PIN_CTX_MEM_REQ      0x000B0000
#define    MID_DSPC_SYNC_RESPONSE        0x000C0000
#define    MID_DSPC_CONTEXT_STALL        0x000D0000



/****************************************************************************/
/*           DSP Commo Hardware/ Microcode Error Return Codes               */
/****************************************************************************/

#define    MID_DSPC_CHECKSUM_FAILED      0x80010000


/****************************************************************************/
/*           DSP Commo Register Datastream Error Return Codes               */
/****************************************************************************/

#define    MID_DSPC_SE_LENGTH_ERROR          0x40010000
#define    MID_DSPC_NON_RECOVERABLE_ERROR    0x40020000
#define    MID_DSPC_INVALID_CONTEXT_ID       0x40030000
#define    MID_DSPC_INVALID_REQUEST          0x40040000
#define    MID_DSPC_FUNCTION_NOT_SUPPORTED   0x40050000
#define    MID_DSPC_DMA_COMPLETE_ERROR       0x40060000



#endif /* _H_MID_CTX */
