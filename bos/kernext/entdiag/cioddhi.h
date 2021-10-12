/* @(#)94	1.2  src/bos/kernext/entdiag/cioddhi.h, diagddent, bos411, 9428A410j 5/14/93 08:22:43 */
#ifndef _H_CIODDHI
#define _H_CIODDHI

/*
 * COMPONENT_NAME: sysxcio -- Common Communications Code Device Driver Head
 *
 * FUNCTIONS: cioddhi.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************/

#define DISABLE_CL2_INTRS(lvl)  lvl=i_disable(INTCLASS2)
#define DISABLE_INTERRUPTS(lvl) lvl=i_disable(INTMAX)
#define ENABLE_INTERRUPTS(lvl)  i_enable(lvl)
#define KMALLOC(size)           xmalloc ((uint)(size), (uint)0, pinned_heap)
#define KMFREE(ptr)             xmfree ((void *)ptr, pinned_heap)
#define SLEEP(el)               e_sleep ((int *)(el), EVENT_SIGRET)
#define WAKEUP(el)              e_wakeup ((int *)(el))
#define FREEMBUF(o)             m_freem(o)

#   define TRACE1(s)              cio_save_trace((ulong)1,s)
#   define TRACE2(s,a2)           cio_save_trace((ulong)2,s,a2)
#   define TRACE3(s,a2,a3)        cio_save_trace((ulong)3,s,a2,a3)
#   define TRACE4(s,a2,a3,a4)     cio_save_trace((ulong)4,s,a2,a3,a4)
#   define TRACE5(s,a2,a3,a4,a5)  cio_save_trace((ulong)5,s,a2,a3,a4,a5)

#ifdef DEBUG
#   define DTRACE1(s)             cio_save_trace((ulong)1,s)
#   define DTRACE2(s,a2)          cio_save_trace((ulong)2,s,a2)
#   define DTRACE3(s,a2,a3)       cio_save_trace((ulong)3,s,a2,a3)
#   define DTRACE4(s,a2,a3,a4)    cio_save_trace((ulong)4,s,a2,a3,a4)
#   define DTRACE5(s,a2,a3,a4,a5) cio_save_trace((ulong)5,s,a2,a3,a4,a5)
#   define DASSERT(a)		  ASSERT(a)
#else
#   define DTRACE1(s)
#   define DTRACE2(s,a2)
#   define DTRACE3(s,a2,a3)
#   define DTRACE4(s,a2,a3,a4)
#   define DTRACE5(s,a2,a3,a4,a5)
#   define DASSERT(a)
#endif

/*
    Macro to provide permanent trace points for system activity monitoring.
 */

#define TRC_WQUE (0x77515545) /* ciowrite -- write data has just been queued */
#define TRC_WBEG (0x77424547) /* xxx_xmit or xxx_offl -- data dequeued to adp*/
#define TRC_WEND (0x77454E44) /* cio_xmit_done -- write is complete          */
#define TRC_RDAT (0x72444154) /* xxx_offl -- a packet was received           */
#define TRC_RNOT (0x724E4F54) /* cio_proc_recv -- kernel proc passed data    */
#define TRC_RQUE (0x72515545) /* cio_proc_recv -- read data queued for user  */
#define TRC_ROVR (0x724F5652) /* cio_proc_recv -- user proc read queue ovfl  */
#define TRC_REND (0x72454E44) /* cioread -- data delivered to user proc      */

#define AIXTRACE(tp,mm,ch,mb,ln) {                                      \
   TRCHKGT (TRACE_HOOKWORD | HKTY_GT | 0,                               \
      (ulong)(tp), (ulong)(mm), (ulong)(ch), (ulong)(mb), (ulong)(ln)); \
}

/*
   The result of using this macro is that XXX_timeout_func is entered after
   ms milliseconds.  XXX_timeout_func then queues an offlevel call with a
   who_queued value of OFFL_TIMO.  Only one timeout request can be in effect
   for each DDS at one time using this macro since only one timer structure
   has been set up within each DDS.
   It is the intent of this macro that it be used to simulate a hardware
   interrupt in those cases where an actual interrupt can not be expected
   (for example, during some phases of adapter initialization).  Note that
   there is a separate mechanism for a watchdog timer (for use when a hardware
   interrupt is expected but may not occur due to some error condition).
*/
#define QUEOFFLMS(ms) {                                                  \
   CIO.systimer_ptr->timeout.it_value.tv_sec  =  (ms) / 1000;            \
   CIO.systimer_ptr->timeout.it_value.tv_nsec = ((ms) % 1000) * 1000000; \
   tstart(CIO.systimer_ptr);                                             \
}

/*
 * Macros to get/put caller's parameter block when only an address is supplied.
 * Useful for "arg" in ioctl and for extended paramters on other dd entries.
 * Value is 0 if ok, otherwise see errno.h.  Typical usage is:
 * if (rc = COPYIN (devflag, arg, &localdata, sizeof(localdata)))
 *    return (rc);
 */
#define COPYIN(dvf,usa,dda,siz)                               \
( ((char *)usa == (char *)NULL) ? (EFAULT) :                  \
      ( (dvf & DKERNEL) ? (bcopy(usa,dda,siz), 0) :           \
            ( ((rc = copyin(usa,dda,siz)) != 0) ? rc : 0 ) ) )

