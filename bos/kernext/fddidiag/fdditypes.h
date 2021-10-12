/* @(#)95       1.1  src/bos/kernext/fddidiag/fdditypes.h, diagddfddi, bos411, 9428A410j 11/1/93 11:01:15 */
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_FDDITYPES
#define _H_FDDITYPES

/*
 * The cmd_tag structure is used to keep track of an HCR command sent to the 
 * card.  The structure remembers the cmd_code (the HCR command) and the CPB
 * (command parameter block), the ctl/stat fields to control the command and 
 * the function to call for cmd specific completion handling.
 */
struct	cmd_tag
{
	short	cmd_code;		/* the command code */
	short	ctl;			/* control flags */
	uint	stat;			/* completion status of the command */
	short	cpb_len;		/* len of cpb (in words) */
	ushort	cpb[FDDI_CPB_SIZE];	/* command parameter block */
	int	(*cmplt)();		/* specific func called on completion */
	uint	pri;			/* indicates priority cmd required */
};
typedef	struct cmd_tag fddi_cmd_t;


/* -------------------------------------------------------------------- */
/* User's open element structure 					*/
/* -------------------------------------------------------------------- */
/* This structure is used to describe a user who has opened 
 * the FDDI device driver.  The open element describes either a user mode
 * or kernel mode user.  This structure is used for validating access to
 * the device, routing receive data, passing asynchronous status to
 * the user, etc.
 */
struct open_element 
{
	struct acs_tag	*p_acs;		/* the adap control struct for user */

	dev_t 		devno; 		/* our own devno  */
	chan_t 		chan; 		/* our own channel number */
	ulong		devflag; 	/* flags from open call */

	ulong		open_id; 	/* 
					 * id passed in in the open extension
					 * by kernel users only  
					 */
	void 		(*rcv_fn)(); 	/* 
					 * addr of kproc rcv "interrupt" 
					 * entry 
					 */
	void 		(*stat_fn)();	/* 
					 * addr of kproc sta "interrupt" 
					 * entry 
					 */
	void 		(*tx_fn)(); 	/* addr of kproc tx  "interrupt" 
					 * entry 
					 */

	int 		tx_fn_needed; 	/* TRUE if this kernel user needs 
					 * tx_fn  
					 */
	int		tx_acks;	/* number of TX acks pending for 
					 * this user 
					 */
	int 		selectreq;	/* flags indicating selnotify 
					 * requested 
					 */
	int		netid_cnt;	/* number of netids this user 
					 * has started 
					 */

	/* 
	 * User mode portion of the open element vars
	 */
	struct mbuf 	*p_rcv_q_head;  /* rcv queue mgmt for user open only*/
	struct mbuf 	*p_rcv_q_tail;	/* rcv queue tail ptr */
	int		rcv_cnt;	/* # of packets in receive queue */
	int	 	rcv_event; 	/* event list for e_sleep during read */
	int		stat_cnt;	/* # of status blocks in queue */
	int		stat_ls_cnt;	/* distance into the stat que of a */
					/*  lost status buffer */
	cio_stat_blk_t	*p_statq;	/* ptr to the status block que */
	int 		stat_que_ovflw;	/* the status que overflowed */
	short		stat_in;	/* stat que next in element index */
	short		stat_out;	/* stat que next out element index */
	short		stat_full;	/* is the status que full */
	ushort		addrs;		/* Bit flag used to identify addresses 
					 * in the acs that this user has set.
					 * a '1' in the flag correspondes to 
					 * the index in the acs structure by its
					 * position shifted over 
					 */
}; 
typedef struct open_element fddi_open_t;
/*
 * 	Adapter descriptor:
 *	We define 'addr' as 'hi' and 'lo' not to mirror what the
 *	adapter has but to make sure this struct is the right
 *	size. We treat 'addr' as a uint but if declared as such
 *	then the 'struct fddi_adap' is 12 bytes long (compiler
 *	pads the structure to make sure it alligns on the most
 *	demanding allignment any of it's members require.
 */
struct fddi_adap
{
	ushort	addr_hi;	/* Buffer address high */
	ushort	addr_lo;	/* Buffer address low */
	ushort	cnt;		/* Buffer size in bytes */
	ushort	ctl;		/* control flags */
	ushort	stat;		/* status flags */
};
typedef struct fddi_adap fddi_adap_t;

