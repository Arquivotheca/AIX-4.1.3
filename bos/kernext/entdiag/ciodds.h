/* @(#)97	1.1.2.3  src/bos/kernext/entdiag/ciodds.h, diagddent, bos411, 9428A410j 5/14/93 08:34:38 */
#ifndef _H_CIODDS
#define _H_CIODDS

/*
 * COMPONENT_NAME: sysxcio -- Common Communications Code Device Driver Head
 *
 * FUNCTIONS: ciodds.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*****************************************************************************/
   enum pend {
      pend_none   = 0x0000,
      pend_rx     = 0x0001,
      pend_rx_err = 0x0002,
      pend_tx     = 0x0010,
      pend_tx_err = 0x0020,
      pend_ex     = 0x0100
   };

   struct offlevel_que_tag { /* structure in DDS for controlling offlevel que*/
      struct intr ihs;       /* kernel-required interrupt handler structure  */
      int         scheduled; /* TRUE if offlevel handler has been i_sched'ed */
      int         next_in;   /* index to next insertion point                */
      int         next_out;  /* index to next removal point (empty if == in) */
      enum pend   pending;   /* pend interrupts for off level	   */
      offl_elem_t offl_elem[MAX_OFFL_QUEUED]; /* the actual offlevel task que*/
   };
   typedef volatile struct offlevel_que_tag offlevel_que_t;

/*****************************************************************************/
   struct netid_elem_tag {  /* element of netid table attached to end of DDS */
      ulong   ds;           /* device-specific information                   */
      chan_t  chan;         /* the channel which started this netid          */
      ushort  length;       /* number of bytes in netid                      */
      netid_t netid;        /* the network session identifier                */
   };
   typedef volatile struct netid_elem_tag netid_elem_t;

/*****************************************************************************/
   typedef struct {                   /* common work section of the DDS      */
      int            alloc_size;      /* amount of memory for XXX_del_cdt    */
      dev_t          devno;           /* our own devno                       */
      struct trb    *systimer_ptr;    /* system timer pointer                */
      int            intr_registered; /* interrupt registered                */
      int            timr_registered; /* watchdog timer registered           */
      int            xmt_fn_needed;   /* TRUE if any kernel proc needs tx_fn */
      int	     xmit_event;      /* TRUE if any process blocked on xmit */
      int            num_allocates;   /* number of channels allocated (ddmpx)*/
      int            num_opens;       /* number of channels opened (ddopen)  */
      int            mode;            /* device name extension character     */
      int            num_netids;      /* number of netid's currently started */
      int            cls_event;       /* event list for e_sleep during close */
      netid_elem_t  *netid_table_ptr; /* common netid table                  */
      device_state_t device_state;    /* device (=minor =adapter) state      */
      chan_state_t   chan_state[MAX_OPENS];/* state of each channel          */
      open_elem_t   *open_ptr[MAX_OPENS];  /* ptrs for each channel          */
      int            badframe_count;  /* number of bad frames on issued      */
      int            promiscuous_count;/* number of promiscuous on issued    */
   } dds_cio_t;


/*****************************************************************************/
   typedef volatile struct {       /* this is it -- the entire DDS           */
      struct intr     ihs_section; /* interrupt handler control struct       */
      offlevel_que_t  ofl_section; /* offlevel handler control and que       */
      ddi_t           ddi_section; /* the ddi as passed to ddconfig          */
      ccc_vpd_blk_t   vpd_section; /* vital product data                     */
      query_stats_t   ras_section; /* ras statistics                         */
      dds_cio_t       cio_section; /* common work area                       */
      dds_wrk_t       wrk_section; /* device specific work area              */
                                   /* xmt que list control is allocated here */
                                   /* netid table is allocated here          */
   } dds_t;

/* macros for accessing DDS contents require that the dds pointer be dds_ptr */
/* example: to access the netid table address use "CIO.netid_table_ptr"      */

#define IHS dds_ptr->ihs_section
#define WDT dds_ptr->wrk_section.wdt_section
#define OFL dds_ptr->ofl_section
#define DDI dds_ptr->ddi_section
#define VPD dds_ptr->vpd_section
#define RAS dds_ptr->ras_section
#define CIO dds_ptr->cio_section
#define WRK dds_ptr->wrk_section

/*****************************************************************************/
typedef volatile struct {
   int       initialized;          /* device driver initialized?             */
   int       num_dds;              /* number of dds's currently have         */
   dev_t     dev_no[MAX_ADAPTERS_INDEX]; /* content minor numbers                  */
   dds_t *dds_ptr[MAX_ADAPTERS_INDEX];   /* dds pointers for each adapter          */
} dd_ctrl_t;


#endif /* ! _H_CIODDS */



