/* @(#)38       1.8  src/bos/kernext/mps_tok/mps_dd.h, sysxmps, bos41J, 9511A_all 3/7/95 15:27:16 */
/*
 *   COMPONENT_NAME: sysxmps
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_MPS_DD
#define _H_MPS_DD

#define   TOK_RCV_OP           		(0x00000040)

/*****************************************************************************/
/*           trace table and component dump table                            */
/*****************************************************************************/

typedef struct {
   Simple_lock  trace_slock;            	/* lock for the trace table */
   int          next_entry;                     /* index for the next entry */
   ulong        table[MPS_TRACE_SIZE];          /* trace table              */
} mps_trace_t;

typedef struct {
   int          count;                          /* number of cdt entries used */
   struct cdt_head  head;                       /* cdt head    */
   struct cdt_entry entry[MPS_CDT_SIZE];        /* cdt entries */
} mps_cdt_t;

/*****************************************************************************/
/*                      device states                                        */
/*****************************************************************************/

typedef enum {
        CLOSED = 0,                     /* initial device state             */
        OPEN_SETUP,                     /* setup initiated                  */
        OPEN_PENDING,                   /* open initiated                   */
        OPENED,                         /* opened successfully, functioning */
        CLOSE_PENDING,                  /* close initiated                  */
        LIMBO,                          /* error recovery period            */
        LIMBO_EXIT_PENDING,             /* reopen initiated                 */
        DEAD                            /* fatal hardware error encountered */
} DEVICE_STATE;

/*****************************************************************************/
/*                      watchdog timer states                                */
/*****************************************************************************/

typedef enum {             
   INACTIVE,                 /* WDT is inactive                              */
   TX1,                      /* xmit 1 in progress                           */
   TX2,                      /* xmit 2 in progress                           */
} WDT_SETTER;

/*****************************************************************************/
/*           multicast address registration table                            */
/*****************************************************************************/

typedef struct multi_slot {
        char m_addr[CTOK_NADR_LENGTH];
        int  ref_count;
} multi_slot_t;

typedef struct mps_multi {
        struct mps_multi *next;         /* point to the table extension      */
        short  in_use;                  /* no. of slots in use in this table */
        multi_slot_t m_slot[MULTI_ENTRY];
} mps_multi_t;

/*****************************************************************************/
/*                  Transmit List type definition                            */
/*****************************************************************************/
typedef struct xmit_buf {
  	ulong       	data_pointer ; 	/* data buffer pointer               */
  	ushort        	buf_len;        /* buffer length                     */
  	ushort 		reserved;       /* reserved                          */
} xmit_buf_t;

 typedef struct TX_LIST {
  	ulong       	fw_pointer;     /* transmit forward pointer          */
  	ulong       	xmit_status;    /* transmit status                   */
  	ushort        	frame_len;      /* frame length                      */
  	ushort 		buf_count;      /* data buffer length                */
        xmit_buf_t      xbuf[30];
  	ulong 		reserved;       /* reserved                          */
} tx_list_t;

int	tx_desc_len[30] = {0,	12 + (1 * 8),	 12 + (2 * 8),
				12 + (3 * 8),	 12 + (4 * 8),
				12 + (5 * 8),	 12 + (6 * 8),
				12 + (7 * 8),	 12 + (8 * 8),
				12 + (9 * 8), 	 12 + (10 * 8),
				12 + (11 * 8),	 12 + (12 * 8),
				12 + (13 * 8),	 12 + (14 * 8),
				12 + (15 * 8),	 12 + (16 * 8),
				12 + (17 * 8),	 12 + (18 * 8),
				12 + (19 * 8),	 12 + (20 * 8),
				12 + (21 * 8),	 12 + (22 * 8),
				12 + (23 * 8), 	 12 + (24 * 8),
				12 + (25 * 8),	 12 + (26 * 8),
				12 + (27 * 8),	 12 + (28 * 8),
				12 + (29 * 8)
			};

struct xmit_elem {                      /* a transmit queue element          */
        struct mbuf     *mbufp;         /* pointer to mbuf with data         */
        short           bytes;          /* number of bytes in packet         */
        tx_list_t       *tx_ds;         /* xmit descriptor                   */
};
typedef struct xmit_elem xmit_elem_t;