/*
 * HOST TRANSMIT DESCRIPTOR 
 *	The Host tx descriptor structure is the interface between
 *	the host and the adapter for writes.
 *
 * The 'smallframe' is a pinned chunk of memory used to dma
 *	'short' frames to the adapter. The frame is copied
 *	into a section of the 'smallframe' which is already setup
 *	for dma. This costs a buffer copy but save a 'd_master' which
 *	is an overall significant savings for small frames.
 * 
 */
struct fddi_tx
{
	fddi_adap_t 	adap;		/* the mirror of the adapt desc */
	short		offset;		/* offset of desc on adapter */
	struct mbuf	*p_mbuf;	/* ptr to the frame (chain of mbufs) */
	fddi_open_t	*p_open;	/* set when caller wants tx back */
	uint		p_d_addr;	/* dma region address for this desc */
	uint		p_d_sf;		/* dma region for smallframe */
	char		*p_sf;		/* 'smallframe' address */
	ushort		flags;		/* identifies if the regular frame
					 * or small frame is in use for this
					 * descriptor. 
					 */
	short		eof_jump;	/* relative idx to EOF from SOF desc */
	cio_write_ext_t	wr_ext;		/* write extension */
}; 
typedef	struct fddi_tx 	fddi_tx_t;

/*
 * 	fddi_tx_que is the structure of que info for the user tx que prior to
 * 	going on the adapter's queue
 */
struct fddi_tx_que
{
	struct mbuf	*p_mbuf;	/* ptr to the frame (chain of mbufs) */
	fddi_open_t	*p_open;	/* set when caller wants tx back */
	cio_write_ext_t	wr_ext;		/* write extension */
	uint		gather_cnt;     /* the number of mbufs for this write */
};
typedef struct fddi_tx_que fddi_tx_que_t;

/* 
 * This is the command structure used  to download microcode and perform
 * some diagnostic ioctls.  It is valid in the shared memory portion of 
 * the adapter memory when the DD bit is set in pos register 2.
 */
struct fddi_icr_cmd			
{
	ushort 	local_addr;
	ushort 	len3;		/* length of the 3rd data location */
	ushort	hi_addr3;	/* hi 16 bits of the 3rd data location */
	ushort 	lo_addr3;	/* lo 16 bits of the 3rd data location */
	ushort 	len2;		/* length of the 2nd data location */
	ushort	hi_addr2;	/* hi 16 bits of the 2nd data location */
	ushort 	lo_addr2;	/* lo 16 bits of the 2nd data location */
	ushort 	len1;		/* length of the 1st data location */
	ushort	hi_addr1;	/* hi 16 bits of the 1st data location */
	ushort 	lo_addr1;	/* lo 16 bits of the 1st data location */
	ushort	cmd;		/* holds the icr command to the card */
				/* when this field is written the card is */
				/* interrupted to handle the command */
};
typedef struct fddi_icr_cmd fddi_icr_cmd_t;

struct fddi_addrs
{
	uchar 		addr[FDDI_NADR_LENGTH];	/* address */
	uint		cnt;
};
typedef struct fddi_addrs fddi_addrs_t;
	

/* -------------------------------------------------------------------- */
/* ACS control structure 						*/
/* -------------------------------------------------------------------- */
#define FDDI_FR  	(0x1)  	/* flag for the card type : front royal */
#define FDDI_SC		(0x2)  	/* flag for the card type : scarborough */

#define FDDI_FR_FRU1	"81F9003"
#define FDDI_FR_FRU2	"BM_DVT0"
#define FDDI_SC_FRU	"93F0377"
#define FDDI_FC_FRU	"93F0379"
#define FDDI_FN_LEN	(7)

struct fddi_acs_ctl_tag
{
	dev_t		devno;		/* our devno */
	lock_t		acslock;	/* This device's lock.  Used for 
					 * serialization of ioctls, opens, 
					 * closes
					 */
	uchar 		first_open;	/* This is the first time thru the open
					 * entry point for this acs
					 */
	int		open_cnt;	/* # of channels opened for this adap */
	int		mode;		/* device name ext char (i.e. "D") */
	int		card_type;	/* flag holding the type of card */

	fddi_open_t 	*p_netids[FDDI_MAX_NETIDS]; 
				/* netid table. consists of ptrs to the
				 * open elements that started the netid.
				 * The netid divided by 2 is the index
				 * into the netid table.  All odd netids
				 * are invalid.  Valid netids are
				 * even numbers between 0 and 254.
				 */

	int		netid_cnt;	/* number of netids in use */
	cio_stat_blk_t 	limbo_blk;	/* CIO_NET_RCVRY_ENTER blk */

