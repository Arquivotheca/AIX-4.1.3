static char sccsid[] = "@(#)44  1.11  src/bos/kernext/disp/ped/intr/intrindex.c, peddd, bos411, 9428A410j 3/31/94 21:34:58";

/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_intr_gettextfontindex
 *		mid_timeout_gettextfontindex
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
#include "hw_seops.h"		/* needed for MID_WR_HOST_COMO() to work    */
#include "rcm_mac.h"            /* rcm-related macros.  Using FIND_GP       */
#include "midwidmac.h"		/* needed for MID_MARK_BUFFERS_SWAPPED call */

#include "mid_dd_trace.h"       /* for memory trace                         */
MID_MODULE ( dsp_getfontindex );



/*----------------------------------------------------------------------------
Define watchdog timer handler function for mid_gettextfontindex().  
When the watchdog timer pops, the appropriate handler gets called 
and wakes up the process at a point where it can recover.
----------------------------------------------------------------------------*/

void mid_timeout_gettextfontindex( w )
mid_watch_gettextfontindex_t *w;
{

        BUGLPR(dbg_dsp_getfontindex,0 ,
        ("TIMED OUT - CALLING WATCHDOG TIMER: mid_watch_gettextfontindex()\n"));
        BUGLPR(dbg_dsp_getfontindex,0 , 
		("     w->sleep_addr=0x%x\n", w->sleep_addr ));

        e_wakeup( w->sleep_addr );

}



/*---------------------------------------------------------------------*/
/*                                                                     */
/* IDENTIFICATION: MID_INTR_GETTEXTFONTINDEX                           */
/*                                                                     */
/* DESCRIPTIVE NAME:                                                   */
/*                                                                     */
/* FUNCTION:                                                           */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/* INPUTS:                                                             */
/*                                                                     */
/* OUTPUTS:                                                            */
/*                                                                     */
/* CALLED BY:                                                          */
/*                                                                     */
/* CALLS:                                                              */
/*                                                                     */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------

              M I D _ I N T R _ G E T T E X T F O N T I N D E X




Interrupt Level Get Text Font Index (mid_intr_gettextfontindex)


DESCRIPTION

Read current Text Font Index value out of the 3DM1 Status Block on the adapter.

long mid_intr_gettextfontindex( ddf )
		midddf_t		*ddf;


INPUT PARAMETERS:

ddf		Pointer to structure of type  "midddf_t".  This structure has
		a field which points to the DDF data link list, "ddf_data".
		This structure will also have a pointer to the correlator 
		count,  "ddf_correlator_count", which is used to track last
		assigned correlator value.



RETURN VALUES:

none.



PSEUDOCODE:

	Enable bus
	
	Read text font index off of adapter

	Locate structure associated with current interrupt, and ...

	Assign text font index into ddf structure 

	Wake up sleeping process

	Return

-----------------------------------------------------------------------------*/


