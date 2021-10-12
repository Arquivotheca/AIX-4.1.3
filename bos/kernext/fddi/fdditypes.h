/* @(#)80       1.11  src/bos/kernext/fddi/fdditypes.h, sysxfddi, bos411, 9428A410j 3/30/94 15:38:10 */
/*
 *   COMPONENT_NAME: SYSXFDDI
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_FDDITYPES
#define _H_FDDITYPES

/* -------------------------------------------------------------------- */
/* Define Device Structure                                		*/
/* -------------------------------------------------------------------- */
#define FDDI_USR_DATA_LEN	(32)    /* length of the usr data */
#define FDDI_PASSWD_SZ		(8)	/* 8 byte PMF password */

struct fddi_dds
{
	int 	bus_type;       /* the bus type */
	int 	bus_id;         /* the bus id */
	int 	bus_intr_lvl;   /* the interrupt level */
	int 	intr_priority;  /* for use with i_init */
	uint 	slot;           /* card slot number of primary card */
	uchar 	*bus_io_addr;   /* PIO bus address for IO */
	uchar 	*bus_mem_addr;  /* PIO bus address for MEMORY */
	uint 	dma_lvl;        /* DMA arbitration level */
	uint 	dma_base_addr;  /* DMA base address */
	uint 	dma_length; 	/* length of DMA address space */
	uchar	lname[ERR_NAMESIZE];	/* device logical name (i.e. fddi0) */
	uchar	alias[ERR_NAMESIZE];	/* alias to the device (i.e. fi0) */

	uchar	use_alt_addr;	/* TRUE => use the following
					 * MAC SMT addr otherwise get from
					 * the VPD.
					 */
	/* alternate MAC SMT addr */
	uchar	alt_addr[CFDDI_NADR_LENGTH];

	uint 	tvx;		/* value of the tvx command in activation*/
	uint 	t_req;		/* value of the t_req command in activation*/

	uchar	pmf_passwd[FDDI_PASSWD_SZ];	/* PMF password */

	

	uchar	user_data[FDDI_USR_DATA_LEN];	/* user data */			
	uint	tx_que_sz;

};
typedef struct fddi_dds fddi_dds_t; 

/* -------------------------------------------------------------------- */
/*  FDDI VPD structure and status codes                           	*/
/* -------------------------------------------------------------------- */

#define FDDI_VPD_VALID      0x00    /* VPD obtained is valid */
#define FDDI_VPD_NOT_READ   0x01    /* VPD has not been read from adapter */
#define FDDI_VPD_INVALID    0x02    /* VPD obtained is invalid */
#define FDDI_VPD_LENGTH     257    /* VPD length for Primary card in bytes */
#define FDDI_XCVPD_LENGTH   257    /* VPD length for Xtender card in bytes */

struct fddi_vpd
{
	ulong	status; 		/* status of VPD */
	uchar 	vpd[FDDI_VPD_LENGTH];	/* VPD */
	ulong 	l_vpd; 			/* length of VPD */
	ulong	xc_status;		/* status of Extender card VPD */
	uchar	xcvpd[FDDI_XCVPD_LENGTH]; /* VPD of Xtender card */
	ulong	l_xcvpd;		/* length of Extender card VPD */
};
typedef struct fddi_vpd fddi_vpd_t;

struct fddi_dnld
{
	caddr_t	p_mcode;		/* pointer to the microcode */
	uint	l_mcode;		/* length of the microcode */
};
typedef struct fddi_dnld fddi_dnld_t;

/* 
 * This is the command structure used  to download microcode 
 * It is valid in the shared memory portion of 
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

/*
 * The fddi_cmd structure is used to keep track of an HCR command sent to the 
 * card.  The structure remembers the cmd_code (the HCR command) and the CPB
 * (command parameter block), the ctl/stat fields to control the command and 
 * the function to call for cmd specific completion handling.
 */
