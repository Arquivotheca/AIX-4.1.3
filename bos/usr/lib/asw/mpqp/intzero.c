static char sccsid[] = "@(#)30	1.12  src/bos/usr/lib/asw/mpqp/intzero.c, ucodmpqp, bos411, 9428A410j 10/28/93 10:05:20";
/*--------------------------------------------------------------------------
*
*				  INTZERO.C
*
*  COMPONENT_NAME:  (UCODEMPQ) Multiprotocol Quad Port Adapter Software.
*
*  FUNCTIONS:	host_command, f_dmawork, dma_complete
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
#include "mpqp.pro"	
#include "mpqp.h"
#include "iface.def"
#include "intzero.h"
#include "portcb.def"
#include "portcb.typ"
 
extern ulong_q			rx_dma_q;	/* recv buffer pool */
extern volatile bm_dma_ccb	Bm_Dma;		/* Bus Master Dma cmd block */
extern unsigned short		Dma_Port;	/* BM DMA fairness storage */
extern unsigned char		Dma_BM_Index;	/* Index to Bus Mast DMA addrs*/
extern unsigned short		Recv_Enable;	/* Recv enable/disable */
extern unsigned short		Dma_Retries;	/* BM DMA retry count */



/*---------------------  H O S T _ C O M M A N D  ----------------------*/
/*									*/
/*  NAME: host_command							*/
/*									*/
/*  FUNCTION:								*/
/*	Dispatches host commands as read from the adapter command	*/
/*	queue.  The host places a command block on "arqueue" then	*/
/*	interrupts the adapter; the INT0 handler then calls this	*/
/*	routine to process the command(s).  				*/
/*									*/
/*  EXECUTION ENVIRONMENT:						*/
/*	Runs at interrupt level.					*/
/*									*/
/*  DATA STRUCTURES:							*/
/*	Modifies adapter command queue, rx_dma_q, and tx_dma_q,		*/
/*	tx_free queue, port command queues, and Dma_Count.		*/
/*  									*/
/*  RETURNS:  Nothing   						*/
/*									*/
/*----------------------------------------------------------------------*/

host_command ()
{
	register unsigned short	cb;		/* New command block number */
	register cmd_t		*p_cmd;		/* Ptr to new command block */
	register unsigned char	port;		/* Port No. from CmdBlk */
	t_dma_op		dma_op;		/* DMA operation */
	register int		i;
	long			host_addr;	/* host dma address | index */
 
	/*  A command block was placed on the adapter command queue;	*/
	/*  read and process commands from this queue until it is 	*/
	/*  empty.							*/

	while (( cb = queue_readb( &acqueue )) != (unsigned char)-1 )
	{
	    p_cmd = (cmd_t *)&cmdblk[ cb ];	/* Get command block */
	    port  = p_cmd->port;		/* and port number */
	    switch ( p_cmd->type ) {		/* Select command: */

	        case RX_BUF_IND:		/* RX BUFFER INDICATE */
	            FOOTPRINT('I', 'i', 'r');	/* Iir */
		    disable();

		    /* Return the command block back to the free list.	*/
		    /* The receive buffer is a TCW-mapped real memory	*/
		    /* page in host memory; this address is placed on	*/
		    /* an internal queue for future use.              	*/
		    /* Put the index to the host receive buffer address */
		    /* in the low order byte since this address is 	*/
		    /* always on a 1K boundary.  The driver gets the    */
		    /* address of the DMA'd data using this index.	*/

		    queue_writeb( &tx_free,  cb );	
		    host_addr = p_cmd->host_addr | (uchar)p_cmd->sequence;
	            queue_writel( &rx_dma_q, host_addr );
		    enable();
		    continue;
 
	        case TX_LONG:			/* TRANSMIT COMMAND */
	            FOOTPRINT('I', 'i', 't');	/* Iit */
		    disable();

		    /* This will done by the host once we make transmit	*/
		    /* blocks allocatable (not tied to cmd blocks).	*/

	    	    p_cmd->card_addr = 
			XMIT_BUFFER((cmd_blk *)p_cmd - &cmdblk[0]);

		    /* For transmit commands, a lock is placed on the	*/
		    /* port command queue associated with this transmit	*/
		    /* until the transmit data is DMA'ed to adapter	*/
		    /* memory.  If the queue was empty, port command	*/
		    /* work is scheduled.  DMA is initiated by placing	*/
		    /* a TX DMA element on the TX DMA queue, which is	*/
		    /* processed by f_dmawork.	 Bus Master DMA work	*/
		    /* is then scheduled to activate f_dmawork.		*/ 

		    if ( queue_writeb( &port_cq[port], PCQ_LOCK ) == 2 )
		        SCHED_PCQ_WORK( &pcb[ port ] );
 
		    dma_op.tx.offset = ( port_cq[ port] ).in;
		    dma_op.tx.cmdblk = cb;
		    queue_writew( &tx_dma_q, dma_op.op );

		    work_q[ XMIT_DMA_WORK ] = B_TRUE;
		    Dma_Count += 1;
		    enable();
		    continue;

		case FLUSH_PORT:		/* FLUSH PORT COMMAND */

		    /* Simply call the flush port routine: do not	*/
		    /* schedule port work.				*/

		    FOOTPRINT('I', 'i', 'f');	/* Iif */
		    flush_p( cb );
		    continue;

		case START_RECV:		/* START FRAME RECEPTION */

		    FOOTPRINT('I', 'i', 's');	/* Iis */
		    Recv_Enable = TRUE;
		    queue_writeb( &tx_free, cb );
		    continue;

		case HALT_RECV:			/* HALT FRAME RECEPTION */

		    FOOTPRINT('I', 'i', 'h');	/* Iih */
		    Recv_Enable = FALSE;
		    queue_writeb( &tx_free, cb );

		    /* Initialize queues associated with receive:	*/

		    queue_init( &port_rq[ 0 ], 48 );
		    queue_init( &port_rq[ 1 ], 48 );
		    queue_init( &port_rq[ 2 ], 48 );
		    queue_init( &port_rq[ 3 ], 48 );
		    queue_init( &rx_dma_q, 17 );
		    queue_init( &rx_free,  49 );
		    for ( i = 0; i < 49; i++ )
			queue_writeb( &rx_free, (unsigned char)i );
		    continue;

		default:			/* ALL OTHER COMMANDS */

		    /* If the command type is greater than D0, it is	*/
		    /* an adapter command (diagnostic), execute it 	*/
		    /* immediately (at interrupt level).		*/

	    	    if (( p_cmd->type & 0xF0 ) >= 0xD0 )	
	    	    {
			FOOTPRINT('I', 'i', 'v');	/* Iiv */
			acmdvec( cb );
			queue_writeb( &tx_free, cb );

		    /* Otherwise, place the command on the command	*/
		    /* queue associated with that port.  If it was 	*/
		    /* empty, schedule port command work.		*/

	    	    } else {
		        FOOTPRINT('I', 'i', 'p');	/* Iip */
		        if ( queue_writeb( &port_cq[port], cb ) == 2 )
			    SCHED_PCQ_WORK( &pcb[ port ] );
		    }
		    continue;
	    }
	}
}
 
