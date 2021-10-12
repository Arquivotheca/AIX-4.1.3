static char sccsid[] = "@(#)02   1.5  src/bos/kernext/inputdd/common/ktsmfns.c, inputdd, bos41J, 9519A_all 5/9/95 07:12:49";
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard/Tablet/Sound/Mouse DD - ktsmfns.c
 *
 * FUNCTIONS: ktsm_rring, ktsm_uring, wait_oq
 *
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

#include "ktsm.h"
#include "common.h"


/*****************************************************************************/
/*                                                                           */
/* NAME:        ktsm_rring                                                   */
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

int ktsm_rring(struct rcb *rcb, char *arg)
{

   struct uregring uregring;
   struct inputring ir;
   int rc;
   caddr_t start, end;
   struct xmem wrk_xmem;

                                       /* copy arguments to local mem        */
   rc = copyin(arg, (char *) &uregring, sizeof(struct uregring));

   KTSMDTRACE(ktsm_rring, enter, rc, uregring.ring, uregring.size,
     uregring.report_id, 0);

   if (!rc) {                          /* if ok ...                          */
                                       /* if new ring not specified then     */
      if (uregring.ring == (caddr_t) NULL) {
         ktsm_uring(rcb);              /* just unregister old ring           */
      }
      else {                           /* if new ring, verify that parms are */
                                       /* ok (for now anyway)                */
                                       /* ...copy ring header to local mem   */
         rc = copyin(uregring.ring, (char *) &ir, sizeof(struct inputring));
         if (!rc) {                    /* ...if ok verify pointers and size  */

            KTSMDTRACE(ktsm_rring, header, ir.ir_size, ir.ir_tail,
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
                     ktsm_uring(rcb);  /* unregister old ring                */
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

   KTSMDTRACE1(ktsm_rring, exit, rc);

   return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        ktsm_uring                                                   */
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

void ktsm_uring(struct rcb *rcb)
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
/* NAME:        wait_oq                                                      */
/*                                                                           */
/* FUNCTION:    wait for operation to complete                               */
/*                                                                           */
/* INPUTS:      com  = pointer to common data area                           */
/*                                                                           */
/* OUTPUTS:     0   = successful                                             */
/*              EIO = timeout in micro seconds                               */
/*                                                                           */
/* ENVIRONMENT: process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/


int wait_oq(struct common *com)
{

   int rc;

   KTSMDTRACE1(wait_oq, enter, com->in_progress);

                                        /* wait till I/O completed           */
   while (com->in_progress & (ACK_RQD | UNLOCK_WAIT)) {
      ktsm_sleep(com,FRAME_WAIT);
   }
   if (com->perr || com->outq.error) {  /* if operation failed then          */
      com->outq.error = FALSE;          /*   clear error flag                */
      rc = EIO;                         /*   set error return code           */
   }
   else {
      rc = 0;                           /* else successful                   */
   }

   KTSMDTRACE1(wait_oq, exit, rc);

   return(rc);
}