struct	fddi_cmd
{
	short	cmd_code;		/* the command code */
	short	ctl;			/* control flags */
	uint	stat;			/* completion status of the command */
	short	cpb_len;		/* len of cpb (in words) */
	ushort	cpb[FDDI_CPB_SIZE];	/* command parameter block */
	int	(*cmplt)();		/* specific func called on completion */
	uint	pri;			/* indicates priority cmd required */
};
typedef	struct fddi_cmd fddi_cmd_t;


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
struct fddi_tx_desc
{
	fddi_adap_t 	adap;		/* the mirror of the adapt desc */
	short		offset;		/* offset of desc on adapter */
	struct mbuf	*p_mbuf;	/* ptr to the frame (chain of mbufs) */
	caddr_t 	p_dump_buf;	/* pointer to the mbufs saved for dump*/
	uint		dump_len;	/* length of the dump buffer */
	uint		p_d_addr;	/* dma region address for this desc */
	uint		p_d_sf;		/* dma region for smallframe */
	char		*p_sf;		/* 'smallframe' address */
	short		eof_jump;	/* relative idx to EOF from SOF desc */
}; 
typedef	struct fddi_tx_desc 	fddi_tx_desc_t;

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

typedef char fddi_addr_t[CFDDI_NADR_LENGTH];

/* -------------------------------------------------------------------- */
/*	ACS Transmit Control Variables					*/
/* -------------------------------------------------------------------- */

struct fddi_acs_tx
{

	Simple_lock	lock; 	/* The lock for the tx path */

	/* 
	 * The Host TX Descriptors:
	 *	one for each adapter descriptor
	 *	used for all communication to adapter concerning tx's
	 */
	fddi_tx_desc_t	desc [FDDI_MAX_TX_DESC];

	ushort		hdw_in_use;	/* number of tx desc set for tx */
	uchar		hdw_nxt_req;	/* index where next request goes */
	uchar		hdw_nxt_cmplt;	/* index where next complete will be */
	uint		p_d_base;	/* dma base address for tx */
	uint		p_d_sf;		/* dma base address for smallframes */
	char		*p_sf_cache;	/* address for smallframes */
	struct xmem	xmd;		/* x memory descriptor for tx */
	struct watchdog	wdt;		/* tx watchdog timer structure */
	struct mbuf	*p_sw_que;	/* pointer to the sw queue */
	ushort		sw_in_use;	/* number of mbufs on sw queue */
};
typedef struct fddi_acs_tx fddi_acs_tx_t;

/* -------------------------------------------------------------------- */
/*	ACS Receive Control Variables					*/
/* -------------------------------------------------------------------- */

/*
 * Host RECEIVE descriptor
 */
struct	fddi_rx_desc
{
	short		offset;		/* offset of desc on adapter */
	char		*p_buf;		/* ptr to the frame (pinned heap) */
	char		*p_d_addr;	/* dma region address for this desc */
};
typedef struct fddi_rx_desc fddi_rx_desc_t;

/*
 * The 'fddi_rx_rearm_t' is a WORM (write once read many) structure. Written
 *	one time at config time (when the rdto value is known) and
 *	read each time a descriptor is rearmed.
 */
struct fddi_rx_rearm
{
	ushort	cnt;
	ushort	ctl;
	ushort	stat;
};
typedef struct fddi_rx_rearm fddi_rx_rearm_t;

struct fddi_acs_rx
{
	/*
	 * Host RCV descriptors
	 */
	fddi_rx_desc_t		desc [FDDI_MAX_RX_DESC];
	uchar			nxt_rx;	/* index where next rcv will be */

	fddi_rx_rearm_t		arm_val;/* rearm values for descriptor */
	struct xmem		xmd;	/* for malloc memory */
	uint			l_adj_buf; /* rounded cache buffer length */
	char			*p_rx_cache;	/* address for smallframes */
};
typedef struct fddi_acs_rx fddi_acs_rx_t;

