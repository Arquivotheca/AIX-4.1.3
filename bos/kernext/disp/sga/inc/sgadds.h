/* @(#)24       1.1.1.2  src/bos/kernext/disp/sga/inc/sgadds.h, sgadd, bos411, 9428A410j 10/29/93 10:46:04 */
/*
 * COMPONENT_NAME: (SGADD) Salmon Graphics Adapter Device Driver
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

#ifndef _SGA_DDS
#define _SGA_DDS

#define FAIRNESS_ENABLE    0xff000000
#define NIBBLE_MODE_ENABLE 0x00ff0000




struct  fbdds
{
   ulong    vram_start;             /*                                   */
   short    slot_number;            /* slot number                       */
   short    int_level;              /* interrupt level                   */
   short    int_priority;           /* interrupt priority                */
   short    screen_width_mm;        /* screen width in millimeters       */
   short    screen_height_mm;       /* screen heigth in millimeters      */
   short    reserved1;              /*                                   */
   ulong    display_id;             /* unique display id                 */
   int	    curr_nbr_sims;	    /* current number of simms on sga    */
   int	    prev_nbr_sims;	    /* previous number of simms on sga   */
   ulong    ksr_color_table[16];    /* ksr color pallet                  */
   char     component[16];           /* component name                    */
};

#endif /* _SGA_DDS */
