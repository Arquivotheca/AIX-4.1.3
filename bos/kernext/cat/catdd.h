/* @(#)61  1.41  src/bos/kernext/cat/catdd.h, sysxcat, bos411, 9428A410j 3/3/94 19:15:21 */
/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS: catdd.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_CATDD
#define _H_CATDD


#include <sys/intr.h>           /* interrupt handler structure */
#include <sys/device.h>
#include <sys/types.h>
#include <sys/adspace.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/watchdog.h>
#include <sys/err_rec.h>
#include <sys/lock_def.h>
#include <sys/timer.h>
#include <sys/errids.h>
#include <sys/sleep.h>
#include <sys/ioacc.h>
#include "pscadefs.h"           /* need this for STATAREA typedef */
#include <sys/catuser.h>        /* need this for cadds structure decl */
#include "catproto.h"

#ifdef DEBUG
extern int cadebug;
#define CATDEBUG(args)                  { if (cadebug) printf args; }
#define CATDEBUGL(level,args)   { if (cadebug>=level) printf args; }
#else
#define CATDEBUG(args)
#define CATDEBUGL(level,args)
#endif

/*
** This is defined in /usr/include/sys/trchkid.h now.
** #define HKWD_DD_CATDD        0x35000000
*/

/*
** Device specific trace location identifiers
*/
#define PSCA_INTR               0xc0            /* trace loc word for clean_queue */
#define PSCA_SEND               0xc1            /* trace loc word for send_elem */
#define PSCA_CMD                0xc2
#define PSCA_LINK               0xc3
#define PSCA_SUBC               0xc4
#define PSCA_GEN                0xc5

#define PSCA_TRC_ENTRY  0xd0            /* trace loc word for internal func */
#define PSCA_TRC_EXIT   0xd1            /* trace loc word for internal func */

#define DMA_MAGIC_SPOT  STARTDMA
#define CAT_TIMEOUT             25
#define MAX_ADAPTERS    8

/*************************************************************************
* These are constants that are specific to the hardware or to the POST   *
* of the PSCA card.  They could be different for different versions of   *
* the card.                                                              *
*************************************************************************/
#define DL_WAITING      0x01    /* OPERLVL: board is waiting for download */
#define DL_CMD          0x02    /* OPERLVL: code ready to copy to dram */
#define DL_CONFIRM      0x03    /* OPERLVL: copying to dram */
#define DL_FINISH       0x04    /* ?? OPERLVL: download complete and good */
#define DL_CRCERR       0x05    /* ?? OPERLVL: download complete and bad */
#define GOT_INTR        0x99    /* ?? in ERRCODE:       when interrupt rcvd */
#define POST_RESULT     0x00    /* result from diagnostic routines on pca */

#define OPERLVL         0x18    /* offset: board status register */
#define READY           0x19    /* offset: microcode status register */
#define INTMCI          0x1a    /* offset: MCI wants interrupts */
#define ERRCODE         0x66    /* offset: diag ucode acks int here */

#define SFB_TYPE                0       /* send free buffer type */
#define SFBLIST_TYPE    1       /* control free buffer containing */
                                                        /* a list of send free buffers type */
#define CFB_TYPE                2       /* control free buffer type */

/* Miscellaneous Maximums */
#define CAT_MAX_OPENS           64
#define MAX_SUBCHAN                     CAT_MAX_SC
#define MAX_BUF_LIST            32
#define MAX_CDT_ELEMS           ((MAX_ADAPTERS * 7) + 1)
                                /* # * (struct ca, recv_elems, xmit_elems, */
                                /* dma_elems, status/fifo area) + trc_table */
#define MAX_PCA_XMIT_LEN        64*1024
#define MAX_RETRIES                     20
#define MBUF_MS_WAIT            3000    /* number of milliseconds to wait */
#define MAX_RECV_ELEMS          CAT_MAX_SC * MAX_LINKS
#define MAX_RECV_BUFS           208
#define MAX_DMA_ELEMS           208

#define CDT_FIFO_OFFSET         0               /* Offset of SRAM to put in CDT */
#define CDT_FIFO_SIZE           0x3000  /* Lenght of SRAM to put in CDT */

