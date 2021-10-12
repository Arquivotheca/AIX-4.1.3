/* @(#)71	1.23.2.4  src/bos/kernext/entdiag/entdshi.h, diagddent, bos411, 9428A410j 5/12/92 11:16:22 */
#ifndef _H_ENTDSHI
#define _H_ENTDSHI

/*****************************************************************************/
/*                                                                           */
/* COMPONENT_NAME: sysxent -- Ethernet Communications Code Device Driver     */
/*                                                                           */
/* FUNCTIONS: entdshi.h                                                      */
/*                                                                           */
/* ORIGINS: 27                                                               */
/*                                                                           */
/* (C) COPYRIGHT International Business Machines Corp. 1990                  */
/* All Rights Reserved                                                       */
/* Licensed Materials - Property of IBM                                      */
/*                                                                           */
/* US Government Users Restricted Rights - Use, duplication or               */
/* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*
 * NAME: entdshi.h
 *
 * FUNCTION: see discussion below
 *
 * NOTES:
 * The include files that make up the communications device drivers which
 * use the ciodd.c common code have the following heirarchy (where XXX is
 * the device-specific name such as tok or ent):
 *    comio.h   -- standard communications i/o subsystem declarations (Vol3Ch5)
 *                 needed by ciodd.c and XXXds.c, also needed by users
 *    XXXuser.h -- device-specific declarations (Vol3Ch6)
 *                 needed by ciodd.c and XXXds.c, also needed by users
 *    XXXddi.h  -- device specific part of DDI (defines used by cioddi.h)
 *    cioddi.h  -- common part of DDI definition that is part of DDS
 *                 needed by ciodd.c, XXXds.c, XXXconf.c, not needed by users
 *    cioddhi.h -- high-level independent common declarations
 *                 needed by ciodd.c, XXXds.c, ciodds.h, not needed by users
 *    XXXdshi.h -- high-level independent device-specific declarations
 *                 needed by ciodds.h, ciodd.c, XXXds.c, not needed by users
 *    ciodds.h  -- common part of DDS which depends on all preceding includes
 *                 needed by ciodd.c and XXXds.c, not needed by users
 *    cioddlo.h -- low-level common declarations that may depend on the DDS
 *                 needed by ciodd.c and XXXds.c, not needed by users
 *    XXXdslo.h -- low-level device-specific declarations
 *                 needed by XXXds.c ONLY (not needed by ciodd.c or by users)
 */

/*****************************************************************************/
#define DD_NAME_STR      "entdd"            /*                               */
#define TRACE_HOOKWORD   (HKWD_DD_ENTDD)    /* in <sys/trchkid.h>            */
#define MAX_GATHERS      (ent_MAX_GATHERS)  /*                               */
#define MAX_ADAPTERS     (ent_MAX_ADAPTERS) /*                               */
#define MAX_ADAPTERS_INDEX     (16) /*                               */
#define MAX_OPENS        (ent_MAX_OPENS)    /* max opens per adapter         */
#define MAX_CDT          (0)                /* Component Dump table entrir   */
#define MAX_OFFL_QUEUED  (64)               /* offl's queued per adap        */

#ifdef DEBUG
#define TRACE_TABLE_SIZE (1000)             /* SAVETRACE table (ulongs)      */
#else
#define TRACE_TABLE_SIZE (10)               /* SAVETRACE table (ulongs)      */
#endif /* DEBUG */

#include <sys/listmgr.h>

/* these defines effect the DDS size                                         */
#define ROS_LEVEL_SIZE   (4)        /* bytes in ROS Level                    */
#define PN_SIZE          (8)        /* bytes in Part number                  */
#define EC_SIZE          (6)        /* bytes in Engineering Change number    */
#define DD_SIZE          (2)        /* bytes in Device Driver Level          */
#define MAX_DIAG_QUEUED  (32)       /* maximum diagnostic que                */
#define MAX_EXEC_SIZE    (124)      /* max execute command size              */
#define MAX_ADPT_NETID   (32)       /* maximum adapter netid's               */
#define MAX_MULTI        (10)       /* max multicast addresses               */
#define MAX_EXEC         (8)        /* max Execute command que               */
#define RAM_SIZE         (0x4000)   /* Adapter RAM size - 16 K               */
#define MAX_TX_TCW       (10)       /* Maximum Transmit tcw's                */
#define MAX_RX_TCW       (32)       /* Maximum Receive  tcw's                */

   typedef enum {
      OFFL_WDT,                     /* this must be defined - Common code    */
      OFFL_TIMO,                    /* this must be defined - Common code    */
                                    /* add your own additional codes here    */
      OFFL_START,                   /* First start ioctl state machine       */
      OFFL_INTR                     /* Adapter interrupts                    */
   } OFFL_SETTER;

   typedef volatile struct {        /* device-specific que element           */
      OFFL_SETTER who_queued;       /* common code uses this identifier      */
      uchar       cmd_reg;          /* Adapter Command Register              */
   } offl_elem_t;

   typedef enum {                   /* Who set the watchdog timer            */
      WDT_INACTIVE,                 /* WDT is inactive                       */
      WDT_CONNECT,                  /* dsact in progress                     */
      WDT_XMIT,                     /* xmit in progress                      */
      WDT_CLOSE,                    /* Adapter close in progress             */
      WDT_RESTART,		    /* adapter restart			     */
   } WDT_SETTER;

   typedef enum {                   /* Who set the timeout timer             */
      TO_INACTIVE,                  /* Timer is inactive                     */
      TO_RX_MBUF,		    /* Receive mbuf get wait in progress     */
      TO_START,                     /* start in progress                     */
      TO_CLEANUP,		    /* cleanup attempt in progress	     */
   } TO_SETTER;

