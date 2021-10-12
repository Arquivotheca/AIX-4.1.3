static char sccsid[] = "@(#)86	1.33.2.3  src/bos/kernext/tokdiag/tokintr.c, diagddtok, bos411, 9428A410j 2/17/94 18:43:31";
/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS:  que_oflv(), cio_queofflms_func(), cio_wdt_func(),
 *             cio_cdt_func(), ring_stat_pro(),
 *             cmd_pro(), adap_chk(), oflv_ioctl_to()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/cblock.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include "tok_comio_errids.h"
#include <sys/except.h>
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <stddef.h>
#include <sys/timer.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/watchdog.h>
#include <sys/xmem.h>
#include <sys/comio.h>
#include <sys/trchkid.h>
#include <sys/tokuser.h>
#include <sys/adspace.h>
#include "tokdshi.h"
#include "tokdds.h"
#include "tokdslo.h"
#include "tokproto.h"
#include "tokprim.h"

extern tracetable_t    tracetable;
extern cdt_t   ciocdt;
extern dd_ctrl_t   dd_ctrl;

/*--------------------------  T O K S L I H  ---------------------------*/
/*
 *  NAME: tokslih
 *
 *  FUNCTION:
 *      Token Ring device driver Second Level Interrupt Handler.
 *
 *  EXECUTION ENVIRONMENT:
 *      Runs strictly at interrupt level.
 *
 *  DATA STRUCTURES:
 *      For offlevel processing, builds and queues an offlevel work
 *      element in the offlevel queue.
 *
 *  RETURNS:
 *      INTR_SUCC       The interrupt was accepted and processed.
 *      INTR_FAIL       The interrupt was not accepted.
 *
 *  EXTERNAL PROCEDURES CALLED: que_oflv, attach_bus, pio_read,
 *                              detach_bus, d_complete, d_master,
 *                              i_reset, xmattach, xmdetach, d_kmove,
 *                              pio_write, io_att, io_det,
 *                              enter_limbo, logerr, bug_out
 *                              setjmpx, longjmpx, clearjmpx
 */
/*----------------------------------------------------------------------*/

