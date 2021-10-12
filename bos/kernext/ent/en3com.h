/* @(#)32  1.6  src/bos/kernext/ent/en3com.h, sysxent, bos411, 9431A411a 8/2/94 12:36:02 */
/*
 * COMPONENT_NAME: sysxent -- High Performance Ethernet Device Driver
 *
 * FUNCTIONS: none.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_EN3COM
#define _H_EN3COM



/*****************************************************************************/
/*           trace table and component dump table                 	     */
/*****************************************************************************/

typedef struct {
   Simple_lock	trace_slock;		/* lock for the trace table */
   int		next_entry;			/* index to put data next */
   ulong	table[EN3COM_TRACE_SIZE];	/* trace table */
} en3com_trace_t;

typedef struct {
   int		count;				/* number of cdt entries used */
   struct cdt_head  head;			/* cdt head */
   struct cdt_entry entry[EN3COM_CDT_SIZE];	/* cdt entries */
} en3com_cdt_t;

/*****************************************************************************/
/*                      device states                                        */
/*****************************************************************************/

typedef enum {
	CLOSED = 0,                   	/* initial device state */
	DEAD,				/* fatal hardware error encountered */
	LIMBO,				/* error recovery period */
	OPEN_PENDING,			/* open initiated */
	OPENED,                       	/* opened successfully, functioning */
	CLOSE_PENDING                 	/* close initiated */
} DEVICE_STATE;

/*****************************************************************************/
/*                      limbo states                                         */
/*****************************************************************************/

typedef enum {
	NO_LIMBO = 0,                   /* initial state */
	LIMBO_RESET,			/* hard reset */
	LIMBO_RESET_DONE,		/* second step of reset */
	LIMBO_GET_MBOX_1,		/* get the 1st byte of mailbox addr */
	LIMBO_GET_MBOX_2,		/* get the 2nd byte of mailbox addr */
	LIMBO_GET_MBOX_3,		/* get the 3rd byte of mailbox addr */
	LIMBO_GET_MBOX_4,		/* get the 4th byte of mailbox addr */
	LIMBO_SET_EN,                   /* perform INDICATE-EN command */
	LIMBO_AL_LOC_OFF,               /* perform AL-LOC-OFF command */
	LIMBO_CONFIGURE,                /* perform CONFIGURE command */
	LIMBO_SET_ADDR,                 /* perform SET ADDRESS command */
	LIMBO_CONFIG_LIST,              /* perform CONFIG LIST command */
	LIMBO_REPORT_CONFIG,            /* perform REPORT CONFIG command */
	LIMBO_NO_FILTER,                /* perform SET TYPE command */
	LIMBO_MULTI			/* perform SET MULTICAST command */
} LIMBO_STATE;

/*****************************************************************************/
/*                      Adapter card type definition                         */
/*****************************************************************************/

typedef enum {
      ADPT_LATEST = 0,                   /* Unknown Adapter Card Type       */
      ADPT_10,                           /* Prototype, AT FF, not released  */
      ADPT_20,                           /* Original PS2/FF, First release  */
      ADPT_22,                           /* Original PS2/FF,                */
      ADPT_225,                          /* Original PS2/FF, Parity Update  */
      ADPT_23,                           /* Original PS2/FF, EC Embed       */
      ADPT_235,                          /* Original PS2/FF, EC Embed       */
      ADPT_30                            /* Fixed    PS2/FF, Release 1A     */
} ADAPTER_TYPE;

/*****************************************************************************/
/*           transmit/receive buffer descriptor control structure	     */
/*****************************************************************************/

typedef struct en3com_bdc {
	struct en3com_bdc *next;	/* point to the next descriptor */
	int	flags;			/* flag for marking used	*/
#define BDC_INITED	0x00000001	/* buffer initialized */
#define BDC_IN_USE	0x00000002	/* buffer in use */
#define BDC_BCAST	0x00000004	/* broadcast transmit */
#define BDC_MCAST	0x00000008	/* multicast transmit */
        char    *buf;			/* address of buffer		*/
	char    *dma_io;		/* dma io addresses     */
	ushort	bd_off;			/* offset of the BD on the adapter */
	int	tx_len;			/* length of data. used by tx */
} en3com_bdc_t;

/*****************************************************************************/
/*           multicast address registration table			     */
/*****************************************************************************/

typedef struct multi_slot {
	char m_addr[ENT_NADR_LENGTH];
	int  ref_count;
} multi_slot_t;

typedef struct en3com_multi {
	struct en3com_multi *next;	/* point to the table extension */
	short  in_use;			/* no. of slots in use in this table */
	multi_slot_t m_slot[MULTI_TABLE_SLOT];
} en3com_multi_t;

/*****************************************************************************/
/*                  Work section of device control structure		     */
/*****************************************************************************/

