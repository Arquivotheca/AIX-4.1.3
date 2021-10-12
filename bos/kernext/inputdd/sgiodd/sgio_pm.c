static char sccsid[] = "@(#)28	1.4  src/bos/kernext/inputdd/sgiodd/sgio_pm.c, inputdd, bos41J, 9520A_all 5/15/95 16:41:45";
/*
 * COMPONENT_NAME: (INPUTDD) Serial Graphics I/O device driver
 *
 * FUNCTIONS: reg_pm, ureg_pm, sgio_pm_handler
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

extern sgio_t *sgio_ptr;

/*****************************************************************************/
/*                                                                           */
/* NAME:        reg_pm                                                       */
/*                                                                           */
/* FUNCTION:    This routine initializes a devices pm handle structure and   */
/*              registers it to pm core for Power Management(PM)             */
/*                                                                           */
/* INPUTS:      device = sgio device structure pointer                       */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       Even if an error occurs, serial GIO must work without        */
/*              Power Management(PM)                                         */
/*                                                                           */
/*****************************************************************************/
void reg_pm(struct sgio_device *device)
{
  int    rc = 0;

  SGIODTRACE(reg_pm, enter, device->devno, 0, 0, 0, 0);

  /* Initialize power management timer, if necessary */
  if (sgio_ptr->pm_timer == NULL)
  {
    sgio_ptr->pm_timer = (struct trb *) talloc();
    sgio_ptr->pm_timer->ipri = SGIO_PRIORITY;
    sgio_ptr->pm_timer->flags = T_INCINTERVAL;
    sgio_ptr->pm_timer->func = (void (*) ()) sgio_watchdog;
    sgio_ptr->pm_timer->t_func_addr = (caddr_t) &sgio_ptr->pm_lock;
  }

  /* Make area for logical name string  */
  if((device->pm.device_logical_name = (char *)
    xmalloc(strlen(device->device_dds.devname)+1, 3, pinned_heap)) != NULL)
  {
    /* Initialize pm handle structure */
    device->pm.activity = 0;
    device->pm.mode = PM_DEVICE_FULL_ON;
    device->pm.handler = (int (*) ()) sgio_pm_handler;
    device->pm.private = (caddr_t) device;
    device->pm.devno = device->devno;
    device->pm.attribute = PM_GRAPHICAL_INPUT;
    strcpy(device->pm.device_logical_name, device->device_dds.devname);
    device->pm.pm_version = 0x100;

    /* Register pm structure to pm core */
    if ((rc = pm_register_handle( &device->pm, PM_REGISTER)) == PM_ERROR)
    {
      /* Log error... */
      io_error("reg_pm", FALSE, PM_REG_ERROR, 0, "");

      /* Free allocated area */
      xmfree(device->pm.device_logical_name, pinned_heap);
    }
  }

  SGIOTRACE(reg_pm, exit, device->devno, 0, 0, 0, 0);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        ureg_pm                                                      */
/*                                                                           */
/* FUNCTION:    This routine unregisters pm handle structure                 */
/*                                                                           */
/* INPUTS:      None                                                         */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/
void ureg_pm(struct sgio_device *device)
{
  SGIODTRACE(ureg_pm, enter, device->devno, 0, 0, 0, 0);

  /* Unregister the pm handle structure */
  pm_register_handle(&device->pm, PM_UNREGISTER);

  /* Free area for logical name */
  xmfree(device->pm.device_logical_name, pinned_heap);

  SGIOTRACE(ureg_pm, exit, device->devno, 0, 0, 0, 0);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sgio_pm_handler                                              */
/*                                                                           */
/* FUNCTION:    This routine processes pm requests                           */
/*                                                                           */
/* INPUTS:      None                                                         */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/
long sgio_pm_handler (caddr_t private_data, int mode)
{
  int rc = PM_SUCCESS;
  struct sgio_device *device;

  device = (struct sgio_device *) private_data;

  SGIODTRACE(sgio_pm_handler, enter, device->devno, mode, 0, 0, 0);

  switch(mode)
  {
    case PM_DEVICE_ENABLE:

      /* Re-open ttys for usage, if necessary */
      if (device->fp == (struct file *) -1)
      {
        if ((rc = sgio_tty_open(device)) != 0)
        {
          /* Log error */
          io_error("sgio_pm_handler", TRUE, PM_HANDLER_ERROR, 0, "");
          rc = PM_ERROR;
        }
        else
        {
          /* Update mode and activity flags to reflect current state */
          device->pm.mode = sgio_ptr->pm_mode = mode;
          device->pm.activity = 0;

          /* Notify sikproc I/O operations are reenabled */
          sgio_ptr->list_changed = TRUE;
          device->pm_status = PM_RESET;
          pidsig(sgio_ptr->sikproc_pid, SIGUSR1);
        }
      }
      else
      {
        /* Update mode and activity flags to reflect current state */
        device->pm.mode = sgio_ptr->pm_mode = mode;
        device->pm.activity = 0;
      }

      break;

    case PM_DEVICE_SUSPEND:
    case PM_DEVICE_HIBERNATION:

      /* Update mode flags to reflect current state */
      device->pm.mode = sgio_ptr->pm_mode = mode;

      /* Set device pm_status */
      device->pm_status = PM_LOW_POWER;

      /* Set list changed flag (sikproc to suspend polling on file pointer) */
      sgio_ptr->list_changed = TRUE;

      /* Notify sikproc I/O operations are suspended */
      tstop(sgio_ptr->pm_timer);
      sgio_sleep(sgio_ptr->pm_timer, 3, 0, sgio_ptr->sikproc_pid, 0);

      /* Close ttys */
      fp_close(device->fp);

      /* Set file pointer to negative value (fp_poll will ignore) */
      device->fp = (struct file *) -1;

      /* Update device activity flag to request "enable" on resume */
      device->pm.activity = -1;

      break;

    case PM_DEVICE_FULL_ON:
    case PM_DEVICE_IDLE:

      if (device->pm.mode != PM_DEVICE_ENABLE)
        rc = PM_ERROR;

      /* Update device mode flag to reflect current state; */
      /* sgio mode flag not set (sikproc does not respond to these modes) */
      device->pm.mode = mode;

      break;

    case PM_PAGE_FREEZE_NOTICE:
    case PM_PAGE_UNFREEZE_NOTICE:

      break;

    default:

      rc = PM_ERROR;

      break;
  }

  SGIODTRACE(sgio_pm_handler, exit, device->devno, rc, 0, 0, 0);

  return (rc);
}
