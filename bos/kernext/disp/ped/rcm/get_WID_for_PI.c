static char sccsid[] = "@(#)64  1.6.1.3  src/bos/kernext/disp/ped/rcm/get_WID_for_PI.c, peddd, bos411, 9428A410j 11/3/93 11:52:23";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: get_WID_for_PI
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
MID_MODULE (get_WID_for_PI);
#define    dbg_midwid  dbg_get_WID_for_PI


extern long mid_delete_WID  (
                              ulong       ,
                              midddf_t   *,
                              rcmWG      * ) ;


/***************************************************************************** 
                              Debug Variables
 *****************************************************************************/
#if  0  
#define  MID_DEBUG_GET_FOR_PI 
#endif  




/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:   get_WID_for_PI
  
   Descriptive Name:   Get Window ID for Pixel interpretation (only)
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      

     This routine obtains a window ID guaranteeing the specified window 
     the best possible pixel interpretation.  For this adapter, pixel
     interpretation consists solely of color palette and the frame buffer     
     being displayed.

     In most cases, we can obtain a window ID that allows the exact pixel 
     interpretation.  There are, however, cases where we cannont find a 
     window ID that will yield the proper PI.  In this event, we attempt
     to generate the best possible match.  


 *--------------------------------------------------------------------------* 
  
    Restrictions:
        none 
  
    Dependencies:
      
  
 *--------------------------------------------------------------------------* 
  
    Linkage:

	 Called internally from the Ped device driver routines:

	  . update_win_geom when the window geometry being updated  
               is currently being used to render.
	  . get_WID (for render) 
	
  
 *--------------------------------------------------------------------------* 
  
    INPUT:
          
    OUTPUT: 
          
    RETURN CODES:         
          
          
          
                                 End of Prologue                                
 ******************************************************************************/



