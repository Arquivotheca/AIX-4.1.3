/* @(#)63       1.6  src/bos/kernext/disp/ped/inc/midddfswap.h, peddd, bos411, 9428A410j 3/19/93 18:54:39 */
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


#ifndef _H_MIDDDFSWAP.H
#define _H_MIDDDFSWAP.H

#include "midrcx.h"  	/* for MAX_WIDS				*/




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*  These structures (the gets through the swapbuffer structure) are used  */
/*  to communicate correlator information and function data between the    */
/*  DDF function and its associated interrupt handler.                     */
/*                                                                         */
/*  These typedefs are used within the midddf structure below to reserve   */
/*  actual space for the associated data.                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                      DDF Swapbuffer Data Structures                     */
/*                                                                         */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*-------------------------------------------------------------------------*

    SWAP BUFFER WATCHDOG structure 

    This is the structure that is passed to the watchdog wakeup routine.

  - The entry address is the address of the swap table entry that contains
     this watchdog structure.  This is required to access the flags, the WID
     and perhaps the correlator counts.  
  - The ddf address is needed for the release_guarded_WID function.
  - The index field is simply the swap table index.  It is unneeded, but
     makes the entire swap table entry 12 words (which fits nicely on the
     debug screen). 

 *-------------------------------------------------------------------------*/

typedef struct mid_swap_watch {

        struct watchdog		dog ;

	ulong			*entry  ; 	/* swap table entry address */
	ulong			index  ; 	/* index into the swap table */
	ulong			*ddf   ; 	/* pointer to the top of ddf */

} mid_swap_watch_t ;





/*-------------------------------------------------------------------------*

    SWAP BUFFER TABLE ENTRY structure

    This structure is used to communicate correlator information and    
    function data between mid_swapbuffers() and intr_swapbuffers().          
                                                                             
    The second structure is used to hold up to 16 correlator values read     
    off of the adapter and which represent swaps serviced on the last        
    vertical retrace period.                                                 

 *-------------------------------------------------------------------------*/

#	define SB_ARY_SIZE MAX_WIDS


typedef struct ddf_SBdata {
        mid_wid_t               window_ID;

        ushort 			corr_sent ;   	/* sent count */
        ushort 			corr_recv ;   	/* sent count */

	ulong			flags ; 	/* frame buffer flag */
#define 	SWAP_FB_FLAG		MID_USING_BACK_BUFFER  
#define 	SWAP_SLEEPING  		(1<<1) 
#define 	SWAP_STILL_USING_WID    (1<<2)

	int			sleep_word ; 	

	mid_swap_watch_t	watch ; 	

} ddf_SBdata_t;


/*-------------------------------------------------------------------------- 
    The following structure is the data read from the adapter 
 *-------------------------------------------------------------------------*/

typedef struct ddf_swap_data {
	ulong			number_of_correlators;
	ushort			correlator[SB_ARY_SIZE];
} ddf_swap_data_t;



/*-------------------------------------------------------------------------- 
    This structure is the correlator sent to the adapter on each swap  
    command.  Keep in mind this is a 16 bit field.

      . The upper 8 bits (0-7) are a correlation count.
      . The next bit (bit 8) is a flag indicating what the resulting 
	 Displayed Frame Buffer is (after this Frame Buffer Control Command).
      . The next 3 bits (bits 9-11) are a function ID.
      . The bottom 4 bits (12-15) are the array index. 
 *-------------------------------------------------------------------------*/


#define  MID_SWAP_CORR_CORRELATION_COUNT_MAX 0xFF  /* max correlator count */
#define  MAX_OUTSTANDING_SWAPS 3    /* max number of swaps before sleeping */
#define  MID_SWAP_CORR_CORRELATION_COUNT_WAKEUP	2	/* awake threshold */

#define  MID_SWAP_CORR_CORRELATION_COUNT	0xFF00
#define  MID_SWAP_CORR_FRAME_BUFFER   		0x0080	/* 0=A, 1=B */
#define  MID_SWAP_CORR_FUNCTION_ID    		0x0070	/* see below */
#define  MID_SWAP_CORR_INDEX  			0x000F


/*-------------------------------------------------------------------------- 
    Here are the swap function definitions 
 *-------------------------------------------------------------------------*/

#define  MID_SWAP_DRAW_CHANGE_ONLY	(0L<<4)	/* Draw changes are NOPs */
#define  MID_SWAP_JUST_SWAP 		(1L<<4)	/* Normal swap */
#define  MID_SWAP_DISPLAY_BUFFER	(2L<<4)	/* Display buffer specified */


#endif /* _H_MIDDDFSWAP */
