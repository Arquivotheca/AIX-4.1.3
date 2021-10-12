static char sccsid[] = "@(#)72	1.12  src/bos/kernext/sol/solutil.c, sysxsol, bos411, 9428A410j 6/9/93 10:55:03";

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
#include <sys/trcmacros.h>	/* tracing stuff */
#include <sys/trchkid.h>	/* trace hook ids */
#include <sys/syspest.h>	/* ASSERT */
#include <sys/param.h>		/* for PAGESIZE, PGSHIFT, and NBPB */
#include <sys/sleep.h>		/* for EVENT_NULL */
#include <sys/malloc.h>		/* for xmalloc */
#include <sys/intr.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/timer.h>
#include <sys/m_intr.h>
#include <sys/errno.h>
#include <sys/errids.h>
#include <sys/err_rec.h>
#include <sys/systm.h>
#include <sys/ioacc.h>

#include "soldd.h"

#include "sol_proto.h"
#include "sol_extrn.h"

void err_log(int , int, int , int);

extern int slaih();
extern uchar rec_ts_used;
extern int bad_port[MAX_NUM_SLA];
extern struct sol_ddi sol_ddi;
extern void * timer_sp[MAX_NUM_SLA];
extern caddr_t send_header;
extern int poll_cnt[MAX_NUM_SLA];
extern struct que que;
struct elog {
	int 	active;
	int 	threshold;
	int	count;
	long 	lasttime;
	} elog[MAX_ERRORS];

typedef struct errmsg {
        struct  err_rec0 err;
        int     stat1;
        int     stat2;
} errmsg;

struct mblks {
		int used;
		char *adrs[MAX_NUM_SLA];
	} mblks[2];

void *start_timer[MAX_NUM_SLA];

/**/
/*
 * NAME:  imcs_init
 *                                                                    
 * FUNCTION:   intializes all imcs data structures, and reset the hardware
 *		(real or simulated) to known state
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
 *	 0 -- initialization succeded
 *	 1 -- already initilized
 *	 -1 -- could not get headers to receive
 */  

