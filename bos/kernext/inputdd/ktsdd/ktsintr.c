static char sccsid[] = "@(#)81   1.3  src/bos/kernext/inputdd/ktsdd/ktsintr.c, inputdd, bos411, 9428A410j 6/16/94 09:07:47";
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard/Tablet/Sound DD - ktsintr.c
 *
 * FUNCTIONS: ktsintr, ktsirq_proc, info_int, ktsack, tabdone
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

#include "kts.h"

/*****************************************************************************/
/*                                                                           */
/* NAME:             ktsintr                                                 */
/*                                                                           */
/* FUNCTION:         Process interrupts from 8051 keyboard/tablet adapter    */
/*                                                                           */
/* INPUTS:           intr_ptr = pointer to our interrupt data                */
/*                                                                           */
/* OUTPUTS:          INTR_SUCC if an 8051 interrupt, INTR_FAIL otherwise     */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  ktsintr(struct intr *intr_ptr)
{
   struct common *com;
   struct  kts_port *port;
   int     rc;

   KTSMDTRACE0(ktsintr, enter);

   com = &local->com;                  /* pointer to common data area        */
   rc = INTR_FAIL;
                                       /* get access to I/O bus              */
   port = (struct kts_port *) BUSIO_ATT(com->bus_id, com->bus_addr);
                                       /* read adapter status port           */
   if (read_port(&port->r_port_c, &local->status)) {
     if (local->status & IBF) {        /* if intr from adapter then          */
                                       /* read 8051 data                     */
       if (read_port(&port->r_port_a, &local->data)) {

                                       /* if data read a success then        */
                                       /* process interrupt type             */
         switch (local->status & INT_TYPE) {

           case INFOBYTE:              /* case: informational interrupt      */
             info_int();
             break;

           case KYBDBYTE:              /* case: data ready from kbd          */

                                       /*   if in receive mode then enqueue  */
                                       /*   frame to input queue             */
             if ( com->in_progress == RCV_KBD) {
               put_iq(com, local->data);
             }
             else {                    /*   else go process scan code        */
                                       /*   if at least one channel is open  */
               if (local->key.act_ch != NO_ACT_CH) {
                 keyproc(com, &local->key, local->data);
               }
             }
             break;
          
           case BLK_XFER:              /* case: ready for uart block transfer*/
                                       /*   error if invalid block size      */
             if (local->data != TAB_REPORT_SIZE) {
               io_error("ktsintr", FALSE, ADPT_ERROR, 0, "%02x %02x",
                  local->status, local->data);
             }
             else {                    /*   block active                     */
               local->blk_act = TRUE;
               local->blk_cnt = 0;
             }
             break;

           case UARTBYTE:              /* case: data ready from uart         */
                                       /*   if in receive mode then put data */
                                       /*   on input queue                   */
             if (com->in_progress == RCV_TAB) { 
               put_iq(com, local->data);       
             }
             else {                    /*   else queue data if block active  */
               if (local->blk_act) {
                 local->tab.tab_block_data[local->blk_cnt++] = local->data;
                                       /*   process data after entire        */
                                       /*   block has been received          */
                                       /*   (but only if channel is open)    */
                 if (local->blk_cnt == TAB_REPORT_SIZE) {
                   local->blk_act = FALSE;
                   if (local->tab.oflag) {
                     tablet_proc(&local->tab);
                   }
                 }
               }
             }
             break;

           case COMPCODE:              /* case: 8051 reset compl code        */

                                       /* if expecting BAT from adapter then */
             if (com->in_progress == ADPT_BAT) {
                                       /* if valid BAT then                  */
               if (local->data == GOODMACHINE) {
                                       /*    flag operation as complete      */
                 com->in_progress = NO_OP;
               }
               else {
                 com->in_progress |= FAILED_OP;
               }
                                       /*   wake up any waiting process      */
               e_wakeup(&com->asleep);
               break;
             } 
                                       /* not expecting COMPCODE so fall     */
                                       /* thru to next case to log error     */

           case ERR_CODE:              /* case: error detected by 8051       */
           case REQ_BYTE:              /* case: requested info ready         */
           default:                    /* default: invalid intr code         */
#ifdef GS_DEBUG_TRACE
                                       /*   log error                        */
             io_error("ktsintr", FALSE, ADPT_ERROR, 1, "%02x %02x %04x %04x",
                  local->status, local->data, com->cur_frame,
                  com->in_progress)
#endif
             ;
         }
       }
       i_reset(intr_ptr);              /*   reset system interrupt           */
       rc = INTR_SUCC;                 /*   indicate intr from kbd/tablet    */
     }
   }

   BUSIO_DET((caddr_t) port);          /* release access to the I/O bus      */

   KTSMTRACE(ktsintr, exit, rc, local->status, local->data,
      com->in_progress, local->key.act_ch);

   return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             info_int                                                */
/*                                                                           */
/* FUNCTION:         This module processes informational interrupts from     */
/*                   the 8051 adapter.                                       */
/*                                                                           */
/* INPUTS:           none                                                    */
/*                                                                           */
/* OUTPUTS:          none                                                    */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:            current design does not look at 8051 RAS log            */
/*                                                                           */
/*                   reject code 0x41 has been observed when Reset sent      */
/*                   to keyboard when:                                       */
/*                      1) tablet connected, configured, but not openned     */
/*                         and tablet device is sending data.                */
/*                      2) keyboard has just been plugged in                 */
/*                      3) keyboard is being configured                      */
/*                                                                           */
/*                   adapter seems to get confused with all the tablet       */
/*                   data and the BAT from the keyboard                      */
/*                                                                           */
/*****************************************************************************/

void  info_int(void)
{
   struct common *com;

   com = &local->com;                  /* pointer to common data area        */
                                       /* process unsolicited status ...     */
   if (local->data & STATUS_RPT_ID_MASK) {
                                       /*  speaker tone completed            */
     if (local->data & SPKR_TONE_COMP_MASK) {
       next_sound();
     }
                                       /*  keyboard acknowledge              */
     if (local->data & KBD_RET_ACK_MASK) {
       ktsack();
     }
                                       /* check for and log invalid status   */
     if (local->data & ~(STATUS_RPT_ID_MASK |
          SPKR_TONE_COMP_MASK | KBD_RET_ACK_MASK |
          RAS_LOG_NEAR_OFLO_MASK | RAS_LOG_OFLO_MASK)) {
       io_error("info_int", FALSE, ADPT_ERROR, 0, "%02x %02x %04x %04x",
         local->status, local->data, com->cur_frame, com->in_progress);
     }
   }
   else {
                                       /* process solicited status           */
     switch (local->data) {

       case SPKR_STARTED:              /*  speaker started                   */
       case SPKR_INACTIVE:             /*  speaker is inactive               */
       case SPKR_TERMINATED:           /*  speaker is terminated             */
       case SPKR_PARMS_QUEUED:         /*  spkeaker parms queued             */
       case HOST_CMD_ACK:              /*  adapter acknowledge               */
                                       /*    if expecting adapter ACK then   */
         if (com->in_progress == ADPT_ACK_RQD) {
           tstop(com->wdtimer);        /*    stop watch dog timer            */
                                       /*    if xmited kbd cmd then wait for */
                                       /*    ACK from keyboard               */
           if ((com->cur_frame & CMD_8051) == WRITE_KBD_CMD) {
             com->in_progress = KBD_ACK_RQD;
                                       /*    start watchdog timer            */
             startwd(com, (void *) watch_dog, WDOGTIME);

           }
           else {                      /*    process ack from tablet frame   */
             if ((com->cur_frame & CMD_8051) == WRITE_TAB_CMD) {
               tabdone();
             }
             else {                    /*    else send nxt frame to adapter  */
               send_q_frame(com);
             }
           }
         }

         break;

       case  REJ_KBD_TRANS_BUSY:       /* keyboard xmit busy (see note)      */
                                       /*     set flag so watchdog routine   */
                                       /*     will retry                     */
         if (com->in_progress == ADPT_ACK_RQD) {
            com->in_progress |= FAILED_OP;
            break;
         }

       default:
                                       /*    log invalid status received     */
         io_error("info_int", FALSE, ADPT_ERROR, 1, "%02x %02x %04x %04x",
            local->status, local->data, com->cur_frame, com->in_progress);

         break;
     }
   }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             ktsack                                                  */
/*                                                                           */
/* FUNCTION:         Process ACK response from keyboard                      */
/*                                                                           */
/* INPUTS:           none                                                    */
/*                                                                           */
/* OUTPUTS:          none                                                    */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:            KBD_RESET and LAYOUT_ID_CMD must be last command in     */
/*                   group                                                   */
/*                                                                           */
/*****************************************************************************/

void ktsack(void)
{
   struct common *com;

   com = &local->com;                  /* pointer to common data area        */
                                       /* if expecting keyboard ACK then     */
   if (com->in_progress == KBD_ACK_RQD) {
     tstop(com->wdtimer);              /*  stop watch dog timer              */
                                       /*  if ACK from RESET or LAYOUT ID    */
                                       /*  cmd then                          */
     if ((com->cur_frame == KBD_RESET_CMD) ||
          (com->cur_frame == LAYOUT_ID_CMD)) {
                                       /*   flush input queue                */
       com->inq.head = com->inq.elements;
       com->inq.tail = com->inq.elements;
                                       /*   switch to receive mode           */
       com->in_progress = RCV_KBD;
       e_wakeup(&com->asleep);         /*   wake up any waiting process      */
     }

     else {                            /*  else                              */
       send_q_frame(com);              /*   send nxt frame                   */
     }
   }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             tabdone                                                 */
/*                                                                           */
/* FUNCTION:         Process adapter ack resulting from writing tablet frame */
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

void tabdone(void)
{
   struct common *com;

   com = &local->com;                  /* pointer to common data area        */
     
                                       /* indicate waiting for tablet        */
   com->in_progress = TAB_IN_PROG;     /* frame completion                   */

                                       /* if data frame then clear flag and  */
   if (local->tab_dframe) {            /* start timer for standard delay     */
      local->tab_dframe = FALSE;
      startwd(com, (void *) watch_dog, TABFRAME);
   }
   else {                              /* if cmd frame, see if cmd solicites */
                                       /* a response                         */
     if ((com->cur_frame == READ_TAB_CONFIG_CMD) ||
         (com->cur_frame == READ_TAB_STATUS_CMD)) {
                                       /* it does so ..                      */
                                       /*   flush input queue                */
       com->inq.head = com->inq.elements;
       com->inq.tail = com->inq.elements;
                                       /*   switch to receive mode           */
       com->in_progress = RCV_TAB;
       e_wakeup(&com->asleep);         /*   wake up any waiting process      */
     }
     else {                            /* it does not so ...                 */
                                       /*   ck for RESET cmd as it requires  */
                                       /*   a longer delay                   */
       if (com->cur_frame == TAB_RESET_CMD) {
                                       /*   RESET cmd so set reset delay     */
         startwd(com, (void *) watch_dog, TABRST);
       }
       else {                          /*   use standard delay               */
         startwd(com, (void *) watch_dog, TABFRAME);
                                       /*   if high order bit of cmd is one  */
                                       /*   then next frame is data frame    */
         local->tab_dframe = (com->cur_frame & (OFRAME) NXT_FRAME_DATA) ?
               TRUE : FALSE;
       }
     }
   }
}
