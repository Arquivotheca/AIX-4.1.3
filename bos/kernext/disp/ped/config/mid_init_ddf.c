static char sccsid[] = "@(#)52	1.5.1.7  src/bos/kernext/disp/ped/config/mid_init_ddf.c, peddd, bos411, 9428A410j 6/23/94 10:46:29";
/*
 * COMPONENT_NAME: PEDDD
 *
 * FUNCTIONS: mid_initialize_DDF
 *	
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/types.h>
#include <sys/intr.h>
#include <sys/sleep.h>
#include <sys/syspest.h>
#include <sys/watchdog.h>	/* for watchdog timer initialization 	*/

#include "midddf.h"

BUGXDEF(dbg_midddf);


/*----------------------------------------------------------------------------
declare watchdog timer functions for DDF
----------------------------------------------------------------------------*/

extern void mid_timeout_getcpos( struct mid_watch_getcpos_t * );
extern void mid_timeout_getcolor( struct mid_watch_getcolor_t * );
extern void mid_timeout_getcondition( struct mid_getcondition_t * );
extern void mid_timeout_gettextfontindex( struct mid_gettextfontindex_t * );
extern void mid_timeout_endrender( struct mid_watchdog_data_t * );
extern void mid_timeout_lowwater( mid_watch_lowwater_int_t * );

extern void mid_timeout_swap( mid_swap_watch_t	* );

extern void mid_WID_wakeup( mid_watchdog_t	* );







/*----------------------------------------------------------------------------
Called by mid_open() in middef.c to initialize DDF data structures for
the Device Dependent Fuction portion of the device driver.
----------------------------------------------------------------------------*/