/*****************************************************************************/
/*                      Adapter card type definition                         */
/*****************************************************************************/

typedef enum {
      ACT_UNKNOWN = 0,                   /* Unknown Adapter Card Type       */
      ACT_10,                            /* Prototype, AT FF, not released  */
      ACT_20,                            /* Original PS2/FF, First release  */
      ACT_22,                            /* Original PS2/FF,                */
      ACT_225,                           /* Original PS2/FF, Parity Update  */
      ACT_23,                            /* Original PS2/FF, EC Embed       */
      ACT_235,                           /* Original PS2/FF, EC Embed       */
      ACT_30                             /* Fixed    PS2/FF, Release 1A     */
} ADAPTER_TYPE;

/*****************************************************************************/
/* these types are used to define the dds                                    */
/*****************************************************************************/
typedef ent_query_stats_t query_stats_t;

   /* State variable for first start state machine sequencing                */
   /* Used with state machine variable WRK.adpt_start_state                  */
   typedef enum {
      NOT_STARTED = 0,     /* Initial state after last halt                  */
      HARD_RESET_ON,       /* Adapter Hard Reset turned on - wait 1/2 sec    */
      HARD_RESET_OFF,      /* Adapter Hard Reset turned off - wait 1  sec    */
      SELF_TEST_ON,        /* Adapter Self Tests running - wait for hex 00   */
      GET_EMB_1,           /* Self Tests done - get first EMB byte           */
      GET_EMB_2,           /* Get 2nd Execute mail box byte                  */
      GET_EMB_3,           /* Get 3rd Execute mail box byte                  */
      GET_EMB_4,           /* Get 4th Execute mail box byte                  */
      EXEC_CMDS_STARTED,   /* Execute Mail box config commands started       */
      STARTED,             /* Adapter started & acknowledged                 */
      SLEEPING,            /* Adapter close in process - waiting on wakeup   */
      X_ABORT_1,           /* One TX or RX abort command processed           */
   } START_STATE;

/*
 * typedefs for adapter states
 */
typedef enum {
	normal,		/* normal adapter mode	*/
	error,		/* error state		*/
	cleanup,	/* attempting cleanup	*/
	restart,	/* attempting restart	*/
	broken,		/* hard error		*/
} adap_state_t;

/* 
 * typedefs for mailbox status checking
 */
typedef enum {
	execute,	/* execute mailbox command */
	recv,		/* receive mailbox command */
	xmit,		/* transmit mailbox command */
} mail_box_status_t;

	
/*****************************************************************************/
/*                  Transmit List type definition                            */
/*****************************************************************************/

struct xmit_des {
	ulong		io_addr;	/* io address for DMA		  */
	char		*sys_addr;	/* system address of data buffer  */
	ushort		des_offset;	/* adapter xmit descriptor address */
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
typedef struct xmit_elem xmit_elem_t;

typedef struct {
	ushort status;			/* status field			*/
	ushort el_offset;		/* offset of current end of list */
}XMITMBOX;

#define XMITQ_DEC(x) {(x)--; if ((x) < 0) (x) = (short)DDI.cc.xmt_que_size-1;}
#define XMITQ_INC(x) {(x)++; if ((x)==(short)DDI.cc.xmt_que_size) (x) = 0;}
#define XMITQ_FULL 	( \
 ((WRK.tx_list_next_buf+1 == DDI.cc.xmt_que_size) ? 0:WRK.tx_list_next_buf+1)\
 == WRK.tx_list_next_out)


