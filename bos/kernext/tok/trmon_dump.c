static char sccsid[] = "@(#)20	1.10  src/bos/kernext/tok/trmon_dump.c, sysxtok, bos411, 9428A410j 6/29/94 15:09:38";
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: clean_up_read
 *		dump_setup
 *		ms2time
 *		tok_dump_read
 *		tok_dump_wrt
 *		tokdump
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

#include <sys/cblock.h>
#include <sys/poll.h>
#include <sys/systemcfg.h>
#include <sys/user.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/if_802_5.h>
#include <netinet/ip.h>
#include <netinet/in.h>

#include "tokpro.h"

extern dd_ctrl_t        mon_dd_ctrl;

#define TOK_RS_IIR	0x00400084 	/* Power Classic Int ID reg addr */
#define TOK_PC_IIR	0x00010188	/* Power PC Interrupt ID reg addr */
#define FCF_MASK	0xC0		/* mask for frame type field */
#define MAC_FRAME	0x00		/* Medium Access Control Frame type */
/************************************************************************/
/*                                                                      */
/* NAME:        tokdump                                                 */
/*                                                                      */
/* FUNCTION:    Adapter Driver Dump Routine                             */
/*                                                                      */
/*      This routine handles requests for dumping data to a previously  */
/*      opened device.                                                  */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine is called when there is certain to be limited      */
/*      functionality available in the system.  However, system         */
/*      dma kernel services are available.  The passed data is already  */
/*      pinned by the caller.  There are no interrupt or timer kernel   */
/*      services available.  This routine should run at INTMAX level.   */
/*                                                                      */
/* NOTES:                                                               */
/*      Any lack of resources, or error in attempting to run any        */
/*      command, is considered fatal in the context of the dump.        */
/*      It is assumed that normal operation will not continue once      */
/*      this routine has been executed.					*/
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      p_dds     - adapter unique data structure (one per adapter)     */
/*      uio       - user i/o area struct                                */
/*      dmp_query - kernel dump query structure                         */
/*                                                                      */
/* INPUTS:                                                              */
/*	p_ndd	- pointer to ndd struct					*/
/*      cmd     - parameter specifying the dump operation               */
/*      arg     - pointer to command specific structure                 */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      A zero will be returned on successful completion, otherwise,    */
/*      one of the errno values listed below will be given.             */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      0       - successful completion                                 */
/*      EINVAL  - invalid request                                       */
/*      ENETDOWN - the adapter cannot connect to the network            */
/*      ETIMEDOUT - the DUMPREAD option timed-out                       */
/*                                                                      */
/************************************************************************/
int
tokdump(
	ndd_t	*p_ndd,
	int	cmd,
	caddr_t	arg)
{
    register dds_t         *p_dds;        /* Pointer to the dds     */
    int                    ret_code=0;    /* Exit code              */
    struct dmp_query       dump_ptr;      /* Dump info. structure   */
    int                    tok_dump_wrt();  /* pointer to tokdumpwrt()*/
    
    p_dds = (dds_t *) (((unsigned long) p_ndd) - offsetof(dds_t, ndd));
    TRACE_DBG(MON_OTHER, "dumB", p_dds, cmd, arg);

    /*
     *  Process dump command
     */
    WRK.dump_pri = i_disable(INTMAX);
    switch (cmd) {
	
    case DUMPQUERY:
	dump_ptr.min_tsize = CTOK_MIN_PACKET;
	dump_ptr.max_tsize = 4096;
	TRACE_DBG(MON_OTHER, "dumq", dump_ptr.min_tsize, dump_ptr.max_tsize,
		0);
	bcopy(&dump_ptr,arg,sizeof(struct dmp_query));
	break;
	
    case DUMPSTART:
	TRACE_DBG(MON_OTHER, "dums", WRK.adap_state, 0, 0);
	if (WRK.adap_state != OPEN_STATE) {
	    TRACE_DBG(MON_OTHER, "dumE", ENETDOWN, 0, 0);
	    return(ENETDOWN);
	}
	/*
	 *  Setup the dump receive and transmit
	 *  queues.
	 */
	clean_tx(p_dds);
	if (dump_setup(p_dds)) {
	    TRACE_DBG(MON_OTHER, "dumE", ENETDOWN, 0, 0);
	    return(ENETDOWN);
	}
	NDD.ndd_output = tok_dump_wrt;
	WRK.dump_first_wrt = TRUE;
	break;
	
    case DUMPREAD:
	/*
	 *  call internal routine to execute the command
	 */
	ret_code = tok_dump_read(p_dds, arg);
	if (ret_code == -1) {
	    TRACE_DBG(MON_OTHER, "dumE", ENETDOWN, 0, 0);
	    return(ENETDOWN);
	}
	break;
	
    case DUMPEND:
    case DUMPINIT:
    case DUMPTERM:
	break;
	
    default:
	ret_code = EINVAL;
	break;
	
    }	/* end switch (cmd) */
    
    TRACE_DBG(MON_OTHER, "dumE", ret_code, 0, 0);
    i_enable(WRK.dump_pri);
    return (ret_code);
    
}  /* End of tokdump */

