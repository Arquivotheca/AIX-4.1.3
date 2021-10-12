/* @(#)33  1.9  src/bos/kernext/ient/i_ent.h, sysxient, bos411, 9428A410j 6/10/94 19:03:32 */
/*****************************************************************************/
/*
 *   COMPONENT_NAME: SYSXIENT
 *
 *   FUNCTIONS: NDD_TO_DEV_CTL
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************/
#ifndef _H_IENT
#define _H_IENT

/*****************************************************************************/
/*           trace table and component dump table                            */
/*****************************************************************************/

struct ient_trace_tag
{
   int        next_entry;                     /* index to put data next */
   int        res1;
   int        res2;
   int        res3;
   ulong      table[TRACE_TABLE_SIZE];        /* trace table */
};

typedef struct ient_trace_tag ient_trace_t;

#define MAX_CDT_ENTRIES 4

typedef struct
{
   struct cdt_head  head;                     /* cdt head                   */
   struct cdt_entry entry[MAX_CDT_ENTRIES];   /* cdt entries                */
} ient_cdt_t;

/*****************************************************************************
 * The following structures are malloced as required and are used to
 *   store multicast addresses set through lsa_cfg.  Each element contains
 *   a reference count to allow overlapping address ranges.
 ****************************************************************************/
typedef struct nad
{
    int    count;                   /* count of enables                  */
    struct nad   *link;             /* link to the next nad              */
    char   nadr[ENT_NADR_LENGTH];   /* a net address                     */
} nad_t;

typedef enum                        /* Device status list.               */
{
    CLOSED = 0,                     /* initial device state              */
    OPEN_PENDING,                   /* open initiated                    */
    OPENED,                         /* opened successfully, functioning  */
    CLOSE_PENDING,                  /* close initiated                   */
    LIMBO,                          /* error recovery period             */
    DEAD                            /* fatal hardware error encountered  */
} device_state_t;

typedef enum                        /* Who set the watchdog timer        */
{
    WDT_INACTIVE,                   /* WDT is inactive                   */
    WDT_CONNECT,                    /* dsact in progress                 */
    WDT_XMIT,                       /* xmit in progress                  */
    WDT_ACT,                        /* action cmd in progress            */
    WDT_CLOSE,                      /* Adapter close in progress         */
    WDT_CFG,                        /* configure action command          */
    WDT_IAS                         /* Individual Address setup act cmd  */
} wdt_setter;


/*****************************************************************************/
/*                  Work section of DDS                                      */
/*****************************************************************************/

