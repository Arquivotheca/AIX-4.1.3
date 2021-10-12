static char sccsid[] = "@(#)35	1.5  src/bos/usr/lib/asw/mpqp/portwork.c, ucodmpqp, bos411, 9428A410j 8/23/93 13:23:34";

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: See description below.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
   This module answers the scheduler level PCQ_WORK, port command queue work,
   for every port.

   f_pcqwork : Module entry point from scheduler for level PCQ_WORK

   Input  : port, unsigned character port number
   Output : nothing significant, always returns Zero

   caveats: extreme caution is urged.  There may be a timing window opened
		by the interface between this module and the txbuf fields
		inside the port control block.  We run offlevel, and after
		we have decided not reschedule ourselves, numerous instruc-
		tions execute before we're committed to the change.
*/

#include "mpqp.def"
#include "mpqp.typ"
#include "mpqp.ext"
#include "mpqp.pro"
#include "portcb.typ"
#include "iface.def"

extern int		( *f_portwork [] )();

# define MAJOR( x )	((( x ) >> 4) & 0xF)
# define MINOR( x )	(( x ) & 0xF )

/* Pointer to the address of the pseudo return function for wakeup,
   the scheduler return address */

f_pcqwork ( unsigned char   port )
{
	register volatile t_pcb		*p_pcb;
	unsigned char			q_cmd;
	t_rqe				rqe;
	int				q_lock = ~PCQ_LOCK;
	cmd_t				*p_cmd;

	FOOTPRINT( 'P', 'p', 'a' );		/* Ppa */
	p_pcb = &pcb [port];

	work_q [PCQ_WORK] &= p_pcb->mask_off;

	/* Just check to see what is on the queue first.  If it is PCQ_LOCK */
	/* then it is not necessary to put it back on the queue.  If it is  */
	/* anything else, then read it off the queue for real.	This will   */
	/* prevent doing unnecessary queue_insertb's.			    */
	q_cmd = queue_previewb( p_pcb->cmd_q );
	if ( q_cmd == (unsigned char)0xFF )
	{
	    /* Port Command Queue is empty */
	    FOOTPRINT( 'P', 'p', 'B' );		/* PpB */
	    return;
	}

	/*  If the extracted port command block number equals PCQ_LOCK,	*/
	/*  somebody is preventing us from processing any further down	*/
	/*  this list.  						*/

	if ( q_cmd == PCQ_LOCK )
	{
	    FOOTPRINT( 'P', 'p', 'C' );    	/* PpC */
	    return ( 0 );
	}

	q_cmd = queue_readb( p_pcb->cmd_q );

	/* Vector to the service routine by looking at the major and	*/
	/* minor numbers encoded in the command type.			*/

	p_cmd = (cmd_t *)&cmdblk [q_cmd];
	switch ( MAJOR( p_cmd->type )) {	/* select on major number */

	    case TX_OR_RXBUFIND:		/* TX or RX_BUF_IND Command */
		if ( MINOR( p_cmd->type != RX_BUF_IND )) {
		    FOOTPRINT('P', 'p', 'd');	/* Ppd */
		    q_lock = txCommon((cmd_t *)p_cmd, q_cmd);
		} else {
		    /* Error, the code should never go through this path -  */
		    /* somehow a receive buffer indicate command made it to */
		    /* the port command queue; it should always be caught   */
		    /* when it is on the adapter command queue.  For now,   */
		    /* just log an error drop it.  Queue_unread for the     */
		    /* pcb->cmdq was replaced by queue_insertb to fix this  */
		    /* problem.  					    */
		    rqe.f.port = port;
		    rqe.f.rtype = RQE_STAT_UNSOL;
		    rqe.f.status = q_cmd;
		    rqe.f.sequence = LOST_COMMAND;
		    if ( queue_writel( &arqueue, rqe.val ) < 0 )
		    	    FOOTPRINT('P','p','I');	/* PpI */
                        else {
		    	    FOOTPRINT('P','p','H');	/* PpH */
                            host_intr( 0 );
                        }
		}
		break;

	    case NON_TX_CMD:				/* Non-TX command */
		FOOTPRINT('P', 'p', 'e');	/* Ppe */
		if ( p_cmd->type != STOP_PORT )
		{
		    disable();
		    if (( p_pcb->txbuf0 != NO_BUFNO ) ||
			( p_pcb->txbuf1 != NO_BUFNO ))
		    {
			FOOTPRINT('P', 'p', 'f');	/* Ppf */
			queue_insertb( p_pcb->cmd_q, q_cmd );
			enable();
			return(0);
		    }
		    enable();
		}
		q_lock = ( *f_portwork[ MINOR( p_cmd->type ) ] )
					(p_cmd, XMIT_BUFFER( q_cmd ));
		break;

	    case 0x0B:				/* port-related diags */
	    case 0x0C:
		FOOTPRINT('P', 'p', 'g');	/* Ppg */
		acmdvec( q_cmd );
		queue_writeb( &tx_free, q_cmd );
		break;

	    default:				/* unsupported! */
		FOOTPRINT('P', 'p', 'E');	/* PpE */
		return(0);
	}

	/* Reschedule PCQ_WORK if a transmit is not currently in progress */
	/* and there are more commands on the port command queue.	  */
	if ( q_lock != PCQ_LOCK )	
		if ( !Q_EMPTY( p_pcb->cmd_q ) )
			work_q [PCQ_WORK] |= p_pcb->mask_on;

	FOOTPRINT( 'P', 'p', 'z' );		/* Ppz */
	return ( 0 );
}
