/* @(#)62	1.5  src/bos/kernext/sol/sol_vars.h, sysxsol, bos411, 9428A410j 7/16/91 11:56:57 */
#ifndef _H_SOL_VARS
#define _H_SOL_VARS
/*
 * COMPONENT_NAME: (SYSXSOL) - Serial Optical Link Device Handler Include File
 *
 * FUNCTIONS: sol_vars.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
	definition for the Transmit Header Register (THR) and
	the Primary Receive Header Register (PRHR)

	the PRHR is used to understand what was just received

	the THR is used to control sends.

	The THR is used to cause the hardware to send all types of frame.
	(there are 3 type of serial link (sl) frames: link control or type 1-b,
	device control or type 1-a, and device data or type 2.)
*/
struct sl_frame {
	uint	did_sid;	/* 2nd and 4th byte forced to zero by hardware.
				   In the thr the dest. (high order byte) is
				   the link address of the other side; in the 
				   prhr the destination (high order byte) is 
			 	   the address of our side */
				/* thr hex  128    prhr hex  114 */
	uint	link_ctl;	/* values follow - only first byte used */
				/* thr  hex 12c    prhr  hex 118 */
	union	third_word {
	  uint info;		/* 1st word of info field for type 1-a frames */
	  uint dev_ctl;		/* for type 1-b and 2 frames; byte 1 - d_ctl (values
				   follow); byte 2 - flags; byte 3 and 4 - subchannel */
	} w3;
				/* thr  hex 130    prhr  hex 11c */
	
	union	fourth_word {
	  uint info;		/* second word of info field for type 1-a frames */
	  uint B;		/* B register for type 1-b frames */
	  /* for type 2 frames the B register is not received into */
	} w4;
				/* thr  hex 134    prhr  hex 120 */
	union	fifth_word {
	  uint count;	 	/* last 3 bytes are transmit byte count (in info field)
				   for type 1-a frames */
	  uint rfrst;		/* On rcv. first byte is frame status (values follow) */
	} w5;
				/* thr  hex 138    prhr  hex 124 */
};


/* Each sla has a set of registers, in I/O space, through which it is         */
/* manipulated.  Some registers have to be accessed singly (i.e. load/store)  */
/* others can be accessed via string ops.  Doing the wrong thing, or accessing*/
/* reserved addresses (unimplemented) causes a data storage interrupt.  The   */
/* same goes for trying to access the third sla if it's not there.  The       */
/* commands all return the status1 register - to issue the command, load from */
/* the appropriate address.                                                   */

struct slaregs {
	uint isr;	/* interrupt source reg		rw */ /* hex 0 */
	uint config;	/* configuration and receive parameters	ro *//* hex 4 */
	uint toc;	/* time out counter		ro */ /* hex 8 */
	uint res1[21];	/* reserved */
	uint ch_start;	/* start channel trigger	ro */ /* hex 60 */
	uint ch_stop;	/* stop channel trigger		ro */ /* hex 64 */
	uint ch_op;	/* operational channel trigger	ro */ /* hex 68 */
	uint ch_halt;	/* halt channel trigger		ro */ /* hex 6c */
	uint status1;	/* status 1 trigger		ro */ /* hex 70 */
	uint status2;	/* status 2 trigger		ro */ /* hex 74 */
	uint sample;	/* sample trigger		ro */ /* hex 78 */
	uint ch_cancel;	/* cancel trigger		ro */ /* hex 7c */
	uint res2[37];	/* reserved */
	union {
		uint prhr[5];	/* primary receive header regs	rw - hex 114 */
		struct sl_frame fr;
	} prhr;
	union {
		uint thr[5];	/* transmit header regs	rw */ /* hex 128 */
		struct sl_frame fr;
	} thr;
	uint ccr;		/* channel control register rw */ /* hex 13c */
	uint tcw[NUM_TCWS];	/* tag words		rw */ /* hex 140 */
	/* more reserved stuff at hex 180 */
};
/* the pointer to this structure must be instantiated as a pointer to a volatile structure */