int
tokslih ( register struct intr *ihsptr )
{
        register dds_t  *p_dds = (dds_t *)ihsptr;
        offl_elem_t     owe;            /* offlevel work element */
        ushort          reason;         /* interrupt reason */
        unsigned int    bus;
        unsigned int    pio_attachment;
        int             rc;

        DEBUGTRACE2 ("intB",(ulong)p_dds);          /* tokslih begin */
                                        /* Get type of int from status reg */
	bus = BUSIO_ATT(WRK.bus_id, WRK.bus_io_addr);
        if (rc = BUS_GETSRX( bus + REG_OFFSET( STATUS_REG ), &reason)) {
		if (pio_retry(p_dds, rc, GETSR, (bus + REG_OFFSET(STATUS_REG)),
			&reason))
		{
			BUSIO_DET(bus);
			return(INTR_FAIL);
		}
	}
        if (!(reason & TOK_INT_SYSTEM)) /* is this ours? */
	{
		TRACE3("iNOT", (ulong)p_dds, (ulong)reason );
		BUSIO_DET(bus);
		return(INTR_FAIL);
	}

	if ( (p_dds->wrk.adap_state == RESET_PHASE0) ||
             (p_dds->wrk.adap_state == RESET_PHASE1) ||
             (p_dds->wrk.adap_state == RESET_PHASE2) || 
             (p_dds->wrk.adap_state == ADAP_KILL_PENDING) || 
	     (p_dds->wrk.adap_doa == DEAD_ON_ARRIVAL) )
	{
		TRACE3("FooT", SLIH_6, (ulong)reason );
		PIO_PUTSRX(bus + REG_OFFSET(COMMAND_REG), ACK_INT);
		i_reset(ihsptr);               /* reset interrupt hardware */
		BUSIO_DET(bus);
		return(INTR_SUCC);
	}  /* end if reset adapter spurious interrupt */

	if (reason & MC_ERR) {
		rc = mc_err(p_dds, bus, reason);
		i_reset(ihsptr);               /* reset interrupt hardware */
		BUSIO_DET(bus);
		return(INTR_SUCC);
	}

        /*
         *  If we are not in a dead, null, or downloading microcode
         *  state, read the SSB into command portion of the offlevel
         *  work element.
         */
        if ( (p_dds->wrk.adap_state != DEAD_STATE) &&
             (p_dds->wrk.adap_state != NULL_STATE) &&
             (p_dds->wrk.adap_state != DOWNLOADING)  )
        {
            switch (reason & TOK_IC)        /* dispatch on interrupt code */
            {
               case TRANSMIT_STATUS:
                   rc = d_kmove (&(p_dds->wrk.tx_owe.cmd),
                            p_dds->wrk.p_d_ssb,
                            sizeof(t_ssb),
                            p_dds->wrk.dma_chnl_id,
                            p_dds->ddi.bus_id,
                            DMA_READ);

		   if (rc == EINVAL) 	/* IOCC is NOT buffered */
                      bcopy (p_dds->wrk.p_ssb,
                         &(p_dds->wrk.tx_owe.cmd),
                         sizeof(t_ssb));

                   ++p_dds->wrk.tx_intr_in;
                   ++p_dds->ras.ds.xmit_intr_cnt;
                        /* schedule off-level if necessary */
                   SCHED_OFLV( p_dds );
                   break;

               case RECEIVE_STATUS:
                   rc = d_kmove (&(p_dds->wrk.recv_owe.cmd),
                            p_dds->wrk.p_d_ssb,
                            sizeof(t_ssb),
                            p_dds->wrk.dma_chnl_id,
                            p_dds->ddi.bus_id,
                            DMA_READ);

		   if (rc == EINVAL) 	/* IOCC is NOT buffered */
                      bcopy (p_dds->wrk.p_ssb,
                         &(p_dds->wrk.recv_owe.cmd),
                         sizeof(t_ssb));

                   ++p_dds->wrk.recv_intr_in;
                        /* schedule off-level if necessary */
                   SCHED_OFLV( p_dds );
                   break;

                case ADAPTER_CHK:           /* ADAPTER CHECK */
		    pio_attachment = attach_bus( p_dds );
                    pio_write( p_dds, ADDRESS_REG, ADAPTER_CHK_ADDR );
                    p_dds->wrk.ac_blk.code  = pio_read( p_dds, AUTOINCR_REG );
                    p_dds->wrk.ac_blk.parm0 = pio_read( p_dds, AUTOINCR_REG );
                    p_dds->wrk.ac_blk.parm1 = pio_read( p_dds, AUTOINCR_REG );
                    p_dds->wrk.ac_blk.parm2 = pio_read( p_dds, AUTOINCR_REG );
		    detach_bus( p_dds, pio_attachment );
                    owe.int_reason = reason;
                    owe.who_queued = OFFL_INTR;
                    que_oflv( &p_dds->ofl, &owe );
		    TRACE2("ADCK", reason);
                    break;

                case SCB_CLEAR:             /* SCB CLEAR */
                   if ( p_dds->wrk.limbo == LIMBO_RECV)
                   {   /* we are comming out of limbo */
                       owe.int_reason = reason;
                       owe.who_queued = OFFL_INTR;
                       que_oflv( &p_dds->ofl, &owe );
		       TRACE2("SCBC", reason);
                   } /* end if comming out of limbo */
                   break;

               default:                    /* other valid interrupt */
                   rc = d_kmove (&owe.cmd,
                            p_dds->wrk.p_d_ssb,
                            sizeof(t_ssb),
                            p_dds->wrk.dma_chnl_id,
                            p_dds->ddi.bus_id,
                            DMA_READ);

		   if (rc == EINVAL) 	/* IOCC is NOT buffered */
		      bcopy (p_dds->wrk.p_ssb,
		         &owe.cmd,
			 sizeof(t_ssb));

                   owe.int_reason = reason;
                   owe.who_queued = OFFL_INTR;
                   que_oflv( &p_dds->ofl, &owe );
		   DEBUGTRACE2("DEFL", reason);
                   break;
            } /* end switch (reason) */

        } /* end if !DEAD_STATE || !NULL_STATE  || !DOWNLOADING */
        else
        {
                /*
                 * NOTE:
                 *      At this point, the interrupt must be
                 *      a spurious interrupt or a left over one.
                 *      We can do nothing to process this interrupt.
                 */

            TRACE3 ("FooT",SLIH_5, (ulong)reason);    /* spurrious interrupt */
        }  /* end else if DEAD_STATE || NULL_STATE || DOWNLOADING */

        /* acknowledge the interrupt */
	PIO_PUTSRX(bus + REG_OFFSET(COMMAND_REG), ACK_INT);

	i_reset(ihsptr);               /* reset interrupt hardware */
	BUSIO_DET(bus);
        DEBUGTRACE2 ("intE",(ulong)reason);         /* tokslih end */
        return(INTR_SUCC);
} /* end function tokslih */

int
mc_err(p_dds, bus, reason)
register dds_t  *p_dds;
unsigned int    bus;
ushort          reason;         /* interrupt reason */
{
	/*
	 * NOTE:
	 * Since the adapter has detected a Micro Channel
	 * error, we must clear the error via d_complete().
	 * We can only do this if the ACA is available to
	 * do a d_complete() on.  Doing the d_complete()
	 * will clear the error on our DMA channel, thus
	 * allowing DMA to continue on our DMA channel.
	 */
	TRACE2("FooT", SLIH_0);

	if ( (p_dds->wrk.adap_state != DEAD_STATE) &&
		(p_dds->wrk.adap_state != NULL_STATE) )
	{
                /*
                 *  We're not in the null or dead state, clear
                 *  the error by d_completing and re-d_mastering the ACA.
                 */
		p_dds->wrk.mcerr = d_complete(p_dds->wrk.dma_chnl_id,
			DMA_READ, p_dds->wrk.p_mem_block, PAGESIZE,
                        &p_dds->wrk.mem_block_xmd,
                        p_dds->wrk.p_d_mem_block);

                d_master(p_dds->wrk.dma_chnl_id, DMA_READ|DMA_NOHIDE,
                         p_dds->wrk.p_mem_block, PAGESIZE,
                         &p_dds->wrk.mem_block_xmd, p_dds->wrk.p_d_mem_block);

		TRACE3("FooT", SLIH_4, p_dds->wrk.mcerr);
		logerr( p_dds, ERRID_TOK_MC_ERR );

		/*
		 * Kill the adapter! 
		 * Don't let the adapter continue to DMA data
		 */	
		(void)kill_adap( p_dds );

		if ( p_dds->wrk.limbo == PARADISE )
			enter_limbo( p_dds, TOK_MC_ERROR, reason );
		else if ( !(p_dds->wrk.limbo == NO_OP_STATE) )
			cycle_limbo(p_dds);
	}
	else
	{   /* acknowledge the interrupt */
		TRACE2("FooT", SLIH_3);
		PIO_PUTSRX(bus + REG_OFFSET(COMMAND_REG), ACK_INT);
	}   /* end if IOCC buffer failed succeeded */
	return(0);
}

