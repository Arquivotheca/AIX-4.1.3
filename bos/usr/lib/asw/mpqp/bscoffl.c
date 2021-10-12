static char sccsid[] = "@(#)25	1.24  src/bos/usr/lib/asw/mpqp/bscoffl.c, ucodmpqp, bos411, 9428A410j 4/2/94 00:57:35";

/*--------------------------------------------------------------------------
*
*				  BSCOFFL.C
*
*  COMPONENT_NAME:  (UCODEMPQ) Multiprotocol Quad Port Adapter Software.
*
*  FUNCTIONS:	tx_pre_bsc, bsc_tsm, read_char, bsc_xmit_frame, 
*		bsc_rsm, write_char, attach_srcbuf, attach_destbuf,
*		reset_srcbuf, next_bcc, rx_post_bsc, make_rqe
*		cop_decode
*
*  ORIGINS: 27
*
*  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
*  combined with the aggregated modules for this product)
*                   SOURCE MATERIALS
*  (C) COPYRIGHT International Business Machines Corp. 1989
*  All Rights Reserved
*
*  US Government Users Restricted Rights - Use, duplication or
*  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
*
*--------------------------------------------------------------------------
*/

#include "mpqp.def"
#include "mpqp.typ"
#include "mpqp.ext"
#include "mpqp.h"
#include "mpqp.pro"
#include "portcb.typ"
#include "portcb.def"
#include "iface.def"

/* State Machine states: */

# define XMITINIT		1
# define STSENT			2
# define ENDXMIT		3
# define XINITXMIT		4
# define XSOHSENT		5
# define XSTXSENT		6
# define XMITEND		7
# define INITRECV		8
# define DLESEEN		9
# define BLKRECV		10

/* Control character table macros for EBCDIC and ASCII:	*/

# define SOH	( (unsigned short)0x01 )
# define STX	( (unsigned short)0x02 )
# define ITB	( (unsigned short)0x1F )
# define ETB	( (unsigned short)char_tbl[3] )
# define ETX	( (unsigned short)0x03 )
# define DLE	( (unsigned short)0x10 )
# define ENQ	( (unsigned short)char_tbl[6] )
# define EOT	( (unsigned short)char_tbl[7] )
# define NAK	( (unsigned short)char_tbl[8] )
# define SYN	( (unsigned short)char_tbl[9] )
# define RVI	( (unsigned short)char_tbl[10] )
# define ACK0	( (unsigned short)char_tbl[11] )
# define ACK1	( (unsigned short)char_tbl[12] )
# define WACK	( (unsigned short)char_tbl[13] )
# define EOB	( 0x1000 )

extern const unsigned char	asc_tbl[];	/* ASCII translation table */
extern const unsigned char	ebc_tbl[];	/* EBCDIC translation table */

extern unsigned short	txflgt [ NUM_PORT ];	/* transmit frame length */
extern unsigned short	txinc  [ NUM_PORT ];	/* # chars between SYN's */
extern unsigned char	txcmd  [ NUM_PORT ];	/* transmit command */
extern unsigned char	txftype[ NUM_PORT ];	/* transmit frame type */
extern unsigned short	txccw  [ NUM_PORT ];	/* Channel control word */
extern unsigned short	txlgt  [ NUM_PORT ];	/* transmit length */
extern unsigned short	txioa  [ NUM_PORT ];	/* TX I/O address */
extern unsigned short	tscc_val[ NUM_PORT ];	/* timer timeout value */
extern unsigned short	tscc_typ[ NUM_PORT ];	/* timer type */
extern t_pdma_setup	pdma_cdt;		/* Channel Descriptor Table */

extern tx_ack( t_pcb *p_pcb, bufno *p_last, bufno *p_next );

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
/*                      BISYNC TRANSMIT PROCESSING			*/
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

/*  Transmit pre-processing consists of mapping the source frame data	*/
/*  through a transmit state machine to a destination buffer area.	*/
/*  This buffer must be large enough to accomodate the original data	*/
/*  plus insertion of DLE's, SYN's, and the frame check sequence.  If	*/
/*  S is the size of the source buffer, the maximum destination buffer 	*/
/*  size needed to hold the mapped frame is given by:			*/
/*									*/
/*			D = ( 2 + 16/B )S + 2				*/
/*									*/
/*  where B is the baud rate and D is the destination buffer size.  	*/
/*  For a worst case baud rate of 150 and a source buffer size of 4096,	*/
/*  the destination buffer must be at least 8631 bytes long to handle	*/
/*  maximum character insertion.  Each port needs two extensions for 	*/
/*  double-buffering DMA, or eight in all (69048 bytes in all).   	*/
/*	On cards with 512K of memory, this space is not available, so	*/
/*  transmit mapping is performed using the remaining space in the	*/
/*  source transmit buffer.  For a fixed size transmit buffer of size	*/
/*  T, the largest source frame that can be processed is given by:	*/
/*									*/
/*			S = T/( 3 + 16/B ) - 2				*/
/*  									*/
/*  For a transmit buffer of size T = 4096, the source data can be at	*/
/*  most 1316 bytes (150 baud) to guarantee no buffer overflow.		*/
/*          								*/
/*----------------------------------------------------------------------*/


/*-----------------------  T X _ P R E _ B S C  ------------------------*/
/*									*/
/*  NAME: tx_pre_bsc							*/
/*									*/
/*  FUNCTION:								*/
/*	Bisync transmit pre-processing; this routine maps the source	*/
/*	frame data through the transmit state machine to a transmit	*/
/*	extension buffer, which is later DMA'ed to the DUSCC by the	*/
/*	bsc_xmit_frame routine.						*/
/*									*/
/*  EXECUTION ENVIRONMENT:						*/
/*	Runs at off-level.						*/
/*									*/
/*  DATA STRUCTURES:							*/
/*	Accesses and modifies the port control block.			*/
/*  									*/
/*  RETURNS:    							*/
/*	Length of transmit buffer after pre-processing.			*/
/*									*/
/*----------------------------------------------------------------------*/

