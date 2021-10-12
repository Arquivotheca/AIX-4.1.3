static char sccsid[] = "@(#)42  1.12  src/bos/kernext/disp/ped/intr/intrcolor.c, peddd, bos411, 9428A410j 3/31/94 21:34:40";

/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_intr_getcolor
 *		mid_timeout_getcolor
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
#include "hw_ind_mac.h"		/* for Status Control Block access 	    */
#include "hw_regs_u.h"
#include "hw_seops.h" 		/* needed for MID_WR_HOST_COMO() to work    */
#include "rcm_mac.h"    	/* rcm-related macros.  Using FIND_GP       */
#include "midwidmac.h"		/* needed for MID_MARK_BUFFERS_SWAPPED call */

#include "mid_dd_trace.h"       /* for memory trace                         */
MID_MODULE ( dsp_getcolor );



/*---------------------------------------------------------------------------
Define watchdog timer handler function for mid_getcolor().  When the 
watchdog timer pops, the appropriate handler gets called and wakes up the
process at a point where it can recover.
---------------------------------------------------------------------------*/

void mid_timeout_getcolor( w )
mid_watch_getcolor_t *w;
{

	BUGLPR(dbg_dsp_getcolor,0 , 
	  ("TIMED OUT  ---  CALLING WATCHDOG TIMER:   mid_watch_getcolor()\n"));
	BUGLPR(dbg_dsp_getcolor,0 , ("  w->sleep_addr=0x%x\n", w->sleep_addr ));

        e_wakeup( w->sleep_addr );
}




/*---------------------------------------------------------------------*/
/*                                                                     */
/* IDENTIFICATION: MID_INTR_GETCOLOR                                   */
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

/*--------------------------------------------------------------------------

		M I D _ I N T R _ G E T C O L O R

Interrupt Level Get Current Color (mid_intr_getcolor)


DESCRIPTION

Reads RGB color information off of the adapter and loads it into the structure
set up by the original caller.  The data retrieved is used by the application
(GL) for both RGB and color index interpretation.

long mid_intr_getcolor( ddf )
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

	Get "RGB" colors off of adapter
	
	Disable bus

	Locate DDF structure associated w/ current getcolor interrupt, and ...

	Assign color values into this structure

	Wake up sleeping process

	Return

--------------------------------------------------------------------------*/


