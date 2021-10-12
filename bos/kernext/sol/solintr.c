static char sccsid[] = "@(#)71	1.13  src/bos/kernext/sol/solintr.c, sysxsol, bos411, 9428A410j 4/17/92 10:10:00";

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

#include <sys/syspest.h>	/* for ASSERT, BUGXDEF */
#include <sys/intr.h>		/* flih stuff */
#include <sys/timer.h>		/* for trb */
#include <sys/time.h>		/* for timestruc_t */
#include <sys/limits.h>		/* for INT_MAX */
#include <sys/sleep.h>		/* for EVENT_NULL */
#include <sys/trcmacros.h>	/* tracing stuff */
#include <sys/trchkid.h>	/* tracing hook id's */
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/errids.h>

#include "soldd.h" 

#include "sol_proto.h"

struct irq_tbl irq_tbl;
struct cddq cddq;
struct sla_tbl sla_tbl;
struct isq isq;
struct imcs_addresses imcs_addresses;
int imcs_host = -1;
int poll_cnt[MAX_NUM_SLA];

/*
 *  Each byte in the cck_proc array has three flags:
 *  	PID_PRESENT
 *  	PID_EVER_PRESENT
 *  	PID_HOLDING
 */
uchar	cck_proc[IMCS_PROC_LIMIT];  /* [0..254], but 0 is not allowed */
typedef uchar cck_proc_t;

struct que que;

struct intr sla_handler[MAX_NUM_SLA];	/* structure used by flih */

struct imcsdata imcsdata = {
	FALSE, FALSE, {
		IMCS_VERSION_CLONE, IMCS_VERSION_N	}
};

static struct sl_frame prhr_value;
static int delay_sec;
static uint delay_nsec;
void *send_timer[MAX_NUM_SLA];


extern int bad_port[MAX_NUM_SLA];
extern struct sol_ddi sol_ddi;


/* entries in sladrv */
extern int sla_start();
extern void sla_fatal(), sla_send_auto(), start_rcv_auto(), start_listen();
extern void tservice();

/* entries in ml/vmrts */
extern vm_cinval(), cm_cflush();

/* internal routines */
static boolean_t error_check();
static uint doneint(), ufint(), timedint(), scrint();
static void fatal_error();

/* external routines */

extern void sla_fatal();  /* from sladrv.c */
extern void err_log();

static ushort next_B = 0;

static void
enqueue_to_sla(struct imcs_header *header);
void
enqueue_to_rcv(struct imcs_header *header);
static void
enqueue_to_schn(struct imcs_header *header);


/**/
/*
 * NAME: sla_rcv_done
 *                                                                    
 * FUNCTION: called by the interrupt handler  after a message has been received
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

void sla_rcv_done(int sla_id, uchar source_address)
{
	struct imcs_header *msg_rcved;

	SOL_TRACE("rcvd",sla_id,0,0);
	msg_rcved = passive_hdr(sla_id);

	msg_rcved -> imcs_send_proc = 
			imcs_addresses.sla_addr_tb[sla_id][source_address];
	msg_rcved -> sla_id = sla_id;

	if (msg_rcved -> IMCS_PROTOCOL == IMCS_RTS_RCV_CODE) {
		/* call the notification routine */
		(* msg_rcved -> notify_address) (msg_rcved);
	}
	else { /* rtsi */
	     /* call the slih extension (imcs saved it in the header when it
	     checked if the queue had enough space to received the header) */
		(* msg_rcved -> notify_address) ((caddr_t) 
				((int) msg_rcved - IMCS_RTSI_UHDR_SIZE));
	}

	cdd_rcv_start(sla_id); 	/* get another rtsi header */

}  /* end sla_rcv_done */


/**/
/*
 * NAME: sla_rtr_snd_done
 *                                                                    
 * FUNCTION: called when send completes for an rtr message
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

void sla_rtr_snd_done(int sla_id, uchar source_address)
{

	struct imcs_header *msg_sent;

	SOL_TRACE("rtrs",sla_id,0,0);
	msg_sent = passive_hdr(sla_id);

	msg_sent -> imcs_send_proc = 
			imcs_addresses.sla_addr_tb[sla_id][source_address];

	cdd_rcv_start(sla_id); 	/* get another rtsi header */

	/* call the notification routine */
	(* msg_sent -> notify_address) (msg_sent);

}  /* end sla_rtr_snd_done */



/**/
/*
 * NAME: sla_rcv_error
 *                                                                    
 * FUNCTION: called when an error occurs while receiving a message
                 but the sla is not broken
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
sla_rcv_error(int sla_id)
{
	
	SOL_TRACE("rxerr",sla_id,0,0);
	if (passive_hdr(sla_id) -> IMCS_PROTOCOL == IMCS_RTS_RCV_CODE ||
	    passive_hdr(sla_id) -> IMCS_PROTOCOL == IMCS_RTR_SND_CODE) {
		/* put it back in the subchannel queue */
		requeue_to_schn(passive_hdr(sla_id));
		cdd_rcv_start(sla_id); 	/* get another rtsi header */
	}
	else {
		ASSERT(passive_hdr(sla_id)->IMCS_PROTOCOL ==IMCS_RTSI_RCV_CODE);
		if (rtsi_save(sla_id) != NULL_HDR) {
			struct imcs_header *less_than_perfect;
			ASSERT(rtsi_save(sla_id) -> IMCS_PROTOCOL == 
						IMCS_RTSI_RCV_CODE);
			/* this case occurs when there is an error detected 
			after sending the ack for an rtsi rcv.  At this point 
			rtsi_save is a genuine header (ie.,15 tags are valid), 
			and passive is less then perfect */
			less_than_perfect = passive_hdr(sla_id);
			passive_hdr(sla_id) = rtsi_save(sla_id);
			rtsi_save(sla_id) = NULL_HDR;
			imcs_rcv_fail(less_than_perfect);
		}
	}

}  /* end sla_rcv_error */



/**/
/*
 * NAME: sla_error
 *                                                                    
 * FUNCTION: called when a fatal error is detected (e. g., loss of light)
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
sla_error(int sla_id, int mode)
{
	SOL_TRACE("serr",sla_id,mode,0);

	if (mode) {
		/* this sla will not be used again */
		sla_tbl.sla[sla_id].imcs_st = SLA_FAILED_CODE;
		sla_address(sla_id) = 0;
	}

	/* return the headers to the cdd level */
	cdd_clean_sla(sla_id);

	/* tell the auditor */
	if (mode) imcs_sla_off(sla_id, mode);

}

/**/
/*
 * NAME: sla_address_error
 *                                                                    
 * FUNCTION: called when an error is detected on a destination address
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
sla_address_error(int sla_id)
{
	struct imcs_header * this_msg;

	SOL_TRACE("saer",sla_id,other_addr(sla_id),0);
	ASSERT(sla_tbl.sla[sla_id].connection != POINT_TO_POINT);

	primary_hdr(sla_id) = NULL_HDR;
	/* tell the auditor */
	if(other_addr(sla_id) == INVALID_SLA_ADDR) return; 
	imcs_sla_addr_off(sla_id, other_addr(sla_id));

	cdd_clean_sla(sla_id);


}  /* sla_address_error */



/**/
/*
 * NAME: sla_get_next_addr 
 *                                                                    
 * FUNCTION: finds if possible if the current message can be sent though 
 *		the switch with another address
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: TRUE if there is hope for the sla, i. e., the message has 
 *		another address, or there is some other message queued
 * 	    FALSE otherwise 
 */  


boolean_t
sla_get_next_addr(int sla_id)
{
	int j, p;

	ASSERT(active_hdr(sla_id) != NULL_HDR);
	ASSERT(sla_tbl.sla[sla_id].connection == SWITCH_PRESENT);

	p = active_hdr(sla_id) -> IMCS_DEST_PROC;

	for (j = 0;
	    j < MAX_NUM_SLA &&
	    imcs_addresses.proc[p].sla_addr[sla_id][j] != other_addr(sla_id);
	    j++);
	ASSERT(j < MAX_NUM_SLA);

	for (j++;
	    j < MAX_NUM_SLA &&
	    imcs_addresses.proc[p].sla_addr[sla_id][j] == INVALID_SLA_ADDR;
	    j++);

	if (j < MAX_NUM_SLA) {
		/* there is another address */
		other_addr(sla_id) = imcs_addresses.proc[p].sla_addr[sla_id][j];
		return TRUE;
	}
	else { /* there is no other address */
		return FALSE;
	}

}  /* end sla_get_next_addr */


/**/
/*
 * NAME: slaih
 *                                                                    
 * FUNCTION: sla interrupt handler called by the flih with address 
 *		of intr structure for interrupting sla
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
        The interrupt is handled first by determining what happened.
        Then, considering the event and the current software state
        of the sla, an action to be performed by the channel is selected.
        Lastly, the operation on the channel is performed.


        The possible events that the interrupt handler recognizes are:
                an error occurred
                sla just finished an operation (OP_DONE)
                        syncronization
                        an automatic sequence
                the sla just received a frame
                        request
                        response
                the sla just timed out
                the sla completed a switch connection recovery sequence


        The possible software state of the sla are:
                SLA_IDENT_CODE, meaning that the sla is "identified", i.e.,
                has an harware address as a consequence of a successful
                ala sequence.  In this state the sla is used mostly to
                do automatic sequences.

                SLA_NOTSYNC_CODE, meaning that the sla was not synchronized
                the last time we looked at it.  This implies that, in
                absence of errors, this interrupt is caused by the sla
                achieving character synchronization.
                In this case the sla should start the ala sequence.

                SLA_NOTIDENT_CODE, meaning that the sla is synchronized but
                has not finished yet the ala sequence.


        The possible operation for the channels are:
                CONTINUE, continue with the continous operation of the link,
                performing the operation indicated by imcs (either send, or
                listen -- immediate or not).

                WAIT, continue with the continous operation of the link, but
                listen for a while before doing a send.

                LFA (listen for ala), listend for an ala on the channel for
                a while.  If nothing is received during the listening period,
                this side will send an ala.

                XMT_UD, start or continue a scr sequence.

                SYNC, try again to achive characted synchronization.

 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: INTR_SUCC 
 */  

int
slaih(struct intr *sla_handler)
{
	int sla_id;				/* interrupting sla */
	uint s1_val, s2_val;
	TVARS tvar;
	SLAIOP(slap);
	uint action;			/* a code indicating action to follow */


	/* which sla is interrupting ? */
	sla_id = sla_handler -> level - SLA_INTR_OFFSET;
	ASSERT(sla_id >= 0 && sla_id < MAX_NUM_SLA);


	ADDR_SLA(sla_id, slap);	/* make device addressable */

	/* read status registers */
	s1_val = slap -> status1;
	s1_val |= sla_tbl.sla[sla_id].s1_save; /* or in previous value */
	sla_tbl.sla[sla_id].s1_save = 0;       /* and clear it */

	s2_val = slap -> status2;

	sla_inter(sla_id) = TRUE;		/* mark sla as interrupting */

	SYS_SOL_TRACE("slih",sla_id,s1_val,s2_val);

	if (PUISTATE(s1_val) != PUI_STOPPED && PUISTATE(s1_val) != PUI_NOOP) {
		action = NO_ACTION_CODE;
		goto int_done;
	}

	/* if there is a timer request for this sla stop it */
	STOP_TIMER(sla_tbl.sla[sla_id].time_struct);


	switch (sla_tbl.sla[sla_id].sla_st) {
	case SLA_IDENT_CODE :
		if (s1_val & PRIMARY) { 
			READ_PRHR(slap, prhr_value); 
		}

		/* sort through error conditions.  error_check sets action only
        	if it detects an error */

		if (! error_check(slap, sla_id, s1_val, s2_val, &action) ) {
			/* the sla either received some frame, timed out, 
			finished sending a frame, or this is the end of an 
			automatic sequence */

			action = NO_ACTION_CODE;
			ASSERT(PUISTATE(s1_val) == PUI_STOPPED);  
			    /* will go off rarely on switch on a scr timeout */

			if (s1_val & RESP_TO) {		/* the sla timed out */
				/* note that timeout and scr done can occur at 
				the same time */
				action = timedint(slap, sla_id, s1_val, s2_val);
			}
			else if (s1_val & SCR_DONE) {	
				/* a UD/UDR sequence completed */
				/* note that scr done and op done can occur at 
				the same time */
				action = scrint(slap, sla_id, s1_val, s2_val);
			}
			else if (s1_val & OP_DONE) { 	
				/* the sla completed an operation */
				action = doneint(slap, sla_id, s1_val, s2_val);
			}
			else if ((s1_val & PRIMARY)  && (s1_val & UNEXPD))  {
				/* the values loaded in the prhr are valid and 
				the sla received a frame */
				action = ufint(slap, sla_id, s1_val, s2_val);
			}
			else {	/* do not know why we had an interrupt */
				action = XMT_UD_CODE;
			}
		}  /* end no error */

	break;

	case SLA_NOTSYNC_CODE :
		action = do_notsync(slap, sla_id, s1_val, s2_val);
		break;

	case SLA_NOTIDENT_CODE :
		action = do_notident(slap, sla_id, s1_val, s2_val);
		break;

	case SLA_POLLMODE_CODE  :
		tvar.id = sla_id;
		tvar.func = RESTART_CONT;
		tvar.setup = TRUE;
		SLA_TIMER(sla_id, tvar, tservice, POLL_SEC, POLL_NSEC);
		action = NO_ACTION_CODE;
		break;

	case SLA_DIAGNOSTIC_CODE :
		sla_tbl.sla[sla_id].s1_save = s1_val;
		sla_tbl.sla[sla_id].s2_save = s2_val;
		if (sla_tbl.sla[sla_id].diag_process != EVENT_NULL)
			e_wakeup(&(sla_tbl.sla[sla_id].diag_process));
		action = NO_ACTION_CODE;
		break;

	default :
		PANIC("slaih: sla_st is unknown");
	}  /* end switch on sla st */

int_done:
	switch (action) {

	case NO_ACTION_CODE:
		break;
	case CONTINUE_CODE :
		if (active_hdr(sla_id) != NULL_HDR) { /* imcs wants to send */
			start_send_auto(slap, sla_id);
		}  /* end send */
		else if (passive_hdr(sla_id) != NULL_HDR) { 
				/* imcs want rcv imm. */
			start_rcv_auto(slap, sla_id);
		} /* end listen immediate */
		else {
			start_listen(slap, sla_id);
		}
		break;


	case WAIT_CODE :

		if (active_hdr(sla_id) != NULL_HDR) {
			/* imcs wants to send again, but will wait a while */
			tvar.id = sla_id;
			tvar.func = RESTART_SEND;
			tvar.setup = FALSE;
			SLA_TIMER(sla_id,tvar,tservice, delay_sec, delay_nsec);
		}
		if (passive_hdr(sla_id) != NULL_HDR) { /* imcs want rcv imm. */
			start_rcv_auto(slap, sla_id);
		}
		else {
			start_listen(slap, sla_id);
		}
		break;


	case LFA_CODE : /*listen for ala code. Will retry sending after delay */
		{
			int delay;
			struct timestruc_t time;

			curtime(&time);		/* read the clock */
			delay = time.tv_nsec;
			     /* use the value of the clock as random number */
			if (delay > MAX_ALA_WAIT) delay = MAX_ALA_WAIT;
			else if (delay < MIN_ALA_WAIT) delay = MIN_ALA_WAIT;
			tvar.id = sla_id;
			tvar.func = RESTART_ALA;
			tvar.setup = TRUE;

			SLA_TIMER(sla_id, tvar, tservice, 0, delay);

			start_listen(slap, sla_id);
		}
		break;

	case XMT_UD_CODE :
		sla_scr(sla_id) = TRUE;
		START_SCR(slap, sla_id);
		break;


	case SYNC_CODE :
		/* channel program to synchronize the sla again */
		sla_tbl.sla[sla_id].sla_st = SLA_NOTSYNC_CODE;

	   /* the sla detected loss of light and entered the not operational
	   state.  However, the sla sent NOP for the brief period that it
	   took us to handle this interrupt.  So leave the channel in the
	   no-op state, sending NOP for another millisecond.  Then
	   synchronize.  If the sla is not in the not operational state,
	   wait one millisecond to avoid hogging the machine (the device
	   has interrupted a zillion times in a few seconds).
	*/
			tvar.id = sla_id;
			tvar.func = RESTART_SYNC;
			tvar.setup = TRUE;
		SLA_TIMER(sla_id, tvar,tservice, 0, ONE_MILLI);
		break;

	default :
		PANIC("slaih: int. handler set action flag to unknown value");
	}

	UNADDR_SLA(slap);
	sla_inter(sla_id) = FALSE;

	return INTR_SUCC;

}  /* end slaih */


