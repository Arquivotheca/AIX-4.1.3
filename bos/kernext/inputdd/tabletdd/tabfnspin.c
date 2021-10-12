static char sccsid[] = "@(#)89	1.6  src/bos/kernext/inputdd/tabletdd/tabfnspin.c, inputdd, bos41J, 9509A_all 2/14/95 13:21:06";
/*
 * COMPONENT_NAME: (INPUTDD) Mouse DD - tabfnspin.c
 *
 * FUNCTIONS: send_q_frame, watch_dog, read_port, write_port, io_error,
 *            reg_intr, ureg_intr, mem_dump, my_cdt
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

#include "tab.h"

struct  local *local=NULL;              /* local data anchor                 */

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

   struct tablet_port *port;

   KTSMDTRACE0(t_send_q_frame, enter);

   if (com->outq.out_cnt == 0) {       /* group complete so                  */
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
        local->tab_dframe = FALSE;     /* this is not a data frame           */
     }
   }

   com->cur_frame = get_oq(com);       /* get frame from queue               */
                                       /* get access to I/O bus              */
   port = (struct tablet_port *) BUSIO_ATT(com->bus_id,com->bus_addr);

                                       /* write frame to adapter data port   */
   if (write_port(&port->rw_dll, (char) (com->cur_frame >> 8))) {

                                       /* if ok indicate I/O in progress     */
                                       /*   start watch dog timer            */
      com->in_progress = TAB_IN_PROG;
      if(local->tab_dframe)  {
         local->tab_dframe = FALSE;
         startwd(com,(void*)watch_dog,TABFRAME);
      }


      else  {
         if(com->cur_frame == TAB_RESET_CMD) {
           startwd(com, (void *) watch_dog, TABRST);
      }

                                       /*  Read Tablet Configuration         */
      else if((com->cur_frame == READ_TAB_CONFIG_CMD)
             || (com->cur_frame == READ_TAB_STATUS_CMD)) {
                                       /*   flush input queue                */
            com->inq.head = com->inq.elements;
            com->inq.tail = com->inq.elements;
                                       /*   switch to receive mode           */
            com->in_progress = RCV_TAB;
            e_wakeup(&com->asleep);
          }
                                       /*  Tablet Command other than Reset   */
                                       /*  or Read Configuration             */
          else {
            startwd(com, (void *) watch_dog, TABFRAME);
            local->tab_dframe = (com->cur_frame & (OFRAME) NXT_FRAME_DATA)
                            ? TRUE:FALSE;
         }
      }
   }

   BUSIO_DET((caddr_t) port);          /* detach from I/O bus                */

   KTSMTRACE(t_send_q_frame, exit, com->cur_frame,
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
/*              control when the watch dog timer pops and calls send_q_frame.*/
/*                                                                           */
/*                                                                           */
/*****************************************************************************/


void watch_dog(struct trb *tb)
{
   struct common *com;

   com = &local->com;                  /* pointer to common area             */

   KTSMDTRACE(t_watch_dog, enter, com->cur_frame, com->in_progress,
              com->retry_cnt,tb->flags, com->outq.head);

   if ((com->in_progress == TAB_IN_PROG)) {
     send_q_frame(com);
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
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process or Interrupt                                         */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void io_error(char *rout, int perm, uint err_code, uint loc, char *fmt, ...)
{
   uchar *posreg_ptr;
   uchar posreg_val;
   int old_int;
   struct common *com;
   va_list arg;
   char ss[30];

   va_start(arg,fmt);                  /* log error                          */
   _vsprintf(ss, fmt, arg);
   ktsm_log(RES_NAME_TAB, rout, err_code, loc, ss);

   if (perm) {                         /* if hard error ...                  */
     com = &local->com;                /* pointer to common data struct      */
                                       /* disable tablet interface           */
                                       /*   by asserting adapter reset       */
     posreg_ptr = IOCC_ATT((com->bus_id | 0x80), com->slot_addr + 2);
     old_int = i_disable(INTMAX);
     posreg_val = BUSIO_GETC(posreg_ptr);
     BUSIO_PUTC(posreg_ptr,posreg_val  | RESET_TABLET);
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
/*              EIO = I/O error                                              */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       16550 reset must be asserted for at least 5us as per         */
/*              data sheet. This routine wait for 8us just to be sure.       */
/*                                                                           */
/*****************************************************************************/

int reg_intr(int adpt_rst)
{
   uchar *posreg_ptr;
   uchar posreg_val;
   char tab_rpt;
   struct common *com;
   struct tablet_port *port;
   long rc;
   int   old_int;

   com = &local->com;                  /* point to common extension          */
   if (com->perr) return(EIO);         /* exit if permanent error posted     */

                                       /* pin bottom half of driver          */
   if (rc = pincode((int (*) ())tabintr))
       return(rc);                     /* exit if error                      */

                                       /* tell kernel about slih             */
   com->intr_data.next     = 0;
   com->intr_data.handler  = (int (*) ())tabintr;
   com->intr_data.bus_type = BUS_MICRO_CHANNEL;
   com->intr_data.flags    = 0;
   com->intr_data.level    = com->intr_level;
   com->intr_data.priority = com->intr_priority;
   com->intr_data.bid      = com->bus_id;

   if (i_init(&com->intr_data) != INTR_SUCC) {

      unpincode((int (*) ())tabintr);  /* if error, unpin bottom half and    */
      return(EPERM);                   /*   return                           */
   }

   dmp_add(mem_dump);                  /* register system crash local memory */
                                       /* dump routine                       */

   flush_q(com);                       /* initialize I/O queues              */
   com->outq.error = FALSE;            /* no errors yet                      */

                                       /* get access to I/O bus              */
   port = (struct tablet_port *) BUSIO_ATT(com->bus_id,com->bus_addr);
   rc = EIO;

   if (adpt_rst) {       
                                       /* reset adapter                      */
     posreg_ptr = IOCC_ATT((com->bus_id | 0x80), com->slot_addr + 2);
     old_int = i_disable(INTMAX);
     posreg_val = BUSIO_GETC(posreg_ptr);
     BUSIO_PUTC(posreg_ptr,posreg_val  | RESET_TABLET);
     ktsm_sleep(com, 8);              /* wait for 8us (see NOTES)            */
     posreg_val = BUSIO_GETC(posreg_ptr);
     BUSIO_PUTC(posreg_ptr,posreg_val  & ~RESET_TABLET);
     i_enable(old_int);
     IOCC_DET(posreg_ptr);

                                       /* set 9600 baud rate                 */
     if (write_port(&port->line_ctl, (char) SET_DIVLAT_ACC ))
        if (write_port(&port->rw_dll, (char) BAUD_RATE_9600 ))

                                       /* set line paramters of the 16550 to */
                                       /* 8 Bits/Char, 1 stop bit, odd parity*/
           if (write_port(&port->line_ctl, (char) TAB_LINE_CONTROL))

                                       /* read line status reg to clear it   */
              if (read_port(&port->line_status, &tab_rpt))

                                       /* clear and enable fifo              */
                 if (write_port(&port->idd_fifo_alt, (char) 0x07))

                                       /* enable tablet interrupts           */
                    if (write_port(&port->dlm_ie,
                        (char) SET_RCV_DATA_AVAIL_INT))
                       rc = 0;
   }
   else {
                                       /* enable tablet interrupts           */
      if (write_port(&port->dlm_ie, (char) SET_RCV_DATA_AVAIL_INT))
        rc = 0;
   }

   BUSIO_DET(port);                    /* detach from I/O bus                */
   if (rc) ureg_intr();                /* remove interrupt handler if error  */
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
/*                                                                           */
/*****************************************************************************/


void ureg_intr(void)
{
   struct tablet_port *port;
   struct common *com;

   com = &local->com;                  /* point to common extension          */

   if (!com->perr) {                   /* disable interrupts unless perm err */
     port = (struct tablet_port *) BUSIO_ATT(com->bus_id,com->bus_addr);
     write_port(&port->dlm_ie, 0 );
     BUSIO_DET(port);      
   }

   i_clear(&com->intr_data);           /* undefine interrupt handler         */
   dmp_del(mem_dump);                  /* unregister local memory dump rout. */

   unpincode((int (*) ())tabintr);     /* unpin bottom half                  */

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
/* NAME:        my_cdt                                                       */
/*                                                                           */
/* FUNCTION:    define areas to be included in system dump                   */
/*                                                                           */
/*****************************************************************************/

struct cdt my_cdt =  {
                                        /* cdt_head:                         */
          {DMP_MAGIC,                   /*   magic number                    */
                                        /*   component dump name             */
          {'t', 'a', 'b', 'l', 'e', 't', 'd', 'd', ' ',
           ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
          sizeof(struct cdt)},          /*   length                          */
                                        /* cdt entry:                        */
                                        /*   name of data area               */
          {{{'l', 'o', 'c', 'a', 'l', ' ', ' ', ' '},
           sizeof(struct local),        /*   length of data area             */
           NULL,                        /*   address of area                 */
           {0},                         /*   cross memory parms              */
           }}};

