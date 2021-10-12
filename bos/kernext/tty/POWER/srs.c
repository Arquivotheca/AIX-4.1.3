#ifndef lint
static char sccsid[] = "@(#)75 1.90 src/bos/kernext/tty/POWER/srs.c, sysxsrs, bos41J, 9525D_all 6/19/95 08:24:31";
#endif
/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * FUNCTIONS: srsopen, srsclose, srs_ioctl, srs_ctl, srs_termios_set,
 * srs_termios_get, srs_termiox_get, srs_termiox_set,
 * srs_break_set, srs_break_clear, srsconfig, srsadd, srsdel, srsrsrv,
 * srswput, srswsrv, srs_proc, srs_service, srs_streams_service,
 * srs_offlevel, srs_nslih, srs_8slih,
 * srs_xboxopen, srs_xboxclose, srs_setpos, srs_getpos, srs_getvpd,
 * srs_pio_catch, srs_print, srs_bits, srs_err,
 * srs_set_bit, srs_clr_bit, srs_dowatch, srs_watchdog, srs_xmit
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

/*
 *
 * This is a streams driver module which supports Native Serial ports,
 * the 8-port and 16-port rs232 and rs422 adapters
 */

/*
 * This driver is for National Semiconductor 16550 type uarts.  This
 * piece of code originated at IBM. The Conversion from traditional type
 * driver to Streams driver is done by BULL, Echirolles 
 */


#include <sys/types.h>
#include <sys/systemcfg.h>		/* runtime checks */
#include <sys/sys_resource.h>		/* key possition check */
#include <sys/limits.h>		        /* for MAX_CANON value */
#include <sys/sysmacros.h>              /* minor and major macros */
#include <sys/intr.h>                   /* interrupt handler stuff */
#include <sys/pin.h>                    /* pinning stuff */
#include <sys/adspace.h>                /* io_att and io_det */
#include <sys/iocc.h>                   /* iocc space values */
#include <sys/ioacc.h>                  /* bus access macros */
#include <sys/dma.h>                    /* dma stuff */
#include <sys/xmem.h>                   /* x-memory stuff for dma*/
#include <sys/malloc.h>                 /* malloc services */
#include <sys/errids.h>                 /* get the error id's */
#include <sys/errno.h>                  /* error codes */
#include <sys/uio.h>                    /* struct uio */
#include "ttydrv.h"                     /* instead of the old sys/tty.h */
#include <sys/termio.h>                 /* struct termios */
#include <sys/termiox.h>                /* struct termiox */
#include <sys/ioctl.h>                  /* Ioctl declerations*/
#include <sys/device.h>                 /* for config stuff */
#include <sys/dbg_codes.h>              /* temp for debugger thing */
#include <sys/mstsave.h>                /* temp for debugger thing */
#include <sys/low.h>                    /* temp for debugger thing */
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/lockl.h>                  /* for lockl and unlockl */
#include <sys/atomic_op.h>              /* for atomic operations */
#include <sys/trchkid.h>                /* trace hook id's */
#include <sys/sysinfo.h>                /* sysinfo stuff */
#include <sys/signal.h>                 /* SIGSAK */
#include <sys/pri.h>
#include <sys/sleep.h>                  /* EVENT_NULL */
#include <sys/dump.h>                   /* dump structures */
#include <sys/str_tty.h>                 /* Streams defines */
#include <sys/stream.h>                 /* Streams defines */
#include <sys/stropts.h>                /* Streams defines */
#include <sys/strlog.h>                 /* Streams defines */
#include <sys/strconf.h>                /* Streams defines */
#include <sys/rs.h>                     /* rs's ioctl's */
#include <string.h>                     /* C library string handling function */
#include "common_open.h"                /* Common open discipline functions */
#include <sys/lpio.h>                   /* LPWRITE_REQ and LPWRITE_ACK */

#ifdef _POWER
#include <sys/fp_io.h>
#include <sys/eu.h>
#include "srs.h"                        /* dds for config */

#define DO_PIO

#endif                    /* _POWER */

/* #define srsprintf printf */
#define srsprintf sizeof

#define TRC_RS(w)   ((HKWD_STTY_RS)|w)
enum slih_caller {
    RS_NSLIH, RS_8SLIH };

extern int srs_print(str_rsp_t tp);

#ifdef TTYDBG
static struct str_module_conf
    srs_str_module_conf = { "rs", 'd', RS_PDPF };
#endif	/* TTYDBG */

/* We are supporting several versions of chips in this driver,
 * 2 producers (NSC/VLSI), 4 different native planars, and the ibm boards.
 * All basically the same and very different.
 *
 *  NSC 16550  - native io rel1 product (sio 0x5fdf)
 *  NSC 16552  - ibm mca boards
 *  NSC 16553  - native io rel2 product (sio1 0xe6de)
 * VLSI 16552  - native io rel3 product (sio2 0xfef6)
 *  NSC 16550  - native io pegasus product (sio3 0xd9fe)
 *
 * All are supposed to behave like the 550, but there are (sigh) exceptions.
 *    ::Note 1::
 * 1) The 553 has an afr (alternate function register) that needs to have
 *    the clock set to div 16 and the OUT2 bit set to ignore.  The OUT2 needs
 *    to be set to ignore to work in LOOP BACK MODE.
 *    ::Note 2::
 * 2) Interrupts do not work when the VLSI 552 is in LOOP BACK MODE.
 *    ::Note 3::
 * 3) The VLSI 552 does not have an afr, but does not ignore attempts to set
 *    it, even though the dlab bit is set.  The NSC 550/552 ignores attempt
 *    to set the afr.
 *    ::Note 4::
 * 4) The VLSI 552 needs OUT2 set to enabled interrupts (not documented).
 *    ::Note 5::
 * 5) The VLSI 552 sometimes reports an interrupt status of (4) fifo full
 *    when it really means (12) character timeout.
 *    (have you figured out that the VLSI really isn't suited for us?)
 *    ::Note 6::
 * 6) The sio2 board pos registers don't match the pos registers for
 *    the sio and sio1 products.
 *    Specifically, sio/sio1 use pos4 (bits 4,5) to enable the chip, and
 *    sio2 uses pos5 (bits 0,1) to enable the chip.
 *    ::Note 7::
 * 7) The VLSI 552 reports overrun status on the next received character.
 *    This is different than the NSC 552 that reports it right away.
 *    ::Note 8::
 * 8) The sio3 represents the 3 native lines on the Super I/O III chip on
 *    Pegasus machine.
 *    ::Note 9::
 * 9) With the VLSI chip, defect 167884 was opened because a NIST test
 *    case failed.  The failure was because when characters with bad
 *    parity were received, sometimes, the chip would report the
 *    characters as having good parity.  It was not exactly proven but
 *    just assumed that the LSR/data pairs were being pulled off the
 *    chip too fast.  So the conditional break was added to break out
 *    of the tight inner loop.  The second level loop will catch us
 *    since there will still be interrupts pending.  This slowed the
 *    processing enough to make the LSR have time to be updated
 *    correctly and the test cases all passed.
 */


/*
 * This structure is used to keep adapter configuration information
 * to be used for all the lines on it
 */
struct adap_conf {
    struct rs_adap_dds dds;
    struct adap_conf * next;
};

/*
 * from the id registers we can find out what we have, which slih
 * routine to call and how many ports are on the card.
 */
struct adap_info {
    enum adap_type ra_type;        /* type of adapter */
    uchar ra_id[2];                /* id number */
    char *ra_name;                 /* name for debugging */
    struct intr *ra_intr;          /* config interrupt structure */
    int ra_num;                    /* ports on the adapter */
    ulong *ra_offsets;             /* port offsets */
};


struct rs_intr {                   /* what I give to i_init */
    struct intr ri_intr;           /* must be 1st field */
    struct rs_intr *ri_next;       /* I'll keep a chain of them */
    int ri_use;                    /* use count for free'ing */
    ulong ri_arb;                  /* offset to arbitration register */
    rymp_t *ri_rsp;                /* vector of ym structs */
} *srs_intr_chain;

/*
 * each iocc can have a full compliment of boards so as new boards are
 * developed they should be added in.  I am currently assuming the 8
 * port cards will have 16 ports in the near future but function the same.
 */
/* For pegasus MCA in the extension cabinet, need to reserve 8 more
 * entris in rs_iocc structure. Will be done soon.
 */
struct rs_iocc {
#ifdef _POWER
    rymp_t iocc_native[4]; /* 3 for the 3 native lines of Pegasus,
                              forth to mark end in srs_nslih_peg polling */
    rymp_t iocc_eight[16/* ports */ * 8/* adapters */];
#endif /* _POWER */
};

/*
 * In case you are really confused by this point, lets recap.  We
 * have a vector indexed by SEG2IOCC(buid) which points to rs_iocc
 * struct's.  These structs have a full range of pointers to ym
 * struct's.  The first time an iocc is mentioned, we will allocate an
 * iocc struct and hook it on.  The first time an adapter is
 * mentioned, we will hook on the ym structs.  The reason for the
 * complication is because each slih (represented by its rs_intr
 * struct) will point at one of the vectors of ym structs for the
 * iocc its represents.  slih's do not compare equal if they are for
 * different iocc's.  They also do not compare equal for different
 * interrupt request levels.  But for the 8/16 port cards, we need to
 * get to all of the ym structs for a particular iocc from any
 * slih.  This is because the interrupt arbitration looks at the
 * adapter id and not the interrupt request line.
 */
/* #define Port(rsp) ((struct srs_port *)(io_att(rsp->ry_intr.bid, rsp->ry_port))) */

#define SRS_DECLARES
#ifdef SRS_DECLARES

#define local /* static */

/* Points into the tty structs for an adapter. */
typedef struct {
    char   te_name[DEV_NAME_LN+1];  /* Adapter name */
    int    te_num;                  /* number of entries */
    int    te_priority;             /* interrupt class */
    ushort te_bus;                  /* bus type */
    ushort te_flags;                /* flags for slih */
    uchar  te_anum;                 /* adapter number */
    dev_t  te_parent;               /* expansion unit devno */
    ulong  te_arb;                  /* interrupt arbitration reg addr */
    str_rsp_t te_edge[1];           /* the entries */
} stty_edge;

/* Lots of checks for open and close */
#define dev2tp(dev, tp) \
{ \
    stty_edge *te; \
    int temp; \
    int mnr = minor(dev); \
    tp = ((temp = dev2slot(mnr)) >= srs_cnt || !(te = srs_tty[temp]) || \
         (temp = dev2port(mnr)) >= te->te_num) ? 0 : te->te_edge[temp]; \
}

/* quick dev 2 tp for everything after the open */
#define qdev2tp(dev, tp)  \
  (tp = srs_tty[dev2slot(minor(dev))]->te_edge[dev2port(minor(dev))])

/*
 * =============================================================================
 * Global variables
 * =============================================================================
 */
local stty_edge **srs_tty;
local int srs_cnt;
local struct xmem srs_xmem = { XMEM_GLOBAL, 0};

/*
 * =============================================================================
 * Functions declarations
 * =============================================================================
 */
local int srsopen(queue_t *q,dev_t *devp, int mode, int sflag, cred_t *credp);
local int srsclose(queue_t *q, int mode, cred_t *credp);
local int srs_ioctl(queue_t *q, mblk_t *mp, int fromput);
local int srs_ctl(queue_t *q, mblk_t *mp);
local int srs_termios_set(str_rsp_t tp,struct termios * tios);
local int srs_termiox_set(str_rsp_t tp,struct termiox * tiox);
local int srs_termios_get(str_rsp_t tp,struct termios * tios);
local int srs_termiox_get(str_rsp_t tp,struct termiox * tiox);
local int srs_break_set(str_rsp_t tp, int duration);
local int srs_break_clear(str_rsp_t tp);
extern int srsconfig(dev_t dev, int cmd, struct uio *uio);
local int srsadd(dev_t dev, struct rs_adap_dds *adapConf, struct rs_line_dds *lineConf);
local int srsdel(dev_t dev);
local int srsrsrv(queue_t *q);
local int srswput(queue_t *q,mblk_t *mp);
local int srswsrv(queue_t *q);
local int srs_proc(str_rsp_t tp, enum proc_commands cmd);
local int srs_service(str_rsp_t tp,  enum service_commands cmd,
					  void *varg);
local int srs_offlevel(struct intr *rintr);
local mblk_t *srs_allocmsg(str_rsp_t tp);
local int srs_recover(str_rsp_t tp);
local int srs_timeout(str_rsp_t tp);
int srs_clearbusy(rymp_t rsp);
#ifdef _POWER
local int srs_nslih(struct intr *rintr);
local int srs_nslih_peg(struct intr *rintr);
local int srs_8slih(struct intr *rintr);
local int srs_xboxopen(dev_t parent);
local int srs_xboxclose();
local int srs_setpos(uchar where, uchar which, uchar how);
local int srs_getpos(uchar where, uchar which, uchar *value);
local int srs_getvpd(rymp_t rsp, struct uio *uio, dev_t parent);
#endif /* _POWER */

#ifdef DO_PIO
local void srs_pio_catch(rymp_t rsp, pud_t *pud, int logit);
#endif /* DO_PIO */

/*
local int srs_print(str_rsp_t tp, int v);
local void srs_bits(int f, char **p);
 */
local void srs_err(char *name, int code, int err);

#define DOWATCH /* */
#ifdef DOWATCH
local void srs_set_bit(int which);
local void srs_clr_bit(int which);
local void srs_dowatch(int i, int which);
local void srs_watchdog(struct trb *ticker);
#define swatchmalloc(x) pinmalloc(x)
#define swatchfree(x) pinfree(x)
#else
#define srs_dowatch(a, b)
#define swatchmalloc(x) malloc(x)
#define swatchfree(x) free(x)
#endif

#ifdef SRS_TIMEX
local void srs_xmit(struct trb *trb);
#endif

#define CONFIG_TIME -1

/*
 * =============================================================================
 * Interruption structures
 * =============================================================================
 */
#ifdef _POWER
/* setup intr structs for native i/o */
local struct intr sn_intr = {
    0,                    /* next        */
    srs_nslih,            /* handler     */
    BUS_MICRO_CHANNEL,    /* bus_type    */
    0,                    /* flags       */
    2,                    /* level       */
    CONFIG_TIME,          /* priority    */
    CONFIG_TIME,          /* bus_id      */
};

/* setup intr structs for native i/o of pegasus */
local struct intr sn_intr_peg = {
    0,                  /* next        */
    srs_nslih_peg,      /* handler     */
    BUS_MICRO_CHANNEL,  /* bus_type    */
    INTR_MPSAFE,        /* flags       */
    2,                  /* level       */
    CONFIG_TIME,        /* priority    */
    CONFIG_TIME,        /* bus_id      */
};


local ulong native_offsets[3] = { 0x0, 0x8, 0x50};

/* setup intr struct for 8 ports */
local struct intr seight_intr = {
    0,                  /* next        */
    srs_8slih,          /* handler     */
    BUS_MICRO_CHANNEL,  /* bus_type    */
    INTR_MPSAFE,        /* flags       */
    CONFIG_TIME,        /* level       */
    CONFIG_TIME,        /* priority    */
    CONFIG_TIME,        /* bus_id      */
};
local ulong sixteen_offsets[16] = {
    0x0000, 0x0008, 0x0010, 0x0018, 0x0020, 0x0028, 0x0030, 0x0038,
    0x0800, 0x0808, 0x0810, 0x0818, 0x0820, 0x0828, 0x0830, 0x0838,
};
#endif /* _POWER */

/* ====================== */
/* For configuration time */
/* ====================== */
/* Pointer on adapter configuration structures */
local struct adap_conf * adap_conf_s = NULL;

local struct adap_info sadap_infos[] = {
#ifdef _POWER
    { NativeA_SIO, { 0x5f, 0xdf}, "native i/o", &sn_intr, 2, native_offsets},
    { NativeB_SIO, { 0x5f, 0xdf}, "native i/o", &sn_intr, 2, native_offsets},
    { NativeA_SIO1, { 0xe6, 0xde}, "native i/o", &sn_intr, 2, native_offsets},
    { NativeB_SIO1, { 0xe6, 0xde}, "native i/o", &sn_intr, 2, native_offsets},
    { NativeA_SIO2, { 0xfe, 0xf6}, "native i/o", &sn_intr, 2, native_offsets},
    { NativeB_SIO2, { 0xfe, 0xf6}, "native i/o", &sn_intr, 2, native_offsets},
    { NativeA_SIO3, { 0xd9, 0xfe}, "native i/o", &sn_intr_peg, 3,
                                                              native_offsets},
    { NativeB_SIO3, { 0xd9, 0xfe}, "native i/o", &sn_intr_peg, 3,
                                                              native_offsets},
    { NativeC_SIO3, { 0xd9, 0xfe}, "native i/o", &sn_intr_peg, 3,
                                                              native_offsets},
	
    { Eight_232, { 0xd0, 0xed}, "rs-232 8port", &seight_intr, 8,
		sixteen_offsets},
    { Eight_422, { 0xd1, 0xed}, "EIA-530 8port", &seight_intr, 8,
		sixteen_offsets},
    { Eight_Mil, { 0xd2, 0xed}, "MIL-188 8port", &seight_intr, 8,
		sixteen_offsets},
    { Sixteen_232, { 0xd6, 0xed}, "16 port 232", &seight_intr, 16,
		sixteen_offsets},
    { Sixteen_422, { 0xd3, 0xed}, "16 port 422", &seight_intr, 16,
		sixteen_offsets},
#endif /* _POWER */
    { Unknown_Adap, 0, 0, "Unknown" },
};

/* a vector of iocc's that we know about */
local struct rs_iocc *srs_ioccs[IOCC_CNT];

#ifdef _POWER
/* a vector which maps the eight ports intr settings into bus levels */
local int srs_intr8[] = {
    -1, -1, -1, 0, -1, 1, -1, -1, -1, 2, 3, 4, 5, -1, 6, 7};

local struct file *srs_xbox;             /* file pointer to expansion box */
local int (*sxbox_assist)(int *mask);    /* pointer to xbox assist routine */
local dev_t srs_xdevt;                   /* dev_t for expansion box */
local int sxbox_opencnt = 0;             /* xbox open/close counter */
local lock_t sxbox_lock = SIMPLE_LOCK_AVAIL;    /* xbox lock */
#endif /* _POWER */

extern ulong d_offset;            /* #### temp for debugger hook */
extern int dbg_pinned;            /* must be true to access d_offset */
char sresource[16] = "rs";
int smin_priority = INT_TTY;
static int clr_busy_wdog;         /* watch dog to reset t_busy for the line
                                     accessed at the same time by the low level
                                     debuger */

/*
 * =============================================================================
 * Streams declarations.
 * =============================================================================
 */
static struct module_info sminfo = {
    DRIVER_ID,"rs",0,INFPSZ,1024,128
  };

static struct qinit srs_rinit = {
    NULL,srsrsrv,srsopen,srsclose,NULL,&sminfo,NULL
  };

static struct qinit srs_winit = {
    srswput,srswsrv,NULL,NULL,NULL,&sminfo,NULL
  };

static struct streamtab srsinfo = { &srs_rinit,&srs_winit,NULL,NULL
								  };

static strconf_t srsconf = {
    "rs",                           /* sc_name    */
    &srsinfo,                       /* sc_str    */
    STR_NEW_OPEN|STR_MPSAFE,        /* sc_flags    */
    0,                              /* sc_major    */
    SQLVL_QUEUE,                    /* sc_sqlevel    */
    0                               /* sc_sqinfo    */
  };

/*
 * tioc_reply structures array. It is given as an answer to TIOC_REQUEST
 * M_CTLs.
 */

static struct tioc_reply
srs_tioc_reply[] = {
        { RS_SETA, sizeof(struct rs_info), TTYPE_COPYIN },
        { RS_GETA, sizeof(struct rs_info), TTYPE_COPYOUT },
};


#endif /* SRS_DECLARES */


#define SRS_HWAT          MAX_CANON-CLSIZE    /* Constants, for the moment */
#define SRS_LWAT          CLSIZE


/*
 * srsopen
 *    Checks if the minor number corresponds to a configured port.
 *    Links the queues to the str_rs structure.
 *    Initializes the line.
 *    Calls the open entrypont of the open discipline.
 *
 * ARGUMENTS:
 *      q       - address of STREAMS read queue
 *      devp    - pointer to serial line device number
 *      flag    - open(2) flags (Read|Write)
 *      sflag   - type of STREAMS open (MODOPEN, 0 [normal], CLONEOPEN)
 *      credp   - pointer to cred structure
 *
 * RETURN VALUE:
 *
 *      ENODEV  - no such serial line
 *
 */

local int 
srsopen(queue_t *q, dev_t *devp, int mode, int sflag, cred_t *credp)
{
    DoingPio;
    register rymp_t rsp;
    register str_rsp_t tp;
    register int err=0;
    int lrc;
    int old_intr_soft, old_intr_hard;
    int old_hupcl;

	
    Enter(TRC_RS(TTY_OPEN), *devp, q->q_ptr, mode, sflag, 0);
	
    dev2tp(*devp, tp);
    if (!tp)
	  Return(ENODEV);
    if (!(rsp = tp->t_hptr)) {
		srs_err(sresource, ERRID_TTY_PROG_PTR, 0);
		Return(ENODEV);
    }
	
    if (!rsp->ry_conf)            /* Port is not really configured */
	  Return(ENODEV);
	srsprintf("Port is configured one\n");
	
    if(tp->t_dev != *devp) Return(ENODEV);
	
		
    if ((!rsp->ry_open) && (!tp->t_wopen)) {        /* first open */
	register struct srs_port *port;
		
	srsprintf("First open\n");
        if ((err = pin(rsp, sizeof(*rsp))) ||
                (err = pin(tp, sizeof(*tp)))) {
                unpin(rsp, sizeof(*rsp));
                Return(err);
        }

        old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
	tp->t_wopen++;

	port = Port(rsp);
        old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);

	StartPio(srs_pio_catch, rsp, err = EIO; break);
	SR(port->r_fcr, 0);
	SR(port->r_fcr, rsp->ry_rtrig | FIFO_ENABLE);
	rsp->ry_msr = GR(port->r_msr) & 0xf0;
	SR(port->r_ier, ERBDAI|ELSI|EMSSI);
	/* On the VLSI 16552, OUT2 needs to be set to enable interrupts
	 * THIS IS NOT DOCUMENTED IN THE VLSI 16552 SPEC !!!
	 * This also has the side effect of TSCD showing in the modem
	 * status bits when in LOOP MODE.
         * See ::Note 4::
	 */
        if (rsp->ry_type != NativeC_SIO3) { /* Added because of 142177 */
	SR(port->r_mcr, ((rsp->ry_mil) || IsNative(rsp->ry_type) ?
			 ((GR(port->r_mcr) & ~LOOP) | OUT2) :
			 (GR(port->r_mcr) & ~(LOOP|OUT2))));
        }
	/* On 16553, need to set afr to ignore output of out2 bit.
	 * Tested other configurations of out2 set/unset and they
	 * did not work.  This must be a hardware problem.
	 * See ::Note 1:: ::Note 3::
	 */
        if ((NativeA_SIO1 == rsp->ry_type) || (NativeB_SIO1 == rsp->ry_type)) {
	    set_dlab(port);
	    SR(port->r_afr, (GR(port->r_afr) | DISAB_DIV13 | LOOP_NOCARE));
	    clr_dlab(port);
        }
	EndPio();
        unlock_enable(old_intr_hard, &tp->t_lock_hard);
	io_det(port);
	if (err) {
                unlock_enable(old_intr_soft, &tp->t_lock_soft);
		Return(err);
	}
		
	/*
         * Push default values down into the chip
         */
        srs_termiox_set(tp, &(tp->t_termiox));
        srs_termios_set(tp, &(tp->t_termios));
		
        q->q_ptr = (caddr_t)tp;
        WR(q)->q_ptr = (caddr_t)tp;
        tp->t_ctl = q;
	rsp->ry_fcnt = rsp->ry_tbc;	/* 156569 */
	rsp->fake_xmit = 0;		/* 156569 */
	rsp->force_xmit = 0;		/* 156569 */
	
        /* Allocate a message in advance to contain received data */
        if (!tp->t_inmsg) {
            srs_allocmsg(tp);
        }
        if (IsNative(rsp->ry_type) && (dbg_pinned) && (rsp->ry_port == d_offset)) {
            if (!(clr_busy_wdog)) {
                clr_busy_wdog = timeout(srs_clearbusy, rsp, 10*hz);
            }
        }
        unlock_enable(old_intr_soft, &tp->t_lock_soft);
    }
    else {     /* If exclusive open mode is set, check the credentials */
        if ((tp->t_excl) && (credp->cr_uid != 0)) {
            Return(EBUSY); /* Resouce busy */
        }
    }


    /* open discipline: will wit for the CD_ON if OPEN_REMOTE and mode is
       not DNDELAY */
    old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
    tp->t_wopen++;
    unlock_enable(old_intr_soft, &tp->t_lock_soft);
    if (err = openDisc_open(&tp->t_openRetrieve, tp, tp->t_open_disc, srs_service,
                      (tp->t_clocal) ? OPEN_LOCAL : OPEN_REMOTE,
                      mode, !tp->t_isopen, &tp->t_event)) {
        old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
        srsprintf("Problem during function open of open discipline.\n");
        tp->t_wopen--;
        if  ((!rsp->ry_open) && (tp->t_wopen == 1)) {  /* The only open in progress */
            rsp->ry_open = 1; /* Just to make srsclose() make the full cleaning */
            old_hupcl = (tp->t_hupcl);  /* () because not sure how will the 
                                           interprete it */
            tp->t_hupcl = 1;  /* To ensure dropping DTR and RTS */
            unlock_enable(old_intr_soft, &tp->t_lock_soft);
            srsclose(q, mode, credp);
            old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
            tp->t_hupcl = old_hupcl;  /* To ensure dropping DTR and RTS */
        }
        unlock_enable(old_intr_soft, &tp->t_lock_soft);
        Return(err);
    };
    old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
    tp->t_wopen--;
		
    rsp->ry_open = 1;
    tp->t_isopen = 1;
    unlock_enable(old_intr_soft, &tp->t_lock_soft);
	
    Return(err);
}


/*
 * srsclose
 *      Closes current open discipline.
 *      Flushs queues and disconnects them from private structures.
 *      Cansels the pending bufcall and timeout requests.
 *
 *
 * ARGUMENTS:
 *      q       - address of STREAMS read queue
 *      mode    - type of STREAMS open (MODOPEN, 0 [normal], CLONEOPEN)
 *      credp   - pointer to cred structure
 *
 * RETURN VALUE:
 *
 *      ENODEV  - no such serial line
 *
 */

