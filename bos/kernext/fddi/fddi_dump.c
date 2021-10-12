static char sccsid[] = "@(#)60	1.5  src/bos/kernext/fddi/fddi_dump.c, sysxfddi, bos411, 9428A410j 5/2/94 13:32:32";
/*
 *   COMPONENT_NAME: sysxfddi
 *
 *   FUNCTIONS: fddi_dump
 *		fddi_dump_output
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "fddiproto.h"
#include <sys/time.h>
#include <sys/dma.h>
#include <sys/dump.h>
#include <sys/mbuf.h>
#include <sys/sleep.h>
#include <sys/malloc.h>

#define FDDI_DUMP_TRACE(a1,a2,a3,a4)					\
{									\
	fddi_dump_trace(a1,a2,a3,a4);					\
}
/* externs */
extern	fddi_tbl_t	fddi_tbl;

uint	awscnt,txcnt, rxcnt_err, rxcnt_ok;

/*
 * NAME:     fddi_dump
 *
 * FUNCTION: provides access to the dump functions of the driver
 *
 * EXECUTION ENVIRONMENT:  Kernel process thread
 *
 *	This function can be called with interrupts disabled to
 *	the same level as our operation level, a less favored
 *	level, or wide open. We need to provide for the most secure
 *	thread by disabling interrupts of other threads to the most 
 *	protected case.
 *
 * NOTES: 
 *
 * RETURNS:  
 *
 * 	0	- Success
 *	ENOMEM 	- No mbufs were available
 *	ETIMEDOUT - Passed wait time with nothing to receive
 *	ENETDOWN - Indicates Network is in unrecoverable state
 */

