static char sccsid[] = "@(#)86	1.9.1.7  src/bos/kernel/ios/POWER/init_misc.c, sysios, bos411, 9436B411a 8/29/94 15:15:56";

/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: init_misc
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

#include <sys/types.h>
#include <sys/intr.h>
#include <sys/iocc.h>
#include <sys/syspest.h>
#include <sys/timer.h>
#include <sys/systemcfg.h>
#include "dma_hw.h"
#ifdef _RS6K_SMP_MCA
#include <sys/buid.h>
#endif

#ifdef _POWER_RS
extern struct intr mischand0;		/* keep in pinned part of kernel */
extern struct intr mischand1;		/* keep in pinned part of kernel */
#endif /* _POWER_RS */
#ifdef _POWER_PC
extern struct intr mischand[MAX_NUM_IOCCS];
#include "intr_hw.h"
extern struct iocc_info iocc_info[MAX_NUM_IOCCS];
#endif /* _POWER_PC */

extern struct intr epowhand;		/* keep in pinned part of kernel */
#ifdef _RS6K_SMP_MCA
extern struct intr epowhand_dev;	/* keep in pinned part of kernel */
#endif
extern struct intr *epowhandler;	/* keep in pinned part of kernel */
extern struct intr scuhand;		/* keep in pinned part of kernal */
extern int num_ioccs;			/* number of IOCCs in the system */
extern struct trb *epow_trb;		/* pointer to epow timer block   */

/*
 * NAME:  init_misc
 *
 * FUNCTION:  Initialize machine dependent interrupt handlers.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called during system initialization. It is
 *	called by intr_init.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_init
 */

void
init_misc( )

{
#ifdef _POWER_RS
	extern	 int		i_misc_pwr();  /* misc interrupt handler */
#endif /*_POWER_RS */
#ifdef _POWER_PC
	extern	 int		i_misc_ppc();  /* misc interrupt handler */
	int			idx;		/* index for IOCCs	*/
#endif /*_POWER_PC */
	extern	 int		i_scu();   /* scu interrupt handler  */

	register int		rc;	   /* i_init return code */

	/*
	 * Define the handler for the IOCC miscellaneous interrupts.
	 * This handler processes I/O bus errors such as time outs
	 * and asynchronous channel checks.
	 */
#ifdef _POWER_RS
	if( __power_rs() ) {
		mischand0.next = (struct intr *)NULL;
		mischand0.bus_type = BUS_MICRO_CHANNEL;
		mischand0.flags = 0;
		mischand0.level = INT_IOCCMISC;
		mischand0.priority = INTMAX;
		mischand0.bid = IOCC0_BID;
		mischand0.handler = i_misc_pwr;

		/* Define the handler for misc. interupts */
		rc = i_init( &mischand0 );
		assert( rc == INTR_SUCC );

	/*
	 * Define misc. interrupt handler for optional second IOCC if
	 * it is present
	 */
		if (num_ioccs == 2)
		{
			mischand1.next = (struct intr *)NULL;
			mischand1.bus_type = BUS_MICRO_CHANNEL;
			mischand1.flags = 0;
			mischand1.level = INT_IOCCMISC;
			mischand1.priority = INTMAX;
			mischand1.bid = IOCC1_BID;
			mischand1.handler = i_misc_pwr;

			/* Define the handler for misc. interupts */
			rc = i_init(&mischand1);
			assert(rc == INTR_SUCC);
		}
	}
#endif /* _POWER_RS */
#ifdef _RS6K
	if( __rs6k() ) {
	    for( idx=0; idx < MAX_NUM_IOCCS; idx++ ) {
		if( iocc_info[idx].bid != 0xffffffff ) {	
			mischand[idx].next = (struct intr *)NULL;
			mischand[idx].bus_type = BUS_MICRO_CHANNEL;
			mischand[idx].flags = 0;
			mischand[idx].level = INT_IOCCMISC;
			mischand[idx].priority = INTMAX;
			mischand[idx].bid = iocc_info[idx].bid;
			mischand[idx].handler = i_misc_ppc;

			/* Define the handler for misc. interupts */
			rc = i_init(&mischand[idx]);
			assert(rc == INTR_SUCC);
		}
	    }
	}
#endif /* _RS6K */
#ifdef _POWER_RS
	if( __power_rs() ) {
		/*
		 * Define the handler for the scu memory error interrupts.
		 */
		scuhand.next = (struct intr *)NULL;
		scuhand.handler = i_scu;
		scuhand.bus_type = BUS_PLANAR;
		scuhand.flags = INTR_NOT_SHARED;

#if defined(_POWER_RS1) || defined(_POWER_RSC)
		if( __power_set( POWER_RS1 | POWER_RSC )) {
			scuhand.level = INT_SCUB_RS1;
		}
#endif /* _POWER_RS1 || _POWER_RSC */

#ifdef _POWER_RS2
		if( __power_rs2() ) {
			scuhand.level = INT_SCUB_RS2;
		}
#endif /* _POWER_RS2 */

		scuhand.priority = INTMAX;
		scuhand.bid = IOCC_BID;

	
		/*
		 * Define the memory error external check interrupt handler
		 */
		rc = i_init( &scuhand );
		assert( rc == INTR_SUCC );
	}
#endif /* _POWER_RS */
}

