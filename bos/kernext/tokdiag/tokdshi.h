/* @(#)33	1.32  src/bos/kernext/tokdiag/tokdshi.h, diagddtok, bos411, 9428A410j 6/25/92 17:10:16 */
/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS: tokdshi.h
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


#ifndef _H_TOKDSHI
#define _H_TOKDSHI

/*****************************************************************************/
   struct sll_elem_tag {        /* used to implement singly-linked list mgmt */
      struct sll_elem_tag *next;     /* all elements must start w this item  */
   };
   typedef volatile struct sll_elem_tag  sll_elem_t;
   typedef sll_elem_t                   *sll_elem_ptr_t;

/*****************************************************************************/
   struct s_link_list_tag {    /* control structure for managing linked lists*/
      int          num_free;   /* number of elements linked on free list     */
      int          num_elem;   /* number of elements linked on active list   */
      int          max_elem;   /* maximum elements allowed (initial num_free)*/
      int          elem_size;  /* the size of one element                    */
      ulong       *hwm_ptr;    /* pointer to high water mark (max queued)    */
      sll_elem_ptr_t area_ptr; /* address of area where elements allocated   */
      sll_elem_ptr_t limt_ptr; /* address just past area of elements         */
      sll_elem_ptr_t free_ptr; /* pointer to first free element (if any)     */
      sll_elem_ptr_t head_ptr; /* pointer to first active element (if any)   */
   };
   typedef volatile struct s_link_list_tag s_link_list_t; /* each que has one*/


/*****************************************************************************/
struct xmit_des {
	ulong		io_addr;	/* io address for DMA		  */
	char		*sys_addr;	/* system address of data buffer  */
};
typedef struct xmit_des xmit_des_t;

struct xmit_elem {			/* a transmit queue element	*/
	struct mbuf	*mbufp;		/* pointer to mbuf with data	*/
	chan_t		chan;     	/* which opener this element is from */
	cio_write_ext_t	wr_ext;   	/* complete extension supplied  */
	short		bytes;		/* number of bytes in packet    */
	char		free;		/* flag to free mbuf		*/
	char		in_use;		/* use flag			*/
	xmit_des_t	*tx_ds;		/* xmit descriptor		*/
};
typedef struct xmit_elem xmt_elem_t;

#define XMITQ_DEC(x) {(x)--; if ((x) < 0) (x) = (short)DDI.xmt_que_size-1;}
#define XMITQ_INC(x) {(x)++; if ((x)==(short)DDI.xmt_que_size) (x) = 0;}
#define XMITQ_FULL 	( \
 ((WRK.tx_list_next_buf+1 == DDI.xmt_que_size) ? 0:WRK.tx_list_next_buf+1)\
 == WRK.tx_list_next_out)

/*****************************************************************************/
   struct rec_elem_tag {              /* a receive queue element (user proc) */
      struct rec_elem_tag  *next;     /* must be first for list management   */
      struct mbuf          *mbufp;    /* pointer to mbuf with data           */
      int                   bytes;    /* total number of bytes of data       */
      cio_read_ext_t        rd_ext;   /* complete extension to be returned   */
   };
   typedef volatile struct rec_elem_tag rec_elem_t; /* each read has one     */

/*****************************************************************************/
   struct sta_elem_tag {              /* a status queue element (user proc)  */
      struct sta_elem_tag  *next;     /* must be first for list management   */
      cio_stat_blk_t        stat_blk; /* the status block to be returned     */
   };
   typedef volatile struct sta_elem_tag sta_elem_t; /* each stat blk has one */

/*****************************************************************************/
   typedef enum {
      DEVICE_NOT_CONN = 0,	/* not connected */
      DEVICE_CONN_IN_PROG,	/* connection in progress */
      DEVICE_CONNECTED,   	/* connected */
      DEVICE_DISC_IN_PROG 	/* disconn in progress */
   } device_state_t;