/**/
/*
 * NAME: prog_check 
 *                                                                    
 * FUNCTION: handles programming check errors
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
prog_check(slap, sla_id, s1_val, s2_val)
SLAIOP(slap);
int sla_id;
uint s1_val, s2_val;
{
	
	err_log(6,ERRID_SLA_PROG_ERR,s1_val,s2_val);

	SOL_TRACE("pchk",sla_id,s1_val,s2_val);
	if (s2_val & RBUFCHK) {
		/* receiver did not map enough space for message */
		ASSERT (primary_hdr(sla_id) != NULL_HDR);
		ASSERT(primary_hdr(sla_id)-> IMCS_OP_CODE == IMCS_RECEIVE_CODE);
		primary_hdr(sla_id) -> outcome = (ushort) IMCS_TAG_CHK;
		if (sla_dma(sla_id)) {
			sla_rdma_unh(slap, sla_id);
		}
		if (primary_hdr(sla_id) -> IMCS_GO)  /* rtr rcv */
			cdd_send_done(sla_id);
		else
			if (! primary_hdr(sla_id) -> IMCS_AUTOMATIC)
				sla_rcv_done(sla_id, 
					EXTRACT_SADDR(prhr_value.did_sid));
		/* do nothing for rtsi receive */

		primary_hdr(sla_id) = NULL_HDR;

	}  /* end rcv buffer check */

	if (s2_val & XBUFCHK) {
		/* this is caused by an incorrect tag as the first tcw */
		/* later could be caused by less tcw space as xmt count reg. */
		ASSERT(primary_hdr(sla_id) != NULL_HDR);
		ASSERT(primary_hdr(sla_id) -> IMCS_OP_CODE == IMCS_SEND_CODE);
		primary_hdr(sla_id) -> outcome = (ushort) IMCS_TAG_CHK;
		if (primary_hdr(sla_id) -> IMCS_GO)  /* rts(i) send */
			cdd_send_done(sla_id); /* return header to user */
		else /* rtr snd */
			sla_rtr_snd_done(sla_id, 0);
		primary_hdr(sla_id) = NULL_HDR;
		if (sla_dma(sla_id)) {
			sla_sdma_unh(slap, sla_id);/*and recover the send dma */
		}

	}

	if (s2_val & CMDCHK) PANIC("slaih: command check");

}   /* end prog_check */

/**/
/*
 * NAME: check_addr_mis
 *                                                                    
 * FUNCTION: checks that address mismacthes are genuine
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: FALSE -- there really is not an address mismatch, or
 *                   the frame is such that an address mismacth does
 *                   not matter; or
 *          TRUE -- there is an address mismatch error
 */  


static boolean_t
check_addr_mis(slap, sla_id, s1_val, s2_val)
SLAIOP(slap);
int sla_id;
uint s1_val, s2_val;
{
	boolean_t error;

	/* if we received a frame, and */
	if (s1_val & PRIMARY &&
	    /* there is no error in status 1, and */
	! (s1_val & (CHAN_CHK | PROG_CHK | FRM_DISC | BSY_DISC | RJT_DISC |
	    ABT_XMT) ) &&
	    /* there is no error in status 2, and */
	! (s2_val & (RST_RCVD | SNDCNTCHK | XMT_FAIL | LINK_SIG_ERR |
	    NOPSQ_REC | OFFSQ_REC) ) &&
	    /* this is not an aborted frame, and */
	(EOF_RCVD(prhr_value.w5.rfrst) != EOF_ABORT) &&
	    /* there is no error in the frame status bits */
	! (prhr_value.w5.rfrst & (CRC_CHK|STRUCT_CHK|CDVL_CHK | SFRS_CHK) ) ) {
		/* there is a frame, and no error other than address mismatch */
	SOL_TRACE("amis",sla_id,prhr_value.link_ctl,prhr_value.w3.dev_ctl);
		switch (prhr_value.link_ctl) {
		case LINK_RESPONSE_BSY :
		case LINK_REQUEST_ALA :
		case DYNAMIC_PORT_RJT :
		case LINK_RESPONSE_RJT :
			/*these frame are accepted even with address mismatch */
			error = FALSE;
			break;

		case DEVICE_CTL_DATA :
			switch (DEV_CTL(prhr_value.w3.dev_ctl)) {
			case DEV_RESP_ACK :
				if (sla_tbl.sla[sla_id].connection == 
							 DX_SWITCH_PRESENT &&
				    (SWAP(prhr_value.did_sid) == 
							slap->thr.fr.did_sid) )
					error = FALSE;
				else /* frame really has the wrong address */
					error = TRUE;
				break;
			case DEV_REQ_SEND :
			case DEV_REQ_SEND_IMM :
			case DEV_REQ_RCV :
				switch (LINKSTATE(s1_val)) {
				case LI_CWAIT :
				case LI_CTRY :
					if (((sla_tbl.sla[sla_id].connection == 
							SWITCH_PRESENT) ||
					    (sla_tbl.sla[sla_id].connection ==
							DX_SWITCH_PRESENT)) &&
			 /* if the connection is switched, and the destination 
			address of the frame received is equal to the source 
			address of the frame that we send */
					(EXTRACT_DADDR(prhr_value.did_sid) ==  
					EXTRACT_SADDR(slap -> thr.fr.did_sid)) )
						error = FALSE;
					else /* frame has the wrong address */
						error = TRUE;
					break;
				default :
					error = TRUE;
				}  /* end switch on LINKSTATE */
				break;
			default :
			/* all other device frame should not have addr. mis. 
		        indication */
				error = TRUE;
			}
			break;

		default :
		/* all other frame should no address mismatch indication */
			error = TRUE;
		}  /* end switch on link ctl */
	}
	else /* there is some other error */
		error = TRUE;

	return error;

}  /* end check_addr_mis */


/**/
/*
 * NAME: error_check
 *                                                                    
 * FUNCTION: : checks if there is an error.  It sets action if there is an error

 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *       possible settings for action are  (set ONLY IF it returns TRUE)
 *       XMT_UD_CODE -- used for most errors
 *       NO_ACTION_CODE -- for severe errors handled in sla_fatal
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: TRUE if error detected
 *	    FALSE otherwise
 */  

static boolean_t
error_check(slap, sla_id, s1_val, s2_val, actionp)
SLAIOP(slap);
int sla_id;
uint s1_val, s2_val;
uint *actionp;
{
	uint action;
	boolean_t error;


	if(s1_val & RESP_TO) { /* response timeout -1,0 used to indicate this */
		err_log(2,ERRID_SLA_FRAME_ERR,-1,0); 
	}
	if(s2_val & SNDCNTCHK) {/* send count check 0,-1 used to indicate this*/
		err_log(2,ERRID_SLA_FRAME_ERR,0,-1); 
	}

	if (ANYCHK(s1_val)) {
		error = TRUE;
		action = XMT_UD_CODE;  /* default action */

		SYS_SOL_TRACE("echk",sla_id,s1_val,s2_val);
		if (s1_val & CHAN_CHK) {
			action = NO_ACTION_CODE;
			sla_channel_check(slap, sla_id, s2_val);
		}

		if (s1_val & LINK_CHK) {
			/* if the frame received is a busy or reject frame, and
	         	there were no other errors */
			if(s2_val & ADDR_MIS) {
				error = 
				   check_addr_mis(slap, sla_id, s1_val, s2_val);
			}

			if (s1_val & PRIMARY) { /* prhr loaded */
				uint frame_status;
				frame_status = prhr_value.w5.rfrst;
			}

			if (s2_val & XMT_FAIL) {   /* driver failed */
				if (action != NO_ACTION_CODE) {
					/* fatal_error not yet called */
					fatal_error(slap, sla_id, DRV_ERROR, 
								LCC_DRV_FAIL);
					action = NO_ACTION_CODE;
					err_log(5,ERRID_SLA_DRIVER_ERR,s1_val,
					    s2_val);
				}
			}
			else if (SIGNAL_FAILURE(slap, s2_val) ) {  
				/* signal was lost */
				if (action != NO_ACTION_CODE) {
					/* fatal_error not yet called */
					fatal_error(slap, sla_id, SIG_ERROR, 
								LCC_SIG_FAIL);
					action = NO_ACTION_CODE;
					err_log(4,ERRID_SLA_SIG_ERR,s1_val,s2_val);
					SYS_SOL_TRACE("serr",sla_id,s1_val,s2_val);
				}
			}
			else if (s2_val & NOPSQ_REC) {
			/*the not-operational sequence was detected the 
	         	other side channel was halted or is experiencing 
	         	a severe error */
				if (action != NO_ACTION_CODE) {
					/* fatal_error not yet called */
					fatal_error(slap, sla_id, NOP_ERROR, 
								LCC_NOPSQ_RC);
					action = NO_ACTION_CODE;
				}
			}
			else if (s2_val & OFFSQ_REC) {
				/* the offline sequence was detected the other
	         		side channel is stopping */
				if (action != NO_ACTION_CODE) {
					/* fatal_error not yet called */
					fatal_error(slap, sla_id, OFF_ERROR, 
								LCC_OFFSQ_RC);
					action = NO_ACTION_CODE;
				}
			}
		}  /* end LINK_CHK */

		if (s1_val & PROG_CHK) {
			prog_check(slap, sla_id, s1_val, s2_val);
		}  /* end PROG_CHK */

		if (error) {
			*actionp = action;
		}

	} /* end any checks */
	else /* no erros */
	{
		error = FALSE;
		if (PUISTATE(s1_val) == PUI_NOOP) {

			/* treat it as a signal failure */
			fatal_error(slap, sla_id, SIG_ERROR, LCC_SIG_FAIL);
			action = NO_ACTION_CODE;
			error = TRUE;
		}
	}

	return error;

}  /* end error_check */



/**/
/*
 * NAME: fatal_error
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


static void
fatal_error(slap, sla_id, mode, index)
SLAIOP(slap);
int sla_id;
uint mode;
int index;
{
	/* first recover the dma */
	if (primary_hdr(sla_id) != NULL_HDR && sla_dma(sla_id)) {
		if (primary_hdr(sla_id) -> IMCS_OP_CODE == IMCS_SEND_CODE)
			sla_sdma_unh(slap, sla_id);
		else
			sla_rdma_unh(slap, sla_id);
	}

	sla_fatal(slap, sla_id, mode, index);

}  /* end fatal_error */


/**/
/*
 * NAME: xmt_ack
 *                                                                    
 * FUNCTION: transmits an ack frame
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: TRUE if the ack was transmitted
 *          FALSE otherwise
 */  

static boolean_t
xmt_ack(slap, sla_id, ack_frame, actionp)
SLAIOP(slap);
int sla_id;
uint ack_frame;
uint * actionp;
{
	uint s1_val;
	int start_rc;
	boolean_t rc;


	rc = TRUE;

	SOL_TRACE("txak",sla_id,0,0);
	switch (start_rc = sla_send_ack(slap, sla_id, SWAP(prhr_value.did_sid),
	    ack_frame, &s1_val) ) {
	case SLAS_DONE :
		if (error_check(slap, sla_id, s1_val, slap -> status2, actionp))
			rc = FALSE;
		break;
	case SLAS_FATAL :
	case SLAS_SCR_PROGRESS :
		*actionp = NO_ACTION_CODE;  /* recovery done in sla_start */
		rc = FALSE;
		break;
	case SLAS_DISCARDED :
	case SLAS_SCR_DONE :
	case SLAS_UNEXPD :
		*actionp = XMT_UD_CODE;
		rc = FALSE;
		break;
	default :
		PANIC("slaih (ident) : unknown return code from sla_send_ack");
	}  /* end switch on return code from send ack */

	return rc;
}  /* end xmt_ack */


/**/
/*
 * NAME: xmt_nack
 *                                                                    
 * FUNCTION: transmits a nack frame
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: default action unless an error occurs
 */  

static uint
xmt_nack(slap, sla_id, nack_code, default_action)
SLAIOP(slap);
int sla_id;
uint nack_code;
uint default_action;
{
	uint action;
	uint s1_val;
	int start_rc;

	SOL_TRACE("txnk",sla_id,0,0);
	action = default_action;  /* if there are no errors, that is */

	#ifdef PREFRAME
	sla_xmt_sof(slap, sla_id);  /* load regs with sof char */
	switch (sla_start(slap, sla_id, sla_tbl.sla[sla_id].ccr_all |
	    TRANSPARENT | SXMT) ) {
	case SLAS_DONE :
		s1_val = sla_spin(slap, sla_id);
		if (error_check(slap, sla_id, s1_val, slap -> status2, &action))
			return action;
		break;
	case SLAS_FATAL :
	case SLAS_SCR_PROGRESS :
		return NO_ACTION_CODE;
		break;
	case SLAS_SCR_DONE :
	case SLAS_DISCARDED :
		break;
	default : 
		PANIC("xmt_nack : unknown return code from sla_start");
	}  /* end switch */
	#endif PREFRAME

	switch (start_rc = sla_send_nack(slap, sla_id, SWAP(prhr_value.did_sid),
	    nack_code, &s1_val) ) {
	case SLAS_DONE :
		error_check(slap, sla_id, s1_val, slap -> status2, &action);
		break;
	case SLAS_FATAL :
	case SLAS_SCR_PROGRESS :
		action = NO_ACTION_CODE;
		break;
	case SLAS_DISCARDED :
	case SLAS_SCR_DONE :
	case SLAS_UNEXPD :
		action = XMT_UD_CODE;
		break;
	default :
		PANIC("slaih (ident) : unknown return code from sla_send_nack");
	}  /* end switch on return code from send nack */

	return action;
}  /* end xmt_nack */


/**/
/*
 * NAME: rcv_end
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
 * RETURNS: 
 */  

static uint
rcv_end(slap, sla_id)
SLAIOP(slap);
int sla_id;
{
	uint action;
	uint rc;
	boolean_t ack_sent;

	#ifdef DEBUG
	ack_sent = ((slap -> ccr & AUTO_ACK) != 0);
	#endif DEBUG

	SOL_TRACE("rxnd",sla_id,0,0);

	ASSERT(primary_hdr(sla_id) != NULL_HDR);
	ASSERT(primary_hdr(sla_id) -> IMCS_OP_CODE == IMCS_RECEIVE_CODE);

	/* fill in the length */
	primary_hdr(sla_id) -> IMCS_MSGLEN = CNT_RCVD(prhr_value.w5.count);
	primary_hdr(sla_id) -> outcome = IMCS_ACK;  /* set a good outcome */

	switch (primary_hdr(sla_id) -> IMCS_PROTOCOL) {

	case IMCS_RTR_RCV_CODE :
                primary_hdr(sla_id) -> outcome = IMCS_ACK;
		WAIT_SLA(action);  /* default action */
		switch (sla_tbl.sla[sla_id].ack_type) {
		case 0 :
			ASSERT(! ack_sent);
			/* send the ack */
			if (xmt_ack(slap, sla_id, DEV_RESP_ACK, &action))
				cdd_send_done(sla_id);  
					/* if sent, tell imcs io done */
			break;
		case ACK_SENT :
			ASSERT(ack_sent);
			sla_tbl.sla[sla_id].ack_type = 0;
			cdd_send_done(sla_id);
			break;
		case ACK_TURN :
			PANIC("ack turn to be sent on rcv rtr");
		default :
			PANIC("unknown ack type in rcv rtr");
		}  /* end switch on ack_type */
		break;

	case IMCS_RTS_RCV_CODE :
                primary_hdr(sla_id) -> outcome = IMCS_ACK;
		action = CONTINUE_CODE;
		WRT_B_REG(passive_hdr(sla_id), prhr_value.w4.B);
		switch (sla_tbl.sla[sla_id].ack_type) {
		case 0 :
			ASSERT(! ack_sent);
			/* send the ack */
			if (xmt_ack(slap, sla_id, DEV_RESP_ACK, &action))
				sla_rcv_done(sla_id, 
					EXTRACT_SADDR(prhr_value.did_sid));
			break;
		case ACK_SENT :
			ASSERT(ack_sent);
			sla_tbl.sla[sla_id].ack_type = 0;
			sla_rcv_done(sla_id, EXTRACT_SADDR(prhr_value.did_sid));
			break;
		case ACK_TURN :
			ASSERT(! ack_sent);
			sla_tbl.sla[sla_id].ack_type = 0;
			/* send the ack turnaround */
			if (xmt_ack(slap, sla_id, DEV_RESP_ACKT, &action))
				sla_rcv_done(sla_id, 
					EXTRACT_SADDR(prhr_value.did_sid));
			break;
		default :
			PANIC("unknown ack type in rcv rts");
		}  /* end switch on ack_type */
		break;

	case IMCS_RTSI_RCV_CODE :
		/* put B register value in header */
		WRT_B_REG(passive_hdr(sla_id), prhr_value.w4.B);
		/* ask imcs if the receiver has space for the message */
		rc = imcs_rcv_done(sla_id);
		action = CONTINUE_CODE;  /* default action */
		switch (sla_tbl.sla[sla_id].ack_type) {
		case 0 :
			ASSERT(! ack_sent);
			if (rc == IMCS_ACK) {
				if (xmt_ack(slap,sla_id,DEV_RESP_ACK, &action))
				  /* send the ack */
					sla_rcv_done(sla_id, 
					     EXTRACT_SADDR(prhr_value.did_sid));
			}
			else /* imcs wants to nack */
				action = xmt_nack(slap, sla_id, rc, action);
			break;
		case ACK_SENT :
			ASSERT(ack_sent);
			sla_tbl.sla[sla_id].ack_type = 0;
			ASSERT (rc == IMCS_ACK);  /* too late to nack */
			sla_rcv_done(sla_id, EXTRACT_SADDR(prhr_value.did_sid));
			break;
		case ACK_TURN :
			PANIC("ack turn to be sent on rcv rtsi");
		default :
			PANIC("unknown ack type in rcv rtsi");
		}  /* end switch on ack_type */
		break;

	default :
		PANIC("rcv_end: unknown protocol");
	}

	return action;

}  /* end rcv_end */


