/* @(#)19        1.2  src/bos/kernel/sys/POWER/darticdd.h, dd_artic, bos411, 9428A410j 6/10/94 08:32:45
 *
 * COMPONENT_NAME: dd_artic -- ARTIC Diagnostic Device Driver
 *
 * FUNCTIONS: Include file
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * This include file is used by ARTIC diagnostic device driver and
 * its configuration method
 *
 */

/*
 *      Control Register Address Offsets
 *
 *       Actual IO address is oa_adapters[cardnumber].baseio + OFFSET,
 *       where OFFSET is:
 *
 *              DREG               Registers INITREG0-3 can be read from
 *                                 DREG by first programming the INITREG
 *                                 "pointer" value into PTRREG.
 *
 *              PTRREG             This is where the register "pointer" goes.
 *
 *              TASKREG            Called TREG in the documentation, this
 *                                 register contains the task number of the
 *                                 interrupting task from the board.  0xFF or
 *                                 0xFE means this board did not interrupt.
 *
 *              COMREG             Used to re-initialize the board.
 *
 *              CPUPAGE            Used to determine what board memory appears
 *                                 in kernel memory window.
 *
 *              DREG registers:
 *
 *              INITREG0        -
 *              INITREG1        -  Contains PROM ready bit (bit 6)
 *              INITREG2        -
 *              INITREG3        -
 *
 */

#include <sys/device.h>
#include <sys/xmem.h>

#define COMREG          0x06
#define CPUPAGE         0x05
#define TASKREG         0x04
#define DREG            0x03
#define PTRREG          0x02
#define MEGREG          0x01
#define LOCREG          0x00

/*      PTRREG Register "Pointer" Values (program PTRREG, then read from DREG) */

#define INITREG0        0x12
#define INITREG1        0x10
#define INITREG2        0x08
#define INITREG3        0x13
#define PCPAR2          0x11

/*      Bit values for above registers */

#define NMIVALUE        0x02    /* for COMREG NMI generation            */
#define CR_RESETVAL     0x11    /* reset value for COMREG               */
#define DEGATERAM       0x04    /* for COMREG degate RAM bit            */
#define PROMREADY       0x40    /* INITREG1 prom ready bit              */
#define NOTPROMREADY    0xBF    /* INITREG1 prom ready bit              */
#define INTBOARD        0x09    /* PTRREG interrupt bit                 */
#define INTENABLE       0x10    /* COMREG interrupt enable bit          */
#define CARD_ENABLE     0x01    /* POS register 2 (SE bit)              */
#define FAIRNESS_ENABLE 0x01    /* POS register 5 (fairness enable bit) */
#define PARITY_ENABLE   0x20    /* POS register 5 (parity enable bit)   */

/*      Interrupt level equates (values found in POS2 >> 1) */

#define INTLEVEL_3              0
#define INTLEVEL_4              1
#define INTLEVEL_7              2
#define INTLEVEL_9              3
#define INTLEVEL_10             4
#define INTLEVEL_11             5
#define INTLEVEL_12             6

/*      Base IO address equates (values found in POS2 >> 4) */

#define BASEIO_2A0              0
#define BASEIO_6A0              1
#define BASEIO_AA0              2
#define BASEIO_EA0              3
#define BASEIO_12A0             4
#define BASEIO_16A0             5
#define BASEIO_1AA0             6
#define BASEIO_1EA0             7
#define BASEIO_22A0             8
#define BASEIO_26A0             9
#define BASEIO_2AA0             10
#define BASEIO_2EA0             11
#define BASEIO_32A0             12
#define BASEIO_36A0             13
#define BASEIO_3AA0             14
#define BASEIO_3EA0             15

