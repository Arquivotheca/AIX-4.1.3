/* @(#)47	1.3  src/bos/kernel/sys/POWER/mpadd.h, mpainc, bos411, 9428A410j 6/24/93 07:59:29 */
/*
 *   COMPONENT_NAME: (MPAINC) MP/A HEADER FILES
 *
 *   FUNCTIONS: 
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

/*
 *
 */

#include <sys/types.h>
#include <sys/intr.h>
#include <sys/lockl.h>
#include <sys/iocc.h>
#include <sys/termio.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/err_rec.h>
#include <sys/mpqp.h>
#include <sys/mpadefs.h>

/*------------------------------------------------------------------------*/
/*  Device Definitions Structure:                                         */
/*------------------------------------------------------------------------*/

typedef struct mpadds
{
	/* BUS INFORMATION */
	ulong bus_id;   /* I/O bus ID (to pass to IOCC_ATT and BUSMEM_ATT) */
	int   bus_type; /* Bus type (BUS_MICRO_CHANNEL) for intr.h struct */

	/* ADAPTER INFORMATION */
	int slot_num;           /* microchannel slot card found in */
	ulong io_addr;          /* I/O base address. */
	int int_lvlA;           /* bus interrupt level A */
	int int_lvlB;           /* bus interrupt level B */
	int intr_priority;      /* System interrupt priority */
	int dma_lvl;            /* to set arbitration level */

	/* DEVICE SPECIFIC INFORMATION */
	char resource_name[16];         /* Device logical name */
	ushort card_id;                 /* Card ID returned by POS registers */
	unsigned short rdto;            /* receive data transfer offset */
};

#ifndef _H_MPADD
#define _H_MPADD

typedef struct errormsg { 
        struct  err_rec0 err;
        char    file[32];
        int     data1;       /* use data1 and data2 to show detail  */
        int     data2;       /* data in the errlog report. Define   */
                             /* these fields in the errlog template */
                             /* These fields may not be used in all */
                             /* cases.                              */
} ;

/******************************************************************************
 *   MP/A SDLC Internal Trace Table
 ***************************************************************************/
typedef struct mpa_trace_tag
{
        int     next;   /* next index hole to put trace data in the table */
        int     res1;
        int     res2;
        int     res3;
        ulong   table[MPA_TRACE_SIZE];
}mpa_trace_t;

/******************************************************************************
 *   MSQP card register state structure.                                   *
 ***************************************************************************/
typedef struct Q_STATE
{
    uchar     port_a_8255;
    uchar     port_b_8255;
    uchar     port_c_8255;
    uchar     mode_sel_8255;
    int       cnt_0_8254;           /* not used */
    int       cnt_1_8254;           /* not used */
    int       cnt_2_8254;           /* not used */
    uchar     statreg_8273;
    uchar     resreg_8273;
    uchar     tx_ir_8273;
    uchar     rx_ir_8273;
    uchar     port_a_8273;
    uchar     port_b_8273;
    uchar     oper_mode_8273;    /* these last fout regs are internal to */
    uchar     serial_io_8273;    /* 8273 and can't be read, so I set up  */
    uchar     one_bit_8273;      /* these values as I set up these regs  */
    uchar     data_xfer_8273;
} card_state_t;
/*
** Device-specific statistics
** Returned in driver statistics structure.
*/
typedef struct mpa_stats {
        ulong sta_que_overflow;         /* status lost, full status que */
        ulong recv_lost_data;           /* receive packet lost*/
        ulong total_intr;               /* total interrupts */
        ulong recv_not_handled;         /* interrupts not handled, */
        ulong xmit_not_handled;         /* interrupts not handled, */
        ulong recv_irpt_error;          /* interrupts with no results */
        ulong xmit_irpt_error;          /* interrupts with no results */
        ulong recv_intr_cnt;            /* number of receive interrupts */
        ulong xmit_intr_cnt;            /* number of transmit interrupts */
        ulong rec_no_mbuf;              /* no mbuf available */
        ulong xmit_sent;                /* transmit commands send to MPA */
        ulong xmit_dma_completes;       /* transmit dma transfers completed */
        ulong recv_sent;                /* receive dma transfers completed */
        ulong recv_dma_completes;       /* receive dma transfers completed */
        ulong recv_crc_errors;
        ulong recv_aborts;
        ulong recv_idle_detects;
        ulong recv_eop_detects;
        ulong recv_frame_to_small;
        ulong recv_dma_overruns;
        ulong recv_buf_overflow;
        ulong recv_cd_failure;
        ulong recv_irpt_overruns;
        ulong xmit_early_irpts;
        ulong xmit_completes;
        ulong xmit_dma_underrun;
        ulong xmit_cts_errors;
        ulong xmit_aborts;
        ulong recv_completes;
        ulong io_irpt_error;
        int   bps_rate;
        ulong recv_pio_byte;
        ulong xmit_pio_byte;
        ulong irpt_fail;
        ulong irpt_succ;
} mpa_stats_t;
/*
** Driver statistics structure.
** Returned by CIO_QUERY ioctl operation
*/
typedef struct QUERY {
        cio_stats_t cc;                 /* General COMIO statistics */
        mpa_stats_t ds;                 /* Device specific statistics */
} mpa_query_t;