int fddi_dump( 
            fddi_acs_t *p_acs,
            int cmd,
            caddr_t arg)
{
	uint			ctl;
	short			stat;
	short			len, offset;
	int 			i,j,x;
	struct mbuf 		*p_tmp;
	struct dmp_query 	*p_query;
	int 			bus, ioa;
	fddi_rx_desc_t		*p_rx;
	cfddi_hdr_t		*p_hdr;
	fddi_adap_t		tmp_adap;
	struct dump_read	*p_read;
	struct timestruc_t  	current_time,   /* Keeps the current time */
                        	timeout_time,   /* Time out value         */
                        	temp_time;
	uchar			netid;
	ushort			hsr;
	ushort			tmp_smt_control;
	extern			void fddi_dump_trace();
	extern			int fddi_dump_output();



	FDDI_DUMP_TRACE("dfdB",p_acs, cmd, arg);

	switch (cmd)
	{
	case DUMPINIT:
		for (i=0; i<FDDI_MAX_TX_DESC; i++)
		{
			p_acs->tx.desc[i].p_dump_buf = 
				xmalloc((p_acs->rx.l_adj_buf), 
					DMA_L2PSIZE, pinned_heap);
			if (p_acs->tx.desc[i].p_dump_buf == NULL)
			{
				for (j=0; j<i; j++)
					xmfree(p_acs->tx.desc[j].p_dump_buf,
						pinned_heap);
				FDDI_DUMP_TRACE("dfd1",p_acs, cmd, ENOMEM);
				return (ENOMEM);
			}
		}
	
		break;
	
	case DUMPQUERY:
		p_query = (struct dmp_query *)arg;
		p_query->min_tsize = p_acs->ndd.ndd_mintu;
		p_query->max_tsize = CLBYTES;
		break;	

	case DUMPSTART:
		if (p_acs->dev.state == LIMBO_STATE || 
			p_acs->dev.state == LIMBO_RECOVERY_STATE || 
			p_acs->dev.state == DEAD_STATE)
		{
			FDDI_DUMP_TRACE("dfd2",p_acs, cmd, p_acs->dev.state);
			return (ENETDOWN);
		}

		awscnt = txcnt = rxcnt_err = rxcnt_ok = 0;

		ioa = BUSIO_ATT (p_acs->dds.bus_id, p_acs->dds.bus_io_addr);
		PIO_PUTSRX (ioa + FDDI_HMR_REG, 0xffff);
		BUSIO_DET (ioa);

		p_acs->ndd.ndd_output = (int(*)())fddi_dump_output;

		p_acs->rx.arm_val.ctl = SWAPSHORT(FDDI_RX_CTL_BDV);
		p_acs->dev.smt_control = 0;
		
		bus = BUSMEM_ATT (p_acs->dds.bus_id, 
			(ulong)p_acs->dds.bus_mem_addr);
		ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

		tmp_smt_control =SWAPSHORT(p_acs->dev.smt_control);

		BUS_GETSRX(ioa+FDDI_HSR_ADDR, &hsr);

		PIO_PUTSTRX(bus + FDDI_CPB_SHARED_RAM + 4, 
			&tmp_smt_control, 2);

		PIO_PUTSRX(ioa + FDDI_HCR_REG, FDDI_HCR_WR_SMT_CTL);

    		/*
     		 *  Set up time information
     		 */
    		MS2TIME(5, temp_time);
    		curtime(&current_time);
    		ntimeradd(current_time, temp_time, timeout_time);

		for (i=0; i<10; i++)
		{
			while (TRUE)
			{
            			if (ntimercmp(current_time, timeout_time, >))
				{
                			break;
            			}
            			curtime(&current_time);
			}

			BUS_GETSRX(ioa+FDDI_HSR_ADDR, &hsr);
	
			if (hsr & FDDI_HSR_ERRORS)
			{
				BUSIO_DET (ioa);
				BUSMEM_DET(bus);
				FDDI_DUMP_TRACE("dfd3",p_acs, hsr, 0);
				return (ENETDOWN);
			}

			if (hsr & FDDI_HSR_CCI)
				break;
		}

		if (i == 10)
		{
			BUSIO_DET (ioa);
			BUSMEM_DET(bus);
			FDDI_DUMP_TRACE("dfd4",p_acs, hsr, i);
			return (ENETDOWN);
		}

		PIO_PUTSRX(ioa + FDDI_HCR_REG, FDDI_HCR_ABORT_TX);
    		/*
     		 *  Set up time information
     		 */
    		MS2TIME(5, temp_time);
    		curtime(&current_time);
    		ntimeradd(current_time, temp_time, timeout_time);

		for (i=0; i<10; i++)
		{
			while (TRUE)
			{
            			if (ntimercmp(current_time, timeout_time, >))
				{
                			break;
            			}
            			curtime(&current_time);
			}

			BUS_GETSRX(ioa+FDDI_HSR_ADDR, &hsr);

			if (hsr & FDDI_HSR_ERRORS)
			{
				BUSIO_DET (ioa);
				BUSMEM_DET(bus);
				FDDI_DUMP_TRACE("dfd5",p_acs, hsr, 0);
				return (ENETDOWN);
			}

			if (hsr & FDDI_HSR_CCI)
				break;

		}

		if (i == 10)
		{
			BUSIO_DET (ioa);
			BUSMEM_DET(bus);
			FDDI_DUMP_TRACE("dfd6",p_acs, hsr, i);
			return (ENETDOWN);
		}

		p_acs->tx.hdw_nxt_req = 0;

		bzero(&tmp_adap, sizeof(fddi_adap_t));
		for (i=0; i<FDDI_MAX_TX_DESC; i++)
			PIO_PUTSTRX (bus + p_acs->tx.desc[i].offset, 
				&tmp_adap, sizeof (fddi_adap_t));
		
		for (i=0; i<FDDI_MAX_TX_DESC; i++)
		{
			d_master(	p_acs->dev.dma_channel,
					DMA_WRITE_ONLY,
					p_acs->tx.desc[i].p_dump_buf,
					p_acs->rx.l_adj_buf,
					&p_acs->rx.xmd,
					p_acs->tx.desc[i].p_d_addr);
		}

		PIO_PUTSRX(ioa + FDDI_HCR_REG, FDDI_HCR_ABORT_RX);

    		/*
     		 *  Set up time information
     		 */
    		MS2TIME(5, temp_time);
    		curtime(&current_time);
    		ntimeradd(current_time, temp_time, timeout_time);

		for (i=0; i<10; i++)
		{
			while (TRUE)
			{
            			if (ntimercmp(current_time, timeout_time, >))
				{
                			break;
            			}
            			curtime(&current_time);
			}

			BUS_GETSRX(ioa+FDDI_HSR_ADDR, &hsr);

			if (hsr & FDDI_HSR_ERRORS)
			{
				BUSIO_DET (ioa);
				BUSMEM_DET(bus);
				FDDI_DUMP_TRACE("dfd7",p_acs, hsr, 0);
				return (ENETDOWN);
			}

			if (hsr & FDDI_HSR_CCI)
				break;

		}

		if (i == 10)
		{
			BUSIO_DET (ioa);
			BUSMEM_DET(bus);
			FDDI_DUMP_TRACE("dfd8",p_acs, hsr, i);
			return (ENETDOWN);
		}

		BUSIO_DET (ioa);
		BUSMEM_DET(bus);

		start_rx(p_acs);
		break;

	case DUMPEND:
		p_acs->dev.state = DEAD_STATE;
		break;

	case DUMPTERM:
		for (i=0; i<FDDI_MAX_TX_DESC; i++)
			xmfree(p_acs->tx.desc[i].p_dump_buf, pinned_heap);
		break;

	case DUMPREAD:

		if (p_acs->dev.state == DEAD_STATE)
		{
			FDDI_DUMP_TRACE("dfd9",p_acs, cmd, p_acs->dev.state);
			return(ENETDOWN);
		}

		bus = BUSMEM_ATT (p_acs->dds.bus_id, 
			(ulong)p_acs->dds.bus_mem_addr);

		ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

		p_read = (struct dump_read *)arg;

    		/*
     		 *  Set up time information
     		 */
    		MS2TIME((p_read->wait_time*10), temp_time);
    		curtime(&current_time);
    		ntimeradd(current_time, temp_time, timeout_time);

		while (TRUE)
		{
			p_rx = &p_acs->rx.desc[p_acs->rx.nxt_rx];
			PIO_GETLRX(bus+p_rx->offset+ offsetof(fddi_adap_t, ctl),
				&ctl);
			if (p_acs->dev.pio_rc)
			{
				p_acs->dev.state = DEAD_STATE;
				disable_card(p_acs);
				BUSMEM_DET(bus);
				BUSIO_DET (ioa);
				FDDI_DUMP_TRACE("dfda",p_acs,cmd, 
					p_acs->dev.pio_rc);
				return (ENETDOWN);
			}
                	
			/* get the individual fields out of the int */
                	stat = ctl >> 16;
                	ctl = ctl & 0xFFFF;

			/*
                 	 * if the desc has the BDV bit set OR
                 	 * the adapter has not set SV (status valid)
                 	 * there is nothing to receive yet
                 	 */
			if (stat & FDDI_RX_STAT_SV)
			{
				PIO_GETSRX(bus+p_rx->offset+
					offsetof(fddi_adap_t,cnt), &len);

                		if (p_acs->dev.pio_rc)
				{
					p_acs->dev.state = DEAD_STATE;
					disable_card(p_acs);
					BUSMEM_DET(bus);
					BUSIO_DET (ioa);
					FDDI_DUMP_TRACE("dfdb",p_acs,
						cmd, p_acs->dev.pio_rc);
					return(ENETDOWN);
				}

				d_bflush (p_acs->dev.dma_channel, 
					p_acs->dds.bus_id,
                        		p_rx->p_d_addr);

				p_hdr = (cfddi_hdr_t *)p_rx->p_buf;
				
				if (p_hdr->src[0] & FDDI_RI_SA)
				{
					netid=p_hdr->data[p_hdr->data[0]&0x1e];
					offset = 24 + (p_hdr->data[0]&0x1e);
				}
				else 
				{
					netid = p_hdr->data[0];
					offset = 24;
				}
	
				if ((len <= 500) &&
					((p_hdr->fc & FDDI_FC_MSK) == 
						FDDI_FC_LLC) &&
					(netid == 0xaa) &&
					!(stat & FDDI_RX_STAT_BTS) &&
					!(stat & FDDI_RX_STAT_FCS))
				{
					rxcnt_ok++;
					
					p_read->dump_bufread->m_len =len-offset;
					p_read->dump_bufread->m_next = NULL;
	
					p_read->dump_bufread->m_data += offset;

					p_read->dump_bufread->m_pkthdr.len = 
						len-offset;

					p_read->dump_bufread->m_flags |= 
							M_PKTHDR;

					cache_inval(p_rx->p_buf, len);

					bcopy(p_rx->p_buf+offset,
						MTOD(p_read->dump_bufread,
							caddr_t),
						len-offset);

					FDDI_REARM();
                        		PIO_PUTSRX(ioa+FDDI_NS1_REG, 
						FDDI_NS1_RX);

					break;
				}

				FDDI_REARM();
                        	PIO_PUTSRX(ioa+FDDI_NS1_REG, FDDI_NS1_RX);
			}

            		/*
             		 *  Check to see if timeout has been reached.
             	 	 */
            		if (ntimercmp(current_time, timeout_time, >)) 
			{
				rxcnt_err++;
				FDDI_DUMP_TRACE("dfdc",p_acs->rx.nxt_rx, 
					p_rx->offset, stat);

				BUSMEM_DET(bus);
				BUSIO_DET (ioa);
				return(ETIMEDOUT);
			}
                	curtime(&current_time);

		}

                if (FDDI_IS_BCAST(p_hdr->dest))
                {
                	p_read->dump_bufread->m_flags |= M_BCAST;
		}
                else if (p_hdr->dest[0] & FDDI_GRP_ADDR)
                {
                	p_read->dump_bufread->m_flags |= M_MCAST;
                }

		BUSIO_DET (ioa);
		BUSMEM_DET(bus);
		break;
	};

	FDDI_DUMP_TRACE("dfdE",p_acs, cmd, 0);
	return (0);
}