/**/
/*
 * NAME: snd_end
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
 * RETURNS: 
 */  

snd_end(slap, sla_id, rc, df_action)
SLAIOP(slap);
int sla_id;
ushort rc;
uint df_action;
{
	uint action;

	ASSERT(primary_hdr(sla_id) != NULL_HDR);

	SOL_TRACE("txnd",sla_id,0,0);
	primary_hdr(sla_id) -> outcome = rc;

	delay_sec = SEND_TIME_SEC;
	delay_nsec = SEND_TIME_NSEC;

	switch (primary_hdr(sla_id) -> IMCS_PROTOCOL) {
	case IMCS_RTR_SND_CODE :
		primary_hdr(sla_id) = NULL_HDR;
		sla_rtr_snd_done(sla_id, EXTRACT_SADDR(prhr_value.did_sid));
		action = CONTINUE_CODE;
		break;


	case IMCS_RTS_SND_CODE :
		primary_hdr(sla_id) = NULL_HDR;
		cdd_send_done(sla_id);
		action = df_action;
		/* can set the action to continue to
		   see what will happen in no wait is done
		   even when the ack turnaround is received,
		   or wait to wait all the times regardless
		   of type of ack 
		action = WAIT_CODE;
		action = CONTINUE_CODE;
		*/
		break;

	case IMCS_RTSI_SND_CODE :
		primary_hdr(sla_id) = NULL_HDR;
		cdd_send_done(sla_id);
		action = CONTINUE_CODE;
		break;

	case IMCS_RTR_RCV_CODE :
		primary_hdr(sla_id) = NULL_HDR;
		cdd_send_done(sla_id);
		WAIT_SLA(action);
		break;

	default :
		PANIC("snd_end: unknown protocol");
	}

	ASSERT(primary_hdr(sla_id) == NULL_HDR);

	return action;

}  /* end snd_end */


/**/
/*
 * NAME: doneint
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
 * RETURNS: 
 */  

static uint
doneint(slap, sla_id, s1_val, s2_val)
SLAIOP(slap);
int sla_id;
uint s1_val, s2_val;
{
	uint action;

	SOL_TRACE("dint",sla_id,s1_val,s2_val);
	poll_cnt[sla_id] = 0;

	switch (prhr_value.link_ctl) {

	case DEVICE_CTL_DATA :
		if (primary_hdr(sla_id) == NULL_HDR) {
			/* end of frame send */
			action = CONTINUE_CODE;
		}
		else {  /*  automatic sequence completion */

			switch (DEV_CTL(prhr_value.w3.dev_ctl)) {
			case DEV_RESP_ACK :
				/* end of rtsi send, rts send, rtr send */
				action =snd_end(slap, sla_id, (ushort) IMCS_ACK,
				    CONTINUE_CODE);
				break;

			case DEV_SEND :
				/* end of rtsi rcv, rts rcv, rtr rcv */
				action = rcv_end(slap, sla_id);
				break;

			case DEV_RESP_ACKT :
			case DEV_RESP_NAK :
			case DEV_REQ_SEND :
			case DEV_REQ_SEND_IMM :
			case DEV_REQ_RCV :
			case DEV_REQ_INT :
			case DEV_RESP_SEND :
				/* when these frame are received, the op-done 
				flag should not be set */
				action = XMT_UD_CODE;
				break;

			default :
				action = XMT_UD_CODE;
				break;
			} /* end switch on device ctl */

		} /* end of automtic sequence completion */
		break;

	case LINK_RESPONSE_RJT :
	case LINK_RESPONSE_BSY :
	case LINK_RESPONSE_ACK :
	case LINK_RESPONSE_TEST :
	case LINK_REQUEST_TEST :
	case DYNAMIC_PORT_RJT :
	case DYNAMIC_PORT_BSY :
	case LINK_REQUEST_SCN :
	case LINK_REQUEST_RESET :
	case LINK_REQUEST_ALA :
		/* when these frame are received, the op-done flag should 
		not be set */
		action = XMT_UD_CODE;
		break;

	default :
		action = XMT_UD_CODE;
		break;

	}  /* end switch on link ctl */

	return action;

}  /* end doneint */


/**/
/*
 * NAME: collision
 *                                                                    
 * FUNCTION: test for a collision
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: TRUE if collision detected
 *	    FALSE otherwise
 */  

static int
collision(slap, sla_id, s1_val, actionp)
SLAIOP(slap);
int sla_id;
uint s1_val;
uint *actionp;
{
	int start_rc;

	if (primary_hdr(sla_id) != NULL_HDR &&
	    primary_hdr(sla_id) == active_hdr(sla_id)) {
		primary_hdr(sla_id) = NULL_HDR;
		switch (sla_tbl.sla[sla_id].connection) {
		case POINT_TO_POINT :
			switch (LINKSTATE(s1_val)) {
			case LI_CWAIT:
				/*
		  if we detect a frame in connect wait state, then our frame was
		  busied, since the other side was in stopped state.
	 	    our side			other side
		    send connect frame		stopped
		    	------------------------->
						auto busy
			<------------------------
		    connect wait
						send connect frame
		  	<------------------------
		    UFI (this one)

		  We handle this interrupt ignoring the collision situation,
		  since the other side missed it.
		  */
				return FALSE;
				break;

			case LI_CTRY :
				if (my_addr(sla_id) < other_addr(sla_id) ) { 
						/* low side looses */
					*actionp = WAIT_CODE;
					delay_sec = 2;  /* this period shold be
					 greater than the hardware timeout 
					 period */
					delay_nsec = 0;
				}
				else /* high side retries */
					*actionp = CONTINUE_CODE;

				return TRUE;
				break;

			default :
				*actionp = XMT_UD_CODE;
				break;
			} /* end switch on link state */
			break;


		case SWITCH_PRESENT :
			/* the switch arbitrates all collisions */
			return FALSE;
			break;


		case DX_SWITCH_PRESENT :
			switch (LINKSTATE(s1_val)) {
			case LI_CWAIT:
				return FALSE;
				break;

			case LI_CTRY :
				*actionp = WAIT_CODE;
				/* always defer to the switch if the delay is 
				not greater than the timeout period of the NSC 
				switch's sla, then we will loop when a connect 
				frame is dropped (due to the fifo bug) */
				delay_sec = 5;
				delay_nsec = 0;
				return TRUE;
				break;

			default :
				*actionp = XMT_UD_CODE;
				break;
			} /* end switch on link state */
			break;


		default :
			PANIC("unknown connection in collision");
		}  /* end switch on connection */
	SOL_TRACE("colx",sla_id,s1_val,0);
	}
	else /* no potential collision */
		return FALSE;

}  /* end collision */


/**/
/*
 * NAME: rt_resp
 *                                                                    
 * FUNCTION: sends the response frame after the first interrupt
 *        for the rtr and rts protocol
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: action code
 */  

static uint
rt_resp(slap, sla_id, ack_type)
SLAIOP(slap);
int sla_id;
uchar ack_type;
{
	uint action = NO_ACTION_CODE;

	#ifdef PREFRAME
	sla_xmt_sof(slap, sla_id);  /* load regs with sof char */
	switch (sla_start(slap, sla_id, sla_tbl.sla[sla_id].ccr_all |
	    TRANSPARENT | SXMT) ) {
	case SLAS_DONE :
		{
			uint s1_val;
			s1_val = sla_spin(slap, sla_id);
			if (error_check(slap, sla_id, s1_val, slap -> status2, 
				&action))
				return action;
		}
		break;
	case SLAS_FATAL :
		return NO_ACTION_CODE;
		break;
	case SLAS_SCR_PROGRESS :
		primary_hdr(sla_id) = NULL_HDR;  
			/* we don't have to recover the dma */
		sla_rcv_error(sla_id);	/* give rcv hdr back top imcs */
		return NO_ACTION_CODE;
		break;
	case SLAS_SCR_DONE :
		sla_rcv_error(sla_id);	/* give rcv hdr back to imcs */
		return CONTINUE_CODE;
		break;
	case SLAS_DISCARDED :
	default : 
		PANIC("rts_int : unknown return code from sla_start");
	}  /* end switch */
	#endif PREFRAME

	primary_hdr(sla_id) -> imcs_address = SWAP(prhr_value.did_sid);

	cprgs(&(primary_hdr(sla_id) -> imcs_address),
	    &(slap -> thr.fr), SLA_CHPR_SZ);

	switch (sla_start(slap, sla_id,0) ) {
	case SLAS_DONE :
		sla_tbl.sla[sla_id].ack_type = ack_type;
		break;
	case SLAS_FATAL :
		break;
	case SLAS_SCR_PROGRESS :
		/* scr is in progress, channel will interrupt when done */
		primary_hdr(sla_id) = NULL_HDR;  /* no dma recovery needed */
		sla_rcv_error(sla_id);	/* give hdr back to imcs */
		break;
	case SLAS_DISCARDED :
		/* under noisy condition this can happen */
		primary_hdr(sla_id) = NULL_HDR;  /* no dma recovery needed */
		sla_rcv_error(sla_id);  /* give the header back to imcs */
		action = XMT_UD_CODE;
		break;
	case SLAS_SCR_DONE :
		sla_rcv_error(sla_id);	/* give hdr back to imcs */
		action = CONTINUE_CODE;
		break;
	case SLAS_UNEXPD :
		PANIC("rt_resp: sla_fstart returned SLAS_UNEXPD");
		break;
	default :
		PANIC("rt_resp: unknown return code from sla_fstart");
		break;
	}  /* end switch */

	return action;

}  /* end rt_resp */


/**/
/*
 * NAME: rts_int
 *                                                                    
 * FUNCTION: handles the first interrupt for rts receive
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: action code
 */  

static uint
rts_int(slap, sla_id)
SLAIOP(slap);
int sla_id;
{
	ushort rc;

	uint action;
	uchar ack_type;


	#ifdef DEBUG
	if (passive_hdr(sla_id) != NULL_HDR)
		if (passive_hdr(sla_id) -> IMCS_PROTOCOL == IMCS_RTSI_RCV_CODE)
			rc = 
			  cdd_schn_int(sla_id, DEV_SUBCH(prhr_value.w3.dev_ctl),
			    		prhr_value.w4.B, IMCS_RECEIVE_CODE);
		else {
			if (passive_hdr(sla_id) -> IMCS_PROTOCOL == 
							IMCS_RTS_RCV_CODE) {
				ASSERT(passive_hdr(sla_id)->IMCS_SUBCHANNEL == 
					     DEV_SUBCH(prhr_value.w3.dev_ctl) );
				rc = IMCS_ACK;
			}
			else {
		/* we had previously received an rtr frame, and responded to it.
		 However the last ack was not received.  Now return the send
		 header to imcs, and do the receive for this rts sequence */
				ASSERT (passive_hdr(sla_id)->IMCS_PROTOCOL == 
							IMCS_RTR_SND_CODE);
				sla_rcv_error(sla_id);
				rc = cdd_schn_int(sla_id, 
					DEV_SUBCH(prhr_value.w3.dev_ctl),
				        prhr_value.w4.B, IMCS_RECEIVE_CODE);
			}
		}
	else
		rc = cdd_schn_int(sla_id, DEV_SUBCH(prhr_value.w3.dev_ctl),
		    prhr_value.w4.B, IMCS_RECEIVE_CODE);
	#else
	if (passive_hdr(sla_id) != NULL_HDR &&
	    passive_hdr(sla_id) -> IMCS_PROTOCOL != IMCS_RTSI_RCV_CODE)
		/* we had started an automatic sequence, and this somehow
	     did not complete (due to lost acks) */
		sla_rcv_error(sla_id);  /* return header to imcs */
	rc = cdd_schn_int(sla_id, DEV_SUBCH(prhr_value.w3.dev_ctl),
	    prhr_value.w4.B, IMCS_RECEIVE_CODE);
	#endif

	ASSERT (rc == IMCS_ACK || rc == IMCS_NACK_NS);

	if (rc == IMCS_ACK) { /* send a response */
		primary_hdr(sla_id) = passive_hdr(sla_id);

		if (active_hdr(sla_id) != NULL_HDR) {
			primary_hdr(sla_id)->imcs_ccr |= 
						sla_tbl.sla[sla_id].ccr_all;
			primary_hdr(sla_id) -> imcs_ccr &= ~(AUTO_ACK);
			ack_type = ACK_TURN;
		}
		else {
			primary_hdr(sla_id)->imcs_ccr |= 
					sla_tbl.sla[sla_id].ccr_all | AUTO_ACK;
			ack_type = ACK_SENT;
		}

		action = rt_resp(slap, sla_id, ack_type);
	}
	else { /*  send a nack frame */
		primary_hdr(sla_id) = NULL_HDR;
		action = xmt_nack(slap, sla_id, (uint) rc, CONTINUE_CODE);
	}

	return action;

}  /* end rts_int */



/**/
/*
 * NAME: rtr_int
 *                                                                    
 * FUNCTION: handles the first interrupt for rtr send
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: action code
 */  


static uint
rtr_int(slap, sla_id)
SLAIOP(slap);
int sla_id;
{
	ushort rc;
	uint action;

	#ifdef DEBUG
	if (passive_hdr(sla_id) != NULL_HDR)
		if (passive_hdr(sla_id) -> IMCS_PROTOCOL == IMCS_RTSI_RCV_CODE)
			rc=cdd_schn_int(sla_id,DEV_SUBCH(prhr_value.w3.dev_ctl),
			    prhr_value.w4.B, IMCS_SEND_CODE);
		else {
			ASSERT(passive_hdr(sla_id)->IMCS_PROTOCOL == 
							IMCS_RTR_SND_CODE);
			if (passive_hdr(sla_id) -> IMCS_SUBCHANNEL ==
			    DEV_SUBCH(prhr_value.w3.dev_ctl) ) {
				rc = IMCS_ACK;
			}
			else {
		/* we had previously received an rtr frame, and responded to it.
		 However the last ack was not received.  Now return the send
		 header to imcs, and do the send for this new rtr sequence */
				sla_rcv_error(sla_id);
				rc = cdd_schn_int(sla_id, 
					DEV_SUBCH(prhr_value.w3.dev_ctl),
				        prhr_value.w4.B, IMCS_SEND_CODE);
			}
		}
	else
		rc = cdd_schn_int(sla_id, DEV_SUBCH(prhr_value.w3.dev_ctl),
		    prhr_value.w4.B, IMCS_SEND_CODE);
	#else
	if (passive_hdr(sla_id) != NULL_HDR &&
	    passive_hdr(sla_id) -> IMCS_PROTOCOL != IMCS_RTSI_RCV_CODE)
		/* we had started an automatic sequence, and this somehow
	     did not complete (due to lost acks) */
		sla_rcv_error(sla_id);  /* return header to imcs */
	rc = cdd_schn_int(sla_id, DEV_SUBCH(prhr_value.w3.dev_ctl),
	    prhr_value.w4.B, IMCS_SEND_CODE);
	#endif

	ASSERT (rc == IMCS_ACK || rc == IMCS_NACK_NS);

	if (rc == IMCS_ACK) { /* send a response (send frame) */
		primary_hdr(sla_id) = passive_hdr(sla_id);

		primary_hdr(sla_id) -> imcs_ccr |= sla_tbl.sla[sla_id].ccr_all;

		action = rt_resp(slap, sla_id, 0);
	}
	else { /*  send a nack frame */
		primary_hdr(sla_id) = NULL_HDR;  /* no dma recovery needed */
		action = xmt_nack(slap, sla_id, (uint) rc, CONTINUE_CODE);
	}

	return action;

}  /* end rtr_int */



/**/
/*
 * NAME: rtsi_int
 *                                                                    
 * FUNCTION: handles the interrupt for rtsi receive
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: action code 
 */  

