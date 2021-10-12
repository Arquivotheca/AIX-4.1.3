/* @(#)49	1.10  src/bos/kernel/sys/POWER/vscsidd.h, sysxscsi, bos411, 9428A410j 6/3/94 15:12:56 */

#ifndef _H_VSCSIDD
#define _H_VSCSIDD
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Device Driver
 *
 * FUNCTIONS:	NONE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
 *
 * NAME:        vscsidd.h
 *
 * FUNCTION:    IBM SCSI Protocol Driver Header File
 *
 *              This protocol driver is the interface between a SCSI device
 *              driver and the SCSI adapter driver.  It executes commands
 *              from multiple drivers which contain generic SCSI device
 *              commands, and manages the execution of those commands.
 *              Several ioctls are defined to provide for system management
 *              and adapter diagnostic functions.
 *
 */


/************************************************************************/
/* Include Definitions Needed For This File                             */
/************************************************************************/
#include <sys/types.h>
#include <sys/watchdog.h>
#include <sys/device.h>
#include <sys/dump.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/scb_user.h>
#include <sys/scsi.h>
#include <sys/scsi_scb.h>

/************************************************************************/
/* General Device Driver Defines                                        */
/************************************************************************/

#ifdef VSC_TRACE
/*  The trace table length is TRACE_ENTRIES number of struct trace_element */
/*  plus two extra words to mark the beginning and end of the trace table. */
/*  TRACE_TABLE_LENGTH is in units of unsigned integers.                   */
#define TRACE_ENTRIES   64
#define TRACE_TABLE_LENGTH TRACE_ENTRIES * sizeof(struct trace_element) / 4 + 2
#endif VSC_TRACE

#ifndef TRUE
#define TRUE            1
#define FALSE           0
#endif
#define LPAGESIZE       PAGESIZE                /* internal page size limit  */

#define MAX_LUNS        32                       /* MAX LUN's per device      */
#define DEVPOINTERS     512                     /* MAX devices per bus       */
#define TMPOINTERS      16                      /* MAX target mode devices   */
#define SC_TM_BUFSIZE   0x1000                  /* 4096                      */
#define SC_TM_MIN_NUMBUFS  16                   /* min # of buffers for      */
                                                /* a tm device               */
#define SC_NUM_CMD_WORDS 8                      /* number of 32 bit words in */
                                                /* free command list         */
                                                /* driver can request        */
#define MAX_PREEMPTS 90                         /* max amount of times a cmd */
                                                /* can be preempted before a */
                                                /* it is assumed timed out   */
#define VSC_BITS_PER_WORD 0x20                  /* used to find free cmd_elem*/
#define ENABLE          0                       /* enable id option          */
#define DISABLE         1                       /* disable id option         */
#define T_DISABLE       3                       /* temp disable id option    */
#define RESET_CMD_T_O   8                       /* timeout value for ABORT,  */
                                                /* BDR, or SCSI bus reset    */
#define WAITING_FOR_RESOURCES   1
#define SCSI_HASH       0x07                    /* adapter mask hash value   */
#define VSC_EVENT_MASK  0x8000                  /* mask to determine type of */
                                                /* reply elem (good or error)*/
/*
 * SCSI Dump States
 */
#define	DUMP_IDLE	0x01
#define	DUMP_INIT	0x02
#define	DUMP_START	0x03

/* the following are all mask values used to set the media flags in the      */
/* SCSI Command Descriptor Block (CDB)                                       */
#define VSC_SIMPLE_Q    0x00000000              /* simple q_msg mask value   */
#define VSC_ORDERED_Q   0x00020000              /* ordered q_msg mask value  */
#define VSC_HEAD_OF_Q   0x00010000              /* head of q_msg mask value  */
#define VSC_NO_Q        0x00030000              /* no q_msg mask value       */
#define VSC_NO_DISC     0x00000100              /* do not allow disconnect   */
                                                /* mask value                */