#define NO_FAIL                         0x80    /* change for diagnostic tests */

/*
** Miscellaneous Macros
*/

#ifdef DEBUG
#define MAX_FT_ENTRIES          2048
struct ft {int ft_index; int ft_entries[MAX_FT_ENTRIES]; };
extern struct ft func_trace;
#define F_TRACE(x)      {\
        func_trace.ft_entries[func_trace.ft_index] = x; \
        if (++func_trace.ft_index == MAX_FT_ENTRIES) \
                func_trace.ft_index = 0; \
}
#else
#define F_TRACE(x)
#endif /* DEBUG */


#define KMALLOC(dtyp)   (dtyp *)xmalloc(sizeof(dtyp), 2, pinned_heap)

#define KFREE(buf)              xmfree((buf), pinned_heap)
#define ALIGN32(x)              (((x)+3)&0xFFFFFFFC)


#define DISABLE_INTERRUPTS(lvl) \
								lvl=i_disable(ca->ofl.priority);

#define ENABLE_INTERRUPTS(lvl)  i_enable(lvl);
								
		

#define DISABLE_INTERRUPTSmax(lvl) lvl=i_disable(INTMAX)
#define ENABLE_INTERRUPTSmax(lvl)  i_enable(lvl)

#define LOCK(lvl) 	\
					if ( ! ca->in_kernel ) { \
					lvl = lockl(&ca->locking_adap_lock,LOCK_SHORT ) ;  \
					cat_gen_trace("LOCK",ca->locking_adap_lock,__LINE__,FNUM); \
					} 

#define UNLOCK(lvl)  \
					if ( ! ca->in_kernel) { \
					cat_gen_trace("UNLK",ca->locking_adap_lock,__LINE__,FNUM); \
					if (lvl != LOCK_NEST){ \
						  unlockl(&ca->locking_adap_lock);  \
						 if ( ca->flags & CATIINSTALL) i_sched(&(ca->ofl)); \
						} \
					}
#define OFFSET(p,q)      (&((struct p *)0)->q)



#define ISLOCKED(x)             ((x) != LOCK_AVAIL)
#define OWNER(x)                ((x) != LOCK_AVAIL && (x) == getpid())
#define NOT_OWNER(x)    ((x) == LOCK_AVAIL || (x) != getpid())

/*
** Miscellaneous Definitions
*/
#define DD_NAME_STR             "catdd"
#define POS_RESET               0x04    /* Reset bit in POS register 4 */
#define CTLBUFLEN               256     /* PSCA control buffer length (bytes)*/

typedef struct cdtbl {
        struct cdt_head header;
        struct cdt_entry entry[MAX_CDT_ELEMS];
} cdt_t;

typedef struct callout {
        struct callout *t_next; /* next callout */
        void (*t_func)();               /* Function to call on timeout */
        char *t_arg;                    /* Timeout argument vector */
        ulong t_seq;                    /* ID number */
        ulong t_val;                    /* Max num of timeout events to wait */
} timeout_t;

typedef struct {
        uchar command;                  /* Control Command */
        uchar cmdmod;                   /* Command modifier */
        uchar correl;                   /* Command correlator (for acks) */
        uchar subchan;                  /* Subchannel for this command */
        uchar chanstat;                 /* Channel Status returned-resets/errs*/
        uchar ccw;                              /* Command Control Word */
        uchar origcmd;                  /* Command being ACKed or err'd */
        uchar retcode;                  /* Return code/Error code for command */
        uchar data[2];                  /* Command specific data */
        ushort length;                  /* length of control buffer */
        ulong buffer;                   /* Offset of buf from start shared RAM*/
        int cmd_event;                  /* Event to wait on */
        timeout_t timer;                /* Timeout argument */
} cmd_t;

/*
**  Buffer FIFO Control Structure
*/
struct fifoctl {                        /* FIFO Control Structure  */
        ulong fifo;                             /* address of FIFO in SRAM */
        ushort fifosize;                /* no. of FIFO slots  */
        ushort nextslot;                /* next slot to add/get  */
};