/***************************************************************************
* All commands are sent to the 8273 via the WR_CMD_OFFSET reg              *
* The command phase is not complete until the command and all              *
* required parameters have been sent to the adapter and the                *
* CBSY bit has returned to 0. The following structure is used              *
* to send commands.                                                        *
***************************************************************************/
typedef struct COMMAND {
      uchar cmd;
      uchar parm[4];
      int parm_count;
      uchar flag;
#define RETURN_RESULT        0x01    /* Set if result expected. */
      uchar result;
} cmd_phase_t;

typedef struct FRAME {
      uchar  addr;
      uchar  cntl;
      uchar  data[MAX_FRAME_SIZE - 2];
} frame_t;

/*------------------------------------------------------------------------*/
/*  Offlevel Interrupt definition:                                        */
/*------------------------------------------------------------------------*/

typedef struct OFFL_INTR
{
        struct intr     offl_intr;      /* system wide bit */
        struct acb      *p_acb_intr;    /* pointer to acb for i_sched */
} t_offl_intr;


typedef struct irpt_elem {
      uchar ip_state;
#define IP_ACTIVE              0x01
#define IP_COMPLETE            0x02
#define IP_ABORTED             0x04

      uchar type_flag;
#define RECV_RESULT  0x01
#define XMIT_RESULT  0x02
#define MPA_RESET    0x04
      union {
	     struct recv_result {
		uchar RIC;
		uchar R0;
		uchar R1;
		uchar ADR;
		uchar CNTL;
	     } rcv;
	     uchar TIC;
      } tp;
      struct irpt_elem   *ip_next;
} irpt_elem_t;

typedef struct recv_elem {
	struct mbuf *rc_mbuf;           /* mbuf for rcv data */
	uchar    rc_flags;              /* same values as xm_flags     */
	int rc_count;                   /* number of bytes received */
	int rc_index;                   /* points to next byte to read */
	caddr_t rc_data;                /* mbuf data pointer.          */
	uchar rc_state;                 /* active, complete, aborted, or */
					/* has an MBUF to be freed       */
#define RC_ACTIVE              0x01
#define RC_COMPLETE            0x02
#define RC_ABORTED             0x04
#define RC_MBUF                0x08     /* added for defect #091440 */
	struct recv_elem *rc_next;      /* pointer to next element */
} recv_elem_t;

typedef struct xmit_elem {
	struct mbuf *xm_mbuf;       /* pointer to mbufs containing data */
	uchar    xm_flags;
#define RESTART_RECV          0x01  /* Indicate to restart receiver */
#define XMIT_FINAL            0x02  /* Poll/Final Bit mask */
	int      xm_index;          /* used for pio xfers to index mbuf data */
	caddr_t  xm_data;           /* mbuf data pointer */
	netid_t  xm_netid;
	int xm_length;              /* total length of data */
	uchar xm_ack;               /* transmit acknowledgement flag */
	ulong xm_writeid;               /* writeid from caller */
	uchar xm_state;                 /* active, complete, aborted */
#define XM_ACTIVE              0x01
#define XM_COMPLETE            0x02
#define XM_ABORTED             0x04
	struct xmit_elem *xm_next;      /* pointer to next element  */
} xmit_elem_t;


/*
** The dma request structure describes a DMA transfer
*/
typedef struct dma_req {
	union {
		xmit_elem_t *xmit_ptr;  /* pointer to xmit element */
		recv_elem_t *recv_ptr;  /* pointer to receive element */
	} p;                                            /* pointer to transmission request */
	uchar dm_req_type;                      /* request type -- xmit or recv */
#define DM_RECV                 1
#define DM_XMIT                 2
	uchar dm_state;                         /* state of this request */
#define DM_FREE                 0
#define DM_READY                1
#define DM_STARTED              2
#define DM_ABORTED              3
	caddr_t dm_buffer;              /* pointer to device driver buffer */
	int dm_length;                  /* length of data to transfer */
	ushort dm_flags;                /* direction of transfer etc */
	struct xmem *dm_xmem;           /* cross memory descriptor */
	uchar adr;                      /* for buffered mode writes */
	uchar cntl;                     /* for buffered mode writes */
	struct dma_req *dm_next;        /* pointer to next DMA request */
} dma_elem_t;

typedef struct open_struc {
	struct kopen_ext    mpa_kopen;  /* struct kopen_ext passed to open */
	uchar op_flags;                         /* flag bits for this open */
#define OP_FREE                         0       /* open structure is free */
#define OP_OPENED                       1       /* open structure is valid */
#define XMIT_OWED                       2       /* Need to call (*op_xmit_fn)() */
#define LOST_STATUS                     4       /* The status queue has been overrun */
#define SENT_LOST_STATUS                8       /* Sent lost status notification */
#define OP_CE_OPEN                   0x10  /* This open was in '/C' mode */
	ushort op_select;                       /* used for select events */
	ulong op_mode;                          /* mode passed to open() */
						/* defined in fcntl.h    */
	chan_t op_chan;                         /* chan for this open    */
	pid_t  op_pid;
} open_t;


