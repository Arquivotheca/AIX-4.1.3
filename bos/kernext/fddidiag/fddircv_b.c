static char sccsid[] = "@(#)90	1.2  src/bos/kernext/fddidiag/fddircv_b.c, diagddfddi, bos411, 9428A410j 11/8/93 09:51:56";
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: FDDI_REARM
 *		fddi_nullsap
 *		fddi_rcv_deque
 *		fddi_start_rcv
 *		rcv_enque
 *		rcv_frame
 *		rcv_handler
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "fddiproto.h"
#include <sys/ioacc.h>
#include <sys/adspace.h>
#include <sys/dma.h>
#include <sys/mbuf.h>
#include <sys/errno.h>
#include <sys/poll.h>
#include "fddi_comio_errids.h"
#include <sys/sleep.h>
#include <sys/trchkid.h>	/* for performance hooks */

/*
 * NAME: fddi_rcv_deque, and rcv_enque
 *                                                                    
 * FUNCTION: dequeue a rcv'd packet in response to a user process read and
 *	     enqueue a rcv'd packet from the fiber onto a user process' queue
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 *	Queuing works like this:
 *
 *	head _________>  _______      ______   Multiple buffers per frame are
 *	tail __		|	|    |	    |	linked together with 'm_next'.	
 *	      |		|	|--->|	    |--->\
 *	      |		|_______|    |______|
 *	      |		    |
 *	      |		    | 		Frames linked together with 'm_nextpkt'.
 *	      |		 ___V___     
 *	      |		|	|   	
 *	      |		|	|--->\
 *	      |		|_______|    
 *	      |		    |
 *	      |		   ...
 *	      |_______>	 ___V___     
 *			|	|   	
 *			|	|--->\
 *			|_______|    
 *			    |
 *			   ---  	end of queue is guarenteed to be NULL
 *
 * RETURNS: 
 */  

int
fddi_rcv_deque (
	register	fddi_acs_t	*p_acs,
	register 	fddi_open_t	*p_open,
	register 	struct mbuf	**p_frame)
{
	int			ipri;

	/*
	 * serialize with rcv_enque running at higher priority
	 *	(Or we could grap the wrong head->m_nextpkt)
	 */
	ipri = i_disable (INTOFFL1);

	/* 
	 * Check the rcv queue to see if there is anything to read.
	 */
	if (p_open->rcv_cnt == 0)
	{
		/* Check for no delay */
		if (p_open->devflag & DNDELAY)
		{
			/* 
			 * nothing to read and caller doesn't want to wait 
			 */
			(*p_frame) = 0;
			i_enable(ipri);
			return (0); 
		}
		if (e_sleep (&(p_open->rcv_event), EVENT_SIGRET) != EVENT_SUCC)
		{
			/* 
			 * interrupted system call 
			 */
			i_enable(ipri);
			return (EINTR);
		}
		if (p_acs->dev.state != FDDI_OPEN)
		{
			/*
			 * state changed while we slept
			 */
			i_enable(ipri);
			return(EINVAL);
		}
	}

	/* deque frame */
	if (((*p_frame) = p_open->p_rcv_q_head) != NULL)
	{
		p_open->p_rcv_q_head = p_open->p_rcv_q_head->m_nextpkt;

		/* decrement queue count */
		p_open->rcv_cnt--;

		/* return frame */
		(*p_frame)->m_nextpkt = NULL;
	}

	i_enable(ipri);
	return (0);
}

