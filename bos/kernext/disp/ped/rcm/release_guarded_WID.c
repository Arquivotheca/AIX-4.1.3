static char sccsid[] = "@(#)75	1.7.1.5  src/bos/kernext/disp/ped/rcm/release_guarded_WID.c, peddd, bos411, 9428A410j 11/3/93 11:54:31";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_WID_wakeup
 *		release_guarded_WID
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


/***************************************************************************** 
                                  INCLUDES
 *****************************************************************************/

#include <sys/types.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/xmem.h>
#include <sys/rcm_win.h>
#include <sys/aixgsc.h>
#include <sys/rcm.h>
#include "mid.h"
#include "midhwa.h"
#include "midddf.h"
#include "midrcx.h"
#include "midRC.h"
#include "midwidmac.h"
#include "hw_regs_u.h"
#include "hw_macros.h"
#include "hw_ind_mac.h"

#include "mid_dd_trace.h"

/***************************************************************************** 
                                 Externals
 *****************************************************************************/
MID_MODULE (release_guarded_WID);
#define dbg_midwid  dbg_release_guarded_WID



/***************************************************************************** 
                                 Externals
 *****************************************************************************/
extern long mid_delete_WID  (
                              ulong       ,
                              midddf_t   *,
                              rcmWG      * ) ;


extern long release_guarded_WID (
	                  	midddf_t   *,
	                  	mid_wid_t  ) ;


extern long mid_WID_wakeup (mid_watchdog_t   *) ;




/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    release_guarded_WID
  
   Descriptive Name:   Release Guarded Window ID 
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      

     This routine changes a WID from the GUARDED (or HELD) state to the 
     UNSHARED state.  

 *--------------------------------------------------------------------------* 
  
    Restrictions:
         none yet
  
    Dependencies:

  
 *--------------------------------------------------------------------------* 
  
    Linkage:
  
 *--------------------------------------------------------------------------* 
  
    INPUT:
          
    OUTPUT: 
          
    RETURN CODES:         
       . MID_ERROR - if WID for passed window geometry is not currently 
                      guarded.
          
                                 End of Prologue                                
 ******************************************************************************/


long release_guarded_WID (
	                  midddf_t   *ddf ,	/* ptr to adapter structure */ 
	                  mid_wid_t  curr_wid  )	/* current WID */

{
	midWG_t		*midWG;

	mid_rcx_t	*midRCX ;

	mid_wid_data_t		*wid_data ;
	mid_wid_status_t 	*top, *entry ;

	int             curr_state ;


/***************************************************************************** 

    BRIEF REVIEW: 

	 Change WID from GUARDED (or HELD) state to UNSHARED.

 *****************************************************************************/
	


	/*----------------------------------------------------------------
          Init pointer to WID list
	  Get WID entry
	------------------------------------------------------------------*/
	wid_data = &(ddf -> mid_wid_data) ;
       	entry = &(wid_data -> mid_wid_entry [curr_wid]) ; 
	midWG = (midWG_t *) (entry->pwidWG);


    	MID_DD_ENTRY_TRACE (release_guarded_WID, 1, RELEASE_WID_SWAP, ddf, 
				ddf, entry, curr_wid, midWG ) ; 


        BUGLPR(dbg_midwid, BUGNTA, ("Entering release_guarded_WID.\n"));

    	/*-------------------------------------------------------------
        	First, a little error checking.
     	*---------------------------------------------------------------*/


        BUGLPR (dbg_midwid, BUGNTA,
            ("ddf = 0x%8X, WID list = 0x%8X,midWG = 0x%8X\n",
                   ddf,wid_data,midWG));

	/*----------------------------------------------------------------
	  Get current WID state (for this window).
	------------------------------------------------------------------*/
        curr_state = wid_data -> mid_wid_entry [curr_wid].state ; 

        BUGLPR (dbg_midwid, BUGNTA,
               ("WID = %X,  state = %d \n",curr_wid, curr_state) );


	/********************************************************************* 
	    Ensure WID is on GUARDED LIST
	 *********************************************************************/

	if (curr_state != MID_WID_GUARDED)
	{	
        	BUGLPR(dbg_midwid, BUGNTA, ("WID was not GUARDED!\n") );
        	BUGLPR(dbg_midwid, BUGNTA, ("EXIT release_guarded_WID.\n"));

        	/************************************************************* 
        	   Trace at the exit point 
         	*************************************************************/

        	MID_DEBUG_WID_LIST (release_guarded_WID, 1, ddf) ; 
	
        	MID_DD_EXIT_TRACE (release_guarded_WID, 1,  RELEASE_WID_SWAP, 
				ddf, ddf, entry, 0xE0, curr_state); 

		return (MID_ERROR) ;
	}




        /*--------------------------------------------------------
            Remove from guarded list. 
         *--------------------------------------------------------*/
        MID_DELETE_WID_ENTRY (entry) ; 

	ddf->mid_guarded_WID_count--;


        /*--------------------------------------------------------
           IF window (geometry) has already been deleted

	   If the window geometry has already been deleted, we
	   put the WID on the unused list. 
	   If it is still in use, it goes on the unshared list
	   (as the second entry). 
         *--------------------------------------------------------*/
	if (entry->pwidWG == NULL) 
	{ 
        	/*--------------------------------------------------------
         	   Add to next to top of unused list (and set the state).
        	 *--------------------------------------------------------*/
		entry -> state = MID_WID_UNUSED ;      
		top = &(wid_data-> unused_list.Top) ;

	} 

	else 	/* the WG is still alive and the WID still in use */
	{ 
        	/*--------------------------------------------------------
          	  Now add to next to top of unshared list (and set the state
		  to unshared).  Note that the use_count is already 1.  
		  Similarly, the window geometry chain is unchanged.  
        	 *--------------------------------------------------------*/
		entry -> state = MID_WID_UNSHARED ;      

		top = &(wid_data -> unshared_list.Top) ;
		if (top -> pNext != &(wid_data->unshared_list.Bottom))
			top = wid_data -> unshared_list.Top.pNext ;

	} 

        /*--------------------------------------------------------
          Do the actual list add here. 
         *--------------------------------------------------------*/
	MID_ADD_WID_AFTER (top, entry) ; 



	/********************************************************************* 

	    Now check if anyone is waiting for a guarded WID.

	 *********************************************************************/

	if (ddf-> WID_sleep_top != NULL)    
	{ 
	    /***************************************************************** 
	        Now check if we can do the wake-up  
	     *****************************************************************/

            if (ddf->mid_guarded_WID_count < 
		((ddf->num_DWA_WIDS) - MID_NUM_WIDS_EXCLUDED_FROM_MULTI_BUF) )
	    { 
		midRCX = ddf-> WID_sleep_top ;  
		ddf-> WID_sleep_top = midRCX-> pNext_sleep ;

                if (ddf-> WID_sleep_top == NULL)
                {
                        ddf-> WID_sleep_bottom = NULL ;
                }

		midRCX->flags.waiting_for_WID = 0 ; 

        	MID_DD_TRACE_PARMS (release_guarded_WID, 1, WID_WAKEUP, ddf,
                                        midRCX, ddf-> WID_sleep_top,
                                                ddf-> WID_sleep_bottom,
                                                *(ulong *)&(midRCX->flags) );

		/*---------------------------------------------------
	 	     And actually wakeup the process here. 
	 	 *--------------------------------------------------*/
		e_wakeup ( &(midRCX-> context_sleep_event_word) ) ; 
	    } 
	} 


        /************************************************************* 
           Trace at the exit point 
        *************************************************************/

        MID_DEBUG_WID_LIST (release_guarded_WID, 1, ddf) ; 
	
        MID_DD_EXIT_TRACE (release_guarded_WID, 2, RELEASE_WID_SWAP, ddf,
					ddf, entry, curr_wid, 0xF0); 

	return (MID_RC_OK) ;
}




