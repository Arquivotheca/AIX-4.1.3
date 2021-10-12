/* @(#)79	1.2  src/bos/kernext/disp/ped/intr/mid_check_water_level.h, peddd, bos411, 9428A410j 3/19/93 18:59:32 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_check_water_level
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
  
   Function Name:    mid_check_water_level 
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        Check if FIFO filling/draining state has changed. 
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
      This routine determines whether a High Water or Low Water Condition
      has occurred (and not been previously detected). 

      It is expected (but not required) that this routine is called at 
      "New Context Handling Complete" time, since this is when the 
      fifo_size and Next SE size fields are updated in the Adapter Status Area.

      If a new High Water or Low Water condition is detected, the appropriate
      processing is performed. 


 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
       1. This routine performs I/O (sometimes).  It is assumed that bus
           access is provided by the caller. 
       2. This routine uses the previous_RCX field in ddf.  It is assumed
           that the caller has verified that this field does exist and is
           valid. 
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
           Normal call

  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the device dependent physical device structure 
            . a ulong containing the current fifo_size. 
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
                                 End of Prologue                                
 ******************************************************************************/



MID_MODULE(water_level);


long mid_check_water_level ( 
			    midddf_t	*ddf,
			    ulong	fifo_size )
{
	mid_rcx_t  *pmidRCX ;

	ulong       fifo_limit ; 
	ulong       next_SE_size ; 

	HWPDDFSetup;			/* Set up addressability to hw */


	/*------------------------------------------------------------------*
	  Init: 
	    . (Previous) midRCX pointer 
	 ------------------------------------------------------------------*/

	pmidRCX = ((mid_rcx_t *)(ddf-> previous_context_RCX->pData)) ; 


	/*------------------------------------------------------------------*
   	   Trace the entry parms
	 *------------------------------------------------------------------*/
    	MID_DD_ENTRY_TRACE (water_level, 1, DSP_CHKWATER, ddf, ddf, 
			    pmidRCX, *(ulong *)(&pmidRCX->flags), fifo_size );


        BUGLPR(dbg_water_level, 1, ("Entering mid_check_water_level, \n" ));
        BUGLPR(dbg_water_level, 2, ("Entry parms: ddf = 0x%X \n",ddf ));
        BUGLPR(dbg_water_level, 2, ("             fifo_size = %X\n",fifo_size));
        BUGLPR(dbg_water_level, 2, ("             mid RCX   = 0x%X\n",pmidRCX));
        BUGLPR(dbg_water_level, 2, ("             RCX flags = %X\n",
							pmidRCX->flags));



        /*******************************************************************

          We will split the work into two cases: 
	    . currently waiting for High Water and
	    . currently waiting for Low Water.

          If we are currently waiting for a High Water Interrupt, then 
          we will check to see if the High Water condition has occurred.
          (If so, we will process it.)

          Similarly, if we are waiting for a Low Water Interrupt, we will
	  check to see if that condition has occurred.   The check for the
	  Low Water condition is mor involved, so we will expand on it below.

        *******************************************************************/


	if (!(pmidRCX->flags.waiting_for_FIFO_low_water_mark) )
	{
       	    switch (pmidRCX-> type)
       	    { 
		case RCX_2D : 	
		    fifo_limit = MID_RCX_2D_HIGH_WATER_FIFO_SIZE ; break ; 
		case RCX_3DM1 :
		    fifo_limit = MID_RCX_3DM1_HIGH_WATER_FIFO_SIZE ; break ; 
		case RCX_3DM2 : 
		    fifo_limit = MID_RCX_3DM2_HIGH_WATER_FIFO_SIZE ; break ; 
       	    } 

       	    if (fifo_size >= fifo_limit)
       	    { 
       	        mid_high_water_com (ddf, pmidRCX) ;
       	    } 
	}

	else /* must be waiting for the low water condition */ 
	{
        /*******************************************************************

          We are waiting for low water. 

	  There are two low water conditions: 
	    . fifo_size is smaller than the low_water_mark,
	    . next SE is not yet complete in the FIFO (fake low water).

          In either event, we will perform the low water processing. 

        *******************************************************************/

       	    switch (pmidRCX-> type)
       	    { 
		case RCX_2D : 	  fifo_limit = RCX_2D_LWM_MARK ; break ; 
		case RCX_3DM1 :	  fifo_limit = RCX_3DM1_LWM_MARK ; break ; 
		case RCX_3DM2 :	  fifo_limit = RCX_3DM2_LWM_MARK ; break ; 
       	    } 

       	    if (fifo_size <= fifo_limit)
       	    { 
        	BUGLPR(dbg_water_level, 1, ("Real LOW WATER \n") );
		pmidRCX->flags.low_water_condition_pending = 1 ; 
       	    } 
	
       	    else /* not the regular low water condition */
       	    { 
                /*--------------------------------------------------------*
                  Not the regular low water condition.  So we will read the 
		   "Next SE size" and check for a fake low water condition. 
		   Note that the next_SE_size is in bytes.  It must be 
		   converted to words. 
                 *--------------------------------------------------------*/
        	MID_RD_ASCB_VALUE (4, next_SE_size) ; 
        	BUGLPR(dbg_water_level, 1, ("next SE size is 0x%X \n", 
							next_SE_size));

       	    	if ( (next_SE_size>>2) > fifo_size)
       	    	{ 
        		BUGLPR(dbg_water_level, 1, ("FAKE LOW WATER \n") );
			pmidRCX->flags.low_water_condition_pending = 1 ; 
       	    	} 
       	    } 
	}


        /********************************************************************* 
           Trace at the exit point 
         *********************************************************************/

        MID_DD_EXIT_TRACE (water_level, 2, DSP_CHKWATER, ddf, ddf, 
		next_SE_size, *(ulong *)(&pmidRCX->flags), fifo_size );


        BUGLPR(dbg_water_level, 2, ("             RCX flags = %X\n",
							pmidRCX->flags));
	BUGLPR(dbg_water_level, 1, ("Leaving mid_check_water_level \n"));

	return (0);
}
