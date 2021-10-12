static char sccsid[] = "@(#)66	1.7  src/bos/kernel/ios/POWER/i_misc_pwr.c, sysios, bos411, 9436B411a 8/29/94 15:11:33";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: i_misc_pwr, check_bus_timeout, i_scu
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef _POWER_RS

#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/iocc.h>
#include <sys/nio.h>
#include <sys/mstsave.h>
#include <sys/low.h>
#include <sys/intr.h>
#include <sys/dbg_codes.h>
#include <sys/adspace.h>
#include <sys/errids.h>
#include <sys/time.h>
#include <sys/ctlaltnum.h>
#include <sys/iplcb.h>
#include <sys/buid.h>
#include <sys/machine.h>
#include <sys/except.h>
#include <sys/buid0.h>
#include <sys/buid.h>
#include <sys/vmker.h>
#include <sys/timer.h>
#include <sys/signal.h>
#include "intr_hw.h"
#include "dma_hw.h"
#include "interrupt.h"
 
struct intr     mischand0;	/* keep in pinned part of kernel */
struct intr	mischand1;	/* For optional second IOCC */

/*
 * various error log structures
 */
extern	struct miscerr  misc_log;
extern	struct scuerr	exdma_log;
extern	struct scuerr	scrub_log;
extern	struct epowerr	suspend_log;
extern	struct epowerr	resume_log;

#ifdef _POWER_RSC
	struct	dmaerr_rsc {             /* RSC dma error template */
		struct  err_rec0 e;
		ulong   eesr;           /* EESR register */
		ulong   eear;           /* EEAR register */
		ulong   sccr;           /* SCCR register */
		ulong   sim;            /* ASCII sim number */
	} dmaerr_log = { ERRID_EXCHECK_RSC, "SYSIOS ", 0, 0, 0, 0};

#endif /* _POWER_RSC */

extern	int		epow_status;	/* power status flag */
extern	struct 	trb	*epow_trb;	/* pointer to timer block for epow */

/*
 * NAME:  i_misc_pwr
 *
 * FUNCTION:  
 *	Process the IOCC miscellaneous interrupts.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called in an interrupt execution environment.
 *
 *      It cannot page fault.
 *
 * NOTES:  
 *	This version of this interrupt handler only calls the debugger
 *	when a ctl-alt-anything key sequence occurs. It is NOT product
 *	level code. It is intended to aid in debugging by providing a
 *	way to enter the debugger other than by previously set break
 *	points.
 *
 *
 * RETURN VALUE DESCRIPTION:
 *	INTR_SUCC
 *
 * EXTERNAL PROCEDURES CALLED:
 *	debugger
 *	i_reset
 *	io_att
 *	io_det
 */
int
i_misc_pwr(
	struct intr *handler)		/* intr struct */
{
	volatile struct	iocc *i;	/* IOCC				*/
	int halt_code;
	int dbg_code;
	int iocc_num;

	/*
	 * Verify that the interrupt priority is valid.
	 */
	ASSERT( csa->prev != NULL );
	ASSERT( csa->intpri == INTMAX );

	/*
	 * Setup addressibility to the IOCC.
	 */

	i = (struct iocc volatile *) (io_att(BUS_TO_IOCC(handler->bid),0) +
			         	     IO_IOCC);
	iocc_num = IOCC_NUM_MSK( handler->bid );
	/*
	 * System Attention Interrupt should not be recieved
	 */
	if ( i->int_misc & IMI_KBDEXT )
	{
#if 0
		volatile struct nio_key *k; /* I/O address keyboard */

		ctlaltnum(NUMPAD_4);
		/* reset the interrupt */
		k=(struct nio_key volatile *)(io_att(NIO_BID,0)+
								NIO_KEYBOARD);
		k->ctlalt = 0;
		io_det(k);
		i->int_misc = ~ IMI_KBDEXT;
		i_reset(handler);
#endif
		/*
		 *  Log the errors.
		 */
		misc_log.iocc = handler->bid;
		misc_log.bsr = i->bus_status;
		misc_log.misc = i->int_misc;
		errsave(&misc_log, sizeof(misc_log));
		halt_code = 0x54000000;
		dbg_code = DBG_KBDEXT;
	}
	else if ( i->int_misc & IMI_TIMEOUT )
	{
		/*
		 *  Log the errors.
		 */
		misc_log.iocc = handler->bid;
		misc_log.bsr = i->bus_status;
		misc_log.misc = i->int_misc;
		errsave(&misc_log, sizeof(misc_log));
		halt_code = 0x53000000 | iocc_num;
		dbg_code = DBG_BUS_TIMEOUT;
	}
	else if ( i->int_misc & IMI_CHANCK )
	{
		misc_log.iocc = handler->bid;
		misc_log.bsr = i->bus_status;
		misc_log.misc = i->int_misc;
		errsave(&misc_log, sizeof(misc_log));
		halt_code = 0x52000000 | iocc_num;
		dbg_code = DBG_CHAN_CHECK;
	}
	else
	{
		/*
		 * No miscellaneous interrupts found for this IOCC.
		 * Clean up and return to the caller.
		 */
		io_det(i);
		return( INTR_FAIL );
	}

	halt_display(csa->prev, halt_code, dbg_code);

}

/*
 * NAME: check_bus_timeout
 *
 * FUNCTIONS: corrects an RSC bug where an DSI can occur durring
 *	IOCC register accesses.  A bus timeout must have occured
 *	while the IOCC pio was pending.  This code will examine
 *	DSIs to determine first if they are to IOCC control space
 *	and then if they are valid.  Invalid DSIs will be assumed
 *	to be a bus timeout.
 *
 * NOTES:
 *	The RSC bus timeout DSI will not occur on a time delay command
 *
 * EXECUTION ENVIORMENT:
 *	Called from the system io exception slih.  This code will
 *	only run on RSC machines
 *
 * RETURNS: None
 */
