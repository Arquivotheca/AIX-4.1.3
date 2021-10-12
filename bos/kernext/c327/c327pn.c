static char sccsid[] = "@(#)08  1.12 src/bos/kernext/c327/c327pn.c, sysxc327, bos41J, 9524A_all 6/10/95 09:36:44";
/*
 * COMPONENT_NAME: (SYSXC327) c327 pinned device driver entry points
 *
 * FUNCTIONS:    c327ConnectWait(), c327Disconnect(), c327SaveTrace(), 
 *     c327SimulateReset(), c327_cfg_init(), 
 *     c327_cfg_term(), c327_getc(), c327_ios(), c327_off_level(), 
 *     c327_putc(), c327intr(), gets_busmem(), pio_error(), puts_busmem(), 
 *     init_handler()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#   include <sys/adspace.h>
#   include <sys/ddtrace.h>
#   include <sys/device.h>
#   include <sys/errno.h>
#   include <sys/except.h>
#   include <sys/intr.h>
#   include <sys/ioacc.h>
#   include <sys/iocc.h>
#   include <sys/malloc.h>
#   include <sys/param.h>
#   include <sys/pin.h>
#   include <sys/timer.h>
#   include <sys/trchkid.h>
#   include <sys/types.h>
#   include <sys/io3270.h>
#   include "tcadefs.h"
#   include "c327dd.h"
#   include "tcadecls.h"
#   include "dftnsSubr.h"

#ifdef _POWER_MP
#include <sys/lock_def.h> 
#include <sys/lock_alloc.h>
#include <sys/lockname.h>

Simple_lock c327_intr_lock;
Complex_lock c327_lock;
#else
lock_t c327_lock;
#endif

extern COMMON_AREA dftnsCommon;

/*
**global pinned variables
*/

char  tca_sess_type;      /* type of session - term, slow term, printer */
char  printerAddr;           /* address of printer port */

cardData  tca_data[MAX_MINORS];  /* indexed by minor device number */

DDS_CONTROL dev_control[MAX_MINORS];
TIMR_QUE timr_que;
INTR_QUE intr_que;
int  c327_dev_major;
int  c327_intr_level;
struct bus_struct {
   ulong bus_id;   /* microchannel bus identifier */
   short count;    /* Reference count for each bus. Equals number of adapters
                      configured for each bus */
   boolean intr_added;  /* i_init completed - defect 180148 */
} global_bus_id[MAX_NUM_BUS];

volatile int  c327_first_time = TRUE;     /* First pass flag      */
struct devsw  c327_dsw;

int c327intr(struct intr *);

struct intr   c327offlstruct;                      /* off level structure */

/* interrupt structures (one per I/O Bus) */
struct intr   c327intrstruct1;
struct intr   c327intrstruct2;

/* increased the pio retry count from 3 to 10 - defect 180148 */
#undef PIO_RETRY_COUNT
#define PIO_RETRY_COUNT 10

/*PAGE*/
/*
 * NAME: c327intr()
 *                                                                    
 * FUNCTION: interrupt handler
 *           
 * EXECUTION ENVIRONMENT: 
 *                        
 *                        
 * (NOTES:) 
 *
 *
 * (RECOVERY OPERATION:) 
 *
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
 *          
 */  