#define VSC_ASYNC       0x00000200              /* send command asynch mask  */
#define VSC_READ        0x00001000              /* data being sent to init   */
#define VSC_WRITE       0x00000800              /* data being sent to target */
#define VSC_ABORT       0x00004000              /* simple abort mask value   */
#define VSC_ADAP_ONLY   0x00080000              /* clear adapter queue only  */
                                                /* for this abort.           */
#define VSC_SCSI_RESET  0x0000C000              /* SCSI bus reset mask value */
#define VSC_BDR         0x00002000              /* bus device reset BDR mask */
#define VSC_RESUME      0x00000400              /* resume device queue mask  */
#define VSC_Q_SUSPENDED 0x2000                  /* indicates if the cmd queue*/
                                                /* was suspended for this    */
                                                /* error element             */
/* Device error codes for rpl_element_def */

#define VSC_CANCELED_BY_RESET 0x01              /* indicates cmd failed by   */
                                                /* a SCSI bus reset          */
#define VSC_SCSI_FAULT        0x02              /* indicates cmd failed by   */
                                                /* a SCSI interface fault    */
#define VSC_BUS_FREE          0x11              /* unexpected bus free while */
                                                /* adapter processing cmd    */
#define VSC_INVALID_PHASE     0x13              /* an unexpect SCSI phase    */
                                                /* sequence occurred while   */
                                                /* adapter processing cmd    */
#define VSC_NEED_I_RESET      0x14              /* adapter detects a hung    */
                                                /* internal SCSI bus         */
#define VSC_NEED_E_RESET      0x15              /* adapter detects a hung    */
                                                /* external SCSI bus         */
#define VSC_MESSAGE_REJECTED  0x12              /* a SCSI target rejected a  */
                                                /* mandatory SCSI message    */
#define VSC_I_TERM_PWR        0x04              /* cmd failed no;term pwr    */
#define VSC_E_TERM_PWR        0x05              /* cmd failed no;term pwr    */
#define VSC_I_DIFF_SENSE      0x06              /* cmd failed;diff sense err */
#define VSC_E_DIFF_SENSE      0x07              /* cmd failed;diff sense err */
#define VSC_SEL_TIMEOUT       0x10              /* cmd failed;device could   */
                                                /* not select device         */
                                                

/* Command error codes for rpl_element_def */

#define VSC_ABORTED_BY_SYSTEM  0x04             /* indicates cmd failed by   */
                                                /* abort or BDR from system  */
#define VSC_ABORTED_BY_RESET   0x05             /* indicates cmd failed by   */
                                                /* a reset of the adapter    */
#define VSC_ABORTED_BY_ADAP    0x24             /* indicates cmd failed by   */
                                                /* adapter initiated abort   */
#define VSC_DMA_ERROR          0x22             /* dma error detected for cmd*/
 
/* The following two macros are used to mark the index in the                */
/* scsi->free_cmd_list as either in use or free.  Each bit in the            */
/* free_cmd_list corresponds to a cmd_elem in the scsi->cmd_pool.            */

/* Macro for setting a tag as in use ( 0 = in use ) */
#define VSC_GETTAG(word,tag)         word &= (~(1<<(31-(tag%32))))

/* Macro for clearing a tag as free  ( 1 = free ) */
#define VSC_FREETAG(word,tag)       word |= ( 1<<( 31-(tag%32)))


struct wtimer    {
        struct  watchdog dog;               /* the watchdog struct           */
        struct  scsi_info       *scsi;      /* ptr to scsi struct            */
        int     index;                      /* index to dev_info struct      */
        ushort  timer_id;                   /* my internal timer id val      */
#define SCSI_CDAR_TMR    1                  /* id of cdar timer              */
#define SCSI_DEV_TMR     2                  /* id of a device timer          */
#define SCSI_CANCEL_TMR  3                  /* id of an abort, BDR, or resume*/
                                            /* timer                         */
#define SCSI_RESET_TMR   4                  /* id of a SCSI bus reset timer  */
        uchar   save_time;                  /* used to manage the active q   */
};