typedef struct {
  uchar		net_addr[ENT_NADR_LENGTH];/* actual network address in use */
  uchar 	vpd_na[ENT_NADR_LENGTH];  /* network address from VPD   */
  uchar		vpd_rosl[ROS_LEVEL_SIZE]; /* ROS level from VPD         */
  uchar		vpd_pn[PN_SIZE];	  /* Part number from VPD       */
  uchar		vpd_ec[EC_SIZE];	  /* EC number from VPD 	*/
  uchar		vpd_dd[DD_SIZE];	  /* DD number from VPD         */
  ushort	vpd_ros_length;		  /* actual length of ROS in VPD */
  ushort	vpd_hex_rosl;		  /* Converted VPD hex ROS level */
  ushort	vpd_hex_dd;		  /* Converted VPD hex DD level  */

      /* Information from adapter in "report configuration command"          */
  ulong   	main_offset;            /* adapter main memory offset      */
  ulong   	rv_mail_box;            /* Receive  Mailbox offset         */
  ulong   	tx_mail_box;            /* Transmit Mailbox offset         */
  ulong   	exec_mail_box;          /* Execute  Mailbox offset         */
  ulong   	stat_count_off;         /* Statistics Counters Offset      */
  ushort  	adpt_ram_size;          /* Adapter RAM size in bytes       */
  ushort  	buf_des_reg_size;       /* Buffer Descriptor Region Size   */
  ulong   	tx_list_off;            /* Transmit List Start Offset      */
  ulong   	rv_list_off;            /* Receive  List Start Offset      */
  ushort  	tx_list_cnt;            /* Transmit List Count             */
  ushort  	rv_list_cnt;            /* Receive  List Count             */
  ushort  	version_num;            /* Firmware Version number         */

  uint       	tx_tcw_base;		/* bus base addr for TX buffers   */
  uint       	rv_tcw_base;            /* bus base addr for receive buffer */
  short      	txd_cnt;            	/* Transmit buffer descriptor in use */
  short      	rvd_cnt;            	/* Receive buffer descriptor in use */
  uchar		*tx_buf;		/* beginning of the tx buffer pool */
  uchar		*rv_buf;		/* beginning of the rv buffer pool */
  struct xmem	txbuf_xmem;		/* xmem descriptor for transmit buf */
  struct xmem	rvbuf_xmem;		/* xmem descriptor for receive buf */

  uchar		pos_reg[NUM_POS_REG];   /* POS registers image */
  int	     	dma_channel;		/* for use with DMA services      */
  int		dma_fair;		/* DMA fairness			*/
  int		dma_addr_burst;		/* DMA address burst management */
  int		channel_alocd;		/* DMA channel state		*/
  int		tx_buf_alocd;		/* transmit buffer allocated or not */
  int		rv_buf_alocd;		/* receive buffer allocated or not */
  int		pos_parity;		/* POS parity enable		*/
  int		fdbk_intr_en;		/* feedback intr enable - POS4 bit 7 */
  int		intr_inited;		/* flag for interrupt registration */
  int		tx_wdt_inited;		/* flag for tx watchdog registration */
  int		ctl_wdt_inited;		/* flag for ctl watchdog registration */
  /* Patch code for adapter gate array problem-dummy i/o read to the bus */
  uchar 	gate_array_fix;
  ADAPTER_TYPE 	card_type;              /* Which version adptr was detected*/
  ulong		vpd_chk_flags;		/* flags for vpd verification	*/
#define	VPD_FOUND	0x00000001	/* Vital Product Data header found */
#define NA_FOUND	0x00000002	/* Network Address found flag */
#define RL_FOUND	0x00000004	/* ROS Level found flag */
#define PN_FOUND	0x00000008	/* Part Number found flag */
#define EC_FOUND	0x00000010	/* EC Number found flag */
#define DD_FOUND	0x00000020	/* DD Number found flag */
#define CRC_VALID	0x00000040	/* CRC is valid flag */
#define VPD_OK		0x0000007F	/* ORed value of all above */

  int 		restart_count;		/* times the error recovery performed */
  int		promiscuous_count;	/* promiscuous mode reference count */
  int		badframe_count;		/* save bad packet reference count */
  int		otherstatus;		/* all other async status ref count */
  int		multi_promis_mode;	/* flag for special multicast mode */
  int		enable_multi;		/* enable multicast reference count */
  int		filter_count;		/* number of filters registered */
  int		multi_count;		/* total multicast addr. registered */
  en3com_multi_t	multi_table;	/* first multicast addr. table */

  en3com_bdc_t  *txd_first;		/* txd queue first pointer */
  en3com_bdc_t  *txd_last;		/* txd queue last pointer  */
  en3com_bdc_t  *txd_avail;		/* first available txd on the txd q */
  en3com_bdc_t  *rvd_first;		/* rvd queue first pointer */
  en3com_bdc_t  *rvd_last;		/* rvd queue last pointer */
  en3com_bdc_t 	txd[MAX_TXD];		/* txd list */
  en3com_bdc_t 	rvd[MAX_RVD];		/* rvd list */

} en3com_wrk_t;

