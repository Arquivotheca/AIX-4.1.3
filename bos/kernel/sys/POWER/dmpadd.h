/* @(#)51       1.2  src/bos/kernel/sys/POWER/dmpadd.h, sysxdmpa, bos411, 9435C411a 8/31/94 19:02:03 */
/*
 *   COMPONENT_NAME: (MPAINC) MP/A HEADER FILES
 *
 *   FUNCTIONS: COPYIN
 *		COPYOUT
 *		DISABLE_INTERRUPTS
 *		ENABLE_INTERRUPTS
 *		KFREE
 *		KMALLOC
 *		M_INPAGE
 *		PIO_GETC
 *		PIO_PUTC
 *		SLEEP
 *		SWAPLONG
 *		SWAPSHORT
 *		WAKEUP
 *		
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
#ifndef _H_MPADD
#define _H_MPADD


/*
 *
 */

#include <sys/intr.h>
#include <sys/types.h>
#include <sys/lockl.h>
#include <sys/iocc.h>

/***************************************************************************
* All commands are sent to the 8273 via the WR_CMD_OFFSET reg              *
* The command phase is not complete until the command and all              *
* required parameters have been sent to the adapter and the                *
* CBSY bit has returned to 0. The following structure is used              *
* to send commands.                                                        *
***************************************************************************/
/* trace hook for mpa diag driver */
#define HKWD_DD_MPADD             0x35100000    /* MPA single port */

#ifndef ERRID_MPA_ERROR
#define ERRID_MPA_ERROR      0xacab5df2         /* MPA */
#endif

#define KMALLOC(dtyp)   (dtyp *)xmalloc(sizeof(dtyp), 2, pinned_heap)
#define KFREE(buf)              xmfree((buf), pinned_heap)
#define OPENP           acb->open_struct
#define DISABLE_INTERRUPTS(lvl) lvl=i_disable(INTCLASS2)
#define ENABLE_INTERRUPTS(lvl)  i_enable(lvl)
#define SLEEP(el)       e_sleep (el, EVENT_SIGRET)
#define WAKEUP(el)      e_wakeup (el)
#define DDS             acb->mpaddp
#define SLOT            (DDS.slot_num)
#define ARB             (DDS.dma_lvl)
#define MPA_IOCC_ATT    (ulong)(IOCC_ATT(DDS.bus_id,IO_IOCC+(SLOT << 16)))
#define MPA_CHAN_STAT   (ulong)(IOCC_ATT(DDS.bus_id,0x004F0060))
#define MPA_DMA_STAT    (ulong)(IOCC_ATT(DDS.bus_id,IO_IOCC+(ARB << 16)+0x60))
#define MPA_CCR_ATT     (ulong)(IOCC_ATT(DDS.bus_id,IO_IOCC+0x2C))
#define MPA_BUSIO_ATT   (ulong)(BUSIO_ATT(DDS.bus_id, DDS.io_addr))
#define CBSY_0           2
#define CRBF_1           3
#define CPBF_0           4
#define DD_EXIT_OFFL     0xC0     /* trace hook word for offlevel exit */
#define DD_ENTRY_OFFL    0xC1     /* trace hook word for offlevel entry */
#define MPA_RECV_ENAB    0xC2     /* trace hook word for recv enabled   */
#define MPA_RECV_Q       0xC3     /* trace hook word for recv on dma q  */
#define MPA_RECV_DISAB   0xC4     /* trace hook word for recv disabled  */
#define MPA_RECV_D_Q     0xC5     /* trace hook word for recv off dma q */
#define MPA_XMIT_DATA    0xC6     /* trace hook word for xmit data check*/


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
	struct mbuf *rc_mbuf_head;      /* head of mbuf chain rcv data */
	struct mbuf *rc_mbuf_tail;      /* tail of mbuf chain rcv data */
	uchar    rc_flags;              /* same values as xm_flags     */
	int rc_count;                   /* number of bytes received */
	int rc_index;                   /* points to next byte to read */
	caddr_t rc_data;                /* mbuf data pointer.          */
	uchar rc_state;                 /* active, complete, aborted */
#define RC_ACTIVE              0x01
#define RC_COMPLETE            0x02
#define RC_ABORTED             0x04
	struct recv_elem *rc_next;      /* pointer to next element */
} recv_elem_t;