/*
 *  structure passed to protocol driver config entry point
 *  at configuration time.
 */
struct vscsi_ddi {
        char    resource_name[16];          /* resource name for err log     */
        char    parent_lname[16];           /* logical name  of parent       */
        int     cmd_delay;                  /* parent's cmd_delay attribute  */
                                            /* reset or BDR before sending   */
                                            /* commands to devices which     */
                                            /* have the SC_DELAY_CMD bit set */
        uint     num_tm_bufs;               /* number of target mode buffers */
                                            /* to allocate for target mode   */
        uint   parent_unit_no;              /* unit number of parent         */
	uint	intr_priority;	            /* parent's interrupt priority   */
        uint     sc_im_entity_id;           /* ndd entity id for SCSI init   */
                                            /* mode ndd_add_filter           */
        uint     sc_tm_entity_id;           /* ndd entity id for SCSI target */
                                            /* mode ndd_add_filter           */
        uchar   bus_scsi_id;                /* the adapter's SCSI id on this */
                                            /* bus                           */
        uchar   wide_enabled;               /* value indicates whether SCSI  */
                                            /* wide is enabled or not.       */
        uchar   location;                   /* indicates location of bus on  */
                                            /* adapter: 0 = internal         */
                                            /*          1 = external         */
        uchar   num_cmd_elems;              /* number of command elements to */
                                            /* malloc for this vscsi instance*/
};


/*
 *  structure that the protocol driver uses to
 *  queue active commands
 */
struct  cmd_elem {
        struct  rpl_element_def reply;         /* contains reply ctrl elem   */
        struct  ctl_elem_blk    ctl_elem;      /* ctrl_elem_blk              */
        struct  ctl_element_def request;       /* contains request ctrl elem */
        struct  pd_info         pd_info1;      /* describes buffer           */
        struct  sc_buf          *bp;           /* pointer to sc_buf          */
        struct  scsi_info       *scsi;         /* ptr to the scsi struct     */
        uchar    cmd_type;                     /* indicates if proc lvl, intr*/
                                               /* lvl, or strategy cmd_elem  */
#define PROC_LVL_CANCEL      1                 /* proc lvl cancel cmd_elem   */
#define INTR_LVL_CANCEL      2                 /* intr lvl cancel cmd_elem   */
#define DEV_STRAT_CMD        3                 /* strategy initiated cmd_elem*/
#define INTR_LVL_RESET       4                 /* intr lvl SCSI bus reset    */
        uchar    cmd_state;                    /* current state of command   */
#define INACTIVE        0                      /* command is not active;     */
                                               /* cmd_elem is free           */
#define IS_ACTIVE       1                      /* command is active at       */
                                               /* adapter device driver      */
#define INTERRUPT_RECVD 2                      /* interrupt received for cmd */
#define WAIT_FOR_TO_2   3                      /* waiting for cmd_elem after */
                                               /* it has timed out           */
        uchar   preempt;                       /* count of number of preempts*/
        uchar   tag;                           /* position in free array     */
};



/*
 *  virtual device structure, may have several per adapter.
 */
