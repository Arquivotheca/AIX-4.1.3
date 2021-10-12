static char sccsid[] = "@(#)48  1.14.1.10  src/bos/kernext/disp/ped/intr/intrswap.c, peddd, bos411, 9428A410j 3/31/94 21:35:05";

/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_intr_swapbuffers
 *              mid_timeout_swap
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
#ifndef  BUGPVT
#define  BUGPVT BUGACT
#endif
#include <sys/systm.h>
#include <sys/errno.h>
#define  Bool unsigned
#include <sys/aixfont.h>
#include <sys/display.h>
#include "midddf.h"
#include "midhwa.h"
#include "hw_regs_k.h"
#include "hw_macros.h"
#include "hw_ind_mac.h"                 /* for Status Control Block access   */
#include "hw_regs_u.h"
#include "rcm_mac.h"                    /* rcm-related macros. Using FIND_GP */
#include "mid_rcm_mac.h"                /* for MID_BREAK */
#include "midwidmac.h"                  /* for MID_MARK_BUFFERS_SWAPPED call */
#include "mid_dd_trace.h"

MID_MODULE (dsp_swap);



/*----------------------------------------------------------------------------

	        M I D _ I N T R _ S W A P B F F E R S

Interrupt Level Swapbuffers (mid_intr_swapbuffers)


DESCRIPTION


long mid_intr_swapbuffers( ddf )
	        midddf_t                *pd;


INPUT PARAMETERS:

pd              Physical display structure.


RETURN VALUES:

none.



PSEUDOCODE:

	enable bus

	MID_RD_FBCSB_ARRAY              - read swap correlators off of adapter

	disable bus

	cleanup structures              - those for which swaps occurred

	MID_MARK_BUFFERS_SWAPPED        - for those WID's which were swapped


----------------------------------------------------------------------------*/




/*----------------------------------------------------------------------------

	SWAP BUFFERS WATCHDOG TIMEOUT ROUTINE

	Here we define the watchdog timeout routine for the swapbuffer
	function.

----------------------------------------------------------------------------*/

void mid_timeout_swap( w )
mid_swap_watch_t        *w;
{
	ddf_SBdata_t    *entry ;

	entry = w-> entry ;

	BUGLPR(dbg_dsp_swap, 0,
	        ("SWAP TIMED OUT, WID = %X\n", entry->window_ID));

	MID_DD_TRACE_PARMS (dsp_swap, 1, DSP_SWAP, (midddf_t *)(w->ddf), 0,
	                0xE0E00E0E, entry-> watch.index, entry-> window_ID ) ;

	MID_BREAK ( DSP_SWAP, w, entry, w->ddf,
	                entry-> watch.index, entry-> window_ID) ;



	/*--------------------------------------------------
	    If we are sleeping, because we have wrapped,
	    then we just do a wakeup.
	--------------------------------------------------*/
	if ( entry-> flags & SWAP_SLEEPING )
	{
	        entry-> flags &= ~SWAP_SLEEPING ;

	        /*--------------------------------------------------
	            Set the received count to (sent - 2) to skip all the
	            missed correlators.   This should also cause the
	            pixel interpretation to be corrected at the next swap.
	        --------------------------------------------------*/
	        if (entry-> corr_sent < 2)
	            entry-> corr_recv = MID_SWAP_CORR_CORRELATION_COUNT_MAX +
	                                entry-> corr_sent - 2 ;
	        else  /* just do a simple decrement */
	            entry-> corr_recv = entry-> corr_sent - 2 ;

	        e_wakeup ( &(entry->sleep_word) ) ;

	}

	else  /* the last swap(s) did not flush */
	{

	        /*--------------------------------------------------
	           Release the WID.
	        --------------------------------------------------*/
	        BUGLPR(dbg_dsp_swap,0,("Call release_guarded_WID \n"));
	        BUGLPR(dbg_dsp_swap,0,("ddf = %X, WID = %X \n",
	                        entry->watch.ddf, entry->window_ID));

	        release_guarded_WID (entry->watch.ddf, entry->window_ID);



	        /*------------------------------------------------------------
	          Re-initialize the swap table entry to its initial values.
	        ------------------------------------------------------------*/
	        BUGLPR(dbg_dsp_swap, 5, ("Cleaning up structures. \n"));

	        entry-> window_ID = MID_WID_NULL;

	        BUGLPR(dbg_dsp_swap,0,("ddf = %X, WID = %X \n",
	                        entry->watch.ddf, entry->window_ID));

	}

}





