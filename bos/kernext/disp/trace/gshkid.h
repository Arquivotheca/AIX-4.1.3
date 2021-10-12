/* @(#)59	1.10  src/bos/kernext/disp/trace/gshkid.h, sysxdisp, bos41J, 9507A 1/26/95 15:47:58 */

#ifndef _H_GSHKID
#define _H_GSHKID

/*
 *
 * COMPONENT_NAME: (base_graphics) Graphics Systems Trace 
 *
 * FUNCTIONS:  Definition of Major trace hook ID for GS
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/************************************************************************
 									 
  Graphics Systems was originally assigned hook IDs in the range
  0x500 through 0x51F.  Most of these hook IDs have been used.
  The definition of these hookwords is contained in sys/disptrc.h.

  In 1992, Graphics Systems was allocated another set of system trace 
  hook IDs:  0x700 through 0x78F.  The definition of these hook IDs
  is contained in this file. 
  
******************************************************************************/


  enum GS_HKWD_set_2 { 
	HKWD_GS_set2_start 	= 0x700,

	HKWD_GS_GC_IO	 	= 0x700,	/* class architecture I/O */
	HKWD_GS_GC1	 	= 0x701, 	/* class 1 macros */
	HKWD_GS_GC2	 	= 0x702,	/* class 2 macros */
	HKWD_GS_GC3	 	= 0x703,	/* class 3 macros */

	HKWD_GS_BBL	 	= 0x704,	/* GXT150 adapter macros */
	HKWD_GS_BBL_DDX		= 0x705,	/* GXT150 adapter DDX */

	HKWD_GS_SMT		= 0x706,	/* Shared Memory Transport */
	HKWD_GS_LFT		= 0x707,	/* Low Function Terminal */
	HKWD_GS_BBL_DD 		= 0x708,	/* GXT150 adapter device driver */
	HKWD_GS_INPUT_DD 	= 0x709,	/* Input DD */
	HKWD_GS_SOFT3D 		= 0x70A,	/* Soft 3D */

	HKWD_GS_2D 		= 0x70B,	/* Generic 2D */
	HKWD_GS_X11_DIX		= 0x70C,	/* Generic DI X server */
	HKWD_GS_X11_MI		= 0x70D,	/* ported MIT MI code */
	HKWD_GS_X11_CFB		= 0x70E,	/* ported MIT CFB code */
	HKWD_GS_X11_CFB24	= 0x70F,	/* ported MIT 24-bit CFB */

	HKWD_GS_SKY_DDX		= 0x710,	/* Skyway DDX code */
	HKWD_GS_SGA_DDX		= 0x711,	/* Gt1 DDX code */
	HKWD_GS_SGA_DDX_1	= HKWD_GS_SGA_DDX,	/* Gt1 1-bit ddx */
	HKWD_GS_SGA_DDX_4	= HKWD_GS_SGA_DDX,	/* Gt1 4-bit ddx */
	HKWD_GS_SGA_DDX_8	= HKWD_GS_SGA_DDX,	/* Gt1 8-bit ddx */
	HKWD_GS_WGA_DDX		= 0x712,	/* Gt1x DDX code */
	HKWD_GS_PED_DDX		= 0x713,	/* Gt3/Gt4 DDX code */
	HKWD_GS_GEM_DDX		= 0x714,	/* GTO DDX code */
	HKWD_GS_RUBY_DDX	= 0x715,	/* GXT1000 DDX code */
	HKWD_GS_MAG_DDX		= 0x716,	/* xxxxx DDX code */

	HKWD_GS_X_INPUT		= 0x717,	/* X input functions */
	HKWD_GS_X_TRANS		= 0x718,	/* X transport layer */
	HKWD_GS_RMS		= 0x719,	/* RMS */
	HKWD_GS_X_ANC		= 0x71A,	/* X ancillary buf ext'n */
	HKWD_GS_X_MBX		= 0x71B,	/* X Multibuffer extension */

	HKWD_GS_RBY_3DM5	= 0x71C,	/* GXT1000 mod5 layer */
	HKWD_GS_BAUD_DD         = 0x71D,        /* baud device driver   */
	HKWD_GS_LTBL_DD		= 0x71E,	/* LTBL device driver */
	HKWD_GS_SKBL_DD		= 0x71F,	/* SKBL device driver */
	HKWD_GS_IGA_DD		= 0x720,	/* IGA device driver */
	HKWD_GS_GGA_DD		= 0x721,	/* GGA device driver */
	HKWD_GS_SGIO_DD		= 0x722,	/* Serial GIO  device driver */

	HKWD_GS_set2_end	= 0x78F } ;

 									 
#endif /* _H_GSHKID */

