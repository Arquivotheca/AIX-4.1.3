static char sccsid[] = "@(#)62	1.13  src/bos/kernel/db/POWER/dbxlate.c, sysdb, bos411, 9435B411a 8/23/94 09:53:30";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/seg.h>
#include <sys/systemcfg.h>
#include <sys/machine.h>
#include "parse.h"
#include "debvars.h"

extern ulong max_real;

#define MAX_INVALID_BITS 	127
#define INVALID_MEMORY_BIT	0x80000000

#define SREG(x)  ((x)>>SEGSHIFT)        /* Segment reg. #               */
#define SEG(x)   (SRTOSID(debvars[IDSEGS+SREG(x)].hv)) /* seg. id.      */

#ifdef _POWER
#define IS_IOSPACE(x) ((x) & 0x80000000)
#endif  /* _POWER */

/*
 * NAME: dbxlate
 *
 * FUNCTION: 	See if an address is in memory.
 *
 * RETURNS:	TRUE if successful, FALSE otherwise
 */

int
debug_xlate(ulong addr,int virt)
{
	uint segnum, i;
	ulong bitno, wordno;
	extern ulong bad_memory_bits[4]; /* reference to pl8 bad_mem struc */

	if (virt) {			/* check for mapped address */
#ifdef _POWER_PC
		if (__power_pc() && !(__power_601())) {/* Use BATs */
			/* 
			 * Scan BATs in debvars for match
			 * If match found return TRUE
			 * Assuming 256M mapped for a BAT since that is how
			 * the pio services does things.
			 */
			segnum = (uint)(addr >> SEGSHIFT);
			for (i=0; i < NUM_KERNEL_BATS; i++) {
				if (debvars[IDBATU + i].hv != 0) {
					if (BAT_ESEG(debvars[IDBATU + i].hv) == segnum) {
						return(TRUE);
					}
				}
			}
		} /* If we're still here do segment i/o space check */
#endif /* ifdef _POWER_PC */
		
		/*
	 	 * since the i/o space on rios requires interpreting
	 	 * a segment register it is considered here as 
	 	 * virtual address space
	 	 */
		if (IS_IOSPACE(debvars[IDSEGS+SREG(addr)].hv)) {
			return(TRUE);
		}

		/* 
		 * If there is no BAT hit for a non-601 PowerPC and T=0
		 * for the other machines then let the VMM routine lqra
		 * figure things out.
		 */
		return(lqra(SEG(addr),addr) != -1);
	}
	else {	/* real address */
		return( in_real_mem(addr) );
	}
}