	/*
	 * info found in the VPD 
	 */
	uchar		vpd_addr[FDDI_NADR_LENGTH];
	/*
	 * source addresses and station id from VPD or DDS
	 */
	uchar		long_src_addr[FDDI_NADR_LENGTH];
	uchar		short_src_addr[2];
	fddi_addrs_t 	addrs[FDDI_MAX_ADDRS];
	int		addr_index; 	/* used to index through the array
					 * of addrs during the sequential
					 * set addr commands used to restore 
					 * the addrs after limbo.
					 */
};
typedef struct fddi_acs_ctl_tag fddi_acs_ctl_t;


/* -------------------------------------------------------------------- */
/* Device work structure 						*/
/* -------------------------------------------------------------------- */

struct fddi_acs_dev_tag
{
	struct intr	ihs;		/* offlevel handler structure */
	int		state;		/* device state machine. */
	ushort		rop;		/* is the ring op bit set */
	int		dma_channel;	/* dma channel returned by d_init() */
	uint		oflv_events;	/* HSR and other events*/
	uint		oflv_copy;	/* copy used in fddi_oflv */
	ushort		oflv_running;	/* Is oflv currently running? */


	/* ----------------------------------------------------	*/
	/*		Command Control Variables		*/
	/* ----------------------------------------------------	*/

	fddi_cmd_t	pri_blk;	/* priority command block */
	fddi_cmd_t	cmd_blk;	/* regular command block */
	fddi_cmd_t	*p_cmd_prog;	/* command in progress */
	uint		pri_que;	/* commands in the priority queue */
	struct watchdog cmd_wdt;	/* watchdog timer for command */

	/* ----------------------------------------------------	*/
	/*		Deactivation/Activation Variables	*/
	/* ----------------------------------------------------	*/
	struct watchdog close_wdt;	/* timer to wait for tx completes */
	struct watchdog limbo_wdt;	/* timer to start activation again */
	int		limbo_to;	/* disables hsr reads after the reset */
	int		close_event;	/* sleep call */
	int		limbo_event;	/* sleep call */

	uint 		smt_event_mask;	/* SMT Event Mask word. Corresponds 
					 * to the high and low SMT event words
					 * in the link statistics
					 */

	uint 		smt_error_mask;	/* SMT Error Mask word. Corresponds 
					 * to the high and low SMT error words
					 * in the link statistics
					 */

	ushort 		smt_control;	/* SMT control Word. It is used in 
					 * the Connect cmd to the adapter.
					 */
	ushort		attach_class;	/* Attachment class */

	uchar 		tx_class_port_a;	/* Transmitter class for port A
						 * and B 
						 */
	uchar 		tx_class_port_b;	


	/* ----------------------------------------------------	*/
	/*	FDDI_DWNLD and FDDI_MEM_ACC ioctl vars		*/
	/* ----------------------------------------------------	*/
	struct watchdog	dnld_wdt;	/* download watch dog timer */
	int		ioctl_event;	/* download event */
	int		ioctl_status;	/* download status */
 	fddi_hcr_cmd_t	hcr_cmd;

	fddi_icr_cmd_t	icr_cmd;
	uchar		*p_ubuf[3];	/* ptrs to the user buffers */
	uint		l_kbuf[3];	/* length of the kernel buffers */
	uint		l_ubuf[3];	/* length of the user buffers */
	uchar		*p_kbuf[3];	/* ptrs to xmalloc buffers */
	uint		p_d_kbuf[3];	/* DMA address space for diagnostics */

	ulong		dma_status;	/* status of the DMA operation */
	struct xmem	dma_xmd;	/* cross memory descriptor for
					 * the kernel buffers used in the
					 * FDDI_MEM_ACC and FDDI_DWNLD ioctl
					 */

