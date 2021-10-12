/* @(#)99	1.18  src/bos/kernext/tok/toktypes.h, sysxtok, bos41B, 412_41B_sync 12/19/94 08:47:28 */
/*
 *   COMPONENT_NAME: SYSXTOK
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

#ifndef _H_TOKTYPES
#define _H_TOKTYPES

/*-----------------------------------------------------------------*/
/*                     ACA control #defines                        */
/*-----------------------------------------------------------------*/
#define NTCW       63             /* # of available TCWs */

#define RECV_AREA_OFFSET  0       /*
				   * Offset where the receive
				   * buffers bus memory addresses
				   * will start.  This is needed for
				   * the call to reg_init() during
				   * receive setup.
				   */

#define TX_AREA_OFFSET   (RCV_CHAIN_SIZE << DMA_L2PSIZE)
                                  /*
                                   *   Offset where the buffers for
                                   *   transmission bus memory addresses
                                   *   will start.  This is needed for the
                                   *   call to reg_init() during TX setup.
                                   */

#define ACA_SIZE           0x40   /*  Adapter Control Area Size
                                   *  It can hold 0x40 entries that
                                   *  are each 0x40 bytes in size.
                                   *  The ACA takes up 1 TCW.
                                   */
                                   /* SCB location in ACA */
#define ACA_SCB_BASE       (0x00 * IOCC_CACHE_SIZE)

                                   /* SSB location in ACA */
#define ACA_SSB_BASE       (0x01 * IOCC_CACHE_SIZE)

                                   /* Product ID location in ACA */
#define ACA_PROD_ID_BASE   (0x02 * IOCC_CACHE_SIZE)

                                   /* Adapter Error Log location in ACA */
#define ACA_ADAP_ERR_LOG_BASE  (0x03 * IOCC_CACHE_SIZE)

                                   /* Ring Information location in ACA */
#define ACA_RING_INFO_BASE     (0x04 * IOCC_CACHE_SIZE)

                                   /* Adapter addresses location in ACA */
#define ACA_ADAP_ADDR_BASE     (0x06 * IOCC_CACHE_SIZE)

                                   /* Adapter Open Parameter block */
#define ACA_OPEN_BLOCK_BASE    (0x07 * IOCC_CACHE_SIZE)

                                   /* Receive Chain Start location in ACA */
#define ACA_RCV_CHAIN_BASE     (0x08 * IOCC_CACHE_SIZE)

                                   /* Transmit Chain Start location in ACA */
#define ACA_TX_CHAIN_BASE      (ACA_RCV_CHAIN_BASE +   \
                                    (RCV_CHAIN_SIZE * IOCC_CACHE_SIZE) )


#define P_A_UCODE_LEVEL        0x0202     /*
                                           *   Location of the pointer
                                           *   to the microcode level
                                           */

#define P_A_ADDRS              0x0204     /*
                                           *   Location of the pointer
                                           *   to the adapter addresses
                                           */

#define P_A_PARMS              0x0206     /*
                                           *   Location of the pointer
                                           *   to the adapter parameters
                                           */

struct xmit_des {
    int			io_addr;	/* I/O (DMA) addr of data buffer */
    char		*sys_addr;	/* system address of data buffer */
    int			count;		/* # of data bytes this buffer */
};
typedef struct xmit_des xmit_des_t;

struct xmit_elem {			/* a transmit queue element	*/
    struct mbuf		*mbufp;		/* pointer to first mbuf */
    int			in_use;		/* queue element in use flag */
    int			tx_ds_num;	/* index of first xmit buffer */
    int			num_lists;	/* number of xmit lists required */
    int			num_bufs;	/* number of xmit buffers required */
};
typedef struct xmit_elem xmt_elem_t;

typedef struct {       /* device-specific que element */
    ushort   cmd;            /* CMD processed by adapter */
    ushort   stat0;          /* status 0 */
    ushort   stat1;          /* status 1 */
    ushort   stat2;          /* status 2 */
} intr_elem_t;