struct  scsi_info {
	struct	vscsi_ddi    ddi;             /* ddi for proto driver        */
        struct 	wtimer    cdar_wdog;          /* command delay after reset   */
                                              /* timer                       */
        struct 	wtimer    reset_wdog;          /* SCSI bus reset timer       */
        struct  cmd_elem  reset_cmd_elem;     /* cmd_elem used to send SCSI  */
        struct  ns_com_status status_filter;  /* status filter used during   */
                                              /* ns_add_status call          */
        lock_t  scsi_lock;                    /* per SCSI bus lock           */
        Simple_lock ioctl_lock;               /* MP lock for ioctls          */
        dev_t   devno;                        /* proto major/minor number    */
                                              /* bus resets for this bus     */
        int     open_event;                   /* event word for async open   */
        int     ioctl_event;                  /* event word for ioctls       */
        uint    free_cmd_list[SC_NUM_CMD_WORDS];/* free cmd_elem list        */
        struct  scsi_shared *shared;          /* pointer to the shared struct*/
	struct	dev_info  *dev[DEVPOINTERS];  /* pointers to devinfo struct's*/
	struct	tm_info   *tm[TMPOINTERS];    /* array of tm_info structs    */
        struct  b_link    *head_free;         /* head of free b_link list    */
	struct  b_link    *b_pool;            /* memory for all b_links      */
        struct  b_link    *read_bufs;         /* head of doubly linked read  */
                                              /* buf list                    */
	struct	cmd_elem  *cmd_pool;          /* ptr to start of cmd_elem    */
                                              /* pool                        */
        struct  scsi_info *next;              /* pointer to next scsi_info   */
	struct  gwrite    *head_gw_free;      /* head gathered write free    */
	struct  gwrite    *tail_gw_free;      /* tail gathered write free    */
        uchar   proc_results;                 /* flag indicating results of  */
                                              /* a proc lvl scsi bus command */
#define GOOD_COMPLETION 0                     /* completes without error     */
        char    proc_sleep_id;                /* denotes the id of NDD_TGT_  */
                                              /* ADD_DEV proc waiting        */
        uchar   dump_state;                   /* indicates state of dump dev */
        uchar   opened;                       /* open flag                   */
        uchar   num_tm_devices;               /* number of active target mode*/
                                              /* devices on this SCSI bus    */
        uchar   any_waiting;                  /* count of device queues      */
                                              /* waiting for resources       */
	uchar   pending_err;                  /* pending target mode errors  */
};


/*
 *  shared virtual device structure. several virtual devices
 *  that have a common parent adapter share this structure
 */
struct	scsi_shared {
        struct scb_filter im_filter;          /* initiator mode ndd filter   */
        struct scb_filter tm_filter;          /* target mode ndd filter      */
	ndd_t	*ndd;			      /* ptr to the adapter's ndd    */
        struct  scsi_shared  *next;           /* pointer to chain            */
        uint   p_unit_no;                     /* unit number of parent adap  */
        uint   num_of_opens;		      /* count of number of opens    */
        uint   num_cfgs;		      /* count number of configured  */
                                              /* virtual scsi for this shared*/
	ulong	dump_state;		      /* status as a dump device     */
};


 /*  device structure, one per started device */
struct  dev_info        {
        struct 	wtimer		wdog;          /* cmd timer - 1 per device   */
        struct	cmd_elem  command_element;     /* cmd_elem use to issue abort*/
                                               /* and resume cmds on intr lvl*/
	struct	sc_buf  	*head_act;     /* active command list head   */
	struct	sc_buf  	*tail_act;     /* active command list tail   */
	struct	sc_buf		*head_pend;    /* pending command list head  */
	struct	sc_buf		*tail_pend;    /* pending command list tail  */
        struct  sc_buf  *cmd_save_ptr;         /* used for check cond error  */
                                               /* recov when cmd tag queuing */
	void   (*async_func)();		       /* async entry point in head  */
        uint            async_correlator;      /* as passed to async event   */
	int	dev_event;		       /* device event word for      */
                                               /* e_sleep                    */
	uchar	num_act_cmds;	               /* number of active cmds for  */
                                               /* this device                */
	uchar	trace_enabled;		       /* zero means trace internal  */
                                               /* trace enabled              */
	uchar	qstate;		               /* state of the command queue */
                                               /* per device                 */
#define	HALTED  	0x01                   /* abort, SCSI reset, BDR, or */
                                               /* resume is in progress      */
#define CDAR_ACTIVE     0x02                   /* command delay after reset  */
                                               /* in progress for this dev   */
#define CAC_ERROR       0x04                   /* contingent alligience error*/
                                               /* recovery in progress       */
#define PENDING_ERROR   0x08                  
#define RESET_IN_PROG   0x10                   /* SCSI bus reset in progress */
#define Q_STARVE        0x20                   /* queue starved for timeout  */
	uchar	stop_pending;		       /* indicates if SCIOSTOP is   */
                                               /* pending for this device    */
        uchar  dev_queuing;                    /* indicates if head is       */
                                               /* ctq for this device        */
        uchar   need_resume_set;               /* indicates if waiting for   */
                                               /* resume indication in sc_buf*/
        uchar   cc_error_state;                /* used for check condition   */
                                               /* error recovery/detection   */
                                               /* when cmd tag queueing      */
        uchar  waiting;                        /* indicates if waiting for   */
                                               /* resources (cmd_elem)       */
        uchar need_to_resume_queue;            /* indicates if VSC_RESUME    */
                                               /* needs to be set to resume  */
                                               /* a frozen device queue      */
        uchar resvd0;                          /* reserved for future use    */
        uchar resvd1;                          /* reserved for future use    */
        uchar resvd2;                          /* reserved for future use    */

};