int c327intr(struct intr *ihsptr)
{
   DDS_DATA *dds_ptr;
   uchar    interrupt_status;
   uchar    conn_ctrl_reg;
   uchar    io_ctrl_reg;
   uchar    vis_snd_reg;
   uchar    keystroke;
   boolean  need_offlevel_to_run;
   int      ndx;
   int      return_code;
   int      saved_intr_level;

   C327PERF( 0x0100 );

#ifdef _POWER_MP
   DISABLE_INTERRUPTS(saved_intr_level);
#endif

   return_code = INTR_FAIL;                        /* initialize */

   need_offlevel_to_run = FALSE;

   for (ndx=0; ndx < MAX_MINORS; ndx++)   /* which card(s) is interrupting */
   {
      /* is this a valid card and good DDS */
      if (dev_control[ndx].dds_state > DDS_AVAILABLE)
      {
         dds_ptr = dev_control[ndx].dds_ptr;    /* get pointer to DDS */

         /* get interrupt status register for card */
         interrupt_status = GETC_BUSIO (dds_ptr, adapter_intr_stat_reg);

         if (interrupt_status & 0x80) /* this one's for us */
         {

            C327PERF( (int)HDW.io_port & 0xFF0 );

            /* reset the interrupt status reg by outputting what we input */
            PUTC_BUSIO (dds_ptr, adapter_intr_stat_reg, interrupt_status);

            return_code = INTR_SUCC;

            WRK.intr_reg_save = interrupt_status;  /* save interrupt status */

            switch (dev_control[ndx].dds_state)
            {
               case DDS_OPENED_DIAG:
                  WRK.diag_intr_stat |= interrupt_status;
                  WRK.diag_intr_stat2 |= interrupt_status;

                  /*disable interrupts if test mode else they're continuous */
                  io_ctrl_reg = GETC_BUSIO (dds_ptr, adapter_io_ctrl_reg);

                  PUTC_BUSIO (dds_ptr, adapter_io_ctrl_reg, 0x00);

                  conn_ctrl_reg = GETC_BUSIO (dds_ptr, adapter_conn_ctrl_reg);

                  if ( (conn_ctrl_reg & CONN_CTRL_TEST) != 0 )
                  {
                     conn_ctrl_reg = conn_ctrl_reg | CONN_CTRL_INT_INH;

                     PUTC_BUSIO(dds_ptr, adapter_conn_ctrl_reg, conn_ctrl_reg);
                  }

                  PUTC_BUSIO (dds_ptr, adapter_io_ctrl_reg, io_ctrl_reg);

                  break;            /* no offlevel processing for diag mode */

               case DDS_OPENED_CUT:
               case DDS_OPENED_CUTFT:
                  if (interrupt_status & CUT_KEYACCPT) /* check keystrk ack*/
                  {
                     interrupt_status &= ~CUT_KEYACCPT;

                     if (WRK.CUT_que_nextout != WRK.CUT_que_nextin)
                     { /* any more  keystrokes in que ?*/

                        keystroke = WRK.CUT_que[WRK.CUT_que_nextout++];

                        if (WRK.CUT_que_nextout >= CUT_QUE_SIZE)
                           WRK.CUT_que_nextout = 0;

                        PUTC_BUSIO (dds_ptr, adapter_scan_code_reg, keystroke);
                        PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg, (0x08|
                                 GETC_BUSIO (dds_ptr, adapter_conn_ctrl_reg)));

                        WRK.CUT_key_out_busy = TRUE;
                     }
                     else
                     {
                        WRK.CUT_key_out_busy = FALSE;
                     }
                  }

                  WRK.CUT_intr_stat |= interrupt_status;

                  if ( interrupt_status & CUT_VISOUND )
                  {
                     /* get new visual/sound register setting */
                     vis_snd_reg = GETC_BUSIO (dds_ptr, adapter_vis_snd_reg);

                     /* outputting anything clears "sound alarm" bit */
                     PUTC_BUSIO (dds_ptr, adapter_vis_snd_reg, 0x00);

                     WRK.CUT_vis_snd |= vis_snd_reg;
                  }

                  /*special code to mask (ignore) interrupts not wanted now*/
                  if (((interrupt_status & 0x3F) & WRK.CUT_intr_mask) == 0)
                     break;

                  if ( (interrupt_status & 0x3F) == CUT_VISOUND)
                  {
                    /* this intr was ONLY because of visual/sound reg update */
                     if ((vis_snd_reg & (WRK.CUT_intr_mask >> 8)) == 0)
                        break;
                  }
                  /*If neither of these if statements cause a break, we want */
                  /*to fall thru to next case statement to que offlevel      */


               case DDS_OPENED_DFTNS:
                  /* put dds_ptr in OffLevel que for CUT, DFTNS modes */
                  intr_que.dds_ptr[intr_que.ndx_in++] = dds_ptr;

                  if (intr_que.ndx_in >= INTR_QUE_SIZE)
                     intr_que.ndx_in = 0;

                  need_offlevel_to_run = TRUE;  /* may need to run OffLevel */
                  break;

               default:
                  break;
            }                                          /* end switch */
         }
      }
   }                                                   /* end for */

   if ( need_offlevel_to_run == TRUE )
      i_sched ( &c327offlstruct );                     /* schedule offlevel */

   if (return_code == INTR_SUCC)
      i_reset (ihsptr);

#ifdef _POWER_MP
   RESTORE_INTERRUPTS(saved_intr_level);
#endif

   C327PERF( 0x0101 );

   return (return_code);
}                                                      /* end c327intr */

/*PAGE*/
/*
 * NAME: pio_error()
 *                                                                    
 * FUNCTION: This routine handles permanent pio errors
 *           
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 *
 * (RECOVERY OPERATION:) 
 *
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: nothing
 */  

void pio_error (DDS_DATA * dds_ptr)
{
   label_t          jump_buf;
   volatile int     retry=PIO_RETRY_COUNT;
   volatile char    *busio_addr;

   HDW.PIO_error = TRUE;            /* set permanent PIO error on this card */
   C327TRACE2 ("PIOP", dds_ptr);

   busio_addr = BUSIO_ATT(HDW.c327_busid, HDW.io_port);   /* access mem bus */

   while (TRUE)
   {
      /* Set up for context save by doing setjmpx.  If it returns zero, */
      /* all is OK; otherwise an exception occurred.                    */
      if (setjmpx( &jump_buf ) == 0)
      {
         if ( retry-- )          /* if retries have not been used up, try */
            /* to disable interrupts from card */
            BUSIO_PUTC ((busio_addr + adapter_conn_ctrl_reg),
                        CONN_CTRL_INT_INH);
         break;
      }
   }
   clrjmpx( &jump_buf );                      /* remove exception handler */

   BUSIO_DET(busio_addr);                                /* restore bus */

   dftnsPIOError (dds_ptr);                   /* tell TCA about PIO error */
}
/*PAGE*/
/*
 * NAME: c327_getc()
 *                                                                    
 * FUNCTION:
 *           
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *     arguments dds_ptr: pointer to dds_struct for determining 
 *                        the base address for the io_port
 *                        ( 2D0, 6D0, AD0, ED0 ) or
 *                        beginning address of adapter memory
 *
 *               address: register number or
 *                        offset from beginning of adapter memory
 *                                                                   
 * (NOTES:) 
 *
 *
 * (RECOVERY OPERATION:) 
 *
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: rc - value of register contained in reg_num
 */  

