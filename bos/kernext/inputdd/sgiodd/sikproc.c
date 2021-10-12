static char sccsid[] = "@(#)62	1.5  src/bos/kernext/inputdd/sgiodd/sikproc.c, inputdd, bos41J, 9520A_all 5/15/95 16:42:15";
/*
 * COMPONENT_NAME: (INPUTDD) Serial Graphics I/O device driver
 *
 * FUNCTIONS: create_sikproc, term_sikproc, sikproc, purge_sigs,
 *            sikproc_query, sikproc_retry, poll_update, perform_io,
 *            dials_io, dials_read, lpfks_io, lpfks_read
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

#ifdef _KERNSYS
#include <sys/processor.h>
#else
#define _KERNSYS 1
#include <sys/processor.h>
#undef _KERNSYS
#endif

#include <sys/sgio.h>

extern sgio_t *sgio_ptr;
int sikproc_event = EVENT_NULL;
int sikproc_control_event = EVENT_NULL;


/*****************************************************************************/
/*                                                                           */
/* NAME:        create_sikproc                                               */
/*                                                                           */
/* FUNCTION:    This routine creates the serial I/O kernel process.          */
/*                                                                           */
/* INPUTS:      None                                                         */
/*                                                                           */
/* OUTPUTS:     0     = success                                              */
/*              EPERM = failure of either creatp or initp                    */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/
int create_sikproc()
{
  int rc = 0, sikproc();

  SGIODTRACE0(create_sikproc, enter);

  /* Create sikproc process */
  if ((sgio_ptr->sikproc_pid = creatp()) == -1)
  {
    /* Return error */
    return(EPERM);
  }

  /* Get thread id of sgio device driver */
  sgio_ptr->sgiodd_tid = thread_self();

  /* Set sikproc termination flag */
  sgio_ptr->sikproc_term = FALSE;

  /* Initialize sikproc */
  if (initp(sgio_ptr->sikproc_pid, sikproc, &sgio_ptr, sizeof(struct sgio *),
     "sikproc") != 0)
  {
    /* Return error */
    rc = EPERM;
  }
  else
  {
    /* Wait until sikproc is ready to process list entries */
    et_wait(SKPROC_WAIT_INIT, SKPROC_WAIT_INIT, 0);
  }

  SGIOTRACE(create_sikproc, exit, rc, 0, 0, 0, 0);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        term_sikproc                                                 */
/*                                                                           */
/* FUNCTION:    This routine terminates the serial I/O kernel process.       */
/*                                                                           */
/* INPUTS:      None                                                         */
/*                                                                           */
/* OUTPUTS:     0 = success                                                  */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       This process waits for the serial I/O kernel process to      */
/*              notify its successful termination to ensure proper cleanup.  */
/*                                                                           */
/*****************************************************************************/
int term_sikproc()
{
  int old_priority;
  struct trb *sikproc_control_timer = NULL;

  SGIODTRACE0(term_sikproc, enter);

  /* Allocate timer */
  sikproc_control_timer = (struct trb *) talloc();
  sikproc_control_timer->ipri = SGIO_PRIORITY;
  sikproc_control_timer->flags |= T_INCINTERVAL;
  sikproc_control_timer->func = (void (*) ()) sgio_watchdog;
  sikproc_control_timer->t_func_addr = (caddr_t) &sikproc_control_event;

  /* Set sikproc termination flag */
  sgio_ptr->sikproc_term = TRUE;

  /* Signal sikproc and sleep */
  sgio_sleep(sikproc_control_timer, 3, 0, sgio_ptr->sikproc_pid, 0);

  tfree(sikproc_control_timer);

  SGIOTRACE0(term_sikproc, exit);

  return(0);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sikproc                                                      */
/*                                                                           */
/* FUNCTION:    This routine is the "main" of the serial I/O kernel process. */
/*              It processes I/O for the serial GIO devices through the tty  */
/*              subsystem.                                                   */
/*                                                                           */
/* INPUTS:      flag      = ignored                                          */
/*              sgio_pptr = address of the sgio anchor structure pointer     */
/*              length    = ignored                                          */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       In addition to the above listed inputs and outputs, the      */
/*              serial I/O kernel process is responsible for handling the    */
/*              actual asynchronous I/O to the individual serial GIO devices.*/
/*                                                                           */
/*****************************************************************************/
sikproc(int flag, sgio_t **sgio_pptr, int length)
{
  int array_size = 0;
  int poll_count = 0;
  int pending = 0, sikproc_timer_running = FALSE;
  int fail_opens;
  struct sgio *sgio_ptr;
  struct pollfd *poll_array = NULL;
  struct trb *sikproc_timer = NULL, *timer = NULL;

  /* Bind kernel process to master processor */
  switch_cpu(MP_MASTER, SET_PROCESSOR_ID);

  SGIODTRACE0(sikproc, enter);

  sgio_ptr = *sgio_pptr;

  /* Isoliate sikproc from the current group's signals */
  setpgid(0,0);

  /* Make parent be the 'init' process */
  setpinit();

  /* Purge any signals received to this point */
  purge_sigs();

  /* Post initialization completion to create_sikproc */
  et_post(SKPROC_WAIT_INIT, sgio_ptr->sgiodd_tid);

  /* Allocate device query timer */
  timer = (struct trb *) talloc();
  timer->ipri = SGIO_PRIORITY;
  timer->flags |= T_INCINTERVAL;
  timer->func = (void (*) ()) sgio_watchdog;
  timer->t_func_addr = (caddr_t) &sikproc_event;

  /* Allocate device query timer */
  sikproc_timer = (struct trb *) talloc();
  sikproc_timer->ipri = SGIO_PRIORITY;
  sikproc_timer->flags |= T_INCINTERVAL;
  sikproc_timer->func = (void (*) ()) sikproc_query;
  sikproc_timer->t_func_addr = (caddr_t) sgio_ptr;

  /* Initialize device query flag */
  sgio_ptr->query_devices = FALSE;

  /* Infinite I/O processing loop */
  for( ; ; )
  {
    /* If no remaining open devices... */
    if (poll_count == 0)
    {
      /* If device query timer running... */
      if (sikproc_timer_running == TRUE)
      {
        /* Stop device query timer */
        tstop(sikproc_timer);
        sikproc_timer_running = FALSE;
      }

      e_sleep(&sikproc_event, EVENT_SIGRET);
    }
    else
    {
      /* If device query timer is not running... */
      if (sikproc_timer_running == FALSE)
      {
        /* then, should it be started... */
        if ((sgio_ptr->pm_mode == PM_NONE) || (sgio_ptr->pm_complete == FALSE))
        {
          /* Start device query timer */
          sikproc_timer->timeout.it_value.tv_sec = 10;
          sikproc_timer->timeout.it_value.tv_nsec = 0;
          tstart(sikproc_timer);
          sikproc_timer_running = TRUE;
        }
      }
      else 
      {
        /* else, should it be stopped... */
        if ((sgio_ptr->pm_mode != PM_NONE) && (sgio_ptr->pm_complete == TRUE))
        {
          /* Stop device query timer */
          tstop(sikproc_timer);
          sikproc_timer_running = FALSE;
        }
      }

      /* If all devices powered down... */
      if (((sgio_ptr->pm_mode == PM_DEVICE_SUSPEND) ||
         (sgio_ptr->pm_mode == PM_DEVICE_HIBERNATION)) &&
         (sgio_ptr->pm_complete == TRUE))
      {
        /* Suspend polling operations */
        e_sleep(&sikproc_event, EVENT_SIGRET);
      }
      else
      {
        /* Poll tty(s) for input */
        pending = fp_poll(poll_array, (0<<16) | (poll_count), -1, 0);
      }
    }

    /* Purge signals */
    purge_sigs();

    lockl(&sgio_ptr->sgio_lock, LOCK_SHORT);

    /* If sikproc termination requested... */
    if (sgio_ptr->sikproc_term == TRUE)
    {
      /* End processing loop */
      unlockl(&sgio_ptr->sgio_lock);
      break;
    }

    /* If device(s) opened/closed... */
    if (sgio_ptr->list_changed == TRUE)
    {
      /* Clear opening device(s) failure flag */
      fail_opens = FALSE;

      /* Update polling array */
      poll_update(sgio_ptr, &poll_array, &poll_count, &array_size, &fail_opens);
    }

    /* Perform device I/O */
    perform_io(sgio_ptr, timer, fail_opens);

    /* Reset device query timer, if necessary */
    if (sgio_ptr->query_devices == TRUE)
    {
      /* Reset timer and query status */
      sgio_ptr->query_devices = FALSE;
      tstop(sikproc_timer);
      sikproc_timer->timeout.it_value.tv_sec = 10;
      sikproc_timer->timeout.it_value.tv_nsec = 0;
      tstart(sikproc_timer);
    }

    unlockl(&sgio_ptr->sgio_lock);
  }

  /* Free poll array memory, if necessary */
  if (array_size > 0)
    xmfree(poll_array, pinned_heap);

  /* Stop and free device query timers */
  tstop(sikproc_timer);
  tfree(sikproc_timer);
  tstop(timer);
  tfree(timer);

/*
  SGIOTRACE0(sikproc, exit);
*/

  /* Post termination completion */
/*
  e_wakeup(&sikproc_control_event);
*/
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        purge_sigs                                                   */
/*                                                                           */
/* FUNCTION:    This routine clears all available signals delivered to the   */
/*              serial I/O kernel process.                                   */
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
int purge_sigs()
{
  int rc = 0;

  while (rc = sig_chk())
    ;
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sikproc_retry                                                */
/*                                                                           */
/* FUNCTION:    This routine services device output retry timer interrupts.  */
/*              In this case, servicing entails simply sending a signal to   */
/*              the serial I/O kernel process, which will cause it to        */
/*              process any device I/O requests.                             */
/*                                                                           */
/* INPUTS:      trb = timer request block pointer                            */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/
void sikproc_retry(struct trb *trb)
{
  sgio_t *sgio_ptr;

  SGIODTRACE0(sikproc_retry, enter);

  sgio_ptr = (sgio_t *) trb->t_func_addr;
  pidsig(sgio_ptr->sikproc_pid, SIGUSR1);

  SGIOTRACE0(sikproc_retry, exit);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sikproc_query                                                */
/*                                                                           */
/* FUNCTION:    This routine services device query timer interrupts. In this */
/*              case, servicing entails setting the query devices flag TRUE  */
/*              and signalling the serial I/O kernel process, which will     */
/*              cause it to check for the serial GIO devices presence and    */
/*              process any device I/O requests.                             */
/*                                                                           */
/* INPUTS:      trb = timer request block pointer                            */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/
void sikproc_query(struct trb *trb)
{
  sgio_t *sgio_ptr;

  SGIODTRACE0(sikproc_query, enter);

  sgio_ptr = (sgio_t *) trb->t_func_addr;
  sgio_ptr->query_devices = TRUE;
  pidsig(sgio_ptr->sikproc_pid, SIGUSR1);

  SGIOTRACE0(sikproc_query, exit);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        poll_update                                                  */
/*                                                                           */
/* FUNCTION:    This routine updates the tty file pointer polling array.     */
/*              Since the number of polled ttys is variable, the array is    */
/*              dynamically allocated and maintained. Only opened devices    */
/*              are added to the polling array.                              */
/*                                                                           */
/* INPUTS:      sgio_ptr   = sgio device driver anchor structure pointer     */
/*              poll_array = address of the polling array pointer            */
/*              poll_count = poll array device count pointer                 */
/*              array_size = poll array size pointer (>= poll_count)         */
/*              fail_opens = fail opening devices flag pointer               */
/*                                                                           */
/* OUTPUTS:     poll_array = polling array pointer (possible new array)      */
/*              poll_count = updated poll array device count                 */
/*              array_size = updated poll array size pointer (>= poll_count) */
/*              fail_opens = updated fail opening devices flag               */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       The fail_opens flag will remain false unless an array with   */
/*              sufficient space cannot be allocated from the pinned heap.   */
/*              The initialized device list must be locked prior to calling  */
/*              this routine since the list is traversed for device info.    */
/*                                                                           */
/*****************************************************************************/
int poll_update(struct sgio *sgio_ptr, struct pollfd **poll_array,
      int *poll_count, int *array_size, int *fail_opens)
{
  int index = 0, old_poll_count = 0;
  struct pollfd *old_poll_array = NULL;
  sgio_device_t *device;

  SGIODTRACE0(poll_update, enter);

  /* If current poll array is too small... */
  if ((sgio_ptr->open_count - sgio_ptr->close_count) > *array_size)
  {
    /* Save current poll array in case of error */
    old_poll_array = *poll_array;
    old_poll_count = *poll_count;

    /* Allocate memory for poll array */
    *poll_count = sgio_ptr->open_count - sgio_ptr->close_count;
    if ((*poll_array = (struct pollfd *)
       xmalloc(*poll_count * sizeof(struct pollfd), 3,
       pinned_heap)) == NULL)
    {
      /* Retrieve old poll array and update count */
      *poll_array = old_poll_array;
      *poll_count = old_poll_count - sgio_ptr->close_count;
      old_poll_array = NULL;
      old_poll_count = 0;

      /* Set failure flag for opening devices */
      *fail_opens = TRUE;
    }
    else
      *array_size = *poll_count;

    /* Free memory for current poll array, if necessary */
    if (old_poll_array != NULL)
    {
      xmfree(old_poll_array, pinned_heap);
      old_poll_array = NULL;
      old_poll_count = 0;
    }
  }
  else
    /* Array is already large enough, update count */
    *poll_count = sgio_ptr->open_count - sgio_ptr->close_count;

  /* Update poll array entries (tty file pointers) */
  device = sgio_ptr->device;
  while (device != NULL)
  {
    switch (device->command)
    {
      case SGIO_INIT_CMD:
      case SGIO_TERM_CMD:

        break;

      case SGIO_CLOSE_CMD:

        device->status = SUCCESS;
        e_wakeup(&device->sleep_event);

        break;

      case NONE:

        if (device->device_open == TRUE)
        {
          if ((device->pm_status == PM_LOW_POWER) ||
             (device->pm_status == PM_LOW_POWER_COMPLETE))
          {
            (*poll_array + index)->fd = -1;
          }
          else
          {
            (*poll_array + index)->fd = (long) device->fp;
          }
          (*poll_array + index)->events = POLLIN;
          index++;
        }

          break;

      case SGIO_OPEN_CMD:

        if (*fail_opens == TRUE)
        {
          device->status = FAIL;
          e_wakeup(&device->sleep_event);
        }
        else
        {
          if ((device->pm_status == PM_LOW_POWER) ||
             (device->pm_status == PM_LOW_POWER_COMPLETE))
          {
            (*poll_array + index)->fd = -1;
          }
          else
          {
            (*poll_array + index)->fd = (long) device->fp;
          }
          (*poll_array + index)->events = POLLIN;
          index++;
        }

        break;

      case SGIO_IOCTL_CMD:
      case SGIO_DIALS_CMD:
      case SGIO_LPFKS_CMD:

        if ((device->pm_status == PM_LOW_POWER) ||
           (device->pm_status == PM_LOW_POWER_COMPLETE))
        {
          (*poll_array + index)->fd = -1;
        }
        else
        {
          (*poll_array + index)->fd = (long) device->fp;
        }
        (*poll_array + index)->events = POLLIN;
        index++;
    }
    device = device->next;
  }

  /* Poll array update complete - reset change flag */
  sgio_ptr->list_changed = FALSE;

  SGIOTRACE(poll_update, exit, *fail_opens, 0, 0, 0, 0);

  return(0);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        perform_io                                                   */
/*                                                                           */
/* FUNCTION:    This routine traverses the initialized device list and       */
/*              performs generic I/O operations (i.e. send device            */
/*              configuration commands for configuration and hot-plugging).  */
/*                                                                           */
/* INPUTS:      sgio_ptr   = sgio device driver anchor structure pointer     */
/*              trb        = sleep timer request block pointer               */
/*              fail_opens = fail opening devices flag                       */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       The initialized device list must be locked prior to calling  */
/*              this routine since the list is traversed for device info.    */
/*                                                                           */
/*****************************************************************************/
int perform_io(struct sgio *sgio_ptr, struct trb *timer, int fail_opens)
{
  uchar io_buf;
  int nread, nwritten, i, retry_count;
  int io_done;
  struct pollfd poll_io;
  sgio_device_t *device;

  SGIODTRACE0(perform_io, enter);

  /* Initialize power management raise power completion flag */
  if (sgio_ptr->pm_mode != PM_NONE)
    sgio_ptr->pm_complete = TRUE;

  device = sgio_ptr->device;
  while (device != NULL)
  {
    /* If device is open or opening (closing not considered open)... */
    if ((((device->command == SGIO_OPEN_CMD) && (fail_opens == FALSE)) ||
       ((device->device_open == TRUE) && (device->command != SGIO_CLOSE_CMD))) &&
       (device->pm_status != PM_LOW_POWER_COMPLETE))
    {
      /* Power management operations not completed */
      if (sgio_ptr->pm_mode == PM_DEVICE_ENABLE)
      {
        if ((device->pm_status == PM_RESET) || (device->pm_status == PM_REINIT))
        {
          /* Power management operations not completed */
          sgio_ptr->pm_complete = FALSE;
        }
      }
      else if ((sgio_ptr->pm_mode == PM_DEVICE_SUSPEND) ||
              (sgio_ptr->pm_mode == PM_DEVICE_HIBERNATION))
      {
        if (device->pm_status != PM_LOW_POWER_COMPLETE)
        {
          /* Power management operations not completed */
          sgio_ptr->pm_complete = FALSE;
        }
      }

      if ((device->pm_status == PM_RESET) ||
         ((sgio_ptr->query_devices == TRUE) && (sgio_ptr->pm_mode == PM_NONE)))
      {
        /* If device has received input since last query... */
        if ((device->recent_io == TRUE) && (device->pm_status != PM_RESET))
          device->recent_io = FALSE;
        else
        {
          /* Send device read configuration command */
          io_buf = READ_CONF;
          fp_write(device->fp, &io_buf, 1, 0, SYS_ADSPACE, &nwritten);
          fp_ioctl(device->fp, TCSBRK, 1);
          sgio_sleep(timer, 0, SGIO_CMD_DELAY, 0, 0);

          /* Initialize configuration poll structure */
          poll_io.fd = (long) device->fp;
          poll_io.events = POLLIN;

          /* Allow 30ms for configuration response */
          fp_poll(&poll_io, (0<<16) | 1, 30, 0);
          switch (device->device_dds.device_class)
          {
            case S_DIALS:

              if (device->pm_status == PM_RESET)
                dials_read(device, PM_TRUE);
              else
                dials_read(device, TRUE);

              break;

            case S_LPFKS:

              if (device->pm_status == PM_RESET)
                lpfks_read(device, PM_TRUE);
              else
                lpfks_read(device, TRUE);

              break;
          }
        }
      }

      switch (device->device_dds.device_class)
      {
        case S_DIALS:

          dials_io(sgio_ptr, device, timer);

          break;

        case S_LPFKS:

          lpfks_io(sgio_ptr, device, timer);

          break;
      }
    }
    else if (device->command == SGIO_INIT_CMD)
    {
      /* Send device reset command. Drain write queue prior to delay
         to ensure device gets the 500ms required for reset. */
      io_buf = RESET;
      fp_write(device->fp, &io_buf, 1, 0, SYS_ADSPACE, &nwritten);
      fp_ioctl(device->fp, TCSBRK, 1);
      sgio_sleep(timer, 0, SGIO_RESET_DELAY, 0, 0);

      /* Initialize configuration poll structure */
      poll_io.fd = (long) device->fp;
      poll_io.events = POLLIN;

      /* Send read configuration command */
      retry_count = 5;
      do
      {
        io_buf = READ_CONF;
        fp_write(device->fp, &io_buf, 1, 0, SYS_ADSPACE, &nwritten);
        fp_poll(&poll_io, (0<<16) | 1, 30, 0);
      }
      while ((poll_io.revents == 0) && (--retry_count > 0));

      /* Read device response if available */
      io_buf = 0x0;
      if (poll_io.revents != 0)
        fp_read(device->fp, &io_buf, 1, 0, SYS_ADSPACE, &nread);

      /* If proper response received... */
      if (((io_buf == LPFK_RESP) && (device->device_dds.device_class == S_LPFKS)) ||
         ((io_buf == DIAL_RESP) && (device->device_dds.device_class == S_DIALS)))
        device->status = SUCCESS;
      else
        device->status = FAIL;

      e_wakeup(&device->sleep_event);
    }

    /* Notif power management of completion, if necessary */
    if (device->pm_status == PM_LOW_POWER)
    {
      /* Request complete (don't send again later with closed file pointer) */
      device->pm_status = PM_LOW_POWER_COMPLETE;
      e_wakeup(&sikproc_control_event);
    }

    device = device->next;
  }

  SGIOTRACE0(perform_io, exit);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        dials_io                                                     */
/*                                                                           */
/* FUNCTION:    This routine performs I/O operations for the specified       */
/*              serial dials device.                                         */
/*                                                                           */
/* INPUTS:      sgio_ptr = sgio device driver anchor structure pointer       */
/*              device   = dials device structure pointer                    */
/*              trb      = sleep timer request block pointer                 */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       The initialized device list must be locked prior to calling  */
/*              this routine since the list is traversed for device info.    */
/*                                                                           */
/*****************************************************************************/
int dials_io(struct sgio *sgio_ptr, struct sgio_device *device,
             struct trb *timer)
{
  int nwritten, i;
  uchar io_buf[2];

  SGIODTRACE0(dials_io, enter);

  /* If device has a pending open or set granularity command... */
  if (device->pm_status == PM_LOW_POWER)
    ;
  else if ((device->status == PENDING) || (device->pm_status == REINIT) ||
     (device->pm_status == PM_REINIT))
  {
    /* If pending command is open... */
    if ((device->command == SGIO_OPEN_CMD) || (device->pm_status == REINIT))
    {
      /* Send dials enable command */
      io_buf[0] = ENABLE;
      fp_write(device->fp, &io_buf, 1, 0, SYS_ADSPACE, &nwritten);
      fp_ioctl(device->fp, TCSBRK, 1);
      sgio_sleep(timer, 0, SGIO_CMD_DELAY, 0, 0);
    }

    /* Send dials granularity command */
    for (i = 0; i < 8; i++)
      if (((unsigned char) (device->device_data.dials.select) >> i & 0x01) ||
         (device->pm_status == REINIT))
      {
        io_buf[0] = DIALS_SET | device->device_data.dials.granularity[i];
        io_buf[1] = 1 << i;
        fp_write(device->fp, &io_buf, 2, 0, SYS_ADSPACE, &nwritten);
        fp_ioctl(device->fp, TCSBRK, 1);
        sgio_sleep(timer, 0, SGIO_CMD_DELAY, 0, 0);
        device->device_data.dials.position[i] = SGIO_DIALS_SET1;
      }

    if ((device->pm_status == REINIT) || (device->pm_status == PM_REINIT))
    {
      device->pm_status = PM_NONE;
    }
    else if (device->status == PENDING)
    {
      device->status = SUCCESS;
      e_wakeup(&device->sleep_event);
    }
  }

  /* Read any available input */
  dials_read(device, FALSE);

  SGIOTRACE0(dials_io, exit);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        dials_read                                                   */
/*                                                                           */
/* FUNCTION:    This routine performs input operations for the specified     */
/*              serial dials device.                                         */
/*                                                                           */
/* INPUTS:      device         = dials device structure pointer              */
/*              query_response = read configuration query response flag      */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       The initialized device list must be locked prior to calling  */
/*              this routine since the list is traversed for device info.    */
/*                                                                           */
/*****************************************************************************/
int dials_read(struct sgio_device *device, int query_response)
{
  int nread, query_done = TRUE;
  uchar io_buf;
  char temp, dial_num;
  short new, delta;

  SGIODTRACE(dials_read, enter, device->status, query_response, 0, 0, 0);

  if ((query_response == TRUE) || (query_response == PM_TRUE))
    query_done = FALSE;

  do
  {
    fp_read(device->fp, &io_buf, 1, 0, SYS_ADSPACE, &nread);

    /* If any input read... */
    if (nread == 1)
    {
      device->recent_io = TRUE;
      device->pm.activity = 1;

      if ((io_buf == DIAL_RESP) && (query_done == FALSE))
      {
        query_done = TRUE;
      }
      else if (device->rcb.ring_ptr != NULL)
      {
        struct ir_dials event;

        /* If first byte of valid command... */
        if (!(io_buf & 0x80))
        {
          device->device_data.dials.last = io_buf;
          continue;
        }

        temp = device->device_data.dials.last;
        dial_num = (temp >> 3) & 0x07;

        /* If first complete command after setting granularity... */
        if (device->device_data.dials.position[dial_num] == SGIO_DIALS_SET1)
        {
          device->device_data.dials.position[dial_num] = SGIO_DIALS_SET2;
          continue;
        }

        new = delta = ((temp & 1) << 7) | (io_buf & 0x7F);

        /* If second complete command after setting granularity... */
        if (device->device_data.dials.position[dial_num] == SGIO_DIALS_SET2)
        {
          device->device_data.dials.position[dial_num] = new;
          continue;
        }

        delta -= device->device_data.dials.position[dial_num];
        device->device_data.dials.position[dial_num] = new;

        /* If CCW... */
        if (temp & 0x04)
        {
          if (delta > 0)
            delta -= 256;
        }
        else
        {
          if (delta < 0)
            delta += 256;
        }
  
        /* Add to input ring */
        event.dials_header.report_size = sizeof(struct ir_dials);
        event.dials_number = dial_num;
        event.dials_value = delta;
        sgio_putring(&device->rcb, (struct ir_report *) &event, NULL);
      }
    }
  }
  while (nread == 1);

  if (query_response == TRUE)
  {
    if (query_done == FALSE)
    {
      device->device_present = FALSE;
    }
    else if (device->device_present == FALSE)
    {
      device->device_present = TRUE;
      device->status = REINIT;
    }
  }
  else if (query_response == PM_TRUE)
  {
    if (query_done == TRUE)
      device->pm_status = PM_REINIT;
  }

  SGIOTRACE(dials_read, exit, device->status, device->device_present, 0, 0, 0);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        lpfks_io                                                     */
/*                                                                           */
/* FUNCTION:    This routine performs I/O operations for the specified       */
/*              serial lpfks device.                                         */
/*                                                                           */
/* INPUTS:      sgio_ptr = sgio device driver anchor structure pointer       */
/*              device   = lpfks device structure pointer                    */
/*              trb      = sleep timer request block pointer                 */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       The initialized device list must be locked prior to calling  */
/*              this routine since the list is traversed for device info.    */
/*                                                                           */
/*****************************************************************************/
int lpfks_io(struct sgio *sgio_ptr, struct sgio_device *device,
             struct trb *timer)
{
  uchar io_buf[5];
  int i, nread, nwritten;
  int retry_count = 1, io_done = FALSE, query_response;
  struct pollfd poll_io;

  SGIODTRACE(lpfks_io, enter, device->retry_count, 0, 0, 0, 0);

  /* Handle device's pending lpfks requests... */
  if (device->pm_status == PM_LOW_POWER)
  {
    /* Set lpfks set leds command */
    io_buf[0] = LPFK_SET;
    for (i = 1; i <= 4; i++)
      io_buf[i] = 0x00;

    /* Send lpfks set leds command */
    fp_write(device->fp, &io_buf, 5, 0, SYS_ADSPACE, &nwritten);

    /* Initialize I/O poll structure */
    poll_io.fd = (long) device->fp;
    poll_io.events = POLLIN;

    /* Give lpfks up to 30ms to respond */
    fp_poll(&poll_io, (0<<16) | 1, 30, 0);

    /* Any set led requests must now wait until PM power up */
    device->retry_count = 0;
  }
  else if (((device->status == PENDING) || (device->pm_status == REINIT) ||
          (device->pm_status == PM_REINIT)) && (device->retry_count > 0))
  {
    /* Enable device upon pending open command or reinitialization request */
    if ((device->command == SGIO_OPEN_CMD) || (device->pm_status == REINIT) ||
       (device->pm_status == PM_REINIT))
    {
        /* Send lpfks enable command */
        io_buf[0] = ENABLE;
        fp_write(device->fp, &io_buf, 1, 0, SYS_ADSPACE, &nwritten);
    }

    /* Set lpfks set leds command */
    io_buf[0] = LPFK_SET;
    for (i = 0; i < 4; i++)
      io_buf[i+1] = device->device_data.lpfks[i];

    /* Send lpfks set leds command */
    fp_write(device->fp, &io_buf, 5, 0, SYS_ADSPACE, &nwritten);

    /* Initialize I/O poll structure */
    poll_io.fd = (long) device->fp;
    poll_io.events = POLLIN;

    /* Give lpfks up to 30ms to respond */
    fp_poll(&poll_io, (0<<16) | 1, 30, 0);
  }

  /* Read any available input */
  lpfks_read(device, FALSE);

  /* Determine status of the current request */
  if ((device->pm_status == REINIT) && (device->retry_count == 1))
  {
    /* Hot plugged device not accepting set leds command. */
    device->pm_status = PM_NONE;

    /* Log error */
    io_error("lpfks_io", FALSE, SIKPROC_ERROR, 0, NULL);
  }
  else if ((device->pm_status == PM_REINIT) && (device->retry_count == 1))
  {
    /* Device not accepting set leds command. Request PM reset again. */
    device->pm_status = PM_RESET;
  }
  else if ((device->status == PENDING) && (device->retry_count == 1))
  {
    /* No ACK received for set lpfks command... */
    device->status = FAIL;
    e_wakeup(&device->sleep_event);
  }
  else if (device->retry_count > 1)
  {
    device->retry_count--;

    /* Set device I/O retry timer */
    tstop(device->retry_timer);
    device->retry_timer->timeout.it_value.tv_sec = 1;
    device->retry_timer->timeout.it_value.tv_nsec = 0;
    device->retry_timer->func = (void (*) ()) sikproc_retry;
    device->retry_timer->t_func_addr = (caddr_t) sgio_ptr;
    tstart(device->retry_timer);
  }

  SGIOTRACE0(lpfks_io, exit);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        lpfks_read                                                   */
/*                                                                           */
/* FUNCTION:    This routine performs input operations for the specified     */
/*              serial dials device.                                         */
/*                                                                           */
/* INPUTS:      device         = lpfks device structure pointer              */
/*              query_response = read configuration query response flag      */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       The initialized device list must be locked prior to calling  */
/*              this routine since the list is traversed for device info.    */
/*                                                                           */
/*****************************************************************************/
int lpfks_read(struct sgio_device *device, int query_response)
{
  int io_done = FALSE, query_done = TRUE, nread;
  uchar io_buf;

  SGIODTRACE(lpfks_read, enter, device->status, query_response, 0, 0, 0);

  if ((query_response == TRUE) || (query_response == PM_TRUE))
    query_done = FALSE;

  do
  {
    /* Read input */
    fp_read(device->fp, &io_buf, 1, 0, SYS_ADSPACE, &nread);

    /* If any input read... */
    if (nread == 1)
    {
      device->recent_io = TRUE;
      device->pm.activity = 1;

      /* Handle input accordingly */
      if (io_buf == LPFK_ACK)
      {
        /* Mark set leds output complete */
        io_done = TRUE;
      }
      else if ((io_buf == LPFK_RESP) && (query_done == FALSE))
      {
        query_done = TRUE;
      }
      else if ((io_buf != LPFK_NAK) && (device->rcb.ring_ptr != NULL))
      {
        struct ir_lpfk event;

        event.lpfk_header.report_size = sizeof(struct ir_lpfk);
        event.lpfk_number = io_buf;

        /* Add to input ring */
        sgio_putring(&device->rcb, (struct ir_report *) &event, NULL);
      }
    }
  }
  while (nread == 1);

  if ((io_done == TRUE) &&
          ((device->pm_status == REINIT) || (device->pm_status == PM_REINIT)))
  {
    device->pm_status = PM_NONE;
  }
  else if ((io_done == TRUE) && (device->status == PENDING))
  {
    device->status = SUCCESS;
    e_wakeup(&device->sleep_event);
  }

  if (query_response == TRUE)
  {
    if (query_done == FALSE)
    {
      device->device_present = FALSE;
    }
    else if (device->device_present == FALSE)
    {
      device->device_present = TRUE;
      device->retry_count = 3;
      device->pm_status = REINIT;
    }
  }
  else if (query_response == PM_TRUE)
  {
    if (query_done == TRUE)
    {
      device->retry_count = 3;
      device->pm_status = PM_REINIT;
    }
  }

  SGIOTRACE(lpfks_read, exit, device->status, device->pm_status,
            device->device_present, 0, 0);
}
