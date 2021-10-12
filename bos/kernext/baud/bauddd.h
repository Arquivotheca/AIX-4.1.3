/* ?? */
/*
** BAUD Device Driver Structures and Definitions
**
**
** (C) Copyright IBM Corp., 1994
** All rights reserved.
** U.S. Government Users Restricted Rights -- Use, duplication,
** or disclosure restricted by GSA ADP Schedule Contract with
** IBM Corp.
** Program Property of IBM.
**
*/


/*
** BAUD Device Driver Device Dependent Structure
*/
struct baud_dds {
    ulong  bus_id;           /* This is bus id obtained from the parent */
    ushort slotno;           /* Slot number in MicroChannel */
    ushort intr_priority;    /* Interrupt priority */
    ushort cap_dma_lvl;      /* DMA level */
    ushort play_dma_lvl;     /* DMA level */
    ushort bus_intr_lvl;     /* Bus Interrupt Level */
    ulong  bus_io_addr;      /* Bus I/O Address */
    ulong  bus_mem_addr;     /* Bus Memory Address */
    int    channel_id_rd;    /* DMA id returned from d_init() */
    int    channel_id_wr;    /* DMA id returned from d_init() */
    int    capture_busy;     /* DMA id returned from d_init() */
    int    playback_busy;    /* DMA id returned from d_init() */
    adspace_t *adsp;         /* pointer to memory struct */
    char   *bus_segr;        /* Segment/pointer to io bus of adapter */
    char   *pos_segr;        /* Segment/pointer to pos regs of adapter */
    ulong  window_size;      /* DMA Window Size */
    char   lname[8];         /* Logical Name */
};

/* IOCTL Operation Codes */
#define    READ_ALL_DEVICE_IDS    10
#define    READ_ALL_POS_REGISTERS 20
#define    READ_DEVID_AT_SLOT     30
#define    READ_POS_REG_AT_SLOT   40
#define    GET_PTR_TO_IOCC        50
#define    REL_PTR_TO_IOCC        55
#define    GET_PTR_TO_REGS        60
#define    REL_PTR_TO_REGS        65
#define    CAPTURE_DMA_STATUS     70
#define    PLAYBACK_DMA_STATUS    80

/* Magic numbers */
#define    IOCC_SEG_ADR         0x820C00e0
#define    CARD_SEG_ADR         0x820C0020

#ifdef _KERNEL

#define POSBASE            0x400000 /* IOCC Address of POS Registers */
#define POS_PORT0_CLOCK    (4)      /* Port clocks for Fake Card */
#define POS_PORT1_CLOCK    (14)
#define BAUD_CARD_ID       0xE5DF   /* POS ID of Fake card */
#define CARD_ADDRESS       0x5      /* Card Address */
#define CARD_NUMBER        0x2a0    /* Card Value */

/*
** PIO Operation (pio_parms.op) Values
*/
#define    WBYTE        1    /* Write byte (8-bits) */
#define    RBYTE        2    /* Read byte (8-bits) */
#define    WSHORT       3    /* Write short (16-bits */
#define    RSHORT       4    /* Read short (16-bits) */
#define    WLONG        5    /* Write long (16-bits */
#define    RLONG        6    /* Read long (16-bits) */
#define    BLKXFER      7    /* Block Data Transfer */
#define    ONE_PAGE     1    /* Number of pages to DMA xfer to */



/*
** FAKE Device Driver Locking Strategies Structure
*/
#define NCHANS 3
#define NADAPTERS 9

typedef struct channel {        /* Finest Granularity Lock - PORT */
    lock_t chan_lock;
} channel_t;

typedef struct adapter {        /* Finer Granularity Lock - ADAPTER */
    channel_t channels[NCHANS];
    lock_t adapter_lock;
} adapter_t;

typedef struct driver {         /* Fine Granularity Lock - DRIVER */
    adapter_t adapt[NADAPTERS];
    lock_t    driver_lock;
} driver_t;