uchar c327_getc(c327io_t io_type, DDS_DATA  *dds_ptr, uint address)
{
   int              rc;
   uchar            rtnchar;
   label_t          jump_buf;
   volatile int     retry=PIO_RETRY_COUNT;
   volatile boolean error=FALSE;
   volatile char    *busio_addr;




   if ( HDW.PIO_error )                /* if permanent PIO error    */
      return(0);                       /* return w/o doing anything */

   switch (io_type)
   {
      case GETCIO:
         /*
         ** access io
         */
         busio_addr = BUSIO_ATT(HDW.c327_busid, HDW.io_port);
         break;
      case GETCMEM:
         /*
         ** access mem
         */
         busio_addr = BUSMEM_ATT(HDW.c327_busid, HDW.bus_mem_beg);
   }

   while (TRUE)
   {
      /* Set up for context save by doing setjmpx.  If it returns zero, */
      /* all is OK; otherwise an exception occurred.                    */
      if ( (rc = setjmpx( &jump_buf ) ) == 0 )
      {
         if ( retry-- )          /* if retries have not been used up do I/O */
            switch (io_type)
            {
               case GETCIO:
                  rtnchar =  BUSIO_GETC (busio_addr + address);
                  break;
               case GETCMEM:
                  rtnchar =  BUS_GETC (busio_addr + address);
            }
         else                    /* permanent PIO error */
            error = TRUE;
         break;
      }
      else /* exception has occurred or reoccurred - if PIO error, retry; */
           /* otherwise go to next exception handler */
      {
         C327TRACE2 ("PIOT", dds_ptr);
         if (rc != EXCEPT_IO)
            longjmpx(rc);
      }
   }
   clrjmpx( &jump_buf );                      /* remove exception handler */

   /* C327TRACE3("getc", address, rtnchar); */

   BUSIO_DET(busio_addr);                                /* restore bus */

   if ( error )
      pio_error (dds_ptr);

   return(rtnchar);
}
/*PAGE*/
/*
 * NAME: c327_putc()
 *                                                                    
 * FUNCTION:
 *           
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *     arguments dds_ptr: pointer to dds_struct for determining 
 *                        the base address for the io_port
 *                        ( 2D0, 6D0, AD0, ED0 ) or
 *                        beginning address of adapter memory
 *
 *               address: register number or
 *                        offset from beginning of adapter memory
 *                                                                   
 * (NOTES:) 
 *
 *
 * (RECOVERY OPERATION:) 
 *
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: nothing
 */  

#define SLOTADDR(addr)    (IO_CONFIG | (addr << 16))

void 
c327_putc (c327io_t io_type, DDS_DATA *dds_ptr, uint address, uchar char_out)
{
   int              rc;
   label_t          jump_buf;
   volatile int     retry=PIO_RETRY_COUNT;
   volatile boolean error=FALSE;
   volatile char    *busio_addr;

   if ( HDW.PIO_error )                /* if permanent PIO error    */
      return;                          /* return w/o doing anything */

   switch (io_type)
   {
      case PUTCIO:
         /*
         ** access io
         */
         busio_addr = BUSIO_ATT(HDW.c327_busid, HDW.io_port);
         break;
      case PUTCMEM:
         /*
         ** access mem
         */
         busio_addr = BUSMEM_ATT(HDW.c327_busid, HDW.bus_mem_beg);
         break;
      case PUTCIOCC:
         /*
         ** access POS registers
         */
         busio_addr = IOCC_ATT(HDW.c327_busid, SLOTADDR(HDW.slot_number));
   }

   /* C327TRACE3("putc", address, char_out); */

   while (TRUE)
   {
      /* Set up for context save by doing setjmpx.  If it returns zero, */
      /* all is OK; otherwise an exception occurred.                    */
      if ( (rc = setjmpx( &jump_buf ) ) == 0 )
      {
         if ( retry-- )          /* if retries have not been used up do I/O */
            switch (io_type)
            {
               case PUTCIO:
               case PUTCIOCC:
                  BUSIO_PUTC (busio_addr + address, char_out);
                  break;
               case PUTCMEM:
                  BUS_PUTC (busio_addr + address, char_out);
            }
         else                    /* permanent PIO error */
            error = TRUE;
         break;
      }
      else /* exception has occurred or reoccurred - if PIO error, retry; */
           /* otherwise go to next exception handler */
      {
         C327TRACE2 ("PIOT", dds_ptr);
         if (rc != EXCEPT_IO)
            longjmpx(rc);
      }
   }
   clrjmpx( &jump_buf );                      /* remove exception handler */

   BUSIO_DET(busio_addr);                                /* restore bus */

   if ( error )
      pio_error (dds_ptr);
}
/*PAGE*/
/*
 * NAME: c327_ios()
 *                                                                    
 * FUNCTION:
 *           
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *     arguments dds_ptr: pointer to dds_struct for determining 
 *                        beginning address of adapter memory
 *                        
 *                offset: offset from beginning address of adapter
 *                        memory to read from or write to
 *                        
 *                memory: address in memory to write to or read from
 *                     
 *                length: number of characters to move
 *                                                                   
 * (NOTES:) 
 *
 *
 * (RECOVERY OPERATION:) 
 *
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 0 if successful
 *          EFAULT if location is invalid
 */  

