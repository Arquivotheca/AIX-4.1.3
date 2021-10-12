static char sccsid[] = "@(#)63	1.3  src/bos/kernext/inputdd/sgiodd/sgioutil.c, inputdd, bos41J, 9513A_all 3/28/95 16:00:41";
/*
 * COMPONENT_NAME: (INPUTDD) Serial Graphics I/O device driver
 *
 * FUNCTIONS: sgio_rring, sgio_uring
 *
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
/* NAME:        sgio_log                                                     */
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

void sgio_log(char *res_name, char *rout, uint err_code, uint loc,
              char *details)
{

   ERR_REC(80) er;

   SGIOTRACE1(sgio_log, enter, err_code);
   BUGLPR(0,0,(">>>>> sgio_log: \n"));
   BUGLPR(0,0,("  resource name = %s \n", res_name));
   BUGLPR(0,0,("  routine name  = %s \n", rout));
   BUGLPR(0,0,("  error code    = %d \n", err_code));
   BUGLPR(0,0,("  location      = %d \n", loc));
   BUGLPR(0,0,("  details       = %s \n", details));

   /* Set template id # */
   er.error_id = ERRID_GRAPHICS;

   /* Set resource name */
   sprintf(er.resource_name,"%s",res_name);

   /* Format error log entry */
   sprintf(er.detail_data,"%10s                  %02d     %02d      %s",
     rout, err_code, loc, details);

   /* Log error in system log */
   errsave(&er, ERR_REC_SIZE+strlen(er.detail_data)+1);
}