/*
 *  pass2_regs_t is a template for the Systems Interface register set
 *  on the token ring adapter
 */

typedef struct PASS2_REGS
{
    ushort      data;
    ushort      data_autoincr;
    ushort      addr;
    ushort      cmd;
    uchar       enable;
    uchar       reset;
    uchar       imask_enable;
    uchar       imask_disable;
    ushort      timer;
    ushort      status;
} pass2_regs_t;

/* SSB structure Template */
typedef struct SSB
{
    ushort       c_type;         /* Command type */
    ushort       stat0;          /* status 0 */
    ushort       stat1;          /* status 1 */
    ushort       stat2;          /* status 2 */
} t_ssb;  /* end struct SSB */

/* SCB structure template */
typedef struct SCB
{
    ushort  adap_cmd;            /* Adapter Command */
    ushort  addr_field1;         /* Address Field 1 */
    ushort  addr_field2;         /* Address Field 2 */
} t_scb; /* End struct SCB definition */

typedef struct DWNLD_CCB
{
    ushort      cmd;
    ushort      segment[2];
    ushort      recs;
} t_dwnld_ccb;

typedef struct TIMER_DATA
{
    caddr_t	p_dds;		/* pointer to DDS */
    int		cmd;		/* command being processed */
    uint	run;		/* Is the timer currently running? */
} timer_data_t;

typedef struct ADAP_CHECK_BLOCK
{
    ushort  code;
    ushort  parm0;
    ushort  parm1;
    ushort  parm2;
} adap_check_blk_t;

/*
 *  Transmit List type definition
 */
#define TX_LIST_SIZE   32

typedef struct GATHER_BLOCK
{
    ushort       cnt;            /* Amount of data at address to xfer */
    ushort       addr_hi;    /* high-order 2 bytes of address */
    ushort       addr_lo;    /* low-order 2 bytes of address */
}  gather_block_t;


typedef struct TX_LIST
{
    struct TX_LIST	*p_d_fwdptr;    /* Pointer to next in TX chain */
    ushort		tx_cstat;       /* TX CSTAT */
    ushort		frame_size;     /* TX Frame Size */
    gather_block_t	gb[3];          /* Data gather locations */
    xmt_elem_t		*p_tx_elem;     /* pointer to TX element */
    uchar		padd[IOCC_CACHE_SIZE - TX_LIST_SIZE];
} t_tx_list;

/*
 *  Receive List type definition
 */
typedef struct recv_list {
    struct recv_list	*next;          /* pointer to next in chain */
    ushort		status;         /* receive status */
    ushort		frame_size;     /* size of the entire frame */
    ushort		count;          /* bytes in this receive list */
    ushort		addr_hi;        /* high 16 bits of data address */
    ushort		addr_lo;        /* low 16 bits of data address */
    uchar		unused[IOCC_CACHE_SIZE - 14];
} recv_list_t;

/*
 *      Token-Ring Adapter Open Options Structure
 */
typedef struct TOK_ADAP_OPEN_OPTIONS
{
    ushort	options;            /* Open Options */
    uchar	node_addr[6];       /* Node address */
    uchar	grp_addr[4];        /* Group address */
    uchar	func_addr[4];       /* Functional address */
    ushort	rcv_list_size;      /* RCV List Element size in bytes */
    ushort	xmit_list_size;     /* TX List Element size in bytes */
    short	buf_size;           /* Adapter Buffer Size */
    short	res1;               /* Extnl RAM Start Addr. - not used */
    short	res2;               /* Extnl RAM End Addr. - not used */
    char	xmit_buf_min_cnt;   /* Minimum # of Adap. Buffers to */
                                    /* reserve for transmission */
    char	xmit_buf_max_cnt;   /* Maximum # of Adapter buffers to */
                                    /* use for transmit data */
    ushort	prod_id_addr1;      /* High-order 2 bytes of product ID */
                                    /* address */
    ushort	prod_id_addr2;      /* Low-order 2 bytes of product ID */
                                    /* address */
} tok_adap_open_options_t;

