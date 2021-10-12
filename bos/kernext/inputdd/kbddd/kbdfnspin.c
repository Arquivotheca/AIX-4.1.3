static char sccsid[] = "@(#)85	1.5  src/bos/kernext/inputdd/kbddd/kbdfnspin.c, inputdd, bos41J, 9509A_all 2/14/95 13:20:30";
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard DD - kbdfnspin.c
 *
 * FUNCTIONS: send_q_frame, watch_dog, read_port, write_port,
 *            io_error, reg_intr, ureg_intr, sv_rflush,
 *            sv_sak, sv_alarm, put_sq, next_sound, stop_sound.
 *            mem_dump
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

#include "kbd.h"

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
/* NOTES:       Frames are stored on the output queue using format defined   */
/*              by the hardware for the RS1/RS2 platforms. The actual        */
/*              keyboard frame is located in the high order byte.            */
/*                                                                           */
/*****************************************************************************/


void send_q_frame(struct common *com)
{

   struct kbd_port *port;

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
       com->retry_cnt = 0;             /*    clear retry counter             */
       com->in_progress = KBD_ACK_RQD; /*    indicate I/O in progress        */
     }
   }

   com->cur_frame = get_oq(com);       /* get frame from queue               */
                                       /* get access to I/O bus              */
   port = (struct kbd_port *) BUSIO_ATT(com->bus_id,com->bus_addr);
                                       /* write frame to adapter data port   */
   if (write_port(&port->rd_wd_port, (char) (com->cur_frame >> 8))) {
                                       /*   start watch dog timer            */
     startwd(com, (void *) watch_dog, WDOGTIME);
   }

   BUSIO_DET((caddr_t) port);          /* detach from I/O bus                */

   KTSMTRACE(k_send_q_frame, exit, com->cur_frame, 
          com->in_progress, com->outq.dq_ptr, com->outq.out_cnt, 0);

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
/*              If the retry count has not been exceeded, this routine resets*/
/*              dq_ptr to the start of the transmission group and calls      */
/*              send_q_frame() to start the re-transmission of all frames    */
/*              in the transmission group.                                   */
/*                                                                           */
/*****************************************************************************/