static uint
rtsi_int(slap, sla_id)
SLAIOP(slap);
int sla_id;
{
	ushort rc;
	int start_rc;
	uint action;
	uint s1_val;
	struct sl_frame thr_val;


	if (passive_hdr(sla_id) == NULL_HDR) {
		/* channel had been set up in listen, not immediate,
	     mode --> cannot receive yet for rtsi */
		action=xmt_nack(slap,sla_id,(uint)IMCS_NACK_RTSI,CONTINUE_CODE);
	}
	else {  /* passive not null */
		if (passive_hdr(sla_id)->IMCS_PROTOCOL != IMCS_RTSI_RCV_CODE) {
		/* we had previously received an rtr frame, and responded to it.
	       However the last ack was not received.  Now return the send
	       header to imcs, and do the receive for this rtsi sequence */
			ASSERT (passive_hdr(sla_id) -> IMCS_PROTOCOL == 
							IMCS_RTR_SND_CODE);
			sla_rcv_error(sla_id);
		}
		if (passive_hdr(sla_id) == NULL_HDR) {
			primary_hdr(sla_id)=NULL_HDR;/*no dma recovery needed */
			action = xmt_nack(slap, sla_id, (uint) IMCS_NACK_RTSI,
			    CONTINUE_CODE);
		}
		else {
			ASSERT(passive_hdr(sla_id) -> IMCS_PROTOCOL == 	
							IMCS_RTSI_RCV_CODE);
			action = NO_ACTION_CODE;
			primary_hdr(sla_id) = passive_hdr(sla_id);
			LOAD_TAGS(slap, sla_id);	
			/* load the tag table from the header */
			/* send a response to send */
			thr_val.did_sid = SWAP(prhr_value.did_sid);
			thr_val.link_ctl = DEVICE_CTL_DATA;
			thr_val.w3.dev_ctl = DEV_RESP_SEND;
			thr_val.w4.info = THR_DONT_CARE;
			thr_val.w5.count = THR_DONT_CARE;
			WRITE_THR(slap, thr_val);
			switch (sla_start(slap, sla_id, 
				sla_tbl.sla[sla_id].ccr_all | XMT_AUTO_CODE) ) {
			case SLAS_DONE :
			case SLAS_FATAL :
				break;
			case SLAS_SCR_PROGRESS :
		/* we were unable to start the channel to send the response
		   send frame.  This mean that the data transfer did not
		   start, and the dma cannot be hung.  Set primary to null
		   to avoid the dma recovery */
				primary_hdr(sla_id) = NULL_HDR;
				break;
			case SLAS_DISCARDED :
		/* under noisy condition this happens.  This automatic
		   sequence should be aborted */
				primary_hdr(sla_id) = NULL_HDR;
				action = XMT_UD_CODE;
				break;
			case SLAS_SCR_DONE :
		/* we were unable to start the channel to send the response
		   send frame.  This mean that the data transfer did not
		   start, and the dma cannot be hung.  Continue with normal
		   operation */
				action = CONTINUE_CODE;
				break;
			case SLAS_UNEXPD :
				PANIC("rtsi_int: sla_start ret. SLAS_UNEXPD");
				break;
			default :
				PANIC("rtsi_int: unknown ret code from sla_start");
			}  /* end switch */
		}
	}

	return action;

}  /* end rtsi_int */



/**/
/*
 * NAME: xmit_link_ack 
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
 * RETURNS: 
 */  


static uint
xmt_link_ack(slap, sla_id, address, default_action)
SLAIOP(slap);
int sla_id;
uint address;
uint default_action;
{
	uint action;
	uint s1_val;
	int start_rc;

	action = default_action;  /* if there are no errors, that is */

	switch (start_rc = sla_send_link_ack(slap, sla_id, address, &s1_val) ) {
	case SLAS_DONE :
		error_check(slap, sla_id, s1_val, slap -> status2, &action);
		break;
	case SLAS_FATAL :
	case SLAS_SCR_PROGRESS :
		action = NO_ACTION_CODE;
		break;
	case SLAS_DISCARDED :
	case SLAS_SCR_DONE :
		break;
	default :
		PANIC("slaih (ident) : unknown return code from sla_send_rjt");
	}  /* end switch on return code from send link ack */

	return action;

}  /* end xmt_link_ack */


/**/
/*
 * NAME: xmt_rjt 
 *                                                                    
 * FUNCTION: transmits a reject frame
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: action code
 */  

static uint
xmt_rjt(slap, sla_id, count, info, default_action)
SLAIOP(slap);
int sla_id;
uint count, info;
uint default_action;
{
	uint action;
	uint s1_val;
	int start_rc;

	action = default_action;  /* if there are no errors, that is */

	switch (start_rc = sla_send_rjt(slap, sla_id, count, info, &s1_val) ) {
	case SLAS_DONE :
		error_check(slap, sla_id, s1_val, slap -> status2, &action);
		break;
	case SLAS_FATAL :
	case SLAS_SCR_PROGRESS :
		action = NO_ACTION_CODE;
		break;
	case SLAS_DISCARDED :
	case SLAS_SCR_DONE :
	case SLAS_UNEXPD :
		action = XMT_UD_CODE;
		break;
	default :
		PANIC("slaih (ident) : unknown return code from sla_send_rjt");
	}  /* end switch on return code from send rjt */

	return action;

}  /* end xmt_rjt */


/**/
/*
 * NAME: bsy_retry
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
 * RETURNS: 
 */  

static uint
bsy_retry(slap, sla_id, dsec, dnsec)
SLAIOP(slap);
int sla_id;
int dsec, dnsec;
{
	int start_try;
	boolean_t done;
	TVARS tvar;
	sla_tbl.sla[sla_id].event_count++;
	if (sla_tbl.sla[sla_id].event_count > 1000) {
		sla_addr_unh(slap, sla_id);
		return NO_ACTION_CODE;
	}

	for (start_try = 0, done = FALSE;
	    start_try < MAX_START_TRY && ! done;
	    start_try++)
		switch (sla_start(slap, sla_id,	
		    XMT_CON_CODE | AUTO_SEQ | RETRY |
		    sla_tbl.sla[sla_id].ccr_con) ) {
		case SLAS_DONE :
		case SLAS_FATAL :
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
		case SLAS_UNEXPD :
		/* this is another case of collision where the other side
	         connect frame reached us at the exact time that the
	         channel was started.  There should be an interrupt
	         pending for this message. */
			done = TRUE;
			break;
		case SLAS_DISCARDED :
		case SLAS_SCR_DONE :
			break;
		default :
			PANIC("start_send_auto: unknown return code from sla_start");
		}  /* end switch */

	if (! done) /* the channel would not start */
		sla_fatal(slap, sla_id, DID_NOT_START, OTH_NO_START);
	else {
		/* start a timer to check that the sla does not
	     get stuck on this destination address */
		tvar.id=sla_id;
		tvar.func = SLA_ADDR_UNH;
		tvar.setup = TRUE;
		SLA_TIMER(sla_id, tvar, tservice, dsec, dnsec);
	}

	return NO_ACTION_CODE;

}  /* end bsy_retry */


/**/
/*
 * NAME: ufint
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
 * RETURNS: 
 */  

static uint
ufint(slap, sla_id, s1_val, s2_val)
SLAIOP(slap);
int sla_id;
uint s1_val, s2_val;
{
	uint action;

SOL_TRACE("ufnt",sla_id,prhr_value.link_ctl,prhr_value.w3.dev_ctl);

	switch (prhr_value.link_ctl) {

	case DEVICE_CTL_DATA :

		switch (DEV_CTL(prhr_value.w3.dev_ctl)) {

		case DEV_RESP_NAK :
			/*end of rtsi,rts and rtr send,and also end of rtr rcv*/
			action=snd_end(slap,sla_id, (ushort) prhr_value.w4.info,
			    CONTINUE_CODE);
			break;

		case DEV_REQ_SEND :
			if (! collision(slap, sla_id, s1_val, &action))
				action = rts_int(slap, sla_id);
			break;

		case DEV_REQ_RCV :
			if (! collision(slap, sla_id, s1_val, &action))
				action = rtr_int(slap, sla_id);
			break;

		case DEV_REQ_SEND_IMM :
			if (! collision(slap, sla_id, s1_val, &action))
				action = rtsi_int(slap, sla_id);
			break;

		case DEV_REQ_INT :
			/* send a device ack frame */
			action = CONTINUE_CODE;
			xmt_ack(slap, sla_id, DEV_RESP_ACK, &action);
			break;

		case DEV_SEND :
			/* this frame is expected */
			if (SCH_SDMA == DEV_SUBCH(prhr_value.w3.dev_ctl)) {
				/* this is the send sdma recovery frame */
				action = XMT_UD_CODE;
			}
			else {
				action = XMT_UD_CODE;
			}
			break;

		case DEV_RESP_ACKT :
			/* end of rtsi send, rts send, rtr send */
			action=snd_end(slap,sla_id,(ushort)IMCS_ACK, WAIT_CODE);
			break;

		case DEV_RESP_SEND :
		case DEV_RESP_ACK :
			/* these frame are expected */
			action = XMT_UD_CODE;
			break;

		default :
			action = XMT_UD_CODE;
			break;

		} /* end switch on device ctl */
		break;


	case LINK_RESPONSE_BSY :
		/* Retry in a while, listening */
		if (primary_hdr(sla_id) != NULL_HDR &&
		    primary_hdr(sla_id) == active_hdr(sla_id) ) {
			action = bsy_retry(slap, sla_id, PBSY_SEC, PBSY_NSEC);
		}
		else {
			action = XMT_UD_CODE;
		}
		break;


	case DYNAMIC_PORT_BSY :
		if (primary_hdr(sla_id) != NULL_HDR &&
		    primary_hdr(sla_id) == active_hdr(sla_id) ) {
			if (sla_get_next_addr(sla_id) )
				action = CONTINUE_CODE;
			else {
				/* start the channel to do a retry */
				action=
			 	     bsy_retry(slap,sla_id,PBSY_SEC,PBSY_NSEC);
			}
		}
		else {
			action = XMT_UD_CODE;
		}
		break;


	case LINK_RESPONSE_RJT :
		switch(RJT_INFO1(prhr_value.w3.info)) {
		case RJT_LINK_SIGERR :
		case RJT_CODE_VERR :
		case RJT_FRAME_SERR :
		case RJT_CRC_ERR :
		case RJT_LINK_AERR :
		case RJT_RES_FERR :
			action = XMT_UD_CODE;
			break;

		case RJT_UNINS_LCF :
		/* we must have sent a garbage frame, or the link messed
		   it up */
			action = XMT_UD_CODE;
			break;

		case RJT_NON_SPEC_CODE :
			action = XMT_UD_CODE;
			{
				int code;
				code = 0;
				if (CNT_RCVD(prhr_value.w5.count) == 2)
					code = RJT_INFO2(prhr_value.w3.info);
				switch (code) {
				case RJT_LCTL_FUN_ERR :
			/* we are sending frames other that ala to a non
			   identified site */
					if (sla_tbl.sla[sla_id].connection == 
							      SWITCH_PRESENT) {
SOL_TRACE("rlfe",sla_id,prhr_value.w3.info,0);
						sla_address_error(sla_id);
					}
					else {
				/* go back to the not identified state */
						fatal_error(slap, sla_id, 
						        ALA_RECEIVED, ALA_RCVD);
					}
					break;
				case RJT_RST_DISABLE :
			/* we sent a reset frame, but the other side had the
			   reset function disabled */
				case RJT_UNINS_DCF :
			/* we either sent a garbage data frame or the
			   link messed it up */
				default :
					break;
				}  /* end switch on code */
			}
			break;

		default :
			action = XMT_UD_CODE;
			break;
		}  /* end switch on first byte of reject info */
		break;


	case DYNAMIC_PORT_RJT :
		action = XMT_UD_CODE;
		switch(RJT_INFO1(prhr_value.w3.info)) {
		case PRJT_ADDR_ERR :
			if (CNT_RCVD(prhr_value.w5.count) == 2) {
				int code;
				code = RJT_INFO2(prhr_value.w3.info);
				code &= 0xff7fffff;  /* take out the S bit */
				switch (code) {
				case PRJT_DID_ADDR_ERR :
SOL_TRACE("pdae",sla_id,prhr_value.w3.info,0);
					sla_address_error(sla_id);
					break;
				case PRJT_SID_ADDR_ERR :
			/* go back to not-identified state to do another ala */
					fatal_error(slap, sla_id, ALA_RECEIVED,
								 ALA_RCVD);
					break;
				default :
					PANIC("unknown address source");
					break;
				}  /* end switch on code */
			}
			else PANIC("with count less than 2");
			break;
		case PRJT_DSINT_REQ :
		case PRJT_UNDA_ERR :
		case PRJT_DESTP_MALF :
                	if (primary_hdr(sla_id) == active_hdr(sla_id)) {
				primary_hdr(sla_id) = NULL_HDR;
				sla_address_error(sla_id);
			}
			break;
		default :
			action = XMT_UD_CODE;
			break;
		}
		break;


	case LINK_REQUEST_RESET :
		primary_hdr(sla_id) = NULL_HDR;
		if (slap -> ccr & RSTD) {
			/* reception of the reset frame was disabled */
			action = xmt_rjt(slap, sla_id, 2, RJT_NON_SPEC_CODE |
					 RJT_RST_DISABLE, CONTINUE_CODE);
		}
		else {
			/* reception of the reset frame was enabled */
			action = xmt_link_ack(slap, sla_id, RST_ACK_ADDRESS,
						 CONTINUE_CODE);
		}
		break;


	case LINK_REQUEST_TEST :
	case LINK_REQUEST_SCN :
		action = XMT_UD_CODE;
		break;


	case LINK_REQUEST_ALA :
		switch (sla_tbl.sla[sla_id].connection) {
		case POINT_TO_POINT :
			fatal_error(slap, sla_id, ALA_RECEIVED, ALA_RCVD);
			action = do_notident(slap, sla_id, s1_val, s2_val);
			break;
		case SWITCH_PRESENT :
			/* this does happen!  Maybe the switch has this port
		   wrapped */
			action = XMT_UD_CODE;
			break;
		case DX_SWITCH_PRESENT :
			PANIC("ala received through a nsc switch");
			break;
		}  /* end switch on connection */
		break;


	case LINK_RESPONSE_ACK :
	case LINK_RESPONSE_TEST :
		action = XMT_UD_CODE;
		break;


	default :
		primary_hdr(sla_id) = NULL_HDR;
		action = xmt_rjt(slap, sla_id, 1, RJT_UNINS_LCF, CONTINUE_CODE);
		break;

	}  /* end switch on link ctl */

	return action;

}  /* end ufint */


/**/
/*
 * NAME: timedint
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
 * RETURNS: 
 */  

static uint
timedint(slap, sla_id, s1_val, s2_val)
SLAIOP(slap);
int sla_id;
uint s1_val, s2_val;
{
	uint action;
	if (SOLIDLY_ON(slap, s2_val, LINK_SIG_ERR)) {
		fatal_error(slap, sla_id, SIG_ERROR, LCC_SIG_FAIL);
		action = NO_ACTION_CODE;
	}
	else {
		action = CONTINUE_CODE;
		if (primary_hdr(sla_id) != NULL_HDR) {
			if (LINKSTATE(s1_val) == LI_RUNNING)
			/* send ud, to synchronize with the other side,
		 then do the dma recovery */
				action = XMT_UD_CODE;
			else { 
		/* the time out could be due to the other side being down */
				if (s1_val & PRIMARY && prhr_value.link_ctl == 
							LINK_RESPONSE_BSY) {
					INIT_REGS(slap);
				}
				if (primary_hdr(sla_id) -> IMCS_GO == 
								IMCS_DELAY)
					sla_rcv_error(sla_id);	 
					/* this will fix up headers, etc. */

				primary_hdr(sla_id) = NULL_HDR;
			}
		}  /* end primary is not null */
		else {  /* primary hdr null */
			ASSERT(LINKSTATE(s1_val) != LI_RUNNING);
		}
		if (sla_tbl.sla[sla_id].connection == SWITCH_PRESENT)
			/* if this timeout is due to the fifo bug and the
	       switch has seen the connect frame then these ud should
	       drop the connection */
			action = XMT_UD_CODE;
	}

	return action;

}  /* end timedint */


/**/
/*
 * NAME: scrint
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
 * RETURNS: 
 */  

static uint
scrint(slap, sla_id, s1_val, s2_val)
SLAIOP(slap);
int sla_id;
uint s1_val, s2_val;
{
	uint action;
	TVARS tvar;

	sla_scr(sla_id) = FALSE;
	sla_tbl.sla[sla_id].ack_type = 0;  /* null out this field,
					      which might be set, 
					      since sequence aborted */

	if (s1_val & SCR_ACTV) /* scr still active */
		return XMT_UD_CODE;

	/* set the default action */
	delay_sec = 0;
	delay_nsec = ONE_MILLI;
	action = WAIT_CODE;

	if (primary_hdr(sla_id) != NULL_HDR) {

		/* do the dma recovery if needed */
		if (sla_dma(sla_id)) {
			if (primary_hdr(sla_id) -> IMCS_OP_CODE == 
							IMCS_SEND_CODE) {
				switch (sla_tbl.sla[sla_id].connection) {

				case SWITCH_PRESENT :
					/* do the recovery */
					sla_sdma_unh(slap, sla_id);
					/* send ud transparent */
					sla_xmt_cont(slap, sla_id,UD_SEQ_ADDR);
					tvar.id=sla_id;
					tvar.func = RESTART_RECPOLL;
					tvar.setup = TRUE;
					SLA_TIMER(sla_id, tvar,tservice, 
							POLL2_SEC, POLL2_NSEC);
					action = NO_ACTION_CODE;
					sla_scr(sla_id) = TRUE;  
						/* prevent uses of the sla */
					break;
				case POINT_TO_POINT :
				case DX_SWITCH_PRESENT :
			/* do the recovery later to make sure that the
		     slower nodes get the hint that we are in
		     recovery mode */
					tvar.id=sla_id;
					tvar.func = RESTART_RECOVERY;
					tvar.setup = TRUE;
					SLA_TIMER(sla_id, tvar,tservice, 
						   POLL2_SEC, POLL2_NSEC / 2);
					action = NO_ACTION_CODE;
					sla_scr(sla_id) = TRUE;  
					/* prevent uses of the sla */
					break;
				default :
					PANIC("unknown connection in scrint dma recovery");
				}  /* end switch on connection type */
			}
			else {
				sla_rdma_unh(slap, sla_id);
			}
		}  /* end dma recovery */
		else {
			/* no dma recovery.  Change the default action */
			action = CONTINUE_CODE;
		}

		/* see if we should wait for the other side dma recovery */
		if (primary_hdr(sla_id) -> IMCS_OP_CODE == IMCS_RECEIVE_CODE &&
		    sla_sdma_wait(sla_id) ) {
			/*leave sla stopped, and try to catch the nos later */
			tvar.id=sla_id;
			tvar.func = RESTART_RECPOLL;
			tvar.setup = TRUE;
			SLA_TIMER(sla_id,tvar,tservice,POLL2_SEC,POLL2_NSEC*2);
			action = NO_ACTION_CODE;
			sla_scr(sla_id) = TRUE;  /* prevent uses of the sla */
		}

		if (! primary_hdr(sla_id) -> IMCS_GO)
			sla_rcv_error(sla_id);	
			 /* this will fix up headers, etc. */
		primary_hdr(sla_id) = NULL_HDR;
	}
	else if (! sla_dma(sla_id) && 
	    ! sla_sdma_wait(sla_id) )
		/* change the default action */
		action = CONTINUE_CODE;

	ASSERT(primary_hdr(sla_id) == NULL_HDR);

	return action;

}  /* end scrint */