/*
 *      Token-Ring Adapter Initialization Parameter Structure
 */
typedef struct ADAP_I_PARMS
{
    short	init_options;
    uchar	cmd;
    uchar	xmit;
    uchar	rcv;
    uchar	ring;
    uchar	scb_clear;
    uchar	adap_chk;
    ushort	rcv_burst_size;
    ushort	xmit_burst_size;
    ushort	dma_abort_thresh;
    short	scb_add1;
    short	scb_add2;
    short	ssb_add1;
    short	ssb_add2;
} tok_adap_i_parms_t;

/* -------------------------------------------------------------------- */
/*  Token-Ring information						*/
/* -------------------------------------------------------------------- */

typedef struct RING_INFO            /* Token-Ring Information */
{
   unsigned short  adap_phys_addr[2];      /* Adapter Physical Address */
   uchar	   upstream_node_addr[6];  /* Upstream Node Address */
   unsigned short  upstream_phys_addr[2];  /* Upstream Physical Addr */
   unsigned short  last_poll_addr[3];  /* Last Poll Address */
   unsigned short  author_env;         /* Authorized Environment */
   unsigned short  tx_access_prior;    /* Transmit Access Priority */
   unsigned short  src_class_author;   /* Source Class Authorization */
   unsigned short  last_atten_code;    /* Last Attention Code */
   unsigned short  last_src_addr[3];   /* Last Source Address */
   unsigned short  last_bcon_type;     /* Last Beacon Type */
   unsigned short  last_maj_vector;    /* Last Major Vector */
   unsigned short  ring_status;        /* Ring Status */
   unsigned short  sft_err_time_val;   /* Soft Error Timer Value */
   unsigned short  front_end_time_val; /* Front End Timer Value */
   unsigned short  res1;               /* Reserved */
   unsigned short  monitor_err_code;   /* Monitor Error Code */
   unsigned short  bcon_tx_type;       /* Beacon Transmit Type */
   unsigned short  bcon_rcv_type;      /* Beacon Receive Type */
   unsigned short  frame_corr_save;    /* Frame Correlator Save */
   unsigned short  bcon_station_naun[3];   /* Beaconing Station NAUN */
   unsigned short  res2[2];                /* Reserved */
   unsigned short  bcon_station_phys_addr[2];  /* Beaconing Station */
                                               /* Physical Address */
} tok_ring_info_t;

typedef struct ADAP_ADDR		/* adapter addresses */
{
    ushort		adap_node_addr[3]; /* adapter node address */
    ushort		adap_group_addr[2]; /* adapter group address */
    ushort		adap_funct_addr[2]; /* adapter functional address */
} tok_adap_addr_t;

typedef struct TOK_ERROR_LOG_SENSE_DATA
{
    struct  err_rec0	errhead;
    char		file[32];	/* file name of error */
    uint		rr_entry;       /* Reason for Entering Ring Recovery */
    ulong		limcycle;       /* Count of Network Recovery cycles */
    uint		adap_state;     /* Adapter State */
    uint		limbo;          /* Limbo Mode State */
    uint		footprint;      /* Device handler's footprint */
    int			slot;           /* Adapter's slot number */
    uchar		cfg_pos[8];	/* Adapter POS Register settings */
    uchar		tok_addr[6];	/* Network Address in use */
    ushort		afoot;          /* Last adapter error code */
    uint		mcerr;		/* Last Micro Channel Error */
    uint		iox;		/* Last PIO exception */
    uint		pio_rc;		/* Set if there has been a fatal pio */
					/* exception */
    uint		pio_addr;	/* Set if there has been a fatal pio */
					/* exception */
    adap_check_blk_t	ac_blk;         /* Last Adapter Check code */
    u_long		ring_status;	/* Last Ring Status */
    u_long		wdt_opackets;	/* xmit packet count at WDT start */
    u_long		opackets_lsw;	/* transmit packet count (lsw) */
    u_long		ipackets_lsw;	/* receive packet count (lsw) */
} tok_error_log_data_t;

