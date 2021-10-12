/* @(#)67	1.2  src/bos/kernext/inputdd/inc/kts.h, inputdd, bos41J, 9509A_all 2/14/95 13:20:06  */
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard/Tablet/Sound DD - kts.h
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
#include "tabext.h"

/*****************************************************************************/
/* local storage data structure                                              */
/*****************************************************************************/


struct local {
   struct  common com;                 /* common data                        */
   struct  kbdext key;                 /* keyboard extension                 */
   struct  tabext tab;                 /* tablet extension                   */

   int blk_cnt;                        /* uart block count                   */

   int sq_head;                        /* index of next sound queue element  */
                                       /*   to process                       */
   int sq_tail;                        /* index of next free location on     */
                                       /*   on sound queue                   */
   struct sound_q {                    /* circular FIFO sound queue          */
      ushort volume;                   /*   speaker volume                   */
      ushort freq;                     /*   alarm frequency                  */
      ushort duration;                 /*   alarm duration                   */
   } sound_q[SOUND_Q_SIZE];

                                       /* The following is saved here for RAS*/
   char  status;                       /*   8051 status (port c)             */
   char  data;                         /*   8051 data  (port a)              */

                                       /* tablet status report               */
   char tab_block_data[TAB_REPORT_SIZE]; 

   uchar volume;                       /* current adapter volume             */
   uchar alarm_active;                 /* TRUE if alarm is on                */
   uchar ih_inst;                      /* TRUE if interrupt handler installed*/
   uchar tab_dframe;                   /* TRUE if next frame in group is data*/
   uchar blk_act;                      /* TRUE if tab block xfer active      */
};

/*****************************************************************************/
/* proto types and references                                                */
/*****************************************************************************/

/* functions                                                                 */
extern int    ktsmpx(dev_t, int *, char *);
extern int    ktsopen(dev_t, uint, chan_t, caddr_t);
extern int    ktsclose(dev_t, chan_t, caddr_t);
extern int    ktsioctl(dev_t, int, caddr_t, uint, chan_t, caddr_t);
extern int    enter_diag(void);
extern int    ktsconfig(dev_t, long, struct uio *);
extern void   cleanup(dev_t);
extern int    initadpt(struct ktsmdds *, dev_t);
extern int    addswitch(dev_t);
extern int    qvpd(struct uio *);

extern void   send_q_frame(struct common *);
extern void   watch_dog(struct trb *);
extern int    read_port(char *, char *);
extern int    write_port(char *, char);
extern int    write_sport(short *, short);
extern void   io_error(char *, int, uint, uint, char *, ...);
extern int    reg_intr(int);
extern void   ureg_intr(void);
extern int    sv_rflush(dev_t, caddr_t);
extern int    sv_sak(dev_t, caddr_t);
extern int    sv_alarm(dev_t, caddr_t);
extern int    put_sq(struct ksalarm *);
extern int    next_sound(void);
extern void   stop_sound(void);
extern int    chg_clicker(uint);
extern struct cdt * mem_dump(int);
extern int    ktsintr(struct intr *);
extern void   info_int(void);
extern void   ktsack(void);
extern void   tabdone(void);

/* data                                                                      */
extern struct  local *local;
extern lock_t  kts_lock;
extern void   *service_vector[];
extern OFRAME init_8051[];
extern struct cdt my_cdt;