void
rcv_enque (
	register fddi_open_t	*p_open,
	register struct mbuf	*p_frame)
{
	/*
	 * If the queue is not at the maximum size ... (the max is set in the 
	 * dds by the user at configure time
	 */
	if (p_open->p_acs->dds.rcv_que_size != p_open->rcv_cnt)
	{

		if (p_open->p_rcv_q_head == NULL)
		{
			/* enque frame to empty queue */
			p_open->p_rcv_q_head = p_frame;
		}
		else
		{
			/* enque frame to end of queue */
			p_open->p_rcv_q_tail->m_nextpkt = p_frame;
		}

		/* adjust tail ptr */
		p_open->p_rcv_q_tail = p_frame;
		p_open->p_rcv_q_tail->m_nextpkt = NULL;
	
		/* increment queue count */
		p_open->rcv_cnt++;
		if ( p_open->rcv_cnt > p_open->p_acs->ras.cc.rec_que_high)
			p_open->p_acs->ras.cc.rec_que_high = p_open->rcv_cnt;
	}
	else
	{
		p_open->p_acs->ras.ds.rcv_que_ovflw++;
		m_freem(p_frame);
	}

	/* done */
	return;
}

/*
 * NAME: rcv_frame
 *                                                                    
 * FUNCTION: process a completed frame
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt thread only
 *                                                                   
 * NOTES: 
 *
 *	Called from: rcv_handler()
 *	Calls to:
 *		p_open->rcv_fn ()
 *		m_getclust ()
 *		bcopy ()
		rcv_enqueu ()
 *
 *	Check for errors:
 *		dma errors
 *		netid not ours
 *		illegal frame size
 *
 *		Check for routing information upto 14 hops (30 bytes)
 *	Error code is at the end of this routine to avoid code cache hits.
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 *		The rcv buf is processed here and usually given to the
 *	user. In all cases the data is copied out of the 4500 byte malloc'd
 * 	memory on the descriptor list and then given to the user.  To rearm
 *	the descriptor the rearm_val struct is recopied to the descriptor list
 *	on the card.  The address of the buffer does not need to be recopied as
 *	the same buffer is reused each time.  Also there is always a buffer for
 *	the descriptor list as the same one is reused.  This hopefully prevents
 *	the card from dropping frames due to no space on the descriptor list.
 *		The rearm is coded to occur in each error condition (this is 
 *	keep the code from flowering under to many nested if-then-elses and 
 *	still allow the rearm to occur before the frame is given to the user in
 *	what might be an arbitrarily long period of time (relatively)).
 *
 * RETURNS:  void
 *
 */  

/*							
 * invalidate the cache for this buffer		
 */

#define FDDI_REARM()							\
{									\
	cache_inval(p_rcvd->p_buf, len);				\
	PIO_PUTSTRX(bus+p_rcvd->offset+offsetof(fddi_adap_t,cnt), 	\
			&p_acs->rcv.arm_val, 				\
			sizeof (p_acs->rcv.arm_val));			\
}