long mid_intr_swapbuffers( pd )
struct phys_displays *pd;
{
	midddf_t        *ddf = (midddf_t *) pd->free_area;
	short           i, j, k;          /* loop counters                  */
	ddf_swap_data_t swap_correlators; /* hold swap corr. from adapter   */

	ulong           index;          /* swap array index embedded in corr*/
	ulong           function ;      /* swap type, "swap", "A", "B" */
	ulong           correlation_count ;
	ulong           next_in, temp_count  ;

	ulong           CSB_status_word;     /* Context Status Block status */
	ulong           error ;

	mid_wid_status_t *WID_entry ;
	midWG_t         *midWG ;
	mid_wid_t       WID ;

	gscCurrDispBuff_t  newCurrDispBuff;
	int  rc;
	HWPDDFSetup;                    /* gain access to the adapter        */


	MID_DD_ENTRY_TRACE (dsp_swap, 4, DSP_SWAP, ddf, 0, 0, 0, 0 );

	BUGLPR(dbg_dsp_swap, 1, ("Entering mid_intr_swapbuffers. \n"));



	/*---------------------------------------------------------------------
	A call is issued to read a status area on the hardware to obtain all of
	the correlators for buffer swap events that occured on the last
	vertical retrace.  Normal case will be that one buffer swap event is
	being serviced at one time, but it could be as high as 16.  Since up to
	16 applications can be double-buffering simultaneously, the potential
	is there for all 16 buffer swaps to occur on the same vertical retrace
	cycle.
	---------------------------------------------------------------------*/

	BUGLPR(dbg_dsp_swap, 3, ("Calling MID_RD_FBCSB_ARRAY. \n"));

	MID_RD_FBCSB_ARRAY( MID_FBCSB_LENGTH,
	        &swap_correlators.number_of_correlators, 1 );

	BUGLPR(dbg_dsp_swap, 4, ("Number of correlators: 0x%x \n",
	        swap_correlators.number_of_correlators));

	if ( swap_correlators.number_of_correlators > 2 )
	{       /* read all 16 correlators */
	        MID_RD_FBCSB_ARRAY( MID_FBCSB_CORRELATOR_1_2,
	                &swap_correlators.correlator[0], 8 );
	} else
	{       /* read first 2 correlators */
	        MID_RD_FBCSB_ARRAY( MID_FBCSB_CORRELATOR_1_2,
	                &swap_correlators.correlator[0], 1 );
	}

	BUGLPR(dbg_dsp_swap, 1, ("Correlator #0 =: 0x%x \n",
	        swap_correlators.correlator[0]));
	BUGLPR(dbg_dsp_swap, 5, ("Correlator #1 =: 0x%x \n",
	        swap_correlators.correlator[1]));
	BUGLPR(dbg_dsp_swap, 5, ("Correlator #2 =: 0x%x \n",
	        swap_correlators.correlator[2]));
	BUGLPR(dbg_dsp_swap, 5, ("Correlator #3 =: 0x%x \n",
	        swap_correlators.correlator[3]));
	BUGLPR(dbg_dsp_swap, 5, ("Correlator #14 =: 0x%x \n",
	        swap_correlators.correlator[14]));
	BUGLPR(dbg_dsp_swap, 5, ("Correlator #15 =: 0x%x \n",
	        swap_correlators.correlator[15]));





	/*-----------------------------------------------------------------
	Determine which of the swapbuffer requests pending have been
	serviced, call MID_MARK_BUFFERS_SWAPPED to change the pixel
	interpretation for that window ID, and re-initialize any structures
	used in preparation for the next call.
	-----------------------------------------------------------------*/
#define CORR       swap_correlators.correlator

