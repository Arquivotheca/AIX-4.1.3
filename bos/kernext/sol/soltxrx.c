static char sccsid[] = "@(#)55	1.11  src/bos/kernext/sol/soltxrx.c, sysxsol, bos411, 9428A410j 4/17/92 10:11:39";

/*
 * COMPONENT_NAME: (SYSXSOLDD) Serial Optical Link Device Driver 
 *
 * FUNCTIONS: 
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

#include <sys/param.h>		/* PAGESIZE */
#include <sys/intr.h>		/* for i_enable, i_disable */
#include <sys/syspest.h>	/* for ASSERT */
#include <sys/trcmacros.h>	/* tracing stuff */
#include <sys/trchkid.h>	/* trace hook ids */
#include <sys/time.h>		/* for timestruc_t */
#include <sys/timer.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/ioacc.h>

#include "soldd.h"

#include "sol_proto.h"
#include "sol_extrn.h"

extern struct sol_ddi sol_ddi;
extern void * timer_sp[MAX_NUM_SLA];
extern int poll_cnt[MAX_NUM_SLA];

extern vm_cinval();
extern void tservice();

/**/
/*
 * NAME:  sla_fatal
 *                                                                    
 * FUNCTION:  This function is called to schedule recovery action whenever
 *		unexpected conditions occur
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function executes both from process and interrupt level.
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *	the sla_tbl structure is modified heavily to reflect the mode
 *	of recovery 
 *
 * RETURNS: NONE
 */  

void
sla_fatal( SLAIOP(slap),
	int sla_id,
	int mode,
	int index)
{
	uint s1_val;
	int sec,nsec;
	TVARS tvar;

	STOP_TIMER(sla_tbl.sla[sla_id].time_struct);

	sec =0;
	nsec = ONE_MILLI;
	tvar.setup=TRUE;
	tvar.id = sla_id;
	tvar.func =0;

	sla_tbl.sla[sla_id].ccr_con = 0;  /* reset the point to point discard,
		                                     and retry bits */
	sla_tbl.sla[sla_id].event_count = 0;
	sla_tbl.sla[sla_id].s1_save = 0;
	sla_tbl.sla[sla_id].connection = 0;
	sla_tbl.sla[sla_id].ack_type = 0;

	sla_scr(sla_id) = FALSE;
	sla_sdma_wait(sla_id) = FALSE;

	switch (sla_tbl.sla[sla_id].sla_st) {
	case SLA_IDENT_CODE :
		primary_hdr(sla_id) = NULL_HDR;
		sla_error(sla_id, mode); /* this should alert the widzard */
		INIT_TAGS(slap);  /* reset the tag array, just to
		                     ensure that we won't write in this
				     memory when the channel starts again */
		break;
	case SLA_NOTIDENT_CODE :
		ala_state(sla_id) = 0;
		break;
	default :
		break;	
	}  /* end switch in sla state */

	sla_tbl.sla[sla_id].sla_st = SLA_NOTSYNC_CODE;

	s1_val = slap -> sample;

	SYS_SOL_TRACE("fatl",mode,index,s1_val);
	switch(mode) {

	case OFF_ERROR :  /* ols detected */
	case NOP_ERROR :  /* nos detected */
		sla_tbl.sla[sla_id].sla_st = SLA_POLLMODE_CODE;
		tvar.func=RESTART_CONT;
		if (PUISTATE(s1_val) != PUI_NOOP) {
			if(mode == NOP_ERROR) 
				sla_xmt_cont(slap, sla_id,OFF_SEQ_ADDR);
			else
				sla_xmt_cont(slap, sla_id,UD_SEQ_ADDR);
			sec=POLL_SEC;
			nsec=POLL_NSEC;
		}
		break;

	case DRV_ERROR :  /* driver failure */
	case SIG_ERROR :  /* signal failure */
		if (PUISTATE(s1_val) == PUI_NOOP) {
                        tvar.func=RESTART_SYNC;
		}
		else
			sla_sync(slap, sla_id);
		break;

	case CHANNEL_CHECK :  /* parity checks */
		INIT_TAGS(slap);
		INIT_REGS(slap);
		sla_tbl.sla[sla_id].sla_st = SLA_POLLMODE_CODE;
                tvar.func=RESTART_CONT;
		break;

	case BSY_ERROR :  	/* channel keeps receiveing busy frames */
	case DID_NOT_START :  	/* channel would not start */
				/* this sometimes happens, in very noisy 
				conditions send offlines for a full second */
		sla_tbl.sla[sla_id].sla_st = SLA_POLLMODE_CODE;
		sla_xmt_cont(slap, sla_id,OFF_SEQ_ADDR);
		sec=1;
		nsec=0;
		tvar.func=RESTART_CONT;
		break;

	case ALA_RECEIVED :  	/* received an ala in identified state */
				/* go back to the not-identified state */
		sla_tbl.sla[sla_id].sla_st = SLA_NOTIDENT_CODE;
		break;

	case SLA_STOPPED :  /* stop request on sla */
		if (! sla_no_ODC(sla_id) && PUISTATE(s1_val) != PUI_NOOP)
/*			sla_xmt_cont(slap, sla_id,OFF_SEQ_ADDR); */
		sla_started(sla_id) = FALSE;
		break;

	case DIAG :  /* diagnostic request */
		sla_tbl.sla[sla_id].sla_st = SLA_DIAGNOSTIC_CODE;
		INIT_REGS(slap);
		if (! sla_no_ODC(sla_id) && PUISTATE(s1_val) != PUI_NOOP)
			sla_xmt_cont(slap, sla_id,OFF_SEQ_ADDR);
		break;

	default :
		break;

	}  /* end switch */
	if(tvar.func) 
		SLA_TIMER(sla_id,tvar,tservice,sec,nsec);
}  

/**/
/*
 * NAME: sla_nop_start
 *                                                                    
 * FUNCTION: recovery function when starts are requested on a non operational
		channel 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: SLAS_FATAL - hopefully, otherwise we painic
 */  

static int
sla_nop_start( SLAIOP(slap),
	int sla_id)
{

	uint s1_val, s2_val;
	s1_val = slap -> status1;
	s2_val = slap -> status2;
SOL_TRACE("nops",sla_id,s1_val,s2_val);

	if (SIGNAL_FAILURE(slap, s2_val) ) 
		sla_fatal(slap, sla_id, SIG_ERROR, LCC_SIG_FAIL);
	else if (s2_val & NOPSQ_REC) 
		sla_fatal(slap, sla_id, NOP_ERROR, LCC_NOPSQ_RC);
	else if (s2_val & OFFSQ_REC) 
		sla_fatal(slap, sla_id, OFF_ERROR, LCC_OFFSQ_RC);
	else if (s1_val & CHAN_CHK) 
		sla_channel_check(slap, sla_id, s2_val);
        else {
		OPERATE_CHANNEL(slap, sla_id, s1_val, s2_val);
        	/* channel must now be in the stopped state */
        	if (PUISTATE(s1_val) != PUI_STOPPED) {
               		sla_tbl.sla[sla_id].imcs_st = SLA_BROKEN_CODE;
			return SLAS_FATAL;
		}
		else {
			if( BUS_GETLX((long *) &slap->ch_start, &s1_val)) {
               			sla_tbl.sla[sla_id].imcs_st = SLA_BROKEN_CODE;
				return SLAS_FATAL;
			}
			else return SLAS_DONE;
		}
	}		
	return SLAS_FATAL;

}  /* end sla_nop_start */