void io_error(char *rout, int perm, uint err_code, uint loc, char *fmt, ...)
{
  char ss[30];
  va_list argp;

  /* Initialize variable argument list */
  va_start(argp, fmt);

  _vsprintf(ss, fmt, argp);
  sgio_log(RES_NAME_SGIO, rout, err_code, loc, ss);

  /* Terminate variable argument list */
  va_end(argp);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sgio_uring                                                   */
/*                                                                           */
/* FUNCTION:    register input ring which reside in user address space       */
/*                                                                           */
/* INPUTS:      rcb = pointer to ring control block                          */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       rcb->ring_ptr must be set to NULL before detach/unpin so     */
/*              that there is not a race condition with interrupt handler    */
/*                                                                           */
/*****************************************************************************/

void sgio_uring(struct rcb *rcb)
{
   caddr_t ptr;


   ptr = rcb->ring_ptr;                /* save pointer to ring               */
   rcb->ring_ptr = (caddr_t) NULL;     /* clear ring pointer                 */

   if (ptr != NULL) {                  /* only do if ring registered         */

     xmdetach(&rcb->xmdb);             /*   detach from memory               */
                                       /*   unpin memory                     */
     unpinu(ptr, rcb->ring_size, (short) UIO_USERSPACE);
   }
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        sgio_rring                                                   */
/*                                                                           */
/* FUNCTION:    register input ring which reside in user address space       */
/*                                                                           */
/* INPUTS:      rcb = pointer to ring control block                          */
/*              arg = pointer to uregring structure in user address space    */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              EIO    = I/O error                                           */
/*              EFAULT = insufficient authority                              */
/*              ENOMEM = insufficient memory for required paging operation   */
/*              ENOSPC = insufficient file or paging space                   */
/*              EPERM  = attach failed                                       */
/*              EINVAL = invalid ring parameters                             */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       rcb->ring_ptr must be set as the very last step so that      */
/*              there is not a race condition with interrupt handler         */
/*                                                                           */
/*****************************************************************************/

int sgio_rring(struct rcb *rcb, char *arg)
{

   struct uregring uregring;
   struct inputring ir;
   int rc;
   caddr_t start, end;
   struct xmem wrk_xmem;

                                       /* copy arguments to local mem        */
   rc = copyin(arg, (char *) &uregring, sizeof(struct uregring));

   SGIODTRACE(sgio_rring, enter, rc, uregring.ring, uregring.size,
     uregring.report_id, 0);

   if (!rc) {                          /* if ok ...                          */
                                       /* if new ring not specified then     */
      if (uregring.ring == (caddr_t) NULL) {
         sgio_uring(rcb);              /* just unregister old ring           */
      }
      else {                           /* if new ring, verify that parms are */
                                       /* ok (for now anyway)                */
                                       /* ...copy ring header to local mem   */
         rc = copyin(uregring.ring, (char *) &ir, sizeof(struct inputring));
         if (!rc) {                    /* ...if ok verify pointers and size  */

            SGIODTRACE(sgio_rring, header, ir.ir_size, ir.ir_tail,
              ir.ir_head,0,0);

            start = uregring.ring + sizeof(struct inputring);
            end = start + ir.ir_size;
            if ((uregring.size < (sizeof(struct inputring) + ir.ir_size)) ||
               (ir.ir_size ==  0 ) ||
               (ir.ir_tail >= end ) ||
               (ir.ir_head >= end )  ||
               (ir.ir_tail < start) ||
               (ir.ir_head < start)) {
               rc = EINVAL;
            }
            else {                     /* if ok attach to user memory        */
               wrk_xmem.aspace_id = XMEM_INVAL;
               if (xmattach((char *) uregring.ring, uregring.size,
                   &wrk_xmem, USER_ADSPACE) != XMEM_SUCC) {
                  rc = EPERM;
               }
               else {                  /* if ok, pin ring                    */
                  rc = pinu(uregring.ring, uregring.size,
                      (short) UIO_USERSPACE);
                  if (rc) {            /* ...failed, detach ring             */
                     xmdetach(&wrk_xmem);
                  }
                  else {               /* if ok ...                          */
                     sgio_uring(rcb);  /* unregister old ring                */
                                       /* save new xmem attach in rcb        */
                     bcopy((char *) &wrk_xmem, (char *) &rcb->xmdb,
                        sizeof(struct xmem));
                                       /* save PID of ring owner             */
                     rcb->owner_pid = getpid();
                                       /* copy new ring parms to rcb         */
                     rcb->rpt_id =   uregring.report_id;
                     rcb->ring_size = uregring.size;
                                       /* WARNING: must be last (see NOTES)  */
                     rcb->ring_ptr = uregring.ring;
                  }
               }
            }
         }
      }
   }

   SGIODTRACE1(sgio_rring, exit, rc);

   return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sgio_xcpi                                                    */
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

void sgio_xcpi(caddr_t sce, caddr_t sink, uchar cnt, struct rcb *rcb)
{

                                       /* copy data to buffer if             */
   if (rcb->owner_pid == KERNEL_PID) { /* ... buffer is in kernel space      */
      bcopy((char *) sce, (char *) sink, cnt);
   }
   else  {                             /* ... buffer is in user space        */
      if (xmemin(sce, sink, cnt, &rcb->xmdb) != XMEM_SUCC) {
                                       /*       copy out failure so          */
                                       /*       log error                    */
         io_error("sgio_xcpi", FALSE, XMEMCPY_ERROR, 0, NULL);
      }
   }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sgio_xcpo                                                    */
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

void sgio_xcpo(caddr_t sce, caddr_t sink, uchar cnt, struct rcb *rcb)
{

                                       /* copy data to buffer if             */
   if (rcb->owner_pid == KERNEL_PID) { /* ... buffer is in kernel space      */
      bcopy((char *) sce, (char *) sink, cnt);
   }
   else  {                             /* ... buffer is in user space        */
      if (xmemout(sce, sink, cnt, &rcb->xmdb) != XMEM_SUCC) {
                                       /*       copy out failure so          */
                                       /*       log error                    */
         io_error("sgio_xcpo", FALSE, XMEMCPY_ERROR, 0, NULL);
      }
   }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sgio_rflush                                                  */
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

void sgio_rflush(int intr_priority, struct rcb *rcb)
{
   struct inputring head;              /* local copy of input ring header    */
   int tmp;  
                                  
   SGIODTRACE0(sgio_rflush, enter);

                                       /* continue only if ring registered   */
   if (rcb->ring_ptr != (caddr_t) NULL) {

                                       /* start critical section             */
      tmp = i_disable(intr_priority);
                                       /* get local copy of ring header      */
      sgio_xcpi(rcb->ring_ptr, (caddr_t) &head,
            (uchar) sizeof(struct inputring), rcb);
                                       /* set head = tail = start            */
      head.ir_head =  rcb->ring_ptr + sizeof(struct inputring);
      head.ir_tail = head.ir_head;
      head.ir_overflow = IROFCLEAR;    /* clear overflow flag                */ 
                                       /* copy local copy back to user space */
      sgio_xcpo((caddr_t) &head, rcb->ring_ptr,
          (uchar) sizeof(struct inputring), rcb);

      i_enable(tmp);                   /* end of critical section            */
   }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        sgio_putring                                                 */
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

void sgio_putring(struct rcb *rcb, struct ir_report *event, void *notify)
{

   struct inputring head;              /* local copy of input ring header    */
   caddr_t  start;                     /* pointer to start of reporting area */
   int      max_element_size;          /* number of bytes till end of        */
                                       /*   reporting area                   */
   int  need_to_notify = FALSE;
   caddr_t  new_tail;
   struct timestruc_t ts;

   SGIODTRACE(sgio_putring, enter, rcb->ring_ptr, event->report_size,
          rcb->owner_pid, 0 ,0 );

                                       /* continue only if ring registered   */
   if (rcb->ring_ptr != (caddr_t) NULL) {

                                       /* get local copy of ring header      */
      sgio_xcpi(rcb->ring_ptr, (caddr_t) &head,
            (uchar) sizeof(struct inputring), rcb);

      SGIODTRACE(sgio_putring, header, head.ir_size, head.ir_head,
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
               sgio_xcpo((caddr_t) event, head.ir_tail,
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
               sgio_xcpo((caddr_t) event, head.ir_tail,
                   (uchar) max_element_size, rcb);
                                       /* copy 2nd chuck to start of ring    */
                                       /* NOTE: if report exactly fit at end */
                                       /* of ring, then there is nothing     */
                                       /* else to copy                       */
               if (event->report_size > (uchar) max_element_size) {
                  sgio_xcpo((caddr_t)((caddr_t) event + max_element_size),
                       start, (uchar) (event->report_size -
                         (uchar) max_element_size), rcb);
               }
                                       /* update tail pointer                */
               head.ir_tail = new_tail;
            }
         }                             /* copy local copy of tail ptr and    */
                                       /* overflow flag to input ring        */
         sgio_xcpo((caddr_t) &head.ir_tail,
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

   SGIODTRACE(sgio_putring, exit, need_to_notify, head.ir_tail,
       head.ir_overflow, 0,0);
}