int
imcs_init()
{
	int error_code;
	int i;
	struct irq_block irq;	/* all these used to check sizes */
	caddr_t addr;
        struct trb *trb;

	if (imcsdata.imcs_initialized) return 1; /* already initialiazed */
	
	mblks[0].used =0;
	mblks[1].used =0;

	/* various checks */
	ASSERT(sizeof(struct imcs_header) == IMCS_HDR_SIZE);
	ASSERT(NUM_HDR_TCWS == NUM_TCWS);
	ASSERT( (1 << PGSHIFT) == PAGESIZE);  /* PGSHIFT is used as log
				base 2 of PAGESIZE */
	ASSERT(sizeof(struct sla) == SIZE_OF_SLAS);

	/* initialize the imcs send queues, */
	for (i = 0; i < MAX_IMCS_SQ_ANCHOR; i++) {
		isq.anchor_rtsi[i] = NULL_HDR;
		isq.anchor_rts[i] = NULL_HDR;
	}

	/* the imcs receive queues, */
	for (i = 0; i < IRQ_TBL_SIZE; i++)
		irq_tbl.anchor[i] = NULL_IRQ;

	irq_tbl.free_list = NULL_IRQ;

	/* the receiver header area, */
	init_imcs_hdr();

	/* the imcs cdd queues, */
	for (i = 0; i < CDDQ_TABLE_SIZE; i++)
		cddq.anchor[i] = NULL_HDR;

	/* the address/processor table */
	imcs_flush_addr();	/* empty tables */

	/* the sla data structure, */
	sla_tbl.number_sla = 0;

	addr = xmalloc(IMCS_LINE_SIZE * 16, IMCS_LOG2_LINE_SIZE, pinned_heap);
	bzero((caddr_t) addr, IMCS_LINE_SIZE * 16);
	ASSERT(addr != NULL);
	sla_tbl.buffer = (int) addr;
	ASSERT(sla_tbl.buffer != NULL);
	for (i = 0; i < MAX_NUM_SLA; i++) {
		active_hdr(i) = NULL_HDR;
		passive_hdr(i) = NULL_HDR;
		primary_hdr(i) = NULL_HDR;
		rtsi_save(i) = NULL_HDR;
		sla_tbl.sla[i].time_struct = (void *) NULL;
		sla_tbl.sla[i].s1_save = 0;
		sla_tbl.sla[i].s2_save = 0;
		sla_tbl.sla[i].ccr_all = 0;
		sla_tbl.sla[i].ccr_con = 0;
		sla_tbl.sla[i].diag_process = EVENT_NULL;
		sla_address(i) = 0;
		sla_tbl.sla[i].status.word = 0;  /* set all bits off */
		sla_tbl.sla[i].event_count = 0;
		sla_tbl.sla[i].imcs_st = SLA_INITIAL_CODE;
		sla_tbl.sla[i].sla_st = SLA_NOTSYNC_CODE;
		sla_tbl.sla[i].set_queue = INVALID_SETQ;
		sla_tbl.sla[i].connection = 0;
		sla_tbl.sla[i].ack_type = 0;
	}

	init_auditor(); /* and the imcs auditor */

	/* let the driver set up the right info */
	if (error_code = sla_init()) return error_code;

/* now that the number of sla in known initialize the buffer pool area */
/* this sets up the pool data structure, inclusing the stubs used to link
pages.  Also it gets the pages needed to do rtsi rcv, if rtsi receive
of pages is enabled */

	ipool_init(sla_tbl.number_sla * (NUM_TCWS - 1));

	/* start to receive on all sla's */
	if (error_code = req_imcs_hdr(sla_tbl.number_sla)) return error_code;

	imcsdata.imcs_initialized = TRUE;

	for(i=0; i < MAX_ERRORS ; i++) {
        	elog[i].active = TRUE;
        	elog[i].threshold = 0 ;
        	elog[i].count = 0;
        	elog[i].lasttime = 0;
        } 

	return 0;
}



/**/
/*
 * NAME: imcs_terminate
 *                                                                    
 * FUNCTION: cleans up imcs, so that the kernel extension can be unloaded
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
imcs_terminate()
{
int cnt;


	if (!imcsdata.imcs_initialized) return; /* not initialized */
	imcsdata.imcs_initialized = FALSE;
	imcs_host = -1;
	xmfree((void *) sla_tbl.buffer, pinned_heap);
	xmfree(send_header,pinned_heap);

	for(cnt=0; cnt < mblks[0].used; cnt++) 
		xmfree(mblks[0].adrs[cnt],pinned_heap);
	for(cnt=0; cnt < mblks[1].used; cnt++) 
		xmfree(mblks[1].adrs[cnt],pinned_heap);

	sla_tbl.buffer = 0;

	sla_terminate();  /* terminate the driver */

}  /* end imcs_terminate */

/**/
/*
 * NAME: init
 *                                                                    
 * FUNCTION: initializes one sla
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
        0 -- initialization succeded
        1 -- initialization failed
 */  

