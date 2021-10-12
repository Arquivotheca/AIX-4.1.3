static char sccsid[] = "@(#)81	1.30  src/bos/kernext/mpqp/mpqopen.c, sysxmpqp, bos411, 9434B411a 8/22/94 16:20:57";

/*
 * COMPONENT_NAME: (MPQP) Multiprotocol Quad Port Device Driver
 *
 * FUNCTIONS: mpqopen
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
#include <sys/types.h>
#include <sys/comio.h>
#include <sys/device.h>
#include <sys/intr.h>
#include <sys/ioctl.h>
#include <sys/malloc.h>
#include <sys/mpqp.h>
#include <sys/mpqpdd.h>
#ifdef _POWER_MPQP
#include "mpqpxdec.h"
#endif /* _POWER_MPQP */
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>

/*******************************************************************
 *    External Declarations	                                   *
 *******************************************************************/

extern t_acb		*acb_dir[];	/* ACB directory */

extern t_mpqp_dds	*dds_dir[];	/* DDS directory */

/*******************************************************************
 *      Global declarations for the MPQP Device Driver             *
 *******************************************************************/

mpqopen ( dev_t 		devno,
	  unsigned long		devflag,
	  int			chan,
	  struct kopen_ext	*p_ext )

{
	int 		adapt_num;	/* adapter number, zero based	*/
	int		c_loop;		/* loop control counter 	*/
	int		ilev;		/* interrupt level for adapter 	*/
	int		port_num;	/* port number within adapter  	*/
	t_acb		*p_acb;		/* ptr to adapter control block	*/
	t_chan_info 	*p_tmp_chinfo;  /* temporary channel info ptr 	*/
	t_mpqp_dds	*p_dds;		/* ptr to device data structure	*/
	t_sel_que	*p_sq_elem1;	/* work ptr to select queue element*/
	t_sel_que	*p_sq_elem2;	/* work ptr to select queue element*/
	int		rc;		/* general return code 		*/
	int		old_pri;	/* interrupt level		*/
	unsigned long 	bus_sr;		/* bus segment reg		*/
	unsigned char 	*p_io;		/* ptr to io base		*/
	unsigned char 	comreg;		/* COMREG on Portmaster		*/

	/* put the trace hook for open entry */

	DDHKWD5(HKWD_DD_MPQPDD, DD_ENTRY_OPEN, 0, devno, 0, 0, 0, 0);
	MPQTRACE4("OpnE", devno, devflag, chan);
#ifdef _POWER_MPQP
        MPQP_SIMPLE_LOCK( &(mpqp_mp_lock) );
#endif /* _POWER_MPQP */


	/* if minor number is invalid, return error */
	if (minor(devno) >= (MAX_ADAPTERS*NUM_PORTS))
	{
		MPQTRACE2("Ope1", devno);
		DDHKWD5( HKWD_DD_MPQPDD, DD_EXIT_OPEN, EINVAL, devno, 0,0,0,0);
#ifdef _POWER_MPQP
        	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		return(EINVAL);
	}

	/* if the channel number out of range (only 0 is valid for now) */
	if ( chan != 0 )
	{
		MPQTRACE3("Ope2", devno, chan);
		DDHKWD5( HKWD_DD_MPQPDD, DD_EXIT_OPEN, ECHRNG, devno, 0,0,0,0);
#ifdef _POWER_MPQP
        	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		return(ECHRNG);
	}

	/* get dds pointer from dds directory */
	p_dds = dds_dir[minor(devno)];

	/* if port not configured, return error */
	if (p_dds == NULL)
	{
		MPQTRACE3("Ope3", devno, chan);
		DDHKWD5( HKWD_DD_MPQPDD, DD_EXIT_OPEN, EINVAL, devno, 0,0,0,0);
#ifdef _POWER_MPQP
        	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		return(EINVAL);
	}

	adapt_num = p_dds->dds_hdw.slot_num;
	p_acb = acb_dir[adapt_num];