struct tm_info {
        void            (*recv_func)(); /* recv bufs function in head        */
	void            (*async_func)();/* async entry point in head         */
        uint            async_correlator; /* as passed to async event        */
        int             num_bufs;       /* no. of bufs(high limit) alloced   */
        int             num_to_resume;  /* low water mark for reenable id    */
        int             num_bufs_qued;  /* no. of bufs with data qued        */
        uint            tm_correlator;  /* as passed to start target         */
        uint            buf_size;       /* size of the read buffers          */
        uchar           stopped;        /* False: enabled, True: disabled    */
        uchar           previous_error; /* indicates if error is pending     */
	uchar	        trace_enabled;  /* zero means internal trace enabled */
};

struct  b_link {
        uint    tm_correlator;  /* same as that given to adap driver         */
        dev_t   adap_devno;     /* device major/minor of this adapter        */
        caddr_t data_addr;      /* kernel space addr where data begins       */
        int     data_len;       /* length of valid data in buffer            */
        ushort  user_flag;      /* flags set for the user.  may be one       */
                                /* or more of the following:                 */
#ifndef TM_HASDATA
#define TM_HASDATA      0x04    /* set if this is a valid tm_buf             */
#define TM_MORE_DATA    0x08    /* set if more data coming for this Send     */
#define TM_ERROR        0x8000  /* set if any error occurred on this Send    */
                                /* (mutually exclusive with TM_MORE_DATA)    */
#endif
        uchar   user_id;        /* SCSI id which sent the data               */
        uchar   owner_id;       /* id which malloced this buffer             */
        uchar   status_validity;        /* valid values shown below:         */
/* #define      SC_ADAPTER_ERROR        2  general_card_status is valid      */
        uchar   general_card_status;    /* defined under struct sc_buf       */
        uchar   resvd1;                 /* reserved for expansion            */
        uchar   resvd2;                 /* reserved for expansion            */
        uchar   resvd8;         /* buffer tag used to enable this buffer     */
        uchar   resvd9;         /* 0 => enable, 1 => disable                 */
        ushort  resvd10;        /* owner state flags                         */
#define TM_ALLOCATED    0x01    /* when set, means the buffer is malloced.   */
                                /* (may or may not be in process of being    */
                                /* enabled)                                  */
#define TM_ENABLED      0x02    /* when set, means buffer has been           */
                                /* completely enabled                        */
        struct  scsi_info *sp;  /* pointer to owning scsi_info struct        */
        caddr_t buf_addr;       /* original (saved) buffer address           */
        uint    buf_size;       /* original (saved) buffer length            */
        uint    resvd11;        /* I/O bus DMA address used for buffer       */
        int     resvd12;        /* number of tcws alloced for buffer         */
        int     resvd13;        /* index of starting tcw for buffer          */
        struct  b_link *next;   /* either NULL or link to next b_link        */
                                /* Used when this is on a free list          */
        struct  b_link *forw;   /* NULL or forward link when on rdbuf list   */
        struct  b_link *back;   /* NULL or back link when on rdbuf list      */
        uint    resvd3;         /* reserved for expansion                    */
        uint    resvd4;         /* reserved for expansion                    */
        uint    resvd5;         /* reserved for expansion                    */
        uint    resvd6;         /* reserved for expansion                    */
};


