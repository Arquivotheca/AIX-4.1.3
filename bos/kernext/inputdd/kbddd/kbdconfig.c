static char sccsid[] = "@(#)82	1.4  src/bos/kernext/inputdd/kbddd/kbdconfig.c, inputdd, bos41J, 9509A_all 2/14/95 13:20:24";
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard DD - kbdconfig.c
 *
 * FUNCTIONS:  kbdconfig, cleanup, initadpt, addswitch, qvpd
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


#include  "kbd.h"

lock_t  kbd_lock=LOCK_AVAIL;           /* process level lock                */


/*****************************************************************************/
/*                                                                           */
/* NAME:             kbdsconfig                                              */
/*                                                                           */
/* FUNCTION:         This module manages the task of allocating storage for  */
/*                   and configuring the keyboard.                           */
/*                                                                           */
/* INPUTS:           devno = major and minor device number                   */
/*                   cmd   = command, i.e. what to do: initialize, terminate,*/
/*                              or query vital product data                  */
/*                   uiop  = pointer to a uio structure containing the dds   */
/*                                                                           */
/* OUTPUTS:          0      = success                                        */
/*                   EINTR  = failure                                        */
/*                   ENOSPC                                                  */
/*                   EFAULT                                                  */
/*                   ENOMEM                                                  */
/*                   EINVAL                                                  */
/*                   EPERM                                                   */
/*                   EIO                                                     */
/*                   ENXIO                                                   */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  kbdconfig(dev_t devno, long cmd, struct uio *uiop)
{

  struct ktsmdds    dds;               /* local copy of dds                  */
  int rc = 0;

  KTSMDTRACE(kbdconfig, enter, devno, cmd, 0, 0, 0);

                                       /* get lock                           */
  if (lockl(&kbd_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = EPERM;
  }
  else {

    switch(cmd) {                      /* switch on command                  */

                                       /*------------------------------------*/
      case CFG_INIT:                   /* case: initialize                   */
                                       /*------------------------------------*/

                                       /* move dds to kernel memory          */
        if (uiop == (struct uio *) NULL) {
          rc = EINVAL;                 /* ...error                           */
          break;
        }
        if (rc = uiomove((caddr_t) &dds, sizeof(struct ktsmdds),
              UIO_WRITE, uiop)) {
          if (rc < 0) rc = EPERM;      /* ...error                           */
          break;
        }

        if (!local) {                  /* allocate local storage             */
          local = (struct local *)
              xmalloc(sizeof(struct local), 2, pinned_heap);
          if (!local) {                /* ...error                           */
            rc = ENOMEM;
            break;
          }
                                       /* clear out local storage            */
          bzero ((char *) local, sizeof(struct local));

                                       /* allocate I/O wait timer            */
          if ((local->com.stimer = talloc()) == NULL ) {
            rc = EPERM;                /* ...error                           */
            break;
          }
                                       /* allocate watch dog timer           */
          if ((local->com.wdtimer = talloc()) == NULL ) {
            rc = EPERM;                /* ...error                           */
            break;
          }
                                       /* allocate alarm timer               */
          if ((local->sdd_time_ptr = talloc()) == NULL) {
            rc = EPERM;                /* ...error                           */
            break;
          }
                                       /* initialize sleep anchor            */
          local->com.asleep = EVENT_NULL;
                                       /* no active keyboard channels        */
          local->key.act_ch = NO_ACT_CH;
        }

                                       /* configuring adapter                */
        if (dds.device_class == ADAPTER) {
           rc = initadpt(&dds, devno); 
        }
        else {                         /* configuring keyboard               */
          if (dds.device_class == KBD) {
            rc = initkbd(&local->com, &local->key, &dds, devno);
          }
          else {
            rc = EINVAL;               /* invalid device class               */
          }
        }
        break;


                                       /*------------------------------------*/
      case CFG_TERM:                   /* case: terminate                    */
                                       /*------------------------------------*/

        if (!local) {                  /* error if not configured            */
          rc = ENXIO;
        }
        else {                         /* terminating adapter                */
          if (devno == local->com.adpt_devno) {
                                       /*   error if keyboard is configured  */
            if (local->key.kbd_devno) {
                rc = EBUSY;
            }
            else {                     /*   mark adapter as termianted       */
              local->com.adpt_devno = 0;
            }
          }
          else {                       /* terminating keyboard               */
            if (devno == local->key.kbd_devno) {
                                       /*   error if channel open            */
              if (local->key.act_ch != NO_ACT_CH) {
                rc = EBUSY;
              }
              else {                   /*   mark keyboard as terminated      */
                local->key.kbd_devno = 0;
              }
            }
            else {                     /* error if not any known device      */
              rc = ENXIO;
            }
          }
        }
        break;

                                       /*------------------------------------*/
      case CFG_QVPD:                   /* case: query vital product data     */
                                       /*------------------------------------*/

        if (!local) {                  /* error if not configured            */
          rc = ENXIO;
        }
        else {                         /* only valid if for adapter          */
          if (devno != local->com.adpt_devno) {
            rc = EINVAL;
          }
          else {                       /* error if no uio structure          */
            if (uiop == (struct uio *) NULL) {
              rc = EINVAL;
            }
            else {
              rc = qvpd(uiop);
            }
          }
        }
        break;

                                       /*------------------------------------*/
      default:                         /* default: unknown command           */
                                       /*------------------------------------*/

        rc = EINVAL;
        break;

    }

    cleanup(devno);                    /* release resource                   */
    unlockl(&kbd_lock);                /* release lock                       */
  }

  KTSMTRACE(kbdconfig, exit, rc, devno, cmd, 0, 0);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             cleanup                                                 */
/*                                                                           */
/* FUNCTION:         this module frees all local resources                   */
/*                                                                           */
/* INPUTS:           devno = major and minor device numbers                  */
/*                                                                           */
/* OUTPUTS:          none                                                    */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void cleanup(dev_t devno)
{

   if (local) {                        /* if resources allocated and         */
                                       /* nothing configured then            */
     if ((!local->com.adpt_devno) &&
       (!local->key.kbd_devno)) { 

       devswdel(devno);                /* clear switch table                 */

       if (local->com.stimer)          /* free I/O wait timer                */
         tfree(local->com.stimer);
       if (local->com.wdtimer)         /* free watch dog timer               */
         tfree(local->com.wdtimer);
       if (local->sdd_time_ptr)        /* free alarm timer                   */
         tfree(local->sdd_time_ptr);
                                       /* clear out local memory             */
       bzero ((char *) local, sizeof(struct local));
                                       /* free it                            */
       xmfree((caddr_t) local, pinned_heap);

       local = NULL;
     }
   }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             initadpt                                                */
/*                                                                           */
/* FUNCTION:         This module initializes the adapter                     */
/*                                                                           */
/* INPUTS:           devno = device number                                   */
/*                   dds_ptr = pointer to dds structure                      */
/*                                                                           */
/* OUTPUTS:          none                                                    */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int initadpt(struct ktsmdds *dds_ptr, dev_t devno)
{
  int rc = 0;

  if (!local->com.adpt_devno) {        /* skip if already configured         */
                                       /* save bus ID                        */
    local->com.bus_id = dds_ptr->bus_id | 0x000C0020;
                                       /* save slot address                  */
    local->com.slot_addr = (dds_ptr->slot_addr << 16) | 0x00400000;
                                       /* save adapter I/O address           */
    local->com.bus_addr = dds_ptr->bus_io_addr;
                                       /* save interrupt level               */
    local->com.intr_level = dds_ptr->bus_intr_lvl;
                                       /* save interrupt priority            */
    local->com.intr_priority = dds_ptr->intr_priority;
    rc = addswitch(devno);             /* add device to switch table         */
    if (!rc)                           /* if all ok then mark adapter        */
      local->com.adpt_devno = devno;   /* configured by saving our devno     */
  }

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             addswitch                                               */
/*                                                                           */
/* FUNCTION:         This module adds the device driver to the device        */
/*                   switch table.                                           */
/*                                                                           */
/* INPUTS:           devno = major and minor device numbers                  */
/*                                                                           */
/* OUTPUTS:          0      = success                                        */
/*                   ENOMEM = failure                                        */
/*                   EINVAL                                                  */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  addswitch(dev_t devno)
{

   int           rc;
   struct devsw  tmp_devsw;

   void nodev(void);
                                       /* copy pointers to functions into    */
                                       /* the local device switch table      */
   tmp_devsw.d_open     = (int (*) ()) kbdopen;
   tmp_devsw.d_close    = (int (*) ()) kbdclose;
   tmp_devsw.d_read     = (int (*) ()) nodev;
   tmp_devsw.d_write    = (int (*) ()) nodev;
   tmp_devsw.d_ioctl    = (int (*) ()) kbdioctl;
   tmp_devsw.d_strategy = (int (*) ()) nodev;
   tmp_devsw.d_select   = (int (*) ()) nodev;
   tmp_devsw.d_config   = (int (*) ()) kbdconfig;
   tmp_devsw.d_print    = (int (*) ()) nodev;
   tmp_devsw.d_dump     = (int (*) ()) nodev;
   tmp_devsw.d_mpx      = (int (*) ()) kbdmpx;
   tmp_devsw.d_revoke   = (int (*) ()) nodev;
   tmp_devsw.d_dsdptr   = NULL;
   tmp_devsw.d_ttys     = NULL;
   tmp_devsw.d_selptr   = NULL;
   tmp_devsw.d_opts     = 0;
                                       /* add to the device switch table     */
   return(devswadd(devno, &tmp_devsw));

}


/*****************************************************************************/
/*                                                                           */
/* NAME:             qvpd                                                    */
/*                                                                           */
/* FUNCTION:         This module returns information on what device(s)       */
/*                   are connected to adapter. The data is used by the       */
/*                   define_children() step of the adpater's configuration   */
/*                   method. The data returned is NOT traditional VPD data.  */
/*                                                                           */
/* INPUTS:           uio pointer which points to an area of memory where the */
/*                      data will be stored                                  */
/*                                                                           */
/* OUTPUTS:          0      = success                                        */
/*                   ENOMEM = failure                                        */
/*                   EIO                                                     */
/*                   ENOSPC                                                  */
/*                   EFAULT                                                  */
/*                   EPERM                                                   */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  qvpd(struct uio *uiop )
{
   uchar  data[27];
   int rc;
   int index = 0;
                                       /* get keyboard ID if keyboard has    */
   if (!local->key.kbd_devno) {        /* not yet been configured            */
     if (!reg_intr(FALSE)) { 
        get_kbd_id(&local->com, &local->key);
        ureg_intr();   
     }
   }
                                        /* decide what data should be rtn'ed */
   switch (local->key.kbd_type) {
  
      case KS101:
         bcopy("keyboard/std_k/kb101 kbd ", data ,25);
         index = 25;
         break;
      case KS102:
         bcopy("keyboard/std_k/kb102 kbd ", data, 25);
         index = 25;
         break;
      case KS106:
         bcopy("keyboard/std_k/kb106 kbd ", data, 25);
         index = 25;
         break;
      default:
         break;
   }

   data[index] = '\0';                 /* null terminate data                */
                                       /* write data out to user space       */
   rc = uiomove(data, index+1, UIO_READ, uiop);
   if (rc < 0) rc = EPERM;

   return(rc);
}