	port_num = p_dds->dds_dvc.port_num;

	MPQTRACE4("Opn1", p_dds, adapt_num, p_acb);
	MPQTRACE4("Opn2", port_num, p_dds->dds_dvc.port_state,
	    p_acb->n_open_ports);

	if /* port state != unopened, closed or dormant */
	(p_dds->dds_dvc.port_state != DORMANT_STATE)
	{
		/* can't allow multiple simultaneous opens */
		MPQTRACE3("Ope4", devno, p_dds->dds_dvc.port_state);
		DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_OPEN, EBUSY, devno, 0, 0,0,0);
#ifdef _POWER_MPQP
        	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		return (EBUSY);
	}

	p_dds->dds_dvc.port_state = OPEN_REQUESTED;

	/* here we check to see whether any ports have been opened on */
	/* the indicated adapter.  If none have, we will enable the   */
	/* interrupts on the adapter.                                 */

	if /* no port opens have occured for this adapter */
	(p_acb->n_open_ports == 0)
	{
		MPQTRACE2("Opa1", p_acb);

		p_acb->cmd_queue_lock = LOCK_AVAIL;

		p_acb->halt_complete = TRUE;  /* no pending HALTS outstanding */

		MPQTRACE4("Opa2", devno, p_acb, rc);

		/* enable interrupts on the adapter */

		bus_sr = BUSIO_ATT(p_acb->io_segreg_val, 0);

		p_io = (unsigned char *)( p_acb->io_base + bus_sr );

		comreg = PIO_GETC( p_io + COMREG );

		PIO_PUTC( p_io + COMREG, comreg | COM_IE );

		BUSIO_DET( bus_sr );

	} /* end of no open ports loop on this adapter */

	if /* first time through successfully, allocate channel structure */
	(p_dds->dds_wrk.p_chan_info[chan] == NULL)
	{
		/* allocate memory for channel related structures */

		p_dds->dds_wrk.p_chan_info[chan] = p_tmp_chinfo =
		    (t_chan_info *)xmalloc((uint)sizeof(t_chan_info),(uint)2,
					pinned_heap);
		MPQTRACE2("Opc1", p_tmp_chinfo);

		if /* memory allocation failed */
		( p_tmp_chinfo == NULL)
		{
			MPQTRACE2("Ope6", p_dds);
			DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_OPEN, ENOMEM, devno,0,
				0,0,0);
#ifdef _POWER_MPQP
        		MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
			return(ENOMEM);
		}

		bzero((void *)p_tmp_chinfo, (uint)sizeof(t_chan_info));
		/* set major/minor device number */
		p_tmp_chinfo->devno = devno;	
		p_tmp_chinfo->rcv_event_lst = EVENT_NULL;
		p_tmp_chinfo->xmt_event_lst = EVENT_NULL;
		p_acb->txfl_event_lst = EVENT_NULL;

	}

	/* now we fetch the temporary channel info pointer */

	p_tmp_chinfo = p_dds->dds_wrk.p_chan_info[chan];

	/* set common values for user and kernel llc calls */

	p_tmp_chinfo->devflag = devflag; /* device flags opened with */

	/* now we diverge for kernel or user mode open */

	if /* open call is from a kernel extension llc */
	( devflag & DKERNEL )
	{
		MPQTRACE1("Opk1");

		/* save function call entry points */

		/* receive function */
		p_tmp_chinfo->mpq_kopen.rx_fn = p_ext->rx_fn;

	 	/* status function */
		p_tmp_chinfo->mpq_kopen.stat_fn = p_ext->stat_fn;

		/* transmit function */
		p_tmp_chinfo->mpq_kopen.tx_fn = p_ext->tx_fn;

 		/* open identifier */
		p_tmp_chinfo->mpq_kopen.open_id = p_ext->open_id;

		/* allocate a single select queue element for single thread */
		/* llc processing of receives and status on a port */

		p_tmp_chinfo->p_sel_avail = 
			(t_sel_que *)xmalloc((uint)sizeof(t_sel_que), (uint)2,
						pinned_heap);

		if /* memory allocation failed... */
		( p_tmp_chinfo->p_sel_avail == NULL )
		{
			/* free channel information block...*/

			xmfree((void *)p_tmp_chinfo,pinned_heap);

			/* reset channel info block pointer in dds */

			p_dds->dds_wrk.p_chan_info[port_num] = NULL;

			/* return error, no memory available */

			MPQTRACE3("Ope7", devno, p_dds);
			DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_OPEN, ENOMEM, devno, 
				0,0,0,0);
#ifdef _POWER_MPQP
        		MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
			return (ENOMEM);
		}

		p_ext->status = CIO_OK;	/*set status to OK */

	}
	else /* open is called from a user process llc */
	{
	MPQTRACE3("Opu1", port_num, chan);

		/* First we to allocate a page of memory for the select    */
		/* queue elements.  Allowing select events to be queued    */
		/* before the system dies.  This should handle the port    */
		/* because the llc should handle things so that there are  */
		/* never that many outstanding events */

		p_tmp_chinfo->p_sel_avail = (t_sel_que *)xmalloc(4096,12,
						pinned_heap);

		if /* memory allocation failed... */
		( p_tmp_chinfo->p_sel_avail == NULL )
		{
			/* free channel information block...*/

			xmfree((void *)p_tmp_chinfo,pinned_heap);

			/* reset channel info block pointer in dds */

			p_dds->dds_wrk.p_chan_info[port_num] = NULL;

			/* return error, no memory available */

			MPQTRACE3("Ope8", devno, p_dds);
			DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_OPEN, ENOMEM, devno, 
				0,0,0,0);
#ifdef _POWER_MPQP
        		MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
			return (ENOMEM);
		}

		/* save address of select queue page for later free in close */

		p_tmp_chinfo->p_beg_page = p_tmp_chinfo->p_sel_avail;

		/* zero out select queue */
		bzero((void *)p_tmp_chinfo->p_sel_avail, 4096);	

		/* use two select queue elements for CIO_LOST_STATUS & */
                /* CIO_AYSNC_STATUS (CIO_LOST_DATA) responses */
 
		p_tmp_chinfo->p_lost_stat = (p_tmp_chinfo->p_sel_avail)++;
		p_tmp_chinfo->p_lost_rcv = (p_tmp_chinfo->p_sel_avail)++;

		/* now link all of the select queue elements to the available */
		/* chain & set the receive and status pointers to NULL	 */

		p_sq_elem1 = p_sq_elem2 = p_tmp_chinfo->p_sel_avail;

		c_loop = (4096/sizeof(t_sel_que)) - 2; 

		while /* the number of select queue elements in a 4K page */
		( --c_loop )
		{
			p_sq_elem2++;
			p_sq_elem1->p_sel_que = p_sq_elem2;
			p_sq_elem1++;
		}
		p_sq_elem1->p_sel_que = NULL;
		p_tmp_chinfo->p_rcv_head = NULL;
		p_tmp_chinfo->p_rcv_tail = NULL;
		p_tmp_chinfo->p_stat_head = NULL;
		p_tmp_chinfo->p_stat_tail = NULL;

	} /* end of open call from user level llc code */

	if /* the current open request was for diagnostic open */
	( p_acb->diag_flag == 1 )
	{
		d_unmask( p_acb->dma_channel_id );
		p_acb->diag_flag = 2;
	}

	/* set port state variable to open */

	p_dds->dds_dvc.port_state = OPEN;
	p_dds->dds_wrk.buff_ctr = 0;	/* intialize the rx buffers */

	p_acb->n_open_ports++; /* increment number of open ports */

	MPQTRACE4("OpnX", devno, p_acb->n_open_ports, p_acb->n_open_ports);
	DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_OPEN,0, devno, 0, 0, 0, 0);
#ifdef _POWER_MPQP
        MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */

	return(0);
}