long get_WID_for_PI (
                           gscDev    *pGD,	/* ptr to device depend data */
                           rcmWG     *pWG   )	/* ptr to DI win geom struct */
{
midddf_t 	*ddf = (midddf_t *) (pGD -> devHead.display -> free_area) ;
mid_wid_data_t  	*wid_data ;

mid_wid_status_t 	*WID_entry ,		/* WID entry in loop */
                 	*WID_last,		/* end of loop comparison */
                 	*last_shared ,		/* last shared WID */
                 	*last_unshared ,	/* last unshared WID */
                 	*sharing_entry ,	/* entry force to combine */ 
                 	*loser ;		/* loser's WID entry */

midWG_t   		*midWG , 		/* dev dependent win geom */
          		*last_WG ,     		/* last_WG in WG chain */
          		*loser_midWG ; 		/* loser's device dep WG */

mid_wid_t		wid, new_wid ;

ushort 			color ;
midPI_t          	WGpi ; 

int     rc ; 

int    			saved_intr_priority ;
int    			our_intr_data ;

HWPDDFSetup ;       



	midWG =	(midWG_t *)pWG->pPriv;
	wid = midWG->wid;		/* grab current WID */

    	MID_DD_ENTRY_TRACE (get_WID_for_PI, 2, GET_WID_PI, ddf,
				 pGD, pWG, wid, 0 ) ; 


	/*-------------------------------------------------------------
	    First, a little error checking.
 	*---------------------------------------------------------------*/
        BUGLPR(dbg_midwid, BUGNTA, ("Entering get_WID_for_PI. \n"));
	BUGLPR(dbg_midwid, BUGNTA, ("pGD=0x%8X, pWG=0x%8X \n", pGD , pWG ));

	if (pGD == NULL || pWG == NULL)
	{
		BUGLPR(dbg_midwid, 0, ("One of these is zero.\n" ));
		BUGLPR(dbg_midwid, 0, ("EXIT with no activity.\n\n"));

		return(MID_ERROR);
	}

		


        /********************************************************************* 
	  
	  INITIALIZATION:

	      Init pointer to the WID list. 
	      Get this device driver's interrupt priority.
	      Then get the current color from the WG. 

         *******************************************************************/
	
	our_intr_data = pGD -> devHead.display->interrupt_data.intr.priority-1 ;

	wid_data = &(ddf -> mid_wid_data) ;

	color = ((midWG_t *)(pWG->pPriv)) -> pi.pieces.color ; 


	BUGLPR(dbg_midwid, BUGNTA, ("WID = %X , color = %d \n", wid, color) );






        /********************************************************************* 
	   CASE 0:  window doesn't need a WID (NULL region)
	  
	    In the case where the region is NULL (and the WID is not guarded),
	    no WID is required.  The important case here is X's fullscreen
	    window.  Currently, other parts of the DD require a window for DMA.
	    So this is the only X window we know about.  We don't want to
	    spend a WID on it. 
 	  
         *********************************************************************/
	
	if ( (midWG->pRegion == NULL) &&
	     (wid_data->mid_wid_entry[wid].state != MID_WID_GUARDED) )
	{
		BUGLPR(dbg_midwid, 0, ("REGION is NULL\n") );

		if (wid != MID_WID_NULL) 
	        { 
			BUGLPR(dbg_midwid, 0, ("PUT WID %X BACK\n", wid));
			/*------------------------------------------------
			   Delete the old WID from wherever it was. 
			------------------------------------------------*/
			mid_delete_WID (NULL, ddf, pWG) ; 
	        } 

        	/***************************************************** 
        	   Trace at the exit point 
         	*****************************************************/

        	MID_DD_EXIT_TRACE (get_WID_for_PI, 1, GET_WID_PI, ddf, 
					   pGD, pWG, midWG->wid, 0xF7); 

		BUGLPR(dbg_midwid, BUGNFO,
			("EXIT 7: get_PI === WID = %X ===\n\n", midWG->wid));
		return (MID_NEW_WID) ;
	}




        /********************************************************************* 
	   CASE 1:  window already has unshared WID 
	  
	    Handle the easiest case first: the WG already has a WID 
	    it can use.
            (Ensure the window has a valid ID, first).
	  
	    If we do already have an unshared WID, then ensure the PI
	    PI (pixel interpretation) is OK and return its' current WID.
 	  
         *********************************************************************/
	
	if (wid != MID_WID_NULL) 
	{
		if ( (wid_data->mid_wid_entry[wid].state == MID_WID_UNSHARED) ||
	    	     (wid_data->mid_wid_entry[wid].state == MID_WID_GUARDED) )
		{
		BUGLPR(dbg_midwid, BUGNTA, ("WID %X already unshared.\n",wid));

			/*---------------------------------------------------
		 	     if color is wrong, fix it! 
		 	 *--------------------------------------------------*/

			FIX_COLOR_PALETTE (ddf, wid, color,
    						midWG-> pi.pieces.flags) ; 


        		/***************************************************** 
        		   Trace at the exit point 
         		*****************************************************/

        		MID_DD_EXIT_TRACE (get_WID_for_PI, 1, GET_WID_PI, ddf, 
					   pGD, pWG, midWG->wid, 0xF1); 

			BUGLPR(dbg_midwid, BUGNFO,
			("EXIT 1: get_PI === WID = %X ===\n\n", midWG->wid));
			return (MID_OLD_WID) ;
		}
	


            /***************************************************************** 
	       CASE 1A:  window already has the correct Pixel Interpretation
	  
	        It is thought that the code design precludes us from getting  
	        here with the PI already correct.  However, if we do, we make
                a quick check here, that it is not already correct. 
	  
             *****************************************************************/
	
		if (wid_data -> mid_wid_entry[wid].pi.PI == midWG -> pi.PI)
		{

        	    /***************************************************** 
        	       Trace at the exit point 
         	    *****************************************************/

        	    MID_DD_EXIT_TRACE (get_WID_for_PI, 1, GET_WID_PI, ddf, 
					   pGD, pWG, midWG->wid, 0xF2); 

		    BUGLPR(dbg_midwid, BUGNFO,
			("EXIT 2: PI OK. === WID = %X ===\n\n", midWG->wid));
		    return (MID_OLD_WID) ;

		}
	}



        /********************************************************************* 
         ********************************************************************* 
                                DISABLE INTERRUPTS 
         ********************************************************************* 
         *********************************************************************/
	saved_intr_priority = i_disable (our_intr_data) ;



	/*------------------------------------------------------------
	   Delete the old window from wherever it was. 
	------------------------------------------------------------*/
	mid_delete_WID (NULL, ddf, pWG) ; 





        /********************************************************************* 

	   CASE 2:  Matching pixel interpretation exists on the SHARED LIST
	  
 	    This section checks to see if one of the SHARED window IDs is
            already using the same pixel interpretation as this window.
            If so, this window can use it too.

 	    Assuming a matching SHARED WID is available,  we will add this
            window (geometry) to the WID's WG chain and update a few other 
            relavent data structure fields: 
             . mid WG's WID

         *********************************************************************/

#ifdef  MID_DEBUG_GET_FOR_PI
	midl_print (ddf, "all") ;         /* Write the WID list */ 
#endif
        

        /*-------------------------------------------------------------------
	   The search for a matching pixel interpretation traverses the 
           SHARED LIST from bottom to top.  This is done in an attempt to
	   have the oldest windows (those rendering least) do the most
 	   sharing of WIDs.  
         --------------------------------------------------------------------*/

#define MID_NO_MATCHING_PI    0  
#define MID_MATCHING_PI_FOUND 1 


        rc = MID_NO_MATCHING_PI ;     
        midWG = (midWG_t *) (pWG -> pPriv) ;
        WGpi = midWG -> pi ;     

        WID_last = &(wid_data -> shared_list.Top) ; 
        WID_entry =  wid_data -> shared_list.Bottom.pPrev ; 

	BUGLPR(dbg_midwid, BUGNTA, 
		("top of shared list = 0x%8X, bottom entry = 0x%8X \n", 
			WID_last, WID_entry ));
	BUGLPR(dbg_midwid, BUGNTA, 
		("PI to match = %X (for midWG %8X) \n", WGpi.PI, midWG )) ; 


        while (WID_entry != WID_last)
        { 
            if (WID_entry -> pi.PI == WGpi.PI)
            { 
	        BUGLPR(dbg_midwid, BUGNTA, 
			("MATCHING PI FOUND! midWG = 0x%X, PI %X\n",
			WID_entry -> pwidWG, WID_entry -> pi.PI ));

#ifdef  MID_DEBUG_GET_FOR_PI
		mid_print_WG_chain (WID_entry) ; 	/* print WG chain */
#endif

        	/*-----------------------------------------------------------
                   MATCHING PI found   

                   put this WG on top of WG chain 
         	 *-----------------------------------------------------------*/
                midWG -> pPrev = 0 ;
                midWG -> pNext = WID_entry -> pwidWG ;

                midWG -> pNext -> pPrev = midWG ;
                WID_entry -> pwidWG = midWG ;

                /*---------------------------------------------------------
                   Now increment the shared WID's use count 
                 --------------------------------------------------------*/
                WID_entry -> use_count += 1 ;		/* incr use count */

                /*---------------------------------------------------------
                   now update the device dependent WG structure   
                 --------------------------------------------------------*/
                midWG -> wid = WID_entry -> mid_wid ;



#ifdef  MID_DEBUG_GET_FOR_PI
		mid_print_WG_chain (WID_entry) ; 	/* print WG chain */
#endif

                rc = MID_MATCHING_PI_FOUND ;     
                break ; 
            } 
            WID_entry = WID_entry -> pPrev ; 
        } 





        /*-------------------------------------------------------------------

	   The search (in the SHARED LIST) is over.  Now check if it was 
           successful. 
           
           If it was successful, then we do our usual fixing up of the the   
           WID planes and the color map association. 

         --------------------------------------------------------------------*/

        if ( rc == MID_MATCHING_PI_FOUND )     
        { 
	        wid = midWG -> wid;		/* grab new WID */
		BUGLPR(dbg_midwid, BUGNTA, ("New wid is %X.\n", wid));

        	/************************************************************* 
	                       Re-enable interrupts 
         	*************************************************************/
		i_enable (saved_intr_priority) ; 

                WRITE_WID_PLANES (pGD, midWG -> pWG,
                                  wid,
                                  midWG -> pRegion,  
                                  MID_UPPER_LEFT_ORIGIN ) ; 


        	/***************************************************** 
        	   Trace at the exit point 
         	*****************************************************/

        	MID_DD_EXIT_TRACE (get_WID_for_PI, 1, GET_WID_PI, ddf, 
					   pGD, pWG, midWG->wid, 0xF3); 

		BUGLPR(dbg_midwid, BUGNFO,
			("EXIT 3: get_PI === WID = %X ===\n\n", midWG->wid));
		return (MID_NEW_WID) ;
        } 






        /********************************************************************* 
	   CASE 3:  an UNUSED ID is available      
	  
 	    The next easiest case is to get an unused window ID (assuming
            one is available).   get_unused_WID does this.

 	    Assuming an UNUSED ID is available, get_unused_WID has returned
            it to us and managed the necessary WID table fields.  It also 
            updates the window geometry, so the only things left to do, are
            the adapter I/O: 
             . notify the adapter of our new ID--color mapping,
             . write the WID planes with the new ID

         *********************************************************************/

	rc = get_unused_WID (MID_GET_UNSHARED_WID, ddf, pWG) ;

	if ( rc == MID_WID_ASSIGNED )		 /* if WID was available */
	{
	        wid = midWG -> wid;		/* grab new WID */
		BUGLPR(dbg_midwid, BUGNTA, ("New wid is %X.\n", wid));

        	/************************************************************* 
	                       Re-enable interrupts 
         	*************************************************************/
		i_enable (saved_intr_priority) ; 


		FIX_COLOR_PALETTE (ddf, wid, color, midWG-> pi.pieces.flags) ; 

                WRITE_WID_PLANES (pGD, midWG -> pWG,
                                  wid,
                                  midWG -> pRegion,  
                                  MID_UPPER_LEFT_ORIGIN ) ; 


        	/***************************************************** 
        	   Trace at the exit point 
         	*****************************************************/

        	MID_DD_EXIT_TRACE (get_WID_for_PI, 1, GET_WID_PI, ddf, 
					   pGD, pWG, midWG->wid, 0xF4); 

		BUGLPR(dbg_midwid, BUGNFO,
			("EXIT 4: get_PI === WID = %X ===\n\n", midWG->wid));

		return (MID_NEW_WID) ;

	}

	

	

        /********************************************************************* 

	   CASE 4:  Matching pixel interpretation exists on the UNSHARED LIST
	  
 	    This section checks to see if one of the UNSHARED window IDs is
            already using the same pixel interpretation as this window.
            If so, this window will use it too.  This means, however, that
            we will be taking the other window off the UNSHARED list.

 	    Assuming a matching UNSHARED WID is available,  we must: 
             . delete that WID entry from the UNSHARED list,
             . add that WID to the SHARED list, 
             . add our (new) window (geometry) to the WID's WG chain, 
             . and update the WID in the device dependent WG data structure. 

         *********************************************************************/

        /*-------------------------------------------------------------------
	   The search for a matching pixel interpretation traverses the 
           UNSHARED LIST from bottom to top.  This is done in an attempt to
	   have the oldest windows (those rendering least) do the most
 	   sharing of WIDs.  
         --------------------------------------------------------------------*/

        rc = MID_NO_MATCHING_PI ;     


        WID_last = wid_data -> unshared_list.Top.pNext ; 
        WID_entry = wid_data -> unshared_list.Bottom.pPrev ; 

	BUGLPR(dbg_midwid, BUGNTA, 
		("top entry of unshared list = 0x%8X, bottom entry = 0x%8X \n", 
			WID_last, WID_entry ));
	BUGLPR(dbg_midwid, BUGNTA, 
		("PI to match = %X (for midWG %8X) \n", WGpi.PI, midWG )) ; 


        if ( WID_last != &(wid_data -> unshared_list.Bottom) ) 
        { 
	BUGLPR(dbg_midwid, BUGNTA, ("unshared list not empty \n"));

        while (WID_entry != WID_last)
        { 
            if (WID_entry -> pi.PI == WGpi.PI)
            { 
		BUGLPR(dbg_midwid, BUGNTA, 
			("MATCHING PI FOUND! midWG = 0x%X, PI %X\n",
			WID_entry -> pwidWG, WID_entry -> pi.PI ));

#ifdef  	MID_DEBUG_GET_FOR_PI
		mid_print_WG_chain (WID_entry) ; 	/* print WG chain */
#endif
                /*-----------------------------------------------------------
	           MATCHING PI FOUND 
                  
	             First, move the WID from the UNSHARED list to the 
 	             top of the SHARED list.
                 ------------------------------------------------------------*/
                MID_DELETE_WID_ENTRY (WID_entry) ; 
                MID_ADD_WID_AFTER( (&(wid_data->shared_list.Top)), WID_entry) ;

                /*----------------------------------------------------
                   Update the WID entry state (to SHARED) 
                 ----------------------------------------------------*/
                WID_entry -> state = MID_WID_SHARED ;
                WID_entry -> use_count = 2 ;		/* init use count */


                /*-----------------------------------------------------------
	           Now put our new window (geometry) on the end of the WG chain 
		   for this WID.
                 ------------------------------------------------------------*/
                WID_entry -> pwidWG -> pNext = midWG ;
                midWG -> pPrev = WID_entry -> pwidWG ;
                midWG -> pNext = 0 ;


                /*-----------------------------------------------------------
	           And finally, update the device dependent WG structure 
                 ------------------------------------------------------------*/
                midWG -> wid = WID_entry -> mid_wid ;


#ifdef  	MID_DEBUG_GET_FOR_PI
		mid_print_WG_chain (WID_entry) ; 	/* print WG chain */
#endif
                rc = MID_MATCHING_PI_FOUND ;     
                break ; 
            } 
            WID_entry = WID_entry -> pPrev ; 
        } 


        }   /* end of unshared list not empty */








        /********************************************************************* 

	   The search (in the UNSHARED LIST) is over.  Now check if it was 
           successful. 
           
           If it was successful, then we do our usual fixing up of the the   
           WID planes and the color map association. 

         *********************************************************************/

        if ( rc == MID_MATCHING_PI_FOUND )     
        { 
                wid = midWG -> wid ; 
		BUGLPR(dbg_midwid, BUGNTA, ("New wid is %X.\n", wid));

        	/************************************************************* 
	                       Re-enable interrupts 
         	*************************************************************/
		i_enable (saved_intr_priority) ; 


                WRITE_WID_PLANES (pGD, midWG -> pWG,
                                  wid,
                                  midWG -> pRegion,  
                                  MID_UPPER_LEFT_ORIGIN ) ; 


#ifdef  	MID_DEBUG_GET_FOR_PI
		midl_print (ddf, "all") ;         /* Write the WID list */ 
#endif

        	/***************************************************** 
        	   Trace at the exit point 
         	*****************************************************/

        	MID_DD_EXIT_TRACE (get_WID_for_PI, 1, GET_WID_PI, ddf, 
					   pGD, pWG, midWG->wid, 0xF5); 

		BUGLPR(dbg_midwid, BUGNFO,
			("EXIT 5: get_PI === WID = %X ===\n\n", midWG->wid));
		return (MID_NEW_WID) ;
        } 





        /********************************************************************* 
         ********************************************************************* 

	   To get here the real easy cases have been exhausted.  The only way
	   to get a WID is to force windows with the same PI, but different
	   WIDs, to use the same WID.  Since the number of windows actively
	   rendered to is ordinarily much smaller than the total number of 
	   windows, it makes sense to group the less active windows together.

	   This allows the more active windows to have their own WIDs and 
	   therefore decreases WID contention.

	   We do not have to drain the FIFO at this point, because all WID 
	   management (WID writes and Set Window Parameters) are done       
	   either through the FIFO (X's FIFO) or during the context switch    

         ********************************************************************* 
         *********************************************************************/

									        




        /********************************************************************* 

	   CASE 5:  NO WIDS AVAILABLE, STEAL ONE              
	  
	   We have tried all the easy cases and no WIDs are available,
	   so see if we can steal one. 

         *********************************************************************/


	rc = steal_WID (pGD, pWG) ;  
	
	if ( rc == MID_WID_ASSIGNED )		 /* if WID was available */
	{
		new_wid	= midWG -> wid ;	/* get the new WID */
		BUGLPR(dbg_midwid, BUGNTA, ("New wid is %X.\n", new_wid));

        	/************************************************************* 
	                       Re-enable interrupts 
         	*************************************************************/
		i_enable (saved_intr_priority) ; 


		FIX_COLOR_PALETTE (ddf,new_wid, color, midWG->pi.pieces.flags);

                WRITE_WID_PLANES (pGD, midWG -> pWG,
                                  new_wid,
                                  midWG -> pRegion,  
                                  MID_UPPER_LEFT_ORIGIN ) ; 

        	/***************************************************** 
        	   Trace at the exit point 
         	*****************************************************/

        	MID_DD_EXIT_TRACE (get_WID_for_PI, 1, GET_WID_PI, ddf, 
					   pGD, pWG, midWG->wid, 0xF6); 

		BUGLPR(dbg_midwid, BUGNFO,
			("EXIT 6: get_PI === WID = %X ===\n\n", midWG->wid));

		return (MID_NEW_WID) ;

	}
 




        /********************************************************************* 
         ********************************************************************* 

	   CASE 6:  LAST CHANCE(s): 
	  
	   Nothing else has worked.   Our requestor will not get good visuals.
	   The requesting window will be forced to share apixel interpretation
	   with some other window. 
	   The first choice for sharing a pixel interpretation is the PI
	   for the last (oldest) WID on the shared list.  If however, the
	   shared list is empty, we will share a pixel interpretation with
	   the oldest window on the unshared list. 

	   Note that both lists cannot be empty.  This would imply that
	   all the WIDs are on the GUARDED list, which is not allowed.   

         ********************************************************************* 
         *********************************************************************/

        last_shared =  wid_data -> shared_list.Bottom.pPrev ; 
        last_unshared =  wid_data -> unshared_list.Bottom.pPrev ; 

        if ( last_shared != &(wid_data -> shared_list.Top) )
        { 
	    BUGLPR(dbg_midwid, BUGNFO,("Last chance, shared list not empty\n"));

            /*---------------------------------------------------------
               CASE 6A:  SHARED LIST NOT EMPTY 

               Our requestor will simply be added to the WG chain of the 
               last entry on the shared list.      
             *--------------------------------------------------------*/

            sharing_entry =  last_shared ;
            /*---------------------------------------------------------
               Now increment the shared WID's use count 
             --------------------------------------------------------*/
            sharing_entry -> use_count += 1 ;		/* incr use count */


        } 


        else  /* the shared list is empty */
        { 
	    BUGLPR(dbg_midwid, BUGNFO, ("Last chance, shared list empty \n"));

            /*---------------------------------------------------------
               CASE 6A:  SHARED LIST EMPTY 

               However, there must be entries on the unshared list.  
               The requestor is forced to share pixel interpretation with 
               the oldest WID on the unshared list.  

               Note that this WID entry must also be moved to the shared list.
             *--------------------------------------------------------*/

            sharing_entry =  last_unshared ;


            /*---------------------------------------------------------
               Put the new shared entry onto the shared list.  Note the 
               "last_shared" still points to the top of the list. 

               Note that in this case the WID entry gets moved before
               the WG chain is updated. 
              --------------------------------------------------------*/
            MID_ADD_WID_AFTER (last_shared, sharing_entry) ;

            /*----------------------------------------------------
               Update the WID entry state (to SHARED) 
             ----------------------------------------------------*/
            sharing_entry -> state = MID_WID_SHARED ;
            sharing_entry -> use_count = 2 ;		/* init use count */
    
        } 



        /********************************************************************* 
          We have found the WID entry with which we will share pixel
          interpretation.  Add the requester's WG to the top of the
          WG chain (because it is easiest).  Then update the WID planes.
        **********************************************************************/


        /*---------------------------------------------------------
           Now add our requestor to the front of the WG chain.  
         --------------------------------------------------------*/
        midWG -> pPrev = NULL ;  
        midWG -> pNext = sharing_entry -> pwidWG ;

        midWG -> pNext -> pPrev = midWG ;
        sharing_entry -> pwidWG = midWG ;

        /*---------------------------------------------------------
           Now increment the shared WID's use count 
         --------------------------------------------------------*/
        sharing_entry -> use_count += 1 ;		/* incr use count */
    
        /*---------------------------------------------------------
           Update the requester's WID. 
          --------------------------------------------------------*/
        midWG -> wid = sharing_entry -> mid_wid ;



       	/************************************************************* 
	                       Re-enable interrupts 
       	*************************************************************/
	i_enable (saved_intr_priority) ; 


        /*---------------------------------------------------------
           Now write the requester's WID planes. 
          --------------------------------------------------------*/
  
        WRITE_WID_PLANES (pGD, midWG -> pWG,
        		  midWG -> wid , 
                          midWG -> pRegion,
                          MID_UPPER_LEFT_ORIGIN ) ; 
  


       	/***************************************************** 
       	   Trace at the exit point 
       	*****************************************************/

       	MID_DD_EXIT_TRACE (get_WID_for_PI, 1, GET_WID_PI, ddf, 
					   pGD, pWG, midWG->wid, 0xF0); 

	BUGLPR(dbg_midwid, BUGNFO,
			("Exit 0: get_PI === WID = %X ===\n\n", midWG->wid));

	return (MID_NEW_WID) ;

} /* get_WID_for_PI */