/**/
/*
 * NAME: sla_start 
 *                                                                    
 * FUNCTION: start the channel for normal operation
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS:	SLAS_DONE 	start successful 
 *		SLAS_FATAL 	sla_fatal called to do recovery
 *		SLAS_DISCARDED  did not start retry
 *		SLAS_SCR_PROGRESS scr in progress	
 *		SLAS_SCR_DONE	scr completed retry start
 *		SLAS_UNEXPD	unexpected channel return state unknown
 */  

int
sla_start( SLAIOP(slap),
	int sla_id,
	uint ccr_value)
{

	if(ccr_value ) slap->ccr = ccr_value;
	CHECK_REG(slap);

	if (PUISTATE(slap -> sample) == PUI_STOPPED) {

		uint s1_val, s2_val;

		if( BUS_GETLX((long *) &slap->ch_start, &s1_val)) 
			return(sla_nop_start(slap,sla_id));
 
		if ( (s1_val & 0xF7000000) == 0) {
			ASSERT(PUISTATE(s1_val)== PUI_WORK1||(s1_val&OP_DONE));
			return SLAS_DONE;
		}

		s2_val = slap -> status2;

		if (PUISTATE(s1_val) != PUI_STOPPED &&
		    PUISTATE(s1_val) != PUI_NOOP) {
			if (s1_val & UNEXPD) {
				sla_tbl.sla[sla_id].s1_save=s1_val&S1_BIT_MASK;
				return SLAS_UNEXPD;
			}
			else if (s1_val & SCR_ACTV)
				return SLAS_SCR_PROGRESS;
			else if (s1_val & (LINK_CHK | FRM_DISC))
				return SLAS_DONE;
			else {
				PANIC("sla_fstart: channel running");
			}
		}  /* end channel not stopped */

		if (s1_val & CMD_RJT) {
			if (s1_val & LINK_CHK) {
				if (SIGNAL_FAILURE(slap, s2_val) ) {
					sla_fatal(slap, sla_id, SIG_ERROR, 
								LCC_SIG_FAIL);
					return SLAS_FATAL;
				}
				else if (s2_val & NOPSQ_REC) {
					sla_fatal(slap, sla_id, NOP_ERROR, 
								LCC_NOPSQ_RC);
					return SLAS_FATAL;
				}  /* end NOPSQ_REC */
				else if (s2_val & OFFSQ_REC) {
					sla_fatal(slap, sla_id, OFF_ERROR, 
								LCC_OFFSQ_RC);
					return SLAS_FATAL;
				}  /* end OFFSQ_RQ  */
				else if (s1_val & (FRM_DISC | BSY_DISC | 
								RJT_DISC)) {
					return SLAS_DISCARDED;
				}   /* end frame rejected */
				else if (s2_val & ADDR_MIS) {
	  	  /* this case is not in the architecture but we see it
	           under noisy conditions.  Noise was mistaken for a
	           frame, and since there was no frame, the address
	           was incorrect.  We treat this as a frame discarded */
					return SLAS_DISCARDED;
				}
				else {
					return SLAS_DISCARDED;
				}
			}  /* end LINK_CHK */
			else if (s1_val & SCR_DONE) {
			/* while we thinking about starting the channel, 
			the channel did a scr by itself (thanks to the auto 
			scr bit) note, however, that we are not necessarily 
			seeing ud. Possible explanation:  We decided to do 
			SCR, and at the same time the other side is doing SCR.*/
				return SLAS_SCR_DONE;
			}  /* end SCR_DONE */
			else if (s1_val & SCR_ACTV) {
			/* we are seeing UD's, because the other side started a
			 SCR again we might not see ud's in status2, since we 
			might have reset s2 when it was read in the interrupt 
			handler */
				sla_scr(sla_id) = FALSE;
				sla_do_scr(slap, sla_id);
				return SLAS_SCR_PROGRESS;
			} /* end SRC_ACTV */
			else {
                                sla_fatal(slap,sla_id, SIG_ERROR,LCC_SIG_FAIL);
                                return SLAS_FATAL;
                        }
                } /* end CMD_RJT */
                else if (s1_val & CHAN_CHK) {
                        sla_channel_check(slap, sla_id, s2_val);
                        return SLAS_FATAL;
                }
                else if (s1_val & CMD_RJT2 ) {
                        sla_fatal(slap, sla_id, SIG_ERROR,LCC_SIG_FAIL);
                        return SLAS_FATAL;
                }  /* end CMD_RJT2 */
                else if (s1_val & UNEXPD) {
                        sla_tbl.sla[sla_id].s1_save = s1_val & S1_BIT_MASK;
                        return SLAS_UNEXPD;
                }
                else if (s1_val & LINK_CHK) {
                        if (s2_val & OFFSQ_REC) {
                                sla_fatal(slap, sla_id,OFF_ERROR, LCC_OFFSQ_RC);
                                return SLAS_FATAL;
                        }  /* end OFFSQ_RQ  */
                }  /* end link check alone */
                else {
                        sla_fatal(slap, sla_id, SIG_ERROR, LCC_SIG_FAIL);
                        return SLAS_FATAL;
                }
        }
        else {
                /* channel went not operational while it was stopped */
                return sla_nop_start(slap, sla_id);
        }

        sla_fatal(slap, sla_id, SIG_ERROR, LCC_SIG_FAIL);
        return SLAS_FATAL;

}  /* sla_start */

/*
 * NAME: sla_pstart
 *                                                                    
 * FUNCTION: priority channel start
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 	SLAS_DONE	start succeded
 *		SLAS_FATAL 	sla_fatal called for scheduling
 */  

int
sla_pstart( SLAIOP(slap),
	int sla_id,
	uint ccr_value)
{
	int tmp;

	uint s1_val;
	slap -> ccr = ccr_value;

	ASSERT(ccr_value & PRIORITY);
	tmp=slap->sample;
	SOL_TRACE("psrt",sla_id,tmp,ccr_value);

	if( BUS_GETLX((long *) &slap->ch_start, &s1_val)) 
			return(sla_nop_start(slap,sla_id));
	else 
		return SLAS_DONE;

}  /* sla_pstart */


/**/
/*
 * NAME: sla_set_active
 *                                                                    
 * FUNCTION: write the channel program to the header 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void 
sla_set_active( struct imcs_header * h)
{
	h -> imcs_linkctl = DEVICE_CTL_DATA;

	if (h -> IMCS_OP_CODE == IMCS_RECEIVE_CODE)
		h -> imcs_devctl = (ushort) (DEV_REQ_RCV >> 16);
	else
		if (h -> IMCS_AUTOMATIC)
			h -> imcs_devctl = (ushort) (DEV_REQ_SEND_IMM >> 16);
		else
			h -> imcs_devctl = (ushort) (DEV_REQ_SEND >> 16);

	h -> imcs_ccr = XMT_CON_CODE | AUTO_SEQ;
}  /* end sla_set_active */