/* -------------------------------------------------------------------- */
/*	Adapter's link statistic structures				*/
/* -------------------------------------------------------------------- */
struct	fddi_adap_links
{
	ushort		smt_error_lo;	/* smt error */
	ushort		smt_error_hi;	/* smt error */
	ushort		smt_event_lo;	/* smt event */
	ushort		smt_event_hi;	/* smt event */
	ushort		cpv;		/* connection policy violation */
	ushort		port_event;	/* port event word */
	ushort		setcount_lo;	/* set count used in command protocol */
	ushort		setcount_hi;	/* set count used in command protocol */
	ushort		aci_code;	/* adapter check interrupt id-code */
	ushort		pframe_cnt;	/* purge frame counter */
	ushort 		ecm_sm;         /* ecm state machine */
	ushort 		pcm_a_sm;       /* port a pcm state machine */
	ushort 		pcm_b_sm;       /* port b pcm state machine */
	ushort 		cfm_a_sm;       /* port a cfm state machine */
	ushort 		cfm_b_sm;       /* port b cfm state machine */
	ushort 		cf_sm;          /* cf state machine */
	ushort		mac_cfm_sm;     /* mac cfm state machine */
	ushort 		rmt_sm;         /* rmt state machine */
	ushort 		sba_alloc_lo;   /* sba allocation low */
	ushort 		sba_alloc_hi;   /* sba allocation hi */
	ushort 		tneg_lo;        /* t_neg word low */
	ushort 		tneg_hi;        /* t_new word hi */
	ushort 		payload_lo;     /* sba payload desired word low */
	ushort 		payload_hi;     /* sba payload desired word low */
	ushort 		overhead_lo;    /* sba overhead desired low */
	ushort 		overhead_hi;    /* sba overhead desired hi */
	ushort 		res1[7];       	/* reserved for future use */
	ushort		ucode_ver;	/* microcode version level */
	ushort		res2;		/* reserved for future use */
	ushort		res3;		/* reserved for future use */

};
typedef struct fddi_adap_links fddi_adap_links_t;

/* -------------------------------------------------------------------- */
/*	ACS Device Driver Variables					*/
/* -------------------------------------------------------------------- */


struct fddi_acs_dev
{
	int		state;   	/* This field holds the state of 
					 * the device driver
					 */
	ushort		rop;		/* is the ring op bit set */
	ushort		stuck;		/* are any of the ports stuck */
	int		dma_channel;	/* dma channel returned by d_init() */

	ushort		hsr_events; 	/* This field holds the copy of the HSR
					 * that the fddi_slih routine uses.
					 */

	/* -------------------------------------------- */
	/*  The command sub section 		 	*/
	/* -------------------------------------------- */

	fddi_cmd_t	pri_blk;	/* priority command block */
	fddi_cmd_t	cmd_blk;	/* regular command block */
	fddi_cmd_t	*p_cmd_prog;	/* command in progress */
        uint            pri_que;        /* commands in the priority queue */


	struct watchdog cmd_wdt;	/* watchdog timer for command */
	int		cmd_status; 	/* results of a command */
	int		cmd_event;	/* sleep call : wait for cmd cmplt */
	int		cmd_wait_event;	/* sleep call : wait for cmplt of top */
					/* 	half command */
	
	/* -------------------------------------------- */
	/*   link statistics subsection		 	*/
	/* -------------------------------------------- */

	fddi_ndd_stats_t	ls_buf;

	/* -------------------------------------------- */
	/*  The lock sub section 		 	*/
	/* -------------------------------------------- */

	Simple_lock	cmd_lock; 	/* The lock for the command section */
	Simple_lock	slih_lock; 	/* The lock for the slih routine */
	
	/* -------------------------------------------- */
	/*  The limbo sub section 		 	*/
	/* -------------------------------------------- */

	struct watchdog limbo_wdt;	/* timer to start activation again */

	/* -------------------------------------------- */
	/*  The download sub section 		 	*/
	/* -------------------------------------------- */

	fddi_icr_cmd_t	icr;
        uint            p_d_kbuf;    /* DMA address space for diagnostics */

	ulong		dma_status;	/* status of the DMA operation */
	struct xmem	dma_xmd;	/* cross memory descriptor for
					 * the kernel buffers used in the
					 * FDDI_MEM_ACC and FDDI_DWNLD ioctl
					 */
	struct watchdog dnld_wdt;	/* timer for the download process */
	