typedef struct stat_elem {
	cio_stat_blk_t stat_blk;                /* common I/O status block */
	uchar  st_state;
#define ST_ACTIVE              0x01
	struct stat_elem *st_next;    /* pointer to the next status element */
} stat_elem_t;

typedef struct xmit_elem {
	struct mbuf *xm_mbuf;       /* pointer to mbufs containing data */
	uchar    xm_flags;
#define USE_PIO               0x01
	int      xm_index;          /* used for pio xfers to index mbuf data */
	caddr_t  xm_data;           /* mbuf data pointer */
	netid_t  xm_netid;
	int xm_length;                          /* total length of data */
	uchar xm_ack;                           /* transmit acknowledgement flag */
	ulong xm_writeid;                       /* writeid from caller */
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


/*************************************************************************
 *  Multi Protocol Single Port Device Driver Defines                     *
 ************************************************************************/

#define ACMD_ACQ        (unsigned short)0
#define IOCC_SEG_REG    0x02000000
#define IO_SEG_REG      0x820c0020
#define MAX_ADAPTERS    2               /* number of adapters supported */
#define MPA_MAX_OPENS   1

#define POS0           0x100            /* POS Register 0 IOCC offset   */
#define P0_F            0xFF            /* POS Card ID low, MPA        */
#define POS1           0x101            /* POS Register 1 IOCC offset   */
#define P1_F            0xDE            /* POS1 Card ID high, MPA      */

#define POS2           0x102            /* POS Register 2 IOCC offset   */
#define P2_ENABLE       0x01            /* -sleep/+ENABLE               */
#define P2_SDLC_MODE    0x10            /* Set SDLC mode                */
#define P2_ALT_SDLC     0x12            /* Set alternate SDLC mode      */
#define P2_DMA_ENABLED  0x20            /* Set if dma enabled for SDLC mode*/
#define P2_NOT_V25      0x40            /* 0 if V.25 bis exits*/

#define POS3           0x103            /* POS Register 3 IOCC offset   */
#define P3_ARB_1        0x01            /* arb level for SDLC */
#define P3_ARB_7        0x07            /* arb level for ALT SDLC */


/***************************************************************************
*                 Receive commands  parameters and results                 *
***************************************************************************/

#define GEN_RECEIVE_CMD      0xC0    /* cmd to to general receive          */
				     /* parms are LSB then MSB of recv len */
#define SEL_RECEIVE_CMD      0xC1    /* cmd to to selective receive        */
				     /* parms are LSB then MSB of recv len */
				     /* then match addr 1, match addr 2    */
#define LOOP_RECEIVE_CMD     0xC2    /* cmd to to selective loop receive   */
				     /* parms are LSB then MSB of recv len */
				     /* then match addr 1, match addr 2    */
#define DISABLE_RECV_CMD     0xC5    /* cmd to to disable the receiver     */


#define RIC_MASK_LAST_BYTE   0x1F    /* used to and with RIC to remove the */
				     /* last byte bit count                */


/***************************************************************************
*                 Transmit commands  parameters and results                *
***************************************************************************/

#define XMIT_CMD             0xC8    /* cmd to to xmit                     */
				     /* parms are LSB then MSB of xmit len */
				     /* in buffered mode send A, and C too */
#define XMIT_ABORT_CMD       0xCC    /* cmd to abort xmit                  */
				     /* No parms.                          */
#define LOOP_XMIT_CMD        0xCA    /* cmd to do loop xmit                */
				     /* parms are LSB then MSB of xmit len */
				     /* in buffered mode send A, and C too */
#define LOOP_ABORT_CMD       0xCE    /* cmd to abort loop xmit             */
				     /* No parms.                          */
#define TRANSPARENT_XMIT_CMD 0xC9    /* cmd to do transparent xmit         */
				     /* parms are LSB then MSB of xmit len */
#define TRANSPARENT_ABORT_CMD 0xCD   /* cmd to do transparent xmit         */
				     /* No parms.                          */


/***************************************************************************
*              Status reg definitions. Status read from                    *
*                      RD_STAT_OFFSET                                      *
***************************************************************************/
#define TX_RESULT_READY        0x01  /* 1 = Tx result in TxI/R            */
#define RX_RESULT_READY        0x02  /* 1 = Rx result in RxI/R            */
#define TX_IRPT_ACTIVE         0x04  /* 1 = xmit irpt active              */
#define RX_IRPT_ACTIVE         0x08  /* 1 = recv irpt active              */
#define CRBF                   0x10  /* 1 = command result buffer full    */
#define CPBF                   0x20  /* 1 = command parm buffer full      */
#define CBF                    0x40  /* 1 = command buffer full           */
#define CBSY                   0x80  /* 1 = in command phase              */
#define IRPT_PENDING           0x0C  /* to see if there is a TX or RX     */
				     /* interrupt in the status reg       */

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
	int int_lvl;            /* bus interrupt level */
	int intr_priority;      /* System interrupt priority */
	int dma_lvl;            /* to set arbitration level */

	/* DEVICE SPECIFIC INFORMATION */
	char resource_name[16];         /* Device logical name */
	ushort card_id;                 /* Card ID returned by POS registers */
	char mode[8];                   /* sdlc or bsc mode */
	int rdto;                       /* receive data transfer offset */
};

