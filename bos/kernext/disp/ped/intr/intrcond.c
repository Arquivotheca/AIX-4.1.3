static char sccsid[] = "@(#)46  1.11  src/bos/kernext/disp/ped/intr/intrcond.c, peddd, bos411, 9428A410j 3/31/94 21:34:46";

/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_intr_getcondition
 *		mid_timeout_getcondition
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
#include "hw_ind_mac.h"		/* for Status Control Block access           */
#include "hw_regs_u.h"
#include "hw_seops.h" 		/* needed for MID_WR_HOST_COMO() to work     */
#include "rcm_mac.h"            /* rcm-related macros.  Using FIND_GP        */
#include "midwidmac.h"		/* needed for MID_MARK_BUFFERS_SWAPPED call  */

#include "mid_dd_trace.h"       /* for memory trace                          */
MID_MODULE ( dsp_getcondition );



/*----------------------------------------------------------------------------
Define watchdog timer handler function for mid_getcondition().
When the watchdog timer pops, the appropriate handler gets called
and wakes up the process at a point where it can recover.
----------------------------------------------------------------------------*/

mid_timeout_getcondition( w )
mid_watch_getcondition_t *w;
{

        BUGLPR(dbg_dsp_getcondition,0 ,
          ("TIMED OUT  ---  CALLING WATCHDOG TIMER:   mid_watch_getcond()\n"));
        BUGLPR(dbg_dsp_getcondition,0 , 
		("    w->sleep_addr=0x%x\n", w->sleep_addr ));

        e_wakeup( w->sleep_addr );

}



/*---------------------------------------------------------------------*/
/*                                                                     */
/* IDENTIFICATION: MID_INTR_GETCONDITION                               */
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

/*---------------------------------------------------------------------------

                M I D _ I N T R _ G E T C O N D I T I O N




Interrupt Level Get Current 3DM1 status register (mid_intr_getcondition)


DESCRIPTION

Reads 3DM1 status word off the adapter.  This data is used by the caller 
(GraPHIGS) to determine status on up to 32 separate events and values, such as
culling and pruning.

long mid_intr_getcondition( ddf )
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
	
	Read current condition status word off of adapter

	Disable bus

	Locate structure associated w/ current get_condition interrupt, and ...

	Assign current condition into this structure

	Wake up sleeping process

	Return
	
---------------------------------------------------------------------------*/