static int
init(sla_id)
int sla_id;
{
	uint s1_val, s2_val;
	SLAIOP(slap);
	int i;
	struct trb * t;

	ADDR_SLA(sla_id,slap);

	s1_val = slap -> sample;	/* touch the sample register */
	sla_tbl.number_sla++;			/* count this sla */
	sla_present(sla_id) = TRUE;
	if (PUISTATE(slap -> sample) != PUI_NOOP) {
		/* sla is opearational.  Halt it */
		BUS_GETLX((long *) &slap->ch_halt, &s1_val);
	}

	slap -> isr = sla_id + SLA_INTR_OFFSET;   /* store known value in isr */

	/* read and save the configuration register */
	sla_tbl.sla[sla_id].config_reg = EXTRACT_CONFIG(slap -> config);

	switch (sla_tbl.sla[sla_id].config_reg) {
	case SLAC_INIT :
	case SLAC_CMB2E :
		sla_dma(sla_id) = TRUE;
		break;

	case SLAC_CMB2S :
	case SLAC_CMB3E :
	case SLAC_4S :
		break;

	default :
		PANIC("slainit: unknown configuration value");
	}  /* end switch on configuration */


	/* clear junk out of registers status 1 and 2 */
	s2_val = slap -> status2;
	s1_val = slap -> status1;


	/* initialize the tag table and regs to avoid parity errors later */
	INIT_REGS(slap);
	INIT_TAGS(slap);


	/* read registers 1 and 2 */
	s2_val = slap -> status2;
	s1_val = slap -> status1;


	/* make sure an optical card is attached to the sla */
	if (s2_val & NO_OPCARD) {
		sla_no_ODC(sla_id) = TRUE;
	}


	/* disable the reset bit in the sla and set the auto busy */
	sla_tbl.sla[sla_id].ccr_all = AUTO_BUSY | RSTD;


	/* get a timer structure for later use */
	t = talloc();
	ASSERT(t != (struct trb *) NULL);
	sla_tbl.sla[sla_id].time_struct = (void *) t;
	t -> flags = 0;			/* initialize timer structure */
	t -> func = NULL;
	t -> func_data = 0;
	t -> ipri = IMCS_INT_PRIORITY;
	/* lock to crystal timers */

	t = talloc();
	ASSERT(t != (struct trb *) NULL);
	start_timer[sla_id] = (void *) t;
	t -> flags = 0;			/* initialize timer structure */
	t -> func = NULL;
	t -> func_data = 0;
	t -> ipri = IMCS_INT_PRIORITY;


	/* turn on laser,set auto scr flag (effective only on channel start) */
	sla_tbl.sla[sla_id].ccr_all |=  AUTO_SCR | DRIVER;

	sla_tbl.sla[sla_id].sla_st = SLA_NOTSYNC_CODE;

	/* install the interrupt handler */
	sla_handler[sla_id].next = (struct intr *) NULL;
	sla_handler[sla_id].handler = (int (*) ()) slaih;
	sla_handler[sla_id].bus_type = BUS_PLANAR;
	sla_handler[sla_id].flags = INTR_NOT_SHARED;
	sla_handler[sla_id].level = sla_id + SLA_INTR_OFFSET;
	sla_handler[sla_id].priority = IMCS_INT_PRIORITY;
	sla_handler[sla_id].bid = ( sol_ddi.sla_buid[sla_id] << 20);

/* this will be moved eventually */
	i_init(&sla_handler[sla_id]);

	UNADDR_SLA(slap);
	return 0;

}  /* end init */


/**/
/*
 * NAME:  sla_init
 *                                                                    
 * FUNCTION: try to initialize all slas
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
 *       0 -- initialization succeded
 *       1 -- initialization failed
 */  

int
sla_init()
{
	int i, rc;
	long *off,*ud,*udr,*sof;

	/* write continous sequences in buffer area */
	
	off = (long *) (sla_tbl.buffer + OFF_SEQ_ADDR);
	ud = (long *) (sla_tbl.buffer + UD_SEQ_ADDR);
	udr = (long *) (sla_tbl.buffer + UDR_SEQ_ADDR);
	sof = (long *) (sla_tbl.buffer + SOF_SEQ_ADDR);

	for (i = 0; i < IMCS_LINE_SIZE / 2; i++) {
		*off++ = OFF_SEQ_W1;
		*off++ = OFF_SEQ_W2;
		*ud++ = UD_SEQ_W1;
		*ud++ = UD_SEQ_W2;
		*udr++ = UDR_SEQ_W1;
		*udr++ = UDR_SEQ_W2;
		*sof++ = IDLE_SEQ_W1;
		*sof++ = IDLE_SEQ_W2;
	}

	/* ovewrite one word with a word containing the sof character */
	/* the third to last */
	sof = (long *) (sla_tbl.buffer + SOF_SEQ_ADDR + 61 * 4); 
	*sof = SOF_WORD;


	sla_tbl.number_sla = 0;
	rc = 0;
	for (i = 0; i < MAX_NUM_SLA; i++) {
		int this_rc;
		sla_tbl.sla[i].io_address = ( sol_ddi.sla_buid[i] << 20);
		if (sol_ddi.port_state[i] != SOL_NO_PORT) {
			this_rc = init(i);
		}
		if (this_rc) rc = this_rc;
	}

	return rc;

}  /* end sla_init */