/*****************************************************************************/
   typedef enum {
      CHAN_AVAIL = 0,                /* ok to allocate (in ddmpx)            */
      CHAN_ALLOCATED,                /* ok to open (in ddopen)               */
      CHAN_OPENED,                   /* ok to read/write/ioctl/select/close  */
      CHAN_CLOSED                    /* ok to deallocate (in ddmpx)          */
   } chan_state_t;

   struct open_elem_tag {
      int           alloc_size;    /* amount of memory for XXX_del_cdt       */
      dev_t         devno;         /* our own devno                          */
      chan_t        chan;          /* our own channel number                 */
      ulong         devflag;       /* flags from open call                   */
      int           xmt_event;     /* event list for e_sleep during write    */

      ulong         open_id;       /* from open extensn of kernel open only  */
      void        (*rec_fn)();     /* address of kproc rec "interrupt" entry */
      void        (*sta_fn)();     /* address of kproc sta "interrupt" entry */
      void        (*xmt_fn)();     /* address of kproc xmt "interrupt" entry */
      int           xmt_fn_needed; /* TRUE if this kernel user needs xmt_fn  */

      s_link_list_t rec_que;       /* rec queue managemt for user open only  */
      s_link_list_t sta_que;       /* sta queue managemt for user open only  */
      int           sta_que_ovrflw;/* the status que overflowed              */
      int           selectreq;     /* flags indicating selnotify requested   */
      int           rec_event;     /* event list for e_sleep during read     */

      /* if user process open, then rec que and sta que are allocated here   */
   };
   typedef volatile struct open_elem_tag open_elem_t; /* each open adds one  */





/*************************************************************************
 *                 Begining of original tokdshi.h                        *
 ************************************************************************/

#define TRACE_HOOKWORD     HKWD_DD_TOKDD   /* define hook for tracing */
#ifdef TOKDEBUG_TRACE
#define TRACE_TABLE_SIZE (500)              /* SAVETRACE table (ulongs)      */
#else
#define TRACE_TABLE_SIZE (60)               /* SAVETRACE table (ulongs)      */
#endif /* DEBUG */
#define DD_NAME_STR        "tokdd"         /* define name for dumping */
#define MAX_GATHERS        TOK_MAX_GATHERS     /* specific to generic */
#define MAX_ADAPTERS       TOK_MAX_ADAPTERS    /* specific to generic */
#define MAX_TX_TCW       (8)    /* Maximum Transmit tcw's  */
#define TX_DUMP_TIMEOUT  1000   /* Dump write timeout value */

#define MAX_CDT         (1)     /* device-specific CDT entries per adapter */

#include <sys/listmgr.h>

#define MAX_OFFL_QUEUED  (10)        /* offl's queued per adap  */
#define MAX_OPENS        (TOK_MAX_OPENS)   /* max opens per device    */

# define MAX_RECV_CHAIN         16

   typedef enum {
      OFFL_WDT = 0,        /* this must be defined for common code */
      OFFL_TIMO,           /* this must be defined for common code */
                           /* add your own additional codes here */
      OFFL_INTR,
      OFFL_START,
      OFFL_ADAP_BRINGUP,
      OFFL_MBUF_TIMER,
      OFLV_IOCTL
   } OFFL_SETTER;

   typedef volatile struct {       /* device-specific que element */
      OFFL_SETTER who_queued;      /* common code uses this identifier */
      unsigned short   int_reason;     /* interrupt reason from adapter */
      unsigned short   cmd;            /* CMD processed by adapter */
      unsigned short   stat0;          /* status 0 */
      unsigned short   stat1;          /* status 1 */
      unsigned short   stat2;          /* status 2 */
   } offl_elem_t;

   typedef enum {                   /* Who set the watchdog timer            */
      WDT_INACTIVE,                 /* WDT is inactive                       */
      WDT_CONNECT,                  /* dsact in progress                     */
      WDT_XMIT,                     /* xmit in progress                      */
      WDT_CLOSE                     /* Adapter close in progress             */
   } WDT_SETTER;

/*
 *  pass2_regs_t is a template for the Systems Interface register set
 *  on the token ring adapter
 */

typedef struct PASS2_REGS
{
   unsigned short      data;
   unsigned short      data_autoincr;
   unsigned short      addr;
   unsigned short      cmd;
   unsigned char       enable;
   unsigned char       reset;
   unsigned char       imask_enable;
   unsigned char       imask_disable;
   unsigned short      timer;
   unsigned short      status;
} pass2_regs_t;



/* SSB structure Template */
typedef struct SSB
{
    unsigned short       c_type;         /* Command type */
    unsigned short       stat0;          /* status 0 */
    unsigned short       stat1;          /* status 1 */
    unsigned short       stat2;          /* status 2 */
} t_ssb;  /* end struct SSB */

