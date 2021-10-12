/* @(#)67  1.9.1.8  src/bos/kernext/disp/ped/rcm/get_WID_for_render.c, peddd, bos411, 9435B411a 8/30/94 18:22:41 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: BUGLPR
 *		MID_DDT_PI
 *		get_WID
 *		if
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

#include <rcm_mac.h>
#include "mid_rcm_mac.h"
#include "mid_dd_trace.h"

#define dbg_midwid  dbg_get_WID

/***************************************************************************** 
                                 Externals
 *****************************************************************************/
MID_MODULE (get_WID);

extern long mid_delete_WID  (
                              ulong       ,
                              midddf_t   *,
                              rcmWG      * ) ;



/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:   get_WID_for_render
  
   Descriptive Name:   Get Window ID for render
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      

     This routine ensures a window ID suitable for rendering is assigned  
     to the specified window.  "A window ID suitable for rendering" means
     a window ID which will give us proper window clipping and visuals.   
     (Proper clipping and visuals are discussed further below.)  The window 
     ID is updated in the device dependent portion of the window geometry 
     that is passed.  A return code indicates how successful we were.

     In this case, "visuals" means how the pixels in a particular window             appear (or are presented) on the screen.  For PED, this simply means
     which color table is being used and to which buffer (front or back)             the graphics process is rendering.   

     Proper (window) clipping is guaranteed in a few different ways: 

       . If we can assign a unique window ID to the window, we can 
           guarantee that clipping works.  This is probably the easiest 
           method.  We will spend a fair amount of effort pursuing a unique
           window ID.  We may even try to rearrange the other windows' IDs 
           to accomplish this. 

       . Rectangular 2D windows are guaranteed to have their viewports  
           coincide with the window geometry.  (This is guaranteed by the 
           window system -- our friend Mr X.)  So nothing further is required              for these windows. 
  
      It is possible that none of the above cases can be satisfied.  In this
      event, we will force some windows with similar visuals to share window
      IDs and hope for the best.


 *--------------------------------------------------------------------------* 
  
    Restrictions:
        none 
  
    Dependencies:
      
  
 *--------------------------------------------------------------------------* 
  
    Linkage:

	 Called internally from the Ped device driver routines:

	  . update_win_geom when the window geometry being updated  
               is currently being used to render.
	  . bind to ensure the new window has an unshared window ID
	  . start_switch to ensure the new window has an unshared window ID
  
 *--------------------------------------------------------------------------* 
  
    INPUT:
          
    OUTPUT: 
          
    RETURN CODES:         
          
          
          
                                 End of Prologue                                
 ******************************************************************************/