	/* -------------------------------------------- */
	/* The counter section.  For the smt_control    */
	/*  options.					*/
	/* -------------------------------------------- */

	ushort 		smt_control;	

	uint		multi_cnt;
	uint		prom_cnt;
	uint		bea_cnt;
	uint		smt_cnt;	
	uint		nsa_cnt;
	uint		bf_cnt;

	/* -------------------------------------------- */
	/* Thresholds (prevent excessive error logging) */
	/* -------------------------------------------- */
	int thresh_rtt;
	int thresh_trc;
	int thresh_sbf;

	/* -------------------------------------------- */
	/*  Misc 		 		 	*/
	/* -------------------------------------------- */

	uchar		pos[8];		/* POS register 0 */
	int		mcerr;		/* Last MC error code */
	int		pio_rc;		/* Set if there has been a pio error */
	int		iox;		/* Last PIO exception code */
	time_t		stime;		/* The starting time (lbolt) */
	ushort		stest[FDDI_STEST_CNT];
	fddi_addr_t	vpd_addr;

	uint 		smt_event_mask;
	uint 		smt_error_mask;
	ushort		addr_index;
	ushort		attach_class;
	int		card_type;
};
typedef struct fddi_acs_dev fddi_acs_dev_t;

/* -------------------------------------------------------------------- */
/*	ACS Address section						*/
/* -------------------------------------------------------------------- */
struct fddi_addr_elem
{
	fddi_addr_t	addr;
	int		addr_cnt;
};
typedef struct fddi_addr_elem fddi_addr_elem_t;

#define FDDI_MAX_ADDR_BLK (20)
struct fddi_addr_blk
{
	struct fddi_addr_blk *next;
	struct fddi_addr_blk *prev;
	int		blk_cnt;
	fddi_addr_elem_t addrs[FDDI_MAX_ADDR_BLK];
};
typedef struct fddi_addr_blk fddi_addr_blk_t;

struct fddi_acs_addrs
{
	fddi_addr_t	src_addr;		/* Holds the card's source 
						 * address 
						 */

						/* Holds the group addresses set
						 * on the adapter 
						 */
	fddi_addr_elem_t 	hdw_addrs[FDDI_MAX_HDW_ADDRS];  

	int 		hdw_addr_cnt;		/* Holds the count of addresses
						 * set on the adapter
						 */
	fddi_addr_blk_t	*sw_addrs;	/* Holds the group addresses set
						 * but unable to fit on the 
						 * adapter
						 */
	int		sw_addr_cnt;		/* Holds the count of addresses
						 * on the software list of
						 * addresses set
						 */
};
typedef struct fddi_acs_addrs fddi_acs_addrs_t;
	

/* -------------------------------------------------------------------- */
/* Adapter control structure 						*/
/* -------------------------------------------------------------------- */
struct fddi_acs 
{
	ndd_t			ndd; 	/* The copy of the ndd structure for
					 * this adapter 
					 */
	struct intr		ihs;	/* interrupt handler structure */
	fddi_dds_t		dds;	/* Define Device Structure via config */
	fddi_vpd_t		vpd;	/* Vital Product Data */
	fddi_acs_tx_t		tx;	/* tx control area */
	fddi_acs_rx_t		rx;	/* rx control area */
	fddi_spec_stats_t	ls;	/* statistics */
	fddi_acs_dev_t		dev;	/* device control area */
	fddi_acs_addrs_t	addrs;  /* address area */
};
typedef struct fddi_acs fddi_acs_t;

/* -------------------------------------------------------------------- */
/* 			FDDI Fixed storage area				*/
/* -------------------------------------------------------------------- */

#ifdef FDDI_DEBUG
#define FDDI_TRACE_SIZE		(500*4) /* max number of trace table entries */
#else
#define FDDI_TRACE_SIZE		(32*4) /* max number of trace table entries */
#endif

struct fddi_trace
{
	int 	res[3];
	int	next;	/* next index hole to put trace data in the table */
	int	res1;
	int	res2;
	int	res3;
	ulong	table[FDDI_TRACE_SIZE];
};
typedef struct fddi_trace fddi_trace_t;

