static char sccsid[] = "@(#)86   1.3  src/bos/kernext/inputdd/kbddd/kbdintr.c, inputdd, bos411, 9428A410j 4/16/94 14:10:10";
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard DD - kbdintr.c
 *
 * FUNCTIONS: kbdintr, kbderr, kbdack
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

#include "kbd.h"

/*****************************************************************************/
/*                                                                           */
/* NAME:             kbdintr                                                 */
/*                                                                           */
/* FUNCTION:         This module is the interrupt handler for the keyboard,  */
/*                   is called by the First Level Interrupt Handle when an   */
/*                   interrupt is detected from one of the devices on our    */
/*                   interrupt level.  This module determines if the kybd    */
/*                   caused the interrupt by checking the status of the stat */
/*                   port.  If it is an kybd interrupt then kbdintr must     */
/*                   determine the reason and process it accordingly.        */
/*                                                                           */
/* INPUTS:           intr_ptr = pointer to our interrupt data                */
/*                                                                           */
/* OUTPUTS:          INTR_SUCC if an kybd interrupt, INTR_FAIL otherwise     */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  kbdintr(struct intr *intr_ptr)
{
   struct common *com;
   struct  kbd_port *port;

   KTSMDTRACE0(kbdintr, enter);

   com = &local->com;                  /* pointer to common data area        */
                                       /* get access to I/O bus              */
   port = (struct kbd_port *) BUSIO_ATT(com->bus_id,com->bus_addr);

                                       /* read kbd adapter status port       */
   if (read_port(&port->rs_wc_port, &local->status)) {
      if (!(local->status & RX_FULL)) {/* if no rcv data then not our intr   */
         BUSIO_DET((caddr_t) port);    /*   detach from I/O bus and return   */
         return(INTR_FAIL);            /*   failed                           */
      }                               

      else {                           /* if xmit buffer full then top half  */
                                       /* is trying to send frame so dump    */
         if (!(local->status & TX_EMPTY)) {
                                       /* rcv'ed data so frame will be xmited*/
               read_port(&port->rd_wd_port, &local->data);
         }
         else {                        
                                       /* process data when errors detected  */
            if (local->status & KEYERROR) { 
               kbderr(com,port);       /* by the hardware                    */
            }
            else {                     /* if good data rcv'ed from kbd       */
                                       /* read data from adapter port        */
               if (read_port(&port->rd_wd_port, &local->data)) {

                  switch (local->data) {
                     case KBD_ACK:     /* process ACK response               */
                        kbdack(com);
                        break;

                     case OVERRUN:     /* ignore overrun                     */

                     case KBD_ECHO:    /* should never get ECHO              */
                        break;

                     case KBD_RESEND:  /* just set error flag if RESEND      */
                                       /* watch dog routine will recover     */
                        if (com->in_progress & KBD_ACK_RQD) {
                           com->in_progress |= OP_RCV_ERR;
                        }
                        break;  

                     default:          /* process other responses:           */
                                       /*   if in receive mode then enqueue  */
                                       /*   frame to input queue             */
                        if ( com->in_progress & RCV_KBD) {
                           put_iq(com, local->data);
                        }
                        else {         /*   else go process scan code        */
                                       /*   if at least one channel is open  */
                           if (local->key.act_ch != NO_ACT_CH) {
                             keyproc(com, &local->key, local->data);
                           }
                        }
                        break;
                  }
               }
            }
         }
      }
   }
   BUSIO_DET((caddr_t) port);          /* release access to the I/O bus      */
   i_reset(intr_ptr);                  /* reset interrupt                    */

   KTSMTRACE(kbdintr, exit, local->status, local->data, com->cur_frame,
       com->in_progress, local->key.act_ch);

   return(INTR_SUCC);                  /* return - interrupt serviced        */
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             kbderr                                                  */
/*                                                                           */
/* FUNCTION:         Process line errors                                     */
/*                                                                           */
/* INPUTS:           com = pointer to common area                            */
/*                   port = pointer to attached I/O ports                    */
/*                                                                           */
/* OUTPUTS:          N/A                                                     */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:            A 30ms watch dog timer is started when a frame is       */
/*                   sent to the keyboard. When the timer pop's, the timer   */
/*                   routine resends the first frame in the current          */
/*                   transmission group. The watch dog timer is stopped by   */
/*                   the keyboard interrupt handler when a valid response    */
/*                   is received from the keyboard. If a RESEND response     */
/*                   is received following a frame transmission or if a      */
/*                   receive parity error is detected by the hardware, the   */
/*                   interrupt handler just lets the watchdog timer pop      */
/*                   resulting in the retransmission of the frame.           */
/*                                                                           */
/*****************************************************************************/

void kbderr(struct common *com, struct kbd_port *port)
{


   if (com->in_progress == NO_OP) {    /* if scan code from keyboard         */
                                       /*   send RESEND cmd to keyboard      */
                                       /*   so keyboard will re-xmit frame   */
      write_port(&port->rd_wd_port, (char) K_RESEND_CMD);
                                       /*   bump error counter               */
      if ((com->rcv_err_cnt++) > RCV_ERR_THRESHOLD) {
                                       /*   if over threshold, log error     */
         io_error("kbderr", FALSE, RCV_ERROR, 0, NULL);
         com->rcv_err_cnt = 0;         /*   and reset counter                */
      }
   }
   else {                              /* if receiving frames from kbd       */
                                       /*   send RESEND to kbd if retries    */
                                       /*   have not been exhausted          */
      if (com->in_progress & RCV_KBD) {
         com->in_progress |= OP_RCV_ERR;
         if (com->retry_cnt < K_RETRY) {
            com->retry_cnt++;
            write_port(&port->rd_wd_port, (char) K_RESEND_CMD);
         }
      }
      else {                           /* if sending frames to kbd let       */
                                       /*   watch dog timer handle error but */
                                       /*   change timer value if RESET cmd  */
                                       /*   sent just in case kbd executed   */
                                       /*   the RESET cmd                    */
         if (com->in_progress & KBD_ACK_RQD) {
            com->in_progress |= OP_RCV_ERR;
            if (com->cur_frame == KBD_RESET_CMD) {
               startwd(com, (void *) watch_dog, RSTWD);
            }
         }
      }
   }                                  /* flush frame that had rcv error      */
   if (!read_port(&port->rd_wd_port, &local->data)) {
                                      /* stop watch dog timer if error      */
     if (com->in_progress & KBD_ACK_RQD) {
       tstop(com->wdtimer);   
     }
   }
}



/*****************************************************************************/
/*                                                                           */
/* NAME:             kback                                                   */
/*                                                                           */
/* FUNCTION:         Process ACK response from keyboard                      */
/*                                                                           */
/* INPUTS:           com = pointer to commone area                           */
/*                                                                           */
/* OUTPUTS:          N/A                                                     */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:            KBD_RESET and LAYOUT_ID_CMD must be last command in     */
/*                   group                                                   */
/*                                                                           */
/*****************************************************************************/

void kbdack(struct common *com)
{

                                       /* if expecting ACK then              */
   if (com->in_progress & KBD_ACK_RQD) {
      tstop(com->wdtimer);             /* stop watch dog timer               */
                                       /*  if ACK from RESET or LAYOUT ID    */
                                       /*  cmd then                          */
      if ((com->cur_frame == KBD_RESET_CMD) ||
          (com->cur_frame == LAYOUT_ID_CMD)) {
                                       /*   flush input queue                */
         com->inq.head = com->inq.elements;
         com->inq.tail = com->inq.elements;
                                       /*   switch to receive mode           */
         com->in_progress = RCV_KBD;
         com->retry_cnt = 0;           /*   reset retry counter              */
         e_wakeup(&com->asleep);       /*   wake up any waiting process      */

      }
      else {                           /*  else                              */
         send_q_frame(com);            /*     send nxt frame to keyboard     */
      }
   }
}
