static char sccsid[] = "@(#)87  1.8.1.8  src/bos/kernext/disp/ped/intr/dsp_newctx.c, peddd, bos411, 9428A410j 4/8/94 16:12:47";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: dsp_newctx
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
#include <sys/dma.h>
#include <sys/syspest.h>
#include <sys/display.h>

#include "hw_dd_model.h"
#include "midhwa.h"
#include "hw_dsp.h"
#include "hw_regs_k.h"
#include "hw_regs_u.h"
#include "hw_typdefs.h"
#include "hw_macros.h"
#include "hw_ind_mac.h"
#include "hw_PCBkern.h"

#include "midddf.h"
#include "mid_rcm_mac.h"
#include "mid_dd_trace.h"

MID_MODULE (dsp_newctx);


/*************************************************************************** 
   Externals defined herein: 
 ***************************************************************************/

long dsp_newctx (
		struct phys_displays *, 
		midddf_t * );


  

/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    dsp_newctx 
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        SLIH, processor for "New Context Processing Completion"
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
      This routine handles "New Context Processing Complete" DSP status
      from the adapter.  This includes the following tasks: 
        . sending new Window Parameters as required, 
        . writing the WID planes (as required) 

   Notes:                                         
      Currently this routine does not wakeup the heavy switch routine.
      This is done as a part of FIFO available processing. 


 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
       1. This routine performs I/O (sometimes).  It is assumed that bus
           access is provided by the caller. 
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
           This routine is called from the dsp status processor (intr_dsp)
	   when the "New Context Processing Complete" dsp status is received 
	   from the adapter. 

  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the physical device structure
            . a pointer to the device dependent physical device structure 
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
                                 End of Prologue                                
 ******************************************************************************/