/*
 * NAME:     fddi_dump_output
 *
 * FUNCTION: provides a transmit function for the driver.
 *
 * EXECUTION ENVIRONMENT:  Kernel process thread
 *
 *	This function can be called with interrupts disabled to
 *	the same level as our operation level, a less favored
 *	level, or wide open. We need to provide for the most secure
 *	thread by disabling interrupts of other threads to the most 
 *	protected case.
 *
 * NOTES: 
 *
 * RETURNS:  
 *
 * 	0	- Success
 *	ENETDOWN - Indicates Network is in unrecoverable state
 */

int fddi_dump_output( 
            fddi_acs_t *p_acs,
            struct mbuf *p_mbuf)
{
	struct timestruc_t 	current_time,   /* Keeps the current time */
    				timeout_time,   /* Time out value         */
    				temp_time;
	uint			ctl;
	short			stat;
	short			len;
	fddi_tx_desc_t  	*p_nxt;         /* ptr for end of frame */
	int			bus,ioa;

	
	FDDI_DUMP_TRACE("dfoB",p_acs, p_mbuf, p_mbuf->m_len);

	/*
         *  Set up time information
         */
        MS2TIME(FDDI_TX_WDT_RESTART, temp_time);
        curtime(&current_time);
        ntimeradd(current_time, temp_time, timeout_time);