/*
** The structure used to describe a data transmission from the
** PCA to the Microchannel.
*/
typedef struct recv_elem {
        struct open_struc *rc_open;     /* pointer to open structure */
        struct mbuf *rc_mbuf_head;      /* mbuf list head for rcv data */
        struct mbuf *rc_mbuf_tail;      /* mbuf list tail for rcv data */
        int rc_count;                           /* number of bytes received */
        int rc_resid;                           /* number of bytes remainig */
        uchar rc_scid;                          /* rcvd subchannel id */
        uchar rc_linkid;                        /* linkid for CLAW mode */
        uchar rc_state;                         /* empty, in_progress or complete */
#define RC_FREE                 0xFF
#define RC_EMPTY                0
#define RC_INPROGRESS   1
#define RC_COMPLETE             2
#define RC_ABORTED              4
/* d51658 #define RC_RETRY         8      d50453 flag to indicate in retry  */
        struct recv_elem *rc_next;      /* pointer to next element */
        struct recv_elem *rc_last;      /* pointer to previous element */
        uchar rc_ccw;                           /* ccw used by host (pass it through) */
/* d51658       int rc_num_xfers;  total number of buffers counter  d50453 */
/* d51658       int rc_num_dups;   counter of buffers received during retry d50453*/
} recv_elem_t;

typedef struct stat_elem {
        cio_stat_blk_t stat_blk;                /* common I/O status block */
        struct stat_elem *stat_next;    /* pointer to the next status element */
        struct stat_elem *stat_last;    /* pointer to the last status element */
} stat_elem_t;


#define MAX_LINKS       32
#define LK_NAME_LEN     8

/*
** Link structure, for the 3088 mode 1 is created per subchannel as link 0
*/
typedef struct link {
        struct open_struc *lk_open;     /* pointer to owner open_t */
        uchar lk_sc_id;                         /* Subchannel ID */
        uchar lk_actual_id;                     /* "Real" link ID as seen by ucode */
        uchar lk_appl_id;                       /* link ID originally given to appl */
        uchar lk_state;
#define LK_NEW          1
#define LK_REQ          2
#define LK_RESP         3
#define LK_FIRM         4
#define LK_DISC         5
#define LK_DELETE       6 
#define LK_CLOSED	    7
        int lk_correl;
        recv_elem_t *lk_recv_elem;
        char lk_WS_appl[LK_NAME_LEN];   /* keep WS_appl and H_appl in order */
        char lk_H_appl[LK_NAME_LEN];
        int  overrun; /* d50453 mark as an overrun link.  */
} link_t;

/*
** for claw only one subchannel_t is actually used (for the read
** subchannel), but two are created so as to stop other modes
** from using the write subchannel
*/
typedef struct subchannel {
        uchar sc_id;            /* subchannel id */
        uchar sc_state;         /* subchannel state  */
#define SC_CLOSED       1
#define SC_STARTING     2
#define SC_OPEN         3
#define SC_CLOSING      4
#define SC_SETUP        5
#define SC_DELETE	    6
        uchar sc_group;         /* ID of subchannel group leader */
        uchar sc_subset;        /* # SC's in group */
        uchar specmode;         /* SC special mode */
        uchar shrtbusy;         /* how to send short-busy status */
        uchar startde;          /* start with unsolicited device end? */
        int num_links;          /* num of active links using this subchannel */
/*
** sc->links[] is an unordered (by link_id) array of
** link_t structures, while sc->link[] is an ordered
** array (indexed by link_id) that gets filled in once
** we receive a valid link_id
*/
        link_t *link[MAX_LINKS];/* pointers to links for quick access */
        link_t **links;                 /* for claw mode -- */
                                                        /* only one link is needed for each pair     */
                                                        /* of sc's the read sc contains the link     */
                                                        /* table , when the subchannel is set to     */
                                                        /* claw mode, the table is created for 3088  */
                                                        /* mode, link points to link 0 for 3088 mode */
        int sc_stop_ack;                /* event to wait on until stop ack */
#define CAT_STOP_EVENT  EVENT_KERNEL+1
        uchar sc_flags;   /* d49490 for remembering parity error occurance */
#define CAT_BAD_PARITY  0x01  /* d49490 */
} subchannel_t;

