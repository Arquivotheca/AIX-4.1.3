static char sccsid[] = "@(#)95	1.2  src/bos/kernext/disp/ped/rcm/mid_ctxdma.c, peddd, bos411, 9428A410j 3/19/93 19:07:43";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_ctx_dcomplete
 *		mid_ctx_dmaster
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
#include <sys/dma.h>

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

MID_MODULE (mid_ctxdma);


/*************************************************************************** 
   Externals : 
 ***************************************************************************/

extern long mid_ctx_dmaster ( midddf_t *ddf );

  
extern long mid_ctx_dcomplete ( midddf_t *ddf );

  


/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    mid_ctx_dmaster
  
   Descriptive Name:  Perform the d_master for adapter initiated 
		      context DMA operations
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
	This routine performs the d_master operation for context switch DMA
	operations.  It also notifies the adapter when the DMA operation is
	ready to go. 

	Note that currently the context space is always pinned, so pinning
	is not required from this routine.

 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
           This routine is called out of the interrupt handler 
	   (intr/dsp_pinctx.c) when the "Pin Context Memory" dsp status 
	   is received from the adapter.

 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the device dependent physical device structure 
          

    RETURN CODES:  none 

          
                                 End of Prologue                                
 ******************************************************************************/


long mid_ctx_dmaster ( midddf_t	*ddf )
{

	mid_rcx_t 	*pmidRCX ; 	/* temp var for ctx being DMA'ed */

	HWPDDFSetup ;			/* Set up addressability to adapter */


	pmidRCX = ddf -> ctx_DMA_AI_midRCX ;

	MID_DD_ENTRY_TRACE (mid_ctxdma, 1, CTX_DMASTER, ddf, ddf,
			    pmidRCX->pRcx, pmidRCX, ddf -> ctx_DMA_AI_length );
	

        ddf -> ctx_DMA_AI_sys_addr = pmidRCX-> hw_rcx ;  /* ctx save area */
        ddf -> ctx_DMA_AI_bus_addr = ddf->ctx_DMA_adapter_init_base_bus_addr +
                                (((ulong) (ddf-> ctx_DMA_AI_sys_addr) & 0xFFF));
        ddf -> ctx_DMA_AI_xs = &(pmidRCX-> xs) ;  		/* xmem desc  */


#define  dbg_midctx 	dbg_mid_ctxdma
        BUGLPR(dbg_midctx, 3,("AI_chan_id = 0x%8X\n",ddf->ctx_DMA_AI_channel));
        BUGLPR(dbg_midctx, 3,("AI_dma_flags = 0x%8X\n",ddf->ctx_DMA_AI_flags));
        BUGLPR(dbg_midctx,3,("AI_data_addr = %8X\n",ddf->ctx_DMA_AI_sys_addr));
        BUGLPR(dbg_midctx, 3,("AI_size    = 0x%8X\n",ddf->ctx_DMA_AI_length ));
        BUGLPR(dbg_midctx, 3,("AI_cmd_addr = %8X\n",ddf->ctx_DMA_AI_bus_addr));
        BUGLPR(dbg_midctx, 3, ("AI_descriptor = 0x%8X\n",ddf->ctx_DMA_AI_xs ));
        BUGLPR(dbg_midctx, 3,
		 	("AI_xs.aspace   = 0x%8X \n", pmidRCX-> xs.aspace_id));

        BUGLPR(dbg_midctx, BUGNTA, ("Calling d_master (\n"));



        d_master (
                  	ddf -> ctx_DMA_AI_channel,
                  	ddf -> ctx_DMA_AI_flags,
                  	ddf -> ctx_DMA_AI_sys_addr,
                  	ddf -> ctx_DMA_AI_length,
                  	ddf -> ctx_DMA_AI_xs,
                  	ddf -> ctx_DMA_AI_bus_addr
                 ) ;


        /*----------------------------------------------------------------
           Set the flag to indicate that the host initiated
           DMA of a context.
        ------------------------------------------------------------------*/

	ddf -> dom_flags |= ADAPTER_INITIATED_CTX_DMA_IN_PROCESS;


	/*------------------------------------------------------------------*
	  Now notify the adapter that the DMA operation is ready

	    Note that we always use the same correlator for context DMA 
	    operations, (since there can be only one context DMA operation 
	    in progress, the correlator is not really needed). 

	 *------------------------------------------------------------------*/

	MID_ContextMemoryPinned ( 0xFFFB, 		   /* correlator */
				ddf-> ctx_DMA_AI_bus_addr ,/* bus address */
				ddf-> ctx_DMA_AI_length ,  /* length */
				pmidRCX 		   /* context ID */
				) ;		
        	
        	
        BUGLPR(dbg_mid_ctxdma, 2, ("Adapter notified \n"));





        BUGLPR(dbg_mid_ctxdma, 1, ("Leaving ctx_dma_master \n"));

	MID_DD_EXIT_TRACE (mid_ctxdma, 1, CTX_DMASTER, ddf, ddf,
				pmidRCX-> pRcx, pmidRCX, 0xF0 );
	
	
}







