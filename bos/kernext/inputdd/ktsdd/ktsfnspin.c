static char sccsid[] = "@(#)80	1.5  src/bos/kernext/inputdd/ktsdd/ktsfnspin.c, inputdd, bos41J, 9509A_all 2/14/95 13:20:59";
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard/Table/Sound DD - ktsfnspin.c
 *
 * FUNCTIONS: send_q_frame, watch_dog, read_port, write_port, write_sport,
 *            io_error, reg_intr, ureg_intr, sv_rflush, sv_sak, sv_alarm,
 *            put_sq, next_sound,  stop_sound, chg_clicker, mem_dump
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

#include "kts.h"

struct  local *local=NULL;             /* local data anchor                 */

/*****************************************************************************/
/*                                                                           */
/* NAME:        send_q_frame()                                               */
/*                                                                           */
/* FUNCTION:    Send next frame on output queue to device                    */
/*                                                                           */
/* INPUTS:      com = pointer to common data area                            */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/


void send_q_frame(struct common *com)
{

   struct kts_port *port;

   KTSMDTRACE1(k_send_q_frame, enter, com->outq.out_cnt);



   if (com->outq.out_cnt == 0) {       /* if entire transmission group sent  */
                                       /*  point head to start of nxt group  */
     com->outq.head = com->outq.dq_ptr;
                                       /*  if nothing on queue then          */
     if (com->outq.head == com->outq.tail) {
       com->in_progress=NO_OP;         /*    indicate nothing going on       */
       e_wakeup(&com->asleep);         /*    wake up any waiting process     */
       return;                         /*    exit                            */
     }
     else {                            /*  if queue is not empty then        */
                                       /*    get frame cnt for next group    */
       com->outq.out_cnt = get_oq(com);
       local->tab_dframe=FALSE;        /*    next tablet frame must be cmd   */
       com->retry_cnt = 0;             /*    clear retry counter             */
     }
   }

   com->cur_frame = get_oq(com);       /* get frame from queue               */
                                       /* get access to I/O bus              */
   port = (struct kts_port *) BUSIO_ATT(com->bus_id,com->bus_addr);

                                       /* write frame to adapter data port   */
   if (write_sport(&port->w_port_a, (short) com->cur_frame)) {
                                       /* if ok                              */
     com->in_progress = ADPT_ACK_RQD;  /*   indicate I/O in progress         */
                                       /*   start watch dog timer            */
     startwd(com, (void *) watch_dog, WDOGTIME);
   }

   BUSIO_DET((caddr_t) port);          /* detach from I/O bus                */

   KTSMTRACE(k_send_q_frame, exit, com->cur_frame,
          com->in_progress, com->outq.out_cnt, 0, 0);

}


/*****************************************************************************/
/*                                                                           */
/* NAME:        watch_dog                                                    */
/*                                                                           */
/* FUNCTION:    Service watch dog timer interrupt                            */
/*                                                                           */
/* INPUTS:      tb = address of watchdog trb                                 */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:       This is the I/O watch dog timer service routine. It gets     */
/*              control when the watch dog timer pops because the device     */
/*              did not acknowledge a transmission within the time limit.    */
/*                                                                           */
/*              This routine also gets control after the delay period        */
/*              following a tablet transmission. This is not an error        */
/*              condition. The tablet does not respond with a ack following  */
/*              a transmission, so the watchdog timer is used to delay       */
/*              the transmission of the next frame in order that the         */
/*              tablet has time to process the previous frame.               */
/*                                                                           */
/*****************************************************************************/


