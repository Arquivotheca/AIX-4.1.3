static char sccsid[] = "@(#)80	1.63  src/bos/kernext/mpqp/mpqoffl.c, sysxmpqp, bos41J, 9519A_all 5/4/95 11:27:59";

/*
 * COMPONENT_NAME: (MPQP) Multiprotocol Quad Port Device Driver
 *
 * FUNCTIONS: mpqoffl
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

/**********************************************************************
 *  Includes							      *
 **********************************************************************/

#include <errno.h>
#include <sys/types.h>
#include <sys/adspace.h>
#include <sys/comio.h>
#include <sys/device.h>
#include <sys/dma.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/intr.h>
#include <sys/malloc.h>
#include <sys/mpqp.h>
#include <sys/mpqpdd.h>
#ifdef _POWER_MPQP
#include "mpqpxdec.h"
#endif /* _POWER_MPQP */
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/xmem.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>

/*******************************************************************
 *    External Function Declarations                               *
 *******************************************************************/
extern void mpqtimer(t_mpqp_dds *);

extern int		que_command(t_acb *, 
				    adap_cmd_t *,
				    unsigned char *,
				    unsigned int,
				    unsigned int );

extern void increment_stats ( unsigned int *, unsigned int *, unsigned short );

/*******************************************************************
 *    Internal Declarations                               	   *
 *******************************************************************/


/* enque a receive response from adapter */

static void	que_recv( t_mpqp_dds	*,
			  unsigned int,
			  struct mbuf	*);

/* enque a status response from adapter */

void		que_stat( t_mpqp_dds *,
		          unsigned int,
		          unsigned int,
		          unsigned int,
		          unsigned int,
		          unsigned int,
		          unsigned int);

/* enque a status reponse when stat. or receive queues are full */

static void 	que_lost_stat( t_mpqp_dds *,
			  unsigned int,
			  unsigned int);

/* process completed command	*/

static void	proc_cmd_cmplt( t_acb		*,
			        t_mpqp_dds	*,
			        unsigned int,
			        unsigned int );

/* process solicited response */

static void 	proc_sol_resp( t_acb	*,
			       t_mpqp_dds *,
			       unsigned int );

/* process command failure (nack) */

static void	proc_cmd_fail( t_acb	*,
			       t_mpqp_dds *,
			       unsigned int,
			       unsigned int );

/* process unsolicited port status */

static void	proc_unsol_stat( t_acb	*,
			         t_mpqp_dds *,
				 unsigned short,
			         unsigned int );

/* process adapter error */

static void	proc_adap_err( t_acb	*,
			       t_mpqp_dds *,
			       unsigned int );

/* process diagnostic error */

static void	proc_diag_err( t_acb	*,
			       t_mpqp_dds *,
			       unsigned int );

/* examine threshold values */

int chk_threshold ( 	t_mpqp_dds   *,
			unsigned short,
			unsigned int );

/* Wait on halt completion timeout function */
void halt_compl_timer ( t_mpqp_dds * );

/*******************************************************************
 *    Internal Declarations                               	   *
 *******************************************************************/

/*
 * NAME: mpqoffl
 *                                                                    
 * FUNCTION: High level description of what the procedure does
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	Environment-Specific aspects, such as - 
 *	Preemptable        : Maybe
 *	VMM Critical Region: Yes
 *	Runs on Fixed Stack: Yes
 *	May Page Fault     : Yes
 *      May Backtrack      : Yes
 *                                                                   
 * NOTES: More detailed description of the function, down to
 *	    what bits / data structures, etc it manipulates. 
 *
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.
 *
 * RETURN VALUE DESCRIPTION: NONE
 */  

mpqoffl (
    t_offl_intr *p_intr_ds)
{
    volatile t_acb		*p_acb;        /* interrupting adapter's acb */
    t_mpqp_dds			*p_dds;	       /* dds pointer */
    unsigned int		resp_elem;     /* response queue element */
    unsigned int		rqe_cmd;       /* RQE command field */
    unsigned int		rqe_port;      /* RQE port field */
    unsigned short		rqe_seqno;     /* RQE status field */
    unsigned int		rqe_stat;      /* RQE status field */
    unsigned int		rqe_type;      /* RQE type field */
    unsigned int		sleep_flag;    /* que_cmd sleep flag */
    long			adap_q_readl();

#ifdef _POWER_MPQP
    int				old_pri;

    MPQP_LOCK_DISABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */

    /* get pointer acb for interrupting adapter */
    p_acb = (t_acb *)p_intr_ds->p_acb_intr;
    p_acb->arq_sched = FALSE;

    MPQTRACE3("offl", p_intr_ds, p_acb);

    /*  Read the next RQE from the adapter response queue and	*/
    /*  extract each RQE field:					*/

    while ((resp_elem = 
		adap_q_readl(p_acb, p_acb->p_adap_rsp_que)) != -1)
    {
	rqe_type  = RQE_TYPE( resp_elem );
	rqe_port  = RQE_PORT( resp_elem );
	rqe_stat  = RQE_XESTATUS( resp_elem );
	rqe_cmd   = RQE_COMMAND( resp_elem );
	rqe_seqno = RQE_SEQUENCE( resp_elem );

	MBUFTRACE2("Orqe", resp_elem);

	/* get the dds pointer for the port in question */
	if ((unsigned char)rqe_port > (NUM_PORTS - 1) )
	{
#ifdef _POWER_MPQP
		MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */
		MPQTRACE2( "of2a",resp_elem);
		return;
	}

	/* port not configured ( dds pointer is null ) */
	if ((p_dds = p_acb->p_port_dds[rqe_port]) == (t_mpqp_dds *)NULL) 
	{
#ifdef _POWER_MPQP
        	MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */
		MPQTRACE2("off3", p_dds);

		return;
        }
	MPQTRACE3("of3A", p_dds, rqe_cmd);

	if /* there is an invalid port number in the rqe */
	   ( (rqe_cmd != STRT_CARD_RST) && (rqe_port >= NUM_PORTS) )
	{
#ifdef _POWER_MPQP
        	MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */
		MPQTRACE4("off4", STRT_CARD_RST, rqe_port, NUM_PORTS);
		mlogerr(ERRID_MPQP_ADPERR, p_dds->dds_dvc.port_num, 
			__LINE__, __FILE__, rqe_cmd, rqe_port, p_dds);

		return;
	}

	MPQTRACE4("off5", p_dds, p_dds->dds_dvc.port_state, CLOSED);

	if /* port from the rqe is closed and this is not an adapter */
	   /* status response element, discard the rqe */
	   ( DVC.port_state == CLOSED )  
	{
#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */
		MPQTRACE1("offb");

		return;
	}
	/* this flag is set whenever there is a response queue element
	   and will be cleared by mpqselect */

	WRK.cmd_avail_flag = TRUE;

	/* Use the RQE type to select on which offlevel interrupt	*/
	/* handler to use:						*/

	switch (rqe_type)
	{
	    case XMIT_COMPLETE:		/* TX acknowledgement */
		proc_tx_ack( p_acb, p_dds, resp_elem, rqe_seqno );
	        break;

	    case RECV_COMPLETE_DMA:	/* RX data ready */
		proc_rx_data( p_acb, p_dds, resp_elem );
	        break;

	    case COMMAND_SUCCESS:	/* Command complete */
		proc_cmd_cmplt( p_acb, p_dds, rqe_cmd, resp_elem );
	        break;

	    case SOL_STATUS:		/* Solicited port status response */
		proc_sol_resp( p_acb, p_dds, resp_elem );
	        break;

	    case XMIT_ERROR:		/* TX Error response */
		proc_tx_err( p_acb, p_dds, resp_elem, rqe_seqno );
	        break;

	    case RECV_COMPLETE:		/* RX Error response */
		proc_rx_err( p_acb, p_dds, resp_elem );
	        break;

	    case COMMAND_FAILURE:	/* Command Failure response */
		proc_cmd_fail( p_acb, p_dds, rqe_cmd, resp_elem );
	        break;

	    case UNSOL_STATUS:		/* Unsolicited Port Status */
		proc_unsol_stat( p_acb, p_dds, rqe_seqno, resp_elem );
	        break;

	    case RECOV_ERROR:		/* Adapter Error, EDRR valid */
		proc_adap_err( p_acb, p_dds, resp_elem );
	        break;

	    case DIAGNOSTIC_ERROR:	/* Diagnostic Error, EDRR valid */
		proc_diag_err( p_acb, p_dds, resp_elem );
	        break;

	    default:			/* invalid response */
		MPQTRACE4("off4", STRT_CARD_RST, rqe_port, NUM_PORTS);
		mlogerr(ERRID_MPQP_ADPERR, DVC.port_num, 
			__LINE__,__FILE__,resp_elem, p_dds);
	        break;

	} 
    } 
#ifdef _POWER_MPQP
    MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */
    MPQTRACE1("OffX");	

    return;
}