long dsp_newctx ( 
		struct phys_displays 	*pd,
		midddf_t		*ddf  )
{

	rcx        *pRCX ;
	rcx        *pPrevRCX ;
	mid_rcx_t  *pmidRCX, *pPrevmidRCX, *midRCX_wakeup ;

	gRegion	    region ;

	rcmWG      *pWG ;
	midWG_t    *pmidWG ;
	midWG_t    *pDelayed ;
	midWG_t    *temp ;

	mid_wid_status_t       *WID_entry ; 

	ulong       fifo_size, i  ; 
	ulong       bim_status ; 
	int	    mask ;
	ulong       context_flags  ; 
	ushort      WID_fb ; 

	ushort      dom_flags ; 


	HWPDDFSetup;			/* gain access to the hardware */





        BUGLPR(dbg_dsp_newctx, 1, ("Entering dsp_newctx, \n" ));



	/*------------------------------------------------------------------*
	  Init some variables:  

	    . Current (new) context pointer 
	    . midRCX pointer for the same context 
	    . the domain flags 
	    . Current WG (and midWG) pointer for that context 

	 *------------------------------------------------------------------*/

	pmidRCX = ddf -> current_context_midRCX ; 
	pRCX = pmidRCX -> pRcx ; 
    	dom_flags =  ddf -> dom_flags ; 

        pWG = pRCX -> pWG ;


	/*------------------------------------------------------------------*
   	   Trace the entry parms
	 *------------------------------------------------------------------*/
    	MID_DD_ENTRY_TRACE (dsp_newctx, 1, DSP_NEWCTX, ddf, pd, 
				pRCX, dom_flags, pWG ) ;


        BUGLPR(dbg_dsp_newctx, 2, ("Entry parms: ddf = 0x%X \n",ddf ));
        BUGLPR(dbg_dsp_newctx, 2, ("             dom_flags = %X\n",dom_flags));
        BUGLPR(dbg_dsp_newctx, 2, ("             RCX       = 0x%X\n",pRCX));


	/*------------------------------------------------------------------*
	  Check to ensure that this is NOT an extraneous status code
	   from the adapter. 
	 *------------------------------------------------------------------*/

    	if ( !(dom_flags & MID_CONTEXT_SWITCH_IN_PROCESS) ) 
    	{
        	BUGLPR(dbg_dsp_newctx, 0, ("CTX switch NOT IN PROGRESS\n\n"));
        	return (-1) ;
    	}


        BUGLPR(dbg_dsp_newctx, 2, ("CTX switch IS progress\n"));



        /*******************************************************************

          Check if the preceding context needs to be re-enqueued

          If the fifo size of the last context was non-zero or driver
          is waiting for the low water mark interrrupt , then it needs
          to be re-NQed. 

          Also call mid_check_water_level to check if the high_water/low_water
	  state has changed.  

        *******************************************************************/

        BUGLPR(dbg_dsp_newctx, 2, ("CTX switch IS progress\n"));

	pPrevRCX = ddf-> previous_context_RCX ;  

        MID_RD_ASCB_VALUE (3, fifo_size) ; 
        BUGLPR(dbg_dsp_newctx, 1, ("FIFO size is 0x%X \n", fifo_size));


        if ( pPrevRCX != NULL ) 
        { 
	    pPrevmidRCX = ((mid_rcx_t *)(pPrevRCX->pData)) ; 

       	    BUGLPR(dbg_dsp_newctx, 3, (
		"Previous context = 0x%8X ,  type =%X,   PID = %6X \n",
		pPrevRCX, ((mid_rcx_t *)(pPrevRCX->pData))-> type,
		pPrevRCX-> pProc-> procHead.pid ));

            /*----------------------------------------------------------------*
              Check for any late-breaking changes in the high water /      
	      low water status. 
             *----------------------------------------------------------------*/

	    mid_check_water_level (ddf, fifo_size) ; 


            /*----------------------------------------------------------------*
              Check if the previous context needs to be re-enqueued.  This is
	      This is equivalent to checking if there is anything in the FIFO.
             *----------------------------------------------------------------*/

            if ( (fifo_size != 0 ) ||
	         (pPrevmidRCX-> flags.waiting_for_FIFO_low_water_mark) )
            { 
       		BUGLPR(dbg_dsp_newctx,1, ("Re-NQ'ing ctx: %8X !!\n",pPrevRCX));

        	MID_DD_TRACE_PARMS (dsp_newctx, 4, START_SWITCH_RENQ, ddf, pd,
			pPrevRCX, pPrevmidRCX-> type, fifo_size ) ;

		(*pPrevRCX-> pDomain-> pDev-> devHead.pCom-> rcm_callback-> 
			put_on_fault_list)  (pPrevRCX) ; 
	    }  

            else  /* FIFO was drained */
	    {  
		BUGLPR(dbg_dsp_newctx, 1, 
			("FIFO DRAINED !!   fifo size = 0x%X \n", fifo_size));

		MID_DD_TRACE_PARMS (dsp_newctx, 4, START_SWITCH_NO_RENQ, ddf,
			pd, pPrevRCX, pPrevmidRCX-> type, fifo_size ) ;
            } 
        } 
        else  /* NO previous context */
	{  
	    BUGLPR(dbg_dsp_newctx, 1, ("NO Previous context !! \n"));
	}  




        /*******************************************************************

          Perform the context updates as required 

        *******************************************************************/



      	/*-------------------------------------------------------* 
      	   The default context has no WG, so we have this special  
      	   code here. 
      	 *-------------------------------------------------------*/

        if ( ((pmidRCX-> flags.default_context) == 1) || (pWG == NULL))
	{ 
        	BUGLPR(dbg_dsp_newctx, 1, ("Default context \n"));

      		/*---------------------------------------------------* 
      	   	  Update the window parameters for the default window.
      	 	 *---------------------------------------------------*/

       		region.numBoxes = 1 ; 
       		region.pBox = &(region.extents) ;   

       		region.extents.ul.x = 0 ;
       		region.extents.ul.y = 0 ;
       		region.extents.lr.x = 1280 ;
       		region.extents.lr.y = 1024 ;


       		write_window_parms_PCB (ddf, pmidRCX,
                       			0, 
                       			(&region), 
                       			(&(region.extents.ul)), 
                       			1280, 1024) ; 


		if ( !(dom_flags & MID_CONTEXT_DELETION_IN_PROCESS) ) 
		{ 
      			/*---------------------------------------------------* 
      		 	  Write the WID planes (for the default context). 
      		 	 *---------------------------------------------------*/

			write_WID_planes_PCB (	ddf,
						0, 
                               			(&region), 
       						MID_UPPER_LEFT_ORIGIN) ; 
		} 
           }




	    else 	/* not the default context */
	    { 
        	    BUGLPR(dbg_dsp_newctx, 1, ("Regular (non-def) ctx\n"));

      			/*---------------------------------------------------* 
      		 	  Write the WID planes for all necessary windows.
      		 	  This is done by satisfying all the delayed WID writes.
      			 *---------------------------------------------------*/
			pDelayed = ddf-> dom_delayed_WID_writes_top ; 

        		BUGLPR(dbg_dsp_newctx, 1, ("Top of delayed WID loop," 
					" first WG = 0x%8X \n", pDelayed )) ;

			while (pDelayed != NULL)
			{
			    if ((pDelayed->wgflags) & MID_WID_WRITE_DELAYED)
			    { 
				pDelayed-> wgflags &= ~(MID_WID_WRITE_DELAYED);

        		        BUGLPR(dbg_dsp_newctx, 1, ("     delayed WID "
			          "write for WG = 0x%8X \n", pDelayed )) ;

				write_WID_planes_PCB (	ddf,
						pDelayed -> wid,
						pDelayed -> pWG -> wg.pClip, 
       						MID_UPPER_LEFT_ORIGIN) ; 

				temp = pDelayed -> pNext_ctx_sw_delay ;
				pDelayed -> pNext_ctx_sw_delay = NULL ;

				pDelayed = temp ;
			    } 
			    else 
			    { 
        		        BUGLPR(dbg_dsp_newctx, 0, ("DELAYED WID FLAG "
			          "NOT SET, WG = 0x%8X \n", pDelayed )) ;

        		        BUGLPR(dbg_dsp_newctx,0,("TERM LOOP EARLY\n"));

        		        MID_ASSERT(0, DSP_NEWCTX, ddf, 
					ddf-> dom_delayed_WID_writes_top, 
						pDelayed, 0, 0) ;
        		        break ;
			    } 
			}

			ddf-> dom_delayed_WID_writes_top = NULL ;
			ddf-> dom_delayed_WID_writes_bot = NULL ;


#if 0
            	    	if (pmidRCX-> type != RCX_2D) 
			{ 
				pmidWG = ((midWG_t *)(pWG -> pPriv)) ;

				fix_color_palette_PCB (ddf, pmidWG->wid) ;
			} 

#endif 


      			/*---------------------------------------------------* 
      		 	  Perform any delayed Assoc WID with color palette I/O.
      			 *---------------------------------------------------*/
        		BUGLPR(dbg_dsp_newctx, 1, 
				("Top of delayed Assoc loop,  count = %X \n", 
				ddf-> dom_delayed_Assoc_updates_count )) ;

        		MID_DD_TRACE_PARMS (dsp_newctx, 3, DSP_NEWCTX, ddf, 0,
                        	ddf->dom_delayed_Assoc_updates_count,
                        	ddf->dom_delayed_Assoc_updates[0],
                        	ddf->dom_delayed_Assoc_updates[1] );


			for (	i=0 ;
				i < ddf-> dom_delayed_Assoc_updates_count ;
				i += 1 ) 
			{
				WID_entry = 
				    &(ddf-> mid_wid_data.mid_wid_entry
					[ ddf-> dom_delayed_Assoc_updates[i] ]);

				WID_entry-> WID_flags &= 
					~(WID_DELAYED_ASSOC_UPDATE) ;

        		        BUGLPR(dbg_dsp_newctx, 1, 
					("     delayed Assoc for WID = %X \n",
					 	WID_entry-> mid_wid )) ;

				fix_color_palette_PCB (ddf,WID_entry->mid_wid);

        		    	MID_DD_TRACE_PARMS (dsp_newctx, 3, DSP_NEWCTX, 
					ddf, WID_entry-> WID_flags,
                        		ddf->dom_delayed_Assoc_updates_count,
                        		ddf->dom_delayed_Assoc_updates[0],
                        		ddf->dom_delayed_Assoc_updates[1] );
			} 

			ddf-> dom_delayed_Assoc_updates_count = 0 ;



              	/*----------------------------------------------------------*
               	  Update the client clip (if necessary)
              	     This must be processed right here as it must be:
                     . after the WID planes are written so we get 
                         proper clipping to the window and
                     . before the geometry_changed flag is reset in the
                         write_window_parms subroutine
              	 *----------------------------------------------------------*/

              	if (pmidRCX->flags.geometry_changed == 1)
                {
                  mid_set_client_clip (ddf->pdev, pRCX, 
                                       pRCX->pWA->wa.pRegion, pWG, 1) ;
                }



      		/*---------------------------------------------------* 
      	 	  Now update the window parms for the current window.
      		 *---------------------------------------------------*/
		if ( (dom_flags & CONTEXT_UPDATES_REQUIRED) ) 
	    	{ 
    		    	ddf -> dom_flags &= ~(CONTEXT_UPDATES_REQUIRED) ;       
        	    	BUGLPR(dbg_dsp_newctx, 2, ("CTX updates required \n"));

            	    	if (pmidRCX-> type != RCX_2D) 
			{ 
				pmidWG = ((midWG_t *)(pWG -> pPriv)) ;

           			write_window_parms_PCB (ddf, pmidRCX, 
						 pmidWG -> wid,
                               			 pWG-> wg.pClip, 
                                		&(pWG-> wg.winOrg), 
					 	 pWG-> wg.width, 
						 pWG-> wg.height ) ; 
			} 
	    	} 
    	    }



        /*******************************************************************
          Check if there is a HOST initiated context switch pending.
        *******************************************************************/

        if (ddf->dom_flags & HOST_INITIATED_CTX_DMA_IN_PROCESS) 
	{
		int	drc ;

        	MID_BREAK (DSP_NEWCTX, ddf, ddf->dom_flags, 0, 0, 0) ;

        	ddf->dom_flags &= ~HOST_INITIATED_CTX_DMA_IN_PROCESS ;

  		ddf->ctx_DMA_HI_midRCX->flags.context_on_adapter = 1;

                BUGLPR(dbg_dsp_newctx, 2, ("Call d_complete\n"));

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
                        BUGLPR(dbg_dsp_newctx, 0, 
				("d_complete error:  %d\n", drc));

                        MID_DD_TRACE_PARMS (dsp_newctx, 1,
                                                GENERIC_PARM3, ddf, ddf,
                                                hkwd_DISPLAY_MIDDDF_DSP_NEWCTX,
                                                drc, 0xEEEE0007 );
                }
	}




        /*******************************************************************
          Check if there is an adapter initiated context switch pending.
        *******************************************************************/

        if (ddf->dom_flags & ADAPTER_INITIATED_CTX_DMA_IN_PROCESS) 
	{
        	MID_BREAK (DSP_NEWCTX, ddf, ddf->dom_flags, 0, 0, 0) ;

        	ddf->dom_flags &= ~ADAPTER_INITIATED_CTX_DMA_IN_PROCESS ;

        	mid_ctx_dcomplete (ddf) ; 
	}


       pmidRCX->flags.context_on_adapter = 1;

