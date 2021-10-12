static char sccsid[] = "@(#)95	1.4  src/bos/kernext/inputdd/mousedd/msefnspin.c, inputdd, bos41J, 9509A_all 2/14/95 13:20:38";
/*
 * COMPONENT_NAME: (INPUTDD) Mouse DD - msefnspin.c
 *
 * FUNCTIONS: send_q_frame, watch_dog, read_port, read_char, write_port,
 *            io_error, reg_intr, ureg_intr, enable_intr, disable_intr,
 *            mem_dump, rst_mse_adpt
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

#include "mse.h"

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

   struct mouse_port *port;

   KTSMDTRACE0(m_send_q_frame, enter);

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
        com->retry_cnt = 0;            /*     clear retry counter            */
     }
   }

   com->cur_frame = get_oq(com);       /* get frame from queue               */
                                       /* get access to I/O bus              */
   port = (struct mouse_port *) BUSIO_ATT(com->bus_id,com->bus_addr);
                                       /* write frame to adapter data port   */
   if (write_port(&port->w_data_cmd, (short) com->cur_frame)) {
                                       /* if ok                              */
      com->in_progress = MSE_ACK_RQD;  /*   indicate I/O in progress         */
                                       /*   start watch dog timer            */
      startwd(com, (void *) watch_dog, WDOGTIME);
   }

   BUSIO_DET((caddr_t) port);          /* detach from I/O bus                */

   KTSMTRACE(m_send_q_frame, exit, com->cur_frame,
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

   KTSMTRACE(m_watch_dog, enter, com->cur_frame, com->in_progress,
       com->retry_cnt, tb->flags, com->outq.head);

                                       /* if expecting ACK then ...          */
   if (com->in_progress & MSE_ACK_RQD) {

                                       /* if retry count is not exceeded     */
      if (com->retry_cnt < M_RETRY) {
        com->retry_cnt++;              /*   update retry count               */
        com->outq.dq_ptr =             /*   reset ptr back to start of       */
           com->outq.head;             /*     transmission group             */
                                       /*   get number of frames in group    */
        com->outq.out_cnt = get_oq(com);
        send_q_frame(com);             /*   resend first frame in group      */
      }
      else {                           /* else retries are exhausted so      */
                                       /*   log error                        */
#ifndef GS_DEBUG_TRACE
        if (com->cur_frame != M_RESET)
#endif
          io_error("watch_dog", FALSE, MIO_ERROR, 0, "%08x %04x %04x",
            local->mouse_rpt,  com->cur_frame, com->in_progress);
        flush_q(com);                  /* flush queues to wake up top half   */
      }
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

int read_port(long *addr, long *data)
{
   int retry;

   for (retry = 0; retry < PIO_RETRY_COUNT; retry++)  {
       if (!BUS_GETLX(addr, data)) return(TRUE);
   }
                                       /* unrecoverable PIO error            */
   io_error("read_port", TRUE, PIO_ERROR, 0, "%x", addr);
   return(FALSE);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        read_char                                                    */
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

int read_char(char *addr, char *data)
{
   int retry;

   for (retry = 0; retry < PIO_RETRY_COUNT; retry++)  {
       if (!BUS_GETCX(addr, data)) return(TRUE);
   }
                                       /* unrecoverable PIO error            */
   io_error("read_char", TRUE, PIO_ERROR, 0, "%x", addr);
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

int write_port(short *addr, short data)
{
   int retry;

   for (retry = 0; retry < PIO_RETRY_COUNT; retry++)  {
       if (!BUS_PUTSX(addr, data)) return(TRUE);
   }
                                       /* unrecoverable PIO error            */
   io_error("write_port", TRUE, PIO_ERROR, 0, "%x %04x", addr, data);
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
/*     A major hardware error has occurred if this routine has been          */
/*     called, so what we would like to do is to assert adapter reset        */
/*     (so that the adapter can not cause an interrupt), log an error, and   */
/*     bail out.  The adapter can be reset via the POS regs which            */
/*     means that we don't have to worry about bus exceptions.               */
/*     HOWEVER, on some machines, the interrupt line gliches or floats       */
/*     high when reset is asserted (see CMVC defect 71045). Therefore this   */
/*     routine will toggle the reset line instead of leaving it asserted     */
/*     and hopefully mouse interrupts will be disabled following the reset   */
/*     (as per hardware spec).                                               */
/*                                                                           */
/*****************************************************************************/

void io_error(char *rout, int perm, uint err_code, uint loc, char *fmt, ...)
{
   va_list arg;
   char ss[30];

   va_start(arg,fmt);                  /* log error                          */
   _vsprintf(ss, fmt, arg);
   ktsm_log(RES_NAME_MSE, rout, err_code, loc, ss);

   if(perm) {                          /* if hard error then                 */
     rst_mse_adpt();                   /* reset mouse adapter                */
     flush_q(&local->com);             /* reset I/O queues                   */
     local->com.perr = TRUE;           /* remember we're hosed               */
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
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int reg_intr(int adpt_rst)
{
   struct common *com;
   struct mouse_port *port;
   long rc;

   com = &local->com;                  /* point to common extension          */
   if (com->perr) return(EIO);         /* exit if permanent error posted     */

                                       /* pin bottom half of driver          */
   if (rc = pincode((int (*) ())mseintr))
       return(rc);                     /* exit if error                      */

   flush_q(com);                       /* initialize I/O queues              */
   com->outq.error = FALSE;            /* no errors yet                      */
   local->block_mode = FALSE;          /* not in block mode                  */

   rst_mse_adpt();                     /* reset mouse adapter                */

                                       /* tell kernel about slih             */
   com->intr_data.next     = 0;
   com->intr_data.handler  = (int (*) ())mseintr;
   com->intr_data.bus_type = BUS_MICRO_CHANNEL;
   com->intr_data.flags    = 0;
   com->intr_data.level    = com->intr_level;
   com->intr_data.priority = com->intr_priority;
   com->intr_data.bid      = com->bus_id;

   if (i_init(&com->intr_data) != INTR_SUCC) {
      unpincode((int (*) ())mseintr);  /* if error, unpin bottom half and    */
      return(EPERM);                   /*   return                           */
   }

   dmp_add(mem_dump);                  /* register system crash local memory */
                                       /* dump routine                       */

                                       /* get access to I/O bus              */
   port = (struct mouse_port *) BUSIO_ATT(com->bus_id,com->bus_addr);
   if (rc = enable_intr(port)) {       /* enable interrutps                  */
      ureg_intr();                     /* failed so unregister intr handler  */
   }
   BUSIO_DET(port);                    /* detatch from I/O bus               */

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
/*     On some machines, the interrupt line gliches or floats high when      */
/*     reset is asserted (see CMVC defect 71045). Therefore this routine     */
/*     will toggle the reset line instead of leaving it asserted and         */
/*     hopefully mouse interrupts will be disabled following the reset       */
/*     as per hardware spec.                                                 */
/*                                                                           */
/*****************************************************************************/

void ureg_intr(void)
{

   rst_mse_adpt();                     /* reset mouse adapter                */
   i_clear(&local->com.intr_data);     /* undefine interrupt handler         */
   dmp_del(mem_dump);                  /* unregister local memory dump rout. */
   unpincode((int (*) ())mseintr);     /* unpin bottom half                  */

}

/*****************************************************************************/
/*                                                                           */
/* NAME:        enable_intr                                                  */
/*                                                                           */
/* FUNCTION:    enable interrutps                                            */
/*                                                                           */
/* INPUTS:      port = pointer to adapter registers                          */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*              EIO = I/O error                                              */
/*                                                                           */
/* ENVIRONMENT: process or interrupt                                         */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int enable_intr(struct mouse_port *port)
{
   int count;
   char data;

   for (count=0;count<M_RETRY;count++) {
                                       /*   send enable interrupt cmd to     */
                                       /*   mouse adapter                    */
      if (!write_port(&port->w_data_cmd, (short) M_ENABLE_INT)) break;
                                       /*   get adapter status               */
      if (!read_char(&port->r_astat_reg, &data)) break;
      if (data & INTERRUPT_EN_MASK) {  /*   if interrupts are enabled        */
         return(0);                    /*   then return ok                   */
      }
   }
   if (count == M_RETRY) {             /* log error if not PIO failure       */
     io_error("enable_intr", TRUE,  MIO_ERROR, 0, "%02x", data);
   }
   return(EIO);
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        disable_intr                                                 */
/*                                                                           */
/* FUNCTION:    disable interrutps                                           */
/*                                                                           */
/* INPUTS:      port = pointer to adapter registers                          */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*              EIO = I/O error                                              */
/*                                                                           */
/* ENVIRONMENT: process or interrupt                                         */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int disable_intr(struct mouse_port *port)
{
   int count;
   char data;

   for (count=0;count<M_RETRY;count++) {
                                       /*   send enable interrupt cmd to     */
                                       /*   mouse adapter                    */
      if (!write_port(&port->w_data_cmd, (short) M_DISABLE_INT)) break;
                                       /*   get adapter status               */
      if (!read_char(&port->r_astat_reg, &data)) break;
                                       /*   if interrupts are disabled       */
      if (!(data & INTERRUPT_EN_MASK)) {
         return(0);                    /*   then return ok                   */
      }
   }
   if (count == M_RETRY) {             /* log error if not PIO failure       */
     io_error("disable_intr", TRUE, MIO_ERROR, 0, "%02x", data);
   }
   return(EIO);
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
/* NAME:        rst_mse_adpt                                                 */
/*                                                                           */
/* FUNCTION:    reset mouse adapter                                          */
/*                                                                           */
/* INPUTS:      none                                                         */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt or process                                         */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void rst_mse_adpt(void)
{
   uchar *posreg_ptr;
   uchar posreg_val;
   int old_int;

   posreg_ptr = IOCC_ATT((local->com.bus_id | 0x80), local->com.slot_addr + 2);
   old_int = i_disable(INTMAX);
   posreg_val = BUSIO_GETC(posreg_ptr);
   BUSIO_PUTC(posreg_ptr,posreg_val  | RESET_MOUSE);
   BUSIO_PUTC(posreg_ptr,posreg_val  & ~RESET_MOUSE);
   i_enable(old_int);
   IOCC_DET(posreg_ptr);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        mse_button_cmds                                              */
/*                                                                           */
/* FUNCTION:    commands to read number of buttons on mouse                  */
/*                                                                           */
/*****************************************************************************/

OFRAME mse_button_cmds[] = {
                             6,      /* frame count                          */
                             M_SET_RES,
                             M_RES_1,
                             M_RESET_SCALE,
                             M_RESET_SCALE,
                             M_RESET_SCALE,
                             M_STATUS_REQ
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
          {'m', 'o', 'u', 's', 'e', 'd', 'd', ' ',
           ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
          sizeof(struct cdt)},          /*   length                          */
                                        /* cdt entry:                        */
                                        /*   name of data area               */
          {{{'l', 'o', 'c', 'a', 'l', ' ', ' ', ' '},
           sizeof(struct local),        /*   length of data area             */
           NULL,                        /*   address of area                 */
           {0},                         /*   cross memory parms              */
           }}};