/*--------------------- T O K _ D U M P _ R E A D ----------------------*/
/*                                                                      */
/*  NAME: tokdumpread                                                   */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Processes a dump read request                                   */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Called by the dump entry point.                                 */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS: a -1 if there was a failure and 0 upon success.            */
/*                                                                      */
/*----------------------------------------------------------------------*/
tok_dump_read(
    register dds_t      *p_dds,         /* Pointer to dds               */
    caddr_t		arg)		/* Pointer to read paramters	*/
{
    struct dump_read    dump_parms;	/* mbuf pointer & wait time	*/
    struct mbuf		*p_mbuf;	/* mbuf pointer from dump_parms */
    ushort              reason;         /* Interrupt reason             */
    uchar               dsap;           /* DSAP				*/
    uchar               *p_dsap;        /* DSAP address			*/
    int                 sof,            /* Start of frame indicator     */
			eof,            /* End of frame indicator       */
			bad_frame,      /* Invalid dump frame indicator */
			done,           /* Got a valid frame            */
			index,          /* Index into recv_list array   */
			size,           /* Size of frame                */
			copied,         /* Whether it's first copy      */
			count,          /* The number of bytes to copy  */
			rc,             /* General return code          */
			macsize,        /* size of mac header		*/
			bus;            /* Bus I/O address              */
    char                *mbuf_index;    /* pointer to dump data area    */
    char                *data_area;     /* Data area in recv_list       */
    struct timestruc_t  current_time,   /* Keeps the current time       */
    			timeout_time,   /* Time out value               */
    			temp_time;
    struct ie5_mac_hdr  *p_mac;         /* Pointer to MAC header        */
    recv_list_t         recv_list;      /* Temporary recv_list          */
    ulong		irr;		/* IOCC Int Request Reg pointer */
    ulong		irr_value;	/* IOCC Interrupt Request Reg   */
    ulong		mask;		/* bus_intr_lvl mask		*/
    
    TRACE_DBG(MON_OTHER, "drdB", p_dds, 0, 0);

    bcopy (arg, &dump_parms, sizeof(dump_parms));
    p_mbuf = dump_parms.dump_bufread;
    p_mbuf->m_next = NULL;
    p_mbuf->m_flags |= M_PKTHDR;

    /*
     *  Set up time information
     */
    ms2time(dump_parms.wait_time, &temp_time);
    curtime(&current_time);
    ntimeradd(current_time, temp_time, timeout_time);
    
    /*
     *  If have not started to process a receive interrupt then poll 
     *  for a receive intterupt
     */
    if (!WRK.dump_read_started) {
	/*
	 *  Get the bus attachment.
	 */
	bus = BUSIO_ATT(DDI.bus_id, DDI.io_port);
	
	/*
	 *  Poll the adapter until timeout or interrupt is detected
	 *
	 *  NOTE: previous levels of the adapter card do not set the
	 *  status register correctly in polling mode.  To get around
	 *  that:
	 *  - do NOT disable interrupts from the card
	 *  - read the IOCC interrupt request register and look for an
	 *    interrupt pending on the interrupt level which the TR
	 *    card is running on
	 *  - if an interrupt is pending, then read the status register
	 *    and process the interrupt
	 */
	
	mask = 0x80000000 >> DDI.intr_level;
	for (;;) {
	    if (__power_rs())   /* IRR is in different places */
	    {
		irr = (uint)IOCC_ATT(DDI.bus_id, TOK_RS_IIR); 
	    }
	    else
	    {
		irr = (uint)IOCC_ATT(DDI.bus_id, TOK_PC_IIR); 
	    }
	    TOK_GETLX(irr, &irr_value);
	    IOCC_DET(irr);
	    
	    if (irr_value & mask)
	    {
		/* Read in the interrupt reason from adapter */
		TOK_GETSRX( bus + STATUS_REG, &reason);
			/*
			BUSIO_DET(bus);
			WRK.dump_read_started = FALSE;
			TRACE_DBG(MON_OTHER, "drdE", -1, 1, 0);
			return(-1);
		    */
		TRACE_DBG(MON_OTHER, "drdr", reason, 0, 0);
	    } else {
		reason = 0;	
	    }
	    /*
	     *  Check to see if the adapter has set the system 
	     *  interrupt bit 
	     */
	    if ( reason & TOK_INT_SYSTEM) {
		/*
		 *  Validate that it is a receive interrupt
		 */
		if ((reason & TOK_IC) == RECEIVE_STATUS) {
		    /*
		     *  Copy in the receive status work 
		     *  element from DMA.
		     */
		    if (WRK.do_dkmove) {
			d_kmove (&(WRK.recv_iwe.cmd),
				  WRK.p_d_ssb,
				  sizeof(t_ssb),
				  WRK.dma_chnl_id,
				  DDI.bus_id,
				  DMA_READ);
		    } else {
			bcopy (WRK.p_ssb,
			       &(WRK.recv_iwe.cmd),
			       sizeof(t_ssb));
		    }
		    
		    TOK_PUTSRX(bus + COMMAND_REG, ACK_INT);
		    BUSIO_DET(bus);
		    WRK.dump_read_started = TRUE;
		    break;
		}
		/*
		 *  Read in the status information for a 
		 *  transmit interrupt and save it.
		 */
		if ((reason & TOK_IC) == TRANSMIT_STATUS) {
		    /*
		     *  Copy in the transmit status work 
		     *  element from DMA.
		     */
		    if (WRK.do_dkmove) {
			d_kmove (&(WRK.tx_iwe.cmd),
				  WRK.p_d_ssb,
				  sizeof(t_ssb),
				  WRK.dma_chnl_id,
				  DDI.bus_id,
				  DMA_READ);
		    } else {
			bcopy (WRK.p_ssb,
			       &(WRK.tx_iwe.cmd),
			       sizeof(t_ssb));
		    }
		    
		    TOK_PUTSRX(bus + COMMAND_REG, ACK_INT);
		    continue;
		}
		/*
		 *  Some other interrupt like CK or some other 
		 *  fatal error. So just acknowledge the 
		 *  interrupt and continue.
		 */
		TOK_PUTSRX(bus + COMMAND_REG, ACK_INT);
		TRACE_DBG(MON_OTHER, "drdc", 0, 0, 0);
		continue;
	    }
	    /*
	     *  Check to see if timeout has been reached.
	     */
	    if (ntimercmp(current_time, timeout_time, >)) {
		BUSIO_DET(bus);
		TRACE_DBG(MON_OTHER, "drdE", ETIMEDOUT, 0, 0);
		return (ETIMEDOUT);
	    }
	    curtime(&current_time);
	    
	}
	
	/*
	 *  Get what the last receive list is for current frame and
	 *  save off the first receive list so that the receive lists
	 *  processed as part of this receive interrupt can be
	 *  re-armed.
	 */
	WRK.dump_read_last = (recv_list_t *) 
	    ((WRK.recv_iwe.stat1 << 16) | WRK.recv_iwe.stat2);
	WRK.sav_index = WRK.read_index;
	
    } /* End of !dump_read_started */
    
    /*
     *  Read recv_lists until have a valid frame or until 
     *  end of recv_lists
     */
    sof = eof = done = bad_frame = copied = FALSE;
    mbuf_index = MTOD(p_mbuf, char *);
    
    while (TRUE) {
	/*
	 *  Set the dump index to the read index
	 */
	index = WRK.read_index;
	
	/* 
	 *  Read in recv_list from DMA.
	 */
	if (WRK.do_dkmove) {
	    d_kmove (&recv_list,
		      WRK.recv_list[index],
		      sizeof(recv_list_t),
		      WRK.dma_chnl_id,
		      DDI.bus_id,
		      DMA_READ);
	} else {
	    bcopy (WRK.recv_vadr[index],
		   &recv_list,
		   sizeof(recv_list_t));
	}
	
	/*
	 *  In middle of frame without receiving start of frame
	 */
	if ((recv_list.status & VALID) && !sof) {
	    TRACE_DBG(MON_OTHER, "drdE", -1, 2, 0);
	    return(-1);
	}
	
	/*
	 *  if this is a sof recv_list
	 */
	if (recv_list.status & START_OF_FRAME) {
	    /*
	     *  Maximum frame size for dump response is 500 bytes
	     */
	    if ((recv_list.frame_size >= 500) || 
		(recv_list.frame_size < NDD.ndd_mintu)) 
		bad_frame = TRUE;
	    
	    /*
	     *  Reset all variables to read in a new frame.
	     */
	    if (eof) {
		mbuf_index = MTOD(p_mbuf, char *);
	    }
	    sof = TRUE;
	    eof = FALSE;
	    size = recv_list.frame_size;
	}
	
	/*
	 *  Copy data to dump user mbuf - if it's a good frame.
	 */
	if (!bad_frame) {
	    rc =  d_complete (WRK.dma_chnl_id,DMA_READ,
				  MTOD(WRK.recv_mbuf[index],
				       unsigned short *), 
				  WRK.recv_mbuf[index]->m_len,
				  M_XMEMD(WRK.recv_mbuf[index]),
				  WRK.recv_addr[index]);
	    data_area = MTOD(WRK.recv_mbuf[index], char *);
	    /*
	     *  If this is the first recv_list of the frame
	     *  then copy the total amount of data in the
	     *  receive list, otherwise just copy the amount
	     *  of data in the recv_list up to the total frame
	     *  size.
	     */
	    if (!copied) {
		copied = TRUE;
		count = recv_list.count;
	    } else
		count = size - recv_list.count;
	    
	    /*
	     *  Copy the data and update the index into the dump
	     *  mbuf data area.
	     *
	     *  NOTE: normally the cache_inval would be done after
	     *  the bcopy.  The dump will include everything which
	     *  is in memory INCLUDING the TR DD receive buffers.
	     *  If the cache_inval is done after the bcopy, when
	     *  the dump DD copies the TR DD receive buffer to its
	     *  transmit mbuf that action caused the buffer to be
	     *  read into the memory cache.  The TR adapter will
	     *  update system memory, but not the memory cache.
	     *  When the TR DD reads the receive buffer, it will
	     *  get the data from the PREVIOUS reception instead of
	     *  the current data.
	     */
	    cache_inval(data_area, count);
	    bcopy(data_area, mbuf_index, count);
	    
	    mbuf_index += count;
	}
	
	WRK.read_index = (WRK.read_index + 1) % RCV_CHAIN_SIZE;
	
	/*
	 *  If eof recv_list
	 */
	if (recv_list.status & END_OF_FRAME) {
	    /*
	     *  If frame is NOT to an individual address, it is a bad frame
	     */
	    if (*(uchar *)((p_mbuf->m_data)+2) & MULTI_BIT_MASK) {
		bad_frame = TRUE;
	    }

	    /*
	     *  If frame is a mac frame then mark it as a bad frame
	     */
	    p_mac = MTOD(p_mbuf, struct ie5_mac_hdr *);
	    if ((p_mac->mac_fcf & FCF_MASK) == MAC_FRAME) {
		bad_frame = TRUE;
	    }
	    macsize = mac_size(p_mac);

	    /*
	     *  Filter off all frames that are not to AA SAP
	     */
	    p_dsap = MTOD(p_mbuf, uchar *);
	    dsap = *(uchar *)(p_dsap + macsize);
	    if (dsap != 0xaa) {
		bad_frame = TRUE;
	    }
	    
	    TRACE_DBG(MON_OTHER, "drd2", p_mbuf, bad_frame, dsap);
	    /* 
	     *  If the frame was bad then drop frame and read in 
	     *  next frame, otherwise process frame.
	     */
	    if (bad_frame) {
		sof=FALSE;
		eof = TRUE;
		bad_frame = FALSE;
		copied = FALSE;
	    } else {
		/*
		 * skip MAC hdr, LLC hdr, and SNAP hdr
		 */
		p_mbuf->m_pkthdr.len = size - (macsize +
			sizeof(struct ie2_llc_snaphdr) );
		p_mbuf->m_len = size - (macsize +
			sizeof(struct ie2_llc_snaphdr) );
		p_mbuf->m_data = (caddr_t)p_mac + macsize +
			sizeof(struct ie2_llc_snaphdr);
		TRACE_DBG(MON_OTHER, "drd3", p_mbuf->m_data, p_mbuf->m_len, 0);
		done = TRUE;
	    }
	}
	
	/*
	 *  If the last recv_list then issue receive valid and quit
	 */
	if (WRK.recv_list[index] == WRK.dump_read_last) {
	    /*    
	     *  Update the RCV List elements so the
	     *  adapter can use the buffer locations for
	     *  rcv data
	     */
	    while (TRUE) {
		/* 
		 *  Read in recv_list from DMA.
		 */
		if (WRK.do_dkmove) {
		    d_kmove (&recv_list,
			      WRK.recv_list[WRK.sav_index],
			      sizeof(recv_list_t),
			      WRK.dma_chnl_id,
			      DDI.bus_id,
			      DMA_READ);
		} else {
		    bcopy (WRK.recv_vadr[WRK.sav_index],
			   &recv_list,
			   sizeof(recv_list_t));
		}
		
		recv_list.status = (FRAME_INTERRUPT | VALID);
		recv_list.frame_size = 0;
		
		/* 
		 *  update the ACS recv list element
		 */
		if (WRK.do_dkmove) {
		    d_kmove( &recv_list, 
			     WRK.recv_list[WRK.sav_index],
			     sizeof(recv_list_t),
			     WRK.dma_chnl_id,
			     DDI.bus_id,
			     DMA_WRITE_ONLY);
		} else {
		    bcopy( &recv_list,
			  WRK.recv_vadr[WRK.sav_index],
			  sizeof(recv_list_t) );
		}
		
		/*
		 * increment our index to the next recv list
		 * slot if we are not at the current read index 
		 * 
		 * if we are at the current read index, break
		 * from this while loop.
		 */
		if ( WRK.sav_index != index )
		    WRK.sav_index = (WRK.sav_index + 1) % RCV_CHAIN_SIZE;
		else
		    break;
	    }  
	    
	    WRK.dump_read_started = FALSE;
	    bus = BUSIO_ATT(DDI.bus_id, DDI.io_port);
	    TOK_PUTSRX(bus + COMMAND_REG, RCV_VALID);
	    BUSIO_DET(bus);
	    break;
	}
	
	/*
	 *  If a valid frame was received then return
	 */
	if (done)
	    break;
	
    } /* End while (TRUE) */
    
    TRACE_DBG(MON_OTHER, "drdE", 0, 0, 0);
    
    return(0);
    
} /* End of tok_dump_read() */

