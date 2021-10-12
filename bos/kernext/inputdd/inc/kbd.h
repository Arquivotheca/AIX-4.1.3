/* @(#)65   1.4  src/bos/kernext/inputdd/inc/kbd.h, inputdd, bos41J, 9510A_all 3/7/95 09:54:46  */
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard/Sound DD - kbd.h
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

#include "ktsm.h"
#include "common.h"
#include "kbdext.h"
#ifdef PM_SUPPORT
#include <sys/pm.h>
#include <sys/pmdev.h>
#endif /* PM_SUPPORT */

/*****************************************************************************/
/* local storage data structure                                              */
/*****************************************************************************/


struct local {
   struct  common com;                 /* common data                        */
   struct  kbdext key;                 /* keyboard extension                 */
   struct  trb  * sdd_time_ptr ;       /* sound duration timer               */
#ifdef PM_SUPPORT
   struct  pm_handle pm;               /* pm handle structure                */
#endif /* PM_SUPPORT */

   int sq_head;                        /* index of next sound queue element  */
                                       /*   to process                       */
   int sq_tail;                        /* index of next free location on     */
                                       /*   on sound queue                   */
   struct sound_q {                    /* circular FIFO sound queue          */
      ulong  sec;                      /*  "seconds" part of duration        */
      ulong  nsec;                     /*  "nano-seconds" part of duration   */
      char  locmd;                     /*  value to stuff into port 0x54     */
      char  hicmd;                     /*  value to stuff into port 0x55     */
   } sound_q[SOUND_Q_SIZE];

   uchar alarm_active;                 /* TRUE if alarm is on                */
   uchar ih_inst;                      /* TRUE if interrupt handler installed*/
                                       /* for use in problem determination...*/
                                       /* (W/R by kbdintr, RO for all others)*/
   uchar status;                       /*   save adapter status here         */
   uchar data;                         /*   save adapter data here           */
};

/*****************************************************************************/
/* proto types and references                                                */
/*****************************************************************************/

/* functions                                                                 */
extern int    kbdmpx(dev_t, int *, char *);
extern int    kbdopen(dev_t, uint, chan_t, caddr_t);
extern int    kbdclose(dev_t, chan_t, caddr_t);
extern int    kbdioctl(dev_t, int, caddr_t, uint, chan_t, caddr_t);
extern int    chg_clicker(uint);
extern int    enter_diag(void);
extern int    kbdconfig(dev_t, long, struct uio *);
extern void   cleanup(dev_t);
extern int    initadpt(struct ktsmdds *, dev_t);
extern int    addswitch(dev_t);
extern int    qvpd(struct uio *);

extern void   send_q_frame(struct common *);
extern void   watch_dog(struct trb *);
extern int    read_port(char *, char *);
extern int    write_port(char *, char);
extern void   io_error(char *, int, uint, uint, char *, ...);
extern int    reg_intr(int);
extern void   ureg_intr(void);
extern int    sv_rflush(dev_t, caddr_t);
extern int    sv_sak(dev_t, caddr_t);
extern int    sv_alarm(dev_t, caddr_t);
extern int    put_sq(struct ksalarm *);
extern int    next_sound(void);
extern void   stop_sound(void);
extern struct cdt * mem_dump(int);
extern int    kbdintr(struct intr *);
extern void   kbderr(struct common *, struct kbd_port *);
extern void   kbdack(struct common *);

#ifdef PM_SUPPORT
extern void   reg_pm(struct ktsmdds *, dev_t);
extern void   ureg_pm(void);
extern int    pm_proc(caddr_t, int);
#endif /* PM_SUPPORT */

/* data                                                                      */
extern struct  local *local;
extern lock_t  kbd_lock;
extern void   *service_vector[];
extern struct cdt my_cdt;