/*--------------------------  T O K O F L V  ---------------------------*/
/*
*  NAME: tokoflv
*
*  FUNCTION:
*      Token Ring device driver offlevel interrupt handler.
*
*  EXECUTION ENVIRONMENT:
*      Runs strictly at interrupt level.
*
*  DATA STRUCTURES:
*      Removes offlevel work elements from the offlevel queue.
*
*  RETURNS:
*      0
*
*/
/* ----------------------------------------------------------------------*/

int
tokoflv ( register offlevel_que_t *ofl_ptr )
{
    register dds_t *p_dds;                 /* computed from ofl_ptr */
    register int   saved_intr_level;
    int            done=FALSE;          /* stopping flag */
    register offl_elem_t    *p_owe;        /* pointer to offl int elem */

    DEBUGTRACE2 ("OFFb",(ulong)ofl_ptr);         /* tokoflv begin */

    p_dds = (dds_t *)( ( (unsigned long)ofl_ptr) - offsetof( dds_t, ofl) );

    saved_intr_level = i_disable(INTCLASS2);
    p_dds->ofl.scheduled = FALSE;
    p_dds->ofl.running = TRUE;

    while (!done) {
	do
	{  /* begin TX and RCV processing loop */
	   /* process packets from receive chain */
		while (WRK.recv_intr_in != WRK.recv_intr_out ) {   
			tok_receive(p_dds, &(p_dds->wrk.recv_owe));
			++p_dds->wrk.recv_intr_out;
		}

		/* fill transmit chain with queued packets */
		if (p_dds->wrk.tx_intr_in != p_dds->wrk.tx_intr_out ) {
			(void)tx_oflv_pro(p_dds, &(p_dds->wrk.tx_owe) );
			++p_dds->wrk.tx_intr_out;
		} 
	}  /* end TX and RCV processing do-while loop */
	while ( (p_dds->wrk.recv_intr_in != p_dds->wrk.recv_intr_out) ||
		(p_dds->wrk.tx_intr_in != p_dds->wrk.tx_intr_out) );


	/* process off level work element */
	if (p_dds->ofl.next_out != p_dds->ofl.next_in) {  
		/* dequeue work element */
		p_owe = (offl_elem_t *)
			&p_dds->ofl.offl_elem[p_dds->ofl.next_out];
		if (++p_dds->ofl.next_out >= MAX_OFFL_QUEUED)
		p_dds->ofl.next_out = 0;
		i_enable(saved_intr_level); 

		switch( p_owe->who_queued ) {
		    case OFFL_INTR:                 /* FROM SLIH: */
			switch (p_owe->int_reason & TOK_IC) {
				case RING_STATUS:
				    ring_stat_pro(p_dds, p_owe);
				    break;
				case CMD_STATUS:
				    DEBUGTRACE1("CMDP");
				    cmd_pro(p_dds, p_owe);
				    break;
				case IMPL_FORCE:
				    /*
				    * Check the current state of limbo. If
				    * we are in PARADISE, then enter limbo,
				    * otherwise we have a Double Jeopardy
				    * situation...cycle the recovery scheme.
			    	    */
				    TRACE2("FooT", OFLV_0);
				    saved_intr_level = i_disable(INTCLASS2);
				    p_dds->wrk.limbo_owe = *p_owe;
				    if ( p_dds->wrk.limbo == PARADISE )
					    enter_limbo( p_dds, TOK_IMPL_FORCE,
						  p_owe->int_reason);
				    else if (!(WRK.limbo == NO_OP_STATE) )
					    cycle_limbo(p_dds);
				    i_enable(saved_intr_level);
				    break;
				case ADAPTER_CHK:
				    adap_chk(p_dds, p_owe);
				    break;
				case SCB_CLEAR:
				    egress_limbo(p_dds);
				    break;
				default:
				    TRACE2("FooT", OFLV_1);
				    break;
			}   /* end switch of Interrupt reason */
			break;
		    case OFFL_MBUF_TIMER:
			load_recv_chain(p_dds);
			break;
		    case OFFL_ADAP_BRINGUP:
			oflv_bringup(p_dds, p_owe);
			break;
		    default:                  /* IOCTL TIMER: */
			oflv_ioctl_to(p_dds, p_owe);
			break;
		}   /* end switch of whodunnit? */

	        saved_intr_level = i_disable(INTCLASS2);
	}   /* end if OFLV work queue !empty loop */
	else 
	{
		if ( (p_dds->wrk.recv_intr_in == p_dds->wrk.recv_intr_out) &&
			(p_dds->wrk.tx_intr_in == p_dds->wrk.tx_intr_out) )
		{
			done = TRUE;
			p_dds->ofl.running = FALSE;
			i_enable(saved_intr_level); /* back to off level */
		}
	}

    }   /* end OFLV processing while loop */

    DEBUGTRACE2 ("OFFe",(ulong)0);
    return (0);
}

