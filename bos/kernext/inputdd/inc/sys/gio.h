/* @(#)76   1.2 src/bos/kernext/inputdd/inc/sys/gio.h, inputdd, bos411, 9428A410j 6/6/94 10:37:33 */
/*
 * COMPONENT_NAME: (INPUTDD) Graphics I/O device driver
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_GIO
#define _H_GIO 1

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/intr.h>
#include <sys/timer.h>
#include <sys/cblock.h>
#include <sys/except.h> 		/* pio exception header */
#include <sys/watchdog.h>
#include <sys/inputdd.h>   /* common inputdd structs/defines */            
#include <sys/syspest.h>
#include <sys/uio.h>
#include <sys/xmem.h>
#include <sys/time.h>

#ifdef GIODEBUG
#define GIODBTRACE1(a1)       gio_trace(1, a1, 0, 0, 0, 0)
#define GIODBTRACE2(a1,a2)    gio_trace(2, a1, (int)a2, 0, 0, 0)
#define GIODBTRACE3(a1,a2,a3) gio_trace(3, a1, (int)a2, (int)a3, 0, 0)

#define GIODBTRACE4(a1,a2,a3,a4) \
        gio_trace(4, a1, (int)a2, (int)a3, (int)a4, 0)

#define GIODBTRACE5(a1,a2,a3,a4,a5) \
        gio_trace(5, a1, (int)a2, (int)a3, (int)a4, (int)a5)

#define GIOTRACE1(a1)       gio_trace(1, a1, 0, 0, 0, 0)
#define GIOTRACE2(a1,a2)    gio_trace(2, a1, (int)a2, 0, 0, 0)
#define GIOTRACE3(a1,a2,a3) gio_trace(3, a1, (int)a2, (int)a3, 0, 0)

#define GIOTRACE4(a1,a2,a3,a4) \
        gio_trace(4, a1, (int)a2, (int)a3, (int)a4, 0)

#define GIOTRACE5(a1,a2,a3,a4,a5) \
        gio_trace(5, a1, (int)a2, (int)a3, (int)a4, (int)a5)
#else
#define GIODBTRACE1(a1)
#define GIODBTRACE2(a1,a2)
#define GIODBTRACE3(a1,a2,a3)
#define GIODBTRACE4(a1,a2,a3,a4)
#define GIODBTRACE5(a1,a2,a3,a4,a5)

#define GIOTRACE1(a1)       gio_trace(a1, 0, 0, 0, 0)
#define GIOTRACE2(a1,a2)    gio_trace(a1, (int)a2, 0, 0, 0)
#define GIOTRACE3(a1,a2,a3) gio_trace(a1, (int)a2, (int)a3, 0, 0)

#define GIOTRACE4(a1,a2,a3,a4) \
        gio_trace(a1, (int)a2, (int)a3, (int)a4, 0)

#define GIOTRACE5(a1,a2,a3,a4,a5) \
        gio_trace(a1, (int)a2, (int)a3, (int)a4, (int)a5)
#endif

extern struct uregring  lpfk_ring;  /* ring registration */
extern struct uregring  dials_ring;
extern struct ir_lpfk   lpfk_report; /* device report struct */
extern struct ir_dials  dials_report;
#define NOT_REGISTERED 0
#define REGISTERED 1
extern uchar  dials_ring_registered;
extern uchar  lpfk_ring_registered;

/* 
 * This is what is fed down to the config routine.  The seg and offset are
 * used during normal processing.  iseg and ioffset are used to get to the
 * POS registers.  type, flags, bus_intr_lvl and priority are put into the
 * intr struct used for the slih.
 */
struct gio_dds {
    char   gd_name[16];			/* name of special file */
    ulong  gd_seg;			/* normal segment register value */
    ulong  gd_offset;			/* adapter offset */
    ulong  gd_iseg;			/* POS regs segment register value */
    ulong  gd_ibase;			/* POS regs offset */
    ulong  gd_frequency;		/* oscillator frequency */
    ushort gd_type;			/* usually BUS_MICRO_CHANNEL */
    ushort gd_flags;			/* usually 0 */
    int    gd_priority;			/* usually INTCLASS3 */
    int    bus_intr_lvl;		/* interrupt level */
    uchar  gd_devtype;			/* type of devices */
					/* values for gd_devtype */
#define  GIO_ADAP	0		/* gio adapter card */
#define  GIO_LPFK	1		/* lpfk attached to GIO adapter */
#define  GIO_DIALS	2		/* dials attached to GIO adapter */
#define  SIO_LPFK	3		/* lpfk on standard I/O port*/
#define  SIO_DIALS	4		/* dials on standard I/O port */
};

