static char sccsid[] = "@(#)43  1.13  src/bos/kernext/disp/ped/intr/intrcpos.c, peddd, bos411, 9428A410j 3/31/94 21:34:53";

/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_intr_getcpos
 *		mid_timeout_getcpos
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#define LOADABLE

#include <sys/types.h>
#include <sys/iocc.h>
#include <sys/proc.h>
#include <sys/mdio.h>
#include <sys/conf.h>
#include <sys/intr.h>
#include <sys/uio.h>
#include <sys/dma.h>
#include <sys/pin.h>
#include <sys/device.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/errids.h>
#include <sys/syspest.h>
#ifndef BUGPVT
#define BUGPVT BUGACT
#endif
#include <sys/systm.h>
#include <sys/errno.h>
#define Bool unsigned
#include <sys/aixfont.h>
#include <sys/display.h>
#include "hw_dd_model.h"
#include "midddf.h"
#include "midhwa.h"
#include "hw_regs_k.h"
#include "hw_macros.h"
#include "hw_ind_mac.h"		/* for Status Control Block access          */
#include "hw_regs_u.h"
#include "hw_seops.h" 		/* needed for MID_WR_HOST_COMO() to work    */
#include "rcm_mac.h"            /* rcm-related macros.  Using FIND_GP       */
#include "midwidmac.h"		/* needed for MID_MARK_BUFFERS_SWAPPED call */

#include "mid_dd_trace.h"       /* for memory trace                         */
MID_MODULE ( dsp_getcpos );



/*----------------------------------------------------------------------------
Define watchdog timer handler function for mid_getcpos().  When 
the watchdog timer pops, the appropriate handler gets called and 
wakes up the process at a point where it can recover.
----------------------------------------------------------------------------*/

void mid_timeout_getcpos( w )
mid_watch_getcpos_t *w;
{

        BUGLPR(dbg_dsp_getcpos,0 ,
       	  ("TIMED OUT  ---  CALLING WATCHDOG TIMER:   mid_watch_getcpos()\n"));  
        BUGLPR(dbg_dsp_getcpos,0 , ("  w->sleep_addr=0x%x\n", w->sleep_addr ));

        e_wakeup( w->sleep_addr );

}




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* IDENTIFICATION: MID_INTR_GETCPOS                                         */
/*                                                                          */
/* DESCRIPTIVE NAME:                                                        */
/*                                                                          */
/* FUNCTION:                                                                */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/* INPUTS:                                                                  */
/*                                                                          */
/* OUTPUTS:                                                                 */
/*                                                                          */
/* CALLED BY:                                                               */
/*                                                                          */
/* CALLS:                                                                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/




/*----------------------------------------------------------------------------

			M I D _ I N T R _ G E T C P O S

Interrupt Level Get Current Character Position (mid_intr_getcpos)


DESCRIPTION

Reads character position data off of the adapter in window coordinates.  These
are changed by MOD2 to screen coordinates.  This interrupt handler then 
writes the data into the caller's structure and awakens the function which 
generated the adapter read.

long mid_intr_getcpos( ddf )
		midddf_t		*ddf;


INPUT PARAMETERS:

ddf		Pointer to structure of type  "midddf_t".  This structure has
		a field which points to the DDF data linked list, "ddf_data".
		This structure will also have a pointer to the correlator 
		count,  "ddf_correlator_count", which is used to track last
		assigned correlator value.



RETURN VALUES:

none.



PSEUDOCODE:

	Enable bus
	
	Read character postion off of adapter

	Disable bus

	Locate structure associated with current correlator, and ...

	Assign (X,Y) character position into this structure

	Wake up sleeping process

	Return
	
----------------------------------------------------------------------------*/