void c327_ios (c327io_t io_type, DDS_DATA *dds_ptr, 
    int offset, char *memory, int length)
{
   int              rc;
   label_t          jump_buf;
   volatile int     retry=PIO_RETRY_COUNT;
   volatile boolean error=FALSE;
   volatile char    *busmem_addr;

   if ( HDW.PIO_error )                /* if permanent PIO error    */
      return;                          /* return w/o doing anything */
/* 
** access mem bus
*/
   busmem_addr = BUSMEM_ATT(HDW.c327_busid, HDW.bus_mem_beg);

   while (TRUE)
   {
      /* Set up for context save by doing setjmpx.  If it returns zero, */
      /* all is OK; otherwise an exception occurred.                    */
      if ( (rc = setjmpx( &jump_buf ) ) == 0 )
      {
         if ( retry-- )          /* if retries have not been used up do I/O */
            switch (io_type)
            {
               case GETCMEML:
                  bcopy ((void *)(busmem_addr + offset),
                         (void *)memory, (uint)length);
                  break;
               case PUTCMEML:
                  bcopy ((void *)memory,
                         (void *)(busmem_addr + offset), (uint)length);
                  break;
               case BZEROMEM:
                  bzero ((void *)busmem_addr, (uint)length);
            }
         else                    /* permanent PIO error */
            error = TRUE;
         break;
      }
      else /* exception has occurred or reoccurred - if PIO error, retry; */
           /* otherwise go to next exception handler */
      {
         C327TRACE2 ("PIOT", dds_ptr);
         if (rc != EXCEPT_IO)
            longjmpx(rc);
      }
   }
   clrjmpx( &jump_buf );                      /* remove exception handler */
   BUSMEM_DET(busmem_addr);                                /* restore bus */

   if ( error )
      pio_error (dds_ptr);
}

/*PAGE*/
/*
 * NAME: gets_busmem()
 *                                                                    
 * FUNCTION: moves uiop->uio_resid bytes from adapter 
 *           memory location off_set to kernel space 
 *           pointed to by uio structure
 *           
 *
 *           
 * EXECUTION ENVIRONMENT: It will be executed as an argument
 *                        to pio_assist routine along with a 
 *                        recovery routine.
 *                        
 *                        
 *                                                                   
 *     arguments dds_ptr: pointer to dds_struct for determining 
 *                        beginning address of adapter memory
 *                        
 *               off_set: offset from beginning address of adapter
 *                        memory to start uiomove
 *                     
 *                  uiop: pointer to uio struct  
 *                                                                   
 * (NOTES:) 
 *
 *
 * (RECOVERY OPERATION:) 
 *
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 0 if successful
 *          EFAULT if location is invalid
 */  

int gets_busmem (DDS_DATA *dds_ptr, int off_set, struct uio * uiop)
{
   int           rc;
   int           ndx;
   volatile char *busmem_addr;

   if ( HDW.PIO_error )                /* if permanent PIO error    */
      return(EIO);                     /* return w/o doing anything */
/* 
** access mem bus 
*/
   busmem_addr = BUSMEM_ATT(HDW.c327_busid, HDW.bus_mem_beg);

   for (ndx = 0; ndx < PIO_RETRY_COUNT; ndx++)
   {
      if ( ( rc = uiomove ((caddr_t)(busmem_addr + off_set), uiop->uio_resid,
            UIO_READ, uiop ) ) == 0 )
         break;
      C327TRACE2 ("PIOT", dds_ptr);
   }
   BUSMEM_DET(busmem_addr);                                /* restore bus */

   if (rc)
      pio_error (dds_ptr);

   return(rc); 
}
/*PAGE*/
/*
 * NAME: puts_busmem()
 *                                                                    
 * FUNCTION: moves uiop->uio_resid bytes from kernel 
 *           memory pointed to in uio struct to adapter 
 *           memory location pointed to by off_set 
 *           
 * EXECUTION ENVIRONMENT: It will be executed as an argument
 *                        to pio_assist routine along with a 
 *                        recovery routine.
 *                        
 *                        
 *                                                                   
 *     arguments dds_ptr: pointer to dds_struct for determining 
 *                        beginning address of adapter memory
 *                        
 *               off_set: offset from beginning address of adapter
 *                        memory to start uiomove
 *                     
 *                  uiop: pointer to uio struct  
 *                                                                   
 * (NOTES:) 
 *
 *
 * (RECOVERY OPERATION:) 
 *
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 0 if successful
 *          EFAULT if location is invalid
 */  

int puts_busmem (DDS_DATA *dds_ptr, int off_set, struct uio * uiop)
{
   int           rc, ndx;
   volatile char *busmem_addr;

   if ( HDW.PIO_error )                /* if permanent PIO error    */
      return(EIO);                     /* return w/o doing anything */

   busmem_addr = BUSMEM_ATT(HDW.c327_busid, HDW.bus_mem_beg); /* access mem bus */

   for (ndx = 0; ndx < PIO_RETRY_COUNT; ndx++)
   {
      if ( ( rc = uiomove ((caddr_t)(busmem_addr + off_set), uiop->uio_resid,
            UIO_WRITE, uiop ) ) == 0 )
         break;
      C327TRACE2 ("PIOT", dds_ptr);
   }
   BUSMEM_DET(busmem_addr);                              /* restore bus */

   if (rc)
      pio_error (dds_ptr);

   return(0); 
}
/*PAGE*/
/*
 * NAME: c327SaveTrace()
 *                                                                    
 * FUNCTION: enter trace data in the trace table 
 *           
 * EXECUTION ENVIRONMENT: 
 *                        
 *                        
 * (NOTES:) 
 *
 *
 * (RECOVERY OPERATION:) 
 *
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
 *          
 */  

