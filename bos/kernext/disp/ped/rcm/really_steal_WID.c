/* @(#)43  1.4  src/bos/kernext/disp/ped/rcm/really_steal_WID.c, peddd, bos41J 6/7/95 15:27:30 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: really_steal_WID
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
MID_MODULE (really_steal_WID);
#define dbg_midwid  dbg_really_steal_WID



/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:   really_steal_WID            
  
   Descriptive Name:   Get a WID from a window by forcing it to share
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      

     The function of this routine is to "steal" a WID from one of the   
     windows on the unshared list.  We can steal the window's WID only if:
      . the window is not currently rendering, and
      . we can preserve the display buffer.   
     
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
	
  
 *--------------------------------------------------------------------------* 
  
    INPUT:
          
    OUTPUT: 
          
    RETURN CODES:         
          
          
          
                                 End of Prologue                                
 ******************************************************************************/


long really_steal_WID (
		gscDev    *pGD,		/* ptr to device depend data */
		rcmWG     *pWG )  	/* ptr to DI win geom struct */
{
midddf_t  *ddf ;         		/* ptr to device depend data */
mid_wid_data_t  	*wid_data ;	/* ptr to WID list */

mid_wid_status_t 	*loser ; 
mid_wid_status_t 	*unshared_WID_entry; 
mid_wid_status_t	*test_unshared_WID_entry;
mid_wid_status_t	*unshared_loop_end ;

mid_wid_status_t	*shared_WID_entry;	/* entry in shared list */
mid_wid_status_t	*shared_loop_end ;


midWG_t  	*midWG;  		/* dev dependepent WGs */
midWG_t  	*last_WG;  		/* dev dependepent WGs */
midWG_t  	*loser_midWG ;		/* dev dependepent WGs */

ushort    	unshared_FB ;			/* FB to match */
ushort    	shared_FB ;			/* FB to match */
ushort    	test_FB ;			/* FB to match */

mid_wid_t	wid, new_wid ;

int        found ;				/* temp copy of return code */
#define	_NO 	0
#define	_YES	1



	/*----------------------------------------------------------------
          Trace the input parameters     
	 *---------------------------------------------------------------*/

    	ddf = (midddf_t *) (pGD -> devHead.display -> free_area) ;



	/*-------------------------------------------------------------
          First, a little error checking.
	 *---------------------------------------------------------------*/

        BUGLPR(dbg_midwid, BUGNTA, ("Entering really_steal_WID. \n"));
	BUGLPR(dbg_midwid, BUGNTA, ("pGD=0x%8X, pWG=0x%8X \n", pGD , pWG ));

	if (pGD == NULL || pWG == NULL)
	{
		BUGLPR(dbg_midwid, 0, ("One of these is zero.\n" ));
		BUGLPR(dbg_midwid, 0, ("EXIT with no activity.\n\n"));

		return(MID_ERROR);
	}

		


        /********************************************************************* 
	  
	  INITIALIZATION:

	      Init pointers to the 
		. the ddf,
		. the WID list,
		. the last unshared WID entry and
		. the last shared WID entry. 

         *******************************************************************/
	
	wid_data = &(ddf -> mid_wid_data) ;

	midWG = (midWG_t *) (pWG -> pPriv) ;

        unshared_WID_entry =  wid_data->unshared_list.Bottom.pPrev ;

    	MID_DD_ENTRY_TRACE (really_steal_WID, 2, REALLY_STEAL_WID, ddf, ddf,
		   pWG, MID_DDT_WID(midWG->wid), MID_DDT_PI(midWG));


        MID_ASSERT( (unshared_WID_entry != &(wid_data->unshared_list.Top)),
                        GET_WID_RENDER, ddf, pWG, midWG, 0, 0xDDDD0500) ;

	BUGLPR(dbg_midwid, 1, ("ddf = 0x%8X, midWG = 0x%8X \n", ddf, midWG));
    	MID_DEBUG_WID_LIST (really_steal_WID, 2, ddf) ; 


       

	found = _NO ; 

        unshared_loop_end = &(wid_data->unshared_list.Top) ;
        while ( (unshared_WID_entry != unshared_loop_end) && (found != _YES) )
        { 
            /***************************************************************** 
	                        TOP OF OUTER LOOP 
             *****************************************************************/

	    BUGLPR(dbg_midwid, 2, ("\n---- TOP of OUTER steal loop ----\n")) ;



            /***************************************************************** 

	       PART 1:  Try to match an UNSHARED FB with a SHARED FB

	       OK, we've got a WID entry from the unshared list.  Now try to
	       match its displayed frame buffer with (the FB of) each of the 
               WIDs on the SHARED list.  Again we start from the bottom of
               the shared list in our never-ending attempt to be fair. 

             *****************************************************************/

            unshared_FB = unshared_WID_entry->pi.pieces.flags ; 
	    BUGLPR(dbg_midwid, 2, ("unshared entry to match 0x%X, FB %X\n",
			unshared_WID_entry, unshared_FB )) ; 

            shared_loop_end = &(wid_data-> shared_list.Top) ; 
            shared_WID_entry =  wid_data-> shared_list.Bottom.pPrev ; 

	    BUGLPR(dbg_midwid, 2, ("shared list top 0x%8X, bottom 0x%8X\n",
			shared_loop_end, shared_WID_entry )) ; 


            while (shared_WID_entry != shared_loop_end)
            { 
	    	BUGLPR(dbg_midwid, BUGNTA, ("-- TOP of inner steal loop --\n"));
	    	BUGLPR(dbg_midwid, BUGNTA, ("shared entry 0x%X,  FB %X\n",
			shared_WID_entry, shared_WID_entry-> pi.pieces.flags));

                if (unshared_FB == shared_WID_entry->pi.pieces.flags)
                { 
	    	    BUGLPR(dbg_midwid, 2, ("shared PI MATCHES unshared !!\n"));


                    /****************************************************** 
                       MATCHING FB FOUND (on the shared list)              

                       Save it, and update the WG chain(s). 
                     ******************************************************/

#ifdef  	    MID_DEBUG_STEAL_WID
		    mid_print_WG_chain (shared_WID_entry) ; /* print WG chain*/
#endif  

                    /*---------------------------------------------------------
                       Get the loser's WG address.  Then update the found
                       WIDs WG chain and the receipient's WG wid.

                       Note that we leave this window in the same position
                       in the UNSHARED list. 
                     --------------------------------------------------------*/
                    wid = unshared_WID_entry->mid_wid ;
                     
                    loser_midWG = unshared_WID_entry->pwidWG ;
                    loser_midWG->wid = shared_WID_entry->mid_wid ; 

         	    /*---------------------------------------------------------
           	        Now increment the shared WID's use count 
         	     --------------------------------------------------------*/
        	    shared_WID_entry->use_count += 1 ;


         	    /*---------------------------------------------------------
           	        Put winner's info into the stolen unshared WID entry
         	     --------------------------------------------------------*/
                    unshared_WID_entry->pwidWG = midWG ;
                    midWG->pPrev = NULL ; 
                    midWG->pNext = NULL ; 
                    midWG->wid = wid ;


                    /*---------------------------------------------------------
                       Put the window losing the WID onto the shared WG chain.
                     --------------------------------------------------------*/
                    loser_midWG->pPrev = NULL ;
                    loser_midWG->pNext = shared_WID_entry-> pwidWG ;

                    loser_midWG->pNext->pPrev = loser_midWG ;
                    shared_WID_entry-> pwidWG = loser_midWG ;


#ifdef  MID_DEBUG_STEAL_WID
	midl_print (ddf, "all") ;         /* Write the WID list */ 
	mid_print_WG_chain (shared_WID_entry) ; /* WG chain */
#if  0 
	mid_print_WG (pWG) ;         		/* Print the winner's WG */ 
	mid_print_WG (loser_midWG -> pWG) ; 	/* Print the loser's WG */ 
	mid_print_WG (loser_midWG -> pNext-> pWG) ; 	/* Print next WG */ 
#endif  
#endif  /* MID_DEBUG_STEAL_WID */


#if 0
                    goto GOT_WID ;
#endif
                    found = _YES ;
                    break ; 
                }  /* end of found matching PI */ 

                shared_WID_entry = shared_WID_entry-> pPrev ; 
            }  /* end of shared list loop */






        /********************************************************************* 

	   PART 2:  Try to match an UNSHARED FB with another UNSHARED FB

	   OK, the current window (from the unshared list) does not match FB
	   with any of the SHARED windows.  So now we'll try to match PIs
	   with the other windows on the UNSHARED list. 
           As usual we search from the bottom to the top. 

           This section of code requires there be at least 3 unshared WIDs
           for the match to be successful.  We will not touch the first WID
           as this is assumed to be the current renderer and there must be
           two other WIDs to combine.  We have already ensured that there
           are at least two WIDs on the unshared list.  The leading check
           on the other loop will cause us to immediately drop out if there
           is no third entry. 

         *********************************************************************/

            test_unshared_WID_entry = unshared_WID_entry-> pPrev ; 

            while ( (test_unshared_WID_entry != unshared_loop_end) && 
							(found != _YES) )
            { 
	    	BUGLPR(dbg_midwid, BUGNTA, 
			("----- TOP of 2nd inner steal loop -----\n" ));
	    	BUGLPR(dbg_midwid, BUGNTA,("test unshared ntry 0x%8X, PI %X\n",
		   test_unshared_WID_entry, test_unshared_WID_entry->pi.PI));

                if (unshared_FB == test_unshared_WID_entry-> pi.pieces.flags)
                { 
	    	    BUGLPR(dbg_midwid, BUGNTA, ("unshared FB MATCHES !!\n" )) ;
                    /********************************************************** 

                       MATCHING FB FOUND (on the unshared list)              

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
                    wid = unshared_WID_entry-> mid_wid ;
	    	    BUGLPR(dbg_midwid, BUGNTA, ("WID found = %X\n" , wid )) ;
	    	    BUGLPR(dbg_midwid, BUGNTA, 
	       	      ("WID shared = %X\n",  test_unshared_WID_entry->mid_wid));

                    loser_midWG = unshared_WID_entry -> pwidWG ;
                    loser_midWG-> wid = test_unshared_WID_entry-> mid_wid ; 

                    unshared_WID_entry-> pwidWG = midWG ;
                    midWG-> pPrev = NULL ; 
                    midWG-> pNext = NULL ; 
                    midWG-> wid = wid ;


                    /*---------------------------------------------------------
                       Combine the two previously unshared windows.  
                     --------------------------------------------------------*/
                    test_unshared_WID_entry-> pwidWG-> pNext = loser_midWG ;
                    loser_midWG-> pPrev = test_unshared_WID_entry-> pwidWG ;



                    /*---------------------------------------------------------
                       Delete this entry from the UNSHARED LIST.     
                     --------------------------------------------------------*/
                    MID_DELETE_WID_ENTRY (test_unshared_WID_entry) ; 


                    /*---------------------------------------------------------
                       And add it to the SHARED LIST.     

                       NOTE that shared_WID_last still points to the
                            top of the shared_WID_list. 
                     --------------------------------------------------------*/
                    MID_ADD_WID_AFTER(shared_loop_end, test_unshared_WID_entry); 

                    /*---------------------------------------------------------
                       Update the WID entry state (to SHARED) 
                     --------------------------------------------------------*/
                    test_unshared_WID_entry-> state = MID_WID_SHARED ;
                    test_unshared_WID_entry-> use_count = 2 ;



#ifdef  MID_DEBUG_STEAL_WID
		midl_print (ddf, "all") ;         	/* Write the WID list */
		mid_print_WG_chain (test_unshared_WID_entry) ; /* WG chain */
#if  0  
		mid_print_WG (pWG) ;   			/* Print winner's WG */ 
		mid_print_WG (loser_midWG -> pPrev -> pWG) ; 	/* Print the */
		mid_print_WG (loser_midWG -> pWG) ; 		/* WG chain */
#endif 
#endif  /* MID_DEBUG_STEAL_WID */

#if 0
                    goto GOT_WID ;
#endif
                    found = _YES ;
                    break ; 
                } /* end of found matching PI */ 

                test_unshared_WID_entry = test_unshared_WID_entry-> pPrev ; 
            }  /* end of inner unshared loop */ 


            unshared_WID_entry = unshared_WID_entry-> pPrev ; 
        }   /* end of outer loop */




        if ( !(found) )
	{
        /********************************************************************* 

	   There were no matches in the unshared entries.  We must have
	   multiple shared entries, displaying the same frame buffer. 
	   Combine them. 

         *********************************************************************/
		mid_wid_status_t 	*shared_WID_entry ;
		mid_wid_status_t 	*test_entry ;
		

		BUGLPR(dbg_midwid, 1, ("unshared list empty \n" ));
        	/*-----------------------------------------------------------* 

		   Now set up a loop to combine shared entries displaying the
		   same frame buffer. 

		   Note there must be at least three entries on the shared list

         	*-----------------------------------------------------------*/
        	shared_WID_entry = wid_data-> shared_list.Bottom.pPrev ;

        	MID_ASSERT ((shared_WID_entry != &(wid_data->shared_list.Top)),
                        REALLY_STEAL_WID, ddf, pWG, midWG, 0, 0xEEEE0501) ;

        	MID_ASSERT ((test_entry != &(wid_data->shared_list.Top)),
                        REALLY_STEAL_WID, ddf, pWG, midWG, 0, 0xEEEE0502) ;

		found = _NO ; 
		while (found != _YES)
		{ 
        	    test_entry = shared_WID_entry->pPrev ;
        	    MID_ASSERT((test_entry != &(wid_data->shared_list.Top)),
                        REALLY_STEAL_WID, ddf, pWG, midWG, 0, 0xEEEE0502) ;

		    shared_FB = shared_WID_entry->pi.pieces.flags ;

		    while (test_entry !=  &(wid_data->shared_list.Top) )
		    { 
		 	if (test_entry->pi.pieces.flags == shared_FB)
			{ 
        		    /************************************* 
           	    		  	Match found 
         		     **************************************/
			    found = _YES ; 



            		    loser =  test_entry ;
            		    loser_midWG = loser-> pwidWG ;
            		    wid = loser-> mid_wid ;

            		    /*---------------------------------------------
            		       Combine the WG chains of the two shared entries.

            		       	First we must walk down the WG chain of one 
				shared entry, to find the end.  Then we attach 
				the WG chain of the other entry at this point.
             		    --------------------------------------------*/

            		    last_WG = shared_WID_entry -> pwidWG ;
            		    while ( last_WG->pNext != NULL )
            		    {
                		    last_WG = last_WG->pNext ;
            		    }

            		    last_WG->pNext = loser_midWG ;
            		    loser_midWG->pPrev = last_WG  ;


            		    /*---------------------------------------------
            		       Combine the use count(s)
             		    --------------------------------------------*/
            		    shared_WID_entry-> use_count += loser-> use_count ;


            		    /*---------------------------------------------
               		       Now write the WID planes of all the windows in 
				the WG chain that were just moved.
		    
               		    	Since the midWG pointer is already set up, it 
			    	is not re-initialized.
              		    --------------------------------------------*/

            		    for  ( /* loser_midWG = loser -> pwidWG */ ;
                   		    loser_midWG != NULL ;
                   		    loser_midWG = loser_midWG -> pNext )
            		    {
				    loser_midWG->wid = shared_WID_entry-> mid_wid;
                		    WRITE_WID_PLANES (pGD, loser_midWG-> pWG,
                                  		    shared_WID_entry-> mid_wid,
                                  		    loser_midWG->pWG->wg.pClip,
                                  		    MID_UPPER_LEFT_ORIGIN ) ;
            		    }

         	    	    /*---------------------------------------------------
           	              Put winner's info into the stolen shared WID entry
         	     	    ---------------------------------------------------*/
                    	    test_entry->pwidWG = midWG ;
                    	    midWG->pPrev = NULL ; 
                    	    midWG->pNext = NULL ; 
                    	    midWG->wid = wid ;

			    break ; 
			 } 
		 	else  /* match not found yet */ 
			{ 
        			test_entry =  test_entry->pPrev ;
			} 
		    } 
        	    shared_WID_entry = shared_WID_entry->pPrev ;
		} 
	} 



        MID_ASSERT ( (found), REALLY_STEAL_WID, ddf, pWG, midWG, 
					0, 0xEEEE05FF) ;



GOT_WID : ;
        /********************************************************************* 

	   We get here if we have successfully found a WID.  This goto   
           mechanism provides an easy method of exitting the nested loops.

         *********************************************************************/

        /*---------------------------------------------------------
           Now write the WID planes for the loser 
         --------------------------------------------------------*/

        WRITE_WID_PLANES (pGD, loser_midWG-> pWG,
                    	  loser_midWG-> wid , 
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

        WRITE_WID_PLANES (pGD, midWG-> pWG,
                          wid,
                          midWG-> pRegion , 
                          MID_UPPER_LEFT_ORIGIN ) ; 

           
        /********************************************************************* 
           Trace at the exit point 
         *********************************************************************/
	BUGLPR(dbg_really_steal_WID, BUGNFO,
		("Leaving really_steal_WID == WID = %X ==\n\n", midWG->wid));

    	MID_DEBUG_WID_LIST (really_steal_WID, 1, ddf) ; 

    	MID_DD_EXIT_TRACE (really_steal_WID, 1, REALLY_STEAL_WID, ddf, ddf,
		   pWG, MID_DDT_WID(midWG->wid), MID_DDT_PI(midWG) | 0xF0);

        return (MID_WID_ASSIGNED) ; 
}
