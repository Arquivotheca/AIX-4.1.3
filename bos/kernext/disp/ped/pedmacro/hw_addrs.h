/* @(#)13       1.6.1.3  src/bos/kernext/disp/ped/pedmacro/hw_addrs.h, pedmacro, bos411, 9428A410j 3/23/94 17:06:17 */

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

#ifndef _H_MID_ADDRS
#define _H_MID_ADDRS

/****************************************************************************/
/*                   BIM Host Register Address Definitions                  */
/****************************************************************************/

#define   MID_ADR_FIFO_DATA       0x2000
#define   MID_ADR_FREE_SPACE      0x2040

#define   MID_ADR_PCB_DATA        0x1000

#define   MID_ADR_IND_CONTROL     0x3070
#define   MID_ADR_IND_ADDRESS     0x3074
#define   MID_ADR_IND_DATA        0x3078

#define   MID_ADR_DSP_CONTROL     0x0068
#define   MID_ADR_CARD_INT_MASK   0x007C
#define   MID_ADR_HOST_INT_MASK   0x0080
#define   MID_ADR_HOST_STATUS     0x0084
#define   MID_ADR_CARD_COMMO      0x0088
#define   MID_ADR_HOST_COMMO      0x008C

#define   MID_ADR_POS_REG_0       0x0000
#define   MID_ADR_POS_REG_1       0x0004
#define   MID_ADR_POS_REG_2       0x0008
#define   MID_ADR_POS_REG_3       0x000C
#define   MID_ADR_POS_REG_4       0x0010
#define   MID_ADR_POS_REG_5       0x0014
#define   MID_ADR_POS_REG_6       0x0018
#define   MID_ADR_POS_REG_7       0x001C

/****************************************************************************/
/*                                DMA Definitions                           */
/****************************************************************************/

#define   DMA_ID_COUNT           12     /* Need more address space if > 16 */
	                                /* Value should match MAX_DMA_ID   */
#define   DMA_ADR_WIDTH  0x000FA000     /* 1M of width for each ID         */

#define   FIFO_DMA_ID             0
#define   READ_BLIT_DMA_ID        1
#define   READ_BLIT_DMA_ID_2      2     /* Allocate 2M for blit reads      */
#define   WRITE_BLIT_DMA_ID       3
#define   WRITE_BLIT_DMA_ID_2     4     /* Allocate 2M for blit writes     */
#define   DMA_SE_DMA_ID           5
#define   FONT_DMA_ID             6
#define   PICK_DMA_ID             7
#define   CONTEXT_RD_DMA_ID       8
#define   CONTEXT_WR_DMA_ID       9
#define   CONTEXT_MV_DMA_ID      10
#define   TRACE_DMA_ID           11

#define   FIFO_DMA_OFFSET         (FIFO_DMA_ID       * DMA_ADR_WIDTH)
#define   READ_BLIT_DMA_OFFSET    (READ_BLIT_DMA_ID  * DMA_ADR_WIDTH)
#define   WRITE_BLIT_DMA_OFFSET   (WRITE_BLIT_DMA_ID * DMA_ADR_WIDTH)
#define   DMA_SE_DMA_OFFSET       (DMA_SE_DMA_ID     * DMA_ADR_WIDTH)
#define   FONT_DMA_OFFSET         (FONT_DMA_ID       * DMA_ADR_WIDTH)
#define   PICK_DMA_OFFSET         (PICK_DMA_ID       * DMA_ADR_WIDTH)
#define   CONTEXT_RD_DMA_OFFSET   (CONTEXT_RD_DMA_ID * DMA_ADR_WIDTH)
#define   CONTEXT_WR_DMA_OFFSET   (CONTEXT_WR_DMA_ID * DMA_ADR_WIDTH)
#define   CONTEXT_MV_DMA_OFFSET   (CONTEXT_MV_DMA_ID * DMA_ADR_WIDTH)
#define   TRACE_DMA_OFFSET        (TRACE_DMA_ID      * DMA_ADR_WIDTH)

#endif  /* _H_MID_HW_ADDRS */