long mid_initialize_DDF( ddf )
midddf_t		*ddf;
{
	int		rc;
	short		i;

	struct watchdog	*getcpos_watchdog_struct_ptr;  /* local variables   */
	struct watchdog	*getcolor_watchdog_struct_ptr; /* to hold pointers  */
	struct watchdog	*getcondition_watchdog_struct_ptr; /* to watchdog   */
	struct watchdog	*gettextfontindex_watchdog_struct_ptr; /* structures*/
	struct watchdog	*endrender_watchdog_struct_ptr;



        /*-------------------------------------------------------------------
        Initilize link lists which which will hold data for each DDF call.
        -------------------------------------------------------------------*/

	BUGLPR(dbg_midddf, BUGACT, 
		("Initializing ddf_data linked list structures\n"));
        
        for (i=0; i<ARY_SIZE-1; i++)
        {
       	        ddf->ddf_data_getcolor[i].next =
                        &ddf->ddf_data_getcolor[i+1];
                ddf->ddf_data_getcpos[i].next =
                        &ddf->ddf_data_getcpos[i+1];
                ddf->ddf_data_getcolor[i].next =
                        &ddf->ddf_data_getcolor[i+1];
                ddf->ddf_data_get_projection_matrix[i].next =
                        &ddf->ddf_data_get_projection_matrix[i+1];
                ddf->ddf_data_get_modelling_matrix[i].next =
                        &ddf->ddf_data_get_modelling_matrix[i+1];
                ddf->ddf_data_getcondition[i].next =
                        &ddf->ddf_data_getcondition[i+1];
                ddf->ddf_data_gettextfontindex[i].next =
                        &ddf->ddf_data_gettextfontindex[i+1];
                ddf->ddf_data_endrender[i].next =
                        &ddf->ddf_data_endrender[i+1];
         }



         /*------------------------------------------------------------------
             Initialize swapbuffers window ID's to MID_WID_NULL
         ------------------------------------------------------------------*/

#define  SWAP_LIST 	ddf->ddf_data_swapbuffers_hold_list

         for (i=0; i<MAX_WIDS; i++)
         {
		SWAP_LIST[i].window_ID = MID_WID_NULL;

		SWAP_LIST[i].watch.dog.next = NULL ;
		SWAP_LIST[i].watch.dog.prev = NULL ;
		SWAP_LIST[i].watch.dog.func = mid_timeout_swap ;
		SWAP_LIST[i].watch.dog.count = 0 ;
		SWAP_LIST[i].watch.dog.restart = 15 ;

		SWAP_LIST[i].watch.entry = (ulong *) (&(SWAP_LIST[i])) ;
		SWAP_LIST[i].watch.index = i ; 
		SWAP_LIST[i].watch.ddf = (ulong *) (ddf) ;


#ifdef  ENABLE_WATCHDOGS  /* Disable the watchdog timers.  They can cause   */
                          /* problems when the adapter is legitimately      */
                          /* taking more time than the watchdog is set for. */


#endif
		w_init( &(SWAP_LIST[i].watch.dog) ) ;
         }




        /*------------------------------------------------------------------
	Initialize last array element for each type to NULL.  This is
	done to facilitate dynamic allocation scheme.
	------------------------------------------------------------------*/

        ddf->ddf_data_getcolor[ARY_SIZE-1].next = NULL;
       	ddf->ddf_data_getcpos[ARY_SIZE-1].next = NULL;
        ddf->ddf_data_getcolor[ARY_SIZE-1].next = NULL;
        ddf->ddf_data_get_projection_matrix[ARY_SIZE-1].next = NULL;
        ddf->ddf_data_get_modelling_matrix[ARY_SIZE-1].next = NULL;
        ddf->ddf_data_getcondition[ARY_SIZE-1].next = NULL;
        ddf->ddf_data_gettextfontindex[ARY_SIZE-1].next = NULL;
        ddf->ddf_data_endrender[ARY_SIZE-1].next = NULL;



        /*------------------------------------------------------------------
        Set initial value of DDF correlator to 1.  The choice of "1" is
        arbitrary.
        ------------------------------------------------------------------*/

        ddf->correlator_count = INITIAL_CORRELATOR_VALUE;



        /*------------------------------------------------------------------
	Establish anchor value for e_sleep() calls in DDF functions.
        ------------------------------------------------------------------*/

	ddf->e_sleep_anchor = EVENT_NULL;



        /*--------------------------------------------------------------------
	Define watchdog timers and initialize watchdog structure values.
        --------------------------------------------------------------------*/

	for ( i=0; i<ARY_SIZE; i++ )
	{

		/*------------------------------------------------------------
		Assign pointers to existing watchdog structures to local
		variables to use for initialization.
		------------------------------------------------------------*/

		getcpos_watchdog_struct_ptr = 
			( &(ddf->ddf_data_getcpos[i].watchdog_data.dog) );	
		getcolor_watchdog_struct_ptr = 
			( &(ddf->ddf_data_getcolor[i].watchdog_data.dog) );	
		getcondition_watchdog_struct_ptr = 
			( &(ddf->ddf_data_getcondition[i].watchdog_data.dog) );	
		gettextfontindex_watchdog_struct_ptr = 
		    ( &(ddf->ddf_data_gettextfontindex[i].watchdog_data.dog) );	
		endrender_watchdog_struct_ptr = 
			( &(ddf->ddf_data_endrender[i].watchdog_data.dog) );	
		BUGLPR(dbg_midddf, 5, 
			("getcpos_watchdog_struct_ptr = 0x%x i=%d\n",
			getcpos_watchdog_struct_ptr, i ));



		/*============================================================

		Initialize watchdog structures:

		============================================================*/



		/*------------------------------------------------------------
			Set next and previous fields in watchdog chain.
		------------------------------------------------------------*/

		getcpos_watchdog_struct_ptr->next		= NULL;
		getcpos_watchdog_struct_ptr->prev		= NULL;

		getcolor_watchdog_struct_ptr->next		= NULL;
		getcolor_watchdog_struct_ptr->prev		= NULL;

		getcondition_watchdog_struct_ptr->next		= NULL;
		getcondition_watchdog_struct_ptr->prev		= NULL;

		gettextfontindex_watchdog_struct_ptr->next	= NULL;
		gettextfontindex_watchdog_struct_ptr->prev	= NULL;

		endrender_watchdog_struct_ptr->next		= NULL;
		endrender_watchdog_struct_ptr->prev		= NULL;


		BUGLPR(dbg_midddf, 5, 
			("watchdog_data.dog.next =  0x%x\n",
			ddf->ddf_data_getcpos[i].watchdog_data.dog.next));
        
		BUGLPR(dbg_midddf, 5, 
			("watchdog_data.dog.prev =  0x%x\n",
			ddf->ddf_data_getcpos[i].watchdog_data.dog.prev));
        


		/*------------------------------------------------------------
				Set restart period.
		------------------------------------------------------------*/

		getcpos_watchdog_struct_ptr->restart		= 
					GETCPOS_WATCHDOG_PERIOD;
		getcolor_watchdog_struct_ptr->restart		= 
					GETCOLOR_WATCHDOG_PERIOD;
		getcondition_watchdog_struct_ptr->restart	= 
					GETCONDITION_WATCHDOG_PERIOD;
		gettextfontindex_watchdog_struct_ptr->restart	= 
					GETTEXTFONTINDEX_WATCHDOG_PERIOD;
		endrender_watchdog_struct_ptr->restart	= 
					ENDRENDER_WATCHDOG_PERIOD;
        
		BUGLPR(dbg_midddf, 5, 
			("watchdog_data.dog.restart =  0x%x\n",
			ddf->ddf_data_getcpos[i].watchdog_data.dog.restart));


		/*-----------------------------------------------------------
				Assign watchdog functions.
		-----------------------------------------------------------*/

		getcpos_watchdog_struct_ptr->func		= 
					mid_timeout_getcpos;
		
		getcolor_watchdog_struct_ptr->func		= 
					mid_timeout_getcolor;
	
		getcondition_watchdog_struct_ptr->func		= 	
					mid_timeout_getcondition;

		gettextfontindex_watchdog_struct_ptr->func	=
					mid_timeout_gettextfontindex; 

		endrender_watchdog_struct_ptr->func		= 	
					mid_timeout_endrender;

		



		/*-----------------------------------------------------------
			Initialize count fields to zero.
		-----------------------------------------------------------*/

		
		getcpos_watchdog_struct_ptr->count		= 0;
		
		getcolor_watchdog_struct_ptr->count		= 0;
	
		getcondition_watchdog_struct_ptr->count		= 0;

		gettextfontindex_watchdog_struct_ptr->count	= 0;
        
		endrender_watchdog_struct_ptr->count		= 0;

		BUGLPR(dbg_midddf, 5, 
			("watchdog_data.dog.count =  0x%x\n",
			ddf->ddf_data_getcpos[i].watchdog_data.dog.count));




#ifdef  ENABLE_WATCHDOGS  /* Disable the watchdog timers.  They can cause   */
                          /* problems when the adapter is legitimately      */
                          /* taking more time than the watchdog is set for. */

        	/*------------------------------------------------------------
			Register DDF watchdog timers with the kernel.
       		------------------------------------------------------------*/

		w_init( getcpos_watchdog_struct_ptr );
		w_init( getcolor_watchdog_struct_ptr );
		w_init( getcondition_watchdog_struct_ptr );
		w_init( gettextfontindex_watchdog_struct_ptr );
		w_init( endrender_watchdog_struct_ptr );
#endif


	}





	ddf->mid_lowwater_watchdog.dog.next	= NULL;
	ddf->mid_lowwater_watchdog.dog.prev	= NULL;
	ddf->mid_lowwater_watchdog.dog.restart	= LOW_WATER_WATCHDOG_PERIOD;
	ddf->mid_lowwater_watchdog.dog.func	= mid_timeout_lowwater;
	ddf->mid_lowwater_watchdog.dog.count	= 0;

	ddf->mid_lowwater_watchdog.ddf		= (char *) ddf ;


#ifdef  ENABLE_WATCHDOGS  /* Disable the watchdog timers.  They can cause   */
                          /* problems when the adapter is legitimately      */
                          /* taking more time than the watchdog is set for. */


	w_init( &(ddf->mid_lowwater_watchdog.dog) );
#endif


       	/*------------------------------------------------------------
		Init the WID watchdog timer 
      	------------------------------------------------------------*/

	ddf->WID_watch.dog.next		= NULL;
	ddf->WID_watch.dog.prev		= NULL;
	ddf->WID_watch.dog.restart 	= 2 ;
	ddf->WID_watch.dog.func		= mid_WID_wakeup ;
	ddf->WID_watch.dog.count 	= 0;

	ddf->WID_watch.ddf		= (char *) ddf ;

	w_init( &(ddf->WID_watch.dog) );

}
