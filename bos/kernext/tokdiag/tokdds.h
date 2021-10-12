/* @(#)77	1.12  src/bos/kernext/tokdiag/tokdds.h, diagddtok, bos411, 9428A410j 3/2/92 14:56:12 */

/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS: tokdds.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_TOKDDS
#define _H_TOKDDS

/*****************************************************************************/
   struct offlevel_que_tag { /* structure in DDS for controlling offlevel que*/
      struct intr ihs;       /* kernel-required interrupt handler structure  */
      int         scheduled; /* TRUE if offlevel handler has been i_sched'ed */
      int         running;   /* TRUE if offlevel handler is currently running*/
      int         next_in;   /* index to next insertion point                */
      int         next_out;  /* index to next removal point (empty if == in) */
      offl_elem_t offl_elem[MAX_OFFL_QUEUED]; /* the actual offlevel task que*/
   };
   typedef volatile struct offlevel_que_tag offlevel_que_t;

/*****************************************************************************/
   struct netid_elem_tag {  /* element of netid table attached to end of DDS */
      char	   inuse;   /* This netid in use?			     */
      chan_t       chan;    /* the channel which started this netid          */
      unsigned int faddr;   /* functional address                            */
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
      int            xmit_event;      /* TRUE if any process blocked on xmit */
      int            num_allocates;   /* number of channels allocated (ddmpx)*/
      int            num_opens;       /* number of channels opened (ddopen)  */
      int            mode;            /* device name extension character     */
      int            num_netids;      /* number of netid's currently started */
      netid_elem_t   netid_table[TOK_MAX_NETIDS]; /* common netid table      */
      device_state_t device_state;    /* device (=minor =adapter) state      */
      chan_state_t   chan_state[MAX_OPENS];/* state of each channel          */
      open_elem_t   *open_ptr[MAX_OPENS];  /* ptrs for each channel          */
      int            dump_inited;     /* Dump init flag                      */
      int            dump_started;    /* Dump start flag                     */
      int            dump_read_started; /* flag for receive int. processing  */
      int            dump_pri;        /* Dump interrupt level                */
      int            sav_index;       /* Index to start of used rcv lists    */
      int            dump_first_wrt;  /* Flag for transmit processing        */
      struct in_addr *dmp_ip_addr;    /* local station IP address          */
      recv_list_t   *dump_read_last;  /* Last recv_list for this recv. int.  */
      xmit_des_t    *dump_read_buf[ RCV_CHAIN_SIZE ];  /* array of buffers   */
   } dds_cio_t;

/*****************************************************************************/
   typedef volatile struct {       /* this is it -- the entire DDS */
      struct intr     ihs; /* interrupt handler control struct */
      struct watchdog wdt; /* watchdog timer control struct */
      offlevel_que_t  ofl; /* offlevel handler control and que */
      ddi_t           ddi; /* the ddi as passed to ddconfig */
      tok_vpd_t         vpd;
      query_stats_t   ras; /* ras statistics */
      dds_cio_t       cio; /* common work area */
      dds_wrk_t       wrk; /* device specific work area */
                                   /* xmt que list control is allocated here */
                                   /* netid table is allocated here */
   } dds_t;

/* macros for accessing DDS contents require that the dds pointer be p_dds */
/* example: to access the netid table address use "CIO.netid_table"      */

#define IHS p_dds->ihs
#define WDT p_dds->wdt
#define OFL p_dds->ofl
#define DDI p_dds->ddi
#define VPD p_dds->vpd
#define RAS p_dds->ras
#define CIO p_dds->cio
#define WRK p_dds->wrk

#endif /* ! _H_CIODDS */