/* SCB structure template */
typedef struct SCB
{
    unsigned short  adap_cmd;            /* Adapter Command */
    unsigned short  addr_field1;         /* Address Field 1 */
    unsigned short  addr_field2;         /* Address Field 2 */
} t_scb; /* End struct SCB definition */



typedef struct DWNLD_CCB
{
    unsigned short      cmd;
    unsigned short      segment[2];
    unsigned short      recs;
} t_dwnld_ccb;

typedef struct TIMER_DATA
{
   caddr_t         p_dds;          /* pointer to DDS */
   offl_elem_t     owe;            /* OFLV work element */
   unsigned int    run;            /* Is the timer currently running? */
} timer_data_t;

typedef struct ADAP_CHECK_BLOCK
{
   unsigned short  code;
   unsigned short  parm0;
   unsigned short  parm1;
   unsigned short  parm2;
} adap_check_blk_t;


#define IOCC_CACHE_SIZE    0x40    /* IOCC Cache size in bytes */
#define RCV_CHAIN_SIZE     0x10    /* Receive Chain Size */
#define TX_CHAIN_SIZE      MAX_TX_TCW    /* Transmit Chain Size */
#define ROS_LEVEL_SIZE 4
#define MICROCODE_LEVEL_SIZE    3

/*
*  Transmit List type definition
*/
#define TX_LIST_SIZE   0x2c

typedef struct GATHER_BLOCK
{
   unsigned short       cnt;            /* Amount of data at address to xfer */
   unsigned short       addr_hi;    /* high-order 2 bytes of address */
   unsigned short       addr_lo;    /* low-order 2 bytes of address */
}  gather_block_t;


typedef struct TX_LIST
{
   struct TX_LIST       *p_d_fwdptr;    /* Pointer to next in TX chain */
   unsigned short       tx_cstat;       /* TX CSTAT */
   unsigned short       frame_size;     /* TX Frame Size */
   gather_block_t       gb[3];          /* Data gather locations */
   xmt_elem_t           *p_tx_elem;     /* pointer to CIO TX element */
   unsigned short       *p_d_addr[3];   /* Bus addr. of 1st gather location */
   unsigned char        padd[IOCC_CACHE_SIZE - TX_LIST_SIZE];
} t_tx_list;

/*
*  Receive List type definition
*/
typedef struct recv_list {
   struct recv_list     *next;          /* pointer to next in chain */
   unsigned short       status;         /* receive status */
   unsigned short       frame_size;     /* size of the entire frame */
   unsigned short       count;          /* bytes in this receive list */
   unsigned short       addr_hi;        /* high 16 bits of data address */
   unsigned short       addr_lo;        /* low 16 bits of data address */
   unsigned char        unused[IOCC_CACHE_SIZE - 14];
} recv_list_t;

/*
 *      Token-Ring Adapter Open Options Structure
 */
typedef struct TOK_ADAP_OPEN_OPTIONS
{
   unsigned short   options;            /* Open Options - The PAD Routing */
                                        /* Field bit MUST be set to 1 */
   unsigned short   node_addr1;         /* High-order 2 bytes of node addr */
   unsigned short   node_addr2;         /* Middle 2 bytes of node address */
   unsigned short   node_addr3;         /* Low-order 2 bytes of node addr. */
   unsigned short   grp_addr1;          /* High-order 2 bytes of group addr */
   unsigned short   grp_addr2;          /* Low-order 2 bytes of group addr. */
   unsigned short   func_addr1;         /* High-order 2 bytes of func addr. */
   unsigned short   func_addr2;         /* Low-order 2 bytes of func addr. */
   unsigned short   rcv_list_size;      /* RCV List Element size in bytes */
   unsigned short   xmit_list_size;     /* TX List Element size in bytes */
   short            buf_size;           /* Adapter Buffer Size */
   short            res1;               /* Extnl RAM Start Addr. - not used */
   short            res2;               /* Extnl RAM End Addr. - not used */
   char             xmit_buf_min_cnt;   /* Minimum # of Adap. Buffers to */
                                        /* reserve for transmission */
   char             xmit_buf_max_cnt;   /* Maximum # of Adapter buffers to */
                                        /* use for transmit data */
   unsigned short   prod_id_addr1;      /* High-order 2 bytes of product ID */
                                        /* address */
   unsigned short   prod_id_addr2;      /* Low-order 2 bytes of product ID */
                                        /* address */
} tok_adap_open_options_t;