local int 
srsclose(queue_t *q, int mode, cred_t  *credp)
{
    register str_rsp_t tp = (str_rsp_t)q->q_ptr;
    dev_t dev = tp->t_dev;
    DoingPio;
    register rymp_t rsp;
    register caddr_t base_page;
    register struct srs_port *port;
    register int err = 0;   /* priviously initialized with call to ttyclose */
    register volatile must_sleep = 0; /* force reading of tp->t_draining
                                         each time we are woke up */
    int lrc;
    int old_intr_soft, old_intr_hard;
	
    Enter(TRC_RS(TTY_CLOSE), dev, tp, mode, 0, 0);
	
    if (!tp)
	  Return(ENODEV);
    if (!(rsp = tp->t_hptr)) {
	srs_err(sresource, ERRID_TTY_PROG_PTR, 0);
	Return(ENODEV);
    }
	
    if (!rsp->ry_open)             /* was this even open ? */
	  Return(EINVAL);

    /* At least one process is opening this tty, if it finishes
       its open successfully, it won't reinitialize everything.
       The last process leaving unseccessfully the openDisc_open()
       will call srsclose() */
    if (tp->t_wopen > 1) { 
        Return(0);
    }

    old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);

    /* Unblock the receive side and flush the input events */
    if (tp->t_block) srs_proc(tp, T_UNBLOCK);
    srs_proc(tp,T_RFLUSH);
	
    /* Drain data in write queue before leaving
    /* Output is resumed if t_localstop is set, that is, an M_STOP
       message was recieved.
       If t_localstop is clear, that means we are flow-controlled by
       a remote device, which will unblock the situation when it is
       ready */

    if (tp->t_draining = (tp->t_outmsg) || (WR(q)->q_first)) {
        if (tp->t_localstop) {
            srs_proc(tp, T_RESUME);
        }
	if (rsp->fake_xmit) {								/* 156569 */
        	tp->t_busy = 1;								/* 156569 */
        	base_page = io_att(rsp->ry_intr.bid, 0);			 	/* 156569 */
        	port = (struct srs_port *)(base_page + rsp->ry_port);			/* 156569 */
        	old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);		/* 156569 */
        	StartPio(srs_pio_catch, rsp, err = EIO; break);				/* 156569 */
        	SR(port->r_ier, GR(port->r_ier)|ETHREI);				/* 156569 */
        	EndPio();								/* 156569 */
        	unlock_enable(old_intr_hard, &tp->t_lock_hard);				/* 156569 */
        	io_det(base_page);							/* 156569 */
	}										/* 156569 */
        while (must_sleep = tp->t_draining) {
	   rsp->force_xmit = 1;								/* 156569 */
           e_assert_wait(&(tp->t_event), INTERRUPTIBLE);
           unlock_enable(old_intr_soft, &tp->t_lock_soft);
           if (e_block_thread() != THREAD_AWAKENED) {
               srs_flush(tp, FLUSHW, 0);                                                 /* 169065 */
               old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
               tp->t_draining = 0;
               err = EINTR;
               break;
           }
           old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
        }
    }
/* Go back to bed if finished draining out data but the remote device is not ready for more
   output. This is to prevent overwriting to the device after the next open */
        tp->t_draining = (tp->t_stop && (tp->t_softchars[STARTCHAR] != _POSIX_VDISABLE));
        while (must_sleep = tp->t_draining) {
           e_assert_wait(&(tp->t_event), INTERRUPTIBLE);
           unlock_enable(old_intr_soft, &tp->t_lock_soft);
           if (e_block_thread() != THREAD_AWAKENED) {
               old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
               tp->t_draining = 0;
               err = EINTR;
               break;
           }
           old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
        }


    tp->t_flow_state = 0;
    tp->t_hardpacing = tp->t_softpacing = 0;

    tp->t_stop=0;    
    tp->t_isopen=0;    
    tp->t_wopen=0;
	
    base_page = io_att(rsp->ry_intr.bid, 0);
    port = (struct srs_port *)(base_page + rsp->ry_port);
    old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);
    StartPio(srs_pio_catch, rsp, err = EIO; break);
    SR(port->r_ier, 0);
    EndPio();
    unlock_enable(old_intr_hard, &tp->t_lock_hard);
    	
    io_det(base_page);
	
    srs_dowatch(0, minor(dev));

    unlock_enable(old_intr_soft, &tp->t_lock_soft);

    /* Call open discipline close */
    if (openDisc_close(tp->t_openRetrieve, tp->t_hupcl)) {
        srsprintf("Problem during function close of open discipline.\n");
    }

    old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);

    rsp->ry_xcnt = 0;  /* clean all the output staff */
    tp->t_busy = 0;
    tp->t_localstop = 0;

    rsp->ry_open = 0;
    tp->t_ctl = 0;    
	
    tp->t_openRetrieve = 0;

    tp->t_sched = 0;
	
	/* Don't forget bufcall and timeout pending requests */
    if (tp->t_bufcall) {
        unbufcall(tp->t_bufcall);
        tp->t_bufcall = 0;
    }
    if (tp->t_timeout) {
        untimeout(tp->t_timeout);
        tp->t_timeout = 0;
    }
    if (tp->t_alloctimer) {
        untimeout(tp->t_alloctimer);
        tp->t_alloctimer = 0;
    }
    /* No need for locking now since neither an interrupt nor bufcall answer
       is expected 
     */
    /* In normal cases this is not needed, since we drop output, before
       leaving. But if we exit the e_block_theread with THREAD_INTERRUPTED, and
       interrupts are disabled, t_outmsg isn't freed.
    */
    if (tp->t_outmsg) {
        freemsg(tp->t_outmsg);
        tp->t_outmsg = 0;
    }
    if (tp->t_inmsg) {
        freemsg(tp->t_inmsg);
        tp->t_inmsg = 0;
    }
    unlock_enable(old_intr_soft, &tp->t_lock_soft);

    if (IsNative(rsp->ry_type) && (dbg_pinned) && (rsp->ry_port == d_offset)) {
        if (clr_busy_wdog) {
            untimeout(clr_busy_wdog);
            clr_busy_wdog = 0;
        }
    }

    assert(!unpin(tp, sizeof(*tp)));
    assert(!unpin(rsp, sizeof(*rsp)));

    Return(err);
}

/*
 * srs_flush
 *      Flushs read  and write queues, accordung to rw parameter.
 *      if message is set MFLUSH it is sent upstream.
 *
 *
 * ARGUMENTS:
 *      tp      - address of str_rs structure.
 *      rw      - FLUSHW or FLUSHR flags.
 *      recycle - if set, sent an M_FLUSH with FLUSHR.
 *
 * RETURN VALUE:
 *
 *      None.
 *
 */

local int
srs_flush(str_rsp_t tp,int rw,int recycle)
{
    int old_intr_soft, old_intr_hard;
	
    if (rw & FLUSHW) {
        flushq(WR(tp->t_ctl),FLUSHDATA);
        old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
        srs_proc(tp,T_WFLUSH);
        unlock_enable(old_intr_soft, &tp->t_lock_soft);
    }
    if (rw & FLUSHR) {
        flushq(tp->t_ctl,FLUSHDATA);
        old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
        srs_proc(tp,T_RFLUSH);
        unlock_enable(old_intr_soft, &tp->t_lock_soft);
        if (recycle)
            putctl1(tp->t_ctl->q_next,M_FLUSH,FLUSHR);
    }
}

/*
 * srs_ioctl
 *      Processes the M_IOCTL messages.
 *      Commands to be perfomed by service procedure are put
 *      back in the queue.
 *      An M_IOCAK is sent upstreams if successfull processing,
 *      otherwise an M_IONACK with error code.
 *
 *
 * ARGUMENTS:
 *      q       - address of STREAMS write queue
 *      mp      - M_IOCTL to process.
 *      fromput - 1 if called by put procedure.
 *
 * RETURN VALUE:
 *
 *      0 if ioctl message is successfully processed or delayed to
 *      the service procedure. 
 *      -1 if bad ioctl.
 *      1 if unable to process the ioctl immediately by service routine
 *
 */

local int 
srs_ioctl(queue_t *q, mblk_t *mp,int fromput)
{
    DoingPio;
    struct iocblk *iocp;
    register str_rsp_t tp = (str_rsp_t)q->q_ptr;
    register rymp_t rsp;
    register struct srs_port *port;
    struct rs_info rs_info, *rinfo;
    int err = 0, er2;
    int old_intr_soft, old_intr_hard;
    unsigned char outtype;
    unsigned long outcnt;
    struct termios *tios;
    struct termiox *tiox;
    int signal_temp;
	
    Enter(TRC_RS(TTY_IOCTL), tp->t_dev, tp,
          ((struct iocblk *)mp->b_rptr)->ioc_cmd, 0, 0);
	
    iocp = (struct iocblk *)mp->b_rptr;
	
    if (iocp->ioc_count == TRANSPARENT) {
        mp->b_datap->db_type = M_IOCNAK; /*Not Okay */
        iocp->ioc_error = EINVAL;
        qreply(q,mp);
        Return(-1);
    }
    iocp->ioc_error =0;
    outtype = M_IOCACK;
    outcnt = 0;
	
    srsprintf("srs_ioctl cmd = %x\n",iocp->ioc_cmd);
    if (!fromput) /* from service routine */
        switch (iocp->ioc_cmd) {
	  case TIOCSETAW:
	  case TIOCSETAF:
	      if (iocp->ioc_count != sizeof (struct termios) || !mp->b_cont) {
	          outtype = M_IOCNAK;
	       	  iocp->ioc_error = ENOSPC;
	          break;
	      }
	      /* Wait for the o/p to drain and flush I/p and set params */
	      if (iocp->ioc_cmd == TIOCSETAF) {
	    	  flushq(RD(q),FLUSHDATA);
	    	  putctl1(tp->t_ctl->q_next,M_FLUSH,FLUSHR);
	      }
	      srs_termios_set(tp,(struct termios *)mp->b_cont->b_rptr);
	      break;

	  case TCSBRK:
	  case TCSBREAK:
          {   int arg;
	      if(iocp->ioc_count != sizeof (int) || !mp->b_cont) {
	    	  outtype = M_IOCNAK;
	    	  iocp->ioc_error = ENOSPC;
	    	  break;
	      }
	      arg = *(int *)mp->b_cont->b_rptr;

              /* No break to be set. Just drain output */
              if (iocp->ioc_cmd == TCSBRK && arg) {
                  /* Draining not possible if output stopped.
                     What if nothing to drain? */
                  if (tp->t_stop) {
                      /* srswsrv will be scheduled either by an M_START 
                         message, or a remote flow control condition, like
                         the reception of an XON character. */
                      Return(1);
                  }
                  break;
              }

              if (arg == 0) {
                  arg = (250 * HZ) / 1000; /* break for 0.25 seconds. */
              } else {
                  arg = (arg < 10 ? 10 : arg)*HZ/1000;
              }
              /* send a break */
              tp->t_ioctlmsg = mp; /* Will reappear, when PSE handling of
                 timeouts on ioctls is clearer. */
              if (srs_break_set(tp, arg))
		{
		  tp->t_ioctlmsg = NULL;
	    	  outtype = M_IOCNAK;
	    	  iocp->ioc_error = EAGAIN;
	    	  break;
		}
              Return(0);    /* M_IOCACK will be replyed after timeout */
              /* break; */
          }
          case RS_SETA:
	      if (iocp->ioc_count != sizeof (struct rs_info) || !mp->b_cont) {
	    	  outtype = M_IOCNAK;
	    	  iocp->ioc_error = ENOSPC;
	    	  break;
	      }
	      rsp = tp->t_hptr;
		  
	      rs_info = *(struct rs_info *)mp->b_cont->b_rptr;
	      if (rs_info.rs_rtrig > 3 || rs_info.rs_rtrig < 0 ||
	    	  rs_info.rs_tbc < 0 || rs_info.rs_tbc > 16) {
	    	  outtype = M_IOCNAK;
	    	  iocp->ioc_error = EINVAL;
	    	  break;
	      }
		  
#ifdef _POWER
	      if (rs_info.rs_mil && rsp->ry_type != Eight_Mil) {
	    	  outtype = M_IOCNAK;
	    	  iocp->ioc_error = EINVAL;
	    	  break;
	      }
	      
	      if (rs_info.rs_mil != rsp->ry_mil) {
	    	  port = Port(rsp);
	    	  StartPio(srs_pio_catch, rsp, io_det(port); Return(EIO));
	    	  if (rs_info.rs_mil)        /* set invert mode */
	    		SR(port->r_mcr, GR(port->r_mcr)|OUT2);
	    	  else
	    		SR(port->r_mcr, GR(port->r_mcr)&~OUT2);
	    	  EndPio();
	    	  io_det(port);
	    	  rsp->ry_mil = rs_info.rs_mil;
	      }
#endif /* _POWER */
		  
	      rs_info.rs_rtrig <<= 6;
	      if (rs_info.rs_tbc == 0)
	       	 rs_info.rs_tbc = 16;
	      rsp->ry_tbc = rs_info.rs_tbc;
		  
	      if (rs_info.rs_rtrig == rsp->ry_rtrig &&
	    	  rs_info.rs_mil == rsp->ry_mil)
	    	  break; 
		  
              if (rs_info.rs_rtrig != rsp->ry_rtrig) {
	    	  port = Port(rsp);
                  old_intr_hard = disable_lock(tp->t_priority,
                                               &tp->t_lock_hard);
	    	  StartPio(srs_pio_catch, rsp,
                           io_det(port);
                           unlock_enable(old_intr_hard, &tp->t_lock_hard);
                           Return(EIO));
    	          SR(port->r_fcr, rsp->ry_rtrig | FIFO_ENABLE);
		  EndPio();
                  unlock_enable(old_intr_hard, &tp->t_lock_hard);
	          io_det(port);
    		  rsp->ry_rtrig = rs_info.rs_rtrig;
    	      }
	      break;

    	 case TCSETXF:
    	 case TCSETXW:
	      if (iocp->ioc_count != sizeof (struct termiox) || !mp->b_cont) {
	          outtype = M_IOCNAK;
	       	  iocp->ioc_error = ENOSPC;
	          break;
	      }
	      /* Wait for the o/p to drain and flush I/p and set params */
	      if (iocp->ioc_cmd == TCSETXF) {
	    	  flushq(RD(q),FLUSHDATA);
	    	  putctl1(tp->t_ctl->q_next,M_FLUSH,FLUSHR);
	      }
	      srs_termiox_set(tp,(struct termiox *)mp->b_cont->b_rptr);
	      break;

         default:
	      /* I should never be here */
	      break;
       }
	
     else {             /* From Put routine */
    	switch (iocp->ioc_cmd) {
    	  case TIOCSETAW:
    	  case TIOCSETAF:
    	  case TCSBRK:
    	  case TCSBREAK:
    	  case RS_SETA:
    	  case TCSETXF:
    	  case TCSETXW:
       	    putq(q,mp);
	    rsp = tp->t_hptr;		/* 156569 */
	    rsp->force_xmit = 1;	/* 156569 */
            Return(0);

    	  case TIOCGETA:
    	    if (iocp->ioc_count < sizeof(struct termios) || !mp->b_cont) {
       	        outtype = M_IOCNAK;
		iocp->ioc_error = ENOSPC;
		break;
	    }
            tios= (struct termios *)mp->b_cont->b_rptr;
    	    bzero((char * )tios,sizeof(struct termios));
            srs_termios_get(tp,tios);    
            outcnt = sizeof(struct termios);
            break;

          case TIOCSETA:
    	    if (iocp->ioc_count < sizeof(struct termios) || !mp->b_cont) {
                outtype = M_IOCNAK;
            	iocp->ioc_error = ENOSPC;
                break;
            }
            tios= (struct termios *)mp->b_cont->b_rptr;
            srs_termios_set(tp,tios);
    	    break;

    	  case RS_GETA:
            if (iocp->ioc_count < sizeof(struct rs_info) || !mp->b_cont) {
            	outtype = M_IOCNAK;
    	    	iocp->ioc_error = ENOSPC;
    	    	break;
    	    }
            rinfo= (struct rs_info *)mp->b_cont->b_rptr;
            bzero((char * )rinfo,sizeof(struct rs_info));
            rsp = tp->t_hptr;
            rinfo->rs_dma = 0;
            rinfo->rs_rtrig = rsp->ry_rtrig >> 6;
            rinfo->rs_tbc = rsp->ry_tbc;
            rinfo->rs_mil = rsp->ry_mil;
            outcnt = sizeof(struct rs_info);
            break;
			
          case TXTTYNAME:
            if (iocp->ioc_count < TTNAMEMAX || !mp->b_cont) {
            	outtype = M_IOCNAK;
            	iocp->ioc_error = ENOSPC;
            	break;
            }
            bcopy(tp->t_name,(char *)mp->b_cont->b_rptr, sizeof(tp->t_name));
            outcnt=sizeof(tp->t_name);
            break;
			
            /* Set the window size: Just reply a positive acknolegment */
    	  case TIOCSWINSZ:
            break;
			
    	  case TIOCEXCL:
            tp->t_excl = 1;
            break;
			
    	  case TIOCNXCL:
            tp->t_excl = 0;
            break;
			
    	  case TCSAK:
    	    if (iocp->ioc_count < sizeof(int) || !mp->b_cont) {
                outtype = M_IOCNAK;
            	iocp->ioc_error = ENOSPC;
                break;
            }
            tp->t_sak = (*((int *) (mp->b_cont->b_rptr))) ? 1 : 0;
            break;
			
    	  case TIOCSDTR:     
            srs_service(tp, TS_GCONTROL, &signal_temp);
            if (!(signal_temp & TSDTR)) {
            	srs_service(tp, TS_SCONTROL, signal_temp|TSDTR);
            }
            break;

          case TIOCCDTR:
            srs_service(tp, TS_GCONTROL, &signal_temp);
            if (signal_temp & TSDTR) { 
            	srs_service(tp, TS_SCONTROL, signal_temp & ~TSDTR);
            }
            break;
			
          case TIOCMBIS:
            if(iocp->ioc_count != sizeof (int) || !mp->b_cont) {
            	outtype = M_IOCNAK;
            	iocp->ioc_error = ENOSPC;
            	break;
            }
            srs_service(tp, TS_GCONTROL, &signal_temp);
            if (*((int *)mp->b_cont->b_rptr) & TIOCM_DTR) {
            	signal_temp |= TSDTR;
            }
            if (*((int *)mp->b_cont->b_rptr) & TIOCM_RTS) {
            	signal_temp |= TSRTS;
            }
            srs_service(tp, TS_SCONTROL, signal_temp);
            iocp->ioc_count = 0;
            /* outcnt = sizeof(int); */
            break;
			
          case TIOCMBIC:
            if(iocp->ioc_count != sizeof (int) || !mp->b_cont) {
                outtype = M_IOCNAK;
                iocp->ioc_error = ENOSPC;
                break;
            }
            srs_service(tp, TS_GCONTROL, &signal_temp);
            if (*((int *)mp->b_cont->b_rptr) & TIOCM_DTR) {
		signal_temp &= ~TSDTR;
            }
            if (*((int *)mp->b_cont->b_rptr) & TIOCM_RTS) {
		signal_temp &= ~TSRTS;
            }
            srs_service(tp, TS_SCONTROL, signal_temp);
            iocp->ioc_count = 0;
            /* outcnt = sizeof(int); */
            break;
			
          case TIOCMSET:
            if(iocp->ioc_count != sizeof (int) || !mp->b_cont) {
				outtype = M_IOCNAK;
				iocp->ioc_error = ENOSPC;
				break;
            }
            signal_temp = 0;
            if (*((int *)mp->b_cont->b_rptr) & TIOCM_DTR) {
		signal_temp |= TSDTR;
            }
            if (*((int *)mp->b_cont->b_rptr) & TIOCM_RTS) {
		signal_temp |= TSRTS;
            }
            srs_service(tp, TS_SCONTROL, signal_temp);
            iocp->ioc_count = 0;
            /* outcnt = sizeof(int); */
            break;
			
	  case TIOCMGET: {
            int cntl_tmp = 0;
            if(iocp->ioc_count != sizeof (int) || !mp->b_cont) {
		outtype = M_IOCNAK;
		iocp->ioc_error = ENOSPC;
		break;
            }
            rsp = tp->t_hptr;
            signal_temp = 0;
            srs_service(tp, TS_GCONTROL, &cntl_tmp);
            if (cntl_tmp & TSDTR) signal_temp |= TIOCM_DTR;
            if (cntl_tmp & TSRTS) signal_temp |= TIOCM_RTS;
            if (rsp->ry_msr&DCD) signal_temp |= TIOCM_CAR;
            if (rsp->ry_msr&RI) signal_temp |= TIOCM_RNG;
            if (rsp->ry_msr&DSR) signal_temp |= TIOCM_DSR;
            if (rsp->ry_msr&CTS) signal_temp |= TIOCM_CTS;
            *((int *)mp->b_cont->b_rptr) = signal_temp;
            outcnt = sizeof(int);
            break;
            }
			
    	  case TCGETX:
    	    if (iocp->ioc_count < sizeof(struct termiox) || !mp->b_cont) {
       	        outtype = M_IOCNAK;
		iocp->ioc_error = ENOSPC;
		break;
	    }
            tiox = (struct termiox *)mp->b_cont->b_rptr;
    	    bzero((char * )tiox,sizeof(struct termiox));
            srs_termiox_get(tp, tiox);    
            outcnt = sizeof(struct termiox);
	    break;

    	  case TCSETX:
    	    if (iocp->ioc_count < sizeof(struct termiox) || !mp->b_cont) {
                outtype = M_IOCNAK;
            	iocp->ioc_error = ENOSPC;
                break;
            }
            tiox = (struct termiox *)mp->b_cont->b_rptr;
            srs_termiox_set(tp,tiox);
    	    break;

    	  case TIOCOUTQ: {
            mblk_t *mp1;
            int outsize = 0;
    	    if (iocp->ioc_count < sizeof(int) || !mp->b_cont) {
                outtype = M_IOCNAK;
            	iocp->ioc_error = ENOSPC;
                break;
            }
            for (mp1 = q->q_first; mp1; mp1 = mp1->b_next) {
                if (mp1->b_datap->db_type == M_DATA) {
                    outsize += mp1->b_wptr - mp1->b_rptr;
                }
            }
	    rsp = tp->t_hptr;
            outsize += rsp->ry_xcnt;
            * (int *) mp->b_cont->b_rptr = outsize;
            outcnt = sizeof(int);
    	    break;
          }

    	  case TCLOOP:
    	    if (iocp->ioc_count < sizeof(int) || !mp->b_cont) {
                outtype = M_IOCNAK;
            	iocp->ioc_error = ENOSPC;
                break;
            }
            if (*((int *)mp->b_cont->b_rptr) == 1) {
                /* Enter the UART diagnostic loop mode */
                srs_service(tp, TS_LOOP, LOOP_ENTER);
            } else {
                /* Leave loop mode */
                srs_service(tp, TS_LOOP, LOOP_EXIT);
            }
            outcnt = sizeof(int);
    	    break;

    	  default:
            outtype = M_IOCNAK;
            break;
        }
    }
    mp->b_datap->db_type = outtype;
    iocp->ioc_count = outcnt;
    if (mp->b_cont)
	  mp->b_cont->b_wptr = mp->b_cont->b_rptr + outcnt;
    qreply(q,mp);
    Return((outtype == M_IOCACK)?0: -1);
}

/*
 * srs_ctl
 *      Processes the M_CTL messages.
 *      If successfully processed, the same  M_CTL is
 *      possibly sent upstream, otherwise it is freed.
 *
 *
 * ARGUMENTS:
 *      q       - address of STREAMS write queue
 *      mp      - M_CTL to process.
 *
 * RETURN VALUE: 0.
 *
 */

int
srs_ctl(queue_t *q, mblk_t *mp)
{
    mblk_t *mp1;
    struct iocblk *iocp;
    register str_rsp_t tp = (str_rsp_t)q->q_ptr;
    register rymp_t rsp = tp->t_hptr;
    struct termios *tios;
    struct termiox *tiox;
	
    iocp = (struct iocblk *)mp->b_rptr;
	
    switch (iocp->ioc_cmd) {
      case TIOCGETA:
	if ((mp1 = mp->b_cont) &&
            (mp1->b_datap->db_lim - mp1->b_rptr >= sizeof(struct termios))) {
            tios= (struct termios *)mp1->b_rptr;
            bcopy((caddr_t) &(tp->t_termios), (caddr_t) tios, sizeof(struct termios));
            mp1->b_wptr = mp1->b_rptr + sizeof( struct termios);
        } else {
            freemsg(mp);
            return(0);
        }
        break;

      case MC_CANONQUERY:
	if ((mp1 = mp->b_cont) &&
            (mp1->b_datap->db_lim - mp1->b_rptr >= sizeof(struct termios))) {
            tios = (struct termios *)mp1->b_rptr;
            bzero((char * )tios,sizeof(struct termios));
            tios->c_iflag |= (IXON | IXOFF | IXANY);
            iocp->ioc_cmd = MC_PART_CANON;
            mp1->b_wptr = mp1->b_rptr + sizeof( struct termios);
        } else {
            freemsg(mp);
            return(0);
        }
        break;

     case TCGETX:
	if ((mp1 = mp->b_cont) &&
            (mp1->b_datap->db_lim - mp1->b_rptr >= sizeof(struct termiox))) {
            tiox = (struct termiox *)mp1->b_rptr;
    	    bzero((char * )tiox,sizeof(struct termiox));
            srs_termiox_get(tp,tiox);    
            mp1->b_wptr = mp1->b_rptr + sizeof( struct termiox);
        } else {
            freemsg(mp);
            return(0);
        }
	break;

      case TIOC_REQUEST:
      case TIOC_REPLY: {
        register int reply_size = 2 * sizeof(struct tioc_reply);
        iocp->ioc_cmd = TIOC_REPLY;
        if (!(mp1 = allocb(reply_size, BPRI_MED)))
            break; /* just reply with the same message, next RS_SETA and 
                      RS_GETA will arrive transparent and will fail */
        iocp->ioc_count = reply_size;
        bcopy(srs_tioc_reply, mp1->b_rptr, reply_size);
        mp1->b_wptr = mp1->b_rptr + reply_size;
        mp->b_cont = mp1;
        break;
      }
		
      case TIOCGETMODEM:
	if ((mp1 = mp->b_cont) &&
            (mp1->b_datap->db_lim - mp1->b_rptr >= sizeof(char))) {
	    int cntl_tmp = 0;
	    srs_service(tp, TS_GSTATUS, &cntl_tmp);
	    if (cntl_tmp & TSCD)
		*((char *)mp1->b_rptr) = 1;
	    else
		*((char *)mp1->b_rptr) = 0;
            mp1->b_wptr = mp1->b_rptr + sizeof(char);
	} else {
            freemsg(mp);
            return(0);
        }
        break;
		
      case TIOCMGET:
	if ((mp1 = mp->b_cont) &&
            (mp1->b_datap->db_lim - mp1->b_rptr >= sizeof(int))) {
            int signal_temp = 0;
            int cntl_tmp = 0;
            srs_service(tp, TS_GCONTROL, &cntl_tmp);
            if (cntl_tmp & TSDTR) signal_temp |= TIOCM_DTR;
            if (cntl_tmp & TSRTS) signal_temp |= TIOCM_RTS;
            if (rsp->ry_msr&DCD) signal_temp |= TIOCM_CAR;
            if (rsp->ry_msr&RI) signal_temp |= TIOCM_RNG;
            if (rsp->ry_msr&DSR) signal_temp |= TIOCM_DSR;
            if (rsp->ry_msr&CTS) signal_temp |= TIOCM_CTS;
            *((int *)mp1->b_rptr) = signal_temp;
            mp1->b_wptr = mp1->b_rptr + sizeof(int);
        } else {
            freemsg(mp);
            return(0);
        }
        break;

      case TXTTYNAME:
        if (!(mp1 = mp->b_cont) ||
            (mp1->b_datap->db_lim - mp1->b_rptr < TTNAMEMAX)) {
            freemsg(mp);
            return(0);
        }
        bcopy(tp->t_name,(char *)mp1->b_rptr,sizeof(tp->t_name));
        mp1->b_wptr = mp1->b_rptr + sizeof(tp->t_name);
        break;


      default:
        freemsg(mp);
        return(0);
    }
    qreply(q,mp);
    return(0);
}

