static char sccsid[] = "@(#)44  1.3  src/bos/kernext/disp/ped/intr/intrendr.c, peddd, bos411, 9428A410j 5/13/94 14:55:24";
/*
 * COMPONENT_NAME: PEDDD
 *
 * FUNCTIONS: mid_timeout_endrender, mid_intr_endrender
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/types.h>
#include <sys/intr.h>
#include <sys/errids.h>
#include "midddf.h"
#include "mid_dd_trace.h"

MID_MODULE ( dsp_endrender );	/* defines trace variable		     */

ERR_REC(10)	ER;		/* defines error log template		     */

/*----------------------------------------------------------------------------

FUNCTION NAME:  mid_timeout_endrender

FUNCTION:  This routine is the watchdog timer handler function for 
	   mid_endrender().  When the watchdog timer pops, this handler gets
	   called and wakes up the process that invoked mid_endrender().

RESTRICTIONS:  None

DEPENDENCIES:  None

INPUT:   w - pointer to the watchdog timer data structure for the timer
	     that popped

OUTPUT:  None

RETURNS: None

-----------------------------------------------------------------------------*/

mid_timeout_endrender( w )
	mid_watchdog_data_t *w;
{

        BUGLPR(dbg_dsp_endrender, 1, ("End render watchdog timer popped\n"));
        BUGLPR(dbg_dsp_endrender, 3, ("Process wakeup addr = 0x%x\n",
				      *(w->sleep_addr)));
        e_wakeup( w->sleep_addr );

}

/*----------------------------------------------------------------------------

FUNCTION NAME:  mid_intr_endrender

FUNCTION:  This routine is the interrupt handler for the End Rendering
	   command element executed interrupt.  This routine wakes up the
	   process that issued the corresponding end render request.

RESTRICTIONS:  None

DEPENDENCIES:  None

INPUT:   ddf	    - pointer to device dependent physical device structure
	 correlator - correlator associated with current interrupt

OUTPUT:  None

RETURNS: None

-----------------------------------------------------------------------------*/

void mid_intr_endrender(ddf, correlator)
        midddf_t	*ddf;
        ushort          correlator; 
{
        ddf_data_t	*current_ptr;	/* used to search linked list of    
					   pending end render requests	     */
	int		error = FALSE;	/* indicates whether an error has 
					   occurred			     */

        MID_DD_ENTRY_TRACE (dsp_endrender, 1, DSP_END_RENDER, ddf,
			    0, correlator, 0, 0);
        BUGLPR(dbg_dsp_endrender, 1, ("Entering mid_intr_endrender\n"));
        BUGLPR(dbg_dsp_endrender, 3, ("ddf = 0x%x\n", ddf));
        BUGLPR(dbg_dsp_endrender, 3, ("correlator = 0x%x\n", correlator));

        /*---------------------------------------------------------------------
	Get a pointer to the first pending end render request.
	---------------------------------------------------------------------*/

	current_ptr = ddf->ddf_data_endrender;

        /*--------------------------------------------------------------------
	Find the end render request that corresponds to this interrupt.  The
        request will have a correlator that matches the correlator associated
	with this interrupt.
	---------------------------------------------------------------------*/

        while (current_ptr->correlator != correlator) {

                if (current_ptr->next != NULL) {

                	/*----------------------------------------------------
			Try the next pending end render request.
                	-----------------------------------------------------*/

			current_ptr = current_ptr->next;

		} else {

                	/*----------------------------------------------------
                	We are at the end of the list of pending end render
			requests and have not found the request that 
			corresponds to this interrupt.  This should never
                	happen.  Log an error.
                	-----------------------------------------------------*/

        		BUGLPR(dbg_dsp_endrender, 1, 
			       ("Correlating end render request not found\n"));

			ER.error_id = ERRID_MID_SW;
			strcpy(ER.resource_name, "MIDDD");
			sprintf(ER.detail_data, " %d ", ddf->slot);

			/* Call system error logging routine */
			errsave(&ER, ERR_REC_SIZE + (strlen(ER.detail_data)+1));
			
                        error = TRUE;
			break;
                }
        }

        /*--------------------------------------------------------------------
	Continue if the end render request that corresponds to this interrupt
	was found.
	---------------------------------------------------------------------*/

	if ( !error ) {

		/*------------------------------------------------------------
		Wake up the process that issued the end render request if the
		process is sleeping.
		-------------------------------------------------------------*/

		if (current_ptr->sleep_flg) {

			/*----------------------------------------------------
			The process is asleep so wake it up.  Note that the 
			process may not actually be asleep because of a bug
			in e_sleep.  So set sleep_flg to MID_DO_NOT_SLEEP so
			that when the process resumes execution in 
			mid_endrender() it does not issue another e_sleep call.
			-----------------------------------------------------*/

                	current_ptr->sleep_flg = MID_DO_NOT_SLEEP;

			BUGLPR(dbg_dsp_endrender, 4, 
			       ("About to wake up process, addr = 0x%x\n",
				current_ptr->wakeup_address));

			e_wakeup(&current_ptr->wakeup_address);

			BUGLPR(dbg_dsp_endrender, 4, ("Process awake\n"));

		} else {

			/*----------------------------------------------------
			The process has not tried to go to sleep yet because
			it has not gotten that far in mid_endrender().  This
			will happen when this interrupt is received before
			the process has a chance to disable interrupts after
			issuing the End Rendering SE to the adapter.  In
			this case, set the int_rcvd flag so that the process
			will know that this interrupt has already been
			received when it resumes execution in mid_endrender().
			-----------------------------------------------------*/

			current_ptr->int_rcvd++;

			BUGLPR(dbg_dsp_endrender, 4, 
			       ("Process did not sleep\n"));
		}
	}

        MID_DD_EXIT_TRACE(dsp_endrender, 2, DSP_END_RENDER, ddf,
        	          0, error, current_ptr->sleep_flg,
			  current_ptr->wakeup_address);
        BUGLPR(dbg_dsp_endrender, 2, ("Leaving mid_intr_endrender\n"));
}