long mid_intr_getcondition( ddf )
                midddf_t                *ddf;
{
	ulong		condition_flag; /* MOD1 condition flag value         */
	ushort		ga_correlator;  /* MOD1 General Attribute correlator */
        ushort          event_correlator; /* store corr value from SCB       */
        short           tmp_correlator;   /* from getcondition linked list   */
        ddf_data_getcondition_t  *tmp_next_ptr; /* ptr to next array element */
        ddf_data_getcondition_t  *current_ptr;  /* ptr to current struct     */
        short           i, k;			/* loop counters / indices   */
        ulong           screen_char_position; /* local var. for X-Y pair    */

        HWPDDFSetup;                /* gain access to hardware pointer      */

        BUGLPR(dbg_dsp_getcondition, 1, ("Entering mid_intr_getcondition()\n"));



	/*--------------------------------------------------------------------
	Set current event correlator variable to the correlator of the 
	event which we are currently servicing.  The value was saved 
	previously in the "ddf" structure by the caller of this function.
	The high 2 bytes are AND'ed out since we want only the correlator
	value itself.
	--------------------------------------------------------------------*/

	event_correlator = (0x0000FFFF & ddf->dsp_status);
	BUGLPR(dbg_dsp_getcondition, 4, ("event_correlator=0x%x \n", 
		event_correlator));

        MID_DD_ENTRY_TRACE (dsp_getcondition, 1, DSP_GET_CONDITION, ddf,
                0, event_correlator, 0, 0 );



	/*--------------------------------------------------------------------
	Call hardware macro to read 3DM1 Status Control Block data into  
	"midddf_t"  data structure.  This data includes a 32-bit unsigned 
	integer (32 flags) and two operation correlators.   One of the 
	correlators was passed in by the caller on a  "mid_setcondition()"  
	call.  This value is only used by the caller to associate a particular  
	"MID_setcondition()"  call with output from a particular  
	"MID_getcondition()"  call.   The other correlator was generated by
	"mid_getcondition()"  and will be used to associate this particular 
	set of data with the  "mid_getcondition()"  call that is responsible 
	for it.  In other words, one of the correlators is used by the caller 
	to correlate calls with data, and the other is used by the device 	
	driver to correlate calls with data.  This simply ensures that if 
	another call to  "mid_getcondition()"  occurs before the information 
	for the first call gets back to the first caller, that we have a way 
	of making a one-to-one mapping between caller and results.  The integer 
	status word is deposited in the  "mid_getcondition_t"  structure set up 
	by the original caller.  There is a pointer to this structure in the 
	"midddf_t" structure so that the interrupt handler knows where to 
	place the data.
	--------------------------------------------------------------------*/

	MID_RD_M1SB_VALUE( MID_M1SB_CONDITION_FLAG, condition_flag );

	BUGLPR(dbg_dsp_getcondition, 2, ("From adapter: \n" ));
	BUGLPR(dbg_dsp_getcondition, 2, ("   condition_flag=0x%x: \n",
		condition_flag ));


	MID_RD_M1SB_VALUE( MID_M1SB_CORRELATOR, ga_correlator );

	BUGLPR(dbg_dsp_getcondition, 2, ("   ga_correlator =0x%x: \n",
		ga_correlator  ));

        MID_DD_TRACE_PARMS (dsp_getcondition, 1, DSP_GET_CONDITION_PARMS, ddf,
                0, event_correlator, condition_flag, ga_correlator );



		
        /*---------------------------------------------------------------------
        Match correlator value from Status Control Block on the adapter
        with correlator(s) in "ddf_data_getcondition" structure.

	event_correlator corresponds to the interrupt being serviced.
	tmp_correlator cycles through the available correlators until it
	finds a match.
	--------------------------------------------------------------------*/

        i = 0;
	current_ptr    = &ddf->ddf_data_getcondition[i];
        tmp_next_ptr   = ddf->ddf_data_getcondition[i].next;
        tmp_correlator = ddf->ddf_data_getcondition[i].correlator;

        BUGLPR(dbg_dsp_getcondition, 4, 
		("tmp_correlator = 0x%x, event_correlator=0x%x\n", 
		tmp_correlator, event_correlator));



        while (tmp_correlator != event_correlator)
        {

       		BUGLPR(dbg_dsp_getcondition, 4, 
			("tmp_correlator = 0x%x, event_correlator=0x%x\n", 
			tmp_correlator, event_correlator));


                /*-----------------------------------------------------------
                If tmp_next_ptr is NULL, then we have looked through
                all the correlators we have for interrupts of this type and
                have not found a match for the correlator we received on the
                interrupt.  This should never happen.
                -----------------------------------------------------------*/

                if (tmp_next_ptr == NULL)
                {
                        BUGLPR(dbg_dsp_getcondition, 0,
                        ("++++++MATCHING CORRELATOR NOT FOUND FOR: 0x%x +++++",
	                        event_correlator));

                        return(99);
                }


        	/*-----------------------------------------------------------
		Keep track of current place in the linked list so when 
		we exit, we don't have to back track to the previous 
		element in the list.
        	-----------------------------------------------------------*/

        	current_ptr    = tmp_next_ptr;

                tmp_correlator = tmp_next_ptr->correlator;
                tmp_next_ptr   = tmp_next_ptr->next;

        }



	/*---------------------------------------------------------------------
	Write data from adapter to ddf_data_getcondition structure so that
	mid_getcondition() will be able to pass it back to the caller.
	---------------------------------------------------------------------*/

	current_ptr->ddf_getcondition.status_word = condition_flag;
	current_ptr->ddf_getcondition.correlator  = ga_correlator;



        /*--------------------------------------------------------------------
	Dump ddf_data array for debugging.
        --------------------------------------------------------------------*/

	BUGLPR(dbg_dsp_getcondition, 5, ("DUMPING ddf_data_getcondition_t:\n"));
	for (k=0; k<4; k++)
	{
        BUGLPR(dbg_dsp_getcondition, 5, 
	("  ddf->ddf_data_getcondition[%d].ddf_getcondition.correlator=0x%x\n",
	k,  ddf->ddf_data_getcondition[k].ddf_getcondition.correlator));
        BUGLPR(dbg_dsp_getcondition, 5, 
	("  ddf->ddf_data_getcondition[%d].ddf_getcondition.wait_flag=0x%x\n",
	k,  ddf->ddf_data_getcondition[k].ddf_getcondition.wait_flag));
        BUGLPR(dbg_dsp_getcondition, 5, 
	("  ddf->ddf_data_getcondition[%d].ddf_getcondition.status_word=0x%x\n",
	k,  ddf->ddf_data_getcondition[k].ddf_getcondition.status_word));
		BUGLPR(dbg_dsp_getcondition, 5, 
			("  ddf->ddf_data_getcondition[%d].correlator=0x%x\n", 
			k,  ddf->ddf_data_getcondition[k].correlator));
	        BUGLPR(dbg_dsp_getcondition, 99, 
		("  ddf->ddf_data_getcondition[%d].wakeup_address=0x%x\n", 
			k,  ddf->ddf_data_getcondition[k].wakeup_address));
	       	BUGLPR(dbg_dsp_getcondition, 99, 
			("  ddf->ddf_data_getcondition[%d].next=0x%x\n", 
			k,  ddf->ddf_data_getcondition[k].next));
       		BUGLPR(dbg_dsp_getcondition, 99, 
			("  ddf->ddf_data_getcondition[%d].user_data=0x%x\n", 
			k,  ddf->ddf_data_getcondition[k].user_data));
	}


	/*---------------------------------------------------------------------
	Wake up process that generated the original  "mid_getcondition()"  
	call.  The address of this call is in the  "wakeup_address"  field of 
	the  "ddf_data_getcondition_t"  structure.
	---------------------------------------------------------------------*/

        BUGLPR(dbg_dsp_getcondition, 3, 
		("Wake up sleeper. wakeup_address = 0x%x\n",
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

		BUGLPR(dbg_dsp_getcondition, 3, ("After e_wakeup call.\n"));
	}
	else
	{
		current_ptr->int_rcvd++;
	}


        BUGLPR(dbg_dsp_getcondition, 1, ("Leaving mid_intr_getcondition()\n"));

        MID_DD_EXIT_TRACE (dsp_getcondition, 1, DSP_GET_CONDITION, ddf,
                0, 
		event_correlator, 
		current_ptr->sleep_flg, 
		current_ptr->int_rcvd );


	return;

}	/* End of mid_intr_getcondition */