/* structure used for IOCTL SCSI cmds */
struct vsc_buf {
        struct sc_buf      scbuf;
        struct scsi_info   *scsi;
};


struct vsc_cdt_tab {                    /* component dump table struct  */
        struct  cdt_head   vsc_cdt_head;/* header to the dump table     */
        struct  cdt_entry  vsc_entry[MAX_ADAPTERS*2];
                                        /* space for each minor + trace */
};

/* global information concering all instances of vscsi devices               */
typedef struct{
	uint	num_open_scsi;	
	uint	num_cfgs;	
	uint	top_pinned;            /* indicates if top portion is pinned */
        lock_t  vsc_lock;
} vsc_info_t;

vsc_info_t vsc_info = {0, 0, 0, LOCK_AVAIL};

/*
 * Type 1 Parameter Descriptor.
 */
struct type1_pd_def
{
    /* First 32 bit Word                                                */
    uint resvd3   : 16;  /* bits 31 - 16                                */
    uint chain    : 1;   /* Bit is low when no more chained data.       */
    uint resvd2   : 12;  /* bits 14 - 3                                 */
    uint error    : 2;   /* Error code.                                 */
    uint resvd1   : 1;   /* bit  0                                      */
    
    uint size;         /* Size of valid data in bytes                   */
    caddr_t addr_msw;  /* Most significant 32 bit word of data address  */
    caddr_t addr_lsw;  /* Least significant 32 bit word of data address */
};
typedef struct type1_pd_def type1_pd_def_t;

/*
 * Type 2 Parameter Descriptor.
 */
struct type2_pd_def
{
    /* First 32 Bit Word                                     */
    uint resvd3   : 16;  /* bits 31 - 16                     */
    uint complete : 1;   /* Completion bit low when no data. */
    uint resvd2   : 12;  /* bits 14 - 3                      */
    uint error    : 2;   /* Error code.                      */
    uint resvd1   : 1;   /* bit  0                           */
    
    /* Second 32 Bit Word                                    */
    uint resvd6   : 12;  /* bits 31 - 20                     */
    uint pun      : 4;   /* Physical unit number (SCSI id).  */
    uint resvd5   : 5;   /* bit  15 - 11                     */
    uint lun      : 3;   /* Logical unit number              */
    uint resvd4   : 4;   /* bits 7 - 4                       */
    uint bus      : 4;   /* Bus number                       */
    
    uint resvd7;
    uint resvd8;
};
typedef struct type2_pd_def type2_pd_def_t;

/*
 *  Definition for a SCSI Info Event Element.
 */
struct info_event_elem_def 
{
    ctl_elem_hdr_t  header;
    type2_pd_def_t  init_desc;
    type1_pd_def_t  buf_desc[10];
};
typedef struct info_event_elem_def info_event_elem_def_t;

struct error_log_def {                  /* driver error log structure   */
        struct err_rec0 errhead;        /* error log header info        */
        struct rc       data;           /* driver dependent err data    */
};


/* 
 * gwrite structure is used for management of buffers allocated
 * specifically to handle initiator-mode gathered writes
 */