/*
 *  proc_cmd_cmplt - process command completion.  This function is
 *		     invoked whenever a command complete response queue
 *		     element has been received from the adapter.
 */

static void proc_cmd_cmplt ( t_acb		*p_acb,
			     t_mpqp_dds		*p_dds,
			     unsigned int	cmd,
			     unsigned int	resp_elem )
{

    unsigned int	code;
    unsigned int	opt0;
    unsigned int	opt1;
    unsigned int	opt2;
    unsigned int	opt3;

    code = opt0 = opt1 = opt2 = opt3 = 0;
    MPQTRACE4("PCMC", cmd, p_dds, resp_elem);
    switch (cmd)	/* switch on completed command type */
    {

	case XMIT_LONG:
	case XMIT_GATHER:   /* This is a transmit ack */
	    {
		/* wake up any blocked xmit */

	        e_wakeup( &(p_dds->dds_wrk.p_chan_info[0]->xmt_event_lst) );
		
		return;

	    }
	    break;

	case STRT_CARD_RST:
	    p_acb->adapter_state = RESET;
	    return;

	case SET_PARAM:
		/* log that parameters have been changed */
	case TERM_PORT:
	case FLUSH_PORT:
	case STRT_AUTO_RSP:
	case MP_TRACEON:
	case MP_TRACEOFF:
		return;

	case START_PORT:
	    {
		if /* port state is Start Port Requested */
		   ( p_dds->dds_dvc.port_state == START_REQUESTED )
		{
		    p_dds->dds_dvc.port_state = STARTED;
		    code = CIO_START_DONE;
     		    opt0 = CIO_OK;
		}
		break;
	    }

	case STOP_PORT:
	    {
	        MPQTRACE5("PCM1", p_dds->dds_dvc.port_state, 
			  p_dds->dds_wrk.sleep_on_halt, 
			  &p_dds->dds_wrk.halt_sleep_event,
			  p_dds->dds_wrk.halt_sleep_event);
		if /* port state is Stop Port Requested */
		   ( p_dds->dds_dvc.port_state == HALT_REQUESTED )
		{
		    p_dds->dds_dvc.port_state = HALTED;
		    code = CIO_HALT_DONE;
		    opt0 = CIO_OK;
		}
		if (WRK.sleep_on_halt)
		{
		    untimeout((int (*)(void))mpqtimer, p_dds);

		    /* e_sleep has been issued waiting for this event */
		    if (WRK.halt_sleep_event != EVENT_NULL) {
			MPQTRACE2("PCM2",WRK.halt_sleep_event);
			p_dds->dds_wrk.sleep_on_halt = FALSE;
			e_wakeup((void *)&p_dds->dds_wrk.halt_sleep_event);
		    }
		    /* e_sleep not issued yet, but it will be; so wait briefly*/
		    else 
			timeout((int (*)(void))halt_compl_timer,(int)p_dds,
			    (HZ*1/4));
		}
		MPQTRACE1("PCM3");		  
		break;
	    }
	default:
	    {
		code = cmd;
		opt0 = CIO_OK;
		break;
	    }
 

    } /* end of switch on completed command type */

    /* now go enque or send status to proper place */

    que_stat(p_dds,resp_elem, code, opt0, opt1, opt2, opt3);

    return;
}

/*
 *  proc_sol_resp - process solicited response.  This function is
 *		    invoked whenever a solicited response queue
 *		    element has been received from the adapter.
 */

static void proc_sol_resp ( t_acb		*p_acb,
			    t_mpqp_dds		*p_dds,
			    unsigned int	resp_elem )

{
    return;
}

/*
 *  proc_cmd_fail - process command failure response.  This function is
 *		    is invoked when the offlevel handler gets a command
 *		    complete negative acknowledgement indicating the
 *		    command in question failed in its assigned task.
 */

static void proc_cmd_fail ( t_acb		*p_acb,
			    t_mpqp_dds		*p_dds,
			    unsigned int	cmd,
			    unsigned int	resp_elem )
{
    unsigned int	code;
    unsigned int	opt0;
    unsigned int	opt1;
    unsigned int	opt2;
    unsigned int	opt3;
    unsigned short	sub_stat;

    code = opt0 = opt1 = opt2 = opt3 = 0;

    /* first we get the command failure status field for certain cases */

    sub_stat = RQE_STATUS(resp_elem);
    MPQTRACE5("PCME",  p_dds, resp_elem, cmd, sub_stat);
    switch (cmd)	/* switch on completed command type */
    {

	case SET_PARAM:
	case TERM_PORT:
	case FLUSH_PORT:
	case STRT_AUTO_RSP:
	case MP_TRACEON:
	case MP_TRACEOFF:
	    {
		return;
	    }

	case START_PORT:
	    {
		if /* port state is Start Port Requested */
		   ( p_dds->dds_dvc.port_state == START_REQUESTED )
		{
		    p_dds->dds_dvc.port_state = OPEN;
		    code = CIO_START_DONE;

		    if ( (sub_stat >= MP_XT1_TIMER )  &&
			( sub_stat <= MP_X_DCE_READY_TIMER ) )
		    {
			opt0 = MP_X21_TIMEOUT;
			opt1 = sub_stat;
		    }
		    else
		    {
			opt0 = sub_stat;
		    }
		}
		break;
	    }

	case STOP_PORT:
	    {
		if /* port state is Stop Port Requested */
		   ( p_dds->dds_dvc.port_state == HALT_REQUESTED )
		{
		    code = CIO_HALT_DONE;
		    opt0 = CIO_HARD_FAIL;
		}
		break;
	    }
	default:
	    {
		code = cmd;
		opt0 = CIO_HARD_FAIL;
		break;
	    }
 

    } /* end of switch on completed command type */

    /* now go enque or send status to proper place */

    que_stat(p_dds,resp_elem, code, opt0, opt1, opt2, opt3);

    return;
}

/*
 *  proc_unsol_stat - process unsolicited port status. This function is
 *		      invoked when the offlevel gets a response queue
 *		      element which indicates an unsolicited status was
 *		      received from the indicated port.
 */

static void proc_unsol_stat ( t_acb		*p_acb,
			      t_mpqp_dds	*p_dds,
			      unsigned short	seq,
			      unsigned int	resp_elem )