void watch_dog(struct trb *tb)
{
  struct common *com;

  com = &local->com;                   /* pointer to common area             */

  KTSMTRACE(k_watch_dog, enter, com->cur_frame, com->in_progress,
       com->retry_cnt, 0, 0);
                                       /* if waiting for ACK then ...        */
  if (com->in_progress & ACK_RQD) {
                                       /* if not tablet process delay then   */
    if(com->in_progress != TAB_IN_PROG) {
                                       /* try again if last op failed and    */
                                       /* retries are not exhausted          */
      if ((com->in_progress & FAILED_OP) && (com->retry_cnt < K_RETRY)) {
        com->retry_cnt++;
        com->outq.dq_ptr = com->outq.head;
        com->outq.out_cnt = get_oq(com);
      }
      else {                           /* log error if no retry              */
#ifndef GS_DEBUG_TRACE
        if ((com->in_progress & KBD_ACK_RQD) &&
            (com->cur_frame != KBD_RESET_CMD)) 
#endif
           io_error("watch_dog", FALSE, ADPT_ERROR, 0, "%04x %04x",
             com->cur_frame, com->in_progress);
        while(com->outq.out_cnt) {     /* flush transmission group           */
          get_oq(com);
        }
        com->outq.error = TRUE;        /* indicate that there was a problem  */
      }
    }
    send_q_frame(com);                 /* process next frame                 */
  }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        read_port                                                    */
/*                                                                           */
/* FUNCTION:    Read adapter port with PIO error recovery                    */
/*                                                                           */
/* INPUTS:      addr = adapter port address                                  */
/*              data = address of location to put data read from adapter     */
/*                                                                           */
/* OUTPUTS:     TRUE if successfull                                          */
/*              FALSE if unrecoverable PIO error occurred                    */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:       Must be attached to I/O bus before calling this routine      */
/*                                                                           */
/*****************************************************************************/


int read_port(char *addr, char *data)
{
   int retry;

   for (retry = 0; retry < PIO_RETRY_COUNT; retry++)  {
       if (!BUS_GETCX(addr, data)) return(TRUE);
   }
                                       /* unrecoverable PIO error            */
   io_error("read_port", TRUE, PIO_ERROR, 0, "%x", addr);
   return(FALSE);
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        write_port                                                   */
/*                                                                           */
/* FUNCTION:    Write char data to adapter port with PIO error recovery      */
/*                                                                           */
/* INPUTS:      addr = adapter port address                                  */
/*              data = data to be written to adapter port                    */
/*                                                                           */
/* OUTPUTS:     TRUE if successfull                                          */
/*              FALSE if unrecoverable PIO error occurred                    */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:       Must be attached to I/O bus before calling this routine      */
/*                                                                           */
/*****************************************************************************/


int write_port(char *addr, char data)
{
   int retry;

   for (retry = 0; retry < PIO_RETRY_COUNT; retry++)  {
       if (!BUS_PUTCX(addr, data)) return(TRUE);
   }
   return(FALSE);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        write_sport                                                  */
/*                                                                           */
/* FUNCTION:    Write short data to adapter port with PIO error recovery     */
/*                                                                           */
/* INPUTS:      addr = adapter port address                                  */
/*              data = data to be written to adapter port                    */
/*                                                                           */
/* OUTPUTS:     TRUE if successfull                                          */
/*              FALSE if unrecoverable PIO error occurred                    */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:       Must be attached to I/O bus before calling this routine      */
/*                                                                           */
/*****************************************************************************/

int write_sport(short *addr, short data)
{
   int retry;


   for (retry = 0; retry < PIO_RETRY_COUNT; retry++)  {
       if (!BUS_PUTSX( addr, data)) return(TRUE);
   }
                                       /* unrecoverable PIO error            */
   io_error("write_sport", TRUE, PIO_ERROR, 0, "%x %04x", addr, data);
   return(FALSE);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        io_error                                                     */
/*                                                                           */
/* FUNCTION:    Log I/O error                                                */
/*                                                                           */
/* INPUTS:      rout = name of routine detecting error                       */
/*              perm = TRUE if hard error                                    */
/*              err_code = error code to be logged                           */
/*              loc = location code to be logged                             */
/*              fmt = format string for detailed information (or NULL)       */
/*                    (this is a limited printf like fmt string)             */
/*              ... = detailed information to be formated                    */
/*                                                                           */
/*              Warning: formated string limited to 30 characters and        */
/*                       5 variables                                         */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process or Interrupt                                         */
/*                                                                           */
/* NOTES:                                                                    */
/*     When this routine is called with perm = TRUE, a major hardware error  */
/*     has occurred. What we would like to do is to assert adapter reset     */
/*     (so that the adapter can not cause an interrupt), log an error, and   */
/*     bail out.  The adapter can be reset via the POS regs which            */
/*     means that we don't have to worry about bus exceptions.               */
/*     HOWEVER, when the 8051 is held in reset, the interrupt                */
/*     request line to the 8255 floats high on some machines which causes    */
/*     an interrupt. If the 8255 is also held in reset, the interrupt        */
/*     line to the main processor floats high on some machines causing       */
/*     the workstation to hang.                                              */
/*                                                                           */
/*     So given the above, all that is done is interrupts are masked using   */
/*     control provided by the 8255 and an error is logged. Masking the      */
/*     interrutps could result in a bus exception but nothing can be done    */
/*     about that. Also, if the 8051 is really in bad shape, the alarm could */
/*     be sounding. Once again, there is not much we can do about that at    */
/*     this point.                                                           */
/*                                                                           */
/*****************************************************************************/

void io_error(char *rout, int perm, uint err_code, uint loc, char *fmt, ...)
{
   va_list arg;
   char ss[30];
   struct kts_port *port;
   struct common *com;

   va_start(arg,fmt);                  /* log error                          */
   _vsprintf(ss, fmt, arg);
   ktsm_log(RES_NAME_KTS, rout, err_code, loc, ss);

   com = &local->com;                  /* pointer to common area             */

   if (perm) {                         /* if hard error                      */
                                       /* get access to I/O bus              */
     port = (struct kts_port *) BUSIO_ATT(com->bus_id,com->bus_addr);
                                       /* disable interrupts                 */
     write_port(&port->w_port_ced, (char) DISABLE_INT);
     BUSIO_DET(port);                  /* detatch from I/O bus               */
     flush_q(com);                     /* rst I/O queues to wake up top half */
     com->perr = TRUE;                 /* remember we're hosed               */
   }
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        reg_intr                                                     */
/*                                                                           */
/* FUNCTION:    Register interrupt handler                                   */
/*                                                                           */
/* INPUTS:      adpt_rst:                                                    */
/*                TRUE if adapter is to be reset                             */
/*                FALSE if no adapter reset desired                          */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*              EPERM = failed to register interrupt handler                 */
/*              ENOMEM = unable to pin driver, out of memory                 */
/*              EIO    = i/o error occurred                                  */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int reg_intr(int adpt_rst)
{
   uchar *posreg_ptr;
   uchar posreg_val;
   int old_int;
   int rc;
   struct kts_port *port;
   struct common *com;


/* pin code and register interrupt handler                                   */
   com = &local->com;                  /* point to common extension          */

   if (com->perr) return(EIO);         /* exit if permanent error posted     */

                                       /* proceed only if in diagnostic      */
                                       /* mode or both keyboard and tablet   */
                                       /* devices are closed                 */
   if ((!local->key.diag) &&
      ((local->key.act_ch != NO_ACT_CH) ||
      (local->tab.oflag))) return(0);
                                       /* pin bottom half of driver          */
   if (rc = pincode((int (*) ())ktsintr))
       return(rc);                     /* exit if error                      */

   flush_q(com);                       /* initialize I/O queues              */
   com->outq.error = FALSE;            /* no errors                          */
   local->volume = NOVOLUME;           /* don't know what volume is set to   */

                                       /* tell kernel about slih             */
   com->intr_data.next     = 0;
   com->intr_data.handler  = (int (*) ())ktsintr;
   com->intr_data.bus_type = BUS_MICRO_CHANNEL;
   com->intr_data.flags    = 0;
   com->intr_data.level    = local->com.intr_level;
   com->intr_data.priority = local->com.intr_priority;
   com->intr_data.bid      = local->com.bus_id;

   if (i_init(&com->intr_data) != INTR_SUCC) {

      unpincode((int (*) ())ktsintr);  /* if error, unpin bottom half and    */
      return(EPERM);                   /*   return                           */
   }
   local->ih_inst = TRUE;              /* interrupt handler defined          */
   dmp_add(mem_dump);                  /* register system crash local memory */
                                       /* dump routine                       */

/* reset and initialize adapter  (8051 micro-processor)                      */
                                       /* get access to I/O bus              */
   port = (struct kts_port *) BUSIO_ATT(com->bus_id,com->bus_addr);

   if (adpt_rst) {                     /* if adapter to be reset then ...    */
                                       /* get access to iocc                 */
      posreg_ptr = (uchar *) IOCC_ATT((com->bus_id | 0x80),
          com->slot_addr + 2);

      old_int = i_disable(INTMAX);     /* start critical section             */
                                       /* get contents of pos reg            */
      posreg_val = BUSIO_GETC(posreg_ptr);
                                       /* assert 8051 and 8255 reset         */
      BUSIO_PUTC(posreg_ptr,(posreg_val | RESET_KEYBOARD_8051) &
           ~NOT_RESET_8255);
                                       /* clear 8255 reset                   */
      BUSIO_PUTC(posreg_ptr,posreg_val | RESET_KEYBOARD_8051 |
            NOT_RESET_8255);
                                       /* configure the 8255                 */
      write_port(&port->w_port_ced, (char) CNFGDATA);
                                       /* clear 8051 reset                   */
      BUSIO_PUTC(posreg_ptr,(posreg_val & ~RESET_KEYBOARD_8051) |
             NOT_RESET_8255);
      i_enable(old_int);               /* end critcal section                */

      com->in_progress=ADPT_BAT;       /* waiting for adapter BAT            */

                                       /* enable interrupts                  */
      write_port(&port->w_port_ced, (char) ENABLE_INT);

      IOCC_DET(posreg_ptr);            /* detach from iocc                   */
      BUSIO_DET(port);                 /* detatch from I/O bus               */

                                       /* wait for 8051 completion code      */
      ktsm_sleep(com, ADPT_RST_WAIT);
                                       /* if adapter reset failed            */
      if (com->in_progress != NO_OP)  {
                                       /*   log error                        */
         io_error("reg_intr", FALSE,  ADPT_ERROR, 0, "%02x %04x",
            local->data, com->in_progress );
         rc = EIO;
      }
      else {
         put_oq(com, init_8051);       /* send initialization cmds to adapter*/
         rc = wait_oq(com);
      }
   }

/* enable interrupts from the adapter                                        */

   else {                              /* not doing adapter reset so         */
                                       /* just enable interrupts             */
      if (!write_port(&port->w_port_ced, (char) ENABLE_INT)) {
                                       /* log error                          */
         io_error("reg_intr", FALSE,  PIO_ERROR, 0, "%x %02x",
              &port->w_port_ced, ENABLE_INT);
         com->perr = TRUE;             /* remember we're hosed               */
         rc = EIO;
      }
      BUSIO_DET(port);                 /* detatch from I/O bus               */
   }
   if (rc) ureg_intr();                /* undefine IH if error               */

   return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        ureg_intr                                                    */
/*                                                                           */
/* FUNCTION:    Un-register interrupt handler                                */
/*                                                                           */
/* INPUTS:      TRUE if adapter to be held in reset                          */
/*              FALSE if just disable adapter interrupts                     */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process or interrupt                                         */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void ureg_intr(void)
{
  struct kts_port *port;
  struct common *com;

                                       /* proceed only if in diagnostic      */
                                       /* mode or both keyboard and tablet   */
                                       /* devices are closed                 */
  if ((local->key.diag) ||
     ((local->key.act_ch == NO_ACT_CH) &&
     (!local->tab.oflag))) {

    com = &local->com;                 /* point to common extension          */
    stop_sound();                      /* turn off alarm                     */
    wait_oq(com);                      /* wait for all I/O to complete       */
                                       /* get access to I/O bus              */
    port = (struct kts_port *) BUSIO_ATT(com->bus_id,com->bus_addr);
                                       /* disable interrupts                 */
    write_port(&port->w_port_ced, (char) DISABLE_INT);
    BUSIO_DET(port);                   /* detatch from I/O bus               */

    if (local->ih_inst) {              /* only do if IH installed ...        */
       i_clear(&com->intr_data);       /* undefine interrupt handler         */
       dmp_del(mem_dump);              /* unregister local memory dump rout. */
       unpincode((int (*) ())ktsintr); /* unpin bottom half                  */
       local->ih_inst = FALSE;         /* IH is no longer installed          */
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sv_rflush                                                    */
/*                                                                           */
/* FUNCTION:    This module flushes the input ring. It is called via         */
/*              the service vector                                           */
/*                                                                           */
/* INPUTS:      devno = device number (ignored)                              */
/*              arg   = pointer to NULL (ignored)                            */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*              -1 = kernel has not opened channel                           */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int sv_rflush(dev_t devno, caddr_t arg)
{
  int rc = -1;
  if (local) {
    rc = sv_proc(&local->com, &local->key, KSVRFLUSH, arg);
  }
  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sv_sak                                                       */
/*                                                                           */
/* FUNCTION:    Enable/disable SAK. This routine is called via               */
/*              the service vector                                           */
/*                                                                           */
/* INPUTS:      devno = device number (ignored)                              */
/*              arg = pointer to int containing:                             */
/*                    KSSAKDISABLE = disable SAK                             */
/*                    KSSAKENABLE  = enable SAK                              */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*              EINVAL = invalid parameter                                   */
/*              -1 = kernel has not opened channel                           */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:       No attempt is made to recover key events if in SAK and       */
/*              disable is received                                          */
/*                                                                           */
/*****************************************************************************/

int sv_sak(dev_t devno, caddr_t arg)
{
  int rc = -1;
  if (local) {
    rc = sv_proc(&local->com, &local->key, KSVSAK, arg);
  }
  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sv_alarm                                                     */
/*                                                                           */
/* FUNCTION:    Sound alarm. This routine is called via the service          */
/*              vector                                                       */
/*                                                                           */
/* INPUTS:      devno = device number (ignored)                              */
/*              arg = pointer to ksalarm structure                           */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*              EINVAL = invalid parameter                                   */
/*              EBUSY = queue is full, request ignored                       */
/*              EIO = I/O error                                              */
/*              -1 = kernel has not opened channel                           */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int sv_alarm(dev_t devno, caddr_t arg)
{
  int rc = -1;
  if (local) {
    rc = sv_proc(&local->com, &local->key, KSVALARM, arg);
  }
  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        put_sq                                                       */
/*                                                                           */
/* FUNCTION:    Put request on sound queue                                   */
/*                                                                           */
/* INPUTS:      arg   = pointer to ksalarm structure                         */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*              EINVAL = invalid parameter                                   */
/*              EBUSY = queue is full                                        */
/*              EIO = I/O error                                              */
/*              EPERM = could not allocate timer                             */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int put_sq(struct ksalarm *arg)
{

  int rc;
  int tmp;
  int freq;
  struct sound_q *sqe;


  KTSMDTRACE(put_sq, enter,  arg->duration, arg->frequency, 0,0,0);

                                       /* make sure parms are valid          */
  if ((arg->duration > MAX_ALARM_DURATION) ||
      (arg->frequency > MAX_ALARM_FREQ) ||
      (arg->frequency < MIN_ALARM_FREQ)) {
    rc = EINVAL;
  }
  else {                               /* sound alarm only if some volume    */
                                       /* and some time                      */
    if ((local->key.volume == SPEAKER_OFF) ||
        (arg->duration == 0)) {
      rc = 0;
    }
    else {                             /* return busy if queue is full       */
      if ((local->sq_tail + 1 ) == local->sq_head) {
        rc = EBUSY;
      }
      else {                           /* put request on queue               */
        sqe = &local->sound_q[local->sq_tail];

        sqe->duration =  arg->duration;

        if (arg->frequency <= 45)
          sqe->freq = 16384 + ((6000/arg->frequency) - 9);
        else
        if ((arg->frequency >= 46) && (arg->frequency <= 90))
          sqe->freq = 8192 + ((12000/arg->frequency) - 9);
        else
        if ((arg->frequency >= 91) && (arg->frequency <= 181))
          sqe->freq = 4096 + ((24000/arg->frequency) - 9);
        else
        if ((arg->frequency >= 182) && (arg->frequency <= 362))
          sqe->freq = 2048 + ((48000/arg->frequency) - 9);
        else
        if ((arg->frequency >= 363) && (arg->frequency <= 724))
          sqe->freq = 1024 + ((96000/arg->frequency) - 10);
        else
        if ((arg->frequency >= 725) && (arg->frequency <= 1432))
          sqe->freq = 512 + ((192000/arg->frequency) - 11);
        else
          sqe->freq = 256 + ((384000/arg->frequency) - 12);

        sqe->volume = local->key.volume;

        KTSMDTRACE(put_sq, qe, sqe->volume, sqe->freq,
                 sqe->duration, local->sq_head, local->sq_tail);

                                       /* update sound queue tail index      */
        local->sq_tail = (local->sq_tail < (SOUND_Q_SIZE-2)) ?
              (local->sq_tail+1) : 0;

                                       /* start critical section             */
        tmp = i_disable(local->com.intr_priority);

                                       /* if not sounding alarm              */
        if (!local->alarm_active) {
          rc = next_sound();           /*   start sound request              */
        }
        else {                         /* else do nothing more               */
          rc = 0;
        }
        i_enable(tmp);                 /* end critical section               */
      }
    }
  }

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        next_sound                                                   */
/*                                                                           */
/* FUNCTION:    process next sound request on queue                          */
/*                                                                           */
/* INPUTS:      none                                                         */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*              EIO = I/O error                                              */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int next_sound(void)
{

   OFRAME sound_cmd[6];
   struct common *com;
   struct sound_q *sqe;
   int index;

   com = &local->com;
                                       /* if queue is empty                  */
   if (local->sq_head == local->sq_tail)  {
                                       /* restore clicker volume             */
      if (local->key.click != local->volume) {
         local->volume = local->key.click;
         switch(local->volume) {
            case KSCLICKLOW:
               put_oq1(com,(OFRAME) SET_SPKR_VOL_LOW);
               break;
            case KSCLICKMED:
               put_oq1(com, (OFRAME) SET_SPKR_VOL_MED);
               break;
            case KSCLICKHI:
               put_oq1(com, (OFRAME) SET_SPKR_VOL_HI);
               break;
            default:
               put_oq1(com, (OFRAME) SET_SPKR_VOL);
         }
      }
                                       /* alarm is off                       */
      local->alarm_active = FALSE;
   }
   else {                              /* else process next request ...      */
      sqe = &local->sound_q[local->sq_head];

      index = 1;                       /*  change volume as required         */
      if (sqe->volume != local->volume) {
         switch (sqe->volume) {
            case KSAVOLMED:
               sound_cmd[1] = SET_SPKR_VOL_MED;
               local->volume = KSAVOLMED;
               break;
            case KSAVOLHI:
               sound_cmd[1] = SET_SPKR_VOL_HI;
               local->volume = KSAVOLHI;
               break;
            default:
               sound_cmd[1] = SET_SPKR_VOL_LOW;
               local->volume = KSAVOLLOW;
         }
         index = 2;
      }
                                       /* send commands to sound alarm       */
      sound_cmd[index++] = SET_SPK_FREQ_HI  | (sqe->freq & 0xFF00);
      sound_cmd[index++] = SET_SPK_FREQ_LO  | ((sqe->freq & 0x00FF) << 8);
      sound_cmd[index++] = SET_SPK_DURATION | (sqe->duration & 0xFF00);
      sound_cmd[index] = WRITE_SPK_CMD | ((sqe->duration & 0x00FF) << 8);
      sound_cmd[0] = index;
      put_oq(com,sound_cmd);
                                       /* alarm is on                        */
      local->alarm_active = TRUE;
                                       /* update sound queue head index      */
      local->sq_head = (local->sq_head < (SOUND_Q_SIZE-2)) ?
              (local->sq_head+1) : 0;
   }

   KTSMDTRACE(next_sound, exit, local->sq_head,
          local->sq_tail,0,0,0);

   return(0);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        stop_sound                                                   */
/*                                                                           */
/* FUNCTION:    Stop current sound, flush sound queue                        */
/*                                                                           */
/* INPUTS:      none                                                         */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void stop_sound(void)
{
   int tmp;
                                       /* start critical section             */
   tmp = i_disable(local->com.intr_priority);
   local->sq_head = 0;                 /* flush alarm queue                  */
   local->sq_tail = 0;
   if (local->alarm_active) {          /* if alarm is on then                */
                                       /*   turn it off                      */
      put_oq1(&local->com, (OFRAME) TERMINATE_SPKR);
      local->alarm_active = FALSE;     /* indicate that alarm is off         */
   }
   i_enable(tmp);                      /* end critical section               */
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        chg_clicker                                                  */
/*                                                                           */
/* FUNCTION:    Change clicker volume                                        */
/*                                                                           */
/* INPUTS:      volume = new clicker volume                                  */
/*                                                                           */
/* OUTPUTS:     0 = success                                                  */
/*              EIO = I/O error                                              */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int chg_clicker(uint volume)
{
   int rc;
   struct common *com;
   int tmp;

   com = &local->com;
   rc = 0;
                                       /* start critical section             */
   tmp = i_disable(com->intr_priority);
   local->key.click = volume;          /* save new clicker volume            */

   if (!local->alarm_active) {         /* if not sounding alarm              */

                                       /* if adapter volume is not same as   */
                                       /* requested clicker volume then set  */
                                       /* new volume                         */
      if (volume != local->volume) {
         local->volume = volume;
         switch(volume) {
            case KSCLICKLOW:
               put_oq1(com, (OFRAME) SET_SPKR_VOL_LOW);
               break;
            case KSCLICKMED:
               put_oq1(com, (OFRAME) SET_SPKR_VOL_MED);
               break;
            case KSCLICKHI:
               put_oq1(com, (OFRAME) SET_SPKR_VOL_HI);
               break;
            default:
               put_oq1(com, (OFRAME) SET_SPKR_VOL);
         }
         i_enable(tmp);                /* end critical section               */
         rc = wait_oq(com);            /* wait for completion                */
      }
      else {
        i_enable(tmp);                 /* end critical section               */
      }
   }
   else {
      i_enable(tmp);                   /* end critical section               */
   }

   return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        mem_dump                                                     */
/*                                                                           */
/* FUNCTION:    returns address of areas to be included in system dump       */
/*                                                                           */
/* INPUTS:      pass = 1 for start dump                                      */
/*                     2 for end of dump                                     */
/*                                                                           */
/* OUTPUTS:     address of componet dump table structure                     */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

struct cdt * mem_dump(int pass)
{
                                       /* address of local data structure    */
  my_cdt.cdt_entry[0].d_ptr = (caddr_t) local;
  return(&my_cdt);

}


/*****************************************************************************/
/*                                                                           */
/* NAME:        service_vector                                               */
/*                                                                           */
/* FUNCTION:    externalize certain routines                                 */
/*                                                                           */
/*****************************************************************************/

void * service_vector[] =  {
             (void *) sv_alarm,        /* routine to sound alarm             */
             (void *) sv_sak,          /* routine to enable/disable sak      */
             (void *) sv_rflush        /* routine to flush queue             */
};

/*****************************************************************************/
/*                                                                           */
/* NAME:        init_8051                                                    */
/*                                                                           */
/* FUNCTION:    8051 initialization commands                                 */
/*                                                                           */
/*****************************************************************************/

OFRAME init_8051[] = {
             5,
             DONT_SRCH_SYS_ATN,         /* ignore 3 key sequences            */
             DO_REP_KBD_ACK,            /* send us ACK from keyboard         */
             AUTO_CLICK_ON,             /* auto click on                     */
             ENABLE_TAB,                /* enable tablet interface           */
             DONT_BLK_TAB_BYTES         /* don't block tablet frames         */
        };

/*****************************************************************************/
/*                                                                           */
/* NAME:        my_cdt                                                       */
/*                                                                           */
/* FUNCTION:    define areas to be included in system dump                   */
/*                                                                           */
/*****************************************************************************/

struct cdt my_cdt =  {
                                        /* cdt_head:                         */
          {DMP_MAGIC,                   /*   magic number                    */
                                        /*   component dump name             */
          {'k', 't', 's', 'd', 'd', ' ', ' ', ' ',
           ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
          sizeof(struct cdt)},          /*   length                          */
                                        /* cdt entry:                        */
                                        /*   name of data area               */
          {{{'l', 'o', 'c', 'a', 'l', ' ', ' ', ' '},
           sizeof(struct local),        /*   length of data area             */
           NULL,                        /*   address of area                 */
           {0},                         /*   cross memory parms              */
           }}};