/**/
/*
 * NAME: start_send_auto
 *                                                                    
 * FUNCTION: start an automatic sequence send
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *	 the message addressed by the active field of the sla structure is used
 *
 *       rtsi send : a device ctl/request to send immediate frame, with
 *               B register set to the imcs queue id is sent.
 *       rts send : a device ctl/request to send frame with subchannel
 *               linkage field set to the imcs subchannel is sent.
 *       rtr receive : a device ctl/request to receive frame with suchannel
 *               linkage field set to the imcs subchannel is sent
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
start_send_auto( SLAIOP(slap),
	int sla_id)
{
	int start_try;
	boolean_t done;
	uint s1_val;
	struct sl_frame thr_val;


	CHECK_REG(slap);

	SYS_SOL_TRACE("snda",sla_id,active_hdr(sla_id),0);
	#ifdef PREFRAME
	sla_xmt_sof(slap, sla_id);  /* load registers to send the sof char */
	for (start_try = 0, done = FALSE;
	    start_try < MAX_START_TRY && ! done;
	    start_try++)
		switch (sla_start(slap, sla_id, sla_tbl.sla[sla_id].ccr_all |
		    TRANSPARENT | SXMT) ) {
		case SLAS_DONE :
			done = TRUE;
			s1_val = sla_spin(slap, sla_id);
			/* check if channel actually stopped */
			if (ANYCHK(s1_val)) {
			/* the frame was sent, but there was an error.  
			The dma contains the preframe */
				sla_sdma_unh(slap, sla_id);
				sla_scr(sla_id) = TRUE;
				START_SCR(slap, sla_id);
				return;
			}
			break;
		case SLAS_FATAL :
		case SLAS_SCR_PROGRESS :
		case SLAS_UNEXPD :
			return;
			break;
		case SLAS_DISCARDED :
		case SLAS_SCR_DONE :
			/* in these cases we retry */
			break;
		default : 
			PANIC("start_send_auto: unknown return from sla_xmt_sof");
		}  /* end switch */

	if (! done) {
		/* the channel would not start */
		sla_fatal(slap, sla_id, DID_NOT_START, OTH_NO_START);
		return;
	}
	#endif PREFRAME

	primary_hdr(sla_id) = active_hdr(sla_id);

	primary_hdr(sla_id) -> imcs_address = sla_address(sla_id);
	primary_hdr(sla_id) -> imcs_ccr |= sla_tbl.sla[sla_id].ccr_con;

	cprgs(&(primary_hdr(sla_id) -> imcs_address),
	    &(slap -> thr.fr), SLA_CHPR_SZ);

	for (start_try = 0, done = FALSE;
	    start_try < MAX_START_TRY && ! done;
	    start_try++)
		switch (sla_start(slap, sla_id,0) ) {
		case SLAS_DONE :
		case SLAS_FATAL :
		case SLAS_UNEXPD :
		/* this is another case of collision where the other side
	         connect frame reached us at the exact time that the
	         channel was started.  There should be an interrupt
	         pending for this message. */
			done = TRUE;
			break;
		case SLAS_SCR_PROGRESS :
		/* src is in progress and sla_start started our side to in 
	         SCR mode.  When SCR finished there will be an interrupt, 
	         and we will send the message then.  However, there will
	         be no need to do dma recovery */
			primary_hdr(sla_id) = NULL_HDR;
			done = TRUE;
			break;
		case SLAS_DISCARDED :
		case SLAS_SCR_DONE :
			break;
		default :
			PANIC("start_send_auto: unknown return from sla_start");
		}  /* end switch */

	if (! done) /* the channel would not start */
		sla_fatal(slap, sla_id, DID_NOT_START, OTH_NO_START);

}  /* end start_send_auto */


/**/
/*
 * NAME: sla_send
 *                                                                    
 * FUNCTION:    try to send or receive the message addressed by active
 *               field of the sla structure.  
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *		This routine must be called while interrupts from the 
 *		sla are masked.
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 	TRUE 	if the message was sent or will be sent 
 *			in the interrupt handler.
 *               FALSE otherwise 
 */  

int sla_send(int sla_id)
{
	int rc;
	uint s1_val;
	SLAIOP(slap);
	TVARS tvar;
	if(active_hdr(sla_id) == NULL_HDR)
		return FALSE;

	ADDR_SLA(sla_id,slap);


	if (sla_inter(sla_id) )
		/* channel is currently in the interrupt handler.  The interrupt
	     handler will send this packet at interrupt end */
		rc = TRUE;
	else if (sla_scr(sla_id) )
		/* the channel is doing recovery.  Better try another sla */
		rc = FALSE;
	else {
		/* try to stop the channel.  A channel stop alone has
	     caused a DSI, because the channel was no-op.  Therefore
	     the test.  If the channel is no-op than one hope link
	     check will be on, and one further hopes thet an interrupt
	     will be pending.  If link check is set then this is
	     included in the interruptable condition, which also takes
	     care of interrupts not happening. */
		if (PUISTATE(slap -> sample) != PUI_NOOP)
			BUS_GETLX((long *) &slap -> ch_stop,&s1_val);/* try to stop channel */
		else
			s1_val = slap -> sample;
		if (s1_val & CMD_PEND) {
			/* the channel did not stop.  Better try another sla */
			rc = FALSE;
			ASSERT(PUISTATE(s1_val) == PUI_WORK2);
			/* save the bit returned by stop */
			sla_tbl.sla[sla_id].s1_save = s1_val & S1_BIT_MASK;
		}
		else {  /* channel stopped */
		/* we set rc to true even is there is the interrupt pending
	       because, even though this means that we cannot send,
	       we have to start a timer for this condition.  If we do not
	       make this sla busy, and we return here for some other
	       message we will ASSERT in SLA_TIMER */
			rc = TRUE;
			if (s1_val & INTERRUPT_PENDING) {
				/* the channel stopped on its own, and the 
				interrupt is pending */
				sla_tbl.sla[sla_id].s1_save=s1_val& S1_BIT_MASK;
				tvar.func=RESTART_INT;
				tvar.setup=FALSE;
				tvar.id=sla_id;
				SLA_TIMER(sla_id, tvar,tservice,0,ONE_MILLI);
			}
			else {
				start_send_auto(slap, sla_id);
			}
		}
	} /* end not interrupting and not doing scr */
	/* otherwise either send was called from the interrupt handler,
	   or the send will be done when scr completes */

	UNADDR_SLA(slap);

	SYS_SOL_TRACE("send",rc,sla_id,active_hdr(sla_id));
	return rc;

}  /* end sla_send */


/**/
/*
 * NAME:  start_listen
 *                                                                    
 * FUNCTION: start the channel in listen mode
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
start_listen(slap, sla_id)
SLAIOP(slap);
int sla_id;
{
	int start_try;
	uchar done;

SOL_TRACE("slis",sla_id,0,0);
	CHECK_REG(slap);
	INIT_TAGS(slap);  /* set all tags to invalid to avoid dma screw ups */

	for (start_try = 0, done = FALSE;
	    start_try < MAX_START_TRY && ! done;
	    start_try++)
		switch (sla_start(slap, sla_id, LISTEN_CODE |
		    sla_tbl.sla[sla_id].ccr_all) ) {
		case SLAS_DONE :
		case SLAS_FATAL :
		case SLAS_UNEXPD :
		/* the other side connect frame reached us at the exact
	         time that the channel was started.  There should be
	         an interrupt pending */
		case SLAS_SCR_PROGRESS :
			/* src is in progress and sla_start started our side
	         to in SCR mode.  When SCR finished there will be
	         an interrupt, and we will put the channel in receive
	         mode then.  However, there will be no need to do dma
	         recovery */
			done = TRUE;
			break;
		case SLAS_DISCARDED :
		case SLAS_SCR_DONE :
			break;
		default :
			PANIC("start_send_auto: unknown return from sla_start");
		}  /* end switch */

	if (! done) {
		/* the channel would not start */
		sla_fatal(slap, sla_id, DID_NOT_START, OTH_NO_START);
	}

}  /* end start_listen */