/**/
/*
 * NAME: terminate
 *                                                                    
 * FUNCTION: brings down one sla.  It removes the interrupt handler, frees
 *	the timer structure and stops the sla.  The sla is left in 
 *	stopped state transmitting idles (since laser cannot be turned off).
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
terminate(sla_id)
int sla_id;
{
	SLAIOP(slap);
	uint s1_val;
	int processor_priority;

	processor_priority = i_disable(IMCS_INT_PRIORITY);

	ADDR_SLA(sla_id,slap);
	/* stop the sla */
	if (PUISTATE(slap -> sample) != PUI_NOOP)
		BUS_GETLX((long *) &slap->ch_cancel, &s1_val);

	UNADDR_SLA(slap);

	sla_tbl.sla[sla_id].imcs_st = SLA_FAILED_CODE;

	STOP_TIMER(sla_tbl.sla[sla_id].time_struct);
        STOP_TIMER(timer_sp[sla_id]);
        STOP_TIMER(start_timer[sla_id]);

	i_enable(processor_priority);

	i_clear(&sla_handler[sla_id]); /* remore ih from system */

	tfree(sla_tbl.sla[sla_id].time_struct); /* return structure to timer */
        tfree(timer_sp[sla_id]);
        tfree(start_timer[sla_id]);
	sla_tbl.sla[sla_id].time_struct = (void *) NULL;

}  /* end terminate */


/**/
/*
 * NAME: sla_terminate
 *                                                                    
 * FUNCTION: try to terminate all sla's
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

void sla_terminate()
{
	int i;

	for (i = 0; i < MAX_NUM_SLA; i++) {
		if (sla_present(i)) {
			terminate(i);
		}
	}

}  /* end sla_terminate */



/**/
/*
 * NAME:  do_restart
 *                                                                    
 * FUNCTION: starts one sla (do not confuse it with sla_start) 
 *		for normal operation.
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
do_restart(sla_id)
int sla_id;
{
	SLAIOP(slap);
	uint s1_val, s2_val;

	if (sla_no_ODC(sla_id)) return;

	bad_port[sla_id]=0;
	ADDR_SLA(sla_id,slap);

	/* clear junk out of registers status 1 and 2 */
	s2_val = slap -> status2;
	s1_val = slap -> status1;

	/* clear s1 and s2 save */
	sla_tbl.sla[sla_id].s1_save = 0;
	sla_tbl.sla[sla_id].s2_save = 0;

	if (s2_val & NO_OPCARD) goto err_out;
SOL_TRACE("rsrt",sla_id,s1_val,s2_val);
	switch (PUISTATE(s1_val)) {
	case PUI_NOOP :
		/* get the channel out of not-operational state */
		OPERATE_CHANNEL(slap, sla_id, s1_val, s2_val);
		/* channel must now be in the stopped state */
		if (PUISTATE(s1_val) != PUI_STOPPED) {
			sla_tbl.sla[sla_id].imcs_st = SLA_BROKEN_CODE;
			goto err_out;
		}
		break;
	case PUI_STOPPED :
		s1_val = slap -> sample;
		break;
	case PUI_WORK1 :
	case PUI_WORK2 :
		BUS_GETLX((long *) &slap->ch_cancel, &s1_val);
		break;
	default :
		PANIC("unknown pu interface state");
	}

	slap -> isr = sla_id + SLA_INTR_OFFSET;   /* store known value in isr */

	/* initialize the tag table, regs to avoid parity errors later */
	INIT_REGS(slap);
	INIT_TAGS(slap);

	/* start channel for synchronization */
	sla_tbl.sla[sla_id].sla_st = SLA_NOTSYNC_CODE;
	sla_started(sla_id) = TRUE;
	SYNCHR_CHANNEL(slap, sla_id);  /*sla will interrupt when synchronized */