	p_nxt = &(p_acs->tx.desc[p_acs->tx.hdw_nxt_req]);
	bus = BUSMEM_ATT (p_acs->dds.bus_id, (ulong)p_acs->dds.bus_mem_addr);

	while (txcnt > 0)
	{
		/*
                 * Get the adapter status on the next descriptor
                 */
		PIO_GETLRX(bus+p_nxt->offset+ offsetof(fddi_adap_t, ctl),&ctl);

		/* get the individual fields out of the int */
		stat = ctl >> 16;
		ctl = ctl & 0xFFFF;

		if ((stat & FDDI_TX_STAT_BTI) ||
			(p_acs->dev.pio_rc))
		{
			p_acs->dev.state = DEAD_STATE;
			BUSMEM_DET(bus);
			FDDI_DUMP_TRACE("dfo1",p_acs,stat, p_acs->dev.pio_rc);
			return (ENETDOWN);
		}

		if (stat & FDDI_TX_STAT_SV)
		{
			d_complete(	p_acs->dev.dma_channel,
					DMA_READ,
					p_nxt->p_dump_buf,
					p_nxt->dump_len,
					&p_acs->rx.xmd,
					p_nxt->p_d_addr);

			INCREMENT (p_acs->tx.hdw_nxt_req, 1, FDDI_MAX_TX_DESC);
			p_nxt = &(p_acs->tx.desc[p_acs->tx.hdw_nxt_req]);
			break;
		}

		/*
                 *  Check to see if timeout has been reached.
                 */
                if (ntimercmp(current_time, timeout_time, >)) 
		{
                    	BUSMEM_DET(bus);
			FDDI_DUMP_TRACE("dfo2",p_acs,0,0);
                    	return (ENETDOWN);
                }
                curtime(&current_time);
	}