/*
** BAUD Device Driver Information
*/
struct baud_control {
    struct intr baud_intr;      /* Interrupt Structure for interrupt handler */
    struct watchdog baud_watch; /* Watchdog timer structure */
    struct trb *ftrbp;           /* Fine granularity timer structure */
    struct baud_dds fdds;       /* Device Dependent Structure */
    lock_t ctrl_lock;            /* Lock word for this control structure */
    struct {                     /* Cross Memory Xfer Control Buffer */
        caddr_t bufaddr;         /* Pointer to data buffer */
        ulong buflen;            /* Length of data transfer */
        struct xmem dp;          /* Cross Memory descriptor */
        caddr_t UsrSeg;          /* Save segment register for intr handler */
    } cxfer_buf;
    struct {                     /* Cross Memory Xfer Control Buffer */
        caddr_t bufaddr;         /* Pointer to data buffer */
        ulong buflen;            /* Length of data transfer */
        struct xmem dp;          /* Cross Memory descriptor */
        caddr_t UsrSeg;          /* Save segment register for intr handler */
    } pxfer_buf;
    struct {                     /* PIO parameters */
        caddr_t data;            /* Pointer to data buffer */
        ushort  size;            /* 64k Max data transfer */
        char    op;              /* Operation: WBYTE, RBYTE, etc. */
        char    reserved;        /* Reserved Space */
        ulong   io_offset;       /* Offset from base address */
        caddr_t eaddr;           /* Effective address from attach macro */
    } pio_parms;
    ulong tcw_start_addr;        /* TCWs starting address */
    ulong resid;                 /* Amount of data actually transferred */
    struct pio_parms *piop;      /* Programmable I/O parameters */
    int   bytes_in;              /* Number of bytes xfer'd from adapter */
    int   bytes_out;             /* Number of bytes xfer'd to adapter */
    int   sleep_anchor;          /* Event word for esleep() */
    int   xfer_error;            /* Cross Memory transfer error */
    ulong adapt_ref_cnt;         /* Is adapter already setup? */
};

struct baud_cdt_tab {           /* Component dump table information */
    struct cdt_head baud_cdt_head;
    struct cdt_entry baud_entry;
};

typedef volatile struct {
    ulong configured;            /* Has this driver been configured? */
    ulong num_dds;               /* Number of devices installed */
    struct baud_control *baudcp[NADAPTERS];
    ulong reserved;              /* Reserved for future use */
} baud_ctrl_t;

/*
 * BAUD Device Driver Trace Macros and Definitions
 */

#define FAKEHKWD1(hkid,loc,errno,data) \
    (TRCHKLT( (hkid|(loc<<8)|errno), data ))
#define FAKEHKWD2(hkid,loc,errno,data1,data2) \
    (TRCHKL2T( (hkid|(loc<<8)|errno), data1, data2 ))
#define FAKEHKWD3(hkid,loc,errno,data1,data2,data3) \
    (TRCHKL3T( (hkid|(loc<<8)|errno), data1, data2, data3 ))
#define FAKEHKWD4(hkid,loc,errno,data1,data2,data3,data4) \
    (TRCHKL4T( (hkid|(loc<<8)|errno), data1, data2, data3, data4 ))
#define FAKEHKWD5(hkid,loc,errno,data1,data2,data3,data4,data5) \
    (TRCHKGT( (hkid|(loc<<8)|errno), data1,data2,data3,data4,data5 ))
#define FAKEHKGEN(hkid,devno,devln,devnm) \
    (TRCGENKT(0,hkid,devno,devln,devnm))

#define HKWD_FAKE_DD        0x01000000    /* FAKE hook id */

#define FAKE_ENTRY_OPEN      0x01
#define FAKE_EXIT_OPEN       0x02
#define FAKE_ENTRY_CLOSE     0x03
#define FAKE_EXIT_CLOSE      0x04
#define FAKE_ENTRY_READ      0x05
#define FAKE_EXIT_READ       0x06
#define FAKE_ENTRY_WRITE     0x07
#define FAKE_EXIT_WRITE      0x08
#define FAKE_ENTRY_CONFIG    0x09
#define FAKE_EXIT_CONFIG     0x0a
#define FAKE_ENTRY_WATCHDOG  0x0b
#define FAKE_EXIT_WATCHDOG   0x0c
#define FAKE_ENTRY_MPX       0x0d
#define FAKE_EXIT_MPX        0x0e
#define FAKE_ENTRY_IOCTL     0x0f
#define FAKE_EXIT_IOCTL      0x10
#define FAKE_ERROR_RPT       0x11

/* The following error ids were obtained from the system and added here. */

#define ERRID_FAKEDD_DEVSW   0x3097cbae
#define ERRID_FAKEDD_USER    0x0069985c


#endif /* _KERNEL */

#ifndef _KERNEL

#define HKWD_FAKE_TST       0x01100000    /* Application hook id */
#define FTESTHKWD(hkid, loc) TRCHKLT((hkid|(loc<<8)|0), 0)

#define APPL_CALLING_OPEN    0x01
#define APPL_CALLING_CLOSE   0x02
#define APPL_CALLING_READ    0x03
#define APPL_CALLING_WRITE   0x04
#define APPL_CALLING_IOCTL   0x05

#endif /* _KERNEL */

#ifdef DEBUG
#define DEBUG1(a)    printf(a)
#define DEBUG2(a,b)    printf(a,b)
#define DEBUG3(a,b,c)    printf(a,b,c)
#define DEBUG4(a,b,c,d)    printf(a,b,c,d)
#else
#define DEBUG1(a)
#define DEBUG2(a,b)
#define DEBUG3(a,b,c)
#define DEBUG4(a,b,c,d)
#endif