struct	gwrite {
	caddr_t	buf_addr;	/* original (saved) buffer address */
	int	buf_size;	/* original (saved) buffer length */
	uint	dma_addr;	/* I/O bus DMA address used for buffer */
	struct	gwrite *next;	/* either NULL or link to next gwrite */
};


#ifdef VSC_TRACE

struct cmd_trace {
    struct scsi_cdb cdb;
    uchar error;
    uchar scsi_status;
    uchar cmd_error;
    uchar device_error;
};

struct trace_element {
  uint  type;          /*'CMDO'      'CMDI'      'CANO'  'CANI'     */
  union {
      struct cmd_trace       cmd;
      info_event_elem_def_t info_event;
      uint                  scsi_id;
      buf_pool_elem_t       pool_elem;
  }  data;
};

#endif VSC_TRACE

/* the following macro is used to quickly generate the device index.    */
/* the macro assumes "a" is SCSI ID, and "b" is LUN                     */
#define INDEX(a,b)      ((((a) & 0x0F)<<5) | ((b) & 0x1F))

/* this macro returns the scsi id from a previously generated index.    */
#define SID(x)          ((x)>>5)

/* this macro returns the lun id from a previously generated index.     */
#define LUN(x)          ((x) & 0x1F)




#ifndef _NO_PROTO
/*****************************************************************************/
/*     internal functions                                                    */
/*****************************************************************************/

int    vsc_config(dev_t devno, int op, struct uio * uiop);
struct scsi_info *vsc_alloc_scsi(dev_t devno);
struct scsi_shared *vsc_alloc_shared(uint p_unit_no);
void   vsc_free_scsi(struct scsi_info * scsi);
void   vsc_free_shared(struct scsi_shared * shared);
int    vsc_open(dev_t devno, ulong devflag, int chan, int ext);
void   vsc_fail_open(int undo_level, struct scsi_info *scsi, int ret_code, 
                     dev_t devno);
int    vsc_alloc_cmdpool(struct scsi_info *scsi);
void   vsc_init_cancel_cmd_elem(struct scsi_info *scsi, struct cmd_elem *cmd);
struct cmd_elem *vsc_get_cmd_elem(struct scsi_info *scsi);
int    vsc_close(dev_t devno, int chan);
int    vsc_ioctl(dev_t devno, int cmd, int arg, ulong devflag, int chan,
       int ext);
int    vsc_init_dev(struct scsi_info *scsi, int dev_index);
int    vsc_clear_dev(struct scsi_info *scsi, int dev_index);
void   vsc_free_gwrite (struct scsi_info *scsi);
int    vsc_halt_dev(struct scsi_info *scsi, int dev_index);
int    vsc_inquiry(struct scsi_info *scsi, int arg, dev_t devno, int devflag);
int    vsc_issue_bdr(struct scsi_info *scsi, int dev_index);
int    vsc_start_unit(struct scsi_info *scsi, int arg, dev_t devno, 
       int devflag);
int    vsc_test_unit_rdy(struct scsi_info *scsi, int arg, dev_t devno, 
       int devflag);
int    vsc_read_blk(struct scsi_info *scsi, int arg, dev_t devno, int devflag);
int    vsc_register_async(struct scsi_info *scsi, int arg, ulong devflag);
struct vsc_buf *vsc_bld_vsc_buf();
void   vsc_iodone( struct sc_buf * bp);
void   vsc_ioctl_sleep(struct buf *bp, struct scsi_info *scsi);
void   vsc_fail_cmd(struct  scsi_info *scsi, int dev_index);
void   vsc_deq_active(struct  scsi_info *scsi, struct sc_buf *scp, 
       int dev_index);
int    vsc_strategy(struct sc_buf *bp );
int    vsc_init_gw_buf (struct scsi_info *scsi, struct sc_buf *bp,
			int *old_pri);