/*****************************************************************************/
/*
* NAME:     que_oflv
*
* FUNCTION: queue an interrupt for offlevel processing
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RETURNS:  FALSE if no problems (TRUE if offlevel queue is full)
*
*/
/*****************************************************************************/

int que_oflv (
	register offlevel_que_t *offl_ptr,  /* pointer to offlevel que struct */
	register offl_elem_t    *elem_ptr)  /* pointer to offlevel que elem   */
{
	register int tempndx;
	int saved_intr_level;

	/* que_oflv begin */
	DEBUGTRACE4 ("QUEb", (ulong)offl_ptr, (ulong)elem_ptr,
	(ulong)elem_ptr->who_queued);


	DISABLE_INTERRUPTS(saved_intr_level);
	/*
	 *  First check if this is to sched a
	 *  non-queueable interrupt ( i.e. TX or RECV )
	 *  If so, sched OFLV and exit.
	 */

	if (elem_ptr == NULL) {
		/* schedule off-level if necessary */
		if ((!offl_ptr->scheduled) && (!offl_ptr->running)) {
 			DEBUGTRACE1("ISC1");
			/* first item is struct intr */
 			i_sched ((struct intr *) offl_ptr); 
 			offl_ptr->scheduled = TRUE;
		}
	} else {
		if ((tempndx = offl_ptr->next_in+1) >= MAX_OFFL_QUEUED)
  			tempndx = 0;
		if (tempndx == offl_ptr->next_out) {
  			ENABLE_INTERRUPTS(saved_intr_level);
			/* que_oflv end (que overflow) */
  			TRACE2 ("QUE1", (ulong)TRUE); 
  			return (TRUE); /* queue overflow error return */
		} else {
  			offl_ptr->offl_elem[offl_ptr->next_in] = *elem_ptr;
  			offl_ptr->next_in = tempndx;

  			/* schedule off-level if necessary */
  			if ((!offl_ptr->scheduled) && (!offl_ptr->running)) {
				DEBUGTRACE1("ISC2");
				/* first item is struct intr */
     			i_sched ((struct intr *) offl_ptr); 
     			offl_ptr->scheduled = TRUE;
  			}
		}
	}   /* end else queueable request */

	ENABLE_INTERRUPTS(saved_intr_level);
	DEBUGTRACE2 ("QUEe", (ulong)FALSE);

	return (FALSE);
} /* end que_oflv */

/*****************************************************************************/
/*
* NAME:     cio_queofflms_func
*
* FUNCTION: timeout handler entered after delay specified with QUEOFFLMS
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RETURNS:  nothing
*
*/
/*****************************************************************************/

void cio_queofflms_func (
struct trb *systimer)          /* pointer to timer control structure */
{
register dds_t *p_dds;
offl_elem_t     offl_elem;

DEBUGTRACE2 ("TIMb",(ulong)systimer); /* cio_queofflms_func begin */

p_dds = (dds_t *) systimer->func_data; /* get argument from timer struct*/
DEBUGTRACE2 ("TIMd",(ulong)p_dds); /* cio_queofflms_func p_dds */

/* queue offlevel processing for timeout for this dds */
offl_elem.who_queued = OFFL_TIMO;
if (que_oflv (&(OFL), &offl_elem))
{
/* cio_queofflms_func ERROR (offl queue full) */
TRACE2 ("TIM1",(ulong)p_dds); 
RAS.ds.timo_lost++;
}

DEBUGTRACE1 ("TIMe"); /* cio_queofflms_func end */
return;
} /* end cio_queofflms_func */

/*****************************************************************************/
/*
* NAME:     cio_wdt_func
*
* FUNCTION: process watchdog timer interrupt
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RETURNS:  nothing
*
*/
/*****************************************************************************/

void cio_wdt_func (
register struct watchdog *wds_ptr) /* pointer to watchdog timer structure */
{
register dds_t *p_dds;
ulong           delta;
offl_elem_t     offl_elem;

DEBUGTRACE2 ("WDTb",(ulong)wds_ptr); /* cio_wdt_func begin */

p_dds = (dds_t *) 0; /* arbitrary reference point to compute delta */
delta = ((ulong) (&(WDT))) - ((ulong) (p_dds));
p_dds = (dds_t *) (((ulong) wds_ptr) - delta);
DEBUGTRACE2 ("WDTd",(ulong)p_dds); /* cio_wdt_func p_dds */

/* queue offlevel processing for watchdog timer timeout for this dds */
offl_elem.who_queued = OFFL_WDT;
if (que_oflv (&(OFL), &offl_elem))
{
TRACE2 ("WDT1", (ulong)p_dds); /* cio_wdt_func ERROR (offl que full) */
RAS.ds.wdt_lost++;
}

DEBUGTRACE1 ("WDTe"); /* cio_wdt_func end */
return;
} /* end cio_wdt_func */

