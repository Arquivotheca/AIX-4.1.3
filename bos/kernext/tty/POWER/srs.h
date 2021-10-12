/* @(#)70 1.19 src/bos/kernext/tty/POWER/srs.h, sysxsrs, bos41B, 412_41B_sync 12/1/94 05:28:01 */
/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * ORIGINS: 27, 83
 *
 */
/*
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _H_SRS
#define _H_SRS

/* For stream structures */
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strlog.h>
#include <sys/strconf.h>

#include <sys/cblock.h>
#include <sys/intr.h>
#include <sys/str_tty.h>
#include <termios.h>         /* Not in the sys directory */
#include <sys/termiox.h>

#ifdef _KERNEL
#include <sys/adspace.h>                /* io_att and io_det */
#include <sys/iocc.h>                   /* iocc space values */
#include <sys/ioacc.h>                  /* bus access macros */
#include "ttydrv.h"
#include <sys/except.h>                 /* PIO stuff */
#endif /* _KERNEL */

#define RS_TIMEX
#define RBSIZE	1024

#define RS_CONFIG
#ifdef RS_CONFIG

enum adap_type {
    Unknown_Adap,
#ifdef _POWER
    NativeA_SIO, NativeB_SIO,
    NativeA_SIO1, NativeB_SIO1, NativeA_SIO2, NativeB_SIO2,
    NativeA_SIO3, NativeB_SIO3, NativeC_SIO3,
    /* add next native here */
    Native_Last,
    Eight_232, Eight_422, Eight_Mil, Sixteen_232,
    Sixteen_422,
#endif /* _POWER */
};

#define NATIVE_MASK 0x9
#define IsNative(x) ((x) < Native_Last)
#define IsNativeA(x) (((x)&NATIVE_MASK) == 1)

/* There are two kinds of DDS structures: One for */
/* adapters configuration and one for lines */
/* configuration */

/* ====================== */
/* RS_ADAP_DDDS structure */
/* ====================== */
/* 
 * This structure (or something like it) is fed down at config time.  The
 * adapter type must be one that is known.  The other fields are fed down
 * and checked with those in the known interrupt structure.  Any field marked
 * as CONFIG_TIME is allowed to be set.  Any other field is compared and
 * must match or the config will fail.
 * 
 * Note: from the slot number we can find the id registers; from the
 * id registers we can find the adapter type.  On 8 port case, we set
 * in the adapter number which defines the port address.  With native,
 * we simply know the base address of the adapter is at 0.  In either
 * case we can figure out the addresses of the ports.  It seems that
 * slot 0xf is used for native i/o but we won't make that assumption.
 *
 * The items marked p are stored in the per port private structure.  
 * The items marked e are kept in the edge structure and the items 
 * marked i are kept in the slih structure.
 */
struct rs_adap_dds {
    enum   dds_type which_dds;        /* dds identifier */
    char   rc_name[DEV_NAME_LN+1];    /* adapter name */
    enum   adap_type rc_type;         /* p type of adapter */
    uchar  rc_anum;                   /* e adapter number */
    uchar  rc_slot;                   /* p slot number */
    dev_t  rc_parent;                 /* e expansion unit devno */
    int    rc_level;                  /* p interrupt level */
    int    rc_priority;               /* e interrupt class */
    uchar  rc_dma;                    /* p dma channel */
    ulong  rc_xtal;                   /* p xtal frequency */
    ushort rc_bus;                    /* e bus type (pc or micro channel */
    ushort rc_flags;                  /* e sharable interrupts flag */
    ulong  rc_arb;                    /* i arbitration register */
    ulong  rc_nseg;                   /* p seg reg for normal access */
    ulong  rc_base;                   /* p base address of adapter */
    ulong  rc_iseg;                   /* p seg reg for id access */
    ulong  rc_ibase;                  /* p base of id registers */
};

