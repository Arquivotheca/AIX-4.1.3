/* @(#)77 1.6 src/bos/kernel/sys/ppdd.h, sysxprnt, bos41J, 9510A_all 3/6/95 23:21:50 */
#ifndef _H_PPDD
#define _H_PPDD
/*
 * COMPONENT_NAME: (SYSXPRNT) Parallel Printer Unix Driver include file
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */                                                                   

/*
 *    includes need to build/ define this structure
 */

#include <sys/types.h>
#include <sys/watchdog.h>
#include <sys/cblock.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/errids.h>
#include <sys/device.h>
#include <sys/adspace.h>

#ifdef	PM_PHASE2
#include <sys/pmdev.h>
#endif	/* PM_PHASE2 */


#define BDS      1      /* use Family One card values   */
#define NIOCARD  1      /* using native io card */
#define USEINTR	 1	/* interrupt handler's allowed	*/

#define HACKINIT	(!(!hackvar || ( hackvar = 0 )))

/*************** Constant, Type, and Macro Definitions *****************/

/* Special Characters */

#define FF	0x0c	/* Form Feed */

/* Control Register Bit Masks   */

#define PP_STROBE       0x01    /* Strobe bit mask              */
#define PP_AUTOFD       0x02    /* Auto line feed bit mask      */
#define PP_INIT         0x04    /* Reset of this starts printer */
				/* strobe low to reset printer  */
#define PP_SLCT_IN      0x08    /* Select_In bit mask           */
#define PP_IRQ_EN       0x010   /* Interrupt enable bit mask    */
#define PP_DIR          0x020   /* Direction bit mask           */
				/* 0=write to port, 1=read      */
#define PP_NOTUSED      0x0c0   /* Unused bits, set to 0        */

/* Performance Register Bit Masks   */

#define PP_FIFO         0x01    /* Performance Enable           */
#define PP_AUTOFD	0x02	/* Auto line feed bit mask	*/
#define PP_FIFO_CLR     0x08    /* FIFO clear bit               */

/* Status Register Bit Masks and Macros */

#ifndef NIOCARD
#define PP_UNUSED	0x03	/* Bit 0,1,2: Unused on Ser/Par	Adapter */
#define PP_RESERVED PP_UNUSED
#else
#define PP_RESERVED	0x03	/* Bit 0,1: Reserved, bits 0 and 1	*/
#define PP_IRQ_STAT	0x04	/* Bit 2: Interrupt Status bit mask, on = 0 */
#endif

#define PP_ERROR_B	0x08	/* Bit 3: Error bit mask, on = 0	*/
#define PP_SELECT	0x010	/* Bit 4: Select bit mask, on = 1	*/
#define PP_PE		0x020	/* Bit 5: Page End bit mask, on = 1	*/
#define PP_ACK		0x040	/* Bit 6: Acknowledge bit mask, on = 0	*/
#define PP_BUSY_BIT	0x080	/* Bit 7: Busy bit mask, on = 0		*/

/* Macros for Checking Status */

#define PP_FATAL_TST(reg) (((reg) & ~PP_RESERVED) == PP_ERROR_B)

/* Timer Macros */

#ifndef TIME_REQ_SECONDS
#define TIME_REQ_SECONDS(tvp) (tvp)->timeout.it_value.tv_sec
#endif
#ifndef TIME_REQ_NANOSECS
#define TIME_REQ_NANOSECS(tvp) (tvp)->timeout.it_value.tv_nsec
#endif
#define PPMALLOC(dtyp) (dtyp *)xmalloc((uint)sizeof(dtyp),(uint)3,pinned_heap)

/* Timer Constants */

