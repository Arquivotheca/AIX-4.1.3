/* @(#)51  1.3.1.1  src/bos/kernext/ientdiag/i_ciodds.h, diagddient, bos411, 9428A410j 11/10/93 14:09:31 */
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
      int            intr_registered; /* interrupt registered                */
      int            timr_registered; /* watchdog timer registered           */
      int            xmt_fn_needed;   /* TRUE if any kernel proc needs tx_fn */
      int            num_allocates;   /* number of channels allocated (ddmpx)*/
      int            num_opens;       /* number of channels opened (ddopen)  */
      int            mode;            /* device name extension character     */
      int            num_netids;      /* number of netid's currently started */
      int            cls_event;       /* event list for e_sleep during close */
      netid_elem_t  *netid_table_ptr; /* common netid table                  */
      s_link_list_t  xmt_que;         /* common xmt queue                    */
      device_state_t device_state;    /* device (=minor =adapter) state      */
      chan_state_t   chan_state[MAX_OPENS];/* state of each channel          */
      open_elem_t   *open_ptr[MAX_OPENS];  /* ptrs for each channel          */
   } dds_cio_t;

/*****************************************************************************/
   typedef volatile struct {       /* this is it -- the entire DDS           */
      struct intr     	ihs_section; /* interrupt handler control struct     */
      struct watchdog 	wdt_section; /* watchdog timer control struct        */
      ddi_t           	ddi_section; /* the ddi as passed to ddconfig        */
      ccc_vpd_blk_t  	vpd_section; /* vital product data                   */
      ent_query_stats_t ras_section; /* ras statistics                       */
      dds_cio_t       cio_section; /* common work area                       */
      dds_wrk_t       wrk_section; /* device specific work area              */
                                   /* xmt que list control is allocated here */
                                   /* netid table is allocated here          */
   } dds_t;

/* macros for accessing DDS contents require that the dds pointer be dds_ptr */
/* example: to access the netid table address use "CIO.netid_table_ptr"      */

#define IHS dds_ptr->ihs_section
#define WDT dds_ptr->wdt_section
#define DDI dds_ptr->ddi_section
#define VPD dds_ptr->vpd_section
#define RAS dds_ptr->ras_section
#define CIO dds_ptr->cio_section
#define WRK dds_ptr->wrk_section

#endif /* ! _H_CIODDS */