/**/
/*
 * NAME: sla_set_passive
 *                                                                    
 * FUNCTION: writes the channel program to the header 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
sla_set_passive( struct imcs_header * h)
{
	h -> imcs_linkctl = DEVICE_CTL_DATA;

	if (h -> IMCS_OP_CODE == IMCS_RECEIVE_CODE)
		/* this generates a sth instruction */
		h -> imcs_devctl = (ushort) (DEV_RESP_SEND >> 16);
	else
		h -> imcs_devctl = (ushort) (DEV_SEND >> 16);

	WRT_B_REG(h, 0);  /* this generates a st instruction */
	h -> imcs_ccr = XMT_AUTO_CODE;
}  /* end sla_set_passive */


/**/
/*
 * NAME:  start_rcv_auto
 *                                                                    
 * FUNCTION: start the rtsi receive
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
start_rcv_auto( SLAIOP(slap),
	int sla_id)
{
	int start_try;
	boolean_t done;

SOL_TRACE("rxau",sla_id,0,0);
	CHECK_REG(slap);
	primary_hdr(sla_id) = passive_hdr(sla_id);
	primary_hdr(sla_id) -> imcs_ccr = LISTEN_CODE | AUTO_SEQ |
	    sla_tbl.sla[sla_id].ccr_con;

	cprgs(&(primary_hdr(sla_id) -> imcs_ccr),
	    &(slap -> ccr), SLA_CCR_SZ + SLA_TCWS_SZ);

	for (start_try = 0, done = FALSE;
	    start_try < MAX_START_TRY && ! done;
	    start_try++)
		switch (sla_start(slap, sla_id,0) ) {
		case SLAS_DONE :
		case SLAS_FATAL :
		case SLAS_UNEXPD :
			done = TRUE;
			break;
		case SLAS_SCR_PROGRESS :
			primary_hdr(sla_id) = NULL_HDR;
			done = TRUE;
			break;
		case SLAS_DISCARDED :
		case SLAS_SCR_DONE :
			break;
		default :
			PANIC("start_rcv_auto: unknown return from sla_start");
		}  /* end switch */

	if (! done)  /* the channel would not start */
		sla_fatal(slap, sla_id, DID_NOT_START, OTH_NO_START);

}  /* end start_rcv_auto */


/**/
/*
 * NAME: sla_rcv_rtsi
 *                                                                    
 * FUNCTION: tries to start the channel in listens immediate mode
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void sla_rcv_rtsi(int sla_id)
{
	uint s1_val;
	SLAIOP(slap);
	TVARS tvar;
	ASSERT(passive_hdr(sla_id) != NULL_HDR);
	if (primary_hdr(sla_id) != NULL_HDR) return;
SOL_TRACE("rxsi",sla_id,0,0);
	ADDR_SLA(sla_id,slap);

	if (! sla_inter(sla_id) && ! sla_scr(sla_id) ) {
		BUS_GETLX((long *) &slap -> ch_stop,&s1_val); /* try to stop the channel */
		if (s1_val & CMD_PEND) {
			ASSERT(PUISTATE(s1_val) == PUI_WORK2);
			/* save the bit returned by stop */
			sla_tbl.sla[sla_id].s1_save = s1_val & S1_BIT_MASK;
		}
		else {  /* channel stopped */
			if (s1_val & INTERRUPT_PENDING) {
				/* the channel stopped on its own, 
				and the interrupt is pending */
				sla_tbl.sla[sla_id].s1_save=s1_val&S1_BIT_MASK;
                                tvar.func=RESTART_INT;
                                tvar.setup=FALSE;
                                tvar.id=sla_id;
                                SLA_TIMER(sla_id, tvar,tservice,0,ONE_MILLI);
			}
			else {
				ASSERT(PUISTATE(s1_val) == PUI_STOPPED);
				ASSERT(primary_hdr(sla_id) == NULL_HDR);
				start_rcv_auto(slap, sla_id);
			}
		}
	} /* end not interrupting and not doing scr */
	/* otherwise rcv_rtsi was called from the interrupt handler */

	UNADDR_SLA(slap);

}  /* end sla_rcv_rtsi */


/**/
/*
 * NAME:  xmt_ala
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
xmt_ala( SLAIOP(slap),
	int sla_id)
{
	int start_rc, start_try;
	boolean_t done;
	struct sl_frame thr_val;


	if (sla_tbl.sla[sla_id].connection == POINT_TO_POINT) {
		uint address;
		address = (uint) imcs_find_slaa(sla_id);
		thr_val.did_sid = (address << 8) | ( (address - 1) << 24);
		ala_state(sla_id) = ALA_SENT;
	}
	else {
		thr_val.did_sid = SWITCH_ALA_ADDRESS;
		ala_state(sla_id) = ALA_SENT_TO_SWITCH;
	}

	/* put ala message in the thr */
	thr_val.link_ctl = LINK_REQUEST_ALA;
	thr_val.w3.info = THR_DONT_CARE;
	thr_val.w4.info = THR_DONT_CARE;
	thr_val.w5.count = 0;

	WRITE_THR(slap, thr_val);

	for (start_try = 0, done = FALSE;
	    start_try < MAX_START_TRY && ! done;
	    start_try++)
		switch (start_rc = sla_start(slap, sla_id, XMT_CON_CODE |
		    RTV_4 |
		    sla_tbl.sla[sla_id].ccr_all) ) {
		case SLAS_DONE :
		case SLAS_FATAL :
		case SLAS_SCR_PROGRESS :
		case SLAS_UNEXPD :
			done = TRUE;
			break;
		case SLAS_DISCARDED :
		case SLAS_SCR_DONE :
			break;
		default: 
			PANIC("send_ala: unknown return from sla_start");
		}  /* end switch */

	if (! done)
		if (SOLIDLY_ON(slap, slap -> status2, LINK_SIG_ERR) )
			/* the light went off */
			sla_fatal(slap, sla_id, SIG_ERROR, LCC_SIG_FAIL);
		else
			sla_fatal(slap, sla_id, DID_NOT_START, OTH_NO_START);

}  /* end xmt_ala */

/**/
/*
 * NAME: sla_send_ala
 *                                                                    
 * FUNCTION: try to send an ala frame
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  


void
sla_send_ala( SLAIOP(slap),
	int sla_id)
{
	uint s1_val;

	s1_val = slap -> sample;

	if (PUISTATE(s1_val) == PUI_WORK1 && LINKSTATE(s1_val) == LI_LISTEN) {
		/* avoid stopping the channel if already running */
		BUS_GETLX((long *) &slap -> ch_stop,&s1_val);/* try to stop channel */
		if (s1_val & CMD_PEND) {
			ASSERT(PUISTATE(s1_val) == PUI_WORK2);
			/* save the bit returned by stop for on op completed */
			sla_tbl.sla[sla_id].s1_save = s1_val & S1_BIT_MASK;
		}
		else {  /* channel stopped */
			ASSERT(PUISTATE(s1_val) == PUI_STOPPED);
			xmt_ala(slap, sla_id);
		}  /* end channel stopped */
	}  /* end channel listening */

}  /* end sla_send_ala */


