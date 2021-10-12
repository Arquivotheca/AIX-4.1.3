static char sccsid[] = "@(#)78  1.5.1.3  src/bos/kernext/disp/ped/rcm/steal_WID.c, peddd, bos411, 9428A410j 11/3/93 11:57:33";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: steal_WID
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
MID_MODULE (steal_WID);
#define dbg_midwid  dbg_steal_WID

extern long mid_delete_WID  (
                              ulong       ,
                              midddf_t   *,
                              rcmWG      * ) ;




/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:   steal_WID            
  
   Descriptive Name:   Get a WID from a window by forcing it to share
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      

     The function of this routine is to "steal" a WID from one of the   
     windows on the unshared list.  We can steal the window's WID only if:
      . the window is not currently rendering, and
      . we can preserve pixel interpretation. 
     
     The implementation of this task is split into two parts or sections.
     The first section attempts to match any of the windows on the unshared
     list with (the pixel interpretations) of those windows on the shared            appear (or are presented) on the screen.  For PED, this simply means
     list.  As usual, the loops are written to search the lists starting 
     from the bottom.  The lists are kept (mostly) in a "most recently used"  
     order.  This is all part of the design point to keep unshared WIDs for   
     the most actively rendering windows. 

     The second section attempts to find two windows from the unshared
     list with the same pixel interpretation.  In this event, the two  
     windows are combined and moved to the shared list.  This case moves 
     two windows off the unshared list while the first case moves only one
     window the unshared list.  (In both, cases, the new window for which
     we are stealing the WID will be put "back" onto the unshared list.)
     Since it is more desireable to keep the unshared list as long as 
     possible, the first section is done first. 


 *--------------------------------------------------------------------------* 
  
    Restrictions:
        none 
  
    Dependencies:
      
  
 *--------------------------------------------------------------------------* 
  
    Linkage:

	 Called internally from the Ped device driver routines:

	  . get_WID_for_render
	  . get_WID_for_PI
	
  
 *--------------------------------------------------------------------------* 
  
    INPUT:
          
    OUTPUT: 
          
    RETURN CODES:         
          
          
          
                                 End of Prologue                                
 ******************************************************************************/


