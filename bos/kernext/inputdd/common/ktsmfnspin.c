static char sccsid[] = "@(#)50   1.3  src/bos/kernext/inputdd/common/ktsmfnspin.c, inputdd, bos411, 9428A410j 4/21/94 14:13:30";
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard/Tablet/Sound/Mouse DD - ktsmfnspin.c
 *
 * FUNCTIONS: ktsm_sleep, ktsm_wakeup, ktsm_putring, ktsm_xcpi, ktsm_xcpo,
 *            ktsm_rflush, _vsprintf, ktsm_log, put_iq, put_oq1, put_oq2,
 *            put_oq, get_oq, flush_q, startwd, get_iq
 *
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

#include "ktsm.h"
#include "common.h"

#ifndef ERRID_GRAPHICS
#ifdef ERRID_HFTERR
#define ERRID_GRAPHICS   ERRID_HFTERR
#else
#define ERRID_GRAPHICS   0xf10d0eff
#endif
#endif

/*****************************************************************************/
/*                                                                           */
/* NAME:        ktsm_sleep                                                   */
/*                                                                           */
/* FUNCTION:    This module sleeps for the specified time to allow an        */
/*              operation to complete.                                       */
/*                                                                           */
/* INPUTS:      com        = pointer to common data area                     */
/*              delay      = time to sleep in microseconds                   */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt or process                                         */
/*                                                                           */
/* NOTES:       e_sleep called therefore this routine MUST not be            */
/*              used by an interrupt handler. It can be used by              */
/*              a process who has raised its priority via i_disable()        */
/*                                                                           */
/*              While calling process sleeps, the kernel will most           */
/*              likely dispatch another process and will set the             */
/*              interrupt priority accordingly (most likely INTBASE).        */
/*              This will enable INTCLASS3 and allow standard I/O            */
/*              devices to interrupt the system. Interrupt priority          */
/*              will be re-established when the calling process wakes up     */
/*                                                                           */
/*              Priority is raised to INTMAX before starting timer to        */
/*              block interrupts from occuring between the time the          */
/*              timer is started and the time that the process is            */
/*              put to sleep. An interrupt during this time could            */
/*              result in the timer poping before the process is put         */
/*              to sleep causing the process to sleep forever.               */
/*                                                                           */
/*****************************************************************************/

void ktsm_sleep(struct common *com, ulong delay)
{
   int old_int;
   ulong   sec = 0 ;
   ulong   usec;


   KTSMDTRACE1(ktsm_sleep, enter, delay);

/* start timer                                                               */

   if ( delay >= 1000000 )  {          /* handle period longer than a second */
        sec = ( delay/1000000 ) ;
        usec = delay % 1000000 ;
   }
   else
        usec = delay;


                                       /* zero out structure                 */
   bzero (com->stimer , sizeof(struct trb)) ;

                                       /* set time interval                  */
   com->stimer->timeout.it_value.tv_sec = sec;
   com->stimer->timeout.it_value.tv_nsec = usec * 1000 ;
   com->stimer->flags |= T_INCINTERVAL ;
                                       /* timer interrupt handler            */
   com->stimer->func = (void (*) ()) ktsm_wakeup ;
   com->stimer->ipri = INTCLASS3 ;     /* run IH at priority of class 3      */
   com->stimer->t_union.addr = (caddr_t) com;

                                       /* mask all interrupts to make sure   */
                                       /* that we get to sleep before timer  */
   old_int = i_disable(INTMAX);        /* pops (see notes)                   */

                                       /* no one better be sleeping on       */
   VERIFY(com->asleep == EVENT_NULL);  /* anchor                             */

   tstart(com->stimer) ;               /* start timer                        */

/* sleep till it pop's                                                       */

   e_sleep(&com->asleep, EVENT_SHORT ) ;

   i_enable(old_int);
   tstop(com->stimer) ;               /* timer will still be running if     */
                                      /* wakeup is from I/O so stop it      */

   KTSMDTRACE0(ktsm_sleep, exit);

}