/* -------------------------------------------------------------------- */
/*      Structures for adapter statistics                               */
/* -------------------------------------------------------------------- */
typedef struct ADAP_ERROR_LOG               /* Error Log Data Area */
{
    unsigned char         line_err_count;
    unsigned char         internal_err_count;
    unsigned char         burst_err_count;
    unsigned char         ari_fci_err_count;
    unsigned char         abort_del_err_count;  /*  Abort Delimeter*/
    unsigned char         res1;                   /*  Reserved */
    unsigned char         lost_frame_err_count;
    unsigned char         rec_cong_err_count;   /* Rec. Congestion */
    unsigned char         frame_cpy_err_count;
    unsigned char         res2;                   /* Reserved */
    unsigned char         token_err_count;
    unsigned char         res3;                   /* Reserved */
    unsigned char         dma_bus_err_count;
    unsigned char         dma_parity_err_count;
} tok_adap_error_log_t;

/* -------------------------------------------------------------------- */
/*         Product ID Information Structure                             */
/* -------------------------------------------------------------------- */

/*
 *  The following Product ID Information is taken from the
 *  Token-Ring Network Architecture Reference manual in
 *  the MAC Frames section.
 */
typedef struct PROD_ID
{
   unsigned char   hardware;      /*
                                   * Bits 0-3 - Reserved
                                   * Bits 4-7 - Product classification
                                   *       0x1:    IBM Hardware
                                   *       0x3:    IBM or non-IBM HW
                                   *       0x4:    IBM software
                                   *       0x9:    Non-IBM hardware
                                   *       0xc:    Non-IBM software
                                   *       0xe:    IBM or non-IBM SW
                                   */

   unsigned char   format_type;   /*
                                   * Format type:
                                   *   0x10 - Product instance is
                                   *       identified by a serial number
                                   *       (that is, IBM plant of manufacture
                                   *       and sequence number) unique by
                                   *       machine type.
                                   *
                                   *   0x11 - Product instance is
                                   *       identified by a serial number
                                   *       unique by machine type and
                                   *       model number.
                                   *
                                   *   0x12 - Product instance is identified
                                   *       by machine type (as in Format 0x10)
                                   *       This format provides the model
                                   *       number not to identify a product
                                   *       instance uniquely, but for
                                   *       additional information only.
                                   */


   unsigned char   machine_type[4];
                                          /*
                                           * Machine type: 4 numeric EBCDIC
                                           * characters
                                           */


   unsigned char   mach_model_num[3];     /*
                                           *  Machine model number: 3 upper
                                           *  case alphanumeric EBCDIC chars
                                           *  for format types 0x11 and 0x12;
                                           *  these bytes are reserved by IBM
                                           *  future use in format type 0x10.
                                           */

   unsigned char   sn_modifier[2];        /*
                                           *  Serial number modifier -
                                           *  IBM plant of manufacture: 2
                                           *  numeric EBCDIC characters.
                                           */

   unsigned char   seq_num[7];            /*
                                           *  Sequence number: 7 upper case
                                           *  alphanumeric EBCDIC characters,
                                           *  right justified with EBCDIC zeros
                                           *  (0xf0) fill on the left.
                                           */


} tok_prod_id_t;

/*---------------------------------------------------------------------------*/
/*                            WORK AREA OF DDS                               */
/*---------------------------------------------------------------------------*/

struct dds_wrk {
	Simple_lock	tx_slock;	/* transmit/control lock  */
	Simple_lock	slih_slock;	/* SLIH lock */