typedef subchannel_t sc_t;

typedef struct open_struc {
        ulong op_open_id;                       /* used for kernel opens only */
        recv_elem_t *recv_act;          /* pointer to 1st recv queue element */
        stat_elem_t *stat_act;          /* status queue for user processes */
        stat_elem_t *stat_free;         /* status queue for user procs */
        subchannel_t *op_default;       /* pointer to default subchannel */
        void (*op_rcv_fn)();            /* pointer to kernel proc recv func */
        void (*op_xmit_fn)();           /* pointer to kernel proc xmit func */
        void (*op_stat_fn)();           /* pointer to kernel proc stat func */
        int op_rcv_event;                       /* used for waiting for receive data */
        uchar op_num_scopen;            /* number of subchannels started */
        uchar op_flags;                         /* flag bits for this open */
#define OP_FREE                         0       /* open structure is free */
#define OP_OPENED                       1       /* open structure is valid */
#define XMIT_OWED                       2       /* Need to call (*op_xmit_fn)() */
#define LOST_STATUS                     4       /* The status queue has been overrun */
#define SENT_LOST_STATUS        8       /* Sent lost status notification */
#define OP_CE_OPEN                      0x10/* This open was in '/C' mode */
#define OP_SYNC_MODE                    0x20/* Can't lose asynchronous status */
        ushort op_select;                       /* used for select events */
        ulong op_mode;                          /* mode passed to open() */
        chan_t op_chan;                         /* multiplexed DD ID */
        pid_t op_pid;                           /* PID of the process */
	   uchar op_sc_opened;             /* ix29279 boolean indicator for scstarts */
		timeout_t  etimer;				/* ix41127 event timers */
} open_t;

/*
** PCA buffer descriptor
** Used by PSCAXLST command to describe an array of buffers
** being transmitted.
*/
typedef struct xbuf {
        ushort length;                  /* length of buffer */
        ushort reserved;                /* reserved */
        uchar *buffer;                  /* offset of buffer in shared RAM */
} xbuf_t;

#define PIO_THRESHOLD   1024
/*
** The structure used to describe a data transmission from the
** Microchannel to the PCA.
*/
typedef struct xmit_elem {
        open_t *xm_open;                        /* pointer to open structure */
        cmd_t xm_cmd;                           /* transmit PSCA command buffer */
        struct mbuf *xm_mbuf;           /* pointer to mbufs containing data */
        caddr_t *xm_pca_ctrlbuf;        /* address of pca control buffer */
        xbuf_t xm_pca_lst[MAX_BUF_LIST];/* list of PCA buffers */
        int xm_length;                          /* total length of data */
        uchar xm_num_bufs;                      /* number of PCA buffers */
        uchar xm_ack;                           /* transmit acknowledgement flag */
        uchar xm_scid;                          /* subchannel id */
        uchar xm_linkid;                        /* linkid for CLAW mode */
        uchar xm_retrys;                        /* number of send retry attempts */
        ulong xm_writeid;                       /* writeid from caller */
        struct xmit_elem *xm_next;      /* pointer to next element  */
        struct xmit_elem *xm_last;      /* pointer to previous element  */
} xmit_elem_t;