/*
 * srs_termios_set
 *      Sets the c_cflag, the IXON, IXOFF, IXANY bits
 *      of the c_iflag and VSTART and VSTOP charcters
 *      of the c_cc[] from the given tios.
 *      Calls srs_service to perform harware operations.
 *
 *
 * ARGUMENTS:
 *      tp      - str_rs struct pointer.
 *      tios    - pointer to termios structure to be set.
 *
 * RETURN VALUE: 0.
 *
 */

local int
srs_termios_set(str_rsp_t  tp, struct termios * tios)
{
    parity_t newparity;
    csize_t newcsize;
    tcflag_t t_iflag;
	
    if (((cfgetospeed(tios) == cfgetispeed(tios)) || 
         (cfgetospeed(tios) == 0) ||
         (cfgetispeed(tios) == 0 && (cfgetospeed(tios)))) &&
        tp->t_ospeed != compatspcodes[cfgetospeed(tios)]) {
        tp->t_ospeed = compatspcodes[cfgetospeed(tios)];
        srs_service(tp,TS_SBAUD,tp->t_ospeed);
    }

    switch (tios->c_cflag & CSIZE) {
	  case CS5:
		newcsize = bits5;
		break;
	  case CS6:
		newcsize = bits6;
		break;
	  case CS7:
		newcsize = bits7;
		break;
	  case CS8:
		newcsize = bits8;
		break;
	  default:
		break;
    }
    if (tp->t_csize != newcsize)
        srs_service(tp,  TS_SBPC, newcsize);

    if (tp->t_stopbits != ((tios->c_cflag & CSTOPB)? stop2: stop1)) {
        tp->t_stopbits = (tios->c_cflag & CSTOPB)? stop2: stop1;
        srs_service(tp,  TS_SSTOPS, tp->t_stopbits);
    }
	
    if(tios->c_cflag & PARENB) {
        if(tios->c_cflag & PAREXT) {
	    newparity = (tios->c_cflag&PARODD)?markpar:spacepar;
        } else {
	    newparity = (tios->c_cflag&PARODD)?oddpar:evenpar;
        }
    } else {
        newparity = (tios->c_cflag&PARODD)?oddonly:nopar; }

    if (tp->t_parity != newparity)
        srs_service(tp,  TS_SPARITY, newparity);
	
    /* Update the CLOCAL flag */
    if(tios->c_cflag & CLOCAL) {
		tp->t_clocal = 1; }
    else {
		tp->t_clocal = 0; }
	
    /* Update the CREAD flag */
    if(tios->c_cflag & CREAD) {
		tp->t_cread = 1; }
    else {
		tp->t_cread = 0; }
	
    /* Update the HUPCL flag */
    if(tios->c_cflag & HUPCL) {
		tp->t_hupcl = 1; }
    else {
		tp->t_hupcl = 0; }
	
    /* Set the xon/xoff remote/local flow control modes */

    /* Set the interesting special characters */

    tp->t_softchars[STARTCHAR] = tios->c_cc[VSTART];
    tp->t_softchars[STOPCHAR] = tios->c_cc[VSTOP];
    tp->t_softpacing = tios->c_iflag;

    return(0);
} /* End int srs_termios_set(...) */

/*
 * srs_termios_get
 *      Gets the c_cflag, the IXON, IXOFF, IXANY bits 
 *      of the c_iflag and VSTART and VSTOP characters
 *      of the c_cc[] from the local str_rs structure.
 *
 *
 * ARGUMENTS:
 *      tp      - str_rs struct pointer.
 *      tios    - pointer to termios structure to be returned.
 *
 * RETURN VALUE: 0.
 *
 */

int
srs_termios_get(str_rsp_t  tp, struct termios * tios)
{
    tcflag_t speed;
    int i;
    
    for (i = 0; (i <= 15 && compatspeeds[i].sp_speed != tp->t_ospeed); i++);
    if (i <= 15) { 
        speed =  compatspeeds[i].sp_code;
        cfsetospeed(tios, speed);
        cfsetispeed(tios, speed);
    }
	
    switch (tp->t_csize) {
	  case bits5:
		tios->c_cflag |= CS5;
		break;
	  case bits6:
		tios->c_cflag |= CS6;
		break;
	  case bits7:
		tios->c_cflag |= CS7;
		break;
	  case bits8:
		tios->c_cflag |= CS8;
		break;
	  default:
		break;
    }
    switch(tp->t_stopbits) {
	  case stop2:
		tios->c_cflag |= CSTOPB;
		break;
	  case stop1:
		tios->c_cflag &= ~CSTOPB;
		break;
	  default:
		break;
    }
    
    if (tp->t_parity == oddonly) {
        tios->c_cflag |= PARODD;
    } else {
        srs_service(tp, TS_GPARITY, &(tp->t_parity));
        switch (tp->t_parity) {
	      case nopar:
		    break;
	      case markpar:
		    tios->c_cflag |= PAREXT | PARENB | PARODD;
		    break;
	      case spacepar:
		    tios->c_cflag |= PAREXT | PARENB;
                    break;
	      case oddpar:
		    tios->c_cflag |= PARENB | PARODD;
		    break;
	      case evenpar:
		    tios->c_cflag |= PARENB;
                    break;
        }
    }
	
    if (tp->t_clocal) {
        tios->c_cflag |= CLOCAL;
    }
	
    if (tp->t_cread) {
        tios->c_cflag |= CREAD;
    }
	
    if (tp->t_hupcl) {
        tios->c_cflag |= HUPCL;
    }
    
    tios->c_iflag = tp->t_softpacing ;
    tios->c_cc[VSTART] = tp->t_softchars[STARTCHAR];
    tios->c_cc[VSTOP] = tp->t_softchars[STOPCHAR];
	
    return(0);
}

/*
 * srs_termiox_set
 *      Sets the x_hflag from the given tiox.
 *
 * ARGUMENTS:
 *      tp      - str_rs struct pointer.
 *      tiox    - pointer to termiox structure to be set.
 *
 * RETURN VALUE: 0.
 *
 */

local int
srs_termiox_set(str_rsp_t  tp, struct termiox * tiox)
{

/* Warning!! mutual exclusion of the CTSXON and CDXON remote flow control
   misses. idem for RTSXOFF and DTRXOFF */

    if (tiox->x_hflag & CTSXON) {
        int status;
        srs_service(tp, TS_GSTATUS, &status);
        if (!(status & TSCTS)) {
            tp->t_flow_state |= HARD_REMOTE_STOP;
            tp->t_stop = 1;
        }
        else {
            tp->t_flow_state &= ~HARD_REMOTE_STOP;
            tp->t_stop = 0;
        }
    }
	
    if (tiox->x_hflag & CDXON) {
        int status;
        srs_service(tp, TS_GSTATUS, &status);
        if (!(status & TSCD)) {
            tp->t_flow_state |= HARD_REMOTE_STOP;
            tp->t_stop = 1;
        }
        else {
            tp->t_flow_state &= ~HARD_REMOTE_STOP;
            tp->t_stop = 0;
        }
    }
    tp->t_hardpacing = tiox->x_hflag;
    return(0);
} /* End int srs_termiox_set(...) */

/*
 * srs_termiox_get
 *      Gets the x_hflag from the local str_rs structure.
 *
 * ARGUMENTS:
 *      tp      - str_rs struct pointer.
 *      tiox    - pointer to termiox structure to be returned.
 *
 * RETURN VALUE: 0.
 *
 */

int
srs_termiox_get(str_rsp_t  tp, struct termiox * tiox)
{
    tiox->x_hflag = tp->t_hardpacing ;
    tiox->x_sflag = tp->t_termiox.x_sflag;
    return(0);
}

/*
 * srs_break_set
 *      Sets break condition for the requested duration.
 *
 *
 * ARGUMENTS:
 *      tp       - str_rs struct pointer.
 *      duration - time for which the break condition is set.
 *
 * RETURN VALUE: 0.
 *
 */

int 
srs_break_set(str_rsp_t tp, int duration)
{
    if (duration) {
        if (tp->t_timeout) {
            untimeout(tp->t_timeout);
        }
        tp->t_timeout = timeout(srs_break_clear, (caddr_t)tp, duration);
	if (tp->t_timeout == NULL)
		return(-1);
    }
    srs_service(tp,TS_SBREAK,0);
    return(0);
}

/*
 * srs_break_clear
 *      Clears break condition.
 *
 *
 * ARGUMENTS:
 *      tp       - str_rs struct pointer.
 *
 * RETURN VALUE: 0.
 *
 */

int 
srs_break_clear(str_rsp_t tp)
{
    mblk_t *mp;     /* See note with TCSBRK processing */
    tp->t_timeout = 0;
    srs_service(tp,TS_CBREAK,0);
    if (mp = tp->t_ioctlmsg) {
        tp->t_ioctlmsg = 0;
        mp->b_datap->db_type = M_IOCACK;
        ((struct iocblk *)(mp->b_rptr))->ioc_count = 0;
        qreply(WR(tp->t_ctl), mp);
    }
    /* Some message may have been chained while break condition is set */
    if (WR(tp->t_ctl)->q_first) {
        qenable(WR(tp->t_ctl));
    }
    return(0);
}

/*
 * srsconfig
 *      This is the external entry point to the driver.
 *      It is called to configure, and unconfigure adappters and lines, and
 *      to query the Vital Product Data.
 *      We allocate one structure per new adapter name. When a line is
 *      configured, these structures are used to really configure the card.
 *      The configuration of a card is actually made only at the first
 *      configuration of one of its lines.
 *
 *
 * ARGUMENTS:
 *      dev     - Devno of the line to (un)configure.
 *      cmd     - CFG_INIT, CFG_TERM or CFG_QVPD.
 *      uiop    - pointer to uio stucture containing the DDS.
 *
 * RETURN VALUE:
 *     0 on succes, errno error code otherwise.
 *
 */

int 
srsconfig(dev_t dev, int cmd, struct uio *uiop)
{
    DoingPio;
/*    static int lock = SIMPLE_LOCK_AVAIL; */
    static int lock = LOCK_AVAIL;
    int ret;
    int result = 0, re2;
    int min, maj;
    int index;                              /* used for searching */
    dev_t newmin, chkmin;
    str_rsp_t tp;
    rymp_t rsp;
    enum dds_type      which_dds;           /* dds identifier for CFG_TERM */
    struct rs_adap_dds adap_dds;            /* used with uiomove */
    struct rs_line_dds line_dds;            /* used with uiomove */
    struct adap_conf * current_adap_conf;   /* scanning pointer */
    struct adap_conf * tmp_adap_conf;       /* pointer for new allocation*/
    struct adap_info * current_adap;        /* scanning pointer */
	
#ifdef TTYDBG
    int ttydbg_error = 0;
    struct	tty_to_reg srs_tty_to_reg = { dev, "", "rs", 0 };
/*
    rubbish
*/
#endif	/* TTYDBG */

    Enter(TRC_RS(TTY_CONFIG), 0, 0, cmd, 0, 0);
	
    srsprintf("srsconfig entered for %x\n", dev);
	
    ret = lockl(&lock, LOCK_SHORT);
    srsconf.sc_major = major(dev);
    switch (cmd) {
      case CFG_INIT:
        switch (uiop->uio_resid) {
            /* Adpater configuration */
          case sizeof(struct rs_adap_dds):
            if (result = uiomove(&adap_dds, sizeof(struct rs_adap_dds),
                                 UIO_WRITE, uiop)) {
                srs_err(sresource, ERRID_COM_CFG_UIO, result);
                srsprintf("uiomove failed with %d\n", result);
                break;
            };
            if (adap_dds.which_dds != RS_ADAP_DDS) {
                srs_err(sresource, ERRID_COM_CFG_RESID, uiop->uio_resid);
                srsprintf("DDS type not correct\n");
                result = EINVAL;
                break;
            };
            /* Check if adapter is known */
            current_adap = sadap_infos;
            while ((current_adap->ra_type != Unknown_Adap) &&
		   (current_adap->ra_type != adap_dds.rc_type)) {
                current_adap++;
            }
            /* If we find Unknown_Adap (the last choice of the sadap_infos
             * structure), it means that the given adapter is unknown
             */
            if (current_adap->ra_type == Unknown_Adap) {
                srs_err(adap_dds.rc_name, ERRID_COM_CFG_UNK, adap_dds.rc_type);
                srsprintf("unknown adapter\n");
                result = EINVAL;
                break;
            };
			
            /* Check if this adapter has already been configured */
            current_adap_conf = adap_conf_s;
            while (current_adap_conf &&
                   strcmp(current_adap_conf->dds.rc_name, adap_dds.rc_name)) {
                current_adap_conf = current_adap_conf->next;
            }
            if (current_adap_conf) {
                /* We must be sure that no line is currently configured */
                index = 0;
                while (index < srs_cnt) {
		    if (srs_tty[index] &&
			!strcmp(srs_tty[index]->te_name, adap_dds.rc_name)) {
			/*
			 * we have found an entry for this adapter. we will
			 * exit this loop with index < srs_cnt. that will
			 * tell the code below that this adapter is already
			 * configured.
			 */
			break;
		    }
		    /* look at next entry */
		    index++;
                }
                /* If no line is found, I can update existing adapter
                 * configuration informations. Otherwise, I return an error
                 */
                if (index >= srs_cnt) {
                    /* Clean before writing new values */
                    bzero((char *)&(current_adap_conf->dds),
                          sizeof(current_adap_conf->dds));
                    bcopy((char *)&adap_dds, (char *)&(current_adap_conf->dds),
                          sizeof(current_adap_conf->dds));
                }
                else {
                    result = EBUSY;
                    srs_err(adap_dds.rc_name, ERRID_COM_CFG_PORT, 0);
                    srsprintf("some ports are still configured\n");
                    break;
                } /* End if (index >= srs_cnt) */
            }
            else {
                if (tmp_adap_conf = (struct adap_conf *)
                                    swatchmalloc(sizeof(struct adap_conf))) {
                    /* Clears the struct */
                    bzero((char *)tmp_adap_conf, sizeof(struct adap_conf));
                    /* Updates the structure */
                    /* Only the dds part and not the next pointer */
                    bcopy((char *)&adap_dds, (char *)&(tmp_adap_conf->dds),
                              sizeof(tmp_adap_conf->dds));
                    /* Updates adapter configuration list */
                    current_adap_conf = adap_conf_s;
                    adap_conf_s = tmp_adap_conf;
                    tmp_adap_conf->next = current_adap_conf;
                }
                else {
                    srs_err(adap_dds.rc_name, ERRID_RS_MEM_EDGE, 0);
                    srsprintf("can't swatchmalloc adap_conf\n");
                    result = ENOMEM;
                    break;
                } /* End if (tmp_adap_conf = ...) */
            } /* End if (current_adap_conf) */
            break;
            
            /* Line configuration */
          case sizeof(struct rs_line_dds):
            if (result = uiomove(&line_dds, sizeof(struct rs_line_dds),
                                 UIO_WRITE, uiop)) {
                srs_err(sresource, ERRID_COM_CFG_UIO, result);
                srsprintf("uiomove failed with %d\n", result);
                break;
            };
            if (line_dds.which_dds != RS_LINE_DDS) {
                srs_err(sresource, ERRID_COM_CFG_RESID, uiop->uio_resid);
                srsprintf("DDS type not correct\n");
                result = EINVAL;
                break;
            };
            /* When initialization of the first line of an adapter */
            /* we make declaration to the Streams framework and */
            /* we pin the offlevel handler code */
            if (!srs_tty) {
                if (result = str_install(STR_LOAD_DEV,&srsconf)) {
                    srs_err(line_dds.rc_name, ERRID_COM_CFG_DEVA, result);
                    srsprintf("str_install failed with %d\n", result);
                    break;
                };

#ifdef TTYDBG
                /* registering the driver into the ttydbg extension */
                ttydbg_error = tty_db_register(&srs_str_module_conf);
#endif  /* TTYDBG */
            };
            /* To verify if adapter (parent) is configured */
            current_adap_conf = adap_conf_s;
            while (current_adap_conf && 
                   strcmp(current_adap_conf->dds.rc_name, line_dds.adap_name)) {
                current_adap_conf = current_adap_conf->next;
            }
            if (!current_adap_conf) {
                result = ENOTREADY;
                sprintf("%s parent adapter not found as configured\n",
                        line_dds.adap_name);
                break;
            };
            min = minor(dev);
            maj = major(dev);
            newmin = min - dev2port(min);
            chkmin = RSDEV(&(current_adap_conf->dds));
            if (newmin != chkmin) {
                result = EINVAL;
                srs_err(line_dds.rc_name, ERRID_COM_CFG_MNR, newmin);
                srsprintf("bad minor number\n");
                break;
            }
            
            /* We try to find the tty structure.  If we do, then the  */
            /* adapter has been configured.  Otherwise we need to call */
            /* srsadd to setup the adapter. */
            dev2tp(dev, tp);
            
            if (!tp) {
                /* We call srsadd with adapter and line DDS */
                if (result = srsadd(makedev(maj, newmin),
                                    &(current_adap_conf->dds),
                                    &(line_dds))) { /* bad status */
                    srsprintf("srsadd not happy\n");
                    break;
                };
                dev2tp(dev, tp);
            };
			
            /* Save the last name for the case where we get really messed */
            /* up and don't know our name. */
            
            /* If adapter has already been configured, t_openRetrieve must not
             * have value. This field is updated only on first open and
             * clear at close time.
             */
            if (!tp || !(rsp = tp->t_hptr)) {
                srs_err(line_dds.rc_name, ERRID_TTY_PROG_PTR, 0);
                result = ENXIO;                /* somewhat random return code */
            };
            
            if (rsp->ry_conf) {
                result = EBUSY;
                srs_err(tp->t_name, ERRID_COM_CFG_PORT, 0);
                srsprintf("%x port already configured\n", dev);
                break;
            };
            
            /* Added to handle NatieveC on Pegasus because it's on
               the Standard IO adapter and has a different xtal frequency */
            if (current_adap_conf->dds.rc_type == NativeC_SIO3) {
                rsp->ry_xtal = current_adap_conf->dds.rc_xtal;
                rsp->ry_type = NativeC_SIO3;
            }
			
            /*
             * get the new default settings. This operation is redundant if
             * it is the first line to be configured on the adapter.
             */
            bcopy(line_dds.rc_name, tp->t_name, sizeof(tp->t_name));

            bcopy(&(line_dds.ctl_modes), &(tp->t_termios),
                  sizeof(struct termios));

            bcopy(&(line_dds.disc_ctl), &(tp->t_termiox),
                  sizeof(struct termiox));
            if (tp->t_termiox.x_sflag & DTR_OPEN) {
                tp->t_open_disc = DTRO_OPEN;
            } else {
                if (tp->t_termiox.x_sflag & WT_OPEN) {
                    tp->t_open_disc = WTO_OPEN;
                }
            }
            rsp->ry_conf = 1;
#ifdef TTYDBG
            /* Open the ttydbg extension for the driver.  */
            bcopy(line_dds.rc_name, &(srs_tty_to_reg.ttyname),
                  sizeof(srs_tty_to_reg.ttyname));
            srs_tty_to_reg.private_data = tp;
            ttydbg_error = tty_db_open(&srs_tty_to_reg);
            /* What to do if erronous returned code ?? */
#endif	/* TTYDBG */


			
            break;
            
          default:
            srs_err(sresource, ERRID_COM_CFG_RESID, uiop->uio_resid);
            srsprintf("resid not correct\n");
            result = EINVAL;
        } /* End switch (uiop->uio_resid) */
        
        break;
		
	  case CFG_TERM:
        switch (uiop->uio_resid) {
            /* Adpater unconfiguration */
          case sizeof(struct rs_adap_dds):
            if (result = uiomove(&adap_dds, sizeof(struct rs_adap_dds),
                                 UIO_WRITE, uiop)) {
                srs_err(sresource, ERRID_COM_CFG_UIO, result);
                srsprintf("uiomove failed with %d\n", result);
                break;
            };
            if (adap_dds.which_dds != RS_ADAP_DDS) {
                srs_err(sresource, ERRID_COM_CFG_RESID, uiop->uio_resid);
                srsprintf("DDS type not correct\n");
                result = EINVAL;
                break;
            };
            /* Check if this adapter has already been configured */
            tmp_adap_conf = adap_conf_s;
            current_adap_conf = adap_conf_s;
            while (current_adap_conf &&
                   strcmp(current_adap_conf->dds.rc_name, adap_dds.rc_name)) {
                tmp_adap_conf = current_adap_conf;
                current_adap_conf = current_adap_conf->next;
            }
            if (current_adap_conf) {
                /* We must be sure that no line is currently configured */
                /* It is the case if no stty_edge structure */
                /* exists for this adapter name */
                index = 0;
                while (index < srs_cnt) {
		    if (srs_tty[index] &&
			!strcmp(srs_tty[index]->te_name, adap_dds.rc_name)) {
			/*
			 * we have found an entry for this adapter. we will
			 * exit this loop with index < srs_cnt. that will
			 * tell the code below that this adapter still has
			 * configured devices attached to it.
			 */
			break;
		    }
		    /* look at next entry */
                    index++;
                }
                /* If no line is found, I can free the structure in
                 * adap_conf_s list Otherwise, I return an error
                 */
                if (index >= srs_cnt) {
                    if (current_adap_conf == adap_conf_s) {
                        adap_conf_s = current_adap_conf->next;
                    }
                    else {
                        tmp_adap_conf->next = current_adap_conf->next;
                    }
                    assert(!swatchfree(current_adap_conf));
                }
                else {
                    result = EBUSY;
                    srs_err(adap_dds.rc_name, ERRID_COM_CFG_PORT, 0);
                    srsprintf("some ports are still configured\n");
                } /* End if (index >= srs_cnt) */
            }
            else {
                srsprintf("%s adapter was not configured\n", adap_dds.rc_name);
                result = EINVAL;
            } /* End if (current_adap_conf) */
			
            break;
			
          case sizeof(which_dds):
            if (result = uiomove(&which_dds, sizeof(which_dds),
                                 UIO_WRITE, uiop)) {
                srs_err(sresource, ERRID_COM_CFG_UIO, result);
                srsprintf("uiomove failed with %d\n", result);
                break;
            };
            if (which_dds != RS_LINE_DDS) {
                srs_err(sresource, ERRID_COM_CFG_RESID, uiop->uio_resid);
                srsprintf("DDS type not correct\n");
                result = EINVAL;
                break;
            };
#ifdef TTYDBG
            /* Close the ttydbg extension for the driver.
               Do it here, before deletion of structures. */
            dev2tp(dev, tp);
            srs_tty_to_reg.private_data = tp;
            bcopy(tp->t_name, &(srs_tty_to_reg.ttyname),
                  sizeof(srs_tty_to_reg.ttyname));
            ttydbg_error = tty_db_close(&srs_tty_to_reg);
#endif	/* TTYDBG */
			
            if (result = srsdel(dev)) {
                break;
            };

            if (!srs_tty) {                  /* cleanup after last delete */
                if (re2 = str_install(STR_UNLOAD_DEV,&srsconf)) {
                    srs_err(sresource, ERRID_COM_CFG_DEVD, result);
                    srsprintf("str_install unload failed with %d\n", result);
                    result = re2;
                };
#ifdef TTYDBG
                /* unregistering the module into the ttydbg extension */
                ttydbg_error = tty_db_unregister(&srs_str_module_conf);
#endif	/* TTYDBG */
            };
			
            break;
			
          default:
            srs_err(sresource, ERRID_COM_CFG_RESID, uiop->uio_resid);
            srsprintf("resid not correct\n");
            result = EINVAL;
        } /* End switch (uiop->uio_resid) */
		
        break;
		
#ifdef _POWER
	  case CFG_QVPD: {
		  char *pos;
		  volatile int i;
		  
		  dev2tp(dev, tp);
		  if (!tp) {
			  result = ENODEV;
			  break;
		  }
		  rsp = tp->t_hptr;
		  if (!(rsp->ry_xbox)) {                /* not in X box */
			  pos = io_att(rsp->ry_iseg, rsp->ry_ibase);
			  i = 0;
			  StartPio(srs_pio_catch, rsp,
					   if (ret == LOCK_SUCC) unlockl(&lock);
					   io_det(pos);
					   Return(EIO));
			  while (i < 8 &&
                                !(result = ureadc(BUSIO_GETC(pos + i), uiop))) {
				  ++i;
			  }
			  EndPio();
			  
			  i = 1;
			  StartPio(srs_pio_catch, rsp,
					   if (ret == LOCK_SUCC) unlockl(&lock);
					   io_det(pos);
					   Return(EIO));
			  while (i < (1<<16) && uiop->uio_resid && !result) {
				  BUSIO_PUTC((pos + 6), (i & 0xff));
				  BUSIO_PUTC((pos + 7), ((i >> 8) & 0xff));
				  result = ureadc(BUSIO_GETC(pos + 3), uiop);
				  ++i;
			  }
			  EndPio();
			  
			  /* restore state */
			  StartPio(srs_pio_catch, rsp,
					   if (ret == LOCK_SUCC) unlockl(&lock);
					   io_det(pos);
					   Return(EIO));
			  BUSIO_PUTC((pos + 6), 0);
			  BUSIO_PUTC((pos + 7), 0);
			  EndPio();
			  io_det(pos);
		  } else {
			  stty_edge *te;
			  
			  te = srs_tty[dev2slot(minor(dev))];
			  result = srs_getvpd(rsp, uiop, te->te_parent);
		  }
		  break;
	  } /* End case CFG_QVPD */
#endif
        
      default:
        result = EINVAL;
        break;
    } /* End switch (cmd) */
    
    if (ret == LOCK_SUCC) {
        unlockl(&lock);
    };
	
    srsprintf("srsconfig returned %d\n", result);
    Return(result);
}

/*
 * This function is sorta big and ugly but it is only called once for
 * each adapter configured into the system.
 */

/*
 * Things malloc'd: 1) The edge vector to go from minor number to the
 * edge structure.  This is grown to the largest slot we have an
 * adapter in.  If it wasn't for the watchdog timer, this would not be
 * pinned.  1.5) The edge structure is a list of pointers to tty
 * structures which is indexed by port number thus giving a two
 * dimensional effect index by slot and port.  2) iocc struct is
 * pinned.  It is released only after all all adapters on the iocc
 * have been un-configured.  3) slih is also pinned and has a use
 * count which is decremented when an adapter that uses it is
 * unconfigured.  When the use count goes to zero, it is released.  4)
 * Big ym/tty array is pinned at open time and is released when
 * the adapter it relates to is unconfigured.
 */

int 
srsadd(dev_t dev, struct rs_adap_dds * adapConf,
		 struct rs_line_dds * lineConf)
{
#define SRS_ADD
#ifdef SRS_ADD
    DoingPio;
    register struct adap_info *adap;    /* adapter type */
    register struct intr *intr;         /* temp for speed */
    register struct rs_intr *slih;      /* rs_intr to use */
    register int temp;                  /* index counter */
    register rymp_t rsp;                /* temp pointers */
    register str_rsp_t tp;              /* tty pointer */
    register struct rs_iocc *iocc;      /* iocc to use */
    register rymp_t *iocc_rsp;          /* pointer ... */
    register int min;                   /* minor device number */
    register int maj;                   /* major device number */
    register int iocc_num;              /* which iocc */
    register int err, err2;
    register caddr_t id_ptr;
    int new_iocc;            /* iocc was created */
    int inxbox = adapConf->rc_arb == 0x0134;
    stty_edge *te;
	
