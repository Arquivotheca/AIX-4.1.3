static char sccsid[] = "@(#)67  1.4  src/bos/kernel/ios/POWER/d_protect.c, sysios, bos41J, 9519B_all 5/10/95 09:20:28";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: d_protect
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/dma.h>
#include <sys/adspace.h>
#include <sys/syspest.h>
#include <sys/intr.h>
#include <sys/sysdma.h>
#include <sys/systemcfg.h>
#include "dma_hw.h"

/******************************************************************************
*
* NAME: d_protect
*
* FUNCTION: This service sets the Page Protect Key in each
*	    TCE associated with an address range.
*
* EXECUTION ENVIRONMENT:
*
*       This service can be called by a program executing on either
*       the interrupt level or the process level.
*
*       It only page faults on the stack when called under a process.
*
* NOTES:
*
* RETURN VALUE DESCRIPTION: 0 on successful completion.
*
******************************************************************************/
int
d_protect(
	caddr_t baddr,				/* buffer address */
	caddr_t daddr,				/* bus address */
	ulong   length,				/* range */
	int	 key,				/* authorization key */
	ulong   buid)				/* bus unit ID */
{
	register uint  ioccaddr;		/* io_att return value */
	register uint	num_tcws;		/* number of TCWs to map */
	register int	rc;			/* return code */
	volatile uint	*t;			/* tcw */
	uint invaddr;				/* tlb address to ivalitate */
#ifdef _POWER_PC
	int	i;				/* count/index variable */
	struct	iocc_info *d;			/* pointer to iocc info struct*/
	uint	*end_tce_addr;			/* last TCE address */
#endif /* _POWER_PC */

	ASSERT( length > 0 );
	ASSERT( ( key >= 0 ) && ( key <= 7 ));

	/*
	 *  Determine the number of TCW's  needed.
	 */
        num_tcws = ((uint)(baddr + length - 1 ) >> DMA_L2PSIZE ) -
                   ((uint)baddr >> DMA_L2PSIZE ) + 1;

	rc = 0;

#ifdef _POWER_PC
	if (__power_pc()) {
		/*
		 * 	For the PowerPC platform
		 */
		for ( i = 0; i < MAX_NUM_IOCCS; i++) {
			if ((iocc_info[i].bid & 0x1FF00000) == 
                            (buid & 0x1FF00000))
				/*
				 *	Search through the IOCC structs
				 *	to find the TCE table for this IOCC
				 */
				break;
		}
		/*
		 * 	Make sure we found it
		 */
		assert(i != MAX_NUM_IOCCS);
		
		/*
		 * 	Set up a pointer to this info structure
		 */
		d = (struct iocc_info *)&iocc_info[i];
		
		/*
		 * 	Compute ending TCE address
		 */
		end_tce_addr = (uint *)TCE_EFF_ADDR_PPC(d->tce_offset, 
					(CALC_TCE_NUM_PPC(daddr) + num_tcws));

		for (t=(uint *)TCE_EFF_ADDR_PPC(d->tce_offset, 
			CALC_TCE_NUM_PPC(daddr)); t < end_tce_addr; t++) {
			/*
			 *	Update a TCE for each page that contains
			 *	part of the input buffer.
			 *      The upper nibbles (RPN) field) of the TCE
			 *      are set to 0 to cure an Ionian (IOCC)
			 *      problem wherein the PRN field is OR'd
			 *      to a TCE fetch address sent to the system.
			 */
			*(volatile uint *)t = MASTER_TCE_PPC(0, key, 0);
		}
		/*
		 *	Perform SYNC to ensure that all the updated TCEs
		 *	are globally seen.
		 */
		__iospace_sync();

	}
#endif /* _POWER_PC */

#ifdef _POWER_RS
	if (__power_rs()) {
		/* 
		 * initialize address to invalidate
		 */
		invaddr = (uint)daddr;

		/*
		 *  Setup access to the IOCC.
		 */
	        ioccaddr = (uint)io_att(IOCC_HANDLE(buid), 0);

		/*
		 *  Update one TCW for each page that contains part of 
		 *  the input buffer.
		 */
		for ( t = (uint *)TCW_EFF_ADDR(CALC_TCW_NUM(daddr));
		      t < (uint *)TCW_EFF_ADDR(CALC_TCW_NUM(daddr)+num_tcws);
		      t++)
		{
			/*
			 *  Imperative that 0XF is used as a buffer number here.
			 *  Although we do not buffer the TCW's when bypass
			 *  is set, the IOCC does trash the registers associated
			 *  with that buffer.
			 */
			rc = store_tcw( t, MASTER_TCW( -1, 0XF, key, 0) );
	
			/* RSC requires a tlbi be done on the effective 
			 * bus address after every TCW write
			 */
			tlbi(invaddr);
			invaddr += DMA_PSIZE;
	
			if ( rc != 0 )
				return ( rc );
		}

		/*
		 *  Release access to IOCC
		 */
       		 io_det( ioccaddr );
	}
#endif /* _POWER_RS */
	return( rc );
}

#ifdef _POWER_RS


/******************************************************************************
*
* NAME: store_tcw
*
* FUNCTION: This service will setup a structure to
*	    pass parameters to an exception handler routine
*	    which invokes a routine to do a store to a TCW.
*
* EXECUTION ENVIRONMENT:
*
*       This service can be called by a program executing on either
*       the interrupt level or the process level.
*
*       It only page faults on the stack when called under a process.
*
* NOTES:
*
* RETURN VALUE DESCRIPTION: return code from pio_assist.
*			    0 = success
*
******************************************************************************/


int
store_tcw( 
	volatile ulong *t,		/* address of TCW */
	ulong data)			/* data */
{
        register int rc;
        int s_tcw();
	volatile struct	 parmlist pl, *ioparms;	/* param list for pio_assist */

	ioparms = &pl;
	ioparms->t = t;
	ioparms->data = data;
	rc = pio_assist( (caddr_t)ioparms, s_tcw, NULL);
	return( rc );
}

/******************************************************************************
*
* NAME: s_tcw
*
* FUNCTION: This service stores a word of data to
*	    a TCW.
*
* EXECUTION ENVIRONMENT:
*
*       This service can be called by a program executing on either
*       the interrupt level or the process level.
*
*       It only page faults on the stack when called under a process.
*
* NOTES: An exception handler has been set up for this operation.
*
* RETURN VALUE DESCRIPTION: 0 on successful completion.
*
******************************************************************************/

int
s_tcw(
	caddr_t p)	/* pointer to parmlist */
{
	volatile struct	 parmlist  *ioparms;	

        ioparms = (struct parmlist *) p;
	*(ioparms->t) = ioparms->data;
        return(0);
}

#endif /* _POWER_RS */