/*
 *      BUS ID defintions
 *
 *      These are used with the system macros BUSIO_ATT,  BUSMEM_ATT, and IOCC_ATT.
 *      These values get put into the RISC CPU segment registers, which control
 *      access to the various buses in the system.  The value is a bit pattern that
 *      controls which bus, read/write control, address incrementing, and other
 *      characteristics.  See the IBM RS/6000 hardware documentation for more
 *      information.
 *
 *      Summary:
 *
 *              Use ARTIC_BUS_ID as the bus ID parameter with the following system
 *              macros (defined in adspace.h):  BUSMEM_ATT, BUSIO_ATT, and IOCC_ATT
 */

#define ARTIC_BUS_ID    0x000C0020

/*
 *      Note: EXTCLKX25BRD is placed in POS4 on the X25 adapter to enable
 *            external clocking.
 */

#define EXTCLKX25BRD    0x40

/*
 *      Misc RCM definitions...
 *
 *      TASK0            Easy way to specify RCM task number (0)
 *
 *      OK_TO_INT1       If either of these values in pc_select byte of IB, its
 *      OK_TO_INT2       ok to interrupt.
 *
 *      CHK_TASK         A parameter to taskbusy(task,checkwhat).  If CHK_TASK
 *      CHK_BUFF_TOO     is passed, only the task busy bit in the PSB for that
 *                       task is checked.  If CHK_BUFF_TOO is passed, then the
 *                       output buffer busy bit is also checked.  taskbusy()
 *                       returns TRUE if the task (or task OR buffer) is busy.
 *
 *      RCMTIMEOUT       When sending an RCM command using sendRCMcmd(), this
 *                       is the amount of time in seconds it will wait to be
 *                       awakened by the interrupt handler.  Only applys to calls
 *                       made with parameter waitf (wait flag) == TRUE.
 *
 */

#define         TASK0           0

#define         OK_TO_INT1      0xFF
#define         OK_TO_INT2      0xFE

#define         CHK_TASK        1
#define         CHK_BUFF_TOO    2

#define         RCMTIMEOUT      5

 /*     MCA device identifiers                  */

#define         PORTM_POS0              0x70
#define         PORTM_POS1              0x8F
#define         SP5_POS0                0x71
#define         SP5_POS1                0x8F
#define         MP2_POS0                0xF0
#define         MP2_POS1                0xEF

 /*     definitions for window size     */

#define WNDW_64K        0xffff  /* 16 bit mask for window size 64K */
#define WNDW_32K        0x7fff  /* 15 bit mask for window size 32K */
#define WNDW_16K        0x3fff  /* 14 bit mask for window size 16K */

#define NOT_64K 0xffff0000      /* Mask out below 64K                   */
#define NOT_32K 0xffff8000      /* Mask out below 32K                   */
#define NOT_16K 0xffffc000      /* Mask out below 16K                   */

#define POSWIN_8K       0                       /* window values in POS (devdata)       */
#define POSWIN_16K      1
#define POSWIN_32K      2
#define POSWIN_64K      3
#define POSWIN_128K     4

#define SHIFT_8K        3                       /* shift values for CPUPAGE operations */
#define SHIFT_16K       2
#define SHIFT_32K       1
#define SHIFT_64K       0

#define WNDW_SHIFT      SHIFT_32K   /* current window size is 32K     */
#define WNDW_MSK        WNDW_32K    /* current window size is 32K     */
#define WNDW_NOT        NOT_32K     /* current window size is 32K     */
#define WNDW_POS        POSWIN_32K  /* current window size is 32K     */
#define WNDW_SIZE       0x00008000  /* current window size is 32K     */

/*
 *      Define SLOTADDR for use with IOCC_ATT to read POS registers
 */

#define  SLOTADDR(addr)                 (IO_CONFIG | (addr << 16))

/*
 *      Number of milliseconds per timer tick
 */

#define NMS_PER_TICK    (1000/HZ)


/*
 *      typedef DARTIC_IntrPlus is used to "add" the cardnumber to the AIX defined
 *      structure "intr" (defined in intr.h).  It is used to bind the interrupt
 *      handler to the interrupt level.
 */