	int		adap_state;     /* current adapter state           */
	uint		limbo;          /* Network Rcvry (limbo) state flag */
	uint		bringup;        /* Initialization state           */
	uint		open_status;    /* Status of the open             */
	int		ndd_stime;	/* start time of the ndd statistics */
	int		dev_stime;	/* start time of the dev statistics */
	int		do_dkmove;	/* d_kmove (1) vs bcopy (0) flag */
    uint		connect_sb_sent; /* NDD_CONNECTED status block sent */
    uint		bcon_sb_sent;	/* Flag for status block sent */
    uint		rr_entry;       /* reason for entering Limbo  */
    uint		rr_errid;       /* error logged on entering limbo */
    ulong		limcycle;       /* Limbo cycle count */
    int			pio_errors;     /* number of PIO errors */
    uint		piox;		/* Last PIO exception (temporary) */
    uint		pio_rc;		/* permanent PIO exception */
    uint		pio_addr;	/* address of permanent PIO error */
    uint		mcerr;		/* Last Micro Channel Error */
    
    int			footprint;	/* audit trail for adapt operations */
    uint		bugout;		/* reason for bugout */
    ushort		afoot;      	/* Adapter failure code footprint */
    ushort		open_fail_code;	/* OPEN Failure code returned by  */
                                        /* the adapter */
    ushort		ring_status;	/* Last Ring Status */
    
    uint		dma_chnl_id;    /* dma channel id...used for all   */
                                        /* d_xxx kernel services routines  */
	int		alloc_size;     /* size of DDS including xmit queue */
    adap_check_blk_t	ac_blk;     	/* Adapter Check code */
    intr_elem_t		limbo_iwe;	/* Limbo INTR Work Element */
                                        /* that triggered entry into */
                                        /* recovery logic */
    tok_adap_error_log_t adap_err_log;  /* adapter error log data */
    tok_ring_info_t	ring_info;      /* adapter ring info */
    tok_adap_addr_t	adap_addr;      /* adapter addresses */
    int			func_addr_ref[31];  /* reference counts for the bits */
					/* in the functional address */
    
    /* ------------------------------------------------------------------ */
    /*		Adapter Control Area control variables			  */
    
    
    ushort		*p_mem_block;  /* pointer to dynamic work block */
    ushort		*p_d_mem_block; /* DMA addr of p_mem_block */
    struct xmem		mem_block_xmd; /* Cross memory descriptor for */
                                       /* the dynamic work block */
    
    t_scb		*p_scb;        /* pointer to the SCB */
    t_scb		*p_d_scb;      /* holds the DMA addr of p_scb */
    
    t_ssb		*p_ssb;        /* pointer to the SSB */
    t_ssb		*p_d_ssb;      /* holds the DMA addr of p_ssb */
    
    tok_prod_id_t	*p_prod_id;    /* pointer to Product ID Information */
    tok_prod_id_t	*p_d_prod_id;  /* holds the DMA addr of p_prod_id */
    
    tok_adap_error_log_t  *p_errlog;   /* pointer to adapter error log */
    tok_adap_error_log_t  *p_d_errlog; /* holds DMA addr of p_errlog */

    tok_ring_info_t	*p_ring_info;   /* pointer to Ring Information */
    tok_ring_info_t	*p_d_ring_info; /* holds DMA addr of ring info */

    tok_adap_addr_t	*p_adap_addr;   /* pointer to adapter addresses */
    tok_adap_addr_t	*p_d_adap_addr; /* holds DMA addr of adapter addr */

	/*
	 *	data structures for transmit
	 */