/**/
/*
 * NAME: sla_poll
 *                                                                    
 * FUNCTION: handles sla special error recovery, e. g., transmitting
 *           OLS and waiting for UD.
 *
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *       this is used when the sla has failed, and sla_fatal has already
 *       been called to clean up and initiate recovery.
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
sla_poll( SLAIOP(slap),
	int sla_id)
{
	uint s1_val, s2_val, s2_val1;
	int new_idle_count;
	TVARS tvar;
	ASSERT(sla_tbl.sla[sla_id].sla_st == SLA_POLLMODE_CODE);
	ASSERT(primary_hdr(sla_id) == NULL_HDR);
	ASSERT(active_hdr(sla_id) == NULL_HDR);
	ASSERT(passive_hdr(sla_id) == NULL_HDR);

        tvar.func=RESTART_CONT;
        tvar.setup=TRUE;
        tvar.id=sla_id;
	s1_val = slap -> sample;

SOL_TRACE("spl0",sla_id,s1_val,0xFFFFFFFF);

	switch (PUISTATE(s1_val)) {
	case PUI_NOOP :
		OPERATE_CHANNEL(slap, sla_id, s1_val, s2_val);
		if (sdma(sla_id)) {
			sdma(sla_id) = FALSE;
			sla_sdma_unh(slap, sla_id);
			s1_val = slap -> sample;
		}
		if (rdma(sla_id)) {
			rdma(sla_id) = FALSE;
			sla_rdma_unh(slap, sla_id);
			s1_val = slap -> sample;
		}
		break;

	case PUI_STOPPED :
		break;

	case PUI_WORK1 :
		BUS_GETLX((long *) &slap -> ch_cancel,&s1_val);/* try to stop channel */
		break;

	default :
		PANIC("unexpected pui state in sla poll");
	}   /* end switch */

	if (PUISTATE(s1_val) != PUI_STOPPED) {
		sla_tbl.sla[sla_id].imcs_st = SLA_BROKEN_CODE;
		return;
	}

	s2_val1 = slap -> status2;  /* read s2 twice */
	s2_val = slap -> status2;

	s2_val |= s2_val1; /* or the values to make NSC box happy */


	new_idle_count = 0;  /* this is the default new value */

SOL_TRACE("spl1",sla_id,s1_val,s2_val);
	if (s2_val & XMT_FAIL) {
		sla_tbl.sla[sla_id].sla_st = SLA_NOTSYNC_CODE;
		sla_sync(slap, sla_id);
	}
	else if (s2_val & SIG_FAIL) {
		if(poll_cnt[sla_id]++ > 10) 
		sla_tbl.sla[sla_id].sla_st = SLA_NOTSYNC_CODE;
		sla_sync(slap, sla_id);
	}
	else if (s2_val & (TAGPARITY | BUFPARITY | STORCHK)) {
		INIT_TAGS(slap);
		INIT_REGS(slap);
		SLA_TIMER(sla_id, tvar,tservice, 0, ONE_MILLI);
	}
	else if (s2_val & NOPSQ_REC) {
		if(poll_cnt[sla_id]++ > 10) 
		sla_xmt_cont(slap, sla_id,OFF_SEQ_ADDR);
		SLA_TIMER(sla_id, tvar,tservice, POLL_SEC, POLL_NSEC);
	}
	else if (s2_val & OFFSQ_REC) {
		sla_xmt_cont(slap, sla_id,UD_SEQ_ADDR);
		SLA_TIMER(sla_id, tvar,tservice, POLL_SEC, POLL_NSEC);
	}
	else if (s2_val & UDSEQ_REC) {
		sla_tbl.sla[sla_id].sla_st = SLA_NOTSYNC_CODE;
		sla_do_scr(slap, sla_id);
	}
	else if (s2_val & UDRSEQ_REC) {
		sla_tbl.sla[sla_id].sla_st = SLA_NOTSYNC_CODE;
		sla_do_scr(slap, sla_id);
	}
	else if (s2_val & LINK_SIG_ERR) {
		sla_tbl.sla[sla_id].sla_st = SLA_NOTSYNC_CODE;
		sla_sync(slap, sla_id);
	}
	else {
		/* we are seing idles */
		if (sla_tbl.sla[sla_id].event_count < MAX_IDLE_COUNT) {
			int msec;
			new_idle_count = sla_tbl.sla[sla_id].event_count + 1;
			CH_TRANSPARENT(slap, sla_id);  /* continue to send */
			msec = sla_tbl.sla[sla_id].event_count;
			SLA_TIMER(sla_id, tvar,tservice, 0, msec*10*ONE_MILLI);
		}
		else {  /* we have idles for more than 2*MAX_IDLE_COUNT msecs */
			sla_tbl.sla[sla_id].sla_st = SLA_NOTSYNC_CODE;
			sla_sync(slap, sla_id);  /* get back to int. driven processing */
		}
	}

	sla_tbl.sla[sla_id].event_count = new_idle_count;

}  /* end sla_poll */


/**/
/*
 * NAME: sla_recovery
 *                                                                    
 * FUNCTION: call the send dma recover and starts polling
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
sla_recovery( SLAIOP(slap),
	int sla_id)
{
	TVARS tvar;
	sla_sdma_unh(slap, sla_id);
	sla_xmt_cont(slap, sla_id,UD_SEQ_ADDR);
	tvar.func=RESTART_RECPOLL;
	tvar.id = sla_id;
	tvar.setup=TRUE;
	SLA_TIMER(sla_id, tvar,tservice, POLL_SEC, POLL_NSEC);

}  /* end sla_recovery */


/**/
/*
 * NAME: sla_recpoll
 *                                                                    
 * FUNCTION: handles the special case where we are generating/seeing nos
 *           because the dma had to be recovered from sending
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
sla_recpoll( SLAIOP(slap),
	int sla_id)
{
	uint s1_val, s2_val, s2_val1;
	TVARS tvar;
	BUS_GETLX((long *) &slap -> ch_cancel,&s1_val);/* try to stop channel */

	s2_val1 = slap -> status2;  /* read s2 twice */
	s2_val = slap -> status2;

	if (s2_val & XMT_FAIL) {
		sla_fatal(slap, sla_id, DRV_ERROR, LCC_DRV_FAIL);
	}
	else if (s2_val & SIG_FAIL) {
		sla_fatal(slap, sla_id, SIG_ERROR, LCC_SIG_FAIL);
	}
	else if (s2_val & NOPSQ_REC) {
		sla_xmt_cont(slap, sla_id,OFF_SEQ_ADDR);
		tvar.func=RESTART_RECPOLL;
		tvar.id = sla_id;
		tvar.setup=TRUE;
		SLA_TIMER(sla_id, tvar,tservice, POLL_SEC, POLL_NSEC);
	}
	else if (s2_val & OFFSQ_REC) {
		sla_xmt_cont(slap, sla_id,UD_SEQ_ADDR);
		tvar.func=RESTART_RECPOLL;
		tvar.id = sla_id;
		tvar.setup=TRUE;
		SLA_TIMER(sla_id, tvar,tservice, POLL_SEC, POLL_NSEC);
	}
	else if (s2_val & UDSEQ_REC) {
		sla_do_scr(slap, sla_id);
	}
	else if (s2_val & UDRSEQ_REC) {
		sla_do_scr(slap, sla_id);
	}
	else {
		/* we are seing idles */
		sla_do_scr(slap, sla_id);
	}

}  /* end sla_recpoll */