/**/
/*
 * NAME: do_notsync 
 *                                                                    
 * FUNCTION: starts the ala sequence
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
    side effects:   if the link fails, then the sla is marked failed
                if the synchronization succedes, then the sla state is changed
                to not identified.

    We have synchronised successfully - now send ALA; this is a link control
    frame with DID of FF00 and SID of 0000

    Either there is a switch, in which cases we will receive next either
    a ack frame containing our address or a rjt frame not due to link
    protocol, or there is no switch in which case we will receive a rjt
    frame due to link protocol.
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: SYNC -- syncronization must be re-initiated
          XMT_UD -- start the ud sequence
          NO_ACTION -- the sla driver failed or it sent the ala
 */  


uint do_notsync(SLAIOP(slap) , int sla_id, uint s1_val, uint s2_val)
{
	uint error, action;
	TVARS tvar;

	action = NO_ACTION_CODE;

	if (! sla_started(sla_id) ) return action;
	SYS_SOL_TRACE("nsyn",sla_id,s1_val,s2_val);

	if (s2_val & LINK_SIG_ERR) {
		if (PUISTATE(s1_val) == PUI_NOOP)
			if (s1_val & CHAN_CHK)
				sla_channel_check(slap, sla_id, s2_val);
			else
				action = SYNC_CODE;
		else
			action = SYNC_CODE;
	}
	else {
		if (s1_val & LINK_CHK) {
			if (s2_val & XMT_FAIL) {  /* driver failed */
				sla_fatal(slap,sla_id, DRV_ERROR, LCC_DRV_FAIL);
			}
			else if (s2_val & SIG_FAIL) {  /* signal was lost */
				sla_fatal(slap,sla_id, SIG_ERROR, LCC_SIG_FAIL);
			}
			else if (s2_val & NOPSQ_REC) {
				sla_fatal(slap,sla_id, NOP_ERROR, LCC_NOPSQ_RC);
			}
			else if (s2_val & OFFSQ_REC) {
				sla_fatal(slap,sla_id, OFF_ERROR, LCC_OFFSQ_RC);
			}
			else if (s2_val & UDSEQ_REC) {
				action = XMT_UD_CODE;
			}
			else if (s1_val & (FRM_DISC | BSY_DISC | RJT_DISC) ) {
				tvar.id=sla_id;
				tvar.func = START_ALA;
				tvar.setup = FALSE;
				SLA_TIMER(sla_id, tvar,tservice,SYNC_TIME_SEC,
							 SYNC_TIME_NSEC);
			}
			else {
				action = XMT_UD_CODE;
			}
		} /* end link check */
                else {  /* no errors */
                        if (SOLIDLY_ON(slap, s2_val, UDSEQ_REC)) {
                                sla_xmt_cont(slap, sla_id,UDR_SEQ_ADDR);
                                tvar.id=sla_id;
                                tvar.func = RESTART_CONTSCR;
                                tvar.setup = TRUE;
                                SLA_TIMER(sla_id, tvar,tservice, 0,
                                                                10 * ONE_MILLI);
                        }
                        else {
                                tvar.id=sla_id;
                                tvar.func = START_ALA;
                                tvar.setup = FALSE;
                                SLA_TIMER(sla_id, tvar,tservice,SYNC_TIME_SEC,
                                                         SYNC_TIME_NSEC);
                        }
                }  /* end no errors */

	}  /* end signal ok */

	return action;

}  /* end do_notsync */


/**/
/*
 * NAME: ala_error_check 
 *                                                                    
 * FUNCTION: tests for an ala error.  Sets action if there is an error

        error checking is sligtly different for unidentinfied slas, 
	where some errors are normal.  We sort through the error 
	conditions setting error to
        FALSE if there are no errors, or if the errors are "acceptable";
        1 for fatal errors;
        2 if there is an error that should be handled by re-issuing the
                syncronization command;
        3 for errors that are handled specially
        4 for errors that should be handled by starting a SCR sequence
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 	TRUE if error detected
 *		FALSE otherwise 
 */  

static boolean_t
ala_error_check(slap, sla_id, s1_val, s2_val, actionp)
int sla_id;
SLAIOP(slap);
uint s1_val, s2_val;
uint *actionp;
{
	boolean_t error;
	uint action;
	union {
		uint tagword;
		struct tag tcw;
	} mytag;


	error = FALSE;
	action = XMT_UD_CODE;

	if (s1_val & PROG_CHK) {
        SOL_TRACE("ale1",sla_id,s1_val,s2_val);

		if (s2_val & RBUFCHK) {
			error = TRUE;
		}
		if (s2_val & XBUFCHK) {
			PANIC("ala_error_check: program check/transmit buffer check");
		}
		if (s2_val & CMDCHK) {
			PANIC("ala_error_check: program check/command check");
		}
	}  /* end PROG_CHK */


	if (s1_val & LINK_CHK) {
        SOL_TRACE("ale2",sla_id,s1_val,s2_val);


		if (s1_val & FRM_DISC) {
			error = TRUE;
		}

		if (s1_val & BSY_DISC) {
			error = TRUE;
		}

		if (s1_val & RJT_DISC) {
			error = TRUE;
		}

		if (s2_val & RST_RCVD) {
			error = TRUE;
		}

		if (s2_val & ADDR_MIS && ! (s2_val & LINK_SIG_ERR) ) {
			if (!(s1_val & PRIMARY)) error = TRUE;
		}

		if (s2_val & SNDCNTCHK) {
			error = TRUE;
		}

		if (s1_val & PRIMARY) { /* prhr loaded */
			uint frame_status;
			frame_status = slap -> prhr.fr.w5.rfrst;
			if (EOF_RCVD(frame_status) == EOF_ABORT) {
				error = TRUE;
			}
			if (frame_status & CRC_CHK) {
				error = TRUE;
			}
			if (frame_status & STRUCT_CHK) {
				error = TRUE;
			}
			if (frame_status & CDVL_CHK) {
				error = TRUE;
			}
			if (frame_status & SFRS_CHK) {
				error = TRUE;
			}
		}

		if (s2_val & XMT_FAIL) {  /* driver failed */
			sla_fatal(slap, sla_id, DRV_ERROR, LCC_DRV_FAIL);
			action = NO_ACTION_CODE;
			error = TRUE;
		}
		else if (SIGNAL_FAILURE(slap, s2_val) ||
		    SOLIDLY_ON(slap, s2_val, LINK_SIG_ERR) ) {
			error = TRUE;
			sla_fatal(slap, sla_id, SIG_ERROR, LCC_SIG_FAIL);
			action = NO_ACTION_CODE;
		}
		else if (s2_val & NOPSQ_REC) {
			error = TRUE;
			sla_fatal(slap, sla_id, NOP_ERROR, LCC_NOPSQ_RC);
			action = NO_ACTION_CODE;
		}
		else if (s2_val & OFFSQ_REC) {
			error = TRUE;
			sla_fatal(slap, sla_id, OFF_ERROR, LCC_OFFSQ_RC);
			action = NO_ACTION_CODE;
		}
	}  /* end LINK_CHK */


	if (s1_val & CHAN_CHK) {
        SOL_TRACE("ale3",sla_id,s1_val,s2_val);
		action = NO_ACTION_CODE;
		error = TRUE;
		sla_channel_check(slap, sla_id, s2_val);
	}  /* end CHAN_CHK */


	if (error) {
		*actionp = action;

	}

	return error;

}  /* end ala_error_check */



/**/
/*
 * NAME: xmt_reject 
 *                                                                    
 * FUNCTION: transmits a reject frame
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: action code
 */  

static uint
xmt_reject(slap, sla_id, info)
SLAIOP(slap);
int sla_id;
uint info;
{
	uint s1_val;
	uint action;

	ala_state(sla_id) = LISTENED_FOR_ALA;

	switch (sla_send_rjt(slap, sla_id, 2, info, &s1_val) ) {
	case SLAS_DONE :
		if (! ala_error_check(slap, sla_id, s1_val, slap -> status2, 
								&action) )
			action = LFA_CODE;
		/* otherwise ala_error_check has already set action */
		break;
	case SLAS_FATAL :
	case SLAS_SCR_PROGRESS :
		action = NO_ACTION_CODE;
		break;
	case SLAS_DISCARDED :
		/* intentionally set action to lfa by going through */
	case SLAS_SCR_DONE :
		action = LFA_CODE;
		break;
	case SLAS_UNEXPD :
		action = XMT_UD_CODE;
		break;
	default :
		PANIC("send_rjt: unknown return code from sla_send_rjt");
	}  /* end switch */

	return action;

}  /* end xmt_reject */

/**/
/*
 * NAME: do_ala_rcvd
 *                                                                    
 * FUNCTION: handles frames received during ala sequence
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) if the ala succedes, then the sla state is changed to identified.
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
	  LFA_CODE -- listen for the for a while, then send an ala
          XMT_UD_CODE -- start channel in scr mode
          CONTINUE_CODE -- normal link operation (used when ala completes)
          codes returned by ala_error_check
 */  


static uint
do_ala_rcvd(slap, sla_id)
SLAIOP(slap);
int sla_id;
{
	uint s1_val;
	struct sl_frame prhr_value;
	uint action;
	int rc;
	struct sla_addrmap slaam;


	action = NO_ACTION_CODE;

	READ_PRHR(slap, prhr_value);

        SOL_TRACE("alrx",sla_id,prhr_value.link_ctl,0);

	switch (prhr_value.link_ctl) {
	case LINK_REQUEST_TEST :
	case LINK_RESPONSE_TEST :
	case LINK_REQUEST_SCN :
	case LINK_REQUEST_RESET :
		action = XMT_UD_CODE;
		break;

	case DEVICE_CTL_DATA :
	/* we lost the ack to the ala frame.  Send a reject indicating the
	       non-specific link protocol error and the link control function
	       error (see architecture doc. p. 6-25 */
		ala_state(sla_id) = ALA_NULL;
		action = xmt_reject(slap, sla_id, RJT_NON_SPEC_CODE | 
							RJT_LCTL_FUN_ERR);
		break;

	case LINK_REQUEST_ALA :
		sla_tbl.sla[sla_id].connection = POINT_TO_POINT;  
					/* the switch does not send the ala */
		if (prhr_value.did_sid == SWITCH_ALA_ADDRESS) {
			/* send a reject frame */
			action = xmt_reject(slap, sla_id, RJT_NON_SPEC_CODE | 
						RJT_SID_ADDR_ERR);
		}
		else {
			if (ala_state(sla_id) == ALA_SENT) {
				/* we also sent the ala -- backoff */
				/* send a reject frame */
				action = xmt_reject(slap, sla_id, 
					RJT_NON_SPEC_CODE | RJT_CONT_ERR);
			}
			else {
				uint taddr;
				/* ala completed */
				taddr = SWAP(prhr_value.did_sid);
				switch(rc = sla_send_link_ack(slap, sla_id, 
							taddr, &s1_val) ) {
				case SLAS_DONE :
					if (! ala_error_check(slap, sla_id, 
							s1_val, slap -> status2,
					    &action) ) {
						sla_tbl.sla[sla_id].sla_st = 
								SLA_IDENT_CODE;
	/* set the retry bit.  This will be used for all auto seq. connects */
						sla_tbl.sla[sla_id].ccr_con = 
						sla_tbl.sla[sla_id].ccr_all |
						    RETRY;
						sla_sdma_wait(sla_id) = TRUE;
						primary_hdr(sla_id) = NULL_HDR;
						/* tell imcs about it */
						slaam.count = 1;
						slaam.link_address[0] = 
						     EXTRACT_SADDR(prhr_value.did_sid);
						sla_ala_completed(sla_id,
						    EXTRACT_DADDR(prhr_value.did_sid),
						    (void *) &slaam);
						action = CONTINUE_CODE;
					}
				/* otherwise error has already set action */
					break;
				case SLAS_FATAL :
				case SLAS_SCR_PROGRESS :
					action = NO_ACTION_CODE;
					break;
				case SLAS_DISCARDED :
				case SLAS_SCR_DONE :
					ala_state(sla_id) = LISTENED_FOR_ALA;
					action=LFA_CODE;/* listen for the ala */
					break;
				default :
					PANIC("do_ala_rcvd: unknown return code from sla_send_link_ack");
				}  /* end switch */
			}
		}
		break;

	case LINK_RESPONSE_ACK :  /* ala done */
		switch (EXTRACT_SADDR(prhr_value.did_sid)) {
		case SWITCH_LA :
			sla_tbl.sla[sla_id].connection = SWITCH_PRESENT;
			sla_tbl.sla[sla_id].sla_st = SLA_IDENT_CODE;
			/* write the thr whth our address */
			slap -> thr.fr.did_sid = SWAP(prhr_value.did_sid);
			sla_tbl.sla[sla_id].ccr_con=sla_tbl.sla[sla_id].ccr_all;
			primary_hdr(sla_id) = NULL_HDR;
			/* now get the map from the switch */
			{
				int i, next;
				slalinka_t link_address;
				for (next = 0, i = 0; i < MAX_SWITCH_PORT; i++)
					switch (i) {
					case 0x4 :
					case 0xb :
						/* illegal addresses */
						break;
					default :
						link_address = (slalinka_t) 
								(0xe0 + i);
						if (link_address != 
							EXTRACT_DADDR(prhr_value.did_sid))
							slaam.link_address[next++] = link_address;
						break;
					}
				slaam.count = next;
			}
			/* and tell imcs about it */
			sla_ala_completed(sla_id,
			    EXTRACT_DADDR(prhr_value.did_sid),
			    (void *) &slaam);
			break;

		case DX_SWITCH_LA :
			sla_tbl.sla[sla_id].connection = DX_SWITCH_PRESENT;
			sla_tbl.sla[sla_id].sla_st = SLA_IDENT_CODE;
			/* write the thr with our address */
			slap -> thr.fr.did_sid = SWAP(prhr_value.did_sid);
			/* set the retry bit.  
			This will be used for all auto seq. connects */
			sla_tbl.sla[sla_id].ccr_con=sla_tbl.sla[sla_id].ccr_all
					 | RETRY;
			sla_sdma_wait(sla_id) = TRUE;
			primary_hdr(sla_id) = NULL_HDR;
			/* form the address map */
			{
				int i, next;
				slalinka_t link_address;
				for (next = 0, i = 0; i < MAX_SWITCH_PORT; i++)
					switch (i) {
					case 0x4 :
					case 0xb :
						/* illegal addresses */
						break;
					default :
						link_address = (slalinka_t) (0xc0 + i);
						if (link_address != EXTRACT_DADDR(prhr_value.did_sid))
							slaam.link_address[next++] = link_address;
						break;
					}
				for (i = 0; i < 2; i++) {
					link_address = (slalinka_t) (0xd0 + i);
					if (link_address != EXTRACT_DADDR(prhr_value.did_sid))
						slaam.link_address[next++] = link_address;
				}
				slaam.link_address[next] = (slalinka_t) DX_SWITCH_LA;
				slaam.count = next + 1;
			}
			/* and tell imcs about it */
			sla_ala_completed(sla_id,
			    EXTRACT_DADDR(prhr_value.did_sid),
			    (void *) &slaam);
			break;

		default :  /* connection is point to point */
			sla_tbl.sla[sla_id].connection = POINT_TO_POINT;
			#if 0
			ptpd bit does not work
			    /* enable the point to point discard bit for the sla with
	           higher priority (in this case the one with higher link
	           address).  This sla will never see a frame collision */
			sla_tbl.sla[sla_id].ccr_con = sla_tbl.sla[sla_id].ccr_all |
			    PTPD | RETRY;
			#endif 0
			/* set the retry bit.  This will be used for all auto seq. connects */
			sla_tbl.sla[sla_id].ccr_con = sla_tbl.sla[sla_id].ccr_all |
			    RETRY;
			sla_sdma_wait(sla_id) = TRUE;
			sla_tbl.sla[sla_id].sla_st = SLA_IDENT_CODE;
			primary_hdr(sla_id) = NULL_HDR;
			/* tell imcs about it */
			slaam.count = 1;
			slaam.link_address[0] = EXTRACT_SADDR(prhr_value.did_sid);
			sla_ala_completed(sla_id,
			    EXTRACT_DADDR(prhr_value.did_sid),
			    (void *) &slaam);

			break;
		}  /* end switch on source address */
		action = CONTINUE_CODE; /* put the sla in receive mode */
		break;

	case LINK_RESPONSE_RJT :
		switch(RJT_INFO1(prhr_value.w3.info)) {
		case RJT_UNINS_LCF :
			action = XMT_UD_CODE;
			break;
		case RJT_NON_SPEC_CODE :
			{
				int code;
				code = 0;
				if (CNT_RCVD(prhr_value.w5.count) == 2)
					code = RJT_INFO2(prhr_value.w3.info);
				switch (code) {
				case RJT_CONT_ERR :
					/* the other side picked up our ala and is now listening */
				case RJT_SID_ADDR_ERR :
					/* the other side saw our first ala for the switch and rejected it */
					action = LFA_CODE;
					break;
				default :
					action = XMT_UD_CODE;
					break;
				}  /* end switch on code */
			}
			break;
		case RJT_CRC_ERR :
			/* the IBM switch uses this frame to indicate that it detected a crc error on a frame
		   addressed to it (the ala) */
			action = XMT_UD_CODE;
			break;
		case RJT_LINK_SIGERR :
		case RJT_CODE_VERR :
		case RJT_FRAME_SERR :
		case RJT_LINK_AERR :
		case RJT_RES_FERR :
		default :
			action = XMT_UD_CODE;
			break;
		}   /* end switch on reject info */
		break;

	case DYNAMIC_PORT_RJT :
		/* this reject could be caused by a bug in the switch.  The
	       switch wraps our ala frame, i. e., sends it back to us.
	       We convince ourselves that this is a point-to-point
	       connection and send ala frames with random addresses.
	       The switch send us a reject frame.  Reset connection.
	       The next ala will be addresses to the switch */
		sla_tbl.sla[sla_id].connection = 0;
		switch(RJT_INFO1(prhr_value.w3.info)) {
		case PRJT_ADDR_ERR :
		case PRJT_UNDA_ERR :
		case PRJT_DESTP_MALF :
		case PRJT_DSINT_REQ :
		default :
			action = XMT_UD_CODE;
			break;
		}  /* end switch on reject info */
		break;

	case LINK_RESPONSE_BSY :
		ala_state(sla_id) = LISTENED_FOR_ALA;
		action = LFA_CODE; /* listen for the ala */
		break;

	case DYNAMIC_PORT_BSY :
		ala_state(sla_id) = LISTENED_FOR_ALA;
		action = LFA_CODE; /* listen for the ala */
		break;

	default :
		action = XMT_UD_CODE;
		break;
	}  /* end switch on link ctl */

	return action;

}  /* end do_ala_rcvd */


