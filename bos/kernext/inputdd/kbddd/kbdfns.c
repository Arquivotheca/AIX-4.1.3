static char sccsid[] = "@(#)84   1.2  src/bos/kernext/inputdd/kbddd/kbdfns.c, inputdd, bos411, 9428A410j 6/9/94 07:37:18";
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard DD - kbdfns.c
 *
 * FUNCTIONS: kbdmpx, kbdopen, kbdclose, kbdioctl, chg_clicker,
 *            enter_diag
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

#include "kbd.h"

/*****************************************************************************/
/*                                                                           */
/* NAME:        kbdmpx                                                       */
/*                                                                           */
/* FUNCTION:    Allocate/deallocate device channel                           */
/*                                                                           */
/* INPUTS:      dev = device major/minor number                              */
/*              chanp =                                                      */
/*              channame =                                                   */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              ENXIO = invalid device number                                */
/*              EBUSY  = all channels allocated                              */
/*              ECHRNG = invalid channel number                              */
/*              EINVAL = user requested specific channel                     */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  kbdmpx(dev_t dev, int *channum, char *channame)
{

  int rc;

  KTSMDTRACE0(kbdmpx, enter);

                                       /* try and get lock                   */
  if (lockl(&kbd_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = EPERM;
  }
  else {
    if (!local)                        /* error if no local storage          */
      rc = EPERM;
    else {
                                       /* error if invalid device number     */
      if (dev != local->key.kbd_devno)  {
        rc = ENXIO; 
      }
      else {                           /* allocate/deallocate channel        */ 
         rc =  keympx(&local->key, channum, channame);
      }
    }
    unlockl(&kbd_lock);                /* free lock                          */
  }

  KTSMTRACE(kbdmpx, exit, rc, *channum, 0, 0, 0);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        kbdopen                                                      */
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

int  kbdopen(dev_t dev, uint flag, chan_t chan, caddr_t ext)
{

  int  rc;

  KTSMDTRACE0(kbdopen, enter);
                                       /* try and get lock                   */
  if (lockl(&kbd_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = EPERM;
  }
  else {
    if (!local)                        /* error if no local storage          */
      rc = EPERM;
    else {
                                       /* error if invalid device number     */
      if (dev != local->key.kbd_devno) {
        rc = ENXIO;
      }
      else {                           /* open channel                       */
         rc = keyopen(&local->com, &local->key, flag, chan);
      }
    }
    unlockl(&kbd_lock);                /* free lock                          */
  }

  KTSMTRACE(kbdopen, exit,  rc, dev, flag, chan, local->key.act_ch);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        kbdclose                                                     */
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

int  kbdclose(dev_t dev, chan_t chan, caddr_t ext)
{

  int  rc;

  KTSMDTRACE0(kbdclose, enter);
                                       /* try and get lock                   */
  if (lockl(&kbd_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = EPERM;
  }
  else {
    if (!local)                        /* error if no local storage          */
      rc = EPERM;
    else {
                                       /* error if invalid device number     */
      if (dev != local->key.kbd_devno) {
        rc = ENXIO; 
      }
      else {             
                                       /* close channel                      */
        rc = keyclose(&local->com, &local->key, chan);
      }
    }
    unlockl(&kbd_lock);                /* free lock                          */
  }

  KTSMTRACE(kbdclose, exit,  rc, dev, chan, local->key.act_ch, 0);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        kbdioctl                                                     */
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


int  kbdioctl(dev_t dev, int cmd, caddr_t arg, uint flag,
              chan_t chan, caddr_t  ext)
{
  int    rc;

  KTSMDTRACE0(kbdioctl, enter);

                                       /* try and get lock                   */
  if (lockl(&kbd_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = EPERM;
  }
  else {
    if (!local)                        /* error if no local storage          */
      rc = EPERM;
    else {
                                       /* error if invalid device number     */
      if (dev != local->key.kbd_devno) {
        rc = ENXIO;
      }
      else {                           /* process command                    */
        rc =  keyioctl(&local->com, &local->key, cmd, arg, flag, chan);
      }
    }
    unlockl(&kbd_lock);                /* free lock                          */
  }

  KTSMTRACE(kbdioctl, exit,  rc, chan, flag, cmd, local->key.act_ch);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        chg_clicker                                                  */
/*                                                                           */
/* FUNCTION:    Change clicker volume                                        */
/*                                                                           */
/* INPUTS:      none                                                         */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       This platform does not support clicker so this routine       */
/*              does nothing                                                 */
/*                                                                           */
/*****************************************************************************/

int chg_clicker(uint volume)
{
  return(0);
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
/* NOTES:       this code is platform dependent to allow RS1/RS2 driver      */
/*              to verify that the tablet special file is not open           */
/*              (if the tablet is open, EBUSY is returned)                   */
/*                                                                           */
/*****************************************************************************/

int enter_diag(void)
{
   if (!local->key.diag) {           /* if not in diag mode then             */
                                     /*   dislable keyboard scanning         */
     put_oq1(&local->com, DEFAULT_DISABLE_CMD);
     local->key.diag = TRUE;         /*   driver is in diag mode             */
     ureg_intr();                    /*   clear interrupt handler            */
   }
   return(0);
}