	/*
	 *	Note: depending on the situation the entire transmit data area 
	 *	may be referred to as the transmit buffer.  At other times an
	 *	individual 2K section of the area is referred to as a transmit
	 *	buffer.
	 *
	 *	The transmit buffer (pointed to by xmit_buf) varies in size
	 *	depending upon ring speed and the size of the transmit queue
	 *	(the size of the transmit queue is a convenient way to define
	 *	a variable size transmit buffer).  At a minimum it is big
	 *	enough to hold 2 packets of maximum size for the ring speed
	 *	plus 2 packets of 2K or less in size.  At a maximum there is
	 *	room for 4 maximum size packets plus 2 packets of 2K or less.
	 *
	 *	The transmit buffer is xmalloc'ed & xmattach'ed at open.
	 *
	 *	Associated with each of the transmit buffers is a transmit
	 *	descriptor.  This descriptor has the buffer @, the buffer
	 *	DMA @, and the amount of data in the buffer.  Each of the
	 *	buffers is d_master'ed.
	 *
	 *	The transmit descriptor table (tx_buf_des) is defined to be
	 *	maximum size and may have zero entries which are not used.
	 */

	char		*xmit_buf;	/* pinned area for transmit data */
	struct xmem	xbuf_xd;	/* xmem descriptor for xmit data */
	int		tx_buf_size;	/* total size of the xmit data area */
	int		tx_buf_count;	/* actual # of the transmit buffers */
	int		tx_buf_use_count; /* # of transmit buffers in use */
	int		tx_buf_next_in;   /* next transmit buffer to use */

	xmit_des_t	tx_buf_des[TX_MAX_BUFFERS]; /* transmit descriptors */

	/*
	 *	There are TX_CHAIN_SIZE transmit lists.  NOTE: there
	 *	may be more transmit lists than transmit buffer entries
	 *	available to use at any given time.  This is to allow the
	 *	transmit lists to be defined at compile time while the
	 *	number of transmit buffer entries is defined at run time.
	 *	Note: there may also be transmit buffer entries available, but
	 *	no transmit lists available to use them.  The transmit lists
	 *	are stored in the ACA with each element on an IOCC cache
	 *	boundary (64 bytes).  They are chained together (which is why
	 *	this is often referred to as the transmit chain).
	 */
	int		tx_list_use_count; /* # of xmit lists in use */
					/* total # of lists is TX_CHAIN_SIZE */
	int		tx_list_next_in;   /* next xmit list to use */

	t_tx_list	*p_d_tx_fwds[TX_CHAIN_SIZE]; /* the DMA address of */
					/* the next element in the chain */
	t_tx_list	*p_tx_head;     /* pointer to head of TX chain */
	t_tx_list	*p_d_tx_head;   /* DMA addr of head of TX chain */
	t_tx_list	*p_tx_tail;     /* pointer to tail of TX chain */
	t_tx_list	*p_d_tx_tail;   /* DMA addr of tail of TX chain */
    
	t_tx_list	*p_tx_1st_update;/* 1st location to start processing */
                                    /* the TX completion interrupts on the */
                                    /* TX chain */
	t_tx_list	*p_d_tx_1st_update;   /* DMA addr of 1st location */
    
	t_tx_list	*p_tx_next_avail;   /* Next available location in TX */
                                            /* chain to put Transmit data */
	t_tx_list	*p_d_tx_next_avail;   /* DMA addr of next location */

	/*
	 *	The transmit queue is an array of a user specified number
	 *	of transmit elements which is indexed using the
	 *	tx_que_next_xxx indices.  It is located immediately following
	 *	the DDS.  When a transmit element is added to the queue, the
	 *	mbuf pointer & number of transmit lists and transmit buffers
	 *	required to transmit this packet are set in the transmit
	 *	element.  When the packet is added to the adapter's transmit
	 *	queue, the index of the first transmit buffer is set.
	 */
	xmt_elem_t	*xmit_queue;
	int		tx_que_next_buf; /* Index to next in SW queue */
					 /* tok_output adds to Q using this */
	int		tx_que_next_in;  /* Index to next for adapter */
	int		tx_que_next_out; /* Index to next to be ACKed by HW */
	int		xmits_queued;	 /* #packets NOT given to adapter */
	int		xmits_adapter;	 /* #packets given to adapter */