/**/
/*
 * NAME: do_notident
 *                                                                    
 * FUNCTION:  continues the ala sequence until completed
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) if the link fails (xmt_fail bit on in s2), then the sla is
                marked failed if the ala succedes, then the sla state is 
		changed to identified.

    For a direct connection the chances are high that
    the other sla is sending an ALA "at the same time"; so the problem is
    for both sides to perform a backoff protocol, i.e. listen for a while
    then try again.   The backoff periods of the 2 sides should sooner or
    later be sufficiently different that the ALA of one side reaches the other
    while it is still listening.   To achieve this we start with a seed which
    we hope is unique, the value of the clock at the time of syncrhonization,
    and we generate a sequence of random numbers from that.

    When an sla is started, the start command may be rejected - this means that
    while the sla was stopped, a message came in (and was BUSYed). We see
    the link address mismatch. The sender of the discarded message gets a
    'link control busy' response and backs off.  This SLA waits a little (not
    backing off) and retries.
    Is this true?

    The following protocol should be observed:

        on cmd rjt, wait a bit then retry.
        on receiving 'link control busy', listen

    The general logic is as follows:
        send an ALA, expected response is either RJT or ALA
        if response is RJT, contention has been resolved - continue
        if response is ALA, backoff and listen - if ALA received during
           backoff interval,contention has been resolved.
    This game probably should not continue for ever - when to give up may
    have to be determined experimentally.

 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
          SYNC -- syncronization must be re-initiated
          XMT_UD -- start the ud sequence
          LFA -- listen for the for a while, then sent it
          NO_ACTION -- the sla driver failed
          other codes returned by do_ala_rcvd

 */  

uint do_notident(SLAIOP(slap), int sla_id, uint s1_val, uint s2_val)
{
	uint error, action;


SOL_TRACE("noid",sla_id,s1_val,s2_val);
	error = ala_error_check(slap, sla_id, s1_val, s2_val, &action);

	sla_scr(sla_id) = FALSE;

	if ( ! error) {
		action = NO_ACTION_CODE;

	/* we received something, the sla timed out on a send, or we finished
	     sending a frame */
		if (s1_val & RESP_TO) { /* the sla timed out */
			ala_state(sla_id) = LISTENED_FOR_ALA;
			action = LFA_CODE;
		}
		else if (s1_val & SCR_DONE) {
			ala_state(sla_id) = LISTENED_FOR_ALA;
			action = LFA_CODE;
		}
		else if (s1_val & PRIMARY) {
			/* the values loaded in the prhr are valid */
			if ( (s1_val & UNEXPD) || (s2_val & ADDR_MIS) )
				/* the sla received a frame */
				action = do_ala_rcvd(slap, sla_id);
			else if (s1_val & OP_DONE) {
				/* the sla completed an operation */
				action = do_ala_rcvd(slap, sla_id);
			}
			else {	/* we do not know why we had an interrupt */
				action = XMT_UD_CODE;
			}
		}
		else if (s1_val & OP_DONE) {
	/* this should never happen except that under certain noise conditions
	       if does.  In this cases unexpected is set but primary is not
	       because we did not receive the entire frame */
			action = XMT_UD_CODE;
		}
		else {
			if (s1_val & UNEXPD) {
		/* this is again the case where we get a piece of the frame but
		 not the entire frame */
				action = XMT_UD_CODE;
			}
			else {
				action = XMT_UD_CODE;
			}
		}
	}



	return action;

}  /* end do_notident */



/**/
/*
 * NAME: start_and_spin1
 *                                                                    
 * FUNCTION: starts the channel and spin until completed returns
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
        this flavor will retry if sla_start returns -1 (frame discarded)
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS:        0  -- everything OK
		1,2, -1, -2: channel could not be started
 */  

static int
start_and_spin1(slap, sla_id, ccr_value, s1_valp)
SLAIOP(slap);
int sla_id;
uint ccr_value;
uint * s1_valp;
{
	int start_rc, start_try;
	uchar done;

	CHECK_REG(slap);

	for (start_try = 0, done = FALSE;
	    start_try < MAX_START_TRY && ! done;
	    start_try++)
		switch (start_rc = sla_start(slap, sla_id, ccr_value) ) {
		case SLAS_DONE :
			done = TRUE;
			*s1_valp = sla_spin(slap, sla_id);
			break;
		case SLAS_FATAL :
		case SLAS_SCR_PROGRESS :
		case SLAS_SCR_DONE :
			done = TRUE;
			break;
		case SLAS_DISCARDED :
			break;
		case SLAS_UNEXPD :
			PANIC("start_and_spin1: sla_start returned SLAS_UNEXPD");
			break;
		default: 
			PANIC("start_and_spin1: unknown return code from sla_start");
		}  /* end switch */

	return start_rc;

}  /* end start_and_spin1 */


/**/
/*
 * NAME: start_and_spin2
 *                                                                    
* FUNCTION: starts the channel and spin until completed returns
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:)
        this flavor will not retry 
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:        0  -- everything OK
                1,2, -1, -2: channel could not be started
 */  

static int
start_and_spin2(slap, sla_id, ccr_value, s1_valp)
SLAIOP(slap);
int sla_id;
uint ccr_value;
uint * s1_valp;
{
	int start_rc;

	CHECK_REG(slap);

	switch (start_rc = sla_start(slap, sla_id, ccr_value) ) {
	case SLAS_DONE :
		*s1_valp = sla_spin(slap, sla_id);
		break;
	case SLAS_FATAL :
	case SLAS_SCR_PROGRESS :
	case SLAS_DISCARDED :
	case SLAS_SCR_DONE :
		break;
	case SLAS_UNEXPD :
		PANIC("start_and_spin2: sla_start returned SLAS_UNEXPD");
		break;
	default: 
		PANIC("start_and_spin2: unknown return code from sla_start");
	}  /* end switch */

	return start_rc;

}  /* end start_and_spin2 */


/**/
/*
 * NAME: sla_send_ack
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
 * RETURNS: 
 */  

int
sla_send_ack(slap, sla_id, link_address, ack_frame, s1_valp)
SLAIOP(slap);
int sla_id;
uint link_address, ack_frame;
uint * s1_valp;
{
	struct sl_frame thr_val;

	CHECK_REG(slap);

	thr_val.did_sid = link_address;
	thr_val.link_ctl = DEVICE_CTL_DATA;
	thr_val.w3.dev_ctl = ack_frame;
	thr_val.w4.info = THR_DONT_CARE;
	thr_val.w5.count = THR_DONT_CARE;

	WRITE_THR(slap, thr_val);

	return start_and_spin2(slap, sla_id,
	    sla_tbl.sla[sla_id].ccr_all | XMT_DIS_CODE, s1_valp);

}  /* end sla_send_ack */


/**/
/*
 * NAME: sla_send_link_ack
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
 * RETURNS: 
 */  


int
sla_send_link_ack(slap, sla_id, link_address, s1_valp)
SLAIOP(slap);
int sla_id;
uint link_address;
uint * s1_valp;
{
	struct sl_frame thr_val;

	CHECK_REG(slap);

	thr_val.did_sid = link_address;
	thr_val.link_ctl = LINK_RESPONSE_ACK;
	thr_val.w3.info = THR_DONT_CARE;
	thr_val.w4.info = THR_DONT_CARE;
	thr_val.w5.count = 0;

	WRITE_THR(slap, thr_val);

	return start_and_spin1(slap, sla_id,
	    sla_tbl.sla[sla_id].ccr_all | XMT_DIS_CODE, s1_valp);

}  /* end sla_send_link_ack */


/**/
/*
 * NAME: sla_send_nack
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
 * RETURNS: 
 */  

int
sla_send_nack(slap, sla_id, link_address, nack_value, s1_valp)
SLAIOP(slap);
int sla_id;
uint link_address;
uint nack_value;
uint * s1_valp;
{
	struct sl_frame thr_val;

	CHECK_REG(slap);

	thr_val.did_sid = link_address;
	thr_val.link_ctl = DEVICE_CTL_DATA;
	thr_val.w3.dev_ctl = DEV_RESP_NAK;
	thr_val.w4.info = nack_value;
	thr_val.w5.count = THR_DONT_CARE;

	WRITE_THR(slap, thr_val);

	return start_and_spin2(slap, sla_id,
	    sla_tbl.sla[sla_id].ccr_all | XMT_DIS_CODE, s1_valp);

}  /* end sla_send_nack */


/**/
/*
 * NAME: sla_send_rjt
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
 * RETURNS: 
 */  

int
sla_send_rjt(slap, sla_id, count, info, s1_valp)
SLAIOP(slap);
int sla_id;
uint count;
uint info;
uint * s1_valp;
{
	struct sl_frame thr_val;

	CHECK_REG(slap);

	thr_val.did_sid = sla_address(sla_id) | RJT_DID_ADDRESS;
	thr_val.link_ctl = LINK_RESPONSE_RJT;
	thr_val.w3.info = info;
	thr_val.w4.info = THR_DONT_CARE;
	thr_val.w5.count = count;

	WRITE_THR(slap, thr_val);

	return start_and_spin1(slap, sla_id,
	    sla_tbl.sla[sla_id].ccr_all | XMT_DIS_CODE, s1_valp);

}  /* end sla_send_rjt */


/**/
/*
 * NAME: sla_send_bsy
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
 * RETURNS: 
 */  

int
sla_send_bsy(slap, sla_id, s1_valp)
SLAIOP(slap);
int sla_id;
uint * s1_valp;
{
	struct sl_frame thr_val;

	CHECK_REG(slap);

	thr_val.did_sid = sla_address(sla_id) | BSY_DID_ADDRESS;
	thr_val.link_ctl = LINK_RESPONSE_BSY;
	thr_val.w3.info = THR_DONT_CARE;
	thr_val.w4.info = THR_DONT_CARE;
	thr_val.w5.count = 0;

	WRITE_THR(slap, thr_val);

	return start_and_spin1(slap, sla_id,
	    sla_tbl.sla[sla_id].ccr_all | XMT_DIS_CODE, s1_valp);

}  /* end sla_send_bsy */


/**/
/*
 * NAME: sla_xmt_sof
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
sla_xmt_sof(slap, sla_id)
SLAIOP(slap);
int sla_id;
{
	union {
		uint tagword;
		struct tag tcw;
	} mytag;


	CHECK_REG(slap);

	mytag.tagword = imcslra(sla_tbl.buffer + SOF_SEQ_ADDR);
	mytag.tcw.count = 4;	/* one line */
        slap -> tcw[0] = mytag.tagword;
        slap -> tcw[1] = LAST_TAG;    

}  /* end sla_xmt_sof */


/**/
/*
 * NAME: sla_sdma_unh
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

void sla_sdma_unh(SLAIOP(slap), int sla_id)
{
	uint s1_val1, s1_val2;
	struct sl_frame thr_value;

	CHECK_REG(slap);

	s1_val1 = slap -> sample;

	/* we have, so far, three ways to recover the send dma
	   1) send the fram and abort it;
	   2) send the frame and wait until done
	   3) send the frame in wrap 10 bit mode
	   Only the third way works.
	*/

	if (PUISTATE(s1_val1) == PUI_NOOP) {
		/*sdma(sla_id) = TRUE;*/
	}
	else {
		ASSERT(PUISTATE(s1_val1) == PUI_STOPPED);
		slap -> thr.fr.link_ctl = DEVICE_CTL_DATA;
		slap -> thr.fr.w3.dev_ctl = DEV_SEND | SCH_SDMA;
		slap -> thr.fr.w4.B = IMCS_SDMA_QUEUE;

		switch (sla_pstart(slap, sla_id,
		    PRIORITY | INITIATE | SXMT | AUTO_BUSY |
		    DRIVER | DIAG_C) ) {
		case 0 :
			s1_val1 = sla_spin(slap, sla_id);
			break;
		default :
			PANIC("sdma: priority start did not work");
		}  /* end switch on return from sla_pstart */

	/* since we turned the channel in wrap 10 bit mode, the other side
	     is seeing no-op, and should send us ols.  Give the other side a
	     chance to see our nos */
		{
			volatile uint dummy;
			int i, j;
			for (i = 0; i < 300; i++) j = dummy;  
				/* wait 300 instructions */
		}

		switch (PUISTATE(s1_val1) ) {  
				/* this is the s1_val read after channel
			          stopped in spin */
		case PUI_NOOP :
			PANIC("noop channel after sending dma send frame");
			break;

		case PUI_STOPPED :
			break;

		case PUI_WORK1 :
			/* channel did not finish */
			PANIC("work1 channel after sending dma send frame");
			break;

		default :
			PANIC("unknown channel after sending dma send frame");
			break;

		}  /* end switch on PUI state */
	}

}  /* end sla_sdma_unh */


/**/
/*
 * NAME: sla_rdma_unh
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

void sla_rdma_unh(SLAIOP(slap), int sla_id)
{
	uint s1_val;

	CHECK_REG(slap);

	s1_val = slap -> sample;

	if (PUISTATE(s1_val) == PUI_NOOP) {
		#if 0
		rdma(sla_id) = TRUE;
		#endif
	}
	else {
		ASSERT(PUISTATE(s1_val) == PUI_STOPPED);

		slap -> ccr = BUFAC | BUF_WRITE | DRIVER;

		{
			volatile uint dummy;
			int i, j;
			for (i = 0; i < 300; i++) j = dummy;  
			/* wait 300 instructions */
		}
	}

}  /* end sla_rdma_unh */


/**/
/*
 * NAME: sla_channel_check 
 *                                                                    
 * FUNCTION: bails out for channel check (there can be more than one)
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

void sla_channel_check(SLAIOP(slap), int sla_id, uint s2_val)
{
	int found;

	found = FALSE;
	if (s2_val & TAGPARITY) {
		sla_fatal(slap, sla_id, CHANNEL_CHECK, CCC_TAG_PAR);
		found = TRUE;
	}

	if (s2_val & BUFPARITY) {
		sla_fatal(slap, sla_id, CHANNEL_CHECK, CCC_BUF_PAR);
		found = TRUE;
	}

	if (s2_val & STORCHK)
		sla_fatal(slap, sla_id, CHANNEL_CHECK, CCC_STOR_CHK);

	if(found) {
		err_log(1,ERRID_SLA_PARITY_ERR,-1,s2_val);
	}

}  /* end sla_channel_check */