/*****************************************************************************/
/*                  Receive List type definition                             */
/*****************************************************************************/
typedef struct RX_LIST {
  	ulong       	fw_pointer;     /* receive forward pointer           */
  	ulong      	recv_status;    /* receive status                    */
  	ulong       	data_pointer;   /* data buffer pointer               */
  	ushort 		data_len;       /* data buffer length                */
  	ushort        	fr_len;         /* frame length                      */
} rx_list_t;

/*****************************************************************************/
/*                  Work section of device control structure                 */
/*****************************************************************************/

typedef struct {
      	int     ctl_event;                /* Event of ctl                    */
        int	promiscuous_count;        /* promiscuous count               */
      	int     retcode[ret_code_size];   /* event return code               */

      	tok_adapter_log adap_log;         /* Adapter log                     */

      	/* Address of LAP registers                                          */
      	ushort  asb_address;              /* address of ARB                  */
      	ushort  srb_address;              /* address of SRB                  */
      	ushort  arb_address;              /* address of ARB                  */
      	ushort  trb_address;              /* address of TRB                  */
      	ushort  parms_addr;               /* address of adapter parameters   */

  	int     dma_channel;              /* for use with DMA services       */
  	int     channel_alocd;            /* DMA channel state               */
  	int     setup;                    /* flag for mps set up             */
  	int     intr_inited;              /* flag for interrupt registration */
  	int     wdt_inited;               /* flag for watchdog registration  */
        WDT_SETTER	tx1wdt_setter;	  /* flag for tx watchdog setter     */
        WDT_SETTER	tx2wdt_setter;	  /* flag for tx watchdog setter     */

	/*
         *  Information on POS registers and Network address               
	 */
        uchar  pos_reg[NUM_POS_REG];         /* Adapter POS registers        */
        uchar  mps_addr[CTOK_NADR_LENGTH];   /* actual net address in use    */

	/*
         *  Adapter Control Area control variables                   
	 */
        struct xmem rx_mem_block;
        struct xmem rx_xmemp[MAX_RX_LIST];
        uchar  *rx_p_mem_block;       /* pointer to dynamic control block    */

        struct xmem tx1_mem_block;
        struct xmem tx1_xmemp[MAX_TX_LIST];
        uchar  *tx1_p_mem_block;      /* pointer to dynamic control block    */

        struct xmem tx2_mem_block;
        struct xmem tx2_xmemp[MAX_TX_LIST];
        uchar  *tx2_p_mem_block;      /* pointer to dynamic control block    */

	/*
         * Information for transmit channel 1                          
	 */
        short   tx1_elem_next_in;          /* Index to next in for adapter  */
        short   tx1_elem_next_out;         /* next to be ACKed by adapter   */
        short   tx1_frame_pending;         /* number of Tx frame pending    */

        short   tx1_buf_next_in;           /* Index to next in for adapter  */
        short   tx1_buf_next_out;          /* next to be ACKed by adapter   */
        short   tx1_buf_use_count;         /* number of Tx buf used         */

	xmit_elem_t tx1_queue[160];        /* TX software queue             */
        tx_list_t  *tx1_list[MAX_TX_LIST]; /* tx memory descriptor list     */
        tx_list_t  *tx1_vadr[MAX_TX_LIST]; /* virt transmit memory list     */
        tx_list_t   tx1_temp[MAX_TX_LIST]; /* temp Tx1 transmit memory list */
        uchar      *tx1_buf[MAX_TX_LIST];  /* array of memory               */
        uchar      *tx1_addr[MAX_TX_LIST]; /* dma addrs of memory           */
	uchar	    tx1_retry;		   /* Tx retry index                */

	/*
         * Information for transmit 1 dma address range                        
	 */
        short      tx1_dma_next_in;       /* Index to next in for adapter*/
        short      tx1_dma_next_out;      /* next to be ACKed by adapter */
        short      tx1_dma_use_count;          /* number of xmit buffers used */
        uchar      *tx1_dma_addr[MAX_TX_LIST]; /* array of DMA memory         */
        struct xmem tx1_dma_xmemp[MAX_TX_LIST];

	/*
         * Information for transmit channel 2                          
	 */
        short   tx2_elem_next_in;          /* Index to next in for adapter  */
        short   tx2_elem_next_out;         /* next to be ACKed by adapter   */
        short   tx2_frame_pending;         /* number of Tx frame pending    */

        short   tx2_buf_next_in;           /* Index to next in for adapter  */
        short   tx2_buf_next_out;          /* next to be ACKed by adapter   */
        short   tx2_buf_use_count;         /* number of Tx buf used         */

	xmit_elem_t tx2_queue[160];        /* TX software queue             */
        tx_list_t  *tx2_list[MAX_TX_LIST]; /* tx memory descriptor list     */
        tx_list_t  *tx2_vadr[MAX_TX_LIST]; /* virt transmit memory list     */
        tx_list_t   tx2_temp[MAX_TX_LIST]; /* temp Tx2 transmit memory list */
        uchar      *tx2_buf[MAX_TX_LIST];  /* array of memory               */
        uchar      *tx2_addr[MAX_TX_LIST]; /* dma addrs of memory           */
	uchar	    tx2_retry;		   /* Tx retry index                */

	/*
         * Information for transmit 2 dma address range                        
	 */
        short      tx2_dma_next_in;            /* Index to next in for adapter*/
        short      tx2_dma_next_out;           /* next to be ACKed by adapter */
        short      tx2_dma_use_count;          /* number of xmit buffers used */
        uchar      *tx2_dma_addr[MAX_TX_LIST]; /* array of DMA memory         */
        struct xmem tx2_dma_xmemp[MAX_TX_LIST];

	/*
         * Information for receive list                                     
	 */
        char        *recv_buf;                /* pinned buf for receive data*/
        int          read_index;              /* receive buffer array index */
        rx_list_t   *recv_list[MAX_RX_LIST];  /* Rx buffer descriptor list  */
        rx_list_t   *recv_vadr[MAX_RX_LIST];  /* vart receive buffer list   */
        struct mbuf *recv_mbuf[MAX_RX_LIST];  /* array of mbufs             */
        uchar       *recv_addr[MAX_RX_LIST];  /* dma addrs of mbufs         */
        struct mbuf *mhead;                   /* head of array of mbufs     */
        struct mbuf *mtail;                   /* tail of array of mbufs     */

	/*
         * Dump variable 
	 */
        int          dump_started;
        int          dump_MISR;

  	int          ndd_limbo;               /* ndd_limbo flag             */
  	int          multi_count;             /* total group addr. reg      */
  	mps_multi_t  multi_table;             /* first group addr. table    */
  	mps_multi_t  *multi_last;             /* last  group addr.          */
  	mps_multi_t  *new_multi;              /* new group addr. table      */
  	mps_multi_t  *free_multi;             /* free group addr table.     */
  	ulong        func_ref_cnt[32];        /* first func addr ref        */

        int          ndd_stime;        /* start time of the ndd statistics  */
        int          dev_stime;        /* start time of the dev statistics  */
        int          iocc;                    /* flags for IOCC machine     */
        int          pio_rc;                  /* PIO error flag             */
        int          hard_err;                /* hard error flag            */
        int          t_len;                   /* total data len in mbuf chain */
} mps_wrk_t;