/* simple algorithm, requires second to be evenly divided by timer interval   */
#define PP_ONE_SEC        1000000000    /* timer interval of one second in ns */
#define PP_TMR_R1_SEC     0             /* timer interval in seconds for R1   */
#define PP_TMR_R1_NS      250000000     /* timer interval in nano secs for R1 */
#define PP_TMR_NIO_SEC     1            /* timer interval in seconds for NIO  */
#define PP_TMR_NIO_NS      0            /* timer interval in nano secs for NIO*/
#define PP_TMR_ADP_SEC     1            /* timer interval in seconds for ADP  */
#define PP_TMR_ADP_NS      0            /* timer interval in nano secs for ADP*/

/* Some Return Values	(Internal to Driver)	   */

#define PPDONE		1	/* All data in output queue has been sent     */
#define PPBUSY		3	/* Printer is busy			      */
#define PPABORT		4	/* ppsleep returns upon cancel or fatal error */
#define PPCONTINUE	5	/* ppsleep returns on successfull i/o	      */

/* PIO Macros */

#define PP_PIO_PUTC    pp_pio_putc(loc)
#define PP_PIO_GETC    pp_pio_getc(loc)

#define PUTC            1
#define GETC            2


/* Convereged interface status  */

#define PPDESELECT        0       /* Printer not selected                       */
#define PPCANREQ          1       /* Cancel switch is depressed                 */
#define PPRESERVED2       2       /* Reserved                                   */
#define PPNOERROR         3       /* Error dose not exist in the printer        */
#define PPPE              4       /* Paper end or no paper                      */
#define PPRESERVED5       5       /* Reserved                                   */
#define PPRESERVED6       6       /* Reserved                                   */
#define PPPERMERROR       7       /* Printer dead                               */
#define PPECCERROR        8       /* ECC error                                  */
#define PPRESERVED9       9       /* Reserved                                   */
#define PPPRINTCOMP      10       /* Print complete                             */
#define PPRESERVED11     11       /* Reserved                                   */
#define PPASFJAM         12       /* ASF JAM                                    */
#define PPRESERVED13     13       /* Reserved                                   */
#define PPHEADALARM      14       /* High temperature head alarm                */
#define PPNOTRMR         15       /* Not receive machine ready                  */

/* Interface Constants */

#define PPDATAREG        0      /* Displacement to data reg                   */
#define PPSTATREG        1      /* Displacement to Status reg                 */
#define PPCTRLREG        2      /* Displacement to control reg                */
#define PPPFRREG         3      /* Displacement to High performance register  */
#define PPCIDREG     0x00000000 /* Address of card id                         */
#define PPDAR        PPCIDREG+4 /* Address of Diskette Arbitration Register   */
#define PPCARDIDR11  0x5F       /* Card id for release one nio card           */
#define PPCARDIDR12  0xdf       /* Card id for release one nio card           */
#define PPCARDIDSA1   0xfe      /* Card id for salmon nio card                */
#define PPCARDIDSA2   0xf6      /* Card id for salmon nio card                */
#define PPCARDIDNIO1  0xe6      /* Card id for release two nio card           */
#define PPCARDIDNIO2  0xde      /* Card id for release two nio card           */
#define PPCARDIDIOD1  0xd9      /* Card id for IOD card                       */
#define PPCARDIDIOD2  0xfe      /* Card id for IOD card                       */ 

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *      global data definitions:                                             *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define PP_SNDLST_AB    0x70
#define PP_SLPSIG_AB    0x71
#define PP_SIGFLUSH     0x72
#define PP_SNDFF        0x73


/* Miscellaneous Constants */

#define PP_ADAP_MINOR	0x9860	/* Minor number for parallel adapter	      */
#define DMAMAX          32768   /* Maximum bytes for DMA transfer             */
#define DMAMIN          256     /* Minimum bytes for DMA transfer             */
#define PIOBUFFSZ       4096    /* Buffer size for PIO mode of operation      */
#define PPINTRMASK	0x04	/* Mask for Disabling Interrupts	      */
#define PPCLISTMAX	256	/* Maximum Permissible Clist Size	      */
#define PPANYDEV	-1	/* arg to ck_open_dev requesting check whether*/
				/* any device whatsoever is open	      */