/*------------------------------------------------------------------------*/
/*  Adapter Control Block definition:                                     */
/*------------------------------------------------------------------------*/

typedef struct acb
{
	struct intr       caih_struct;     /* irpt struct for level 3     */
					   /* must be at the top of acb   */
	struct intr       ofl;            /* offlevel interrupt structure */
	open_t            open_struct;     /* the open struct */
	ulong flags;
#define RECEIVER_ENABLED  0x00000001
#define STARTED_CIO       0x00000002
#define MPAINIT           0x00000004      /* Driver has been initialized */
#define OPEN_DIAG         0x00000008
#define MPA_CE_OPEN       0x00000010
#define MPADMACHAN        0x00000020
#define MPAIINSTALL3      0x00000040
#define MPADEAD           0x00000080
/* available              0x00000100      */
#define NEED_IRPT_ELEM    0x00000200
#define NEED_RECV_ELEM    0x00000400
#define NEED_DMA_ELEM     0x00000800
#define RC_GET_PTRS       0x00001000
#define RC_FREE_DMA       0x00002000
#define RC_FREE_RECV      0x00004000
#define RC_START_RECV     0x00008000
#define RC_WAIT_IDLE      0x00010000
#define RC_DISCARD        0x00020000
#define RC_TAP_USER       0x00040000
#define RC_SAVE_CNT       0x00080000
#define MPA_IRPT          0x00100000
#define RECV_DMA_ON_Q     0x00200000
#define PIO_MODE          0x00400000     /* xfers use pio when set */
#define NO_RECV           0x00800000     /* xfers use pio when set */
#define XM_DISCARD        0x01000000
#define IRPT_QUEUED       0x02000000
#define IRPT_PIO_ERR      0x04000000

	irpt_elem_t   *irpt_free;        /* free iprt list pointer     */
	irpt_elem_t   *act_irpt_head;    /* head of irpt results q     */
	irpt_elem_t   *act_irpt_tail;    /* tail of irpt results q     */
#define MAX_IRPT_QSIZE    10
	recv_elem_t     *recv_free;        /* free recv list pointer     */
	recv_elem_t     *act_recv_head;    /* head of recv q             */
	recv_elem_t     *act_recv_tail;    /* tail of recv q             */
#define MAX_RECV_QSIZE     20
	xmit_elem_t     *xmit_free;        /* free xmit list pointer     */
	xmit_elem_t     *act_xmit_head;    /* head of xmit q             */
	xmit_elem_t     *act_xmit_tail;    /* tail of xmit q             */
#define MAX_XMIT_QSIZE     20
	dma_elem_t      *dma_free;          /* free dma list pointer      */
	dma_elem_t      *act_dma_head;      /* head of dma q              */
	dma_elem_t      *act_dma_tail;      /* tail of dma q              */
	dma_elem_t      *hold_recv;         /* hold recvs while xmits work*/
#define MAX_DMA_QSIZE     20
	stat_elem_t     *stat_free;        /* free stat list pointer     */
	stat_elem_t     *act_stat_head;    /* head of stat q             */
	stat_elem_t     *act_stat_tail;    /* tail of stat q             */
#define MAX_STAT_QSIZE     20
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
	int             op_rcv_event;   /* used for waiting for receive data */
	dev_t           dev;
	lock_t          adap_lock;
	struct  mpadds  mpaddp;         /* dds struct */
	int             num_opens;
	mpa_query_t     stats;              /* DD statistics */
	mpa_start_t     strt_blk;        /* start session parms from user */
	uchar           station_type;    /* flag for secondary or primary */
	uchar           station_addr;    /* address if secondary station */
	char            err_text[64];    /* for errlog text messages */
	char            first_1[16];    /* for errlog text messages */
	struct acb *next;                /* pointer to next adapter struct */
} acb_t;