/*
 * NAME:  epowinit
 *
 * FUNCTION:  Initialize EPOW interrupt handler
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called during system initialization. It is
 *	called out of the init table after timer initialization.
 *
 * RETURN VALUE DESCRIPTION:
 *	none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_init
 *	assert
 */

epowinit()
{
	extern	 void		epow_timer(); /* epow timer handler  */
	extern	 int		i_epow();  /* epow interrupt handler */

	int	rc;		/* Return code */

#ifdef _RSPC_UP_PCI
	if( !__rspc_up_pci() ) {
#endif /* _RSPC_UP_PCI */
		/* Allocate the EPOW timer block */
		epow_trb = (struct trb *)talloc();
		epow_trb->flags = 0;
		epow_trb->ipri = INTEPOW;
		epow_trb->func = (void (*)())epow_timer;
		epow_trb->func_data = (ulong)0;
	
		/*
		 * Define the handler for the EPOW interrupts.
		 * This handler processes EPOW conditions.
		 */
		INIT_EPOW( &epowhand, i_epow, 0 );

		/*
		 *  Register the EPOW interrupt handler.
		 */
		rc = i_init( &epowhand );
		assert( rc == INTR_SUCC );
		epowhandler = &epowhand;
#ifdef _RS6K_SMP_MCA
		if ( __rs6k_smp_mca()) {
		  /*
		   * On a RS6K_SMP_MCA plateform, the epow interrupt goes
		   * through an MCA interrupt line. The epow handler got
		   * to be registered at two different level.
		   * The standard one, so a driver can still register
		   * its own epow handler.
		   * The other level is to handle the EPOW interrupt on
		   * RS6K_SMP_MCA plateform.
		   */
		  epowhand_dev.next = (struct intr *)NULL;
		  epowhand_dev.handler = i_epow;
		  epowhand_dev.bus_type = BUS_MICRO_CHANNEL;
		  epowhand_dev.flags = 0;
		  epowhand_dev.priority = INTEPOW;
		  epowhand_dev.level = 8;
		  epowhand_dev.bid = BUID_TO_IOCC(IOCC0_BUID);
		  rc = i_init( &epowhand_dev );
		  assert( rc == INTR_SUCC );
		  epowhandler = &epowhand_dev;

		  /*
		   * link this handle to the epow handle
		   */
		  epowhand.next = &epowhand_dev;
		}
#endif

#ifdef _RSPC_UP_PCI
	}
#endif /* _RSPC_UP_PCI */
}
