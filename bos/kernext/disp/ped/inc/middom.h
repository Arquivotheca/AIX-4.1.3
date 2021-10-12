/* @(#)93	1.7.2.2  src/bos/kernext/disp/ped/inc/middom.h, peddd, bos411, 9428A410j 3/24/94 15:23:20 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: *		
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


#ifndef _H_MIDDOM
#define _H_MIDDOM


/*---------------------------------------------------------------------------*
  Miscellaneous definitions: 
    DI = device independent  
    DD = device dependent  
    RCM = rendering context manager
    WID = Window ID 
    mid = MID = mid-level graphics adapter (Pedernales, Lega)
 *---------------------------------------------------------------------------*/





/******************************************************************************

   Device dependent DOMAIN data fields

 ******************************************************************************/


	/*-------------------------------------------------------------------*
	   DMA fields 

	   The following fields are used to allow separate bus address 'windows'
	   for DMA of fonts and contexts.  These fields are initialized in
	   cfg_init.c.
	 *-------------------------------------------------------------------*/
	   
	ulong		font1_bus_addr;
	ulong		font2_bus_addr;
	ulong		ctx_DMA_host_init_base_bus_addr ;
	ulong		ctx_DMA_adapter_init_base_bus_addr ;


#define MID_FONT1_BASE_BUSADDR				0x600000
#define MID_FONT2_BASE_BUSADDR				0x780000
#define MID_CTX_HOST_INIT_BASE_BUSADDR			0x900000
#define MID_CTX_ADAPTER_INIT_BASE_BUSADDR		0x980000
/*---------------------------------------------------------------------------*
   means:   10M of bus space is allocated for the entire device driver. 
   	    This is split up as follows: 
   	      . the first 6M is used for ddf DMA operations (pixel blits),
   	      . the next 3M is used for font operations, and
	      . the last 1M is for heavy context switches.

   	    The font area is split into two pieces to allow two fonts to be
	    open (available to the adapter) at one time.

   	    The context area is split into two pieces because a context restore
	    (host initiated) must be requested (and set up) as part of the 
	    context switch command and off-load context switches (adapter 
	    initiated) may need to occur before the context restore. 

   Use:     Add these symbols to d_dma_area[0].bus_addr to get to the 
	    correct offset for the corresponding DMA.
 *--------------------------------------------------------------------------*/





	/*--------------------------------------------------------------------* 
	   Contains the address of the domain's (adapter's) current context. 
	 *--------------------------------------------------------------------*/
	mid_rcx_t  *current_context_midRCX  ;	/* midRCX for the current ctx */


	/*--------------------------------------------------------------------* 
	   Contains the address of the previous context. 
	 *--------------------------------------------------------------------*/
	rcx	*previous_context_RCX  ;	/* RCX for previous ctx */


	/*--------------------------------------------------------------------* 
	   Contains the address of the default context. 
	 *--------------------------------------------------------------------*/
	rcx	*default_context_RCX  ;		/* RCX for the default ctx */





	/*-------------------------------------------------------------------*
	   more context switch DMA fields 

	   Two context DMA operations can be in "process" at any time.  
	   This simply means that we may have two bus areas prepped for DMA 
	   at one time.  (The two operations are a system requested context    
	   restore and an adapter requested context offload.) 
	   
	   We keep sufficient information here to perform the d_complete 
	   for those operations (for each of those operations).

	     . (addr of) the context being DMA'ed (not req'd for dcomplete),
	     . the (address of the) cross memory descriptor, 
	     . system (virtual) memory address (from context),
	     . bus address, (BASE context DMA address + offset),    
	     . length,
	     . channel ID, and
	     . flags. 

	 *-------------------------------------------------------------------*/
	   
	mid_rcx_t	*ctx_DMA_HI_midRCX ; 
	int 		 ctx_DMA_HI_channel ; 
	int 		 ctx_DMA_HI_flags ; 
	char		*ctx_DMA_HI_sys_addr ; 
	char    	*ctx_DMA_HI_bus_addr ; 
	int 		 ctx_DMA_HI_length ; 
	struct 	xmem 	*ctx_DMA_HI_xs ;	



	mid_rcx_t	*ctx_DMA_AI_midRCX ; 
	int 		 ctx_DMA_AI_channel ; 
	int 		 ctx_DMA_AI_flags ; 
	char		*ctx_DMA_AI_sys_addr ; 
	char    	*ctx_DMA_AI_bus_addr ; 
	int 		 ctx_DMA_AI_length ; 
	struct 	xmem 	*ctx_DMA_AI_xs ;	




	/*--------------------------------------------------------------------* 
	   The flags describe the state of the domain during a context switch.  
	   (The flags are defined below - after all the fields are defined.)
	 *--------------------------------------------------------------------*/
	ushort	volatile	dom_flags ;	/* flags, defs at bottom */


	/*--------------------------------------------------------------------* 
	   There are a couple of condition for which the mid-level adapter 
	   device driver's heavy switch controller (HSC) sleeps:  
	    . previous switch completion and 
	    . fifo available.
	   
	   Here are the respective sleep words. 
	 *--------------------------------------------------------------------*/
	int	dom_switch_sleep_word ;	/* wait here for previous to complete */
	int	dom_fifo_sleep_word ;	/* wait here for fifo available       */
	int	volatile	dom_switch_sleep_flag ;
	int	volatile	dom_fifo_sleep_flag ;


        /*--------------------------------------------------------------------*
           It is necessary to keep a watchdog timer for the low water interrupt
           because after a process is blocked from a High Water Mark interrupt,
	   we are depending on a subsequent interrupt (ostensibly, the adapter's
	   low water interrupt) to unblock the process.  
         *--------------------------------------------------------------------*/
        mid_watch_lowwater_int_t mid_lowwater_watchdog ;

