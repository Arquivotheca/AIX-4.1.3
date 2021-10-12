static char sccsid[] = "@(#)03  1.5  src/bos/kernext/disp/ped/ddf/mid_get_correlator.c, peddd, bos411, 9428A410j 11/3/93 11:14:14";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_get_correlator
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




#define Bool unsigned

#include <sys/types.h>          /* added for i_disable / i_enable       */

#include <sys/xmem.h>
#include <sys/rcm_win.h>
#include <sys/aixgsc.h>
#include <sys/rcm.h>
#include <sys/syspest.h>
#include <sys/errno.h>          /* added for i_disable / i_enable       */
#include <sys/intr.h>           /* added for i_disable / i_enable       */

#include <sys/ioacc.h>          /* I/O access for BUS access commands   */
#include "hw_dd_model.h"
#include "hw_regs_u.h"
#include "hw_typdefs.h"

#include "midddf.h"
#include "mid.h"

#include "midhwa.h"             /* BUS access definitions in here            */


BUGXDEF(dbg_midddf);
BUGXDEF(dbg_middd);




/*-----------------------------------------------------------------------------
Define interrupt disable/enable value.  This value defined in middef.c   
-----------------------------------------------------------------------------*/

#define PDEV_PRIORITY (pdev->devHead.display->interrupt_data.intr.priority-1)



/*-----------------------------------------------------------------------------

			M I D _ G E T _ C O R R E L A T O R

Get Unique Correlator macro  (mid_get_correlator)

DESCRIPTION

This is simply a macro which returns a unique correlator for the DDF functions
on Pedernales (and possible RCM functions).


INPUT PARAMETERS

ddf             Pointer to  "midddf_t"  structure.  This structure is part of
                the physical displays structure but is used only by the DDF
                functions. The  "midddf_t"  structure holds pointers and data
                specific to each of the DDF functions and is used to
                communicate this information from caller to interrupt handler
                and back.  This structure also holds a running counter for
		correlator values.

RETURN VALUE

correlator	A pseudo-unique, 16-bit value returned to caller.


PSEUDOCODE

	Disable interrupts.
	
	Increment correlator field in "mid_ddf" structure.

	Read  "ddf->correlator_count"  into  "correlator".

	Enable CPU interrupts.

-----------------------------------------------------------------------------*/


long mid_get_correlator( pdev, correlator )
gscDev 			*pdev;		/* device structure pointer	     */
unsigned short		*correlator;
{
	midddf_t	*ddf = 
			(midddf_t *) pdev->devHead.display->free_area;
	int	old_interrupt_priority;	/* save old priority for enable    */
	ushort		tmp_correlator; /* ensures intgrity of correlator  */

	BUGLPR(dbg_midddf, 99, ("Entering mid_get_correlator \n"));



	/*--------------------------------------------------------------------
	Disable CPU interrupts so that we can guarantee a "unique" correlator 
	value.  If interrupts were not disabled first, then "correlator_count"  
	could be changed by another process which interrupts us before we are 
	able to read it into our local variable.  If the interrupting process 
	were to increment "correlator_count" after we do, but before we can do 
	the read, then both the interrupting process and us will end up with 
	the same correlator value.  So we must disable interrupts before 
	incrementing "correlator_count".
	--------------------------------------------------------------------*/

	BUGLPR(dbg_midddf, 99, ("Disabling CPU interrupts - start.\n"));

        old_interrupt_priority = i_disable(PDEV_PRIORITY);

	BUGLPR(dbg_midddf, 99, ("Disabling CPU interrupts - done.\n"));


	/*--------------------------------------------------------------------
	Increment "correlator_count".  If correlator increments to zero, 
	increment to one.  Zero is used as a NULL test value. 
	--------------------------------------------------------------------*/

	(ddf->correlator_count)++; 
	BUGLPR(dbg_midddf, 99, ("ddf->correlator_count=0x%x.\n",
		ddf->correlator_count));
	if (ddf->correlator_count == 0)
		ddf->correlator_count++;



	/*--------------------------------------------------------------------
	Copy "correlator_count"  into our local variable, "tmp_correlator".
	--------------------------------------------------------------------*/

	tmp_correlator = ddf->correlator_count;



	/*--------------------------------------------------------------------
	Copy temp value into caller's area.
	--------------------------------------------------------------------*/
	*correlator = tmp_correlator;



	/*--------------------------------------------------------------------
	Re-enable CPU interrupts.
	--------------------------------------------------------------------*/

        i_enable(old_interrupt_priority);


	
	BUGLPR(dbg_midddf, 99, ("Leaving  mid_get_correlator. \n"));
	return(0);	/* end of mid_get_correlator */
}
