static char sccsid[] = "@(#)81	1.2  src/bos/kernel/ios/POWER/busreg.c, sysios, bos411, 9428A410j 4/14/94 14:08:04";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: 
 *	bus_register
 *	d_map_init
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
#include <sys/ioacc.h>
#include <sys/syspest.h>
#include <sys/dma.h>
#include <sys/xmem.h>

extern struct businfo bid_table[];

struct xmem xmg = { {-1,-1},	/* have to set both shorts of the first */
                    0,		/* element of the union in order to set */
                    0,		/* aspace_id to XMEM_GLOBAL             */
                    0 };

struct xmem *xmem_global = &xmg;

/*
 * NAME: bus_register
 *
 * FUNCTION:
 *      The bus_register service is called by a loaded bus extension
 *      entry point to register its bus type with the operating
 *      system.  This allows the operating system to build a table
 *      of bus types indexed by the bid parameter and find the
 *      appropriate bus specific information.  This service provides
 *      the kernel with the information required to setup PIOs and
 *      DMA for a specific bus.  If the same bid is registered
 *      multiple times, it is the last registration that takes
 *      effect.
 *
 * NOTES:
 *	This is an undocumented kernel service
 *
 * EXECUTION ENVIRONMENT:
 *	This is only callable from the process environment
 *
 * RETURNS:
 *	0 - success
 */	
int
bus_register(
	struct businfo *bp)		/* bus structure to register */
{
	int index;

	index = BID_INDEX(bp->bid);
	assert(index < MAX_BID_INDEX);

	bid_table[index] = *bp;

	return(0);
}


/*
 * NAME: d_map_init
 *
 * FUNCTION: This routine is the registration service for the d_map*
 *	     DMA interfaces.  These interfaces make up the non-Microchannel
 *	     DMA support.
 *
 * EXECUTION ENVIRONMENT:
 *           This routine is called from the process level only
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION: d_handle structure pointer
 *			     DMA_FAIL - couldn't register
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 */
d_handle_t 
d_map_init(int bid,		/* unique bus id, type & number       */
	   int flags,		/* describe device capabilities       */
	   int bus_flags,	/* flags specific to the target bus   */
	   uint channel)	/* channel assignment.dev/bus specific*/
{

	int	index;

	index = BID_INDEX(bid);		/* get bus table index */
	if (index >= MAX_BID_INDEX)
		return(DMA_FAIL);

	/*
	 * Invoke the bus specific d_map_init routine
	 */
	return(bid_table[index].d_map_init(bid, flags, bus_flags, channel));
		
}