/* typedef struct slaregs volatile * slaiop;  this typedef does not work */
#define SLAIOP(p)	struct slaregs volatile *p

#define SLA_THR_SZ	sizeof(struct sl_frame)

/*
	data structure to hold a set of addresses as returned by the switch
*/

struct sla_addrmap {
	short count;
	uchar link_address[MAX_SWITCH_ADDR];
};


/*
	format of the "hello" message
*/

/* version 1 of the hello message */
struct imcs_hello_message1 {
	uint imcs_version;
	short sla_id;
	short imcs_pid;  /* -1 means sender of msg does not have an imcs id */
	uchar link_address;
	uchar configuration_reg;
	char type;		
	char machine_type;
	int machine_id;
	char filler[112];	/* make it 128 long */
};

/*
  format of the imcs processor/(qid or channel)  send queues for rts and rtsi
*/

struct isq {
	struct imcs_header *anchor_rtsi[MAX_IMCS_SQ_ANCHOR];
	struct imcs_header *anchor_rts[MAX_IMCS_SQ_ANCHOR];
};

/*
  imcs receive queue format
  irq = imcs receive queue (really an imcs account)
  see comment at bottom
*/


struct irq_tbl {  /* declaration */
	struct irq_block * free_list;		/* head of free list */
	struct irq_block * anchor[IRQ_TBL_SIZE];
};

struct irq_block {
	struct irq_block * next;	/* next controll block on hash class or
					   free list */
	struct irq_block * waiting;	/* next qid waiting to send deblock message */
	void (*imcs_slih_ext)();	/* called when receive ends */
	long rcv_count;		/* number of messages received */
	uint queue_id;		/* queue id */
	short max_num_hdr;		/* declared limit on receive headers */
	short hdr_in_use;		/* number used */
	short num_buf;		/* number of frames given to imcs */
	uchar pids[IMCS_PROC_LIMIT];	/* flags for each processor */
#define NACKED_BUFFER	1	/* true means queue had to nack */
#define NACKED_HEADER	2	/* true means queue had to nack */
	uchar status;			/* possible values follow */
	uchar type;			/* type of queue, values follow */
};

#define NULL_IRQ 	(struct irq_block *) NULL
/*
	various counters etc. to log sla events
*/

struct tag {
	unsigned pageno: 20;
	unsigned offset: 6;
	unsigned  count: 6;
	} ;


struct imcs_header {
	/* the following field filled by caller of imcs_sendmsg */
	union imcs_protocol {
		ushort code[2];		/* values follow */
		struct action {
			unsigned op_code : 1;
			unsigned go : 1;
			unsigned automatic : 1;
			unsigned filler : 13;	/* get to next half word */
			unsigned queue : 1;	
		} action;
	} imcs_protocol;

	/* end of field filled by caller of imcs_sendmsg */

	/* the next 2 fields are not used by imcs, and can be used freely */
	uint user_word1;
	uint user_word2;

	short imcs_send_proc;	/* sending processor */
	ushort outcome;		/* imcs reports outcome, values follow */

	/* imcs internal use only */
	struct imcs_header *imcs_chain_word;	/* for queueing */
	struct imcs_header *cdd_chain_word;	/* for queueing */
	struct imcs_header *next_imcsq_chain;	/* for queueing */
	struct imcs_header *next_cddq_chain;	/* for queueing */

	/* the following 2 fields filled by caller of imcs_sendmsg */

	void (*notify_address)();	/* function called at xmit end */

	short dest_proc_token;		/* destination processor */

	/* end of field filled by caller of imcs_sendmsg */

	short sla_id;			/* port used to receive */

	ulong imcs_address;
	ulong imcs_linkctl;
	ushort imcs_devctl;

	/* the following 4 fields must be filled by caller of imcs_sendmsg */