/*---------------------- T O K _ D U M P _ W R T -----------------------*/
/*                                                                      */
/*  NAME: tok_dump_wrt                                                  */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Processes a dump write request                                  */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Called on the process thread by the dump DD.                    */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS: a -1 if there was a failure and 0 upon success.            */
/*                                                                      */
/*----------------------------------------------------------------------*/

tok_dump_wrt (
    ndd_t	*p_ndd,
    struct mbuf *p_mbuf)
{
    dds_t              *p_dds;         /* Pointer to dds               */
    ushort             reason;         /* Interrupt reason             */
    int                rc,             /* General return code          */
		       done,           /* Flag for tx processing       */
		       bus;            /* Bus I/O address attachment   */
    struct timestruc_t current_time,   /* Keeps the current time       */
		       timeout_time,   /* Time out value               */
		       temp_time;
    xmt_elem_t         *xlm;           /* Transmit element pointer     */
    xmit_des_t         *xd;            /* pointer to tx descriptor     */
    register int       len;            /* Length of data in mbuf       */
    t_tx_list          tmp_tx_list;    /* Temporary tx_list            */
    t_tx_list          *p_tx_tmp,      /* first tx_list to be updated  */	
		       *p_d_tx_tmp;    /* same as above but dma addr   */
    t_scb              scb;            /* System command block         */
    ulong	       irr;	  	/* IOCC Int Request Reg pointer */
    ulong	       irr_value;	/* IOCC Interrupt Request Reg   */
    ulong	       mask;		/* bus_intr_lvl mask		*/
    int		       mlen, mptr, blen, bptr, i, tx_ds_num;
    
    
    TRACE_DBG(MON_OTHER, "dwrB", 0, 0, 0);
    
