/* @(#)64   1.6  src/bos/kernext/inputdd/inc/common.h, inputdd, bos41J, 9519A_all 5/9/95 07:22:40  */
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard/Tablet/Sound/Mouse DD - common.h
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************/
/* common data structure                                                     */
/*****************************************************************************/

typedef  unsigned char IFRAME;         /* input data frame (from device)     */
typedef  unsigned short OFRAME;        /* output data frame  (to device)     */

struct common {

   struct  intr   intr_data;           /* slih interrupt struct              */

   ulong   bus_id;                     /* bus ID                             */
   ulong   slot_addr;                  /* slot address                       */
   ulong   bus_addr;                   /* port address                       */

   struct  trb  * wdtimer;             /* watch dog timer                    */
   struct  trb  * stimer;              /* wait on I/O timer                  */

   dev_t   adpt_devno;                 /* devno of adapter                   */

   int     intr_level;                 /* interrupt level                    */
   int     intr_priority;              /* interrupt priority                 */
   int     retry_cnt;                  /* I/O retry counter                  */
   int     rcv_err_cnt;                /* receive error count                */
   int     asleep;                     /* sleep anchor                       */

   OFRAME  cur_frame;                  /* frame last xmit'ed                 */

   ushort   in_progress;               /* work in progress flag              */
#define NO_OP                 0        /*   none                             */
#define KBD_ACK_RQD           0x0001   /*   waiting for kbd ACK              */
#define TAB_IN_PROG           0x0002   /*   waiting for tablet ack           */
#define ADPT_ACK_RQD          0x0004   /*   waiting for adapter ack          */
#define MSE_ACK_RQD           0x0008   /*   waiting for mouse ack            */
#define RCV_KBD               0x0010   /*   waiting for keyboard data        */
#define RCV_TAB               0x0020   /*   waiting for tablet data          */
#define RCV_MSE               0x0040   /*   waiting for mouse data           */
#define ADPT_BAT              0x0080   /*   waiting for BAT from adapter     */
#define UNLOCK_WAIT           0x0100   /*   waiting for adapter to unlock    */
#define FAILED_OP             0x4000   /*   last operation failed            */
#define OP_RCV_ERR            0x8000   /*   rcv error has occurred during    */
                                       /*     operation (or'ed on to others) */
                                       /*   Waiting for some kind of ACK     */
#define ACK_RQD     (KBD_ACK_RQD | TAB_IN_PROG | ADPT_ACK_RQD | MSE_ACK_RQD)
                                       /*   Waiting to receive data          */
#define RCV_MODE    (RCV_KBD | RCV_TAB | RCV_MSE | ADPT_BAT)

   uchar   perr;                       /* TRUE if permanent I/O err occurred */

#ifdef PM_SUPPORT
   uchar   pm_in_progress;             /* TRUE if pm event is being processed*/
   uchar   pm_registered;              /* TRUE if pm handle structure is     */
                                       /* registered to pm core              */
#endif /* PM_SUPPORT */

   struct {                            /* input queue                        */
      IFRAME  *head;                   /*   de-queue pointer                 */
      IFRAME  *tail;                   /*   en-queue pointer                 */
      IFRAME  elements[10];            /*   queue                            */
   } inq;

   struct {                            /* output queue                       */
      OFRAME  *head;                   /*   xmit group start ptr             */
      OFRAME  *tail;                   /*   en-queue pointer                 */
      OFRAME  *dq_ptr;                 /*   de-queue pointer                 */
      uchar   out_cnt;                 /*   xmit group frame count           */
      uchar   error;                   /*   error flag                       */
      OFRAME  elements[32];            /*   queue                            */
   } outq;

};

/*****************************************************************************/
/* global proto types and references                                         */
/*****************************************************************************/

/* functions                                                                 */
extern void   ktsm_sleep(struct common *, ulong);
extern void   ktsm_wakeup(struct trb *);
extern void   ktsm_putring(struct rcb *, struct ir_report *, void *);
extern void   ktsm_xcpo(caddr_t, caddr_t, uchar, struct rcb *);
extern void   ktsm_xcpi(caddr_t, caddr_t, uchar, struct rcb *);
extern void   ktsm_rflush(struct common *, struct rcb *);
extern void   _vsprintf(char *, char *, va_list);
extern void   ktsm_log(char *, char *, uint, uint, char *);
extern void   ktsm_uring(struct rcb *);
extern int    ktsm_rring(struct rcb *, char *);
extern void   put_iq(struct common *, IFRAME);
extern void   put_oq1(struct common *, OFRAME);
extern void   put_oq2(struct common *, OFRAME, OFRAME);
extern void   put_oq(struct common *, OFRAME *);
extern OFRAME get_oq(struct common *);
extern void   flush_q(struct common *);
extern void   startwd(struct common *, void *, ulong);
extern int    get_iq(struct common *, int , ulong, IFRAME *);
extern int    wait_oq(struct common *com);

/* driver specific routines                                                  */
extern void   io_error(char *, int, uint, uint, char *, ...);
extern int    reg_intr(int);