/* ====================== */
/* RS_LINE_DDDS structure */
/* ====================== */
struct rs_line_dds {
    enum   dds_type which_dds;        /* dds identifier */
    char   rc_name[DEV_NAME_LN+1];    /* line name */
    char   adap_name[DEV_NAME_LN+1];  /* Adpater (parent) name of the line */
    struct termios ctl_modes;         /* control modes */
    struct termiox disc_ctl;          /* open and flow disciplines */
    uchar  tbc;                       /* transmit buffer count */
    uchar  rtrig;                     /* fifo level */
};

#endif /* RS_CONFIG */

#ifdef _KERNEL

#ifdef _POWER
#define IOCC_CNT 4
#define SEG2IOCC(seg) ((((seg)>>20)&0xff)-0x20)

/* Given a pointer to a dds, this computes the base minor number */
#define RSDEV(dds) ((SEG2IOCC((dds)->rc_nseg)<<14) | \
    ((dds)->rc_arb == 0x134 ? 0x1000 : 0) | \
    ((((dds)->rc_slot + 1) & 0xf) << 8))

/* Minor number to slot/iocc number */
#define dev2slot(mnr) (mnr>>8)

/* Minor number to port number */
#define dev2port(mnr) (mnr&0xff)
#endif /* _POWER */

#ifdef _POWER
/* 
 * In the R2, we use the micro channel's pos registers for the id 
 * registers and to setup the board -- surprise!!!
 */
#define RS_IOCC(slot) (0x00400000+((slot)<<16))
#endif /* _POWER */

#endif /* _KERNEL */

#define TTY_TRCID HKWD_TTY_RS
#define TRC_SIZE 16

/* RS private data structure. There is one structure per tty */
typedef struct str_rs * str_rsp_t;
struct str_rs {
    char t_name[DEV_NAME_LN+1];       /* resource name for this tty */
    dev_t t_dev;            /* major/minor number */

/* replaced here for db_print in user mode */
    void *t_hptr;           /* hardware storage */
#ifdef _KERNEL
    struct ccblock t_tbuf;  /* transmit buffer */
    char t_pribuf;          /* for priority characters */
#endif /* _KERNEL */
    queue_t *t_ctl;         /* queue pointer*/
    mblk_t *t_outmsg;       /* msg block on x'sion */
    mblk_t *t_inmsg;        /* msg block on reception */
    mblk_t *t_ioctlmsg;     /* M_IOCACK to be replyed after a timeout */
    int t_wopen;            /* # of opens in progress */

                            /* Other things */
    int t_event;            /* event list for e_sleep */
    Simple_lock t_lock_soft;/* lock for off-level and treads */
    Simple_lock t_lock_hard;/* lock for slih's */
    int t_priority;         /* slih's priority */
    int t_timeout;	    /* Call to timeout from M_DELAY and M_BREAK */
    int t_alloctimer;	    /* Call to timeout from srs_allocmsg */
    uchar t_draining;	    /* data is being drained at close time */
    uchar t_sched;	    /* off-level should be rescheduled */
    uint t_hupcl : 1;	    /* current HUPCL termios cfalg */
    uint t_excl : 1;	    /* exclusive open mode */
    int t_bufcall;	    /* bufcall utility was called */

                            /* Bits and things */
    struct termios t_termios; /* Initial line settings */
    struct termiox t_termiox; /* For hardware pacing and open disciplines */
    tcflag_t t_softpacing;    /* Current termios c_iflag */
    cc_t t_softchars[2];      /* Current termios VSTART and VSTOP chars */
    unsigned short t_hardpacing;    /* Current termiox x_hflag */

#ifdef _KERNEL
    baud_t t_ospeed;        /* output speed */
    baud_t t_ispeed;        /* input speed */
    stop_t t_stopbits;  /* number of stop bits */
    parity_t t_parity;  /* parity style */
    csize_t t_csize;    /* character size */
#endif /* _KERNEL */

                            /* Hardware state flags */
    uint t_carrier : 1;     /* carrier flag */
    uint t_clocal : 1;      /* local/remote open mode */
    uint t_isopen : 1;      /* tty is open */
    uint t_stop : 1;        /* output is stopped, remote pacing */

    uint t_block : 1;       /* input is stopped, local pacing */
    uint t_ctlx : 1;        /* got a control X */
    uint t_sak : 1;         /* sak is enabled */
    uint t_cread : 1;       /* termios cread flag */