    /*
     *  Get adapter structure
     */
    p_dds = (dds_t *) (((unsigned long) p_ndd) - offsetof(dds_t, ndd));

    /* The driver uses the buffers as one large buffer for dump.  Since the
     * first buffer holds the starting buffer address and dma address it will
     * be used by dump.  This depends on the buffers being allocated in a  chunk
     * and d_mastered in order.
     */

    xd = &WRK.tx_buf_des[0];

    /* 
     *  If a dump read has not finished processing then we need to 
     *  finish that processing
     */
    if (WRK.dump_read_started) {
	clean_up_read(p_dds);
    }
    
    /*
     *  Because of timing problems during a dump, the transmit status of
     *  the write will not be processed until the next time a transmit is
     *  is issued.  Because of this it is necessary to verify that the
     *  last transmit packet was issued before trying to issue the next
     *  transmit.  If the last transmit was issued then just re-validate
     *  the transmit element and issue the next transmit.  If the 
     *  transmit has not been issued then need to just return as if the
     *  transmit had been issued.
     */
    if (!WRK.dump_first_wrt) {
	
	/*
	 *  If the transmit status has not been read in 
	 *  yet then poll for the transmit status.
	 */
	if (WRK.tx_iwe.stat0 == 0xffff)	{
	    /*
	     *  Set up time information
	     */
	    ms2time(TX_DUMP_TIMEOUT, &temp_time);
	    curtime(&current_time);
	    ntimeradd(current_time, temp_time, timeout_time);
	    
	    bus = BUSIO_ATT(DDI.bus_id, DDI.io_port);
	    
	    /*
	     *  Poll the adapter until timeout or interrupt is 
	     *  detected
	     *
	     *  NOTE: previous levels of the adapter card do not
	     *  set the status register correctly in polling mode.
	     *  To get around that:
	     *  - do NOT disable interrupts from the card
	     *  - read the IOCC interrupt request register and look
	     *    for an interrupt pending on the interrupt level
	     *    which the TR card is running on
	     *  - if an interrupt is pending, then read the status
	     *    register and process the interrupt
	     */
	    mask = 0x80000000 >> DDI.intr_level;
	    
	    for (;;) {
		if (__power_rs())
		{
		    irr = (uint)IOCC_ATT(DDI.bus_id, TOK_RS_IIR); 
		}
		else
		{
		    irr = (uint)IOCC_ATT(DDI.bus_id, TOK_PC_IIR); 
		}
		TOK_GETLX(irr, &irr_value);
		IOCC_DET(irr);
		if (irr_value & mask) {
		    /*
		     *  Read in status element from adapter 
		     */
		    TOK_GETSRX( bus + STATUS_REG, &reason);
		    TRACE_DBG(MON_OTHER, "dwrr", reason, 0, 0);
		} else {
		    reason = 0;	
		}
		/*
		 *  Check if the adapter has set system 
		 *  interrupt bit
		 */
		if ( reason & TOK_INT_SYSTEM) {
		    /*
		     *  Process unexpected read interrupt.
		     */
		    if ((reason & TOK_IC) == RECEIVE_STATUS) {
			/*
			 *  Copy in the receive status 
			 *  work element from DMA.
			 */
			if (WRK.do_dkmove) {
			    d_kmove(&(WRK.recv_iwe.cmd),
				    WRK.p_d_ssb,
				    sizeof(t_ssb),
				    WRK.dma_chnl_id,
				    DDI.bus_id,
				    DMA_READ);
			} else {
			    bcopy (WRK.p_ssb,
				   &(WRK.recv_iwe.cmd),
				   sizeof(t_ssb));
			}
			
			TOK_PUTSRX(bus + COMMAND_REG, ACK_INT);
			/*
			 *  Clean up the receive queue 
			 *  by dropping all of the 
			 *  frames.
			 */
			WRK.dump_read_last = (recv_list_t *) 
				((WRK.recv_iwe.stat1 << 16)
				 | WRK.recv_iwe.stat2);
			WRK.sav_index = WRK.read_index;
			clean_up_read(p_dds);
			continue;
		    }
		    
		    /*
		     *  Validate that it is a transmit 
		     *  interrupt.
		     */
		    if ((reason & TOK_IC) == TRANSMIT_STATUS) {
			/*
			 *  Copy in the transmit status 
			 *  work element from DMA.
			 */
			if (WRK.do_dkmove) {
			    d_kmove (&(WRK.tx_iwe.cmd),
				      WRK.p_d_ssb,
				      sizeof(t_ssb),
				      WRK.dma_chnl_id,
				      DDI.bus_id,
				      DMA_READ);
			} else {
			    bcopy (WRK.p_ssb,
				   &(WRK.tx_iwe.cmd),
				   sizeof(t_ssb));
			}
			
			TOK_PUTSRX(bus + COMMAND_REG, ACK_INT);
			BUSIO_DET(bus);
			break;
		    } 
		    /*
		     * Some other interrupt like CK or
		     * some other fatal error. So just
		     * acknowledge the interrupt and
		     * continue.
		     */
		    TOK_PUTSRX(bus + COMMAND_REG, ACK_INT);
		    TRACE_DBG(MON_OTHER, "dwrc", 0, 0, 0);
		    continue;
		}
		/*
		 *  Check to see if timeout has been reached.
		 */
		if (ntimercmp(current_time, timeout_time, >)) {
		    BUSIO_DET(bus);
		    TRACE_DBG(MON_OTHER, "dwrE", ENETDOWN, reason, 0);
		    return (0);
		}
		curtime(&current_time);
		
	    } /* End of for(;;) */
	    
	} /* End of stat0 == 0xffff */
	
	/*
	 *  If there was a list error then quit processing
	 */
	if ((WRK.tx_iwe.stat0 & 0xff00) == LIST_ERROR) {
	    TRACE_DBG(MON_OTHER, "dwrE", -1, 0, 0);
	    return(-1);
	}
	
	/*
	 *  If received a transmit frame interrupt then just 
	 *  continue processing
	 */
	if ((WRK.tx_iwe.stat0 & 0xff00) == TX_CMD_COMPLETE) {
	    
	    p_tx_tmp = WRK.p_tx_1st_update;
	    p_d_tx_tmp = WRK.p_d_tx_1st_update;
	    
	    /*
	     *  Copy the adapters transmit list into
	     *  our copy of the transmit list.
	     */
	    move_tx_list(p_dds, &tmp_tx_list, p_tx_tmp, p_d_tx_tmp, 
			 DMA_READ);
	    

	    /*
	     *   issue the d_complete for the transmit data area(s)
	     *   d_completes all of the buffer used.
	     */
	     d_complete(WRK.dma_chnl_id, DMA_WRITE_ONLY,
			xd->sys_addr, xd->count,
			(struct xmem *)&WRK.xbuf_xd, xd->io_addr);

	    
	    /*
	     *  Clear our copy of the transmit list and
	     *  rewrite it to the adapters tx list.
	     */
	    tmp_tx_list.tx_cstat = 0;
	    tmp_tx_list.frame_size = 0;
	    tmp_tx_list.p_tx_elem = NULL;
	    bzero(&tmp_tx_list.gb, sizeof(tmp_tx_list.gb));
	    
	    move_tx_list( p_dds, &tmp_tx_list, p_tx_tmp,
			 p_d_tx_tmp, DMA_WRITE_ONLY);
	    
	}
	
    } /* End of not first write */
    