    min = minor(dev);
    maj = major(dev);
	
	/* find adap info for this type */
    for (adap = sadap_infos;
    	 adap->ra_type != Unknown_Adap && adap->ra_type != adapConf->rc_type;
    	 ++adap);
	/* The last one is unkown adap type in the search list above */
    if (adap->ra_type == Unknown_Adap) {
    	srs_err(adapConf->rc_name, ERRID_COM_CFG_UNK, adapConf->rc_type);
    	srsprintf("unknown adapter\n");
    	return(EINVAL);
    }
	
    /* Now we have got a proper adap info; let's print it */
    srsprintf("adapter name: %s ", adap->ra_name);
    intr = adap->ra_intr;
	
    /*
     * Check any and all reasons this could fail because of bad config
     * parameters before we start malloc'ing stuff.  The init could
     * still fail due to system problems.
     */
	
    /* Also note bus_type,flags,level are set in intr struct declared */
    /* Only priority,bid are set for CONFIG_TIME */
    /* For the declared once match with dds struct passed must be okay */
    /* For eg. */
    /* for native;bus_type is not CONFIG_TIME but should match with DDS info*/
	
    /* check slih parameters -- sorta ugly */
    if (intr->bus_type != CONFIG_TIME && intr->bus_type != adapConf->rc_bus) {
    	srs_err(adapConf->rc_name, ERRID_COM_CFG_BUST, adapConf->rc_bus);
    	srsprintf("bad bus type\n");
    	return(EINVAL);
    }
/* This code will reappear as soon as it safety considerations are clearer */
/*    if (intr->flags != CONFIG_TIME && intr->flags != adapConf->rc_flags) {
    if (intr->flags != CONFIG_TIME) {
    	srs_err(adapConf->rc_name, ERRID_COM_CFG_IFLG, adapConf->rc_flags);
    	srsprintf("bad flags\n");
    	return(EINVAL);
    } */
    if (intr->level != CONFIG_TIME && intr->level != adapConf->rc_level) {
    	srs_err(adapConf->rc_name, ERRID_COM_CFG_ILVL, adapConf->rc_level);
    	srsprintf("bad interrupt level\n");
    	return(EINVAL);
    }
    if (intr->priority != CONFIG_TIME &&
        intr->priority != adapConf->rc_priority) {
    	srs_err(adapConf->rc_name, ERRID_COM_CFG_INTR, adapConf->rc_priority);
    	srsprintf("bad interrupt priority\n");
    	return(EINVAL);
    }
    if (intr->bid != CONFIG_TIME && intr->bid != adapConf->rc_nseg) {
    	srs_err(adapConf->rc_name, ERRID_COM_CFG_BUSI, adapConf->rc_nseg);
    	srsprintf("bad bus ID\n");
    	return(EINVAL);
    }
	
    /*
     * See if this adapter has already been configured.  If this is
     * the first mention of the iocc, we know that it has not been
     * configured.  We don't allocate the iocc yet.
     */
    iocc_num = SEG2IOCC(adapConf->rc_nseg);
    if (iocc_num < 0 || iocc_num >= IOCC_CNT) {
    	srs_err(adapConf->rc_name, ERRID_COM_CFG_BUSID, iocc_num);
    	srsprintf("bus id out of range\n");
    	return(EINVAL);
    }
	
    /* if unknown iocc (or new one )then make one */
    new_iocc = 0;
    if (!(iocc = srs_ioccs[iocc_num])) {
    	new_iocc = 1;
    	if (!(iocc = (struct rs_iocc *)pinmalloc(sizeof(*iocc)))) {
           	srs_err(adapConf->rc_name, ERRID_RS_MEM_IOCC, 0);
    		srsprintf("can't pinmalloc iocc\n");
    		return(ENOMEM);
    	}
    	bzero(iocc, sizeof(*iocc));
    	srs_ioccs[iocc_num] = iocc;
    }
	
    /*
     * Finally go out and poke the card and make sure its there. We
     * want to do this last or the poking around might mess up a
     * previously configured card up.  We also setup adapter specific
     * data here.
     */
    err = 0;
#ifdef _POWER
    if (inxbox) {
    	if (err = srs_xboxopen(adapConf->rc_parent))
    	  return(err);
    } else
#endif                    /* _POWER */
      id_ptr = io_att(adapConf->rc_iseg, adapConf->rc_ibase);
	
    StartPio(srs_pio_catch, 0, err = EIO; break);
    switch (adap->ra_type) {
#ifdef _POWER
	  case NativeA_SIO:
	  case NativeB_SIO: {
              int pos0, pos1, ptype = adap->ra_type;
	      iocc_rsp = iocc->iocc_native;
	  	
          /*
           * Reconfigure the same board or its not really there */
	      if (iocc_rsp[adapConf->rc_anum]) {
	          err = EEXIST;
	          break;
              }
              pos0 = BUSIO_GETC(id_ptr);
              pos1 = BUSIO_GETC(id_ptr+1);
              for (adap = sadap_infos;
                   adap->ra_type != Unknown_Adap &&
                   (IsNativeA(ptype) != IsNativeA(adap->ra_type) ||
                   pos0 != adap->ra_id[0] || pos1 != adap->ra_id[1]);
                   ++adap);
              if (adap->ra_type == Unknown_Adap) {
                  srs_err(adapConf->rc_name, ERRID_COM_CFG_UNK,
                          adapConf->rc_type);
                  srsprintf("unknown adapter\n");
                  err = EINVAL;
                  break;
              }
              /* Needed for pegasus because it has a different SLIH */
              intr->handler = adap->ra_intr->handler; 
              intr->flags = adap->ra_intr->flags; 
              break;
          }
	  case Eight_232:
	  case Eight_422:
	  case Eight_Mil:
	  case Sixteen_232:
	  case Sixteen_422:
		iocc_rsp = iocc->iocc_eight;
		
		/* Reconfigure the same board or its not really there */
		if (adapConf->rc_level < 0 || adapConf->rc_level > 15 ||
			srs_intr8[adapConf->rc_level] == -1 ||
                        adapConf->rc_anum > 8)
		  err = EINVAL;
		else if (iocc_rsp[adapConf->rc_anum])
		  err = EEXIST;
		else if (inxbox) {
			uchar pos0, pos1;
			
			if (!(err = srs_getpos(adapConf->rc_slot, 0, &pos0)) &&
		            !(err = srs_getpos(adapConf->rc_slot, 1, &pos1)) &&
			    (pos0 != adap->ra_id[0] || pos1 != adap->ra_id[1])) {
			    srsprintf("bad pos\n");
			    err = ENODEV;
			} else 
			  err = fp_ioctl(srs_xbox, EU_ADD8, adapConf->rc_anum); 
		} else if (BUSIO_GETC(id_ptr) != adap->ra_id[0] ||
			       BUSIO_GETC(id_ptr+1) != adap->ra_id[1])
		  /* poked the board, me thinks!!! */
		  err = ENODEV;
		break;
#endif                    /* _POWER */
		
	  default:
		err = EINVAL;
    }
    EndPio();
	
#ifdef _POWER
    if (inxbox) {
		int nerr = srs_xboxclose();
		if (!err)
		  err = nerr;
    } else
#endif                    /* _POWER */
	  io_det(id_ptr);
	
    if (err) {
		/* Error so undo some actions done already above */
		if (new_iocc) {
			assert(!pinfree(iocc));
			srs_ioccs[iocc_num] = 0;
		}
		if (err == ENODEV) {
			srs_err(adapConf->rc_name, ERRID_COM_CFG_NADP, err);
			srsprintf("adapter not present\n");
		} else {
			srs_err(adapConf->rc_name, ERRID_COM_CFG_ADPT, err);
			srsprintf("adapter already configured\n");
		}
		return(err);
    }
	
    /*
     * end of param checking... put anything else you can think of
     * before here if it can cause the init to fail due to a
     * configuration parameter.
     *
     * 1) Check for enough srs_tty positions and also check to make
     * sure they are nil.
     */
    if (dev2slot(min) >= srs_cnt) {
		stty_edge **new;
		/* more is to be added now */
		temp = (dev2slot(min) + 1) * sizeof(stty_edge *);
		/* get sufficient memory to have in entries srs_tty for the one
		   we are going to add also. ie. existing + new one
		 */
		if (!(new = swatchmalloc(temp))) {
			srs_err(adapConf->rc_name, ERRID_RS_MEM_EDGEV, 0);
			srsprintf("can't swatchmalloc srs_tty\n");
			return(ENOMEM);
		}
		/* Clear the struct */
		bzero(new, temp);
		if (srs_tty) {
			/* copy the existing info onto new one */
			bcopy(srs_tty, new, srs_cnt * sizeof(stty_edge *));
			/* release the old one */
			assert(!swatchfree(srs_tty));
		}
		/* init srs_tty for the newly expanded struct */
		srs_tty = new;
		srs_cnt = dev2slot(min) + 1;
    }
	
    /* 1.5 allocate edge structure */
    temp = sizeof(stty_edge) + ((adap->ra_num - 1) * sizeof(str_rsp_t));
    if (!(te = srs_tty[dev2slot(min)] = swatchmalloc(temp))) {
		srs_err(adapConf->rc_name, ERRID_RS_MEM_EDGE, 0);
		srsprintf("can't swatchmalloc edge structure\n");
		return(ENOMEM);
    }
    /* Clear and intialize the edge values */
    bzero(te, temp);
    strncpy(te->te_name, adapConf->rc_name, DEV_NAME_LN);
    te->te_name[DEV_NAME_LN] = '\0';
    te->te_num = adap->ra_num; /* no of ports on adapter */
    te->te_priority = adapConf->rc_priority;
    if (smin_priority > adapConf->rc_priority)
      smin_priority = adapConf->rc_priority;
    te->te_bus = adapConf->rc_bus; /* bus type micro channel or pcbus, etc */
    te->te_anum = adapConf->rc_anum; /* Adapter no. */
    te->te_parent = adapConf->rc_parent;
    te->te_arb = adapConf->rc_arb;
	
    /* look for identical slih */
    /* remember, that intr points to intr struct obtained from adap_info */
    for (slih = srs_intr_chain;
    	 slih && !(slih->ri_intr.handler == intr->handler &&
            	   slih->ri_intr.bus_type == adapConf->rc_bus &&
                   slih->ri_intr.level == adapConf->rc_level &&
                   slih->ri_intr.priority == adapConf->rc_priority &&
                   slih->ri_intr.bid == adapConf->rc_nseg &&
                   slih->ri_rsp == iocc_rsp &&
                   slih->ri_arb == adapConf->rc_arb);
         slih = slih->ri_next);
	
    if (!slih) {
		/* didn't find a slih? No, Problem, I make one now */
		srsprintf("new slih ");
		if (!(slih = (struct rs_intr *)pinmalloc(sizeof(*slih)))) {
			srs_err(adapConf->rc_name, ERRID_COM_MEM_SLIH, 0);
			srsprintf("can't pinmalloc slih\n");
			return(ENOMEM);
		}
		bzero(slih, sizeof(*slih));
		/* Let us update the fields from intr pointer and DDS */
		slih->ri_intr.next = 0;        /* fill in the fields */
		slih->ri_intr.handler = intr->handler;
		slih->ri_intr.flags = intr->flags;
		slih->ri_intr.bus_type = adapConf->rc_bus;
		slih->ri_intr.level = adapConf->rc_level;
		slih->ri_intr.priority = adapConf->rc_priority;
		slih->ri_intr.bid = adapConf->rc_nseg;
		slih->ri_arb = adapConf->rc_arb;
		if (i_init(slih) != INTR_SUCC) {
			assert(!pinfree(slih));
			srs_err(adapConf->rc_name, ERRID_COM_CFG_SLIH, 0);
			srsprintf("i_init of slih failed\n");
			return(EIO);
		}
		slih->ri_next = srs_intr_chain;    /* hook'm up */
		srs_intr_chain = slih;
		slih->ri_use = 0;
		slih->ri_rsp = iocc_rsp;
    }
    ++slih->ri_use;
	
    /*
     * we need to allocate ym structs for each port, hook them up
     * to the tty structs (and back), as well as fill them in.  We
     * also need to make the vector for the iocc point to the ym
     * structs so the slih can find them.
     */
    temp = (sizeof(struct rs_ym) + sizeof(struct str_rs)) * adap->ra_num;
    if (!(rsp = (rymp_t )malloc(temp))) {
		srs_err(adapConf->rc_name, ERRID_RS_MEM_PVT, 0);
		srsprintf("can't malloc ym struct\n");
		return(ENOMEM);
    }
    bzero(rsp, temp);            /* zap to zero's */
    tp = (str_rsp_t)((char *)rsp +        /* tty structs are above ym's */
					 (sizeof(struct rs_ym) * adap->ra_num));
	
    /*
     * Fill stuff in -- if we have an error we keep going and let
     * srsconfig call srsdel to delete the junk we just created
     */
    err = 0;
    /* lets start from beginning till no of ports present for adapter type */
    for (temp = 0; temp < adap->ra_num; ++temp, ++tp, ++rsp, ++min) {
    	te->te_edge[temp] = tp;        /* fill in edge vector */
    	tp->t_hptr = (caddr_t)rsp;    /* fill in tty struct */
#ifdef SRS_TIMEX
    	rsp->ry_timer = talloc();
    	rsp->ry_timer->func_data = rsp;
    	rsp->ry_timer->func = srs_xmit;
    	rsp->ry_timer->ipri = adapConf->rc_priority;
#endif    
    	tp->t_dev = makedev(maj, min);
        tp->t_priority = adapConf->rc_priority;
      
        /*
         * Get the  termios structure brought by the dds.
         * only c_cflag, IXON, IXOFF, IXANY bits of c_iflag and c_cc[VSTART]
         * and c_cc[VSTOP] are driver's business. All other flags are kept
         * unchanged.
         */
         bcopy(&(lineConf->ctl_modes),
               &(tp->t_termios), sizeof(struct termios));

        /* This chip does not support different input and output speed */
        if (cfgetospeed(&(lineConf->ctl_modes)) !=
            cfgetispeed(&(lineConf->ctl_modes))) {
    		cfsetospeed(&tp->t_termios, B9600);
    		cfsetispeed(&tp->t_termios, B9600);
        }
      
        /*
         * Get the  termiox structure brought by the dds.
         */
         bcopy(&(lineConf->disc_ctl),
               &(tp->t_termiox), sizeof(struct termiox));
    	
    	/* Make up off level structure */
    	INIT_TTY_OFFL(&rsp->ry_intr, srs_offlevel, adapConf->rc_nseg);
		
	lock_alloc(&tp->t_lock_soft, LOCK_ALLOC_PIN, RS_T_LOCK_SOFT,
							(short int) min);
	lock_alloc(&tp->t_lock_hard, LOCK_ALLOC_PIN, RS_T_LOCK_HARD,
							(short int) min);
        simple_lock_init(&(tp->t_lock_soft));
        simple_lock_init(&(tp->t_lock_hard));
        tp->t_event = EVENT_NULL;
	tp->t_rptr = tp->t_wptr = &tp->t_rbuf[0];	

    	/* Fill in ym struct */
    	rsp->ry_level = adapConf->rc_level;
    	rsp->ry_tp = tp;
    	bcopy(lineConf->rc_name, tp->t_name, sizeof(tp->t_name));
    	rsp->ry_slot = adapConf->rc_slot;
    	rsp->ry_port = adapConf->rc_base + adap->ra_offsets[temp];
    	rsp->ry_xtal = adapConf->rc_xtal;    /* save xtal */
    	rsp->ry_rtrig = (lineConf->rtrig << 6);
    	rsp->ry_tbc = lineConf->tbc;
    	rsp->ry_ibase = adapConf->rc_ibase;
    	rsp->ry_iseg = adapConf->rc_iseg;
		
    	/* Adapter Specific initialization per port */
    	switch (rsp->ry_type = adap->ra_type) {
#ifdef _POWER
        case NativeA_SIO:
        case NativeA_SIO1:
        case NativeA_SIO2:
            iocc_rsp[temp] = rsp;
            if (!temp) { /* port A */
                rsp->ry_dma = adapConf->rc_dma & 0xf;
                /* given: rsp->ry_type = NativeA_??? */
            } else {                    /* port B */
                rsp->ry_dma = (adapConf->rc_dma >> 4) & 0xf;
                rsp->ry_type += 1;
            }
            rsp->ry_xbox = 0;
            break;

        case NativeB_SIO:
        case NativeB_SIO1:
        case NativeB_SIO2:
            iocc_rsp[temp] = rsp;
            if (!temp) { /* port A */
                rsp->ry_dma = adapConf->rc_dma & 0xf;
                rsp->ry_type -= 1;
            } else {                    /* port B */
                rsp->ry_dma = (adapConf->rc_dma >> 4) & 0xf;
                /* given: rsp->ry_type = NativeB_??? */
            }
            rsp->ry_xbox = 0;
            break;

			/* No DMA for Pegasus*/
        case NativeA_SIO3:
        case NativeB_SIO3:
	case NativeC_SIO3:
    	    iocc_rsp[temp] = rsp;
	    rsp->ry_xbox = 0;
   	    break;
			
        case Eight_232:
        case Eight_422:
        case Eight_Mil:
        case Sixteen_232:
        case Sixteen_422:
	    /* 8 is used to shift bits in the adapter arbitration reg */
            iocc_rsp[(temp*8) + adapConf->rc_anum] = rsp;
            rsp->ry_dma = 0;
            rsp->ry_xbox = inxbox;
            /* Default mil board to regular polarity */
            if (rsp->ry_type == Eight_Mil)
                rsp->ry_mil = 1;
            break;
#endif                    /* _POWER */
			
	}
    }
	
    /* Finally we need to setup a few pos registers */
#ifdef _POWER
    if (inxbox) {
		if (err = srs_xboxopen(te->te_parent))
		  return(err);
    } else
#endif                    /* _POWER */
	  id_ptr = io_att(adapConf->rc_iseg, adapConf->rc_ibase);
	
    /* Adapter Specific initialization per adapter */
	
    StartPio(srs_pio_catch, rsp, err = EIO; break);
    switch (adap->ra_type) {
#ifdef _POWER
	  case NativeA_SIO:
	  case NativeB_SIO:
	  case NativeA_SIO1:
	  case NativeB_SIO1:
	  case NativeA_SIO2:
	  case NativeB_SIO2:
	  case NativeA_SIO3:
	  case NativeB_SIO3:
	  case NativeC_SIO3:
		/* setup the arbitration */
		BUSIO_PUTC(id_ptr + 3, adapConf->rc_dma);
        if (!(NativeA_SIO2 == adap->ra_type || NativeB_SIO2 == adap->ra_type))
           /* SIO && SIO1 hardware */
            BUSIO_PUTC(id_ptr + 4, (BUSIO_GETC(id_ptr + 4)|0x30));
        else
        /* SIO2 hardware, See ::Note 6:: */
            BUSIO_PUTC(id_ptr + 5, (BUSIO_GETC(id_ptr + 5)|0x03));
        break;
		
	  case Eight_232:
	  case Eight_422:
	  case Eight_Mil:
	  case Sixteen_232:
	  case Sixteen_422: {
		uchar pos = (srs_intr8[adapConf->rc_level]<<4) |
                            (adapConf->rc_anum<<1)|0x81;
		/* setup intr request and adapter number -- enable interrupts */
		if (inxbox) {
		    if ((err = srs_setpos(adapConf->rc_slot, 2, pos)) ||
                        (err = srs_setpos(adapConf->rc_slot, 5, 0x08)))
	    	        fp_ioctl(srs_xbox, EU_DEL8, adapConf->rc_anum);
		} else {
		    BUSIO_PUTC(id_ptr + 2, pos);
		    BUSIO_PUTC(id_ptr + 5, 0x08); /* turn on parity on bus */
		}
    	  }
	  break;
#endif                    /* _POWER */
		
    }
    EndPio();
	
#ifdef _POWER
    if (inxbox) {
		int nerr = srs_xboxclose();
		if (!err)
		  err = nerr;
    } else
#endif                    /* _POWER */
	  io_det(id_ptr);
	
    srsprintf("srsadd return %d\n", err);
    return(err);
#endif                    /* SRS_ADD */
}

int 
srsdel(dev_t dev)
{
#define SRS_DEL
#ifdef SRS_DEL
    DoingPio;
    register struct adap_info *adap;    /* adapter type */
    register int min;                   /* minor device number */
    int newdev;                         /* new device number */
    int last_min;                       /* last minor number */
    register rymp_t *iocc_rsp;
    register struct intr *intr;         /* temp for speed */
    register struct rs_intr **slih;     /* rs_intr to use */
    register rymp_t rsp;                /* temp pointers */
    register struct rs_iocc *iocc;      /* iocc to use */
    register str_rsp_t tp;              /* pointer to tty structure */
    uchar id0, id1;                     /* pos register 0, 1, and 4 */
    int index;                          /* index for srs_tty */
    int rv_flag;                        /* flag for srs_tty */
    int iocc_num;
    stty_edge *te;
    str_rsp_t *start, *end;
    int result;
    register str_rsp_t tp1;		/* pointer to tty structure */
    register int temp;			/* index counter */
	
    min = minor(dev);
	
    /* If this guy is not configured, then tell'm off */
    dev2tp(dev, tp);
    if (!tp || !(rsp = tp->t_hptr) || !rsp->ry_conf)
	  return(ENODEV);
    if (rsp->ry_open || tp->t_wopen)
	  return(EBUSY);
	
    for (adap = sadap_infos;
		 adap->ra_type != Unknown_Adap && adap->ra_type != rsp->ry_type;
		 ++adap);
    if (adap->ra_type == Unknown_Adap)
	  return(EINVAL);
    intr = adap->ra_intr;
	
    rsp->ry_conf = 0;
	
	/* May be need to verify tty busy? if so retain ry_conf=1 and return */
	
    te = srs_tty[dev2slot(min)];
	
    /*
     * See if any port still configured on this adapter.  When we are
     * done, rsp points to the first guy which is the base of the big
     * malloc we did in srsadd
     */
    start = te->te_edge;
    end = start + te->te_num;
    while (start < end)
	  if ((tp = *--end) && (rsp = tp->t_hptr) && rsp->ry_conf)
        return(0);
	
    srsprintf("unconfig adapter name: %s ", adap->ra_name);
	
#ifdef _POWER
    /* reset intr request and adapter number -- disable interrupts */
    if (rsp->ry_xbox) {            /* if in xbox */
    	       if ((result = srs_xboxopen(te->te_parent)))
		  return(result);
		srs_setpos(rsp->ry_slot, 2, 0);
		srs_setpos(rsp->ry_slot, 5, 0);
		fp_ioctl(srs_xbox, EU_DEL8, te->te_anum);
		if ((result = srs_xboxclose()))
		  return(result);
    } else if (!IsNative(adap->ra_type)) {
		/* disable all but the NIO adapters */
		char *pos = io_att(rsp->ry_iseg, rsp->ry_ibase);
		
		StartPio(srs_pio_catch, rsp, io_det(pos); return(EIO));
		BUSIO_PUTC(pos + 2, 0);
		BUSIO_PUTC(pos + 5, 0); 
		EndPio();
    }
#endif                /* _POWER */
	
    iocc_num = SEG2IOCC(rsp->ry_intr.bid);
    if (iocc_num < 0 || iocc_num >= IOCC_CNT) {
		srs_err(tp->t_name, ERRID_COM_CFG_BUSID, iocc_num);
		srsprintf("bus id out of range\n");
		return(EINVAL);
    }
	
    /* If this iocc is not configured something is really wrong */
    if (!(iocc = srs_ioccs[iocc_num])) {
		srs_err(tp->t_name, ERRID_RS_PROG_IOCC, 0);
		srsprintf("iocc not configured\n");
		return(EINVAL);
    }
	
    /* Select the iocc */
    switch (adap->ra_type) {
#ifdef _POWER
          case NativeA_SIO:
          case NativeB_SIO:
          case NativeA_SIO1:
          case NativeB_SIO1:
          case NativeA_SIO2:
          case NativeB_SIO2:
          case NativeA_SIO3:
          case NativeB_SIO3:
	  case NativeC_SIO3:
		iocc_rsp = iocc->iocc_native;
		break;
		
	  case Eight_232:
	  case Eight_422:
	  case Eight_Mil:
	  case Sixteen_232:
	  case Sixteen_422:
		iocc_rsp = iocc->iocc_eight;
#endif                    /* _POWER */
		
    }
	
    /* All the checks are done so lets start zapping things */
	
    /* (3) look for slih and free it if unused */
    for (slih = &srs_intr_chain;
	 *slih && !((*slih)->ri_intr.handler == intr->handler &&
		    (*slih)->ri_intr.bus_type == te->te_bus &&
                    (*slih)->ri_intr.level == rsp->ry_level &&
                    (*slih)->ri_intr.priority == te->te_priority &&
                    (*slih)->ri_intr.bid == rsp->ry_intr.bid &&
                    (*slih)->ri_rsp == iocc_rsp &&
                    (*slih)->ri_arb == te->te_arb);
	 slih = &(*slih)->ri_next);
	
    if (!(*slih)) {            /* didn't find a slih */
		srs_err(tp->t_name, ERRID_RS_PROG_SLIH, 0);
		srsprintf("Can't find slih!!!\n");
    } else if (--(*slih)->ri_use == 0) {
		register struct rs_intr *t = *slih;
		
		*slih = (*slih)->ri_next;    /* unlink from chain */
		srsprintf("discard unused slih ");
		i_clear(t);            /* remove from system */
		assert(!pinfree(t));
    }
	
    /* (4) free up big malloc of ym and tty structs */
#ifdef SRS_TIMEX
    tstop(rsp->ry_timer);
    tfree(rsp->ry_timer);
#endif    
   tp1 = (str_rsp_t)((char *)rsp +	/* tty structs are above ym's */
					(sizeof(struct rs_ym) * adap->ra_num));
    for (temp = 0; temp < adap->ra_num; ++temp, ++tp1) {
        lock_free(&tp1->t_lock_soft);
        lock_free(&tp1->t_lock_hard);
    }
    assert(!free(rsp));
	
    /* (2) Clean up and delete iocc struct */
    for (index = 0; index < adap->ra_num; ++index)
	  switch (adap->ra_type) {
#ifdef _POWER
                case NativeA_SIO:
                case NativeB_SIO:
                case NativeA_SIO1:
                case NativeB_SIO1:
                case NativeA_SIO2:
                case NativeB_SIO2:
                case NativeA_SIO3:
                case NativeB_SIO3:
		case NativeC_SIO3:
		  iocc_rsp[index] = 0;
		  break;
		case Eight_232:
		case Eight_422:
		case Eight_Mil:
		case Sixteen_232:
		case Sixteen_422:
		  iocc_rsp[(index*8) + te->te_anum] = 0;
		  break;
#endif                    /* _POWER */
	  }
    rv_flag = 1;
#ifdef _POWER
    for (index = 0; rv_flag && index < 3; ++index)
	  if (iocc->iocc_native[index])
        rv_flag = 0;
	/* May need to modify 16*18 since there are 2 MCA */
    for (index = 0; rv_flag && index < 16*8; ++index)
	  if (iocc->iocc_eight[index])
        rv_flag = 0;
#endif                    /* _POWER */
	
    if (rv_flag) {
		srsprintf("delete unused iocc struct ");
		assert(!pinfree(iocc));
		srs_ioccs[iocc_num] = 0;
    }
	
    /* (1.5) delete the edge vector into the tty structures */
    assert(!swatchfree(te));
    srs_tty[dev2slot(min)] = 0;
	
    /* (1) cleanup srs_tty and free if not used */
    for (index = 0; index < srs_cnt && !srs_tty[index]; ++index);
    if (index >= srs_cnt) {
		assert(!swatchfree(srs_tty));
		srs_tty = 0;
		srs_cnt = 0;
    }
	
    srsprintf("deleted successfully\n");
    return(0);
#endif                    /* SRS_DEL */
}