#define COPYOUT(dvf,dda,usa,siz)                              \
( ((char *)usa == (char *)NULL) ? (EFAULT) :                  \
      ( (dvf & DKERNEL) ? (bcopy(dda,usa,siz), 0) :           \
            ( ((rc = copyout(dda,usa,siz)) != 0) ? rc : 0 ) ) )


/*****************************************************************************/
/* This macro determines if the data portion of an mbuf resides within one   */
/* page -- if TRUE is returned, the data does not cross a page boundary.  If */
/* FALSE is returned, the data does cross a page boundary and cannot be      */
/* d_matered.                                                                */
/*****************************************************************************/
#define M_INPAGE(m)                                             \
((((int)MTOD((m), uchar *) & ~(PAGESIZE - 1)) + PAGESIZE) >=    \
   ((int)MTOD((m), uchar *) + (m)->m_len))

/*****************************************************************************/
   struct sll_elem_tag {        /* used to implement singly-linked list mgmt */
      struct sll_elem_tag *next;     /* all elements must start w this item  */
   };
   typedef volatile struct sll_elem_tag  sll_elem_t;
   typedef sll_elem_t                   *sll_elem_ptr_t;

/*****************************************************************************/
   struct s_link_list_tag {    /* control structure for managing linked lists*/
      int          num_free;   /* number of elements linked on free list     */
      int          num_elem;   /* number of elements linked on active list   */
      int          max_elem;   /* maximum elements allowed (initial num_free)*/
      int          elem_size;  /* the size of one element                    */
      ulong       *hwm_ptr;    /* pointer to high water mark (max queued)    */
      sll_elem_ptr_t area_ptr; /* address of area where elements allocated   */
      sll_elem_ptr_t limt_ptr; /* address just past area of elements         */
      sll_elem_ptr_t free_ptr; /* pointer to first free element (if any)     */
      sll_elem_ptr_t head_ptr; /* pointer to first active element (if any)   */
   };
   typedef volatile struct s_link_list_tag s_link_list_t; /* each que has one*/

/*****************************************************************************/


/*****************************************************************************/
   struct rec_elem_tag {              /* a receive queue element (user proc) */
      struct rec_elem_tag  *next;     /* must be first for list management   */
      struct mbuf          *mbufp;    /* pointer to mbuf with data           */
      int                   bytes;    /* total number of bytes of data       */
      cio_read_ext_t        rd_ext;   /* complete extension to be returned   */
   };
   typedef volatile struct rec_elem_tag rec_elem_t; /* each read has one     */

/*****************************************************************************/
   struct sta_elem_tag {              /* a status queue element (user proc)  */
      struct sta_elem_tag  *next;     /* must be first for list management   */
      cio_stat_blk_t        stat_blk; /* the status block to be returned     */
   };
   typedef volatile struct sta_elem_tag sta_elem_t; /* each stat blk has one */

/*****************************************************************************/
   typedef enum {
      DEVICE_NOT_CONN = 0,/* not connected - initial state and after dsinact */
      DEVICE_CONN_IN_PROG,/* connection in progress -set just before XXXdsact*/
      DEVICE_CONNECTED,   /* connected - set by XXX_conn_done                */
      DEVICE_DISC_IN_PROG /* disconn in progress - set just before XXXdsinact*/
   } device_state_t;

/*****************************************************************************/
   typedef enum {
      CHAN_AVAIL = 0,                /* ok to allocate (in ddmpx)            */
      CHAN_ALLOCATED,                /* ok to open (in ddopen)               */
      CHAN_OPENED,                   /* ok to read/write/ioctl/select/close  */
      CHAN_CLOSED                    /* ok to deallocate (in ddmpx)          */
   } chan_state_t;

   struct open_elem_tag {
      int           alloc_size;    /* amount of memory for XXX_del_cdt       */
      dev_t         devno;         /* our own devno                          */
      chan_t        chan;          /* our own channel number                 */
      ulong         devflag;       /* flags from open call                   */
      int           xmt_event;     /* event list for e_sleep during write    */

      ulong         open_id;       /* from open extensn of kernel open only  */
      void        (*rec_fn)();     /* address of kproc rec "interrupt" entry */
      void        (*sta_fn)();     /* address of kproc sta "interrupt" entry */
      void        (*xmt_fn)();     /* address of kproc xmt "interrupt" entry */
      int           xmt_fn_needed; /* TRUE if this kernel user needs xmt_fn  */

      s_link_list_t rec_que;       /* rec queue managemt for user open only  */
      s_link_list_t sta_que;       /* sta queue managemt for user open only  */
      int           sta_que_ovrflw;/* the status que overflowed              */
      int           selectreq;     /* flags indicating selnotify requested   */
      int           rec_event;     /* event list for e_sleep during read     */

      /* if user process open, then rec que and sta que are allocated here   */

      int           prom_on_cnt; /* no of promisc. on issued by this open    */
      int           badframe_on_cnt; /* no of badfr. issued by this open     */
   };
   typedef volatile struct open_elem_tag open_elem_t; /* each open adds one  */

#endif /* ! _H_CIODDHI */