#define LOW_WATER_WATCHDOG_PERIOD 10		/* 10 sec. LWM timeout */


#define RCX_2D_LWM_MARK		3750		/* words, relative to bottom */
#define RCX_3DM1_LWM_MARK	2095		/* words, relative to bottom */
#define RCX_3DM2_LWM_MARK	2095		/* words, relative to bottom */

#define HIGH_WATER_THRESHOLD	48		/* words, relative to top */

	/* The following FIFO sizes are all in words. */  

#define MID_RCX_2D_HIGH_WATER_FIFO_SIZE		4*1024
#define MID_RCX_3DM1_HIGH_WATER_FIFO_SIZE	16*1024
#define MID_RCX_3DM2_HIGH_WATER_FIFO_SIZE	4*1024



	/*--------------------------------------------------------------------* 
	   During a context switch it is necessary to delay WID writes until    
	   the adapter can handle them (after it finishes its "new context
	   processing").  Therefore, we keep a list of window geometries
	   (WGs) for which WID writes were attempted during a context switch. 
	   The following fields maintain top and bottom pointers.
	 *--------------------------------------------------------------------*/
	midWG_t		*dom_delayed_WID_writes_top ;
	midWG_t		*dom_delayed_WID_writes_bot ;


	/*--------------------------------------------------------------------* 
	   We also keep a list of WIDs requiring Assoc color Palette updates.
	   Here is a count field and an array. 
	 *--------------------------------------------------------------------*/
	ulong		dom_delayed_Assoc_updates_count ;
	mid_wid_t	dom_delayed_Assoc_updates[MID_NUM_WIDS] ;










/******************************************************************************
   DOMAIN Context switching state flags

   The following flags define the various states of a mid-level graphics 
   adapter domain, during a context switch. 
 ******************************************************************************/


#define	MID_CONTEXT_SWITCH_IN_PROCESS  (1L<<0)     
/*----------------------------------------------------------------------------*
    means:  A context switch is in process for this domain
    init:  to 0 when the structure is allocated 
   
    Set:  At the start of the context switch (by mid_start_switch).
    Reset:  At the end of the context switch: 
		. by the interrupt handler, for all switches
		. by the interrupt handler, for light switches
    Referenced:  By the get WID routine to determine whether to write
    		  the WID planes or not. 
 *----------------------------------------------------------------------------*/




#define	CONTEXT_UPDATES_REQUIRED  (1L<<1)     
/*----------------------------------------------------------------------------*
    means:  Either the WID planes or the Window Parameters must be written. 
            Both must be written if the window's geometry changes or if the 
            a new window ID is assigned (because of WID sharing). 
    init:  to 0 when the structure is allocated 
   
    Set:   . update window geometry, if the window is not the current window
           . The start switch routine, if the WID changes (as a result of the
              get WID for render). 
    Reset:  When the updates are issued (in the interrupt handler).
 *----------------------------------------------------------------------------*/




#define	HEAVY_SWITCHING  (1L<<2)     
/*----------------------------------------------------------------------------*
    means:  The context switch in process is also a heavy switch
    init:  to 0 when the context switch begins  
   
    Set:  When it is determined that a heavy switch is necessary.     
          (This is done by the start switch routine (mid_start_switch) 
	  to determine what return code to pass back to the DI layer. 
    Reset:  When the DD heavy switch controller finishes
 *----------------------------------------------------------------------------*/




#define	SLEEPING_FOR_FIFO_AVAILABLE   (1L<<3)     
/*----------------------------------------------------------------------------*
    means:  The DD heavy switch routine has gotten control and gone to sleep
             waiting for the FIFO available event. 
    init:  to 0 when the context switch begins  
   
    Set:  By the DD heavy switch routine (mid_end_switch.c) if the FIFO 
	   available is not received before the heavy switcher gets control. 
    Reset:  By the same routine after it is awoken 
 *----------------------------------------------------------------------------*/





#define	WAITING_FOR_FIFO_AVAILABLE   (1L<<4)    
/*----------------------------------------------------------------------------*
    means:  The heavy switch routine is waiting for FIFO available.

    Reset/init:  to 0 when the context switch begins  
    Set:  By the DD wait_fifo routine which checks for the fifo available
           up to the 20 microseconds, the adapter is spec'ed to respond, but
           takes longer anyway.
 *----------------------------------------------------------------------------*/





