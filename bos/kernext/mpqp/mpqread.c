static char sccsid[] = "@(#)27	1.26  src/bos/kernext/mpqp/mpqread.c, sysxmpqp, bos411, 9434B411a 8/22/94 16:21:10";
/*
 * COMPONENT_NAME: (MPQP) Multiprotocol Quad Port Device Driver
 *
 * FUNCTION: mpqread
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

#include <sys/ddtrace.h>
#include <sys/device.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/intr.h>
#include <sys/malloc.h>
#include <sys/mpqp.h>
#include <sys/mpqpdd.h>
#ifdef _POWER_MPQP
#include "mpqpxdec.h"
#endif /* _POWER_MPQP */
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/trchkid.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/xmem.h>
#include <sys/comio.h>

/*******************************************************************
 *    External Declarations					   *
 *******************************************************************/

extern t_acb		*acb_dir[];	/* ACB directory */

extern t_mpqp_dds	*dds_dir[];	/* DDS directory */

/*******************************************************************
 *      Global declarations for the MPQP Device Driver             *
 *******************************************************************/
/*
 * NAME: mpqread
 *                                                                    
 * FUNCTION: mpqread is called to read from the mutliprotocol quad
 *           port adapter
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	Preemptable        : Yes
 *	VMM Critical Region: Yes
 *	Runs on Fixed Stack: Yes
 *	May Page Fault     : Yes
 *      May Backtrack      : Yes
 *                                                                   
 * NOTES: We set up structs, channel info, etc.  Then we thread through the
 *	  the receive buffer for new data.  We then deque things and return
 *
 * DATA STRUCTURES: uio, read_ext, t_acb, t_mpqp_dds, mbuf, t_sel_que,         
 *		    t_chan_info. 
 *
 * RETURN VALUE DESCRIPTION: Return the packet contents to the user.
 */
/**************************************************************************
 * Pseudocode:
 *
 * BEGIN mpqread:
 * END mpqread:
 */

int mpqread( dev_t 			devno,
	     struct uio			*uiop,
 	     int 			chan,
	     struct read_extension	*p_read_ext)

{
	int 		adapt_num;	/* adapter number, zero based   */
	int		port_num;	/* port number within adapter   */
	volatile int	old_pri;	/* interrupt level save element */
	u_short		pkt_hdr_len;	/* packet header length		*/
	u_short		pkt_length;	/* receive data length */
	u_short		pkt_status;	/* receive packet status */
	t_acb		*p_acb;		/* pointer to adapter control block */
	t_mpqp_dds	*p_dds;		/* pointer to device data structure */
	struct mbuf	*p_mbuf;	/* pointer to the mbuf in question */
	caddr_t 	p_pkt;		/* pointer to the received packet */
	u_short 	*p_shrt_pkt;	/* pointer to the received packet */
	t_sel_que	*p_rcv_elem;	/* pointer to the receive entry */
	volatile t_chan_info *p_tmp_chinfo; /* temporary channel info pointer */
	int		rc;		/* general return code */
	int		rc_sleep;	/* return code from e_sleep */

	/* log trace hook */

	DDHKWD5(HKWD_DD_MPQPDD, DD_ENTRY_READ, 0, devno, 0, 0, 0, 0);
	MPQTRACE4("ReaE", devno, uiop, p_read_ext);
#ifdef _POWER_MPQP
	MPQP_SIMPLE_LOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */

	/* if minor number is invalid, return error */
	if (minor(devno) >= (MAX_ADAPTERS*NUM_PORTS))
	{
		MPQTRACE2("Rea1", devno);
		DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_READ, EINVAL, devno, 0,0,0,0);

#ifdef _POWER_MPQP
        	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		return(EINVAL);
	}

	/* if the channel number out of range (only 0 is valid for now) */
	if ( chan != 0 )
	{
		MPQTRACE3("Rea2", devno, chan);
		DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_READ, ECHRNG, devno, 0, 0, 0, 0);
#ifdef _POWER_MPQP
        	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		return (ECHRNG);
	}

	/* get dds pointer from dds directory */
	p_dds = dds_dir[minor(devno)];

	/* if port not configured, return error */
	if (p_dds == NULL)
	{
		MPQTRACE2("Rea3", devno);
		DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_READ, ENXIO, devno, 0,0,0,0);
#ifdef _POWER_MPQP
        	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		return(ENXIO);
	}

	adapt_num = p_dds->dds_hdw.slot_num;
	p_acb = acb_dir[adapt_num];

	port_num = p_dds->dds_dvc.port_num;


	/* now we go out and get the channel information data structure */
	/* pointer from the dds for this channel.  The channel informa- */
	/* tion data structure contains select queue pointers and other */
	/* necessary information for our continued operation	    */

	p_tmp_chinfo = p_dds->dds_wrk.p_chan_info[chan];/* get that pointer */

	if /* a kernel process has possession of this channel, read should */
	/* never receive control....*/
	( p_tmp_chinfo->devflag & DKERNEL )
	{
		MPQTRACE4("Rea5", devno, adapt_num, p_tmp_chinfo->devflag);
		DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_READ, EINVAL, devno, 0,0,0,0);