/*
 *      Token-Ring Adapter Initialization Parameter Structure
 */
typedef struct ADAP_I_PARMS
{
 short               init_options;
 unsigned char       cmd;
 unsigned char       xmit;
 unsigned char       rcv;
 unsigned char       ring;
 unsigned char       scb_clear;
 unsigned char       adap_chk;
 unsigned short      rcv_burst_size;
 unsigned short      xmit_burst_size;
 unsigned short      dma_abort_thresh;
 short               scb_add1;
 short               scb_add2;
 short               ssb_add1;
 short               ssb_add2;
} tok_adap_i_parms_t;

/*****************************************************************************/

/* convert from ds name to standard */
typedef tok_query_stats_t query_stats_t;

typedef struct TOK_ERROR_LOG_SENSE_DATA
{
   struct  err_rec0    errhead;
   unsigned int        adap_state;     /* Adapter State */
   unsigned int        limbo;          /* Limbo Mode State */
   unsigned int        rr_entry;       /* Reason for Entering Ring Recovery */
   unsigned long       limcycle;       /* Count of Network Recovery cycles */
   unsigned int        soft_chaos;     /* Count of N.R. Entry due to SW err */
   unsigned int        hard_chaos;     /* Count of N.R. Entry due to HW err */
   unsigned int        mcerr;          /* Last Micro Channel Error */
   unsigned short      ring_status;    /* Last Ring Status */
   unsigned short      afoot;          /* Last adapter error code */
   unsigned int        footprint;      /* Device handler's footprint */
   unsigned int        pio_attachment; /* current PIO Attachment key */
   unsigned int        pio_errors;     /* Number of PIO errors */
   unsigned int        piox;	       /* Last PIO exception */
   adap_check_blk_t    ac_blk;         /* Last Adapter Check code */
   unsigned short      tx_cstat;       /* Last TX CSTAT */
   unsigned short      rcv_cstat;      /* Last RCV CSTAT */
   int                  slot;          /* Adapter's slot number */
   unsigned char       cfg_pos[8];     /* Adapter POS Register settings */
   unsigned char       tok_addr[6];    /* Network Address in use */
   unsigned char       tok_vpd_addr[6];    /* VPD's burned in address */
                                           /* VPD's ROS level */
   unsigned char       tok_vpd_rosl[ROS_LEVEL_SIZE];
                                        /* VPD's Microcode level */
   unsigned char       tok_vpd_mclvl[MICROCODE_LEVEL_SIZE];
   tok_ring_info_t     ring_info;      /* Last Token-Ring Info */
   unsigned char       alert[20];      /* Array for alert only data */
} tok_error_log_data_t;

#define SENSE_DATA_SIZE         0x6c    /* error log sense data struct size */

/*---------------------------------------------------------------------------*/
/*                            WORK AREA OF DDS                               */
/*---------------------------------------------------------------------------*/