/*****************************************************************************/
/*                                                                           */
/* NAME:        ktsm_wakeup                                                  */
/*                                                                           */
/* FUNCTION:    Handles ktsm_sleep timer pop                                 */
/*                                                                           */
/* INPUTS:      tb = address of pop'ed trb                                   */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void ktsm_wakeup(struct trb * tb)
{
  struct common *com;

  com = (struct common *) tb->t_union.addr;
  e_wakeup(&com->asleep) ;
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        ktsm_putring                                                 */
/*                                                                           */
/* FUNCTION:    Enqueue event report onto input ring                         */
/*                                                                           */
/* INPUTS:      rcb      = pointer to ring control block                     */
/*              event    = pointer event report to be enqueued               */
/*              notify   = pointer to event notification call back routine   */
/*                         if kernel owns ring                               */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:       Design requires that all programs that place events on the   */
/*              input ring run with the same priority. This provides locking */
/*              in all cases except the SMP environment. It is assumed that  */
/*              the proper priority has been obtained before calling this    */
/*              routine                                                      */
/*                                                                           */
/*              The input ring is assumed to have been pinned by the         */
/*              ring registration ioctl                                      */
/*                                                                           */
/*              An event notification is sent even if the report causes      */
/*              a ring overflow or if the event report is not enqueued due   */
/*              to a copy failure (most unlikely).                           */
/*                                                                           */
/*              ring integrity is lost if cross memory copy fails. Error     */
/*              will be logged                                               */
/*                                                                           */
/*****************************************************************************/

void ktsm_putring(struct rcb *rcb, struct ir_report *event, void *notify)
{

   struct inputring head;              /* local copy of input ring header    */
   caddr_t  start;                     /* pointer to start of reporting area */
   int      max_element_size;          /* number of bytes till end of        */
                                       /*   reporting area                   */
   int  need_to_notify = FALSE;
   caddr_t  new_tail;
   struct timestruc_t ts;

   KTSMDTRACE(ktsm_putring, enter, rcb->ring_ptr, event->report_size,
          rcb->owner_pid, 0 ,0 );

                                       /* continue only if ring registered   */
   if (rcb->ring_ptr != (caddr_t) NULL) {

                                       /* get local copy of ring header      */
      ktsm_xcpi(rcb->ring_ptr, (caddr_t) &head,
            (uchar) sizeof(struct inputring), rcb);

      KTSMDTRACE(ktsm_putring, header, head.ir_size, head.ir_head,
         head.ir_tail, head.ir_overflow, head.ir_notifyreq);

                                       /* if overflow flag clear             */
      if (head.ir_overflow == IROFCLEAR) {

                                       /* start of reporting area            */
         start = rcb->ring_ptr + sizeof(head);

                                        /* verify header                      */
         VERIFY(head.ir_tail <  (start+head.ir_size));
         VERIFY(head.ir_head <  (start+head.ir_size));
         VERIFY(head.ir_tail >= start);
         VERIFY(head.ir_head >= start);

                                       /* Set flag to notify ring owner      */
                                       /* ...always                          */
         if ((head.ir_notifyreq == IRSIGALWAYS) ||
                                       /* ...only if ring is empty           */
           ((head.ir_tail == head.ir_head) &&
           (head.ir_notifyreq == IRSIGEMPTY))) need_to_notify = TRUE;

                                       /* save event report identifier       */
         event->report_id = rcb->rpt_id;
         curtime(&ts);                 /* time stamp in milliseconds         */
         event->report_time = (ts.tv_sec * 1000) +
                 (ts.tv_nsec / NS_PER_MSEC);

                                       /* number of bytes between tail and   */
                                       /* end of queue                       */
         max_element_size = (int) head.ir_size - (int)(head.ir_tail - start);

                                       /* if report will fit on ring without */
                                       /* wrapping tail pointer then ...     */
         if (max_element_size >  (int) event->report_size) {
                                       /* calculate new tail                 */
            new_tail = head.ir_tail + event->report_size;
                                       /* post overflow if enqueuing report  */
                                       /* would write over byte pointed to   */
                                       /* by head pointer or would cause     */
                                       /* tail ptr to equal head ptr         */
            if ((head.ir_tail  < head.ir_head) &&
                (new_tail >= head.ir_head)) {
               head.ir_overflow = IROVERFLOW;
            }
            else {                     /* copy event report to input ring    */
               ktsm_xcpo((caddr_t) event, head.ir_tail,
                    event->report_size, rcb);
                                       /* update tail pointer                */
               head.ir_tail = new_tail;
            }
         }
         else {                        /* else report will cause tail ptr    */
                                       /* to wrap ...                        */

                                       /* calculate new tail pointer         */
            new_tail = start + ((int) event->report_size - max_element_size);
                                       /* post overflow if enqueuing report  */
                                       /* would write over byte pointed to   */
                                       /* by head pointer or would cause     */
                                       /* tail ptr to equal head ptr         */
            if ((head.ir_tail < head.ir_head) ||
               (new_tail >= head.ir_head)) {
               head.ir_overflow = IROVERFLOW;
            }
            else {                     /* copy first chuck of event report   */
                                       /* to end of ring                     */
               ktsm_xcpo((caddr_t) event, head.ir_tail,
                   (uchar) max_element_size, rcb);
                                       /* copy 2nd chuck to start of ring    */
                                       /* NOTE: if report exactly fit at end */
                                       /* of ring, then there is nothing     */
                                       /* else to copy                       */
               if (event->report_size > (uchar) max_element_size) {
                  ktsm_xcpo((caddr_t)((caddr_t) event + max_element_size),
                       start, (uchar) (event->report_size -
                         (uchar) max_element_size), rcb);
               }
                                       /* update tail pointer                */
               head.ir_tail = new_tail;
            }
         }                             /* copy local copy of tail ptr and    */
                                       /* overflow flag to input ring        */
         ktsm_xcpo((caddr_t) &head.ir_tail,
             (caddr_t) &((struct inputring *)( rcb->ring_ptr))->ir_tail,
             (uchar)(sizeof(head.ir_tail)+sizeof(head.ir_overflow)), rcb);

                                       /* notify owner of ring of event      */
         if (need_to_notify) {
                                       /* ...when kernel owns ring           */
            if (rcb->owner_pid == KERNEL_PID) {
               if (notify != NULL)     /*    execute callback if specified   */
                  (*((void (*)()) notify))();
            }
            else {                     /* ...when user owns ring, send SIGMSG*/
               pidsig(rcb->owner_pid, SIGMSG);
            }
         }
      }
   }

   KTSMDTRACE(ktsm_putring, exit, need_to_notify, head.ir_tail,
       head.ir_overflow, 0,0);
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        ktsm_xcpi                                                    */
/*                                                                           */
/* FUNCTION:    Copy data to local memory from user or kernel buffer         */
/*                                                                           */
/* INPUTS:      sce  = source pointer (kernal or user address space)         */
/*              sink = sink pointer (local memory)                           */
/*              cnt  = number of bytes to be copied out                      */
/*              rcb  = pointer to ring control structure                     */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void ktsm_xcpi(caddr_t sce, caddr_t sink, uchar cnt, struct rcb *rcb)
{

                                       /* copy data to buffer if             */
   if (rcb->owner_pid == KERNEL_PID) { /* ... buffer is in kernel space      */
      bcopy((char *) sce, (char *) sink, cnt);
   }
   else  {                             /* ... buffer is in user space        */
      if (xmemin(sce, sink, cnt, &rcb->xmdb) != XMEM_SUCC) {
                                       /*       copy out failure so          */
                                       /*       log error                    */
         io_error("ktsm_xcpi", FALSE, XMEMCPY_ERROR, 0, NULL);
      }
   }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        ktsm_xcpo                                                    */
/*                                                                           */
/* FUNCTION:    Copy data from local memory to user or kernel buffer         */
/*                                                                           */
/* INPUTS:      sce = source pointer (local memory)                          */
/*              sink = sink pointer (kernal or user address space)           */
/*              cnt  = number of bytes to be copied out                      */
/*              rcb  = pointer to ring control structure                     */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void ktsm_xcpo(caddr_t sce, caddr_t sink, uchar cnt, struct rcb *rcb)
{

                                       /* copy data to buffer if             */
   if (rcb->owner_pid == KERNEL_PID) { /* ... buffer is in kernel space      */
      bcopy((char *) sce, (char *) sink, cnt);
   }
   else  {                             /* ... buffer is in user space        */
      if (xmemout(sce, sink, cnt, &rcb->xmdb) != XMEM_SUCC) {
                                       /*       copy out failure so          */
                                       /*       log error                    */
         io_error("ktsm_xcpo", FALSE, XMEMCPY_ERROR, 0, NULL);
      }
   }
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        ktsm_rflush                                                  */
/*                                                                           */
/* FUNCTION:    flush input ring                                             */
/*                                                                           */
/* INPUTS:      com = pointer to common area                                 */
/*              rcb = pointer to rcb structure                               */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/* ENVIRONMENT: Process or interrupt                                         */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void ktsm_rflush(struct common *com, struct rcb *rcb)
{
   struct inputring head;              /* local copy of input ring header    */
   int tmp;  
                                  

   KTSMDTRACE0(ktsm_rflush, enter);

                                       /* continue only if ring registered   */
   if (rcb->ring_ptr != (caddr_t) NULL) {

                                       /* start critical section             */
      tmp = i_disable(com->intr_priority);
                                       /* get local copy of ring header      */
      ktsm_xcpi(rcb->ring_ptr, (caddr_t) &head,
            (uchar) sizeof(struct inputring), rcb);
                                       /* set head = tail = start            */
      head.ir_head =  rcb->ring_ptr + sizeof(struct inputring);
      head.ir_tail = head.ir_head;
      head.ir_overflow = IROFCLEAR;    /* clear overflow flag                */ 
                                       /* copy local copy back to user space */
      ktsm_xcpo((caddr_t) &head, rcb->ring_ptr,
          (uchar) sizeof(struct inputring), rcb);

      i_enable(tmp);                   /* end of critical section            */
   }
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        _vsprintf                                                    */
/*                                                                           */
/* FUNCTION:    format string from variable number of arguments              */
/*                                                                           */
/* INPUTS:      ss = address of buffer                                       */
/*              fmt = pointer to format string                               */
/*              arg = argument list                                          */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt or process                                         */
/*                                                                           */
/* NOTES:       this is a very limited version of libc's vsprintf duplicated */ 
/*              here as vsprintf is not available to the kernel              */
/*                                                                           */
/*              supported formats: %c %x %d %02x (uchar) %04x (ushort) %08x  */
/*              supported number of variables: 5                             */
/*                                                                           */
/*****************************************************************************/

void _vsprintf(char *ss, char *fmt, va_list arg)
{
   ulong zz[5];
   int cnt;
   char *ptr;

   *ss = '\0';                         /* null terminate for case of NULL fmt*/
   ptr = fmt;
   cnt=0;
                                      /* load variables based on fmt string  */
   while ((ptr) && (cnt < 5)) {
     if(ptr = strchr(ptr, (int) '%')) {
        if(!(strncmp(ptr,"%x",2))) {
           zz[cnt++] = (ulong) (va_arg(arg, uint));
        }
        else {
          if(!(strncmp(ptr,"%02x",4))) {
             zz[cnt++] = (ulong) (va_arg(arg, uchar));
          }
          else {
            if(!(strncmp(ptr,"%04x",4))) {
              zz[cnt++] = (ulong) (va_arg(arg, ushort));
            }
            else {
              if(!(strncmp(ptr,"%08x",4))) {
                zz[cnt++] = (ulong) (va_arg(arg, uint));
              }
              else {
                if(!(strncmp(ptr,"%d", 2))) {
                  zz[cnt++] = (ulong) (va_arg(arg, uint));
                }
                else {
                  if(!(strncmp(ptr,"%c",2))) {
                    zz[cnt++] = (ulong) (va_arg(arg, uchar));
                  }
                }
              }
            }
          }
        }
        ptr++;
     }
   }
                                       /* format data                        */
   sprintf(ss, fmt, zz[0], zz[1], zz[2], zz[3], zz[4]);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        ktsm_log                                                     */
/*                                                                           */
/* FUNCTION:    Puts entry into error log                                    */
/*                                                                           */
/* INPUTS:      res_name = error log resource name                           */
/*              rout = name of routine posting error                         */
/*              err_code = error code                                        */
/*              loc = location codede                                        */
/*              details = additional information                             */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt or process                                         */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void ktsm_log(char *res_name, char *rout, uint err_code, uint loc,
          char *details)
{

   ERR_REC(80) er;

   KTSMTRACE1(ktsm_log, enter, err_code);
   BUGLPR(0,0,(">>>>> ktsm_log: \n"));
   BUGLPR(0,0,("  resource name = %s \n", res_name));
   BUGLPR(0,0,("  routine name  = %s \n", rout));
   BUGLPR(0,0,("  error code    = %d \n", err_code));
   BUGLPR(0,0,("  location      = %d \n", loc));
   BUGLPR(0,0,("  details       = %s \n", details));

   er.error_id = ERRID_GRAPHICS;         /* Template Id #                    */
                                         /* resource name                    */
   sprintf(er.resource_name,"%s",res_name);
                                         /* format error log entry           */
   sprintf(er.detail_data,"%10s                  %02d     %02d      %s",
       rout, err_code, loc, details);
                                         
                                         /* log error in system log          */
   errsave(&er, ERR_REC_SIZE+strlen(er.detail_data)+1);

}

/*****************************************************************************/
/*                                                                           */
/* NAME:        put_iq                                                       */
/*                                                                           */
/* FUNCTION:    Put frame on input queue.                                    */
/*                                                                           */
/* INPUTS:      com   = pointer to common data area                          */
/*              frame = frame to placed on queue                             */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:       The input queue is used to pass frames received from the     */
/*              device by the interrupt handler to the top half of the       */
/*              driver.                                                      */
/*                                                                           */
/*****************************************************************************/


void put_iq(struct common *com, IFRAME frame)
{

  KTSMDTRACE(put_iq, enter, frame, com->inq.tail, 0, 0, 0);

                                       /* if there is room on the queue      */
  if (com->inq.tail < (IFRAME *)((char *)(com->inq.elements)
     + sizeof(com->inq.elements))) {
     *com->inq.tail = frame;           /*   put frame on queue at tail       */
     com->inq.tail++;                  /*   update tail pointer              */
  }

  e_wakeup(&com->asleep);              /* wake up any process waiting on I/O */
                                       /*   (may be none)                    */

  KTSMDTRACE(put_iq, exit, frame, com->inq.tail, 0, 0, 0);

}

/*****************************************************************************/
/*                                                                           */
/* NAME:        put_oq1                                                      */
/*                                                                           */
/* FUNCTION:    Put one immediate frame on output queue                      */
/*                                                                           */
/* INPUTS:      com   = pointer to common data area                          */
/*              d0    = frame to be placed on queue                          */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void put_oq1(struct common *com, OFRAME d0)
{
OFRAME frame[2];

   KTSMDTRACE1(put_oq1, enter, d0);

   frame[0] = 1;
   frame[1] = d0;
   put_oq(com, frame);
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        put_oq2                                                      */
/*                                                                           */
/* FUNCTION:    Put two immediate frames on output queue                     */
/*                                                                           */
/* INPUTS:      com   = pointer to common data area                          */
/*              d0    = frame to be placed on queue                          */
/*              d1    = frame to be placed on queue                          */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void put_oq2(struct common *com, OFRAME d0, OFRAME d1)
{
OFRAME frame[3];

   KTSMDTRACE(put_oq2, enter, d0, d1, 0, 0, 0);

   frame[0] = 2;
   frame[1] = d0;
   frame[2] = d1;
   put_oq(com, frame);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        put_oq                                                       */
/*                                                                           */
/* FUNCTION:    Put frame(s) on output queue and send first frame to device  */
/*              if I/O is not already in progress.                           */
/*                                                                           */
/* INPUTS:      com   = pointer to common data area                          */
/*              frame = pointer to array of frames to be placed on queue     */
/*                      (first in array is frame count, array must be in     */
/*                       pinned memory)                                      */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/* ENVIRONMENT: Process or interrupt                                         */
/*                                                                           */
/* NOTES:       To facilitate error retry, the output queue is segmented     */
/*              into transmission groups. Each call to put_oq() defines      */
/*              a transmission group. Each group consists of a frame count   */
/*              followed by 1 or more frames. The queue head points to the   */
/*              start of the transmission group (count field) while dq_ptr   */
/*              points to the next frame in the group to be transmitted.     */
/*              cur_frame holds the frame being transmitted.  When an        */
/*              errors occurs, such as a no-response timeout, the entire     */
/*              transmission group is resent. This is accomplished by        */
/*              setting dq_ptr equal to the queue head and starting the      */
/*              process over. The queue head is moved to the next            */
/*              transmission group only after all frames in the group have   */
/*              be successfully sent and acknowledged by the device          */
/*                                                                           */
/*              Frame count for transmission group must not be zero          */
/*                                                                           */
/*              Command causing device to send data (eg: RESET) must be      */
/*              located at the end of a transmission group                   */
/*                                                                           */
/*****************************************************************************/

void put_oq(struct common *com, OFRAME *frame)
{
   int     cnt;
   int     tmp;
   OFRAME  data;

   KTSMDTRACE(put_oq, enter, com->outq.head,
         com->outq.tail, com->outq.dq_ptr, com->perr, 0);

   if (!com->perr) {                   /* do not proceed if perm. error      */
                                       /* start critical section             */
      tmp = i_disable(com->intr_priority);

      cnt = (int) *frame;              /* first element in array is frame cnt*/
      do {
         *com->outq.tail = *frame;     /* put frame on queue at tail         */
         frame++;                      /* point to next frame in array       */
                                       /* if tail points to end of queue     */
         if (com->outq.tail == ((OFRAME *) ((char *)(com->outq.elements) +
                    sizeof(com->outq.elements)) - 1))
                                       /* then point tail to start of queue  */
            com->outq.tail = com->outq.elements;
         else                          /* else just point to next sequential */
            com->outq.tail++;          /*   location                         */
                                       /* assert if queue overflow (ie: tail */
                                       /*   is equal to head)                */
         VERIFY(com->outq.tail != com->outq.head);
      } while(cnt--);                  /* continue till all frames on queue  */

      if (com->in_progress == NO_OP) { /* if nothing is progress then        */
         send_q_frame(com);            /*   send first frame from group      */
      }
      i_enable(tmp);                   /* end critical section, restore      */
                                       /*   priority                         */
   }

   KTSMDTRACE(put_oq, exit, com->outq.head,
         com->outq.tail, com->outq.dq_ptr, com->perr, 0);

}


/*****************************************************************************/
/*                                                                           */
/* NAME:        get_oq                                                       */
/*                                                                           */
/* FUNCTION:    Get frame from output queue                                  */
/*                                                                           */
/* INPUTS:      com = pointer to common data area                            */
/*                                                                           */
/* OUTPUTS:     Frame from queue                                             */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/


OFRAME get_oq(struct common *com)
{
  OFRAME tmp;

  KTSMDTRACE(get_oq, enter, com->outq.dq_ptr, com->outq.out_cnt,
      0, 0, 0);

  tmp = *com->outq.dq_ptr;             /* get frame from queue               */
                                       /* if at end of queue                 */
  if (com->outq.dq_ptr == ((OFRAME *) ((char *)(com->outq.elements) +
                 sizeof(com->outq.elements)) - 1))
                                       /* then point to start of queue       */
     com->outq.dq_ptr = com->outq.elements;
  else                                 /* else point to next sequential      */
     com->outq.dq_ptr++;               /*   location                         */
  com->outq.out_cnt--;                 /* subtract 1 from count of remaining */
                                       /*  frames in this transmission group */

  KTSMDTRACE(get_oq, exit, tmp, com->outq.dq_ptr, com->outq.out_cnt,
      0, 0);

  return(tmp);                         /* return frame to caller             */
}



/*****************************************************************************/
/*                                                                           */
/* NAME:        flush_q                                                      */
/*                                                                           */
/* FUNCTION:    Flushes output queue                                         */
/*                                                                           */
/* INPUTS:      com = pointer to common data area                            */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void flush_q(struct common *com)
{
   int tmp;

                                       /* start of critical section          */
   tmp = i_disable(com->intr_priority);
                                       /* flush output queue                 */
   com->outq.head = (OFRAME *) &com->outq.elements;
   com->outq.tail = (OFRAME *) &com->outq.elements;
   com->outq.dq_ptr = (OFRAME *) &com->outq.elements;
   com->outq.out_cnt = 0;
   com->outq.error = TRUE;             /* indicate error                     */
   com->in_progress = NO_OP;           /* nothing going on                   */
   e_wakeup(&com->asleep);             /* wake up any process waiting on I/O */

   i_enable(tmp);                      /* end of critical section            */
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        startwd                                                      */
/*                                                                           */
/* FUNCTION:    Start watchdog timer                                         */
/*                                                                           */
/* INPUTS:      com = pointer to common area                                 */
/*              intr_handler = pointer to timer interrupt handler            */
/*              value = timer length in nano-seconds                         */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/


void startwd(struct common *com, void *intr_handler, ulong value)
{

   KTSMDTRACE1(startwd, enter, value);

   tstop(com->wdtimer);                /* stop watch dog                     */
                                       /* start again with new values        */
   bzero (com->wdtimer, sizeof(struct trb));
   com->wdtimer->timeout.it_value.tv_sec  = 0;
   com->wdtimer->timeout.it_value.tv_nsec = value;
   com->wdtimer->flags |= T_INCINTERVAL;
   com->wdtimer->func = (void (*) ()) intr_handler;
   com-> wdtimer->ipri = com->intr_priority;
   com->wdtimer->t_union.addr = (caddr_t) com;
   tstart(com->wdtimer);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        get_iq                                                       */
/*                                                                           */
/* FUNCTION:    get frame(s) off input queue                                 */
/*                                                                           */
/* INPUTS:      com  = pointer to common data area                           */
/*              cnt  = number of frames to fetch                             */
/*              to   = timeout for first frame in micro seconds              */
/*              buff = address of buffer to put frames                       */
/*                                                                           */
/* OUTPUTS:     number of frames received                                    */
/*                                                                           */
/* ENVIRONMENT: process                                                      */
/*                                                                           */
/* NOTES:       The driver must be in receive mode when this routine is      */
/*              called or put_oq() must have just been called to send cmd(s) */
/*              to the device which results in an ACK response before data   */
/*              is transmited                                                */
/*                                                                           */
/*****************************************************************************/


int get_iq(struct common *com, int cnt, ulong to, IFRAME *buff)
{

   int i, tmp;
   IFRAME *fp;

   KTSMDTRACE(get_iq, enter, cnt, to, com->inq.head,
       com->inq.tail,0);

   if (wait_oq(com)) {                 /* wait till cmd complete             */
      i = -1;                          /* command failed                     */
   }
   else {                              /* ok so far                          */
      i = 0;                           /* zero byte received cnt             */
      while (i < cnt) {   
                                       /* wait for response                  */
         if (com->inq.tail == com->inq.head) {
            ktsm_sleep(com, to);
            if (com->inq.tail == com->inq.head) break;
         }                             /* copy response to caller's buffer   */
         bcopy((char *) com->inq.head, (char *) buff, sizeof(IFRAME));
         i++;                          /* update cnt of frames rcv'ed        */
         buff++;                       /* update pointers                    */
         com->inq.head++;
         to = RCVTO;                   /* set timeout for subsequent frames  */
      }
   }
                                       /* start of critical section          */
   tmp = i_disable(com->intr_priority);
   send_q_frame(com);                  /* send next frame on queue           */
   i_enable(tmp);                      /* end of critical section            */


   KTSMDTRACE(get_iq, exit, i , *buff, *(buff+1), *(buff+2), *(buff+3));

   return(i);
}

