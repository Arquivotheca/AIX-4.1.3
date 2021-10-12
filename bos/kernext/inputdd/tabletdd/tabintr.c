static char sccsid[] = "@(#)90   1.2  src/bos/kernext/inputdd/tabletdd/tabintr.c, inputdd, bos411, 9428A410j 3/25/94 07:15:01";
/*
 * COMPONENT_NAME: (INPUTDD) Tablet    DD - tabintr.c
 *
 * FUNCTIONS: tabintr, tab_proc
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

#include "tab.h"

/*****************************************************************************/
/*                                                                           */
/* IDENTIFICATION:   tabintr                                                 */
/*                                                                           */
/* FUNCTION:         This module is the interrupt handler for the tablet. It */
/*                   is called by the first level interrupt handler when an  */
/*                   interrupt is asserted by one of the devices on our      */
/*                   interrupt level. This routine process those interrupts  */
/*                   associated with the tablet.                             */
/*                                                                           */
/* INPUTS:           intr_ptr = pointer to our interrupt data                */
/*                                                                           */
/* OUTPUTS:          INTR_SUCC if an tablet interrupt, INTR_FAIL otherwise   */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  tabintr(struct intr *intr_ptr)
{

   struct tablet_port *port;   
   struct common *com;
   char tab_rpt;
   uchar line_sts;
   int rc;

   KTSMDTRACE0(tabintr, enter);

   com = &local->com;                  /* pointer to common data area        */
   rc = INTR_FAIL;                     /* indicate interrupt not processed   */

                                       /* get access to I/O bus              */
   port  = (struct tablet_port *) BUSIO_ATT(com->bus_id,com->bus_addr);

                                       /* read interrupt id register         */
   if ( read_port(&port->idd_fifo_alt, &tab_rpt)) {

     if (!(tab_rpt & IS_INT)) {        /* continue only if 16550 interrupt   */
       if (local->tab.oflag) {         /* if device is open then             */
         tab_proc(port);               /* buffer/process event (6 frames)    */
       }
       else {                          /* if device is not open then         */
                                       /* put data frame in input queue      */
                                       /* when frame available,  there are   */
                                       /* no errors, and in receive mode     */
         if (read_port(&port->line_status, &line_sts))  {
           if (line_sts & IS_DATA_READY) {
             if (read_port(&port->rw_dll, &tab_rpt))  {
               if (!(line_sts &  (IS_FRAMING_ERROR | IS_PARITY_ERROR))) {
                 if ( com->in_progress == RCV_TAB) {
                   put_iq(com, tab_rpt);
                 }
               }
             }
           }
         }
       }
       i_reset(intr_ptr);              /* reset system interrupt             */
       rc = INTR_SUCC;                 /* indicate interrupt processed       */
     }
   }


   BUSIO_DET(port);                    /* release access to the I/O bus      */

   KTSMTRACE(tabintr, exit, rc, tab_rpt, com->cur_frame,
         com->in_progress, line_sts);

   return(rc);                         /* return the return code to FLIH     */
 
}


/*****************************************************************************/
/*                                                                           */
/* IDENTIFICATION:   tab_proc                                                */
/*                                                                           */
/* FUNCTION:         Buffer/process tablet event                             */
/*                                                                           */
/* INPUTS:           port = pointer to I/O port                              */
/*                                                                           */
/* OUTPUTS:          none                                                    */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/*                                                                           */
/* NOTES:            Because of the 16550 fifo and the way that the tablet   */
/*                   protocol is defined, there is no way to recover data    */
/*                   when an error occurs. The best that can be done is to   */
/*                   flush the entire status report in which an error is     */
/*                   reported, resync, and let the operator re-enter data    */
/*                                                                           */
/*****************************************************************************/

void tab_proc(struct tablet_port *port)

{

   uchar   line_sts ;
   uchar   data;

   while(1)
   {

/* assemble a valid 6 frame status report from the tablet                    */

      while((local->tab_bytes_read < 6) || local->flush_err) 
      {
                                       /* return and wait for next intr if   */
                                       /* fifo is empty                      */
         if(!(read_port(&port->line_status, &line_sts )))  {
            return;
         }
         if (( line_sts & IS_DATA_READY ) == 0x0 )
            return;
                                       /* read data from fifo                */
         if(!(read_port(&port->rw_dll, &data)))  {
            return;
         }
         
         KTSMTRACE(tabproc, rcv,  data, line_sts,
              local->flush_err, local->tab_bytes_read, 0);

                                       /* check for errors                   */
                                       /* ..don't care about overrun error as*/
                                       /*   frame sequence check below should*/
                                       /*   detect the problem plus we       */
                                       /*   don't want to flush any good     */
                                       /*   complete reports in the fifo     */
                                       /*   because of an overrun deeper in  */
                                       /*   the fifo                         */
         if (line_sts &  (IS_FRAMING_ERROR | IS_PARITY_ERROR)) {
                                       /* error detected, put in flush mode  */
            local->flush_err = TRUE;
         }
         else {                        /* frame is good...                   */
                                       /* if in flush mode trying to re-sync */
                                       /* look for frame with bit 7 (LSB)    */
                                       /* equal to 1 as this marks the first */
                                       /* frame of a tablet status report    */

            if (local->flush_err) {    /* in flush mode, looking for frame 1 */
               if (data & 1) {
                                       /* found it, exit flush mode          */
                  local->flush_err = FALSE;
                                       /* set index to first position        */
                  local->tab_bytes_read = 0;
               }
            }
            else {                     /* frame good and not in flush mode   */
                                       /* verify frame sequence (ie: bit 7   */
                                       /* (LSB) should be zero in all but    */
                                       /* first frame)                       */

                                       /* if not the first frame             */
               if (local->tab_bytes_read) {   
                  if (data & 1) local->flush_err = TRUE;
               }                       
                                       /*  else if first frame               */
               else {
                  if (!(data & 1)) local->flush_err = TRUE;
               }
            }                          /* if not in flush mode, buffer data  */
            if (!local->flush_err)  {
               local->tab.tab_block_data[local->tab_bytes_read++] = data;
            }
         }
      }

      local->tab_bytes_read = 0;       /* reset tablet bytes read counter    */
      tablet_proc(&local->tab);        /* process event                      */
   }
}