void c327SaveTrace (char *arg1,
    int arg2, int arg3, int arg4, int arg5)
{
   TRCHKGT((HKWD_DD_C327DD | (DD_C327_HOOK<<8)),
           (uint)( (arg1[0]<<24) | (arg1[1]<<16) | \
                   (arg1[2]<<8)  |  arg1[3] ), \
           arg2, arg3, arg4, arg5);
} /*                 end c327SaveTrace                                     */
/*PAGE*/
/*
 * NAME: c327_off_level()
 *                                                                    
 * FUNCTION: Delayed hardware interrupt processing 
 *           called by system routine i-sched 
 *           
 * EXECUTION ENVIRONMENT: 
 *                        
 *                        
 * (NOTES:) 
 *
 *
 * (RECOVERY OPERATION:) 
 *
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
 *          
 */  

int c327_off_level(void)
{
   int            saved_intr_level;
   int            card;
   DDS_DATA       *dds_ptr;

   C327PERF( 0x0200 );

   DISABLE_INTERRUPTS(saved_intr_level);

   if ( (timr_que.ndx_out == timr_que.ndx_in) &&
        (intr_que.ndx_out == intr_que.ndx_in)  )
   {
      RESTORE_INTERRUPTS(saved_intr_level);
      C327PERF( 0x0201 );
      return(0);                             /* if queues are empty, return */
   }

   while ( timr_que.ndx_out != timr_que.ndx_in )
   {
      dds_ptr = timr_que.dds_ptr[timr_que.ndx_out]; /* do 1 from timout que */
      for (card = 0; card < MAX_MINORS; card++)
      {
         if (dds_ptr == dev_control[card].dds_ptr)
         {
            switch (dev_control[card].dds_state)
            {
               case DDS_OPENED_DFTNS:
                  RESTORE_INTERRUPTS(saved_intr_level);
                  if (WRK.waiting_for_restart)
                     InitCard (dds_ptr);
                  else
                     dftnsProcessTimeout (dds_ptr);
                  DISABLE_INTERRUPTS(saved_intr_level);
                  break;
               default:
                  break;
            }                                            /* end switch     */
         }                                               /* end if         */
      }                                                  /* end for        */
      if (++timr_que.ndx_out >= TIMR_QUE_SIZE)
         timr_que.ndx_out = 0;
   }                                                     /* end while      */

   while( intr_que.ndx_out != intr_que.ndx_in ) /* do ALL adapter interupts */
   {
      dds_ptr = intr_que.dds_ptr[intr_que.ndx_out];/* do 1 frm interupt que */
      for (card = 0; card < MAX_MINORS; card++)
      {
         if (dds_ptr == dev_control[card].dds_ptr)
         {
            switch (dev_control[card].dds_state)
            {
               case DDS_OPENED_DFTNS:
                  RESTORE_INTERRUPTS(saved_intr_level);
                  dftnsProcessInterrupt(dds_ptr);
                  DISABLE_INTERRUPTS(saved_intr_level);
                  break;
               case DDS_OPENED_CUT:
               case DDS_OPENED_CUTFT:
                  RESTORE_INTERRUPTS(saved_intr_level);
                  c327cutProcessInterrupt (dds_ptr);
                  DISABLE_INTERRUPTS(saved_intr_level);
                  break;
               case DDS_OPENED_DIAG:
                  break;
               default:
                  break;
            }                                             /* end switch     */
         }                                                /* end if         */
      }                                                   /* end for        */
      if (++intr_que.ndx_out >= INTR_QUE_SIZE)
         intr_que.ndx_out = 0;
   }                                                       /* end while     */

   RESTORE_INTERRUPTS(saved_intr_level);
   C327PERF( 0x0201 );
   return(0);
}                                                       /* end c327OffLevel */
/*PAGE*/

/*
 * NAME: init_handler()
 *                                                                    
 * FUNCTION: Set all of the fields for a pointer to a c327_dds structure and
 *           make a call to i_init() to set up the interrupt handler
 *           used because of multiple structures for multiple busses.
 *           
 * EXECUTION ENVIRONMENT: 
 *                        
 *                        
 * (NOTES:) 
 *
 *
 * (RECOVERY OPERATION:) 
 *
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: none
 *          
 */  

int init_handler ( struct intr  *init_ptr, DDS_DATA *dds_ptr, c327_dds *ddi_ptr)
{
   int rc;

   init_ptr->handler  = (int (*)(void))c327intr;
   init_ptr->bus_type  = ddi_ptr->bus_type;
#ifdef _POWER_MP
   init_ptr->flags     = INTR_MPSAFE;
#else
   init_ptr->flags     = 0;
#endif
   init_ptr->level     = ddi_ptr->bus_intr_lvl;
   init_ptr->priority  = c327_intr_level;
   init_ptr->bid       = HDW.c327_busid;

   return( INTR_SUCC );
}

/*
 * NAME: c327_cfg_init()
 *                                                                    
 * FUNCTION: configure one adapter and initialize device driver if
 *           configuring the first adapter
 *           
 * EXECUTION ENVIRONMENT: 
 *                        
 *                        
 * (NOTES:) 
 *
 *
 * (RECOVERY OPERATION:) 
 *
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
 *          
 */  

