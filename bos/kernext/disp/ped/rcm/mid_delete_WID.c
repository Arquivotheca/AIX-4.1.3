static char sccsid[] = "@(#)73	1.8  src/bos/kernext/disp/ped/rcm/mid_delete_WID.c, peddd, bos411, 9428A410j 11/3/93 11:57:23";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_delete_WID
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
MID_MODULE (delete_WID);
#define 	dbg_midwid    dbg_delete_WID   



/***************************************************************************** 
                                 Externals
 *****************************************************************************/
extern long mid_delete_WID  (
                              ulong       ,
                              midddf_t   *,
                              rcmWG      * ) ;



/***************************************************************************** 
                              Debug Variables
 *****************************************************************************/
#if  0  
#define  MID_DEBUG_DELETE_WID
#endif  



/****************************************************************************** 
                              
                             Start of Prologue  
  
   Function Name:    mid_delete_WID 
  
   Descriptive Name:  Delete (this window's) Use of the Passes Window ID
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
  
     
     This routine checks the passed window ID and does the appropriate   
     processing.  Note that the window ID is actually passed in (as part 
     of the) window geometry parameter.   A leading check is performed to
     ensure that the window (geometry) actually has a WID assigned. 

     If the ID is currently on: 
       . the UNSHARED list:  the WID is moved to the unused list.  
       . the GUARDED list:  the WID is moved to the unused list.  
       . the SHARED list:  see next paragraph 
       . the UNUSED list:  we fall through to the default case and
                            nothing is done.  This should never occur. 
     
     If the ID is currently on the SHARED list, the window (WG) is removed
     from the ID's window chain.  The window chain is then rechecked to
     see if it is either empty or consists of a single window.  
     If it is a single window, the WID is moved to the UNSHARED list.  
     If the window chain is empty, the WID is moved to the UNUSED list.
  

 *--------------------------------------------------------------------------* 
  
    Restrictions:
  
  
    Dependencies:
  
  
 *--------------------------------------------------------------------------* 
  
    Linkage:
           Called internally from the WID handling code of the Ped device
           driver.  
           Delete window geometry is a likely candidate.


 *--------------------------------------------------------------------------* 
  
    INPUT:  These are the input parameters to this function: 
              . a request code  (see midRC.h for definitions), 
                  (the request code is currently unused)          
              . a pointer to the device dependent data structure 
                  (which contains the window ID list), 
              . a pointer to the device independent window geometry structure
          
    OUTPUT:  none 
          
    RETURN CODES:  none         
         . MID_PARM_ERROR -- if either the ddf or WG pointers are NULL.
         . MID_RC_OK -- in all other cases
          
                                 End of Prologue                                
 ******************************************************************************/

long mid_delete_WID  (
                              ulong       request,
                              midddf_t   *ddf,
                              rcmWG      *pWG ) 