void
rcv_frame (
	fddi_acs_t	*p_acs,		/* */
	fddi_rcv_t	*p_rcvd, 	/* addr of the desct holding frame */
	short		len,		/* # of bytes copied into the frame */
	int		bus)		/* 
					 * buf holds the addr of the shared mem
					 * on the card
					 */
{
	struct mbuf	*p_frame;	/* points to the mbuf for the frame */
	int		rc, len1;
	cio_read_ext_t	read_ext;
	fddi_open_t	*p_open;
	netid_t		netid;
	fddi_hdr_t	*p_pac;		/* received packet ptr */

	FDDI_DBTRACE("RrfB", p_acs, p_acs->ctl.devno, len);
	/*
 	*	illegal frame size
 	*/
	if (len > FDDI_MAX_PACKET)
	{
		/* illegal frame size */
		p_acs->ras.cc.rx_err_cnt++;
		d_complete(p_acs->dev.dma_channel, DMA_READ | DMA_NOHIDE,
			p_rcvd->p_buf, p_acs->rcv.l_adj_buf,
			&p_acs->rcv.xmd, p_rcvd->p_d_addr);
			
		FDDI_REARM();
		FDDI_TRACE("Rrf1", len, FDDI_MAX_PACKET, 0);
		return;
	}

	/* do DMA cleanup for frame */
	rc = d_complete(p_acs->dev.dma_channel, DMA_READ | DMA_NOHIDE,
			p_rcvd->p_buf, len,
			&p_acs->rcv.xmd, p_rcvd->p_d_addr);
			
	if (rc != DMA_SUCC)
	{
		/* dma error(s) */
		fddi_logerr(p_acs, ERRID_FDDI_RCV,
			__LINE__, __FILE__);
		p_acs->ras.cc.rx_err_cnt++;
		FDDI_REARM();
		FDDI_TRACE("Rrf2", rc, DMA_SUCC, 0);
		return;
	}
	p_pac = (struct pac *)p_rcvd->p_buf;
	if (!(p_pac->fc & FDDI_FC_ADDR))
	{
		/*
		 * The frame has a short address, we only support long address 
		 * (48 bit addresses as opposed to 16 bit)
	 	 */
		p_acs->ras.ds.pkt_rej_cnt++;
		FDDI_REARM();
		FDDI_TRACE("Rrf3", p_pac->fc, FDDI_FC_LLC, 0);
		return;
	}


	if ((p_pac->fc & FDDI_FC_MSK) == FDDI_FC_LLC)
	{
		/*
	 	 * If there is routing, the first byte will be the 
	 	 * length of the routing info and can be used as an 
	 	 * index in the data section of the hdr to find the 
	 	 * netid just after the routing information. 
	 	 */
		if (p_pac->src[0] & FDDI_RI_SA)
			netid = p_pac->data[p_pac->data[0] & 0x1e];
		else 	
			netid = p_pac->data[0];

	}
	else 
	{
		
		netid = FDDI_SMT_NETID;
		FDDI_TRACE("RCV1",p_pac->fc, netid, p_acs->ctl.p_netids[netid]);
	}

	if ((p_open = p_acs->ctl.p_netids[netid]) == NULL)
	{
                /* if netid = 00 then see if we need to respond */
                if (netid == 0x00) {
                  FDDI_TRACE ("NET0", len, 0, 0);
                  fddi_nullsap (p_acs, p_pac, len);
		  FDDI_REARM();
		  return;
                }

		/* not our netid or illegal netid */
		p_acs->ras.ds.pkt_rej_cnt++;
		FDDI_REARM();
		FDDI_TRACE("Rrf4", netid, 0, 0);
		return;
	}

	/* 
	 * increment the rcv_frame count and the rcv_byte count.  Incrementing 
	 * both the low and high counts as appropriate.
	 */
	p_acs->ras.cc.rx_frame_lcnt++;
	if (p_acs->ras.cc.rx_frame_lcnt == 0)
		p_acs->ras.cc.rx_frame_mcnt++;

	p_acs->ras.cc.rx_byte_lcnt += len;
	if (p_acs->ras.cc.rx_byte_lcnt < len)
		p_acs->ras.cc.rx_byte_mcnt++;

	/* Get an mbuf (little or big) to copy the data into. */
	if (len <= MHLEN - p_acs->dds.rdto)
		p_frame = m_gethdr(M_DONTWAIT, MT_DATA);
	else
		p_frame = m_getclust(M_DONTWAIT, MT_DATA);
	if (p_frame == NULL)
	{
		fddi_logerr(p_acs, ERRID_FDDI_NOMBUFS,
			__LINE__, __FILE__);
		++p_acs->ras.ds.rcv_no_mbuf;
		FDDI_REARM();
		FDDI_TRACE("Rrf5", 0, 0, 0);
		return;
	}

	/* 
	 * if the data wont fit in one cluster (leaving room for rdto) then 
	 * get another buffer; remembering the amount of data to put in the 
	 * first buffer in len1.
	 */
	if ( len <= (len1 =(CLBYTES - p_acs->dds.rdto)) )
	{
		p_frame->m_data += p_acs->dds.rdto;

		p_frame->m_len = len;
		p_frame->m_next = NULL;

		bcopy(p_rcvd->p_buf,MTOD(p_frame, char *),len);
	}
	else
	{
		/* we will need another mbuf */
		p_frame->m_next = m_getclust(M_DONTWAIT, MT_DATA);

		if (p_frame->m_next == NULL)
		{
			fddi_logerr(p_acs, ERRID_FDDI_NOMBUFS,
				__LINE__, __FILE__);
			m_freem(p_frame);
			++p_acs->ras.ds.rcv_no_mbuf;
			FDDI_TRACE("Rrf6", 0, 0, 0);
			FDDI_REARM();
			return;
		}

		/* 
		 * copy the first chunk of the frame
		 * into the p_frame mbuf.
		 */
		p_frame->m_data += p_acs->dds.rdto;
		p_frame->m_len = len1;

		bcopy(p_rcvd->p_buf,MTOD(p_frame, char *),len1);

		/*
		 * copy the 2nd part of the frame into
		 * the p_frame->m_next mbuf
		 */
		p_frame->m_next->m_len = len-len1;

		p_frame->m_next->m_next = NULL;

		bcopy(p_rcvd->p_buf + len1,
			 MTOD(p_frame->m_next, char *), (len-len1));
	}

	/* this is in for tcpip performance; saves them the time of walking 
	 * the chain
	 */
	p_frame->m_pkthdr.len = len;
	p_frame->m_flags |= M_PKTHDR;

	/* fill out read extension */
	read_ext.status = CIO_OK;

	/* call rearm before the frame is passed back to the user */
	FDDI_REARM();

	FDDI_DBTRACE("Rrf6", p_frame, 0, 0);
	/* pass completed frame to owner */
	if (p_open->devflag & DKERNEL)
	{ 
		/* call kernel routine */
		FDDI_DBTRACE("Rrf7", p_open->open_id, &read_ext, p_frame);
		(*(p_open->rcv_fn)) (p_open->open_id, &read_ext, p_frame);
		FDDI_DBTRACE("Rrf8", p_open->open_id, &read_ext, p_frame);
	}
	else	
	{
		/* 
		 * queue rcv frame for user 
		 */
		rcv_enque (p_open, p_frame);

		/* Check for processes waiting on this event */

		if (p_open->rcv_event != EVENT_NULL)
		{
			/* sleeping process */
			e_wakeup(&p_open->rcv_event);
		}
		if (p_open->selectreq & POLLIN)
		{
			/* polled process */
			selnotify (p_open->devno, p_open->chan, POLLIN);
			p_open->selectreq = 0;
		}
	}
	FDDI_DBTRACE("RrfE", p_frame, 0, 0);
	return ;
}