    char t_localstop;	    /* stopped because of an M_STOP */
    char t_busy;	    /* outputs in progress */
    short t_flow_state;     /* flow control state */

                            /* Open disciplines */
    int t_open_disc;        /* Current open discipline */
                            /* 0 == dtropen */
                            /* 1 == wtopen */
                            /* 2 == riopen */
    caddr_t t_openRetrieve; /* To retrieve current open discipline */

    uchar  *t_rptr;	    /* ptr to where to read */
    uchar  *t_wptr;	    /* ptr to where to write */
    uchar  t_rbuf[RBSIZE];  /* receive buffer */
};

/* Values of t_flow_state */
#define SOFT_REMOTE_STOP  0x01
#define SOFT_LOCAL_STOP   0x02
#define HARD_REMOTE_STOP  0x04
#define HARD_LOCAL_STOP   0x08

#define STARTCHAR 0
#define STOPCHAR 1

/* RS hardware data structure */
typedef struct rs_ym *rymp_t;
struct rs_ym {
    struct intr ry_intr;            /* 1st field!! post to off-level */
    int ry_level;                   /* hardware level for this port */
    ulong ry_iseg;                  /* saved from rc_iseg */
    ulong ry_ibase;                 /* saved from rc_ibase */
    str_rsp_t ry_tp;                /* RS private data structure pointer */
    ulong ry_port;                  /* port offset */
    struct clist ry_data;           /* data from slih to offlevel */
    char *ry_xdata;                 /* pointer to next char to send */
    uint ry_xcnt;                   /* number of bytes left to send */
    uint ry_fcnt;		    /* free FIFO count    156569 */
    ulong ry_xtal;                  /* xtal on board */
    uchar ry_rtrig;                 /* fifo level */
    uchar ry_tbc;                   /* transmit buffer count */
    uchar ry_msr;                   /* last msr offlevel saw */
    uchar ry_posted;                /* has been posted to offlevel */
    int ry_xmit;                    /* xmit empty interrupt */ /* 167561 */
    uchar ry_dma;                   /* dma channel number */
    uchar ry_slot;                  /* slot board is in */
    uint ry_open : 1;               /* 1st thing set on open */
    uint ry_watch : 1;              /* watch dog timer flag */
    uint ry_mil : 1;                /* 1 means invert buffers */
    uint ry_xbox : 1;               /* in the expansion box */
    uint ry_conf : 1;               /* tty is configured */
    uint ry_dsleep : 1;             /* sleeping for dma change */
    uint ry_ioff : 1;               /* intr turned off in slih */
    uint force_xmit : 1;	    /* force xmit_done intr    156569 */
    uint fake_xmit : 1;             /* 1 = fake xmit_done intr, 0 = real xmit_done intr  156569 */
    enum adap_type ry_type;         /* adapter type */
    uchar ry_trace[TRC_SIZE];       /* buf for tracing */
    int ry_tridx;                   /* index into ry_trace */
    int ry_ocnt, ry_icnt, ry_bcnt;  /* out, in, bad char counts */
    int ry_watched;                 /* times watchdog has bit us */
#ifdef RS_TIMEX
    struct trb *ry_timer;           /* lost character timer */
#endif
    int  ry_ioffcnt;                    /* intr turned off count */
};

#ifdef _KERNEL

/* Moved here from srs.c because srs_db.c need it */


#define DO_PIO

#define RS_PORT
#ifdef RS_PORT