/*
 * srsrsrv
 *      Read servce procedure.
 *      Prioritary messages are forwarded upstreams immediately.
 *      Ordnary messages are sent when the module abive the driver can
 *      accept them. If not, they are left for back-enabling.
 *
 *
 * ARGUMENTS:
 *      q       - Driver's Streams read queue.
 *
 * RETURN VALUE: 0.
 *
 */

local int 
srsrsrv(queue_t *q)
{
    register str_rsp_t tp = (str_rsp_t) q->q_ptr;
    register rymp_t rsp = tp->t_hptr;
    mblk_t *mp;
    int old_intr_soft, old_intr_hard;
	
    Enter(TRC_RS(TTY_RSRV), tp->t_dev, tp, q->q_count, 0, 0);
	
    srsprintf("Enter srsrsrv(q=0x%x)\n",q);

    while (mp = getq(q)) {
		
        switch (mp->b_datap->db_type) {
	  case M_DATA:
            /* One possible enhencement: do this in the off-level routine; */
	    if (!tp->t_cread) {
                freemsg(mp);
                break;
            }
	  default:
            if((QPCTL <= mp->b_datap->db_type)|| canput(q->q_next)){
                putnext(q,mp);
                srsprintf("queued message further\n");
            }
            else {
                putbq(q,mp);
                goto local_flow_check;
            }
            break;
        }
    }

local_flow_check:

    /*
     * Immediate local flow control handling:
     * call current pacing dicpline service entry point to block/unclock
     * input.
     */
    /* Don't wait that that we get full before pacing the othe side */

     old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
     if (((((tp->t_wptr - tp->t_rptr) & (RBSIZE -1)) > SRS_HWAT) ||
          (q->q_count > (q->q_lowat + q->q_hiwat)>>1)) &&
         !(tp->t_block)) {
         srs_proc(tp, T_BLOCK);
     }
     if ((((tp->t_wptr - tp->t_rptr) & (RBSIZE - 1)) <= SRS_LWAT) && (q->q_count <= q->q_lowat) &&
         (tp->t_block)) {
        srs_proc(tp, T_UNBLOCK);
     }
     unlock_enable(old_intr_soft, &tp->t_lock_soft);
    
    srsprintf("Exit from Read Service Routine\n");
    Return(1);
}

/*
 * srswput
 *      Driver's write put procedure.
 *
 *
 * ARGUMENTS:
 *      q       - Driver's Streams write queue.
 *      mp      - message to process.
 *
 * RETURN VALUE: 0.
 *
 */

local int
srswput(queue_t *q, mblk_t *mp)
{
    register str_rsp_t tp = (str_rsp_t) q->q_ptr;
    register mblk_t *nmp;
    int old_intr_soft, old_intr_hard;
    struct ccblock xoffblk;
	
    Enter(TRC_RS(TTY_WPUT), tp->t_dev, tp, mp, mp->b_datap->db_type, 0);
	
    switch (mp->b_datap->db_type) {
		
      case M_BREAK:
      case M_DELAY:
        putq(q,mp);
        break;

      case M_DATA:
        /* if no more left, it returns */
        while (mp) {
            nmp = unlinkb(mp);
            if(q->q_count == 0){
                 old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
                 if ((tp->t_busy) || (tp->t_stop)){
                     unlock_enable(old_intr_soft, &tp->t_lock_soft);
                     putq(q,mp);
                     mp = nmp;
                     continue;
                 }
                 if(mp->b_wptr > mp->b_rptr) {
                     tp->t_tbuf.c_count = (mp->b_wptr - mp->b_rptr);
                     tp->t_tbuf.c_ptr = mp->b_rptr;
		     if (tp->t_outmsg) freemsg(tp->t_outmsg);		/* 156569 */
                     tp->t_outmsg = mp;
                     srsprintf("Calling srs_proc with T_OUTPUT\n ");
                     srs_proc(tp,T_OUTPUT);
                     unlock_enable(old_intr_soft, &tp->t_lock_soft);
                     /* output the message */
                     mp = nmp;
                     continue;
                 }
                 unlock_enable(old_intr_soft, &tp->t_lock_soft);
                 freemsg(mp);
                 mp = nmp;
            } else
                 putq(q, mp);
                 mp = nmp;
        }
        break;

      case M_IOCTL:
        old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
        srs_ioctl(q,mp,1);
        unlock_enable(old_intr_soft, &tp->t_lock_soft);
        break;

      case M_CTL:
        srs_ctl(q,mp);
        break;

      case M_FLUSH:{
	  int flag = 0;
		  
	  if(*mp->b_rptr & FLUSHW) {
		  flag |= FLUSHW;
		  *mp->b_rptr &= ~FLUSHW;
	  }
	  if( *mp->b_rptr & FLUSHR) {
		  flag |= FLUSHR;
		  qreply(q,mp);
	  }
	  else 
            freemsg(mp);
	  srs_flush(tp,flag,0);    
	  }
          break;
		
        /* These messages are generated by stream head to control
           the output flow */
      case M_STOP:
        old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
        srs_proc(q->q_ptr,T_SUSPEND);
        tp->t_localstop = 1;
        unlock_enable(old_intr_soft, &tp->t_lock_soft);
        freemsg(mp);
        break;

      case M_START:
        old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
        srs_proc(q->q_ptr,T_RESUME);
        tp->t_localstop = 0;
        unlock_enable(old_intr_soft, &tp->t_lock_soft);
        freemsg(mp);
        break;
		
        /* These messages are generated by ldterm to control the input flow */
      case M_STOPI:
      {
        old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
        if (tp->t_softpacing & IXOFF) {
            xoffblk.c_ptr = &(tp->t_softchars[STOPCHAR]);
            xoffblk.c_count = 1;
            srs_service(tp, TS_PRIOUTPUT, &xoffblk);
        } else {
            srs_proc(q->q_ptr,T_BLOCK);
        }
        unlock_enable(old_intr_soft, &tp->t_lock_soft);
	freemsg(mp);
	break;
      }
      case M_STARTI:
        old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
        if (tp->t_softpacing & IXOFF) {
            xoffblk.c_ptr = &(tp->t_softchars[STARTCHAR]);
            xoffblk.c_count = 1;
            srs_service(tp, TS_PRIOUTPUT, &xoffblk);
        } else {
            srs_proc(q->q_ptr,T_UNBLOCK);
        }
        unlock_enable(old_intr_soft, &tp->t_lock_soft);
	freemsg(mp);
	break;
		
      /* M_PROTO messages: sent by sptr module */
      case M_PROTO: {
        mblk_t *mp1;

        if (*(int *)mp->b_rptr != LPWRITE_REQ) {
            freemsg(mp);
            break;
        }
        mp1 = unlinkb(mp);    /* save the M_PROTO block to be
                                 enqueued at the end */
        while (mp1) {
            nmp = unlinkb(mp1);
            putq(q,mp1);
            mp1 = nmp;
        }
        putq(q,mp);
        break;
      }

      default:
        freemsg(mp);
        break;
    }
    Return(0)
}

/*
 * srswsrv
 *      Write servce procedure.
 *      Processes as many messages as hardware allows.
 *      If exits before completion than will be called
 *      again (qenable) by off-level routine.
 *
 *
 * ARGUMENTS:
 *      q       - Driver's Streams write queue.
 *
 * RETURN VALUE: 0.
 *
 */

int 
srswsrv(queue_t *q)
{
    str_rsp_t tp = (str_rsp_t)q->q_ptr;
    mblk_t *mp;
    int old_intr_soft;
	
    Enter(TRC_RS(TTY_WSRV), tp->t_dev, tp, q->q_count, 0, 0);
	
    srsprintf("Enter srswsrv(q=0x%x)\n",q);
	
    while (mp=getq(q)) {
	switch(mp->b_datap->db_type) {
	  case M_DELAY:
            old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
	    if (!(tp->t_stop)) {
		if (tp->t_timeout)
		    untimeout(tp->t_timeout);

		tp->t_timeout = timeout(srs_timeout,
                                        (caddr_t) tp, * ((int *) mp->b_rptr));
		srs_proc(tp,T_SUSPEND);
	    }
            unlock_enable(old_intr_soft, &tp->t_lock_soft);
	    freemsg(mp);
       	    break;

	  case M_BREAK:
            old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
	    if (tp->t_busy) {
	    	putbq(q,mp);
                unlock_enable(old_intr_soft, &tp->t_lock_soft);
	    	Return(0);
	    }
            unlock_enable(old_intr_soft, &tp->t_lock_soft);
	    if (*(int *)mp->b_rptr)
	          /* send a break */
		  srs_break_set(tp, 0);
	    else
		  /* clear the break */
		  srs_break_clear(tp);
	    freemsg(mp);
	    break;

	  case M_IOCTL:
            old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
            if (tp->t_busy) {
	        putbq(q,mp);
                unlock_enable(old_intr_soft, &tp->t_lock_soft);
                Return(0);
            }
            if (srs_ioctl(q,mp,0) > 0) {
	        putbq(q,mp);
                unlock_enable(old_intr_soft, &tp->t_lock_soft);
                Return(0);
            }
            unlock_enable(old_intr_soft, &tp->t_lock_soft);
            break;
	  case M_DATA:
                old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
		if ((tp->t_busy) || (tp->t_stop)) {
			putbq(q,mp);
                        unlock_enable(old_intr_soft, &tp->t_lock_soft);
			Return(0);
		}
		if(mp->b_wptr > mp->b_rptr) {
			tp->t_tbuf.c_count = (mp->b_wptr - mp->b_rptr);
			tp->t_tbuf.c_ptr = mp->b_rptr;
			if (tp->t_outmsg) freemsg(tp->t_outmsg);	/* 156569 */
			tp->t_outmsg = mp;
			srsprintf("Calling srs_proc with T_OUTPUT\n");
			srs_proc(tp,T_OUTPUT);
                        unlock_enable(old_intr_soft, &tp->t_lock_soft);
			/* output the message */
			break;
		}
                unlock_enable(old_intr_soft, &tp->t_lock_soft);
		freemsg(mp);
		break;

          /* Inform the serial printer module that its last output request
             was terminated */
	  case M_PROTO:
                /* last M_DATA not completely transmitted yet */
                old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
                if ((tp->t_outmsg) && (((rymp_t) (tp->t_hptr))->ry_xcnt)) {
		    putbq(q,mp);
                    unlock_enable(old_intr_soft, &tp->t_lock_soft);
		    Return(0);
                }
                unlock_enable(old_intr_soft, &tp->t_lock_soft);
                mp->b_datap->db_type = M_PCPROTO;
                *(int *)mp->b_rptr = LPWRITE_ACK;
                qreply(q, mp);
		break;

	  default:
		freemsg(mp);
		break;
	}/* end of switch */
    }/* end of while */

    /* All pending data at close time was drained down, and nothing
     * being transmitted:
     * wake up closing thread.
     * This is needed here because the last can other than M_DATA.
     */
    old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
    if (!(mp) && (tp->t_draining) && !(tp->t_busy)) {
        tp->t_draining = 0;
        unlock_enable(old_intr_soft, &tp->t_lock_soft);
        TTY_WAKE(tp);  /* may be modified by a e_wakeupx */
    } else {
        unlock_enable(old_intr_soft, &tp->t_lock_soft);
    }
    Return(0);
    srsprintf("EXit from Write Service routine\n");
}

/* bottom half of rs driver */

/*
 * srs_proc
 *      Called by srs_service routine when requested service is
 *      TS_PROC and sub-command is T_SUSPEND, T_RESUME, T_WFLUSH
 *      or T_RFFLUSH.
 *      Called by srswsrv and off-level routine to output data to
 *      uart reggisters, when possible.
 *      In both cases, srs_proc is called at smin_priority level with the
 *      per-line lock (tp->t_lock field).
 *
 *
 * ARGUMENTS:
 *      tp      - str_rs structure pointer.
 *      cmd     - enum proc_commands to be perfomed.
 *
 * RETURN VALUE: 0.
 *
 */

local int 
srs_proc(str_rsp_t tp, enum proc_commands cmd)
{
#define SRS_PROC
#ifdef SRS_PROC
    DoingPio;
    register rymp_t rsp = tp->t_hptr;
    register struct srs_port *port, *outport;
    register int result = 0;
    register int cnt;
    register char *ptr, c;
    register caddr_t base_page;
    register int old_intr_hard;
	
    Enter(TRC_RS(TTY_PROC), tp->t_dev, tp, cmd, 0, 0);
	
    if (!rsp) {
		srs_err(sresource, ERRID_TTY_PROG_PTR, 0);
		Return(ENODEV);
    }
    srsprintf("rsp OK\n");
    base_page = io_att(rsp->ry_intr.bid, 0);
    outport = port = (struct srs_port *)(base_page + rsp->ry_port);
    
    srsprintf("Outputport is %x\n",outport);
    switch (cmd) {
      case T_WFLUSH:            /* flush output side */
	rsp->ry_xcnt = 0;        /* sent that block */
        if (tp->t_outmsg) {
            freemsg(tp->t_outmsg);
            tp->t_outmsg = (mblk_t *)NULL;
        }
	/* fall through */
		
      case T_RESUME:            /* resume output */
        if (tp->t_softpacing & IXON) {
            tp->t_flow_state &= ~SOFT_REMOTE_STOP;
        }
        if (tp->t_hardpacing & (CTSXON | CDXON)) {
            tp->t_flow_state &= ~HARD_REMOTE_STOP;
        }
	tp->t_stop = 0;            /* and then fall through */
	qenable(WR(tp->t_ctl)); 
		
      case T_OUTPUT:            /* start output */
	srsprintf("In T_OUTPUT \n");
		
	if (tp->t_stop || tp->t_busy)    /* output blocked or in progress */
    	    break;
		
	if (!rsp->ry_xcnt) {        /* need another buffer */
	    if(tp->t_tbuf.c_count) {
		  rsp->ry_xcnt = tp->t_tbuf.c_count;
		  rsp->ry_xdata = tp->t_tbuf.c_ptr;
		  srsprintf("rsp xcnt and data init\n");
	    }
	    else break;
        }
		
	rsp->ry_watch = 0;        /* do this before busy set */
	tp->t_tbuf.c_ptr = (char *)NULL;
	tp->t_tbuf.c_count = 0;
	switch (rsp->ry_type) {
#ifdef _POWER
        case NativeA_SIO:
        case NativeB_SIO:
        case NativeA_SIO1:
        case NativeB_SIO1:
        case NativeA_SIO2:
        case NativeB_SIO2:
        case NativeA_SIO3:
        case NativeB_SIO3:
        case NativeC_SIO3:
       	    goto dump_chars;
			
        case Eight_232:
        case Eight_422:
        case Eight_Mil:
        case Sixteen_232:
        case Sixteen_422:
	    outport = (struct srs_port *)((ulong)outport|0x0400);

        dump_chars:
            if ((cnt = rsp->ry_tbc) > rsp->ry_xcnt) /* get cnt of chars */
                cnt = rsp->ry_xcnt;
	    old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);	/* 156569 */
	    cnt = MIN(cnt, rsp->ry_fcnt);					/* 156569 */
            rsp->ry_xcnt -= cnt;    /* keep track */
            rsp->ry_ocnt += cnt;
            ptr = rsp->ry_xdata;
            while ((cnt -=1) >= 0) {    /* iocc breaks this up */
                StartPio(srs_pio_catch, rsp,
                         io_det(base_page);
                         unlock_enable(old_intr_hard, &tp->t_lock_hard);
                         Return(EIO));
                BUSIO_PUTC(&outport->r_thr, *ptr++);
		rsp->ry_fcnt--;				/* 156569 */
                EndPio();
            }
            rsp->ry_xdata = ptr;
            srsprintf("enabling Trans intr\n");
	    if (!rsp->ry_fcnt || rsp->force_xmit) tp->t_busy = 1;	/* 156569 */
	    if (rsp->force_xmit) rsp->force_xmit = 0;			/* 156569 */
	    if (tp->t_busy) {						/* 156569 */
		rsp->fake_xmit = 0;					/* 156569 */
            	SR(port->r_ier, GR(port->r_ier)|ETHREI);		/* 156569 */
	    }								/* 156569 */
	    else {							/* 156569 */
		rsp->fake_xmit = 1;					/* 156569 */
	        SR(port->r_ier, GR(port->r_ier) &~ETHREI);		/* 156569 */
	    }								/* 156569 */
            unlock_enable(old_intr_hard, &tp->t_lock_hard);

            break;
#endif                    /* _POWER */
	}
	/* tp->t_busy = 1;   		156569 */
        srsprintf("End of T_OUTPUT\n");
	break;
		
      case T_SUSPEND:            /* suspend output */
        if (tp->t_softpacing & IXON) {
            tp->t_flow_state |= SOFT_REMOTE_STOP;
        }
        if (tp->t_hardpacing & (CTSXON | CDXON)) {
            tp->t_flow_state |= HARD_REMOTE_STOP;
        }
	tp->t_stop = 1;
	break;
		
      case T_RFLUSH: {          /* flush input side */
        uchar read_char;
        tp->t_rptr = tp->t_wptr; 	
        /* flush the characters in the FIFO */
        StartPio(srs_pio_catch, rsp, io_det(base_page); Return(EIO));
	for (cnt=0; cnt <= 15; cnt++) {
            /* Do what is done by CHECK_XON_PACING macro in the slihs,
               except that characters are not interesting if not used by flow
               control */
            read_char = GR(port->r_rbr);
            if ((tp->t_softpacing & IXON) &&
                (read_char != (uchar)_POSIX_VDISABLE)) {
                if (!(tp->t_flow_state & SOFT_REMOTE_STOP)) {
                    if (read_char == tp->t_softchars[STOPCHAR]) {
                        tp->t_flow_state |= SOFT_REMOTE_STOP;
                        tp->t_stop = 1;
                    }
                } else {
                    if ((read_char == tp->t_softchars[STARTCHAR]) ||
                        (tp->t_softpacing & IXANY)) {
                        tp->t_flow_state &= ~SOFT_REMOTE_STOP;
                        if (!(tp->t_flow_state & HARD_REMOTE_STOP)) {
                            tp->t_stop = 0;
                        }
                    }
                }
            }
        }
        EndPio();
	break;
      }
      case T_BLOCK:
       /* If both hard and software local flow control are provided
           then send the stop character before dropping the EIA signal
         */
        if (tp->t_softpacing & IXOFF) {
            struct ccblock xoffblk;
            xoffblk.c_ptr = &(tp->t_softchars[STOPCHAR]);
            xoffblk.c_count = 1;
	    srs_service(tp, TS_PRIOUTPUT, &xoffblk);
            tp->t_flow_state |= SOFT_LOCAL_STOP;
        }
        /* Only one type of local hardware pacing at a time */
        if (tp->t_hardpacing & (RTSXOFF)) {
            int modem_cntl;
            srs_service(tp, TS_GCONTROL, &modem_cntl);
            srs_service(tp, TS_SCONTROL, modem_cntl & ~TSRTS);
            tp->t_flow_state |= HARD_LOCAL_STOP;
        } else {
            if (tp->t_hardpacing & (DTRXOFF)) {
                int modem_cntl;
                srs_service(tp, TS_GCONTROL, &modem_cntl);
                srs_service(tp, TS_SCONTROL, modem_cntl & ~TSDTR);
                tp->t_flow_state |= HARD_LOCAL_STOP;
            }
        }
        tp->t_block = 1;
        break;
        
      case T_UNBLOCK:
       /* If both hard and software local flow control are provided
           then set the EIA signal before sending the stop character
         */
        if (tp->t_softpacing & IXOFF) {
            struct ccblock xoffblk;
            xoffblk.c_ptr = &(tp->t_softchars[STARTCHAR]);
            xoffblk.c_count = 1;
	    srs_service(tp, TS_PRIOUTPUT, &xoffblk);
            tp->t_flow_state &= ~SOFT_LOCAL_STOP;
        }
        /* Only one type of local hardware pacing at a time */
        if (tp->t_hardpacing & (RTSXOFF)) {
            int modem_cntl;
            srs_service(tp, TS_GCONTROL, &modem_cntl);
            srs_service(tp, TS_SCONTROL, modem_cntl | TSRTS);
            tp->t_flow_state &= ~HARD_LOCAL_STOP;
        } else {
            if (tp->t_hardpacing & (DTRXOFF)) {
                int modem_cntl;
                srs_service(tp, TS_GCONTROL, &modem_cntl);
                srs_service(tp, TS_SCONTROL, modem_cntl | TSDTR);
                tp->t_flow_state &= ~HARD_LOCAL_STOP;
            }
        }
        tp->t_block = 0;
        break;

      default:
	result = -1;
    }
	
    io_det(base_page);
    srsprintf("Endof srs_proc\n");
    Return(result);
#endif                    /* SRS_PROC */
}

/*
 * srs_service
 *      This function pushes values down into and gets them from
 *      the chip 
 *      Is is called by other parts of the driver on some M_IOCTL
 *      or M_CTL, or M_BREAK processing.
 *      It is also called by the the open and flow controle
 *      disciplines.
 *
 *
 * ARGUMENTS:
 *      tp      - str_rs structure pointer.
 *      cmd     - enum service_commands to be perfomed.
 *      varg    = Command argument or sub-command for TS_S* commands.
 *              = address of a storage to contain requested information for
 *                TS_G* commands.
 *
 * RETURN VALUE:
 *      0 on success, otherwise error code.
 *
 */

local int 
srs_service(str_rsp_t tp,  enum service_commands cmd,
			  void *varg)
{
#define SRS_SERVICE
#ifdef SRS_SERVICE
    DoingPio;
    rymp_t rsp = tp->t_hptr;
    struct srs_port *port;
    uchar reg;
    int result = 0;
    int old_intr_hard;
    int baud_temp, *ptr;
    int arg = (int)varg;
	
    Enter(TRC_RS(TTY_SERVICE), tp->t_dev, tp, cmd, varg, 0);
	
    if (!rsp) {
		srs_err(sresource, ERRID_TTY_PROG_PTR, 0);
		Return(ENODEV);
    }
    port = Port(rsp);
	
    switch (cmd) {
		
      case TS_SCONTROL:
        old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);
        StartPio(srs_pio_catch, rsp,
                 io_det(port);
                 unlock_enable(old_intr_hard, &tp->t_lock_hard);
                 Return(EIO));
        reg = GR(port->r_mcr);
        EndPio();
		
        if (arg&TSDTR)
		  reg |= DTR;
        else
		  reg &= ~DTR;
        if (arg&TSRTS)
		  reg |= RTS;
        else
		  reg &= ~RTS;
		
        StartPio(srs_pio_catch, rsp,
                 io_det(port);
                 unlock_enable(old_intr_hard, &tp->t_lock_hard);
                 Return(EIO));
        SR(port->r_mcr, reg);
        EndPio();
        unlock_enable(old_intr_hard, &tp->t_lock_hard);
        break;
		
      case TS_GCONTROL:
        StartPio(srs_pio_catch, rsp, io_det(port); Return(EIO));
        reg = GR(port->r_mcr);
        EndPio();
		
        arg = (reg & DTR) ? TSDTR : 0;
        if (reg&RTS)
		  arg |= TSRTS;
		
        *(int *)varg = arg;
        break;

      case TS_GSTATUS:
        arg = 0;
        StartPio(srs_pio_catch, rsp, io_det(port); Return(EIO));
        reg = GR(port->r_msr) & 0xf0;
        EndPio();
        if (reg&DCD) arg |= TSCD;
        if (reg&RI) arg |= TSRI;
        if (reg&DSR) arg |= TSDSR;
        if (reg&CTS) arg |= TSCTS;
        *(int *)varg = arg;
        break;
		
      /* Not used internally by the driver but needed by wtopen */
      case TS_GBAUD:
        *((int *) varg) = tp->t_ospeed;
        break;

      case TS_SBAUD:
	if (arg == 0) {
            old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);
            StartPio(srs_pio_catch, rsp,
                     io_det(port);
                     unlock_enable(old_intr_hard, &tp->t_lock_hard);
                     Return(EIO));
	    reg = GR(port->r_mcr);
	    reg &= ~(DTR|RTS);
	    SR(port->r_mcr, reg);
            unlock_enable(old_intr_hard, &tp->t_lock_hard);
	    EndPio();
            break;
	}
	if (arg < 0)
	    result = EINVAL;
	else {
	    int new, diff;
	
	    /*
	     * Compute what to set the registers to as well as the
	     * amount of slop.  If the slop is too big then reject the
	     * request.
	     */
	    if (arg) {            /* 0 baud implies stopped -- I guess */
		new = arg * 2;
		if (arg == 134)
		    ++new;        /* new == 134.5 * 2 */
		if ((baud_temp = (((2 * rsp->ry_xtal) / new) + 8) / 16) == 0) {
		    result = EINVAL;
		    break;
		}
		new *= 100;
		if ((diff = new - (200 * rsp->ry_xtal) / (baud_temp * 16)) < 0)
		    diff = - diff;
		if ((diff / new) > TTY_BAUD_ERROR_LIMIT) {
		    result = EINVAL;
		    break;
		}
	    } else
	        baud_temp = 0;
			
            old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);
            StartPio(srs_pio_catch, rsp,
                     io_det(port);
                     unlock_enable(old_intr_hard, &tp->t_lock_hard);
                     Return(EIO));
	    set_dlab(port);
	    SR(port->r_dll, baud_temp);
	    SR(port->r_dlm, (baud_temp >> 8) & 0xff);
	    clr_dlab(port);
	    EndPio();
            unlock_enable(old_intr_hard, &tp->t_lock_hard);
			
	    tp->t_ospeed = tp->t_ispeed = (baud_t)arg;
	}
	break;
		
      case TS_SBPC:
	StartPio(srs_pio_catch, rsp, io_det(port); Return(EIO));
	reg = GR(port->r_lcr) & ~(WLS1|WLS0);
	EndPio();
	
	switch (arg) {
	  case bits5:
		break;
	  case bits6:
		reg |= WLS0;
		break;
	  case bits7:
		reg |= WLS1;
		break;
	  case bits8:
		reg |= (WLS1|WLS0);
		break;
	}
        /* no need for protection against slihs since never affected by them */
	StartPio(srs_pio_catch, rsp, io_det(port); Return(EIO));
	SR(port->r_lcr, reg);
	EndPio();
	
	tp->t_csize = (csize_t)arg;
	break;
		
      case TS_GPARITY:
        StartPio(srs_pio_catch, rsp, io_det(port); Return(EIO));
        reg = GR(port->r_lcr);
        EndPio();

        *(int *)varg =
            (int)(!(reg&PEN) ? nopar :
                (reg&STICK) ? ((reg&EPS) ? spacepar : markpar) :
                ((reg&EPS) ? evenpar : oddpar));

        break;

      case TS_SPARITY:
	StartPio(srs_pio_catch, rsp, io_det(port); Return(EIO));
	reg = GR(port->r_lcr)&~(STICK|EPS|PEN);
	EndPio();
		
	switch (arg) {
	  case nopar:
	  case oddonly:
		break;
	  case oddpar:
		reg |= PEN;
		break;
	  case markpar:
		reg |= (STICK|PEN);
		break;
	  case evenpar:
		reg |= (EPS|PEN);
		break;
	  case spacepar:
		reg |= (STICK|EPS|PEN);
		break;
	}
		
	StartPio(srs_pio_catch, rsp, io_det(port); Return(EIO));
	SR(port->r_lcr, reg);
	EndPio();
		
	tp->t_parity = (parity_t)arg;
	break;
		
      case TS_SSTOPS:
	StartPio(srs_pio_catch, rsp, io_det(port); Return(EIO));
	reg = GR(port->r_lcr);
	EndPio();
		
	if ((stop_t)arg==stop1)
	  reg &= ~STB;
	else
	  reg |= STB;
	StartPio(srs_pio_catch, rsp, io_det(port); Return(EIO));
	SR(port->r_lcr, reg);
	EndPio();
	
	tp->t_stopbits = (stop_t)arg;
	break;
	
      case TS_SBREAK:
	StartPio(srs_pio_catch, rsp, io_det(port); Return(EIO));
	SR(port->r_lcr, GR(port->r_lcr)|BREAK);
	EndPio();
	tp->t_busy = 1;
	break;
		
      case TS_CBREAK:
	StartPio(srs_pio_catch, rsp, io_det(port); Return(EIO));
	SR(port->r_lcr, GR(port->r_lcr)&~BREAK);
	EndPio();
	tp->t_busy = 0;
	break;
		
	/* 
	   the prioritary characters are in a ccblock structure pointed to by
	   varg argument. If the UART is ready, send chars right now, otherwise,
	   wait for the end of transmission interruption handler which will
	   start with priorotary chars.
         */
      case TS_PRIOUTPUT:
        if (tp->t_busy) {  /* will be sent later in the off_level handler */
            tp->t_pribuf = *(((struct ccblock *) varg)->c_ptr);
        } else { /* send the control character immediately */
            register struct srs_port *outport;
            register caddr_t base_page;
            base_page = io_att(rsp->ry_intr.bid, 0);
            outport = (struct srs_port *)(base_page + rsp->ry_port);

            switch (rsp->ry_type) {
#ifdef _POWER
            case Eight_232:
            case Eight_422:
            case Eight_Mil:
            case Sixteen_232:
            case Sixteen_422:
                outport = (struct srs_port *)((ulong)outport|0x0400);
            default:
                old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);
                StartPio(srs_pio_catch, rsp,
                         io_det(base_page);
                         unlock_enable(old_intr_hard, &tp->t_lock_hard);
                         Return(EIO));
                BUSIO_PUTC(&outport->r_thr, *(((struct ccblock *)varg)->c_ptr));
                SR(port->r_ier, GR(port->r_ier)|ETHREI);
                EndPio();
                unlock_enable(old_intr_hard, &tp->t_lock_hard);

#endif                    /* _POWER */
            }
            io_det(base_page);
            tp->t_busy = 1;
        }
        break;

    case TS_LOOP:
        if (NativeA_SIO2 == rsp->ry_type || NativeB_SIO2 == rsp->ry_type) {
            result = EINVAL;       /* See ::Note 2:: */
            break;
        }
        old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);
        StartPio(srs_pio_catch, rsp,
                 io_det(port);
                 unlock_enable(old_intr_hard, &tp->t_lock_hard);
                 Return(EIO));
        if (arg == LOOP_ENTER)
            SR(port->r_mcr, (GR(port->r_mcr)|LOOP));
        else if (arg == LOOP_EXIT)
            SR(port->r_mcr, (GR(port->r_mcr) & ~LOOP));
        else
            result = EINVAL;
        EndPio();
        unlock_enable(old_intr_hard, &tp->t_lock_hard);
        break;
		
      case TS_PROC:
        result = srs_proc(tp, arg);
        break;
		
      default:
	result = -1;
	break;
    }
    io_det(port);
	
    Return(result);
