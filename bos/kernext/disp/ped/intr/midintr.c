static char sccsid[] = "@(#)45  1.12.1.11  src/bos/kernext/disp/ped/intr/midintr.c, peddd, bos411, 9428A410j 3/31/94 21:35:11";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_intr
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



/******************************************************************************
   Includes:
 ******************************************************************************/

#include <sys/types.h>
#include <sys/intr.h>
#include <sys/syspest.h>
#include <sys/display.h>

#include "hw_dd_model.h"
#include "midddf.h"
#include "midctx.h"             /* contains DMA_COMPLETE flags */

#define NO_PIO_EXC      1               /* DEFEAT PIO EXCEPTION HANDLING */

#include "midhwa.h"
#include "hw_regs_k.h"          /* BIM status bit definitions */
#include "hw_regs_u.h"          /* BIM status bit definitions */
#include "hw_addrs.h"           /* BIM status bit definitions */
#include "hw_macros.h"
#include "hw_ind_mac.h"                 /* for Status Control Block access   */

#include "mid_pos.h"
#include "mid_dd_trace.h"
#include "mid_rcm_mac.h"

MID_MODULE (midintr);
#define    dbg_midddi   dbg_midintr



/***************************************************************************
   Externals defined herein:
 ***************************************************************************/
long mid_intr( struct phys_displays *);






/******************************************************************************
	
	                       Start of Prologue

   Function Name:    midintr

   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
	                SLIH (Second Level Interrupt Handler)

 *--------------------------------------------------------------------------*

   Function:

	This is the interrupt handler for the Mid-level graphics adapter.
	Its first task is to determine whether the interrupt was generated
	by the adapter serviced by this routine.  Note that there might be
	multiple graphics adapters on the bus; in fact, there might be
	multiple graphics adapters of the same type (eg. mid-level).
	In this event, a separate SLIH exists for each.

	After the adapter determination, this routine is just a big switch.
	Specific interrupt handling routines are called by this
	function to handle specific interrupts.

	Invoked by an adapter interrupt,  "mid_intr()"  determines what type
	of interrupt it is and calls one of several interrupt functions to
	service the interrupt.



 *--------------------------------------------------------------------------*

IMPLEMENTATION:

Assign the local  "ddf"  structure pointer to point to the free area in the
"phys_displays"  structure passed in.

Determine if our adapter caused the interrupt that led to the call of this
function by reading the interrupt status register (dsp_status) on the adapter.
If the interrupt is not from our adapter, return FAIL.  If the interrupt is
from our adapter, then we must determine which DDF function has just been
serviced and call the appropriate interrupt handler.

NOTE:  It is not yet clear how this will be done.  If we do not get enough
       information from the adapter to determine which DDF function just got
       serviced, then we must change the structure of the correlator.  That
       is, we must use the top 4-bits of the correlator to associate the
       correlator and data with the DDF call.

Call the appropriate interrupt handler, passing a pointer to the  "midddf_t"
structure.

 *--------------------------------------------------------------------------*

    Restrictions:
	          none

    Dependencies:
	1.  The rendering context, window geometry structure and
	    window attribute structure are all valid.


 *--------------------------------------------------------------------------*

   Linkage:
	   This function is an aixgsc call and, more or less, one of the
	   GAI RMS functions as well.

	   This function begins as a SetContext call to the RMS layer.
	   Both bind_window and set_rcx can be invoked as a result of a
	   SetContext depending on the particular transitions being made.
	   Refer to the GAI RMS layer spec for more information on this.

	   When the RMS layer translates the SetContext into an
	   aixgsc (BIND_WINDOW) call.  This invokes the device independent
	   RCM who in turn gives control to the appropriate device dependent
	   bind_window routine.

 *--------------------------------------------------------------------------*

    INPUT:

	pd -    Physical Displays structure pointer.  There is a structure
	        of type  "phys_displays"  allocated for each physical display,
	        ie, it is a per-physical-display structure.  There exists a
	        free area in this structure used by each of the DDF functions
	        which holds pointers to data specific to these functions.  It
	        is defined in inc/sys/display.h.


    OUTPUT:  none
	
    RETURN CODES:  none
	
	
	                         End of Prologue
 ******************************************************************************/




long mid_intr( pd )
struct phys_displays *pd;
{
	midddf_t        *ddf = (midddf_t *) pd->free_area;

	ulong           free_space = 0x0bad0bad;        /* free space check */