/**/
/*
 * NAME: cdd_enqueue
 *                                                                    
 * FUNCTION: places message in a cdd queue messages are queued FIFO to send 
	    and rcv subchannel LIFO to rcv
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
cdd_enqueue(header)
struct imcs_header *header;
{
	int i;
	int pid;
	int good_flag=FALSE;

        pid = header -> IMCS_DEST_PROC;
SOL_TRACE("cdeq",pid,header,header->IMCS_GO);
#if 0
	if ((pid > 0) && (pid < IMCS_PROC_LIMIT)) {
		for (i = 0; i < MAX_NUM_SLA ; i++) {
			if(imcs_addresses.proc[pid].sla[i] != INVALID_SLAID) {
				if(!bad_port[i]) {
					good_flag=TRUE;
					break;
				}
			}
		}

		if(!good_flag) {
SOL_TRACE("cdq1",pid,header,header->IMCS_GO);
			header = NULL_HDR;
			return;
		}
	}
#endif
SOL_TRACE("cdq2",pid,header,header->IMCS_GO);

	header -> cdd_chain_word = NULL_HDR;
	header -> next_cddq_chain = NULL_HDR;

	switch (header -> IMCS_GO) {
	case IMCS_INITIATE :
SOL_TRACE("cdq3",pid,header,header->IMCS_GO);
		enqueue_to_sla(header);
		break;
	case IMCS_DELAY :
SOL_TRACE("cdq4",pid,header,header->IMCS_GO);
		if (header -> IMCS_AUTOMATIC)
			enqueue_to_rcv(header);
		else {
			enqueue_to_schn(header);
		}
	}

}  /* end cdd_enqueue */



/**/
/*
 * NAME: enqueue_to_sla
 *                                                                    
 * FUNCTION: enqueues the message to the sla if no sla 
		can send the message immediately
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
	masks interrupts to achive synchronization.  dequeue runs at
  	interrupt level.  handles snd rts and rtsi, and rcv rtr
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  


static void
enqueue_to_sla(header)
struct imcs_header *header;
{

        struct imcs_header *last_msg, *this_msg;
        int qi;
        int processor_priority;
        boolean_t trouble;
        boolean_t que_full;
        int rc;
        struct trb *trb;
        TVARS tvar;
        int opid;
        this_msg = header;
SOL_TRACE("e2sl",this_msg,0,0);

        processor_priority = i_disable(IMCS_INT_PRIORITY);

        while (this_msg != NULL_HDR) {
                opid=this_msg->IMCS_DEST_PROC;
                /* write an id an the message (for linking traces) */
                this_msg -> B_reg_half = next_B++;
                /* build the channel program */
                sla_set_active(this_msg);
                ASSERT(this_msg -> IMCS_GO == IMCS_INITIATE);

                trouble = FALSE;
                que_full = FALSE;

                switch (rc=get_snd_sla(this_msg)) {
                case 0 :
                        break;
                case 1 :            /* enqueue on cdd chain */
                        if(que.cnt[opid] >= HANDFULL*6) {
                                this_msg->outcome= IMCS_DOWN_CONN;
                                que_full = TRUE;
                                break;
                        }
                        this_msg -> outcome = IMCS_CDD_ENQ;
                        qi = SLA_OUTQ +
                        imcs_addresses.proc[this_msg->IMCS_DEST_PROC].set_queue;
                        if (cddq.anchor[qi] == NULL_HDR) { /* queue empty */
                                cddq.anchor[qi] = this_msg;
                                this_msg -> next_cddq_chain = this_msg;
                        }
                        else {
SOL_TRACE("vchk",cddq.anchor[qi], cddq.anchor[qi]->next_cddq_chain, 
    cddq.anchor[qi]->cdd_chain_word);
                                last_msg = cddq.anchor[qi] -> next_cddq_chain;
                                last_msg -> cdd_chain_word = this_msg;
                                cddq.anchor[qi] -> next_cddq_chain = this_msg;
                        }
                        break;
                case -2 :
                case -1 :
                        this_msg -> outcome = IMCS_CDD_ENQ;
			opid=LIMBO_SLOT;
                        if(que.cnt[LIMBO_SLOT] >= HANDFULL*6) {
                                this_msg->outcome= IMCS_DOWN_CONN;
                                que_full = TRUE;
                                break;
                        }
                /* no path exists, or there is no path which is not broken */
                        if (cck_proc[this_msg->IMCS_DEST_PROC] &
                            (PID_HOLDING | PID_PRESENT)) {
                                this_msg -> outcome = IMCS_CDD_ENQ;
                                qi = SLA_OUTQ + LIMBO_QUEUE;
				this_msg -> cdd_chain_word = cddq.anchor[qi];
		                cddq.anchor[qi] = this_msg;
                        }
                        else {
                                trouble = TRUE;
                                if (cck_proc[this_msg->IMCS_DEST_PROC] &
                                    PID_EVER_PRESENT)
                                        this_msg -> outcome = IMCS_NO_CONN;
                                else
                                        this_msg -> outcome = IMCS_NEVER_CONN;
                        }
                        break;
                default :
                        PANIC("get_snd_sla retuns other than 0, 1, -1, -2");
                }  /* end switch on return code from get_snd_sla */
		SYS_SOL_TRACE("e2sl",rc,trouble,que_full);
                if(rc && (!trouble && !que_full))  {
                        ++que.total_queued;
                        if(++que.cnt[opid] >= HANDFULL) {
                                if(que.time_stamp[opid] == 0)
                                        que.time_stamp[opid]=time;
                                if(que.time_running == FALSE) {

                                        tvar.id = 0;
                                        tvar.func = QUEUE_FLUSH;
                                        tvar.setup = FALSE;
                                        START_TIMER(que.timer,tservice,
                                            tvar,1,0);
                                        que.time_running = TRUE;
                                }
                        }
			/* verify_que(que.total_queued); */
                } 
                if (trouble || que_full) {
                        last_msg = this_msg;
                        this_msg = imcs_send_done(last_msg);

                        switch ((int) this_msg) {
                        case -2 :
                                PANIC("imcs_send_done returns -2 when called from enqueue_to_sla");
                                break;
                        case (int) 0 /* NULL_HDR */ :
                                /* call the notification routine */
                                NOTIFY(last_msg);
                                break;
                        default :
				/* enqueue the message returned by imcs */
				this_msg -> cdd_chain_word = NULL_HDR;
				this_msg -> next_cddq_chain = NULL_HDR;

				/* call the notification routine */
				NOTIFY(last_msg);
                                /* since we pulled the next one from the queue
                                   decrement the number queued */

                                if(--que.cnt[opid] < 0) que.cnt[opid]=0;
                                if(--que.total_queued< 0) que.total_queued=0;
				break;
			}  /* end switch */
		}  /* end trouble */
		else
			this_msg = NULL_HDR;

	}  /* end while loop */

	i_enable(processor_priority);

}  /* end enqueue_to_sla */


/**/
/*
 * NAME: enqueue_to_rcv 
 *                                                                    
 * FUNCTION:   Enqueue message to device, if no device available, but this one
		  works for receive.

 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) masks interrupts to achive synchronization.  dequeue runs at
  	  interrupt level.

	  headers are enqueued LIFO, in a stack fashion.

	  handles only rcv rtsi

 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
enqueue_to_rcv(header)
struct imcs_header *header;
{

	int processor_priority;

	ASSERT(header -> IMCS_PROTOCOL == IMCS_RTSI_RCV_CODE);

	processor_priority = i_disable(IMCS_INT_PRIORITY);

	switch (get_rcv_sla(header)) {
	case 0 :
		break;
	case 1 :                 /* enqueue on cdd chain */
		header -> outcome = IMCS_CDD_ENQ;
		header -> cdd_chain_word = cddq.anchor[SLA_INQ];
		cddq.anchor[SLA_INQ] = header;
		break;
	default :
		PANIC("get_rcv_sla returns other than 0 or 1");
	}

	i_enable(processor_priority);

}  /* end enqueue_to_rcv */




/**/
/*
 * NAME: get_snd_sla 
 *                                                                    
 * FUNCTION: enqueue message at device, if possible
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
  if an sla is available to the processor it is claimed by this message
  and used to send the message.  If all sla's to the processor are busy
  sending then nothing happens and 1 is returned
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
    0 -- message enqueued at sla
    1 -- connection exists, but it is busy
   -1 -- no connection exists
   -2 -- connection exists, but it does not work

 */  

static int
get_snd_sla(header)
struct imcs_header *header;
{

	int rc;

	int pid, i;
	int possible_sla; /* sla busy receiving, but good */
	int the_sla;  /* sla picked to send */

	pid = header -> IMCS_DEST_PROC;
SOL_TRACE("gsdB",pid,header,0);
	rc = -1;  /* assume no connection */

	for (i = 0; i < MAX_NUM_SLA && rc; i++) {
		int sla_id;
		sla_id = imcs_addresses.proc[pid].sla[i];
		if (sla_id != INVALID_SLAID)
			if (sla_tbl.sla[sla_id].imcs_st & 
				SLA_SENDING_CODE) {
				if(rc != 2) rc = 1;
				if (active_hdr(sla_id) == NULL_HDR) {
					/* free sla found */
					the_sla = possible_sla = sla_id;
					rc = 2;
					header->outcome = IMCS_SLA_ENQ;
					if (passive_hdr(sla_id) == 
							NULL_HDR ||
					    passive_hdr(sla_id)->IMCS_PROTOCOL != 
						   IMCS_RTS_RCV_CODE) {
						active_hdr(sla_id) = 
								header;
					/* try the first address in the set */
						ASSERT(imcs_addresses.proc[pid].sla_addr[sla_id][0] != INVALID_SLA_ADDR);
						other_addr(sla_id) = imcs_addresses.proc[pid].sla_addr[sla_id][0];
						sla_tbl.sla[sla_id].event_count = 0;
						if (sla_send(sla_id)) rc = 0;
						else active_hdr(sla_id) = NULL_HDR;
					}  /* end the sla is not busy receiving a rts message */
				} /* end: sla available for send found */
		} /* end: the sla can send */
				else if (rc != 1) rc = -2;
	}  /* end of for loop */

	if (rc == 2) {
		rc = 0;
		ASSERT(imcs_addresses.proc[pid].sla_addr[possible_sla][0] != INVALID_SLA_ADDR);
		other_addr(possible_sla) = imcs_addresses.proc[pid].sla_addr[possible_sla][0];
		active_hdr(possible_sla) = header;
	}

SOL_TRACE("gsdE",rc,0,0);
	return rc;

}  /* end get_snd_sla */



/**/
/*
 * NAME: get_rcv_sla 
 *                                                                    
 * FUNCTION: enqueue message at device, if possible
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
    0 -- enqueued at sla
    1 -- all sla's are busy
 */  


static int
get_rcv_sla(header)
struct imcs_header *header;
{

	int sla_id;
	int rc;

	rc = 1;  /* assume all busy */
	for (sla_id = 0;
	    sla_id < MAX_NUM_SLA && rc;
	    sla_id ++)
		if (sla_present(sla_id) && 
		    passive_hdr(sla_id) == NULL_HDR &&
		    sla_tbl.sla[sla_id].imcs_st & SLA_RECEIVING_CODE) {
			rc = 0;
			passive_hdr(sla_id) = header;
			header -> outcome = IMCS_SLA_ENQ;
			sla_rcv_rtsi(sla_id);
		}

	return rc;

}  /* end get_rcv_sla */


/**/
/*
 * NAME: enqueue_to_schn
 *                                                                    
 * FUNCTION: enqueues message to a subchannel (enqueue at tail)
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
enqueue_to_schn(header)
struct imcs_header *header;
{

	int qa;
	struct imcs_header *p, *q;  /* q follows p */
	int processor_priority;

	ASSERT(header -> IMCS_GO == IMCS_DELAY);

	qa = HASH_CDDQ(header -> IMCS_SUBCHANNEL);
	ASSERT(qa >= 0 && qa < NUM_SC_Q);
	if (header -> IMCS_OP_CODE == IMCS_RECEIVE_CODE) qa += NUM_SC_Q;

	q = NULL_HDR;

	processor_priority = i_disable(IMCS_INT_PRIORITY);

	/* build the channel program */
	sla_set_passive(header);

	p = cddq.anchor[qa];

	while (p != NULL_HDR &&
	    p -> IMCS_SUBCHANNEL != header -> IMCS_SUBCHANNEL) {
		q = p;
		p = p -> next_cddq_chain;
	}

	if (p == NULL_HDR) { /* there is no queue for this subchannel */
		header -> cdd_chain_word = header;  /*start the circular list */
		if (q == NULL_HDR)
			cddq.anchor[qa] = header;
		else
			q -> next_cddq_chain = header;
	}
	else {  
		/* p point the the last arrived element for the subchannel,
	   	and the element pointed to by p points to the first */
		header -> cdd_chain_word = p -> cdd_chain_word;
		p -> cdd_chain_word = header;
		header -> next_cddq_chain = p -> next_cddq_chain;
		if (q == NULL_HDR)
			cddq.anchor[qa] = header;
		else
			q -> next_cddq_chain = header; 
				/* only the first element in the queue,
				really the last arrived, has the correct
				pointer to the next subchannel queue */
	}

	i_enable(processor_priority);

}  /* end enqueue_to_schn */


/**/
/*
 * NAME: dequeue_from_schn 
 *                                                                    
 * FUNCTION: dequeue from head
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: address of the dequeued header or NULL_HDR 

*/
struct imcs_header *
dequeue_from_schn(subchannel, op_code)
int subchannel;
uchar op_code;
{
	int qa;
	struct imcs_header *p, *msg, *q;  /* q follows p */

	qa = HASH_CDDQ(subchannel);
	ASSERT(qa >= 0 & qa < NUM_SC_Q);
	if (op_code == IMCS_RECEIVE_CODE) qa += NUM_SC_Q;

	q = NULL_HDR;

	p = cddq.anchor[qa];

	while (p != NULL_HDR && p -> IMCS_SUBCHANNEL != subchannel) {
		q = p;
		p = p -> next_cddq_chain;
	}

	if (p == NULL_HDR) { /* there is no queue for this subchannel */
		msg = NULL_HDR;
	}
	else {
		/* dequeue p -> cdd_chain_word */
		msg = p -> cdd_chain_word;
		ASSERT (msg != NULL_HDR);
		if (msg == p) {  /* subchannel queue has only one element */
			if (q == NULL_HDR)
				cddq.anchor[qa] = p -> next_cddq_chain;
			else q -> next_cddq_chain = p -> next_cddq_chain;
		}
		else
			p -> cdd_chain_word = msg -> cdd_chain_word;
	}

	return msg;

}  /* end dequeue_from_schn */


/**/
/*
 * NAME: requeue_to_schn
 *                                                                    
 * FUNCTION: enqueues message to a subchannel. (enqueue at head) 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 Called by the interrupt handler when an error occurs while receiving
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  


void
requeue_to_schn(header)
struct imcs_header *header;
{

	int qa;
	struct imcs_header *p, *q;  /* q follows p */
	int processor_priority;

	ASSERT(header -> IMCS_GO == IMCS_DELAY);

	qa = HASH_CDDQ(header -> IMCS_SUBCHANNEL);
	ASSERT(qa >= 0 && qa < NUM_SC_Q);
	if (header -> IMCS_OP_CODE == IMCS_RECEIVE_CODE) qa += NUM_SC_Q;

	q = NULL_HDR;

	processor_priority = i_disable(IMCS_INT_PRIORITY);

	p = cddq.anchor[qa];

	while (p != NULL_HDR &&
	    p -> IMCS_SUBCHANNEL != header -> IMCS_SUBCHANNEL) {
		q = p;
		p = p -> next_cddq_chain;
	}

	if (p == NULL_HDR) { /* there is no queue for this subchannel */
		header -> next_cddq_chain = NULL_HDR;
		header -> cdd_chain_word = header;  
		/* re-start the circular list */
		if (q == NULL_HDR)
			cddq.anchor[qa] = header;
		else
			q -> next_cddq_chain = header;
	}
	else {  /* p point the the last arrived element for the subchannel,
		   and the element pointed to by p points to the first */
		header -> cdd_chain_word = p -> cdd_chain_word;
		p -> cdd_chain_word = header;
	}

	i_enable(processor_priority);

}  /* end requeue_to_schn */

/**/
/*
 * NAME: cdd_next
 *                                                                    
 * FUNCTION: send the next message (if any) 
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
cdd_next(sla_id)
int sla_id;
{

	struct imcs_header *p;
	int qi;
	TVARS tvar;

        tvar.setup=TRUE;
        tvar.id = sla_id;
        tvar.func = SEND_NEXT;

	qi = sla_tbl.sla[sla_id].set_queue;
	ASSERT(qi >= 0 && qi < IMCS_SETQ_LIMIT || qi == FAKE_QUEUE);
	qi += SLA_OUTQ;

	p = cddq.anchor[qi];
SOL_TRACE("cdnx",sla_id,qi,p);
	if (p == NULL_HDR) {
		active_hdr(sla_id) = NULL_HDR;
	}
	else {
		ASSERT(p -> outcome == IMCS_CDD_ENQ); /* 11/29 */
		/* dequeue p */
		cddq.anchor[qi] = p -> cdd_chain_word;
		if (cddq.anchor[qi] != NULL_HDR)
			cddq.anchor[qi]->next_cddq_chain = 
			    p->next_cddq_chain;

		active_hdr(sla_id) = p;
		/* try the first address in the set */
		other_addr(sla_id) = 
	    	imcs_addresses.proc[p->IMCS_DEST_PROC].sla_addr[sla_id][0];
		sla_tbl.sla[sla_id].event_count = 0;
		p -> outcome = IMCS_SLA_ENQ;
	}

	SYS_SOL_TRACE("cddn",sla_id,active_hdr(sla_id),0);
}  /* end cdd_next */