/*------------------------------------------------------------------------*/
/*  Adapter Control Block definition:                                     */
/*------------------------------------------------------------------------*/

typedef struct acb
{
	struct intr       ih_structA;     /* irpt struct for level 3     */
	struct intr       ih_structB;     /* irpt struct for level 4     */
					  /* NOTE: do not mv intr structs*/
					  /*       see mpa_intrB routine */
        t_offl_intr       ofl;            /* offlevel interrupt structure */
	open_t            open_struct;    /* the open struct */
	ulong flags;
#define RECEIVER_ENABLED     0x00000001
#define STARTED_CIO          0x00000002
#define MPAINIT              0x00000004      /* Driver has been initialized */
#define RCV_TIMER_ON         0x00000008      /* Receive timer is ON */
#define XMIT_TIMER_ON        0x00000010      /* Transmit timer is ON */
#define MPADMACHAN           0x00000020
#define MPAIINSTALL3         0x00000040	  /* installed level 3 irpt handler */
#define MPADEAD              0x00000080
#define WAIT_FOR_RI          0x00000100 	  /* Wait for Ring Indicator */
#define NEED_IRPT_ELEM       0x00000200
#define NEED_RECV_ELEM       0x00000400
#define NEED_DMA_ELEM        0x00000800
#define RC_GET_PTRS          0x00001000
#define RC_FREE_DMA          0x00002000
#define RC_FREE_RECV         0x00004000
#define RC_START_RECV        0x00008000
#define RC_WAIT_IDLE         0x00010000
#define RC_DISCARD           0x00020000
#define RC_TAP_USER          0x00040000
#define RC_SAVE_CNT          0x00080000
#define END_XMIT             0x00100000     /* End the Transmit phase */
#define RECV_DMA_ON_Q        0x00200000
#define MPAIINSTALL4         0x00400000     /* installed level 4 irpt handler  */
#define WAIT_FOR_EARLY_XMIT  0x00800000  
#define XM_DISCARD           0x01000000
#define IRPT_QUEUED          0x02000000
#define IRPT_PIO_ERR         0x04000000
#define WAIT_FOR_DSR	     0x08000000	 /* waiting for DSR to activate line */
/* available                 0x10000000   */
/* available                 0x20000000   */
/* available                 0x40000000   */
/* available        	     0x80000000	  */

	irpt_elem_t   *irpt_free;        /* free iprt list pointer     */
	irpt_elem_t   *act_irpt_head;    /* head of irpt results q     */
	irpt_elem_t   *act_irpt_tail;    /* tail of irpt results q     */
#define MAX_IRPT_QSIZE     40
	recv_elem_t     *recv_free;        /* free recv list pointer     */
	recv_elem_t     *act_recv_head;    /* head of recv q             */
	recv_elem_t     *act_recv_tail;    /* tail of recv q             */
#define MAX_RECV_QSIZE     40
	xmit_elem_t     *xmit_free;        /* free xmit list pointer     */
	xmit_elem_t     *act_xmit_head;    /* head of xmit q             */
	xmit_elem_t     *act_xmit_tail;    /* tail of xmit q             */
#define MAX_XMIT_QSIZE     40
	dma_elem_t      *dma_free;          /* free dma list pointer      */
	dma_elem_t      *act_dma_head;      /* head of dma q              */
	dma_elem_t      *act_dma_tail;      /* tail of dma q              */
	dma_elem_t      *hold_recv;         /* hold recvs while xmits work*/
#define MAX_DMA_QSIZE     40
	unsigned char   pos0;           /* POS Register 0 Value */
	unsigned char   pos1;           /* POS Register 1 Value */
	unsigned char   pos2;           /* POS Register 2 Value */
	unsigned char   pos3;           /* POS Register 3 Value */
	int             dma_channel;    /* DMA Channel ID returned from  */
					/* d_init call                   */
	cmd_phase_t     cmd_parms;      /* command struct */
	card_state_t    state;          /* struct to hold card reg values */
	int             mbuf_event;
	int             dmabuf_event;
	int             xmitbuf_event;
	int             irptbuf_event;
	int             op_rcv_event;    /* used for waiting for receive data */
	int             que_cmd_event;   /* used for waiting for que'ed cmds */
	dev_t           dev;
	lock_t          adap_lock;
	struct  mpadds  mpaddp;          /* dds struct */
	int             num_opens;
	mpa_query_t     stats;           /* DD statistics */
	t_start_dev     strt_blk;        /* start session parms from user */
	uchar           station_type;    /* flag for secondary or primary */
	uchar           station_addr;    /* address if secondary station */
	char            err_text[64];    /* for errlog text messages */
	char            first_1[16];     /* for errlog text messages */
	int             total_bytes;     /* bytes for current receiver cmd */
	struct acb *next;                /* pointer to next adapter struct */
} acb_t;

#endif /* _H_MPADD */