struct DARTIC_IntrPlus
{
    struct intr     artic_interrupt;  /* interrupt description struct */
    int             cardnumber;       /* interrupting adapter         */
};


/*
 * typedef DARTIC_Adapter will describe the Adapter Table.
 */

#define         DARTIC_EMPTY                0       /* No adapter in slot                                   */
#define         DARTIC_READY                1       /* Adapter passed self test                             */
#define         DARTIC_BROKEN               2       /* Adapter failed self test                             */
#define         DARTIC_NOTREADY             3       /* PROMREADY bit not checked yet...             */

typedef         struct
{
    int     state;          /* adapter functional state;  DARTIC_EMPTY,
                               DARTIC_BROKEN, or DARTIC_READY                   */
    int     cardnumber;     /* adapter sequence number                  */
    int     maxpri;         /* value for RCM MAXPRI for this adapter    */
    int     maxtask;        /* value for RCM MAXTASK for this adapter   */
    int     maxqueue;       /* value for RCM MAXQUEUE for this adapter  */
    int     maxtimer;       /* value for RCM MAXTIMER for this adapter  */
    int     adaptertype;    /* Unique for each EIB+base card combo      */
    int     basetype;       /* MULTIPORT/2, PORTMASTER or SP 5          */
    int     slot;           /* card cage slot the adapter is in.        */
    int     intlevel;       /* adapter interrupt level                  */
    int     windowsize;     /* size of shared memory window             */
    int     maxpage;        /* size of shared memory window             */
    int     dmalevel;       /* dma arbitration level                    */
    int     debugflag;      /* if set, driver may print to console      */
    int     pos2;           /* These POS register values are saved      */
    int     pos3;           /*  during the articconfig driver function  */
    int     pos4;           /*   and are re-programmed into the adapter */
    int     pos5;           /*    during the RESET ioctl.               */
    ulong   module_id;      /* kernel extension module identifier       */
    ulong   basemem;        /* adapter base memory address              */
    ulong   baseio;         /* adapter base I/O address                 */
    ulong   dmamem;         /* adapter DMA memory address               */
    int     bus_id;         /* ID of adapters Microchannel connection   */
} DARTIC_Adapter;

typedef struct
{
    caddr_t uaddr;                  /* Buffer address                */
    uint count;                     /* Number of bytes to xfer       */
    struct  xmem dp;                /* Cross memory descriptor       */
    int     channel_id;             /* Channel ID for DMA operations */
    int     type;                   /* Type of DMA xfer              */
    int     flags;                  /* Flags sent to d_master        */
} DARTIC_DMA;

struct DARTIC_Proc
{
        /*
         *      DARTIC_Proc structures are arranged in a linked list, with the head
         *      pointer being oa_procptr[index], where index is the PID % OAPA_SIZE
         */

        struct DARTIC_Proc  *next;          /* pointer to next DARTIC_Proc struct in list */

        /*
         *      procreg is an array of pointers to ProcReg structures (arranged in a
         *      linked list) that may be allocated for the process.  These are used
         *      for the "Interrupt Register" ioctl.
         */

        struct  ProcReg *prptr[MAXADAPTERS];    /* one linked list per adapter  */

        int     myPID;         /* Calling Processes PID                     */

        int     tocfcount;     /* number of callout table entries added to
                                  kernel                                    */

};

#define PANULL          ((struct DARTIC_Proc *)0)           /* NULL pointer */

struct  ProcReg
{
        struct  ProcReg *next; /* interrupt context link pointer */

        int   reserved;       /* reserved for future use               */
        int   eventcount;     /* event counter - incremented by either */
                              /*  the interrupt handler or the Issue   */
                              /*   Command ioctl                       */
};

#define PRNULL          ((struct ProcReg *)0)           /* NULL pointer */

/* Default parameters */

#define DEF_MAXTASK             0x10    /* Max task number for adapter          */
#define DEF_MAXPRI              0x10    /* Number of task priorities            */
#define DEF_MAXTIME             0x32    /* Max number of timers for adapter     */
#define DEF_MAXQUEUE    0x50    /* Max number of queues for adapter     */