/**/
/*
 * NAME: sla_contscr 
 *                                                                    
 * FUNCTION: continues the scr sequence in transparent mode
 *      this routine will continue to transmit udr's until either
 *      the channel fails or idles are detected
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
sla_contscr( SLAIOP(slap),
	int sla_id)
{
	uint s1_val, s2_val;
	TVARS tvar;
	BUS_GETLX((long *) &slap -> ch_cancel,&s1_val);/* try to stop channel */
	s2_val = slap -> status2;


	if (s2_val & XMT_FAIL) {
		sla_fatal(slap, sla_id, DRV_ERROR, LCC_DRV_FAIL);
	}
	else if (s2_val & SIG_FAIL) {
		sla_fatal(slap, sla_id, SIG_ERROR, LCC_SIG_FAIL);
	}
	else if (s2_val & NOPSQ_REC) {
		sla_fatal(slap, sla_id, NOP_ERROR, LCC_NOPSQ_RC);
	}
	else if (s2_val & OFFSQ_REC) {
		sla_fatal(slap, sla_id, OFF_ERROR, LCC_OFFSQ_RC);
	}
	else if (s2_val & UDSEQ_REC) {
		CH_TRANSPARENT(slap, sla_id);  /* continue to send */
		tvar.func=RESTART_CONTSCR;
		tvar.id = sla_id;
		tvar.setup=TRUE;
		SLA_TIMER(sla_id, tvar,tservice, 0, 100 * ONE_MILLI);
	}
	else if (s2_val & UDRSEQ_REC) {
		sla_do_scr(slap, sla_id);
	}
	else {
		/* we are seing idles */
		sla_do_scr(slap, sla_id);
	}

}  /* end sla_contscr */


/**/
/*
 * NAME: sla_addr_unh
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
sla_addr_unh( SLAIOP(slap),
	int sla_id)
{
	uint s1_val;

	BUS_GETLX((long *) &slap -> ch_cancel,&s1_val);/* try to stop channel */

	primary_hdr(sla_id) = NULL_HDR;

	if (sla_tbl.sla[sla_id].connection == POINT_TO_POINT) {
		sla_fatal(slap, sla_id, BSY_ERROR, OTH_REAL_BSY);
	}
	else {  /* switched connection */
		sla_scr(sla_id) = TRUE;  /* this ensure that the call
	                              to cdd_enqueue from sla_address_error
			                              will not pick this sla */
		sla_address_error(sla_id);
		sla_do_scr(slap, sla_id);
	}

}  /* end sla_addr_unh */



/**/
/*
 * NAME: sla_kill
 *                                                                    
 * FUNCTION: brings down one sla either for diagnostic or for stopping
 *           imcs
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *      call sequence
 *              sla_id(input) - the sla that should be stopped
 *              mode(input) - a code that will be passed to sla_fatal
 *                      (should be either STOPPED or DIAG)
 *              index(input) - the index of the sla data field that will
 *                      be incremented by sla_fatal (should be either OTH_STOP
 *                      or OTH_DIAG)
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
sla_kill(int sla_id, 
	int mode, 
	int index)
{
	SLAIOP(slap);
	uint s1_val;
	int processor_priority;

	processor_priority = i_disable(IMCS_INT_PRIORITY);  /* mask ints */

	ADDR_SLA(sla_id, slap);

	s1_val = slap -> sample;

	switch (PUISTATE(s1_val)) {
	case PUI_NOOP :
	case PUI_STOPPED :
		break;
	default :
		BUS_GETLX((long *) &slap -> ch_cancel,&s1_val);/* try to stop channel */
		break;
	}

	sla_fatal(slap, sla_id, mode, index);
	
	if(mode != DIAG) {
		
		BUS_PUTLX((long *) &slap->ccr, DIAG_B | DRTO | DRIVER ); 
		STOP_TIMER(sla_tbl.sla[sla_id].time_struct);
 		STOP_TIMER(timer_sp[sla_id]);
	}

	UNADDR_SLA(slap);

	i_enable(processor_priority);   /* restore ints */

}  /* end sla_kill */


/**/
/*
 * NAME: start_ala_seq
 *                                                                    
 * FUNCTION: starts the ala sequence if the signal is good
 *           or goes back to synchronization if the signal is bad
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS: NONE
 */  
void
start_ala_seq(int sla_id)
{
	SLAIOP(slap);
	TVARS tvar;
	ADDR_SLA(sla_id, slap);
	SYS_SOL_TRACE("alas",sla_id,0,0);
	if (slap -> status2 & LINK_SIG_ERR) {
		/* 
		 *  If we reach this point, there is a good chance we have
		 *  reached a fatally stable state with the laser-safety
		 *  chip that we will never get out of.  The work-around
		 *  for this is to put the device back in wrap mode to
		 *  shut off the laser, and then restart the whole procedure
		 *  after some random back-off time (defect 47909).
		 */
		{
			int delay;
			struct timestruc_t time;

			sla_kill(sla_id, SLA_STOPPED, OTH_STOP);
			curtime(&time);		/* read the clock */
			delay = (((time.tv_nsec>>8) & 0x0000000f)+5)*ONE_MILLI;
			     /* use the value of the clock as random number */
			     /* 5-20ms delay */
			tvar.id = sla_id;
			tvar.func = RESTART;
			tvar.setup = FALSE;

			SLA_TIMER(sla_id, tvar, tservice, 0, delay);
		}
	}
	else if (slap -> status2 & UDRSEQ_REC) {
		tvar.func=START_ALA;
		tvar.id = sla_id;
		tvar.setup=FALSE;
		SLA_TIMER(sla_id, tvar,tservice, 0, 100 * ONE_MILLI);
	}
	else {
		sla_tbl.sla[sla_id].sla_st = SLA_NOTIDENT_CODE;
		xmt_ala(slap, sla_id);
	}

	UNADDR_SLA(slap);

}  /* end start_ala_seq */


/**/
/*
 * NAME: enqueue 
 *                                                                    
 * FUNCTION: places message in imcs queue, by (destination msg_id,
 *	     destination pid)
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *	  imcs headers of the same queue are threaded by imcs_chain_word
 *   	 different queues that collide on the hash chain are chained by
 *   	 next_imcsq_chain in the first header of the queue.  All queues are
 * 	 FIFO, and linear (not circular or doubly linked).
 *
 *  	masks interrupts to achieve synchronization.  Dequeue runs at
 *  	interrupt level.
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
 *    	TRUE   if message can be sent immediately
 *    	FALSE  otherwise
 */  