/*****************************************************************************/
/*
* NAME:     cio_cdt_func
*
* FUNCTION: process component dump table interrupt
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RETURNS:  pointer to this driver's component dump table
*
*/
/*****************************************************************************/
cdt_t *cio_cdt_func (void)
{
return (&ciocdt);
}

/*
* NAME: ring_stat_pro
* 	Ring Status Processing 
*
* FUNCTION:
*
*      Process Ring Status adapter Interrupts.
*
* EXECUTION ENVIRONMENT:
*
*       This routine executes on the OFLV.
*
* NOTES:
*
*      This function determines if the ring status interrupt will require
*      the device driver to go into limbo mode.  If so, the enter_limbo() OR
*      cycle_limbo() function will be called to initiate the Network Recovery
*      scheme.
*
*      If not, the function determines if an adapter counter overflowed and if
*      one has, the Read adapter Error log command is issued to the adapter
*      via the issue_scb_cmd() primitive.
*
* (RECOVERY OPERATION:) Information describing both hardware and
*       software error recovery.
*
* (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
*
* RETURNS:
*      None.
*/
int
ring_stat_pro( dds_t *p_dds,
	   offl_elem_t *p_owe)

{
struct status_block sb; /* pecse fix */
int rc, reason, sil;
unsigned long errid;

/*
*   Determine if we need to log
*   the Alert.
*
*   NOTE:
*       We will not log the error if we
*       are cycling limbo.
*/

p_dds->wrk.ring_status = p_owe->stat0;	/* save the ring status */

errid = FALSE;
if ( (p_owe->stat0 & RS_SIGNAL_LOSS) ||
(p_owe->stat0 & RS_LOBE_WIRE_FAULT) )
errid = ERRID_TOK_WIRE_FAULT;
else if( p_owe->stat0 & RS_AUTO_REMOVE_1 )
errid = ERRID_TOK_AUTO_RMV;
else if( p_owe->stat0 & RS_REMOVE_RCVD )
errid = ERRID_TOK_RMV_ADAP2;
else if( p_owe->stat0 & RS_HARD_ERROR )
errid = ERRID_TOK_BEACON2;
else if( p_owe->stat0 & RS_SOFT_ERROR )
errid = ERRID_TOK_ESERR;
else if( p_owe->stat0 & RS_COUNTER_OVERFLW )
errid = ERRID_TOK_ESERR;


if (p_owe->stat0 & RS_REMOVE_RCVD)
{
/* We have been kicked off the ring by
 * the LAN manager.  We must shut the device 
 * down.
 */
p_dds->wrk.limbo_owe = *p_owe;
p_dds->wrk.footprint = OFLV_RSP_3;
logerr(p_dds, errid);

TRACE4("FooT", OFLV_RSP_3, (ulong)p_owe->stat0, 
	(ulong) p_dds->wrk.bugout);

rc = bug_out(p_dds, TOK_REMOVED_RECEIVED, p_owe->stat0);

} /* end if Remove received */
else if (p_owe->stat0 & RS_ENTER_LIMBO)
{
/*
*   We have an enter limbo condition.
*   Check the state of limbo.
*/
sil = i_disable(INTCLASS2);
if ( (p_dds->wrk.limbo != PARADISE) &&
(p_dds->wrk.limbo != NO_OP_STATE) )

{   /*
*   Double Jeopardy!
*   We have a valid limbo entry condition, BUT
*   we have not yet exited limbo.
*   Call cycle limbo to continue w/ recovery scheme
*/
TRACE3("FooT", OFLV_RSP_0, (ulong)p_owe->stat0);
rc = cycle_limbo(p_dds);
}   /* end if !PARADISE */
else if (p_dds->wrk.limbo == PARADISE)
{
/*
*   Get the reason for entering limbo
*/
switch (p_owe->stat0 & RS_ENTER_LIMBO)
{
   case RS_LOBE_WIRE_FAULT:
       reason=  TOK_LOBE_WIRE_FAULT;
       break;

   case RS_AUTO_REMOVE_1:
       reason = TOK_AUTO_REMOVE;
       break;

   default:
       reason = TOK_RING_STATUS;
	TRACE5("FooT", OFLV_RSP_4, (ulong)p_owe->cmd, 
		(ulong)p_owe->stat0, p_owe->stat1);
       break;

}  /* end switch */

p_dds->wrk.limbo_owe = *p_owe;
p_dds->wrk.footprint = OFLV_RSP_2;
if (errid != FALSE)
   logerr(p_dds, errid);

TRACE3("FooT", OFLV_RSP_2, (ulong)p_owe->stat0);
rc = enter_limbo(p_dds, reason, p_owe->stat0);

} /* end else enter limbo */

i_enable(sil);

}  /* end if enter limbo condition */
else
{
if (errid != FALSE)		/* we have some error to log */
{
/* Error logging thresholding checks for the
 * Ring Beaconing and Excessive soft errors.
 * We do this so as to not fill up the error log.
 */
if (errid == ERRID_TOK_BEACON2) 
{
	if (p_dds->wrk.rs_bcon_cnt == 0)
	{ 
		logerr(p_dds, errid);
		++p_dds->wrk.rs_bcon_cnt;
	}
	else if (++p_dds->wrk.rs_bcon_cnt > TOK_RS_BEACON2_THRESH)
		p_dds->wrk.rs_bcon_cnt = 0;
}
else if (errid == ERRID_TOK_ESERR)
{
	if (p_dds->wrk.rs_eserr_cnt == 0)
	{ 
		logerr(p_dds, errid);
		++p_dds->wrk.rs_eserr_cnt;
	}
	else if (++p_dds->wrk.rs_eserr_cnt > TOK_RS_ESERR_THRESH)
		p_dds->wrk.rs_eserr_cnt = 0;
}
else
	logerr(p_dds, errid);
} /* end if we have an error to log */


if (p_owe->stat0 & RS_COUNTER_OVERFLW)
	{

		TRACE3("FooT", OFLV_RSP_1, (ulong)p_owe->stat0);
		/*  Read the Adapter Error Log */
		
		/*
		 *  FUTURE FIX:
		 *	Add logic to read the adapter error log AND to
		 *	avoid a race condition with the SCB commands that 
		 *	may be issued on the process thread.
		 *
		 *  rc = issue_scb_command (p_dds, ADAP_ELOG_CMD,
		 *                          p_dds->wrk.p_d_errlog);
		 */

	} /* end if counter overflow */

	/* Determine if we need to build an Async. Status block 
	 */
	
       if ( ( ( p_owe->stat0 & 0xd820 ) ==0 )  &&
	    ( p_dds->wrk.bcon_sb_sent == TRUE ) )
       {
		/* Build the TOK_RING_RECOVERED status block
		 * send status block to all attached users 
		 */
			
		sb.code = CIO_ASYNC_STATUS;
		sb.option[0] = TOK_RING_STATUS;
		sb.option[1] = TOK_RING_RECOVERED;
		sb.option[2] = (unsigned int)p_owe->stat0;
		sb.option[3] = NULL;

		/* set the bcon_sb_sent flag to FALSE.
		 * this will allow us to send a status block
		 * the next time we get a ring beaconing condition 
		 */
			 
		p_dds->wrk.bcon_sb_sent = FALSE;

		/* send status block to all attached users */
		async_status(p_dds, &sb);
	}
	else if ( (p_owe->stat0 & RS_HARD_ERROR) &&
		  !(p_dds->wrk.bcon_sb_sent) )
	{
		/* Build the TOK_RING_BEACONING status block
		 * send status block to all attached users 
		 */
			 
		sb.code = CIO_ASYNC_STATUS;
		sb.option[0] = TOK_RING_STATUS;
		sb.option[1] = TOK_RING_BEACONING;
		sb.option[2] = (unsigned int)p_owe->stat0;
               	sb.option[3] = NULL;

		 /* set the bcon_sb_sent flag to TRUE so we will
		  * not send another status block until we get the
		  * Ring Recovered status from the adapter. 
		  */
			 
		p_dds->wrk.bcon_sb_sent = TRUE;

		/* send status block to all attached users 
		 */
		async_status(p_dds, &sb);

	}

	DEBUGTRACE1("RING");

}  /* end if NOT limbo entry condition */


 return(TRUE);
} /* end function ring_stat_pro */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*********************          COMMAND PRO          ******************/
/*                                                                    */
/*--------------------------------------------------------------------*/
int
cmd_pro(p_dds, p_oflv)      /* Off level processing Command */
   dds_t *p_dds;
   offl_elem_t *p_oflv;          /* off level work que element */
{
/* Declare Local Variables */
   int 			rc, sil;
   int			rejct;
   volatile t_scb	scb;
   volatile t_ssb	ssb;

switch (p_oflv->cmd)          /* Type of command being acknowledged */
{
   case ADAP_OPEN_CMD:                 /* OPEN command */
   {                                  /*  Check OPEN status */
       rc = open_adap_pend(p_dds, p_oflv);
       break;
   }  /* end ADAP_OPEN_CMD case */
   break;                                /* End case 0x0003 */

   case ADAP_CLOSE_CMD:                 /* CLOSE command */
   {                                   /* As A Result Of Detach */
                                        /* From Manager */
       tstop(p_dds->wrk.p_bringup_timer);

       p_dds->wrk.adap_state = CLOSED_STATE;     /* Reset Flags */

       /* p_dds->wrk.recv_mode = FALSE;     Set not in receive mode. */

       e_wakeup(&(p_dds->wrk.close_event));


   }
   break;                               /* End case 0x0007 */

   case ADAP_GROUP_CMD:                 /* Set Group Address */
   {
     /* Process adapter Group Address command */
	sil = i_disable(INTCLASS2);
       if (p_dds->wrk.limbo == LIMBO_GROUP)
           /* we're comming out of limbo */
           rc = egress_limbo( p_dds );
       else
       {   /* in response to a Set Goup Addr ioctl */
          /*
           *   Cancel the Group address failsafe timer.
           *   Wakeup the sleeping process.
           */
           if (p_dds->wrk.group_td.run)
           {
              tstop( p_dds->wrk.p_group_timer );
              p_dds->wrk.group_td.run = FALSE;
           }
           p_dds->wrk.group_wait = FALSE;
           p_dds->wrk.group_status = CIO_OK;
           if (p_dds->wrk.group_event != EVENT_NULL)
               e_wakeup( &p_dds->wrk.group_event );
       }
	i_enable(sil);

       break;
   } /* End case ADAP_GROUP_CMD */

   case ADAP_FUNCT_CMD:                 /* Set Functional Address */
   {
     /* Process adapter functional address command */

	sil = i_disable(INTCLASS2);

       if ( p_dds->wrk.limbo == LIMBO_FUNCTIONAL )
          /* we're coming out of limbo */
           rc = egress_limbo( p_dds );
       else
       {   /* in response to Set Func. Addr ioctl */
          /*
           *   Cancel the Functional address failsafe timer.
           *   Wakeup the sleeping process
           */
           if (p_dds->wrk.functional_td.run)
           {
              tstop( p_dds->wrk.p_func_timer );
              p_dds->wrk.functional_td.run = FALSE;
           }
           p_dds->wrk.funct_wait = FALSE;
           p_dds->wrk.funct_status = CIO_OK;
           if (p_dds->wrk.funct_event != EVENT_NULL)
               e_wakeup( &p_dds->wrk.funct_event );
       }
	i_enable(sil);

       break;
   } /* end case ADAP_FUNC_CMD */

   case ADAP_ELOG_CMD:          /* Read Error Log command */
   {
       /* move/copy the adapter error log counters to the RAS section of */
       /* the DDS via the d_kmove function */

       rc = d_kmove (&(p_dds->ras.ds.adap_err_log),
                     p_dds->wrk.p_d_errlog,
                     sizeof(tok_adap_error_log_t),
                     p_dds->wrk.dma_chnl_id,
                     p_dds->ddi.bus_id,
                     DMA_READ);

	if (rc == EINVAL) 	/* IOCC is NOT buffered */
           bcopy (p_dds->wrk.p_errlog,
       	      &(p_dds->ras.ds.adap_err_log),
              sizeof(tok_adap_error_log_t));

    /* rc = get_ring_info(p_dds); */
   }
   break;                      /* end case ADAP_ELOG_CMD */

   case ADAP_READ_CMD:                 /* READ Adapter Command */
   {
        rc = move_ring_info(p_dds);
	p_dds->wrk.ri_avail = TRUE;
          /*
           *   Cancel the Ring Information failsafe timer.
           *   Wakeup the sleeping process
           */
           if (p_dds->wrk.ring_info_td.run)
           {
              tstop( p_dds->wrk.p_ring_info_timer );
              p_dds->wrk.ring_info_td.run = FALSE;
           }
           p_dds->wrk.ring_info_wait = FALSE;
           p_dds->wrk.ring_info_status = CIO_OK;
           if (p_dds->wrk.ring_info_event != EVENT_NULL)
               e_wakeup( &p_dds->wrk.ring_info_event );

   }
   break;                       /* End 0x000B case  */


   case REJECT_CMD:                 /* REJECT command */
   {
        /*
        *  The CRS_ENTER_LIMBO defines for what Command Reject
        *  statuses do we enter limbo mode on.  They are:
        *      - Illegal Command
        *      - Address Error
        *      - Adapter Closed
        */

       TRACE4("FooT", OFLV_CMD_0, (ulong)p_oflv->stat0, (ulong)p_oflv->stat1);
        p_dds->wrk.footprint = OFLV_CMD_0;
        p_dds->wrk.afoot = (unsigned short) p_oflv->cmd;
       TRACE3("REJT", REJECT_CMD, p_oflv->cmd);

	rc = d_kmove (&scb, p_dds->wrk.p_d_scb, sizeof(scb),
		p_dds->wrk.dma_chnl_id, p_dds->ddi.bus_id, DMA_READ);

	if (rc == EINVAL) 	/* IOCC is NOT buffered */
	   bcopy (p_dds->wrk.p_scb, &scb, sizeof(scb));

	rc = d_kmove (&ssb, p_dds->wrk.p_d_ssb, sizeof(ssb),
		p_dds->wrk.dma_chnl_id, p_dds->ddi.bus_id, DMA_READ);

	if (rc == EINVAL) 	/* IOCC is NOT buffered */
	   bcopy (p_dds->wrk.p_ssb, &ssb, sizeof(ssb));

 	TRACE4("SCB1", scb.adap_cmd, scb.addr_field1, scb.addr_field2);
 	TRACE5("SSB1", ssb.c_type, ssb.stat0, ssb.stat1, ssb.stat2);

       if (p_oflv->stat0 & CRS_ENTER_LIMBO)
       {   /* We have a Limbo entry condition */

	   sil = i_disable(INTCLASS2);
           if (p_dds->wrk.limbo == PARADISE )
	   {
               /*
               *   Enter limbo
               */
		p_dds->wrk.limbo_owe = *p_oflv;
               rc = enter_limbo(p_dds, TOK_CMD_FAIL, p_oflv->stat0);
	   }
           else if (p_dds->wrk.limbo != NO_OP_STATE)
               /*
               *   Double Jeopardy!
               *   We have a valid limbo entry condition
               *   BUT we are not out of limbo yet.  Cycle the
               *   recovery scheme.
               */
               rc = cycle_limbo(p_dds);

	   i_enable(sil);

       }   /* end if go for limbo */

   }
   break;                        /* End Command reject */

   default:                             /* Unexpected or illegal */
   {
        TRACE4("FooT", OFLV_CMD_1, (ulong)p_oflv->cmd, (ulong)p_oflv->stat0);
   }
   break;                              /* command acknowledgment */
                                        /* Do nothing and get out. */
}            /* End Switch */
  return(TRUE);
} /* end function cmd_pro */