#define  MAX_DEV  3			/* max number of devices allowed */

/*
 * This structure is used for each port - the two ports on the GIO adapter
 * card and the two standard I/O serial ports.
 */
typedef struct gio_dev *giod_t;		/* pointer to port structure */
struct gio_dev {
    struct clist go_out;		/* output clist */
    struct clist go_in; 		/* input clist */
    struct trb   *go_timer;		/* delay between char to device */
    uchar  go_happy;			/* set lpfk cmd worked */
    uchar  go_failed;                   /* set lpfk cmd failed */
    uchar  go_busy;			/* output is going */
    uchar  go_idle;			/* idle timer state    */
/*                     0                     timer disabled    */
#define POLLD          1                /*   just checking     */
#define WAITD          2                /*   waiting for device power on */
#define REINIT         4                /*   initializing device         */
    uchar  go_diag;			/* input from device goes to */
					/* go_in rather than user ring */
    uchar  go_type;			/* LPFK_RESP or DIAL_RESP */
    char   go_name[16];			/* name of special file */
    union {
	struct {
            uchar gio_select;           /* granularity change mask */
	    char  gio_dlast;		/* last byte from dials */
	    char  gio_gran[8];		/* granularity */
	    short gio_last[8];		/* last position */
	} gio_dial;
	char gio_lpfk[4];		/* pfks settings */
    } go_state;
};

/*
 * This structure is used for each adapter card.  Note: the two standard I/O
 * serial ports are considered to be two ports on the standard I/O adapter.
 * NOTE: the interrupt structure must be first.
 */
typedef struct gio_adap *giop_t;	/* pointer to adapter */

/*
 * This structure is used for a watchdog timer.
 */
struct gio_watchdog {
    struct watchdog watch;		/* watchdog timer structure */
    giop_t  gd_pointer;			/* pointer to adapter structure */
};

struct gio_adap {			/* one per adapter */
    struct intr ga_intr;		/* slih's interrupt struct */
    struct gio_dev ga_dev[2];		/* devices for each port */
    struct gio_watchdog ga_watch;	/* watchdog timer */
    ulong  ga_offset;			/* adapter offset (30 or 960) */
    ulong  ga_iseg;			/* POS regs segment register value */
    ulong  ga_ibase;			/* POS regs offset */
    int    ga_event;			/* event list to sleep on */
    int    ga_num;			/* # of special files/this adapter */
    int    ga_opened;			/* # of opened files this adapter */
    ulong  ga_frequency;		/* oscillator frequency */
    uchar  ga_watchdog;			/* watchdog timer popped */
};

/*
 * This structure is used for each special file which the user can open -
 * gio(0), lpfk(0), and dials(0)
 */
typedef struct gio_file *giof_t;	/* pointer to file information */
struct gio_file {			/* one per special file */
    giop_t  gf_adap;			/* adapter used by this file */
    int     gf_port;			/* port (0 or 1) used by this file */
					/* (0 for the GIO adapter file) */
    dev_t   gf_devt;			/* device number */
    uchar   gf_type;			/* type of device & adapter */
    uchar   gf_open;			/* special file is opened */
    uchar   gf_pause;			/* doing a pause */
    struct timestruc_t gf_tod;          /* record time of open for use in timestamping events */
    ulong   gf_pid;                     /* PID of user process holding this device open */
                                        /* (process is signaled when event added to input ring) */
    struct xmem gf_dp;                  /* Cross-Memory descriptor used to manipulate input ring */
};

#define GioPort(g) ((struct gio_port *)io_att(g->ga_intr.bid, g->ga_offset))