tx_pre_bsc( 
	cmd_t		*p_cmd,
    	volatile t_pcb	*p_pcb )
{
	register unsigned short		srclen;
	register int			destlen;
	register unsigned char far	*p_src;
	register unsigned char far	*p_dest;
	unsigned short			port;
	unsigned char			ascii, trans, parity;
	unsigned char			memsize = MEMORY_SIZE;
	t_rqe				rqe;
	
	FOOTPRINT( 'b', 't', 'a' );			/* bta */
	port  = p_cmd->port;
	if (p_pcb->port_state == V25_CALL_DIALING)
	    ascii = p_pcb->dial_flags & PCB_PF_ASCII;
	else
	    ascii = p_pcb->data_flags & PCB_PF_ASCII;
	trans = p_cmd->flags & TX_TRANSPARENT;
						
	/*  If this is a 1 or 2 Meg card, use the transmit extension 	*/
	/*  buffers located in the additional memory area -- toggle	*/
	/*  between buffers at (port x 2) and (port x 2 + 1) units 	*/
	/*  from the base of the transmit extension segment.		*/

	if (( memsize == M_1MEG ) || ( memsize == M_2MEG ))
	{
	    p_dest  = XMIT_EXTENSION(( port << 1 ) + ODD( p_pcb->txchan));
	    destlen = TX_EXT_LEN;

	/*  If the card is a 512K, we are forced to use the remaining 	*/
	/*  space in the source transmit buffer:			*/

	} else {
	    p_dest  = XMIT_BUFFER((cmd_blk *)p_cmd - &cmdblk[0]) 
			+ p_cmd->length;
	    destlen = TX_BUF_LEN - p_cmd->length;
	}
	p_cmd->type = TX_LONG;			/* all are made long */

	/*------------------  Hardware Work-around  --------------------*/
	/*  The DUSCC transmit state machine has many problems; in	*/
	/*  particular, it inserts the wrong kind of idle SYN's in	*/
	/*  transparent mode.  To get around this and other problems,	*/
	/*  the transmit state machine is implemented here in software	*/
	/*  and the DUSCC is used in COP DUAL SYN mode on transmits.	*/
	/*  All Bisync transmit processing is done here in software.	*/

	attach_srcbuf ( p_pcb, p_cmd->card_addr,  p_cmd->length );
	attach_destbuf( p_pcb, p_dest, destlen );
	destlen = bsc_tsm( p_pcb, trans, !ascii, txinc[ port ] );

	/*--------------------------------------------------------------*/

	if ( destlen < 0 ) 			/* Transmit overflow? */
	{
	    rqe.f.rtype	   = RQE_TX_NAK;	/* return error RQE */
	    rqe.f.port	   = p_cmd->port;
	    rqe.f.status   = TX_BUF_ERR;	/* not enough space */
	    rqe.f.sequence = p_cmd->sequence;
	    add_rqe( p_pcb, rqe.val );
	    FOOTPRINT( 'b', 't', 'E' );		/* btE */
	    return ( destlen );			/* then return the error */
	}					/* else, good frame: */
	p_pcb->tx_frames++;			/* update statistics */
	p_pcb->tx_bytes += destlen;
	txflgt[port] = destlen;			/* update port frame len */
	p_cmd->card_addr = p_dest;		/* xmit extension address */
	if ( ascii )				/* set parity if ascii */
	    if (p_pcb->port_state == V25_CALL_DIALING) {
            /* For BiSync v25.bis, convert dial string to even or odd   */
            /* parity when dial_flags parity enable bit is set.         */
	    /* Even parity doesn't seem to work anyway, this may be     */
	    /* why there isn't any code for doing BiSync, ASCII, even   */
            /* parity - it may be one of those BiSync ASCII limitations.*/
                if (p_pcb->dial_flags & PCB_AF_PAREN) {
                    if (p_pcb->dial_flags & PCB_AF_ODDPAR) {
                        Ascii_To_Op( p_cmd->card_addr, p_cmd->length );
                        FOOTPRINT( 'b', 't', 'c' );         /* btc */
                    } else {
                        Ascii_To_Ep( p_cmd->card_addr, p_cmd->length );
                        FOOTPRINT( 'b', 't', 'd' );         /* btd */
                    }
		}
            } else {
	        Ascii_To_Op( p_dest, destlen );
                FOOTPRINT( 'b', 't', 'b' );         /* btb */
	    }
	if (!( p_pcb->tx_state & FT_B_ITB )) 
	{
	    p_cmd->flags |= TX_FINAL_BLK;	/* if not ITB, final block */
	    txftype[port] = p_pcb->tx_state | FT_B;	/* update type */
	}
	else txftype[port] |= (FT_B_ITB | FT_B);

	if ( ascii )				/* If ascii, set flag */
	    txftype[port] |= FT_B_ASCII;	/* in frame type */
	txcmd[port]   = txftype[ port ];
	pdma_cdt.p._txla  = &( p_pcb->pdma_tx_llc[p_pcb->txchan & 0x01]);
	pdma_cdt._ccw = _CCW_TX_BSC; 		/* Setup DMA; transmit */
						/* Stop on terminal count */
						/* list chain on term. count */
						/* and on end of message */
	return( destlen );			/* return length */
}

/*---------------------------  B S C _ T S M  --------------------------*/
/*									*/
/*  NAME: bsc_tsm							*/
/*									*/
/*  FUNCTION:								*/
/*	Bisync transmit state machine; both normal and transparent	*/
/*	modes.  Maps transmit data in source buffer to destination	*/
/*	buffer (transmit extension).					*/
/*									*/
/*  EXECUTION ENVIRONMENT:						*/
/*	Can be executed at interrupt or process level.			*/
/*									*/
/*  DATA STRUCTURES:							*/
/*	References and modifies port control block.			*/
/*  									*/
/*  RETURNS:    							*/
/*	Data length of destination buffer.				*/
/*									*/
/*----------------------------------------------------------------------*/

