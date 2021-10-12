/* @(#)80	1.2  src/bos/kernext/disp/ped/intr/mid_hicom.h, peddd, bos411, 9428A410j 3/19/93 19:00:06 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_high_water_com
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
  
   Function Name:    mid_high_water_com 
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        High water common processing
      

 *--------------------------------------------------------------------------* 
  
   Function:                                      
	This routine perform the processing common to all "High Water 
	Conditions". 

      Generally, this routine is called when the High Water Interrupt is
      received from the adapter, however, there is at least one other 
      High Water condition detected by the devive driver (at context switch
      time) which also invokes this processing.

      Processing includes:
        . setting a flag to indicate we have received the High Water condition, 
        . putting the graphics process to sleep (so it does not
           overrun the FIFO).  See the following paragraph.

	Note that the processing does NOT include any I/O, especially
	changing the BIM status mask.


   Notes:                                         
      1. This routine DOES NO I/O. 


 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
                  none
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
           normal call, may be called on the interrupt level 

  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the device dependent physical device structure 
            . a pointer to the appropriate midRCX 
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
                                 End of Prologue                                
 ******************************************************************************/





MID_MODULE(hi_water_com);


long mid_high_water_com ( 
			midddf_t	*ddf, 
			mid_rcx_t	*pmidRCX	)
{
	rcx		*pRCX = pmidRCX-> pRcx ; 

	gscDev		*pGD ;
	devDomain	*pDom ;




	/*------------------------------------------------------------------*
   	   Trace the entry parms
	 *------------------------------------------------------------------*/

        BUGLPR(dbg_hi_water_com, 1, ("Entering mid_high_water \n" ));
        BUGLPR(dbg_hi_water_com, 2, ("         ddf = 0x%X \n", ddf ));
        BUGLPR(dbg_hi_water_com, 2, ("        RCX  = 0x%X \n", pRCX ));
        BUGLPR(dbg_hi_water_com, 2, ("     midRCX  = 0x%X \n", pmidRCX ));

    	MID_DD_ENTRY_TRACE (hi_water_com, 2, HIGH_WATER_COMMON, ddf,
			ddf->current_context_midRCX, 
  			pmidRCX, 
			* (ulong *)( &(pmidRCX-> flags)), 
			pRCX ) ; 




	/*------------------------------------------------------------------*
          Set the "waiting for LOW WATER MARK" flag 
	 *------------------------------------------------------------------*/

    	pmidRCX -> flags.waiting_for_FIFO_low_water_mark = 1 ;

        BUGLPR(dbg_hi_water_com, 2, ("context flags = 0x%X \n", pmidRCX -> flags));



	/*------------------------------------------------------------------*
          Now Block the process 

          This function notifies the (device independent RCM) fault handler
          that this process is to be blocked.  This will cause the process'
          adapter authorization to be removed.  This, in turn, causes the
          next adapter access to generate a graphics fault.  The fault handler
          then processes the block request, putting the process to sleep 
          (uexblock). 
	 *------------------------------------------------------------------*/

    	if (pmidRCX-> flags.default_context != 1)
    	{ 
    		pDom = pRCX-> pDomain ;
    		pGD  = pDom-> pDev ;

    		(*pGD-> devHead.pCom-> rcm_callback-> block_graphics_process)
    				(pDom, pRCX-> pProc) ;
    	} 




        /********************************************************************* 
           Trace at the exit point 
         *********************************************************************/
	/* brkpoint(ddf, pRCX, pmidRCX, 0xbeef); */

        MID_DD_EXIT_TRACE (hi_water_com, 1, HIGH_WATER_COMMON, ddf,
				ddf->current_context_midRCX, 
				pmidRCX,  
				* (ulong *)( &(pmidRCX -> flags)), 
				0xF0 ) ;

	BUGLPR(dbg_hi_water_com, 2, ("Leaving mid_high_water \n"));
	return (0);

}