    /* 
     *  Get the current software transmit element
     */
    xlm = &WRK.xmit_queue[0];
    
    /*
     *   Fill in the data for the transmit element.
     */
    TRACE_DBG(MON_OTHER, "dwr1", p_mbuf, p_mbuf->m_len, 0);

    xlm->mbufp = p_mbuf;
    xd->count = p_mbuf->m_len;
    bcopy(mtod(p_mbuf,caddr_t),xd->sys_addr,p_mbuf->m_len);
    vm_cflush(xd->sys_addr, xd->count);
    
    bzero(&tmp_tx_list, sizeof(tmp_tx_list) );

    /* set up the temp. TX List Chain element */
    tmp_tx_list.gb[0].cnt = (ushort)xd->count;
    tmp_tx_list.gb[0].addr_hi = (ushort)ADDR_HI(xd->io_addr);
    tmp_tx_list.gb[0].addr_lo = (ushort)ADDR_LO(xd->io_addr);


    tmp_tx_list.frame_size =(ushort)xd->count;
    tmp_tx_list.p_tx_elem = xlm;

    /*
     * set CSTAT and move the TX list chain element(s) to the ACA
     */
    tmp_tx_list.tx_cstat = ( TX_START_OF_FRAME | TX_VALID_CHAIN_EL |
			    TX_END_OF_FRAME );
    move_tx_list( p_dds, &tmp_tx_list, WRK.p_tx_next_avail, 
			WRK.p_d_tx_next_avail, DMA_WRITE_ONLY);

    
    /*
     *  Notify the adapter to process the transmit command
     */
    scb.adap_cmd = ADAP_XMIT_CMD;
    scb.addr_field1 = ADDR_HI(WRK.p_d_tx_next_avail);
    scb.addr_field2 = ADDR_LO(WRK.p_d_tx_next_avail);
    if (WRK.do_dkmove) {
        d_kmove(&scb, WRK.p_d_scb, sizeof(scb),
		 WRK.dma_chnl_id, DDI.bus_id, DMA_WRITE_ONLY);
    } else {
	bcopy(&scb, WRK.p_scb, sizeof(scb));
    }
    