/***************************************************************************/
/*
 * This is the whole device control area				
 */
/***************************************************************************/
 

struct en3com_dev_ctl {
      	struct intr     ihs; 		/* interrupt handler ctl struct */
	int 		ctl_correlator; /* point to the dd_ctl table    */
	ndd_t		ndd;		/* ndd for NS ndd chain		*/
	struct en3com_dev_ctl *next;	/* point to the next device 	*/
	int		seq_number;	/*  sequence number */
  	Complex_lock	ctl_clock;	/* control operation lock */
	Simple_lock	cmd_slock;	/* adatper command lock */
	Simple_lock	tx_slock;	/* transmit lock  */
	Simple_lock	slih_slock;	/* SLIH lock */
  	DEVICE_STATE	device_state;   /* main state of the device */
  	LIMBO_STATE	limbo_state;    /* state of the limbo error recovery */
	int		ndd_stime;	/* start time of the ndd statistics */
	int		dev_stime;	/* start time of the dev statistics */
	int		txq_len;	/* current length of transmit queue */
	struct mbuf	*txq_first;	/* transmit queue */
	struct mbuf	*txq_last;	/* transmit queue */
  	int		tx_pending;	/* number of transmit outstanding */
  	int		ctl_status;	/* ioctl command status */
	int 		ctl_pending;	/* ioctl command outstanding flag */
	int		ctl_event;	/* sleep event for ent_ioctl */
	en3com_dds_t	dds;		/* device dependent structure */
	struct watchdog	tx_wdt;		/* watchdog timer for transmit  */
	struct watchdog	ctl_wdt;	/* watchdog timer for ioctl  	*/
	struct trb      *systimer;	/* system timer for error recovery */
      	en3com_vpd_t   	vpd; 		/* vital product data */
	ent_genstats_t	entstats;	/* ethernet generic statistics */
	en3com_stats_t	devstats;	/* 3com spcific statistics */
	ethernet_all_mib_t	mibs;	/* ethernet MIB's 		*/
      	en3com_wrk_t    wrk; 		/* device work area             */
};

typedef struct en3com_dev_ctl en3com_dev_ctl_t;


/***************************************************************************/
/*
 * This is the global device driver control structure
 */
/***************************************************************************/

struct en3com_dd_ctl {
  lock_t		cfg_lock;		/* lockl lock for config */
  Complex_lock		dd_clock;		/* device driver lock */
  en3com_dev_ctl_t  	*p_dev_list;		/* device control list */
  int 			num_devs;		/* count of devices configed */
  int 			open_count;		/* count of devices opened */
  en3com_trace_t	trace;			/* device driver trace table */
  en3com_cdt_t		cdt;			/* device drvier dump table */
};

typedef struct en3com_dd_ctl en3com_dd_ctl_t;


/*
 * Macros for accessing device control area. The pointer to this area has to 
 * be named p_dev_ctl for using these macros.
 */

#define IHS 	p_dev_ctl->ihs
#define NDD 	p_dev_ctl->ndd
#define TXWDT 	p_dev_ctl->tx_wdt
#define CTLWDT 	p_dev_ctl->ctl_wdt
#define VPD 	p_dev_ctl->vpd
#define DDS 	p_dev_ctl->dds
#define ENTSTATS	p_dev_ctl->entstats
#define DEVSTATS	p_dev_ctl->devstats
#define MIB	p_dev_ctl->mibs
#define WRK 	p_dev_ctl->wrk
#define CFG_LOCK	en3com_dd_ctl.cfg_lock
#define DD_LOCK		en3com_dd_ctl.dd_clock
#define TRACE_LOCK	en3com_dd_ctl.trace.trace_slock
#define CTL_LOCK 	p_dev_ctl->ctl_clock
#define CMD_LOCK 	p_dev_ctl->cmd_slock
#define TX_LOCK 	p_dev_ctl->tx_slock
#define SLIH_LOCK 	p_dev_ctl->slih_slock


/*****************************************************************************/
/*                  Error logging type definition                            */
/*****************************************************************************/

#define EN3COM_FNAME_LEN	32

struct error_log_def {
	struct err_rec0   errhead;        /* from com/inc/sys/err_rec.h      */
	uchar fname[EN3COM_FNAME_LEN];    /* filename and line number        */
	uchar pos_reg[NUM_POS_REG];       /* Adapter POS Registers           */
	uchar ent_addr[ENT_NADR_LENGTH];  /* actual net address in use       */
	ulong parm1;                      /* log data 1 		     */
	ulong parm2;                      /* log data 2 		     */
	ulong parm3;                      /* log data 3 		     */
};


#endif /* _H_EN3COM */
