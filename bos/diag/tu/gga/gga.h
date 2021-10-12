/* @(#)70	1.1  src/bos/diag/tu/gga/gga.h, tu_gla, bos41J, 9515A_all 4/6/95 09:26:56 */
/*
*
*   COMPONENT_NAME: tu_gla
*
*   FUNCTIONS: none
*
*   ORIGINS: 27
*
*   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
*   combined with the aggregated modules for this product)
*                    SOURCE MATERIALS
*
*   (C) COPYRIGHT International Business Machines Corp. 1995
*   All Rights Reserved
*   US Government Users Restricted Rights - Use, duplication or
*   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
*/

#ifndef _H_GGA_WGA
#define _H_GGA_WGA

#define PACE_GGA 0x65
#define GGA_START_DIAG 0x66
#define GGA_QUERY_DIAG 0x67
#define GGA_STOP_DIAG 0x68

struct gga_map     /* structure for returning mapping info to apps */
{
        ulong base_address;             /* adapter offset in segment      */
        ulong screen_height_mm;         /* Height of screen in mm         */
        ulong screen_width_mm;          /* Height of screen in mm         */
        ulong screen_height_pix;        /* Height of screen in pixels     */
        ulong screen_width_pix;         /* Height of screen in pixels     */
        ulong color_mono_flag;          /* Flag for color = 1 or mono = 0 */
        ulong  monitor_type;            /* monitor type connected to GGA  */
};

/* GGA Diagnostic Interrupt structure */
struct gga_intr_count
{
  ulong erraddr;
  ulong parity_err;
  ulong inval_cmd;
  ulong resv_access;
  ulong bad_string_op;
  ulong inval_value;
  ulong intr_set;
  ulong blank_set;
};

struct crt_control
{
        ulong   monitor_id;     /* Combination of Cable id and syncs */
        ulong   hrzt_reg;
        ulong   hrzsr_reg;
        ulong   hrzbr_reg;
        ulong   hrzbf_reg;
        ulong   vrtt_reg;
        ulong   vrtsr_reg;
        ulong   vrtbr_reg;
        ulong   vrtbf_reg;
        ulong   x_resolution;   /* pixels in x direction */
        ulong   y_resolution;   /* pixels in y direction */
#define COLOR 1
#define MONO  0
        ulong   color_mono;
#define SYNC_ON_GREEN    0x28
#define NO_SYNC_ON_GREEN 0x00
        ulong   sync_on_green;
        ulong   dot_clock_reg;
        ulong   double_dot_clock;
        uchar   adcntl_reg;     /* sync polarities */
};

#endif /* _H_GGA_WGA */