/*
 * NAME: rcv_handler
 *
 * FUNCTION: interrupt handler for rcv complete
 *
 * EXECUTION ENVIRONMENT: interrupt thread
 *
 * NOTES: 
 *
 *	Called by: the SLIH - fddi_slih ()
 *	Calls to:
 *		rcv_frame ()
 *
 *	We have a receive complete interrupt. 
 *	Checks have been made for serious errors: micro channel 
 *	errors, ring status changes, etc.
 *
 * RECOVERY OPERATION:
 *
 *	The errors detected in this routine are LIMBO entry errors.
 *	This routine detects the following errors: 
 *
 *		Buffer Too Small (BTS)
 *		Incomplete frames:
 *			1. sof but no eof
 *			2. eof but no sof
 *			3. no sof and no eof
 *
 *	We will need to rearm the descriptors as quickly as possible
 *	so that the adapter will always have a place to put incoming
 *	frames. If the adapter runs out of room to put this traffic
 *	then the incoming frames get dropped.  This is handled in rcv_frame
 *	
 * DATA STRUCTURES:
 *
 *	Rcv host descriptors are one to one with the adapter
 *	descriptors.
 *
 *	All rcv host descriptors are armed with 4500 (rounded up to the cache 
 * 	boundaries) of malloc'd memory.  This keeps the frame to one descriptor
 * 	increasing the efficiency of the list for the adapter.
 *
 *	The rcv host descriptors make up a circular queue and are
 *	managed with one index:
 *
 *		rcvd  - points to the descriptor that will
 *			     have the next valid receive.
 * RETURNS: 
 *
 *	0 - successful
 * !!!	1 - LIMBO entry condition encountered
 */