/*
** The dma request structure describes a DMA transfer
*/
typedef struct dma_req {
        open_t *dm_open;                        /* pointer to open structure */
        union {
                xmit_elem_t *xmit_ptr;  /* pointer to xmit element */
                recv_elem_t *recv_ptr;  /* pointer to receive element */
                caddr_t owner_ptr;
        } p;                                            /* pointer to transmission request */
        uchar dm_req_type;                      /* request type -- xmit or recv */
#define DM_XMIT                 1
#define DM_RECV                 2
#define DM_PSEUDO_XMIT  4
#define DM_PSEUDO_RECV  8
        uchar dm_state;                         /* state of this request */
#define DM_READY                1
#define DM_STARTED              2
#define DM_ABORTED              4
#define DM_DONE			  8   /* d50453 */
        uchar dm_end_of_list;           /* indicates the end of a data stream */
        uchar dm_scid;                          /* subchannel id */
        uchar dm_linkid;                        /* linkid for CLAW */
        caddr_t dm_pca_buffer;          /* address of PCA buffer */
        caddr_t dm_rfb;                         /* ptr to PCA buffer to be returned */
        caddr_t dm_buffer;                      /* pointer to device driver buffer */
        int dm_length;                          /* length of data to transfer */
        int dm_dmalen;                          /* length adjusted for PSCA DMA bug */
        ushort dm_flags;                        /* direction of transfer etc */
        struct xmem *dm_xmem;           /* cross memory descriptor */
        struct dma_req *dm_next;        /* pointer to next DMA request */
        struct dma_req *dm_last;        /* pointer to previous DMA request */
} dma_req_t;

/*
 * For each device configured one of these is alloced, pinned, and
 * added to the link list.
 */
struct ca {
        /*
        ** Driver specific data
        */
        struct intr caih_struct;        /* interrupt struct, keep at top! */
        struct intr ofl;                        /* The off-level interrupt struct */
        struct watchdog watch;          /* watchdog timer control struct */
        struct trb *resource_timer;     /* used when out of mbufs */
        dev_t dev;                                      /* major-minor device number */
        lock_t adap_lock;                       /* adapter-wide lock */

/* locking changes start */
        lock_t locking_adap_lock;                       /* adapter-wide lock */
		int    in_kernel;  /* flag to indicate in kernel */
		int		lcklvl;
/* locking changes end */

        cmd_t ca_cmd;                           /* command buffer */
        ulong flags;                            /* state flags */
#define CATFUNC         0x00000001      /* running functional code */
#define CATSETUP        0x00000002      /* Set Adapter cmd has been done */
#define CATABLE         0x00000004      /* CU Table has been downloaded */
#define CATPARMS_SET 0x0000008  /* adapter parameters have been set  */
#define CATDIAG         0x00000010      /* opened in diag mode */
#define CATWDTACT       0x00000020      /* watchdog timer active */
#define CATXMITOWED     0x00000040      /* Need to call (*xmit_fn)()functions */
#define CATINIT         0x00000080      /* Driver has been initialized */
#define CATWDINSTALL 0x00000100 /* Watchdog handler installed */
#define CATDMACHAN      0x00000200      /* DMA channel acquired */
#define CATIINSTALL     0x00000400      /* Interrupt handler installed */
#define CATDEAD         0x00000800      /* adapter is dead */
#define CATRTINSTALL    0x00001000      /* resource timer is installed */
#define CAT_TIMER_ON    0x00002000      /* resource timer is on */
#define CAT_CE_OPEN     0x00004000      /* opened in Customer Engineer mode */
#define CAT_NOSTARTS    0x00008000      /* Customer Engineer put bd.  offline */
#define CAT_PARITY_ERR  0x00010000      /* Parity error detected w/ receive */
#define CAT_PAUSED      0x00020000      /* Adapter paused (~accepting note's) */
#define CAT_SAVED_NOTFY 0x00040000      /* Use ca->saved_notify next... */
#define CAT_TIMER_TRIG  0x00080000      /* d50722 resource timer has been triggered */
#define CAT_CHAN_BLOCK  0x00100000      /*d52590*/ /* hung channel flag */

/* Additional timer flags for the lock timer */
#define CAT_LOCK_TIMER_ON  0x00200000   /*  timer flag for locking being checked */
#define CATLTINSTALL    0x00400000      /* lock timer is installed */
#define CAT_LOCK_TIMER_TRIG  0x00800000   /*  timer flag for locking being checked */
		int 	piorc; 