#define FDDI_MAX_ACS 	(32)		/* Max number of acs's allowed to 
					 * configure 
					 */
struct fddi_tbl
{
	lock_t		table_lock;		/* the dd's global lock */
	int		acs_cnt;		/* # of acs's we have */
	int		open_cnt; 		/* Count of active opens */
	fddi_acs_t 	*p_acs[FDDI_MAX_ACS];	/* acs ptr for each adap */
	Simple_lock	trace_lock;		/* Locks internal trace table*/
	fddi_trace_t	trace;			/* Internal trace table */
};
typedef struct fddi_tbl fddi_tbl_t;

struct fddi_adap_dump
{
	char		pos[8];			/* holds the pos registers */
	short		hsr;			/* Host Status Register */
	short 		hcr;			/* Host Command Register */
	short 		ns1;			/* Node Processor Status Reg 1*/
	short 		ns2;			/* Node Processor Status Reg 2*/
	short		hmr;			/* Host Status Reg Mask */
	short		nm1;			/* ns1 mask */
	short		nm2;			/* ns2 mask */
	short		acr;			/* Alisa Control Register */
	
	fddi_adap_t	tx[FDDI_MAX_TX_DESC];	/* Transmit Descriptors */
	fddi_adap_t	rx[FDDI_MAX_RX_DESC];	/* Receive Descriptors */
};
typedef struct fddi_adap_dump fddi_adap_dump_t;

/* 
 * NB:
 *	The get trace structure and ioctl are for
 *	Device driver and adapter ucode DEBUG purposes
 *	only.
 */
struct	fddi_sif_reg
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
typedef struct fddi_sif_reg fddi_sif_t;


struct fddi_mem_acc
{
	ulong 	status;          /* returned status code */
	ushort 	opcode;          /* Read or Write Appropiate Data Buffer */
	uint 	ram_offset;      /* RAM Offset */
	uint 	num_transfer;    /* Number of Transfer Buffers */
	uchar	*buffer_1;       /* Data Buffer_1 Pointer */
	uint 	buff_len1;       /* Length of Data Buffer 1 */
	uchar 	*buffer_2;       /* Data Buffer_2 Pointer */
	uint 	buff_len2;       /* Length of Data Buffer 2 */
	uchar 	*buffer_3;       /* Data Buffer_3 Pointer */
	uint 	buff_len3;       /* Length of Data Buffer 3 */
};


struct fddi_get_trace
{
	ulong		status;
	uchar		sram[FDDI_SRAM_SIZE];	/* ptr to shared RAM */
	fddi_sif_t	regs;
	fddi_icr_cmd_t	icrs;
	uchar		amd[FDDI_AMDMEM_SIZE];
	uchar		sky[FDDI_SKYMEM_SIZE];
	uchar		dstore[FDDI_DATAMEM_SIZE];
	uchar		pos[8];
};
typedef struct fddi_get_trace fddi_get_trace_t;

/* -------------------------------------------------------------------- */
/*      Structures for Error Logging					*/
/* -------------------------------------------------------------------- */

struct fddi_errlog
{
	struct err_rec0	errhead;
	char		file[32];	/* file name of error */
	uchar		pos[8];		/* current POS register settings */
	uchar		src_addr[CFDDI_NADR_LENGTH];
	ushort		attach_class;	/* Attachment Class */
	uint		mcerr;		/* Last MC error */
	uint		iox;		/* Set to the last PIO error */
	uint		pio_rc;		/* Set if there has been a fatal pio */
					/* exception */

	fddi_spec_stats_t ls;		/* Last snapshot of link statistics */
	ushort		stest[11];	/* adapter self tests results */
	ushort		reserved;	/* reserved */


	uint		state;		
	uint		status1; 	/* The status fields hold three words */
	uint		status2;	/*  of local driver information */
	uint		status3;
};
typedef struct fddi_errlog fddi_errlog_t;

#endif /* endif ! _H_FDDITYPES */