    bus = BUSIO_ATT(DDI.bus_id, DDI.io_port);
    TOK_PUTSRX(bus + COMMAND_REG, EXECUTE);
    BUSIO_DET(bus);
    
    /*
     *  Set the tx_iwe status to be a number that can be checked to
     *  if a new transmit status was received.
     */
    WRK.tx_iwe.stat0 = 0xffff;
    WRK.dump_first_wrt = FALSE;
    
    TRACE_DBG(MON_OTHER, "dwrE", 0, 0, 0);
    
    return(0);
    
} /* End of tok_dump_wrt() */

/*------------------------- D U M P _ S E T U P ------------------------*/
/*                                                                      */
/*  NAME: dump_setup                                                    */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Setups the receive list for dump processing and resets the      */
/*      first forward pointer of the transmit list to be an odd address */
/*      to mark the end of the transmit list.  The remaining transmit   */
/*      buffers are used to fill in any gaps that may exist in the      */
/*      receive list because we were unable to get enough mbufs.  If    */
/*      we use all of the excess transmit buffers and there are still   */
/*      gaps in the receive list then update the forward pointers to    */
/*      skip past those receive lists that do not have valid buffers.   */
/*      Also, issues a transmit halt to purge the last transmit command */
/*      issued.                                                         */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Called by the dump entry point.                                 */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS: a -1 if there was a failure and 0 upon success.            */
/*                                                                      */
/*----------------------------------------------------------------------*/

