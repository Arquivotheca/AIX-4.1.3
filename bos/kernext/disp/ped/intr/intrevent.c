static char sccsid[] = "@(#)47  1.7  src/bos/kernext/disp/ped/intr/intrevent.c, peddd, bos411, 9428A410j 4/21/94 11:05:06";

/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_intr_hostevt
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


#define LOADABLE
#define INTERRUPT_DMA
#define NOT_WORKING 		/* for code not yet working */

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
#include "hw_ind_mac.h"			/* for Status Control Block access   */
#include "hw_regs_u.h"
#include "hw_seops.h" 	/* needed for MID_WR_HOST_COMO() to work */
#include "rcm_mac.h"            /* rcm-related macros.  Using FIND_GP   */
#include "midwidmac.h"	/* needed for MID_MARK_BUFFERS_SWAPPED call	*/


BUGXDEF(dbg_midddi);

/*------------------------------------------------------------------------
 *
 * NAME:        mid_intr_hostevt
 *
 *
 * FUNCTION:    This routine saves the host intr. event information into
 *              the event report buffer.
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is called by the PED interrupt handler, mid_intr(), when
 *      the host interrupt occurs.
 *
 *
 * EXTERNAL REFERENCES:
 *
 * DATA STRUCTURES:
 *
 *      pd          The physical display structure, it holds pointers and
 *                  data which are needed for the interrupt handler.
 *
 * RETURNS:
 *
 *      None.
 */

long mid_intr_hostevt( pd, host_status, dsp_status )
struct phys_displays    *pd;
ulong   host_status;
ulong   dsp_status;
{
	midddf_t        *ddf = (midddf_t *) pd->free_area;

	BUGLPR( dbg_midddi, BUGNTX, ("Entering mid_intr_hostevt()\n") );

	ddf->report.event = host_status;
	ddf->report.time = time;
	ddf->report.data[0] = dsp_status;
	ddf->report.data[1] = 0;
	ddf->report.data[2] = 0;
	ddf->report.data[3] = 0;

	/* Call the callback routine to signal the process.             */

	if ( ddf->callback )
		(ddf->callback)( pd->cur_rcm, &(ddf->report) );

	BUGLPR( dbg_midddi, BUGNTX, ("Leaving  mid_intr_hostevt()\n") );
	return ( 0 );
}