static boolean_t
enqueue( struct imcs_header *header )
{

	boolean_t send_now;

	int qa;
	int pid, msg_id;
	int offset;
	struct imcs_header *p, *q;
	int processor_priority;


	header -> imcs_chain_word = NULL_HDR;
	header -> next_imcsq_chain = NULL_HDR;

	pid = header -> IMCS_DEST_PROC;

	send_now = TRUE;                          /* assume we can send */

	processor_priority = i_disable(IMCS_INT_PRIORITY);

	if (header -> IMCS_PROTOCOL == IMCS_RTSI_SND_CODE) {
		msg_id = header -> IMCS_QUEUE;
		qa = HASH_SQ(pid, msg_id);
		p = isq.anchor_rtsi[qa];
		offset = (int) &(header -> IMCS_QUEUE) - 
		    (int) header;
	}
	else {
		msg_id = header -> IMCS_SUBCHANNEL;
		qa = HASH_SQ(pid, msg_id);
		p = isq.anchor_rts[qa];
		offset = (int) &(header -> IMCS_SUBCHANNEL) - 
		    (int) header;
	}

	if (p == NULL_HDR) {     /* nothing in this hash class */
		if (header -> IMCS_PROTOCOL == IMCS_RTSI_SND_CODE) 
			isq.anchor_rtsi[qa] = header;
		else isq.anchor_rts[qa] = header;
	}
	else {
		/* look for our chain */
		for (q = NULL_HDR;
		    p != NULL_HDR &&
		    ( *(ushort *) ((int) p + offset) != msg_id ||
		    p -> IMCS_DEST_PROC != pid);
		    p = p -> next_imcsq_chain) {
			q = p;
		}

		if (p == NULL_HDR) {      /* no chain found, start new one */
			q -> next_imcsq_chain = header;
		}
		else {                   /* found our chain, queue on it */
			send_now = FALSE;
			for (q = p;
			    p != NULL_HDR &&
			    ( *(ushort *) ((int) p + offset) == msg_id &&
			    p -> IMCS_DEST_PROC == pid);
			    p = p -> imcs_chain_word) {
				q = p;
			}
			header -> imcs_chain_word = q -> imcs_chain_word;
			q -> imcs_chain_word = header;
		}                                          /* there is queue */
	}                                                  /* anchor not null */

	i_enable(processor_priority);

	return send_now;

}  /* end enqueue */


/**/
/*
 * NAME: dequeue
 *                                                                    
 * FUNCTION:   takes the first message from an imcs queue that has the 
 *		pid and msg_id of the message header addressed by header.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This function runs at interrupt level
 *
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
 *   	address of next header for same msg_id/pid queue, if there is one.  
 *	Otherwise NULL_HDR
 */  

static struct imcs_header *
dequeue( struct imcs_header *header )
{

	int pid, msg_id;
	int offset;
	int qa;
	struct imcs_header *p, *q, *next;

	pid = header -> IMCS_DEST_PROC;

	if (header -> IMCS_PROTOCOL == IMCS_RTSI_SND_CODE) {
		msg_id = header -> IMCS_QUEUE;
		qa = HASH_SQ(pid, msg_id);
		p = isq.anchor_rtsi[qa];
		offset = (int) &(header -> IMCS_QUEUE) - 
		    (int) header;
	}
	else {
		msg_id = header -> IMCS_SUBCHANNEL;
		qa = HASH_SQ(pid, msg_id);
		p = isq.anchor_rts[qa];
		offset = (int) &(header -> IMCS_SUBCHANNEL) - 
		    (int) header;
	}

	/* q follows p in the search for the first queue element */
	for (q = NULL_HDR;
	    p != NULL_HDR &&
	    ( *(ushort *) ((int) p + offset) != msg_id ||
	    p -> IMCS_DEST_PROC != pid);
	    p = p -> next_imcsq_chain) {
		q = p;
	}
/* this ASSERT goes off if, due to race condition a message is sent twice */
	ASSERT(p != NULL_HDR);  
				     
	ASSERT(p == header);

	next = p -> imcs_chain_word;

	if (next == NULL_HDR)                       /* last element of queue */
		/* put next chain on previous or on anchor */
		if (q == NULL_HDR)
			if (header -> IMCS_PROTOCOL == IMCS_RTSI_SND_CODE)
				isq.anchor_rtsi[qa] = p -> next_imcsq_chain;
			else isq.anchor_rts[qa] = p -> next_imcsq_chain;
		else q -> next_imcsq_chain = p -> next_imcsq_chain;
	else {  /* more elements in queue */
		/* queue next chain on next element */
		next -> next_imcsq_chain = p -> next_imcsq_chain;
		/* queue our chain on anchor or previous chain */
		if (q == NULL_HDR)
			if (header -> IMCS_PROTOCOL == IMCS_RTSI_SND_CODE)
				isq.anchor_rtsi[qa] = p -> imcs_chain_word;
			else isq.anchor_rts[qa] = p -> imcs_chain_word;
		else q -> next_imcsq_chain = p -> imcs_chain_word;
	}

	return next;

}  /* end dequeue */




/**/
/*
 * NAME:  check_header
 *                                                                    
 * FUNCTION: check for valid destination processor id
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  1 if invalid 
	     0 if valid
 */  

static int
check_header( struct imcs_header *header )
{
	if ( (header -> IMCS_DEST_PROC < 0 ||
	    header -> IMCS_DEST_PROC >= IMCS_PROC_LIMIT) &&
	    header -> IMCS_DEST_PROC != IMCS_FAKE_PID) return 1;

	return 0;

}  /* end check_header */

/**/
/*
 * NAME: imcs_sendmsg
 *                                                                    
 * FUNCTION: sends one message or sets up a receiving subchannel
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *  header and fields pointed to by tagwords must be pinned.  In addition
 *  the memory pointed to by the tagwords must be either flushed (for sends) or
 *  invalidated (for receive) out of the cache.
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 	1 if call to check header indicates invalid id
 *		0 otherwise
 */  

int
imcs_sendmsg( struct imcs_header *header )
{
	int rc;
	int index;

SOL_TRACE("smsg",header,0,0);
	index = (uint) (header -> IMCS_PROTOCOL) >> 13;

	header -> outcome = IMCS_INIT_GCK;

	rc = 0;

	if (header -> IMCS_GO == IMCS_INITIATE) {
		if (rc = check_header(header)) return rc;

		if (header -> IMCS_SERIALIZE) {
			if (enqueue(header))
				cdd_enqueue(header);
		}
		else /* not serialized */
			cdd_enqueue(header);
	}
	else 
		cdd_enqueue(header);

	return rc;

}  /* end imcs_sendmsg */



/**/
/*
 * NAME: imcs_send_done
 *                                                                    
 * FUNCTION: this function is called when a send is completed to determine 
 *		next send operation required
 * 
 * EXECUTION ENVIRONMENT: 
 *      This function runs at interrupt level  
 *
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS: 
 *	NULL_HDR  there are no more messages to send, and the
 *      	  current message should be notified
 *
 *   	other value address of the next message to send
 */  

struct imcs_header *
imcs_send_done( struct imcs_header *header)
{

	struct imcs_header *next;

	if (header -> IMCS_SERIALIZE) {
		next = dequeue(header);
	}
	else next = NULL_HDR;

	return next;

}  /* end imcs_send_done */