/* For documentation purposes */

#define TASK_00         0x00
#define TASK_FE         0xfe

/* Types of DMA transfers allowed by the ICADMASETUP ioctl */

#define ADAPTER_TO_SU   0
#define SU_TO_ADAPTER   1
#define ADAPTER_TO_PEER 2
#define PEER_TO_ADAPTER 3

/*      Ioctl Stuff     */

/* This structure contains configuration information about      */
/* a specific adapter                                           */

typedef struct
{
        ushort io_addr;                /* Base address of adapter's I/O ports */
        uchar maxtask;                 /* Max task number for adapter         */
        uchar maxpri;                  /* Number of task priorities           */
        uchar maxqueue;                /* Max number of queues for adapter    */
        uchar maxtime;                 /* Max number of timers for adapter    */
        uchar int_level;               /* Adapters interrupt level            */
        uchar ssw_size;                /* Size of the shared storage window   */
} ICAPARMS;

/* This structure contains the length and address of an         */
/* input, output or secondary status buffer                                     */

typedef struct
{
        ushort length;                  /* Length of buffer             */
        ushort offset;                  /* Offset of buffer's address   */
        uchar page;                     /* Page of buffer's address     */
} ICABUFFER;


/* Structures used for IOCTL input and output parameters */

typedef struct
{
        uchar reserved1;              /* reserved field               */
        ushort retcode;               /* Return Code                  */
        uint   reserved;              /* reserved field               */
} ICARESET_PARMS;

/*      structure for ioctl ICAREADMEM  */

typedef struct
{
        uchar reserved1;                /* reserved field               */
        ulong  length;                  /* Length                       */
        ushort segpage;                 /* Segment/Page                 */
        ushort offset;                  /* Offset                       */
        uchar *dest;                    /* Destination buffer pointer   */
        uchar addr_format;              /* Address format               */
        ushort retcode;                 /* Return Code                  */
        uint   reserved;                /* reserved field               */
} ICAREADMEM_PARMS;

/*      structure for ICAWRITEMEM ioctl */

typedef struct
{
        uchar reserved1;                /* reserved field               */
        ulong  length;                  /* Length                       */
        ushort segpage;                 /* Segment/Page                 */
        ushort offset;                  /* Offset                       */
        uchar *source;                  /* Source buffer pointer        */
        uchar addr_format;              /* Address format               */
        ushort retcode;                 /* Return Code                  */
        uint   reserved;                /* reserved field               */
} ICAWRITEMEM_PARMS;

/*      structure for ICAINTREG ioctl   */

typedef struct
{
        uchar reserved1;             /* reserved field               */
        uchar reserved2;             /* reserved field               */
        ushort retcode;              /* Return Code                  */
        uint reserved;               /* reserved field               */
} ICAINTREG_PARMS;

/*      structure for ICAINTWAIT ioctl  */

typedef struct
{
        uchar reserved1;             /* reserved field               */
        uchar reserved2;             /* reserved field               */
        uint  timeout;               /* Timeout                      */
        ushort retcode;              /* Return Code                  */
        uint  reserved;              /* reserved field               */
} ICAINTWAIT_PARMS;

/*      structure for ICAINTDEREG ioctl */

typedef struct
{
        uchar reserved1;             /* reserved field               */
        uchar reserved2;             /* reserved field               */
        ushort retcode;              /* Return Code                  */
        uint  reserved;              /* reserved field               */
} ICAINTDEREG_PARMS;

/*      structure for ICAISSUECMD ioctl */

typedef struct
{
        uchar reserved1;             /* reserved field               */
        uchar reserved2;             /* reserved field               */
        uchar   cmdcode;             /* Command Code                 */
        ushort  length;              /* Length of parameter buffer   */
        uint    timeout;             /* Timeout                      */
        uchar  *prms;                /* Pointer to parameters        */
        ushort  retcode;             /* Return Code                  */
        uint    reserved;            /* Reserved field               */
} ICAISSUECMD_PARMS;