	ulong           status_read ;
	ulong           bim_status,
	                dsp_status,
	                mask,
	                fifo_size;

	int             rc;


	HWPDDFSetup;                    /* gain access to the hardware */




    /*------------------------------------------------------------------*
       Trace the entry parms
     *------------------------------------------------------------------*/

	MID_DD_ENTRY_TRACE (midintr, 2, INTERRUPT, ddf, pd, pd, ddf, 0 ) ;

	BUGLPR(dbg_midddi, 2, ("entering mid_intr\n"));



	/*-----------------
	This routine will be invoked when an interrupt occurs on the level
	passed to i_init routine by the device init routine.  This routine
	must look at the adapter and verify that it was the cause of the
	interrupt.  If it caused the interrupt, it will process any tasks
	which must be done on the interrupt level, such as counting
	interrupts when in diagnostic mode.  If it did not cause the interrupt,
	it will return to the caller a code (INTR_FAIL) indicating this adapter
	didn't cause the interrupt.
	-----------------*/



	/*********************************************************************
	   Set up the I/O (bus) access
	 *********************************************************************/


	BUGLPR(dbg_midddi, 3, ("*** set POS to 300.\n"));


	IPIO_EXC_ON();

	MID_SLOW_IO (ddf) ;	              /* Set the BIM speed to 300 */


	/*********************************************************************
	   Determine whether the adapter associated with this interrupt
	   handler caused this interrupt.
	*********************************************************************/

	/*-------------------------------------------------------------------*
	   Read the interrupt status registers on the adapter.
	 *-------------------------------------------------------------------*/

	MID_RD_HOST_STAT (status_read);
	BUGLPR(dbg_midddi, 2, ("bim status read = 0x%X \n", status_read ));

	MID_RD_HOST_INTR (mask);
	BUGLPR(dbg_midddi, 2, ("bim mask 0x%x \n",mask));

	MID_RD_FIFO_FREE (free_space) ;

	MID_DD_TRACE_PARMS (midintr, 1, INTERRUPT_2, ddf, pd,
	                        status_read, mask, free_space ) ;

	bim_status = status_read & mask;
	BUGLPR(dbg_midddi, 2, ("status read & mask = 0x%X \n", bim_status));
	
	if (bim_status == 0)
	{
	        /*-----------------------------------------------------------*
	           This interrupt is not from our adapter.
	           Reset the bus and leave.
	         *-----------------------------------------------------------*/

	        BUGLPR(dbg_midddi, 3, ("*** reset POS to 200(rtn).\n"));
	
	        MID_FAST_IO (ddf) ; 	     /* Set the BIM speed to 200 */
	        IPIO_EXC_OFF();

	        BUGLPR(dbg_midddi, 2, ("not our interrupt. exit mid_intr\n\n"));
	        return(INTR_FAIL);
	}




	

	/*********************************************************************
	 *********************************************************************

	   This interrupt is from our adapter

	   First, save the last interrupt status in ddf.  Then clear the
	   interrupt bits.

	 *********************************************************************
	*********************************************************************/
	BUGLPR(dbg_midddi, 1,
	        ("INTERRUPT:  0x%X, mask = 0x%X \n", status_read, mask));

	ddf->bim_status = bim_status;
	ddf->raw_bim_status = status_read ;


	/*-------------------------------------------------------------------*
	   Clear interrupt status immediately after done using dsp_commo value.
	   Write ones to set them to zeroes.
	 *-------------------------------------------------------------------*/

	MID_WR_HOST_STAT (bim_status);





	/*********************************************************************
	   Check for a diagnostics interrupt
	*********************************************************************/