/**/
/*
 * NAME: receive 
 *                                                                    
 * FUNCTION: enqueues receive stubs to the cdd level
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

static void
receive()
{
	int j;
	struct imcs_header *header;
	caddr_t hdr_vaddr;
	long hdr_raddr;

	#ifdef RTSILONG
	while (ipool_count() >= NUM_TCWS - 1 && count_imcs_hdr() > 0) {
	#else
	while (count_imcs_hdr() > 0) {
	#endif

		hdr_vaddr = get_imcs_hdr();
		ASSERT(hdr_vaddr != NULL);

		hdr_raddr = imcslra(hdr_vaddr);
		header=(struct imcs_header *) (hdr_vaddr + IMCS_RTSI_UHDR_SIZE);
		header -> IMCS_TAG(0) = hdr_raddr;
		header -> IMCS_COUNT(0) = IMCS_RTSI_SHDR_SIZE/IMCS_LINE_SIZE;


		/* get the buffers */
		for (j = 1; j < NUM_TCWS; j ++) {
			#ifdef RTSILONG
			int pageno;
			pageno = ipool_get();
			ASSERT(pageno != -1);
			header -> IMCS_TAG(j) = pageno << L2PAGESIZE;
			header -> IMCS_COUNT(j) = 0;
			#else
			header -> IMCS_TAG(j) = LAST_TCW;
			#endif
		}

		header -> cdd_chain_word = NULL_HDR;
		header -> next_cddq_chain = NULL_HDR;
		header -> IMCS_PROTOCOL = IMCS_RTSI_RCV_CODE;
				/* invalidate the user portion of the header */
		vm_cinval(hdr_vaddr, IMCS_RTSI_UHDR_SIZE);  
				  
		enqueue_to_rcv(header);
	}

}  /* end receive */



/**/
/*
 * NAME: imcs_rethdr 
 *                                                                    
 * FUNCTION: returns a header to imcs pool of headers
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
 *   IMCS_DONE -- queue is credited
 *   QUEUE_NOT_DCL -- queue was not declared yet
 */  


int
imcs_rethdr(queue_id, header)
uint queue_id;
caddr_t header;
{

	struct irq_block *irq;
	int processor_priority, i;

	irq = rqctl_find(queue_id);

	if (irq == NULL_IRQ) return QUEUE_NOT_DCL;

	put_imcs_hdr(header);               /* return header to pool */

	processor_priority = i_disable(IMCS_INT_PRIORITY);/* mask imcs ints */

	irq -> hdr_in_use--;

	for (i=0 ; i<IMCS_PROC_LIMIT ; i++) {
		if (irq->pids[i] & NACKED_HEADER) {
			irq->pids[i] &= ~NACKED_HEADER;
			break;
		}
	}

	receive();

	i_enable(processor_priority);	/* returns to previous process level */

	return IMCS_DONE;

}  /* end imcs_rethdr */


/**/
/*
 * NAME: imcs_addbuf 
 *                                                                    
 * FUNCTION: adds a buffer to a queue account
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS:
 *   IMCS_DONE -- queue is credited
 *   QUEUE_NOT_DCL -- queue was not declared yet
 *   STACK_FULL -- imcs has no space to hold the page
 */  


int
imcs_addbuf(queue_id, pageno)
uint queue_id;
int pageno;
{

	struct irq_block *irq;
	int processor_priority, i;

	irq = rqctl_find(queue_id);

	if (irq == NULL_IRQ) return QUEUE_NOT_DCL;

	if (ipool_put(pageno)) return STACK_FULL;  /* return page to pool */

	processor_priority = i_disable(IMCS_INT_PRIORITY);  /* mask imcs ints */

	irq -> num_buf++;

	for (i=0 ; i<IMCS_PROC_LIMIT ; i++) {
		if (irq->pids[i] & NACKED_BUFFER) {
			irq->pids[i] &= ~NACKED_BUFFER;
			break;
		}
	}

	receive();

	i_enable(processor_priority);	/* returns to previous process level */

	return IMCS_DONE;

}  /* end imcs_addbuf */



/**/
/*
 * NAME: imcs_rcv_start 
 *                                                                    
 * FUNCTION:   part of initialization.  Claims some receive headers for imcs
 *		 and enqueues them at the cdd level.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *	 Should be called by the imcs initialization routines, before the
 *	 predeclared queues are declared because it claims one header for each
 *	 sla.
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
imcs_rcv_start()
{
	int processor_priority;

	processor_priority = i_disable(IMCS_INT_PRIORITY);
	receive();
	i_enable(processor_priority);

}  /* end imcs_rcv_start */



/**/
/*
 * NAME: imcs_rcv_fail
 *                                                                    
 * FUNCTION:  called when an rtsi sequence fails to complete and imcs_rcv_done 
 *		has already prepared the header for the current receive
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  


void
imcs_rcv_fail(header)
struct imcs_header * header;
{
	int tcw;
	struct irq_block *irq;

	irq = rqctl_find(header -> IMCS_QUEUE);
	ASSERT (irq != NULL_IRQ);

	for (tcw = 1; header -> IMCS_TAG(tcw) != LAST_TCW; tcw++) {
		ipool_put(header -> IMCS_TAG(tcw));
		irq -> num_buf++;
	}

	put_imcs_hdr((caddr_t) header);
	irq -> hdr_in_use--;

	irq -> rcv_count--;

	receive();

}   /* end imcs_rcv_fail */



/**/
/*
 * NAME: imcs_rcv_done 
 *                                                                    
 * FUNCTION: determines whether to ack or nack
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS: 
 *	IMCS_ACK - ack
 *	IMCS_NACK_NQ - nack no queue
 *	IMCS_NACK_NR - nack not receiving 
 *	ICMS_NACK_NB - nack no buffers
 */  

uint
imcs_rcv_done(sla_id)
int sla_id;
{

	uint gck_value;
	struct irq_block *irq;
	struct imcs_header *header;

	header = passive_hdr(sla_id);

	/* check that the recepient can receive (has space) */
	irq = rqctl_find(header -> IMCS_QUEUE);
	if (irq == NULL_IRQ) {  /* no such queue */
		gck_value = IMCS_NACK_NQ;
	}
	else
		if (irq -> status & IRQ_RECEIVING) {
			int rcv_len, number_of_frames;
			rcv_len = header -> IMCS_MSGLEN - IMCS_RTSI_SHDR_SIZE;
			number_of_frames = rcv_len >> L2PAGESIZE;  
						/* divide by page size */
			if (rcv_len & (1 << L2PAGESIZE) - 1) number_of_frames++;

			if (irq -> hdr_in_use < irq -> max_num_hdr) {
				if (irq -> num_buf >= number_of_frames) {
					/* queue has buffers and header */
					gck_value = IMCS_ACK;

					header->notify_address = 
					                  irq->imcs_slih_ext;  
					irq -> hdr_in_use++;
					irq -> num_buf -= number_of_frames;
					irq -> rcv_count++;

				} /* q has bufs */
				else {   /*queue has hdr doesn't have buffs */
					gck_value = IMCS_NACK_NB;
					/* remember processor was nacked */
					irq->pids[header->imcs_send_proc] |=
					    NACKED_BUFFER;
				}	/* queue does not have enough frames */
			}
			else {   /* queue does not have headers */
				gck_value = IMCS_NACK_NB;
				/* remember processor was nacked */
				irq->pids[header->imcs_send_proc] |=
				    NACKED_HEADER;
				if (irq->num_buf < number_of_frames)   
					/*queue doesn't have hdr, but has buf */
					/* remember processor was nacked */
					irq->pids[header->imcs_send_proc] |=
					    NACKED_BUFFER;
			}	/* queue has no headers */
		}	/* queue is receiving */
		else {   /* queue is not receiving */
			gck_value = IMCS_NACK_NR;
		}

	return gck_value;

}  /* end imcs_rcv_done */