#endif                    /* SRS_SERVICE */
}

/*
 * srs_allocmsg
 *      Allocate a new message for received input.  Arrange a callback
 *      if we can't get one now.
 *
 * ARGUMENTS:
 *      tp    - pointer to str_rs struct
 *
 * RETURN VALUE:
 *      alocated message.
 *
 */
local mblk_t *
srs_allocmsg(str_rsp_t tp)
{
	register mblk_t *mp;
	
	if (tp->t_inmsg = allocb(SRS_RDBUFSZ, BPRI_MED))
	    return(tp->t_inmsg);
	else {
            if (!(tp->t_bufcall)) {
    	        if (!(tp->t_bufcall = bufcall(SRS_RDBUFSZ, BPRI_MED,
			                      srs_recover, (caddr_t)tp))) {
                   
                   if (!(tp->t_alloctimer))
		       tp->t_alloctimer = timeout(srs_allocmsg,
                                                  (caddr_t)tp, hz);
		   return(0);    
		} else {    /* bufcall request recorded: give up calling it */
                   if (tp->t_alloctimer) {
		       untimeout(tp->t_alloctimer);
                       tp->t_alloctimer = 0;
                   }
                }
            }
	}
	return (0);
}

/*
 * srs_recover
 *      Called through bufcall utility to recover allocb failure.
 *      If an allocation failure happened in the off-level
 *      routine, we schedule it.
 *
 * ARGUMENTS:
 *      tp    - pointer to str_rs struct
 *
 * RETURN VALUE: 0.
 *
 * SIDE EFFECT:
 *    possible scheduling of the srs_offlevel.
 *
 */
local int
srs_recover(str_rsp_t tp)
{
    register mblk_t *mp;
    int old_intr_soft;

    old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
	
    /* Warning! for MP saffety, must be an atomic primitive */
    tp->t_bufcall = 0;

    /* If allocb fails, try again, Forever ?! */
    if (!(srs_allocmsg(tp))) {
        unlock_enable(old_intr_soft, &tp->t_lock_soft);
        return(0);
    }
    
    if (tp->t_sched) {
        /* Warning! for MP saffety, must be an atomic primitive */
        tp->t_sched = 0;
	/* Remember: intr struct is the first field in rym struct */
	i_sched((struct intr *) tp->t_hptr);
    }
    unlock_enable(old_intr_soft, &tp->t_lock_soft);
    return(0);
}

/* 
 * srs_timeout
 *      Function argument to timeout utility.
 *      resets the flag t_timeout int the  str_rs struct.
 *
 * ARGUMENTS:
 *      tp    - pointer to str_rs struct
 * 
 * Return Value: 0
 */

local int
srs_timeout(str_rsp_t tp)
{
    int old_intr_soft;

    old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
    tp->t_timeout = 0;
                      
    /* timeout call is due to M_DELAY processing */
    if (tp->t_stop) {
        srs_proc(tp,T_RESUME);
    }    
    unlock_enable(old_intr_soft, &tp->t_lock_soft);
    return(0);
}

/* Called when q->hiwat is exceeded */
#define LOG_OVERRUN                                           \
    struct hog_err {                                          \
        int dev;                                              \
        int cnt;                                              \
    } hog_err;                                                \
    ERR_REC(sizeof(hog_err)) cfg_err;                         \
    cfg_err.error_id = ERRID_TTY_TTYHOG;                      \
    bcopy(tp->t_name, cfg_err.resource_name, sizeof(cfg_err.resource_name)); \
    hog_err.dev = tp->t_dev;                                    \
    hog_err.cnt = msg_cnt;                                      \
    *(struct hog_err *)cfg_err.detail_data = hog_err;           \
    errsave(&cfg_err, sizeof(cfg_err));                         \
    /* dump current message */                                  \
    mp->b_wptr = mp->b_rptr = mp->b_datap->db_base;             \
    mp = 0;
    
/* 
 * srs_offlevel
 *      Off-level interrupt handler.
 *      This routine completes intrruption handling rescheduled
 *      by slih.
 *      Correctly received characters are put inside M_DATA messages,
 *      erronous data issue an M_BREAK. Modem status change issue
 *      an M_CTL.
 *      On end-of-transmit interrupt, call srs_proc to carry on
 *      transmitting or back-enable the write queue.
 *      Open and flow control disciplines input entrypoints are called
 *      on receiving some events. 
 *
 * ARGUMENTS:
 *      rintr   - pointer to off-level intr struct.
 * 
 * Return Value: 0.
 */

/*
 * The data coming from the slih has the following format:  If we have
 * good data, then the status is 0.  If we have a lsr problem, then the
 * status is equal to the lsr register with the low bit forced to 0.
 * Note that we know that at least one bit will be set or we would not
 * have an lsr problem in the first place.  Last is modem status.  In
 * this case the status is the msr register with the low bit forced on
 * (to make it never be 0) and use the current status in the upper
 * four bits (which could be all 0's).  In this scheme we maintain an
 * old copy of the msr for the delta's and we call the pacing for each
 * change we see.
 */
local int 
srs_offlevel(struct intr *rintr)
{
#define SRS_OFFLEVEL
#ifdef SRS_OFFLEVEL
    DoingPio;
    register rymp_t rsp = (rymp_t)rintr;
    register str_rsp_t tp = rsp->ry_tp;
    register struct clist *cl = &rsp->ry_data;
    register int status, err;
    register uchar data;
    register queue_t *q = tp->t_ctl;
    register int ctlx = tp->t_ctlx;
    register int old_intr_soft;
    register int old_intr_hard;
    register int old_xmit = 0;
    register int sio2 = 0;
    mblk_t *mp = NULL;
    int msg_cnt = 0;    /* Num of bytes in the message */
    char * msg_ptr;
    register enum status status_delta;
    int old_ioff;
	
    Enter(TRC_RS(TTY_OFFL), tp->t_dev, rintr, 0, 0, 0);
	
    srsprintf("In Offlev\n");
    /* Need both hard and soft locks since srs_offlevel can run concurrently
       2 CPUs and the following flags are touched by the slihs */
    old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);

    if (!(rsp->ry_open) && !(tp->t_wopen)) {
        old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);
        rsp->ry_posted = 0;
        rsp->ry_xmit = 0;            /* clear xmit for next time */
        rsp->ry_ioff = 0;
        unlock_enable(old_intr_hard, &tp->t_lock_hard);
        unlock_enable(old_intr_soft, &tp->t_lock_soft);
        Return(0);
    }
    old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);
    rsp->ry_posted = 0;            /* get posted again */
    if (rsp->ry_xmit) {
        old_xmit = 1;              /* save xmit flag */
        rsp->ry_xmit = 0;          /* clear xmit for next time */
    }

    old_ioff = rsp->ry_ioff;
    rsp->ry_ioff = 0;
    unlock_enable(old_intr_hard, &tp->t_lock_hard); /* back to INT_TTY level */
	
    while ((tp->t_inmsg) && (tp->t_rptr != tp->t_wptr)) { 	
        srsprintf("Received Data\n");
    
	status = *tp->t_rptr++;				
        data = *tp->t_rptr++;			
	if (tp->t_rptr >= &tp->t_rbuf[RBSIZE - 1])	
		tp->t_rptr = &tp->t_rbuf[0];	
        if (!(mp)) {
            mp = tp->t_inmsg;
            msg_ptr = mp->b_rptr;
            msg_cnt = 0;
        }
        if (!(status&(BI|FE|PE|OE|1))) { /* good data 99% of the time */
            if (tp->t_sak) {
                if (ctlx && data == ('R' - '@')) {
                    mp->b_datap->db_type = M_PCSIG;
                    *(unsigned char *)mp->b_rptr = SIGSAK;
                    mp->b_wptr = mp->b_rptr+ sizeof (unsigned char);

                    /* Don't need canput testing since M_PCSIG is a
                     * high priority message */
                    putnext(q,mp);

                    mp = tp->t_inmsg = 0; /* To be sure we'll test for
                                           * newly allocated message */
                    srs_allocmsg(tp);
                    ctlx = 0;
                    continue;
                }
                if (ctlx){
                    *msg_ptr++ = 'X' - '@';
                    msg_cnt++;
                }
                if (ctlx = (data == ('X' - '@')))
                    continue;
            }
            *msg_ptr++ = data;
            msg_cnt++;
            /* message got full: enqueue it and allocate another */
            if (msg_cnt >= SRS_RDBUFSZ) {
                if (q->q_count > q->q_hiwat) { /* Start dumping data */
                    /* Log an error */
                    LOG_OVERRUN
                    continue;  /* dump all the next events in the clists */
                }

		if (tp->t_cread) {

		    mp->b_wptr = msg_ptr;
		    mp->b_datap->db_type = M_DATA;

		    srs_allocmsg(tp);
		    if (!q->q_count && canput(q->q_next)) {

			unlock_enable(old_intr_soft, &tp->t_lock_soft);
			putnext(q, mp); /* putq(q, mp); */
			old_intr_soft = disable_lock(INT_TTY,
						     &tp->t_lock_soft);
		    } else
			insq(q, 0, mp);

		    mp = 0;
		}
           }    
        } else if (status&0x01) {    /* modem status change */
            srsprintf("In modem errors  \n");
            if(msg_cnt) {
            /* send up already collected data before notifying status change */
                if (q->q_count > q->q_hiwat) { /* Start dumping data */
                    /* Log an error */
                    LOG_OVERRUN
                    continue; /* next getc() */

                } else {
		    if (tp->t_cread) {
			mp->b_datap->db_type = M_DATA;
			mp->b_wptr = mp->b_rptr + msg_cnt;

			srs_allocmsg(tp);
			if (!q->q_count && canput(q->q_next)) {

			    unlock_enable(old_intr_soft, &tp->t_lock_soft);
			    putnext(q, mp); /* putq(q, mp); */
			    old_intr_soft = disable_lock(INT_TTY,
							 &tp->t_lock_soft);
			} else
			    insq(q, 0, mp);

			if (!(mp = tp->t_inmsg)) {
			    tp->t_sched = 1;
			    break; /* */
			}
			msg_ptr = mp->b_rptr;
			msg_cnt = 0;
		    }
                }
            }
            if (q->q_count > q->q_hiwat) { /* Start dumping data */
                /* Log an error */
                LOG_OVERRUN
                continue; /* next getc() */
            } else {
		if (tp->t_draining && (data == cd_off)) {
               	    tp->t_stop = tp->t_draining = 0;
                    TTY_WAKE(tp);
                } else {
                    mp->b_datap->db_type = M_CTL;
                    *(enum status *)mp->b_rptr = (enum status) data;
                    mp->b_wptr = mp->b_rptr+ sizeof (enum status);
                    insq(q, NULL, mp); /* putq(q, mp); */
                    mp = 0;
                    msg_cnt = 0;
                    srs_allocmsg(tp);
                    if (!(mp = tp->t_inmsg)) {
                        tp->t_sched = 1;
                        break; /* */
                    }
                    msg_ptr = mp->b_rptr;
                }
            }
        } else {       /* Some trouble on the line: erronous data. */
            if(msg_cnt) {
            /* send up already collected data before notifying status change */
                if (q->q_count > q->q_hiwat) { /* Start dumping data */
                    /* Log an error */
                    LOG_OVERRUN
                    continue; /* next getc() */
                } else {
		    if (tp->t_cread) {
			mp->b_datap->db_type = M_DATA;
			mp->b_wptr = mp->b_rptr + msg_cnt;

			srs_allocmsg(tp);
			if (!q->q_count && canput(q->q_next)) {

			    unlock_enable(old_intr_soft, &tp->t_lock_soft);
			    putnext(q, mp); /* putq(q, mp); */
			    old_intr_soft = disable_lock(INT_TTY,
							 &tp->t_lock_soft);
			} else
			    insq(q, 0, mp);

			msg_cnt = 0;
			if (!(mp = tp->t_inmsg)) {
			    tp->t_sched = 1;
			    break; /* */
			}
			msg_ptr = mp->b_rptr;
		    }
                }
            }
            if (status&BI) {
                srsprintf("In break_intr\n");
                status_delta = break_interrupt;
            } else if (status&FE) {
                srsprintf("In frame error\n");
                status_delta = framing_error;
            } else if (status&PE) {
                srsprintf("In parity error\n");
                status_delta = parity_error;
            } else {
                sio2 = (NativeA_SIO2 == rsp->ry_type||NativeB_SIO2 == rsp->ry_type);
                srsprintf("In overrun error\n");
                status_delta = overrun;
            }
            if (q->q_count > q->q_hiwat) { /* Start dumping data */
                /* Log an error */
                LOG_OVERRUN
                continue; /* next getc() */
            } else {
                mp->b_datap->db_type = M_BREAK;
                *(enum status *)mp->b_wptr = status_delta;
                mp->b_wptr += 4;
                *(mp->b_wptr++) = data;
                insq(q, NULL, mp); /* putq(q, mp); */
                mp = 0;
                msg_cnt = 0;
                srs_allocmsg(tp);
                if (!(mp = tp->t_inmsg)) {
                    tp->t_sched = 1;
                    break; /* */
                }
                msg_ptr = mp->b_rptr;
            }
            /* ::Note 7::
             * VLSI reports overrun on the next received char
             * ie data is good data
             */
            if (sio2) {
                if (q->q_count > q->q_hiwat) { /* Start dumping data */
                    /* Log an error */
                    LOG_OVERRUN
                    continue; /* next getc() */
                } else {
		    if (tp->t_cread) {
			mp->b_datap->db_type = M_DATA;
			*(mp->b_wptr++) = data;

			srs_allocmsg(tp);
			if (!q->q_count && canput(q->q_next)) {

			    unlock_enable(old_intr_soft, &tp->t_lock_soft);
			    putnext(q, mp); /* putq(q, mp); */
			    old_intr_soft = disable_lock(INT_TTY,
							 &tp->t_lock_soft);
			} else
			    insq(q, NULL, mp); /* putq(q, mp); */

			msg_cnt = 0;
			if (!(mp = tp->t_inmsg)) {
			    tp->t_sched = 1;
			    break; /* */
			}
			msg_ptr = mp->b_rptr;
		    }
                }
            }
        }
    }
    /*
     * All the received events in clist were habndled, don't forget
     * to enqueue the last one and prepare a message for next call.
     */
     if ((mp) && (msg_cnt)) { /* Message not full and not empty */
         if (q->q_count <= q->q_hiwat) { /* Start dumping data */
	     if (tp->t_cread) {
		 mp->b_wptr = msg_ptr;
		 mp->b_datap->db_type = M_DATA;

		 srs_allocmsg(tp);
		 if (!q->q_count && canput(q->q_next)) {

		     unlock_enable(old_intr_soft, &tp->t_lock_soft);
		     putnext(q, mp); /* putq(q, mp); */
		     old_intr_soft = disable_lock(INT_TTY,
						  &tp->t_lock_soft);
		 } else
		     insq(q, NULL, mp); /* putq(q, mp); */
	     }
         } else {
             LOG_OVERRUN
         }
     }

    /*
     * Immediate local flow control handling:
     * call current pacing dicpline service entry point to block/unclock
     * input.
     */
     /* Don't wait that that we get full before pacing the othe side */
     if (((((tp->t_wptr - tp->t_rptr) & (RBSIZE - 1)) > SRS_HWAT) ||
          (q->q_count > (q->q_lowat + q->q_hiwat)>>1)) &&
         !(tp->t_block)) {
        srs_proc(tp, T_BLOCK);
     }
     if ((((tp->t_wptr - tp->t_rptr) & (RBSIZE - 1)) <= SRS_LWAT) && (q->q_count <= q->q_lowat) &&
         (tp->t_block)) {
        srs_proc(tp, T_UNBLOCK);
     }
    
    /*
     * The following is done after the receive stuff has been
     * processed so that any flow control stuff is done first.  It is
     * possible for xmit to get set after we check it here but posted
     * has already been turned off so a second offlevel will get
     * scheduled if that occurs.  Also, busy is checked first since we
     * don't have a flag to indicate that dma has already been
     * started not to mention that it saves time called proc, etc.
     */

    if (old_xmit) {
    if (tp->t_busy) {
        srsprintf("In xmit compl\n");
        tp->t_busy = 0;        /* not busy any more */
        if (tp->t_pribuf) { /* Send the emergency character, if any */
            register struct srs_port *outport, *port;
            register caddr_t base_page;
            base_page = io_att(rsp->ry_intr.bid, 0);
            outport = port = (struct srs_port *)(base_page + rsp->ry_port);

            switch (rsp->ry_type) {
#ifdef _POWER
            case Eight_232:
            case Eight_422:
            case Eight_Mil:
            case Sixteen_232:
            case Sixteen_422:
                outport = (struct srs_port *)((ulong)outport|0x0400);
            default:
                old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);
                StartPio(srs_pio_catch, rsp,
                         io_det(base_page);
                         unlock_enable(old_intr_hard, &tp->t_lock_hard);
                         goto normal_output);
                BUSIO_PUTC(&outport->r_thr, tp->t_pribuf);
                SR(port->r_ier, GR(port->r_ier)|ETHREI);
                EndPio();
                unlock_enable(old_intr_hard, &tp->t_lock_hard);
            }   
#endif                    /* _POWER */
            io_det(base_page);
            tp->t_busy = 1;
            tp->t_pribuf = 0;
        }   
normal_output:
        if(!rsp->ry_xcnt){
            if (tp->t_outmsg) {
                freemsg(tp->t_outmsg);
                tp->t_outmsg = (mblk_t *)NULL;
            }
            /* enable write queue only if there's at least one message
               waiting to be processed */
            if (WR(tp->t_ctl)->q_first)
                qenable(WR(tp->t_ctl)); 

            /* Wakeup the sleeping process at close time, for draining */
            if (tp->t_draining && !(WR(tp->t_ctl))->q_first) {
                tp->t_draining = 0;
                TTY_WAKE(tp);
            }

        }
        else {
	    rsp->force_xmit = 1;	/* 156569 */
            srs_proc(tp,T_OUTPUT);    /* send the remaining data */
        }
        } else {
	    rsp->force_xmit = 1;	/* 156569 */
            srs_proc(tp,T_RESUME);    /* output resumed by slih */
        }
    }

    /* if we turned off interrupts to get off the slih because
     * we ran out cblocks, we turn them back on here.
     * if we turn them off too many times (<threshold), log an error
     * and kill the port ...
     * allow up to 5 times per open ..
     */
    if (old_ioff) {
        old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);
        if (++(rsp->ry_ioffcnt) % 5) {
            DoingPio;
            register struct srs_port *port;
            port = Port(rsp);
            StartPio(srs_pio_catch, rsp, err = EIO; break);
            SR(port->r_ier, GR(port->r_ier)|(ERBDAI|ELSI|EMSSI));
            EndPio();
            io_det(port);
            unlock_enable(old_intr_hard, &tp->t_lock_hard);
        } else {
            struct hog_err {
                int dev;
                int cnt;
            } hog_err;
            ERR_REC(sizeof(hog_err)) cfg_err;
            cfg_err.error_id = ERRID_TTY_INTR_HOG;
            bcopy(tp->t_name, cfg_err.resource_name,
                sizeof(cfg_err.resource_name));
            hog_err.dev = tp->t_dev;
            hog_err.cnt = rsp->ry_ioffcnt;
            *(struct hog_err *)cfg_err.detail_data = hog_err;
            errsave(&cfg_err, sizeof(cfg_err));

            unlock_enable(old_intr_hard, &tp->t_lock_hard);

            unlock_enable(old_intr_soft, &tp->t_lock_soft);
            putctl2(q->q_next, M_ERROR, EIO, NOERROR);
            /* EIO for the read side, don't change for the writes */

            if (mp = allocb(sizeof(enum status), BPRI_MED)) {
                mp->b_datap->db_type = M_CTL;
                *(enum status *)mp->b_rptr = cd_off;
                mp->b_wptr = mp->b_rptr + sizeof (enum status);
                insq(q, NULL, mp);
            } else {
                srs_err(tp->t_name, ERRID_TTY_BADINPUT, ENOMEM);
            }
            /* I know, signalling cd_off is not always the truth.
               may be EIO  on writes is needed olso.
               If it is the only reason, EIO instead of NOERROR as a 4th
               parameter of the above putctl2() call is enough. */

            old_intr_soft = disable_lock(INT_TTY, &tp->t_lock_soft);
            /* A putcfl(rsp->ry_data) is probably advised here ?? */
        }
    }

    tp->t_ctlx = ctlx;
    unlock_enable(old_intr_soft, &tp->t_lock_soft);
    /* only reenable queue if there's a message to process or
       local flow control to handle */
    if (q->q_first)
        qenable(q); 

    Return(0);
#endif                    /* SRS_OFFLEVEL */
}

/*
 * Macro called by the three SLIHs to check if the input character is
 * used for flow control purpose or is a "normal" character to be sent
 * umpstreams
 */
#define CHECK_XON_PACING						\
    if ((tp->t_softpacing & IXON) && (read_char != (uchar)_POSIX_VDISABLE)) { \
        if (!(tp->t_flow_state & SOFT_REMOTE_STOP)) {			\
            if (read_char == tp->t_softchars[STOPCHAR]) {     		\
                tp->t_flow_state |= SOFT_REMOTE_STOP;			\
                tp->t_stop = 1;                                         \
                char_interesting = 0;                                   \
            } else {                                                    \
                char_interesting =                                      \
                     (read_char == tp->t_softchars[STARTCHAR]) ? 0 : 1; \
            }                                                           \
        } else {                                                        \
            if ((read_char == tp->t_softchars[STARTCHAR]) ||            \
                (tp->t_softpacing & IXANY)) {                           \
                tp->t_flow_state &= ~SOFT_REMOTE_STOP;                  \
                if (!(tp->t_flow_state & HARD_REMOTE_STOP)) {           \
                    tp->t_stop = 0;                                     \
                    rsp->ry_xmit = 1;                                   \
                }                                                       \
            }                                                           \
            char_interesting =                                          \
                     ((read_char == tp->t_softchars[STARTCHAR]) ||      \
                      (read_char == tp->t_softchars[STOPCHAR])) ? 0 : 1;\
        }                                                               \
    } else                                                              \
        char_interesting = 1;

/*
 * Macro that puts the modem status change event in the private ring
 * buffer. It is called by the three SLIHs.
 */
#define REGISTER_NEW_STATUS(new_event)                                  \
            depth = (tp->t_wptr - tp->t_rptr) & (RBSIZE - 1);           \
            if (depth && (depth >= RBSIZE - 2)) {                       \
                rc = -1;                                                \
                srs_err(rsp->ry_tp->t_name, ERRID_TTY_BADINPUT, -1);    \
            } else {                                                    \
                *tp->t_wptr = status;                                   \
                *(tp->t_wptr + 1) = (char) (new_event);                 \
                tp->t_wptr += 2;                                        \
                if (tp->t_wptr >= &tp->t_rbuf[RBSIZE-1])                \
                    tp->t_wptr = &tp->t_rbuf[0];                        \
            }

#ifdef _POWER
/*
 * Disable interrupts and save MSR.
 * Check key position.
 * If key is in service position call the debugger.
 * Enable interrupts and restore MSR.
 */

