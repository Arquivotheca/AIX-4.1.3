/* @(#)33       1.9.2.5  src/bos/kernext/disp/ped/inc/midddf.h, peddd, bos411, 9428A410j 3/31/94 21:33:52 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS:
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_MIDDDF
#define _H_MIDDDF


#include <sys/intr.h>           /* def of intr structure used by display.h */
#include <fkprocFont.h>		/* has define for MAX_PIN_FONTS_ALLOW */
#include <sys/display.h>        /* def phys_display struct -- already inc
                                    by fkprocFont.h */
#include <sys/types.h>
#include <sys/watchdog.h>	/* for sleep timeout */
#include <sys/xmem.h>           /* for cross-memory descriptor */
#include "hw_dd_model.h"
#include "mid.h"
#include "midrcx.h"
#include "midddfswap.h"
#include "midddfgets.h"
#include "midddfpick.h"
#include "midddftrc.h" 





/*-----------------------------------------------------------------* 
     identification: midddf.h					     
     descriptive name: Defines for MID device driver   	     
     function: Device values for use by the DMA and DDI routines     
  		of the MID device driver			     
 *-----------------------------------------------------------------*/

/*-----------------------------------------------------------------* 
     The Ped DDF structure is linked to the adapter (graphics device) 
     data structure (gscDev).  The following macro is provided to     
     obtain addressability to the Ped DDF structure.
 *-----------------------------------------------------------------*/
#define MID_addr_DDF(pDev) ((midddf_t *) (pDev-> devHead.display->free_area)) 


/*---------------------------------------------------------------------------* 
     Several watchdog structures are used in the device driver.  Here is
     a general Mid DD watchdog structure. 
 *---------------------------------------------------------------------------*/
typedef struct mid_watchdog {

 struct watchdog	 dog;
 char		 	*ddf ;

} mid_watchdog_t ;



#define PACE_MID  		0x6900
#define OP_COMPLETE_MASK 	0x80

#define X_MIN				0
#define Y_MIN				0
#define X_MAX				1279
#define Y_MAX				1023


/*--------
  KSR stuff 
  --------*/
#define COLOR_TABLE_SIZE		16
#define ADAPT_MAX_FONTS 		8
#define SPACE				0x0020

#define CURSOR_XOFFSET			31	/* used to init H/W cursor    */
#define CURSOR_YOFFSET			15

#define DOUBLE_UNDERSCORE		2
#define MID_DEFAULT_FONT		0



#define LO_COPY 			3
#define INTR				1  /* send interrupts after DMA       */
#define NOINTR				-1 /* don't send interrupts after DMA */
#define DMA_DONE			-1 /* from gr1_re.c unknown function  */
#define INVALID_MODE			-1 /* called with wrong mode          */

#define MIDNUMBLINK	20		/* Number of colors allowed to blink  */


/*--------
  Font stuff 
  --------*/

/*-------------------------
   DSP COMMON REGISTER:

	For font fault interrupt, the dsp_status (32 bit register) will have
 	the the following information:

	the upper 16 bits of DSP commo will have 0x000a and the lower 16 bits
 	are unused 
------------------------ */

