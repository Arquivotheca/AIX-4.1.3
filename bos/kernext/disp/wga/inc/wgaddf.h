/* @(#)93		1.1	src/bos/kernext/disp/wga/inc/wgaddf.h, wgadd, bos411, 9428A410j 10/28/93 18:52:09 */
/*
 * COMPONENT_NAME: (WGADD) Whiteoak Graphics Adapter Device Driver
 *
 * FUNCTIONS:
 *
 * ORIGINS:	27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _WGA_DDF
#define _WGA_DDF

/******************************************************************
*   identification: WGADDF.H                                      *
*   descriptive name: Defines for WGA device driver               *
*   function: Device values for use by the DMA and DDI routines   *
*              of the WGA device driver                           *
*                                                                 *
******************************************************************/

#define DMA_IN_PROGRESS 0x80000000
#define DMA_WAIT_REQ    0x01000000
#define DMA_COMPLETE    0x00FFFFFF
#define DIAG_MODE       0x00010000
#define DIAG_OFF        0xFF00FFFF
#define EVENT_MODE      0x00008000
#define EVENT_OFF       0xFFFF00FF
#define ASYNC_REQ       0x00000100
#define ASYNC_OFF       0xFFFFFEFF
#define SYNC_REQ        0x00000800
#define SYNC_WAIT_REQ   0x00000200
#define SYNC_NOWAIT_REQ 0x00000400
#define SYNC_OFF        0xFFFFF1FF
#define NORPT_EVENT     0x00000080
#define NORPT_OFF       0xFFFFFF7F



struct wga_ddf 
{
	long cmd;
	caddr_t sleep_addr;
	int (*callback)();        /* Callback routine for event support */
	uchar s_event_mask;
	uchar a_event_mask;
	uchar e_event_mask;
	uchar poll_type;
	caddr_t callbackarg;
	ulong diaginfo[8];
	label_t jmpbuf;
	eventReport report;
	char jumpmode,timerset;
	char scrolltime,jthreshold;
	long jumpcount,lastcount;
	struct trb *cdatime;
 	int (*sm_enq)();

	ulong bpp;			/* bits per pel (vram dependent) */
	uchar num_vram_modules;		/* number of vram modules on WGA */
	ulong monitor_type;		/* monitor type connected to WGA */
	ulong screen_height_mm; 	/* Height of screen in mm */
	ulong screen_width_mm; 		/* Height of screen in mm */

};

#endif /* _WGA_DDF */