        /*
        ** Adapter specific data
        */
        struct cadds caddp;             /* device dependent structure */
        struct fifoctl cmd_f;   /* command  fifo (push) */
        struct fifoctl stat_f;  /* status  fifo (pull) */
        struct fifoctl cfb_f;   /* control free buffer fifo (pull) */
        struct fifoctl dfb_f;   /* dump free buffer fifo (push */
        int pcabuf_event;               /* event flag for PCA buffers */
        ulong base_addr;                /* base address of the shared ram */
        int int_level;                  /* interrupt level of the adapter */
        int retry;                              /* number of retries during pio */
        STATAREA status;                /* PSCA status info */
        cmd_t saved_notify;             /* save notify if no resources */
        int fifos_cdtsize;              /* Length of fifos for CDT */
        caddr_t fifos_cdt;              /* Space for CDT of cmd/stat fifos */

        /*
        ** Open and Subchannel specific data
        */
        caddr_t open_cdtaddr;                   /* allocation address of open structs */
        ulong open_cdtsize;                             /* size of CDT entry for open structs */
        open_t open_lst[CAT_MAX_OPENS]; /* the open structs */
        int num_opens;                                  /* Number of outstanding open()s */
        subchannel_t *sc[MAX_SUBCHAN];  /* pointers to subchannel structs */
        uchar tot_subchan;                              /* total number of subchannels open */
        uchar param_table[MAX_SUBCHAN]; /* set if CLAW mode */

        /*
        ** Receive specific data
        */
        caddr_t recv_cdtaddr;           /* allocation addr of CDT recv list */
        ulong recv_cdtsize;                     /* size of CDT recv list */
        recv_elem_t *recv_free;         /* pointer to free receive elements */
        struct {
                recv_elem_t *recvp;
                ulong rb;
        } rfb_list[MAX_RECV_BUFS];      /* Receive buffer list */

        /*
        ** Transmit specific data
        */
        caddr_t xmit_cdtaddr;   /* allocation addr of CDT xmit list */
        ulong xmit_cdtsize;             /* size of CDT xmit list */
        xmit_elem_t *xmit_act;  /* pointer to active xmit elements */
        xmit_elem_t *xmit_free; /* pointer to free xmit elements */
        ushort xmit_bufs_avail; /* # of transmit buffers available */
        int xmitbuf_event;              /* event flag for xmit elements */
        int mbuf_event;                 /* event flag for xmit mbufs */
        int mbuf_num;      /*d50453new*/

        /*
        ** DMA specific data
        */
        caddr_t dma_cdtaddr;    /* allocation addr of CDT dma list */
        ulong dma_cdtsize;              /* size of CDT dma list */
        dma_req_t *dma_act;             /* active dma request elements */
        dma_req_t *dma_free;    /* free dma request elements */
        int dma_channel;                /* channel id returned from d_init */
        ushort dma_nfree;               /* number of free DMA elements */
        int dmabuf_event;               /* event flag for DMA elements */

        /*
        ** Miscellaneous data
        */
        int num_unsolicited;    /* # of unsolicited status un'acked */
        timeout_t *callout;             /* list of active timeouts */
        ulong timer_seqno;              /* unique seqno for timeouts */
        int   vpd_length;               /* length of VPD information */
        char  vpd[CAT_MAX_VPD_LEN];     /* VitalProductData information */
        cat_query_t stats;              /* DD statistics */
        struct ca *next;                /* pointer to next adapter struct */
};


typedef struct error_log_def {
        struct err_rec0 errhead;/* from com/inc/sys/err_rec.h */
        ushort adap_flags;              /* adapter flags */
        ushort total_sc;                /* total number of subchannels open */
        dev_t devnum;                   /* major/minor device number */
        ushort pio_retries;             /* # of pio retries */
        uint int_not_handled;   /* # of interrupts not handled */
        uint nombufs;                   /* # of recs lost for lack of mbufs */
        uint rcv_ovflows;               /* # of receive overflows */
};

#define SLOT                    (ca->caddp.slot_num)
#define CAT_IOCC_ATT    (ulong)(IOCC_ATT(ca->caddp.bus_id,IO_IOCC+(SLOT << 16)))
#define CAT_MEM_ATT             (ulong)(BUSMEM_ATT(ca->caddp.bus_id, ca->base_addr))

/*
** These typedefs are used by the byte-swapping functions
** letni16() and letni32().
*/
typedef struct {
        uchar byte1;
        uchar byte2;
} twobyte_t;