typedef struct 
{
   unsigned int        dma_chnl_id;    /* dma channel id...used for all   */
                                       /* d_xxx kernel services routines  */
   int                 adap_state; 	/* current adapter state */
   unsigned int        limbo;      /* Network Rcvry (limbo) Mode state flag */
   unsigned int        rr_entry;   /* reason for entering Limbo Mode */
   unsigned int	       adap_doa;   /* Adapter dead on arrival in open pend. */ 
   unsigned long       limcycle;       /* Limbo cycle count */
   unsigned int        soft_chaos;     /* limbo soft error entry counter */
   unsigned int        hard_chaos;     /* limbo hard error entry counter */
   unsigned int		rs_bcon_cnt;	/* Count of Beaconing interupts */
   unsigned int		rs_eserr_cnt;	/* Count of Excessive Soft Err intr */
   unsigned int		bcon_sb_sent;	/* Flag for status block sent */
   unsigned int        entropy;        /* amount of work done counter */
   unsigned int	       piox;		/* Last PIO exception */
   unsigned int	       mcerr;		/* Last Micro Channel Error */

   int                 footprint;	/* audit trail for adapt operations */
   uint			bugout;		/* reason for bugout */
   unsigned short      afoot;      	/* Adapter failure code footprint */
   unsigned short      open_fail_code;	/* OPEN Failure code returned by  */
                                        /* the adapter */
   unsigned short      adap_cmd_fail;	/* Command still in SSB.CMD */
                                        /* adapter failed to execute */
                                        /* this command. */
   unsigned short      ring_status;	/* Last Ring Status */

   struct status_block limbo_blk;	/* Limbo Entry Status block */
   struct status_block start_done_blk;	/* Start done status block */
   struct status_block halt_done_blk;	/* Halt done status block */
   adap_check_blk_t    ac_blk;     	/* Adapter Check code */
   offl_elem_t	       limbo_owe;	/* Limbo OFLV Work Element */
					/* that triggered entry into */
					/* recovery logic */
   WDT_SETTER	wdt_setter;		/* who started the watchdog timer */
   int		rx_buf_reused;		/* signal to load_recv_chain	*/

/* ---------------------------------------------------------------------- */
/*		Adapter Control Area control variables			  */


   unsigned short *p_mem_block;  /* pointer to dynamic work block */
   unsigned short *p_d_mem_block;
   struct xmem    mem_block_xmd;        /* Cross memory descriptor for */
                                        /* the dynamic work block */

   t_scb          *p_scb;         /* pointer to the SCB */
   t_scb          *p_d_scb;       /* holds the DMA addr of p_scb */

   t_ssb          *p_ssb;         /* pointer to the SSB */
   t_ssb          *p_d_ssb;       /* holds the DMA addr of p_ssb */


   tok_prod_id_t  *p_prod_id;     /* pointer to Product ID Information */
   tok_prod_id_t  *p_d_prod_id;   /* holds the DMA addr of p_prod_id */


   tok_adap_error_log_t     *p_errlog;      /* pointer to adapter error log */
   tok_adap_error_log_t     *p_d_errlog;    /* holds DMA addr of p_errlog */

   tok_ring_info_t         *p_ring_info;   /* pointer to Ring Information */
   tok_ring_info_t         *p_d_ring_info; /* holds BUS addr of ring info */
   tok_ring_info_t         ring_info;      /* Will hold the current */
                                           /* copy of Ring Information */
                                           /* that is in the ACA.  The */
                                           /* data in this variable will */
                                           /* be passed to the user in the */
                                           /* TOK_RING_INFO ioctl. */



   /*------------------------------------------------------------------------*/
   /*                      Data Structures for Transmit                      */

   offl_elem_t     tx_owe;      /* Transmit OFLV work element */
   unsigned int    tx_intr_in;  /* TX interrupt IN counter */
   unsigned int    tx_intr_out; /* TX interrupt OUT counter */
   unsigned int    tx_proc_limit;  /* TX max packet processing per. intr. */
   unsigned int    tx_noop_cnt;    /* TX No-Op counter */

   t_tx_list    *p_tx_head;     /* pointer to head of TX chain */
   t_tx_list    *p_tx_tail;     /* pointer to tail of TX chain */
   t_tx_list    *p_d_tx_head;   /* DMA addr of head of TX chain */
   t_tx_list    *p_d_tx_tail;   /* DMA addr of tail of TX chain */

   t_tx_list   *p_tx_1st_update;/* 1st location to start processing the TX */
                                /* completion interrupts on the TX chain */
   t_tx_list   *p_d_tx_1st_update;/* Bus addr of 1st location to start */

   t_tx_list   *p_tx_next_avail;/* Next available location in the TX */
                                /* to put Transmit data */
   t_tx_list   *p_d_tx_next_avail;/* Bus addr of next available location */

   int         tx_chain_full;  /* TX chain managment flags */
   int         tx_chain_empty;
   int         issue_tx_cmd;
   unsigned short tx_cstat;        /* last TX CSTAT */


   t_reg_list   *p_tx_tcw_list; /* pointer to TX TCW managment structure */
   unsigned int tx_tcw_base;    /* Bus Base addr. for TX buffers */
   xmt_elem_t 	*p_tx_elem[TX_CHAIN_SIZE];	/* transmit queue elements 
						/* that are currently in */
						/* the TX chain */
   t_tx_list 	*p_d_tx_fwds[TX_CHAIN_SIZE];
   unsigned short *p_d_tx_addrs[TX_CHAIN_SIZE][3]; /* Bus addresses */
						/* for TX requests */
						/* that are currently */
						/* in the TX chain */

    /* Information about adpater transmit descriptors                      */
    struct xmit_des tx_buf_des[MAX_TX_TCW];

    /* Information for transmit list local queue                           */
    char      *xmit_buf;	    /* pinned buffer of transmit data      */
    int       tx_buf_size;	    /* size of a transmit buffer           */
    struct xmem xbuf_xd;            /* xmem descriptor for transmit data   */
    struct xmit_elem *xmit_queue;
    short     tx_list_next_buf;     /* Index to next in to sofware queue   */
    short     tx_list_next_in;      /* Index to next in for adapter        */
    short     tx_list_next_out;     /* next to be ACKed by adapter         */
    short     tx_tcw_use_count;     /* number of xmit buffers used         */
    short     tx_des_next_in;       /* next transmit descriptor to allocate*/
    ushort    xmits_queued;         /* number of transmits on xmit queue */

    uint      bus_id;		    /* copy of DDI.bus_id 		*/
    uint      bus_io_addr;	    /* copy of DDI.ds.bus_mem_addr 	*/

   /*------------------------------------------------------------------------*/
   /*                      Data Structures for Receive                       */

   offl_elem_t     recv_owe;      /* Receive OFLV work element */
   unsigned int    recv_intr_in;  /* RECV interrupt IN counter */
   unsigned int    recv_intr_out; /* RECV interrupt OUT counter */
   unsigned int    recv_noop_cnt; /* RECV No-Op counter */

   unsigned int  recv_mode;                     /* receive running flag */
   int           read_index;                    /* next read in recv chain */
   struct trb    *mbuf_timer;                   /* used for mbuf retries */
   int           mbuf_timer_on;                 /* when mbuf retry is on */
   recv_list_t   *recv_list[ RCV_CHAIN_SIZE ];  /* dma addrs of recv chain */
   recv_list_t   *recv_vadr[ RCV_CHAIN_SIZE ];  /* virt addrs of recv chain */
   unsigned char *recv_addr[ RCV_CHAIN_SIZE ];  /* dma addrs of mbufs */
   struct mbuf   *recv_mbuf[ RCV_CHAIN_SIZE ];  /* array of mbufs */
   t_reg_list    *recv_tcw_list;                /* receive tcw's */
   unsigned int  recv_tcw_base;                 /* where they start */

   struct mbreq  mbreq;                         /* mbuf requirements */
   unsigned short rcv_cstat;       		/* last RCV CSTAT */

   /*------------------------------------------------------------------------*/

   unsigned int dl_tcw_base;    /* Bus Base addr. for microcode download */

   unsigned int        pio_attachment;         /* pio attachment address */
   unsigned int        pio_errors;             /* count of pio errors */
   unsigned int        pio_block;              /* prevent pio operations */
   lock_t              scb_lock;               /* lock around scb */
   unsigned long iocc_handle;
   unsigned long virtual_handle;

   unsigned char cfg_pos[8];     	/* value of POSn after startup  */
   unsigned char tok_addr[TOK_NADR_LENGTH]; /* actual net addr in use */


   unsigned char tok_vpd_addr[6];      /* VPD's burned in address */
                                        /* VPD's ROS level */
   unsigned char tok_vpd_rosl[ROS_LEVEL_SIZE];
                                        /* VPD's Microcode level */
   unsigned char tok_vpd_mclvl[MICROCODE_LEVEL_SIZE];

      /*
       * Adapter Initialization Parameters and
       * Adapter Open Options that will be given to the adapter.
       */

   tok_adap_open_options_t cfg_adap_open_opts;
   unsigned int    min_packet_len;
   unsigned int    max_packet_len;

   tok_adap_i_parms_t      adap_iparms;
   tok_adap_open_options_t adap_open_opts;
   tok_adap_open_options_t *p_open_opts;   /* pointer to open options block */
   tok_adap_open_options_t *p_d_open_opts; /* holds DMA addr of open opt */


      /*
       *   Diagnostic Adapter Initialization and
       *   Open Options that may be set by a diagnostic user.
       */
   tok_set_adap_i_parms_t  diag_iparms;
   tok_set_open_opts_t     diag_open_opts;

	/*
	 *  Timers for the Activation of the adapter,
	 *  functional address, group address, limbo,  and 
	 *  their associated data structures.
	 */
   struct trb      *p_bringup_timer;
   timer_data_t    time_data;

   timer_data_t        probate_td;     /* Probation timer data element */
   struct trb          *p_probate_timer;   /* PROBATION timer */

   timer_data_t        functional_td;  /* functional Addr. timer data el. */
   struct trb          *p_func_timer;  /* Functional Addr. timer */
   int                 funct_event;    /* Functional Addr. sleep/wakeup var */
   int                 funct_wait;     /* Functional Addr. wait flag */
   int                 funct_status;   /* Functional Addr. return status */

   timer_data_t        group_td;       /* Group Addr. timer data element */
   struct trb          *p_group_timer; /* Group Addr. timer */
   int                 group_event;    /* Group Addr. sleep/wakeup var */
   int                 group_wait;     /* Group Addr. wait flag */
   int                 group_status;   /* Group Addr. return status */
   int                 group_address;          /* Group Address Save */
   int                 funct_address;          /* Adapter functional address */
   int                 group_addr_chan;        /* it's channel */
   int                 mac_frame_active;       /* Mac Frame Active Tbl Ptr */

   timer_data_t        ring_info_td;  /* Ring Info. timer data el. */
   struct trb          *p_ring_info_timer; /* Ring Info. timer */
   int                 ring_info_event;    /* Ring Info. sleep/wakeup var */
   int                 ring_info_wait;     /* Ring Info. wait flag */
   int                 ring_info_status;   /* Ring Info. return status */

   timer_data_t        recv_limbo_td;  /* Receive limbo timer data */
   struct trb          *p_recv_limbo_timer; /* Receive limbo timer */
  /*
   *   Adapter bringup loop/retry variables
   *   These variables are used in walking through the adapter
   *   activation sequence.
   */
   unsigned int    reset_spin;
   unsigned int    reset_retry;
   unsigned int    adap_init_spin;
   unsigned int    adap_init_retry;
   unsigned int    adap_open_retry;
   int             close_event;

  /*
   *   Adapter information variables
   *   These are pointers for information that resides
   *   in adapter memory.
   */
   unsigned short p_a_ucode_lvl;
   unsigned short p_a_addrs;
   unsigned short p_a_parms;
   unsigned short p_a_mac_buf;
   unsigned int   ri_avail;    /* Is ring inforation available? */
   unsigned int   mask_int;    /*
                                *  Mask/Disable adapter interupts
                                *  TRUE = the adapter interrupts
                                *          have been disabled
                                *  FALSE = the adapter interrupts
                                *          are enabled.
                                */

} dds_wrk_t;