/****************************************************************************/
/*                                                                          */
/*    This is the whole device control area                                 */
/*                                                                          */
/****************************************************************************/


struct dev_ctl {
        struct intr     ihs;            /* interrupt handler ctl struct      */
        int             ctl_correlator; /* point to the dd_ctl table         */
        ndd_t           ndd;            /* ndd for NS ndd chain              */
        struct dev_ctl *next;       	/* point to the next device          */
        int             seq_number;     /* sequence number                   */
        Complex_lock    ctl_clock;       /* ioctl lock                        */
        Simple_lock     cmd_slock;      /* adatper command lock              */
        Simple_lock     tx_slock;       /* transmit lock                     */
        Simple_lock     slih_slock;     /* SLIH lock                         */
        DEVICE_STATE    device_state;   /* main state of the device          */
        int             txq1_len;       /* current length of transmit queue 1*/
        struct mbuf     *txq1_first;    /* transmit queue pointer            */
        struct mbuf     *txq1_last;     /* transmit queue pointer            */
        int             txq2_len;       /* current length of transmit queue 2*/
        struct mbuf     *txq2_first;    /* transmit queue pointer            */
        struct mbuf     *txq2_last;     /* transmit queue pointer            */
        int             ctl_pending;    /* ioctl command pending flag        */
        int             open_pending;   /* open command pending flag         */
        mps_vpd_t       vpd;            /* vital product data                */
        mps_dds_t       dds;            /* device dependent structure        */
        struct watchdog tx1_wdt;        /* watchdog timer for transmit 1     */
        struct watchdog tx2_wdt;        /* watchdog timer for transmit 2     */
        struct dev_ctl *tx_dev;     	/* point to this device              */
        struct watchdog ctl_wdt;        /* watchdog timer for ioctl          */
        struct watchdog hwe_wdt;        /* watchdog timer for error revover  */
        struct watchdog lim_wdt;        /* watchdog timer for revover timer  */
        struct watchdog lan_wdt;        /* watchdog timer for lan status     */
        struct dev_ctl *ctl_dev;    	/* point to this device              */
        tok_genstats_t tokstats;        /* token ring generic statistics     */
        tr_mps_stats_t devstats;        /* MPS specific statistics           */
        token_ring_all_mib_t 	mibs;   /* MPS MIB's                         */
        mps_wrk_t       wrk;            /* Wildwood device work area         */
};

