static char sccsid[] = "@(#)76	1.31  src/bos/kernext/mpqp/mpqclose.c, sysxmpqp, bos411, 9434B411a 8/22/94 16:19:39";

/*
 * COMPONENT_NAME: (MPQP) Multiprotocol Quad Port Device Driver
 *
 * FUNCTIONS: mpqclose
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
#include <sys/device.h>
#include <sys/ddtrace.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/ioctl.h>
#include <sys/malloc.h>
#include <sys/timer.h>
#include <sys/sleep.h>
#include <sys/mpqp.h>
#include <sys/mpqpdd.h>
#ifdef _POWER_MPQP
#include "mpqpxdec.h"
#endif /* _POWER_MPQP */
#include <sys/sysmacros.h>
#include <sys/trchkid.h>
#include <sys/types.h>

/*******************************************************************
 *    External Function Declarations                               *
 *******************************************************************/
extern void mpqtimer(t_mpqp_dds *);

extern  int stop_port( unsigned char,
			t_acb	*,
			t_mpqp_dds	*,
			int,
			dev_t,
			unsigned int );

extern int flush_port( unsigned char,
			t_acb	*,
			t_mpqp_dds	*,
			int,
			dev_t,
  			unsigned int );

/*******************************************************************
 *      External declarations for the MPQP Close Function	   *
 *******************************************************************/

extern t_acb		*acb_dir[];	/* ACB directory */

extern t_mpqp_dds	*dds_dir[];	/* DDS directory */

/*
 * NAME: mpqclose
 *                                                                    
 * FUNCTION: Called to close a single port of the multiprotocol 
 *           adaptor.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	Preemptable        : Yes
 *	VMM Critical Region: Yes
 *	Runs on Fixed Stack: Yes
 *	May Page Fault     : No
 *                                                                   
 * (NOTES:) More detailed description of the function, down to
 *	what bits / data structures, etc it manipulates. 
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *	software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */

int mpqclose ( dev_t 		devno,
		int 		chan,
		int 		ext )