int c327_cfg_init(dev_t dev, struct uio *uiop)
{
   ulong  ulng;
   uchar  pos3, pos4, pos5;
   int rc, ndx;
   int adapter_number;

   DDS_DATA *dds_ptr;
   c327_dds *ddi_ptr;
   c327_dds initstruct;

   rc = 0;

   uiomove( (caddr_t)&initstruct, (int)sizeof(initstruct), UIO_WRITE, uiop );
   ddi_ptr = &(initstruct);

   C327TRACE3("cfgi", 1, c327_first_time);
   if (c327_first_time == TRUE)               /* ONLY on first time through */
   {
      c327_dsw.d_open     = (int (*)(void))c327open;
      c327_dsw.d_close    = (int (*)(void))c327close;
      c327_dsw.d_read     = (int (*)(void))c327read;
      c327_dsw.d_write    = (int (*)(void))c327write;
      c327_dsw.d_ioctl    = (int (*)(void))c327ioctl;
      c327_dsw.d_strategy = (int (*)(void))nodev;
      c327_dsw.d_ttys     = (struct tty *)NULL;
      c327_dsw.d_select   = (int (*)(void))c327select;
      c327_dsw.d_config   = (int (*)(void))c327config;
      c327_dsw.d_print    = (int (*)(void))nodev;
      c327_dsw.d_dump     = (int (*)(void))nodev;
      c327_dsw.d_mpx      = (int (*)(void))c327mpx;
      c327_dsw.d_revoke   = (int (*)(void))nodev;
      c327_dsw.d_dsdptr   = (caddr_t)NULL;
      c327_dsw.d_selptr   = (caddr_t)NULL;
#ifdef _POWER_MP
      c327_dsw.d_opts     = (ulong)DEV_MPSAFE;
#else
      c327_dsw.d_opts     = (ulong)0;
#endif

      rc = devswadd( dev, &c327_dsw );
      if (rc != 0)
      {
         return(rc);
      }

#ifdef _POWER_MP
   C327TRACE2("aloc", &c327_lock);
      lock_alloc(&c327_lock, LOCK_ALLOC_PIN, C327_TOP_LOCK, -1);
      lock_init(&c327_lock, TRUE);
   C327TRACE2("aloc", &c327_intr_lock);
      lock_alloc(&c327_intr_lock, LOCK_ALLOC_PIN, C327_INTR_LOCK, -1);
      simple_lock_init(&c327_intr_lock);
#else
	c327_lock = LOCK_AVAIL;
#endif
      
      timr_que.ndx_in      = 0;
      timr_que.ndx_out     = 0;
      intr_que.ndx_in      = 0;
      intr_que.ndx_out     = 0;

      c327_intr_level = ddi_ptr->intr_priority;
      

      c327offlstruct.next     = (struct intr *) NULL;     /* initialize  */
      c327offlstruct.handler  = c327_off_level;          /* offlevel struct */
      c327offlstruct.bus_type = BUS_NONE;

#ifdef _POWER_MP
      c327offlstruct.flags    = INTR_MPSAFE;
#else
      c327offlstruct.flags    = 0;
#endif

      switch (c327_intr_level)
      {
         case INTCLASS1:
            c327offlstruct.level    = INT_OFFL1;
            c327offlstruct.priority = INTOFFL1;
            break;
         case INTCLASS2:
            c327offlstruct.level    = INT_OFFL2;
            c327offlstruct.priority = INTOFFL2;
            break;
         case INTCLASS3:
         default:
            c327offlstruct.level    = INT_OFFL3;
            c327offlstruct.priority = INTOFFL3;
            break;
      }

      for (ndx = 0; ndx < MAX_MINORS; ndx++)
      {
         dev_control[ndx].dds_state = DDS_NOT_EXIST;  /* start with no card */
         dev_control[ndx].dds_ptr   = NULL;
      }

      /* initialize the bus id structure entries so that the initial value
         for the bus is is -99 and all counts are equal to zero */
      for (ndx=0; ndx < MAX_NUM_BUS; ndx++) 
      {
         global_bus_id[ndx].bus_id = -99;  /* so we don't get into trouble */
         global_bus_id[ndx].count = 0;
         global_bus_id[ndx].intr_added = FALSE; /* defect 180148 */
      }

      timeoutcf(5);                        /* define max number of timeouts */

      c327_dev_major  =  major(dev);       /* get the device MAJOR number   */

      c327_first_time = FALSE;         /* set flag so we wont do this again */
      C327TRACE3("cfgi", 2, c327_first_time);

      /* 
      ** init the dft COMMON AREA 
      */
      bzero((void *)&dftnsCommon, sizeof(dftnsCommon));

#ifdef _POWER_MP
      C327TRACE2("aloc", &CMNMIS.common_locked);
      lock_alloc((complex_lock_t)&CMNMIS.common_locked,
		LOCK_ALLOC_PIN, C327_CMN_LOCK, -1);
      lock_init((complex_lock_t)&CMNMIS.common_locked, TRUE);
#else
      CMNMIS.common_locked = LOCK_AVAIL;
#endif

      /* 
      ** init the tca COMMON AREA 
      */
      bzero((void *)tca_data, sizeof(tca_data));
   }                                    /* end special stuff for first trip */
   adapter_number = minor(dev);
   c327_add_dds( ddi_ptr, adapter_number);   /* complete the dds */

   ulng = ddi_ptr->bus_mem_beg;
   pos3 = (ulng >> 13) << 1;                  /* bits 13-19 ===> bits 1-7 */
   pos4 = (ulng >> 20) << 4;                  /* bits 20-23 ===> bits 4-7 */

   switch (ddi_ptr->io_port)                           /* target bits 2-3 */
   {
      case 0x2D0: 
      default: 
         pos4 |=  0;
         break;
      case 0x6D0:
         pos4 |=  4;
         break;
      case 0xAD0: 
         pos4 |=  8;
         break;
      case 0xED0: 
         pos4 |= 12;
         break;
   }
   switch (ddi_ptr->bus_mem_size)                      /* target bits 0-1 */
   {
      case 0x02000: 
      default: 
         pos4 |= 0;
         break;                            /*  8K */
      case 0x04000: 
         pos4 |= 1;
         break;                            /* 16K */
      case 0x10000: 
         pos4 |= 2;
         break;                            /* 64K */
   }

   pos5 = 0xC0;

   dds_ptr = dev_control[adapter_number].dds_ptr;
                                           /* POS reg 2 set on open/close */

   c327offlstruct.bid  = HDW.c327_busid;     /* only has to be done once
                                                   because bus_type = BUS_NONE*/
   HDW.c327_busid      = ddi_ptr->bus_id;    /* Bus id in work area */

   /* section where we figure out what bus id was passed in. This is
      important because we need to register the interrupt handler every
      time we get a new bus id. Also, a reference count for each bus needs to
      be kept so that the cfg_delete routine can determine when to detach
      from the bus. This logic can be easily generalized to allow for
      n busses. However, at this point in time is geared to 2. */

   /* case 1,2: The id is already in a record. Increment the reference count */
   if (global_bus_id[0].bus_id == HDW.c327_busid)
      global_bus_id[0].count++;
   else if (global_bus_id[1].bus_id == HDW.c327_busid)
      global_bus_id[1].count++;

   /* case 3,4: The id was not found. Store id in first empty record */
   else if (global_bus_id[0].count == 0) {
      global_bus_id[0].bus_id = HDW.c327_busid;  /* store id portion */
      global_bus_id[0].count ++;
      /* load values and do an i_init() */
      rc = init_handler (&c327intrstruct1,dds_ptr,ddi_ptr);
      if (rc != INTR_SUCC)
      {
         global_bus_id[0].count = 0;
         global_bus_id[0].bus_id = -99;
      }
   }
   else {
      global_bus_id[1].bus_id = HDW.c327_busid;  /* store id portion */
      global_bus_id[1].count ++;
      /* load values and do an i_init() */
      rc = init_handler (&c327intrstruct2,dds_ptr,ddi_ptr);
      if (rc != INTR_SUCC)
      {
         global_bus_id[1].count = 0;
         global_bus_id[1].bus_id = -99;
      }
   }
   if (rc != INTR_SUCC)	/* the i_init call failed for some reason */
   {
      dev_control[adapter_number].dds_state = DDS_NOT_EXIST; /* mark DDS as */
      dev_control[adapter_number].dds_ptr = NULL;            /* nonexistent & */
      xmfree((void *)dds_ptr, pinned_heap );                 /* free it */

      for (ndx=0; ndx < MAX_MINORS; ndx++)           /* if any DDS is still */
      {                                              /* in existence, exit */
         if ( dev_control[ndx].dds_state != DDS_NOT_EXIST ) 
            return ( rc );
      }

      c327_first_time = TRUE;                 /* flag for c327_cfg_init */

      timeoutcf(-5);                          /* undefine max # of timeouts */

      devswdel( dev );                        /* delete switch table entry */

      return ( rc );
   }

   PUTC_IOCC (dds_ptr, 3, pos3);               /* set other POS registers */
   PUTC_IOCC (dds_ptr, 4, pos4);
   PUTC_IOCC (dds_ptr, 5, pos5);

   /* DDHKGEN (HKWD_DD_C327DD, dev, sizeof(HDR.dev_name), HDR.dev_name); */

   C327TRACE3("cfgi", 3, c327_first_time);
   return ( 0 );
}                                                       /* end c327_cfg_init */
/*PAGE*/
/*
 * NAME: c327_cfg_term()
 *                                                                    
 * FUNCTION: unconfig one adapter and delete device driver if unconfiging
 *           the last adapter
 *           
 * EXECUTION ENVIRONMENT: 
 *                        
 *                        
 * (NOTES:) 
 *
 *
 * (RECOVERY OPERATION:) 
 *
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
 *          
 */  

