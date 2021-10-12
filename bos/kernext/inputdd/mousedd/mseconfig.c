static char sccsid[] = "@(#)93	1.7  src/bos/kernext/inputdd/mousedd/mseconfig.c, inputdd, bos41J, 9509A_all 2/14/95 13:20:45";
/*
 * COMPONENT_NAME: (INPUTDD) Mouse DD - mseconfig.c
 *
 * FUNCTIONS: mseconfig, cleanup, addswitch, qvpd, get_id, initadpt, initmse
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


#include  "mse.h"

lock_t  mse_lock=LOCK_AVAIL;           /* process level lock                */


/*****************************************************************************/
/*                                                                           */
/* NAME:             msesconfig                                              */
/*                                                                           */
/* FUNCTION:         This module manages the task of allocating storage for  */
/*                   and configuring the mouse                               */
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

int  mseconfig(dev_t devno, long cmd, struct uio *uiop)
{

  struct ktsmdds    dds;               /* local copy of dds                  */
  int               rc = 0;

  KTSMDTRACE(mseconfig, enter, devno, cmd, 0, 0, 0);

                                       /* get lock                           */
  if (lockl(&mse_lock,LOCK_SHORT) != LOCK_SUCC) {
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

                                       /* allocate local storage             */
        if (!local) {    
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
                                       /* initialize sleep anchor            */
          local->com.asleep = EVENT_NULL;
        }

                                       /* configuring adapter                */
        if (dds.device_class == ADAPTER) {
          rc = initadpt(&dds, devno);
        }
        else {                         /* configuring mouse                  */
          if (dds.device_class == MOUSE) {
            rc = initmse(&dds, devno);
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
                                       /*   error if mouse is configured     */
            if (local->mse_devno) {
                rc = EBUSY;
            }
            else {                     /*   mark adapter as termianted       */
              local->com.adpt_devno = 0;
            }
          }
          else {                       /* terminating mouse                  */
            if (devno == local->mse_devno) {
              if (local->oflag) {      /*   error if device is open          */
                rc = EBUSY;
              }
              else {                   /*   mark mouse as terminated         */
                local->mse_devno = 0;
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
    unlockl(&mse_lock);                /* release lock                       */
  }

  KTSMTRACE(mseconfig, exit, rc, devno, cmd, 0, 0);

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
       (!local->mse_devno)) {

       devswdel(devno);                /* clear switch table                 */

       if (local->com.stimer)          /* free I/O wait timer                */
         tfree(local->com.stimer);
       if (local->com.wdtimer)         /* free watch dog timer               */
         tfree(local->com.wdtimer);
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
/* NAME:             initmse                                                 */
/*                                                                           */
/* FUNCTION:         This module initializes the mouse                       */
/*                                                                           */
/* INPUTS:           dds_ptr = pointer to dds structure                      */
/*                   devno = device number                                   */
/*                                                                           */
/* OUTPUTS:          0 = ok                                                  */
/*                   !0 = error                                              */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int initmse(struct ktsmdds *dds_ptr, dev_t devno)
{
  int  rc = 0;

  if (!local->com.adpt_devno) {        /* bad news if adapter has not been   */
    rc = EINVAL;                       /* configured                         */
  }
  else {
   if (!local->mse_devno) {            /* skip if mouse already configured   */

                                       /* if run time config then            */
    if (dds_ptr->ipl_phase == RUNTIME_CFG) {
      rc = get_id();                   /*   reset mouse and get ID           */
      if (!rc) {                       /*   if physical mouse is not same as */
                                       /*   defined mouse then               */
        if (dds_ptr->device_type != local->mouse_type) {
          rc = ENODEV;                 /*     set error code                 */
        }
      }
    }
    else {                             /* else ipl config so just            */
                                       /*   set device type from dds         */
      local->mouse_type = dds_ptr->device_type;
    }

    if (!rc) {                         /* if all ok, mark mouse configured   */
      local->mse_devno = devno;        /* by saving device number            */
    }
   }
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
   tmp_devsw.d_open     = (int (*) ()) mseopen;
   tmp_devsw.d_close    = (int (*) ()) mseclose;
   tmp_devsw.d_read     = (int (*) ()) nodev;
   tmp_devsw.d_write    = (int (*) ()) nodev;
   tmp_devsw.d_ioctl    = (int (*) ()) mseioctl;
   tmp_devsw.d_strategy = (int (*) ()) nodev;
   tmp_devsw.d_select   = (int (*) ()) nodev;
   tmp_devsw.d_config   = (int (*) ()) mseconfig;
   tmp_devsw.d_print    = (int (*) ()) nodev;
   tmp_devsw.d_dump     = (int (*) ()) nodev;
   tmp_devsw.d_mpx      = (int (*) ()) nodev;
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
/*                   are connected to adapter                                */
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

                                       /* get mouse ID if mouse has          */
   if (!local->mse_devno) get_id();    /* not yet been configured            */

   switch (local->mouse_type)
   {
      case MOUSE3B:
         bcopy("mouse/std_m/mse_3b mouse ", data ,25);
         index = 25;
         break;
      case MOUSE2B:
         bcopy("mouse/std_m/mse_2b mouse ", data ,25);
         index = 25;
         break;
      default:
         break;
   }

   data[index] = '\0';                 /* null terminate                     */

                                       /* write data out to user space       */
   rc = uiomove(data, index+1, UIO_READ, uiop);
   if (rc < 0) rc = EPERM;

   return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             get_id                                                  */
/*                                                                           */
/* FUNCTION:         Reset device and get ID                                 */
/*                                                                           */
/* INPUTS:           None                                                    */
/*                                                                           */
/* OUTPUTS:          0 = successful                                          */
/*                   EIO                                                     */
/*                   EPERM                                                   */
/*                   ENOMEM                                                  */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int get_id(void)
{
  int rc, retry;
  IFRAME  buff[3];
  struct  common *com;
  int cnt;

  local->mouse_type = 0;

  if (rc=reg_intr(TRUE)) return(rc);   /* define interrupt handler           */

  rc = EIO;
  retry = R_RETRY;
  com = &local->com;                   /* pointer to common area             */
  
  while(retry--) {
                                       /* send RESET cmd to mouse            */
    put_oq1(com, (OFRAME) M_RESET);
                                       /* wait for completion of RESET       */
    if ((cnt = get_iq(com, 2, RESETTO, buff)) != 2) {
      if (cnt >= 0) {                  /* if no response then                */
        if (retry) continue;           /* try again if retries not exhausted */
                                       /* else log error                     */
        io_error("get_id", FALSE, MIO_ERROR, 0, "%08x %04x",
            local->mouse_rpt, com->in_progress);
      }
    }
    else {
      if (buff[0] != M_BAT_CC) {       /* log error if invalid BAT rcv'ed    */
        io_error("get_id", FALSE, MDDBAT_ERROR, 1, "%02x %02x",
             buff[0], buff[1]);
      }
      else {                           /* log error if invalid id received   */
        if (buff[1] != M_DEFAULT_ID) {
          io_error("get_id", FALSE,  MDDID_ERROR, 2, "%02x %02x",
              buff[0], buff[1]);
        }
        else {                         /* send cmds to get number of buttons */
          put_oq(com, mse_button_cmds);
                                       /* get button info                    */
          if ((cnt = get_iq(com,3, RCVTO, buff)) != 3) {
            if (cnt >= 0) {            /* if no response then                */
              if (retry) continue;     /* try again if retries not exhausted */
                                       /* else log error                     */
              io_error("get_id", FALSE, MIO_ERROR, 3, "%08x %04x",
                 local->mouse_rpt, com->in_progress);
            }
          }
          else {

            switch (buff[1]) {
                                       /* 3 button mouse attached            */
              case M_LOG_3_BUTTON:
                local->mouse_type = MOUSE3B;
                rc = 0;
                break;

              case M_LOG_2_BUTTON:
              case M_IBM_2_BUTTON:
                local->mouse_type = MOUSE2B;
                rc = 0;
                break;

              default:                 /* unsupported mouse attached         */
                 io_error("get_id", FALSE, MDDTYPE_ERROR, 4, "%02x", buff[1]);
            }
          }
        }
      }
    }
    break;
  }
  ureg_intr();                         /* unregister interrupt handler       */
  return(rc);
}