void
rcv_handler (
	fddi_acs_t	*p_acs,
	int		bus)
{
	uint		ctl;
	short		len;
	ushort		stat;
	fddi_rcv_t	*p_rcvd;

	FDDI_DBTRACE("RrhB", p_acs, p_acs->ctl.devno, p_acs->rcv.rcvd);

	/* performance trace hooks */
	/*
	 * !!!TRCHKL0T(HKWD_LWR | hkwd_ddrcv_in);
	 *TRCHKL0T(HKWD_LWR2 | hkwd_ddrcv_in);
	 */

	while (TRUE)
	{
		/* get ptr to next rcv desc to be processed */
		p_rcvd = &p_acs->rcv.desc[p_acs->rcv.rcvd];

		/* 
		 * get the ctl and stat fields of the descriptor with 
		 * one get long, the order of the shorts will be 
		 * reversed from the swap long.
		 */
		PIO_GETLRX(bus+p_rcvd->offset+
			offsetof(fddi_adap_t, ctl), &ctl);

		/* get the individual fields out of the int */
		stat = ctl >> 16;
		ctl = ctl & 0xFFFF;

		/*
		 * if the desc has the BDV bit set OR
		 * the adapter has not set SV (status valid)
		 * there is nothing for us to process
		 */
		if ( (ctl & FDDI_RCV_CTL_BDV) ||
			(!(stat & FDDI_RCV_STAT_SV)) )
			break;
		
		/*
		 * !!! the following statement will need to be changed
		 * to account for the LF bit being set when a SMT frame
		 * is sent by the TX side. 
		 */
		if ((stat & FDDI_RCV_STAT_BTS) || (stat & FDDI_RCV_STAT_FCS))
		{
			/*
			 * if any other bits are set in the rcv desc.
		 	 * we have a Limbo entry condition.
		 	 */
			fddi_logerr(p_acs, ERRID_FDDI_RCV,
					__LINE__, __FILE__);
			FDDI_TRACE("Rrh1",p_acs->rcv.rcvd, stat, ctl);
			fddi_enter_limbo(p_acs,FDDI_RCV_ERROR, stat);
		}
		else 
		{
			PIO_GETSRX(bus + p_rcvd->offset +
				offsetof(fddi_adap_t,cnt), &len);

			/* Process the frame */
			rcv_frame (p_acs, p_rcvd, len, bus);
		}

		/* bump next rcv indicator */
		INCREMENT (p_acs->rcv.rcvd, 1, FDDI_MAX_RX_DESC);
	} /* end while (TRUE) */

	FDDI_DBTRACE("RrhE", p_acs, bus, p_acs->rcv.rcvd);
	/* ok */
	return;
}


/*
 * NAME: fddi_start_rcv
 *                                                                    
 * FUNCTION: start receives                      
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES: 
 *
 *	Called by: act_cmplt() 
 *
 *	This is one of last routine called in the activation 
 *	sequence. Interrupts are enabled and normal processing
 *	begins immediately. 
 *
 * 	Assumptions:
 *		tx and rcv interrupts masked off,
 *		state changed to FDDI_OPEN by the act_cmplt().
 *
 * RECOVERY OPERATION: 
 *
 *	The only way this routine can fail is if a PIO fails and
 *	in this case the error is not bubbled up but handled
 *	immediately as an unrecoverable error.
 *
 * DATA STRUCTURES: 
 *
 *	rcv descriptor array in the open structure
 *
 * RETURNS:  none 
 */

