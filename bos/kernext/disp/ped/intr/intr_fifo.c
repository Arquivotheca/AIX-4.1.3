static char sccsid[] = "@(#)91  1.4.1.6  src/bos/kernext/disp/ped/intr/intr_fifo.c, peddd, bos411, 9428A410j 4/8/94 16:12:56";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: intr_fifo_av
 *		intr_fifo_hwm
 *		intr_fifo_lwm
 *		mid_timeout_lowwater
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


/****************************************************************************** 
   Includes:  
 ******************************************************************************/

#include <sys/types.h>
#include <sys/intr.h>
#include <sys/syspest.h>
#include <sys/display.h>
#include <sys/watchdog.h>       /* for watchdog timer initialization    */

#include "hw_dd_model.h"
#include "hw_dsp.h"
#include "midddf.h"
#include "midhwa.h"
#include "hw_regs_k.h"
#include "hw_macros.h"
#include "hw_PCBkern.h"

#include "rcm_mac.h"
#include "mid_rcm_mac.h"
#include "mid_dd_trace.h"

MID_MODULE (intr_fifo);
#define dbg_middd dbg_intr_fifo


/*************************************************************************** 
   Externals defined herein: 
 ***************************************************************************/

extern long intr_fifo_av (
			struct phys_displays *, 
			midddf_t * );

extern long intr_fifo_hwm (
			struct phys_displays *, 
			midddf_t * );

extern long intr_fifo_lwm (
			struct phys_displays *, 
			midddf_t * );

extern long mid_high_water_com (
			midddf_t *, 
			mid_rcx_t * );

extern long mid_low_water_com (
			midddf_t *, 
			mid_rcx_t * );

extern long mid_check_water_level (
				midddf_t *, 
				ulong );

extern long mid_timeout_lowwater ( mid_watch_lowwater_int_t * );


  