#if 0
       if (pmidRCX-> type != RCX_2D) 
	{ 
        	MID_BREAK (DSP_NEWCTX, ddf, pWG, pmidWG, 
				pmidWG->pi.PI, ddf->mid_wid_data.
				mid_wid_entry[pmidWG->wid].pi.PI) ;
	} 

#endif 

        /***********************************************************
          Notify adapter that the context state updates are finished.
        ***********************************************************/
        BUGLPR(dbg_dsp_newctx,1, ("Issue Ctx State update complete\n"));

	MID_ContextStateUpdateComplete () ;



        /*******************************************************************
          Check if there is anybody just waiting to get their grubby 
          paws on the WID list.  If so, wake 'em up.
        *******************************************************************/

        midRCX_wakeup = ddf->WID_ctx_sw_sleep_top ;
        while  (midRCX_wakeup != NULL) 
        { 
		midRCX_wakeup->flags.waiting_for_WID = 0 ;

                MID_DD_TRACE_PARMS (dsp_newctx, 1,  WID_CTX_WAKEUP, ddf, ddf,
				midRCX_wakeup, midRCX_wakeup->pNext_sleep,
				*(ulong *)&(midRCX_wakeup->flags) );
		if ( ddf->num_graphics_processes )
		     e_wakeup ( &(midRCX_wakeup-> context_sleep_event_word) ) ;

		 midRCX_wakeup = midRCX_wakeup->pNext_sleep ;
        } 

        ddf->WID_ctx_sw_sleep_top = NULL ;




        /*******************************************************************

          The wakeup processing for the heavy switch routine is performed
          by the FIFO available routine. 

        *******************************************************************/


	intr_fifo_av (pd, ddf) ;




        /********************************************************************* 
           Trace at the exit point 
         *********************************************************************/

        MID_DD_EXIT_TRACE (dsp_newctx, 3, DSP_NEWCTX, ddf, pd, 
    				pmidRCX, ddf -> dom_flags, 0xF0); 

	BUGLPR(dbg_dsp_newctx, 2, ("dom flags = 0x%4X \n", ddf -> dom_flags));
	BUGLPR(dbg_dsp_newctx, 1, ("Leaving dsp_newctx \n"));
	return (0);

}