struct srs_port {
    char r_rbr;                /* 0 */
#define r_thr r_rbr
#define r_dll r_rbr
    char r_ier;                /* 1 */
#define r_dlm r_ier
#define ERBDAI 0x01
#define ETHREI 0x02
#define ELSI   0x04
#define EMSSI  0x08
    char r_iir;                /* 2 */
#define r_fcr r_iir
#define FIFO_ENABLE 0x01        /* enable fifo's */
#define RFIFO_RESET 0x02        /* reset receive fifo */
#define XFIFO_RESET 0x04        /* reset xmit fifo */
#define DMA_MODE    0x08        /* use mode 1 */
#define TRIG_MASK   0xC0
#define TRIG_01     0x00
#define TRIG_04     0x40    
#define TRIG_08     0x80    
#define TRIG_14     0xC0    
#define r_afr r_iir
#define CON_WRITE   0x01        /* gang writes */
#define BAUDOUT     0x02        /* BAUDOUT select */
#define RXRD_SEL    0x04        /* RXRD select */
#define LOOP_NOCARE 0x06        /* loop back don't care */
#define DISAB_DIV13 0x10
    char r_lcr;                /* 3 */
#define WLS0  0x01
#define WLS1  0x02
#define STB   0x04
#define PEN   0x08
#define EPS   0x10
#define STICK 0x20
#define BREAK 0x40
#define DLAB  0x80
    char r_mcr;                /* 4 */
#define DTR   0x01
#define RTS   0x02
#define OUT1  0x04
#define OUT2  0x08
#define LOOP  0x10
    char r_lsr;                /* 5 */
#define DR    0x01
#define OE    0x02
#define PE    0x04
#define FE    0x08
#define BI    0x10
#define THRE  0x20
#define TEMT  0x40
#define EFIFO 0x80
#define LSR_MASK (OE|PE|FE|BI)
    char r_msr;                /* 6 */
#define DCTS  0x01
#define DDSR  0x02
#define TERI  0x04
#define DDCD  0x08
#define CTS   0x10
#define DSR   0x20
#define RI    0x40
#define DCD   0x80
    char r_scr;                /* 7 */
};

/* 
 * The native adapter has outbound dma capabilities controlled by a 
 * register at address 0x41.  The two bits for port 1 is 0x5 and the 
 * two bits for port 2 is 0xa.  The macro DMA_BITS provides these 
 * given a private struct.
 */
#define NIO_DMA_REG 0x41
#define DMA_BITS(rsp) (IsNativeA((rsp)->ry_type) ? 0x5 : 0xa)

#define GR(x) BUSIO_GETC(&(x))
#define SR(x, v) BUSIO_PUTC(&(x), v)
#define set_dlab(port) SR(port->r_lcr, GR(port->r_lcr)|DLAB)
#define clr_dlab(port) SR(port->r_lcr, GR(port->r_lcr)&~DLAB)
#define Port(rsp) ((struct srs_port *)(io_att(rsp->ry_intr.bid, rsp->ry_port)))
#endif /* RS_PORT */

/*
 * =============================================================================
 * PIO defines and structures
 * =============================================================================
 */
#ifdef DO_PIO
/* void srs_pio_catch(rymp_t rsp, pud_t *pud, int logit); */
typedef struct pud {
    struct pio_except p_except;
    int p_check;
    int p_mask;
} pud_t;
#define DoingPio    void (*PioF)(); volatile int Rtry; int Setrc;\
                    label_t JmpBuf; void *Arg; pud_t Pud
#define StartPio(func, arg, thunk)      \
{                                       \
    PioF = func;                        \
    Arg = arg;                          \
    Rtry=PIO_RETRY_COUNT;               \
    while (Setrc = setjmpx(&JmpBuf)) {  \
        if (Setrc == EXCEPT_IO ||       \
            Setrc == EXCEPT_IO_IOCC) {  \
            if (--Rtry <= 0) {          \
                if (PioF)               \
                    PioF(Arg, &Pud, 1, __LINE__); \
                thunk;                  \
            } else if (PioF)            \
                PioF(Arg, &Pud, 0, __LINE__);     \
        } else                          \
            longjmpx(Setrc);            \
    }                                   \
    if (Rtry) {

#define EndPio()            \
        if (Rtry != PIO_RETRY_COUNT &&  \
                PioF)                   \
            PioF(Arg, &Pud, 2, __LINE__);         \
        clrjmpx(&JmpBuf);               \
    }                                   \
}

#else /* ~DO_PIO */
#define DoingPio int NotUsed
#define StartPio(a,b,c)
#define EndPio()
#endif /* DO_PIO */



#endif /* _KERNEL */

#endif /* _H_SRS */