/**/
/*
 * NAME: cdd_send_done 
 *                                                                    
 * FUNCTION:  called by the interrupt handler.  It sends the next message
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
cdd_send_done(sla_id)
int sla_id;
{

	struct imcs_header *msg_sent, *p, *next_msg;
	int qi;

	msg_sent = active_hdr(sla_id);
	if(que.cnt[msg_sent->IMCS_DEST_PROC]) {
		if(--que.cnt[msg_sent->IMCS_DEST_PROC]< 0) 
			que.cnt[msg_sent->IMCS_DEST_PROC] = 0;
		if(--que.total_queued < 0) que.total_queued=0;
	}
	if(que.cnt[msg_sent->IMCS_DEST_PROC] == 0) {
		que.time_stamp[msg_sent->IMCS_DEST_PROC] = 0;
	} else if (que.time_stamp[msg_sent->IMCS_DEST_PROC] != 0) {
		que.time_stamp[msg_sent->IMCS_DEST_PROC] = time;
	}

	if(que.time_running == TRUE && que.total_queued < HANDFULL) {
		STOP_TIMER(que.timer);
		que.time_running = FALSE;
	}
	cdd_next(sla_id);

	next_msg = imcs_send_done(msg_sent);

	switch ((int) next_msg) {
	case -2 :
		/* do not notify.  This message was blocked
	       by imcs */
		break;
	case (int) 0 /*NULL_HDR*/ :
		/* call the notification routine */
		NOTIFY(msg_sent);
		break;
	default :
		/* enqueue the message returned by imcs */
		next_msg -> cdd_chain_word = NULL_HDR;
		next_msg -> next_cddq_chain = NULL_HDR;
		enqueue_to_sla(next_msg);

		/* call the notification routine */
		NOTIFY(msg_sent);
		break;
	}  /* end switch */


}  /* end cdd_send_done */



/**/
/*
 * NAME: cdd_schn_int 
 *                                                                    
 * FUNCTION:   called by the interrupt handler, when a request for a 
 *		particul subchannel arrives.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
   If there are no headers for the queued for the subchannel this
   procedure will either invoke the slih for the range to which
   the subchannel belongs or return IMCS_NACK_NS (no subchannel).
   If the slih is invoked, and it returns NULL, then this procedure
   also returnes IMCS_NACK_NS.  Otherwise, if all is well, this
   procedure returns IMCS_ACK, and set the passive address for the
   sla to the header that should be used to receive (or send for rtr).

 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
    	IMCS_ACK or IMCS_NACK_NS
 */  

ushort
cdd_schn_int(sla_id, subchannel, B_reg, op_code)
int sla_id;
uint subchannel, B_reg, op_code;
{
	ushort rc;

	struct imcs_header * msg;

	msg = dequeue_from_schn(subchannel, op_code);

	if (msg == NULL_HDR) {
		/* there are no header enqueued to
	     this subchannel */
		struct irq_block * irq;
		irq = rqctl_find((subchannel & 0x0000ff80));
		if (irq != NULL_IRQ && irq -> status & IRQ_RECEIVING) {
			/* cast pointer to slih to pointer to function
	       which returns an imcs header address */
			struct imcs_header * (* routine) ();
			routine=(struct imcs_header *(*)())(irq->imcs_slih_ext);
			msg = (* routine) (subchannel);  
				/* and invoke slih */
			if (msg != NULL_HDR) sla_set_passive(msg);
		}
	}

	if (msg == NULL_HDR) { /* there is no queue for this subchannel */
		rc = IMCS_NACK_NS;
	}
	else {
		rc = IMCS_ACK;

		/* save rtsi rcv stub */
		ASSERT(rtsi_save(sla_id) == NULL_HDR);
		if (passive_hdr(sla_id) != NULL_HDR)
			rtsi_save(sla_id) = passive_hdr(sla_id);

		/* and put msg on the sla */
		passive_hdr(sla_id) = msg;
	}

	return rc;

}  /* end cdd_schn_int */


/**/
/*
 * NAME: cdd_rcv_start 
 *                                                                    
 * FUNCTION: called by the interrupt handler when receive completes to 
 *	restart the sla.  It dequeues another header for the sla.
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
cdd_rcv_start(sla_id)
int sla_id;
{

	if (rtsi_save(sla_id) == NULL_HDR)
		if (cddq.anchor[SLA_INQ] != NULL_HDR) {
			passive_hdr(sla_id) = cddq.anchor[SLA_INQ];
			cddq.anchor[SLA_INQ] = 
					cddq.anchor[SLA_INQ] -> cdd_chain_word;
		}
		else
			passive_hdr(sla_id) = NULL_HDR;
	else {
		ASSERT(rtsi_save(sla_id)->IMCS_PROTOCOL == IMCS_RTSI_RCV_CODE);
		passive_hdr(sla_id) = rtsi_save(sla_id);
		rtsi_save(sla_id) = NULL_HDR;
	}

}  /* end cdd_rcv_start */


/**/
/*
 * NAME: cdd_clean_sla
 *                                                                    
 * FUNCTION: removes both the active and passive header from an sla.
                called when the sla fails
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
cdd_clean_sla(sla_id)
int sla_id;
{
	struct imcs_header * p;
SOL_TRACE("cddc",sla_id,active_hdr(sla_id),passive_hdr(sla_id));

	if (active_hdr(sla_id) != NULL_HDR) {
     /* requeue the message to the cdd level.  If there is another sla to the
     destination processor it will be eventually used to send this
     message.  Otherwise, the header's notification routine will be called
     informing the imcs user that there is no connection to the processor */
		p = active_hdr(sla_id);
		active_hdr(sla_id) = NULL_HDR;
		cdd_enqueue(p);
	}

	if (passive_hdr(sla_id) != NULL_HDR) {
		p = passive_hdr(sla_id);
		passive_hdr(sla_id) = NULL_HDR;
		if (p -> IMCS_PROTOCOL == IMCS_RTS_RCV_CODE ||
		    p -> IMCS_PROTOCOL == IMCS_RTR_SND_CODE)
			/* put it back in the subchannel queue */
			requeue_to_schn(p); 
			/* it is placed at the head of the queue */
		else {
			ASSERT(p -> IMCS_PROTOCOL == IMCS_RTSI_RCV_CODE);
			if (rtsi_save(sla_id) != NULL_HDR) {
				imcs_rcv_fail(p);
			}
			else
				cdd_enqueue(p);
		}
	}

	if (rtsi_save(sla_id) != NULL_HDR) {
		p = rtsi_save(sla_id);
		rtsi_save(sla_id) = NULL_HDR;
		ASSERT(p -> IMCS_PROTOCOL == IMCS_RTSI_RCV_CODE);
		cdd_enqueue(p);
	}

	if (sla_tbl.sla[sla_id].set_queue == FAKE_QUEUE) {
		/* the sla was either sending the hello message,
	     or set up to send the hello message next */
		struct imcs_header * hello_msg;
		hello_msg = cddq.anchor[SLA_OUTQ + FAKE_QUEUE];
		if (hello_msg != NULL_HDR) {
			cddq.anchor[SLA_OUTQ + FAKE_QUEUE] = NULL_HDR;
			cdd_enqueue(hello_msg);
		}
	}

}  /* end cdd_clean_sla */

/**/
/*
 * NAME: cdd_merge 
 *                                                                    
 * FUNCTION: done when two set queues are merged to bring all
                    things to normal
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
cdd_merge(going_away_q, staying_q)
int going_away_q, staying_q;
{
	int q_stay, q_go;
	struct imcs_header *last_msg;

	q_stay = SLA_OUTQ + staying_q;
	q_go = SLA_OUTQ + going_away_q;

	if (cddq.anchor[q_go] != NULL_HDR) { /* queue empty */
		ASSERT(cddq.anchor[q_go] -> outcome == IMCS_CDD_ENQ);
		ASSERT(cddq.anchor[q_go] -> next_cddq_chain -> outcome == IMCS_CDD_ENQ);
		if (cddq.anchor[q_stay] == NULL_HDR) { /* queue empty */
			cddq.anchor[q_stay] = cddq.anchor[q_go];
		}
		else {
			ASSERT(cddq.anchor[q_stay] -> outcome == IMCS_CDD_ENQ);
			ASSERT(cddq.anchor[q_stay] -> next_cddq_chain -> outcome == IMCS_CDD_ENQ);
			last_msg = cddq.anchor[q_stay] -> next_cddq_chain;
			last_msg -> cdd_chain_word = cddq.anchor[q_go];
			cddq.anchor[q_stay] -> next_cddq_chain = 
			    cddq.anchor[q_go] -> next_cddq_chain;
			cddq.anchor[q_go] -> next_cddq_chain = NULL_HDR;
		}

		cddq.anchor[q_go] = NULL_HDR;
	}

}  /* end cdd_merge */


/**/
/*
 * NAME: cdd_reshuffle 
 *                                                                    
 * FUNCTION: done when set queue assignment changes to bring all
                        things to normal 
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
cdd_reshuffle()
{
	struct imcs_header * old_anchors[IMCS_SETQ_LIMIT];
	int queue;

	/* save the old queue and make all queue empty */
	for (queue = 0; queue < IMCS_SETQ_LIMIT; queue++) {
		int qi;
		qi = SLA_OUTQ + queue;
		old_anchors[queue] = cddq.anchor[qi];
		cddq.anchor[qi] = NULL_HDR;
	}

	/* now requeue all the messages */
	for (queue = 0; queue < IMCS_SETQ_LIMIT; queue++) {
		struct imcs_header *this, *next;
		struct imcs_header *next_in_cdd_chain, *prev_in_cdd_chain;
		for (this = old_anchors[queue]; this != NULL_HDR; 
						this = next_in_cdd_chain) {
			next_in_cdd_chain = this -> cdd_chain_word;
			ASSERT(this -> outcome == IMCS_CDD_ENQ);
			cdd_enqueue(this);
		}
	}

}  /* end cdd_reshuffle */


/**/
/*
 * NAME: cdd_return_msgs 
 *                                                                    
 * FUNCTION: imcs signals its users that a processor has gone down
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

cdd_return_msgs(int processor_id, short outcome, int qa)
{
	struct imcs_header *this, *next;
	struct imcs_header *next_in_cdd_chain, *prev_in_cdd_chain;
	int cnt=0;
	ASSERT(processor_id >= 0 && processor_id < IMCS_PROC_LIMIT);

	qa+=SLA_OUTQ;

	prev_in_cdd_chain = NULL_HDR;

	for (this=cddq.anchor[qa];this != NULL_HDR;this=next_in_cdd_chain) {
		next_in_cdd_chain = this -> cdd_chain_word;

		if (this -> IMCS_DEST_PROC == processor_id) {
			++cnt; /* number of messages being dequeued */
			/* dequeue this from the cdd chain */
			if (prev_in_cdd_chain == NULL_HDR) {
				/* this is at the head of the list */
				cddq.anchor[qa] = this -> cdd_chain_word;
				if (cddq.anchor[qa] != NULL_HDR)  
					/* adjust the pointer to last */
					cddq.anchor[qa]->next_cddq_chain = 
							this->next_cddq_chain;
			}
			else {
				prev_in_cdd_chain->cdd_chain_word = 
							this->cdd_chain_word;
                                if (cddq.anchor[qa]->next_cddq_chain == this)
                                        cddq.anchor[qa]->next_cddq_chain =
                                            prev_in_cdd_chain;

			}

			/* remove the entire imcs chain if any */
			for (; this != NULL_HDR; this = next) {
				this -> outcome = outcome;
				next = imcs_send_done(this);

				switch ( (int) next) {
				case -2 :
					PANIC("imcs_send_done returns -2 when called from enqueue_to_sla");
					break;
				case (int) 0 /* NULL_HDR */ :
		/* no more header for this subchannel or queues, do the next
		   header enqueued to the cdd level */
					break;
				default :
					break;
				}  /* end switch */
				NOTIFY(this); 	    
			}  /* end for loop on imcs chain */
		}
		else {
			prev_in_cdd_chain = this;
		}
	}  /* end for loop */

	return cnt;
}  /* end cdd_return_msgs */


/**/
/*
 * NAME: cdd_close 
 *                                                                    
 * FUNCTION: called when imcs is stopped cleans up all queues
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
cdd_close()
{
	int qa, processor_id;
	int processor_priority;
	struct imcs_header *tc, *nc, *p, *n;
	processor_priority = i_disable(IMCS_INT_PRIORITY);

/* call the notification routine for all headers in the subchannel queues
   both receive for rts and send for rtr */
	for (qa = 0; qa < SLA_OUTQ; qa++) {
		for (tc = cddq.anchor[qa]; tc != NULL_HDR; tc = nc) {
			nc = tc -> next_cddq_chain; 
				/* nc is the next subchannel queue */
			p = tc -> cdd_chain_word; /* get the first header */
			tc -> cdd_chain_word = NULL_HDR;  /* break the chain */
			for (;p != NULL_HDR; p = n) {
				n = p -> cdd_chain_word;
				p -> outcome = IMCS_DYING;
				p -> IMCS_MSGLEN = 0;
				NOTIFY(p); 	
			}
		}
		cddq.anchor[qa] = NULL_HDR;
	}

	/* call the notification routine for all outboud messages, send rts(i),
	   and receive rtr */
	for (processor_id = 0; processor_id < IMCS_PROC_LIMIT; processor_id++){ 
		if (imcs_addresses.proc[processor_id].set_queue!=INVALID_SETQ){
			(void) cdd_return_msgs(processor_id, IMCS_DYING,
			    imcs_addresses.proc[processor_id].set_queue);
		}
		(void) cdd_return_msgs(processor_id, IMCS_DYING, LIMBO_QUEUE);
	}

	/* now return all rtsi headers to the pool and the pages to the vmm */
	for (p = cddq.anchor[SLA_INQ]; p != NULL_HDR; p = n) {
		if(p == p ->cdd_chain_word) 
			n = NULL_HDR;
		else 
			n = p -> cdd_chain_word;
		#ifdef RTSILONG
		/* return pages to the pager */
		PANIC("imcs cannot return pages to pager");
		#endif
		put_imcs_hdr((caddr_t)((int)p - IMCS_RTSI_UHDR_SIZE));
	}
	cddq.anchor[SLA_INQ] = NULL_HDR;

	i_enable(processor_priority);

}  /* end cdd_close */

/*^L*/
/*
 * NAME: check_que_size
 *
 * FUNCTION: Test for any queues with no activity for more than 5 seconds
 *	     and flush the queue.
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

check_que_size()
{
	int processor_priority;
	int num;
	int cnt;
	TVARS tvar;
	for(cnt=0; cnt < IMCS_PROC_LIMIT; cnt++) {
		if(que.cnt[cnt] >= HANDFULL && time-que.time_stamp[cnt] > 5) {
SOL_TRACE("cqs0",cnt,que.cnt[cnt],imcs_addresses.proc[cnt].set_queue);
			processor_priority = i_disable(IMCS_INT_PRIORITY);
			if (imcs_addresses.proc[cnt].set_queue != 
			    INVALID_SETQ) {
SOL_TRACE("cqs1",cnt,que.cnt[cnt],0);
				num = cdd_return_msgs(cnt,IMCS_DOWN_CONN,
				    imcs_addresses.proc[cnt].set_queue);

				que.total_queued-=num;
				que.cnt[cnt]-=num;
				que.time_stamp[cnt]=0;	

				num = cdd_return_msgs(cnt,IMCS_DOWN_CONN,
				   LIMBO_QUEUE); 

				que.total_queued-=num;
				que.cnt[LIMBO_SLOT]-=num;

				if(que.total_queued < HANDFULL ) {
					STOP_TIMER(que.timer);
					que.time_running = FALSE;
					i_enable(processor_priority);
					break;
				}
			}
			i_enable(processor_priority);
		}
	}
	if(que.total_queued >= HANDFULL) {
SOL_TRACE("cqs2",que.total_queued,0,0);
       		tvar.id = 0;
       		tvar.func = QUEUE_FLUSH;
       		tvar.setup = FALSE;
       		START_TIMER(que.timer,tservice,tvar,1,0);
		que.time_running = TRUE;
	}
}

verify_que(cnt) 
int cnt;
{
int cntr;
int total=0;
struct imcs_header *msg;
	for(cntr=SLA_OUTQ; cntr < SLA_OUTQ+5; ++cntr) {
		if(cddq.anchor[cntr] != NULL_HDR) {
			++total;
			msg=cddq.anchor[cntr];
			while(msg->cdd_chain_word != NULL_HDR) {
				++total;
				msg = msg->cdd_chain_word;
			}
		}
	}
	cnt -= que.cnt[0];
	if(cnt != total) 
		brkpoint();
}
			
	
