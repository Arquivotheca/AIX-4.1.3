static char sccsid[] = "@(#)88  1.8  src/bos/kernext/disp/ped/intr/dsp_pick.c, peddd, bos411, 9428A410j 2/4/94 14:37:07";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: dsp_pick
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
#include <sys/uio.h>
#include <sys/dma.h>
#include <sys/syspest.h>
#include <sys/display.h>

#include "hw_dd_model.h"
#include "midhwa.h"
#include "hw_regs_k.h"
#include "hw_macros.h"
#include "hw_ind_mac.h"			/* for Status Control Block access   */
#include "hw_regs_u.h" 

#include "midddf.h"
#include "mid_dd_trace.h"

MID_MODULE (dsp_pick);
#define dbg_midddi  dbg_dsp_pick


/*************************************************************************** 
   Externals defined herein: 
 ***************************************************************************/

long dsp_pick (
		struct phys_displays *, 
		midddf_t *,  
		ushort     );


  

/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    dsp_pick
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        SLIH, dsp status processor for PICK EVENTS
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
      Process pick completion DSP status 


 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
                  none
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
           This routine is called from the real SLIH (midintr) when 
	   pick complete dsp status is received from the adapter. 

  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the physical device structure
            . a pointer to the device dependent physical device structure 
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
                                 End of Prologue                                
 ******************************************************************************/

  