{
    int error;
    unsigned int	code;	/* status block code value */
    unsigned int	opt0;	/* option 0 */
    unsigned int	opt1;	/* option 1 */
    unsigned int	opt2;	/* option 2 */
    unsigned int	opt3;	/* option 3 */
    unsigned short	data1, data2;
    unsigned short	sub_stat;

    code = opt0 = opt1 = opt2 = opt3 = 0;

    /* first we get the command failure status field for certain cases */

    sub_stat = RQE_STATUS(resp_elem);

    error = data1 = data2 = 0;
    MPQTRACE5("PUNS", p_dds, resp_elem, seq, sub_stat);

    /* Examine adapter return code (seq) and post/queue the correct	*/ 
    /* stat. queue element.						*/ 

    switch (seq)
    {
	case MP_X21_TIMEOUT:
	    opt1 = 99;			/* indicate which timer expired */
	case MP_ADAP_NOT_FUNC:
	case MP_DSR_ON_TIMEOUT:
	case MP_DSR_ALRDY_ON:
	case MP_X21_RETRIES_EXC:
	    code = CIO_START_DONE;
	    opt0 = seq;
	    if /* start requested for this port */
	       ( p_dds->dds_dvc.port_state == START_REQUESTED )
	    {
		p_dds->dds_dvc.port_state = OPEN;
	    }
	    break;

	case MP_X21_CLEAR:
	    opt0 = seq;
	    if /* start requested for this port */
	       ( p_dds->dds_dvc.port_state == START_REQUESTED )
	    {
	        code = 0;
		/* p_dds->dds_dvc.port_state = OPEN; */
	    }
	    else
	    {
		code = CIO_ASYNC_STATUS;
	    }
	    break;

	case MP_RCV_TIMEOUT:
	case MP_DSR_DROPPED:
	    opt0 = seq;
	    code = CIO_ASYNC_STATUS;
	    break;

	case MP_RDY_FOR_MAN_DIAL:
	    code = MP_RDY_FOR_MAN_DIAL;
	    opt0 = CIO_OK;
	    break;

	case MP_TX_UNDERRUN:		/* these are logged in chk_threshold */
	case MP_CTS_UNDERRUN:		/* if the corresponding threshold is */
        case MP_CTS_TIMEOUT:            /* met.                              */
        case MP_LOST_COMMAND:
	    break;

	default:			/* othewise an asynchronous status */
	    opt0 = seq;
	    code = CIO_ASYNC_STATUS;
	    break;

    } /* end of switch */

    /* enqueueing of status response */
    if ( code > 0 ) 			/* valid code field value */
    {
       que_stat(p_dds, resp_elem, code, opt0, opt1, opt2, opt3);
    }

    /* Examine adapter return code (seq) and post/queue the correct	*/ 
    /* error log entry.							*/ 

    switch (seq)
    {
	case MP_X21_TIMEOUT:
	    error = ERRID_MPQP_X21TO;
	    data1 = sub_stat;
	    break;

	case MP_ADAP_NOT_FUNC:
	    error = ERRID_MPQP_ADPERR;
	    data1 = RQE_PORT(resp_elem);
	    data2 = RQE_COMMAND(resp_elem);
	    break;

	case MP_DSR_ON_TIMEOUT:
	    DDS_STATS.dsr_timeout++;
	    error = ERRID_MPQP_DSRTO;
	    break;

	case MP_DSR_ALRDY_ON:
	    DDS_STATS.dsr_offtimeout++;
	    error = ERRID_MPQP_DSROFFTO;
	    break;

	case MP_X21_RETRIES_EXC:
	    DDS_STATS.x21_stat++;
	    error = ERRID_MPQP_X21CECLR;
	    break;

	case MP_X21_CLEAR:
	    DDS_STATS.x21_stat++;
	    error = ERRID_MPQP_X21DTCLR;
	    break;

	case MP_DSR_DROPPED:
	    DDS_STATS.dsr_dropped++;
	    error = ERRID_MPQP_DSRDRP;
	    break;

	case MP_RCV_TIMEOUT:
	    error = ERRID_MPQP_RCVERR;
	    DDS_STATS.rx_timeout++;
	    break;

	case MP_TX_UNDERRUN:
	    DDS_STATS.tx_underrun++;
	    error = ERRID_MPQP_XMTUND;
	    break;

	case MP_CTS_UNDERRUN:
	    DDS_STATS.cts_underrun++;
	    error = ERRID_MPQP_CTSDRP;
	    break;

	case MP_CTS_TIMEOUT:
	    DDS_STATS.cts_timeout++;
	    error = ERRID_MPQP_CTSTO;
	    break;

        case MP_LOST_COMMAND:
            error = ERRID_MPQP_RCVERR;
            data1 = resp_elem;
            MPQTRACE2( "PUNA", resp_elem );
            break;

	default:
	    break;
    } /* end of switch */

    if ( error )	 			/* valid error log entry */
    {
     /*  Log an error if the error threshold warrants it:	*/
     if (chk_threshold(p_dds, THRES_UNSOL_STAT, resp_elem)) /*unsol type thres*/
     {
	mlogerr( error, DVC.port_num, __LINE__, __FILE__, data1, data2, p_dds);
     } 
    }

    return;
}

/*--------------------  P R O C _ R X _ D A T A  -----------------------*/
/*                                                                      */
/*  NAME: proc_rx_data                                                  */
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Proceses receive data from the adapter.  Maintains the		*/
/*	adapter receive chain structure; d_completes the "ready" 	*/
/*	mbuf and hands it up to the DLC, then d_masters a new mbuf 	*/
/*	and places it in the receive chain.				*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	This function runs only at offlevel.				*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*	Alters the receive map chain and receive statistics.		*/
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  d_complete, d_master, que_recv,	*/
/*				 m_get, m_clget, m_free, que_command	*/
/*                                                                      */
/*----------------------------------------------------------------------*/

proc_rx_data (
	t_acb		*p_acb,
	t_mpqp_dds	*p_dds,
	unsigned int	rqe )
{
	register int	i, tag;		/* index of receive map */

	MPQTRACE4( "PRXD", DVC.port_num, p_dds, rqe );
	
	/*  Extract the receive map tag from the RQE, use this to index	*/
	/*  the receive map table.  If the tag is out of range, log an	*/
	/*  error and quit.	 					*/

	tag = RQE_XESTATUS( rqe );
	if (( tag >= NUM_RECV_TCWS ) || ( !RECVMAP[ tag ].p_mbuf ))
	{
	    MPQTRACE2( "PRXt", tag );
	    mlogerr( ERRID_MPQP_ADPERR, DVC.port_num,__LINE__,__FILE__,0,0,p_dds);
	    return;
	}

	/*  Trace point "PRXm": mbuf pointer, mbuf data address, mbuf	*/
	/*  bus address (TCW).						*/

	MBUFTRACE5( "PRXm", RECVMAP[ tag ].p_mbuf, 
	    		    RECVMAP[ tag ].p_data, 
			    RECVMAP[ tag ].p_mbuf->m_data,
			    RECVMAP[ tag ].bus_addr );

	/*  D_complete the mbuf associated with this receive and issue	*/
	/*  it to the user of this port via que_recv. 			*/
	
	i = d_complete( p_acb->dma_channel_id, DMA_READ,
		RECVMAP[ tag ].p_data, CLBYTES, 
		M_XMEMD( RECVMAP[ tag ].p_mbuf ),
		RECVMAP[ tag ].bus_addr );

	if ( i != DMA_SUCC )		/* error on d_complete? */
	{
	    MPQTRACE2( "PRXd", tag );
	    mlogerr( ERRID_MPQP_ADPERR, DVC.port_num,__LINE__,__FILE__,0,0,p_dds);
	    return;
	}
	RECVMAP[ tag ].p_data = NULL;
	que_recv( p_dds, rqe, RECVMAP[ tag ].p_mbuf );
	RECVMAP[ tag ].p_mbuf = NULL;

	p_acb->c_rcv++;			/* increment statistics */

	/*  Replenish the receive table with mbufs; not only this	*/
	/*  receive map, but any maps that may have been left empty	*/
	/*  because no mbufs were available at the time.		*/

	for ( i = 0; i < NUM_RECV_TCWS; i++ )
	    if ( !RECVMAP[ i ].p_mbuf )
		if ( !load_recv_map( p_acb, p_dds, i, TRUE )) 
		    break;
	return;
}
	
/*-------------------  L O A D _ R E C V _ M A P -----------------------*/
/*                                                                      */
/*  NAME: load_recv_map                                                 */
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Loads the receive chain element (map) with an mbuf, prepares	*/
/*	it for DMA access, then issues it's address and index to the	*/
/*	adapter via a receive buffer indicate command.			*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	This function runs at process level or offlevel.		*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*	Alters the receive map chain element.				*/
/*                                                                      */
/*  RETURNS: FALSE if error occurs.                                     */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  d_master, m_get, m_clget,		*/
/*				 que_command				*/
/*                                                                      */
/*----------------------------------------------------------------------*/

