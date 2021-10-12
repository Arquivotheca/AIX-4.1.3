/* @(#)39	1.5  src/bos/kernext/disp/sky/inc/ddsent.h, lftdd, bos411, 9428A410j 4/18/94 14:08:27 */
/*
 *   COMPONENT_NAME: SYSXDISPSKY
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define FAIRNESS_ENABLE    0xff000000
#define NIBBLE_MODE_ENABLE 0x00ff0000

struct  ddsent
{
   ulong    io_bus_addr_start;      /*                                   */
   ulong    io_bus_mem_start;       /*                                   */
   ulong    vram_start;             /*                                   */
   ulong    dma_range1_start;       /* dma address start                 */
   ulong    dma_range2_start;       /* dma address start                 */
   ulong    dma_range3_start;       /* dma address start                 */
   ulong    dma_range4_start;       /* dma address start                 */
   ulong    dma_channel;            /* dma channel                       */
   short    slot_number;            /* slot number                       */
   short    int_level;              /* interrupt level                   */
   short    int_priority;           /* interrupt priority                */
   short    screen_width_mm;        /* screen width in millimeters       */
   short    screen_height_mm;       /* screen heigth in millimeters      */
   short    reserved1;              /*                                   */
   ulong    display_id;             /* unique display id                 */
   ulong    busmask;                /* mask for bus optional items i.e.  */
				    /* fairness etc.                     */
   ulong    ksr_color_table[16];    /* ksr color pallet                  */
   char     component[16];           /* component name                    */
};