/*--------------------  R E C V _ D M A _ W O R K  ---------------------*/
/*									*/
/*  NAME: recv_dma_work							*/
/*									*/
/*  FUNCTION:								*/
/*	Initiates Bus Master DMA transfers for receive frame		*/
/*	data copied to host memory.  Sets up global variables		*/
/*	Dma_Oper and Dma_Type to communicate the nature of the DMA	*/
/*	operation to the interrupt handler dma_complete, which handles	*/
/*	completion of the Bus Master DMA activity initiated here.	*/
/*									*/
/*  EXECUTION ENVIRONMENT:						*/
/*	Runs at off-level.						*/
/*									*/
/*  DATA STRUCTURES:							*/
/*	Modifies port response queues, rx_dma_q, work_q,		*/
/*	Dma_Oper, Dma_Type, and ReRoute.				*/
/*  									*/
/*  RETURNS:  Nothing   						*/
/*									*/
/*----------------------------------------------------------------------*/

recv_dma_work ( int		ignore )
{
	register int		i, port;
	long			host_addr;
	long			rqe;
	unsigned char far	*p_buf;
	rx_buf_hdr far		*p_rxhdr;	/* Rx Buffer Header */
	cmd_t			*p_cmd;		/* Ptr to command block */

	FOOTPRINT('D', 'r', 'a');		/* Dra */
	if ( Bm_Dma.running )			/* Last DMA still busy? */
	{
	    FOOTPRINT( 'D', 'r', 'B' );		/* DrB */
	    if ( ++Dma_Retries >= MAX_BM_DMA )	/* retry too many times? */
		Bm_Dma.running = FALSE;		/* then abort last DMA */
	    return;
	} else {				/* otherwise, */
	    work_q[ RECV_DMA_WORK ] = B_FALSE;	/* deschedule recv DMA */
	    Dma_Retries = 0;			/* and start DMA: */
 
	    /*  Scan all four ports for receive DMA work -- always	*/
	    /*  start after the last handled receive port to maintain 	*/
	    /*  DMA fairness.						*/

	    for ( i = 0, port = ++Dma_Port % NUM_PORT;
	          i < NUM_PORT;
	          i++,   port = ++port % NUM_PORT )
	    {
	        /*  Read the port response queue -- if no rqe, then 	*/
	        /*  there is no receive DMA work for this port.		*/

	        if ( !ReRoute[ port ] )
	            continue;
	        if (( rqe = queue_readl( &port_rq[port] )) < 0 )
	        {
	            ReRoute[port] = FALSE;
	            continue;
	        }
		/*  If receive is disabled, put the receive buffer back	*/
		/*  on the receive list and abort the receive DMA.	*/

	        Dma_Oper.rx.val = rqe;
		if (! Recv_Enable )	/* throw away if recv disabled */
		{
	    	    queue_writeb( &rx_free, Dma_Oper.rx.f.status );
		    continue;
		}
	        Dma_Type = port | RECV_DMA;	/* Mark as recv */
 
	        p_rxhdr = (rx_buf_hdr far *)RECV_BUFFER( Dma_Oper.rx.f.status );

		/* If there are no receive TCWs available, insert the RQE at */
		/* the beginning of the PRQ to try again later.	  	     */
	        if (( host_addr = queue_readl( &rx_dma_q )) < 0 )
	        {
	            FOOTPRINT('D', 'r', 'D');	/* DrD */
	            queue_insertl( &port_rq[port], rqe );
	            work_q[ RECV_DMA_WORK ] = B_TRUE;
	            return;			/* reschedule and return */
	        }

	        /* Initialize CCW, transfer one block, stop and interrupt */

	        disable();
		Dma_BM_Index = (unsigned char) (host_addr & 0xff );
		host_addr &= 0xffffff00;	/* remove index */
	        Dma_Port = port;		/* save for next time */
	        Bm_Dma.cc_reg    = BDCCW_START    | BDCCW_TCINTR   | 
				   BDCCW_HOSTINCR | BDCCW_CARDINCR | 
				   BDCCW_MEMORY;
	        Bm_Dma.t_count   = p_rxhdr->length + p_rxhdr->offset;
	        Bm_Dma.card_addr = (unsigned char far *)p_rxhdr;
	        Bm_Dma.host_addr = host_addr;	/* host address */
	        Bm_Dma.running = TRUE;		/* Set lock */
	        FOOTPRINT('D', 'r', 'f');	/* Drf */
	        start_bm_dma( &Bm_Dma );	/* Start DMA */
	        enable();
	        break;
	    }
	}
    	return;
}