load_recv_map( 
	t_acb		*p_acb,
	t_mpqp_dds	*p_dds,
	int		map_index,
	int		ndelay )	/* TRUE if sleep not allowed */
{
	adap_cmd_t	adap_cmd;	/* adapter command block */
	struct mbuf	*m;		/* mbuf pointer */


	/*  Get a new mbuf for the recv map and set it up for DMA via	*/
	/*  d_master.  Put the new mbuf in the indexed receive map 	*/
	/*  element.							*/

	if (!( m = m_getclust( ndelay ? M_DONTWAIT : M_WAIT, MT_DATA )))
	{
	    MBUFTRACE1( "LrmE" );
	    mlogerr( ERRID_MPQP_BFR, DVC.port_num, __LINE__,__FILE__,0,0,p_dds);
	    return( FALSE );
	} 
	RECVMAP[ map_index ].p_mbuf = m;
	RECVMAP[ map_index ].p_data = MTOD( m, char * );
	bzero( RECVMAP[map_index].p_data, CLBYTES);
		
	MPQTRACE5( "Lrmp", p_dds, m, m->m_data,
			   RECVMAP[ map_index ].bus_addr );

	d_master( p_acb->dma_channel_id, DMA_READ,
		RECVMAP[ map_index ].p_data, CLBYTES, M_XMEMD( m ),
		RECVMAP[ map_index ].bus_addr );

	/*  Issue a new d_mastered m_buf to the adapter 	*/
	/*  via the receive buffer indicate command.		*/

	bzero( &adap_cmd, sizeof( adap_cmd ));
	adap_cmd.type      = RCV_BUF_INDC;
	adap_cmd.host_addr = SWAPLONG( RECVMAP[ map_index ].bus_addr );
	adap_cmd.sequence  = SWAPSHORT( map_index );


	if ( que_command( p_acb, &adap_cmd, NULL, 0, ndelay ) < 0 )
	{
	    MPQTRACE2( "LrmA", adap_cmd.host_addr );
	    mlogerr( ERRID_MPQP_ADPERR, DVC.port_num, __LINE__,__FILE__,0,0,p_dds);
	    return( FALSE );
	}

	return( TRUE );
}

/*---------------------  P R O C _ R X _ E R R  ------------------------*/
/*                                                                      */
/*  NAME: proc_rx_err                                                   */
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Processes receive errors.  This function is invoked when	*/
/*	a response queue element is returned indicating an error	*/
/*	condition that occurred during the receive.			*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	This function runs only at process level.			*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:   mlogerr	  			*/
/*                                                                      */
/*----------------------------------------------------------------------*/

proc_rx_err (
	t_acb		*p_acb,
	t_mpqp_dds	*p_dds,
	unsigned int	rqe )
{

	MPQTRACE4( "PRXE", DVC.port_num, p_dds, rqe );

	if (! p_acb->recv_enabled )	/* if receives are disabled, 	*/
	    return;			/* ignore the interrupt.	*/

	/* update receive counters */
	increment_stats ( &DDS_STATS.rx_frame_lcnt, &DDS_STATS.rx_frame_mcnt,1);

	DDS_STATS.rx_err_cnt++;		/* update receive error counters */
	DDS_STATS.rx_err_dcnt++;

	switch( RQE_SEQUENCE( rqe )) {

	    case MP_RX_FRAME_ERR:
		DDS_STATS.asy_framing++;
		break;

	    case MP_RX_BSC_FRAME_ERR:
		DDS_STATS.asy_framing++;
		break;

	    case MP_RX_OVERRUN:
		DDS_STATS.rx_overrun++;
		break;

	    case MP_FRAME_CRC:
		DDS_STATS.CRC_error++;
		break;

	    case MP_SDLC_SHRT_FRM:
		DDS_STATS.short_frame++;
		break;

	    case MP_BUF_STAT_OVFLW:
		DDS_STATS.buffer_overflow++;
		break;

	    case MP_SDLC_ABORT:
		DDS_STATS.abort_detect++;
		break;

	    case MP_RX_PARITY_ERR:
		if ( WRK.data_proto & DATA_PROTO_BSC )
		    DDS_STATS.bsc_parity++;
		else
		    DDS_STATS.asy_parity++;
		break;

	    case MP_RX_BSC_PAD_ERR:
		DDS_STATS.rx_overrun++;
		break;

	    default:
	       mlogerr( ERRID_MPQP_RCVERR, DVC.port_num, __LINE__, __FILE__, 
                       rqe, 0, p_dds);
	       return;

	}

        /*  Log an error if the error threshold warrants it:	*/
	if ( chk_threshold(p_dds, THRES_RX_ERR, rqe) ) /* rx error type thres */
	{
	 mlogerr(ERRID_MPQP_RCVERR,DVC.port_num,__LINE__,__FILE__,rqe,0,p_dds);
	} 

	return;
}

/*---------------------  P R O C _ T X _ A C K  ------------------------*/
/*                                                                      */
/*  NAME: proc_tx_ack                                                   */
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Processes transmit acknowledgements.  This function is 		*/
/*	invoked when a response queue element indicating a transmit 	*/
/*	acknowledgement has been received for the process that		*/
/*	requested it.							*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	This function runs only at process level.			*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*	Alters the transmit map. 					*/
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  free_xmit_map, que_stat, mlogerr  	*/
/*                                                                      */
/*----------------------------------------------------------------------*/

proc_tx_ack (
	t_acb		*p_acb,
	t_mpqp_dds	*p_dds,
	unsigned int	rqe,
	unsigned int	seqno )
{
	MPQTRACE4( "PTXA", DVC.port_num, p_dds, rqe );

	if (! WRK.xmit_enabled )	/* if transmits are disabled, 	*/
	    return;			/* ignore the interrupt.	*/

	/*  Check the map index from the sequence field of the RQE; 	*/
	/*  if it is a bad index, log an error and return.		*/

	if ( seqno >= NUM_PORT_TCWS )
	{
	    MPQTRACE2( "PTXa", seqno );
	    mlogerr(ERRID_MPQP_ADPERR,DVC.port_num,__LINE__,__FILE__,0,0,p_dds);
	    return;
	}
	/*  Free the map's resources and return it back to the free 	*/
	/*  list then return status if required.			*/
	
	if ( XMITMAP[ seqno ].flags & CIO_ACK_TX_DONE )
	{
	    que_stat( p_dds, rqe, CIO_TX_DONE, CIO_OK, 
		   XMITMAP[ seqno ].write_id, XMITMAP[ seqno ].m, NULL );
	}
	free_xmit_map( p_acb, p_dds, seqno );
	return;
}

/*---------------------  P R O C _ T X _ E R R  ------------------------*/
/*                                                                      */
/*  NAME: proc_tx_err                                                   */
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Processes transmit errors.  This function is invoked when	*/
/*	a response queue element is returned indicating an error	*/
/*	condition that occurred during the transmit.			*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	This function runs only at process level.			*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*	Alters the transmit map table. 					*/
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  free_xmit_map, que_stat, mlogerr  	*/
/*                                                                      */
/*----------------------------------------------------------------------*/

proc_tx_err (
	t_acb		*p_acb,
	t_mpqp_dds	*p_dds,
	unsigned int	rqe,
	unsigned int	seqno )
{
    	unsigned int 	error;

	MPQTRACE4( "PTXE", DVC.port_num, p_dds, rqe );

	if (! WRK.xmit_enabled )	/* if transmits are disabled, 	*/
	    return;			/* ignore the interrupt.	*/

	/*  Check the map index from the sequence field of the RQE; 	*/
	/*  if it is a bad index, log an error and return.		*/

	if ( seqno >= NUM_PORT_TCWS )
	{
	    MPQTRACE2( "PTXe", seqno );
	    mlogerr(ERRID_MPQP_ADPERR,DVC.port_num,__LINE__,__FILE__,0,0,p_dds);
	    return;
	}
	/*  Free the map's resources, then decode the type of error	*/
	/*  and handle it accordingly:					*/
	
	free_xmit_map( p_acb, p_dds, seqno );

	DDS_STATS.tx_err_cnt++;		/* update transmit error counters */
	DDS_STATS.tx_err_dcnt++;

	switch( RQE_XESTATUS( rqe )) {

	    case MP_CTS_UNDERRUN:
		DDS_STATS.cts_underrun++;
		error = ERRID_MPQP_CTSDRP;
		break;

	    case MP_TX_UNDERRUN:
		DDS_STATS.tx_underrun++;
		error = ERRID_MPQP_XMTUND;
		break;

	    case MP_X21_CLEAR:
		DDS_STATS.x21_stat++;
		error = ERRID_MPQP_X21DTCLR;
		break;

	    case MP_TX_FAILSAFE_TIMEOUT:
		DDS_STATS.tx_timeout++;
		error = ERRID_MPQP_XFTO;
	        que_stat ( p_dds, rqe, 
		    CIO_TX_DONE, MP_TX_FAILSAFE_TIMEOUT, 0, 0, 0);
		break;

	    default:
	     mlogerr(ERRID_MPQP_ADPERR,DVC.port_num,__LINE__,__FILE__,rqe,0,p_dds);
	     return;
	}

	/*  Log an error if the error threshold warrants it:	*/
	if ( chk_threshold(p_dds, THRES_TX_ERR, rqe) ) /* tx error type thres */
	{
	    mlogerr( error, DVC.port_num,__LINE__,__FILE__,rqe,0, p_dds);
        } 
	return;
}