err_out:
	UNADDR_SLA(slap);

}  /* end restart */


/**/
/*
 * NAME: sla_restart 
 *                                                                    
 * FUNCTION: starts the normal operation of the slas (not to be confused with
 *           channel start)
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
sla_restart()
{
	int i;
	for (i = 0; i < MAX_NUM_SLA; i++) {
		/* if the sla is in diagnostic mode then leave it alone.
	     restart will be called when diagnostic ends */
		if (sla_present(i) &&
		    (sla_tbl.sla[i].sla_st != SLA_DIAGNOSTIC_CODE))
			restart(i);
	}

}  /* end sla_restart */

/**/
/*
 * NAME: sla_close
 *                                                                    
 * FUNCTION: kill all the sla's
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
sla_close()
{
	int i;

	for (i = 0; i < MAX_NUM_SLA; i++) {
		if (sla_present(i) &&
		    (sla_tbl.sla[i].sla_st != SLA_DIAGNOSTIC_CODE))
			sla_kill(i, SLA_STOPPED, OTH_STOP);
	}

}  /* end sla_close */

/**/
/*
 * NAME: imcs_start
 *                                                                    
 * FUNCTION:  starts imcs so we can send and receive
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS:  	 0 if succeded
 *		-1 if not initialized
 *		-2 if already started 
 */  

int
imcs_start(processor)
int processor;
{
        struct trb *trb;

	if (!imcsdata.imcs_initialized) return -1; /* not initialized */

	/* the next two line should execute atomically */
	if (imcsdata.imcs_started) return -2; /* already started */
	imcsdata.imcs_started = TRUE;
	clear_start_parms(); 
	bzero((void *) &que, sizeof(struct que));
        trb = talloc();
	trb -> flags = 0;			/* initialize timer structure */
	trb -> func = NULL;
	trb -> func_data = 0;
	trb -> ipri = IMCS_INT_PRIORITY;
        que.timer = (void *) trb;
	imcs_host = processor;
SOL_TRACE("isrt",0,0,0);
	imcs_rcv_start();

	sla_restart();

	return 0;
}

/**/
/*
 * NAME: imcs_stop
 *                                                                    
 * FUNCTION: stops the slas, and imcs can no longer send and receive.  It
 *           also cleans up the cdd queue so imcs can be unconfigured
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS:      0 if succeded
 *              -1 if not initialized
 *              -2 if already started
 
 */  


int
imcs_stop()
{

	if (!imcsdata.imcs_initialized) return -1; /* not initialized */

	/* the next two line should execute atomically */
	if (!imcsdata.imcs_started) return -2; /* already started */
	imcsdata.imcs_started = FALSE;

	/* stop and free the que timer */
	STOP_TIMER(que.timer);
	tfree(que.timer);

	/* bring the slas down */
	sla_close();

	/* flush the address tables */
	imcs_flush_addr();

	bzero(cck_proc, IMCS_PROC_LIMIT);

	/* clean the cdd level */
	cdd_close();

	return 0;
}


/**/
/*
 * NAME: sla_xmt_cont
 *                                                                    
 * FUNCTION: starts a continuos sequence
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS:NONE
 */  

void
sla_xmt_cont(slap, sla_id, offset)
SLAIOP(slap);
int sla_id;
uint offset;
{
	union {
		uint tagword;
		struct tag tcw;
	} mytag;
	uint s1_val;

	mytag.tagword = imcslra(sla_tbl.buffer + offset);
	mytag.tcw.count = 4;	/* four lines 256 bytes total  */
        slap -> tcw[0] = mytag.tagword;
        slap -> tcw[1] = LAST_TAG;    

	CH_TRANSPARENT(slap, sla_id);


}  /* end sla_xmt_cont */