/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    intr_fifo_av 
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        SLIH, processor for "FIFO Available" Status
      

 *--------------------------------------------------------------------------* 
  
   Function:                                      
      This routine handles "FIFO Available" (BIM) adapter status.

      This simply entails "waking up" the heavy switch routine.
      Actually, due to race conditions within the machine, it is possible
      that the heavy switch routine may not yet be sleeping.  In this event,
      we merely set a flag indicating the "FIFO AVAILABLE" condition has 
      occurred.  This flag is checked by the heavy switch routine and 
      prevents the sleep/wakeup from ever occurring.


   Notes:                                         
      This routine does NO I/O. 


 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
                  none
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:

	   This routine is called as part of the processing for the
	   "New Context Processing Completion" DSP status.   The processing 
	   routine (dsp_newctx) calls this routine to handle the FIFO 
	   available condition (which is implicit with the "New Context
	   Processing complete" status). 

	   Note that it is possible to receive the FIFO available as
	   a separate (BIM) status interrupt from the adapter, however,
	   it is desireable to NOT handle the FIFO available as
	   a separate interrupt, since this requires two trips through the
	   the interrupt handling path. 

  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the physical device structure
            . a pointer to the device dependent physical device structure 
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
                                 End of Prologue                                
 ******************************************************************************/

  


long intr_fifo_av ( 
		struct phys_displays 	*pd,
		midddf_t		*ddf  )
{

	rcx        *pRCX ;
	mid_rcx_t  *pmidRCX ;
	ulong	   mask ;
	ulong	   status ;

	HWPDDFSetup;			/* gain access to the hardware */




	/*------------------------------------------------------------------*
   	   Trace the entry parms
	 *------------------------------------------------------------------*/

        BUGLPR(dbg_intr_fifo, 1, ("Entering intr_fifo_av, \n" ));



	/*------------------------------------------------------------------*
	  Init some variables:  

	    . Current (new) context pointer 
	    . midRCX pointer for the same context 

	 *------------------------------------------------------------------*/

	pmidRCX =  ddf -> current_context_midRCX ; 
	pRCX = pmidRCX -> pRcx ; 



    	MID_DD_ENTRY_TRACE (intr_fifo, 2, INTR_FIFO_AVAILABLE, ddf, pd, 
				pRCX, ddf -> dom_flags, pmidRCX ) ;


	/*------------------------------------------------------------------*
	  Check to ensure that this is NOT an extraneous status code
	   from the adapter. 
	 *------------------------------------------------------------------*/

    	if ( !((ddf -> dom_flags) & MID_CONTEXT_SWITCH_IN_PROCESS) ) 
    	{
        	BUGLPR(dbg_intr_fifo, 0, ("CTX switch NOT IN PROGRESS\n\n"));
        	return (-1) ;
    	}

        BUGLPR(dbg_intr_fifo, 2, ("CTX switch IS IN progress\n"));


	/*------------------------------------------------------------------*
	  Reset context switch in progress to flag multiple interrupts
	   as extraneous.    
	 *------------------------------------------------------------------*/

    	ddf -> dom_flags &= ~(MID_CONTEXT_SWITCH_IN_PROCESS) ; 



        /*******************************************************************
          Check for the existence of any spurious Low Water Mark interrupt.
          This should not occur but if it does we need to reset it and log
          an error.
        *******************************************************************/

        status = ddf->raw_bim_status ; 
        if( status & MID_K_HOST_IO_LOW_WATER_MARK )
        {
                /*-----------------------------------------------*
                 Discard the spurious LWM interrupt at this time.
                *------------------------------------------------*/

		MID_WR_HOST_STAT ( MID_K_HOST_IO_LOW_WATER_MARK ) ;
                MID_DD_TRACE_PARMS (intr_fifo, 2,
                                        UNEXPECTED_LWM_INT, ddf,
                                        pd, *(ulong *)(&(pmidRCX -> flags)),
                                        status,
                                        ddf -> dom_flags ) ;
                BUGLPR(dbg_intr_fifo, 0,
                         ("Unexpected LWM interrupt has occurred\n\n"));
        }





	if ( ((ddf -> dom_flags) & HEAVY_SWITCHING) )      
    	{
        	BUGLPR(dbg_intr_fifo, 3, ("Heavy switch is process \n"));

        	/***********************************************************
        	 ***********************************************************

      	 	   HEAVY SWITCH in PROCESS 

      	 	   We must: 
      	 	     . check for a pending low water condition 
      	 	     . manage the BIM status mask bits (for High Water
      	 	     	and Low Water),
      	 	     . either wakeup the heavy switch routine or set a flag 
			telling it to "NOT sleep". 

        	 ***********************************************************
        	***********************************************************/


	

		/*----------------------------------------------------------*
		 If there is a pending low water condition, handle it now    
		 (for the heavy switch case). 
		*-----------------------------------------------------------*/
	
		if ( pmidRCX-> flags.low_water_condition_pending )
		{
			mid_low_water_com (ddf, pmidRCX) ;
		}



		/*----------------------------------------------------------*
		 We need to update the interrupt mask to include the correct 
		 settings of the high and low water mark interrupt bits.

		 First however, we will reset any pending High Water 
		 interrupts.  (An unavoidable interrupt occurs if the
		 FIFO is restored to a value above the High Water Mark.)
		*-----------------------------------------------------------*/
	
                if ( status & MID_K_HOST_IO_PIO_HI_WATER_HIT )
                {
                        MID_WR_HOST_STAT ( MID_K_HOST_IO_PIO_HI_WATER_HIT ) ;

                        MID_DD_TRACE_PARMS (intr_fifo, 2,
                                        UNEXPECTED_HWM_INT, ddf,
                                        pd, *(ulong *)(&(pmidRCX -> flags)),
                                        status,
                                        ddf-> dom_flags ) ;
                }


		mask = ddf->host_intr_mask_shadow ;
	
		if ( pmidRCX -> flags.waiting_for_FIFO_low_water_mark )
		{
			mask |= MID_K_HOST_IO_LOW_WATER_MARK ;
			mask &= ~MID_K_HOST_IO_PIO_HI_WATER_HIT ;
		}
		else
		{
			mask &= ~MID_K_HOST_IO_LOW_WATER_MARK ;
			mask |= MID_K_HOST_IO_PIO_HI_WATER_HIT ;
		}
	
		if ( mask ^ ddf->host_intr_mask_shadow ) 
		{
			MID_DD_TRACE_PARMS (intr_fifo, 4, 
					START_SWITCH_INT_MASK, ddf,
					pd, *(ulong *)(&(pmidRCX -> flags)),
					mask,
					ddf -> dom_flags ) ;
	
			ddf->host_intr_mask_shadow = mask ;
			MID_WR_HOST_INTR (mask) ;
			BUGLPR(dbg_intr_fifo, 2,
					("Interrupt mask written = 0x%4X\n", 
					mask));
		}
	


                /*--------------------------------------------------------*
                 Ensure the heavy switch gets started up again.  This is 
                 either a wakeup or flag management (if the sleep hasn't
                 occurred yet).
                 *--------------------------------------------------------*/

		if ( ((ddf -> dom_flags) & SLEEPING_FOR_FIFO_AVAILABLE ) )      
    		{
        		BUGLPR(dbg_intr_fifo, 2, ("Sleeping.  WAKE UP !!\n"));

			ddf -> dom_fifo_sleep_flag = 1;	

                        if ( ddf->num_graphics_processes )
                        /*-----------------------------------------------*
                          Heavy switch is sleeping; wake it up.
                         *-----------------------------------------------*/
                                e_wakeup ( &(ddf -> dom_fifo_sleep_word) ) ;
    		}

		else  /* heavy switch is NOT yet sleeping */
    		{
        		BUGLPR(dbg_intr_fifo, 2, ("NOT yet sleeping. \n"));

    			ddf -> dom_flags |=  FIFO_ALREADY_AVAILABLE ;           
    		}

    	}    /* end of HEAVY SWITCH in PROCESS */



        /********************************************************************* 
           Trace at the exit point 
         *********************************************************************/

        MID_DD_EXIT_TRACE (intr_fifo, 1, INTR_FIFO_AVAILABLE, ddf, pd, 
    				mask, ddf -> dom_flags, 0xF0); 

	BUGLPR(dbg_intr_fifo, BUGNTX, ("Leaving fifo_available \n"));
	return (0);

}






  

/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    intr_fifo_hwm 
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        SLIH, processor for "High Water Mark" Status
      

 *--------------------------------------------------------------------------* 
  
   Function:                                      
      This routine handles "High Water Mark" (BIM) adapter status.

      This means the application has put a lot of commands in the FIFO
      (and is way ahead of the adapter).   Therefore, we will let the 
      adapter try to catch up by putting the application to sleep. 

      Other than that, we do a small amount of administrative tasks to     
      remember we are sleeping because of a full FIFO: 
        . set a flag, 
        . change the BIM status mask to enable the "low water mark"
           interrupt, should the FIFO clear. 


   Notes:                                         
      1. This routine DOES I/O. 


 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
                  none
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
           This routine is called from the mid-level graphics adapter SLIH
	   (midintr) when FIFO "High Water Mark" status is received from 
	   the adapter. 

  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the physical device structure
            . a pointer to the device dependent physical device structure 
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
                                 End of Prologue                                
 ******************************************************************************/

 


long intr_fifo_hwm ( 
		struct phys_displays 	*pd,
		midddf_t		*ddf  )
{
	mid_rcx_t  *pmidRCX =  ddf -> current_context_midRCX ; 
	ulong	status_mask ;

	HWPDDFSetup ;



	/*------------------------------------------------------------------*
   	   Trace the entry parms
	 *------------------------------------------------------------------*/

        BUGLPR(dbg_intr_fifo, 1, ("Entering intr_fifo_hwm, \n" ));

    	MID_DD_ENTRY_TRACE (intr_fifo, 2, INTR_FIFO_FULL, ddf, pd, 
  			pmidRCX, 
			* (ulong *)( &(pmidRCX -> flags)), 
			ddf ) ; 





	/*------------------------------------------------------------------*
	  Check to ensure that this is NOT an extraneous interrupt  
	   from the adapter. 
	 *------------------------------------------------------------------*/

    	if ( pmidRCX -> flags.waiting_for_FIFO_low_water_mark ) 
    	{
        	BUGLPR(dbg_intr_fifo, 0, ("CTX already ASLEEP \n\n"));
        	return (-1) ;
    	}

        BUGLPR(dbg_intr_fifo, 3, ("CTX NOT already asleep\n"));



#if 0
        MID_WR_HOST_STAT ( MID_K_HOST_IO_LOW_WATER_MARK ) ;
#endif
	/*------------------------------------------------------------------*
	  Most of the real function is in the following routine. 
	 *------------------------------------------------------------------*/

	mid_high_water_com (ddf, pmidRCX) ; 


	/*------------------------------------------------------------------*

          Now alter the adapter (BIM) status mask
            . disabling the high water mark and enabling the low water mark

	 *------------------------------------------------------------------*/

	status_mask = ddf->host_intr_mask_shadow ;
        BUGLPR(dbg_intr_fifo, 2, ("status mask read = %X\n", status_mask));

	status_mask |= MID_K_HOST_IO_LOW_WATER_MARK ;
	status_mask &= ~(MID_K_HOST_IO_PIO_HI_WATER_HIT);

	ddf->host_intr_mask_shadow = status_mask ;
	MID_WR_HOST_INTR (status_mask) ;


        BUGLPR(dbg_intr_fifo, 2, ("status mask written = %X\n", status_mask));



	/*------------------------------------------------------------------*
          Start watchdog
	 *------------------------------------------------------------------*/

	w_start(&(ddf->mid_lowwater_watchdog.dog));



        /********************************************************************* 
           Trace at the exit point 
         *********************************************************************/
	/* brkpoint(ddf, pmidRCX, 0xbeef); */

        MID_DD_EXIT_TRACE (intr_fifo, 1, INTR_FIFO_FULL, ddf, pd, 
				pmidRCX,  
				* (ulong *)( &(pmidRCX -> flags)), 
				0xF0 ) ;

	BUGLPR(dbg_intr_fifo, 2, ("Leaving fifo_hwm \n"));
	return (0);

}





  

/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    intr_fifo_lwm 
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        SLIH, processor for "Low Water Mark" Status
      

 *--------------------------------------------------------------------------* 
  
   Function:                                      
      This routine handles "Low Water Mark" (BIM) adapter status.

      This means the adapter's FIFO is empty (or near empty) because the
      the current process is not sending commands to the FIFO. 
      This could be caused by at least two things:
        . the application is simply not using the adapter right now.
           In this event, we will steal the adapter's graphics time slice.
        . or the application might have previously filled the FIFO, and
           been put to sleep because it was getting too far ahead of the  
           the adapter. 
           In this event, we will will simply wake up the graphics process
           (plus some other mundane bookkeeppiinngg stuff). 


   Notes:                                         
      1. This routine DOES I/O. 


 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
                  none
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
           This routine is called from the mid-level graphics adapter SLIH
	   (midintr) when FIFO "Low Water Mark" status is received from 
	   the adapter. 

  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the physical device structure
            . a pointer to the device dependent physical device structure 
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
                                 End of Prologue                                
 ******************************************************************************/

  



long intr_fifo_lwm ( 
		struct phys_displays 	*pd,
		midddf_t		*ddf  )
{
	mid_rcx_t  *pmidRCX =  ddf -> current_context_midRCX ; 
	ulong	status_mask ;

	HWPDDFSetup ;



	/*------------------------------------------------------------------*
   	   Trace the entry parms
	 *------------------------------------------------------------------*/

        BUGLPR(dbg_intr_fifo, BUGNTA, ("Entering intr_fifo_lwm, \n" ));

    	MID_DD_ENTRY_TRACE (intr_fifo, 2, INTR_FIFO_EMPTY, ddf, pd, 
				pmidRCX,  
				* (ulong *)( &(pmidRCX -> flags)), 
				ddf ) ;


	/*------------------------------------------------------------------*
	  If application was not previously put to sleep, then just ignore.
	 *------------------------------------------------------------------*/

    	if ( ! pmidRCX-> flags.waiting_for_FIFO_low_water_mark ) 
    	{
        	BUGLPR(dbg_intr_fifo, 1, ("APP not SLEEPING \n\n"));
        	return (0) ;
    	}

        BUGLPR(dbg_intr_fifo, 2, ("APP was ASLEEP \n"));




	/*------------------------------------------------------------------*
	  Most of the real function is in the following routine. 
	 *------------------------------------------------------------------*/

	mid_low_water_com (ddf, pmidRCX) ; 


	/*------------------------------------------------------------------*
          Stop the watchdog function(s) 
	 *------------------------------------------------------------------*/

	pmidRCX-> low_water_switch_count = 0;

	w_stop(&(ddf->mid_lowwater_watchdog.dog)); 


	/*------------------------------------------------------------------*

          I/O SECTION

          Reset any pending High Water Interrupts 

          Next alter the adapter (BIM) status mask
            . disabling the high water mark and enabling the low water mark

	 *------------------------------------------------------------------*/

	MID_WR_HOST_STAT(MID_K_HOST_IO_PIO_HI_WATER_HIT);


	status_mask = ddf->host_intr_mask_shadow ;
        BUGLPR(dbg_intr_fifo, 2, ("status mask read = %X\n", status_mask));

	status_mask &= ~(MID_K_HOST_IO_LOW_WATER_MARK) ;
	status_mask |=  (MID_K_HOST_IO_PIO_HI_WATER_HIT);

	ddf->host_intr_mask_shadow = status_mask ;
	MID_WR_HOST_INTR (status_mask) ;
        BUGLPR(dbg_intr_fifo, 2, ("status mask written = %X\n", status_mask));



        /********************************************************************* 
           Trace at the exit point 
         *********************************************************************/

        MID_DD_EXIT_TRACE (intr_fifo, 1, INTR_FIFO_EMPTY, ddf, pd, 
				pmidRCX,  
				* (ulong *)( &(pmidRCX -> flags)), 
				0xF0 ) ;

	BUGLPR(dbg_intr_fifo, 2, ("Leaving fifo_lwm \n"));
	return (0);

}


#include "mid_hicom.h"
#include "mid_lowcom.h"

#include "hw_regs_u.h"
#include "hw_regs_k.h"
#include "hw_ind_mac.h"
#include "mid_check_water_level.h"





long mid_timeout_lowwater ( mid_watch_lowwater_int_t *w_ptr)
{

	struct midddf	*ddf 	= (struct midddf*) (w_ptr->ddf) ; 
	gscDev		*pGD 	= ddf-> pdev ; 
	mid_rcx_t  	*pmidRCX = ddf-> current_context_midRCX ; 
	rcx        	*pRCX 	= pmidRCX-> pRcx ; 

	ulong      status_mask ; 

	HWPDDFSetup ; 


        MID_DD_ENTRY_TRACE (intr_fifo, 1, LOW_WATER_WATCHDOG, ddf, pGD, 
				pRCX, *(ulong *)( &(pmidRCX -> flags)), 
				0xEEEE0001 ) ;
        BUGLPR(dbg_intr_fifo, 0, ("\n !!!!!  LOW WATER TIMEOUT  !!!!! \n\n")) ;

    	MID_BREAK (LOW_WATER_WATCHDOG, ddf, pRCX, pmidRCX, 
					*(ulong *)( &(pmidRCX->flags)), 0);

    	mid_low_water_com ( ddf, pmidRCX) ; 


	/*------------------------------------------------------------------*
          Fix the status mask (so we look for a High Water Interrupt now)
	 *------------------------------------------------------------------*/

	PIO_EXC_ON() ;

	status_mask = ddf->host_intr_mask_shadow ;
        BUGLPR(dbg_intr_fifo, 2, ("status mask read = %X\n", status_mask));

	status_mask &= ~(MID_K_HOST_IO_LOW_WATER_MARK) ;
	status_mask |=  (MID_K_HOST_IO_PIO_HI_WATER_HIT);

	ddf->host_intr_mask_shadow = status_mask ;
	MID_WR_HOST_INTR (status_mask) ;

	PIO_EXC_OFF() ;

        BUGLPR(dbg_intr_fifo, 2, ("status mask written = %X\n", status_mask));



        MID_DD_EXIT_TRACE (intr_fifo, 1, LOW_WATER_WATCHDOG, ddf, pGD, 
				pRCX, *(ulong *)( &(pmidRCX -> flags)), 
				0xEEEE0001 ) ;
}