	uchar		pos0;		/* POS register 0 */
	uchar		pos1;		/* POS register 1 */
	uchar		pos2;		/* POS register 2 */
	uchar		pos3;		/* POS register 3 */
	uchar		pos4;		/* POS register 4 */
	uchar		pos5;		/* POS register 5 */
	uchar		pos6;		/* POS register 6 */
	uchar		pos7;		/* POS register 7 */
	int		mcerr;		/* Last MC error code */
	int		iox;		/* Last PIO exception code */
	uchar		piowall;	/* has a fatal PIO error occured */
	uchar		carryover;	/* do we have a carry over error */
	uchar		stest_cover;	/* do we have a stest carry over error*/
	uchar		thresh_rtt;	/* contains the count of rtt events */
					/*  between errlogs */
	uchar		thresh_trc;	/* contains the count of trc events */
					/*  between errlogs */
	uchar		thresh_stuck;	/* contains the count of stuck events */
					/*  between errlogs */
	uchar		thresh_tme;	/* contains the count of tme events */
					/*  between errlogs */
	uchar		thresh_sbf;	/* contains the count of sbf events */
					/*  between errlogs */
	ushort		stestrc[FDDI_STEST_CNT];
};
typedef struct fddi_acs_dev_tag fddi_acs_dev_t;


/* -------------------------------------------------------------------- */
/*	Transmit Control Variables					*/
/* -------------------------------------------------------------------- */

struct fddi_acs_tx_tag
{

	/* 
	 * The Host TX Descriptors:
	 *	one for each adapter descriptor
	 *	used for all communication to adapter concerning tx's
	 */
	fddi_tx_t	desc [FDDI_MAX_TX_DESC];
	short		in_use;		/* number of tx desc set for tx */
	uchar		needed;		/* user needs to know when ok to tx */
	uchar		nxt_req;	/* index where next request goes */
	uchar		nxt_cmplt;	/* index where next complete will be */
	uint		p_d_base;	/* dma base address for tx */
	uint		p_d_sf;		/* dma base address for smallframes */
	char		*p_sf_cache;	/* address for smallframes */
	int		event;		/* sleep here when queue is full */
	struct xmem	xmd;		/* x memory descriptor for tx */
	struct watchdog	wdt;		/* tx watchdog timer structure */
	fddi_tx_que_t	*p_tx_que;	/* pointer to the tx que in memory */
	uint		tq_in;		/* index to the next insertion to tq*/
	uint		tq_out;		/* index to the next to tx on tq */
	char		tq_cnt;		/* number of mbuf currently in que */
};
typedef struct fddi_acs_tx_tag fddi_acs_tx_t;


/* -------------------------------------------------------------------- */
/*	Receive Control Variables					*/
/* -------------------------------------------------------------------- */

/*
 * Host RECEIVE descriptor
 */
struct	fddi_rcv
{
	short		offset;		/* offset of desc on adapter */
	char		*p_buf;		/* ptr to the frame (pinned heap) */
	char		*p_d_addr;	/* dma region address for this desc */
};
typedef struct fddi_rcv fddi_rcv_t;

/*
 * The 'fddi_rcv_rearm_t' is a WORM (write once read many) structure. Written
 *	one time at config time (when the rdto value is known) and
 *	read each time a descriptor is rearmed.
 */
struct rcv_rearm_tag
{
	ushort	cnt;
	ushort	ctl;
	ushort	stat;
};
typedef struct rcv_rearm_tag fddi_rcv_rearm_t;

struct fddi_acs_rcv_tag
{
	/*
	 * Host RCV descriptors
	 */
	fddi_rcv_t		desc [FDDI_MAX_RX_DESC];
	uchar			rcvd;	/* index where next rcv will be */
	fddi_rcv_rearm_t	arm_val;/* rearm values for descriptor */
	struct xmem		xmd;	/* for malloc memory */
	uint			l_adj_buf; /* rounded cache buffer length */
	char			*p_rcv_cache;	/* address for smallframes */
};
typedef struct fddi_acs_rcv_tag fddi_acs_rcv_t;


/* -------------------------------------------------------------------- */
/* Adapter control structure 						*/
/* -------------------------------------------------------------------- */
struct acs_tag 
{
	struct intr		ihs;	/* interrupt handler structure */
	fddi_acs_tx_t		tx;	/* tx control area */
	fddi_query_stats_t	ras;	/* statistics */
	fddi_acs_rcv_t		rcv;	/* rcv control area */
	fddi_acs_dev_t		dev;	/* device control area */
	fddi_acs_ctl_t		ctl;	/* ACS control area */
	fddi_dds_t		dds;	/* Define Device Structure via config */
	fddi_vpd_t		vpd;	/* Vital Product Data */
};
typedef struct acs_tag fddi_acs_t;

/* -------------------------------------------------------------------- */
/* 			FDDI Fixed storage area				*/
/* -------------------------------------------------------------------- */
/*
 * The open table anchor (fddi_ctl.p_open_tab) is a pointer to an array
 *	of pointers to open structures. The array of pointers or table grows 
 * 	dynamically as opens are requested. The initial size of the table is 
 *	ZERO so the first open will cause the open table to grow to 
 *	FDDI_OPEN_TAB_SIZE. From then on the table will grow by the same 
 *	increment as more entries are needed.
 */