/**/
/*
 * NAME: sla_do_scr
 *                                                                    
 * FUNCTION: starts the channel in scr mode sending either UD or UDR
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
sla_do_scr(slap, sla_id)
SLAIOP(slap);
int sla_id;
{

	START_SCR(slap, sla_id);

}  /* end sla_do_scr */


/**/
/*
 * NAME: sla_sync
 *                                                                    
 * FUNCTION: starts the channel in activate mode
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
sla_sync(slap, sla_id)
SLAIOP(slap);
int sla_id;
{
	uint s1_val, s2_val;
	int cnt;
	if (sla_no_ODC(sla_id)) return;

	s1_val = slap -> sample;

	if (PUISTATE(s1_val) == PUI_NOOP) {
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
	}

	if (PUISTATE(s1_val) != PUI_STOPPED) {
		s1_val = slap->ch_cancel;
		for(cnt=0; cnt < 50; cnt++) {
			if(PUISTATE(s1_val) == PUI_STOPPED) { 
				SYNCHR_CHANNEL(slap, sla_id);
				break; 	
			}
			else continue;
			sla_tbl.sla[sla_id].imcs_st = SLA_BROKEN_CODE;
		}
	}
	else {
		SYNCHR_CHANNEL(slap, sla_id);
	}

}  /* end sla_sync */


/**/
/*
 * NAME: sla_spin
 *                                                                    
 * FUNCTION: This function sits and spins waiting for an operation to complete
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: value of status1 register 
 */  

uint
sla_spin(slap, sla_id)
SLAIOP(slap);
int sla_id;
{
	int num_spins;
	volatile int dummy;
	uint s1_val;

	s1_val = slap -> sample;

	num_spins = 0;
	while (! (s1_val & OP_DONE) && PUISTATE(s1_val) != PUI_STOPPED &&
	    PUISTATE(s1_val) != PUI_NOOP &&
	    num_spins < 20) {
		dummy = s1_val;
		s1_val = slap -> sample;
		num_spins++;
	}

	s1_val = slap -> status1;  /* get value of register, else hardware will
						reject successive starts */
	return s1_val;

}  /* end sla_spin */

/**/
/*
 * NAME: tservice
 *                                                                    
 * FUNCTION: This is a general purpose timer service routine.  Whenever
 *	a timer expires the function whose value is in the func member of
 *	The TVAR structure will be called.
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
tservice(t)
struct trb *t;
{
	TVARS tvar;
        SLAIOP(slap);
        struct intr ih_s;

	*((ulong *)&tvar) = t->t_func_data;
	if(tvar.func != QUEUE_FLUSH)
		if(bad_port[tvar.id]) return;
	SYS_SOL_TRACE("timr",tvar.id,tvar.func,0);
	if(tvar.setup) 
        	ADDR_SLA(tvar.id,slap);
	switch(tvar.func) {
		case RESTART:		restart(tvar.id);
					break;
		case RESTART_ALA : 	sla_send_ala(slap, tvar.id);
					break;
		case RESTART_SYNC :	sla_sync(slap, tvar.id);
					break;
		case RESTART_CONT :	sla_poll(slap, tvar.id);
					break;
                case RESTART_CONTSCR :  sla_contscr(slap, tvar.id);
                                        break;
		case RESTART_RECOVERY:	sla_recovery(slap, tvar.id);
					break;
		case RESTART_RECPOLL :  sla_recpoll(slap, tvar.id);
					break;	
		case SLA_ADDR_UNH :	sla_addr_unh(slap, tvar.id);
					break;
		case RESTART_INT : 	ih_s.level = tvar.id+SLA_INTR_OFFSET;
        				slaih(&ih_s);  
					break;	
		case START_ALA : 	start_ala_seq(tvar.id);
					break;
		case RESTART_SEND :	sla_send(tvar.id);
					break;
		case TIMER_RECOVER : 	rec_ts_used = FALSE;
        				recover_imcs_two(tvar.id);
					break;
		case RESTART_HELLO : 	imcs_start_hello(tvar.id);
					break;
        	case TIMER_HELLO :	imcs_hello_work(tvar.id);
					break;
		case DIAG_TIMER:	imcs_diag_timer(tvar.id);
					break;
		case LOCK_XTAL_ON:	lock_xtal_start(slap,tvar.id,ON);
					break;
		case LOCK_XTAL_OFF:	lock_xtal_start(slap,tvar.id,OFF);
					break;
		case LOCK_XTAL_DONE:	lock_xtal_start(slap,tvar.id,DONE);
					break;
		case SEND_NEXT:		cdd_next(tvar.id);
					break;
		case QUEUE_FLUSH:	check_que_size();
					break;
		default:		break;
	}

	if(tvar.setup) 
        	UNADDR_SLA(slap);
}


/**/
int
imcs_diag_start(sla_id)
int sla_id;
{
        boolean_t diagnostics = TRUE;

        if (sla_id < 0) {
          /* this is a call from sladrive */
          sla_id = (-sla_id) - 1;
          diagnostics = FALSE;
        }

        if (sla_id < 0 || sla_id >= MAX_NUM_SLA) {
          return -1;
        }

        if (! sla_present(sla_id)) return -1;

        if (sla_tbl.sla[sla_id].sla_st == SLA_DIAGNOSTIC_CODE) return 1;

        if (diagnostics)
          sla_kill(sla_id, DIAG, OTH_DIAG);
        else
          sla_kill(sla_id, SLADRIVE, OTH_DIAG);

        return 0;

}  /* end imcs_diag_start */