{

/***************************************************************************** 
   Local variables: 
 *****************************************************************************/

midWG_t             *midWG ;            /* Ped win geom extension */
mid_wid_t            current_wid ;     /* current WID being used by WG */ 
mid_wid_status_t    *wid_entry ;       /* Pointer to WG's WID wid array entry */
mid_wid_status_t    *bottom, *top ;    /* temp top and bottom entry pointers */
int                  wid_state ;       /* current state of this WG's WID */
 
int    			saved_intr_priority ;
int    			our_intr_data ;


 

    /*-------------------------------------------------------------------* 
           Gather the trace data 
    *--------------------------------------------------------------------*/

    midWG =  ((struct _midWG *) (pWG -> pPriv)) ; 
    current_wid = midWG -> wid ; 

    wid_entry = & (ddf -> mid_wid_data.mid_wid_entry[current_wid]) ;
    wid_state = wid_entry -> state  ;

    MID_DD_ENTRY_TRACE (delete_WID, 1, DELETE_WID, ddf, 
			ddf, pWG, current_wid, wid_state ) ; 

    MID_DEBUG_WID_LIST (delete_WID, 9, ddf) ; 


    BUGLPR(dbg_midwid,  BUGNFO, ("Top of mid_delete_WID \n") );  
    BUGLPR(dbg_midwid,  BUGNFO+1, 
              ("request = %d, ddf = 0x%8X, WG = 0x%8X\n", request, ddf, pWG) );

        /*-------------------------------------------------------------------* 
            First validate the parameters.
        *--------------------------------------------------------------------*/
	if ( (ddf == NULL) | (pWG == NULL) )
	{
		BUGLPR(dbg_midwid, 0, ("*** PARAMETER ERROR *** \n\n") );

    		MID_DD_EXIT_TRACE (delete_WID, 2, DELETE_WID, ddf, ddf,
					0xEEEE, pWG, ddf ); 

		return (MID_PARM_ERROR) ;
	}



    /*-------------------------------------------------------------------* 
           Get the window's ID and check if it is a valid one.  
    *--------------------------------------------------------------------*/

    if (current_wid == MID_WID_NULL)    
    {
        BUGLPR(dbg_midwid,BUGNFO,("WID is NULL -- EXIT mid_delete_WID \n") ); 

    	MID_DD_EXIT_TRACE (delete_WID, 2, DELETE_WID, ddf, 
			ddf, pWG, midWG->wid, wid_entry->state); 

        return (MID_RC_OK) ;
    }




/***************************************************************************** 
 ***************************************************************************** 
                            DISABLE INTERRUPTS 
 ***************************************************************************** 
 *****************************************************************************/
    saved_intr_priority = i_disable (our_intr_data) ;


/***************************************************************************** 

    Fetch the WID's state and branch into cases:  

      CASE 1:  WID is UNUSED  (Nothing is done) 
      CASE 2:  WID is UNSHARED 
      CASE 3:  WID is HELD (or GUARDED) 
      CASE 4:  WID is SHARED  (This is by far the trickiest case.) 

 *****************************************************************************/

    wid_entry = & (ddf -> mid_wid_data.mid_wid_entry[current_wid]) ;
    wid_state = wid_entry -> state  ;
    BUGLPR(dbg_midwid,BUGNFO,("WID = %X, state = %d\n", current_wid,wid_state));

    switch (wid_state)  
    {
        /**************************************************************** 
          case 1:  WID is UNUSED  

          nothing need be done if the WID was already unused 
         ****************************************************************/

        case MID_WID_UNUSED :    

            BUGLPR(dbg_midwid, BUGNFO+1, ("WID is UNUSED \n") );      
            
            break ;



        /**************************************************************** 
          case 2:  WID is GUARDED 
         ****************************************************************/

        case MID_WID_GUARDED :    
            BUGLPR(dbg_midwid, BUGNFO+1, ("WID is GUARDED \n") );      

            /*--------------------------------------------------* 
		Guarded WIDs are released out of intrswap ONLY after
		the swap is complete.  All we do here is clear the
		device dependent WG pointer.	
             *--------------------------------------------------*/
		wid_entry -> pwidWG = NULL;

	     break;


        /**************************************************************** 
           case 3:  WID is UNSHARED
         ****************************************************************/
	case MID_WID_UNSHARED : 
            BUGLPR(dbg_midwid, BUGNFO+1, ("WID is UNSHARED \n") );      

            /*--------------------------------------------------* 
               First remove this from the unshared list. 
             *--------------------------------------------------*/
	    MID_DELETE_WID_ENTRY (wid_entry) ; 


            /*--------------------------------------------------* 
               Now put this on the bottom of the unused list.
             *--------------------------------------------------*/
            bottom = &(ddf -> mid_wid_data.unused_list.Bottom) ;

            MID_ADD_WID_BEFORE (wid_entry, bottom) ;


            /*------------------------------------------------------* 
               and change the WID state and set the WG pointer to NULL.
               The WG pointer is used during handling of a swap buffers
               interrupt and must be set to NULL if the WG is deleted.
               (This covers that case and potentially others.) 
             *------------------------------------------------------*/
            wid_entry -> state = MID_WID_UNUSED ;   
            wid_entry -> pwidWG = NULL ;   
	    /* wid_entry -> use_count = 0 ; */

    	    MID_DEBUG_WID_LIST (delete_WID, 2, ddf) ; 

            break ;











        /**************************************************************** 
          case 4:  WID is SHARED
         ****************************************************************/

        case MID_WID_SHARED :  
            BUGLPR(dbg_midwid, BUGNFO, ("** OH NO! **   WID is SHARED \n\n") );
            BUGLPR(dbg_midwid, BUGNFO, ("use cnt= %d\n", wid_entry->use_count));

    	    MID_DEBUG_WID_LIST (delete_WID, 5, ddf) ; 

#ifdef 	    MID_DEBUG_DELETE_WID
    	    /*----------------------------------------------------------------* 
      	    	Print the window Geometry group, too.         
     	    *----------------------------------------------------------------*/
    	    mid_print_WG_chain (wid_entry) ; 

#endif 	    /* MID_DEBUG_DELETE_WID */


            /*--------------------------------------------------* 
               This is case is more involved.      

               First, we must remove the WG from the WID WG chain.
             *--------------------------------------------------*/


            if (midWG -> pPrev == NULL)  
            {
                BUGLPR(dbg_midwid, BUGNFO+1, ("window was at top of chain\n")); 
                /*------------------------------------------* 
                   This window was on the top of the list     
                 *-------------------------------------------*/
                if (midWG -> pNext != NULL)  
                {
                    BUGLPR(dbg_midwid, BUGNFO+1, ("window NOT on bottom\n")); 
                    /*-----------------------------------------------------* 
                       This window was also the bottom (was not the ONLY)
                       Therefore, start the WID WG chain at the next window
                       and make the previous pointer for the new top of    
                       the list a NULL pointer.  

                       If this window WAS the only window on the chain, we 
                       don't do anything here, as the code in the next     
                       section will move this WID to the UNUSED list.      
                       Also, this probably can't happen, as this would be  
                       an WID with only one window on the SHARED list.     
                     *-----------------------------------------------------*/
                    wid_entry -> pwidWG = midWG -> pNext ;
                    midWG -> pNext -> pPrev = NULL ;
                }
            }

            else if (midWG -> pNext == NULL)  
            {
                BUGLPR(dbg_midwid, BUGNFO+1, ("window was bottom of chain\n"));
                /*----------------------------------------------------* 
                    This window was on the bottom of the list
                    (Make the previous window, the bottom.)    
                 *-----------------------------------------------------*/
                midWG -> pPrev -> pNext = NULL ;
            }

            else
            {
                BUGLPR(dbg_midwid, BUGNFO+1, ("window in middle of chain\n"));
                /*----------------------------------------------------* 
                   This window was somewhere in the middle  
                 *----------------------------------------------------*/
                midWG -> pPrev -> pNext = midWG -> pNext ;
                midWG -> pNext -> pPrev = midWG -> pPrev ;
            }





            /*--------------------------------------------------------------* 
              Now decrement the use count.

              If the use count has gone to 1, the WID entry can be moved to 
              the (bottom) of the UNSHARED list. 
              If the use count has gone to zero, then this WID can be put
              back on the UNUSED list.  This case is probably not possible
              considering we have the previous case covered, however, it is
              included for completeness. 
             *--------------------------------------------------------------*/

            wid_entry -> use_count = wid_entry -> use_count - 1 ;



           /************************************************************ 
             If the use count is 1 the WID is UNSHARED !       
             If the use count is 0 the WID is UNUSED !       
            ************************************************************/
            if (wid_entry -> use_count <=  1) 
            {
                if (wid_entry -> use_count ==  1) 
                {
                    BUGLPR(dbg_midwid, BUGNFO+1, ("Move WID to UNSHARED \n"));
                    wid_entry -> state = MID_WID_UNSHARED ;   
                    bottom = &(ddf -> mid_wid_data.unshared_list.Bottom) ;
                }

                else /* use count = 0 */ 
                {
                    BUGLPR(dbg_midwid, BUGNFO+1, ("Move WID to UNUSED \n"));
                    wid_entry -> state = MID_WID_UNUSED ;   
                    bottom = &(ddf -> mid_wid_data.unused_list.Bottom) ;
                }

                /*--------------------------------------------------* 
                    Now remove the entry from its current list: 
                 *--------------------------------------------------*/
	        MID_DELETE_WID_ENTRY (wid_entry) ; 

                /*--------------------------------------------------* 
                   and then put it on the bottom of its new list.
                 *--------------------------------------------------*/
                MID_ADD_WID_BEFORE (wid_entry, bottom) ;


	    } /* end of use count = 0 or 1 */


    	    MID_DEBUG_WID_LIST (delete_WID, 1, ddf) ; 

#ifdef 	    MID_DEBUG_DELETE_WID
    	    /*----------------------------------------------------------------* 
      	    	Print the window Geometry group, too.         
     	    *----------------------------------------------------------------*/
    	    mid_print_WG_chain (wid_entry) ; 

#endif 	    /* MID_DEBUG_DELETE_WID */


	    break ;






        /**************************************************************** 
          case 5:  otherwise (should not happen)
         ****************************************************************/

        default:              
            BUGLPR(dbg_midwid, 0, ("*** ERROR ***  WID state unknown, " 
				   "WID = %X\n\n",wid_entry->mid_wid ) ); 
    	    MID_DEBUG_WID_LIST (delete_WID, 0, ddf) ; 

            break ; 

    } /* end of switch */




    /*-----------------------------------------------------------------------
	     enable interrupts again 
     *----------------------------------------------------------------------*/
    i_enable (saved_intr_priority) ; 


    /********************************************************************* 
       Trace at the exit point 
     *********************************************************************/

    MID_DEBUG_WID_LIST (delete_WID, 8, ddf) ; 


    MID_DD_EXIT_TRACE (delete_WID, 2, DELETE_WID, ddf, 
			ddf, pWG, midWG->wid, wid_entry->state); 

    BUGLPR(dbg_delete_WID, BUGNFO+1,
		("Leaving delete_WID  ==== WID = %X =====\n\n", midWG->wid));

    return (MID_RC_OK) ;
}