/*--------------------  X M I T _ D M A _ W O R K  ---------------------*/
/*									*/
/*  NAME: xmit_dma_work							*/
/*									*/
/*  FUNCTION:								*/
/*	Initiates Bus Master DMA transfers for transmit frame		*/
/*	data copied from host memory.  Sets up global variables		*/
/*	Dma_Oper and Dma_Type to communicate the nature of the DMA	*/
/*	operation to the interrupt handler dma_complete, which handles	*/
/*	completion of the Bus Master DMA activity initiated here.	*/
/*									*/
/*  EXECUTION ENVIRONMENT:						*/
/*	Runs at off-level.						*/
/*									*/
/*  DATA STRUCTURES:							*/
/*	Modifies port response queues, tx_dma_q, work_q,		*/
/*	Dma_Oper, Dma_Type, and ReRoute.				*/
/*  									*/
/*  RETURNS:  Nothing   						*/
/*									*/
/*----------------------------------------------------------------------*/

xmit_dma_work ( int		ignore )
{
	register int		i, port;
	long			host_addr;
	long			rqe;
	unsigned char far	*p_buf;
	cmd_t			*p_cmd;		/* Ptr to command block */

	FOOTPRINT('D', 't', 'a');		/* Dta */
	if ( Bm_Dma.running )			/* Last DMA still busy? */
	{
	    FOOTPRINT( 'D', 't', 'B' );		/* DtB */
	    if ( ++Dma_Retries >= MAX_BM_DMA )	/* retry too many times? */
		Bm_Dma.running = FALSE;		/* then abort last DMA */
	    return;
	} else {				/* otherwise, */
	    work_q[ XMIT_DMA_WORK ] = B_FALSE;	/* deschedule xmit work */
	    Dma_Retries = 0;
	    if ( !Q_EMPTY( &tx_dma_q ))		/* something on the queue? */
	    {					
	        /* If anything is on the TX_DMA_Q, get it's cmd block	*/
	        /* and transmit buffer, then set up transmit DMA.	*/

		disable();
	        Dma_Oper.op = queue_readw( &tx_dma_q );
	        if ( Dma_Oper.op != (ushort)-1 )
	        {
	            p_cmd = (cmd_t *)&cmdblk[ Dma_Oper.tx.cmdblk ];
	            p_buf = XMIT_BUFFER( Dma_Oper.tx.cmdblk );
	            Dma_Type = p_cmd->port;

		    /* Initialize CCW, transfer one block, stop and interrupt */

		    Bm_Dma.cc_reg    = BDCCW_START    | BDCCW_FROMHOST | 
				       BDCCW_TCINTR   | BDCCW_HOSTINCR | 
				       BDCCW_CARDINCR | BDCCW_MEMORY;
		    Bm_Dma.t_count   = p_cmd->length;
		    Bm_Dma.card_addr = p_buf;	
		    Bm_Dma.host_addr = p_cmd->host_addr;
	            Bm_Dma.running = TRUE;		/* Set lock */
    	            FOOTPRINT('D', 't', 't');		/* Dtt */
	            start_bm_dma( &Bm_Dma );		/* Start DMA */
	        }
		enable();
	    }
	}
    	FOOTPRINT('D', 't', 'z');			/* Dtz */
    	return;
}