/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    mid_WID_wakeup
  
   Descriptive Name:   wakeup processes waiting for guarded WIDs
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      

 *--------------------------------------------------------------------------* 
  
    Restrictions:
         none yet
  
    Dependencies:

  
 *--------------------------------------------------------------------------* 
  
    Linkage:
  
 *--------------------------------------------------------------------------* 
  
    INPUT:
          
    OUTPUT: 
          
    RETURN CODES:         
          
                                 End of Prologue                                
 ******************************************************************************/


long mid_WID_wakeup (mid_watchdog_t *dog)
{
	midddf_t	*ddf ;

	mid_rcx_t	*midRCX ;





    	/*-------------------------------------------------------------
        	First, init a couple of pointers, then trace
     	*---------------------------------------------------------------*/

	ddf = dog->ddf ;  
	midRCX = ddf->WID_sleep_top ;  


    	MID_DD_ENTRY_TRACE (release_guarded_WID, 1, WID_WAKEUP, ddf, ddf,
                  	ddf-> WID_sleep_top, ddf-> WID_sleep_bottom, midRCX) ; 

        BUGLPR(dbg_midwid, BUGNTA, ("Entering mid_WID_wakeup.\n"));



    	/*-------------------------------------------------------------
        	If there's anyone to wake-up, wake them up.  
     	*---------------------------------------------------------------*/

        if (midRCX != NULL) 
	{
		ddf-> WID_sleep_top = midRCX-> pNext_sleep ;

        	if (ddf-> WID_sleep_top == NULL)
        	{
                	ddf-> WID_sleep_bottom = NULL ;
        	}

		midRCX->flags.waiting_for_WID = 0 ; 
	
		/*---------------------------------------------------
 		     And actually wakeup the process here. 
 	 	*--------------------------------------------------*/
		e_wakeup ( &(midRCX-> context_sleep_event_word) ) ; 



		/*---------------------------------------------------
 		     And restart the timer 
 	 	*--------------------------------------------------*/

        	if (ddf-> WID_sleep_top != NULL)
        	{
			w_start ( &(ddf-> WID_watch.dog) ) ; 
        	}

        /************************************************************* 
           Trace at the exit point 
        *************************************************************/

        MID_DD_EXIT_TRACE (release_guarded_WID, 1, WID_WAKEUP, ddf,
                  	midRCX, ddf-> WID_sleep_top, ddf-> WID_sleep_bottom,
                       *(ulong *)&(midRCX->flags) ) ; 

        }

	return (MID_RC_OK) ;
}
