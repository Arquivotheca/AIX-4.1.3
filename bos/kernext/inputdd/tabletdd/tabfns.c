static char sccsid[] = "@(#)88   1.3  src/bos/kernext/inputdd/tabletdd/tabfns.c, inputdd, bos411, 9428A410j 6/9/94 07:41:35";
/*
 * COMPONENT_NAME: (INPUTDD) Mouse DD - tabfns.c
 *
 * FUNCTIONS:  tabopen, tabclose, tabioctl
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

#include "tab.h"

/*****************************************************************************/
/*                                                                           */
/* NAME:        tabopen                                                      */
/*                                                                           */
/* FUNCTION:    Process open request                                         */
/*                                                                           */
/* INPUTS:      dev = device major/minor number                              */
/*              flag = open flags                                            */
/*              chan = channel number  (not used)                            */
/*              ext = extension parms  (not used)                            */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              EINTR = interrupted system call                              */
/*              ENXIO = invalid device number                                */
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

int  tabopen(dev_t dev, uint flag, chan_t chan, caddr_t ext)
{

  int  rc;
  IFRAME buff[6];



  KTSMDTRACE0(tabopen, enter);
                                       /* try and get lock                   */
  if (lockl(&tab_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = EPERM;
  }
  else {
    if (!local)                        /* error if no local storage          */
      rc = EPERM;

    else {
      if (dev != local->tab.tab_devno) /* error if invalid device number     */
        rc = ENXIO;

      else {                           /* error if device already open       */
        if (local->tab.oflag)
          rc = EBUSY;

        else {                      
          if (flag & DKERNEL) {        /* error if open from kernel proc     */
            rc = EACCES;
          }
          else {
                                       /* reset local defaults               */ 
            local->flush_err        = 0;
            local->tab_bytes_read   = 0;
            if (!(rc = tabletopen(&local->com, &local->tab))) {
                                       /* ok, indicate device is open        */
              local->tab.oflag = TRUE;
            }
          }
        }
      }
    }
    unlockl(&tab_lock);                /* free lock                          */
  }

  KTSMTRACE1(tabopen, exit,  rc);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        tabclose                                                     */
/*                                                                           */
/* FUNCTION:    Process close request                                        */
/*                                                                           */
/* INPUTS:      dev = device major/minor number                              */
/*              chan = channel number (not used)                             */
/*              ext = extension parms (not used)                             */
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

int  tabclose(dev_t dev, chan_t chan, caddr_t ext)
{
  int rc = 0;

  KTSMDTRACE0(tabclose, enter);
                                       /* try and get lock                   */
  if (lockl(&tab_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = EPERM;
  }
  else {
    if (!local)                        /* error if no local storage          */
      rc = EPERM;

    else {
      if (dev != local->tab.tab_devno) /* error if invalid device number     */
        rc = ENXIO;

      else {                           /* continue only if device open       */
        if (local->tab.oflag) {
                                       /* disable tablet                     */
          put_oq1(&local->com, DISABLE_TAB_CMD);
          wait_oq(&local->com);
                                       /* unregister input ring              */
          ktsm_uring(&local->tab.rcb);
                                       /* channel is no longer open          */
          local->tab.oflag = FALSE;

          ureg_intr();                 /* unregister intr handler            */
        }
      }
    }
    unlockl(&tab_lock);                /* free lock                          */
  }

  KTSMTRACE1(tabclose, exit,  rc);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        tabioctl                                                     */
/*                                                                           */
/* FUNCTION:    Process ioctl request                                        */
/*                                                                           */
/* INPUTS:      dev = device major/minor number                              */
/*              cmd = function requested via ioctl                           */
/*              arg = pointer to argument passed via ioctl                   */
/*              flag = open flags                                            */
/*              chan = channel number   (not used)                           */
/*              ext = extension parms   (not used)                           */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              ENXIO = invalid device number                                */
/*              EIO    = I/O error                                           */
/*              EINVAL = invalid cmd or argument                             */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/


int  tabioctl(dev_t dev, int cmd, caddr_t arg, uint flag,
              chan_t chan, caddr_t  ext)
{
  int   rc = 0;
  ulong ldata;
  uint  data;
  struct devinfo info;
  struct tabqueryid t_query;


  KTSMDTRACE0(tabioctl, enter);

                                       /* try and get lock                   */
  if (lockl(&tab_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = EPERM;
  }
  else {
    if (!local)                        /* error if no local storage          */
      rc = EPERM;

    else {
      if (dev != local->tab.tab_devno) /* error if invalid device number     */
        rc = ENXIO;

      else {
        if (!local->tab.oflag)  {      /* error if device is not open        */
          rc = EACCES;
        }
        else {
          rc =  tabletioctl(&local->com, &local->tab, cmd, arg);
        }
      }
    }
    unlockl(&tab_lock);                /* free lock                          */
  }

  KTSMTRACE(tabioctl, exit,  rc, cmd, 0, 0, 0);
  return(rc);
}