typedef struct {
 unsigned char   ac;                             /* access control */
 unsigned char   fc;                             /* Frame control */
 unsigned char   dest_addr[6];                   /* dest address */
 unsigned char   src_addr[6];                    /* source address */
 unsigned char   ctl1;                     /* route length   */
 unsigned char   ctl2;                     /* route direction */
 unsigned char   route_info[16];                 /* route info     */
 unsigned char   lsap;                           /* dest SAP                  */
 unsigned char   rsap;                           /* ssap                      */
 unsigned char   ctl;                            /* Control Field             */
 unsigned char   data[1];                         /* data or xid               */
        } in_test_packet;

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
#define XID_TEST_FC     0x40
typedef struct {
 unsigned char   ac;                             /* access control */
 unsigned char   fc;                             /* frame control */
 unsigned char   src_addr[6];                    /* source address */
 unsigned char   dest_addr[6];                   /* dest address */
 unsigned char   ctl1;                           /* route control (len) */
 unsigned char   ctl2;                           /* route control (len) */
 unsigned char   route_info[16];                 /* route information */
        } out_test_packet_hdr;

typedef struct {
 unsigned char   rsap;                           /* dest SAP                  */
 unsigned char   lsap;                           /* ssap                      */
 unsigned char   ctl;                            /* Control Field             */
 unsigned char   data[1];                         /* data or xid               */
        } out_test_packet_data;

#endif /* ! _H_TOKDSHI */