long get_WID (
               int        req_in,		/* request code  */
               gscDev    *pGD,			/* ptr to device depend data */
               rcmWG     *pWG, 		  	/* ptr to DI win geom struct */
               rcx       *pRCX  )		/* ptr to DI rcx struct */
{

midddf_t 	*ddf = (midddf_t *) (pGD -> devHead.display -> free_area) ;

rcmProc                 *pProc;
rcx                     *pRCX_sleep ;

mid_wid_data_t  	*wid_data ;

mid_wid_status_t 	*WID_entry ,		/* WID list entry */
                 	*entry ,		/* stolen WID entry */
                 	*top ,			/* entry to insert after */
                 	*last_shared ,		/* last shared WID */
                 	*last_unshared ,	/* last unshared WID */
                 	*sharing_entry ,	/* entry force to combine */ 
                 	*loser ;		/* loser's WID entry */

mid_rcx_t    		*midRCX , 		/* dev dependent RCX struct */
             		*next_sleep ;		/* ptr to next RCX on sleep */

midWG_t   		*midWG , 		/* dev dependent win geom */
          		*last_WG ,     		/* last_WG in WG chain */
          		*loser_midWG ; 		/* loser's device dep WG */

mid_wid_t		wid, new_wid ;
ushort 			color ;

int    			WID_state ; 
int    			new_state ; 

int    			saved_intr_priority ;

int     do_color ;			/* Flag for updating color mapping */
int     do_WID_planes ;			/* Flag for updating WID planes */
int     first_loop ;			/* Flag for updating WID planes */
#	define		_NO  0 
#	define		_YES 1 

int        rc_in ;				/* return code of called func */
int        rc_out ;				/* return code for caller */

int        request ;				/* request code to pass on */




    /********************************************************************* 
           Trace / echo the input parms for debug
     *********************************************************************/
    midWG = 	(midWG_t *)pWG->pPriv;

    MID_DD_ENTRY_TRACE (get_WID, 2, GET_WID_RENDER, ddf, pRCX, pWG, 
		MID_DDT_WID(midWG->wid) | (req_in << 4), MID_DDT_PI(midWG));


    /*-------------------------------------------------------------
        First, a little error checking.
     *---------------------------------------------------------------*/

        BUGLPR(dbg_get_WID,BUGNTA, ("Entering get_WID, req = %d\n",req_in));
	BUGLPR(dbg_get_WID, BUGNTA, ("pGD=0x%8X, pWG=0x%8X \n", pGD , pWG ));

	if (pGD == NULL || pWG == NULL)
	{
		BUGLPR(dbg_get_WID, 0, ("One of these is zero.\n" ));
		BUGLPR(dbg_get_WID, 0, ("EXIT with no activity.\n\n"));

		return(MID_ERROR);
	}

		


        /********************************************************************* 
	  
	  INITIALIZATION:

	      Init pointers to the ddf and the WID list. 

         *******************************************************************/
	
	wid_data = &(ddf -> mid_wid_data) ;

	do_color = _YES ;         	/* set color processing to be done */
	do_WID_planes = _NO  ;          /* WID planes, however, default off */





        MID_DEBUG_WID_LIST (get_WID, 2, ddf) ; 


        /********************************************************************* 
         ********************************************************************* 
                                DISABLE INTERRUPTS 
         ********************************************************************* 
         *********************************************************************/


        /*-------------------------------------------------------------------*
         
          Make sure all required pointers are valid (non-NULL and in kernel).

         *-------------------------------------------------------------------*/

        if(ddf->pdev && !(((unsigned long)ddf->pdev) & 0xf0000000)
        && ddf->pdev->devHead.display
        && !(((unsigned long)ddf->pdev->devHead.display) & 0xf0000000)) {
                saved_intr_priority = MID_DD_I_DISABLE (ddf) ;
        }
        else
                return MID_ERROR;


        midRCX = NULL ; 	/* init to 0 to force DSI if used on int lvl*/


        /*-------------------------------------------------------------------* 
	  
	  INITIALIZE:
	        .  the current WID from the WG, 
	        .  the state of that (the current) WID entry, 
	        .  the current color from the WG. 

         *-------------------------------------------------------------------*/

	wid   =  midWG->wid;		
	color =  midWG-> pi.pieces.color ; 

	WID_entry = & (ddf -> mid_wid_data.mid_wid_entry[wid]);
	WID_state = WID_entry -> state ; 

	BUGLPR(dbg_get_WID, BUGNTA, ("WID = %X , color = %d \n", wid, color) );
	BUGLPR(dbg_get_WID, BUGNTA, ("WID state = %d \n", WID_state ) );




    if (req_in != MID_GET_WID_FOR_SWITCH)
    {
       	/*-----------------------------------------------------------
       	  Get the midRCX for this process
       	 *----------------------------------------------------------*/
       	FIND_GP(pGD, pProc);

       	pRCX_sleep = pProc->pDomainCur[0] ;
       	midRCX = ((mid_rcx_t *)(pRCX_sleep->pData)) ;


	/*-------------------------------------------------------------------
	   At the top, we check whether we are in the context switch       
	   window or not.  If so, we simply stop (sleep here) and wait
	   until the context switch finishes. 
	 *------------------------------------------------------------------*/
    	first_loop = 1 ;

    	while (1 == 1) 
    	{ 
	    if (ddf->dom_flags & MID_CONTEXT_SWITCH_IN_PROCESS) 
	    {
                /*-----------------------------------------------------------
                     Add this context (process) to the context sleep chain.
                     Then set up for the sleep (and trace it). 
                 *----------------------------------------------------------*/
                midRCX->pNext_sleep = ddf->WID_ctx_sw_sleep_top ;
                ddf->WID_ctx_sw_sleep_top = midRCX ;

		midRCX-> flags.waiting_for_WID = 1 ;
		midRCX-> context_sleep_event_word = EVENT_NULL ;


        	MID_DD_TRACE_PARMS (get_WID, 1, WID_CTX_SLEEP, ddf, 
				 	midRCX, ddf-> WID_ctx_sw_sleep_top, 
						midRCX-> pNext_sleep, 
					 	*(ulong *)&(midRCX->flags) ); 

		if ( ddf->num_graphics_processes )
                /*---------------------------------------------------
		     In the graphics mode we actually put the process
		     to sleep.
                 *--------------------------------------------------*/
                	while (midRCX-> flags.waiting_for_WID)
                  		e_sleep (&(midRCX-> context_sleep_event_word),
								 EVENT_SHORT);
		else
		{
			i_enable (saved_intr_priority) ;
                	while (midRCX-> flags.waiting_for_WID)
			{
			}
			saved_intr_priority = MID_DD_I_DISABLE (ddf) ;
		}

        	/*-----------------------------------------------------------* 
		  Re-initialize  WID, WID state and WID color (palette)
         	*-----------------------------------------------------------*/

		wid   =  midWG->wid;		
		color =  midWG-> pi.pieces.color ; 

		WID_entry = & (ddf -> mid_wid_data.mid_wid_entry[wid]);
		WID_state = WID_entry -> state ; 

		BUGLPR(dbg_get_WID, 2,("WID = %X, color = %d \n", wid, color));
		BUGLPR(dbg_get_WID, 2, ("WID state = %d \n", WID_state ) );

	    }


	/*-------------------------------------------------------------------
	   At this point, we must check if we've passed our limit of       
	   guarded window IDs.  This limit depends on how many WIDs we have
	   relative to the number of DWA clients currently defined. 

	   If we have enough WIDs for everyone, then there really is no
	   guarded WID limit.  However, if there are not enough WIDs for every
	   client to have its own, then we establish a limit for the number
	   of guarded WIDs.  This limit is the number of DWA WIDs - 2.
	   The extra two WIDs are required for the ensuing algorithm to work
	   properly. 
	 *------------------------------------------------------------------*/



#define _NUM_GUARD_WIDS (ddf->mid_guarded_WID_count) 
#define _NUM_WIDS (ddf->num_DWA_WIDS) 


	if (   (req_in == MID_GET_GUARDED_WID)  
	    && (    ((ddf-> WID_sleep_top != NULL) && (first_loop) )
	         || (_NUM_GUARD_WIDS > _NUM_WIDS - 3)
	         || (   (WID_state != MID_WID_GUARDED) 
	             && (_NUM_GUARD_WIDS == 
			 (_NUM_WIDS - MID_NUM_WIDS_EXCLUDED_FROM_MULTI_BUF) )
		    )
	       )
	   ) 
		{
		    	first_loop = 0 ;

			BUGLPR(dbg_get_WID,  1, ("WID limit reached.\n"));

			/*---------------------------------------------------

		 	   We must put the rascal to sleep  

		 	   First we add the context to the chain of contexts
		 	   (processes) waiting (sleeping) for a guarded WID.

		 	 *--------------------------------------------------*/

			midRCX-> flags.waiting_for_WID = 1 ; 

			if (ddf-> WID_sleep_bottom == NULL)    
			{ 
			    ddf-> WID_sleep_top = midRCX ;  
			} 
			else /* sleep list is not empty -- put on bottom */ 
			{ 
			    ddf-> WID_sleep_bottom->pNext_sleep = midRCX ;  
			} 

			ddf-> WID_sleep_bottom = midRCX ;  
			midRCX->pNext_sleep = NULL ;	/* end of chain */

			/*---------------------------------------------------
		 	   Give up the domain if guarded   
		 	 *--------------------------------------------------*/

            		if (midRCX->flags.domain_guarded_and_current)
            		{                                           
       			    (*pGD->devHead.pCom->rcm_callback->unguard_domain) 
				(pRCX-> pDomain);
            		} 

        		MID_DEBUG_WID_LIST (get_WID, 1, ddf) ; 

        		MID_DD_TRACE_PARMS (get_WID, 1, WID_SLEEP, ddf, 
				 	midRCX, ddf-> WID_sleep_top, 
						ddf-> WID_sleep_bottom, 
					 	*(ulong *)&(midRCX->flags) ); 


       			w_start( &(ddf->WID_watch.dog)) ;

			midRCX-> context_sleep_event_word = EVENT_NULL ; 

			if ( ddf->num_graphics_processes )
			/*---------------------------------------------------
			     In the graphics mode we put the process to sleep.
		 	 *--------------------------------------------------*/
		            while (midRCX-> flags.waiting_for_WID) 
			    	e_sleep ( &(midRCX-> context_sleep_event_word), 
					  EVENT_SHORT) ; 
			else
			{
		 	    i_enable (saved_intr_priority) ;
			    while (midRCX-> flags.waiting_for_WID) 
			    {
			    }
			    saved_intr_priority = MID_DD_I_DISABLE (ddf) ;
			}

        		MID_DD_TRACE_PARMS (get_WID, 1, WID_WAKEUP, ddf, 
				 	midRCX, ddf-> WID_sleep_top, 
						ddf-> WID_sleep_bottom, 
					 	*(ulong *)&(midRCX->flags) ); 

			/*---------------------------------------------------
		 	   WAKE UP is done whenever the guarded WID is
		 	    given up. 

			   Get the domain back if required.
		 	 *--------------------------------------------------*/

            		if (midRCX->flags.domain_guarded_and_current)
            		{                                           
				i_enable (saved_intr_priority) ;

         			MID_GUARD_AND_MAKE_CUR(pGD, pRCX) ;

				saved_intr_priority = MID_DD_I_DISABLE (ddf) ;
            		} 


        		/*---------------------------------------------------* 
			  Re-initialize  WID, WID state and WID color
         		*---------------------------------------------------*/

			wid   =  midWG->wid;		
			color =  midWG-> pi.pieces.color ; 

			WID_entry = & (ddf -> mid_wid_data.mid_wid_entry[wid]);
			WID_state = WID_entry -> state ; 

			BUGLPR(dbg_get_WID, 3, 
				("WID = %X, color = %d\n", wid, color) );
			BUGLPR(dbg_get_WID, 3,("WID state = %d n", WID_state));

		}
		else
		{
			break ;
		}
	}
    } 

	/*-------------------------------------------------------------------
	   convert the request code for the rest of the WID code
	 *-------------------------------------------------------------------*/

	if (req_in == MID_GET_WID_FOR_SWITCH) 
		request = MID_GET_UNSHARED_WID ;
	else  	request = req_in ;






        /********************************************************************* 
         ********************************************************************* 

 	 WHILE GROUP 
	  
	        The following while group allows us to use breaks
	        and have a common exit. 
	  
         ********************************************************************* 
         *********************************************************************/

	/*-------------------------------------------------------------------
	   The midRCX pointer is not used after here, but I reset it just in
	   case because this is a late change.  
	 *------------------------------------------------------------------*/
	midRCX = ((mid_rcx_t *)(pRCX -> pData)) ; 


while ( 1==1 )  /* do forever */
{

        /********************************************************************* 
         ********************************************************************* 
	  
	   CASE 1:  window's current WID is OK 
	  
         ********************************************************************* 
         *********************************************************************/



        /********************************************************************* 

	   CASE 1a:  GUARDED WID REQUEST

         *********************************************************************/
	  
	/*-------------------------------------------------------------------
	   CASE 1a.1:  GUARDED WID REQUEST, window already has GUARDED WID 
	  
	    If we do already have a guarded WID, then ensure the 
	    PI (pixel interpretation) is OK and return. 
	 *------------------------------------------------------------------*/


	if (request == MID_GET_GUARDED_WID )  
	{
		BUGLPR(dbg_get_WID, 3, ("Get GUARDED request \n"));

		if ( WID_state == MID_WID_GUARDED )
		{
			BUGLPR(dbg_get_WID, 3, ("WID already GUARDED\n"));

		    	do_color = _NO ;
                    	do_WID_planes = _NO ;  

			rc_out = MID_OLD_WID ;
			break ; 
		}




	/*-------------------------------------------------------------------
	   CASE 1a.2:  GUARDED WID REQUEST, window already has UNSHARED WID 
	  
	    If we already have an unshared WID, we must move the WID from
	    the UNSHARED list to the guarded list.  Then we ensure the    
	    PI (pixel interpretation) is OK and return. 
	 *------------------------------------------------------------------*/

		if ( WID_state == MID_WID_UNSHARED )
		{
			BUGLPR(dbg_get_WID, 2,("WID %d unshared.\n",wid));


			/*---------------------------------------------------
		 	     Remove the WID entry from the unshared list
		 	 *--------------------------------------------------*/
		 	MID_DELETE_WID_ENTRY (WID_entry) ;


			/*---------------------------------------------------
		 	     Now add it to the top of the Guarded list. 
		 	 *--------------------------------------------------*/
        		top = &(wid_data -> guarded_list.Top) ; 
		 	MID_ADD_WID_AFTER (top, WID_entry) ;


                    	/*----------------------------------------------------
                           Update the WID entry state (to GUARDED) 
                     	------------------------------------------------------*/
                    	WID_entry -> state = MID_WID_GUARDED ;

                    	ddf->mid_guarded_WID_count++;

		    	do_color = _NO ;
                    	do_WID_planes = _NO ;  

			rc_out = MID_OLD_WID ;
			break ;
		}
	}






	  
        /********************************************************************* 
	   CASE 1b:  UNSHARED WID REQUEST, window already has UNSHARED 
	                                                     or GUARDED WID
	  
	    If we already have an unshared or guarded WID, then essentially 
            all we have to do is put this WID on the top of the list and 
	    ensure the PI (pixel interpretation) is OK and return. 

	    NOTE that for a GUARDED WID we must also remove it from the
	          GUARDED list, which implies a a potential wakeup, too. 
         *********************************************************************/

	
	else /*  (request == MID_GET_UNSHARED_WID or SWITCH_WID) */
	{
		BUGLPR(dbg_get_WID, 3, ("Get UNSHARED request \n"));

        	if (WID_state == MID_WID_GUARDED )    
    		{
            		BUGLPR(dbg_get_WID, 3, ("WID is GUARDED \n") ); 

            		/*--------------------------------------------------* 
               		   Since the WID is already guarded, we are done. 
			
		 	   Color palette association checked at exit point
		 	 *--------------------------------------------------*/

			rc_out = MID_OLD_WID ;
			break ;
    		}


#if 0
            	/*--------------------------------------------------* 
               	  Check for a NULL region 

		   In this event, we do not need a WID.  In fact, we'll
	 	   give one back if we have one. 
		 *--------------------------------------------------*/

        	if (midWG-> pRegion == NULL) 
    		{
		    BUGLPR(dbg_get_WID, 0, ("Region is NULL \n"));

        	    if((wid != MID_WID_NULL) && (WID_state != MID_WID_GUARDED))
    		    {
			BUGLPR(dbg_get_WID, 0, ("GIVE WID %X BACK!\n", wid));

			/*--------------------------------------------
	   		    Now delete the WID from wherever it was. 
			--------------------------------------------*/
			mid_delete_WID (request, ddf, pWG) ; 
    		    }

		    do_color = _NO ;
                    do_WID_planes = _NO ;  

		    rc_out = MID_NEW_WID ;
		    break ;
    		}
#endif


            	/*--------------------------------------------------* 
		   Check for the trivial case: 
		   	WG already has an UNSHARED WID
		 *--------------------------------------------------*/

        	if ( WID_state == MID_WID_UNSHARED )    
    		{
			BUGLPR(dbg_get_WID, BUGNTA+2,
		            ("WID %X already UNSHARED\n",wid));

			/*---------------------------------------------------
		 	     Remove the WID entry from the list
		 	 *--------------------------------------------------*/
		 	MID_DELETE_WID_ENTRY (WID_entry) ;


			/*---------------------------------------------------
		 	     Now add it to the top of the unshared list.  
			     This marks this WID as belonging to the 
			     current renderer. 
		 	 *--------------------------------------------------*/
        		top = &(wid_data -> unshared_list.Top) ; 
		 	MID_ADD_WID_AFTER (top, WID_entry) ;

                    	/*----------------------------------------------------
                           Update the WID entry state (to UNSHARED) 
                     	------------------------------------------------------*/
                    	WID_entry -> state = MID_WID_UNSHARED ;

			MID_DEBUG_WID_LIST (get_WID, 2, ddf) ; 


			/*---------------------------------------------------
		 	     Color palette association checked at exit point
		 	 *--------------------------------------------------*/

			rc_out = MID_OLD_WID ;
			break ;
		}  /* end of already unshared state */

	}  /* end of case 1b - UNSHARED WID request */





        /********************************************************************* 
	   CASE 2:  an UNUSED ID is available      
	  
 	    The next easiest case is to get an unused window ID (assuming
            one is available).   get_unused_WID does this.

 	    Assuming an UNUSED ID is available, get_unused_WID has returned
            it to us and managed the necessary WID table fields.  It also 
            updates the window geometry, so the only things left to do, are
            the adapter I/O: 
             . notify the adapter of our new ID--color mapping,
             . write the WID planes with the new ID

         *********************************************************************/

	rc_in = get_unused_WID (request, ddf, pWG) ;

	if ( rc_in == MID_WID_ASSIGNED )	 /* if WID was available */
	{
		wid = midWG -> wid;		/* grab new WID */
		BUGLPR(dbg_get_WID, BUGNTA, 
				("New WID (formerly unused) %X.\n", wid));


		/*---------------------------------------------------
		     Color palette association checked at exit point
		     Set flag to write WID planes, too.  
		  *--------------------------------------------------*/
                do_WID_planes = _YES ;  

		rc_out = MID_NEW_WID ;
		break ;
	}
 
	BUGLPR(dbg_get_WID, BUGNTA+1, ("no UNSHARED ID available\n"))



        /********************************************************************* 
         ********************************************************************* 

			   INTERRUPTS STILL DISABLED 

         ********************************************************************* 
         *********************************************************************/




        /********************************************************************* 
         ********************************************************************* 

	   To get here the real easy cases have been exhausted.  The only way
	   to get a WID is to force windows with the same PI, but different
	   WIDs, to use the same WID.  Since the number of windows actively
	   rendered to is ordinarily much smaller than the total number of 
	   windows, it makes sense to group the less active windows together.

	   This allows the more active windows to have their own WIDs and 
	   therefore decreases WID contention.

	   Note that we do not have to drain the FIFO at this point.  All  
	   WID assignments (Set Window Parms) and WID writes are done through
	   the FIFO (for X) or all through the PCB (at context switch time) for
	   the 3D apps.  Therefore, we need not drain the FIFO. 

         ********************************************************************* 
         *********************************************************************/

	/*------------------------------------------------------------
	   First delete the old window from wherever it was. 
	------------------------------------------------------------*/
	mid_delete_WID (request, ddf, pWG) ; 

									        






        /********************************************************************* 

	   CASE 4:  NO WIDS AVAILABLE, STEAL ONE              
	  
	   We have tried all the easy cases and no WIDs are available,
	   so see if we can steal one. 

         *********************************************************************/


	rc_in = steal_WID (pGD, pWG) ;  
	
	if ( rc_in == MID_WID_ASSIGNED )	/* if WID was available */
	{
		wid	= midWG -> wid ;	/* get the new WID */
		BUGLPR(dbg_get_WID, BUGNTA, ("STOLEN WID is %X.\n", wid));

		/*---------------------------------------------------
		     Move our new WID entry to the top of the list.    
 	             First remove from its current position on the unshared
 	             list. 
	        ----------------------------------------------------*/
		entry = & (ddf -> mid_wid_data.mid_wid_entry[wid]) ;
		MID_DELETE_WID_ENTRY (entry) ;


        	/*-------------------------------------------------------------
 	    	Now put the entry on top of the proper list.   So first we 
 	    	must figure out which list we go on.  We also load a state
 	    	value corresponding to this same list. 
	 	*------------------------------------------------------------*/

		switch (request) 
		{ 
	    	case  MID_GET_UNSHARED_WID:      
	        	top = &(wid_data -> unshared_list.Top) ;
	        	new_state = MID_WID_UNSHARED ;	/* set the state */ 
	        	break ;

	    	case  MID_GET_GUARDED_WID:      
	        	top = &(wid_data -> guarded_list.Top) ;
	        	new_state = MID_WID_GUARDED ;	/* set the state */ 
	        	ddf->mid_guarded_WID_count++;
	        	break ;

	    	default:      				/* should not happen */
	        	break ;
		} 

		MID_ADD_WID_AFTER (top, entry) ;
	
        	/*-------------------------------------------------------------
 	    	    and re-init this to an unshared or guarded WID entry:
	 	*------------------------------------------------------------*/
		entry -> state = new_state ;		/* set the state */ 
		/* entry -> use_count = 1 ;		/* use count = 1 */ 


		/*---------------------------------------------------
		     Color palette association checked at exit point
		     Set flag to write WID planes, too.  
		  *--------------------------------------------------*/

                do_WID_planes = _YES ;  

		rc_out = MID_NEW_WID  ;
		break  ;
	}
 




        /********************************************************************* 
         ********************************************************************* 

	   CASE 5:  LAST CHANCE(s): 
	  
	   Nothing else has worked.   Now we must degrade the visuals of one
	   or more windows.  We will force some window to use a different 
	   pixels interpretation so we can steal that window ID.  There must
	   be at least three non-GUARDED WIDs.  The intent is to use them in
	   the following manner: 
	     . One shared WID displaying buffer A, 
	     . One shared WID displaying buffer B, 
	     . One unshared WID to for the requestor to steal (we will put
	        the former owner of the unshared WID onto one
	        of the shared WID lists). 

	     If there is no unshared entry, we are broken.
	     . at least 2 WIDS on the shared list,
	     . at least 1 WID on each list.

	   If there is a WID on the shared list we will pilfer it and  
	   force the oldest UNSHARED WID user to find a PI.

	   Otherwise, we combine the two oldest shared WID entries and 
	   force them to the same PI!  

         ********************************************************************* 
         *********************************************************************/

	rc_in = really_steal_WID (pGD, pWG) ;  
	
	if ( rc_in == MID_WID_ASSIGNED )	/* if WID was available */
	{
		wid	= midWG -> wid ;	/* get the new WID */
		BUGLPR(dbg_get_WID, BUGNTA, ("STOLEN WID is %X.\n", wid));

		/*---------------------------------------------------
		     Move our new WID entry to the top of the list.    
 	             First remove from its current position on the unshared
 	             list. 
	        ----------------------------------------------------*/
		entry = & (ddf -> mid_wid_data.mid_wid_entry[wid]) ;
		MID_DELETE_WID_ENTRY (entry) ;


        	/*-------------------------------------------------------------
 	    	Now put the entry on top of the proper list.   So first we 
 	    	must figure out which list we go on.  We also load a state
 	    	value corresponding to this same list. 
	 	*------------------------------------------------------------*/

		switch (request) 
		{ 
	    	case  MID_GET_UNSHARED_WID:      
	        	top = &(wid_data -> unshared_list.Top) ;
	        	new_state = MID_WID_UNSHARED ;	/* set the state */ 
	        	break ;

	    	case  MID_GET_GUARDED_WID:      
	        	top = &(wid_data -> guarded_list.Top) ;
	        	new_state = MID_WID_GUARDED ;	/* set the state */ 
	        	ddf->mid_guarded_WID_count++;
	        	break ;

	    	default:      				/* should not happen */
	        	break ;
		} 

		MID_ADD_WID_AFTER (top, entry) ;
	
        	/*-------------------------------------------------------------
 	    	    and re-init this to an unshared or guarded WID entry:
	 	*------------------------------------------------------------*/
		entry-> state = new_state ;		/* set the state */ 


		/*---------------------------------------------------
		     Color palette association checked at exit point
		     Set flag to write WID planes, too.  
		  *--------------------------------------------------*/

                do_WID_planes = _YES ;  

		rc_out = MID_NEW_WID  ;
		break  ;
	}

}



        /********************************************************************* 
         ********************************************************************* 

	   END OF THE OUTER WHILE GROUP  --  Re-enable interrupts 

         ********************************************************************* 
         *********************************************************************/

	i_enable (saved_intr_priority) ; 


        /********************************************************************* 

	   Now fix the WID planes and the pixel interpretation for the
           winner (requesting WG).  Note that the losers' WID planes 
           have already been written (and they get stuck with the resulting
           pixel interpretation). 

         *********************************************************************/

        /*---------------------------------------------------------
           Fix the pixel interpretation for the winner. 
         --------------------------------------------------------*/


	if (do_color == _YES )  
	{ 
		color = ((midWG_t *)(pWG->pPriv)) -> pi.pieces.color ;  
		FIX_COLOR_PALETTE (ddf, wid, color, midWG-> pi.pieces.flags);
	} 

           

        /*---------------------------------------------------------
           Write the WID planes for the invoking window
         --------------------------------------------------------*/

	if (do_WID_planes == _YES )  
	{ 
        	WRITE_WID_PLANES (pGD, midWG -> pWG,
        			  wid, 
               		          midWG -> pRegion,  
               		          MID_UPPER_LEFT_ORIGIN ) ; 
	} 



        /********************************************************************* 
           Trace at the exit point 
         *********************************************************************/

        MID_DEBUG_WID_LIST (get_WID, 1, ddf) ; 

    	MID_DD_EXIT_TRACE (get_WID, 1, GET_WID_RENDER, ddf, 
				pRCX, pWG, MID_DDT_WID(midWG->wid), 
					   MID_DDT_PI(midWG) | 0xF0 );

	BUGLPR(dbg_get_WID, BUGNFO,
		("Leaving get_WID ======== WID = %X ========\n\n", midWG->wid));
	return (rc_out) ;

} /* get_WID */

#include "really_steal_WID.c"