void
fddi_start_rcv (
	fddi_acs_t	*p_acs,
	int		bus)
{
	int	ioa;
	ushort	tmp;
	int	i;
	fddi_rcv_t	*p_rcvd;


	/* initialize each descriptor on the card */
	for (i=0; i<FDDI_MAX_RX_DESC; i++)
	{
		p_rcvd = &(p_acs->rcv.desc[i]);

		/* put the address */
		PIO_PUTLRX(bus + p_rcvd->offset, p_rcvd->p_d_addr);

		/* init the constant section of the descriptor */
		PIO_PUTSTRX(bus+p_rcvd->offset+offsetof(fddi_adap_t,cnt), 
				&p_acs->rcv.arm_val, 
				sizeof (p_acs->rcv.arm_val));
	}

	ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

	/* adjust the host mask register to accept read interrupts */
	PIO_GETSRX(ioa+FDDI_HMR_REG, &tmp);
	PIO_PUTSRX(ioa+FDDI_HMR_REG, tmp & ~(FDDI_HSR_RCI) & ~(FDDI_HSR_RIR));

	/* issue rcv cmd to the NS1 register */
	PIO_PUTSRX(ioa+FDDI_NS1_REG, FDDI_NS1_RCV);

	BUSIO_DET(ioa);
	return;
}


fddi_nullsap (
fddi_acs_t	*p_acs,
fddi_hdr_t	*in_hdr,
int    len)