#define MID_FNT_REQ_MASK   0xffff0000   /* extract upper 16 bits only */
#define MID_FNT_REQ_CODE   0x000a0000   /* 0x000a in upper 16 bits */ 



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                         */
/*  midddf structure.                                                      */
/*                                                                         */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct midddf {

	unsigned long	HWP;		/* base address for this adapter      */
	unsigned long   bus_offset;     /* bus mem start for this adapter     */

	/*--------------------------------------------------------------------* 
	   This section of fields defines the capabilities of the adapter.      
	   It is intended to contain values which might vary due to options
	   or due to differences between the various members of the mid-level
           graphics adapter family. 
	 *--------------------------------------------------------------------*/
	unsigned long   hwconfig;       /* MID_VPD_PPR host i/f & processor card    */
					/* MID_VPD_PGR screen buf & graphics card   */
					/* MID_VPD_POP 24 bit option card           */
					/* MID_VPD_PPC process pipe card            */
					/* MID_VPD_PPZ process with Z buffer        */
					/* MID_VPD_PPI process with 77 Hz           */

	int	max_color_IDs ;		/* # of color palettes supported      */
	int	max_WIDs ;		/* # of window IDs supported          */


	/*--------------------------------------------------------------------* 
	   RCM related fields 
	   
	   The RCM related data is included from the following file. 
	 *--------------------------------------------------------------------*/

#include "midddfrcm.h"



	/*--------------------------------------------------------------------* 
	   KLUDGE
	   
           These should go away sometime.  They are an ad hoc way of getting
           interrupts back from the interrupt handler.
	 *--------------------------------------------------------------------*/
	volatile unsigned long	dsp_status;	/* DSP interrupt status  */
	volatile unsigned long	bim_status;	/* BIM interrupt status  */
	volatile unsigned long	raw_bim_status;	/* BIM interrupt status  */
	volatile unsigned long	host_intr_mask_shadow;  /* BIM interrupt mask */

	struct phys_displays 	*phys_disp ;
	gscDev		*pdev ;


	long		cmd;
#			define SYNC_WAIT_REQ	0x1
#			define DMA_NOWAIT	0x00
#			define DMA_WAIT_REQ	0x8

	long		dma_sleep_flags;

	caddr_t sleep_addr;
	caddr_t dma_sleep_addr;
	rcmProc *pProc_dma ; 

	int (*callback)();
	int (*dcallback)();		/* diagnostic callback routine        */
	caddr_t callbackarg;

	ulong *diaginfo;
#		define EVENT_MODE    0x80000000
#		define EVENT_OFF     0x7FFFFFFF
#		define SYNC_REQ      0x00800000
#		define SYNC_WAIT     0x00010000
#		define SYNC_NOWAIT   0x00020000
#		define SYNC_OFF      0xFF7CFFFF
#		define ASYNC_REQ     0x00008000
#		define ASYNC_OFF     0xFFFF7FFF
#		define NORPT_EVENT   0x00000080
#		define PICK_PENDING  0x00000001

	long		eventcmd;
	long		a_event_mask;
	long		e_event_mask;
	long		s_event_mask;

	long		bufleft,numhits;
	char		*buffer_ptr;
	eventReport	report;


	/*--------------------------------------------------------------------* 

	 *--------------------------------------------------------------------*/



	/*--------------------------------------------------------------------* 
	 *--------------------------------------------------------------------*/

	label_t jmpbuf; 		/* pio exception handling (setjmpx) */
	ulong pio;			/* pio handling active if nonzero   */
	int bus_id;			/* save area for seg register value */

	struct file *mid_ucode_file_ptr;/* microcode file pointer           */
	char component[16];		/* name of hw component from DDS    */

	int dma_result; 		/* set by interrupt-driven DMA	    */
#		define DMA0_COMPLETE 1  /* set by intr handler, ch. 0	    */
#		define DMA1_COMPLETE 2  /* set by intr handler, ch. 1	    */

	struct mid_watch {		/* watchdog timer structure +	    */
		struct watchdog dog;	/* information needed to wakeup     */
		caddr_t *sleep_addr;	/* whoever is sleeping. 	    */
		struct  midddf  *ddf ;	/* ptr to this struct for hwconfig */ 
	} mid_watch;


	pMID_model_data	model_data;	/* for macro layer		    */

#	include "ddfgetstruct.h"

        ddf_SBdata_t		ddf_data_swapbuffers_hold_list[SB_ARY_SIZE];


	/*-----------------
	Used by mid_ddf() to determine when to initialize linked-lists next
	values.  These should only be initialized the first time in.
	-----------------*/
	ushort		first_time_in_DDF;


        /*-----------------
        Establish a pinned anchor position for the e_sleep() calls that
        occur in the ddf functions.
        -----------------*/
        ulong           e_sleep_anchor;

	char            slot;           /* Slot # for POS reg. access   */

        /* 
           No linked list is needed.  We need only one structure to hold 
           pick events' data because pick event is serialized. 
        */
	pick_event_t  pickDataBlock;

	int PickEventInProgress;    /* the lock used to serialize pick event.*/
                                    /* see midddfpick.h                      */ 

	pick_pending_t * pending_pick_headPtr;
	pick_pending_t * pending_pick_tailPtr;

	/*--------------------------------------------------------------------* 
	 Font Support on Pedernales
	 *--------------------------------------------------------------------*/

        /*-----------------
	For this adapter, how many fonts have been pinned so far.  Note
	maximum is 2.
        -----------------*/

	ushort pin_count; 
	
	
        /*-----------------
	Define an array to keep track of pinned fonts & DMA setup per
        font basis for this adapter.

	When a font fault occurs, we process the interrupt and then enqueue
 	a command to ask fkproc to pin the font.  Fkproc calculates font 's
	address and length, pins it , sets up DMA (via d_master), and sends 
        PCB command to signal DMA transferring by adapter.  This structure
	is used to save information passed to d_master so that later on
   	(when the same font is request to be unpinned) we have to issue
	d_complete to undo DMA setup. 
        -----------------*/

/*	pinned_font pin_list[MAX_PIN_FONTS_ALLOW]; */


	/*-----------------
	There is a set number of bus address ranges defineed for DMA'ing
	fonts to the adapter.

	This structure will be used to track allocation and use of these 
	bus address ranges as different fonts are loaded, as well as to
	pass this data to fkproc (font kernel process) for setting up the
	actual DMA.
	-----------------*/

	fkproc_font_pin_req_t font_DMA[MAX_PIN_FONTS_ALLOW];

	
	/*--------------------------------------------------------------------* 
	   Internal trace support data
	 *--------------------------------------------------------------------*/
#	include "ddftrcstruct.h"


	/*--------------------------------------------------------------------* 
            spares for field patches
	 *--------------------------------------------------------------------*/
	ulong	bim_slow_access_count ;
	char    reserved_for_future_use[26];

} midddf_t ;

#define MID_INTR_PRIORITY(ddf) ((ddf)->phys_disp->interrupt_data.intr.priority)


#endif /* _H_MIDDDF */
