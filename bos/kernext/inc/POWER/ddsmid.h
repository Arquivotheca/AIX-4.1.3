/* @(#)76       1.3.1.2  src/bos/kernext/inc/POWER/ddsmid.h, peddd, bos411, 9428A410j 3/19/93 19:14:01 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: 
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





struct  ddsmid
{
   ulong    io_bus_mem_start;       /* adapter address start              */
   ulong    dma_range1_start;       /* dma address start                  */
   ulong    dma_channel;            /* dma channel                        */
   short    slot_number;            /* slot number                        */
   short    int_level;              /* interrupt level                    */
   short    int_priority;           /* interrupt priority                 */
   short    screen_width_mm;        /* screen width in millimeters        */
   short    screen_height_mm;       /* screen height in millimeters       */
   short    refresh_rate;           /* screen refresh rate 60/77 Hz       */
   short    reserved1;              /*                                    */
   ulong    display_id;             /* unique display id                  */
   ulong    ksr_color_table[16];    /* ksr color pallet                   */
   ulong    hwconfig;               /* adapter card configuration         */
   int      microcode_path;         /* file pointer for display microcode */
   char     component[16];          /* configurable device/component name */
   char	    ucode_name[32];	    /* microcode path name                */
};
