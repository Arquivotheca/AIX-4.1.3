/* @(#)35  	1.6.1.4  src/bos/kernext/disp/ped/inc/midddfrcm.h, peddd, bos411, 9428A410j 3/19/93 18:54:27 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: *		
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


#ifndef _H_MIDDDFRCM
#define _H_MIDDDFRCM



    /**************************************************************************

      This file is included in the MIDDLE of the midddf structure. 

      Essentially it is a list of RCM related fields for the ddf structure.

     *************************************************************************/


	/*--------------------------------------------------------------------* 

	   ADAPTER DOMAIN DATA

	   The following group of fields is domain-based data:  that is,    
	   data which must exist on a per domain basis.  Since there is only
	   one (FIFO) domain on the mid_level graphics adapter (family), then
	   it is quite reasonable to put this data in the ddf structure.


	 *--------------------------------------------------------------------*/

#	include "middom.h"        	






	/*--------------------------------------------------------------------* 
	   RCM data
	   
           This section contains the various pieces of the RCM ddf data.    
           This includes: 
             . the window ID list,
             . the count of DWA WIDS,
             . the count of DWA contexts,
             . the count of guarded_WIDs

             . the top and bottom list pointers for the WID sleep list

            NOTE that the mid_wid_data_t requires midrcx.h to be included.
	 *--------------------------------------------------------------------*/

        ushort	num_DWA_WIDS ;  	/* count of DWA WIDS currently in use*/
        ushort	mid_guarded_WID_count ;  /* count of unguarded WIDs */

        ushort	num_graphics_processes ; /* current count of GPs */
        ushort	num_DWA_contexts ;  	/* current count of DWA contexts */


	/*--------------------------------------------------------------------* 
	   WID sleep list (anchor)

 	   No WID list updates (get_WID_for_anything) are allowed during
 	   a context switch.  This is because we have prepared the WIDs for
 	   the context switching on to have a guaranteed unshared WID. 
 	   There are generally pending WID writes (and Assoc CP and Set
 	   Window Parms) to be sent later during the Context State Updates.
 	   The WID list must not change until that is finished.  Any WID
	   operations that are attempted during that time are put to 
 	   sleep. 

 	   When the context switch finishes, all the processes sleeping 
 	   for this reason are awaken. 
	 *--------------------------------------------------------------------*/
	mid_rcx_t  *WID_ctx_sw_sleep_top ;



	/*--------------------------------------------------------------------* 
	   GUARDED WID sleep list (anchor)

 	   There is a limit to the number of WIDs that may be used for 
 	   double buffering (aka swap buffers).  Therefore, it is possible
 	   to have a process request a guarded WID (for double buffering)  
 	   and none are available.  In fact, there is a transient case, where
 	   we may have to put "regular" UNSHRAED_WID request on this list. 

 	   In this event the process is put to sleep.

 	   We keep a top and bottom pointer for the process(es) (contexts)
 	   on this list. 

 	   There is also a watchdog for this list, since there are more sleeps
 	   than wakeups. 
	 *--------------------------------------------------------------------*/
	struct mid_watchdog	WID_watch ;

	mid_rcx_t  *WID_sleep_top ;
	mid_rcx_t  *WID_sleep_bottom ;


	/*--------------------------------------------------------------------* 
	   WID list itself  
	 *--------------------------------------------------------------------*/
        mid_wid_data_t  mid_wid_data ;  /* window ID info - see midrcx.h */



	/*--------------------------------------------------------------------* 
	   Set Window Parameters synchronization data

           Each time a Set Window Parameters command is put into the FIFO,
           we save the associate correlator in the following field and also
           set the flag in the following field. 

           There are a couple places in the RCM code where we must check if
           there are any outstanding SetWindowParamter commands in the FIFO.
           These checks reference and update these two fields. 
           
           NOTE:  There should be at least one set of these fields per domain.
	 *--------------------------------------------------------------------*/
        ushort          mid_last_WID_change_corr ;  /* WID correlator */     
        ushort          WID_change_flag ;           /* WID change flag */     
#	define 		MID_WID_NO_CHANGE_PEND  0 
#	define 		MID_WID_CHANGE_PEND  1 
#       define          WID_CORR_INIT  1




#endif /* _H_MIDDDFRCM */
