static char sccsid[] = "@(#)94   1.1  src/bos/kernext/inputdd/mousedd/msefns.c, inputdd, bos411, 9428A410j 10/24/93 14:43:49";
/*
 * COMPONENT_NAME: (INPUTDD) Mouse DD - msefns.c
 *
 * FUNCTIONS:  mseopen, mseclose,  mseioctl, send_cmd
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "mse.h"

/*****************************************************************************/
/*                                                                           */
/* NAME:        mseopen                                                      */
/*                                                                           */
/* FUNCTION:    Process open request                                         */
/*                                                                           */
/* INPUTS:      dev = device major/minor number                              */
/*              flag = open flags                                            */
/*              chan = channel number  (not used)                            */
/*              ext = extension parms  (not used)                            */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
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

int  mseopen(dev_t dev, uint flag, chan_t chan, caddr_t ext)
{

  int  rc;

  KTSMDTRACE0(mseopen, enter);
                                       /* try and get lock                   */
  if (lockl(&mse_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = EPERM;
  }
  else {
    if (!local) {                      /* error if no local storage          */
      rc = EPERM;
    }
    else {
      if (dev != local->mse_devno)     /* error if invalid device number     */
        rc = ENXIO;

      else {                           /* error if device already open       */
        if (local->oflag)
          rc = EBUSY;

        else {                      
          if (flag & DKERNEL) {        /* error if open from kernel proc     */
            rc = EACCES;
          }
          else {
                                       /* reset to default                   */
            local->mouse_hor_accum    = 0;  
            local->mouse_hor_thresh   = DEFAULT_THRESHOLD;
            local->mouse_ver_accum    = 0;
            local->mouse_ver_thresh   = DEFAULT_THRESHOLD;
            local->mouse_resolution   = DEFAULT_RESOLUTION;
            local->mouse_sample_rate  = DEFAULT_SAMPLE_RATE;
            local->mouse_scaling      = DEFAULT_SCALING;
            local->button_status      = 0;
            local->mouse_overflow     = 0;
                                       /*indicate no input ring              */
            local->rcb.ring_ptr = NULL;
                                       /*   save pid of owner                */
            local->rcb.owner_pid = getpid();
          
                                       /* pin code and register intr handler */
            if (!(rc = reg_intr(FALSE))) {
                                       /* set mouse to default condition     */
              if (rc = send_cmd((OFRAME) M_SET_DEFAULT, (OFRAME) NO_PARM)) {
                ureg_intr();           /* failed, unregister intr handler    */
              }
              else {                   /* ok, indicate device is open        */
                local->oflag = TRUE;
              }
            }
          }
        }
      }
    }
    unlockl(&mse_lock);                /* free lock                          */
  }

  KTSMTRACE1(mseopen, exit,  rc);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        mseclose                                                     */
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

int  mseclose(dev_t dev, chan_t chan, caddr_t ext)
{
  int rc = 0;

  KTSMDTRACE0(mseclose, enter);
                                       /* try and get lock                   */
  if (lockl(&mse_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = EPERM;
  }
  else {
    if (!local)                        /* error if no local storage          */
      rc = EPERM;

    else {
      if (dev != local->mse_devno)     /* error if invalid device number     */
        rc = ENXIO;

      else {                           /* continue only if device open       */
        if (local->oflag) {
                                       /* unregister input ring              */
          ktsm_uring(&local->rcb);
                                       /* channel is no longer open          */
          local->oflag = FALSE;

          ureg_intr();                 /* unregister intr handler            */
        }
      }
    }
    unlockl(&mse_lock);                /* free lock                          */
  }

  KTSMTRACE1(mseclose, exit,  rc);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        mseioctl                                                     */
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


int  mseioctl(dev_t dev, int cmd, caddr_t arg, uint flag,
              chan_t chan, caddr_t  ext)
{
  int   rc = 0;
  ulong ldata;
  uint  data;
  short hor_thresh;
  short ver_thresh;
  struct devinfo info;


  KTSMDTRACE0(mseioctl, enter);
 
                                       /* try and get lock                   */
  if (lockl(&mse_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = EPERM;
  }
  else {
    if (!local) {                      /* error if no local storage          */
      rc = EPERM;
    }
    else {
      if (dev != local->mse_devno) {   /* error if invalid device number     */
        rc = ENXIO;
      }
      else {
        if (!local->oflag) {           /* error if device is not open        */
          rc = EACCES;
        }
        else {                         /* if all ok, process cmd             */
          switch(cmd) {
                                       /*------------------------------------*/
                                       /* Return devinfo structure           */
                                       /*------------------------------------*/

            case IOCINFO:              /* error if NULL argument passed      */
              if (arg == (caddr_t) NULL) {
                rc = EINVAL;
              }
              else {                   /* zero devinfo structure             */
                 bzero(&info, sizeof(info));
                                       /* set type field                     */
                 info.devtype = INPUTDEV;
                                       /* copy to caller's memory            */
                 rc = copyout((char *) &info, (char *) arg,
                        sizeof(info));
              }
              break;
                                       /*------------------------------------*/
                                       /* query mouse device identifier      */
                                       /*------------------------------------*/

            case MQUERYID:             /* error if NULL argument passed      */
              if (arg == (caddr_t) NULL) {
                rc = EINVAL;
              }
              else {                   /* copy id to user space              */
                rc = copyout((char *) &local->mouse_type, (char *) arg,
                     sizeof(uint));
              }
              break;
                                       /*------------------------------------*/
                                       /* register input ring                */
                                       /*------------------------------------*/

            case MREGRING:             /* error if NULL argument passed      */
              if (arg == (caddr_t) NULL)  {
                rc = EINVAL;
              }
              else {                   /* register input ring in user space  */
                rc = ktsm_rring(&local->rcb, (char *) arg);
              }

              break;
                                       /*------------------------------------*/
                                       /* flush input ring                   */
                                       /*------------------------------------*/
            case MRFLUSH:
              ktsm_rflush(&local->com,&local->rcb);
              break;

                                       /*------------------------------------*/
                                       /* set mouse report threshold         */
                                       /*------------------------------------*/
            case MTHRESHOLD:           /* error if NULL argument passed      */
              if (arg == (caddr_t) NULL)  {
                rc = EINVAL;
              }
              else {                   /* get calling parm                   */
                if (!(rc = copyin(arg, (char *) &ldata,
                    sizeof(ldata)))) {
                  hor_thresh = (short) (ldata >> 16);
                  ver_thresh = (short) (ldata & 0xffff);
                                       /* set error code if invalid parms    */
                  if ((hor_thresh < MIN_MOUSE_THRESHOLD)     ||
                      (hor_thresh > MAX_MOUSE_THRESHOLD)     ||
                      (ver_thresh < MIN_MOUSE_THRESHOLD)     ||
                      (ver_thresh > MAX_MOUSE_THRESHOLD))
                         rc = EINVAL;
                  else {               /* set new threshold if valid parms   */
                    local->mouse_hor_thresh = hor_thresh;
                    local->mouse_ver_thresh = ver_thresh;
                                       /* reset accumulations                */
                    local->mouse_hor_accum  = 0;
                    local->mouse_ver_accum  = 0;
                  }
                }
              }

              break;
                                       /*------------------------------------*/
                                       /* set mouse resolution               */
                                       /*------------------------------------*/
            case MRESOLUTION:          /* error if NULL argument passed      */
              if (arg == (caddr_t) NULL)  {
                rc = EINVAL;
              }
              else {                   /* get calling parm                   */
                 if (!(rc = copyin(arg, (char *) &data,
                    sizeof(data)))) {
                                       /* if resolution changed then         */
                  if (local->mouse_resolution != data) {

                    switch (data) {    /* send change to mouse device ...    */

                      case MRES1:      /*    1 count per mm                  */
                        rc = send_cmd((OFRAME) M_SET_RES, (OFRAME) M_RES_1);
                        break;

                      case MRES2:      /*    3 counts per mm                 */
                        rc = send_cmd((OFRAME) M_SET_RES, (OFRAME) M_RES_3);
                        break;

                      case MRES3:      /*    6 counts per mm                 */
                        rc = send_cmd((OFRAME) M_SET_RES, (OFRAME) M_RES_6);
                        break;

                      case MRES4:      /*    12 counts per mm                */
                        rc = send_cmd((OFRAME) M_SET_RES, (OFRAME) M_RES_12);
                        break;

                      default:         /*    invalid request                 */
                        rc = EINVAL;
                    }
                    if (!rc) {         /* save resolution if no errors       */
                      local->mouse_resolution = data;
                    }
                  }
                }
              }

              break;
                                       /*------------------------------------*/
                                       /* set mouse scale factor             */
                                       /*------------------------------------*/
            case MSCALE:               /* error if NULL argument passed      */
              if (arg == (caddr_t) NULL)  {
                rc = EINVAL;
              }
              else {                   /* get calling parm                   */
                if (!(rc = copyin(arg, (char *) &data,
                    sizeof(data)))) {
                                       /* if scale factor changed then       */
                  if (local->mouse_scaling != data) {

                    switch (data) {    /* send change to mouse device ...    */

                      case MSCALE11:   /*    1 to 1 scale                    */
                        rc = send_cmd((OFRAME) M_SET_SCL_2_1,(OFRAME) NO_PARM);
                        break;

                      case MSCALE21:   /*    2 to 1 scale                    */
                        rc = send_cmd((OFRAME) M_RESET_SCALE,(OFRAME) NO_PARM);
                        break;

                      default:         /*    invalid request                 */
                        rc = EINVAL;
                    }
                    if (!rc) {         /* save scale if no errors            */
                      local->mouse_scaling = data;
                    }
                  }
                }
              }

              break;
                                       /*------------------------------------*/
                                       /* set mouse data sample rate         */
                                       /*------------------------------------*/
            case MSAMPLERATE:          /* error if NULL argument passed      */
              if (arg == (caddr_t) NULL)  {
                rc = EINVAL;
              }
              else {                   /* get calling parm                   */
                if (!(rc = copyin(arg, (char *) &data,
                    sizeof(data)))) {
                                       /* if sample rate changed then        */
                  if (local->mouse_sample_rate != data) {

                    switch (data) {    /* send change to mouse device ...    */

                      case MSR10:      /*    10 samples per second           */
                        rc = send_cmd((OFRAME) M_SET_SAM_RATE,
                              (OFRAME) M_RATE_10);
                        break;

                      case MSR20:      /*    20 samples per second           */
                        rc = send_cmd((OFRAME) M_SET_SAM_RATE,
                              (OFRAME) M_RATE_20);
                        break;

                      case MSR40:      /*    40 samples per second           */
                        rc = send_cmd((OFRAME) M_SET_SAM_RATE,
                              (OFRAME) M_RATE_40);
                        break;

                      case MSR60:      /*    60 samples per second           */
                        rc = send_cmd((OFRAME) M_SET_SAM_RATE,
                              (OFRAME) M_RATE_60);
                        break;

                      case MSR80:      /*    80 samples per second           */
                        rc = send_cmd((OFRAME) M_SET_SAM_RATE,
                              (OFRAME) M_RATE_80);
                        break;

                      case MSR100:     /*    100 samples per second          */
                        rc = send_cmd((OFRAME) M_SET_SAM_RATE,
                              (OFRAME) M_RATE_100);
                        break;

                      case MSR200:     /*    200 samples per second          */
                        rc = send_cmd((OFRAME) M_SET_SAM_RATE,
                              (OFRAME) M_RATE_200);
                        break;

                      default:         /*    invalid request                 */
                        rc = EINVAL;
                    }
                    if (!rc) {         /* save sample rate if no errors      */
                      local->mouse_sample_rate = data;
                    }
                  }
                }
              }

              break;
                                       /*------------------------------------*/
                                       /* invalid command                    */
                                       /*------------------------------------*/
            default:
              rc = EINVAL;


          }
        }
      }
    }
    unlockl(&mse_lock);                /* free lock                          */
  }

  KTSMTRACE(mseioctl, exit,  rc, cmd, 0, 0, 0);

  return(rc);
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        send_cmd                                                     */
/*                                                                           */
/* FUNCTION:    unblock, send command to mouse and re-block                  */
/*                                                                           */
/* INPUTS:      cmd = command for mouse                                      */
/*              parm = command parameter                                     */
/*                                                                           */
/* OUTPUTS:     0 = success                                                  */
/*              EIO    = I/O error                                           */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:
                                              */
/*                                                                           */
/*****************************************************************************/

int send_cmd(OFRAME cmd, OFRAME parm)
{

   long rc;
   int count;
   char data;
   long ldata;
   struct mouse_port *port;
   struct common *com;


   KTSMDTRACE(m_send_cmd, enter,  cmd, parm, 0, 0, 0);

   com = &local->com;                  /* pointer to common structure        */
                                       /* get access to I/O bus              */
   port = (struct mouse_port *) BUSIO_ATT(com->bus_id,com->bus_addr);


/* disable block mode                                                        */

   if (!local->block_mode) {           /* nothing to do if block mode is     */
     rc = 0;                           /* already disabled                   */
   }
   else {                              /* block mode is enabled so ...       */
     if (!(rc = disable_intr(port))) { /* disable mouse interrupts           */
       rc = EIO;                       /* assume failure                     */
       for (count=0;count<M_RETRY;count++) {
                                       /* send adapter disable block mode cmd*/
         if (!write_port(&port->w_data_cmd, (short) DIS_BLK_MODE)) break;
                                       /* get adapter status                 */
         if (!read_char(&port->r_astat_reg, &data)) break;
         if (data & CMD_RCVD_BLK_DIS) {/* if command received then           */
            if (data & CMD_RCVD_BUSY) {/*  if busy wait 1 frame time         */
               ktsm_sleep(com,DMBSY);  /*   (1.2 ms) for busy to clear       */
                                       /*  get adapter status                */
               if (!read_char(&port->r_astat_reg, &data)) break;
                                       /*  I/O error if still busy           */
               if (data & CMD_RCVD_BUSY)  {
                  io_error("send_cmd", TRUE, MIO_ERROR, 0, "%02", data);
                  break;
               }
            }
                                       /* flush adapter fifo                 */
            if (!read_port(&port->r_mdata_regs, &ldata)) break;
            if (!read_port(&port->r_mdata_regs, &ldata)) break;
            if (!read_port(&port->r_mdata_regs, &ldata)) break;
            if (!read_port(&port->r_mdata_regs, &ldata)) break;
            rc = enable_intr(port);    /* enable interrupts                  */
            break;
         }
       }
       if (count == M_RETRY) {         /* log error if can not unblock       */
         io_error("send_cmd", TRUE, MIO_ERROR, 1, "%02x", data);
       }
     }
   }

/* send commands to mouse and wait for completion                            */

   if (!rc) {                         /* if no errors so far ...             */
                                      /* disable mouse reports               */
      put_oq1(com,(OFRAME) M_DISABLE);  
      if (parm == NO_PARM) {          /* if no parm, send only 1 frame       */
         put_oq1(com, cmd);
      }
      else {                          /* else send 2 frames                  */
         put_oq2(com, cmd, parm);
      }
      rc = wait_oq(com);              /* wait for completion                 */
   }

/* put mouse adapter in block mode and enable mouse reports                  */

   if (!rc) {                         /* if no errors so far ...             */
      rc = EIO;                       /* assume error                        */
                                      /* retry loop                          */
      for (count=0;count<M_RETRY;count++) {
                                      /* send adapter enable block mode cmd  */
         if (!write_port(&port->w_data_cmd, (short)  EN_BLK_MODE)) break;
                                      /* get adapter status                  */
         if (!read_char(&port->r_astat_reg, &data)) break;
                                      /* if in block mode                    */
         if (data & BLK_MODE_EN) {
            local->block_mode = TRUE; /*   in block mode                     */
                                      /*   enable mouse reports              */
            put_oq1(com, (OFRAME) M_ENABLE); 
            rc = wait_oq(com);        /*   wait for completion               */
            break;
         }
      }
      if (count == M_RETRY) {         /* log error if can not reblock        */
        io_error("send_cmd", TRUE, MIO_ERROR, 2, "%02x", data);
      }
   }

   BUSIO_DET(port);                    /* release access to the I/O bus      */

   KTSMDTRACE1(m_send_cmd, exit, rc);

   return(rc);
}