bsc_tsm ( p_pcb, transparent, crc, insert_count )
	register volatile t_pcb	*p_pcb;
	int	transparent;		/* TRUE if transparent, else normal */
	int	crc;			/* TRUE if CRC, LRC otherwise */
	int	insert_count;		/* how often to insert SYN's */
{
	register unsigned char	*char_tbl;
	register unsigned short	bcc = 0;	
	register short		c, lc;
	unsigned short		state;
	unsigned short		loop;
	unsigned short		total_chars;
	unsigned short 		read_char(), next_bcc(); 

	FOOTPRINT( 'b', 's', 'a' );			/* bsa */
	char_tbl = crc ? ebc_tbl : asc_tbl;
	p_pcb->write_chars = total_chars = 0;
	p_pcb->tx_state = NULL;

	if ( !transparent ) {
	
	    /*  Normal transmit state machine; read characters from	*/
	    /*  the source buffer, calculate and insert block check	*/
	    /*  characters with data into destination buffer.  Insert	*/
	    /*  idle characters (SYN SYN) at regular intervals.		*/

	    state = XMITINIT;
	    LOOP {					/* loop "forever" */
	        c = read_char( p_pcb );			/* get next char */
	        if ( c == EOB )				/* if no more, */
		    state = ENDXMIT;			/* end the transmit */
		if ((p_pcb->write_chars > insert_count) &&
		    (state != ENDXMIT))
		{
		    if (!write_char( p_pcb, SYN ))	/* insert idles */
			return(-1);
		    if (!write_char( p_pcb, SYN ))
			return(-1);
		    total_chars += p_pcb->write_chars;
		    p_pcb->write_chars = 0;		/* some accounting */
		}
		switch ( state ) 
		{	 
		    case XMITINIT:			/* XMIT INIT STATE */
			if (!write_char( p_pcb, c ))	/* always write it */
			    return(-1);
			bcc = next_bcc( c, bcc, crc );	/*   start on bcc */
			if ((c == SOH) ||		/*   if SOH or	*/
			    (c == STX)) {		/*   STX character, */
				state = STSENT;		/*   switch states, */
				if (!(p_pcb->bsc_prev_tx & FT_B_ITB ))
				    bcc = 0;		/*   and reset bcc */
			}
			if ((c == ITB) ||		/*    if ITB or */
			    (c == ETX) ||		/*    ETX char or */
			    (c == ETB)) {		/*    ETB character, */
				if (!write_char( p_pcb, LO_BYTE( bcc )))
				    return(-1);
				if ( crc ) 		/*    write bcc */
				    if (!write_char( p_pcb, HI_BYTE( bcc )))
					return(-1);
				bcc = 0;		/*    clear bcc */
			        if ( c == ITB ) 	/*    if ITB frame, */
				    p_pcb->tx_state = FT_B_ITB;
			}
			break;
		    case STSENT:			/* START SENT STATE */
			if (!write_char( p_pcb, c ))	/*    always write it */
			    return(-1);
			bcc = next_bcc( c, bcc, crc ); 	/*    calculate bcc */
			if ((c == ITB) ||		/*    if ITB or */
			    (c == ETX) ||		/*    ETX char or */
			    (c == ETB)) {		/*    ETB character, */
				if (!write_char( p_pcb, LO_BYTE( bcc )))
				    return(-1);
				if ( crc )  		/*    write bcc */
				    if (!write_char( p_pcb, HI_BYTE( bcc )))
					return(-1);
				bcc = 0;		/*    clear bcc */
			        if ( c == ITB ) 	/*    if ITB frame, */
				    p_pcb->tx_state = FT_B_ITB;
			}
			break;
		    case ENDXMIT:			/* END OF TRANSMIT */
			EXIT_LOOP;			/*    quit */
		}
	    }
	} else {
	    /*  Transparent transmit state machine; read chars from	*/
	    /*  the source buffer, calculate and insert block check	*/
	    /*  characters with data into destination buffer.  Insert	*/
	    /*  idle characters (DLE SYN) at regular intervals.		*/

	    state = XINITXMIT;
	    lc = read_char(p_pcb);			/* get first char */
	    LOOP 
	    {						/* loop "forever" */
	        c = read_char( p_pcb );			/* get next char */
		if (c == EOB) {				/* End of Buffer */
   		    if (( lc == ETX ) ||		/*    end of text? */
   			( lc == ETB ) ||		/*    end of block? */
   			( lc == ITB )) {		/*    end of ITB? */
			if ( state == XSTXSENT )	/*    escape it: */
   			    if (!write_char( p_pcb, DLE ))
				return(-1);
			bcc = next_bcc( lc, bcc, crc );	/*    factor it in */
			if (!write_char( p_pcb, lc ))	/*    then write it */
			    return(-1);
			if (!write_char( p_pcb, LO_BYTE( bcc )))
			    return(-1);
			if ( crc )  			/*    write bcc */
			    if (!write_char( p_pcb, HI_BYTE( bcc )))
				return(-1);

			if ( lc == ITB ) 		/*    if ITB frame, */
			    p_pcb->tx_state = FT_B_ITB;	/*    mark it */
		    } else {
			if (( lc == ENQ ) && 		/*    Enquiry? */
			    ( state == XSTXSENT ))  	/*    in data block? */
			{
   			    if (!write_char( p_pcb, DLE ))
				return(-1);		/*    then escape it */
			}
			if (!write_char( p_pcb, lc ))	/*    and write it */
			    return(-1);
		    }
   		    state = XMITEND;			/*    then quit */
   		} 					/*    not end of buf */
		switch ( state ) 
		{	 

		    case XINITXMIT:			/* INIT XMIT STATE */

			switch(lc)
			{
			    case SOH:			/*    start header  */
			        bcc = 0;			/*    reset bcc */
			        state = XSOHSENT;		/*    change state. */
				break;

			    case STX:
			        if (!write_char( p_pcb, DLE ))
				    return(-1);
			        bcc = 0;			/*    reset bcc */
			        state = XSTXSENT;		/*    change state. */

			        /* DLE&STX are both used in calulating the bcc 
			         * when prev frame was an ITB frame
			         */
			        if (!(p_pcb->bsc_prev_tx & FT_B_ITB))
				    break;
			        bcc = next_bcc( DLE, bcc, crc );
				bcc = next_bcc( lc, bcc, crc );
				break;

			    default:
			        bcc = next_bcc( lc, bcc, crc );	/*    factor in char */
				break;
			}
				
			if (!write_char( p_pcb, lc ))	/*    write it */
			    return(-1);			
			lc = c;				/*    then shift */
			break;

		    case XSOHSENT:			/* SOH SENT STATE */
			if (lc == STX) 			/*    start of text */
			{
			    if (!write_char( p_pcb, DLE ))
			        return(-1);		/*    write it */
			    bcc = next_bcc( DLE, bcc, crc );
			    state = XSTXSENT;		/*    bcc; ch. state */
			}
			bcc = next_bcc( lc, bcc, crc );	/*    bcc last char */
			if (!write_char( p_pcb, lc ))	/*    write it */
			    return(-1);			
			lc = c;				/*    then shift */
			break;

		    case XSTXSENT:			/* STX SENT STATE */
			if (lc == DLE) 			/*    if DLE char */
			    if (!write_char( p_pcb, DLE ))
			        return(-1);		/*    escape it, */
			if (!write_char( p_pcb, lc ))	/*    then write it */
			    return(-1);			
			bcc = next_bcc( lc, bcc, crc );	/*    new bcc, */
			lc = c;				/*    then shift */
			break;

		    case XMITEND:			/* END OF XMIT */
			EXIT_LOOP;			/*    quit */
		}
		if ((p_pcb->write_chars > insert_count) &&
		    (state != XMITEND))
		{
		    if ( state == XSTXSENT ) {		/* insert idles */
			if (!write_char( p_pcb, DLE ))
			    return(-1);			/* DLE-SYN, or */
		    } else {
			if (!write_char( p_pcb, SYN ))
			    return(-1);			/* SYN-SYN */
		    }
		    if (!write_char( p_pcb, SYN ))
		        return(-1);			
		    total_chars += p_pcb->write_chars;
		    p_pcb->write_chars = 0;		/* accounting */
		}
	    }
	}
	total_chars += p_pcb->write_chars;
	FOOTPRINT( 'b', 's', 'z' );			/* bsz */
	return( total_chars );
}