void watch_dog(struct trb *tb)
{
   struct common *com;

   com = &local->com;                  /* pointer to common area             */

   KTSMTRACE(k_watch_dog, enter, com->cur_frame, com->in_progress,
       com->retry_cnt, local->status, local->data);

                                       /* if expecting ACK then ...          */
   if (com->in_progress & KBD_ACK_RQD) {

      if (com->retry_cnt < K_RETRY) {  /* if retry count is not exceeded     */
        com->retry_cnt++;              /*   update retry count               */
        com->outq.dq_ptr =             /*   reset ptr back to start of       */
            com->outq.head;            /*     transmission group             */
                                       /*   get number of frames in group    */
        com->outq.out_cnt = get_oq(com);
      }
      else {                           /* else retries are exhausted so      */
                                       /*   log error                        */
#ifndef GS_DEBUG_TRACE
        if (com->cur_frame != KBD_RESET_CMD)
#endif
          io_error("watch_dog", FALSE, KIO_ERROR, 0, "%02x %02x %04x %04x",
            local->status, local->data, com->cur_frame, com->in_progress);
        while(com->outq.out_cnt) {     /* flush transmission group           */
          get_oq(com);
        }
        com->outq.error = TRUE;        /* indicate that there was a problem  */
      }
      send_q_frame(com);               /* process next frame                 */
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
/* FUNCTION:    Write data to adapter port with PIO error recovery           */
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
                                       /* unrecoverable PIO error            */
   io_error("write_port", TRUE, PIO_ERROR, 0, "%x %02x", addr, data);
   return(FALSE);
}



/*****************************************************************************/
/*                                                                           */
/* NAME:        io_error                                                     */
/*                                                                           */
/* INPUTS:      rout = name of routine detecting error                       */
/*              perm = TRUE if hard error                                    */
/*              err_code = error code to be logged                           */
/*              loc = location code to be logged                             */
/*              fmt = format string for detailed information (or NULL)       */
/*                    (this is a limited printf like fmt string)             */
/*              ... = detailed information to be formated                    */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process or Interrupt                                         */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void io_error(char *rout, int perm, uint err_code, uint loc, char *fmt, ...)
{
   va_list arg;
   char ss[30];
   uchar *posreg_ptr;
   uchar posreg_val;
   int old_int;
   struct common *com;

   va_start(arg,fmt);                  /* log error                          */
   _vsprintf(ss, fmt, arg);
   ktsm_log(RES_NAME_KBD, rout, err_code, loc, ss);

   if (perm) {                         /* if hard error then ...             */
     com = &local->com;                /* pointer to common data struct      */

                                       /* disable keyboard interface         */
                                       /*   by asserting adapter reset       */
     posreg_ptr = IOCC_ATT((com->bus_id | 0x80), com->slot_addr + 2);
     old_int = i_disable(INTMAX);
     posreg_val = BUSIO_GETC(posreg_ptr);
     BUSIO_PUTC(posreg_ptr,posreg_val  | RESET_KEYBOARD);
     i_enable(old_int);
     IOCC_DET(posreg_ptr);

     flush_q(com);                     /* reset I/O queues                   */
     com->perr = TRUE;                 /* remember we're hosed               */
   }
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        reg_intr                                                     */
/*                                                                           */
/* FUNCTION:    Register interrupt handler                                   */
/*                                                                           */
/* INPUTS:      adpt_rst (not used)                                          */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*              EPERM = failed to register interrupt handler                 */
/*              ENOMEM = unable to pin driver, out of memory                 */
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
   struct kbd_port *port;
   struct common *com;


   com = &local->com;                  /* pointer to common area             */
   if (com->perr) return(EIO);         /* exit if permanent error posted     */

                                       /* pin bottom half of driver          */
   if (rc = pincode((int (*) ())kbdintr)) {
     return(rc);                       /* exit if error                      */
   }

   flush_q(com);                       /* initialize I/O queues              */
   com->outq.error = FALSE;            /* no errors yet                      */

                                       /* tell kernel about slih             */
   com->intr_data.next     = 0;
   com->intr_data.handler  = (int (*) ())kbdintr;
   com->intr_data.bus_type = BUS_MICRO_CHANNEL;
   com->intr_data.flags    = 0;
   com->intr_data.level    = com->intr_level;
   com->intr_data.priority = com->intr_priority;
   com->intr_data.bid      = com->bus_id;

   if (i_init(&com->intr_data) != INTR_SUCC) {
      unpincode((int (*) ())kbdintr);  /* if error, unpin bottom half and    */
      return(EPERM);                   /*   return                           */
   }
   local->ih_inst = TRUE;              /* interrupt handler is installed     */
   dmp_add(mem_dump);                  /* register system crash local memory */
                                       /* dump routine                       */

                                       /* reset keyboard adapter             */
   posreg_ptr = IOCC_ATT((com->bus_id | 0x80), com->slot_addr + 2);
   old_int = i_disable(INTMAX);
   posreg_val = BUSIO_GETC(posreg_ptr);
   BUSIO_PUTC(posreg_ptr,posreg_val  | RESET_KEYBOARD);
   BUSIO_PUTC(posreg_ptr,posreg_val  & ~RESET_KEYBOARD);
   i_enable(old_int);
   IOCC_DET(posreg_ptr);
                                       /* make sure alarm hardware is off    */
                                       /* (does not get reset with adpt rst) */
   port = (struct kbd_port *) BUSIO_ATT(com->bus_id, com->bus_addr);
   if (!write_port(&port->spkr_port_hi, (char) SPEAKER_OFF)) {
     ureg_intr();
     rc = EIO;
   }
   BUSIO_DET((caddr_t) port);

   return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        ureg_intr                                                    */
/*                                                                           */
/* FUNCTION:    Un-register interrupt handler                                */
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


void ureg_intr(void)
{
   uchar *posreg_ptr;
   uchar posreg_val;
   int old_int;

   stop_sound();                       /* stop alarm and clear queue         */
   wait_oq(&local->com);               /* wait for all I/O to complete       */
                                       /* inhibit keyboard interrupts        */
                                       /*   by asserting adapter reset       */
   posreg_ptr = IOCC_ATT((local->com.bus_id | 0x80), local->com.slot_addr + 2);
   old_int = i_disable(INTMAX);
   posreg_val = BUSIO_GETC(posreg_ptr);
   BUSIO_PUTC(posreg_ptr,posreg_val  | RESET_KEYBOARD);
   i_enable(old_int);
   IOCC_DET(posreg_ptr);

   if (local->ih_inst) {               /* if interrupt handler installed     */
      i_clear(&local->com.intr_data);  /* undefine interrupt handler         */
      dmp_del(mem_dump);               /* unregister local memory dump rout. */
      unpincode((int (*) ())kbdintr);  /* unpin bottom half                  */
      local->ih_inst = FALSE;          /* interrupt handler is not installed */
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
/* ENVIRONMENT: Process or interrupt                                         */
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


  KTSMDTRACE(put_sq, enter, arg->duration, arg->frequency, 0,0,0);

                                       /* make sure parms are valid          */
  if ((arg->duration > MAX_ALARM_DURATION) ||
      (arg->frequency > MAX_ALARM_FREQ) ||
      (arg->frequency < MIN_ALARM_FREQ)) {
    rc = EINVAL;
  }
  else {                                /* sound alarm only if some volume   */
                                        /* and some time                     */
    if ((local->key.volume == SPEAKER_OFF) ||
        (arg->duration == 0)) {
       rc = 0;
    }
    else {
                                        /* return busy if queue is full      */
        if ((local->sq_tail + 1 ) == local->sq_head) {
          rc = EBUSY;
        }
        else {                          /* put request on queue              */
          sqe = &local->sound_q[local->sq_tail];

          sqe->sec =  arg->duration/128; 
          sqe->nsec = (arg->duration % 128) * (NS_PER_SEC/128);

          freq = 1000000 / (arg->frequency * 4);
          sqe->locmd = (uchar )(freq);
          sqe->hicmd= ENABLE_SPKR | (local->key.volume << 5) |
                     (uchar)(freq>>8);

          KTSMTRACE(put_sq, qe, sqe->sec, sqe->nsec,
                 sqe->hicmd, sqe->locmd, local->sq_tail);

                                       /* update sound queue tail index      */
          local->sq_tail = (local->sq_tail < (SOUND_Q_SIZE-2)) ?
              (local->sq_tail+1) : 0;


                                       /* start critical section             */
          tmp = i_disable(local->com.intr_priority);
                                     
                                       /* if not sounding alarm              */
          if (!local->alarm_active) {
             rc = next_sound();        /*   start sound request              */
          }
          else {                       /* else do nothing more               */
            rc = 0; 
          }
          i_enable(tmp);               /* end critical section               */
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
/* NOTES:       This routine is called directly by put_sq() when the alarm   */
/*              is off and it is called when the sound timer pop's.          */
/*                                                                           */
/*****************************************************************************/

int next_sound(void) 
{

   int rc = EIO;
   struct kbd_port *port;
   struct sound_q *sqe;

                                       /* get access to I/O bus              */
   port = (struct kbd_port *) BUSIO_ATT(local->com.bus_id,
              local->com.bus_addr);

                                       /* if queue is empty                  */
   if (local->sq_head == local->sq_tail)  {
                                       /* turn off sound                     */
      if (write_port(&port->spkr_port_hi, (char) SPEAKER_OFF)) {
                                       /* alarm is off                       */
         local->alarm_active = FALSE; 
         rc = 0;                       /* ok return code                     */
      }
   }
   else {                              /* else process next request ...      */
                                       /* write sound data to adapter        */
      sqe = &local->sound_q[local->sq_head];
      if (write_port(&port->spkr_port_lo, sqe->locmd)) {
         if (write_port(&port->spkr_port_hi, sqe->hicmd)) {

                                       /* alarm is on                        */
            local->alarm_active = TRUE;
                                       /* zero out timer structure           */
            bzero (local->sdd_time_ptr, sizeof(struct trb));
                                       /* set time interval                  */
            local->sdd_time_ptr->timeout.it_value.tv_sec = sqe->sec;
            local->sdd_time_ptr->timeout.it_value.tv_nsec = sqe->nsec;
            local->sdd_time_ptr->flags |= T_INCINTERVAL;
                                       /* timer interrupt handler            */
            local->sdd_time_ptr->func = (void (*) ()) next_sound;
                                       /* timer interrupt priority           */
            local->sdd_time_ptr->ipri = local->com.intr_priority;
                                       /* start timer                        */
            tstart(local->sdd_time_ptr);
                                       /* update sound queue head index      */
            local->sq_head = (local->sq_head < (SOUND_Q_SIZE-2)) ?
                (local->sq_head+1) : 0;

            rc = 0;                    /* all ok                             */
         }
      }
   }

   BUSIO_DET((caddr_t) port);          /* release access to the I/O bus      */

   KTSMTRACE(next_sound, exit, rc, local->sq_head, 
          local->sq_tail, 0, 0);

   return(rc);
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        stop_sound                                                   */
/*                                                                           */
/* FUNCTION:    Stop current sound, flush sound queue, and free timer        */
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

  if (local->alarm_active) {           /* if alarm is on then                */
     tstop(local->sdd_time_ptr);       /*    turn off timer                  */
     local->sq_head = 0;               /*    flush alarm queue               */
     local->sq_tail = 0;
     next_sound();                     /*    turn off alarm                  */
  }
  i_enable(tmp);                       /* end critical section               */
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
/* OUTPUTS:     address of component dump table structure                    */
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
/* NAME:        my_cdt                                                       */
/*                                                                           */
/* FUNCTION:    define areas to be included in system dump                   */
/*                                                                           */
/*****************************************************************************/

struct cdt my_cdt =  {
                                        /* cdt_head:                         */
          {DMP_MAGIC,                   /*   magic number                    */
                                        /*   component dump name             */
          {'k', 'b', 'd', 'd', 'd', ' ', ' ', ' ',
           ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
          sizeof(struct cdt)},          /*   length                          */
                                        /* cdt entry:                        */
                                        /*   name of data area               */
          {{{'l', 'o', 'c', 'a', 'l', ' ', ' ', ' '},
           sizeof(struct local),        /*   length of data area             */
           NULL,                        /*   address of area                 */
           {0},                         /*   cross memory parms              */
           }}};