#define call_dbg							\
{									\
    register int intlev;						\
    register ulong *pskd;						\
    volatile struct sys_resource *sys_resource_ptr;			\
    ulong psr;								\
									\
    /* check dbg_pinned before accessing d_offset--or page fault */	\
    if (dbg_pinned && d_offset == rsp->ry_port) {			\
	intlev = i_disable(INTMAX);					\
	if (__power_rs()) {						\
		pskd = (ulong *) io_att(IOCC_BID, PSR_ADDRESS);		\
		if ((BUSIO_GETL(pskd) & KEY_POS_MASK) == KEY_POS_SERVICE) { \
	    		debugger(0, DBG_KBD, 0);		        \
	    		io_det(pskd);					\
	    		i_enable(intlev);				\
	    		continue;					\
		}							\
		io_det(pskd);						\
	} else {							\
		sys_resource_ptr = sys_res_att();			\
		psr = sys_resource_ptr->sys_regs.pwr_key_status;	\
		if ((psr & KEY_POS_MASK) == KEY_POS_SERVICE) {		\
	    		debugger(0, DBG_KBD, 0);		        \
	    		i_enable(intlev);				\
	    		continue;					\
		}							\
	}								\
	i_enable(intlev);						\
    }									\
}

/*
 * slih used for native i/o since they are sorta unique and the only thing
 * at interrupt level 2.
 */
local int 
srs_nslih(struct intr *rintr)
{
#define SRS_NSLIH
#ifdef SRS_NSLIH
    DoingPio;
    register struct rs_intr *intr = (struct rs_intr *)rintr;
    register rymp_t *rspp;
    volatile register rymp_t rsp = 0;
    volatile register str_rsp_t tp;
    register int bit_vector;
    register int iir;
    register uchar status, msr_delta, data;
    register struct clist *cl;
    volatile int result;
    register caddr_t io_page;
    register char ints_served = 0;	/* 1 rcv, 2 xmt, 4 mdm */
    register int count, rc, sio2, depth;
    register uchar read_char, char_interesting;
	
    Enter(TRC_RS(TTY_SLIH), 0, rintr, RS_NSLIH, 0, 0);
	
    result = INTR_FAIL;			/* nothing serviced yet */
	
    io_page = io_att(intr->ri_intr.bid, 0);
	
    StartPio(srs_pio_catch, rsp, break);
    for ( ; bit_vector = BUSIO_GETC(io_page + intr->ri_arb) & 0xf; rsp = 0) {
    	for (rspp = intr->ri_rsp;
             bit_vector;		/* until no more bits */
             bit_vector >>= 2, ++rspp) { /* look at next two bits */
            rsp = *rspp;		/* point to this port */
            tp = rsp->ry_tp;
            sio2 = (NativeA_SIO2 == rsp->ry_type||NativeB_SIO2 == rsp->ry_type);
            if (bit_vector&1) {		/* chip interrupt */
                register struct srs_port *port;
                register uchar lsr = 0;
                port = (struct srs_port *)(io_page + rsp->ry_port);

                rc = 0;
                while (!((iir = GR(port->r_iir))&1)) {
                    rsp->ry_trace[rsp->ry_tridx++] = iir;
                    rsp->ry_tridx &= (TRC_SIZE-1);
                    switch (iir&0x0e) {
                      case 2:		/* xmit intr */
                        ints_served |= 2;
                        SR(port->r_ier, GR(port->r_ier)&~ETHREI);
                        rsp->ry_xmit =1;
			rsp->ry_fcnt = rsp->ry_tbc;		/* 156569 */
                        break;

                      case 6:		/* lsr bad news */
                        ints_served |= 1;
                        while (((lsr = GR(port->r_lsr))&DR) && !rc) {
                            (status = lsr&LSR_MASK) ? rsp->ry_bcnt++ : 
                            rsp->ry_icnt++;
                            if ((data = GR(port->r_rbr)) == '\\'-'@') {
                            /* call_dbg does a continue if debugger 
                               called so user doesn't see ctrl-\ */
                                call_dbg;
                            }
			    depth = (tp->t_wptr - tp->t_rptr) & (RBSIZE - 1);
			    if (depth && (depth >= RBSIZE - 2)) {
				rc = -1;
                                srs_err(rsp->ry_tp->t_name, ERRID_TTY_BADINPUT, -1);
			    } else {
		            	*tp->t_wptr = status;
			    	*(tp->t_wptr + 1) = data;
			    	tp->t_wptr += 2;
			    	if (tp->t_wptr >= &tp->t_rbuf[RBSIZE-1])
				   	tp->t_wptr = &tp->t_rbuf[0];
			    }
			    /*
			     * See ::Note 9:: above
			     */
			    if (sio2)
				break;
                        }
                        break;
                      case 4:		/* data only -- fifo full */
                        if (!sio2) {
                            ints_served |= 1;
                            switch (rsp->ry_rtrig&TRIG_MASK) {
                                case TRIG_01: count = 1; break;
                                case TRIG_04: count = 4; break;
                                case TRIG_08: count = 8; break;
                                case TRIG_14: count = 14; break;
                            }
                            rsp->ry_icnt += count;
                            do {
                                if ((read_char = GR(port->r_rbr)) == '\\'-'@') {
                                /* call_dbg does a continue if debugger 
                                    called so user doesn't see ctrl-\ */
                                    call_dbg;
                                }
                                CHECK_XON_PACING;
                                if (char_interesting) {
       	           	            depth = (tp->t_wptr - tp->t_rptr) & (RBSIZE - 1);
			    	    if (depth && (depth >= RBSIZE - 2)) {
					rc = -1;
                                	srs_err(rsp->ry_tp->t_name, ERRID_TTY_BADINPUT, -1);
			    	    } else {
					*tp->t_wptr = 0;
					*(tp->t_wptr + 1) = read_char;
					tp->t_wptr += 2;
					if (tp->t_wptr >= &tp->t_rbuf[RBSIZE-1])
					   	tp->t_wptr = &tp->t_rbuf[0];
			    	    }
			    	}
                            } while (--count && !rc);
                            while (((lsr = GR(port->r_lsr))&DR) && !rc) {
                                (status = lsr&LSR_MASK) ? rsp->ry_bcnt++ : 
                                rsp->ry_icnt++;
                                if ((read_char = GR(port->r_rbr)) == '\\'-'@') {
                                    /* call_dbg does a continue if debugger 
                                        called so user doesn't see ctrl-\ */
                                        call_dbg;
                                }
                                CHECK_XON_PACING;
                                if (char_interesting) {
			    	    depth = (tp->t_wptr - tp->t_rptr) & (RBSIZE - 1);
			    	    if (depth && (depth >= RBSIZE - 2)) {
					rc = -1;
                                	srs_err(rsp->ry_tp->t_name, ERRID_TTY_BADINPUT, -1);
			    	    } else {
					*tp->t_wptr = status;
					*(tp->t_wptr + 1) = read_char;
					tp->t_wptr += 2;
					if (tp->t_wptr >= &tp->t_rbuf[RBSIZE-1])
				   	    tp->t_wptr = &tp->t_rbuf[0];
			    	    }
			    	}
                            }
                            break;
                        }
                        /* else: this chip has a problem with intr 4, so
                         * we do a 12 instead.  See ::Note 5:: at top.
                         */
                      case 12:		/* data only */
                        ints_served |= 1;
                        lsr = 0;	/* ok first time through */
                        do {
                            (status = lsr&LSR_MASK) ? rsp->ry_bcnt++ : 
                            rsp->ry_icnt++;
                            if ((read_char = GR(port->r_rbr)) == '\\'-'@') {
                                /* call_dbg does a continue if debugger 
                                    called so user doesn't see ctrl-\ */
                                    call_dbg;
                            }
                            CHECK_XON_PACING;
                            if (char_interesting) {
			        depth = (tp->t_wptr - tp->t_rptr) & (RBSIZE - 1);
			        if (depth && (depth >= RBSIZE - 2)) {
				    rc = -1;
                                    srs_err(rsp->ry_tp->t_name, ERRID_TTY_BADINPUT, -1);
			        } else {
				    *tp->t_wptr = status;
				    *(tp->t_wptr + 1) = read_char;
				    tp->t_wptr += 2;
				    if (tp->t_wptr >= &tp->t_rbuf[RBSIZE-1])
					   tp->t_wptr = &tp->t_rbuf[0];
			        }
			    }
                        } while (((lsr = GR(port->r_lsr))&DR) && !rc);
                        break;

                      case 0:		/* modem status */
                        ints_served |= 4;
                        status = GR(port->r_msr)|0x01;
                        msr_delta = rsp->ry_msr ^ status;
                        rsp->ry_msr = status;
                        if (msr_delta & CTS) {
                            if (status & CTS) {         /* cts_on */
                                if (tp->t_hardpacing & CTSXON) {
                                    tp->t_flow_state &= ~HARD_REMOTE_STOP;
                                    if (!(tp->t_flow_state & SOFT_REMOTE_STOP)) {
                                        tp->t_stop = 0;
                                        /* offlevel will resume output */
                                        rsp->ry_xmit = 1;
                                    }
                                }
                                REGISTER_NEW_STATUS(cts_on);
                            } else {                   /* cts_off */
                                if (tp->t_hardpacing & CTSXON) {
                                    tp->t_flow_state |= HARD_REMOTE_STOP;
                                    tp->t_stop = 1;
                                }
                                REGISTER_NEW_STATUS(cts_off);
                            }
                            if ((tp->t_open_disc == WTO_OPEN) && (tp->t_openRetrieve)) {
                                openDisc_input(tp->t_openRetrieve, &tp->t_event, 0,
                                               (status & CTS)? cts_on : cts_off);
                            }
                        }
                        if (msr_delta & DCD) {
                            if (status & DCD) {         /* cd_on */
                                if (tp->t_hardpacing & CDXON) {
                                    tp->t_flow_state &= ~HARD_REMOTE_STOP;
                                    if (!(tp->t_flow_state & SOFT_REMOTE_STOP)) {
                                        tp->t_stop = 0;
                                        /* offlevel will resume output */
                                        rsp->ry_xmit = 1;
                                    }
                                } else {  /* flow control information, no connexion */
                                    REGISTER_NEW_STATUS(cd_on);
                                }
                                /* Call open discipline input if DTROPEN sleeping */
                                if (tp->t_open_disc == DTRO_OPEN) {
                                    if ((tp->t_wopen > 1) && (tp->t_openRetrieve)) {
                                        openDisc_input(tp->t_openRetrieve, &tp->t_event,
                                                       0, cd_on);
                                    }
                                } else {
                                    if ((tp->t_open_disc == WTO_OPEN) && (tp->t_openRetrieve)) {
                                        openDisc_input(tp->t_openRetrieve, &tp->t_event, 0, cd_on);
                                    }
                                }
                            } else {                   /* cd_off */
                                if (tp->t_hardpacing & CDXON) {
                                    tp->t_flow_state |= HARD_REMOTE_STOP;
                                    tp->t_stop = 1;
                                } else {  /* flow control information, no disconnect */
                                    REGISTER_NEW_STATUS(cd_off);
                                }
                                if ((tp->t_open_disc == WTO_OPEN) && (tp->t_openRetrieve)) {
                                    openDisc_input(tp->t_openRetrieve, &tp->t_event, 0, cd_off);
                                }
                            }
                        }
                        if (msr_delta & DSR) {
                            REGISTER_NEW_STATUS((status&DSR)? dsr_on : dsr_off);
                            if ((tp->t_open_disc == WTO_OPEN) && (tp->t_openRetrieve)) {
                                openDisc_input(tp->t_openRetrieve, &tp->t_event, 0,
                                               (status & DSR)? dsr_on : dsr_off);
                            }
                        }
                        if (msr_delta & RI) {
                            REGISTER_NEW_STATUS((status&RI)? ri_on : ri_off);
                            if ((tp->t_open_disc == WTO_OPEN) && (tp->t_openRetrieve)) {
                                openDisc_input(tp->t_openRetrieve, &tp->t_event, 0,
                                               (status & RI)? ri_on : ri_off);
                            }
                        }
	                break;
                    }
                   if (rc) {
                        /* if there was an error getting a cblock
                         * shut off intr's for this port
                         * and turn it back on in offlevel ...
                         */
                        SR(port->r_ier, GR(port->r_ier)&~(ERBDAI|ELSI|EMSSI));
                        rsp->ry_ioff = 1;
                        break;
                    }
                }
                if (!rsp->ry_posted) {
                    rsp->ry_posted = 1;
                    i_sched(rsp);
                }
                result = INTR_SUCC;
            }
	    if (bit_vector&2) {		/* DMA tc interrupt */
	        rsp->ry_xmit = 1;
		rsp->ry_fcnt = rsp->ry_tbc;		/* 156569 */
	        if (!rsp->ry_posted) {
	            rsp->ry_posted = 1;
	            i_sched(rsp);
	        }
	        result = INTR_SUCC;
	    }
	}
    }
    EndPio();
	
    io_det(io_page);
	
    if (result == INTR_SUCC)
	  i_reset(intr);
    if (ints_served & 1)
	  ++sysinfo.rcvint;
    if (ints_served & 2)
	  ++sysinfo.xmtint;
    if (ints_served & 4)
	  ++sysinfo.mdmint;
    Return(result);
#endif					/* SRS_NSLIH */
}

/* slih for native i/o in pegasus.
 * No more Serial Interrupt Register or Serial Dma Control on
 * the board. Interruption cause is read directly from the 
 * IIR of the UART.
 */

/*
 * Disable interrupts and save MSR.
 * Check key position.
 * If key is in service position call the debugger.
 * Enable interrupts and restore MSR.
 */
/*
 * Warning!! key_position checking is not done till there is a clear decision
 * for Pegasus. two solutions: read directly pksr_add when it is exported by 
 * kernel or call a get_key_pos() kernel service when it is available.
 */

#define call_dbg_peg							\
{									\
    register int intlev;						\
    register ulong *pskd;						\
									\
    /* check dbg_pinned before accessing d_offset--or page fault */	\
    if (dbg_pinned && d_offset == rsp->ry_port) {			\
	intlev = i_disable(INTMAX);					\
        if (get_key_pos() == KEY_POS_SERVICE) {                         \
    	    debugger(0, DBG_KBD, 0);				        \
    	    i_enable(intlev);						\
    	    continue;							\
        }                                                               \
    	i_enable(intlev);						\
    }									\
}

local int
srs_nslih_peg(struct intr *rintr)
{
#define SRS_NSLIH_PEG
#ifdef SRS_NSLIH_PEG
    DoingPio;
    register struct rs_intr *intr = (struct rs_intr *)rintr;
    register rymp_t *rspp;
    volatile register rymp_t rsp = 0;
    register int bit_vector;
    register int iir;
    register uchar status, msr_delta, data;
    register struct clist *cl;
    volatile int result;
    register caddr_t io_page;
    register char ints_served = 0;    /* 1 rcv, 2 xmt, 4 mdm */
    register int count, rc, depth;
    register int old_intr_hard;
    register str_rsp_t tp;
    register uchar read_char, char_interesting;
	
    Enter(TRC_RS(TTY_SLIH), 0, rintr, RS_8SLIH, 0, 0);
	
    result = INTR_FAIL;            /* nothing serviced yet */
	
    io_page = io_att(intr->ri_intr.bid, 0);
	
    StartPio(srs_pio_catch, rsp, break);
    for (rspp = intr->ri_rsp; (*rspp); ++rspp) {
        register struct srs_port *port;
        register uchar lsr = 0;
        rsp = *rspp;        /* point to this port */
        tp = rsp->ry_tp;
        if (!rsp->ry_open && !(tp->t_wopen)) {
            continue;
        }
        rc = 0;
        port = (struct srs_port *)(io_page + rsp->ry_port);
        iir = GR(port->r_iir);
        if (!(iir & 1)) {
        do {
            rsp->ry_trace[rsp->ry_tridx++] = iir;
            rsp->ry_tridx &= (TRC_SIZE-1);
            switch (iir&0x0e) {
              case 2:        /* xmit intr */
                ints_served |= 2;
                old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);
                SR(port->r_ier, GR(port->r_ier)&~ETHREI);
                rsp->ry_xmit =1;
		rsp->ry_fcnt = rsp->ry_tbc;		/* 156569 */
                unlock_enable(old_intr_hard, &tp->t_lock_hard);
                break;
              case 6:        /* lsr bad news */
                ints_served |= 1;
                while (((lsr = GR(port->r_lsr))&DR) && !rc){
                    (status = lsr&LSR_MASK) ? rsp->ry_bcnt++ :
                    rsp->ry_icnt++;
                    if ((data = GR(port->r_rbr)) == '\\'-'@') {
                    /* call_dbg_peg does a continue if debugger
                        called so user doesn't see ctrl-\ */
                        call_dbg_peg;
                    }
		    depth = (tp->t_wptr - tp->t_rptr) & (RBSIZE - 1);
		    if (depth && (depth >= RBSIZE - 2)) {
			    rc = -1;
                            srs_err(rsp->ry_tp->t_name, ERRID_TTY_BADINPUT, -1);
		    } else {
			*tp->t_wptr = status;
			*(tp->t_wptr + 1) = data;
			tp->t_wptr += 2;
			if (tp->t_wptr >= &tp->t_rbuf[RBSIZE-1])
			   tp->t_wptr = &tp->t_rbuf[0];
		    }
                    }
                break;
              case 4:        /* data only -- fifo full */
                ints_served |= 1;
                switch (rsp->ry_rtrig&TRIG_MASK) {
                    case TRIG_01: count = 1; break;
                    case TRIG_04: count = 4; break;
                    case TRIG_08: count = 8; break;
                    case TRIG_14: count = 14; break;
                }
                rsp->ry_icnt += count;
                do {
                    if ((read_char = GR(port->r_rbr)) == '\\'-'@') {
                        /* call_dbg_peg does a continue if debugger
                            called so user doesn't see ctrl-\ */
                        call_dbg_peg;
                    }
                    CHECK_XON_PACING;
                    if (char_interesting) {
		        depth = (tp->t_wptr - tp->t_rptr) & (RBSIZE - 1);
		        if (depth && (depth >= RBSIZE - 2)) {
			    rc = -1;
                            srs_err(rsp->ry_tp->t_name, ERRID_TTY_BADINPUT, -1);
		        } else {
			    *tp->t_wptr = 0;
			    *(tp->t_wptr + 1) = read_char;
			    tp->t_wptr += 2;
			    if (tp->t_wptr >= &tp->t_rbuf[RBSIZE-1])
			       tp->t_wptr = &tp->t_rbuf[0];
		        }
                    }
                } while (--count && !rc);
                while (((lsr = GR(port->r_lsr))&DR) && !rc){
                    (status = lsr&LSR_MASK) ? rsp->ry_bcnt++ :
                    rsp->ry_icnt++;
                    if ((read_char = GR(port->r_rbr)) == '\\'-'@') {
                    /* call_dbg_peg does a continue if debugger
                        called so user doesn't see ctrl-\ */
                        call_dbg_peg;
                    }
                    CHECK_XON_PACING;
                    if (char_interesting) {
		        depth = (tp->t_wptr - tp->t_rptr) & (RBSIZE - 1);
		        if (depth && (depth >= RBSIZE - 2)) {
			    rc = -1;
                            srs_err(rsp->ry_tp->t_name, ERRID_TTY_BADINPUT, -1);
		        } else {
			    *tp->t_wptr = status;
			    *(tp->t_wptr + 1) = read_char;
			    tp->t_wptr += 2;
			    if (tp->t_wptr >= &tp->t_rbuf[RBSIZE-1])
		 	        tp->t_wptr = &tp->t_rbuf[0];
		        }
                    }
                }
                break;
              case 12:        /* data only */
                ints_served |= 1;
                lsr = 0;    /* ok first time through */
                do {
                    (status = lsr&LSR_MASK) ? rsp->ry_bcnt++ :
                    rsp->ry_icnt++;
                    if ((read_char = GR(port->r_rbr)) == '\\'-'@') {
                        /* call_dbg_peg does a continue if debugger
                        called so user doesn't see ctrl-\ */
                        call_dbg_peg;
                    }
                    CHECK_XON_PACING;
                    if (char_interesting) {
		        depth = (tp->t_wptr - tp->t_rptr) & (RBSIZE - 1);
		        if (depth && (depth >= RBSIZE - 2)) {
			    rc = -1;
                            srs_err(rsp->ry_tp->t_name, ERRID_TTY_BADINPUT, -1);
		        } else {
			    *tp->t_wptr = status;
			    *(tp->t_wptr + 1) = read_char;
			    tp->t_wptr += 2;
			    if (tp->t_wptr >= &tp->t_rbuf[RBSIZE-1])
		 	        tp->t_wptr = &tp->t_rbuf[0];
		        }
		    }
                } while (((lsr = GR(port->r_lsr))&DR) && !rc);
                break;
              case 0:        /* modem status */
                ints_served |= 4;
                status = GR(port->r_msr)|0x01;
                msr_delta = rsp->ry_msr ^ status;
                rsp->ry_msr = status;
                if (msr_delta & CTS) {
                    if (status & CTS) {         /* cts_on */
                        if (tp->t_hardpacing & CTSXON) {
                            tp->t_flow_state &= ~HARD_REMOTE_STOP;
                            if (!(tp->t_flow_state & SOFT_REMOTE_STOP)) {
                                tp->t_stop = 0;
                                /* offlevel will resume output */
                                rsp->ry_xmit = 1;
                            }
                        }
                        REGISTER_NEW_STATUS(cts_on);
                    } else {                   /* cts_off */
                        if (tp->t_hardpacing & CTSXON) {
                            tp->t_flow_state |= HARD_REMOTE_STOP;
                            tp->t_stop = 1;
                        }
                        REGISTER_NEW_STATUS(cts_off);
                    }
                    if ((tp->t_open_disc == WTO_OPEN) && (tp->t_openRetrieve)) {
                        openDisc_input(tp->t_openRetrieve, &tp->t_event, 0,
                                       (status & CTS)? cts_on : cts_off);
                    }
                }
                if (msr_delta & DCD) {
                    if (status & DCD) {         /* cd_on */
                        if (tp->t_hardpacing & CDXON) {
                            tp->t_flow_state &= ~HARD_REMOTE_STOP;
                            if (!(tp->t_flow_state & SOFT_REMOTE_STOP)) {
                                tp->t_stop = 0;
                                /* offlevel will resume output */
                                rsp->ry_xmit = 1;
                            }
                        } else {
                            REGISTER_NEW_STATUS(cd_on);
                        }
                        /* Call open discipline input if DTROPEN sleeping */
                        if (tp->t_open_disc == DTRO_OPEN) {
                            if ((tp->t_wopen > 1) && (tp->t_openRetrieve)) {
                                openDisc_input(tp->t_openRetrieve, &tp->t_event, 0, cd_on);
                            }
                        } else {
                            if ((tp->t_open_disc == WTO_OPEN) && (tp->t_openRetrieve)) {
                                openDisc_input(tp->t_openRetrieve, &tp->t_event, 0, cd_on);
                            }
                        }
                    } else {                   /* cd_off */
                        if (tp->t_hardpacing & CDXON) {
                            tp->t_flow_state |= HARD_REMOTE_STOP;
                            tp->t_stop = 1;
                        } else {
                            REGISTER_NEW_STATUS(cd_off);
                        }
                        if ((tp->t_open_disc == WTO_OPEN) && (tp->t_openRetrieve)) {
                            openDisc_input(tp->t_openRetrieve, &tp->t_event, 0, cd_off);
                        }
                    }
                }
                if (msr_delta & DSR) {
                    REGISTER_NEW_STATUS((status&DSR)? dsr_on : dsr_off);
                    if ((tp->t_open_disc == WTO_OPEN) && (tp->t_openRetrieve)) {
                        openDisc_input(tp->t_openRetrieve, &tp->t_event, 0,
                                       (status & DSR)? dsr_on : dsr_off);
                    }
                }
                if (msr_delta & RI) {
                    REGISTER_NEW_STATUS((status&RI)? ri_on : ri_off);
                    if ((tp->t_open_disc == WTO_OPEN) && (tp->t_openRetrieve)) {
                        openDisc_input(tp->t_openRetrieve, &tp->t_event, 0,
                                       (status & RI)? ri_on : ri_off);
                    }
                }
                break;
            }
            if (rc) {
                 /* if there was an error getting a cblock
                  * shut off intr's for this port
                  * and turn it back on in offlevel ...
                  */
                 old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);
                 SR(port->r_ier, GR(port->r_ier)&~(ERBDAI|ELSI|EMSSI));
                 rsp->ry_ioff = 1;
                 unlock_enable(old_intr_hard, &tp->t_lock_hard);
                 break;
            }
        } while (!((iir = GR(port->r_iir))&1));
        old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);
        if (!rsp->ry_posted) {
            rsp->ry_posted = 1;
            i_sched(rsp);
        }
        unlock_enable(old_intr_hard, &tp->t_lock_hard);
        result = INTR_SUCC;
        } /* End of (!((iir = GR(port->r_iir))&1)) */
    } /* End of for(...) */
    EndPio();
	
    io_det(io_page);
	
    if (result == INTR_SUCC)
	  i_reset(intr);
    if (ints_served & 1)
       fetch_and_add(&(sysinfo.rcvint), 1);
    if (ints_served & 2)
       fetch_and_add(&(sysinfo.xmtint), 1);
    if (ints_served & 4)
       fetch_and_add(&(sysinfo.mdmint), 1);
    Return(result);
#endif                    /* SRS_NSLIH _PEG*/
}

/*
 * slih for 8 ports (hope will work for 16 ports as well).  We must
 * remember the following problem.  When we are called at a particular
 * level, we may service cards at a different level.  Indeed we may
 * service some cards but no cards at the level we are called at.  We
 * must not return with a happy status unless we service an interrupt
 * at the level we are called for or folks down the line of slihs will
 * never get called.
 */