int
imcs_diag_sleep(sla_id)
int sla_id;
{
        int rc;
	TVARS tvar;

        if (sla_id < 0 || sla_id >= MAX_NUM_SLA) {
          return -1;
        }

        if (! sla_present(sla_id)) return -1;

        if (sla_tbl.sla[sla_id].sla_st != SLA_DIAGNOSTIC_CODE) return 2;

        if (sla_tbl.sla[sla_id].diag_process != EVENT_NULL) {
          return 3;
        }

	tvar.id = sla_id;
	tvar.func = DIAG_TIMER;
	tvar.setup = FALSE;
        SLA_TIMER(sla_id, tvar, tservice, 20, 0);

        rc = e_sleep(&sla_tbl.sla[sla_id].diag_process, EVENT_SIGRET);

        if (sla_tbl.sla[sla_id].s1_save == 0xffffffff)
          rc = 4;

        return rc;

}  /* end imcs_diag_sleep */

void
imcs_diag_timer(sla_id)
int sla_id;
{

        sla_tbl.sla[sla_id].s1_save = 0xffffffff;
        if (sla_tbl.sla[sla_id].diag_process != EVENT_NULL)
          e_wakeup(&sla_tbl.sla[sla_id].diag_process);

}  /* end imcs_diag_timer */

/*
 * NAME: imcs_diag_stop
 *                                                                    
 * FUNCTION: returns the sla after diagnostics
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
 *         0 -- imcs reclaims device
 *         -1 -- device not present
 *         1 -- device currently owned by imcs, in other words, never given up
 */  

int
imcs_diag_stop(sla_id)
int sla_id;
{
        int processor_priority;

        if (sla_id < 0 || sla_id >= MAX_NUM_SLA) {
          return -1;
        }

        if (! sla_present(sla_id)) return -1;

        if (sla_tbl.sla[sla_id].sla_st != SLA_DIAGNOSTIC_CODE) return 1;

        processor_priority = i_disable(IMCS_INT_PRIORITY);  /* mask ints */
        sla_tbl.sla[sla_id].sla_st = SLA_NOTSYNC_CODE;
        if (imcsdata.imcs_started)
          restart(sla_id);
        else
          sla_kill(sla_id, SLA_STOPPED, OTH_STOP);
        i_enable(processor_priority);   /* restore ints */
        return 0;

}  /* end imcs_diag_stop */


