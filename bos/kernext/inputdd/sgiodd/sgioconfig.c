static char sccsid[] = "@(#)61	1.5  src/bos/kernext/inputdd/sgiodd/sgioconfig.c, inputdd, bos41J, 9520A_all 5/15/95 16:41:53";
/*
 * COMPONENT_NAME: (INPUTDD) Serial Graphics I/O device driver
 *
 * FUNCTIONS: sgio_config, sgio_open, sgio_close, sgio_ioctl, sgio_watchdog,
 *            sgio_sleep, addswitch, add_dev, remove_dev, find_dev
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/sgio.h>

#ifdef GS_DEBUG_TRACE
   GS_TRC_GLB(0,TRC_PRIORITY);
   GS_TRC_MODULE(sgiodd, 0);
#endif

sgio_t sgio = {0, 0, FALSE, FALSE, 0, 0, 0, FALSE, FALSE, PM_NONE,
                      NULL, LOCK_AVAIL, NULL, LOCK_AVAIL};
sgio_t *sgio_ptr = &sgio;


/*****************************************************************************/
/*                                                                           */
/* NAME:        sgio_config                                                  */
/*                                                                           */
/* FUNCTION:    This routine manages the task of allocating storage for      */
/*              and configuring serial GIO devices.                          */
/*                                                                           */
/* INPUTS:      devno = major and minor device number                        */
/*              cmd   = command to perform (i.e. initialize, terminate)      */
/*              uiop  = pointer to a uio structure containing the dds        */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              EBUSY  = device aleady configured on CFG_INIT                */
/*                       device not closed on CFG_TERM                       */
/*              EINVAL = invalid user structure passed                       */
/*              EIO    = invalid command passed                              */
/*              ENOMEM = unable to allocate or pin memory                    */
/*              ENXIO  = invalid device passed on CFG_TERM                   */
/*              EPERM  = unsuccessful requests (i.e. fp_open, create_sikproc)*/
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/
int sgio_config(dev_t devno, int cmd, struct uio *uiop)
{
  int old_priority, rc = 0;
  char tty[12];
  struct termio termio;
  sgio_device_t *device;

  SGIODTRACE(sgio_config, enter, devno, cmd, 0, 0, 0);

  switch (cmd)
  {
    case CFG_INIT:

      /* If no user structure passed... */
      if (uiop == (struct uio *) NULL)
      {
        rc = EINVAL;
        break;
      }

      /* Allocate sgio device structure */
      if ((device = (struct sgio_device *)
         xmalloc(sizeof(struct sgio_device), 3, pinned_heap)) == NULL)
      {
        rc = ENOMEM;
        break;
      }
      bzero(device, sizeof(struct sgio_device));

      /* Copy DDS into sgio device structure from user structure */
      if (uiomove((caddr_t) &device->device_dds, sizeof(struct sgio_dds),
          UIO_WRITE, uiop) != 0)
      {
        xmfree(device, pinned_heap);
        rc = EPERM;
        break;
      }

      /* Open tty for serial device operations */
      if ((rc = sgio_tty_open(device)) != 0)
      {
        xmfree(device, pinned_heap);
        rc =  EPERM;
        break;
      }

      /* Allocate timer and complete device initialization */
      device->devno = devno;
      device->device_open = FALSE;
      device->recent_io = TRUE;
      device->rcb.ring_ptr = (caddr_t) NULL;
      device->device_lock = LOCK_AVAIL;
      device->command = SGIO_INIT_CMD;
      device->status = NONE;
      device->pm_status = PM_NONE;
      device->timer = (struct trb *) talloc();
      bzero(device->timer, sizeof(struct trb));
      device->timer->ipri = SGIO_PRIORITY;
      device->timer->flags |= T_INCINTERVAL;
      device->timer->func = (void (*) ()) sgio_watchdog;
      device->timer->t_func_addr = (caddr_t) &device->sleep_event;
      device->sleep_event = EVENT_NULL;
      device->retry_timer = (struct trb *) talloc();
      bzero(device->retry_timer, sizeof(struct trb));
      device->retry_timer->ipri = SGIO_PRIORITY;
      device->retry_timer->flags |= T_INCINTERVAL;

      /* Lock device list for critical section */
      lockl(&sgio.sgio_lock, LOCK_SHORT);

      /* If this is the first initialized device... */
      if (sgio_ptr->device_count == 0)
      {

        /* Add device to switch table */
        if ((rc = addswitch(devno)) != 0)
        {
          devswdel(devno);
          fp_close(device->fp);
          xmfree(device, pinned_heap);
          unlockl(&sgio.sgio_lock);
          rc =  EPERM;
          break;
        }

        /* Pin sgio anchor structure */
        if ((rc = pincode(sgio_config)) != 0)
        {
          devswdel(devno);
          fp_close(device->fp);
          xmfree(device, pinned_heap);
          unlockl(&sgio.sgio_lock);
          rc = ENOMEM;
          break;
        }

        /* Create sikproc */
        if ((rc = create_sikproc()) != 0)
        {
          unpincode(sgio_config);
          devswdel(devno);
          fp_close(device->fp);
          xmfree(device, pinned_heap);
          unlockl(&sgio.sgio_lock);
          rc =  EPERM;
          break;
        }
      }

      /* Get device from list of initialized devices. If device
         is found, then it is already initialized. */
      if (find_dev(devno, TRUE) != NULL)
      {
        fp_close(device->fp);
        xmfree(device, pinned_heap);
        unlockl(&sgio.sgio_lock);
        rc =  EBUSY;
        break;
      }

      /* Insert device into list of initialized devices */
      add_dev(device);

      /* Unlock device list for critical section */
      unlockl(&sgio.sgio_lock);

      /* Signal sikproc and sleep */
      sgio_sleep(device->timer, 5, 0, sgio_ptr->sikproc_pid, &device->status);

      /* If initialization was not successful... */
      if (device->status != SUCCESS)
      {
        /* If sikproc failed to respond at all... */
        if (device->status == PENDING)
        {
          /* Log error */
          io_error("sgio_config", TRUE, SIKPROC_ERROR, 1, "%d", devno);
        }

        /* Lock device list for critical section */
        lockl(&sgio.sgio_lock, LOCK_SHORT);

        remove_dev(device);

        /* If no initialized devices in list... */
        if (sgio_ptr->device_count == 0)
        {
          devswdel(devno);
          unlockl(&sgio.sgio_lock);
          term_sikproc();         
          unpincode(sgio_config);
        }
        else
          unlockl(&sgio.sgio_lock);

        /* Cleanup device */
        tfree(device->timer);
        fp_close(device->fp);
        xmfree(device, pinned_heap);

        rc =  EPERM;
        break;
      }

      /* Register pm handle structure to pm core */
      reg_pm(device);

      /* Mark device initialization complete; LAST ITEM COMPLETED */
      device->command = NONE;

      break;

    case CFG_TERM:

      /* Lock device list for critical section */
      lockl(&sgio.sgio_lock, LOCK_SHORT);

      /* If device not initialized... */
      if ((device = find_dev(devno, FALSE)) == NULL)
      {
        unlockl(&sgio.sgio_lock);
        rc =  ENXIO;
        break;
      }

      /* If device is either open or opening/closing... */
      if ((device->device_open == TRUE) || (device->command != NONE))
      {
        unlockl(&sgio.sgio_lock);
        rc = EBUSY;
        break;
      }

      /* Set device termination command */
      device->command = SGIO_TERM_CMD;

      /* Unregister pm handle structure */
      ureg_pm(device);

      /* Remove device from list of initialized devices */
      remove_dev(device);

      /* If device list is empty... */
      if (sgio_ptr->device_count == 0)
      {
        /* Remove device from switch table */
        devswdel(devno);
        unlockl(&sgio.sgio_lock);
        term_sikproc();
        unpincode(sgio_config);
      }
      else
        unlockl(&sgio.sgio_lock);

      /* Cleanup device */
      tfree(device->timer);
      fp_close(device->fp);
      xmfree(device, pinned_heap);

      break;

    default:

      /* Invalid */
      rc = EIO;
  }

  SGIOTRACE(sgio_config, exit, rc, devno, cmd, 0, 0);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sgio_open                                                    */
/*                                                                           */
/* FUNCTION:    This routine processes an open request.                      */
/*                                                                           */
/* INPUTS:      devno  = device major/minor number                           */
/*              rwflag = open flags                                          */
/*              chan   = channel number (not used)                           */
/*              ext    = extension parameters (not used)                     */
/*                                                                           */
/* OUTPUTS:     0     = success                                              */
/*              EBUSY = device already open                                  */
/*              ENXIO = invalid device number                                */
/*              EPERM = sikproc failed to respond                            */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/
int sgio_open(dev_t devno, int rwflag, chan_t chan, int ext)
{
  int i, old_priority, rc = 0;
  sgio_device_t *device;

  SGIODTRACE(sgio_open, enter, devno, 0, 0, 0, 0);

  /* Lock device list for critical section */
  lockl(&sgio.sgio_lock, LOCK_SHORT);

  if ((device = find_dev(devno, FALSE)) == NULL)
  {
    unlockl(&sgio.sgio_lock);
    rc = ENXIO;
  }
  else
  {
    /* If device already open or already has a command in progress... */
    if ((device->device_open == TRUE) || (device->command != NONE))
    {
      unlockl(&sgio.sgio_lock);
      rc = EBUSY;
    }
    else
    {
      device->pm.activity = 1;
      device->command = SGIO_OPEN_CMD;
      sgio_ptr->list_changed = TRUE;
      sgio_ptr->open_count++;
      unlockl(&sgio.sgio_lock);

      /* Lock device list for critical section */
      lockl(&device->device_lock, LOCK_SHORT);

      if (device->device_dds.device_class == S_DIALS)
      {
        device->device_data.dials.select = 0x00;
        for (i = 0; i < 8; i++)
          if (device->device_data.dials.granularity[i] != DIAL128RPR)
          {
            device->device_data.dials.granularity[i] = DIAL128RPR;
            device->device_data.dials.select |= (1 << i);
          }
      }
      else if (device->device_dds.device_class == S_LPFKS)
      {
        for (i = 0; i < 4; i++)
          if (device->device_data.lpfks[i] != 0x00)
            device->device_data.lpfks[i] = 0x00;
        device->retry_count = 3;
      }

      /* Signal sikproc and sleep */
      sgio_sleep(device->timer, 5, 0, sgio_ptr->sikproc_pid, &device->status);

      /* If command was successful... */
      if (device->status == SUCCESS)
      {
        device->device_open = TRUE;
      }
      else
      {
        lockl(&sgio.sgio_lock, LOCK_SHORT);
        sgio_ptr->open_count--;
        unlockl(&sgio.sgio_lock);
        
        /* If sikproc did not respond at all... */
        if (device->status == PENDING)
        {
          /* Log error */
          io_error("sgio_open", TRUE, SIKPROC_ERROR, 1, "%d", devno);
        }
        rc = EPERM;
      }

      device->command = NONE;
      unlockl(&device->device_lock);
    }
  }

  SGIOTRACE(sgio_open, exit, rc, devno, 0, 0, 0);
  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sgio_close                                                   */
/*                                                                           */
/* FUNCTION:    This routine processes an close request.                     */
/*                                                                           */
/* INPUTS:      devno = device major/minor number                            */
/*              chan  = channel number (not used)                            */
/*              ext   = extension parameters (not used)                      */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              EINVAL = device not open or command in progress              */
/*              ENXIO  = invalid device number                               */
/*              EPERM  = sikproc failed to respond                           */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/
int sgio_close(dev_t devno, chan_t chan, int ext)
{
  sgio_device_t *device;
  int old_priority, rc = 0;

  SGIODTRACE(sgio_close, enter, devno, 0, 0, 0, 0);

  lockl(&sgio.sgio_lock, LOCK_SHORT);

  if ((device = find_dev(devno, FALSE)) == NULL)
  {
    unlockl(&sgio.sgio_lock);
    rc = ENXIO;
  }
  else
  {
    /* If device not open or already has a command in progress... */
    if ((device->device_open == FALSE) || (device->command != NONE))
    {
      unlockl(&sgio.sgio_lock);
      rc = EINVAL;
    }
    else
    {
      device->pm.activity = 1;
      device->command = SGIO_CLOSE_CMD;
      sgio_ptr->list_changed = TRUE;
      sgio_ptr->close_count++;
      unlockl(&sgio.sgio_lock);

      /* Lock device list for critical section */
      lockl(&device->device_lock, LOCK_SHORT);

      /* Signal sikproc and sleep */
      sgio_sleep(device->timer, 5, 0, sgio_ptr->sikproc_pid, &device->status);
    
      /* Unregister input ring */
      sgio_uring(&device->rcb);
  
      /* If command was successful... */
      if (device->status == SUCCESS)
      {
        device->device_open = FALSE;
      }
      else
      {
        /* If sikproc did not respond at all... */
        if (device->status == PENDING)
        {
          /* Log error */
          io_error("sgio_close", TRUE, SIKPROC_ERROR, 3, "%d", devno);
        }
        rc = EPERM;
      }

      device->command = NONE;
      unlockl(&device->device_lock);

      /* Decrement device list counts */
      lockl(&sgio.sgio_lock, LOCK_SHORT);
      sgio_ptr->close_count--;
      sgio_ptr->open_count--;
      unlockl(&sgio.sgio_lock);
    }
  }

  SGIOTRACE(sgio_close, exit, rc, devno, 0, 0, 0);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sgio_ioctl                                                   */
/*                                                                           */
/* FUNCTION:    This routine processes an ioctl request.                     */
/*                                                                           */
/* INPUTS:      devno = device major/minor number                            */
/*              cmd   = ioctl command to perform                             */
/*              arg   = ioctl argument pointer                               */
/*              chan  = channel number (not used)                            */
/*              ext   = extension parameters (not used)                      */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              EINVAL = device not open or command in progress or invalid   */
/*                       parameters passed                                   */
/*              ENXIO  = invalid device number                               */
/*              EPERM  = sikproc failed to respond                           */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/
int sgio_ioctl(dev_t devno, int cmd, void *arg, int mode, chan_t chan, int ext)
{
  int i,j, rc = 0, changed = FALSE, old_priority;
  unsigned long lpfks;
  unsigned char tmp1, tmp2;
  struct devinfo device_info;
  struct dialsetgrand dials;
  sgio_device_t *device;

  SGIODTRACE(sgio_ioctl, enter, devno, cmd, 0, 0, 0);

  if (mode&DKERNEL)
    rc = EINVAL;
  else
  {
    lockl(&sgio.sgio_lock, LOCK_SHORT);

    /* If device not initialized... */
    if ((device = find_dev(devno, FALSE)) == NULL)
    {
      unlockl(&sgio.sgio_lock);
      rc = ENXIO;
    }
    else
    {
      /* If device not open or already has a command in progress... */
      if ((device->device_open == FALSE) || (device->command != NONE))
      {
        unlockl(&sgio.sgio_lock);
        rc = EINVAL;
      }
      else
      {
        device->pm.activity = 1;

        /* Set appropriate command type */
        if (cmd == DIALSETGRAND)
          device->command = SGIO_DIALS_CMD;
        else if (cmd == LPFKLIGHT)
          device->command = SGIO_LPFKS_CMD;
        else
          device->command = SGIO_IOCTL_CMD;
 
        unlockl(&sgio.sgio_lock);
  
        /* Lock device list for critical section */
        lockl(&device->device_lock, LOCK_SHORT);
  
        switch (cmd)
        {
          case IOCINFO:

            SGIODTRACE0(sgio_ioctl, iocinfo);
      
            /* If no argument passed... */
            if (!arg)
            {
              rc = EINVAL;
              break;
            }
      
            /* Initialize and pass device_info structure to user */
            bzero(&device_info, sizeof(struct devinfo));
            device_info.devtype = DD_INPUT;
            rc = copyout(&device_info, arg, sizeof(struct devinfo));
      
            break;
      
          case DIALREGRING:
          case LPFKREGRING:

            SGIODTRACE(sgio_ioctl, regring, cmd,
              device->device_dds.device_class, arg, 0, 0);
      
            /* If no argument passed... */
            if (!arg)
            {
              rc = EINVAL;
              break;
            }
      
            /* If ioctl type does not match device type... */
            if (((device->device_dds.device_class == S_DIALS) &&
               (cmd != DIALREGRING)) ||
               ((device->device_dds.device_class == S_LPFKS) &&
               (cmd != LPFKREGRING)))
            {
              rc = EINVAL;
              break;
            }
      
            /* Register input ring */
            rc = sgio_rring(&device->rcb, arg);
      
            break;
      
          case DIALRFLUSH:
          case LPFKRFLUSH:

            SGIODTRACE(sgio_ioctl, rflush, cmd,
              device->device_dds.device_class, 0, 0, 0);
      
            /* If ioctl type does not match device type... */
            if (((device->device_dds.device_class == S_DIALS) &&
               (cmd != DIALRFLUSH)) ||
               ((device->device_dds.device_class == S_LPFKS) &&
               (cmd != LPFKRFLUSH)))
            {
              rc = EINVAL;
              break;
            }
      
            /* Flush input ring */
            sgio_rflush(SGIO_PRIORITY, &device->rcb);
      
            break;
      
          case DIALSETGRAND:
      
            /* If no argument passed... */
            if (!arg)
            {
              rc = EINVAL;
              break;
            }
      
            /* If device is not a dials device... */
            if (device->device_dds.device_class != S_DIALS)
            {
              rc = EINVAL;
              break;
            }
      
            /* Get user data */
            if ((rc = copyin(arg, &dials, sizeof(struct dialsetgrand))) != 0)
              break;
      
            SGIODTRACE(sgio_ioctl, dialsetgrand, dials.dial_select, 0, 0, 0, 0);

            /* If nothing to do... */
            if (!dials.dial_select)
              break;
      
            /* Process user data */
            device->device_data.dials.select = 0x00;
            for (i = 0; i < 8; i++)
              if ((unsigned char) (dials.dial_select) >> i & 0x01)
              {
                if ((dials.dial_value[i] < DIAL4RPR) ||
                   (dials.dial_value[i] > DIAL256RPR))
                {
                  rc = EINVAL;
                  break;
                }
                
                if (device->device_data.dials.granularity[i] != dials.dial_value[i])
                {
                  device->device_data.dials.granularity[i] = dials.dial_value[i];
                  device->device_data.dials.select |= (1 << i);
                }
              }

            if ((device->device_data.dials.select != 0x00) && (rc != EINVAL))
            {
              /* Signal sikproc and sleep */
              sgio_sleep(device->timer, 5, 0, sgio_ptr->sikproc_pid, &device->status);
              /* If command was not successful... */
              if (device->status != SUCCESS)
              {
                if (device->status == PENDING)
                {
                  /* Log error */
                  io_error("sgio_ioctl", TRUE, SIKPROC_ERROR, 1, "%d", devno);
                }
                rc = EPERM;
              }
            }
      
            break;
      
          case LPFKLIGHT:
      
            /* If no argument passed... */
            if (!arg)
            {
              rc = EINVAL;
              break;
            }
      
            /* If device is not a lpfks device... */
            if (device->device_dds.device_class != S_LPFKS)
            {
              rc = EINVAL;
              break;
            }
      
            /* Get user data */
            if ((rc = copyin(arg, &lpfks, sizeof(unsigned long))) != 0)
              break;
      
            SGIODTRACE(sgio_ioctl, lpfklight, lpfks, 0, 0, 0, 0);

            /* Process user data */
            for (i = 3; i >= 0; i--)
            {
              tmp1 = 0;
              tmp2 = (unsigned char) ((lpfks >> (i*8)) & 0x000000FF);
              for (j = 7; j >= 0; j--)
                 tmp1 = (tmp1 >> 1) | ((tmp2 >> j) & 0x01) << 7;
              if (device->device_data.lpfks[i] != tmp1)
              {
                device->device_data.lpfks[i] = tmp1;
                changed = TRUE;
              }
            }
      
            if (changed == TRUE)
            {
              device->retry_count = 3;

              /* Signal sikproc and sleep */
              sgio_sleep(device->timer, 5, 0, sgio_ptr->sikproc_pid, &device->status);
              /* If command was not successful... */
              if (device->status != SUCCESS)
              {
                if (device->status == PENDING)
                {
                  /* Log error */
                  io_error("sgio_ioctl", TRUE, SIKPROC_ERROR, 2, "%d", devno);
                }
                rc = EPERM;
              }
            }
      
            break;
      
          default:

            rc = EINVAL;
      
            break;
        }
        device->command = NONE;
        unlockl(&device->device_lock);
      }
    }
  }

  SGIOTRACE(sgio_ioctl, exit, rc, devno, cmd, device->status, 0);
  return(rc);
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        addswitch                                                    */
/*                                                                           */
/* FUNCTION:    This routine adds the device driver to the device            */
/*              switch table.                                                */
/*                                                                           */
/* INPUTS:      devno = device major/minor numbers                           */
/*                                                                           */
/* OUTPUTS:     devswadd return code                                         */
/*                0      = success                                           */
/*                EEXIST = device switch entry already in use                */
/*                ENOMEM = insufficient memory                               */
/*                EINVAL = invalid major device number                       */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/
int addswitch(dev_t devno)
{
  struct devsw sgio_dsw;

  /* Initialize device switch structure */
  sgio_dsw.d_open     = (int (*) ()) sgio_open;
  sgio_dsw.d_close    = (int (*) ()) sgio_close;
  sgio_dsw.d_read     = (int (*) ()) nodev;
  sgio_dsw.d_write    = (int (*) ()) nodev;
  sgio_dsw.d_ioctl    = (int (*) ()) sgio_ioctl;
  sgio_dsw.d_strategy = (int (*) ()) nodev;
  sgio_dsw.d_select   = (int (*) ()) nodev;
  sgio_dsw.d_config   = (int (*) ()) sgio_config;
  sgio_dsw.d_print    = (int (*) ()) nodev;
  sgio_dsw.d_dump     = (int (*) ()) nodev;
  sgio_dsw.d_mpx      = (int (*) ()) nodev;
  sgio_dsw.d_revoke   = (int (*) ()) nodev;
  sgio_dsw.d_ttys     = NULL;
  sgio_dsw.d_dsdptr   = NULL;
  sgio_dsw.d_selptr   = NULL;
  sgio_dsw.d_opts     = 0;

  /* Add to the device switch table */
  return(devswadd(devno, &sgio_dsw));
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        add_dev                                                      */
/*                                                                           */
/* FUNCTION:    This routine inserts a device structure into the list        */
/*              of initialized devices.                                      */
/*                                                                           */
/* INPUTS:      device = sgio device structure pointer                       */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              EINVAL = invalid device structure passed                     */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       The initialized device list must be locked prior to          */
/*              calling this routine.                                        */
/*                                                                           */
/*****************************************************************************/
int add_dev(sgio_device_t *device)
{
  int rc = 0;

  SGIODTRACE0(add_dev, enter);

  /* If a valid device structure passed... */
  if (device != NULL)
  {
    /* Add device to list of initialized devices */
    device->next = sgio_ptr->device;
    sgio_ptr->device = device;
    sgio_ptr->device_count++;
  }
  else
    rc = EINVAL;


  SGIOTRACE(add_dev, exit, rc, sgio_ptr->device_count, 0, 0, 0);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        remove_dev                                                   */
/*                                                                           */
/* FUNCTION:    This routine removes a device structure from the list        */
/*              of initialized devices.                                      */
/*                                                                           */
/* INPUTS:      device = sgio device structure pointer                       */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              EINVAL = invalid device structure passed                     */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       The initialized device list must be locked prior to          */
/*              calling this routine.                                        */
/*                                                                           */
/*****************************************************************************/
int remove_dev(sgio_device_t *device)
{
  int rc = 0;
  sgio_device_t *curr, *prev;

  SGIODTRACE0(remove_dev, enter);

  /* If a valid device structure passed... */
  if (device != NULL)
  {
    prev = NULL;
    curr = sgio_ptr->device;
    while ((curr != device) && (curr != NULL))
    {
      prev = curr;
      curr = curr->next;
    }
  
    if (curr != NULL)
    {
      /* If device to be removed is first in list... */
      if (prev == NULL)
        sgio_ptr->device = curr->next;
      else
        prev->next = curr->next;
  
      sgio_ptr->device_count--;
    }
  }
  else
    rc = EINVAL;


  SGIOTRACE(remove_dev, exit, rc, sgio_ptr->device_count, 0, 0, 0);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        find_dev                                                     */
/*                                                                           */
/* FUNCTION:    This routine locates a device structure in the list of       */
/*              initialized devices.                                         */
/*                                                                           */
/* INPUTS:      devno       = device major/minor number                      */
/*              find_config = boolean flag indicating whether configuring    */
/*                            and terminating devices are to be returned     */
/*                                                                           */
/* OUTPUTS:     pointer = pointer to device if located in list. NULL is      */
/*                        returned if device not located.                    */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       The initialized device list must be locked prior to          */
/*              calling this routine.                                        */
/*                                                                           */
/*****************************************************************************/
sgio_device_t *find_dev(dev_t devno, int find_config)
{
  sgio_device_t *device;

  SGIODTRACE(find_dev, enter, devno, find_config, 0, 0, 0);

  if (sgio_ptr == NULL)
    device = NULL;
  else
  {
    device = sgio_ptr->device;
    while (device != NULL)
      if (device->devno == devno)
      {
        /* In device is initializing or terminating... */
        if ((device->command == SGIO_INIT_CMD) || (device->command == SGIO_TERM_CMD))
        {
          if (find_config == FALSE)
            device = NULL;
          break;
        }
        else
          break;
      }
      else
        device = device->next;
  }


  SGIODTRACE(find_dev, exit, devno, device, 0, 0, 0);

  return(device);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sgio_watchdog                                                */
/*                                                                           */
/* FUNCTION:    This routine services watchdog timer interrupts.             */
/*                                                                           */
/* INPUTS:      trb = timer request block pointer                            */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/
void sgio_watchdog(struct trb *trb)
{
  int *event_addr;
 
  event_addr = (int *) trb->t_func_addr;
  e_wakeup(event_addr);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sgio_sleep                                                   */
/*                                                                           */
/* FUNCTION:    This routine places the calling thread to sleep for the      */
/*              specified amount of time. A status word and/or a process id  */
/*              may be specified. If supplied, the status is set to PENDING  */
/*              and the process is signalled accordingly.                    */
/*                                                                           */
/* INPUTS:      timer  = timer request block pointer                         */
/*              sec    = number of of full seconds to sleep                  */
/*              nsec   = number of of full nano-seconds to sleep             */
/*              pid    = process id to signal (optional)                     */
/*              status = status word (optional)                              */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       This device driver's config, open, close and ioctl entry     */
/*              points utilize this routine to synchronize command requests  */
/*              with the serial I/O kernel process. The serial I/O kernel    */
/*              process simply utilizes this routine as a sleep timer. Thus, */
/*              the optional process id and status word.                     */
/*                                                                           */
/*****************************************************************************/
int sgio_sleep(struct trb *timer, int sec, int nsec, pid_t pid, int *status)
{
  int old_priority;

  timer->timeout.it_value.tv_sec = sec;
  timer->timeout.it_value.tv_nsec = nsec;

  /* Raise priority */
  old_priority = i_disable(INTTIMER);

  /* If status word passed... */
  if (status != NULL)
    *status = PENDING;

  /* If process to be signaled... */
  if (pid != 0)
    pidsig(pid, SIGUSR1);

  /* Start timer */
  tstart(timer);

  /* Sleep on event passed */
  e_sleep((int *) timer->t_func_addr, EVENT_SHORT);

  /* Stop timer */
  tstop(timer);

  /* Reset priority */
  i_enable(old_priority);

  return(0);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sgio_tty_open                                                */
/*                                                                           */
/* FUNCTION:    This routine opens and initializes the specified tty for     */
/*              serial device operationslled accordingly.                    */
/*                                                                           */
/* INPUTS:      device = sgio device structure pointer                       */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/
int sgio_tty_open(sgio_device_t *device)
{
  int rc = 0;
  char tty[12];
  struct termio termio;

  SGIODTRACE(sgio_tty_open, enter, device->devno, 0, 0, 0, 0);

  /* Open tty for serial device operations */
  sprintf(tty, "/dev/%s", device->device_dds.tty_device);
  if ((rc = fp_open(tty, O_RDWR|O_NSHARE|O_NONBLOCK ,0 ,0 , SYS_ADSPACE,
       &device->fp)) == 0)
  {
    /* Setup tty characteristics */
    fp_ioctl(device->fp, TCGETA, &termio, 0);
    termio.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    termio.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    termio.c_iflag |= (INPCK | IGNPAR);
    termio.c_oflag &= ~(OPOST);
    termio.c_cflag &= ~(CSIZE | CSTOPB);
    termio.c_cflag |= (CS8 | PARENB | PARODD | CLOCAL);
    termio.c_cc[VMIN] = 1;
    termio.c_cc[VTIME] = 0;
    fp_ioctl(device->fp, TCSETAW, &termio, 0);
  }

  SGIOTRACE(sgio_tty_open, exit, device->devno, rc, 0, 0, 0);

  return(rc);
}