{
fddi_hdr_t	*out_hdr;
fddi_test_data_t	*in_packet;
fddi_test_data_t	*out_packet;

int 		rc;
int		minor;
int		buf_len;	/* lenght of mbuf/cluster we have for data */
int		len_left;
int		hdr_len;
int		bytes_to_copy;
int		start_the_copy;
int		ri_length;
uchar           control;
uchar		iee_xid[3] = {0x08, 0x01, 0x00};

struct mbuf	*mp, *new_mp, *last_mp;
extern fddi_ctl_t fddi_ctl;


/*
 * The first thing to do here is to see if routing is turned on
*/

  FDDI_TRACE ("nulB", in_hdr, len, 0);
  if (in_hdr->src[0] & FDDI_RI_SA ) {
    FDDI_TRACE ("NSri", 0, 0, 0);
    ri_length= in_hdr->data[0] & ROUTE_LEN_MASK;
    /*
     * handle the packet with RI turned on
     */
  } else {
    /*
     * handle the packe when no RI present
     */
     ri_length=0;

    /*
     * the control must be an XID or TEST
     */
  }


    in_packet = &in_hdr->data[ri_length];
    control=in_packet->ctl& CONTROL_MASK;
    /*
     * if the control byte is not XID ro TEST just get out now
     */
    FDDI_TRACE ("NSin", in_packet, control, 0);
    if ((control != XID_FRAME ) && 
        (control != TEST_FRAME )) {
      return (0);
    }

    /*
     * lets get all the header infor xfered over
     */


    if (len <= MHLEN - p_acs->dds.rdto){
	mp = m_gethdr(M_DONTWAIT, MT_DATA);
        buf_len = MHLEN - p_acs->dds.rdto;
#ifdef DEBUG
FDDI_TRACE ("MBG1", mp, buf_len, 0);
#endif
    } else {
	mp = m_getclust(M_DONTWAIT, MT_DATA);
        buf_len = CLBYTES - p_acs->dds.rdto;
#ifdef DEBUG
FDDI_TRACE ("CLG1", mp, buf_len, 0);
#endif
    } 

    if (mp == NULL)
    {
	fddi_logerr(p_acs, ERRID_FDDI_NOMBUFS,
		__LINE__, __FILE__);
	++p_acs->ras.ds.rcv_no_mbuf;
	FDDI_TRACE("Rrf5", 0, 0, 0);
	return;
    }
    /*
     * for now lets fill in this mbfu, if we are going
     * to need more than one cluster we will take care of
     * it later in the code
     */

    /* zero out the mbuf */
    bzero (MTOD (mp, char *),  buf_len);
   
    /* what about FC or the reserved part */
    out_hdr = MTOD (mp, fddi_hdr_t *);
    out_packet = &out_hdr->data[ri_length];
    /* FDDI_TRACE ("NSop", out_hdr, out_packet, mp); */

    out_hdr->fc = XID_TEST_FC; 
    out_hdr->flag = 0;
    bcopy(&p_acs->ctl.vpd_addr[0], &out_hdr->src[0], FDDI_NADR_LENGTH);
    bcopy(&in_hdr->src[0], &out_hdr->dest[0], FDDI_NADR_LENGTH);

    /*
     * if there is routing information copy it over
     */
    if (ri_length != 0) {
      if ((in_hdr->data[0] & ROUTE_BCAST_MASK)==SINGLE_ROUTE_BCAST) {
      /*
       * need to make it an all route broadcast
       */
         out_hdr->data[0]=ALL_ROUTE_CTL1;
         out_hdr->data[1]=in_hdr->data[1];
      } else {
        out_hdr->data[0]=in_hdr->data[0] ;
        out_hdr->data[1]=in_hdr->data[1] ^ ROUTE_DIR_BIT;
      }
     if (ri_length-2 > 0) {  /* len-2 is route info less RI */
       bcopy (&in_hdr->data[2], &out_hdr->data[2], ri_length-2);
     }
  }


    /*
     * Now copy over the source and destination saps 
     */

    out_packet->lsap = in_packet->rsap | RESP_ON_BIT;
    out_packet->rsap = in_packet->lsap;
    out_packet->ctl = in_packet->ctl |  UFMT_PF_BIT;

    if (control == XID_FRAME ) {
      bcopy (&iee_xid, &out_packet->data[0], 3);
      mp->m_len = 22+ri_length;  /* 16 + rsap +dsap+control+3bytexid = 22 */
      FDDI_TRACE ("XID ", mp->m_len, out_packet, ri_length);
    } else {
      /*
       * control is test frame 
       */
      if ( len-19 > buf_len )
        bytes_to_copy = buf_len-19-ri_length;
      else
        bytes_to_copy = len-19-ri_length;
      
      FDDI_TRACE ("TEST", len, bytes_to_copy, ri_length);
      bcopy (&in_packet->data[0], &out_packet->data[0], bytes_to_copy);
      mp->m_data += p_acs->dds.rdto;
      mp->m_len = bytes_to_copy+19+ri_length;
      mp->m_next = NULL;

      len_left = len - bytes_to_copy-19-ri_length;
      last_mp = mp;
      start_the_copy = bytes_to_copy;

      while (len_left > 0 ) {
        FDDI_TRACE ("MORE", len_left, 0, 0);
	new_mp = m_getclust(M_DONTWAIT, MT_DATA);
        if (new_mp == NULL)
          {
	  fddi_logerr(p_acs, ERRID_FDDI_NOMBUFS,
		__LINE__, __FILE__);
	  ++p_acs->ras.ds.rcv_no_mbuf;
	  FDDI_TRACE("Rrf6", 0, 0, 0);
	  return;
        }
        last_mp->m_next = new_mp;
        if (len_left > CLBYTES)
          bytes_to_copy = CLBYTES;
        else
          bytes_to_copy = len_left;

        bcopy (&in_packet->data[start_the_copy], MTOD (new_mp, char *),
               bytes_to_copy);
        new_mp->m_len = bytes_to_copy;
        new_mp->m_next = NULL;
        len_left = len_left - bytes_to_copy;
        start_the_copy = start_the_copy + bytes_to_copy;
        last_mp = new_mp;
      } /* end of while */
        
    }
  /* 
   * now it is time to get this packet sent off
   * we cannot use the fastwrite routine since it
   * does some permission checking, so we need to
   * put this stuff on the transmit queue ourself
   */

  
  /*
   * The fastwrite routine needs the minor device
   * passed in, the following code is ugly, but
   * it works
   */
 
   for (minor=0; minor< FDDI_MAX_MINOR; minor++) {
     if ( p_acs == fddi_ctl.p_acs[minor]) {
       break;
     }
   }

  /*
   * now call the fastwrite routine
   */

   rc=fddi_fastwrite (mp, minor);
   if (rc != 0) {
     FDDI_TRACE ("FWng",rc, mp, 0);
   }


}


