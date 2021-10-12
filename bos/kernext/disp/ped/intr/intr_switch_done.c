static char sccsid[] = "@(#)94  1.2.1.4  src/bos/kernext/disp/ped/intr/intr_switch_done.c, peddd, bos411, 9428A410j 4/8/94 16:13:05";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: intr_switch_done
 *		
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


/****************************************************************************** 
   Includes:  
 ******************************************************************************/

#include <sys/types.h>
#include <sys/intr.h>
#include <sys/syspest.h>
#include <sys/display.h>

#include "hw_dd_model.h"
#include "hw_dsp.h"
#include "midddf.h"
#include "midhwa.h"
#include "hw_regs_k.h"
#include "hw_macros.h"
#include "hw_PCBkern.h"
#include "midRC.h"

#include "mid_dd_trace.h"

MID_MODULE (intr_switch_done);


/*************************************************************************** 
   Externals defined herein: 
 ***************************************************************************/

extern long intr_switch_done (
				struct phys_displays *, 
				midddf_t * );


  


/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    intr_switch_done 
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        SLIH, processor for "Context Switch Complete"
      

 *--------------------------------------------------------------------------* 
  
   Function:                                      
      This routine handles "Context Switch completion" interrupt status.
      Note, however, that we do NOT, in general, handle this condition as
      an interrupt.  Normally, at the start of a context switch we check if the 
      previous switch has finished.  If it has, we simply reset the status bit
      and proceed. 

      If, however, the previous context switch did not complete (before we 
      attempt the next), the switch routine will go to sleep (via the heavy
      switch controller) and wait for the completion -- via this interrupt.

      This is the only case that this module handles. 


      Note again, that the heavy switch routine performs the actual sleep. 
      Due to race conditions, it is possible that the interrupt is posted    
      before the heavy switch routine gets control and goes to sleep. 
      There are a couple of domain flags that indicate what state the heavy 
      switch routine is in.  The ones important to this routine are: 
        . WAITING_FOR_PREVIOUS_COMPLETION - set as soon as the start switch 
			routine determines the switch must be postponed.
        . SLEEPING_FOR_PREVIOUS_COMPLETION - set when the heavy switch 
			routine gets control and sleeps.
        . PREVIOUS_SWITCH_ALREADY_COMPLETED - set by this routine if the
			interrupt occurs before the sleep.
      

   Notes:                                         
      This routine does performs I/O to alter the BIM interrupt mask.
      (The bit corresponding to this interrupt is reset here, so we  
      we do not get this interrupt on the next context switch.  If we  
      must handle the interrupt again on the next context switch, the  
      start_switch routine will turn the interrupt (mask) back on.   


 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
                  none
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:

	   This routine is called from the SLIH (midintr.c) to handle
	   the context switch completed status bit.

  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the physical device structure
            . a pointer to the device dependent physical device structure 
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
                                 End of Prologue                                
 ******************************************************************************/

  


long intr_switch_done ( 
			struct phys_displays 	*pd,
			midddf_t		*ddf  )
{

	ulong	bim_mask ;

	HWPDDFSetup;			/* gain access to the hardware */




	/*------------------------------------------------------------------*
   	   Trace the entry parms
	 *------------------------------------------------------------------*/

        BUGLPR(dbg_intr_switch_done, 1, ("Entering intr_switch_done, \n" ));



	/*------------------------------------------------------------------*
	  Trace the input 
	 *------------------------------------------------------------------*/

    	MID_DD_ENTRY_TRACE (intr_switch_done, 1, INTR_SWITCH_DONE, ddf, pd, 
					ddf, ddf -> dom_flags, pd) ;


	/*------------------------------------------------------------------*
	  Turn off the context switch completion interrupt mask 
	 *------------------------------------------------------------------*/
#define  MID_K_HOST_IO_CONTEXT_SWITCH_COMPLETED	 MID_K_HOST_IO_DSP_SOFT_INTR0

	 bim_mask = ddf->host_intr_mask_shadow ; 
	 bim_mask &=  ~(MID_K_HOST_IO_CONTEXT_SWITCH_COMPLETED) ;
	 ddf->host_intr_mask_shadow = bim_mask ;
	 MID_WR_HOST_INTR (bim_mask) ; 



	/*------------------------------------------------------------------*
	  Check to ensure that this is NOT an extraneous status code
	   from the adapter. 
	 *------------------------------------------------------------------*/

	if ( !((ddf -> dom_flags) & WAITING_FOR_PREVIOUS_COMPLETION) &&
	     !((ddf -> dom_flags) & HOT_KEY_SLEEPING) )
    	{
        	BUGLPR(dbg_intr_switch_done, 0, ("BOGUS: SW COMP INTR \n\n"));
		BUGLPR(dbg_intr_switch_done, 0,("dom flags = %x",ddf->dom_flags));
		return (MID_ERROR) ;
    	}




        /*******************************************************************

          INTERRUPT is VALID

          Check to set if the heavy switch routine is sleeping yet

        *******************************************************************/

        BUGLPR(dbg_intr_switch_done, 2, ("WAITING for completion \n"));

	if ( ((ddf -> dom_flags) & SLEEPING_FOR_PREVIOUS_COMPLETION) )      
    	{
		BUGLPR(dbg_intr_switch_done, 1, ("Heavy switch sleeping\n"));

      		/*-----------------------------------------------* 
      		  Heavy switch is sleeping; wake it up. 
      		 *-----------------------------------------------*/
                ddf -> dom_switch_sleep_flag = 1 ;   

		if ( ddf->num_graphics_processes )
    			e_wakeup ( &(ddf -> dom_switch_sleep_word) ) ;          
    	}

	else if ( ((ddf -> dom_flags) & HOT_KEY_SLEEPING) )
	{
		BUGLPR(dbg_intr_switch_done, 1, ("Hotkey switch sleeping\n"));

		/*-----------------------------------------------*
		  Heavy switch is sleeping; wake it up.
		 *-----------------------------------------------*/

                ddf -> dom_switch_sleep_flag = 1 ;   

		if ( ddf->num_graphics_processes )
			e_wakeup ( &(ddf -> dom_switch_sleep_word) ) ;
	}

	else  /* heavy switch is NOT yet sleeping */
    	{
       		BUGLPR(dbg_intr_switch_done, 2, ("NOT yet asleep \n"));

    		ddf -> dom_flags |= PREVIOUS_SWITCH_ALREADY_COMPLETED ;
    	}




        /********************************************************************* 
           Trace at the exit point 
         *********************************************************************/

        MID_DD_EXIT_TRACE (intr_switch_done, 2, INTR_SWITCH_DONE, ddf, pd, 
    					bim_mask, ddf -> dom_flags, 0xF0); 

	BUGLPR(dbg_intr_switch_done, BUGNTX, ("Leaving switch_done \n"));
	return (0);

}