dump_setup(p_dds)
    dds_t               *p_dds;       /* Pointer to the dds          */
{
    ushort              reason;       /* Interrupt reason            */
    int                 rc;           /* Return code                 */
    int                 bus;          /* Bus I/O address attachment  */
    struct timestruc_t  current_time, /* Keeps the current time      */
			timeout_time, /* Time out value              */
			temp_time;
    t_scb               scb;          /* System command block        */
    ulong		irr;	      /* IOCC Int Request Reg ptr    */
    ulong		irr_value;    /* IOCC Interrupt Request Reg  */
    ulong	 	mask;	      /* bus_intr_lvl mask	     */
    
    TRACE_DBG(MON_OTHER, "dsuB", 0, 0, 0);
    /*
     *  Issue a transmit halt command.
     */
    scb.adap_cmd = ADAP_XMIT_HALT;
    scb.addr_field1 = 0;
    scb.addr_field2 = 0;
    if (WRK.do_dkmove) {
	d_kmove(&scb, WRK.p_d_scb, sizeof(scb),
		 WRK.dma_chnl_id, DDI.bus_id, DMA_WRITE_ONLY);
    } else {
	bcopy(&scb, WRK.p_scb, sizeof(scb));
    }
    
    bus = BUSIO_ATT(DDI.bus_id, DDI.io_port);
    TOK_PUTSRX(bus + COMMAND_REG, EXECUTE);
    BUSIO_DET(bus);
    
    /*
     *  Set up time information
     */
    ms2time(TX_DUMP_TIMEOUT, &temp_time);
    curtime(&current_time);
    ntimeradd(current_time, temp_time, timeout_time);
    
    bus = BUSIO_ATT(DDI.bus_id, DDI.io_port);
    
    /*
     *  Poll the adapter until timeout or transmit interrupt is detected
     *
     *  NOTE: previous levels of the adapter card do not set the status
     *  register correctly in polling mode.  To get around that:
     *  - do NOT disable interrupts from the card
     *  - read the IOCC interrupt request register and look for an
     *    interrupt pending on the interrupt level which the TR
     *    card is running on
     *  - if an interrupt is pending, then read the status register
     *    and process the interrupt
     */
    
    mask = 0x80000000 >> DDI.intr_level;
    for (;;) {
	if (__power_rs())
	{
	    irr = (uint)IOCC_ATT(DDI.bus_id, TOK_RS_IIR); 
	}
	else
	{
	    irr = (uint)IOCC_ATT(DDI.bus_id, TOK_PC_IIR); 
	}
	TOK_GETLX(irr, &irr_value);
	IOCC_DET(irr);
	
	if (irr_value & mask) {
	    /*
	     *  Read in the status element from adapter.
	     */
	    TOK_GETSRX( bus + STATUS_REG, &reason);
	    TRACE_DBG(MON_OTHER, "dsur", reason, 0, 0);
	} else {
	    reason = 0;	
	}
	
	/*
	 *  Check if the adapter has set system interrupt bit
	 */
	if ( reason & TOK_INT_SYSTEM) {
	    /*
	     *  Process unexpected read interrupt.
	     */
	    if ((reason & TOK_IC) == RECEIVE_STATUS) {
		TRACE_DBG(MON_OTHER, "dsu1", 0, 0, 0);
		/*
		 *  Copy in the receive status work 
		 *  element from DMA.
		 */
		if (WRK.do_dkmove) {
		    d_kmove (&(WRK.recv_iwe.cmd),
			      WRK.p_d_ssb,
			      sizeof(t_ssb),
			      WRK.dma_chnl_id,
			      DDI.bus_id,
			      DMA_READ);
		} else {
		    bcopy (WRK.p_ssb,
			   &(WRK.recv_iwe.cmd),
			   sizeof(t_ssb));
		}
		
		TOK_PUTSRX(bus + COMMAND_REG, ACK_INT);
		BUSIO_DET(bus);
		/*
		 *  Clean up the receive queue by 
		 *  dropping all of the frames.
		 */
		WRK.dump_read_last = (recv_list_t *) 
		    ((WRK.recv_iwe.stat1 << 16) | WRK.recv_iwe.stat2);
		WRK.sav_index = WRK.read_index;
		clean_up_read(p_dds);
		bus = BUSIO_ATT(DDI.bus_id, DDI.io_port);
		continue;
	    }
	    
	    /*
	     *  If it is a transmit status interrupt, get status, & ACK it.
	     */
	    if ((reason & TOK_IC) == TRANSMIT_STATUS) {
		TRACE_DBG(MON_OTHER, "dsu2", 0, 0, 0);
		/*
		 *  Copy in the transmit status work 
		 *  element from DMA.
		 */
		if (WRK.do_dkmove) {
		    d_kmove (&(WRK.tx_iwe.cmd),
			      WRK.p_d_ssb,
			      sizeof(t_ssb),
			      WRK.dma_chnl_id,
			      DDI.bus_id,
			      DMA_READ);
		} else {
		    bcopy (WRK.p_ssb,
			   &(WRK.tx_iwe.cmd),
			   sizeof(t_ssb));
		}
		
		TOK_PUTSRX(bus + COMMAND_REG, ACK_INT);
		break;
	    } 
	    
	    /*
	     *  Some other interrupt like CK or some other fatal 
	     *  error. So just acknowledge the interrupt and 
	     *  continue.
	     */
	    TRACE_DBG(MON_OTHER, "dsuc", 0, 0, 0);
	    TOK_PUTSRX(bus + COMMAND_REG, ACK_INT);
	    continue;
	}
	/*
	 *  Check to see if timeout has been reached.
	 */
	if (ntimercmp(current_time, timeout_time, >)) {
	    TRACE_DBG(MON_OTHER, "dsut", 0, 0, 0);
	    BUSIO_DET(bus);
	    WRK.p_d_tx_fwds[0]=0x1;
	    TRACE_DBG(MON_OTHER, "dsuE", 1, 0, 0);
	    return (0);
	}
	curtime(&current_time);
	
    } /* End of for(;;) */
    
    /*
     *  Update the forward pointer in the TX_LIST so that it will be
     *  a one element list.
     */
    WRK.p_d_tx_fwds[0]=0x1;
    BUSIO_DET(bus);
    TRACE_DBG(MON_OTHER, "dsuE", 0, 0, 0);
    
    return(0);
    
} /* End of dump_setup() */

