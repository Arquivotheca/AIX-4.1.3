static char sccsid[] = "@(#)90  1.11.2.5  src/bos/kernext/disp/ped/intr/intr_dsp.c, peddd, bos411, 9428A410j 2/4/94 14:37:11";
/*
 * COMPONENT_NAME: PEDDD
 *
 * FUNCTIONS: intr_dsp
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
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

#include "mid_rcm_mac.h"
#include "mid_dd_trace.h"

MID_MODULE (intr_dsp);


/*************************************************************************** 
   Externals defined herein: 
 ***************************************************************************/

long intr_dsp (
		struct phys_displays *, 
		midddf_t *,  
		ulong     );


  


/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    intr_dsp 
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        SLIH, dsp status processor
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
      Process or route each of the individual pieces of dsp status.
      Essentially this routine is a big switch.  

   Notes:                                         
      Currently this routine does no I/O. 


 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
                  none
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
           This routine is called from the real SLIH (midintr) when dsp 
           status is received from the adapter. 

  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the physical device structure
            . a pointer to the device dependent physical device structure 
            . the dsp status itself 
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
                                 End of Prologue                                
 ******************************************************************************/

  




long intr_dsp ( 
		struct phys_displays 	*pd,
		midddf_t		*ddf,  
		ulong			status_parm )
{
	ulong 		dsp_status ;
	ushort 		dsp_parm ;
	ulong		ddf_function_indicator; /* function nibble of dsp_st */

	int             rc;
	int             drc;


	HWPDDFSetup;			/* gain access to the hardware */




	/*------------------------------------------------------------------*
   	   Trace the entry parms
	 *------------------------------------------------------------------*/

    	MID_DD_ENTRY_TRACE (intr_dsp, 1, INTR_DSP, ddf, pd,
				 pd, ddf, status_parm ) ;

        BUGLPR(dbg_intr_dsp, BUGNTA, 
		("Entering intr_dsp, dsp = %X\n", status_parm));



	/*------------------------------------------------------------------*
	Check for ERRORs first

	If either of the high-order 2 bits of the dsp_status value are "1",
	then there is a problem and we should return.
		- bit 32 on indicates a hardware error.
		- bit 31 on indicates a datastream error (bad opcode to adapter)
	
	NOTE:
		Do we want to add error-reporting or recovery here????
		Also, what should we return(?) here?
	 *------------------------------------------------------------------*/

	if ( ( status_parm & HARDWARE_ERROR ) ||
	     ( status_parm & DATA_STREAM_ERROR ) )
	{
		miderr( ddf,status_parm,NULL,NULL,NULL,NULL,NULL,NULL );

		/* ------------------------------------------------------
		If the user requested DMA was not completed - DSP status 4006,
		we do not want to return here.  The user process is asleep,
		waiting for DMA to complete and returning would cause the
		process to 'hang'. This condition is handled in the
		DMA_COMPLETE case of the switch statement below.
		-------------------------------------------------------- */

		if ( ( status_parm & 0xFFFF0000 ) != DMA_COMPLETE_ERROR )
		{
			MID_BREAK( INTR_DSP, 0xBADEC0DE, ddf, pd,
			status_parm, dsp_status );
			return (-1) ;
		}
	}


	/*------------------------------------------------------------------*
	  SWITCH on DSP status   
	 *------------------------------------------------------------------*/


	dsp_status = status_parm & 0xFFFF0000;
	dsp_parm =   status_parm & 0x0000FFFF;


	BUGLPR(dbg_intr_dsp, BUGACT, ("TOP of DSP status SWITCH \n" ));
	BUGLPR(dbg_intr_dsp, BUGACT,
		("dsp_status = 0x%X , parm = 0x%X \n", dsp_status, dsp_parm ));


	switch (dsp_status) 
	{ 
		/*----------------------------------------------------------*
		   0001 - ADAPTER RESET COMPLETE  
		 *----------------------------------------------------------*/
		case RESET_COMPLETE :  

			/* do nothing.  
		   	   Just reading the status resets it, so we're done.  */ 
			break  ; 


		/*----------------------------------------------------------*
		   0002 - Microcode CRC check successful
		 *----------------------------------------------------------*/
		case MICROCODE_CRC_PASSED :  

			/* do nothing.  
		   	   Just reading the status resets it, so we're done.  */ 
			break  ; 



		/*----------------------------------------------------------*
		   0003 - Contest Switch Complete  (Retired)
		 *----------------------------------------------------------*/



		/*----------------------------------------------------------*
		   0004 - Input Available 
		 *----------------------------------------------------------*/
		case INQUIRE_DATA_AVAIL :  

			/*----------------------------------------------------
			  There are servel types of input that are grouped into
 			  this same DSP status value.  We must, therefore,
 			  distinguish these various types of input.  This is
 			  done with the high-order nibble of the status para-
 			  meter, that is passed back by the adapter.
 			  
 			  Note that this parameter is simply a correlator that
 			  we supplied to the adapter when the request was 
			  placed in the FIFO.  It is, therefore, up to the 
			  device driver to supply a proper correlator with the
			  correct high-order nibble, so the following switch  
			  statement works correctly. 

			-----------------------------------------------------*/

			ddf_function_indicator = status_parm & 0x0000F000;

        		BUGLPR(dbg_intr_dsp, 2, ("function_indicator = 0x%X\n", 
						ddf_function_indicator));



			switch (ddf_function_indicator)
			{ 
				case GETCOLOR_MASK :  

					mid_intr_getcolor (ddf);
					break  ;

				case GETCPOS_MASK :  

					mid_intr_getcpos (ddf);
					break  ;

				default : 
        				BUGLPR(dbg_intr_dsp, 0, 
						("INVALID DDF GET TYPE = %X\n",
						ddf_function_indicator));

			} 


			break  ; 




		/*----------------------------------------------------------*
		   0005 - Pick Complete 
		 *----------------------------------------------------------*/
		case PICK_COMPLETE :  

			rc = dsp_pick (pd, ddf, dsp_parm) ;

			break  ; 



		/*----------------------------------------------------------*
		   0006 - DMA Complete 
		 *----------------------------------------------------------*/
		case DMA_COMPLETE :  
		case DMA_COMPLETE_ERROR:

                	BUGLPR(dbg_intr_dsp, 2, ("dsp stat: DMA_COMPLETE.\n"));

                	/*-----------------------------------
                	   Invoke callback routine to clean
			   up DMA operation.
                	-----------------------------------*/

                	if ( ( ddf->dcallback ) != NULL )
                	{ 
                		BUGLPR(dbg_intr_dsp, BUGNTA,
                        		("Invoking DMA callback function.\n"));

                		( ddf->dcallback )(ddf->pProc_dma, 0);

                		ddf->dcallback = NULL;

				if ( dsp_status == DMA_COMPLETE )
				{
                		    ddf->dma_result = MID_DMA_NOERROR;
				}
				else  /* if dsp_status was 0x40060000 */
				{
				    ddf->dma_result = MID_DMA_BAD_STRUCT;
				}

				if ( ddf->cmd == DMA_NOWAIT )
				{
					/*-----------------------------------
					  Invoke callback routine to clean
					  up DMA operation and unguard domain
					  guarded in middma.c (vttdma).
					-----------------------------------*/

					BUGLPR(dbg_intr_dsp, 2,
						("Unguard domain\n."));

					MID_UNGUARD_DOMAIN (ddf->pdev, 
					    (ddf->pProc_dma->pDomainCur[0]) ) ; 
				}

				else if ( ddf->cmd == DMA_WAIT_REQ )
				{
				/* **************************************** */
				/* Need to wake up process waiting for DMA  */
				/* compelete in middma.c (vttdma).          */
				/********************************************/

					BUGLPR(dbg_intr_dsp, 1,
						("Waking up process.\n"));

					ddf->dma_sleep_flags = 0x10;

					e_wakeup ( &( ddf->dma_sleep_addr ) );
				}

				else
				{
				    BUGLPR(dbg_intr_dsp, 0, 
						("UNEXPECTED DMA cmplt\n"));
				}

			}

			break;



		/*----------------------------------------------------------*
		   0007 - DMA Complete for Context not during Context Switch
		 *----------------------------------------------------------*/

		case CTX_SAVE_COMPLETE :  
			BUGLPR(dbg_intr_dsp, 1, ("Ctx DMA complete \n"));
			if ( (ddf->dom_flags) &
					HOST_INITIATED_CTX_DMA_IN_PROCESS )
			{
				ddf->dom_flags &=
					~HOST_INITIATED_CTX_DMA_IN_PROCESS ;

				ddf -> ctx_DMA_HI_midRCX->
				       flags.context_on_adapter = 0;

				BUGLPR(dbg_intr_dsp, 2, ("Call d_complete\n"));

				drc = d_complete (
						ddf -> ctx_DMA_HI_channel,
						ddf -> ctx_DMA_HI_flags,
						ddf -> ctx_DMA_HI_sys_addr,
						ddf -> ctx_DMA_HI_length,
						ddf -> ctx_DMA_HI_xs,
						ddf -> ctx_DMA_HI_bus_addr
						) ;


				if(drc != DMA_SUCC)
				{
					BUGLPR(dbg_intr_dsp, 0,
					("d_complete error:  %d\n", drc));

					MID_DD_TRACE_PARMS (intr_dsp, 1,
						GENERIC_PARM3, ddf, ddf,
						hkwd_DISPLAY_MIDDDF_INTR_DSP,
						drc, 0xEEEE0007 );
				}

			}


			break  ; 



		/*----------------------------------------------------------*
		   0008 - New Context Handling Complete 

		   This status informs us the adapter has prepared the 
		   new context and is ready for any late-breaking WID or
		   window geometry changes.  This status also tells us that
		   the FIFO is available (if we did not already know that). 

		   Call dsp_newctx to handle this event. 
		 *----------------------------------------------------------*/
		case NEW_CTX_COMPLETE :  

			dsp_newctx (pd, ddf) ;

			break  ; 




		/*----------------------------------------------------------*
		   0009 - Diagnostic Complete  (RETIRED)
		 *----------------------------------------------------------*/



		/*----------------------------------------------------------*
		   000A - Font Request

		   Adapter needs a font pinned (aka font fault) 
		 *----------------------------------------------------------*/
		case FONT_REQUEST :  

        		mid_font_request(pd,dsp_status);
	
			break  ; 




		/*----------------------------------------------------------*
		   000B - Pin context memory request
		   
		   Call dsp_pinctx to handle this status. 
		 *----------------------------------------------------------*/
		case PIN_CTX_MEM_REQ :  

			dsp_pinctx (pd, ddf) ; 

			break  ; 





		/*----------------------------------------------------------*
		   000C - Sync response (correlated request completion)
		   
		   This is functionally similar to INQUIRE_DATA_AVAIL.      
		   
		   A couple of the Mod 1 gets come through this interface.  
		   It is general enough to allow any function to use, however,
		   the correlators must be unique (to allow us to distinguish).
		 *----------------------------------------------------------*/
		case SYNC_RESPONSE :  

			ddf_function_indicator = status_parm & 0x0000F000;

        		BUGLPR(dbg_intr_dsp, 2, ("sync_indicator = 0x%X\n", 
						ddf_function_indicator));


			switch (ddf_function_indicator)
			{ 
				case GETCONDITION_MASK :  

					mid_intr_getcondition (ddf);
					break  ;

				case GETTEXTFONTINDEX_MASK :  

					mid_intr_gettextfontindex (ddf);
					break  ;

				default : 
        				BUGLPR(dbg_intr_dsp, 0, 
						("UNKNOWN SYNC TYPE = %X\n",
						ddf_function_indicator));

			} 
	
			break  ; 




		/*----------------------------------------------------------*
		   000D - FIFO stalled

        		If a context finishes a set of rendering commands 
        		up to its swap (buffer) point, it runs out of things
        		to do.  If there is an appreciable amount of time
        		left, we will give up the context and let somebody 
        		else run. 

		 *----------------------------------------------------------*/

		case FIFO_STALLED :  

			dsp_stall (pd, ddf) ;

			break  ; 





		/*----------------------------------------------------------*
		   000E - Buffer swap has occurred  
		 *----------------------------------------------------------*/
		case FRAME_BUF_SWAPPED :  

                	mid_intr_swapbuffers ( pd );
	
			break  ; 




		/*----------------------------------------------------------*
		   000F - Mod 1 End Rendering (command element) executed 
		 *----------------------------------------------------------*/

		case END_RENDERING_3DM1 :  

			mid_intr_endrender(ddf, dsp_parm);

			break  ; 




		/*-----------------------------------------------------------*
		   0010 - DMA Completion on a System Initiated context restore
		 *-----------------------------------------------------------*/

		case HI_CTX_DMA_COMPLETE :  

			BUGLPR(dbg_intr_dsp, 2, ("Ctx DMA complete \n"));
			if ( (ddf->dom_flags) & 
					HOST_INITIATED_CTX_DMA_IN_PROCESS )
			{ 
				ddf->dom_flags &=
					~HOST_INITIATED_CTX_DMA_IN_PROCESS ;

               			ddf->ctx_DMA_HI_midRCX->
						flags.context_on_adapter = 1;

				BUGLPR(dbg_intr_dsp, 2, ("Call d_complete\n"));
		
				drc = d_complete (
                  				ddf -> ctx_DMA_HI_channel,
                  				ddf -> ctx_DMA_HI_flags,
                  				ddf -> ctx_DMA_HI_sys_addr,
                  				ddf -> ctx_DMA_HI_length,
                  				ddf -> ctx_DMA_HI_xs,
                  				ddf -> ctx_DMA_HI_bus_addr
                 				) ;


				if(drc != DMA_SUCC)
				{
	 	 			BUGLPR(dbg_intr_dsp, 0, 
					("d_complete error:  %d\n", drc));

					MID_DD_TRACE_PARMS (intr_dsp, 0, 
						GENERIC_PARM3, ddf, ddf, 
						hkwd_DISPLAY_MIDDDF_INTR_DSP, 
						drc, 0xEEEE0001 );
				}
	
			} 
			else 
			{ 
			 	BUGLPR(dbg_intr_dsp, 0, 
					("BOGUS HOST CTX DMA complete \n"));

				MID_DD_TRACE_PARMS (intr_dsp, 0, 
						GENERIC_PARM3, ddf, ddf, 
						hkwd_DISPLAY_MIDDDF_INTR_DSP, 
						drc, 0xEEEE0002 );
			} 

			break  ; 



		/*-----------------------------------------------------------*
		   0011 - DMA Completion on a Adapter Initiated context offload
		 *-----------------------------------------------------------*/

		case AI_CTX_DMA_COMPLETE :  

			BUGLPR(dbg_intr_dsp, 2, ("Ctx DMA complete \n"));
			if ( (ddf->dom_flags) & 
					ADAPTER_INITIATED_CTX_DMA_IN_PROCESS )
			{ 
				ddf->dom_flags &= 
					~ADAPTER_INITIATED_CTX_DMA_IN_PROCESS ;

				mid_ctx_dcomplete(ddf);
			} 
			else 
			{ 
			 	BUGLPR(dbg_intr_dsp, 0, 
					("BOGUS ADAPTER CTX DMA complete \n"));

				MID_DD_TRACE_PARMS (intr_dsp, 0, 
						GENERIC_PARM3, ddf, ddf, 
						hkwd_DISPLAY_MIDDDF_INTR_DSP, 
						drc, 0xEEEE0003 );

			} 

			break  ; 






		/*-----------------------------------------------------------*
		   0012 - Error in context DMA detected by adapter 
		 *-----------------------------------------------------------*/

		case CTX_DMA_PARM_ERROR :  

			BUGLPR(dbg_intr_dsp,0,("ADAP detected Ctx DMA ERR\n"));

			MID_DD_TRACE_PARMS (intr_dsp, 0,GENERIC_PARM3,ddf,ddf, 
				hkwd_DISPLAY_MIDDDF_INTR_DSP,
				status_parm, 0xEEEE0004);

			MID_BREAK( INTR_DSP, 0xEEEE0004, ddf, pd, status_parm,
				   dsp_status );

			break  ; 






		/*----------------------------------------------------------*
		   OTHERWISE (other DSP status falls to here) 
		 *----------------------------------------------------------*/

		default : 
			BUGLPR(dbg_intr_dsp, 0,
			("DSP STATUS NOT RECOGNIZED = 0x%X \n", dsp_status));

			MID_DD_TRACE_PARMS (intr_dsp, 0,GENERIC_PARM3,ddf,ddf, 
				hkwd_DISPLAY_MIDDDF_INTR_DSP,
				dsp_status, 0xEEEE0005);

			MID_BREAK( INTR_DSP, 0xEEEE0005, ddf, pd, status_parm,
				   dsp_status );

			break  ; 

	}






        /********************************************************************* 
           Trace at the exit point 
         *********************************************************************/

        MID_DD_EXIT_TRACE (intr_dsp, 2, INTR_DSP, ddf, pd, 0, 0, 0xF0); 

	BUGLPR(dbg_intr_dsp, BUGNTX, ("Leaving intr_dsp \n"));
	return (0);

}



#include	"dsp_stall.c"
