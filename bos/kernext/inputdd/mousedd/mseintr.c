static char sccsid[] = "@(#)96   1.2  src/bos/kernext/inputdd/mousedd/mseintr.c, inputdd, bos411, 9428A410j 4/16/94 14:03:45";
/*
 * COMPONENT_NAME: (INPUTDD) Mouse DD - mseintr.c
 *
 * FUNCTIONS: mseintr, mouse_proc, mouse_frame, mouse_err
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

#define ABS(x) ((x<0) ? -(x) : (x))

#define MOUSE_RPT local->mouse_rpt


/*****************************************************************************/
/*                                                                           */
/* NAME:             mseintr                                                 */
/*                                                                           */
/* FUNCTION:         This module is the interrupt handler for the mouse,     */
/*                   is called by the First Level Interrupt Handle when an   */
/*                   interrupt is detected from one of the devices on our    */
/*                   interrupt level.  This module determines if the mouse   */
/*                   caused the interrupt.                                   */
/*                                                                           */
/* INPUTS:           intr_ptr = pointer to our interrupt data                */
/*                                                                           */
/* OUTPUTS:          INTR_SUCC if an mouse interrupt, INTR_FAIL otherwise    */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  mseintr(struct intr *intr_ptr)
{
   struct common *com;
   struct mouse_port  *port;
   int rc;

   KTSMDTRACE0(mseintr, enter);

   com = &local->com;                  /* pointer to common data area        */
   rc = INTR_FAIL;                         
                                       /* get access to I/O bus              */
   port = (struct mouse_port *) BUSIO_ATT(com->bus_id,com->bus_addr);

                                       /* get mouse report                   */
   if(read_port((ulong *) &port->r_mdata_regs, &MOUSE_RPT)) {
     if (MOUSE_RPT & M_INTERRUPT) {    /* if interrupt is from mouse         */
                                       /*   if valid 3 frame response then   */
                                       /*     process mouse report if        */
                                       /*     special file is opened         */
       if (!(MOUSE_RPT & M_FACE_ERR) &&
           !(MOUSE_RPT & M_DATA_ERR) &&
            (MOUSE_RPT & REG_1_FULL) &&
            (MOUSE_RPT & REG_2_FULL) &&
            (MOUSE_RPT & REG_3_FULL)) {
         if (local->oflag) {
           mouse_proc();
         }
       }
       else {                          /*   if valid 1 frame response then   */
                                       /*     process mouse frame            */
         if (!(MOUSE_RPT & M_FACE_ERR) &&
             !(MOUSE_RPT & M_DATA_ERR) &&
              (MOUSE_RPT & REG_1_FULL) &&
             !(MOUSE_RPT & REG_2_FULL) &&
             !(MOUSE_RPT & REG_3_FULL)) {
           mouse_frame(com,port);
         }
         else {                        /*   if none of the above then        */
                                       /*     process error                  */
           mouse_err(com,port);
         }
       }

       i_reset(intr_ptr);              /*   reset system interrupt           */
       rc = INTR_SUCC;                 /*   indicate this was mouse interrupt*/
     }
   }

   BUSIO_DET(port);                    /* release access to the I/O bus      */

   KTSMTRACE(mseintr, exit, rc, MOUSE_RPT, com->cur_frame,
         com->in_progress, 0);

   return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             mouse_proc                                              */
/*                                                                           */
/* FUNCTION:         Process mouse report                                    */
/*                                                                           */
/* INPUTS:           none                                                    */
/*                                                                           */
/* OUTPUTS:          none                                                    */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void mouse_proc(void)
{
   struct ir_mouse event;
   uchar buttons;


   buttons = 0;                        /* zero out button status             */

                                       /* dump event if overflow             */
   if (!(MOUSE_RPT & M_X_DATA_OVERFLOW) &&
       !(MOUSE_RPT & M_Y_DATA_OVERFLOW)) {

                                       /* check if left button is down       */
      if (MOUSE_RPT & M_L_BUTTON_STATUS)
         buttons |= MOUSEBUTTON1;

                                       /* check if center button is down     */
      if (MOUSE_RPT & M_C_BUTTON_STATUS)
         buttons |= MOUSEBUTTON2;

                                       /* check if right button is down      */
      if (MOUSE_RPT & M_R_BUTTON_STATUS)
         buttons |= MOUSEBUTTON3;

                                       /* update x accumulations             */
      if (MOUSE_RPT & M_X_DATA_SIGN)
         local->mouse_hor_accum -= (short)((~(MOUSE_RPT >> 8)+1) & 0xff);
      else
         local->mouse_hor_accum += (short)((MOUSE_RPT >> 8) & 0xff);

                                       /* update y accumulations             */
      if (MOUSE_RPT & M_Y_DATA_SIGN)
         local->mouse_ver_accum -= (short)((~MOUSE_RPT+1) & 0xff);
      else
         local->mouse_ver_accum += (short)(MOUSE_RPT & 0xff);

      
      KTSMDTRACE(mseproc, event, buttons, local->mouse_hor_accum,
           local->mouse_ver_accum, local->mouse_hor_thresh,
           local->mouse_ver_thresh);

                                       /* if accumulations exceed thresholds */
                                       /* or there is change in button status*/
                                       /* then                               */
      if ((ABS(local->mouse_hor_accum) >=  local->mouse_hor_thresh) ||
         (ABS(local->mouse_ver_accum) >=  local->mouse_ver_thresh) ||
         (local->button_status != buttons))  {
                                       /* put event on input ring            */
         event.mouse_header.report_size = sizeof(event);
         event.mouse_deltax = local->mouse_hor_accum;
         event.mouse_deltay = local->mouse_ver_accum;
         event.mouse_status = buttons;
         ktsm_putring(&local->rcb, (struct ir_report *) &event, NULL);

                                       /* update saved status                */
         local->button_status = buttons;
         local->mouse_hor_accum   = 0;
         local->mouse_ver_accum   = 0;
      }
   }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             mouse_frame                                             */
/*                                                                           */
/* FUNCTION:         Process one frame response                              */
/*                                                                           */
/* INPUTS:           com = pointer to commone area                           */
/*                   port = pointer to attached I/O ports                    */
/*                                                                           */
/* OUTPUTS:          N/A                                                     */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:            RESEND response from mouse is ignored. If we are sending*/
/*                   frame, watch dog timer will pop and frame will be       */
/*                   resent.                                                 */
/*                                                                           */
/*****************************************************************************/

void mouse_frame(struct common *com, struct mouse_port  *port)
{

                                       /* enqueue frame to input queue if    */
                                       /* in receive mode                    */
   if (com->in_progress & RCV_MSE) {
     put_iq(com, (IFRAME) ((MOUSE_RPT & M_DATA_1) >> 16));
   }
   else {                              /* if expecting "ACK"                 */
     if (com->in_progress & MSE_ACK_RQD) {
                                       /* and "ACK" received then            */
       if ((MOUSE_RPT & M_DATA_1) == M_ACK) {
         tstop(com->wdtimer);          /* stop watch dog timer               */
                                       /*  if ACK from RESET or READ STATUS  */
                                       /*  cmd then                          */
         if ((com->cur_frame == M_RESET) ||
             (com->cur_frame == M_STATUS_REQ)) {
                                       /*   flush input queue                */
           com->inq.head = com->inq.elements;
           com->inq.tail = com->inq.elements;
                                       /*   switch to receive mode           */
           com->in_progress = RCV_MSE;
           com->retry_cnt = 0;         /*   reset retry counter              */
           e_wakeup(&com->asleep);     /*    wake up any waiting process     */
         }
         else {                        /*  else                              */
             send_q_frame(com);        /*     send nxt frame to mouse        */
         }
       }
       else {                          /* if resend received instead of ack  */
                                       /* then indicate bad frame rcv'ed     */
                                       /* (see note - watch dog will resend) */
         if (((MOUSE_RPT & M_DATA_1) == M_INV_CMD_RC) ||
             ((MOUSE_RPT & M_DATA_1) == M_INV_CMDS_RC)) {
           com->in_progress |= OP_RCV_ERR;  
         }
       }
     }
   }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             mouse_err                                               */
/*                                                                           */
/* FUNCTION:         Process interface errors                                */
/*                                                                           */
/* INPUTS:           com = pointer to commone area                           */
/*                   port = pointer to attached I/O ports                    */
/*                                                                           */
/* OUTPUTS:          N/A                                                     */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:            Errors that occur while sending frames are ignored.     */
/*                   Recovery is performed when watch dog timer pop's        */
/*                                                                           */
/*                   If retries have been exhausted, code just exits and     */
/*                   lets watch dog timer routine or wait_iq() clean up      */
/*                                                                           */
/*                   An assumption is made that if mouse is not sending      */
/*                   commands or rcv'ing their responses then the adapter    */
/*                   is blocked                                              */
/*                                                                           */
/*****************************************************************************/

void mouse_err(struct common *com, struct mouse_port  *port)
{

   int   count;
   char data;
   long ldata;

                                       /* if receiving frames from mouse     */
                                       /*   send RESEND to mouse if retries  */
                                       /*   have not been exhausted          */
                                       /*   NOTE: flush input queue as mouse */
                                       /*     resends entire response which  */
                                       /*     may consist of multiple frames */
   if (com->in_progress & RCV_MSE) {
     com->in_progress |= OP_RCV_ERR;   /*    <-- indicate bad frame rcv'ed   */
     if (com->retry_cnt < M_RETRY) {
       com->retry_cnt++;
       com->inq.head = com->inq.elements;
       com->inq.tail = com->inq.elements;
       write_port(&port->w_data_cmd, (short) M_RESEND);
     }
   }
   else {                              /* if sending frames to mouse let     */
                                       /*   watch dog timer handle error but */
                                       /*   change timer value if RESET cmd  */
                                       /*   sent just in case mse executed   */
                                       /*   the RESET cmd                    */
     if (com->in_progress & MSE_ACK_RQD) {
       com->in_progress |= OP_RCV_ERR; /*    <-- indicate bad frame rcv'ed   */
       if (com->cur_frame == M_RESET) {
         startwd(com, (void *) watch_dog, RSTWD);
       }
     }
     else {
                                      /* if interface error then most likely */
                                      /* mouse has been unplugged but will   */
                                      /* attempt to recover anyway ...       */
       if (MOUSE_RPT & M_FACE_ERR) {
                                      /* but first log error                 */
         io_error("mouse_err", FALSE, RCV_ERROR, 1, "%08x", MOUSE_RPT);


      /* when an interface error occurs while in block mode, the mode must   */
      /* disabled and re-enabled. The follow code is an "abbreviated" method */
      /* for doing this. No attempt is made to recover the mouse status      */
      /* report that caused the error. PIO errors are handled by retry       */
      /* loop.                                                               */

                                      /* retry loop                          */
         for (count=0;count<M_RETRY;count++) {
                                      /* send adapter disable block mode cmd */
           BUS_PUTSX(&port->w_data_cmd, (short) DIS_BLK_MODE);
                                      /* get adapter status                  */
           if (!BUS_GETCX(&port->r_astat_reg, &data)) {
                                      /* if block mode is disabled, break out*/
             if ((data & BLOCK_DISABLE_MASK) == CMD_RCVD_BLK_DIS) break;
                                      /* clk line held low so adapter will   */
                                      /* not be rcv'ing data (busy) and thus */
                                      /* should immediately go into non-block*/
                                      /* mode. Anyway can not wait 1.2 ms in */
                                      /* intr handler so if not out of block */
                                      /* mode, retry immediately             */
           }
         }

      /* fell out of loop...assume that block mode is disabled. If not, the  */
      /* rest of this code will not be effective but will not be fatal either*/

                                      /* flush adapter's fifo's              */
         read_port(&port->r_mdata_regs, &ldata);
         read_port(&port->r_mdata_regs, &ldata);
         read_port(&port->r_mdata_regs, &ldata);
         read_port(&port->r_mdata_regs, &ldata);

                                      /* retry loop                          */
         for (count=0;count<M_RETRY;count++) {
                                      /* send adapter enable block mode cmd  */
           BUS_PUTSX(&port->w_data_cmd, (short) EN_BLK_MODE);
                                      /* read mouse adapter status regs      */
           if (!BUS_GETCX(&port->r_astat_reg, &data)) {
                                      /* if in block mode, break out         */
             if (data & BLK_MODE_EN) break;
           }
         }

      /* fell out of loop...assume that adapter is in block mode. If         */
      /* not, "mkdev" cmd will have to used to reconfig mouse                */

                                      /* send "ENABLE" cmd to mouse. Adapter */
                                      /* is looking for this command and will*/
                                      /* not realy go into block mode until  */
                                      /* it is sent.                         */
                                      /* NOTE: there is no recovery for this */
                                      /*   command if it gets garbled        */

         write_port(&port->w_data_cmd, (short) M_ENABLE);
       }
       else {                          /* else treat all other conditions as */
                                       /* data error ...                     */

                                       /*   send RESEND cmd so mouse         */
                                       /*   will re-xmit report              */
         write_port(&port->w_data_cmd, (short) M_RESEND);
                                       /*   bump error counter               */
         if ((com->rcv_err_cnt++) > RCV_ERR_THRESHOLD) {
                                       /*   if over threshold, log error     */
           io_error("mouse_err", FALSE, RCV_ERROR, 2, "%08x", MOUSE_RPT);
           com->rcv_err_cnt = 0;       /*   and reset counter                */
         }
       }
     }
   }
}
