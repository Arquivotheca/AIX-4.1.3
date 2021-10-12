static char sccsid[] = "@(#)65	1.4  src/bos/kernel/ios/POWER/i_misc_ppc.c, sysios, bos411, 9428A410j 5/26/94 11:52:33";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: i_misc_ppc
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */


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
#ifdef _RS6K_SMP_MCA
#include <sys/sys_resource.h>
#include "pegasus.h"
#endif /* _RS6K_SMP_MCA */
#include "intr_hw.h"
#include "dma_hw.h"
#include "interrupt.h"

struct intr	mischand[MAX_NUM_IOCCS];	/* For IOCC misc interrupts */

/*
 * various error log structures
 */
extern	struct miscerr  misc_log;

extern struct iocc_info iocc_info[MAX_NUM_IOCCS];

/*
 * NAME:  i_misc_ppc
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
i_misc_ppc(
	struct intr *handler)		/* intr struct */
{
	volatile struct	iocc_ppc *i;	/* IOCC				*/
	int idx;
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

	i = (struct iocc_ppc volatile *) (io_att(BID_TO_IOCC(handler->bid),0) +
			         	     IO_IOCC_PPC);

	iocc_num = GET_BUID_MSK( handler->bid );

	/*
	 * Scan the iocc_info array to establish the micro channel number
	 */
	for( idx=0; idx < MAX_NUM_IOCCS; idx++) {
		if (GET_BUID_MSK( iocc_info[idx].bid ) == iocc_num) {
			iocc_num = idx << 20;
			break;
		}
	}

	if ( i->int_misc & IMI_TIMEOUT )
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
#ifdef _RS6K_SMP_MCA
	else if ( i->int_misc & IMI_BUMP )
	{
		extern struct intr mdd_intr;
		volatile uchar dummy;
		extern uchar pgs_bump_init_done;

		/* reset BUMP interrupt */
		dummy = ((struct pgs_sys_spec*)
			 (sys_resource_ptr->sys_specific_regs))->res_int_fb;
		if ( pgs_bump_init_done )
			i_sched( &mdd_intr );
		io_det(i);
		return( INTR_SUCC );
	}
#endif /* _RS6K_SMP_MCA */
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