{

	int 		adapt_num;	/* adapter number, zero based */
	int 		port_num;	/* port number within adapter */
	t_acb           *p_acb;         /* pointer to adapter control block */
	t_chan_info 	*p_tmp_chinfo;  /* temporary channel info pointer   */
	t_mpqp_dds	*p_dds;		/* pointer to device data structure */
	unsigned int	rc;		/* generic return code */
	int		old_pri;	/* interrupt level	*/
	unsigned long 	bus_sr;		/* bus segment reg	*/
	unsigned char 	*p_io;		/* pointer to io reg	*/
	unsigned char	comreg;		/* MPQP command reg	*/
	unsigned int    ndelay;     	/* que_cmd sleep flag   */

	/* log a trace hook */
	DDHKWD5 (HKWD_DD_MPQPDD, DD_ENTRY_CLOSE,0,devno,chan,0,0,0);
	MPQTRACE4("ClsE", devno, chan, ext);

#ifdef _POWER_MPQP
        MPQP_SIMPLE_LOCK(&mpqp_mp_lock);
#endif /* _POWER_MPQP */
	ndelay = FALSE;			/* always sleep */

	/* if minor number is invalid, return error */
	if (minor(devno) >= (MAX_ADAPTERS*NUM_PORTS))
	{
		MPQTRACE2("Cle1", devno);
		DDHKWD5(HKWD_DD_MPQPDD,DD_EXIT_CLOSE,EINVAL,devno,0,0,0,0);
#ifdef _POWER_MPQP
                MPQP_SIMPLE_UNLOCK(&mpqp_mp_lock);
#endif /* _POWER_MPQP */

		return(EINVAL);
	}

	/* if the channel number out of range (only 0 is valid for now) */
	if ( chan != 0 )
	{
		MPQTRACE3("Cle2", devno, chan);
		DDHKWD5(HKWD_DD_MPQPDD,DD_EXIT_CLOSE,ECHRNG,devno,chan,0,0,0);
#ifdef _POWER_MPQP
                MPQP_SIMPLE_UNLOCK(&mpqp_mp_lock);
#endif /* _POWER_MPQP */

		return(ECHRNG);
	}

	/* get dds pointer from dds directory */
	p_dds = dds_dir[minor(devno)];

	/* if port not configured, return error */
	if (p_dds == NULL)
	{
		MPQTRACE2("Cle3", devno);
		DDHKWD5(HKWD_DD_MPQPDD,DD_EXIT_CLOSE,EINVAL,devno,0,0,0,0);
#ifdef _POWER_MPQP
                MPQP_SIMPLE_UNLOCK(&mpqp_mp_lock);
#endif /* _POWER_MPQP */

		return(EINVAL);
	}

	adapt_num = p_dds->dds_hdw.slot_num;
	p_acb = acb_dir[adapt_num];

	port_num = p_dds->dds_dvc.port_num;

	/* now we follow down to remove the select queue data structure */
	/* the channel information data structure and null out the      */
	/* pointer in the dds to the channel info ds		        */

	if /* port state is data transfer/started, halt */
	( ( p_dds->dds_dvc.port_state == DATA_XFER ) ||
	    ( p_dds->dds_dvc.port_state == START_REQUESTED ) )
	{
	        if ( p_acb->n_open_ports == 1 )	/* last close? */
	            disable_recv( p_acb );	/* disable frame reception */

		/* we are closing before halting so we must halt and */
		/* possibly flush the port.... */

		/* disable interrupts before issuing halt command to adapter */
#ifdef _POWER_MPQP
		MPQP_LOCK_DISABLE(old_pri,&mpqp_intr_lock);
#else
		old_pri = i_disable(INTOFFL3);
#endif /* _POWER_MPQP */

		p_dds->dds_dvc.port_state = HALT_REQUESTED;

		if ((rc = stop_port(port_num, p_acb, p_dds, chan, devno, 
			ndelay )) != 0)
		{
#ifdef _POWER_MPQP
			MPQP_UNLOCK_ENABLE(old_pri,&mpqp_intr_lock);
			MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#else
			i_enable(old_pri);
#endif /* _POWER_MPQP */

			return(ENOTREADY);
		}
		if ( !ndelay )
		{
		    MPQTRACE3("Clst", p_dds, WRK.halt_sleep_event);
		    timeout((int (*)(void))mpqtimer, (int)p_dds, (HZ*5));
		    p_dds->dds_wrk.sleep_on_halt = TRUE;
		    p_dds->dds_wrk.halt_sleep_event = EVENT_NULL;

		    /* set 5 sec timer in case halt does not complete */
#ifdef _POWER_MPQP
		    MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
		    e_sleep_thread((void *)&WRK.halt_sleep_event,&mpqp_mp_lock,EVENT_SHORT);
		    MPQP_LOCK_DISABLE(old_pri,&mpqp_intr_lock);
#else
		    e_sleep((void *)&WRK.halt_sleep_event,EVENT_SHORT);
#endif /* _POWER_MPQP */
		}

#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE(old_pri,&mpqp_intr_lock);
#else
		i_enable(old_pri);
#endif /* _POWER_MPQP */

		MPQTRACE2("Cls4", p_dds);
	}


	free_xmit( p_acb, p_dds );	/* free xmit resources */

	/* set port state to close requested */

	p_dds->dds_dvc.port_state = CLOSE_REQUESTED;

	p_tmp_chinfo = p_dds->dds_wrk.p_chan_info[chan];

	if /* device was opened by a kernel llc */
	( p_tmp_chinfo->devflag & DKERNEL )
	{
		p_tmp_chinfo->mpq_kopen.rx_fn = NULL;
		p_tmp_chinfo->mpq_kopen.stat_fn = NULL;
		p_tmp_chinfo->mpq_kopen.tx_fn = NULL;
		p_tmp_chinfo->mpq_kopen.open_id = 0;
		xmfree((unsigned char *)p_tmp_chinfo->p_sel_avail, pinned_heap);
                p_tmp_chinfo->p_sel_avail = NULL;

	}
	else /* device was opened by user level llc */
	{
	 	free_rcv_list( p_dds );         /* free queued mbufs */

                p_tmp_chinfo->p_sel_avail = NULL;
                p_tmp_chinfo->p_rcv_head = NULL;
                p_tmp_chinfo->p_rcv_tail = NULL;
                p_tmp_chinfo->p_stat_head = NULL;
                p_tmp_chinfo->p_stat_tail = NULL;
                p_tmp_chinfo->p_lost_stat = NULL;
                p_tmp_chinfo->p_lost_rcv = NULL;
		xmfree((unsigned char *)p_tmp_chinfo->p_beg_page, pinned_heap);
                p_tmp_chinfo->p_beg_page = NULL;
  
	}

	p_tmp_chinfo->devflag = 0; 	/* remove device flags */

	WRK.num_starts = 0;

	if (--p_acb->n_open_ports == 0)
	{
		free_recv( p_acb, p_dds );	/* free recv resources */

		/* This is the last close for the adapter. It's time to */
		/* prevent this adapter from generating interrupts.     */

		MPQTRACE3("Cla1", devno, p_acb);

		/* disable interrupts from the adapter. */

		bus_sr = BUSIO_ATT(p_acb->io_segreg_val,0);

		p_io = (unsigned char *)( p_acb->io_base + bus_sr);

		comreg = PIO_GETC( p_io + COMREG );

		PIO_PUTC(p_io + COMREG, comreg & ~COM_IE );

		BUSIO_DET(bus_sr);

		MPQTRACE2("Clc6", devno);

		p_acb->num_starts = 0;

	}
	else 
	{
	  /* make sure the number of starts do not exceed the number of	*/
	  /* valid open ports.						*/
	  if ( p_acb->num_starts > p_acb->n_open_ports )
	       p_acb->num_starts = p_acb->n_open_ports;
	}

	/* always reset diagnostic flag */
	p_acb->diag_flag = 0;

	/* set port state to closed */
	p_dds->dds_dvc.port_state = CLOSED;

	MPQTRACE4("ClsX", devno, p_dds, p_dds->dds_dvc.port_state);
#ifdef _POWER_MPQP
        MPQP_SIMPLE_UNLOCK(&mpqp_mp_lock);
#endif /* _POWER_MPQP */
	DDHKWD5(HKWD_DD_MPQPDD,DD_EXIT_CLOSE,0,devno,chan,0,0,0);

	return(0);
}
