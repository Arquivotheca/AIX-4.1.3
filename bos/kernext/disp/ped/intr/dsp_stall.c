/* @(#)78	1.2  src/bos/kernext/disp/ped/intr/dsp_stall.c, peddd, bos411, 9428A410j 3/19/93 18:56:26 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: dsp_stall
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
#include <sys/dma.h>

#include "hw_dd_model.h"
#include "hw_dsp.h"
#include "midddf.h"
#include "midhwa.h"


#include "hw_trace.h"
#include "hw_macros.h"
#include "hw_ind_mac.h"

#include "mid_rcm_mac.h"
#include "mid_dd_trace.h"

MID_MODULE (dsp_stall);


/*************************************************************************** 
   Externals defined herein: 
 ***************************************************************************/

long dsp_stall (
		struct phys_displays *, 
		midddf_t *  		);

  


/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    dsp_stall 
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        SLIH, dsp FIFO STALLED status processor
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
      Give up the graphics timeslice if appropriate:

	When a double-buffered application finishes drawing an image, a  
	"SWAP BUFFER" command is placed in the FIFO.  When the adapter
	finds the "SWAP BUFFER" command, it must wait until the next
 	vertical retrace to actually perform the swap.

	If there is an appreciable amount of time left, we will give up 
	the context and let another context run. 


   Notes:                                         
      This routine performas I/O to read the remaining time to retrace.


 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
                  none
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
           This routine is called from the dsp status router (intr_dsp.c).

  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the physical device structure
            . a pointer to the device dependent physical device structure 
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
                                 End of Prologue                                
 ******************************************************************************/

  




long dsp_stall ( 
		struct phys_displays 	*pd,
		midddf_t		*ddf)
{
	int old_intr;
	int stall_period ;

#	define HORIZ_RETRACE_PERIODS_MASK	0x0000ffff

	HWPDDFSetup;			/* gain access to the hardware */


        BUGLPR(dbg_dsp_stall, 1, ("Entering dsp_stall \n"));


	/*------------------------------------------------------------------*
   	   If a context switch is in progress, just ingore and exit.
	 *------------------------------------------------------------------*/

	if (ddf->dom_flags & MID_CONTEXT_SWITCH_IN_PROCESS) 
	{
		return (0) ;
	}


	/*------------------------------------------------------------------*
   	   Next read the amount of time left
	 *------------------------------------------------------------------*/
	MID_RD_CSB_VALUE(MID_CSB_STALL_TIME, stall_period);
	/* MID_RD_CSB_VALUE(MID_CSB_TYPE, ctx_type) ; */

	/*------------------------------------------------------------------*
   	   Finally trace (done after the read so we can get the time data) 
	 *------------------------------------------------------------------*/

    	MID_DD_ENTRY_TRACE (dsp_stall, 1, DSP_FIFO_STALLED, ddf, pd,
			ddf, ddf->current_context_midRCX, stall_period) ;


	if ((stall_period & HORIZ_RETRACE_PERIODS_MASK) > 200)   
	{
		/*----------------------------------------------------------*
		    give_up_time_slice (called later) will assert if we don't 
	 	   enter it with interrupts disabled at INTMAX.   
		 *----------------------------------------------------------*/
		old_intr = i_disable(INTMAX);

	   	(*ddf->pdev->devHead.pCom->rcm_callback-> give_up_timeslice) 
			 ( (ddf->current_context_midRCX->pRcx->pDomain), 
			   (ddf->current_context_midRCX->pRcx->pProc));

		i_enable(old_intr);
	}




        /********************************************************************* 
           Trace at the exit point 
         *********************************************************************/

        MID_DD_EXIT_TRACE (dsp_stall, 2, DSP_FIFO_STALLED, ddf, pd, 
				ddf, ddf->current_context_midRCX, 0xF0); 

	BUGLPR(dbg_dsp_stall, BUGNTX, ("Leaving dsp_stall \n"));
	return (0);
}
