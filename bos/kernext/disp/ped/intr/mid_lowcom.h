/* @(#)81       1.4  src/bos/kernext/disp/ped/intr/mid_lowcom.h, peddd, bos411, 9428A410j 4/14/94 14:51:01 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_low_water_com
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
                              
                               Start of Prologue  
  
   Function Name:    mid_low_water_com
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        low water common processing
      

 *--------------------------------------------------------------------------* 
  
   Function:                                      
	This routine handles the processing that is common for all 
	"Low Water Conditions".   This includes:
          . managing a flag to indicate the Low Water condition occurred, 
          . waking up the graphics process so it can use (fill) the FIFO again,
             See paragraph below on the sleep mechanism(s).
          . This routine does not do I/O -- specifically, it does not manage  
             the BIM status mask. 

      There are two types of "sleep" used:  
	. When the High Water condition occurred, we performed a block request
	   (see the high_water function).  This function reomves adapter
	   access permission from the process.  Then, if (when) process 
	   next attempted to use the FIFO, it was actually blocked. 
           All this magic was performed by the RCM gp_block_gp callback 
	   function.
	. Secondly, it is possible that the High Water condition was caused by
	    the kernel putting something in the FIFO.  In this event, the
	    same "block" trick does not work, because the kernel always has
	    adapter access permission (and hence, would never fault).
      	    In this event, the kernel routine actually checks for this 
	    condition and puts itself to sleep (setting a flag so this
	    routine can perfrom the correct wakeup).

   Notes:                                         
      1. This routine DOES NO I/O. 


 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
                  none
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
           Normal call, probably invoked on the interrupt level 

  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the device dependent physical device structure 
            . a pointer to the appropriate midRCX 
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
                                 End of Prologue                                
 ******************************************************************************/

  

MID_MODULE(lo_water_com);



long mid_low_water_com ( 
			midddf_t	*ddf, 
			mid_rcx_t	*pmidRCX	)
{
	rcx		*pRCX = pmidRCX-> pRcx ; 

	gscDev		*pGD ;
	devDomain	*pDom ;





	/*------------------------------------------------------------------*
   	   Trace the entry parms
	 *------------------------------------------------------------------*/

        BUGLPR(dbg_lo_water_com, BUGNTA, ("Entering lo_water_com_lwm, \n" ));

    	MID_DD_ENTRY_TRACE (lo_water_com, 2, LOW_WATER_COMMON, ddf,
                        	ddf->current_context_midRCX,
				pmidRCX,  
				* (ulong *)( &(pmidRCX -> flags)), 
				pRCX ) ;


	/*------------------------------------------------------------------*
          Reset the low water flags and the watchdog counter
	 *------------------------------------------------------------------*/

    	pmidRCX-> flags.waiting_for_FIFO_low_water_mark = 0 ;
    	pmidRCX-> flags.low_water_condition_pending = 0 ;
	pmidRCX-> low_water_switch_count = 0;

        BUGLPR(dbg_lo_water_com, 2, ("context flags = 0x%X \n", pmidRCX-> flags));




	/*------------------------------------------------------------------*
          Check if the kernel is sleeping, waiting for the FIFO

          If it is, wake it up! 
	 *------------------------------------------------------------------*/

    	if (pmidRCX-> flags.sleeping_for_FIFO == 1)
    	{ 
        	BUGLPR(dbg_lo_water_com, 1, 
			("Low Water, kernel sleeping. WAKE UP !!!!!!!!\n"));
    		/* brkpoint (ddf, pmidRCX, pmidRCX->flags, 0x1ee61ee6) ;*/

    		pmidRCX-> flags.sleeping_for_FIFO = 0 ;
		if ( ddf->num_graphics_processes )
    			e_wakeup ( &(pmidRCX->context_sleep_event_word)) ;  
    	} 



	/*------------------------------------------------------------------*

          Now Unblock the process 

          The process has previously been blocked, due to filling up its   
          FIFO (see the lo_water_com_hwm routine).  Now we must undue what we 
          previously had done. 

	 *------------------------------------------------------------------*/

    	if (pmidRCX-> flags.default_context != 1)
    	{ 
    		pDom = pRCX-> pDomain ;
    		pGD  = pDom-> pDev ;

    		(*pGD-> devHead.pCom-> rcm_callback-> unblock_graphics_process)
    				(pDom, pRCX-> pProc) ;
    	} 





        /********************************************************************* 
           Trace at the exit point 
         *********************************************************************/

        MID_DD_EXIT_TRACE (lo_water_com, 1, LOW_WATER_COMMON, ddf,
                        	ddf->current_context_midRCX,
				pmidRCX,  
				* (ulong *)( &(pmidRCX -> flags)), 
				0xF0 ) ;

	BUGLPR(dbg_lo_water_com, 2, ("Leaving fifo_lwm \n"));
	return (0);

}