int c327_cfg_term(dev_t dev)
{
   DDS_DATA *dds_ptr;
   int rc, ndx, adapter_number;

      C327TRACE3("cfgt", 1, c327_first_time);

   if ( c327_first_time == TRUE ) return( EACCES );  /* can't unconfig when */
                                                     /* nothing has yet */
                                                     /* been configured */

   adapter_number = minor(dev);
   if ( dev_control[adapter_number].dds_state != DDS_AVAILABLE )
      return( EBUSY );                    /* adapter is still in use */

   dds_ptr = dev_control[adapter_number].dds_ptr;
   dev_control[adapter_number].dds_state = DDS_NOT_EXIST; /* mark DDS as */
   dev_control[adapter_number].dds_ptr = NULL;            /* nonexistent & */

   /* Decrement the reference count on the record containing the bus id.
      If the cout goes to zero, the last adapter has been de-configured. Do
      an i_clear() */
   if (global_bus_id[0].bus_id == HDW.c327_busid) {
      global_bus_id[0].count --;
      if (global_bus_id[0].count == 0) {
         global_bus_id[0].bus_id = -99;
      }
    }
    else {
       global_bus_id[1].count --;
       if (global_bus_id[1].count == 0) {
          global_bus_id[1].bus_id = -99;
       }
    }


   xmfree((void *)dds_ptr, pinned_heap );                 /* free it */
      C327TRACE2("term", 2);

   for (ndx=0; ndx < MAX_MINORS; ndx++)             /* if any DDS is still */
   {                                                  /* in existence, exit */
      if ( dev_control[ndx].dds_state != DDS_NOT_EXIST ) 
         return (0);
   }

   c327_first_time = TRUE;                 /* flag for c327_cfg_init */

#ifdef _POWER_MP
      C327TRACE2("free", &c327_lock);
  lock_free((void *)&c327_lock);
      C327TRACE2("free", &c327_intr_lock);
  lock_free((void *)&c327_intr_lock);
      C327TRACE2("free", &CMNMIS.common_locked);
  lock_free((void *)&CMNMIS.common_locked);
#endif

   rc = devswdel( dev );                   /* delete switch table entry */
   if (rc != 0) return ( rc );             /* WARNING */

   timeoutcf(-5);                          /* undefine max # of timeouts */

   C327TRACE3("cfgt", 2, c327_first_time);
   return ( 0 );
}