void   vsc_start (struct  scsi_info *scsi, int dev_index);
int    vsc_start_pending_cmds (struct  scsi_info *scsi, int dev_index);
void   vsc_recv(ndd_t *p_ndd, struct rpl_element_def *reply_elem);
void   vsc_log_err();
void   vsc_process_scsi_reset(struct scsi_info * scsi);
void   vsc_watchdog(struct watchdog * wdog);
void   vsc_scsi_reset(struct scsi_info *scsi);
void   vsc_async_stat(ndd_t *p_ndd, ndd_statblk_t *statblk);
struct cdt *vsc_cdt_func(int arg);
int    vsc_dump(dev_t devno, struct uio * uiop, int cmd, int arg, int chan,
       int ext);
int    vsc_dump_write(struct scsi_info * scsi, struct sc_buf * scp);
int    vsc_dump_start(struct scsi_info * scsi);
int    vsc_alloc_tgt(struct scsi_info *scsi, int arg, ulong devflag);
int    vsc_dealloc_tgt(struct scsi_info *scsi,int arg, ulong devflag);
void   vsc_free_rdbufs (struct scsi_info *scsi, int dev_index);
void   vsc_buf_free (struct tm_buf * tm_ptr);
void   vsc_free_b_link (struct b_link * b_link);
void   vsc_async_notify (struct scsi_info *scsi, int events);
void   vsc_need_disid (struct scsi_info *scsi, uint id);
void   vsc_need_enaid (struct scsi_info *scsi, uint id);
void   vsc_target_receive (struct ndd * adapter_ndd, 
       struct info_event_elem_def *info_event);
int    vsc_alloc_b_link_pool (struct scsi_info * scsi);
struct b_link *vsc_get_b_link (struct scsi_info * scsi);
#ifdef VSC_TRACE
void   vsc_internal_trace(struct scsi_info * scsi, int index, 
       uint * buf, int option, int mode);
#endif  VSC_TRACE

#else
/*****************************************************************************/
/*     internal functions                                                    */
/*****************************************************************************/
int    vsc_config();
struct scsi_info *vsc_alloc_scsi();
struct scsi_shared *vsc_alloc_shared();
void   vsc_free_scsi();
void   vsc_free_shared();
int    vsc_open();
void   vsc_fail_open();
int    vsc_alloc_cmdpool();
void   vsc_init_cancel_cmd_elem();
struct cmd_elem *vsc_get_cmd_elem();
int    vsc_close();
int    vsc_ioctl();
int    vsc_init_dev();
int    vsc_clear_dev();
void   vsc_free_gwrite();
int    vsc_halt_dev();
int    vsc_inquiry();
int    vsc_issue_bdr();
int    vsc_start_unit();
int    vsc_test_unit_rdy();
int    vsc_read_blk();
int    vsc_register_async();
struct vsc_buf *vsc_bld_vsc_buf();
void   vsc_iodone();
void   vsc_ioctl_sleep();
void   vsc_fail_cmd();
void   vsc_deq_active();
int    vsc_strategy();
int    vsc_init_gw_buf();
void   vsc_start ();
int    vsc_start_pending_cmds ();
void   vsc_recv();
void   vsc_log_err();
void   vsc_process_scsi_reset();
void   vsc_watchdog();
void   vsc_scsi_reset();
void   vsc_async_stat();
struct cdt *vsc_cdt_func();
int    vsc_dump();
int    vsc_dump_write();
int    vsc_dump_start();
int    vsc_alloc_tgt();
int    vsc_dealloc_tgt();
void   vsc_free_rdbufs ();
void   vsc_buf_free ();
void   vsc_free_b_link ();
void   vsc_async_notify ();
int    vsc_need_disid ();
int    vsc_need_enaid ();
void   vsc_target_receive ();
int    vsc_alloc_b_link_pool ();
struct b_link *vsc_get_b_link ();
#ifdef VSC_TRACE
void   vsc_internal_trace();
#endif  VSC_TRACE

#endif /* not _NO_PROTO */

#endif /* _H_VSCSIDD */