#define PP_FAIL_ADJUST	0x01	/* Print Failed Here, Adjust u.u_count	      */
#define PP_FAIL_NO_ADJUST 0x02	/* Print Failed Here, Do Not Adjust u.u_count */

/* busy wait definitions */

#define PP_WAIT_SHORT	(NS_PER_SEC / 100)
#define PP_WAIT_LONG	(NS_PER_SEC / 10)
#define PP_WAIT_MIN	100000


/* Declarations */

/*
 *  printer dds structure passed in at config time.
 */

struct ppdds {
	int     bus_type;       /* for intr.h struct            */
	int     level;          /* for intr.h struct            */
	int     priority;       /* for intr.h struct            */
	int     bus_id;         /* for intr.h struct            */
	int     i_flags;        /* for intr.h struct            */
	int     interface;      /* type of interface for status */
#define PPIBM_PC          0     /* Standered IBM PC interface   */
#define PPCONVERGED       1     /* Converged interface          */
	int     modes;          /* line printer modes           */
	char    name[16];       /* device name (/dev/lp0  )     */
	int     slot;           /* slot number                  */
	int     v_timeout;      /* time out value (starting)    */
	int     pp_lin_default; /* starting page length         */
	int     pp_col_default; /* starting page width          */
	int     pp_ind_default; /* starting indent point        */
	int     rg;             /* base io address              */
	int     posadd;         /* pos reg address              */
	int     dma_lvl ;       /* DMA channel address          */
	int	busy_delay;	/* uS before checking busy line */
};

struct prtinfo {
	int     ind;            /* indent level                            */
	int     col;            /* columns per line, including indent      */
	int     line;           /* lines per page                          */
	int     ccc;            /* current character count                 */
	int     mcc;            /* max character count                     */
	int     mlc;            /* max line count                          */
	int     chars_sent ;    /* error recovery mode char counter        */
	int     dummy_send ;    /* set during error recovery non plot      */
	struct clist outq;      /* output queue header                     */
} ;

struct ppiost
{
	int     address;        /* IO address of traction                  */
	char    data ;          /* data to transfer                        */
	char    data1;          /* data to transfer                        */
	char    data2;          /* data to transfer                        */
	char    opflag ;        /* operation flag                          */
	int     busacc ;        /* bus acc information                     */
	struct pp *pp ;         /* pointer to printer structure            */
};

struct ppelog
{
	struct err_rec0 header;
	struct ppiost pioerr ;
};

struct ppwatch                   /*  watch dog timer sctructure           */
{
	struct watchdog watch ; /*   watch dog timer                      */
	void * pp_prt ;         /*   pointer to printer structure         */
};

struct flags
{
	unsigned char ppread ;      /* device open for reading                */
	unsigned char ppwrite ;     /* device open for writing                */
	unsigned char ppopen ;      /* device open                            */
	unsigned char inited ;      /* device inited                          */
	unsigned char ppiodone ;    /* last i/o done was not done in PLOT mode*/
	unsigned char ppinitfail ;  /* initialization failed                  */
	unsigned char ppcancel ;    /* ck.prntr status before entering ppwrite*/
	unsigned char ppoverstrike ;/* processing an overstrike               */
	unsigned char ppdiagmod ;   /* opened in diag mode                    */
	unsigned char pptimerset ;  /* indicates timer turned on              */
	unsigned char ppff_needed ; /* form feed needed                       */
	unsigned char pptimeout ;   /* timeout has occurred                   */
	unsigned char dma_runing ;  /* set when dma is running                */
	unsigned char ppintocc ;    /* interrupt has occurred                 */
	unsigned char ppcharsent;   /*set when character sent to printer      */
	unsigned char dmaaval ;     /* set if dem is available for printer    */
	unsigned char ppdone ;      /* set when transfer complete to printer  */
	unsigned char writeflag ;   /* set if dem is available for printer    */
	unsigned char errrecov ;    /* error recovery flag for write          */
	unsigned char dontsend;	    /* dont send data at timer pop	      */
};