/**************************************************************************
 *   MPA MACRO DEFINITIONS                                               *
 **************************************************************************/

/* Little Endian <--> Big Endian conversion: */

# define SWAPSHORT(x)   (((((unsigned short)(x)) & 0xFF) << 8) |        \
			  (((unsigned short)(x)) >> 8))

# define SWAPLONG(x)    (((((unsigned long)(x)) &       0xFF) << 24) |  \
			 ((((unsigned long)(x)) &     0xFF00) <<  8) |  \
			 ((((unsigned long)(x)) &   0xFF0000) >>  8) |  \
			 ((((unsigned long)(x)) & 0xFF000000) >> 24))

/*----------------------------------------------------------------------*/
/*  M_INPAGE  for checking funky mbufs                                  */
/*  This macro determines if the data portion of an mbuf resides within */
/*  one page -- if TRUE is returned, the data does not cross a page     */
/*  boundary.  If FALSE is returned, the data does cross a page         */
/*  boundary and cannot be d_mastered.                                  */
/*----------------------------------------------------------------------*/

# define M_INPAGE(m)    ((((int)MTOD((m), uchar *)                      \
				& ~(PAGESIZE - 1)) + PAGESIZE) >        \
				    ((int)MTOD((m), uchar *) + (m)->m_len))


/*------------------------------------------------------------------------*/
/*  These BUS accessors are PIO-recovery versions of the original BUS     */
/*  accessor macros.  The essential difference is that retries are        */
/*  performed if pio errors occur; if the retry limit is exceeded, a -1   */
/*  is returned (hence all return an int value).  In the cases of         */
/*  PIO_GETL and PIO_GETLR, the -1 is indistinguishable from all FF's so  */
/*  some heuristic must be used to determine if it is an error (i.e., is  */
/*  all FF's a legitimate read value?).                                   */
/*------------------------------------------------------------------------*/

# define C              1       /* Character type of PIO access */
# define S              2       /* Short type of PIO access */
# define SR             3       /* Short-reversed type of PIO access */
# define L              4       /* Long type of PIO access */
# define LR             5       /* Long-reverse type of PIO access */

# define PIO_GETC( p, a )          ((int) PioGet( p, a ))

# define PIO_PUTC( p, a, v )       ((int) PioPut( p, a, v ))


/*
** Macros to get/put caller's parameter block
** Useful for "arg" in ioctl and for extended paramters on other dd entries.
** Value is 0 if ok, otherwise EFAULT.
*/
#define COPYIN(dvf,usa,dda,siz)                               \
( (usa == NULL) ? (EFAULT) :                                  \
      ( (dvf & DKERNEL) ? (bcopy(usa,dda,siz), 0) :           \
	    ( copyin(usa,dda,siz) ) ) )

#define COPYOUT(dvf,dda,usa,siz)                              \
( (usa == NULL) ? (EFAULT) :                                  \
      ( (dvf & DKERNEL) ? (bcopy(dda,usa,siz), 0) :           \
	    ( copyout(dda,usa,siz) ) ) )


/***************************************************************************
 *  Internal Port States...see port_state variable in dds device section   *
 ***************************************************************************/

#define DORMANT_STATE           0x00    /* initial state */
#define OPEN_REQUESTED          0x01    /* Open in progress */
#define OPEN                    0x02    /* Port opened */
#define START_REQUESTED         0x03    /* Start in progress */
#define STARTED                 0x04    /* Port started */
#define DATA_XFER               0x04    /* Data tranfer state */
#define HALT_REQUESTED          0x05    /* Halt in progress */
#define HALTED                  0x02    /* Port halted */
#define CLOSE_REQUESTED         0x07    /* Close requested */
#define CLOSED                  0x00    /* Port closed */

/**************************************************************************
 *   MPQP TRACE HOOK CONSTANTS                                            *
 **************************************************************************/

#define PORT_NOT_OPEN           0xfe            /* Port State != OPEN */
#define PORT_NOT_STARTED        0xff            /* Port State != STARTED */
#define PIN_CODE_FAIL           0x100           /* pincode attempt failed */
#define ADD_ENTRY_FAIL          0x101           /*      */
#define PORT_ALRDY_OPEN         0x102           /*      */
#define POS_REG_FAIL            0xfa            /*      */


#endif /* _H_MPADD */
