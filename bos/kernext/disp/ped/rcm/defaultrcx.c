static char sccsid[] = "@(#)43	1.3.1.9  src/bos/kernext/disp/ped/rcm/defaultrcx.c, peddd, bos411, 9428A410j 3/31/94 21:36:09";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_create_default_context
 *		mid_load_default_context
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


/***************************************************************************** 
       Includes:  
 *****************************************************************************/

#include <sys/syspest.h>
#include <sys/types.h>
#include <sys/xmem.h>
#include <sys/rcm_win.h>
#include <sys/timer.h>    /* used for TIMER.  temp for debugging */
#include <sys/sleep.h>    
#include <sys/aixgsc.h>
#include <sys/rcm.h>
#include <sys/intr.h>
#include <sys/display.h>	/* added with font (update of curr process) */
#include <sys/sleep.h>	/* added with font (update of curr process) */
#include "mid_pos.h"		/* must precede midhwa.h */
#include "midhwa.h"	

#include "hw_dd_model.h"
#include "hw_macros.h"
#include "hw_typdefs.h"
#include "hw_regs_k.h"
#include "hw_regs_u.h" 
#include "hw_seops.h"   /* CONTEXT_STATE_UPDATE_COMPLETE defined diff places */
                        /* It is also defined in hw_PCBkern.h and hw_PCBrms.h*/
#include "midctx.h"  /* contains definition of DSP status codes */

#include "hw_ind_mac.h"
#include "hw_HCRkern_dd.h" 

#include "midddf.h"
#include "midrcx.h" 
#include "midRC.h"
#include "midwidmac.h"

#include "mid_dd_trace.h"	/* includes disptrc which defines dup symbols */
                         	/* POLYPOINT, POLYLINES, POLYSEGMENT */

MID_MODULE (midswitch);



/***************************************************************************** 
       Externals: 
 *****************************************************************************/

extern mid_create_default_context (midddf_t *) ;

extern mid_load_default_context (midddf_t *) ;



#define dbg_middd dbg_midswitch
#define dbg_midpixi dbg_midswitch
#define dbg_SetContext dbg_midswitch
#define dbg_syncwid dbg_midswitch



#include <sys/pin.h>
#include <sys/malloc.h>


/****************************************************************************** 
                              
                             Start of Prologue  
  
   Function Name:    mid_create_default_context  
  
   Descriptive Name:  Create Default (2D) Context Data Structures
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
  
     This routine creates the default 2D context data structures.  Processing 
     for this includes: 
      . creating the necessary data structures: 
          . a dummy device independent rcx data structure 
          . a device dependent rcx data structure (which does get used)
  
  
 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
                  none
  
  
 *--------------------------------------------------------------------------* 
  
    Linkage:
           This function is called internally from within the device driver. 

 *--------------------------------------------------------------------------* 
  
    INPUT:  These are the input parameters to the create_default_context
             function:
              . pointer to the device dependent data structure
          
          
    OUTPUT:  none 
          
    RETURN CODES:  none         
          
          
                                 End of Prologue                                
 ******************************************************************************/




#define  dbg_middfltctx  dbg_midswitch