/* Per-device data structure */

struct pp {
	unsigned int    ppmodes;/* printer modes                           */
	struct flags flags ;    /* printer flags used for operation        */
	char *  buffer ;        /* buffer address                          */
	int     buf_size  ;     /* buffer size                             */
	int     buf_index ;     /* buffer pointer                          */
	int     buf_count ;     /* buffer size for this transaction        */
	struct uio  *uio ;      /* UIO pointer                             */
	int     bus_id ;        /* Bus ID                                  */
	int     adapter ;       /* adapter type                            */
	int     statreg;        /* pointer to status register              */
	int     ctrlreg;        /* pointer to control register             */
	int     datareg;        /* pointer to data register                */
	int     pfrreg;         /* pointer to data register                */
	int     cidreg1;        /* pointer to cardid register 1            */
	int     cidreg2;        /* pointer to cardid register 2            */
	int     rcvd_sig;       /* received signal, no writes since        */
	int     v_timout;       /* seconds before time out                 */
	int     wdtcnt;         /* Watchdog Counter                        */
	int     interface;      /* Interface type                          */
	struct prtinfo prt ;    /* page info for a page                    */
	int     error;          /* error value to return from entry rtn    */
	int     lastchar ;      /* save point for the last char sent out   */
	dev_t   dev;            /* major-minor device number               */
	int     write_lock;     /* write lock variable                     */
#ifdef _POWER_MP
	Simple_lock intr_lock;
#endif /* _POWER_MP */
	struct  pp      *next;  /* pointer to next pp structure            */
	struct ppelog ppelog ;  /* error log info                          */
	int 	pp_event;	/* used by ppdelay */
	struct trb *tmr;	/* timer struct */
	ulong nsecs;		/* current busy wait delay */
	ulong btimeout;		/* total busy wait for error timeout */
	int timeoutflag;	/* process level timeout flag */
	ulong bflag;		/* busy loop counter */
	ulong avgw;		/* current running busy wait average */
	ulong countw;		/* sample count for average */
	ulong totw;		/* total wait for current character */
	int pop_rc;		/* timer pop return code */
	int busy_delay;		/* uS before checking busy line */
#ifdef _PP_ISA_BUS
	struct io_map iomap;	/* struct for mapping port with iomem_att */
#endif	/* _PP_ISA_BUS */
#ifdef	PM_PHASE2
	struct	pm_handle *pmh;	/* struct for power management handle	*/
	int	pmbusy;		/* device busy flag for PM		*/
	int	pmblock;	/* I/O block flag for PM		*/
	int	pppmblock;	/* event_word for PM I/O block e_wakeup */
	char	pmdatareg;	/* date reg save area for PM		*/
	char	pmctrlreg;	/* ctrl reg save area for PM		*/
#endif	/* PM_PHASE2 */
};

/* Adapter specific config data */
struct ppa {
	int     bus_id ;        /* Bus ID                                  */
	int     rg ;            /* Base address                            */
	int     cidreg1;        /* pointer to cardid register 1            */
};

int ppconfig(
	dev_t,                  /* major-minor device number    */
	int,                    /* command                      */
	struct uio *);          /* uio structure containing device data */

int ppopen(
	dev_t,                  /* major and minor device number */
	ulong,                  /* defined in file.h */
	int,                    /* channel number     */
	int) ;                  /* extension val     */

int ppclose(
	dev_t,                  /* major and minor device number */
	int,                    /* channel number     */
	int) ;                  /* extension val     */

int ppread(
	dev_t,                  /* major and minor device number */
	struct uio *,           /* uio structure containing device data */
	int,                    /* channel number     */
	int);                   /* extension val     */