/*-------------------  F R E E _ X M I T _ M A P -----------------------*/
/*                                                                      */
/*  NAME: free_xmit_map                                                 */
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Removes system memory (in the map indexed by "map") from DMA	*/
/*	access; frees any mbufs from this map, then returns the map	*/
/*	to the transmit map free list.					*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	This function runs at process level or offlevel.		*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*	Alters the transmit map chain element.				*/
/*                                                                      */
/*  RETURNS: FALSE if error occurs.                                     */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  d_complete, m_freem, queue_write	*/
/*                                                                      */
/*----------------------------------------------------------------------*/
	
free_xmit_map (
	t_acb		*p_acb,
	t_mpqp_dds	*p_dds,
	unsigned int	map )
{
	MPQTRACE4( "FTXm", map, DVC.port_num, p_dds );

	/*  If a DMA operation took place, remove the system memory	*/
	/*  page from bus DMA access, free the associated mbuf.		*/

	if ( XMITMAP[ map ].dma_addr )
	{
	    d_complete( p_acb->dma_channel_id, DMA_WRITE_ONLY,
	        XMITMAP[ map ].dma_addr, CLBYTES, 
		M_XMEMD( XMITMAP[ map ].m ), XMITMAP[ map ].bus_addr );

	    MPQTRACE4( "FTXf", XMITMAP[map].m,XMITMAP[map].dma_addr,
		XMITMAP[map].flags);
	    XMITMAP[ map ].dma_addr = NULL;
	    if ( !( XMITMAP[ map ].flags & CIO_NOFREE_MBUF ) )
	    {
	        MBUFTRACE3( "Ffre", XMITMAP[map].m, XMITMAP[map].m->m_data );
	        m_freem( XMITMAP[ map ].m );
	    }
	    XMITMAP[ map ].m = NULL;
	}
	/*  Return the transmit map to the free list:	*/

	XMITMAP[ map ].map_free = TRUE;
	return;
}

/*--------------------  A C U M _ R C V _ S T A T S  -------------------*/
/*                                                                      */
/*  NAME: acum_rcv_stats                                                */
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Accumlates transmit statistics on each good receive.		*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS:  Nothing                                                   */
/*                                                                      */
/*----------------------------------------------------------------------*/

static void acum_rcv_stats (
	t_mpqp_dds *p_dds, 
	unsigned short length ) 
{
	/* update frame count */
	increment_stats ( &DDS_STATS.rx_frame_lcnt, &DDS_STATS.rx_frame_mcnt,1);

	/* update byte count */
	increment_stats ( &DDS_STATS.rx_byte_lcnt, &DDS_STATS.rx_byte_mcnt,
	    length );

	DDS_STATS.rx_dma++;		/* update the number of dma transfers */
	return;
}

/*
 *  que_stat - this function is called to enque a status response.  On entry
 *	       to this routine, the proper select queue is locked and status
 *	       block values have been filled in.  If the request is for a
 *	       kernel process, the kernel function will be directly called
 *	       synchronously and the select queue entry will never be dequeued
 *	       and if it is a user level process the select queue entry will
 *	       be enqueued to the tail of the status select queue.  Further,
 *	       selnotify will be called if necessary.  Finally, for all paths
 *	       the select queue will be unlocked.
 */

void que_stat ( t_mpqp_dds	*p_dds,
		unsigned int	resp_elem,
		unsigned int	code,
		unsigned int	opt0,
		unsigned int	opt1,
		unsigned int	opt2,
		unsigned int	opt3 )

{

    t_chan_info		*p_tmp_chinfo;	/* pointer to channel information ds */
    t_sel_que		*p_tmp_sel_que;	/* pointer to a select queue element */
    t_sel_que		**p_p_head;	/* temporary head pointer */
    t_sel_que		**p_p_tail;	/* temporary tail pointer */
    unsigned short	lcl_sync_flags;	/* sync flags compare buffer */
#ifndef _POWER_MPQP
    int			old_pri;	/* interrupt level      */

    /* disable interrupts until we finish with queuing the stat. element */
    old_pri = i_disable(INTOFFL3);
#endif /* _POWER_MPQP */

    MPQTRACE4("oqsE", p_dds, resp_elem, code);
    MPQTRACE5("oqsa", opt0, opt1, opt2, opt3);
    /* there is only one channel per port allowed */
    /* so the channel structure index is always zero */
    p_tmp_chinfo = p_dds->dds_wrk.p_chan_info[0];

    if /* no available select queue elements, unlock and return */
       ((p_tmp_sel_que = p_tmp_chinfo->p_sel_avail) == (t_sel_que *)NULL)
    {
    	MPQTRACE3("oqs1", p_dds, resp_elem);

	/* post a status queue elemet with CIO_LOST_STATUS */
	que_lost_stat ( p_dds, resp_elem, CIO_LOST_STATUS );

#ifndef _POWER_MPQP
	/* enable interrupts */
	i_enable(old_pri);
#endif

	return;
    }
	
    /* set the status block values */

    p_tmp_sel_que->stat_block.code = code;
    p_tmp_sel_que->stat_block.option[0] = opt0;
    p_tmp_sel_que->stat_block.option[1] = opt1;
    p_tmp_sel_que->stat_block.option[2] = opt2;
    p_tmp_sel_que->stat_block.option[3] = opt3;

    if /* kernel llc has channel open....*/
       ((p_tmp_chinfo->devflag & DKERNEL) == DKERNEL)
    {

	/* simply call the llc's status handling function */
    	MPQTRACE3("oqs2", p_tmp_chinfo->mpq_kopen.stat_fn, resp_elem);

	(*p_tmp_chinfo->mpq_kopen.stat_fn)
	    (p_tmp_chinfo->mpq_kopen.open_id, 
		&(p_tmp_sel_que->stat_block));
		
    }
    else
    {  /* user level llc */

    	MPQTRACE3("oqs3", p_tmp_chinfo->devflag, resp_elem);
	/* reset pointer to first available block from chain */

	p_tmp_chinfo->p_sel_avail = p_tmp_sel_que->p_sel_que; 

	/* set chain field in new block to NULL */

	p_tmp_sel_que->p_sel_que = (t_sel_que *)NULL;

	/* get head and tail pointer values */

	p_p_head = &(p_tmp_chinfo->p_stat_head);
	p_p_tail = &(p_tmp_chinfo->p_stat_tail);
	lcl_sync_flags = POLLPRI;

	if /* list was empty */
	   ( *p_p_head == (t_sel_que *)NULL)
	{
	    *p_p_head = *p_p_tail = p_tmp_sel_que;
	} 
	else
	{
	    (**p_p_tail).p_sel_que = p_tmp_sel_que;	/* set chain ptr */
	    *p_p_tail = p_tmp_sel_que;		/* set tail ptr */
	}

	/* now we set the code and rqe values in the select queue */
	/* element so that read and ioctls can tell what happened */

	p_tmp_sel_que->rqe_value = resp_elem;	/* response queue elt */
	    
	if /* selnotify is to be called for status */
	   ((p_tmp_chinfo->sync_flags & lcl_sync_flags) == lcl_sync_flags )
	{
	    /* reset proper bit in sync flag */
	    p_tmp_chinfo->sync_flags ^= lcl_sync_flags;
	    selnotify(p_tmp_chinfo->devno,0,lcl_sync_flags);
	}

    } /* end of user llc path */

#ifndef _POWER_MPQP
    /* enable interrupts */
    i_enable(old_pri);
#endif /* _POWER_MPQP */

    return;
}

