static char sccsid[] = "@(#)89	1.4  src/bos/kernext/disp/ped/intr/dsp_pinctx.c, peddd, bos411, 9428A410j 3/19/93 18:56:15";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: dsp_pinctx
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
#include <sys/syspest.h>
#include <sys/intr.h> 
#include <sys/display.h>

#include "hw_dd_model.h"
#include "midhwa.h"

#include "hw_regs_k.h"
#include "hw_regs_u.h"
#include "hw_typdefs.h"
#include "hw_macros.h"
#include "hw_ind_mac.h"
#include "hw_PCBkern.h"

#include "midddf.h"
#include "mid_dd_trace.h"

MID_MODULE (dsp_pinctx);


/*************************************************************************** 
   Externals defined herein: 
 ***************************************************************************/

long dsp_pinctx (
		struct phys_displays *, 
		midddf_t * );




/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    dsp_pinctx 
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        SLIH, processor for "Pin Context Memory"
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
      This routine handles "Pin Context Memory" DSP status
      from the adapter.  This includes the following tasks: 
        . reading the Context Memory Request Block (CMRB) to determine
           what context to pin,
        . ensuring that memory space is pinned, and
        . notifying the adapter when it is pinned.

   Notes:                                         
      This routine runs on the interrupt level.  It is not possible for
      this routine to actually do the pin operation (call the pin routine).
      this is because the pin could result in a sleep, due to a paging fault.

      Currently, this problem is dealt with by leaving the context memory
      pinned at all times. 

      Someday, this routine will schedule a kernel process to complete the
      pin operation (and notify the adapter). 


 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
       1. This routine performs I/O.  It is assumed that bus
           access is provided by the caller. 
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
           This routine is called from the dsp status processor (intr_dsp)
	   when the "Pin Context Memory" dsp status is received 
	   from the adapter. 

  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the physical device structure
            . a pointer to the device dependent physical device structure 
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
                                 End of Prologue                                
 ******************************************************************************/



long dsp_pinctx ( 
		struct phys_displays 	*pd,
		midddf_t		*ddf  )
{

	rcx        *pRCX ;
	mid_rcx_t  *pmidRCX ;

	ulong		length ; 

	HWPDDFSetup;			/* gain access to the hardware */




	/*------------------------------------------------------------------*
   	   Trace the entry parms
	 *------------------------------------------------------------------*/
    	MID_DD_ENTRY_TRACE (dsp_pinctx, 3, DSP_PINCTX, ddf, pd, pd, ddf, 0 ) ;

        BUGLPR(dbg_dsp_pinctx, BUGNTA, ("Entering dsp_pinctx, \n" ));



	/*------------------------------------------------------------------*
	  Read the Context Memory Request Block

	    . Read the context ID first, then the length
	    . Then trace what was read from the adapter. 

	 *------------------------------------------------------------------*/

	MID_RD_CMRB_VALUE ( MID_CMRB_CONTEXT_ID, pmidRCX) ;

	MID_RD_CMRB_VALUE ( MID_CMRB_LENGTH, length) ;



	/*------------------------------------------------------------------*
   	   Trace the data read from the adapter
	 *------------------------------------------------------------------*/
    	MID_DD_TRACE_PARMS (dsp_pinctx, 1, DSP_PINCTX_DATA, ddf, pd, 
				ddf, pmidRCX, length ) ;

        



	/*------------------------------------------------------------------*
	  ERROR CHECKING 

	    Verify the context ID: 
	      . first by checking it for zero, 
	      . next, by checking the DI RCX it points to for 0, and
	      . finally, ensuring the DI RCX points back to the DD RCX.

	 *------------------------------------------------------------------*/

	if ( pmidRCX == NULL )
	{ 
        	BUGLPR(dbg_dsp_pinctx, 0, ("CONTEXT ID is NULL \n"));

        	MID_DD_EXIT_TRACE (dsp_pinctx, 2, DSP_PINCTX, ddf, pd, 
    					0, 0, 0xE1); 

        	return (-1) ; 
	} 



	if ( pmidRCX -> flags.default_context == 1)
	{ 
        	BUGLPR(dbg_dsp_pinctx, 0, ("Default context - no save area\n"));
        	BUGLPR(dbg_dsp_pinctx, 0, ("CONTEXT ID = 0x%X\n", pmidRCX));

        	MID_DD_EXIT_TRACE (dsp_pinctx, 2, DSP_PINCTX, ddf, pd, 
    					pmidRCX, 0, 0xE2); 

        	return (-1) ; 
	} 




	if ( pmidRCX -> flags.terminate_context == 1)
	{ 
        	BUGLPR(dbg_dsp_pinctx, 0, ("ctx DEFUNCT - ADAPTER ERROR \n"));
        	BUGLPR(dbg_dsp_pinctx, 0, ("CONTEXT ID = 0x%X\n", pmidRCX));

        	MID_DD_EXIT_TRACE (dsp_pinctx, 2, DSP_PINCTX, ddf, pd, 
   					pmidRCX, 0, 0xE3); 

	} 



	pRCX = pmidRCX -> pRcx ; 

	if ( pRCX == NULL )
	{ 
        	BUGLPR(dbg_dsp_pinctx, 0, ("RESULTING DI RCX is NULL \n"));
        	BUGLPR(dbg_dsp_pinctx, 0, ("CONTEXT ID = 0x%X\n", pmidRCX));

        	MID_DD_EXIT_TRACE (dsp_pinctx, 2, DSP_PINCTX, ddf, pd, 
    					pmidRCX, pRCX, 0xE4); 

        	return (-1) ; 
	} 


	if ( ((mid_rcx_t *)(pRCX -> pData)) != pmidRCX )
	{ 
        	BUGLPR(dbg_dsp_pinctx, 0, ("CTX ID = 0x%X BOGUS\n", pmidRCX));

        	MID_DD_EXIT_TRACE (dsp_pinctx, 2, DSP_PINCTX, ddf, pd, 
				((mid_rcx_t *)(pRCX -> pData)), pRCX, 0xE5); 

        	return (-1) ; 
	} 



	/* -------------------------------------------------------------- *
	  Validate the length 
	 *----------------------------------------------------------------- */

	if ( length > pmidRCX -> size )
	{ 
        	BUGLPR(dbg_dsp_pinctx, 0, 
		("DMA lenght 0x%X BAD (for type %d)\n", length,pmidRCX->type));

        	MID_DD_EXIT_TRACE (dsp_pinctx, 2, DSP_PINCTX, ddf, pd, 
				((mid_rcx_t *)(pRCX -> pData)), length, 0xE6); 

        	return (-1) ; 
	} 



	/* -------------------------------------------------------------- *
	  Save the parameters from the adapter
	*----------------------------------------------------------------- */
	ddf -> ctx_DMA_AI_midRCX = pmidRCX ;
	ddf -> ctx_DMA_AI_length = length ;



	/* -------------------------------------------------------------- *

	  Trigger the DMA operation 
	
	*----------------------------------------------------------------- */

	mid_ctx_dmaster (ddf);




        /********************************************************************* 
           Trace at the exit point 
         *********************************************************************/

        MID_DD_EXIT_TRACE (dsp_pinctx, 2, DSP_PINCTX, ddf, pd, 
    				pmidRCX->type, pmidRCX -> hw_rcx, 0xF0); 

	BUGLPR(dbg_dsp_pinctx, BUGNTX, ("Leaving dsp_pinctx \n"));
	return (0);

}