	ushort imcs_subch;
	ushort B_reg_half;
	ushort imcs_qid;
	long imcs_msglen;

	/* end of field filled by caller of imcs_sendmsg */

	ulong imcs_ccr;

	/* the tags must be filled by caller of imcs_sendmsg */

	union tags {
		ulong tagwords[NUM_HDR_TCWS];
		struct tag tag[NUM_HDR_TCWS];
	} tags;
};

/* values assumed by imcs_protocol.code[0] */
#define IMCS_PROTOCOL	imcs_protocol.code[0]

/* values assumed by imcs_protocol.action.op_code */
#define IMCS_OP_CODE	imcs_protocol.action.op_code

/* values assumed by imcs_protocol.action.go */
#define IMCS_GO		imcs_protocol.action.go

#define	IMCS_AUTOMATIC	imcs_protocol.action.automatic

#define IMCS_SERIALIZE	imcs_protocol.action.queue

#define IMCS_DEST_PROC	dest_proc_token
#define IMCS_SUBCHANNEL	imcs_subch
#define IMCS_QUEUE	imcs_qid
#define IMCS_MSGLEN	imcs_msglen

#define NULL_HDR	(struct imcs_header *) NULL
/* defines for tag words */
#define IMCS_PAGENO(n)	tags.tag[(n)].pageno
#define IMCS_OFFSET(n)	tags.tag[(n)].offset
#define IMCS_COUNT(n)	tags.tag[(n)].count
#define IMCS_TAG(n)	tags.tagwords[(n)]

/*
  represents sla's logically to the imcs cdd layer
  this structure is shared with the device driver
*/

struct sla {				/* descrives one sla */
	uint io_address;		/* address of the sla in io space */
	struct imcs_header *_active;	/* address of header being sent */
#define active_hdr(sla_id)	sla_tbl.sla[sla_id]._active
	struct imcs_header *_passive;	/* address of header being received */
#define passive_hdr(sla_id)	sla_tbl.sla[sla_id]._passive
	struct imcs_header *_primary;	/* address of header being used by driver */
#define primary_hdr(sla_id)	sla_tbl.sla[sla_id]._primary
#define ala_state(sla_id)	sla_tbl.sla[sla_id]._primary
	struct imcs_header *_rtsi_save;
#define rtsi_save(sla_id)	sla_tbl.sla[sla_id]._rtsi_save
	void *time_struct;		/* address of a timer structure */
	uint s1_save;			/* saved value of status 1 register */
	uint s2_save;			/* saved value of status 2 register */
	uint ccr_con;			/* value of ccr to be used with connect data frame */
	uint ccr_all;			/* value of ccr register to be used with all channel starts */
	uint data_frame;		/* expected data frame */
	int diag_process;		/* diagnostic process wait list */
	union {
	  uint	full;
	  struct {
	    uchar other;		/* for switched connections is the address of the switch */
	    uchar zero1;
	    uchar my;
	    uchar zero2;
	  } piece;
	} addr;
#define my_addr(sla_id)		sla_tbl.sla[sla_id].addr.piece.my
#define other_addr(sla_id)	sla_tbl.sla[sla_id].addr.piece.other
#define sla_address(sla_id)	sla_tbl.sla[sla_id].addr.full
	union {
	  uint word;
	  struct {
	    unsigned _present : 1;	/* on if the sla is present */
	    unsigned _odc_absent : 1;	/* on if the ODC is not present */
	    unsigned _dma_req :1;	/* on if dma recovery is required */
	    unsigned _sdma_wait : 1;	/* on if must wait for send dma rec. */
	    unsigned _sdma : 1;		/* on if sender dma needs recovery */
	    unsigned _rdma : 1;		/* on if receiver dma needs recovery */
	    unsigned _inter : 1;	/* on while ih. is running */
	    unsigned _scr : 1;		/* on while channel is recovering */
	    unsigned _started : 1;	/* on if imcs has started */
	  } bits;
	} status;
#define sla_present(sla_id)	sla_tbl.sla[sla_id].status.bits._present
#define sla_no_ODC(sla_id)	sla_tbl.sla[sla_id].status.bits._odc_absent
#define sla_dma(sla_id)		sla_tbl.sla[sla_id].status.bits._dma_req
#define sla_sdma_wait(sla_id)	sla_tbl.sla[sla_id].status.bits._sdma_wait
#define sdma(sla_id)		sla_tbl.sla[sla_id].status.bits._sdma
#define rdma(sla_id)		sla_tbl.sla[sla_id].status.bits._rdma
#define sla_inter(sla_id)	sla_tbl.sla[sla_id].status.bits._inter
#define sla_scr(sla_id)		sla_tbl.sla[sla_id].status.bits._scr
#define sla_started(sla_id)	sla_tbl.sla[sla_id].status.bits._started
	short event_count;		/* idles seen during continuous sequences */
	uchar imcs_st;			/* status -- values follow */
	uchar sla_st;			/* status -- values follow */
	uchar set_queue;		/* identifies set queue that this sla serves */
	uchar config_reg;		/* hardware configuration register */
	uchar connection;		/* values follow */
	uchar ack_type;			/* kind of ack frame -- values follow */
};
/* NOTE:  passive addresses is the header that imcs will use should a message
   arrive.  Normally the sla is in receiving mode using precisely this header.
   During send this header is "downloaded" and imcs tries to send.  If it must
   backup from the send, this is the header that will be loaded.  primary 
   addresses the header that the driver chooses to use.  primary can only be 
   equal to passive or active; otherwise primary is null */

