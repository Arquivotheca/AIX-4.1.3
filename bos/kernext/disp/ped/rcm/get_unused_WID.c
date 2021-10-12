static char sccsid[] = "@(#)70  1.6  src/bos/kernext/disp/ped/rcm/get_unused_WID.c, peddd, bos411, 9428A410j 11/3/93 11:54:24";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: get_unused_WID
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
/* #include <sys/sleep.h> */
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/xmem.h>
#include <sys/rcm_win.h>
#include <sys/aixgsc.h>
#include <sys/rcm.h>
#include "mid.h"
/* #include "midhwa.h"*/
#include "midddf.h"
#include "midrcx.h"
#include "midRC.h"
#include "midwidmac.h"
/* #include "hw_regs_u.h"*/
/* #include "hw_macros.h" */
/* #include "hw_ind_mac.h"*/

#include "mid_dd_trace.h"

/***************************************************************************** 
                                 Externals
 *****************************************************************************/
MID_MODULE (get_unused_WID);
#define    dbg_midwid  dbg_get_unused_WID 

/* #define dbg_middd dbg_get_WID */
/* #define dbg_midpixi dbg_get_WID*/


extern long mid_delete_WID  (
                              ulong       ,
                              midddf_t   *,
                              rcmWG      * ) ;


/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    get_unused_WID
  
   Descriptive Name:   Get Unused Window ID (if available)
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      

     This routine assigns the next available WID to the specified window 
     assuming an available ID does exist.  If not, return code 
     MID_WID_NOT_AVAILBLE is passed back.
  
     This routine takes care of updating the window ID list as necessary.
     In addition, several fields in the window (geometry) are updated --  
     notably the window ID and the WG chain pointers.


 *--------------------------------------------------------------------------* 
  
    Restrictions:
      1.  We should not get here if the window already has a HELD ID.
  
    Dependencies:
      
  
 *--------------------------------------------------------------------------* 
  
    Linkage:
  
 *--------------------------------------------------------------------------* 
  
    INPUT:
          
    OUTPUT: 
          
    RETURN CODES:         
          
          
          
                                 End of Prologue                                
 ******************************************************************************/


long get_unused_WID (
	              int         request,      /* request code */
	              midddf_t   *ddf , 	/* ptr to adapter structure */ 
	              rcmWG      *pWG )	        /* ptr to DI win geom struct */

{
	midWG_t		   *midWG;
	mid_wid_data_t	   *wid_data ;
	mid_wid_status_t   *top, *entry ;
	mid_wid_t      	    new_wid ;
	int                 state ;      /* temp state holder */


/***************************************************************************** 

    BRIEF REVIEW: 

	 Passes back a WID that has just been plucked from the
	 unused list (and is now, therefore, unshared). 

 *****************************************************************************/
	


    	MID_DD_ENTRY_TRACE (get_unused_WID, 1, GET_UNUSED_WID, ddf, 
				ddf, pWG, 0xF0,  request ) ; 


        BUGLPR(dbg_midwid, BUGNTA, ("Entering get_unused_WID. \n"));
	BUGLPR(dbg_midwid, BUGNTA, ("ddf=0x%8X, pWG=0x%8X\n", ddf , pWG ));

    	/*-------------------------------------------------------------
        	First, a little error checking.
     	*---------------------------------------------------------------*/

	/*----------------------------------------------------------------
	  Init some pointers.  Get top of unshared list and first entry.
	------------------------------------------------------------------*/
	wid_data = &(ddf -> mid_wid_data) ;

	top = &(wid_data -> unused_list.Top) ;
	entry = top -> pNext ;

	/*----------------------------------------------------------------
	   Check if unused list is empty.  If it is, there is nothing more  
	   we can do here.  So just leave quietly and orderly. 
	------------------------------------------------------------------*/
	if (entry == &(wid_data -> unused_list.Bottom))
	{
        	BUGLPR(dbg_midwid, BUGNTA, 
        	      ("leaving get_unused_WID:  unused list is empty. \n\n"));

        	/************************************************************* 
           	    Trace at the exit point 
         	*************************************************************/
        	MID_DD_EXIT_TRACE (get_unused_WID, 2, GET_UNUSED_WID, ddf, 
					ddf, pWG, 0xFF, 0 ) ; 

		return (MID_WID_NOT_AVAILABLE) ;
	}
	
		
	/********************************************************************* 
	   else there is an unused ID is available 
	 *********************************************************************/
	 
        BUGLPR(dbg_midwid, BUGNTA, ("unused list not empty. \n"));

	/*------------------------------------------------------------
	   Delete any old window ID from wherever it was.
	------------------------------------------------------------*/
	mid_delete_WID (request, ddf, pWG) ; 


	/*------------------------------------------------------------
	   get the wid from the WID table entry 
	------------------------------------------------------------*/
	new_wid = entry -> mid_wid;



        /*-------------------------------------------------------------
 	    Now put the entry on top of the proper list.   So first we 
 	    must figure out which list we go on.  We also load a state
 	    value corresponding to this same list. 
	 *------------------------------------------------------------*/



	switch (request) 
	{ 
	    case  MID_GET_UNSHARED_WID:      
	        top = &(wid_data -> unshared_list.Top) ;
	        state = MID_WID_UNSHARED ;                 /* set the state */ 
	        break ;

	    case  MID_GET_WID_FOR_PI :      
	        top = wid_data -> unshared_list.Bottom.pPrev ; 
	        state = MID_WID_UNSHARED ;                 /* set the state */ 
	        break ;

	    case  MID_GET_GUARDED_WID:      
	        top = &(wid_data -> guarded_list.Top) ;
	        state = MID_WID_GUARDED ;                 /* set the state */ 
	        break ;

	    default:      /* should not happen */  
	        break ;
	} 



	/********************************************************************* 
	   Move the entry to the top of the list unless this is a  get_WID
	    for PI.  In that event, we put it on the bottom of the list. 

	   This is controlled by the entry after which we insert the new 
	   unused entry.  (This was determined in the switch above.) 
	 *********************************************************************/
	 
	/*------------------------------------------------------------
  	   Remove the new WID from unused table. 
	------------------------------------------------------------*/
	MID_DELETE_WID_ENTRY (entry) ;

	MID_ADD_WID_AFTER (top, entry) ;



        /*-------------------------------------------------------------
 	    and re-init this to an unshared or guarded WID entry:
	 *------------------------------------------------------------*/

	entry -> state = state ;                     /* set the state */ 
	/* entry -> mid_wid = new_wid ; */              /* set the WID */ 
	/* entry -> use_count = 1 ;  */                 /* use count = 1 */ 


        /*-------------------------------------------------------------
 	    now update the device dependent window geometry
	 *------------------------------------------------------------*/
	midWG = (midWG_t *) (pWG -> pPriv) ;  /* get dd WG ptr */ 
        entry -> pwidWG = midWG ; 

	midWG -> wid = new_wid ;	  /* copy WID into WG  */
	midWG -> pNext = NULL ; 	  /* set WG chain ptr to NULL */
	midWG -> pPrev = NULL ; 	  /* set WG chain ptr to NULL */



        BUGLPR(dbg_midwid, BUGNTA, ("leaving get_unused_WID. \n"));
        /************************************************************* 
              Trace at the exit point 
         *************************************************************/

        MID_DEBUG_WID_LIST (get_unused_WID, 1, ddf) ; 

        MID_DD_EXIT_TRACE (get_unused_WID, 2, GET_UNUSED_WID, ddf, 
					ddf, pWG, midWG -> wid, 0xF0); 

	return (MID_WID_ASSIGNED);

}