	for( j=0; j<swap_correlators.number_of_correlators; j++)

	/*-----------------------------------------------------------------
	for the number of swap correlators pulled from the adapter ...
	-----------------------------------------------------------------*/
	{

	        BUGLPR(dbg_dsp_swap, 4, ("correlator[%d]=0x%x. \n",j,
	                swap_correlators.correlator[j]));


	        /*----------------------------------------------------------
	        Isolate least significant 4 bits.  This is the array index
	        into the hold list which corresponds to the swap we are
	        currently servicing.

	        Isolate bits 9-11. This indicates the swap function.

	        Finally, isolate bits 0-7. This is the correlation count.
	        ----------------------------------------------------------*/

	        index = CORR[j] & MID_SWAP_CORR_INDEX;
	        function = CORR[j] & MID_SWAP_CORR_FUNCTION_ID ;
	        correlation_count = (CORR[j] & MID_SWAP_CORR_CORRELATION_COUNT)
	                                        >> 8 ;


	        BUGLPR(dbg_dsp_swap, 1,
	           ("correlator[%d]=0x%x window_ID[%d]=0x%x\n",
	                j, swap_correlators.correlator[j], index,
	                ddf->ddf_data_swapbuffers_hold_list[index].window_ID));

	        BUGLPR(dbg_dsp_swap, 1, ("function = %X,  corr = 0x%X \n",
	                                  function, correlation_count ));




	        /*-------------------------------------------------------
	        If the j'th correlator from adapter matches any
	        correlator in the hold list, then clean up
	        corresponding structures, since it has just
	        been swapped.
	        -------------------------------------------------------*/





	        /*-------------------------------------------------------------
	        Determine what type of swap call we are servicing.

	        The three frame buffer functions handled are:
	                1) "swap" - We really swapped buffers.
	                2) "set display buffer" (to either A or B)
	                3) "set draw buffer" (essentially a NOP with respect
	                        to this module -- nothing else to do.
	        -------------------------------------------------------------*/


#define SWAP_LIST  ddf->ddf_data_swapbuffers_hold_list

	        WID = SWAP_LIST[index].window_ID ;


	        MID_DD_TRACE_PARMS (dsp_swap, 1, DSP_SWAP, ddf, 0,
	                (j << 24) | (function << 16) | CORR[j],
	                (correlation_count << 16 ) | index, WID ) ;



	        /* -----------------------------------------------------------
	            Error checks:
	              . We (may still) have an initialization case, where the
	                 WG is not yet valid.  (WID = D0GF000D indicates this.)
	              . The adapter has been known to send NULL correlators.
	              . The adapter has also been known to send old correlators
	                 (we can only detect if the index is currently unused).
	         * -----------------------------------------------------------*/
	        if ( (CORR[j] != NULL) &&
	             (WID != 0xD06F000D ) &&
	             (WID != MID_WID_NULL) )
	        {



	        /* -----------------------------------------------------------
	            UPDATE the frame buffer flag (in WID entry and midWG)

	             Note that if the window geometry pointer has been
	             NULLed out in the WID entry, it is because the window
	             has been deleted.

	             In this event, there is no need to attempt to track the
	             pixel interpretation (frame buffer being displayed).
	             We are simply flushing the outstanding swap requests.
	         * -----------------------------------------------------------*/

	            WID_entry = &(ddf->mid_wid_data.mid_wid_entry[WID]) ;
	            midWG = WID_entry-> pwidWG ;

	            if (midWG != NULL)
	            {

	                MID_DD_TRACE_PARMS (dsp_swap, 2, DSP_SWAP, ddf, j,
	                        midWG->pWG,
	                        MID_DDT_WID(midWG->wid), MID_DDT_PI(midWG));


	                switch (function)
	                {
	                case MID_SWAP_JUST_SWAP :
	                    BUGLPR(dbg_dsp_swap, 1, ("Function = SWAP \n" ));
	                    /*------------------------------------------------
	                       Then we have a "swap" type swapbuffers call
	                    ------------------------------------------------*/
	                    WID_entry-> pi.pieces.flags ^=MID_USING_BACK_BUFFER;
	                    midWG-> pi.pieces.flags ^= MID_USING_BACK_BUFFER ;

	                    break ;



	                case MID_SWAP_DISPLAY_BUFFER :
	                    BUGLPR(dbg_dsp_swap, 1, ("Function = DISPLAY \n" ));
	                    /*------------------------------------------------
	                       Buffer was specified directly
	                    ------------------------------------------------*/
	                    if ( CORR[j] & MID_SWAP_CORR_FRAME_BUFFER )
	                    {
	                        WID_entry-> pi.pieces.flags |=
	                                        MID_USING_BACK_BUFFER ;
	                        midWG-> pi.pieces.flags |=
	                                        MID_USING_BACK_BUFFER ;

	                    }
	                    else  /* Frame buffer A specified */
	                    {
	                        WID_entry-> pi.pieces.flags &=
	                                        ~MID_USING_BACK_BUFFER ;
	                        midWG-> pi.pieces.flags &=
	                                        ~MID_USING_BACK_BUFFER ;

	                    }

	                    break ;

	            }   /* end of switch */





	            /*---------------------------------------------------------
	                Now check if the adapter missed any FBC requests.
	                If so, correct the "displayed frame buffer flag".
	            ---------------------------------------------------------*/

	            next_in = SWAP_LIST[index].corr_recv + 1;

	            if (next_in > MID_SWAP_CORR_CORRELATION_COUNT_MAX)
	                next_in = 0 ;

	            SWAP_LIST[index].corr_recv = correlation_count ;


	            BUGLPR(dbg_dsp_swap, 1, ("next in = %X \n", next_in));

	            if (correlation_count != next_in)
	            {
	                BUGLPR(dbg_dsp_swap,1,("ADAPTER MISSED SWAP\n\n") );
	                /*-----------------------------------------------------
	                 The adapter did screw up, so copy the FB flag from the
	                 correlator into the data structures.
	                -----------------------------------------------------*/
	                if ( CORR[j] & MID_SWAP_CORR_FRAME_BUFFER )
	                {
	                        WID_entry-> pi.pieces.flags |=
	                                        MID_USING_BACK_BUFFER ;
	                        midWG-> pi.pieces.flags |=
	                                        MID_USING_BACK_BUFFER ;
	                }
	                else  /* Frame buffer A specified */
	                {
	                        WID_entry-> pi.pieces.flags &=
	                                        ~MID_USING_BACK_BUFFER ;
	                        midWG-> pi.pieces.flags &=
	                                        ~MID_USING_BACK_BUFFER ;
	                }

	            }   /* end of correlator mismatch */



	            /*---------------------------------------------------------
	                Now update the "Current Displayed Buffer" field in
	                 user space (for getImage, et.al.)

	                If there is a pointer to user space, and the current
	                displayed buffer has changed, write the new displayed
	                buffer to the user space memory.
	            ---------------------------------------------------------*/

	            /*  compute the new buffer setting */
	            newCurrDispBuff.buffer =
	                     (midWG->pi.pieces.flags & MID_USING_BACK_BUFFER) ?
	                        gsc_DispBuff_B                                :
	                        gsc_DispBuff_A                                ;

	            if (midWG->CurrDispBuff.buffer != newCurrDispBuff.buffer &&
	                midWG->CDBaddr.eaddr != NULL     )
	            {
	                midWG->CurrDispBuff.buffer = newCurrDispBuff.buffer;

	                RCM_UPDATE_CURR_DISP_BUFF (
	                        ddf->pdev,
	                        midWG->CDBaddr,
	                        &(newCurrDispBuff.buffer),
	                        rc);

	                /* if validation failed, midWG->CDBaddr.eaddr will have
	                   been cleared, but not if the usr spc write failed */
	                if (rc != XMEM_SUCC)
	                {
	                    BUGLPR(dbg_dsp_swap, 0,
	                        ("CURR_DISP_BUFF failed, rc = 0x%8x\n", rc)) ;
	                }
	            }

	        }    /* end of midWG = NULL check */






	        /*-------------------------------------------------------------
	            Check if the app is waiting for the adapter to catch up.
	            If so, then check if the adapter has, in fact caught up.
	            (If it has, wakeup the app.)
	        -------------------------------------------------------------*/


	        if ( SWAP_LIST[index].flags & SWAP_SLEEPING )
	        {
	                /*-----------------------------------------------------
	                  Get the current sent count and correct for wrap.
	                -----------------------------------------------------*/
	                temp_count = SWAP_LIST[index].corr_sent ;

	                if ( temp_count < correlation_count )
	                     temp_count += MID_SWAP_CORR_CORRELATION_COUNT_MAX ;


	                /*-----------------------------------------------------
	                  Now check if the adapter has caught up (far enough).
	                  If so, reset the SLEEPING flag (so the app knows this
	                  is the "right" wakeup) and do the wakeup.
	                -----------------------------------------------------*/
	                if ( (correlation_count +
	                        MID_SWAP_CORR_CORRELATION_COUNT_WAKEUP) >
	                        temp_count )
	                {
	                        SWAP_LIST[index].flags &= ~SWAP_SLEEPING ;

	                        MID_DD_TRACE_PARMS (dsp_swap, 3,DSP_SWAP,ddf,0,
	                                0xFF00 | CORR[j],
	                                ((temp_count) << 16 ) |
	                                        SWAP_LIST[index].flags,
	                                        &(SWAP_LIST[index]) ) ;

#if 0
	                        brkpoint ( &(SWAP_LIST[index]),
	                                SWAP_LIST[index].corr_sent,
	                                temp_count,
	                                correlation_count ) ;
#endif

	                        e_wakeup ( &(SWAP_LIST[index].sleep_word) ) ;
	                }
	        }





	        /*-------------------------------------------------------------
	            Now check if the adapter has caught up to the processor
	            (in Frame buffer requests).

	            Note that if the app is still sleeping (waiting to send
	            the next swap request) we cannot release the WID.
	        -------------------------------------------------------------*/

	        if ( (correlation_count == SWAP_LIST[index].corr_sent) &&
	                !( SWAP_LIST[index].flags & SWAP_STILL_USING_WID) )
	        {
	                /*--------------------------------------------------
	                    If so, then this WID and swap structure are no
	                    longer in use by any swap requests and can be
	                    freed up.  First release WID.
	                --------------------------------------------------*/
	                BUGLPR(dbg_dsp_swap,3,("Call release_guarded_WID \n"));

	                release_guarded_WID(ddf, WID);



	                /*-------------------------------------------
	                Re-initialize the  "ddf_data_swapbuffers_hold_
	                list_t"  data structure to initial values to
	                free up for next swapbuffers call.

	                Also stop the watchdog timer.
	                -------------------------------------------*/
	                BUGLPR(dbg_dsp_swap, 5, ("Cleaning up structures. \n"));

	                SWAP_LIST[index].window_ID = MID_WID_NULL;


#define ENABLE_WATCHDOGS
#ifdef  ENABLE_WATCHDOGS  /* Disable the watchdog timers.  They can cause   */
	                  /* problems when the adapter is legitimately      */
	                  /* taking more time than the watchdog is set for. */

	                w_stop ( &(SWAP_LIST[index].watch.dog) ) ;
#endif

	        }
	        /* else there are more FBC commands pending in the FIFO */


	    }  /* NULL correlator and WID check */
	    else /* BAD */
	    {
	        BUGLPR(dbg_dsp_swap,1,("correlator %X ignored, WID = %x\n\n",
	                                        CORR[j], WID) ) ;
	    }
	}  /* for # of swap correlators  */


	BUGLPR(dbg_dsp_swap, 1, ("Leaving mid_intr_swapbuffers. \n"));


	MID_DD_EXIT_TRACE (dsp_swap, 4, DSP_SWAP, ddf, ddf, midWG, 0, 0xF0) ;

	return (0);

}       /* end of mid_intr_swapbuffers */