	intr_elem_t	tx_iwe;		/* Transmit interrupt work element */
    
	int		issue_tx_cmd;	/* issue new TX command or just */
					/* issue a TX list valid command */
    
    /*---------------------------------------------------------------------*/
    /*                    Data Structures for Receive                      */
    
    intr_elem_t		recv_iwe;      /* Receive INTR work element */
    
    uint		recv_mode;  /* receive running flag */
    int			read_index; /* next read in recv chain */
                                      /* dma addrs of recv chain */
    recv_list_t		*recv_list[ RCV_CHAIN_SIZE ];
                                      /* virt addrs of recv chain */
    recv_list_t		*recv_vadr[ RCV_CHAIN_SIZE ];
    uchar		*recv_addr[ RCV_CHAIN_SIZE ];  /* dma addrs of mbufs */
    struct mbuf		*recv_mbuf[ RCV_CHAIN_SIZE ];  /* array of mbufs */
    
    /*--------------------------------------------------------------------*/
    
    uchar		cfg_pos[8];    /* value of POSn after startup  */
    
    uchar		tok_vpd_addr[6];      /* VPD's burned in address */
    uchar		tok_vpd_rosl[ROS_LEVEL_SIZE]; /* VPD's ROS level */
                                              /* VPD's Microcode level */
    uchar		tok_vpd_mclvl[MICROCODE_LEVEL_SIZE];
    
    /*
     * Adapter Initialization Parameters and
     * Adapter Open Options that will be given to the adapter.
     */
    
    tok_adap_i_parms_t	adap_iparms;

    tok_adap_open_options_t adap_open_opts;
    tok_adap_open_options_t *p_open_opts;   /* addr of open options in ACA */
    tok_adap_open_options_t *p_d_open_opts; /* DMA addr of open opt in ACA */
    
    /*
     * Control blocks for transmit timeout watchdog timer
     */
    int			xmit_wdt_inited;
    int			xmit_wdt_active;
    u_long		xmit_wdt_opackets; /* value of ndd_opackets_lsw */
					   /* when the timer was started */
    
    /*
     * Control blocks for watchdog timer and events which go to sleep
     */
    int			bu_wdt_inited;
    int			bu_wdt_cmd;

    int			event_wait;	/* waiting on one of these events */
#define	FUNCT_WAIT	0x01		/* Functional Address wait flag */
#define	GROUP_WAIT	0x02		/* Group Address wait flag */
#define	ELOG_WAIT	0x04		/* adapter error log wait flag */
#define	READ_ADAP_WAIT	0x08		/* read adapter wait flag */

	/*
	 * the following code is used to return the actual adapter addresses
	 * (read from the adapter) on the NDD_MIB_ADDR command
	 */
#ifdef TOK_ADAP_ADDR
#define	READ_ADAP_ADDR	0x10		/* reading adapter addresses instead */
					/* of ring information */
#endif

    int			funct_event;	/* Functional Addr. sleep/wakeup var */
    int			group_event;	/* Group Addr. sleep/wakeup var */
    int			elog_event;	/* adap error log sleep/wakeup var */
    int			read_adap_event; /* read adapter sleep/wakeup var */
   
    int			command_to_do;  /* need to set funct/group address */
					/* received while in the OPEN_PHASE0 */ 
					/* of limbo or open */

    /*
     *   Adapter bringup loop/retry variables
     *   These variables are used in walking through the adapter
     *   activation sequence.
     */
    uint		reset_spin;
    uint		adap_init_spin;
    int			close_event;
    
    /*
     *   Adapter information variables
     *   These are pointers for information that resides
     *   in adapter memory.
     */
    ushort		p_a_ucode_lvl;
    ushort		p_a_addrs;
    ushort		p_a_parms;