/*----------------------  D M A _ C O M P L E T E  ---------------------*/
/*									*/
/*  NAME: dma_complete							*/
/*									*/
/*  FUNCTION:								*/
/*	Bus Master DMA terminal count interrupt handler.  This 		*/
/*	routine is called by a low-level (assembly) interrupt handler	*/
/*	when Bus Master DMA has completed -- the "Dma_Type" global	*/
/*	is checked to determine whether receive or transmit DMA has	*/
/*	finished.  During receive DMA, "Dma_Oper" contains the		*/
/*	associated response queue element.  This element is forwarded	*/
/*	to the host (with interruption) and the receive buffer is put	*/
/*	back on the free list.  For transmit completion, a DMA ack is 	*/
/*	returned (if requested) and port command work is scheduled	*/
/*	to process the transmit data.					*/
/*									*/
/*  EXECUTION ENVIRONMENT:						*/
/*	Runs at interrupt level.					*/
/*									*/
/*  DATA STRUCTURES:							*/
/*	Modifies port response and command queues, arqueue, rx_free, 	*/
/*	and work_q.							*/
/*  									*/
/*  RETURNS:  Nothing   						*/
/*									*/
/*----------------------------------------------------------------------*/

dma_complete ()
{
	byte_q		*p_pcq;			/* Pointer to PCQ */

	FOOTPRINT( 'D', 'b', 'a' );		/* Dba */
	Bm_Dma.running   = FALSE;		/* Release lock */
	Bm_Dma.cc_reg    = NULL;		/* clear block */
	Bm_Dma.t_count   = 0;
	Bm_Dma.card_addr = NULL;
	Bm_Dma.host_addr = NULL;
 
	if ( Dma_Type & RECV_DMA )		/* Receive DMA Terminated */
	{
	    Dma_Type &= ~RECV_DMA;		/* turn off bit, get port # */

	    /*  Move the RQE (saved in Dma_Oper) to the adapter		*/
	    /*  response queue to let the host know that a receive	*/
	    /*  buffer is available; interrupt the host.  Place the	*/
	    /*  receive buffer back on the free queue then schedule	*/
	    /*  port response queue work to unlock this port's 		*/
	    /*  response queue.						*/
 
	    FOOTPRINT( 'D', 'b', 'r' );		/* Dbr */
	    queue_writeb( &rx_free, Dma_Oper.rx.f.status );
	    Dma_Oper.rx.f.status = Dma_BM_Index;
	    if ( queue_writel( &arqueue, Dma_Oper.rx.val ) < 0 )
	        FOOTPRINT( 'D', 'b', 'F' );	/* DbF */
	    else
	    	host_intr( 0 );
	    SCHED_PRQ_WORK( &pcb[ Dma_Type ] );

	} else {				/* Transmit DMA terminated */

	    FOOTPRINT( 'D', 'b', 't' );		/* Dbt */

	    /*  Remove the lock element in the port command queue	*/
	    /*  by writing over it with this command block number.	*/
	    /*  Schedule port command work processing to unlock the	*/
	    /*  port response queue.					*/

	    p_pcq = &port_cq [ Dma_Type ];
	    p_pcq->q[ Dma_Oper.tx.offset ] = Dma_Oper.tx.cmdblk;
	    SCHED_PCQ_WORK( &pcb[ Dma_Type ] );
	}
	Dma_Oper.op = NULL;
	Dma_Type    = NULL;
	Dma_Count -= 1;				/* decrement DMA count */
	if ( Dma_Count )			/* reschedule if more */
	{
	    if ( Q_EMPTY( &tx_dma_q ))		/* who requires it? */
		work_q[ RECV_DMA_WORK ] = B_TRUE;
	    else
		work_q[ XMIT_DMA_WORK ] = B_TRUE;
	}
	FOOTPRINT( 'D', 'b', 'z' );		/* Dbz */
	return;
}