/*****************************************************************************/
/*                  Receive List type definition                             */
/*****************************************************************************/
# define RECV_LIST_SIZE         MAX_RX_TCW


typedef struct {
	ushort	rbufd;				/* recieve buffer offset     */
        char    *buf;				/* address for receive bufs  */
	char    *rdma;				/* receive dma addresses     */
} recv_des_t;

typedef struct {                                /* RECEIVE MAILBOX:          */
        unsigned short  status;                 /* Status field              */
        unsigned short  recv_list;              /* Offset to receive list    */
} RECVMBOX;

typedef struct {                                /* BUFFER DESCRIPTOR:        */
        unsigned char   status;                 /* Status Byte               */
        unsigned char   control;                /* Control Byte              */
        unsigned short  next;                   /* Link to next pkt buffer   */
        unsigned short  count;                  /* Byte count                */
        unsigned short  buflo;                  /* Low part of buffer addr   */
        unsigned short  bufhi;                  /* High part of buffer addr  */
} BUFDESC;

/*****************************************************************************/
/*                  Work section of DDS                                      */
/*****************************************************************************/

typedef struct {


      TO_SETTER   to_setter;            /* Who started the timeout timer     */
      ulong  connection_result; /* start done status CIO_OK or CIO_HARD_FAIL */
      unsigned int    dma_base;         /* beginning of the dma addressing   */
                                        /* area for this adapter             */

      /* Old configuration values                                            */
      int   timr_priority;   /* for use with tstart system timer function    */
      int   offl_level;      /* for use with i_sched                         */
      int   offl_priority;   /* for use with i_sched                         */
      int   min_packet_len;  /* minimum valid packet length                  */
      int   max_packet_len;  /* maximum valid packet length                  */
      int   dma_fair;        /* DMA Fairness enabled                         */
      int   pos_parity;      /* POS Parity Enable                            */
      int   bus_parity;      /* I/O BUS Parity Enable                        */
      int   dma_addr_burst;  /* DMA Address Burst Management                 */


      int   dma_locked;      /* flag for diagnostic DMA ioctls.              */
      struct xmem dma_xatt;  /* cross-memory descriptor for DMA attachments. */

      /* Version 3 adapter card configuration values                         */
      int   adpt_parity_en;  /* Adapter parity enable - POS 4 bit 4          */
      int   fdbk_intr_en;    /* Select Feedback interrupt enable - POS 4 bit7*/


      /* Information on POS registers and Vital Product Data                 */
      uchar ent_addr[ent_NADR_LENGTH];    /* actual net address in use       */
      uchar ent_vpd_addr[ent_NADR_LENGTH]; /* actual net address from VPD    */
      uchar ent_vpd_rosl[ROS_LEVEL_SIZE]; /* actual ROS Level from VPD       */
      uchar ent_vpd_pn[PN_SIZE];          /* actual Part number from VPD     */
      uchar ent_vpd_ec[EC_SIZE];          /* actual EC number from VPD       */
      uchar ent_vpd_dd[DD_SIZE];          /* actual DD number from VPD       */
      ushort ent_vpd_ros_length;          /* Actual Length of VPD entry      */
      ushort ent_vpd_hex_rosl;            /* Converted VPD Hex ROS Level     */
      ushort ent_vpd_hex_dd;              /* Converted VPD Hex DD  Level     */
      uchar pos_reg[8];                   /* Adapter POS Registers           */


      /* Patch code for adapter gate array problem-dummy i/o read to the bus */
      uchar gate_array_fix;

      /* Information for the Diagnostic ioctl for read command register      */
      uchar   io_cmd_q[MAX_DIAG_QUEUED];  /* Diag I/O Command Reg read queue */
      uchar   io_stat_q[MAX_DIAG_QUEUED]; /* Diag I/O Status  Reg read queue */
      int     next_in;                    /* Pointer to next Diag input entry*/
      int     next_out;                   /* Pointer to next Diag outpt entry*/
      int     num_entries;                /* Current number of valid entries */

      /* Information for the multicast list                                  */
      uchar   multi_list[MAX_MULTI] [ent_NADR_LENGTH]; /* array of addresses */
      open_elem_t *multi_open[MAX_MULTI]; /* not NULL if entry valid         */
      ulong   multi_count;                /* Current number of valid entries */

      /* Information for the Type Field list                                 */
      ushort  type_list[MAX_NETID];       /* array of type field             */
      uchar   type_valid[MAX_NETID];      /* non-zero if entry valid         */
      ulong   type_count;                 /* Current number of valid entries */


      /* Flags for state of resources used/maintained by driver              */
      uchar   channel_alocd;              /* State of DMA_CHANNEL            */
 	/* Flag for first call to query RAS counters */
	int	first_flag;


      /* Information for the execute mail box command queue                  */
      uchar      exec_que[MAX_EXEC];      /* Execute mail box command queue  */
      uchar      exec_current_cmd;        /* Execute mail box Current Command*/
      ulong      exec_next_in;            /* Pointer to next exec input entry*/
      ulong      exec_next_out;           /* Pointer to next exec outpt entry*/
      ulong      exec_entries;            /* Current number of valid entries */
      ulong      exec_cmd_in_progres;     /* Command in progress in adapter  */
      ulong      recv_cmd_in_progres;     /* Command in progress in adapter  */
      ulong      xmit_cmd_in_progres;     /* Command in progress in adapter  */

      /* Information from adapter in "report configuration command"          */
      ulong   main_offset;                /* adapter main memory offset      */
      ulong   recv_mail_box;              /* Receive  Mailbox offset         */
      ulong   xmit_mail_box;              /* Transmit Mailbox offset         */
      ulong   exec_mail_box;              /* Execute  Mailbox offset         */
      ulong   stat_count_off;             /* Statistics Counters Offset      */
      ushort  adpt_ram_size;              /* Adapter RAM size in bytes       */
      ushort  buf_des_reg_size;           /* Buffer Descriptor Region Size   */
      ulong   xmit_list_off;              /* Transmit List Start Offset      */
      ulong   recv_list_off;              /* Receive  List Start Offset      */
      ushort  xmit_list_cnt;              /* Transmit List Count             */
      ushort  recv_list_cnt;              /* Receive  List Count             */
      ushort  version_num;                /* Firmware Version number         */


    /* mbuf requirements  for m_reg and m_dereg                              */
    struct mbreq  mbreq;

    /* Information for sync close                                            */
    int          close_event;
    uint       tx_tcw_base;         /* Bus Base addr. for TX buffers       */

    /* NOTE try to keep the following variables together, to minimize the
     * number of cache lines touched on a read/write path
     */

    recv_des_t   recv_list[ RECV_LIST_SIZE ];
    int   rdto;                         /* mbuf data offset for read data  */
    /* Data saved for the last netid received                              */
    netid_t last_netid;                 /* Last Net Id data type           */
    ushort  last_netid_length;          /* Last Length of netid            */
    ulong   last_netid_index;           /* Last index into netid table     */

    /* Information for receive list                                          */

    char	 *recv_buf;		   /* pinned buffer for receive data */
    struct xmem  rbuf_xd;		   /* xmem descriptor for recive data*/
    uint         Recv_Tcw_Base;            /* base dma addr for recv         */
    short        Recv_Index;               /* current recv buffer            */
    short        Recv_El_off;              /* end of list index              */
    int		 rstart_flg;

    /* Information to adapter in "Configure List Command"                  */
    ushort       recv_tcw_cnt;            /* Receive List Count  via TCWs    */
    ushort       xmit_tcw_cnt;            /* Transmit List Count via TCWs    */

    uint         bus_id;		  /* copy of DDI.cc.bus_id         */
    uint         bus_mem_addr;            /* copy of DDI.ds.bus_mem_addr   */
    uint         io_port;
    int	         dma_channel;             /* for use with DMA services     */
    adap_state_t adpt_state;		  /* adapter error state	   */
    mail_box_status_t mbox_status;	  /* state variable for mailbox status checking */
    START_STATE  adpt_start_state;        /* state variable for start up   */
    WDT_SETTER   wdt_setter;              /* Who started the watch dog timer */
    ADAPTER_TYPE card_type;               /* Which version adptr was detected*/
    struct watchdog wdt_section;	  /* watchdog timer                */
    int		 doggies;		  /* restarts mode dogs		   */


    /* Information about adpater transmit descriptors                      */
    struct xmit_des tx_buf_des[MAX_TX_TCW];
      

    /* Information for transmit list local queue                           */
    char      *xmit_buf;	    /* pinned buffer of transmit data      */
    struct xmem xbuf_xd;            /* xmem descriptor for transmit data   */
    struct xmit_elem *xmit_queue;
    short     tx_list_next_buf;     /* Inded to next in to sofware queue   */
    short     tx_list_next_in;      /* Index to next in for adapter        */
    short     tx_list_next_out;     /* next to be ACKed by adapter         */
    short     tx_tcw_use_count;     /* number of xmit buffers used         */
    short     tx_des_next_in;       /* next transmit descriptor to allocate*/
    struct    xmit_des *xmit_el_off; /* Tx list descriptor for current EL  */
    ushort    xmits_queued;         /* number of transmits on xmit queue */

} dds_wrk_t;

#endif /* ! _H_ENTDSHI */