long mid_intr_getcolor( ddf )
midddf_t		*ddf;
{

	ushort		event_correlator;   /* store corr value from SCB    */
	short		tmp_correlator;     /* from getcolor linked list    */
	ddf_data_getcolor_t  *tmp_next_ptr; /* ptr to next array element    */
	ddf_data_getcolor_t  *current_ptr;  /* ptr to current struct        */
	short		i, k;		    /* loop counters                */
	ulong		red, green, blue;   /* local variables to hold RGB  */
					  /* values from M1M StatContrlBlck */

        HWPDDFSetup;                      /* gain access to hardware pointer*/

	BUGLPR(dbg_dsp_getcolor, 1, ("Entering mid_intr_getcolor()\n"));
	BUGLPR(dbg_dsp_getcolor, 4, ("*ddf=0x%x\n", ddf));




	/*-------------------------------------------------------------------
	Set current event correlator variable to the correlator of the 
	event which we are currently servicing.  The value was saved 
	previously in the "ddf" structure by the caller of this function.
	The high 2 bytes are AND'ed out since we want only the correlator
	value itself.
	-------------------------------------------------------------------*/

	event_correlator = (0x0000FFFF & ddf->dsp_status);

        MID_DD_ENTRY_TRACE (dsp_getcolor, 1, DSP_GET_COLOR, ddf,
                0, event_correlator, 0, 0 );

	BUGLPR(dbg_dsp_getcolor, 4, ("event_correlator=0x%x \n", 
		event_correlator));




	/*-------------------------------------------------------------------
	Call hardware macro to read 3DM1M Status Control Block data into  
	"midddf_t"  data structure.  This data includes three floating point 
	values (one each for Red, Green, and Blue) and an operation correlator.
 	The color information is deposited in the  "mid_getcolor_t"  structure 
	set up by the original caller.  There is a pointer to this structure in 
	the  "midddf_t"  structure so that the interrupt handlers know where to 
	place the data.  The correlator is used by the interrupt handler to 
	associate a particular call with these results.  

	Read RGB values off of adapter and assign appropriate fields in
	the  "midddf_t"  structure.
	-------------------------------------------------------------------*/

	MID_RD_M1MSB_VALUE(MID_M1MSB_RED_COLOR, red);
	MID_RD_M1MSB_VALUE(MID_M1MSB_GREEN_COLOR, green);
	MID_RD_M1MSB_VALUE(MID_M1MSB_BLUE_COLOR, blue);

	BUGLPR(dbg_dsp_getcolor, 2, ("raw hex RGB values from M1MSCB:\n"));
	BUGLPR(dbg_dsp_getcolor, 2, ("   red=0x%x green=0x%x blue=0x%x \n", 
		red, green, blue));

        MID_DD_TRACE_PARMS (dsp_getcolor, 1, DSP_GET_COLOR, ddf,
                0, 
                red, 
                green,
                blue );




	/*-------------------------------------------------------------------
	Match correlator value from Status Control Block on the adapter
	with correlator(s) in "ddf_data_getcolor_t" structure. 

        event_correlator corresponds to the interrupt being serviced.
        tmp_correlator cycles through the available correlators until it
        finds a match.
	-------------------------------------------------------------------*/

	i = 0;
	current_ptr    = &ddf->ddf_data_getcolor[i];
	tmp_next_ptr   = ddf->ddf_data_getcolor[i].next;
	tmp_correlator = ddf->ddf_data_getcolor[i].correlator;

        BUGLPR(dbg_dsp_getcolor, 4, 
		("tmp_correlator = 0x%x, event_correlator=0x%x\n", 
		tmp_correlator, event_correlator));



	while (tmp_correlator != event_correlator)
	{

        	BUGLPR(dbg_dsp_getcolor, 4, 
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
        		BUGLPR(dbg_dsp_getcolor, 0, 
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



        /*---------------------------------------------------------
        Since assigning floats in kernel, use dereference trick.
        ---------------------------------------------------------*/

        (*(int *) &(current_ptr->ddf_getcolor.red ))   = red;
        (*(int *) &(current_ptr->ddf_getcolor.green )) = green;
        (*(int *) &(current_ptr->ddf_getcolor.blue ))  = blue;

	BUGLPR(dbg_dsp_getcolor, 4, ("ptr to ddf->ddf_data_getcolor[0]=0x%x\n",
		&ddf->ddf_data_getcolor[0]));
	BUGLPR(dbg_dsp_getcolor, 4, ("RGB values from M1MSCB:\n"));
	BUGLPR(dbg_dsp_getcolor, 4, ("   red=0x%x green=0x%x blue=0x%x \n", 
		red, green, blue));



        /*------------------------------------------------------------------
        Dump ddf_data array for debugging.
        ------------------------------------------------------------------*/

        BUGLPR(dbg_dsp_getcolor, 5, ("DUMPING ddf_data_getcpos_t:\n"));

        for (k=0; k<4; k++)
        {
	        /*-----
                NOTE:  use "(*int *) &" trick so that kernel cannot tell
                       that the red, green and blue fields are floats.
                       Trying to de-refernece a float in the kernel causes a
                       a floating point exception trap.
        	-----*/

                BUGLPR(dbg_dsp_getcolor, 5,
                    ("  ddf->ddf_data_getcolor[%d].ddf_getcolor.red=0x%x\n",
                    k, (*(int *) &ddf->ddf_data_getcolor[k].ddf_getcolor.red)));
                BUGLPR(dbg_dsp_getcolor, 5,
                    ("  ddf->ddf_data_getcolor[%d].ddf_getcolor.green=0x%x\n",k,
                    (*(int *) &ddf->ddf_data_getcolor[k].ddf_getcolor.green)));
                BUGLPR(dbg_dsp_getcolor, 5,
                    ("  ddf->ddf_data_getcolor[%d].ddf_getcolor.blue=0x%x\n",
                    k,(*(int *) &ddf->ddf_data_getcolor[k].ddf_getcolor.blue)));
                BUGLPR(dbg_dsp_getcolor, 5,
                    ("  ddf->ddf_data_getcolor[%d].correlator=0x%x\n",
                    k,  ddf->ddf_data_getcolor[k].correlator));
                BUGLPR(dbg_dsp_getcolor, 5,
                    ("  ddf->ddf_data_getcolor[%d].wakeup_address=0x%x\n",
                    k,  ddf->ddf_data_getcolor[k].wakeup_address));
                BUGLPR(dbg_dsp_getcolor, 5,
                    ("  ddf->ddf_data_getcolor[%d].next=0x%x\n",
                    k,  ddf->ddf_data_getcolor[k].next));
                BUGLPR(dbg_dsp_getcolor, 5,
                    ("  ddf->ddf_data_getcolor[%d].user_data=0x%x\n",
                    k,  ddf->ddf_data_getcolor[k].user_data));
        }



	/*-------------------------------------------------------------------
	Wake up process that generated the original  "mid_getcolor()"  call.  
	The address of this call is in the  "wakeup_address"  field of the  
	"ddf_data getcolor_t"  structure.
	-------------------------------------------------------------------*/

	BUGLPR(dbg_dsp_getcolor, 3,("Wake up sleeper. wakeup_address = 0x%x\n", 
		current_ptr->wakeup_address ));


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

		BUGLPR(dbg_dsp_getcolor, 3, ("After e_wakeup call.\n"));
	}
	else
	{
		current_ptr->int_rcvd++;
	}



	BUGLPR(dbg_dsp_getcolor, 1, ("Leaving  mid_intr_getcolor()\n"));

        MID_DD_EXIT_TRACE (dsp_getcolor, 1, DSP_GET_COLOR, ddf,
                0, 
		event_correlator, 
		current_ptr->sleep_flg, 
		current_ptr->int_rcvd );


	return (0);

}	/* end of mid_intr_getcolor */