long steal_WID (
		gscDev    *pGD,		/* ptr to device depend data */
		rcmWG     *pWG )  	/* ptr to DI win geom struct */
{
midddf_t  *ddf ;         		/* ptr to device depend data */
mid_wid_data_t  	*wid_data ;	/* ptr to WID list */

mid_wid_status_t 	*unshared_WID_entry,	/* entry in unshared list */
			*other_unshared_WID_entry,/* entry in unshared list */
			*shared_WID_entry,	/* entry in shared list */
			*unshared_WID_last,	/* loop end (unshared list) */
			*shared_WID_last;	/* loop end (shared list) */

midWG_t  	*midWG,  		/* dev dependepent WGs */
         	*loser_midWG ;		/* dev dependepent WGs */

mid_wid_t	wid, new_wid ;
ulong    	unshared_PI ;			/* PI to match */
ushort 		color ;

int        rc ;				/* temp copy of return code */



	/*----------------------------------------------------------------
          Trace the input parameters     
	 *---------------------------------------------------------------*/

    	ddf = (midddf_t *) (pGD -> devHead.display -> free_area) ;



	/*-------------------------------------------------------------
          First, a little error checking.
	 *---------------------------------------------------------------*/

        BUGLPR(dbg_midwid, BUGNTA, ("Entering steal_WID. \n"));
	BUGLPR(dbg_midwid, BUGNTA, ("pGD=0x%8X, pWG=0x%8X \n", pGD , pWG ));

	if (pGD == NULL || pWG == NULL)
	{
		BUGLPR(dbg_midwid, 0, ("One of these is zero.\n" ));
		BUGLPR(dbg_midwid, 0, ("EXIT with no activity.\n\n"));

		return(MID_ERROR);
	}

		


        /********************************************************************* 
	  
	  INITIALIZATION:

	      Init pointers to the ddf and the WID list. 

         *******************************************************************/
	
	wid_data = &(ddf -> mid_wid_data) ;

	midWG = (midWG_t *) (pWG -> pPriv) ;

    	MID_DD_ENTRY_TRACE (steal_WID, 2, STEAL_WID, ddf, pWG->wg.pClip, 
                        pWG, MID_DDT_WID(midWG->wid), MID_DDT_PI(midWG));



	BUGLPR(dbg_midwid, BUGNTA, ("ddf = 0x%8X, midWG = 0x%8X \n",ddf,midWG));


    	MID_DEBUG_WID_LIST (steal_WID, 2, ddf) ; 




       

        /********************************************************************* 

	   LOOP SETUP: 

	   The top guy on the unshared list is no longer sacred. 
	   One algorithm guarantees us (we hope) that we have at least 
	   two unGUARDED WIDs. 

	   This leaves several cases: 
	    . two (or more) UNSHARED WIDs
	    . two (or more) SHARED WIDs 
	    . at least one of each (one of each is probably the steady state
		for systems with many DWA contexts running).

 	   This routine cannot do anything if there are no entries on the
	   unshared list.  (It thought that this cannot happen.)

	   We will be looping through each of the WIDs on the unshared list. 
           So init the appropriate loop controls.  As usual, we traverse the
           list in reverse order. 

         *********************************************************************/


        unshared_WID_entry =  wid_data -> unshared_list.Bottom.pPrev ; 
	unshared_WID_last = &(wid_data->unshared_list.Top) ;
	BUGLPR(dbg_midwid, BUGNTA, 
		("top entry on unshared = 0x%8X, next = 0x%8X \n", 
			unshared_WID_last, unshared_WID_last -> pNext ));
	BUGLPR(dbg_midwid, BUGNTA, 
		("bottom entry on unshared = 0x%8X\n", unshared_WID_entry ));

	if (unshared_WID_entry == unshared_WID_last) 
	{ 
        	/************************************************************* 
           	    Trace at the exit point 
         	*************************************************************/

    		MID_DEBUG_WID_LIST (steal_WID, 4, ddf) ; 

        	MID_DD_EXIT_TRACE (steal_WID, 1, STEAL_WID_2, ddf,  pWG,
					unshared_WID_last, 
					unshared_WID_last -> pNext, 
        				unshared_WID_entry 
					);

		BUGLPR(dbg_steal_WID, BUGNFO,
			("EXIT steal_WID: INSUFFICIENT UNSHARED entries\n"));

        	return (MID_WID_NOT_AVAILABLE) ;

	}  /* No entries in unshared WID list */



	BUGLPR(dbg_midwid, 2, ("unshared list has at least ONE entrs\n"));


        /********************************************************************* 

	                        TOP OF OUTER LOOP 

         *********************************************************************/


        while (unshared_WID_entry != unshared_WID_last)
        { 
	    BUGLPR(dbg_midwid, BUGNTA, 
		("\n                ----- TOP of OUTER steal loop ----- \n")) ;




        /********************************************************************* 

	   PART 1:  Try to match an UNSHARED PI with a SHARED PI

	   OK, we've got a WID entry from the unshared list.  Now try to
	   match its pixel interpretation with (the PIs of) each of the 
           windows on the SHARED list.  Again we start from the bottom of      
           the shared list in our never-ending attempt to be fair. 

         *********************************************************************/

            unshared_PI = unshared_WID_entry -> pi.PI ; 
	    BUGLPR(dbg_midwid, 2, ("unshared entry to match 0x%X, PI %X\n",
			unshared_WID_entry, unshared_PI )) ; 

            shared_WID_last = &(wid_data -> shared_list.Top) ; 
            shared_WID_entry =  wid_data -> shared_list.Bottom.pPrev ; 

	    BUGLPR(dbg_midwid, 2, ("shared list top 0x%8X, bottom 0x%8X\n",
			shared_WID_last, shared_WID_entry )) ; 


            while (shared_WID_entry!= shared_WID_last)
            { 
	    	BUGLPR(dbg_midwid, BUGNTA, ("-- TOP of inner steal loop --\n"));
	    	BUGLPR(dbg_midwid, BUGNTA, ("shared entry 0x%X,  PI %X\n",
			shared_WID_entry, shared_WID_entry -> pi.PI )) ; 

                if ( unshared_PI == shared_WID_entry -> pi.PI )
                { 
	    	BUGLPR(dbg_midwid, 2, ("shared PI MATCHES unshared !!\n"));


                    /****************************************************** 

                       MATCHING PI FOUND (on the shared list)              

                       Save it, and update the WG chain(s). 

                     ******************************************************/

	    	    BUGLPR(dbg_midwid, BUGNTA, ("FOUND WID to share = %X\n", 
					shared_WID_entry -> mid_wid));

#ifdef  	    MID_DEBUG_STEAL_WID
		    mid_print_WG_chain (shared_WID_entry) ; /* print WG chain*/
#endif  

                    /*---------------------------------------------------------
                       Get the loser's WG address.  Then update the found
                       WIDs WG chain and the receipient's WG wid.

                       Note that we leave this window in the same position
                       in the UNSHARED list. 
                     --------------------------------------------------------*/
                    wid = unshared_WID_entry -> mid_wid ;
                     
                    loser_midWG = unshared_WID_entry -> pwidWG ;
                    loser_midWG -> wid = shared_WID_entry -> mid_wid ; 

         	    /*---------------------------------------------------------
           	        Now increment the shared WID's use count 
         	     --------------------------------------------------------*/
        	    shared_WID_entry -> use_count += 1 ;


                    unshared_WID_entry -> pwidWG = midWG ;
                    midWG -> pPrev = NULL ; 
                    midWG -> pNext = NULL ; 
                    midWG -> wid = wid ;


                    /*---------------------------------------------------------
                       Put the window losing the WID onto the shared WG chain.
                     --------------------------------------------------------*/
                    loser_midWG -> pPrev = NULL ;
                    loser_midWG -> pNext = shared_WID_entry -> pwidWG ;

                    loser_midWG -> pNext -> pPrev = loser_midWG ;
                    shared_WID_entry -> pwidWG = loser_midWG ;


#ifdef  MID_DEBUG_STEAL_WID
	midl_print (ddf, "all") ;         /* Write the WID list */ 
	mid_print_WG_chain (shared_WID_entry) ; /* WG chain */
#if  0 
	mid_print_WG (pWG) ;         		/* Print the winner's WG */ 
	mid_print_WG (loser_midWG -> pWG) ; 	/* Print the loser's WG */ 
	mid_print_WG (loser_midWG -> pNext-> pWG) ; 	/* Print next WG */ 
#endif  
#endif  /* MID_DEBUG_STEAL_WID */


                    goto GOT_WID ;
                }  /* end of found matching PI */ 

                shared_WID_entry = shared_WID_entry -> pPrev ; 
            }  /* end of shared list loop */






        /********************************************************************* 

	   PART 2:  Try to match an UNSHARED PI with another UNSHARED PI

	   OK, the current window (from the shared list) does not match PIs
	   with any of the SHARED windows.  So now we'll try to match PIs
	   with the other windows on the SHARED list. 
           As usual we search from the bottom to the top. 

           This section of code requires there be at least 3 unshared WIDs
           for the match to be successful.  We will not touch the first WID
           as this is assumed to be the current renderer and there must be
           two other WIDs to combine.  We have already ensured that there
           are at least two WIDs on the unshared list.  The leading check
           on the other loop will cause us to immediately drop out if there
           is no third entry. 

         *********************************************************************/

            other_unshared_WID_entry = unshared_WID_entry -> pPrev ; 

            while (other_unshared_WID_entry != unshared_WID_last)
            { 
	    	BUGLPR(dbg_midwid, BUGNTA, 
			("----- TOP of 2nd inner steal loop -----\n" ));
	    	BUGLPR(dbg_midwid, BUGNTA,("other unshared ntry 0x%8X, PI %X\n",
		   other_unshared_WID_entry, other_unshared_WID_entry->pi.PI));

                if ( unshared_PI == other_unshared_WID_entry -> pi.PI )
                { 
	    	    BUGLPR(dbg_midwid, BUGNTA, ("unshared PI MATCHES !!\n" )) ;
                    /********************************************************** 

                       MATCHING PI FOUND (on the unshared list)              

                       We will take the (original) unshared WID and use that
                       as the WID for the new window.  The other unshared WID
                       will be used as the WID for the two previously unshared
                       entrys, now being combine.  This whole process will be
                       done in the following order: 
                        . save the WID and loser's WG. 
                        . update the WG chain for the stolen WID entry,   
                        . update the WG chain for the other WID entry,   
                        . delete the "other" WID entry from the UNSHARED list,
                        . add it to the (top of) the SHARED list,
                    **********************************************************/


                    /*---------------------------------------------------------
                       Save WID & WG of the window whose WID is being stolen. 
                       Then update the WG chain on the stolen WID entry.   
                     --------------------------------------------------------*/
                    wid = unshared_WID_entry -> mid_wid ;
	    	    BUGLPR(dbg_midwid, BUGNTA, ("WID found = %X\n" , wid )) ;
	    	    BUGLPR(dbg_midwid, BUGNTA, 
	       	      ("WID shared = %X\n", other_unshared_WID_entry->mid_wid));

                    loser_midWG = unshared_WID_entry -> pwidWG ;
                    loser_midWG -> wid = other_unshared_WID_entry -> mid_wid ; 

                    unshared_WID_entry -> pwidWG = midWG ;
                    midWG -> pPrev = NULL ; 
                    midWG -> pNext = NULL ; 
                    midWG -> wid = wid ;


                    /*---------------------------------------------------------
                       Combine the two previously unshared windows.  
                     --------------------------------------------------------*/
                    other_unshared_WID_entry -> pwidWG -> pNext = loser_midWG ;
                    loser_midWG -> pPrev = other_unshared_WID_entry -> pwidWG ;



                    /*---------------------------------------------------------
                       Delete this entry from the UNSHARED LIST.     
                     --------------------------------------------------------*/
                    MID_DELETE_WID_ENTRY (other_unshared_WID_entry) ; 


                    /*---------------------------------------------------------
                       And add it to the SHARED LIST.     

                       NOTE that shared_WID_last still points to the
                            top of the shared_WID_list. 
                     --------------------------------------------------------*/
                    MID_ADD_WID_AFTER(shared_WID_last,other_unshared_WID_entry); 

                    /*---------------------------------------------------------
                       Update the WID entry state (to SHARED) 
                     --------------------------------------------------------*/
                    other_unshared_WID_entry -> state = MID_WID_SHARED ;
                    other_unshared_WID_entry -> use_count = 2 ;




#ifdef  MID_DEBUG_STEAL_WID
		midl_print (ddf, "all") ;         	/* Write the WID list */
		mid_print_WG_chain (other_unshared_WID_entry) ; /* WG chain */
#if  0  
		mid_print_WG (pWG) ;   			/* Print winner's WG */ 
		mid_print_WG (loser_midWG -> pPrev -> pWG) ; 	/* Print the */
		mid_print_WG (loser_midWG -> pWG) ; 		/* WG chain */
#endif 
#endif  /* MID_DEBUG_STEAL_WID */



                    goto GOT_WID ;
                } /* end of found matching PI */ 

                other_unshared_WID_entry = other_unshared_WID_entry -> pPrev ; 
            }  /* end of inner unshared loop */ 



            unshared_WID_entry = unshared_WID_entry -> pPrev ; 
        }   /* end of outer loop */


    /********************************************************************* 
       Trace at the exit point 
     *********************************************************************/


    MID_DD_EXIT_TRACE (steal_WID, 1, STEAL_WID, ddf, 0xFFFF,
                        pWG, MID_DDT_WID(midWG->wid), MID_DDT_PI(midWG));

    BUGLPR(dbg_midwid, BUGNFO, ("No WID STOLEN - returning \n" )) ;
    return (MID_WID_NOT_AVAILABLE) ;





GOT_WID : ;
        /********************************************************************* 

	   We get here if we have successfully found a WID.  This goto   
           mechanism provides an easy method of exitting the nested loops.

         *********************************************************************/

        /*---------------------------------------------------------
           Now write the WID planes for the loser 
         --------------------------------------------------------*/

        WRITE_WID_PLANES (pGD, loser_midWG -> pWG,
                    	  loser_midWG -> wid , 
                          loser_midWG-> pWG-> wg.pClip , 
                          MID_UPPER_LEFT_ORIGIN ) ; 

#if 0
        /*---------------------------------------------------------
           Fix the color association for the winner. 
         --------------------------------------------------------*/

        /* unshared_WID_entry -> pi.pieces.flags =
		     midWG -> pi.pieces.flags ; */

	color = midWG -> pi.pieces.color ;  

	fix_color_palette (ddf, wid, color) ; 
           
#endif

           
        /*---------------------------------------------------------
           and finally, write the WID planes for the winner 
         --------------------------------------------------------*/

        WRITE_WID_PLANES (pGD, midWG->pWG,
                          wid,
                          midWG->pRegion , 
                          MID_UPPER_LEFT_ORIGIN ) ; 

           
        /********************************************************************* 
           Trace at the exit point 
         *********************************************************************/
	BUGLPR(dbg_steal_WID, BUGNFO,
		("Leaving steal_WID ===== WID = %X =====\n\n", midWG->wid));

    	MID_DEBUG_WID_LIST (steal_WID, 1, ddf) ; 

    	MID_DD_EXIT_TRACE (steal_WID, 1, STEAL_WID, ddf, pGD, 
		pWG, MID_DDT_WID(midWG->wid), MID_DDT_PI(midWG) | 0xF0);

        return (MID_WID_ASSIGNED) ; 
}