/*--------------------------------------------------------------------*/
/*                                                                    */
/*********************          Adapter Check        ******************/
/*                                                                    */
/*--------------------------------------------------------------------*/
int adap_chk(p_dds, p_oflv)     /* process adapter check */
   dds_t *p_dds;
   offl_elem_t *p_oflv;           /* off level work que element */
{

   int rc=0, sil;


       /*
       *   FUTURE FIX:
       *       Add adapter check thresholding logic for the following
       *       types of adapter checks:
       *           - illegal OP code
       *           - parity error (local processor bus)
       *           - parity error (external master)
       *           - parity error (SIF master)
       *           - parity error (PH master)
       *           - parity error (Ring Receive)
       *           - parity error (Ring Transmit)
       *           - invalid interrupt
       *           - invalid error interrupt
       *           - invalid XOP
       *           - program check
       *
       *       Thresholds may need to be added/deleted for others.
       */

   TRACE2("FooT", OFLV_ADAP_CHK);
   sil = i_disable(INTCLASS2);
   p_dds->wrk.footprint = OFLV_ADAP_CHK;
   if (p_dds->wrk.limbo == PARADISE)
       /*  We're off to Limboland! */
   {
	p_dds->wrk.limbo_owe = *p_oflv;
       logerr(p_dds, ERRID_TOK_ADAP_CHK);
       rc = enter_limbo(p_dds, TOK_ADAP_CHECK, p_dds->wrk.ac_blk.code);
   }   /* end if enter limbo */
   else if (p_dds->wrk.limbo != NO_OP_STATE)
       /* Double Jeopardy! */
       rc = cycle_limbo(p_dds);

  i_enable(sil);
  return(rc);
  } /* end function adapter check */