long mid_intr_getcpos( ddf )
	midddf_t 	*ddf;
{
        ushort          event_correlator;  /* store corr value from SCB      */
        short           tmp_correlator;    /* from getcpos linked list       */
        ddf_data_getcpos_t  *tmp_next_ptr; /* ptr to next array element      */
        ddf_data_getcpos_t  *current_ptr;  /* ptr to current struct          */
        short           i, k;		   /* loop counters                  */
	ulong		window_char_position; /* local var. for X-Y pair     */

        HWPDDFSetup;                       /* gain access to hardware pointer*/

	BUGLPR(dbg_dsp_getcpos, 1, ("Entering mid_intr_getcpos()\n"));

        BUGLPR(dbg_dsp_getcpos, 5, ("*ddf=0x%x\n", ddf));




	/*-------------------------------------------------------------------
	Set current event correlator variable to the correlator of the 
	event which we are currently servicing.  The value was saved 
	previously in the "ddf" structure by the caller of this function.
	The high 2 bytes are AND'ed out since we want only the correlator
	value itself.
	-------------------------------------------------------------------*/

	event_correlator = (0x0000FFFF & ddf->dsp_status);

	BUGLPR(dbg_dsp_getcpos, 4, ("event_correlator=0x%x \n", 
		event_correlator));

        MID_DD_ENTRY_TRACE (dsp_getcpos, 1, DSP_GET_CPOS, ddf,
                0, event_correlator, 0, 0 );




	/*-------------------------------------------------------------------
	Call hardware macro to read 3DM1M Status Control Block data into  
	"midddf_t" data structure.  This data includes character position in 
	window coordinates and operation correlator.  The correlator will be 
	used by the interrupt handler to associate a particular "mid_getcpos()" 
	function call with these results.  This simply ensures that if another 
	call to "mid_getcpos()" occurs before the information for the first 	
	call gets back to the first caller, that we have a way of making a 
	one-to-one mapping between caller and results.  
	-------------------------------------------------------------------*/

        MID_RD_M1MSB_VALUE(MID_M1MSB_WINDOW_CHAR_POSITION,window_char_position);

        BUGLPR(dbg_dsp_getcpos, 2, 
		("Raw SCREEN character position=0x%x (hex) \n",
		window_char_position ));

        BUGLPR(dbg_dsp_getcpos, 4, 
		("Raw SCREEN character position=  %d (int) \n",
		(ulong) window_char_position  ));




        /*-------------------------------------------------------------------
        Match correlator value from Status Control Block on the adapter
        with correlator(s) in "ddf_data_getcpos_t" structure.

	event_correlator corresponds to the interrupt being serviced.
	tmp_correlator cycles through the available correlators until it
	finds a match.
        -------------------------------------------------------------------*/

        i = 0;
	current_ptr    = &ddf->ddf_data_getcpos[i];
        tmp_next_ptr   = ddf->ddf_data_getcpos[i].next;
        tmp_correlator = ddf->ddf_data_getcpos[i].correlator;

        BUGLPR(dbg_dsp_getcpos, 5, 
		("tmp_correlator = 0x%x, event_correlator=0x%x\n", 
		tmp_correlator, event_correlator));



        while (tmp_correlator != event_correlator)
	/*----------------------------------------
	While correlator match not yet found ...
	----------------------------------------*/
        {

       		BUGLPR(dbg_dsp_getcpos, 3, 
			("tmp_correlator = 0x%x, event_correlator=0x%x\n", 
			tmp_correlator, event_correlator));


                /*------------------------------------------------------------
                If tmp_next_ptr is NULL, then we have looked through
                all the correlators we have for interrupts of this type and
                have not found a match for the correlator we received on the
                interrupt.  This should never happen.
                ------------------------------------------------------------*/

                if (tmp_next_ptr == NULL)
                {
                        BUGLPR(dbg_dsp_getcpos, 0,
                        ("++++++MATCHING CORRELATOR NOT FOUND FOR: 0x%x +++++",
                        event_correlator));
                        return(99);
                }


        	/*------------------------------------------------------------
		Keep track of current place in the linked list so when 
		we exit, we don't have to back track to the previous 
		element in the list.
        	------------------------------------------------------------*/

        	current_ptr    = tmp_next_ptr;

                tmp_correlator = tmp_next_ptr->correlator;
                tmp_next_ptr   = tmp_next_ptr->next;

        }



       	/*------------------------------------------------------------------
	Put current character position data in structure shared with
	mid_getcpos() so that when we awaken mid_getcpos(), the data will be 
	available to pass back to the caller.

	Since the X-Y values were in one ulong value, must split it into
	two shorts as needed by the user.
       	------------------------------------------------------------------*/

        current_ptr->ddf_getcpos.xcpos = 
		(ushort) (window_char_position >> 16);
        current_ptr->ddf_getcpos.ycpos = 
		(ushort) (window_char_position & 0x0000FFFF);
		/*(ushort) (window_char_position && 0x0000FFFF); */

        MID_DD_TRACE_PARMS (dsp_getcpos, 1, DSP_GET_CPOS, ddf,
                0, 
		event_correlator, 
        	current_ptr->ddf_getcpos.xcpos, 
	        current_ptr->ddf_getcpos.ycpos );




        /*------------------------------------------------------------------
	Dump ddf_data array for debugging.
        ------------------------------------------------------------------*/

	BUGLPR(dbg_dsp_getcpos, 5, ("DUMPING ddf_data_getcpos_t:\n"));


	for (k=0; k<4; k++)
	{

	        BUGLPR(dbg_dsp_getcpos, 5, 
			("  ddf->ddf_data_getcpos[%d].ddf_getcpos.xcpos=%d\n",
			k,  ddf->ddf_data_getcpos[k].ddf_getcpos.xcpos));
	        BUGLPR(dbg_dsp_getcpos, 5, 
			("  ddf->ddf_data_getcpos[%d].ddf_getcpos.ycpos=%d\n",
			k,  ddf->ddf_data_getcpos[k].ddf_getcpos.ycpos));
	        BUGLPR(dbg_dsp_getcpos, 5, 
                        ("  ddf->ddf_data_getcpos[%d].correlator=%0x%x\n",
			k,  ddf->ddf_data_getcpos[k].correlator));
	        BUGLPR(dbg_dsp_getcpos, 5, 
			("  ddf->ddf_data_getcpos[%d].wakeup_address=0x%x\n", 
			k,  ddf->ddf_data_getcpos[k].wakeup_address));
	        BUGLPR(dbg_dsp_getcpos, 5, 
			("  ddf->ddf_data_getcpos[%d].next=0x%x\n", 
			k,  ddf->ddf_data_getcpos[k].next));
	        BUGLPR(dbg_dsp_getcpos, 5, 
			("  ddf->ddf_data_getcpos[%d].user_data=0x%x\n", 
			k,  ddf->ddf_data_getcpos[k].user_data));

	}

		
        BUGLPR(dbg_dsp_getcpos, 2, ("Assign X-Y to DDF structure:  \n"));
        BUGLPR(dbg_dsp_getcpos, 2, ("    current_ptr->ddf_getcpos.xcpos=0x%x\n",
                (ushort) current_ptr->ddf_getcpos.xcpos));
        BUGLPR(dbg_dsp_getcpos, 2, ("    current_ptr->ddf_getcpos.ycpos=0x%x\n",
                (ushort) current_ptr->ddf_getcpos.ycpos));



	/*-------------------------------------------------------------------
	Wake up process that generated the original  "mid_getcpos()"  call.  
	The address of this call is in the  "wakeup_address"  field of the  
	"ddf_data getcpos_t"  structure.
	-------------------------------------------------------------------*/

        BUGLPR(dbg_dsp_getcpos, 3, ("Wake up sleeper. wakeup_address = 0x%x\n",
	        ddf->e_sleep_anchor));



	/* -----------------------------------------------------------------
	   The following was added to take care of the 'deadlock' condition
	   when writing to the FIFO (full) with interrupts disabled
	   If sleep_flg is set, the process is asleep, wake it up else
	   set int_rcvd flag so the process will not go to sleep
	------------------------------------------------------------------*/
	   
	if(current_ptr->sleep_flg)
	{
                current_ptr->sleep_flg = 0x10; /* 10 means sleep took place */

		e_wakeup( &current_ptr->wakeup_address );

		BUGLPR(dbg_dsp_getcpos, 3, ("After e_wakeup call.\n"));
	}
	else
	{
		current_ptr->int_rcvd++;
	}



	BUGLPR(dbg_dsp_getcpos, 1, ("Leaving  mid_intr_getcpos()\n"));

        MID_DD_EXIT_TRACE (dsp_getcpos, 1, DSP_GET_CPOS, ddf,
                0, 
		event_correlator, 	
		current_ptr->sleep_flg, 
		current_ptr->int_rcvd );


	return (0);


}	/* end of mid_intr_getcpos */