/*PAGE*/
/*
 * NAME: c327ConnectWait()
 *                                                                    
 * FUNCTION: pause before reconnecting if 
 *           necessary (to avoid re-connect 
 *           within 5 sec)
 *           
 * EXECUTION ENVIRONMENT: 
 *
 *
 * (NOTES:) 
 *
 *
 * (RECOVERY OPERATION:) 
 *
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
 *
 */

void c327ConnectWait (DDS_DATA *dds_ptr, int seconds_to_delay)
{
   int      seconds_passed;
   struct   timestruc_t time_struct;

   seconds_to_delay++;

   if ((seconds_to_delay < 2) || (6 < seconds_to_delay))
      seconds_to_delay = 6;

   curtime((struct timestruc_t *)&time_struct);

   seconds_passed = time_struct.tv_sec - HDW.last_disconnect;

   if ( (HDW.last_disconnect != 0) && (seconds_passed < seconds_to_delay) )
      delay ( (int)(seconds_to_delay - seconds_passed) * HZ );

   return;
}

/*PAGE*/
/*
 * NAME: c327SimulateReset()
 *                                                                    
 * FUNCTION: simulate a power-on reset by going 
 *           into test mode and back out
 *           
 * EXECUTION ENVIRONMENT: 
 *                        
 *                        
 * (NOTES:) 
 *
 *
 * (RECOVERY OPERATION:) 
 *
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
 *          
 */  

void c327SimulateReset (DDS_DATA *dds_ptr)
{
   C327TRACE1 ("SIMR");

   PUTC_BUSIO (dds_ptr, adapter_io_ctrl_reg, 0);
   PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg, CONN_CTRL_INT_INH);
   PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg,
               (CONN_CTRL_INT_INH | CONN_CTRL_TEST));
   PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg, CONN_CTRL_INT_INH);
   PUTC_BUSIO (dds_ptr, adapter_intr_stat_reg, 0x3F);
   PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg,
               (CONN_CTRL_INT_INH | CONN_CTRL_TEST));
   PUTC_BUSIO (dds_ptr, adapter_scan_code_reg, 0x00);
   PUTC_BUSIO (dds_ptr, adapter_term_id_reg,   0xFF);
   PUTC_BUSIO (dds_ptr, adapter_87e_stat_reg,  0xE0);
   PUTC_BUSIO (dds_ptr, adapter_io_ctrl_reg, 1);
   PUTC_BUSIO (dds_ptr, 0x0B, 0xFF);
   PUTC_BUSIO (dds_ptr, 0x0C, 0x00);
   PUTC_BUSIO (dds_ptr, 0x0D, 0x1B);
   PUTC_BUSIO (dds_ptr, adapter_io_ctrl_reg, 2);
   PUTC_BUSIO (dds_ptr, 0x01, 0x1B);
   PUTC_BUSIO (dds_ptr, 0x02, 0xFF);
   PUTC_BUSIO (dds_ptr, 0x03, 0x0F);
   PUTC_BUSIO (dds_ptr, 0x04, 0xFF);
   PUTC_BUSIO (dds_ptr, 0x05, 0x0F);
   PUTC_BUSIO (dds_ptr, 0x07, 0x00);
   PUTC_BUSIO (dds_ptr, 0x09, 0x00);
   PUTC_BUSIO (dds_ptr, adapter_io_ctrl_reg, 0);
   PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg, CONN_CTRL_INT_INH);
   PUTC_BUSIO (dds_ptr, adapter_intr_stat_reg, 0x3F);

   C327_BZERO (dds_ptr, HDW.bus_mem_size);  /* zero bus memory */

   return;
}

/*PAGE*/
/*
 * NAME: c327Disconnect()
 *                                                                    
 * FUNCTION: disconnect the adapter and 
 *           record the time for restarts
 *           
 * EXECUTION ENVIRONMENT: 
 *
 *
 * (NOTES:) 
 *
 *
 * (RECOVERY OPERATION:) 
 *
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
 *
 */

void c327Disconnect (DDS_DATA *dds_ptr)
{
   struct timestruc_t time_struct;
   C327TRACE1 ("DISC");

   c327SimulateReset (dds_ptr);

   curtime(&time_struct);

   HDW.last_disconnect = time_struct.tv_sec;
}