/*
 *  que_recv - this function is called to enque a receive response that has
 *	       been taken from the response queue and requires a posting to the
 *	       select queue or calling the proper function if this is a kernel
 *	       process that has the port open.  
 */

static void 
que_recv ( t_mpqp_dds	*p_dds,
	   unsigned int	rqe_elem,
	   struct mbuf	*p_mbuf )
{

    unsigned short	lcl_sync_flags;	/* sync flags compare buffer */
    unsigned short	*p_packet;	/* pointer to received packet */
    unsigned int        *p_pkt_int;     /* pointer to received packet */
    t_chan_info		*p_tmp_chinfo;	/* pointer to channel information ds */
    t_sel_que		*p_tmp_sel_que;	/* pointer to a select queue element */
    t_sel_que		**p_p_head;	/* temporary head pointer */
    t_sel_que		**p_p_tail;	/* temporary tail pointer */
    unsigned short	tmp_off;	/* offset to beginning of data */
    unsigned short	tmp_lgt;	/* length of data */
    unsigned short	tmp_stat;	/* packet status */

    /* find the channel info pointer...right now */
    /* there is only one channel per port allowed */
    /* so the channel structure index is always zero */

    p_tmp_chinfo = p_dds->dds_wrk.p_chan_info[0];

    if /* no mbuf means this is a receive status...no packet */
       ( p_mbuf == (struct mbuf *)NULL )
    {

	/* here we zero the offset and length */

	tmp_off = tmp_lgt = 0;
    }
    else
    {

	/* first get the pointer to the data packet */

	p_packet = MTOD(p_mbuf, unsigned short *);
        p_pkt_int = MTOD(p_mbuf, unsigned int *);

	/* get offset, length and status information & update mbuf */

	tmp_off = (*p_packet++);
	tmp_off = SWAPSHORT(tmp_off);
	tmp_lgt = (*p_packet++);
	tmp_lgt = SWAPSHORT(tmp_lgt);
    	MPQTRACE5("Qpkt", p_mbuf, p_packet, tmp_off, tmp_lgt);
	/* This is a temporary fix:  if the adapter got a Bus Master 
	   channel check NMI, tmp_off will be wrong.  If it is wrong,
	   log an error, free up the mbuf and don't pass it to the higher 
	   level. */
	if ((tmp_off == 0) || (tmp_off >= CLBYTES))
 	{

	    MBUFTRACE2("Qbof", p_mbuf);
    	
	    m_freem( p_mbuf );
	    p_mbuf = NULL;
	    mlogerr(ERRID_MPQP_ADPERR, p_dds->dds_dvc.port_num, 
		__LINE__, __FILE__, rqe_elem, tmp_off, p_dds);
	    return;
	} 
        p_pkt_int += (int)(tmp_off/sizeof(int));
        MPQTRACE5("Qpk1",*p_pkt_int++,*p_pkt_int++,*p_pkt_int++,*p_pkt_int++);
    }

    /* now status always appears in second half of rqe */

    tmp_stat = RQE_STATUS( rqe_elem );

    /*  If Bisync, if the MSG type indicates a non-data receive  */
    /*  type, free the mbuf (if there is one) and set it to NULL */

    if (( WRK.data_proto == DATA_PROTO_BSC ) && p_mbuf &&
        ((( tmp_stat <= MP_DISC    ) && ( tmp_stat >= MP_ETB     )) ||
	   ( tmp_stat == MP_STX_ENQ ) || ( tmp_stat == MP_SOH_ENQ )) )
    {
	MBUFTRACE2("Qbsc", p_mbuf);
	m_freem( p_mbuf );
	p_mbuf = NULL;
	tmp_off = tmp_lgt = 0;
    } 

    MPQTRACE4("Qrx0", WRK.data_proto, p_mbuf, tmp_stat);

    if /* kernel llc has channel open....*/
       ((p_tmp_chinfo->devflag & DKERNEL) == DKERNEL)
    {
        if /* no available select queue elements, unlock and return */
	   ((p_tmp_sel_que = p_tmp_chinfo->p_sel_avail) == (t_sel_que *)NULL)
        {
            if (p_mbuf)
            {
                MBUFTRACE2("Qrx3", p_mbuf );
                m_freem( p_mbuf );
	    }
	    return;
        }
	if /* mbuf pointer exists */
	   ( p_mbuf != (struct mbuf *)NULL )
	{
	    p_mbuf->m_len = tmp_lgt;
	    p_mbuf->m_data += (unsigned long) tmp_off;
	}

	p_tmp_sel_que->stat_block.code = (unsigned long) tmp_stat;

	/* simply call the llc's receive function with parameters */

    	MPQTRACE5("Qrx1", rqe_elem, p_tmp_chinfo->mpq_kopen.open_id, p_mbuf, 
		p_mbuf->m_data);

	(*p_tmp_chinfo->mpq_kopen.rx_fn)
		(p_tmp_chinfo->mpq_kopen.open_id,
		 &p_tmp_sel_que->stat_block,p_mbuf);

    }
    else
    {  /* user level llc */

	if /* no available queue elements, log an error */
	   (p_tmp_chinfo->p_sel_avail == (t_sel_que *)NULL)
	{
	    mlogerr(ERRID_MPQP_QUE, p_dds->dds_dvc.port_num, 
		    __LINE__, __FILE__, 0, 0, p_dds);

       	    /* post a status queue elemet with CIO_ASYNC_STATUS */
	    que_lost_stat ( p_dds, rqe_elem, CIO_ASYNC_STATUS );

	    /* free up the mbuf since now status has been posted */
	    /* to indicate that the receive queue has overflowed */
	    if ( p_mbuf != (struct mbuf *)NULL )
	    {
		MBUFTRACE2("Qrx2", p_mbuf );
		m_freem ( p_mbuf );
	    }
	}
	else
	{  /* here we queue up a RX data available entry */

	    /* get pointer to first available block */

	    p_tmp_sel_que = p_tmp_chinfo->p_sel_avail;

	    /* reset pointer to first available block from chain */

	    p_tmp_chinfo->p_sel_avail = p_tmp_sel_que->p_sel_que; 

	    /* set chain field in new block to NULL */

	    p_tmp_sel_que->p_sel_que = (t_sel_que *)NULL;

	    /* get head and tail pointer values */

	    p_p_head = &(p_tmp_chinfo->p_rcv_head);
	    p_p_tail = &(p_tmp_chinfo->p_rcv_tail);
	    lcl_sync_flags = POLLIN;

	    if /* list was empty */
	       ( *p_p_head == (t_sel_que *)NULL)
	    {
		*p_p_head = *p_p_tail = p_tmp_sel_que;
	    } 
	    else
	    {
		(**p_p_tail).p_sel_que = p_tmp_sel_que;	/* set chain ptr */
		*p_p_tail = p_tmp_sel_que;		/* set tail ptr */
	    }

	    /* now we set the code and rqe values in the receive queue */
	    /* element so that read can tell what happened */

	    p_tmp_sel_que->rqe_value = rqe_elem; /* response queue elt */
	    
	    p_tmp_sel_que->stat_block.code = (unsigned long) tmp_stat;

	    p_tmp_sel_que->stat_block.option[0] = (unsigned long)p_mbuf;

	    /* set up local sync flags for compare */

	    if /* selnotify is to be called for receive */
	       ((p_tmp_chinfo->sync_flags & lcl_sync_flags) == lcl_sync_flags )
	    {
		/* reset proper bit in sync flag */
		p_tmp_chinfo->sync_flags ^= lcl_sync_flags;
		selnotify(p_tmp_chinfo->devno,0,lcl_sync_flags);
	    }

	} /* end of select queue element available logic */


        /* in case a process is waiting on blocked read, wake em up */

        e_wakeup( &(p_tmp_chinfo->rcv_event_lst) );

    } /* end of user llc path */

    acum_rcv_stats(p_dds,tmp_lgt);
    return;
}
 
/*
 *  que_lost_stat - this function is called to enque a status response.  On 
 * 	            entry, the correct dds is selected.  If this is a kernel
 *                  process then return.  Otherwise, check to see if a status
 *                  has already been queued.  If not, dequeue the element 
 *                  off of the approciate queue, fill in the specified fields
 *                  and enqueue it to the status queue. If the que element 
 *                  must be either a CIO_LOST_STATUS or CIO_ASYNC_STATUS. 
 */