/*
 * NAME: oflv_ioctl_to
 *
 * FUNCTION:
 *      Handles the timeout of an ioctl function call.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the OFLV.
 *
 * NOTES:
 *      This function is scheduled (via the bringup_timer() function)
 *      after an ioctl watch dog timer has popped.
 *
 * RECOVERY OPERATION:
 *      None.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 *  INPUT:      p_dds - pointer to DDS
 *              p_owe - pointer to OFLV work element
 *
 *
 * RETURNS:
 *      None.
 */
int
oflv_ioctl_to(dds_t *p_dds, offl_elem_t *p_owe)
{  /* begin function */

   unsigned int    rc=0;

  /*
   *   Determine what ioctl command timed out.
   *   Take the appropriate action.
   */

   switch(p_owe->cmd)
   {
       case IOCTL_FUNC:
       {
           if (p_dds->wrk.funct_wait)
           {
                TRACE2("FooT", OFLV_IOCTL_0);
                p_dds->wrk.funct_wait = FALSE;
                p_dds->wrk.funct_status = CIO_TIMEOUT;
                if (p_dds->wrk.funct_event != EVENT_NULL)
                    e_wakeup( &(p_dds->wrk.funct_event) );
                break;
           }
       }
       case IOCTL_GROUP:
       {
           if (p_dds->wrk.group_wait)
           {
                TRACE2("FooT", OFLV_IOCTL_1);
                p_dds->wrk.group_wait = FALSE;
                p_dds->wrk.group_status = CIO_TIMEOUT;
                if (p_dds->wrk.group_event != EVENT_NULL)
                    e_wakeup( &(p_dds->wrk.group_event) );
                break;
           }
       }
       case IOCTL_RING_INFO:
       {
           if (p_dds->wrk.ring_info_wait)
           {
                TRACE2("FooT", OFLV_IOCTL_3);
                p_dds->wrk.ring_info_wait = FALSE;
                p_dds->wrk.ring_info_status = CIO_TIMEOUT;
                if (p_dds->wrk.ring_info_event != EVENT_NULL)
                    e_wakeup( &(p_dds->wrk.ring_info_event) );
                break;
           }
       }
       default:
       {
          /* do nothing */
                TRACE2("FooT", OFLV_IOCTL_2);
           break;
       }
   }   /* end switch */

return(rc);

}  /* end function oflv_ioctl_to */