/*--------------------  B S C _ X M I T _ F R A M E  -------------------*/
/*									*/
/*  NAME: bsc_xmit_frame						*/
/*									*/
/*  FUNCTION:								*/
/*	Bisync transmit frame routine; configures DUSCC and starts	*/
/*	next transmit, then re-configures it after transmit.		*/
/*									*/
/*  EXECUTION ENVIRONMENT:						*/
/*	Runs at off-level.						*/
/*									*/
/*  DATA STRUCTURES:							*/
/*	Accesses and modifies the port control block.			*/
/*  									*/
/*  RETURNS:    							*/
/*	Nothing								*/
/*									*/
/*----------------------------------------------------------------------*/

bsc_xmit_frame ( 
	register volatile t_pcb	*p_pcb )
{
	register unsigned char	port = p_pcb->port_no;
	register t_scc_reg	*p_scc = &p_pcb->scc_reg;
	register ioptr		next_dma,last_dma;
	bufno			*last_buf,*next_buf;
	t_rqe			rqe;

	FOOTPRINT( 'b', 'x', 'a' );			/* bxa */
	if (ODD( p_pcb->txchan ))			/* Odd in use? */
	{
	    last_buf = &p_pcb->txbuf1;
	    next_buf = &p_pcb->txbuf0;
	    next_dma = p_pcb->tx_dma_0;
	    last_dma = p_pcb->tx_dma_1;
	} else {					/* Even is in use */
	    last_buf = &p_pcb->txbuf0;
	    next_buf = &p_pcb->txbuf1;
	    next_dma = p_pcb->tx_dma_1;
	    last_dma = p_pcb->tx_dma_0;
	}

	/* This section prepares the DUSCC and DMA for the next 	*/
	/* transmit.  Toggle the channel by incrementing txchan.	*/

	if ( *next_buf != NO_BUFNO )
	{
	    FOOTPRINT( 'b', 'x', 'n' );			/* bxn */
	    ++p_pcb->txchan;				/* toggle channels */
	    txioa[port] = next_dma;			/* get dma channel */
	    txlgt[port] = txflgt[port];

	    /*  If the last frame ended in an ITB character, the line	*/
	    /*  has to stay available for the "rest" of the frame on	*/
	    /*  the next transmit.  This means that the DUSCC must be	*/
	    /*  idling with SYN's, waiting for this frame. If this 	*/
	    /*  frame is also an ITB frame, leave SYN's enabled, else	*/
	    /*  turn off SYN idling.  Do not re-enable the transmitter,	*/
	    /*  as this garbles the frame -- simply enable DMA.		*/

	    if ( p_pcb->bsc_prev_tx & FT_B_ITB ) 	/* if last was ITB */
	    {
		if (!( p_pcb->tx_state & FT_B_ITB ))	/* and this isn't, */
		    SET_TPR( TPR & ~TP_B_ISYN );	/* turn off SYN's */
		txccw[port] = in16( next_dma ) | PDMA_CCW_EN;
		out16( next_dma, txccw[port] );		/* just enable DMA */

	    /*  If the last frame was a regular non-ITB frame, we 	*/
	    /*  assume line turn-around from receive.  The DUSCC must	*/
	    /*  be put into COP DUAL SYN mode (to work around the 	*/
	    /*  Bisync mode transmit bugs); reset the transmitter and	*/
	    /*  configure it with no frame check sequence (we supply it	*/
	    /*  in the transmit state machine).  If this frame is an	*/
	    /*  ITB frame, turn on SYN idling, else, it should already	*/
	    /*  be turned off.  Issue another reset to cause these 	*/
	    /*  changes to go into effect, then drive the request to 	*/
	    /*  to send line after disabling the receiver.  Enable the	*/
	    /*  transmitter -- if internal clocking is used, start the	*/
	    /*  transmitter with pad characters, else simply issue	*/
	    /*  the start of message command.				*/

	    } else {					/* last was non-ITB */
		WRITE_CCR( RESET_TX );			/* reset transmitter */
		SET_CMR1((CMR1 & 0xD8) | C1_2SYN);	/* enter CDS mode */
		SET_CMR2((CMR2 & 0xF8) | NULL );	/* no frame check seq */
		if ( p_pcb->tx_state & FT_B_ITB )	/* if this is an ITB */
		    SET_TPR( TPR | TP_B_ISYN ); 	/* turn on SYN's */
		else					/* otherwise, */
		    SET_TPR( TPR & ~TP_B_ISYN );	/* turn off SYN's */
		WRITE_CCR( RESET_TX );			/* load changes */
		WRITE_CCR( DISABLE_RX );		/* disable receiver */
		WRITE_CCR( ENABLE_TX );			/* enable transmitter */
		SET_OMR( OMR | OM_RTS_ACTIVE );		/* drive RTS line */
		if ( p_pcb->baud_rate )			/* if internal clock */
		{
		    WRITE_TXFIFO( p_pcb->tx_pad );
		    WRITE_TXFIFO( p_pcb->tx_pad );	/* start msg with pad */
		    WRITE_CCR( CC_TX_SOMP );		/* and start transmit */
		} else					/* otherwise, */
		    WRITE_CCR( CC_TX_SOM );		/* start it w/o pad */

		/*  Start transmit failsafe timeout; minimum timeout is	*/
		/*  30 seconds (300 tenths).  Maximum timeout is equal	*/
		/*  the L/10 seconds where L is the length of the 	*/
		/*  transmitted frame.					*/

		/* Set failsafe timer if in DATA_TRANSFER state */
		if ((p_pcb->port_state != V25_CALL_DIALING) &&
		    (p_pcb->port_state != WAIT_FOR_CONNECT) )
		{
		    p_pcb->timer_type = TX_FS_TIMER;
		    tscc_typ[port] = TX_FS_TIMER;
		    tscc_val[port] = MAX( 300, in16( next_dma + PDMA_TC ));
		    start_duscc_timer( p_pcb, tscc_val[port], tscc_typ[port] );
		}
	    }
	}
	/*  This section cleans up from the last transmit.  For Bisync,	*/
	/*  this means restoring the DUSCC back to Bisync mode if there	*/
	/*  are no more transmits.  Since Bisync is half-duplex, the	*/
	/*  line will turn around if the last transmitted frame was not */
	/*  an ITB.							*/
	
	if (*last_buf != NO_BUFNO)
	{
		tx_ack( p_pcb, last_buf, next_buf );	/* Clean up last tx */
	}
	FOOTPRINT( 'b', 'x', 'z' );			/* bxz */
	return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
/*                      BISYNC RECEIVE PROCESSING			*/
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

/*----------------------  R X _ P O S T _ B S C  -----------------------*/
/*									*/
/*  NAME: rx_post_bsc							*/
/*									*/
/*  FUNCTION:								*/
/*	Bisync receive post processing; checks for errors then scans	*/
/*	the receive data to determine the frame type.			*/
/*									*/
/*  EXECUTION ENVIRONMENT:						*/
/*	Runs at off-level.						*/
/*									*/
/*  DATA STRUCTURES:							*/
/*	Accesses and modifies the port control block.			*/
/*  									*/
/*  RETURNS:    							*/
/*									*/
/*----------------------------------------------------------------------*/

# define DISCARD_FRAME						\
	        queue_writeb( &rx_free, p_pcb->rx_last );	\
	        p_pcb->rx_state = NULL;

rx_post_bsc ( 
	register volatile t_pcb	*p_pcb,
	rx_buf_hdr far		*p_buf )
{
	register unsigned char far	*p_data;
	register unsigned short		i, length;
	register unsigned char		*char_tbl;
	unsigned char far		*ptr;
	unsigned short		bcc, frame_type;
	unsigned char		cc1, cc2, last;
	short			offset, ccoff1, ccoff2;
	unsigned char		dma_stat;
	unsigned char		scc_stat;
	unsigned char		ascii = p_pcb->data_flags & PCB_PF_ASCII;
	unsigned char		a, b, c;
	
	FOOTPRINT( 'b', 'r', 'a' );		/* bra */
						/* setup translation table */
	char_tbl = ascii ? asc_tbl : ebc_tbl;
	dma_stat = p_buf->offset >> 8;		/* get dma status */
	scc_stat = p_buf->type;			/* get receiver status */

	/*------------------  Hardware Work-Around  --------------------*/
	/*  The ECHO chip should save the receiver status when it 	*/
	/*  gets the transfer count; but in some cases it fails to 	*/
	/*  save and clear the PAD error bit, which is set later.	*/
	/*  The work-around is to read the receiver status register	*/
	/*  directly and OR it in with the saved status -- this brings	*/
	/*  in any "tardy" status bits.					*/

	scc_stat |= RSR;

	/*--------------------------------------------------------------*/

	p_pcb->bsc_prev_rx = p_pcb->rx_state;	/* shift state */

	/*  Check for errors; parity error, crc error, overrun error, 	*/
	/*  pad error, or buffer overflow.  If an error is found,     	*/
	/*  return an rqe for that error, discard the frame, then quit.	*/
	/*  Don't check CRC bit if in ASCII mode, as the DUSCC chip	*/
	/*  intermittently signals a bad CRC even if it is good!	*/

	if (( scc_stat & 
	    (( ascii ? RS_C_PARITY : RS_C_CRC ) | RS_C_OVERRUN | RS_C_PAD ))
	    || !( dma_stat & DMA_ISR_EOM ))
	{						/* make error rqe */
	    FOOTPRINT( 'b', 'r', 'B' );			/* brB */
	    if ( !ascii && ( scc_stat & RS_C_CRC ))	/* or if CRC error */
	    {
		FOOTPRINT( 'b', 'r', 'C' );		/* brC */
		make_rqe( p_pcb, FRAME_CRC );
	    }
	    if ( ascii && ( scc_stat & RS_C_PARITY ))	/* if parity error */
	    {
		FOOTPRINT( 'b', 'r', 'P' );		/* brP */
		make_rqe( p_pcb, RX_PARITY_ERR );
	    }
	    if (!( dma_stat & DMA_ISR_EOM ))		/* or buffer overflow */
	    {
		FOOTPRINT( 'b', 'r', 'O' );		/* brO */
		make_rqe( p_pcb, BUF_STAT_OVFLW );	
	    }
	    if ( scc_stat & RS_C_PAD )			/* or if PAD error */
	    {
		FOOTPRINT( 'b', 'r', 'E' );		/* brE */
		make_rqe( p_pcb, BSC_PAD_ERR );
	    }
	    DISCARD_FRAME;				/* discard frame */

	} else {

	    /*  No errors, so we assume it is a good frame.  The length	*/
	    /*  is the maximum frame length minus the number of 	*/
	    /*  unwritten bytes as specified in the length field.  The	*/
	    /*  data starts at the beginning of the buffer plus the	*/
	    /*  port control block receive offset.  If the data is in	*/
	    /*  ASCII, convert from odd parity to ASCII; throw the 	*/
	    /*  frame away if a parity error is discovered.		*/

	    offset = 0;
	    length = p_pcb->rx_flgt - p_buf->length;
	    p_data = (unsigned char far *)p_buf + p_pcb->rx_off;
	    if ( ascii ) {
		if ( Ascii_Fr_Op ( p_data, length )) 
		{				/* if parity error, */
		    make_rqe( p_pcb, RX_PARITY_ERR );
		    FOOTPRINT( 'b', 'r', 'P' );	/* brP */
		    DISCARD_FRAME;		/* discard the frame */
		    return(0);			/* and quit */
		}
		/*---------------  Hardware Work-Around ----------------*/
		/*  In ASCII mode, the LRC has to be verified manually	*/
		/*  since the DUSCC cannot consistently come up with an	*/
		/*  accurate LRC value.                               	*/

		attach_srcbuf ( p_pcb, p_data, length );
		if ((length > 2) && bsc_rsm( p_pcb, (int)!ascii ))	
		{				/* if bad bcc, */
		    make_rqe( p_pcb, FRAME_CRC );
		    FOOTPRINT( 'b', 'r', 'C' );	/* brC */
	            DISCARD_FRAME;		/* discard frame */
		    return(0);			/* and quit */
		}
		/*------------------------------------------------------*/
	    }
	    /*  If multipoint operation is enabled, the first chars of	*/
	    /*  the buffer should contain a poll or select address; if	*/
	    /*  this address doesn't match our's, discard the frame.	*/
	    /*  If the last frame ended in an ITB, don't addr compare.	*/

	    if (( p_pcb->flags & PCB_PF_ADCHK ) &&
		!( p_pcb->bsc_prev_rx & FT_B_ITB ))
		if (!(( *p_data == p_pcb->poll_addr ) ||
		      ( *p_data == p_pcb->select_addr )))
		{
		    DISCARD_FRAME;
		    FOOTPRINT( 'b', 'r', 'A' );	/* brA */
		    return(0);
		}

	    /*  Scan the receive buffer for the first two control	*/
	    /*  chars and the last char.  Save their offsets.		*/

	    for ( ccoff1 = 0; ccoff1 <= length; ccoff1++ ) 
	    {
	        if ( ccoff1 == length ) 	/* no control chars? */
	        {
		    make_rqe( p_pcb, BSC_INV_FRAME );
		    DISCARD_FRAME;		/* then discard frame */
		    FOOTPRINT( 'b', 'r', 'I' );	/* brI */
		    return(0);			/* and quit */
	        } 
		cc1 = *( p_data + ccoff1 );
		if (( cc1 == SOH ) || ( cc1 == STX ) || ( cc1 == ITB ) ||
		    ( cc1 == ETB ) || ( cc1 == ETX ))
		{
		    if ( ascii )		/* if ASCII text block */
			length -= 1;		/* discard FCS */
		    break;
		}
		if (( cc1 == DLE ) || ( cc1 == EOT ) || 
		    ( cc1 == ENQ ) || ( cc1 == NAK )) 
		    break;
	    }
	    ccoff2 = length - 1;		/* default 2nd char is last */
						/* look for 2nd control char */
	    for ( i = ccoff1 + 1; i < length; i++ ) {
		cc2 = *( p_data + i );		/* different from last */
		if ( cc2 == DLE ) 
		{
		    ccoff2 = i;			/* found it */
		    break;
		}
		if ( cc2 == STX ) 
		{				/* if ASCII transparent */
		    if ( ascii && ( cc1 == DLE ))
			length -= 1;		/* discard FCS */
		    ccoff2 = i;			/* found it */
		    break;
		}
	    }
	    cc2  = *( p_data + ccoff2 );	/* second control char */
	    last = *( p_data + length - 1 );	/* last char */
	    
	    /*  Perform frame type determination, adjusting the 	*/
	    /*  receive buffer offset and length as necessary:		*/

	    if ( cc1 == SOH ) {				/* SOH  -   -  */
		offset = ccoff1 + 1;			/* skip SOH char */
		length -= ( offset + 1 );		/* shorten buffer */
		if ( cc2 == DLE ) { 			/* if DLE STX, */
		    for ( ptr = p_data + ccoff2; ptr > p_data; ptr-- )
		        *ptr = *(ptr - 1);		/* copy hdr forward */
		    offset++;				/* adjust buffer */
		    length -= 1;			/* for thrown out DLE */
		}
		if ( last == ETX ) 
			frame_type = MSG_SOH_ETX;	/* SOH  -  ETX */
		else if ( last == ITB )
			frame_type = MSG_SOH_ITB;	/* SOH  -  ITB */
		else if ( last == ETB )
			frame_type = MSG_SOH_ETB;	/* SOH  -  ETB */
		else if ( last == ENQ )
			frame_type = MSG_SOH_ENQ;	/* SOH  -  ENQ */
		else
		{
			frame_type = BSC_INV_FRAME;	/* SOH  -   ?  */
		    	FOOTPRINT( 'b', 'r', 'J' );	/* brJ */
		}

	    } else if ( cc1 == DLE ) {			/* DLE  -   -  */
		if ( cc2 == STX ) {
		    offset = ccoff2 + 1;		/* skip DLE STX */
		    length -= ( offset + 1 );		/* shorten buffer */
		    if ( last == ETX ) 
			frame_type = MSG_STX_ETX;	/* DLE STX ETX */
		    else if ( last == ITB )
			frame_type = MSG_STX_ITB;	/* DLE STX ITB */
		    else if ( last == ETB )
			frame_type = MSG_STX_ETB;	/* DLE STX ETB */
		    else if ( last == ENQ )
			frame_type = MSG_STX_ENQ;	/* DLE STX ENQ */
		    else
		    {
			frame_type = BSC_INV_FRAME;	/* DLE STX  ?  */
		    	FOOTPRINT( 'b', 'r', 'K' );	/* brK */
		    }

		} else if ( cc2 == ACK0 ) {		/* DLE ACK0 -  */
			if ( length > 2 ) {
			    frame_type = MSG_DATA_ACK0;
			    length = length - 2;
			} else 
			    frame_type = MSG_ACK0;
		    } else if ( cc2 == ACK1 ) {		/* DLE ACK1 -  */
			if ( length > 2 ) {
			    frame_type = MSG_DATA_ACK1;
			    length = length - 2;
			} else 
			    frame_type = MSG_ACK1;
		    } else if ( cc2 == EOT )
			frame_type = MSG_DISC;		/* DLE EOT  -  */
		    else if ( cc2 == RVI )
			frame_type = MSG_RVI;		/* DLE RVI  -  */
		    else if ( cc2 == WACK )
			frame_type = MSG_WACK;		/* DLE WACK -  */
		    else
		    {
			frame_type = BSC_INV_FRAME;	/* DLE  ?   -  */
		        FOOTPRINT( 'b', 'r', 'N' );		/* brN */
		    }
	    } else if ( cc1 == STX ) {			/* STX  -   -  */
		offset = ccoff1 + 1;			/* skip STX char */
		length -= ( offset + 1);		/* shorten buffer */
		if ( last == ETX ) 
		    frame_type = MSG_STX_ETX;		/* STX  -  ETX */
		else if ( last == ITB )
		    frame_type = MSG_STX_ITB;		/* STX  -  ITB */
		else if ( last == ETB )
		    frame_type = MSG_STX_ETB;		/* STX  -  ETB */
		else if ( last == ENQ )
		    frame_type = MSG_STX_ENQ;		/* STX  -  ENQ */
		else
		{
		    frame_type = BSC_INV_FRAME;		/* STX  -   ?  */
		    FOOTPRINT( 'b', 'r', 'L' );		/* brL */
		}
	    } else {					/* only one ctl char */
	            					/* in this frame     */
		length--;				/* discard end char  */
		if ( (ccoff1 + 1) < length )		/* ctl char should be*/
							/* last char of frame*/
		{
		    frame_type = BSC_INV_FRAME;		/*  ?   -   -  */
		    FOOTPRINT( 'b', 'r', 'Q' );		/* brQ */
		}
		else if ( cc1 == EOT )
		    frame_type = MSG_EOT;		/* EOT  -   -  */
		else if ( cc1 == ETB ) 			/*  -   -  ETB */
		    frame_type = MSG_ETB | p_pcb->recv_type;
		else if ( cc1 == ETX ) 			/*  -   -  ETX */
		    frame_type = MSG_ETX | p_pcb->recv_type;
		else if ( cc1 == ITB ) 			/*  -   -  ITB */
		    frame_type = p_pcb->recv_type;
		else if ( cc1 == ENQ )
		    frame_type = 			/* ENQ  -   -  */
		        (length > 0) ? MSG_DATA_ENQ : MSG_ENQ;
		else if ( cc1 == NAK )
		    frame_type = 			/* NAK  -   -  */
		        (length > 0) ? MSG_DATA_NAK : MSG_NAK;
		else
		{
		    frame_type = BSC_INV_FRAME;		/*  ?   -   -  */
		    FOOTPRINT( 'b', 'r', 'M' );		/* brM */
		}
	    }
	    /*  Set the length, offset, and type fields of the buffer	*/
	    /*  to the correct values, update statistics, then create	*/
	    /*	an rqe that will cause the message type and the	receive	*/
	    /*	data to be passed up to the driver.			*/

	    p_buf->length = length;		/* enter new length & offset */
	    p_buf->offset = p_pcb->rx_off + offset;
	    p_buf->type   = frame_type;		/* and frame type */
	    p_pcb->rx_bytes += length;		/* update statistics */
	    p_pcb->rx_frames++;
	    make_rqe( p_pcb, frame_type );	/* make rqe for recv'd frame */
	    if (( frame_type == MSG_STX_ITB ) || 
		( frame_type == MSG_SOH_ITB ))  /* if ITB frame; */
	    {					
	        p_pcb->recv_type = frame_type;	/* save ITB typ for next time,*/
						/* restart timer, 	      */
		start_duscc_timer( p_pcb, p_pcb->rx_timeout, RX_TIMER );
		p_pcb->rx_state = FT_B_ITB;	/* and mark as ITB frame      */
	    } else {				/* else;		      */
	        p_pcb->recv_type = 0;		/* frame type not needed for  */
						/* next received frame,       */
		if ( p_pcb->timer_type == RX_TIMER )	
		{
	            stop_duscc_timer( p_pcb );	/* turn off receive timer,    */
		}
		p_pcb->rx_state = NULL;		/* and mark as non-ITB 	      */
	    }
	    a = frame_type >> 12 & 0xf;
	    b = frame_type >> 4 & 0xf;
	    c = frame_type & 0xf;
	    FOOTPRINT( (a <=9 ? (a + 0x30) : (a + 0x37) ),  
		       (b <=9 ? (b + 0x30) : (b + 0x37) ),  
		       (c <=9 ? (c + 0x30) : (c + 0x37) ) );
	}
	return(0);
}

/*---------------------------  B S C _ R S M  --------------------------*/
/*									*/
/*  NAME: bsc_rsm							*/
/*									*/
/*  FUNCTION:								*/
/*	Minimal Bisync receive state machine; both normal and		*/
/*	transparent mode -- this is used only to calculate a block	*/
/*	check character for the received frame.  It is assumed that	*/
/*	all idles and inserted DLE's are removed by the hardware.	*/
/*	This function is used only for the hardware work-around		*/
/*	mentioned above.						*/
/*									*/
/*  EXECUTION ENVIRONMENT:						*/
/*	Can be executed at interrupt or process level.			*/
/*									*/
/*  DATA STRUCTURES:							*/
/*	References and modifies port control block.			*/
/*  									*/
/*  RETURNS:    							*/
/*	0 if block check character matches received FCS.		*/
/*	Non-zero if block check character doesn't compare to FCS.	*/
/*									*/
/*----------------------------------------------------------------------*/

bsc_rsm (
	register volatile t_pcb	*p_pcb,
	int			crc )
{
	register unsigned char	*char_tbl;
	register unsigned short	bcc = 0;	
	register short		c;
	register short		prev_c = 0;
	unsigned short		state;
	unsigned short		loop;
	unsigned short 		read_char(), next_bcc(); 

	char_tbl = crc ? ebc_tbl : asc_tbl;		/* select char set */
	state = INITRECV;
	FOOTPRINT( 'b', 'R', 'a' );			/* bRa */
	LOOP {						/* loop "forever" */
	    c = read_char( p_pcb );			/* get next char */
	    if ( c == EOB )				/* if no more, */
	    {
		EXIT_LOOP;				/* exit loop */
	    }
	    bcc = next_bcc( c, bcc, crc );
	    switch ( state ) {	 
		case INITRECV:
		    if (( c == SOH ) ||  		/*    start header? */
			( c == STX )) {			/*    or text? */
		        state = XSOHSENT;		/*    change state. */
			if ( !(p_pcb->bsc_prev_rx & FT_B_ITB ) )
			    bcc = 0;
			break;
		    }
		    if ( c == DLE ) {			/*    DLE? */
			state = DLESEEN;		/*    change state */
			if ( !(p_pcb->bsc_prev_rx & FT_B_ITB ) )
			    bcc = 0;
		    }
		    break;

		case DLESEEN:
		    if ( c == STX ) {			/*    start text? */
			state = BLKRECV;		/*    change states */
			if ( !(p_pcb->bsc_prev_rx & FT_B_ITB ) )
			    bcc = 0;
		    } else 
			bcc = 0;			/*    not STX */
		    break;

		case BLKRECV:
		    break;
	
		default:
		    break;
	    }
	    prev_c = c;
	}						/* end of loop */
	FOOTPRINT( 'b', 'R', 'z' ); 			/* bRz */
	if (bcc && (prev_c == ENQ) )			/* ENQs don't have BCC*/
	    return (0);
	else
	    return( bcc );		/* return final bcc */
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
/*                      BISYNC UTILITY ROUTINES				*/
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

/*--------------------  SOURCE BUFFER PRIMITIVES  ----------------------*/
/*									*/
/*  FUNCTION:								*/
/*	Returns the next character from the source buffer and		*/
/*	increments the read_chars variable in the pcb.			*/
/*									*/
/*  EXECUTION ENVIRONMENT:						*/
/*	Can be executed at interrupt or process level.			*/
/*									*/
/*  DATA STRUCTURES:							*/
/*	Modifies the source buffer pointer and read_chars pcb fields;	*/
/*	references the contents of the source buffer.			*/
/*  									*/
/*  RETURNS:    							*/
/*	Next character value from source buffer.			*/
/*	EOB is end of buffer character.					*/
/*									*/
/*----------------------------------------------------------------------*/

unsigned short
read_char ( p_pcb ) 
	register volatile t_pcb	*p_pcb;
{
	register char	c;

	if (p_pcb->srcptr <= p_pcb->srclimit) 	/* still in buffer? */
	{
	    p_pcb->read_chars++;		/* then do it; */
	    c = *(p_pcb->srcptr)++;		/* get next character */
	    if ( p_pcb->data_flags & PCB_PF_ASCII )
		c &= 0x7F;			/* strip parity if ascii */
	    return( c );
	} else 					/* else, no more, so */
	    return( EOB );			/* return end-of-buffer */
}

attach_srcbuf ( p_pcb, srcbuf, srclen )		/* attach buffer for read */
	register volatile t_pcb	*p_pcb;
  	unsigned char far	*srcbuf;
  	unsigned int		srclen;
{
	p_pcb->srcptr = srcbuf;
	p_pcb->srclen = srclen;
	p_pcb->srclimit = 
	    (unsigned char far *)(p_pcb->srcptr + (p_pcb->srclen - 1));
}

/*-------------------  DESTINATION BUFFER PRIMITIVES  ------------------*/
/*									*/
/*  FUNCTION:								*/
/*	Writes a character to the next position in the destination	*/
/*	buffer and increments the write_chars variable in the pcb.	*/
/*									*/
/*  EXECUTION ENVIRONMENT:						*/
/*	Can be executed at interrupt or process level.			*/
/*									*/
/*  DATA STRUCTURES:							*/
/*	Modifies the destination buffer pointer and write_chars 	*/
/*	fields of the pcb; modifies the contents of the destination	*/
/*	buffer.								*/
/*  									*/
/*  RETURNS:    							*/
/*	TRUE - the write was executed.					*/
/*	FALSE - no more destination buffer space.			*/
/*									*/
/*----------------------------------------------------------------------*/

int
write_char ( p_pcb, c )
	register volatile t_pcb	*p_pcb;
	char	c;
{
	if (p_pcb->destptr <= p_pcb->destlimit) /* still in buffer? */
	{
	    p_pcb->write_chars++;		/* then do it; */
	    *(p_pcb->destptr)++ = c;		/* write it there */
	    return( TRUE );			/* the write was done */
	} else 					/* else, out of room */
	    return( FALSE );			/* return error */
}
		
attach_destbuf ( p_pcb, destbuf, destlen )	/* attach buffer for write */
	register volatile t_pcb	*p_pcb;
  	unsigned char far	*destbuf;
  	unsigned int		destlen;
{
	p_pcb->destptr = destbuf;
	p_pcb->destlen = destlen;
	p_pcb->destlimit = 
	    (unsigned char far *)(p_pcb->destptr + (p_pcb->destlen - 1));
}

/*------------------------  M A K E _ R Q E  ---------------------------*/
/*									*/
/*  FUNCTION:								*/
/*	Creates a response queue element of TYPE.			*/
/*									*/
/*  EXECUTION ENVIRONMENT:						*/
/*	Can be executed at interrupt or process level.			*/
/*									*/
/*  DATA STRUCTURES:							*/
/*	References the port control block.				*/
/*  									*/
/*  RETURNS:    							*/
/*	Nothing								*/
/*									*/
/*----------------------------------------------------------------------*/

make_rqe ( p_pcb, type )
	register volatile t_pcb	*p_pcb;
	unsigned short	type;
{
	t_rqe	rqe;

	rqe.f.sequence = type;			/* type of message */
	rqe.f.port     = p_pcb->port_no;
	rqe.f.status   = p_pcb->rx_last;

	/* Normally, we would not set RX_DMA for non-data message	*/
	/* types (like ACK's, DISC's, etc), but the RX_NODMA path	*/
	/* is hosed -- after about 46 receives, the adapter winds up	*/
	/* up in never-never land (takes cycle power to reset!). 	*/
	/* Instead, pass as RX_DMA then free the mbuf in the driver.	*/

	if (( type == BSC_INV_FRAME ) || ( type == FRAME_CRC )      ||
	    ( type == RX_PARITY_ERR ) || ( type == BUF_STAT_OVFLW ) ||
	    ( type == BSC_PAD_ERR   ))
	{
	    FOOTPRINT( 'b', 'm', 'n' );		/* bmn */
	    rqe.f.rtype = RQE_RX_NODMA;		/* error, no DMA */
	    ReRoute[ p_pcb->port_no ] = FALSE;	/* adapter response q */
	} else {
	    FOOTPRINT( 'b', 'm', 'd' );		/* bmd */
	    rqe.f.rtype = RQE_RX_DMA;		/* if DMA to be done */
	    ReRoute[ p_pcb->port_no ] = TRUE;	/* port response q */
	}
	add_rqe( p_pcb, rqe.val );		/* issue the rqe */
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
/*                      BISYNC STATUS PROCESSING			*/
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

/*
   cop_decode (BiSync, MonoSync, etc.) serves Character Oriented Protocol
   errors from the SCC and allows the common line change code to execute
   when necessary.
*/

void cop_decode ( volatile t_pcb	*p_pcb )
{
	register eblock			*p_eblk = &( p_pcb->estat );
	register unsigned short		q_temp;
	t_rqe				*p_rqe = &( p_pcb->error_rqe );

	FOOTPRINT( 'b', 'c', 'a' );			/* bca */
	p_rqe->f.status = 0;
	p_rqe->f.sequence = 0;
	if ( p_eblk->e_type & ETYP_RXTX_STAT )
	{
		FOOTPRINT( 'b', 'c', 't' );		/* bct */
		p_rqe->f.rtype = RQE_RX_NODMA;
/*
  Check both the Rx and Tx/Rx Status Register, as the interrupt is multiplexed.
*/
		q_temp = p_eblk->e_rsr;		/* Receiver Status Register */
		if ( q_temp & RS_C_OVERRUN )
		{
			FOOTPRINT( 'b', 'c', 'O' );	/* bcO */
			p_rqe->f.sequence = RX_OVERRUN;
		}
		else if ( q_temp & RS_C_PAD )
		{
			FOOTPRINT( 'b', 'c', 'P' );	/* bcP */
			p_rqe->f.sequence = BSC_PAD_ERR;
		}
		else if ( q_temp & RS_C_CRC )
		{
			FOOTPRINT( 'b', 'c', 'R' );	/* bcR */
			p_rqe->f.sequence = FRAME_CRC;
		}
		else if ( q_temp & RS_C_PARITY )
		{
			FOOTPRINT( 'b', 'c', 'T' );	/* bcT */
			p_rqe->f.sequence = RX_PARITY_ERR;
		}

		q_temp = p_eblk->e_trsr;	/* Tx/Rx Status Register */
		if ( q_temp & TRS_TXEMPTY )	/* DMA underran */
		{
			FOOTPRINT( 'b', 'c', 'U' );	/* bcU */
			tx_abort( p_pcb, TX_UNDERRUN );
		}
		else if ( q_temp & TRS_CTSUNDER )	/* CTS underran */
		{
			FOOTPRINT( 'b', 'c', 'C' );	/* bcC */
			tx_abort( p_pcb, CTS_UNDERRUN );
		}
		else if ( q_temp & TRS_DPLL )	/* Phase Locked Loop: !syn */
		{
			FOOTPRINT( 'b', 'c', 'D' );	/* bcD */
			p_rqe->f.sequence = LOST_SYNC;
		}

		if ( p_rqe->f.sequence != 0 )		/* post a rqe? */
		{
			disable_rx( p_pcb );		/* disable receive */
			add_rqe ( p_pcb, p_rqe->val );	/* post rqe to driver */
		}
	}

	if ( p_eblk->e_type & ETYP_ECT_STAT )	/* ECT_STAT eblocks */
	{
		FOOTPRINT( 'b', 'c', 'E' );		/* bcE */
		scc_delta( p_pcb, p_eblk->e_ictsr );
	}					/* ECT_STAT eblocks */

	if ( p_eblk->e_type & ETYP_CIO_STAT )	/* CIO STAT eblocks */
	{
		FOOTPRINT( 'b', 'c', 'S' );		/* bcS */
		cio_delta( p_pcb, p_eblk->e_ciodata );
	}					/* CIO STAT eblocks */

	FOOTPRINT( 'b', 'c', 'z' );			/* bcz */
	return;
}