long mid_create_default_context ( midddf_t   *ddf ) 
{

  rcx                  *pRCX ;
  mid_rcx_t            *pmidRCX ;
  mid_rcx_type_t        ctx_type ;
  mid_create_rcx_t     *pPARM ;  



  MID_DD_ENTRY_TRACE (midswitch, 1, DEFAULT_CONTEXT, ddf, ddf, ddf, 0, 0 ) ;

  BUGLPR(dbg_middfltctx, 1, ("\n\n\n\n"));
  BUGLPR(dbg_middfltctx, 1, ("Top of create_default_context\n"));
  BUGLPR(dbg_middfltctx, 2, ("ddf = 0x%x \n",ddf ) );



  /***************************************************************** 

     ALLOCATE SPACE for the data structures 

     Space is allocated here for the device independent context structure 
     as well as the device dependent context structure. 
     
   *****************************************************************/

  
  /*---------------------------------------------------------------* 
     ALLOCATE SPACE for the device independent RCX
   *---------------------------------------------------------------*/

  pRCX = xmalloc ( sizeof (rcx), 4, pinned_heap) ; 

  BUGLPR(dbg_middfltctx, 1, ("pRCX = 0x%x  size = 0x%x \n",pRCX, sizeof(rcx)) );

  if ( pRCX == NULL )
  {
	BUGLPR(dbg_middfltctx, 0, ("** ERROR ** RCX alloc failed\n\n"));
        BUGLPR(dbg_middfltctx, 1, ("Leaving mid_create_default_ctx\n"));
        return (MID_ERROR) ;
  }




  /*---------------------------------------------------------------* 
     ALLOCATE SPACE for the device dependent RCX
   *---------------------------------------------------------------*/

  pmidRCX = xmalloc ( sizeof (mid_rcx_t), 4, pinned_heap) ; 

  BUGLPR (dbg_middfltctx, 1, ("pmidRCX = 0x%x  size = 0x%x \n",
				pmidRCX, sizeof (mid_rcx_t)) );

  if ( pmidRCX == NULL )
  {
  	xmfree (pRCX, pinned_heap) ; 

	BUGLPR(dbg_middfltctx, 0, ("** ERROR ** midRCX alloc failed\n\n"));
        BUGLPR(dbg_middfltctx, 1, ("Leaving mid_create_default_ctx\n"));
        return (MID_ERROR) ;
  }





  /***************************************************************** 
     Now initialize the data structure(s) just allocated.

     First fill them with zeros, then link the DI and DD context 
     structures and save the passed context type. 
   *****************************************************************/

  /*---------------------------------------------------------------* 
     Init the device independent RCX
   *---------------------------------------------------------------*/

  bzero (pRCX, sizeof (rcx) ) ; 

  pRCX -> pData = (genericPtr) pmidRCX ;


  /*---------------------------------------------------------------* 
     Init the device dependent RCX
   *---------------------------------------------------------------*/

  bzero (pmidRCX, sizeof (mid_rcx_t) ) ; 

  pmidRCX -> pRcx = pRCX ;
  pmidRCX -> type = RCX_2D ;
  pmidRCX -> size = MAX_CTX_LEN_2D ;
  pmidRCX -> xs.aspace_id = XMEM_INVAL;
  pmidRCX-> context_sleep_event_word = EVENT_NULL ;

  pmidRCX -> flags.default_context = 1 ;
  /* pmidRCX -> flags.remove_context = 1 ; */

  ddf -> default_context_RCX = pRCX ;



  BUGLPR(dbg_middfltctx, 4, ("\n") );
  BUGLPR(dbg_middfltctx, 4, 
                        ("DI RCX = 0x%x,  pData = 0x%x\n", pRCX, pRCX->pData) );
  BUGLPR(dbg_middfltctx, 4,
                        ("----------- DD RCX, @ = 0x%x --------- \n", pmidRCX));
  BUGLPR(dbg_middfltctx, 4, ("pRCX   = 0x%x \n", pmidRCX->pRcx) );
  BUGLPR(dbg_middfltctx, 4, ("type   = 0x%x \n", pmidRCX->type) );
  BUGLPR(dbg_middfltctx, 4, ("flags  = 0x%x \n", pmidRCX->flags  ) );
  BUGLPR(dbg_middfltctx, 4, ("hw_rcx = 0x%x \n", pmidRCX->hw_rcx ) );
  BUGLPR(dbg_middfltctx, 4, ("xmem   = 0x%x \n", pmidRCX->xs     ) );




  MID_DD_EXIT_TRACE (midswitch, 1, DEF_CTX_ALLOC, ddf, ddf, 
				pRCX, pmidRCX, *(ulong *)(&(pmidRCX->flags)) );


  BUGLPR(dbg_middfltctx, 1, ("EXIT 2D: Leaving create_def_ctx\n\n"));


  return(MID_LIGHT_SWITCH) ;

}	 /*  end of create default context routine */ 




/****************************************************************************** 
                              
                             Start of Prologue  
  
   Function Name:    mid_load_default_context  
  
   Descriptive Name:  Load Default (2D) Context on Adapter
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
  
     This routine put the default 2D context onto the adpater.  Processing 
     for this includes: 
      . invoking the necessary I/O to put the context on the adapter. 
  
  
 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
                  none
  
  
 *--------------------------------------------------------------------------* 
  
    Linkage:
           This function is called internally from within the device driver. 

 *--------------------------------------------------------------------------* 
  
    INPUT:  These are the input parameters to the create_default_context
             function:
              . pointer to the device dependent data structure
          
          
    OUTPUT:  none 
          
    RETURN CODES:  none         
          
          
                                 End of Prologue                                
 ******************************************************************************/




mid_load_default_context(midddf_t *ddf)