long dsp_pick ( 
		struct phys_displays 	*pd,
		midddf_t		*ddf,  
		ushort 			correlator  ) 
{
	pick_event_t    *pickPtr;
	ulong           pickHits;       /* holds status block pick #hits */
	ulong           pickLength;     /* holds status block pick length */

	ulong  		host_status;
	int             rc;


	HWPDDFSetup;			/* gain access to the hardware */




	/*------------------------------------------------------------------*
   	   Trace the entry parms
	 *------------------------------------------------------------------*/

    	MID_DD_ENTRY_TRACE (dsp_pick, 1, DSP_PICK, ddf, pd,
				 pd, ddf, correlator ) ;

        BUGLPR(dbg_dsp_pick, BUGNTA, 
		("Entering dsp_pick, corr = %X\n", correlator));



	/*------------------------------------------------------------------*
	   get pointer to the pick event block (there is only one because
           we serialize the pick event now in the driver) 
	 *------------------------------------------------------------------*/

	pickPtr = &(ddf->pickDataBlock) ;


	/*------------------------------------------------------------------*
	   Now set up the common data used by all the flavors of pick 
	    . the event mask 
	    . the time       
	 *------------------------------------------------------------------*/

	pickPtr->report.event = pickPtr->event_mask;
	pickPtr->report.time = time;



	/*------------------------------------------------------------------*
	   Now switch on the several cases 
	 *------------------------------------------------------------------*/

	switch ( pickPtr->event_mask )
	{

	/**********************************************************************
	   MOD 1 BEGIN PICK 
	 *********************************************************************/

	case BEGINPICKM1:       		/* Pick first for M1.  */

		MID_RD_M1SB_VALUE( MID_M1SB_PICK_CORRELATOR_PICK_HITS, 
					pickHits );
		MID_RD_M1SB_VALUE( MID_M1SB_PICK_DATA_LENGTH, pickLength );

		if ( ( (pickHits & 0xFFFF0000)>>16 ) == correlator )
		{
			pickPtr->report.data[0] = pickHits & 0x0000FFFF;
			pickPtr->report.data[1] = pickLength;
		}
		else 
		{
			BUGLPR(dbg_midddi, 0, ("Status block query failed.\n"));
		}
		break;




	/**********************************************************************
	   MOD 1 END PICK 
	 *********************************************************************/

	case ENDPICKM1:

		BUGLPR(dbg_midddi, BUGNTA, ("End pick M1.\n"));

		MID_RD_M1SB_VALUE( MID_M1SB_PICK_CORRELATOR_PICK_HITS, 
					pickHits );
		MID_RD_M1SB_VALUE( MID_M1SB_PICK_DATA_LENGTH, pickLength );

		if ( ( (pickHits & 0xFFFF0000)>>16 ) == correlator )
		{
			pickPtr->report.data[0] = pickHits & 0x0000FFFF;
			pickPtr->report.data[1] = pickLength;
		}
		else 
		{
			BUGLPR(dbg_midddi, 0,
				("Status block query failed (0x%x vs 0x%x).\n",
				pickHits, correlator));
		}

		BUGLPR(dbg_midddi, BUGNTA, ("pick_#=0x%x, pickLen=0x%x.\n",
				pickPtr->report.data[0], pickLength));

		pickPtr->report.data[2] = 0;
		pickPtr->report.data[3] = 0;

		/******************************************************* */
		/* The code to clean up after DMA - d_complete - and    */
		/* the unpin stuff has been moved to midevent.c          */
		/* ***************************************************** */

		/*----------------------------------------------------------*
		   Finally, notify the application of completion
		   This may be done via either via a wake_up (synchronous),
		   or a signal (asynchronous nofication).
		 *----------------------------------------------------------*/

		if ( pickPtr->eventcmd & SYNC_WAIT )
		{
			BUGLPR(dbg_midddi, BUGNTA, ("Waking up process \n"));

			/* reset sleep flag so that we can get out of
                           e_sleep in the End Pick case of midevent.c
                        */ 
			pickPtr->sleep_flag = 0;

			e_wakeup ( &( pickPtr->sleep_addr ) );
		}
		else
		{
			BUGLPR(dbg_midddi, BUGNTA,
				("Call callback to signal process \n"));

			if ( pickPtr->callback )
				( pickPtr->callback )( pd->cur_rcm,
						&( pickPtr->report ) );
		}

		break;


	/**********************************************************************
	   MOD 2 END PICK 
	 *********************************************************************/

	case ENDPICKM1M:

		MID_RD_M1MSB_VALUE( MID_M1MSB_PICK_CORRELATOR_PICK_HITS, 
					pickHits );
		MID_RD_M1MSB_VALUE( MID_M1MSB_PICK_DATA_LENGTH, pickLength );

		if ( ( (pickHits & 0xFFFF0000)>>16 ) == correlator )
		{
			pickPtr->report.data[0] = pickHits & 0x0000FFFF;
			pickPtr->report.data[1] = pickLength;
		}
		else 
		{
			BUGLPR(dbg_midddi, 0, ("Status block query failed.\n"));
		}


		pickPtr->report.data[2] = 0;
		pickPtr->report.data[3] = 0;


		/*----------------------------------------------------------*
		   Now clean up the DMA operation  
		   Now done in midevent.c  
		 *----------------------------------------------------------*/

		/*----------------------------------------------------------*
		   Finally, notify the application of completion
		   This may be done via either via a wake_up (synchronous),
		   or a signal (asynchronous nofication).
		 *----------------------------------------------------------*/

		if ( pickPtr->eventcmd & SYNC_WAIT )
		{
			BUGLPR(dbg_midddi, BUGNTA, ("Waking up process \n"));

			/* reset sleep flag so that we can get out of
                           e_sleep in the End Pick case of midevent.c
                        */ 
			pickPtr->sleep_flag = 0;

			e_wakeup ( &( pickPtr->sleep_addr ) );
		}
		else
		{
			BUGLPR(dbg_midddi, BUGNTA,
				("Call callback to signal process \n"));

			if ( pickPtr->callback )
				( pickPtr->callback )( pd->cur_rcm,
						&( pickPtr->report ) );
		}

		break;



	default:

		/* log error indicating upsupported mode        */

		BUGLPR( dbg_midddi, 0, ( "invalid command specified %x \n",
						pickPtr->event_mask ) );
		rc = -1;
	}






        /********************************************************************* 
           Trace at the exit point 
         *********************************************************************/

        MID_DD_EXIT_TRACE (dsp_pick, 2, DSP_PICK, ddf, pd, 
				pickHits, pickLength, 0xF0); 

	BUGLPR (dbg_dsp_pick, BUGNFO, ("Bottom of dsp_pick \n\n") ); 

}