	if ( ddf->hwconfig & MID_CONFIG )
	{
	        MID_RD_CARD_COMO (dsp_status);
	        ddf->dsp_status = dsp_status;

	        if ( bim_status & ddf->a_event_mask )
	        {
	                mid_intr_hostevt ( pd, bim_status, dsp_status );

	                ddf->eventcmd = 0;

	                ddf->a_event_mask &= ~(bim_status &
	                                        ddf->a_event_mask);
	        }
	        else
	        {
	                dsp_status &= 0xFFFF0000;

	                if ( dsp_status == MID_DSPC_DMA_COMPLETE )
	                        {
	                                BUGLPR(dbg_midddi, 1,
	                                ("Invoking DMA callback function.\n"));

	                                if ( ( ddf->dcallback ) != NULL )
	                                ( ddf->dcallback )( pd->cur_rcm,0 );

	                                ddf->dcallback = NULL;

	                                ddf->dma_result = MID_DMA_NOERROR;

	                                if ( ddf->cmd == DMA_WAIT_REQ )
	                                {
	                                /* ***********************************/
	                                /* Need to wake up process waiting   */
	                                /* for DMA complete in middma.c      */
	                                /*************************************/

	                                        BUGLPR(dbg_midddi, 1,
	                                        ("Waking up process.\n"));

	                                        ddf->dma_sleep_flags = 0x10;

	                                        e_wakeup(&(ddf->dma_sleep_addr));
	                                }
	                        }
	        }

	        BUGLPR(dbg_midddi, BUGNTA, ("*** reset POS to 200.\n"));

	        MID_FAST_IO (ddf) ;   	   /* Set the BIM speed to 200 */
	        IPIO_EXC_OFF();

	        i_reset(pd);

	        return( 0 );
	}





	/*********************************************************************
	   Check for the BIM status in the following order:
	    . Bad parity
	    . DSP status
	    . context switch completion status
	    . High water mark
	    . Low water mark
	*********************************************************************/


	if (bim_status & MID_K_HOST_IO_CHANNEL_CHECK )
	{
	        BUGLPR(dbg_midddi, 0, ("ADAPTER REPORTED CHANNEL CHECK!!\n\n"));
	        MID_RD_ASCB_VALUE( 3, fifo_size );
	        MID_BREAK (INTERRUPT, ddf,pd,status_read,bim_status,fifo_size);
	}




	/*---------------------------------------------------*
	  CONTEXT SWITCH COMPLETION

	  NOTE:  Context switch is normally NOT handled as an
	         interrupt.  See intr_switch_done for details.
	 *---------------------------------------------------*/

	if (bim_status & MID_K_HOST_IO_DSP_SOFT_INTR0 )
	{
	        rc = intr_switch_done (pd, ddf) ;
	}



	if (bim_status & MID_K_HOST_IO_PIO_HI_WATER_HIT )
	{
	        rc = intr_fifo_hwm (pd, ddf) ;
	}




	if (bim_status & MID_K_HOST_IO_LOW_WATER_MARK )
	{
	        rc = intr_fifo_lwm (pd, ddf) ;

	}



	if (bim_status & MID_K_HOST_IO_WRITE_DSP_COMMO_REG )
	{

	        /*---------------------------------------------------*
	          Read the DSP status value
	         *---------------------------------------------------*/

	        MID_RD_CARD_COMO (dsp_status);
	        ddf->dsp_status = dsp_status;

	        BUGLPR(dbg_midddi, 1, ("dsp status =  0x%X \n", dsp_status));


	        rc = intr_dsp (pd, ddf, dsp_status) ;

	        /*-------------------------------------------*
	          If an error was reported by the adapter,
	           eventually reload and init the adapter.
	         *-------------------------------------------*/
	        if (rc == -1)
	        {
	        }

	}



	/*---------------------------------------------------*
	  The FIFO available interrupt is not handled here.

	  In the normal case, we wait for it inline in midswitch.c
	  (up to 20 u secs).  If the FIFO is not available at that
	  time, then we have a heavy context switch.  The
	  completion for that is the "new context processing complete"
	  DSP status, which actually occurs just AFTER the
	  FIFO available.  Note, that we do not want to handle two
	  interrupt in that case, so we do the FIFO available
	  processing at that (new context proc complete) time.
	 *---------------------------------------------------*/

#if 0
	if (bim_status & MID_K_HOST_IO_PIO_FIFO_AVAILABLE )
	{

	        rc = intr_fifo_av (pd, ddf) ;
	}
#endif






	/*********************************************************************
	   Clean up the I/O (bus) access
	 *********************************************************************/

	BUGLPR(dbg_midddi, BUGNTA, ("*** reset POS to 200.\n"));

	MID_FAST_IO (ddf) ;     	         /* Set the BIM speed to 200 */

	IPIO_EXC_OFF();

	i_reset(pd);




	/*********************************************************************
	   Trace at the exit point
	 *********************************************************************/

	MID_DD_EXIT_TRACE (midintr, 3, INTERRUPT, ddf, pd, ddf, 0, 0xF0);


	BUGLPR(dbg_midddi, 2, ("Leaving mid_intr()\n"));
	return(INTR_SUCC);

}