void que_lost_stat ( t_mpqp_dds       *p_dds,
                       unsigned int   rqe_elem,
                       unsigned int   code ) 
{
 
    unsigned int        opt0;           /* option 0 */
    unsigned short      lcl_sync_flags; /* sync flags compare buffer */
    t_sel_que		**p_p_head;	/* temporary head pointer */
    t_sel_que		**p_p_tail;	/* temporary tail pointer */
    t_chan_info         *p_tmp_chinfo;  /* pointer to channel information ds */
    t_sel_que           *p_tmp_sel_que; /* pointer to a select queue element */

    MPQTRACE4("oqlE", p_dds, rqe_elem, code);

    /* get channel address, only one channel per port allowed */
    p_tmp_chinfo = p_dds->dds_wrk.p_chan_info[0];

    if /* user llc has channel open....*/
      ((p_tmp_chinfo->devflag & DKERNEL) != DKERNEL)
    {
	switch ( code )
	{ 
	  case CIO_LOST_STATUS:		/* lost status overflow */
	  {
	     /* get pointer to the lost_status queue element */
	     if ((p_tmp_sel_que=p_tmp_chinfo->p_lost_stat) == (t_sel_que *)NULL)
             {
	       return;  /* a pending lost status is already queued */
             }

	     /* dequeue the element off of the p_lost_stat queue */
	     p_tmp_chinfo->p_lost_stat = (t_sel_que *)NULL;

	     /* option 0 is always has a value of 0 */
	     opt0 = 0; 

             MPQTRACE3("oql1", p_tmp_sel_que, code); 
 
	     break;
	  }

	  case CIO_ASYNC_STATUS:		/* lost data from receive */
	  {
	     /* get pointer to the lost_receive queue element */
	     if ((p_tmp_sel_que=p_tmp_chinfo->p_lost_rcv) == (t_sel_que *)NULL)
             {
	       return; /* a pending lost status for receive is already queued */
             }

	     /* dequeue the element off of the p_lost_rcv queue */
	     p_tmp_chinfo->p_lost_rcv = (t_sel_que *)NULL;

             /* set the block option values depending on status block type */
	     opt0 = CIO_LOST_DATA;

             MPQTRACE3("oql2", p_tmp_sel_que, code); 

	     break;
          }

	  default:                            /* must be one or the other */
	     return;
	}

	/* set status block code depending on code value */
        p_tmp_sel_que->stat_block.code = code;

	/* set chain ptr in new block to NULL */
	p_tmp_sel_que->p_sel_que = (t_sel_que *)NULL;

	/* get head and tail pointer values */
	p_p_head = &(p_tmp_chinfo->p_stat_head);
	p_p_tail = &(p_tmp_chinfo->p_stat_tail);

	if /* list was empty */
	   ( *p_p_head == (t_sel_que *)NULL)
	{
	   *p_p_head = *p_p_tail = p_tmp_sel_que;
	} 
	else
	{
	   (**p_p_tail).p_sel_que = p_tmp_sel_que;  /* set chain ptr */
	   *p_p_tail = p_tmp_sel_que;		   /* set tail ptr */
	}

	/* set the options fields */
        p_tmp_sel_que->stat_block.option[0] = opt0;

	/* set the options fields to 0 - not required */
        p_tmp_sel_que->stat_block.option[1] = 0;
        p_tmp_sel_que->stat_block.option[2] = 0;
        p_tmp_sel_que->stat_block.option[3] = 0;

	/* setup call to selnotify */
        lcl_sync_flags = POLLPRI;
        p_tmp_sel_que->rqe_value = rqe_elem;
 
        if /* selnotify is to be called for status */
          ((p_tmp_chinfo->sync_flags & lcl_sync_flags) == lcl_sync_flags )
        {
           /* reset proper bit in sync flag */
           p_tmp_chinfo->sync_flags ^= lcl_sync_flags;
           selnotify(p_tmp_chinfo->devno,0,lcl_sync_flags);
        }

      } /* end of user llc path */

      MPQTRACE3("oqlX", p_dds, code);

      return;
}

/*
 *  proc_adap_err - process adapter error.  This function is invoked when
 *		    a response queue element is received which indicates
 *		    that an adapter error has occurred.
 */

static void proc_adap_err( t_acb		*p_acb,
			   t_mpqp_dds           *p_dds,
			   unsigned int		resp_elem )

{

    MPQTRACE4("PADE", resp_elem, p_acb, p_dds->dds_dvc.port_num);
    mlogerr(ERRID_MPQP_ADPERR, p_dds->dds_dvc.port_num, __LINE__, __FILE__, 
	    (uint)RQE_PORT(resp_elem), (uint)RQE_COMMAND(resp_elem), p_dds);
    return;
}

/*
 *  proc_diag_err - process diagnostic error.  This function is invoked
 *		  when a response queue element indicating an error
 *		  occurred during a diagnostic is received from the
 *		  adapter software.
 */

static void proc_diag_err ( t_acb		*p_acb,
			    t_mpqp_dds		*p_dds,
			    unsigned int	resp_elem )
{
    MPQTRACE4("PPDE", resp_elem, p_acb, p_dds->dds_dvc.port_num);
    return;
}
/*--------------------  C H K __ T H R E S H O L D ---------------------*/
/*                                                                      */
/*  NAME: chk_threshold                                                 */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Examine threshold values and queue up a stat. queue element     */
/*      when required.							*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	This function runs only at offlevel.				*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*	Alters only the t_cio_stats.					*/
/*                                                                      */
/*  RETURNS: 0 = do not issue an error log entry			*/
/*           1 = request an error log entry				*/
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  que_stat				*/
/*                                                                      */
/*----------------------------------------------------------------------*/

