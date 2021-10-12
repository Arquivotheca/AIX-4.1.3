static char sccsid[] = "@(#)26	1.20.1.3  src/bos/kernext/mpqp/mpqintr.c, sysxmpqp, bos411, 9434B411a 8/22/94 16:19:54";
/*
 * COMPONENT_NAME: (MPQP) Multiprotocol Quad Port Device Driver
 *
 * FUNCTIONS: mpqintr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <errno.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/intr.h>
#include <sys/mpqp.h>
#include <sys/mpqpdd.h>
#ifdef _POWER_MPQP
#include "mpqpxdec.h"
#endif /* _POWER_MPQP */
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>

/*******************************************************************
 *    External Function Declarations                               *
 *******************************************************************/


/*******************************************************************
 *    Internal Function Declarations                               *
 *******************************************************************/

int 	mpqintr(struct intr *);		/* SLIH function */

/*******************************************************************
 *      Global declarations for the MPQP Device Driver             *
 *******************************************************************/

t_acb		*acb_dir[NUM_SLOTS * MAX_BUSES];   /* ACB directory */

t_mpqp_dds	*dds_dir[MAX_ADAPTERS*NUM_PORTS];  /* DDS directory */

/*
 * NAME: mpqintr
 *                                                                    
 * FUNCTION: This procedure is the interrupt handler.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	Environment-Specific aspects, such as - 
 *	Preemptable        : No
 *	VMM Critical Region: Yes
 *	Runs on Fixed Stack: Yes
 *	May Page Fault     : Yes
 *      May Backtrack      : Yes
 *                                                                   
 * NOTES: This procedure checks the level and task registers to verify that
 *	  this is our interrupt and processes it
 *
 * DATA STRUCTURES: intr -p_intr_ds and external t_acb -p_acb
 *
 * RETURN VALUE DESCRIPTION: Either INTR_SUCC (interrupt handled successfully)
 *			     or INTR_FAIL (interrput was not ours
 */

int mpqintr ( struct intr	*p_intr_ds )

{
	unsigned long	bus_sr;
	t_acb		*p_acb;		/* adapter control block pointer */
	unsigned char	*p_io;
	unsigned char	serviced;
	unsigned char	taskreg;
	unsigned int	old_pri;

	DDHKWD5 (HKWD_DD_MPQPDD, DD_ENTRY_INTR,0,0,0,0,0,0);

#ifdef _POWER_MPQP
        MPQP_LOCK_DISABLE(old_pri,&mpqp_intr_lock);
#endif /* _POWER_MPQP */

	p_acb = (t_acb *)p_intr_ds;

	MPQTRACE3("intr", p_acb, p_acb->int_lvl );


	serviced = 0;		/* default is we didn't catch it */

	/* set up bus access and get the io base addr  */
	bus_sr = BUSIO_ATT(p_acb->io_segreg_val,0);
	p_io = (unsigned char *)( p_acb->io_base + bus_sr);

	/* read interrupt register, TASKREG, on the adapter */

        if ( (taskreg = PIO_GETC( p_io + TASKREG)) != TR_NOI) 
        { /* if this is our interrupt */
		
		/* check to see if the port is opened */
                if ( p_acb->n_open_ports != 0 )
                {

			/* switch based on the value of the interrupt register */
	
			switch (taskreg)
			{
	
			case TR_WDE:    /* watchdog timer expired */
				serviced = 1;
				p_acb->c_intr_rcvd++;   /* increment int. count */
				break;          /* we ignore this interrupt */

			case TR_DMA0:           /* Port 0 DMA Complete */
			case TR_DMA1:           /* Port 1 DMA Complete */
			case TR_DMA2:           /* Port 2 DMA Complete */
			case TR_DMA3:           /* Port 3 DMA Complete */
				serviced = 1;
				p_acb->c_intr_rcvd++;   /* increment int. count */

				/* save taskreg val */
				p_acb->cur_intr_val = taskreg;

				MPQTRACE2("int2", p_acb->cur_intr_val);
				break;

			case TR_TXFL:   /* Transmit Free List Available i.e.
				   	command blocks available     */
				serviced = 1;
				p_acb->c_intr_rcvd++;   /* increment int. count */

				/* save taskreg val */
				p_acb->cur_intr_val = taskreg;
				p_acb->adapter_state &= ~SUSPENDED;
	
				/* wake up ports waiting for command blocks */
				e_wakeup( &p_acb->txfl_event_lst );
				break;

			default:        /* most real ints caught here */
				serviced = 1;
				p_acb->c_intr_rcvd++;   /* increment int. count */

				/* save taskreg val */
				p_acb->cur_intr_val = taskreg;

				/* sched the offlevel */
				if (p_acb->arq_sched != TRUE)
				{
			    		MPQTRACE1("int3");
			    		p_acb->arq_sched = TRUE;
			    		i_sched(&p_acb->offl.offl_intr);
				}
				else
				{
			    		MPQTRACE2("in3a", p_acb->arq_sched);
				}
				break;

			} /* end of switch based on taskreg value */

		}
	}
	BUSIO_DET( bus_sr );	/* restore addressing on bus */

	if /* we fielded this interrupt, tell the FLIH */
	(serviced)
	{
		i_reset( p_intr_ds );	/* reset to catch other interrupts */
		MPQTRACE1("IinE");
		DDHKWD5 (HKWD_DD_MPQPDD, DD_EXIT_INTR, 0,0,0,0,0,0);
#ifdef _POWER_MPQP
      		MPQP_UNLOCK_ENABLE(old_pri,&mpqp_intr_lock);
#endif /* _POWER_MPQP */


		return (INTR_SUCC);
	}
	MPQTRACE2("IinX", taskreg);

#ifdef _POWER_MPQP
      	MPQP_UNLOCK_ENABLE(old_pri,&mpqp_intr_lock);
#endif /* _POWER_MPQP */

	return ( INTR_FAIL ); /* not our interrupt, tell the FLIH */
}