    uint		mask_int;    /*
			              *  Mask/Disable adapter interupts
                                      *  TRUE = the adapter interrupts
				      *          have been disabled
				      *  FALSE = the adapter interrupts
				      *          are enabled.
				      */

    /*
     *  DUMP variables
     */
    int            dump_read_started; /* flag for receive int. processing  */
    int            dump_pri;        /* Dump interrupt level                */
    int            sav_index;       /* Index to start of used rcv lists    */
    int            dump_first_wrt;  /* Flag for transmit processing        */
    recv_list_t   *dump_read_last;  /* Last recv_list for this recv. int.  */

    int		   freeze_dump;     /* freeze dump taken flag		   */
    ushort	   *freeze_data;    /* freeze dump data area		   */
};
typedef struct dds_wrk dds_wrk_t;


/*
 *	this is it -- the entire dds
 */

struct dds {
	struct intr	ihs;		/* interrupt handler control struct */
	struct dds	*next;
	int		seq_number;	/* sequence number for this dds */
	int 		dds_correlator; /* dds to DD ctl structure pointer */
	struct ndd	ndd;		/* ndd structure */
	tr_mon_dds_t	ddi;		/* the ddi as passed to ddconfig */
	tok_vpd_t	vpd;		/* VPD from device */
	struct watchdog	bu_wdt;		/* bringup watchdog timer */
	struct dds	*bu_wdt_p_dds;	/* this MUST immediately follow */
					/* bu_wdt so bringup_timer can get */
					/* the dds address */
	struct watchdog	xmit_wdt;	/* transmit timeout watchdog timer */
	struct dds	*xmit_wdt_p_dds; /* this MUST immediately follow */
					/* xmit_wdt so xmit_timeout can get */
					/* the dds address */
	tok_genstats_t	tokstats;	/* media specific statistics */
	tr_mon_stats_t	devstats;	/* device specific statistics */
	dds_wrk_t	wrk;		/* device specific work area */
};

typedef struct dds dds_t;

/* macros for accessing DDS contents require that the dds pointer be p_dds */

#define IHS p_dds->ihs
#define NDD p_dds->ndd
#define DDI p_dds->ddi
#define VPD p_dds->vpd
#define BUWDT p_dds->bu_wdt
#define XMITWDT p_dds->xmit_wdt
#define TOKSTATS p_dds->tokstats
#define DEVSTATS p_dds->devstats
#define WRK p_dds->wrk

#define TX_LOCK 	WRK.tx_slock
#define SLIH_LOCK 	WRK.slih_slock

/*----------------------------------------------------------------------
 *
 *	this is the global device driver control structure
 * 
 *----------------------------------------------------------------------*/

typedef struct {
	struct cdt_head  header;
	struct cdt_entry entry[MAX_CDT_ELEMS];
} trmon_cdt_t;

typedef volatile struct {
	Simple_lock	trace_slock;	/* lock for the trace */
	ulong		next_entry;	/* index into table for next add  */
	ulong table[TRACE_TABLE_SIZE];  /* ring buffer of trace data      */
} trmon_trace_t;

typedef volatile struct {
	lock_t		mon_cfg_lock;	/* lockl lock for config	*/
	Simple_lock	dd_slock;	/* lock for system CDT registration */
	dds_t		*p_dds_head;	/* dds pointers for each adapter */
	int		num_devs;	/* count of devices configured	*/
	int		num_opens;	/* count of devices opened	*/
	trmon_cdt_t	cdt;		/* device driver dump table	*/
	trmon_trace_t	trace;		/* device driver trace table	*/
} dd_ctrl_t;

#define CFG_LOCK	mon_dd_ctrl.mon_cfg_lock
#define DD_LOCK		mon_dd_ctrl.dd_slock
#define TRACE_LOCK	mon_dd_ctrl.trace.trace_slock
#endif /* ! _H_TOKTYPES */