/**/
/*
 * NAME: imcs_undeclare
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
imcs_undeclare(queue_id)
int queue_id;
{
        int rc;
        struct irq_block * irq;

        if ((irq = rqctl_unhash(queue_id) ) == NULL_IRQ) {
          rc = QUEUE_NOT_DCL;
        }
        else {
          rqctl_put(irq);
          rc = DCL_DONE;
        }

        return rc;

}  /* end imcs_undeclare */


void err_log(int num,
	int errid,
	int s1_val,
	int s2_val)
{

	errmsg  msglog;

	if (!elog[num].active) return;

        msglog.err.error_id = errid;


        msglog.err.resource_name[0]='o';
        msglog.err.resource_name[1]='p';
        msglog.err.resource_name[2]='s';
        msglog.err.resource_name[3]='0';
        msglog.err.resource_name[4]='\0';
	
        msglog.stat1 = s1_val;
        msglog.stat2 = s2_val;

	switch(errid) {

	case ERRID_SLA_EXCEPT_ERR :
	case ERRID_SLA_PARITY_ERR :
	case ERRID_SLA_DRIVER_ERR :
	case ERRID_SLA_SIG_ERR :
	case ERRID_SLA_CRC_ERR :
	case ERRID_SLA_PROG_ERR :
        		errsave(&msglog,sizeof(errmsg));
			break;
	case ERRID_SLA_FRAME_ERR : 
			if(s1_val == 0) {
				if(elog[num+1].lasttime < time+3600) {
					elog[num+1].active = FALSE;
        				errsave(&msglog,sizeof(errmsg));
				}
				else {
					elog[num+1].lasttime = time;
				}
			}
			else {
				if(elog[num].lasttime < time+3600) {
					elog[num].active = FALSE;
        				errsave(&msglog,sizeof(errmsg));
				}
				else {
					elog[num].lasttime = time;
				}
			}
			break;
	default :	break;
	}
}

/*^L*/
/*
 * NAME: sol_loopback
 *
 * FUNCTION: try to send loopback (hello messages) to all sla's
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

sol_loopback()
{
        int i;

        for (i = 0; i < MAX_NUM_SLA; i++) {
		if (sla_present(i)) {
			imcs_start_hello(i);
		}
        }
	return 0;

}  /* end sol_loopback */

lock_xtal_start(SLAIOP(slap), int sla_id, int mode)
{
       	TVARS tvar;

	int s2_val; 
	int s1_val; 
        tvar.setup=TRUE;
        tvar.id = sla_id;

	if(mode == ON) {
       		BUS_PUTLX((long *) &slap->ccr, DRIVER | LOCK_XTAL_ON_CODE);
        	tvar.func = LOCK_XTAL_OFF;
        	START_TIMER(start_timer[sla_id],tservice,tvar,0,ONE_MILLI*20);
	}
	else if(mode == OFF) {
        	BUS_PUTLX((long *) &slap->ccr, DRIVER | LOCK_XTAL_OFF_CODE);
        	tvar.func = LOCK_XTAL_DONE;
        	START_TIMER(start_timer[sla_id],tservice,tvar,0,ONE_MILLI*20);
	}	
	else {
		s2_val = slap -> status2;
		s1_val = slap -> status1;
SOL_TRACE("xtal",sla_id,s1_val,s2_val);
        	if( PUISTATE(s1_val) == PUI_NOOP) {
                        /* trigger sla from non operational to stopped state */
                        BUS_GETLX((long *) &slap->ch_op, &s1_val);
		}
		do_restart(sla_id);
	}

}	

restart(int sla_id) 
{

        SLAIOP(slap);
       	TVARS tvar;

        ADDR_SLA(sla_id,slap);

	poll_cnt[sla_id] = 0;
        tvar.setup=TRUE;
        tvar.id = sla_id;
        tvar.func = LOCK_XTAL_ON;
       	BUS_PUTLX((long *) &slap->ccr, DRIVER | AUTO_SCR);
       	START_TIMER(start_timer[sla_id],tservice,tvar,0,ONE_MILLI*20);

	UNADDR_SLA(slap);
}