/*      structure for ICAGETPARMS ioctl */

typedef struct
{
        uchar reserved1;             /* reserved field               */
        ICAPARMS cfgparms;           /* Configuration parameters     */
        ushort retcode;              /* Return Code                  */
        uint reserved;               /* reserved field               */
} ICAGETPARMS_PARMS;

/*      structure for ICAGETBUFADDRS ioctl      */

typedef struct
{
        uchar reserved1;             /* reserved field               */
        uchar reserved2;             /* reserved field               */
        ICABUFFER ib;                /* Input buffer information     */
        ICABUFFER ob;                /* Output buffer information    */
        ICABUFFER ssb;               /* SS buffer information        */
        ushort retcode;              /* Return Code                  */
        uint reserved;               /* reserved field               */
} ICAGETBUFADDRS_PARMS;

typedef struct
{
        uchar reserved1;             /* reserved field               */
        uchar reserved2;             /* reserved field               */
        uchar psb;                   /* Primary Status byte           */
        ushort retcode;              /* Return Code                   */
        uint   reserved;             /* reserved field                */
} ICAGETPRIMSTAT_PARMS;


/*      structure for ICAIOWRITE ioctl  */

typedef struct
{
        uchar   reserved1;              /* reserved field                */
        uchar   portnum;                /* I/O port offset              */
        uchar   value;                  /* value to write to port       */
        ushort  retcode;                /* return code                  */
        uint    reserved;               /* reserved field               */
} ICAIOWRITE_PARMS;

/*      structure for ICAIOREAD ioctl   */

typedef struct
{
        uchar   reserved1;              /* reserved field                */
        uchar   portnum;                /* I/O port offset              */
        uchar   value;                  /* returned value               */
        ushort  retcode;                /* return code                  */
        uint    reserved;               /* reserved field               */
} ICAIOREAD_PARMS;

/*      structure for ICAPOSREAD ioctl   */

typedef struct
{
        uchar   reserved1;              /* reserved field                */
        uchar   regnum;                 /* POS register number          */
        uchar   value;                  /* returned value               */
        ushort  retcode;                /* return code                  */
        uint    reserved;               /* reserved field               */
} ICAPOSREAD_PARMS;

/*      structure for ICAPOSWRITE ioctl  */

typedef struct
{
        uchar   reserved1;              /* reserved field                */
        uchar   regnum;                 /* POS regsiter number          */
        uchar   value;                  /* value to write to register   */
        ushort  retcode;                /* return code                  */
        uint    reserved;               /* reserved field               */
} ICAPOSWRITE_PARMS;

/*      structure for ICADMASETUP ioctl  */
typedef struct
{
        uchar   reserved1;              /* reserved field                */
        caddr_t uaddr;                  /* User space buffer             */
        uint    count;                  /* Number of bytes being xferred */
        ulong   physaddr;               /* DMA Xfer physical address     */
        int     type;                   /* Type of DMA transfer          */
        ushort  retcode;                /* return code                   */
        uint    reserved;               /* reserved field                */
} ICADMASETUP_PARMS;

/*      structure for ICADMAREL ioctl  */
typedef struct
{
        uchar   reserved1;              /* reserved field                */
        ushort  retcode;                /* return code                   */
        uint    reserved;               /* reserved field                */
} ICADMAREL_PARMS;

/*      structure for ICAGETADAPTYPE ioctl  */
typedef struct
{
        uchar   reserved1;              /* reserved field                */
        uint    type;                   /* adapter type                  */
        ushort  retcode;                /* return code                   */
        uint    reserved;               /* reserved field                */
} ICAGETADAPTYPE_PARMS;

/*
 *      Ioctl Sub-Command Definitions
 */