#ifdef _POWER_MPQP
        	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		return (EINVAL);
	}

	/* disable interrupts so we can single thread */

#ifdef _POWER_MPQP
	MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
	old_pri = i_disable(INTOFFL3);
#endif /* _POWER_MPQP */

	while /* no packets are available on the queue */
	( p_tmp_chinfo->p_rcv_head == NULL)
	{
		if /* DNDELAY has been set, return immediately */
		( p_tmp_chinfo->devflag & DNDELAY )
		{
			/* end single thread */

#ifdef _POWER_MPQP
        		MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
			i_enable(old_pri);
#endif /* _POWER_MPQP */


			/* set length to zero */

			uiop->uio_resid = 0;

			/* no data, return zero, zero length */

			MPQTRACE4("Rea6", devno, p_mbuf, p_tmp_chinfo->devflag);
			DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_READ, 0, devno,0,0,0,0);
#ifdef _POWER_MPQP
        		MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
			return(0);
		}
		else /* NDELAY not set, suspend somehow until data is received */
		{

			/* now we go do an e_sleep */

#ifdef _POWER_MPQP
			rc_sleep = e_sleep_thread(&(p_tmp_chinfo->rcv_event_lst), &mpqp_mp_lock, EVENT_SIGRET);
#else
			rc_sleep = e_sleep(&(p_tmp_chinfo->rcv_event_lst), 
					   EVENT_SIGRET);
#endif /* _POWER_MPQP */

			MPQTRACE1("Rea7");

			if ( rc_sleep != EVENT_SUCC )
			{
				MPQTRACE4("Rea8", devno, p_mbuf, rc_sleep);
				DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_READ, EINTR, 
				    devno, 0, 0, 0, 0);
#ifdef _POWER_MPQP
                        	MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
        			MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#else
                        	i_enable(old_pri);
#endif /* _POWER_MPQP */

				return( EINTR );
			}

		}

	}

	/* we have a message just a waiting for us so lets deque it and  */
	/* copy it's contents into the user's buffer.		     */

	p_rcv_elem = p_tmp_chinfo->p_rcv_head;	/* point to first elt */

	p_tmp_chinfo->p_rcv_head = p_rcv_elem->p_sel_que; /* deque it */

	/* copy the code field to the status field of read extension */

	if ( p_read_ext != (struct read_extension *)NULL )
	{
#ifdef _POWER_MPQP
          MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
	  i_enable(old_pri);		/* enable interrupts for copyout */
#endif /* _POWER_MPQP */

          if ( copyout( &(p_rcv_elem->stat_block.code), &(p_read_ext->status), 
               sizeof(p_rcv_elem->stat_block.code)) )
          {
              MPQTRACE2( "Rea9", p_read_ext );
#ifdef _POWER_MPQP
	      MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
              return( EFAULT );				/* bad read ext. addr */
          }
#ifdef _POWER_MPQP
          MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
	  old_pri = i_disable(INTOFFL3); /* disable interrupts after copyout */
#endif /* _POWER_MPQP */

	}

	/* get mbuf pointer */

	p_mbuf = (struct mbuf *)p_rcv_elem->stat_block.option[0];

	if /* receive head ptr is now null, make receive tail ptr null */
	( p_tmp_chinfo->p_rcv_head == NULL )
	{
		p_tmp_chinfo->p_rcv_tail = NULL;
	}

	/* now we zero out the select queue element and add it back  */
	/* to the select queue available chain */

	p_rcv_elem->rqe_value = 0;
	p_rcv_elem->stat_block.code = 0;
	p_rcv_elem->stat_block.option[0] = 0;
	p_rcv_elem->p_sel_que = p_tmp_chinfo->p_sel_avail;
	p_tmp_chinfo->p_sel_avail = p_rcv_elem;

#ifdef _POWER_MPQP
        MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
	i_enable(old_pri);			/* reenable interrupts */
#endif /* _POWER_MPQP */


	/* If p_mbuf is NULL. Then there is a status, not a receive buffer */
	if ( p_mbuf == (struct mbuf *)NULL )
	{
		MPQTRACE2("Rbuf", p_mbuf);
#ifdef _POWER_MPQP
	      	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		return (0);
	}

	p_pkt = MTOD(p_mbuf, caddr_t);	/* get the buffer address */

	p_shrt_pkt = (u_short *)p_pkt;

	/* get information from packet header... */

	pkt_hdr_len = PIO_GETSR(p_shrt_pkt++);
	pkt_length = PIO_GETSR(p_shrt_pkt++);
	pkt_status = PIO_GETSR(p_shrt_pkt);

	/* point packet address to start past header */

	p_pkt = p_pkt + pkt_hdr_len;

	MPQTRACE5("Ruio", p_pkt, pkt_length, p_mbuf, uiop);

	/* now we attempt to move the packet contents to the user area */

	rc = uiomove(p_pkt, (unsigned int)pkt_length, UIO_READ, uiop);

	/* now we free the mbuf */

	m_free( p_mbuf );

	MPQTRACE1("ReaX");
	DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_READ, rc, devno, 0, 0, 0, 0);
#ifdef _POWER_MPQP
	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
	return(rc);

}