#define	FIFO_ALREADY_AVAILABLE   (1L<<5)     
/*----------------------------------------------------------------------------*
    means:  The FIFO Available event has occurred before the heavy switch
    		has gotten control. 
    init:  to 0 when the context switch begins  
   
    Set:  When the FIFO available event is received (by the interrupt handler)

    		NOTE:  The context switch must be a heavy one, if the FIFO
    			available is received by the interrupt handler. 
    Reset:  By the heavy switch routine. 
 *----------------------------------------------------------------------------*/


#define	WAITING_FOR_PREVIOUS_COMPLETION   (1L<<6)
/*----------------------------------------------------------------------------*
    means:  The heavy switch routine is waiting for the previous context switch
             to finish. 

    Reset/init:  to 0 when the context switch begins  
    Set:  By the DD switch_end routine which checks for the previous completion
           before proceding with the next.    
 *----------------------------------------------------------------------------*/


#define	SLEEPING_FOR_PREVIOUS_COMPLETION   (1L<<7)
/*----------------------------------------------------------------------------*
    means:  The heavy switch routine has gotten control with the previous flag
	    set and before the context switch actually completed (see next 
	    flag).  In this event the heavy switch routine sleeps. 

    Reset/init:  to 0 when the context switch begins  
    Set:  By the DD switch_end routine which checks for the previous completion
           before proceding with the next.    
 *----------------------------------------------------------------------------*/



#define	PREVIOUS_SWITCH_ALREADY_COMPLETED	(1L<<8)     
/*----------------------------------------------------------------------------*
    means:  The Context switch completion event has occurred before the heavy 
		switch routine has gotten control. 
    init:  to 0 when the context switch begins  
   
    Set:  When the context completion is received (by the interrupt handler)
   	   and WAITING_FOR_PREVIOUS_COMPLETION is set and SLEEPING_FOR_PREVIOUS
    	   is not set. 
    Reset:  By the heavy switch routine. 
 *----------------------------------------------------------------------------*/






#define	PREVIOUS_SWITCH_FINALLY_COMPLETED	(1L<<9)     
/*----------------------------------------------------------------------------*
    means:  The Context switch completion event has occurred and the heavy    
		switch routine has gotten control.  It may or may not have
		slept, but has already done so if it was required. 
    init/reset:  to 0 when the context switch begins  
   
    Set:  When the context completion is processed by the heavy switch routine
   	   (after we attempted a context switch before the previous one 
	   completed). 
 *----------------------------------------------------------------------------*/





#define	HOST_INITIATED_CTX_DMA_IN_PROCESS	(1L<<10)     
/*----------------------------------------------------------------------------*
    means:  The switch routine has instructed the adapter to restore a context
		from system memory.  This also means a d_master operation has
		been performed and we can expect a DMA completion at which time
	  	a d_complete will be performed. 

    init:  to 1 or 0 when the context switch begins (as appropriate)
   
    Reset:  When the DMA completion is processed by the interrupt handler 
 *----------------------------------------------------------------------------*/





#define	ADAPTER_INITIATED_CTX_DMA_IN_PROCESS	(1L<<11)     
/*----------------------------------------------------------------------------*
    means:  The adapter has requested context save area to be pinned so a
		context can be removed from the adapter.  This also means a
		d_master operation has been performed and we can expect the
		adapter to report a DMA completion at which time a d_complete 
		will be performed. 

    init:  to 0 at adapter initialization time 
   
    Set:    When the adapter requests context memory (and the d_master is 
		performed).
    Reset:  When the DMA completion is processed by the interrupt handler 
 *----------------------------------------------------------------------------*/



#define HOT_KEY_IN_PROGRESS    (1L<<12)
/*----------------------------------------------------------------------------*
    means:  The user has initiated a hotkey. In this case the current rcx
	    will be rolled off the adapter via dma. The adapter responds
	    with a dsp_code of 7 which indicates the dma is complete.

    init:   to 0 at adapter initialization time

    Set:    In vttdact before we start rolling contexts off.

    Reset:  When the last real context is replaced by the default context.
 *----------------------------------------------------------------------------*/

#define HOT_KEY_SLEEPING    (1L<<13)
/*----------------------------------------------------------------------------*
    means:  The user has initiated a hotkey. In this case the current rcx
	    will be rolled off the adapter via dma. This flag indicates to
	    the switch routines that the context dma is being synced up and
	    needs to be woken up on completion.

    init:   to 0 at adapter initialization time

    Set:    In vttdact before we go to sleep waiting on the last context
	    switch to complete.

    Reset:  When the last real context is replaced by the default context.
 *----------------------------------------------------------------------------*/



#define MID_CONTEXT_DELETION_IN_PROCESS    (1L<<14)
/*----------------------------------------------------------------------------*
    means:  We are deleting a context.  Part of this processing may cause the
	    default context to be rolled onto the adapter.  If this happens
	    we want to inhibit WID writes. 

    init:   to 0 at adapter initialization time

    Set:    in mid_delete_rcx

    Reset:  in mid_delete_rcx

    Referenced:  in dsp_newctx (before writing WIDs for the default context)
 *----------------------------------------------------------------------------*/





#endif /* _H_MIDDOM */