chk_threshold (
		t_mpqp_dds	*p_dds,
		unsigned short	thres_type,
		unsigned int	rqe )
{
    unsigned short	opt0;		/* option 0 for stat. queue element */

    MPQTRACE2("ThrE", thres_type );

    /* verify if transmit or receive threshold are set - log thresholds */

    if ( (DDS_THRESH.tx_err_thresh  > 0) || (DDS_THRESH.rx_err_thresh  > 0) ||
         (DDS_THRESH.tx_err_percent > 0) || (DDS_THRESH.rx_err_percent > 0) )
    {
	MPQTRACE5("Thrs", DDS_THRESH.tx_err_thresh, DDS_THRESH.tx_err_percent, 
                          DDS_THRESH.rx_err_thresh, DDS_THRESH.rx_err_percent );
	switch ( thres_type )
	{ 
	  case THRES_UNSOL_STAT:	/* Unsoliated status from adapter */
	  {
	   if ( (DDS_THRESH.tx_underrun_thresh > 0) && 
                (DDS_STATS.tx_underrun >= DDS_THRESH.tx_underrun_thresh) ) 
	   {
		DDS_STATS.tx_underrun = 0;
		DDS_STATS.tx_err_dcnt = 0;	/* reset tx - next threshold */
		opt0 = MP_TX_UNDERRUN;
		break;
	   }
	   if ( (DDS_THRESH.tx_cts_drop_thresh > 0) &&
                (DDS_STATS.cts_underrun >= DDS_THRESH.tx_cts_drop_thresh) ) 
	   {
		DDS_STATS.cts_underrun = 0;
		DDS_STATS.tx_err_dcnt = 0;	/* reset tx - next threshold */
		opt0 = MP_CTS_UNDERRUN;
		break;
	   }
	   if ( (DDS_THRESH.tx_cts_timeout_thresh > 0) &&
                (DDS_STATS.cts_timeout >= DDS_THRESH.tx_cts_timeout_thresh) )
	   {
		DDS_STATS.cts_timeout = 0;
		DDS_STATS.tx_err_dcnt = 0;	/* reset tx - next threshold */
		opt0 = MP_CTS_TIMEOUT;
		break;
	   }
	  }

	  case THRES_TX_ERR:		/* Transmit errors */
	  {
	   if ( (DDS_THRESH.tx_cts_drop_thresh > 0) &&
	        (DDS_STATS.cts_underrun >= DDS_THRESH.tx_cts_drop_thresh) )
	   {
		DDS_STATS.cts_underrun = 0;
		DDS_STATS.tx_err_dcnt = 0;	/* reset tx - next threshold */
		opt0 = MP_CTS_UNDERRUN;
		break;
	   }
	   if ( (DDS_THRESH.tx_underrun_thresh > 0) &&
	        (DDS_STATS.tx_underrun >= DDS_THRESH.tx_underrun_thresh) )
	   {
		DDS_STATS.tx_underrun = 0;
		DDS_STATS.tx_err_dcnt = 0;	/* reset tx - next threshold */
		opt0 = MP_TX_UNDERRUN;
		break;
	   }
	   if ( (DDS_THRESH.tx_fs_timeout_thresh > 0) &&
	        (DDS_STATS.tx_timeout >= DDS_THRESH.tx_fs_timeout_thresh) )
	   {
		DDS_STATS.tx_timeout = 0;
		DDS_STATS.tx_err_dcnt = 0;	/* reset tx - next threshold */
		opt0 = MP_TX_FAILSAFE_TIMEOUT;
		break;
	   }
	   if ( (DDS_THRESH.tx_err_thresh > 0) &&
	        (DDS_STATS.tx_err_dcnt >= DDS_THRESH.tx_err_thresh) )
	   {
		DDS_STATS.tx_err_dcnt = 0;	/* reset tx - next threshold */
		opt0 = MP_TOTAL_TX_ERR;
		break;
	   }
	   if ( (DDS_THRESH.tx_err_percent > 0) &&
                ((DDS_STATS.tx_err_cnt * 100 / DDS_STATS.tx_frame_lcnt)
                   >= DDS_THRESH.tx_err_percent) )
	   {
		DDS_STATS.tx_err_cnt = 0;
		DDS_STATS.tx_err_dcnt = 0;	/* reset tx - next threshold */
		opt0 = MP_TX_PERCENT;
		break;
	   }
	  }

	  case THRES_RX_ERR:		/* Receive errors */
	  {
	   if ( (DDS_THRESH.rx_frame_err_thresh > 0) &&
                (DDS_STATS.asy_framing >= DDS_THRESH.rx_frame_err_thresh) )
	   {
		DDS_STATS.asy_framing = 0;
	        DDS_STATS.rx_err_dcnt = 0;	/* reset rx - next threshold */
                if ( WRK.data_proto & DATA_PROTO_BSC )
		    opt0 = MP_RX_BSC_FRAME_ERR;
		else
		    opt0 = MP_RX_FRAME_ERR;
		break;
	   }
	   if ( (DDS_THRESH.rx_overrun_err_thresh > 0) &&
	        (DDS_STATS.rx_overrun >= DDS_THRESH.rx_overrun_err_thresh) )
	   {
		DDS_STATS.rx_overrun = 0;
	        DDS_STATS.rx_err_dcnt = 0;	/* reset rx - next threshold */
	 	opt0 = MP_RX_OVERRUN;
		break;
	   }
	   if ( (DDS_THRESH.rx_frame_err_thresh > 0) &&
	        (DDS_STATS.CRC_error >= DDS_THRESH.rx_frame_err_thresh) )
	   {
		DDS_STATS.CRC_error = 0;
	        DDS_STATS.rx_err_dcnt = 0;	/* reset rx - next threshold */
		opt0 = MP_FRAME_CRC;
		break;
	   }
	   if ( (DDS_THRESH.rx_frame_err_thresh > 0) &&
                (DDS_STATS.short_frame >= DDS_THRESH.rx_frame_err_thresh) )
	   {
		DDS_STATS.short_frame = 0;
	        DDS_STATS.rx_err_dcnt = 0;	/* reset rx - next threshold */
		opt0 = MP_SDLC_SHRT_FRM;
		break;
	   }
	   if ( (DDS_THRESH.rx_dma_bfr_err_thresh > 0) &&
	        (DDS_STATS.buffer_overflow >= DDS_THRESH.rx_dma_bfr_err_thresh))
	   {
		DDS_STATS.buffer_overflow = 0;
	        DDS_STATS.rx_err_dcnt = 0;	/* reset rx - next threshold */
		opt0 = MP_RX_DMA_BFR_ERR;
		break;
	   }
	   if ( (DDS_THRESH.rx_abort_err_thresh > 0) &&
	        (DDS_STATS.abort_detect >= DDS_THRESH.rx_abort_err_thresh) )
	   {
		DDS_STATS.abort_detect = 0;
	        DDS_STATS.rx_err_dcnt = 0;	/* reset rx - next threshold */
		opt0 = MP_RX_ABORT;
		break;
	   }
	   if ( (DDS_THRESH.rx_frame_err_thresh > 0) &&
	        (DDS_STATS.bsc_parity >= DDS_THRESH.rx_frame_err_thresh) )
	   {
		DDS_STATS.bsc_parity = 0;
	        DDS_STATS.rx_err_dcnt = 0;	/* reset rx - next threshold */
		opt0 = MP_RX_BSC_FRAME_ERR;
		break;
	   }
	   if ( (DDS_THRESH.rx_frame_err_thresh > 0) &&
	        (DDS_STATS.asy_parity   >= DDS_THRESH.rx_frame_err_thresh) )
	   {
		DDS_STATS.asy_parity = 0;
	        DDS_STATS.rx_err_dcnt = 0;	/* reset rx - next threshold */
		opt0 = MP_RX_PARITY_ERR;
		break;
	   }
	   if ( (DDS_THRESH.rx_err_thresh > 0) &&
	        (DDS_STATS.rx_err_dcnt >= DDS_THRESH.rx_err_thresh) )
	   {
	        DDS_STATS.rx_err_dcnt = 0;	/* reset rx - next threshold */
		opt0 = MP_TOTAL_RX_ERR;
		break;
	   }
	   if ( (DDS_THRESH.rx_err_percent > 0) &&
	        ((DDS_STATS.rx_err_cnt * 100 / DDS_STATS.rx_frame_lcnt)
	          >= DDS_THRESH.rx_err_percent) )
	   {
		DDS_STATS.rx_err_cnt = 0;
	        DDS_STATS.rx_err_dcnt = 0;	/* reset rx - next threshold */
		opt0 = MP_TOTAL_RX_ERR;
		break;
	   }
	  }
	  default:
		return ( 0 );		/* rc=0 - no error log entry */

	}  /* end of case */

	if ( opt0 > 0 )				/* post a queue element */
	{
	   que_stat( p_dds, rqe, MP_ERR_THRESHLD_EXC , opt0 , 0, 0, 0);
	   MPQTRACE2( "Thrq", opt0 );
	   return ( 1 );		/* rc=1 - request error log entry */ 
	}

    }   /* end of if */

	MPQTRACE1( "ThrX" );
	return ( 0 );			/* rc=0 - no error log entry */
}

/*
** EVERYTHING BELOW RUNS AT CLOCK 
** INTERRUPT LEVEL SO KEEP
** IT SHORT AND SWEET.
*/

/*
** This routine will handle a timer that expires when close
** is waiting on a halt port to complete.
*/
void 
mpqtimer(t_mpqp_dds *p_dds) 
{

     if (p_dds->dds_wrk.sleep_on_halt) 
     {
       p_dds->dds_wrk.sleep_on_halt = FALSE;
       p_dds->dds_dvc.port_state = DORMANT_STATE;
       MPQTRACE2("TMR2",WRK.halt_sleep_event);
       e_wakeup((void *)&p_dds->dds_wrk.halt_sleep_event);
     }
     return;
}

/* This routine is to solve a small timing window in the processing of the 
HALT ioctl.  When the driver is waiting on a halt to complete, it sets 
WRK.sleep_on_halt to TRUE, calls stop_port, and then calls e_sleep to wait 
to be interrupted by the adapter when the halt is completed.  Interrupts 
cannot be disabled around the e_sleep because an interrupt is necessary to 
complete the sequence.  So this timeout was added for the case where the 
interrupt arrives between calling stop_port and calling e_sleep.   */

void 
halt_compl_timer(t_mpqp_dds *p_dds)
{
        MPQTRACE2("TMR3",WRK.halt_sleep_event);
	p_dds->dds_wrk.sleep_on_halt = FALSE;
        e_wakeup((void *)&p_dds->dds_wrk.halt_sleep_event);
        return;
}