volatile struct ient_wrk
{
    wdt_setter  wdt_setter;           /* starter of watch dog timer        */
    ulong    connection_result;       /* start done status                 */
    ulong    channel_allocated;       /*  DMA channel allocated            */
    int      timeout;                 /* xmit timeout condition            */

    lock_t   lock_anchor;             /* serialization lock for IOCTLs     */
    int      sleep_anchor;            /* event word for sleeps             */
    int      ndd_stime;               /* timestamps                        */
    int      dev_stime;

    int      dma_channel;             /* for use with DMA services d_xxxx  */
    int      close_event;             /* information for sync close        */
    int      dump_started;            /* flag for controlling dumps        */
    ulong    dump_dsap;               /* only pass this dsap during dump   */

    struct   xmem    xbuf_xd;         /* cross-memory descriptor           */

    struct   scp     *scp_ptr;        /* pointer to the SCP                */
    struct   iscp    *iscp_ptr;       /* pointer to the ISCP               */
    struct   scb     *scb_ptr;        /* pointer to the SCB                */
    struct   xcbl    *xcbl_ptr;       /* pointer to the xmit CBL           */

    struct   acbl    *acbl_ptr;       /* pointer to the action CBL         */
    struct   acbl    *ncbl_ptr;       /* pointer to the dedicated noop CBL */
    struct   scp     *scp_addr;       /* bus address of the SCP            */
    struct   iscp    *iscp_addr;      /* bus address of the ISCP           */

    struct   scb     *scb_addr;       /* bus address of the SCB            */
    struct   xcbl    *xcbl_addr;      /* bus address of the xmit CBL       */
    struct   acbl    *acbl_addr;      /* bus address of the action CBL     */
    struct   acbl    *ncbl_addr;      /* bus address of the noop CBL       */

    ulong    sysmem;                  /* start of allocated system memory  */
    ulong    sysmem_end;              /* end of allocated system memory    */
    ulong    asicram;                 /* start of allocated ASIC RAM       */
    ulong    asicram_end;             /* end of allocated ASIC RAM         */

    ulong    dma_base;                /* beginning of our IO address space */
    ulong    alloc_size;              /* size of allocated system memory   */
    ulong    control_pending;         /* a control command is pending      */
    ulong    action_que_active;       /* true when scb pts to action que   */

    struct   rfd      *rfd_ptr;       /* pointer to the RFD array          */
    struct   rfd      *rfd_addr;      /* chip address of RFD array         */
    struct   rbd      *rbd_ptr;       /* pointer to the RBD array          */
    struct   rbd      *rbd_addr;      /* chip address of RFD array         */

    struct   rbd      *rbd_el_ptr;    /* addrsss of RBD with the EL bit set*/
    ulong    save_bf;                 /* save bad rcv frames configuration */
    ulong    begin_fbl;               /* 1st free recv buffer in the list  */
    ulong    end_fbl;                 /* last free recv buffer in the list */

    ulong    buffer_in;               /* next free xmit buffer             */
    ulong    buffer_out;              /* current xmit buffer in intr.      */
    ulong    el_ptr;                  /* index of the RFD with the EL bit  */
    ulong    recv_buffers;            /* number of recv buffers allocated  */

    struct   recv_buffer *recv_buf_ptr;  /* pointer to receive buffers     */
    struct   recv_buffer *recv_buf_addr; /* bus address of the RFA         */
    struct   xmit_buffer *xmit_buf_ptr;  /* pointer to xmit buffers        */
    struct   xmit_buffer *xmit_buf_addr; /* bus address of xmit buffers    */

    struct   mbuf    *txq_first;      /* ptr to first on xmit queue        */
    struct   mbuf    *txq_last;       /* ptr to last mbuf on xmit queue    */
    struct   tbd     *tbd_ptr;        /* ptr to TBD array                  */
    struct   tbd     *tbd_addr;       /* chip address of TBD array         */

    ulong    aram_mask;               /* mask for ASIC RAM addresses       */
    ulong    xmits_queued;            /* true when element on waitq        */
    int      xmits_buffered;          /* number of xmits in buffer ring    */
    int      xmits_started;           /* number of CU xmit starts          */ 

    uchar    ent_addr[ENT_NADR_LENGTH];  /* actual net addr in use         */
    uchar    pos_reg[8];                 /* POS register values            */
    uchar    reserved[2];             /* Padding for word alignment        */

    ulong    promiscuous_count;       /* Current nbr of promiscuous users  */
    ulong    badframe_user_count;     /* Current nbr of badframe users     */
    ulong    alt_count;               /* number of alternate addresses     */
    ulong    multi_count;             /* number of all-multicast sets      */

    nad_t    *alt_list;               /* pointer to alt address list       */
    ushort   restart;                 /* TRUE ==> reinitialize the adapter */
    ushort   restart_ru;              /* TRUE ==> reinitialize the RU      */
    ulong    machine;                 /* HW platform                       */
    ulong    do_ca_on_intr;           /* Rainbow hack.                     */

    int      intr_registered;         /* interrupt registered              */
    int      timr_registered;         /* watchdog timer registered         */

    struct   cfg    cur_cfg;          /* Current adapter configuration.    */
    ushort   reserved1;               /* Pad out to word boundary          */
};

typedef volatile struct ient_wrk ient_wrk_t;

/***************************************************************************/
/*
 * This is the whole device control area
 */
/*****************************************************************************/
volatile struct ient_dev_ctl          /* This is it -- the entire DDS      */
{
    struct intr ihs_section;          /* Interrupt handler control struct  */
    ndd_t  ndd_section;               /* NDD for NS ndd chain              */
    int    seq_number;                /* Sequence number                   */
    device_state_t  device_state;     /* Device (=minor =adapter) state    */
    ient_dds_t      dds_section;      /* The ddi as passed to ddconfig     */ 
    struct  watchdog wdt_section;     /* Watchdog timer control struct     */
    ent_genstats_t  entstats;         /* Ethernet generic statistics       */
    ient_stats_t  devstats;           /* Integrated spcific statistics     */
    ethernet_all_mib_t mibs;          /* Ethernet MIB's                    */
    ient_wrk_t wrk_section;           /* Device specific work area         */
    ient_cdt_t   cdt;                 /* Component dump table              */
};

typedef volatile struct ient_dev_ctl ient_dev_ctl_t;


/*
** macros for accessing DDS contents require that the dds pointer be p_dev_ctl
*/
#define IHS        p_dev_ctl->ihs_section
#define NDD        p_dev_ctl->ndd_section
#define WDT        p_dev_ctl->wdt_section
#define DDS        p_dev_ctl->dds_section
#define WRK        p_dev_ctl->wrk_section
#define MIB        p_dev_ctl->mibs
#define ENTSTATS   p_dev_ctl->entstats
#define DEVSTATS   p_dev_ctl->devstats
#define CDT        p_dev_ctl->cdt

#define NDD_TO_DEV_CTL(n) ((ient_dev_ctl_t *)((ulong)n - sizeof(struct intr)))

#endif /* _H_IENT */