/*
 * Convert a micro-second to a rtcl_t time value.
 */
#define	MSEC_PER_SEC	(NS_PER_SEC / NS_PER_MSEC)
ms2time(msec, tp)
    int                     msec;    /*  Inputted timeout value         */
    struct timestruc_t      *tp;     /*  Outputted converted time value */
{
    tp->tv_sec = msec / MSEC_PER_SEC;
    tp->tv_nsec = (msec % MSEC_PER_SEC) * NS_PER_MSEC;
} /* End of ms2time() */

/*--------------------- C L E A N _ U P _ R E A D ----------------------*/
/*                                                                      */
/*  NAME: clean_up_read                                                 */
/*                                                                      */
/*  FUNCTION:                                                           */
/*                                                                      */
/*  This routine reads in all frames currently on the receive chain and */
/*  drops them.  It then updates the receive counters and issues a      */
/*  RECV_VALID to the adapter to notify that we have processed the      */
/*  receive interrupt.                                                  */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*  This routine is called from the tok_dump_wrt entry point to clean   */
/*  up any outstanding read interrupts before processing the write      */
/*  request.                                                            */
/*                                                                      */
/*----------------------------------------------------------------------*/


clean_up_read(p_dds)
    dds_t              *p_dds;         /* Pointer to the dds           */
{
    int                bus;            /* Bus I/O address attachment   */
    int                rc;             /* Return code                  */
    int                last_index;     /* Last recv_list to update     */
    unsigned char      *recvaddr;      /* DMA address of data area     */
    recv_list_t        recv_list;      /* Temporary recv_list          */
    
    TRACE_DBG(MON_OTHER, "dcuB", 0, 0, 0);
    /*
     *  Need to bump the read index until it points to
     *  the last recv_list of last receive chain.
     */
    while (WRK.recv_list[WRK.read_index] != WRK.dump_read_last) {
	WRK.read_index = (WRK.read_index + 1) % RCV_CHAIN_SIZE;
    }
    
    last_index = WRK.read_index;
    
    WRK.read_index = (WRK.read_index + 1) % RCV_CHAIN_SIZE;
    
    /*    
     *  Update the RCV List elements so the
     *  adapter can use the buffer locations for
     *  rcv data
     */
    while (TRUE) {
	/* 
	 *  Read in recv_list from DMA.
	 */
	if (WRK.do_dkmove) {
	    d_kmove (&recv_list,
		      WRK.recv_list[WRK.sav_index],
		      sizeof(recv_list_t),
		      WRK.dma_chnl_id,
		      DDI.bus_id,
		      DMA_READ);
	} else {
	    bcopy (WRK.recv_vadr[WRK.sav_index],
		   &recv_list,
		   sizeof(recv_list_t));
	}
	
	recv_list.status = (FRAME_INTERRUPT | VALID);
	recv_list.frame_size = 0;
	
	/*
	 *  update the ACS recv list element
	 */
	if (WRK.do_dkmove) {
	    d_kmove( &recv_list, 
		     WRK.recv_list[WRK.sav_index],
		     sizeof(recv_list_t),
		     WRK.dma_chnl_id,
		     DDI.bus_id,
		     DMA_WRITE_ONLY);
	} else {
	    bcopy( &recv_list,
		  WRK.recv_vadr[WRK.sav_index],
		  sizeof(recv_list_t) );
	}
	
	/*
	 * increment our index to the next recv list
	 * slot if we are not at the current read index 
	 * 
	 * if we are at the current read index, break
	 * from this while loop.
	 */
	if ( WRK.sav_index == last_index )
	    break;
	WRK.sav_index = (WRK.sav_index + 1) % RCV_CHAIN_SIZE;
    }   /* End of while (TRUE) */
    
    /*
     *  Send a receive valid to the adapter
     */
    WRK.dump_read_started = FALSE;
    bus = BUSIO_ATT(DDI.bus_id, DDI.io_port);
    TOK_PUTSRX(bus + COMMAND_REG, RCV_VALID);
    BUSIO_DET(bus);
    TRACE_DBG(MON_OTHER, "dcuE", 0, 0, 0);
    
} /* End of clean_up_read() */