void
check_bus_timeout(
	uint dar,
	uint srval,
	struct mstsave *mst)
{
	uint ioccoffset;
	volatile struct iocc *i;

	/* Check that the DSI was to IOCC space on IOCC 0
	 */
	if ((BID_TO_BUID(srval) != IOCC0_BUID) || !(srval & IOCCSR_SELECT))
		return;

	/* Check that this was not a privledge exception
	 */
	if (srval & IOCCSR_KEY)
		return;

	/* Check that the faulting address was in the IOCC command/control
	 * region
	 */
	ioccoffset = SEGOFFSET(dar);
	if (ioccoffset < IOCC_COMMAND_START || ioccoffset > IOCC_COMMAND_END)
		return;

	/* Treat the DSI as a bus time out,  Do error loggin
	 * and then halt the sytem
	 */
	i = (volatile struct iocc *)io_att(IOCC0_BID, IO_IOCC);

	misc_log.iocc = IOCC0_BID;
	misc_log.bsr = i->bus_status;
	misc_log.misc = i->int_misc;
	errsave(&misc_log, sizeof(misc_log));
	halt_display(mst, 0x53000000, DBG_BUS_TIMEOUT);
}


/*
 * NAME:  i_scu
 *
 * FUNCTION:
 *      This routine handles scrubbing errors and dma errors.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can only be called by the interrupt handler
 *      that catches the memory error interrupt on interrupt priority INTMAX.
 *
 *      The memory error interupt handler is registered via i_init.
 *      It is called by i_slih.
 *
 *      This routine does not page fault.
 *
 * RETURN VALUE DESCRIPTION:
 *     	INTR_SUCC
 *
 * EXTERNAL PROCEDURES CALLED:
 *	io_att, io_det, halt_display, errsave
 */
int
i_scu(
	struct intr *handler)
{
	volatile ulong *conreg_ptr;
	int i,j;
	ulong buid0addr;
	ulong eesr;
	ulong eear;
	ulong eecar;
	ulong sccr;
	ulong creg[8];
	ulong data[2];

	ASSERT(__power_rs());  /* Only these boxes support scrubbing */

	buid0addr = (ulong)io_att(BUID0, 0);

#ifdef _POWER_RSC
	if (__power_rsc())
	{
		buscpy(buid0addr + EESR_RSC, data, sizeof(data));
		/* Log an error and halt the system
		 */
		dmaerr_log.eesr = data[0];
		dmaerr_log.eear = data[1];
		dmaerr_log.sccr = *(volatile ulong *)(buid0addr + SCCR);
		dmaerr_log.sim =
			encode_sim((dmaerr_log.eear & 0x00ffffff) << 3);
		errsave(&dmaerr_log, sizeof(dmaerr_log));
		halt_display(csa->prev, 0x50000000, DBG_EXT_DMA);
	}
#endif /* _POWER_RSC */

        if( __power_set(POWER_RS1|POWER_RS2))
	{
		/*
		 * Scan the storage configuration registers(CRE) from 0
		 * to 15.  Saving them for later error logging.  Assumption
		 * here is that only 8 will ever be in use at any one
		 * time.  In use is determined to be when the extent size,
		 * bits 16-31, is nonzero.  If more that 8 extents ever
		 * occur i_scu will only log the first 8.
		 */

		/* Read memory configuration registers
		 */
		conreg_ptr = (volatile ulong *)(buid0addr + CRE0);
		for (i = 0, j = 0; i < 16 && j < 8; i++)
		{
			/* Use the eear variable temporarily */

			eear = conreg_ptr[i];
			if( eear & 0xFFFF )
				creg[j++] = eear;
		}

		/* Clear any unused creg elements.  */
		for( ; j < 8; j++ ) {
			creg[j] = 0;
		}

		eesr = *(volatile ulong *)(buid0addr + EESR);
		eear = 0;
		eecar = 0;

		/* load the eear register for non-ecc errors
		 */
		if (eesr & (EESR_MD|EESR_DB|EESR_AD|EESR_MS|EESR_AS))
			eear = *(volatile ulong *)(buid0addr + EEAR);

		/* load the ecar register for ecc errors
		 */
		if (eesr & (EESR_PD|EESR_UD|EESR_PS|EESR_US))
			eecar = *(volatile ulong *)(buid0addr + EECAR);

		/* Halt machine on any type of dma error
		 */
		if (eesr & (EESR_PD|EESR_UD|EESR_MD|EESR_DB|EESR_AD))
		{
			exdma_log.estat = eesr;
			exdma_log.eear_adr = eear;
			exdma_log.eecar_adr = eecar;
			for (i=0; i<8; i++)
				exdma_log.creg[i] = creg[i];
		  	errsave(&exdma_log,sizeof(exdma_log));  
			halt_display(csa->prev,0x50000000,DBG_EXT_DMA);
		}

		/* Halt machine on any type of scrubbing error
		 */
		if (eesr & (EESR_PS|EESR_US|EESR_MS|EESR_AS))
		{
			scrub_log.estat = eesr;
			scrub_log.eear_adr = eear;
			scrub_log.eecar_adr = eecar;
			for (i=0; i<8; i++)
				scrub_log.creg[i] = creg[i];
		  	errsave(&scrub_log,sizeof(scrub_log));   
			halt_display(csa->prev,0x50000000,DBG_EXT_SCR);
		}
	}

	io_det(buid0addr);
	return(INTR_SUCC);

}


#endif /*  _POWER_RS */

