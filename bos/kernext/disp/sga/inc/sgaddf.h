/* @(#)19       1.2  src/bos/kernext/disp/sga/inc/sgaddf.h, sgadd, bos411, 9428A410j 10/29/93 10:45:51 */
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

#ifndef _SGA_DDF
#define _SGA_DDF

/******************************************************************
*   identification: SGADDF.H                                      *
*   descriptive name: Defines for SGA device driver               *
*   function: Device values for use by the DMA and DDI routines   *
*              of the SGA device driver                           *
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



struct sga_ddf {
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

 ulong bpp;                /* bits per pel (vram dependent) */
 uchar num_vram_modules;   /* number of vram modules on SGA */
 uchar monitor_type;       /* monitor type connected to SGA */

};

#endif /* _SGA_DDF */