struct sla_tbl {		/* represents all sla's */
	short number_sla;	/* number of sla present in the machine */
	int buffer;		/* memory image for continous sequences */
	struct sla sla[MAX_NUM_SLA];	/* individual sla's */
};

/*
	miscellaneous declares for imcs
	initialized and in imcs_pin.c
*/

struct imcsdata {
  uchar imcs_initialized;	/* 0 = not initialized, 1 = initialized */
  uchar imcs_started;
  struct {
    char clone;
    short n;
  } imcs_version;
};

/*
	tables for sla addresses
*/


/*
	The imcs_addresses structure allows imcs find out the sla addresses
	of a processor, and which slas in the host machine can be used
	to send to the processor.


	The sla_addr array contains, for each possible processor, a list of
	sla addresses on that processor.

	The sla_to array contains, for each possible processor,
	a list of sla's, on this machine, that talk to that processor.
	If there is a switch, then sla_to contains all the sla's.

	if there is no switch then the sla_to and sla_addr are
	sorted so the corresponding entries refer to the same sla.

	sla_addr_tb is a lookup table that relates an address to a
	processor.
*/

struct imcs_addresses {
	struct {
		slaid_t sla[MAX_NUM_SLA];
		uchar set_queue;	/* q. if all sla busy */
		slaid_t av_sla;		/* next available sla */
		slalinka_t av_addr;	/* and link address */
		uchar filler;	/* this makes each structure 24 bytes long */
		slalinka_t sla_addr[MAX_NUM_SLA][MAX_NUM_SLA];
	} proc[IMCS_PROC_LIMIT];
	short sla_addr_tb [MAX_NUM_SLA][SLA_ADDR_RANGE];
};


struct cddq {
	struct imcs_header *anchor[CDDQ_TABLE_SIZE];
};

#define WRT_B_REG(hdr,value)    *(ulong *)(&(hdr -> B_reg_half)) = (ulong) value

typedef struct {
        uchar id;
        uchar func;
        uchar setup;
	uchar filler;
        }TVARS;

#define LIMBO_SLOT  IMCS_PROC_LIMIT
struct que {

        int cnt[IMCS_PROC_LIMIT+1];
        int time_stamp[IMCS_PROC_LIMIT+1];
        int total_queued;
        void *timer;
	int time_running;
        };


#endif _H_SOL_VARS