long mid_intr_gettextfontindex( ddf )
                midddf_t                *ddf;
{


        ushort          event_correlator; /* store corr value from SCB       */
        ushort          tmp_correlator;   /* from gettextfont linked list    */
        ddf_data_gettextfontindex_t  *tmp_next_ptr; /* ptr to next ary element*/
        ddf_data_gettextfontindex_t  *current_ptr;  /* ptr to current struct */
        short           i, k;		  /* loop counters / indices         */
	ulong		SB_textfontindex; /* hold value from adapter         */ 
	ushort		SB_correlator;    /* Correlator from adapter         */



        HWPDDFSetup;                      /* gain access to hardware pointer */

        BUGLPR(dbg_dsp_getfontindex, 1, 
		("Entering mid_intr_gettextfontindex()\n"));





	/*---------------------------------------------------------------------
	Set current event correlator variable to the correlator of the 
	event which we are currently servicing.  The value was saved 
	previously in the "ddf" structure by the caller of this function.
	The high 2 bytes are AND'ed out since we want only the correlator
	value itself.
	---------------------------------------------------------------------*/

	event_correlator = ( (ushort) (0x0000FFFF & ddf->dsp_status) );

        MID_DD_ENTRY_TRACE (dsp_getfontindex, 1, DSP_GET_FONT_INDEX, ddf,
                0, event_correlator, 0, 0 );


	BUGLPR(dbg_dsp_getfontindex, 4, ("event_correlator=0x%x \n", 
		event_correlator));



	/*---------------------------------------------------------------------
	Call hardware macro to read Text Font Index, and corresponding 
	correlator from the 3DM1 Status Control Block on the adapter.
        ---------------------------------------------------------------------*/

        MID_RD_M1SB_VALUE( MID_M1SB_TEXT_FONT_CORRELATOR, SB_correlator);
        MID_RD_M1SB_VALUE( MID_M1SB_TEXT_FONT, SB_textfontindex);


        MID_DD_TRACE_PARMS (dsp_getfontindex, 1, DSP_GET_FONT_INDEX_PARMS, ddf,
                0, 0x1, event_correlator, SB_textfontindex );


        BUGLPR(dbg_dsp_getfontindex, 3, ("From adapter: \n"));
        BUGLPR(dbg_dsp_getfontindex, 3, 
		("      SB_correlator=0x%x \n",SB_correlator));
        BUGLPR(dbg_dsp_getfontindex, 3, 
		("      SB_textfontindex=0x%x \n", SB_textfontindex));





	
        /*---------------------------------------------------------------------
        Match correlator value from Status Control Block on the adapter
        with correlator(s) in "ddf_data_gettextfontindex" structure.

	event_correlator corresponds to the interrupt being serviced.
	tmp_correlator cycles through the available correlators until it
	finds a match.
        ---------------------------------------------------------------------*/

        i = 0;
	current_ptr    = &ddf->ddf_data_gettextfontindex[i];
        tmp_next_ptr   = ddf->ddf_data_gettextfontindex[i].next;
        tmp_correlator = ddf->ddf_data_gettextfontindex[i].correlator;

        BUGLPR(dbg_dsp_getfontindex, 5, 
		("tmp_correlator = 0x%x, event_correlator=0x%x\n", 
		tmp_correlator, event_correlator));



        while (tmp_correlator != event_correlator)
        {

       		BUGLPR(dbg_dsp_getfontindex, 5, 
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

                        BUGLPR(dbg_dsp_getfontindex, 1,
                        ("++++++MATCHING CORRELATOR NOT FOUND FOR: 0x%x +++++",
                        event_correlator));

                        MID_DD_TRACE_PARMS (dsp_getfontindex, 1,
                                DSP_GET_FONT_INDEX_PARMS, ddf,
                                0, event_correlator, 0xD000D000, i );


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



	/*---------------------------------------------------------------------
	Write data from adapter to ddf_data_gettextfontindex structure so that
	mid_gettextfontindex() will be able to pass it back to the caller.
	---------------------------------------------------------------------*/

	current_ptr->ddf_gettextfontindex.textfontindex = SB_textfontindex;
	current_ptr->ddf_gettextfontindex.correlator    = SB_correlator;

        MID_DD_TRACE_PARMS (dsp_getfontindex, 1, DSP_GET_FONT_INDEX_PARMS, ddf,
                0, 0x3,
                current_ptr->ddf_gettextfontindex.correlator,
                current_ptr->ddf_gettextfontindex.textfontindex );


        BUGLPR(dbg_dsp_getfontindex, 3, 
		("Assign structure values:  index = 0x%x  correlator = 0x%x\n",
        	current_ptr->ddf_gettextfontindex.textfontindex,
		current_ptr->ddf_gettextfontindex.correlator     ));




        /*--------------------------------------------------------------------
	Dump ddf_data array for debugging.
        --------------------------------------------------------------------*/

	BUGLPR(dbg_dsp_getfontindex, 5, 	
			("DUMPING ddf_data_gettextfontindex:\n"));
	for (k=0; k<4; k++)
	{
        BUGLPR(dbg_dsp_getfontindex, 5, 
	("  ddf->ddf_data_gettextfontindex[%d].textfontindex.correlator=0x%x\n",
	k,  ddf->ddf_data_gettextfontindex[k].ddf_gettextfontindex.correlator));
        BUGLPR(dbg_dsp_getfontindex, 5, 
	("  ddf->ddf_data_gettextfontindex[%d].fntindex.textfontindex=0x%x\n",k,
	ddf->ddf_data_gettextfontindex[k].ddf_gettextfontindex.textfontindex));
	BUGLPR(dbg_dsp_getfontindex, 5, 
		("  ddf->ddf_data_gettextfontindex[%d].correlator=0x%x\n", 
		k,  ddf->ddf_data_gettextfontindex[k].correlator));
        BUGLPR(dbg_dsp_getfontindex, 5, 
		("  ddf->ddf_data_gettextfontindex[%d].wakeup_address=0x%x\n", 
		k,  ddf->ddf_data_gettextfontindex[k].wakeup_address));
       	BUGLPR(dbg_dsp_getfontindex, 5, 
		("  ddf->ddf_data_gettextfontindex[%d].next=0x%x\n", 
		k,  ddf->ddf_data_gettextfontindex[k].next));
       	BUGLPR(dbg_dsp_getfontindex, 5, 
		("  ddf->ddf_data_gettextfontindex[%d].user_data=0x%x\n", 
		k,  ddf->ddf_data_gettextfontindex[k].user_data));
	}


	

	/*---------------------------------------------------------------------
	Wake up process that generated the original  "mid_gettextfontindex()"  
	call.  The address of this call is in the  "wakeup_address"  field of 
	the  "ddf_data_gettextfontindex_t"  structure.
	---------------------------------------------------------------------*/

        BUGLPR(dbg_dsp_getfontindex, 4, 
		("Wake up sleeper. wakeup_address = 0x%x\n",
        	ddf->e_sleep_anchor));

        MID_DD_TRACE_PARMS (dsp_getfontindex, 1, DSP_GET_FONT_INDEX_PARMS, ddf,
                0, 0x4, event_correlator, &current_ptr->wakeup_address );



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

        	BUGLPR(dbg_dsp_getfontindex, 2, ("After e_wakeup call.\n"));
        }
        else
        {
                current_ptr->int_rcvd++;
        }


        BUGLPR(dbg_dsp_getfontindex, 1, 
		("Leaving mid_intr_gettextfontindex()\n"));

        MID_DD_EXIT_TRACE (dsp_getfontindex, 1, DSP_GET_FONT_INDEX, ddf,
                0,                 /* 1st value ignored by memory trace  */
                event_correlator,  /* Event correlator from DSP status   */
                SB_textfontindex,  /* Status Block text font index value */
                0 );


	return;

}	/* End of mid_intr_gettextfontindex */
