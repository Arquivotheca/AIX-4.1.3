static char sccsid[] = "@(#)79   1.2  src/bos/kernext/inputdd/ktsdd/ktsfns.c, inputdd, bos411, 9428A410j 6/9/94 07:34:57";
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard/Tablet/Sound DD - ktsfns.c
 *
 * FUNCTIONS: ktsmpx, ktsopen, ktsclose, ktsioctl, enter_diag
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "kts.h"

/*****************************************************************************/
/*                                                                           */
/* NAME:        ktsmpx                                                       */
/*                                                                           */
/* FUNCTION:    Allocate/deallocate device channel                           */
/*                                                                           */
/* INPUTS:      dev = device major/minor number                              */
/*              channum = channel number (or location to rtn ch number)      */
/*              channame = channel name                                      */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              ENXIO = invalid device number                                */
/*              EBUSY  = all channels allocated                              */
/*              ECHRNG = invalid channel number                              */
/*              EINVAL = user requested specific channel                     */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       keyboard supports multiple channels but tablet has only one  */
/*                                                                           */
/*****************************************************************************/

int  ktsmpx(dev_t dev, int *channum, char *channame)
{

  int rc = 0;

  KTSMDTRACE0(ktsmpx, enter);

                                       /* try and get lock                   */
  if (lockl(&kts_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = EPERM;
  }
  else {
    if (!local) {                      /* error if no local storage          */
      rc = EPERM;
    }
    else {
                                       /* if keyboard then                   */
      if (dev == local->key.kbd_devno) { 
                                       /* allocate/deallocate channel        */
         rc =  keympx(&local->key, channum, channame);
      }
      else {       
                                       /* if tablet then                     */
        if (dev == local->tab.tab_devno) { 
          if (channame)   {            /* if allocating channel ...          */
            if (*channame != '\0') {   /*   user can not specify channel num */
              rc = EINVAL;
            }
            else {                     /*   error if device is open          */
                                       /*   or keyboard is in diag mode      */
              if ((local->tab.oflag) || (local->key.diag)) {
                rc = EBUSY;
              }
              else {
                *channum = TABCH;       /*   return channel number            */
              }
            }
          }
          else {                       /* else deallocate channel ...        */
            if ((*channum !=  0) ||    /*   error if channel number is not 0 */
                (local->tab.oflag)) {  /*   or device is open                */
               rc = ECHRNG;
            }
          }
        }
        else {                         /* no such device                     */
          rc = ENXIO;
        }
      }
    }
    unlockl(&kts_lock);                /* free lock                          */
  }

  KTSMTRACE(ktsmpx, exit, rc, *channum, 0, 0, 0);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        ktsopen                                                      */
/*                                                                           */
/* FUNCTION:    Process open request                                         */
/*                                                                           */
/* INPUTS:      dev = device major/minor number                              */
/*              flag = open flags                                            */
/*              chan = channel number                                        */
/*              ext = extension parms                                        */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              ENXIO = invalid device number                                */
/*              ENODEV = invalid keyboard                                    */
/*              EACCES = already open                                        */
/*              EPERM = failed to register interrupt handler                 */
/*              EIO = I/O error                                              */
/*              ENOMEM = unable to pin driver, out of memory                 */
/*              ENOSPC = insufficient paging space                           */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  ktsopen(dev_t dev, uint flag, chan_t chan, caddr_t ext)
{

  int  rc;

  KTSMDTRACE0(ktsopen, enter);
                                       /* try and get lock                   */
  if (rc=lockl(&kts_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = EPERM;
  }
  else {
    if (!local) {                      /* error if no local storage          */
      rc = EPERM;
    }
    else {
                                       /* if keyboard device then            */
      if (dev == local->key.kbd_devno) { 
                                       /* open channel                       */
         rc = keyopen(&local->com, &local->key, flag, chan);
      }
      else {                           /* if tablet then                     */
        if (dev == local->tab.tab_devno) { 
          if (chan != TABCH) {         /* error if invalid channel number    */
            rc = ECHRNG;
          }
          else {                        /* error if tablet is already open   */
                                        /* or keyboard in diag mode          */
            if ((local->tab.oflag) || (local->key.diag)) {
              rc = EBUSY;
            }
            else {
              if (flag & DKERNEL) {    /* error if open from kernel proc     */
                rc = EACCES;
              }
              else {                   /* initialize tablet                  */
                if (!(rc = tabletopen(&local->com, &local->tab))) {
                                       /* tell adapter to block tablet frames*/
                  put_oq2(&local->com, TAB_BLOCK_6, DO_BLK_TAB_BYTES);
                                       /* wait for completion                */
                  if (rc = wait_oq(&local->com)) {
                    ureg_intr();       /* unregister intr handler if failed  */
                  }
                  else {               /* indicate tablet is open if all ok  */
                    local->tab.oflag = TRUE;  
                  }
                }
              }
            }
          }
        }
        else {                         /* no such device                     */
          rc = ENXIO;
        }
      }
    }
    unlockl(&kts_lock);                /* free lock                          */
  }

  KTSMTRACE(ktsopen, exit,  rc, dev, flag, chan, local->key.act_ch);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        ktsclose                                                     */
/*                                                                           */
/* FUNCTION:    Process close request                                        */
/*                                                                           */
/* INPUTS:      dev = device major/minor number                              */
/*              chan = channel number                                        */
/*              ext = extension parms                                        */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              ENXIO = invalid device number                                */
/*              EPERM  = error while defining interrupt handler to system    */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  ktsclose(dev_t dev, chan_t chan, caddr_t ext)
{

  int  rc = 0;

  KTSMDTRACE0(ktsclose, enter);
                                       /* try and get lock                   */
  if (lockl(&kts_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = EPERM;
  }
  else {
    if (!local) {                      /* error if no local storage          */
      rc = EPERM;
    }
    else {
                                       /* if keyboard device then            */
      if (dev == local->key.kbd_devno) { 
                                       /* close channel                      */
        rc = keyclose(&local->com, &local->key, chan);
      }
      else {                           /* if tablet then                     */
        if (dev == local->tab.tab_devno) {
          if (chan != TABCH) {         /* error if invalid channel number    */
            rc = ECHRNG;
          }
          else {
            if (local->tab.oflag) {    /* only do if tablet is open          */
                                       /* unregister input ring              */
              ktsm_uring(&local->tab.rcb);
                                       /* tell adapter not to block tablet   */
                                       /* frames and disable tablet          */
              put_oq2(&local->com, DISABLE_TAB_CMD, DONT_BLK_TAB_BYTES);
                                       /* block xfer is not active           */
              local->blk_act = FALSE;
                                       /* tablet is no longer open           */
              local->tab.oflag = FALSE;
                                       /* unregister intr handler            */
              ureg_intr();             /* if keyboard is not open            */
            }
          }
        }
        else {                         /* no such device                     */
          rc = ENXIO;
        }
      }
    }
    unlockl(&kts_lock);                /* free lock                          */
  }

  KTSMTRACE(ktsclose, exit,  rc, dev, chan, local->key.act_ch, 0);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        ktsioctl                                                     */
/*                                                                           */
/* FUNCTION:    Process ioctl request                                        */
/*                                                                           */
/* INPUTS:      dev = device major/minor number                              */
/*              cmd = function requested via ioctl                           */
/*              arg = pointer to argument passed via ioctl                   */
/*              flag = open flags                                            */
/*              chan = channel number                                        */
/*              ext = extension parms                                        */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              ENXIO = invalid device number                                */
/*              EIO    = I/O error                                           */
/*              EINVAL = invalid cmd or argument                             */
/*              EPERM  = could not allocate timer                            */
/*              ENOMEM = insufficient memory                                 */
/*              ENOSPC = insufficient file system or paging space            */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/


int  ktsioctl(dev_t dev, int cmd, caddr_t arg, uint flag,
              chan_t chan, caddr_t  ext)
{
  int    rc;

  KTSMDTRACE0(ktsioctl, enter);

                                       /* try and get lock                   */
  if (lockl(&kts_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = EPERM;
  }
  else {
    if (!local) {                      /* error if no local storage          */
      rc = EPERM;
    }
    else {
                                       /* if keyboard device then            */
      if (dev == local->key.kbd_devno) {
                                       /* process ioctl command              */
        rc =  keyioctl(&local->com, &local->key, cmd, arg, flag, chan);
      }
      else {                           /* if tablet then                     */
        if (dev == local->tab.tab_devno) {
          if (chan != TABCH) {         /* error if invalid channel number    */
            rc = ECHRNG;
          }
          else {
            if (!local->tab.oflag) {   /* error if tablet is not open        */
              rc = EACCES;
            }
            else {                     /* process command                    */
              rc = tabletioctl(&local->com, &local->tab, cmd, arg);
            }
          }
        }
        else {                         /* no such device                     */
          rc = ENXIO;
        }
      }
    }
    unlockl(&kts_lock);                /* free lock                          */
  }

  KTSMTRACE(ktsioctl, exit,  rc, chan, flag, cmd, local->key.act_ch);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        enter_diag                                                   */
/*                                                                           */
/* FUNCTION:    enter diagnostics mode                                       */
/*                                                                           */
/* INPUTS:      none                                                         */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int enter_diag(void)
{
   if (local->tab.oflag) {           /* can not enter diag if tablet open    */
     return(EBUSY);
   }

   if (!local->key.diag) {           /* if not in diag mode then             */
                                     /*   disable keyboard scanning          */
     put_oq1(&local->com, DEFAULT_DISABLE_CMD);
     local->key.diag = TRUE;         /*   driver is in diag mode             */
     ureg_intr();                    /*   clear interrupt handler            */
   }
   return(0);
}