#define ICARESET        _IOWR('x',0x00,ICARESET_PARMS)
#define ICAREADMEM      _IOWR('x',0x01,ICAREADMEM_PARMS)
#define ICAWRITEMEM     _IOWR('x',0x02,ICAWRITEMEM_PARMS)
#define ICAINTREG       _IOWR('x',0x03,ICAINTREG_PARMS)
#define ICAINTWAIT      _IOWR('x',0x04,ICAINTWAIT_PARMS)
#define ICAINTDEREG     _IOWR('x',0x05,ICAINTDEREG_PARMS)
#define ICAISSUECMD     _IOWR('x',0x06,ICAISSUECMD_PARMS)
#define ICAGETPARMS     _IOWR('x',0x08,ICAGETPARMS_PARMS)
#define ICAGETBUFADDRS  _IOWR('x',0x09,ICAGETBUFADDRS_PARMS)
#define ICAGETPRIMSTAT  _IOWR('x',0x0A,ICAGETPRIMSTAT_PARMS)
#define ICAIOWRITE      _IOWR('x',0x0F,ICAIOWRITE_PARMS)
#define ICAIOREAD       _IOWR('x',0x10,ICAIOREAD_PARMS)
#define ICAPOSWRITE     _IOWR('x',0xF6,ICAPOSWRITE_PARMS)
#define ICAPOSREAD      _IOWR('x',0xF7,ICAPOSREAD_PARMS)
#define ICADMASETUP     _IOWR('x',0xF4,ICADMASETUP_PARMS)
#define ICADMAREL       _IOWR('x',0xF5,ICADMAREL_PARMS)
#define ICASENDCFG      _IOWR('x',0xF0,ICAGETPARMS_PARMS)
#define ICAGETADAPTYPE  _IOWR('x',0xF2,ICAGETADAPTYPE_PARMS)

/*
 *      Union of all IOCTL parameter structures
 */

typedef union
{
        ICARESET_PARMS          icareset;
        ICAREADMEM_PARMS        icareadmem;
        ICAWRITEMEM_PARMS       icawritemem;
        ICAINTREG_PARMS         icaintreg;
        ICAINTWAIT_PARMS        icaintwait;
        ICAINTDEREG_PARMS       icaintdereg;
        ICAISSUECMD_PARMS       icaissuecmd;
        ICAGETPARMS_PARMS       icagetparms;
        ICAGETBUFADDRS_PARMS    icagetbufaddrs;
        ICAGETPRIMSTAT_PARMS    icagetprimstat;
        ICAGETPARMS_PARMS       icasendconfig;
        ICAIOWRITE_PARMS        icaiowrite;
        ICAIOREAD_PARMS         icaioread;
        ICAPOSWRITE_PARMS       icaposwrite;
        ICAPOSREAD_PARMS        icaposread;
        ICAGETADAPTYPE_PARMS    icagetadaptype;
        ICADMASETUP_PARMS       icadmasetup;
        ICADMAREL_PARMS         icadmarel;
} ARTIC_IOCTL_UNION;


/*
 *      Typedefs for Intel->local and local->Intel byte order conversions
 */

typedef union
{
        ushort  b16;
        struct
        {
                uchar   i_low;                  /* Intel format high-order byte */
                uchar   i_high;                 /* Intel format high-order byte */
        } sep;
} SPEC16;

typedef union
{
        struct
        {
                SPEC16  offset;                         /* Intel format address offset  */
                SPEC16  segment;                        /* Intel format address segment */
        } seper;

        struct
        {
                uchar   ll;                                     /* low order word, low order byte       */
                uchar   lh;                                     /* low order word, high order byte      */
                uchar   hl;                                     /* high order word, low order byte  */
                uchar   hh;                                     /* high order word, high order byte     */
        } bytes;

        ulong    b32;                           /*      use as unsigned long            */

        char    *charptr;                               /* as a char  pointer                   */

        SPEC16  *spec16ptr;                             /* as a SPEC16 pointer                  */

} SPEC32;