/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    mid_ctx_dcomplete
  
   Descriptive Name:  Perform the d_complete operation for context DMA
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
	This routine does the cleanup for dapter initiated context DMA 
	operations.  It is called out of the interrupt handler when the 
	DMA completion is received. 

	This routine performs no I/O.

 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
	   mid_ctx_dcomplete is called out of the interrupt handler 
	   (intr/intr_dsp.c) to do d_complete when the context DMA complete	 	   dsp status is received from the adapter.
  

 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the device dependent physical device structure 
          
          
    RETURN CODES:  none 
          
          
                                 End of Prologue                                
 ******************************************************************************/


long mid_ctx_dcomplete ( midddf_t	*ddf )
{


	int	drc ;			/* return code variable */

	mid_rcx_t 	*pmidRCX ; 	/* temp var for ctx being DMA'ed */



	pmidRCX = ddf -> ctx_DMA_AI_midRCX ;

	MID_DD_ENTRY_TRACE (mid_ctxdma, 1, CTX_DCOMPLETE, ddf, ddf,
	         pmidRCX-> pRcx, pmidRCX, (*(ulong*)(&(pmidRCX-> flags))) );
	
	
	pmidRCX-> flags.context_on_adapter = 0; 
	
	BUGLPR (dbg_mid_ctxdma, 1, ("midRCX flags = 0x%X \n", 
				(*(ulong *)(&(pmidRCX-> flags))) ));



        BUGLPR(dbg_midctx, 3,("AI_chan_id = 0x%8X\n",ddf->ctx_DMA_AI_channel));
        BUGLPR(dbg_midctx, 3,("AI_dma_flags = 0x%8X\n",ddf->ctx_DMA_AI_flags));
        BUGLPR(dbg_midctx, 3,("AI_data_add = %8X\n",ddf->ctx_DMA_AI_sys_addr));
        BUGLPR(dbg_midctx, 3,("AI_size     = 0x%8X\n",ddf->ctx_DMA_AI_length));
        BUGLPR(dbg_midctx, 3,("AI_cmd_addr = %8X\n",ddf->ctx_DMA_AI_bus_addr));
        BUGLPR(dbg_midctx, 3,("AI_descriptor = 0x%8X\n",ddf->ctx_DMA_AI_xs ));
        BUGLPR(dbg_midctx, 3,
		 	("AI_xs.aspace   = 0x%8X \n", pmidRCX-> xs.aspace_id));


	BUGLPR(dbg_mid_ctxdma, 1, ("Calling d_complete()\n"));
		
	drc = d_complete (
                  	ddf -> ctx_DMA_AI_channel,
                  	ddf -> ctx_DMA_AI_flags,
                  	ddf -> ctx_DMA_AI_sys_addr,
                  	ddf -> ctx_DMA_AI_length,
                  	ddf -> ctx_DMA_AI_xs,
                  	ddf -> ctx_DMA_AI_bus_addr
                 	) ;


	if(drc != DMA_SUCC)
	{
	 	 BUGLPR(dbg_mid_ctxdma, 0, ("d_complete error:  %d\n", drc));
		 drc = -1;
	}



	BUGLPR(dbg_mid_ctxdma, 1, ("leaving ctx d_complete rc: %d \n", drc));

	MID_DD_EXIT_TRACE (mid_ctxdma, 1, CTX_DCOMPLETE, ddf, ddf,
	           	drc, pmidRCX, (*(ulong*)(&(pmidRCX-> flags))) );
	
	return(drc);
}