	txcnt++;
	bcopy(MTOD(p_mbuf,char *), p_nxt->p_dump_buf, p_mbuf->m_len);
	p_nxt->dump_len = p_mbuf->m_len;

	d_cflush ( p_acs->dev.dma_channel,
			p_nxt->p_dump_buf,
			p_mbuf->m_len,
			p_nxt->p_d_addr);

        *(uint *) &(p_nxt->adap.addr_hi) = SWAPLONG(p_nxt->p_d_addr);
	p_nxt->adap.cnt = SWAPSHORT(p_mbuf->m_len);
	p_nxt->adap.ctl = SWAPSHORT(FDDI_TX_CTL_BDV | FDDI_TX_CTL_EOF | 
					FDDI_TX_CTL_SOF);
	p_nxt->adap.stat = 0;


	/* PIO the tx descriptor to the adapter. */
	PIO_PUTSTRX (bus + p_nxt->offset, &p_nxt->adap, sizeof (p_nxt->adap));


        ioa = (caddr_t) BUSIO_ATT (p_acs->dds.bus_id, 
		(caddr_t) p_acs->dds.bus_io_addr);
        PIO_PUTSRX (ioa + FDDI_NS1_REG, FDDI_NS1_XMA);
        BUSIO_DET (ioa);
	BUSMEM_DET(bus);
	FDDI_DUMP_TRACE("dfoE",p_acs,p_mbuf, p_acs->tx.hdw_nxt_req);
	return(0);
}


/*
 * NAME: fddi_dump_trace
 *                                                                    
 * FUNCTION: 
 *	This routine puts a trace entry into the internal device
 *	driver trace table.
 *                                                                    
 * EXECUTION ENVIRONMENT: process or interrupt environment
 *                                                                   
 * NOTES:  See the design for the trace format
 *
 * RETURNS:  none
 */  

void
fddi_dump_trace (	register uchar	str[],	/* trace data Footprint */
		register ulong	arg2,	/* trace data */
		register ulong	arg3,	/* trace data */
		register ulong	arg4)	/* trace data */

{
	register int	i;
	register char	*p_str;
	register int	ipri;

	/*
	 * get the current trace point index 
	 */
	i= fddi_tbl.trace.next;

	p_str = &str[0];

	/*
	 * copy the trace point data into the global trace table.
	 */
	fddi_tbl.trace.table[i] = *(ulong *)p_str;
	++i;
	fddi_tbl.trace.table[i] = arg2;
	++i;
	fddi_tbl.trace.table[i] = arg3;
	++i;
	fddi_tbl.trace.table[i] = arg4;
	++i;


	if ( i < FDDI_TRACE_SIZE )
	{
		fddi_tbl.trace.table[i] = 0x21212121;	/* end delimeter */
		fddi_tbl.trace.next = i;
	}
	else
	{
		fddi_tbl.trace.table[0] = 0x21212121;	/* end delimeter */
		fddi_tbl.trace.next = 0;
	}


	return;
} /* end fddi_dump_trace */