typedef struct dev_ctl mps_dev_ctl_t;

/*****************************************************************************/
/*                                                                           */
/* This is the global device driver control structure                        */
/*                                                                           */
/*****************************************************************************/

struct ndd_dd_ctl {
  lock_t                cfg_lock;               /* lockl lock for config     */
  Complex_lock          dd_clock;               /* device driver lock        */
  mps_dev_ctl_t         *p_dev_list;  		/* device control list       */
  int                   num_devs;               /* count of devices configed */
  int                   open_count;             /* count of devices opened   */
  mps_trace_t           trace;                  /* device driver trace table */
  mps_cdt_t             cdt;                    /* device drvier dump table  */
};

typedef struct ndd_dd_ctl mps_dd_ctl_t;


/*
 * Macros for accessing device control area. The pointer to this area has 
 * to be named p_dev_ctl for using these macros.
 */

#define IHS     	p_dev_ctl->ihs
#define NDD     	p_dev_ctl->ndd
#define TX1WDT   	p_dev_ctl->tx1_wdt
#define TX2WDT   	p_dev_ctl->tx2_wdt
#define HWEWDT  	p_dev_ctl->hwe_wdt
#define LIMWDT  	p_dev_ctl->lim_wdt
#define LANWDT  	p_dev_ctl->lan_wdt
#define CTLWDT  	p_dev_ctl->ctl_wdt
#define VPD     	p_dev_ctl->vpd
#define DDS     	p_dev_ctl->dds
#define TOKSTATS        p_dev_ctl->tokstats
#define DEVSTATS        p_dev_ctl->devstats
#define MIB   		p_dev_ctl->mibs
#define WRK     	p_dev_ctl->wrk

#define CFG_LOCK        mps_dd_ctl.cfg_lock
#define DD_LOCK         mps_dd_ctl.dd_clock
#define TRACE_LOCK      mps_dd_ctl.trace.trace_slock
#define TX_LOCK         p_dev_ctl->tx_slock
#define CTL_LOCK        p_dev_ctl->ctl_clock
#define CMD_LOCK        p_dev_ctl->cmd_slock
#define SLIH_LOCK       p_dev_ctl->slih_slock

#define FUNCTIONAL      MIB.Token_ring_mib.Dot5Entry 


/*****************************************************************************/
/*                  Error logging type definition                            */
/*****************************************************************************/


#define MPS_FNAME_LEN        32

struct error_log_def {
        struct err_rec0   errhead;        /* from com/inc/sys/err_rec.h      */
        uchar fname[MPS_FNAME_LEN];       /* filename and line number        */
        uchar pos_reg[NUM_POS_REG];       /* Adapter POS registers           */
        uchar mps_addr[CTOK_NADR_LENGTH]; /* actual net address in use       */
        ulong parm1;                      /* log data 1                      */
        ulong parm2;                      /* log data 2                      */
        ulong parm3;                      /* log data 3                      */
};

#endif /* _H_MPS_DD */