#define FDDI_OPEN_TAB_SIZE	32	/* default size of open table */

struct fddi_ctl
{
	uchar		initialized;		/* DD initialized? */
	uchar		first_open;		/* first open for driver */
	lock_t		fddilock;		/* the dd's global lock */
	int		acs_cnt;		/* # of acs's we have */
	int		open_cnt;		/* # of opens we have */
	chan_t		channels;		/* channel to assign in mpx */
	int		open_tab_size;		/* current size of open tab */
	fddi_open_t 	**p_open_tab;		/* ptr to open table */
	fddi_acs_t 	*p_acs[FDDI_MAX_MINOR];	/* acs ptr for each adap */
};
typedef struct fddi_ctl fddi_ctl_t;

#ifdef FDDI_DEBUG
#define FDDI_TRACE_SIZE		(500*4) /* max number of trace table entries */
#else
#define FDDI_TRACE_SIZE		(32*4) /* max number of trace table entries */
#endif

struct fddi_trace_tag
{
	int	next;	/* next index hole to put trace data in the table */
	int	res1;
	int	res2;
	int	res3;
	ulong	table[FDDI_TRACE_SIZE];
};
typedef struct fddi_trace_tag fddi_trace_t;

/* 
 * NB:
 *	The get trace structure and ioctl are for
 *	Device driver and adapter ucode DEBUG purposes
 *	only.
 */
struct	fddi_sif_reg_tag
{
	ushort	hsr; 	/* host status register */
	ushort	hcr; 	/* host command register */
	ushort	ns1;	/* node processor status register 1 */
	ushort	ns2;	/* node processor status register 2 */
	ushort	hmr;	/* HSR mask register */
	ushort	nm1;	/* NS1 mask register */
	ushort	nm2;	/* NS2 mask register */
	ushort	acl;	/* Alisa control register */
};
typedef struct fddi_sif_reg_tag fddi_sif_t;

struct fddi_get_trace_tag
{
	ulong		status;
	fddi_acs_t	*p_acs;
	fddi_trace_t	*p_ddtrace;
	ushort		*p_sram;	/* ptr to shared RAM */
	uint		l_sram;
	fddi_sif_t	*p_regs;
	fddi_icr_cmd_t	*p_icrs;
	fddi_mem_acc_t	*p_amd;
	fddi_mem_acc_t	*p_sky;
	fddi_mem_acc_t	*p_dstore;
	uchar		pos[8];
};
typedef struct fddi_get_trace_tag fddi_get_trace_t;

/*
 * structure for ioctl(FDDI_REG_ACC)
 */
#define FDDI_WRITE_OP	(1)
#define FDDI_READ_OP	(2)

typedef struct {
   ulong status;
   ushort opcode;
   ushort io_reg;
   ushort io_val;
   ulong  option;          /* reserved for diagnostics                       */
} fddi_reg_acc_t;

/*
 *  structure and defines for null sap response 
 */

typedef struct 
{
        uchar   rsap;                           /* dest SAP                  */
        uchar   lsap;                           /* ssap                      */
        uchar   ctl;                            /* Control Field             */
        uchar   data[1];                        /* data or xid               */
} fddi_test_data_t;

#define UFMT_PF_BIT             0x10;
#define RESP_ON_BIT             0x01;
#define ROUTE_LEN_MASK  0x1f
#define ROUTE_DIR_BIT   0x80
#define ROUTE_PRESENT_BIT 0x80          /* high bit of source addr   */
#define SINGLE_ROUTE_BCAST 0xc0
#define ROUTE_BCAST_MASK   0xc0
#define ALL_ROUTE_BCAST  0x80
#define ALL_ROUTE_CTL1  0x82
#define CONTROL_MASK   0xef
#define XID_FRAME       0xaf
#define TEST_FRAME      0xe3
#define XID_TEST_AC     0x01
#define XID_TEST_FC     0x50

#define FDDI_REG_ACC	  	(FDDI_IOCTL | 0x0d04) /* access SIF regs */
#define FDDI_GET_TRACE		(FDDI_IOCTL | 0xdddd)
#define FDDI_ISSUE_RCV_CMD	(FDDI_IOCTL | 0xffff)

#endif /* endif ! _H_FDDITYPES */
