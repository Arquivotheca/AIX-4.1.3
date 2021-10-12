/* @(#)87       1.1  src/bos/kernext/disp/wga/inc/wga.h, wgadd, bos411, 9428A410j 10/28/93 18:24:43 */
/*
 * COMPONENT_NAME:  (WGADD) Whiteoak Graphics Adapter Device Driver
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _WGA_WGA
#define _WGA_WGA

#define PACE_WGA 0x65
#define WGA_START_DIAG 0x66
#define WGA_QUERY_DIAG 0x67
#define WGA_STOP_DIAG 0x68






struct wga_map     /* structure for returning mapping info to apps */
{
	ulong segment_register;        		/* segment register allocated for X     */
	uchar num_vrams;        		/* Number of VRAMs on the WGA */
	ulong monitor_type;     		/* Monitor Sense Bit Code  */
	ulong screen_height_mm; 		/* Height of screen in mm */
	ulong screen_width_mm; 			/* Height of screen in mm */
	ulong screen_height_pix; 		/* Height of screen in pixels */
	ulong screen_width_pix; 		/* Height of screen in pixels */
	ulong color_mono_flag;			/* Flag for color = 1 or mono = 0	*/
	long magic_x;				/* Magic number for cursor x position	*/
	long magic_y;				/* Magic number for cursor y position	*/
};



/* WGA Diagnostic Interrupt structure */
struct wga_intr_count
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
        ulong   monitor_id              /* Combination of Cable id and jumper id */;
        ulong   adcntl_reg;
        ulong   hrzt_reg;
        ulong   hrzsr_reg;
        ulong   hrzbr_reg;
        ulong   hrzbf_reg;
        ulong   vrtt_reg;
        ulong   vrtsr_reg;
        ulong   vrtbr_reg;
        ulong   vrtbf_reg;
        ulong   x_resolution;
        ulong   y_resolution;
#define COLOR 1
#define MONO  0
        ulong   color_mono;
#define SYNC_ON_GREEN   0xc0
#define NO_SYNC_ON_GREEN 0x00
        ulong   sync_on_green;
};

#endif /* _WGA_WGA */
