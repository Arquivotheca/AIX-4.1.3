/* @(#)71   1.5  src/bos/kernext/inputdd/inc/mse.h, inputdd, bos41J, 9510A_all 3/7/95 09:55:00  */
/*
 * COMPONENT_NAME: (INPUTDD) Mouse DD - mse.h
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
#ifdef PM_SUPPORT
#include <sys/pm.h>
#include <sys/pmdev.h>
#endif /* PM_SUPPORT */

/*****************************************************************************/
/* local storage data structure                                              */
/*****************************************************************************/

struct local {
   struct  common com;                 /* common data                        */
   struct  rcb rcb;                    /* ring control block:                */
   dev_t   mse_devno;                  /* devno of mouse                     */
#ifdef PM_SUPPORT
   struct  pm_handle pm;               /* pm handle structure                */
#endif /* PM_SUPPORT */

   ulong   mouse_rpt;                  /* mouse report (R/W only by IH)      */

   uint    mouse_type;                 /* mouse type (see inputdd.h)         */
/*         MOUSE3B           0x01            3 button mouse                  */
/*         MOUSE2B           0x02            2 button mouse                  */

   short   mouse_hor_accum;            /* mouse horizontal accumulator       */
   short   mouse_hor_thresh;           /* mouse horizontal threshold         */
   short   mouse_ver_accum;            /* mouse vertical accumulator         */
   short   mouse_ver_thresh;           /* mouse vertical threshold           */

   char    oflag;                      /* TRUE if device is open             */
   char    block_mode;                 /* TRUE if device is in block mode    */

   char    mouse_resolution;           /* mouse resolution (2^x)             */
   char    mouse_sample_rate;          /* mouse sample rate                  */
   char    mouse_scaling;              /* mouse scaling  0=1:1, 1=2:1        */
   char    button_status;              /* holds old button status            */
   char    mouse_overflow;


};

/*****************************************************************************/
/* proto types and references                                                */
/*****************************************************************************/

/* functions                                                                 */
extern void   send_q_frame(struct common *);
extern void   watch_dog(struct trb *);
extern int    read_long(long *, long *);
extern int    read_char(char *, char *);
extern int    write_port(short *, short);
extern void   io_error(char *, int, uint, uint, char *, ...);
extern int    reg_intr(int);
extern void   ureg_intr(void);
extern int    set_resolution(uint);
extern int    set_sample_rate(uint);
extern int    set_scale_factor(uint);
extern int    send_cmd(OFRAME, OFRAME);
extern int    mseintr(struct intr *);
extern void   mouse_proc(void);
extern void   mouse_frame(struct common *, struct mouse_port *);
extern void   mouse_err(struct common *, struct mouse_port *);
extern int    mseconfig(dev_t, long, struct uio *);
extern void   cleanup(dev_t);
extern int    initadpt(struct ktsmdds *, dev_t);
extern int    initmse(struct ktsmdds *, dev_t);
extern int    addswitch(dev_t);
extern int    qvpd(struct uio *);
extern int    get_id(void);
extern int    mseopen(dev_t, uint, chan_t, caddr_t);
extern int    mseclose(dev_t, chan_t, caddr_t);
extern int    mseioctl(dev_t, int, caddr_t, uint, chan_t, caddr_t);
extern int    disable_intr(struct mouse_port *);
extern int    enable_intr(struct mouse_port *);
extern struct cdt * mem_dump(int);
extern void   rst_mse_adpt(void);    

#ifdef PM_SUPPORT
extern void   reg_pm(struct ktsmdds *, dev_t);
extern void   ureg_pm(void);
extern int    pm_proc(caddr_t, int);
#endif /* PM_SUPPORT */

/* data                                                                      */
extern struct  local *local;
extern lock_t  mse_lock;
extern OFRAME mse_button_cmds[];
extern struct cdt my_cdt;
