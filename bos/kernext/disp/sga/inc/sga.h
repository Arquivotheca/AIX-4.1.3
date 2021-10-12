/* @(#)28       1.2  src/bos/kernext/disp/sga/inc/sga.h, sgadd, bos411, 9428A410j 10/29/93 10:45:10 */
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

#ifndef _SGA_SGA
#define _SGA_SGA



#define PACE_SGA 0x65
#define SGA_START_DIAG 0x66
#define SGA_QUERY_DIAG 0x67
#define SGA_STOP_DIAG 0x68






struct sga_map     /* structure for returning mapping info to apps */
{
  ulong baseaddr;         /* vram base address     */
  ulong screen_height_mm; /* Height of screen in mm */
  ulong screen_width_mm;  /* Height of screen in mm */
		uchar num_vrams;        /* Number of VRAMs on the SGA */
		uchar monitor_type;     /* Monitor Sense Bit Code  */
																										/* -- aviable Macros are:           */
																										/*    MT_8507,  MT_8508, MT_8514/15 */
																										/*    MT_5081,  MT_6091, MT_NONE    */
																										/*    MT_SLR ,  MT_SHR              */
};



/* SGA Diagnostic Interrupt structure */
struct sga_intr_count
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

#endif /* _SGA_SGA */