int ppwrite(
	dev_t,                  /* major and minor device number */
	struct uio *,           /* uio structure containing device data */
	int,                    /* channel number     */
	int);                   /* extension val     */

int ppioctl(
	dev_t,                  /* device number */
	int,                    /* command argument ( which ioctl desired ) */
	int,                    /* address of the user space */
	ulong,                  /* open flags */
	int,                    /* channel number     */
	int);                   /* extension val     */

int ppioctl_read(
	struct pp *,            /* pointer to device structure */
	int,                    /* command argument ( which ioctl desired ) */
	int);                   /* address of the user space */

int ppioctl_write(
	struct pp *,            /* pointer to device structure */
	int,                    /* command argument ( which ioctl desired ) */
	int);                   /* address of the user space */


int ppinit_dev(
	struct pp *) ;          /* pointer to device structure */

int ppopen_dev(
	struct pp *) ;          /* pointer to device structure */

int ppclose_dev(
	struct pp *) ;          /* pointer to device structure */

struct pp * ppget(
	int ) ;                 /* minor device number */

int ppsendchar(
	struct pp *) ;          /* pointer to device structure */

int ppreadchar(
	struct ppiost *) ;      /* address of io structure    */

int ppreadstat(
	struct pp * ) ;         /* pointer to device structure */

int ppwritechar(
	struct ppiost *) ;      /* address of io structure */

int ppreadctrl(
	struct pp *) ;          /* pointer to device structure */

int ppwritectrl(
	struct pp *,            /* pointer to device structure */
	char) ;

int ppreadpfr(
	struct pp *) ;          /* pointer to device structure */

int ppwritepfr(
	struct pp *,            /* pointer to device structure */
	char) ;

int ppwritestat(
	struct pp *,            /* pointer to device structure */
	char) ;

int ppreaddata(
	struct pp *) ;          /* pointer to device structure */

int ppwritedata(
	struct pp * ,           /* pointer to device structure */
	char) ;

struct pp * ppalloc(
	int,                    /* device number   */
	struct uio *,           /* pointer to uio structure  */
	struct pp *);		/* NULL or pointer to pp structure */

void ppfree(
	struct pp *) ;          /* pointer to device structure */

int ppsendff(
	struct pp *) ;          /* pointer to device structure */

int ppsendlist(
	struct pp *) ;          /* pointer to device structure */

int ppibmpc(
	char statusreg) ;

int ppconverged(
	char statusreg,
	struct pp *) ;          /* pointer to device structure */

int pppulst(
	char,
	struct pp *) ;          /* pointer to device structure */

int ppready(
	char,
	struct pp *) ;          /* pointer to device structure */

void ppelog_log(
	struct ppiost *) ;

int ppnio_read(
	struct ppiost *);

int ppnio_write(
	struct ppiost *);

void prnformat(
	 struct prtinfo *,
	 uint,
	 int);

void stuffc(
	 struct prtinfo *,
	 int)  ;

int ppwrited(
	 struct pp *);          /* pointer to device structure */

int pp_pio_getc(
	struct ppiost *);       /* pointer to io struct      */

int pp_pio_putc(
	struct ppiost *);       /* pointer to io struct      */

int pp_pio_retry(
	struct ppiost *,        /* pointer to io struct      */
	int);                   /* exception code            */

int pp_read_POS(
	 struct pp *,           /* pointer to device structure */
	 uint);                 /* offset to reg             */

int pp_write_POS(
	 struct pp *,           /* pointer to device structure */
	 uint,                  /* offset to reg             */
	 uchar);                /* data to write             */

int ppddpin();                  /* pin point for bottom half */


#ifdef DEBUG   /* Debugging Aids */
struct ppdbinfo
{
	ulong hits ;
	ulong misses ;
	ulong waitmin ;
	ulong waitmax ;
	ulong bflagmax ;
	ulong waitavg;
	ulong countw;
} ;
#endif /* DEBUG */

#endif /* _H_PPDD     */