struct gio_port {
    char g_rbr; 			/* 0 */
#define g_thr g_rbr
#define g_dll g_rbr
    char g_ier; 			/* 1 */
#define g_dlm g_ier
#define G_ERBDAI 0x01
#define G_ETHREI 0x02
#define G_ELSI	 0x04
#define G_EMSSI  0x08
    char g_iir; 			/* 2 */
#define g_fcr g_iir
#define G_FIFO_ENABLE 0x01		/* enable fifo's */
#define G_RFIFO_RESET 0x02		/* reset receive fifo */
#define G_XFIFO_RESET 0x04		/* reset xmit fifo */
#define G_DMA_MODE    0x08		/* use mode 1 */
#define g_afr g_iir
#define G_CON_WRITE 0x01
#define G_BAUDOUT   0x02		/* BAUDOUT select */
#define G_RXRD_SEL  0x04		/* RXRD select */
#define G_DISAB_DIV13 0x10
    char g_lcr; 			/* 3 */
#define G_WLS0	0x01
#define G_WLS1	0x02
#define G_STB	0x04
#define G_PEN	0x08
#define G_EPS	0x10
#define G_STICK 0x20
#define G_BREAK 0x40
#define G_DLAB	0x80
    char g_mcr; 			/* 4 */
#define G_DTR	0x01
#define G_RTS	0x02
#define G_OUT1	0x04
#define G_OUT2	0x08
#define G_LOOP	0x10
    char g_lsr; 			/* 5 */
#define G_DR	0x01
#define G_OE	0x02
#define G_PE	0x04
#define G_FE	0x08
#define G_BI	0x10
#define G_THRE	0x20
#define G_TEMT	0x40
#define G_EFIFO 0x80
    char g_msr; 			/* 6 */
#define G_DCTS	0x01
#define G_DDSR	0x02
#define G_TERI	0x04
#define G_DDCD	0x08
#define G_CTS	0x10
#define G_DSR	0x20
#define G_RI	0x40
#define G_DCD	0x80
    char g_scr; 			/* 7 */
};
#define GR(x) BUSIO_GETC(&(x))
#define SR(x, v) BUSIO_PUTC(&(x), v)
#define set_dlab(port) SR(port->g_lcr, GR(port->g_lcr)|G_DLAB)
#define clr_dlab(port) SR(port->g_lcr, GR(port->g_lcr)&~G_DLAB)


#define LPFK_RESP 0x03
#define LPFK_TYPE 0x05
#define DIAL_RESP 0x08
#define DIAL_TYPE 0x06
#define LOOP_RESP READ_CONF		/* Read config comes back if wrapped */

#define RESET	  0x01
#define READ_CONF 0x06
#define ENABLE	  0x08
#define DISABLE   0x09
#define READ_DIAL 0x0b
#define WRAP	  0x0e
#define UNWRAP	  0x0f
#define LPFK_NAK  0x80
#define LPFK_ACK  0x81
#define LPFK_SET  0x94
#define SET_GRAN  0xc0

#define GIO_HZ 1843200

#define GIO_DIAL_INIT 512
#define GIO_DIAL_SECOND 513

#define GIO_GETIDS	22		/* cfggio - what devices on GIO card */
#define GIO_DIAG_CMD	0x01
#define LPFK_SET_CMD	0x94
#define DIALS_SET_CMD	0xC0
#define DEV_LPFKS	32
#define DEV_DIALS	8

#define IDLE_TIME       15              /* idle time before poll   (seconds) */
#define DEAD_TIME       4               /* dead time before ENABLE (seconds) */

typedef struct pud {
    int err;
    int p_check;
    int p_mask;
} pud_t;

#define DoingPio    void (*PioF)(); volatile int Rtry; int Setrc;\
		    label_t JmpBuf; void *Arg; pud_t Pud
#define StartPio(func, arg, thunk)	\
{					\
    PioF = func;			\
    Arg = arg;				\
    Rtry=PIO_RETRY_COUNT;		\
    while (Setrc = setjmpx(&JmpBuf)) {	\
	if (Setrc == EXCEPT_IO ||	\
	    Setrc == EXCEPT_IO_IOCC) {	\
	    if (--Rtry <= 0) {		\
		PioF(Arg, &Pud, 1, Setrc);	\
		thunk;			\
	    } else 			\
		PioF(Arg, &Pud, 0, Setrc);	\
	} else				\
	    longjmpx(Setrc);		\
    }					\
    if (Rtry) {

#define EndPio()			\
	if (Rtry != PIO_RETRY_COUNT)	\
	    PioF(Arg, &Pud, 2, 0); 	\
	clrjmpx(&JmpBuf);		\
    }					\
}

/* giol.c routines used in giou.c */
void gio_wakeup(struct trb *t);
void gio_out(struct trb *t);
int gio_slih(struct intr *intr);
void gio_pio_catch(giop_t g, pud_t *pud, int logit, int err);
void gio_watch(struct watchdog *wp);

#endif _H_GIO