{
  mid_rcx_t	*pmidRCX;
  int           saved_intr_priority;

  /**************************************************************************
       Set up the pointer to the bus address (in ddf),
   **************************************************************************/
  HWPDDFSetup;

  MID_DD_ENTRY_TRACE (midswitch, 1, DEFAULT_CONTEXT, ddf, ddf, 
	ddf->default_context_RCX, ddf->current_context_midRCX->pRcx , 0 ) ;

  BUGLPR(dbg_middfltctx, 1, ("\n\n\n\n"));
  BUGLPR(dbg_middfltctx, 1, ("Top of load_default_context\n"));
  BUGLPR(dbg_middfltctx, 2, ("ddf = 0x%x \n",ddf ) );

  pmidRCX = (mid_rcx_t *)ddf->default_context_RCX->pData;




     /********************************************************************* 

        Put the default context on the adapter.   Use the following parameters:

          OLD CONTEXT PARAMETERS
          . Old Context Handling = 0 = no old context 
          . other old context parameters are not applicable

          NEW CONTEXT PARAMETERS
          . New Context Handling = 1 = create a new context 
          . New Context Type = 2D  
          . low water = 0 (since the interrupt is initially disabled),
          . high water = 32 (the HWM interrupt is initially disabled,
                   however this value is roughly equivalent to a full FIFO),  
          . context ID = device dependent context structure address     
          . other new context parameters are not applicable

      *********************************************************************/

	saved_intr_priority = i_disable (INTMAX) ;

	/*-----------------------------------------------------------------*
         I/O will occur:  Set up for I/O operations: 
          . enable PIO,
          . and save the current bus state. 
	*-----------------------------------------------------------------*/

	PIO_EXC_ON () ;

	MID_SLOW_IO ( ddf );

	MID_SetContext_SE ( ddf, MID_SC_UPDATES_REQUIRED,
			MID_NO_OLD_CTX, MID_INITIATE_NEW_CTX,           
                        0, 0, 0,       /* old ctx ID, addr and length   */  
                        pmidRCX, pmidRCX->type, /* new context ID, type */
                        0, 0,       /* new context addr and length   */
                        64, 2095);     /* FIFO high and low water marks */

	BUGLPR(dbg_middfltctx, 2, ("Context SE written \n") );

	wait_fifo (ddf) ;

	ddf -> current_context_midRCX = pmidRCX ;

	MID_FAST_IO ( ddf );

	PIO_EXC_OFF () ;

	/*******************************************************************
	   Set flag indicating the context is on the adapter.
	   Then set flag indicating there are context state updates to perform.
	   Currently, this is true for any first time context.
	*******************************************************************/

	pmidRCX -> flags.context_on_adapter = 1 ;
	pmidRCX -> flags.adapter_knows_context = 1 ;

	ddf -> dom_flags |= ( MID_CONTEXT_SWITCH_IN_PROCESS |
				CONTEXT_UPDATES_REQUIRED |
				HEAVY_SWITCHING |
				SLEEPING_FOR_FIFO_AVAILABLE |
				WAITING_FOR_FIFO_AVAILABLE );


	ddf -> dom_fifo_sleep_word = EVENT_NULL;
	ddf -> dom_fifo_sleep_flag = NULL;
#if 0
	BUGLPR(dbg_midswitch, 1, ("Going to sleep \n" ) );

	e_sleep ( &(ddf -> dom_fifo_sleep_word), EVENT_SHORT) ;

	ddf -> dom_flags &= ~( MID_CONTEXT_SWITCH_IN_PROCESS |
				CONTEXT_UPDATES_REQUIRED |
				HEAVY_SWITCHING |
				SLEEPING_FOR_FIFO_AVAILABLE |
				WAITING_FOR_FIFO_AVAILABLE );
#endif

	i_enable (saved_intr_priority) ;
	
	while ( !ddf -> dom_fifo_sleep_flag )
	{
		/* Waiting for fifo available.				*/
	}

	ddf -> dom_flags &= ~( MID_CONTEXT_SWITCH_IN_PROCESS |
				CONTEXT_UPDATES_REQUIRED |
				HEAVY_SWITCHING |
				SLEEPING_FOR_FIFO_AVAILABLE |
				WAITING_FOR_FIFO_AVAILABLE );


        BUGLPR(dbg_midswitch, 2, ("midRCX flags = 0x%4X,  dom flags = 0x%4X \n",
		 			pmidRCX-> flags,  ddf-> dom_flags));

	BUGLPR(dbg_middfltctx, 1, ("EXIT 2D: Leaving load_def_ctx\n\n"));

  	MID_DD_EXIT_TRACE (midswitch, 1, DEFAULT_CONTEXT, ddf, ddf, 
			(ddf-> current_context_midRCX)-> pRcx, ddf-> dom_flags,
			*(ulong *)(&(pmidRCX->flags)) );

	return(MID_LIGHT_SWITCH) ;

}	 /*  end of default context routine */ 