typedef struct {
        uchar byte1;
        uchar byte2;
        uchar byte3;
        uchar byte4;
} fourbyte_t;

typedef struct {
        ushort short1;
        ushort short2;
} twoshort_t;

/*
** Structures and macros for pio_assist()
*/
extern int pio_rc;

struct pos_wr {
        struct ca *ca;
        ulong bus_addr;
        ulong addr;
        uchar data;
};

struct pos_rd {
        struct ca *ca;
        ulong bus_addr;
        ulong addr;
        uchar *data;
};

struct mem_acc {
        struct ca *ca;
        ulong bus_addr;
        ulong addr;
        uchar *data;
        ulong len;
};

#define CAT_WRITE1(bus, adr, val) { \
        (ca->piorc) = BUS_PUTCX((bus+(adr&0x0fffffff)), *val); \
        if ((ca->piorc)) \
                (ca->piorc) = BUS_PUTCX((bus+(adr&0x0fffffff)), *val); \
        if ((ca->piorc)) \
                (ca->piorc) = BUS_PUTCX((bus+(adr&0x0fffffff)), *val); \
        if ((ca->piorc)) \
                cat_logerr(ca, ERRID_CAT_ERR8); \
}

#define CAT_READ1(bus, adr, ptr) { \
        (ca->piorc) = BUS_GETCX((bus+(adr&0x0fffffff)), ptr); \
        if ((ca->piorc)) \
                (ca->piorc) = BUS_GETCX((bus+(adr&0x0fffffff)), ptr); \
        if ((ca->piorc)) \
                (ca->piorc) = BUS_GETCX((bus+(adr&0x0fffffff)), ptr); \
        if ((ca->piorc)) \
                cat_logerr(ca, ERRID_CAT_ERR8); \
}

#define CAT_WRITE(bus, adr, val, length) { \
        (ca->piorc) = BUS_PUTSTRX((bus+(adr&0x0fffffff)), val, length); \
        if ((ca->piorc)) \
                (ca->piorc) = BUS_PUTSTRX((bus+(adr&0x0fffffff)), val, length); \
        if ((ca->piorc)) \
                (ca->piorc) = BUS_PUTSTRX((bus+(adr&0x0fffffff)), val, length); \
        if ((ca->piorc)) \
                cat_logerr(ca, ERRID_CAT_ERR8); \
}

#define CAT_READ(bus, adr, ptr, length) {                                                       \
        (ca->piorc) = BUS_GETSTRX((bus+(adr&0x0fffffff)), ptr, length); \
        if ((ca->piorc)) \
                (ca->piorc) = BUS_GETSTRX((bus+(adr&0x0fffffff)), ptr, length); \
        if ((ca->piorc)) \
                (ca->piorc) = BUS_GETSTRX((bus+(adr&0x0fffffff)), ptr, length); \
        if ((ca->piorc)) \
                cat_logerr(ca, ERRID_CAT_ERR8); \
}


#define CAT_POS_WRITE(bus, reg, val) { \
        struct pos_wr   pio; \
        pio.ca                  = ca; \
        pio.bus_addr    = bus; \
        pio.addr                = reg; \
        pio.data                = val; \
        cat_pos_write(&pio); }

#define CAT_POS_READ(bus, reg, ptr) {                                                           \
        struct pos_rd   pio; \
        pio.ca                  = ca; \
        pio.bus_addr    = bus; \
        pio.addr                = reg; \
        pio.data                = ptr; \
        cat_pos_read(&pio); }

#define SLEEP(el)       e_sleep (el, EVENT_SIGRET)
#define WAKEUP(el)      e_wakeup (el)

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

#define SET_GROUP( subc, st ) \
{       \
        int x;  \
        for (x=subc;x<MAX_SUBCHAN;x++) {        \
                if( ca->sc[x] != NULL ) {       \
                        if (ca->sc[x]->sc_group == subc)        \
                                ca->sc[x]->sc_state = st;       \
                        else    \
                                break;  \
                }       \
        }       \
}

#endif  /* _H_CATDD */