#define RSFLIPPED
#ifdef RSFLIPPED            /* flipped data */
#define IVALID 0x8000
#define IMASK  0x007f
#define ISHIFT 0
#define LSHIFT 8
#else                    /* normal data */
#define IVALID 0x0080
#define IMASK  0x007f
#define ISHIFT 8
#define LSHIFT 0
#endif
local int 
srs_8slih(struct intr *rintr)
{
#define SRS_8SLIH
#ifdef SRS_8SLIH
    DoingPio;
    register struct rs_intr *intr = (struct rs_intr *)rintr;
    register rymp_t *rspp = intr->ri_rsp;
    volatile register rymp_t rsp = 0;
    volatile register str_rsp_t tp;
    register int iir;
    register uchar status, msr_delta, data;
    register struct clist *cl;
    volatile int result = INTR_FAIL;    /* nothing serviced yet */
    register ushort magic;
    register caddr_t io_page;
    register uchar lsr;
    register char ints_served = 0;    /* 1 rcv, 2 xmt, 4 mdm */
    register int count, rc, depth;
    register int old_intr_hard;
    register uchar read_char, char_interesting;
	
    Enter(TRC_RS(TTY_SLIH), 0, rintr, 0, 0, 0);
	
    io_page = io_att(intr->ri_intr.bid, 0);
    srsprintf("In 8slih\n");
    StartPio(srs_pio_catch, rsp, break);
    for ( ; !((magic = BUSIO_GETS(io_page + intr->ri_arb))&IVALID); rsp = 0) {
	register struct srs_port *port;
		
	iir = magic>>ISHIFT;
        srsprintf("iir is %x ; magic is %x\n",iir,magic);
		
	if (!(rsp = rspp[(magic>>LSHIFT)&IMASK])) {
		srs_err(sresource, ERRID_RS_8_16_ARB, magic);
		continue;
	}
	port = (struct srs_port *)(io_page + rsp->ry_port);
	rsp->ry_trace[rsp->ry_tridx++] = iir;
	rsp->ry_tridx &= (TRC_SIZE-1);
        tp = rsp->ry_tp;
        rc = 0;
	switch (iir&0x0e) {
	  case 2:                /* xmit intr */
	    srsprintf("In Tx intr \n");
	    ints_served |= 2;
            old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);
	    SR(port->r_ier, GR(port->r_ier)&~ETHREI);
	    /* No timer handling as of now
	    if (!(rsp->ry_timer->flags & T_ACTIVE)) {
	    rsp->ry_timer->timeout.it_value.tv_sec =
	    12 / rsp->ry_tp->t_ospeed;
	    rsp->ry_timer->timeout.it_value.tv_nsec =
	    12000000 / rsp->ry_tp->t_ospeed * 1000;
	    tstart(rsp->ry_timer);
	    }
	    */
	    rsp->ry_xmit =1;
	    rsp->ry_fcnt = rsp->ry_tbc;			/* 156569 */
            unlock_enable(old_intr_hard, &tp->t_lock_hard);
	    srsprintf("Disabled TX interrupt;\n");
	    break;
          case 6:                /* lsr bad news */
	    ints_served |= 1; 
            while (((lsr = GR(port->r_lsr))&DR) && !rc) {
		 (status = lsr&LSR_MASK) ? rsp->ry_bcnt++ : rsp->ry_icnt++;
		 depth = (tp->t_wptr - tp->t_rptr) & (RBSIZE - 1);
		 if (depth && (depth >= RBSIZE - 2)) {
			rc = -1;
                        srs_err(rsp->ry_tp->t_name, ERRID_TTY_BADINPUT, -1);
		 } else {
			*tp->t_wptr = status;
			*(tp->t_wptr + 1) = GR(port->r_rbr);
			tp->t_wptr += 2;
			if (tp->t_wptr >= &tp->t_rbuf[RBSIZE-1])
			   tp->t_wptr = &tp->t_rbuf[0];
		 }
	    }
	    break;
         case 4:                /* data only -- fifo full */
	   ints_served |= 1;
           switch (rsp->ry_rtrig&TRIG_MASK) {
	     case TRIG_01: count = 1; break;
	     case TRIG_04: count = 4; break;
	     case TRIG_08: count = 8; break;
	     case TRIG_14: count = 14; break;
	   }
	   rsp->ry_icnt += count;
	   do {
                read_char = GR(port->r_rbr);
                CHECK_XON_PACING;
                if (char_interesting) {
	    	    depth = (tp->t_wptr - tp->t_rptr) & (RBSIZE - 1);
	    	    if (depth && (depth >= RBSIZE - 2)) {
			    rc = -1;
                            srs_err(rsp->ry_tp->t_name, ERRID_TTY_BADINPUT, -1);
	            } else {
			    *tp->t_wptr = 0;
			    *(tp->t_wptr + 1) = read_char;
			    tp->t_wptr += 2;
			    if (tp->t_wptr >= &tp->t_rbuf[RBSIZE-1])
		   	       tp->t_wptr = &tp->t_rbuf[0];
	            }
                }
            } while (--count && !rc);
            while (((lsr = GR(port->r_lsr))&DR) && !rc) {
	        (status = lsr&LSR_MASK) ? rsp->ry_bcnt++ : rsp->ry_icnt++;
                read_char = GR(port->r_rbr);
                CHECK_XON_PACING;
                if (char_interesting) {
	            depth = (tp->t_wptr - tp->t_rptr) & (RBSIZE - 1);
	            if (depth && (depth >= RBSIZE - 2)) {
			rc = -1;
                        srs_err(rsp->ry_tp->t_name, ERRID_TTY_BADINPUT, -1);
	            } else {
			*tp->t_wptr = status;
			*(tp->t_wptr + 1) = read_char;
			tp->t_wptr += 2;
			if (tp->t_wptr >= &tp->t_rbuf[RBSIZE-1])
			   tp->t_wptr = &tp->t_rbuf[0];
	           }
               }
	   }
	   break;
         case 12:            /* data only */
	   ints_served |= 1;
	   lsr = 0;            /* ok first time through */
	   do {
	       (status = lsr&LSR_MASK) ? rsp->ry_bcnt++ : rsp->ry_icnt++;
               read_char = GR(port->r_rbr);
               CHECK_XON_PACING;
               if (char_interesting) {
	           depth = (tp->t_wptr - tp->t_rptr) & (RBSIZE - 1);
	           if (depth && (depth >= RBSIZE - 2)) {
			rc = -1;
                        srs_err(rsp->ry_tp->t_name, ERRID_TTY_BADINPUT, -1);
	           } else {
			*tp->t_wptr = status;
			*(tp->t_wptr + 1) = read_char;
			tp->t_wptr += 2;
			if (tp->t_wptr >= &tp->t_rbuf[RBSIZE-1])
			   tp->t_wptr = &tp->t_rbuf[0];
	           }
               }
           } while (((lsr = GR(port->r_lsr))&DR) && !rc);
	   break;
         case 0:                /* modem status */
	   ints_served |= 4;
	   status = GR(port->r_msr)|0x01;
           msr_delta = rsp->ry_msr ^ status;
           rsp->ry_msr = status;
           if (msr_delta & CTS) {
               if (status & CTS) {         /* cts_on */
                   if (tp->t_hardpacing & CTSXON) {
                       tp->t_flow_state &= ~HARD_REMOTE_STOP;
                       if (!(tp->t_flow_state & SOFT_REMOTE_STOP)) {
                           tp->t_stop = 0;
                           /* offlevel will resume output */
                           rsp->ry_xmit = 1;
                       }
                   }
                   REGISTER_NEW_STATUS(cts_on);
               } else {                   /* cts_off */
                   if (tp->t_hardpacing & CTSXON) {
                       tp->t_flow_state |= HARD_REMOTE_STOP;
                       tp->t_stop = 1;
                   }
                   REGISTER_NEW_STATUS(cts_off);
               }
               if ((tp->t_open_disc == WTO_OPEN) && (tp->t_openRetrieve)) {
                   openDisc_input(tp->t_openRetrieve, &tp->t_event, 0,
                                  (status & CTS)? cts_on : cts_off);
               }
           }
           if (msr_delta & DCD) {
               if (status & DCD) {         /* cd_on */
                   if (tp->t_hardpacing & CDXON) {
                       tp->t_flow_state &= ~HARD_REMOTE_STOP;
                       if (!(tp->t_flow_state & SOFT_REMOTE_STOP)) {
                           tp->t_stop = 0;
                           /* offlevel will resume output */
                           rsp->ry_xmit = 1;
                       }
                   } else {
                       REGISTER_NEW_STATUS(cd_on);
                   }
                   /* Call open discipline input if DTROPEN sleeping */
                   if (tp->t_open_disc == DTRO_OPEN) {
                       if ((tp->t_wopen > 1) && (tp->t_openRetrieve)) {
                           openDisc_input(tp->t_openRetrieve, &tp->t_event, 0, cd_on);
                       }
                   } else {
                       if ((tp->t_open_disc == WTO_OPEN) && (tp->t_openRetrieve)) {
                           openDisc_input(tp->t_openRetrieve, &tp->t_event, 0, cd_on);
                       }
                   }
               } else {                   /* cd_off */
                   if (tp->t_hardpacing & CDXON) {
                       tp->t_flow_state |= HARD_REMOTE_STOP;
                       tp->t_stop = 1;
                   } else {
                       REGISTER_NEW_STATUS(cd_off);
                   }
                   if ((tp->t_open_disc == WTO_OPEN) && (tp->t_openRetrieve)) {
                       openDisc_input(tp->t_openRetrieve, &tp->t_event, 0, cd_off);
                   }
               }
           }
           if (msr_delta & DSR) {
               REGISTER_NEW_STATUS((status&DSR)? dsr_on : dsr_off);
               if ((tp->t_open_disc == WTO_OPEN) && (tp->t_openRetrieve)) {
                   openDisc_input(tp->t_openRetrieve, &tp->t_event, 0,
                                  (status & DSR)? dsr_on : dsr_off);
               }
           }
           if (msr_delta & RI) {
               REGISTER_NEW_STATUS((status&RI)? ri_on : ri_off);
               if ((tp->t_open_disc == WTO_OPEN) && (tp->t_openRetrieve)) {
                   openDisc_input(tp->t_openRetrieve, &tp->t_event, 0,
                                  (status & RI)? ri_on : ri_off);
               }
           }
	   break;
       }
	  	
       if (!rsp->ry_posted) {
	   rsp->ry_posted = 1;
	   srsprintf("The offlevel scheduled\n");
	   i_sched(rsp);
       }
       if (rsp->ry_level == intr->ri_intr.level)
	   result = INTR_SUCC;
       if (rc) {
           /* if there was an error getting a cblock
            * shut off intr's for this port
            * and turn it back on in offlevel ...
            */
           old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);
           SR(port->r_ier, GR(port->r_ier)&~(ERBDAI|ELSI|EMSSI));
           rsp->ry_ioff = 1;
           unlock_enable(old_intr_hard, &tp->t_lock_hard);
       }
    }
    EndPio();
	
    io_det(io_page);
	
    if (result == INTR_SUCC || magic == 0xffff)
	  i_reset(intr);
    if (ints_served & 1)
       fetch_and_add(&(sysinfo.rcvint), 1);
    if (ints_served & 2)
       fetch_and_add(&(sysinfo.xmtint), 1);
    if (ints_served & 4)
       fetch_and_add(&(sysinfo.mdmint), 1);
    Return(result);
#endif                    /* SRS_8SLIH */
}
#endif /* _POWER */

#ifndef DB_TTY

/*
void
srs_print_help()
{
    printf("\nUsage:\tcall srs_kdb v/x @(str_rs struct)\n");
}
*/

/*
 * srs_kdb
 *    Print function for kdb debugger.
 *      Converts the string to parameter list,
 *      Prints some local information,
 *      Calls the lldb print function srs_print.
 *
 * ARGUMENTS:
 *    buf    : kdb sub-command line.
 *    poff    : index in buf string.
 *    len    : length of buf.
 *    
 */
/*
void
srs_kdb(unsigned char *buf, int *poff, int len)

{
    int ch;
    long tp;
    int v;
	
    /*
     * Read all blanks between function name and first parameter
     *
    while ((len - *poff) > 0 && (buf[*poff] == ' ')) (*poff)++;
    ch = buf[*poff] & 0xFF;
    (*poff)++;
    
    /*
     * Remove all blanks between first and second parameters
     *
    while ((len - *poff) > 0 && (buf[*poff] == ' ')) (*poff)++;
    if ((len - *poff) <= 0) {
        srs_print_help();
        return;
    }
    v = (ch == 'v');
    tp = mi_strtol (buf + *poff, NULL, 16);
    srs_print((str_rsp_t) tp, v);
    return;
}

local int 
srs_print(str_rsp_t tp, int v)
{
#define SRS_PRINT
#ifdef SRS_PRINT
    DoingPio;
    static char *mdm_status[8] = {
		" delta CTS",
		" delta DSR",
		" Ring Fell",
		" delta CD",
		" CTS",
		" DSR",
		" RI",
		" DCD",
    };
    rymp_t rsp = tp->t_hptr;
    int old_lcr;
    struct srs_port *port;
    struct adap_info *adap;
    char l_rbr, l_iir, l_lsr, l_msr, l_ier;
	
    for (adap = sadap_infos;
		 adap->ra_type != Unknown_Adap && adap->ra_type != rsp->ry_type;
		 ++adap);
    printf("%s adapter in slot %d at slih level %d\n", adap->ra_name,
		   rsp->ry_slot, rsp->ry_level);
    printf("seg:0x%08x port:0x%08x iseg:0x%08x ibase:0x%08x\n",
		   rsp->ry_intr.bid, rsp->ry_port, rsp->ry_iseg, rsp->ry_ibase);
	
    port = Port(rsp);
    StartPio(srs_pio_catch, rsp, io_det(port); return 0);
    old_lcr = BUSIO_GETC(&port->r_lcr);
    BUSIO_PUTC(&port->r_lcr, old_lcr&~DLAB);
    if (v) {                /* if verbose *
		l_iir = BUSIO_GETC(&port->r_iir);
		l_rbr = BUSIO_GETC(&port->r_rbr);
		l_lsr = BUSIO_GETC(&port->r_lsr);
		l_msr = BUSIO_GETC(&port->r_msr);
    }
	
    {                    /* Interrupt Enable *
		static char *temp[8] = {
			" Rx Data",
			" Tx Hold Empty",
			" Rx Line Status",
			" Modem Status",
		};
		if (l_ier = BUSIO_GETC(&port->r_ier)) {
			printf("Interrupts enabled for:");
			srs_bits(l_ier, temp);
		} else
		  printf("No interrupts enabled");
    }
	
    {                    /* Line status *
		static char *par[4] = {
			"odd",
			"even",
			"mark",
			"space",
		};
		
		printf("\n%d data bits, %s, %s parity, %s set, dlab %s",
			   5+(old_lcr&(WLS0|WLS1)),
			   old_lcr&STB ? "2 stop bits" : "1 stop bit",
			   old_lcr&PEN ? par[(old_lcr>>4)&3] : "no",
			   old_lcr&BREAK ? "break" : "no break",
			   old_lcr&DLAB ? "set" : "clear");
    }
	
    {                    /* modem control *
		static char *temp[8] = {
			" DTR",
			" RTS",
			" Out1",
			" Out2",
			" Loop",
		};
		printf("\nmodem control bits:");
		srs_bits(BUSIO_GETC(&port->r_mcr), temp);
    }
    printf("\nscratch register = 0x%2x", BUSIO_GETC(&port->r_scr));
	
    {
		static char *temp[8] = {
			" Concurrent Writes",
			" BAUDOUT selected",
			" RXRD selected",
		};
		uchar dlm, dll, afr;
		
		SR(port->r_lcr, old_lcr|DLAB);
		dlm = GR(port->r_dlm);
		dll = GR(port->r_dll);
		afr = GR(port->r_afr);
		SR(port->r_lcr, old_lcr&~DLAB);
		printf("\nDivisor = 0x%02x%02x => ", dlm, dll);
		if (dlm || dll) {
			int temp = (rsp->ry_xtal * 100) / 16 / (dlm * 256 + dll);
			printf("%d.%02d baud", temp / 100, temp % 100);
		} else
		  printf("stopped\n");
		srs_bits(afr, temp);
    }
	
    if (v) {
		static char *i_source[8] = {
			"modem status",
			"transmit empty",
			"receive data",
			"receive line status",
			0,
			0,
			"char timeout",
			0,
		};
		
		printf("\nReceive char = %x, %s pending. fifo's %s",
			   l_rbr,
			   l_iir & 1 ? "no interrupt" : i_source[(l_iir>>1)&7],
			   l_iir & 0x80 ? "enabled" : "disabled");
		{
			printf("\nModem Status:");
			srs_bits(l_msr, mdm_status);
		}
		{
			static char *temp[8] = {
				" data ready",
				" overrun",
				" parity error",
				" framing error",
				" break",
				" xmit hold empty",
				" xmit empty",
				" error in fifo",
			};
			printf("\nLine status:");
			srs_bits(l_lsr, temp);
		}
    }
    printf("\n");
	
    /*dbg_clist("slih queue:", &rsp->ry_data); *
    printf("%d bytes for output at 0x%x\n", rsp->ry_xcnt, rsp->ry_xdata);
    printf("xtal:%d rtrig:0x%x tbc:%d posted:%d xmit:%d \n",
		   rsp->ry_xtal, rsp->ry_rtrig, rsp->ry_tbc, rsp->ry_posted,
		   rsp->ry_xmit);
	
    printf("Last Modem Status:");
    srs_bits(rsp->ry_msr, mdm_status);
	
    printf("\nflags:");
    if (rsp->ry_open)
	  printf(" open");
    if (rsp->ry_watch)
	  printf(" watch");
    if (rsp->ry_mil)
	  printf(" mil");
    if (rsp->ry_xbox)
	  printf(" xbox");
    if (rsp->ry_conf)
	  printf(" conf");
    if (rsp->ry_dsleep)
	  printf(" dsleep");
    {
		int i = rsp->ry_tridx;
		int j = 0;
		
		printf("\nlast %d iir's:", TRC_SIZE);
		do {
			if (--i < 0)
			  i += TRC_SIZE;
			printf(" %02x", rsp->ry_trace[i]);
		} while (i != rsp->ry_tridx);
    }
    printf("\n");
	
    SR(port->r_lcr, old_lcr);        /* restore dlab *
    EndPio();
    io_det(port);
	
    printf("ocnt=%d icnt=%d bcnt=%d",
		   rsp->ry_ocnt, rsp->ry_icnt, rsp->ry_bcnt);
    if (rsp->ry_watched)
	  printf(" watched=%d", rsp->ry_watched);
    printf("\n");
	
    return 0;
}

local void 
srs_bits(int f, char **p)
{
    int none = 1;
	
    for (f &= 0xff ; f; f >>= 1, ++p)
	  if (f & 1 && *p) {
		  if (none)
			printf("%s", *p);
		  else
			printf(",%s", *p);
		  none = 0;
	  }
 #endif                    /* SRS_PRINT *
}
*/
#endif                    /* DB_TTY */

/*
 #ifdef TTYDBG
int
srs_print_empty(str_rsp_t tp)
{
    printf("Sorry, not yet available. Use srs_kdb under KDB\n");
}
 #endif	/* TTYDBG */

#ifdef DOWATCH
local unsigned char *bit_vector;
local int bit_vector_size;

local void 
srs_set_bit(int which)
{
    int index = which >> 3;
    int bit = 1 << (which & 7);
    unsigned char *new_vector, *old_vector;
    int new_size;
	
    if (index >= bit_vector_size) {
    	/* grow in powers of 2 until we are big enough */
    	for (new_size = bit_vector_size ? (bit_vector_size << 1) : 8;
       	     index >= new_size;
    	     new_size <<= 1);
    	
    	if (!(new_vector = xmalloc(new_size, 1, pinned_heap)))
    	    return;
    	bzero(new_vector, new_size);
    	if (old_vector = bit_vector)
    	    bcopy(bit_vector, new_vector, bit_vector_size);
    	bit_vector = new_vector;
    	bit_vector_size = new_size;
    	if (old_vector)
    	    xmfree(old_vector, pinned_heap);
    }
    bit_vector[index] |= bit;
}

local void 
srs_clr_bit(int which)
{
    int index = which >> 3;
    int bit = 1 << (which & 7);
	
    if (index < bit_vector_size)
	  bit_vector[index] &= ~bit;
}

local void 
srs_dowatch(int i, int which)
{
    static use_count;
    static struct trb *ticker;
	
    if (i) {                /* start timer */
	srs_set_bit(which);
	if (!use_count++) {
		ticker = talloc();
		ticker->ipri = smin_priority;
		ticker->flags = 0;
		ticker->func_data = 0;
		ticker->func = srs_watchdog;
		ticker->timeout.it_value.tv_sec = 5;
		ticker->timeout.it_value.tv_nsec = 0;
		tstart(ticker);
	}
    } else {                /* stop timer */
	if (!--use_count) {
		tstop(ticker);
		tfree(ticker);
	}
	srs_clr_bit(which);
    }
}

/* Plop into this puppy once a second and find any dead ports */
local void 
srs_watchdog(struct trb *ticker)
{
    DoingPio;
    register str_rsp_t tp;
    register rymp_t rsp;
    register unsigned char *s, *e, c;
    register int idx1, idx2;
    int hi_pri;
	
    idx1 = 0;
    for (e = (s = bit_vector) + bit_vector_size; s < e; idx1 += 8)
        for (idx2 = idx1, c = *s++; c; c >>= 1, ++idx2)
            if (c & 1) {        /* if pinned */
		qdev2tp(idx2, tp);    /* find tp */
		if (tp && tp->t_isopen && tp->t_busy &&
			(rsp = tp->t_hptr) && !rsp->ry_dma)
		  if (rsp->ry_watch) {
			  register struct srs_port *port;
				  
			  hi_pri = disable_lock(smin_priority, &tp->t_lock_hard);
			  rsp->ry_watched++;
			  rsp->ry_xmit = 1; /* force off level */
			  port = Port(rsp);
			  StartPio(srs_pio_catch, rsp, break);
			  SR(port->r_ier, GR(port->r_ier)&~ETHREI);
			  EndPio();
			  io_det(port);
			  if (!rsp->ry_posted) {
				  rsp->ry_posted = 1;
				  i_sched(rsp);
			  }
			  unlock_enable(hi_pri, &tp->t_lock_hard);
		  } else
			rsp->ry_watch = 1;
            }

    ticker->timeout.it_value.tv_sec = 5;
    ticker->timeout.it_value.tv_nsec = 0;
    tstart(ticker);
}
#endif

#ifdef _POWER
local int 
srs_xboxopen(dev_t parent)
{
    int ret, err = 0;
	
    ret = lockl(&sxbox_lock, LOCK_SHORT);
    if (sxbox_opencnt++ > 0) {
		if (ret == LOCK_SUCC)
		  unlockl(&sxbox_lock);
		return err;
    }
	
    srsprintf("fp_opendev\n");
    if (err = fp_opendev(parent, DREAD|DWRITE|DKERNEL, 0, 0, &srs_xbox)) {
	sxbox_opencnt = 0;
	if (ret == LOCK_SUCC)
	    unlockl(&sxbox_lock);
	srsprintf("fp_opendev returned %d\n", err);
	return err;
    }
	
    if (ret == LOCK_SUCC)
	  unlockl(&sxbox_lock);
	
    if (!sxbox_assist &&
	(err = fp_ioctl(srs_xbox, EU_ASSIST, &sxbox_assist, 0))) {
	srs_xboxclose();
	return err;
    }
    return err;
}

local int 
srs_xboxclose()
{
    int ret, err = 0;
	
    ret = lockl(&sxbox_lock, LOCK_SHORT);
    if (--sxbox_opencnt > 0) {
	if (ret == LOCK_SUCC)
	    unlockl(&sxbox_lock);
	return err;
    }
    /* don't want srs_xboxopen to see a negative count */
    sxbox_opencnt = 0;
	
    srsprintf("fp_close\n");
    /* make sure we have an srs_xbox in case too many closes were called */
    if (srs_xbox) {
	err = fp_close(srs_xbox);
	srs_xbox = 0;
    }
	
    if (ret == LOCK_SUCC)
	  unlockl(&sxbox_lock);
	
    return err;
}

local int 
srs_setpos(uchar where, uchar which, uchar how)
{
    struct euposcb arg;
    int err;
	
    arg.slot = where;
    arg.reg = which;
    arg.value = how;
    srsprintf("setpos %d to %x\n", which, how);
    return fp_ioctl(srs_xbox, EU_SETPOS, &arg, 0);
}

local int 
srs_getpos(uchar where, uchar which, uchar *value)
{
    struct euposcb arg;
    int err;
	
    arg.slot = where;
    arg.reg = which;
    srsprintf("getpos %d ", which);
    err = fp_ioctl(srs_xbox, EU_GETPOS, &arg, 0);
    *value = arg.value;
    srsprintf("value = %x, err = %d\n", arg.value, err);
    return err;
}

#endif /* _POWER */

/* All RS_CFG errors go through here */
local void 
srs_err(char *name, int code, int err)
{
    ERR_REC(sizeof(int)) cfg_err;
    
    cfg_err.error_id = code;
    bcopy(name, cfg_err.resource_name, sizeof(cfg_err.resource_name));
    *(int *)cfg_err.detail_data = err;
    errsave(&cfg_err, sizeof(cfg_err));
}

#ifdef DO_PIO

/* 
 * logit is called with 0 on the times the I/O will be retried.  In 
 * this case, the pud structure is filled but no log entry is made.  
 * If retry count goes to 0, then logit is 1 in which case, the pud 
 * structure is filled and and a log entry is made.  In the case of a 
 * temporary error, the routine is called again with logit equal to 2 
 * in which case, the pud structure (which was filled in by a call 
 * with logit equal to 0) is used to make a log entry.
 */
local void 
srs_pio_catch(rymp_t rsp, pud_t *pud, int logit)
{
    ERR_REC(sizeof(struct pud)) cfg_err;
    char *pos_ptr;

    /* save data if we have to clean up the bus   */
    getexcept(&pud->p_except);
    pud->p_check = 0;
    pud->p_mask = 0;
	
    /* First lets clean up the bus, etc */
    if (rsp && logit != 2) {        /* not the temp error log call */
	if (!rsp->ry_xbox) {        /* not in X box */
		/* 
		 * Muck about with pos register 5 -- we do this 
		 * unprotected right now.
		 */
		pos_ptr = io_att(rsp->ry_iseg, rsp->ry_ibase + 5);
		pud->p_check = BUSIO_GETC(pos_ptr);
		BUSIO_PUTC(pos_ptr, pud->p_check | 0x80);
		io_det(pos_ptr);
	} else if (sxbox_assist)
	  (*sxbox_assist)(&pud->p_mask);
    }
	
    if (logit) {
	cfg_err.error_id = (logit == 1) ? ERRID_COM_PERM_PIO :
	  ERRID_COM_TEMP_PIO;
	bcopy(rsp ? rsp->ry_tp->t_name : sresource, cfg_err.resource_name,
			  sizeof(cfg_err.resource_name));
	*(pud_t *)cfg_err.detail_data = *pud;
	errsave(&cfg_err, sizeof(cfg_err));
    }
}
#endif /* DO_PIO */

#ifdef _POWER
local int 
srs_getvpd(rymp_t rsp, struct uio *uio, dev_t parent)
{
    struct euvpdcb arg;
    int err;

    if (!(err = srs_xboxopen(parent))) {
    arg.slot = rsp->ry_slot;
    arg.uio = uio;
    err = fp_ioctl(srs_xbox, EU_SLOTVPD, &arg, 0);
    }
    if (!err) 
    err = srs_xboxclose();
    else
    srs_xboxclose();
    return(err);
}
#endif /* _POWER */

#ifdef SRS_TIMEX
local void 
srs_xmit(struct trb *trb)
{
    rymp_t rsp = (rymp_t)trb->func_data;

    rsp->ry_xmit = 1;
    if (!rsp->ry_posted) {
    rsp->ry_posted = 1;
    i_sched(rsp);
    }
}
#endif
int
srs_clearbusy(rymp_t rsp)
{
    register str_rsp_t tp = rsp->ry_tp;
    int old_intr_hard;
    register caddr_t io_page;
    register struct srs_port *port;
    register uchar lsr = 0;
    DoingPio;

    if (rsp->ry_open) {
        io_page = io_att(rsp->ry_intr.bid, 0);
        port = (struct srs_port *)(io_page + rsp->ry_port);
        StartPio(srs_pio_catch, rsp, return(0));
        lsr = GR(port->r_lsr);
        EndPio();
        io_det(io_page);
    
        old_intr_hard = disable_lock(tp->t_priority, &tp->t_lock_hard);
        
        if ((tp->t_busy) && (lsr & TEMT) &&
            !(rsp->ry_posted) && !(rsp->ry_xmit)) {
            rsp->ry_xmit = 1;
            rsp->ry_posted = 1;
            i_sched(rsp);
        }
        unlock_enable(old_intr_hard, &tp->t_lock_hard);

        clr_busy_wdog = timeout(srs_clearbusy, rsp, 10*hz);
    }
    else clr_busy_wdog = 0;
}
