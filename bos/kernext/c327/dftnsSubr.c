static char sccsid[] = "@(#)29  1.29 src/bos/kernext/c327/dftnsSubr.c, sysxc327, bos411, 9430C411a 7/27/94 09:33:02";

/*
 * COMPONENT_NAME: (SYSXC327) c327 dft device driver Subroutines
 *
 * FUNCTIONS:    AckCmd(), BroadcastUnSol(), BuildAsyncReq(), CopyAsyncReq(), 
 *     EnqueAsyncRequest(), InitCard(), InterruptDH(), 
 *     ProcessAsyncReq(), SendAS(), SendNextAS(), SendSS(), SetTimer(), 
 *     ShutDown(), UnprotectedSetTimer(), XmitData(), dftnsOpen(), 
 *     dftnsSend(), xfer_data_SNA(), segment_SNA_data() 
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

/*
** These are the functions common to both dftnsCmd and dftnsOffl
**
**
** used externally: UnprotectedSetTimer
** used externally: SetTimer
** used externally: BuildAsyncReq
** local CopyAsyncReq
** used externally: InterruptDH
** used externally: BroadcastUnSol
** used externally: AckCmd  
** local ProcessAsyncReq    
** local EnqueAsyncRequest  
** used externally: SendAS  
** local SendNextAS         
** used externally: SendSS  
** used externally: XmitData
** used externally: InitCard
** used externally: ShutDown
*/

#include <stddef.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>
#include <sys/intr.h>

#ifdef _POWER_MP
#include <sys/lock_def.h> 
extern Simple_lock c327_intr_lock;
#else
#include <sys/lockl.h>
#endif

#include <sys/param.h>
#include <sys/sleep.h>
#include <sys/timer.h>
#include <sys/io3270.h>
#include "c327dd.h"
#include "dftnsDcl.h"
#include "dftnsSubr.h"
#include "tcaexterns.h"
/*
** This is the allocation of DFT's private status data
*/

COMMON_AREA dftnsCommon;

/*
** external variables
*/

extern struct intr   c327offlstruct;
/*PAGE*/
/*
 * NAME: UnprotectedSetTimer()
 *                                                                  
 * FUNCTION: set a timer, but only after 
 *           interrupts are disabled elsewhere
 *           
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

void UnprotectedSetTimer (DDS_DATA *dds_ptr, int timer_number, 
                          TIMER_TYPE timer_type, int timer_msec)
{
   C327UNPTRACE5 ("SETT",dds_ptr,timer_number,timer_type,
                  timer_msec);

   if (timer_number == 1)
   {
      WRK.timer_type_1  = timer_type;
      WRK.timer_count_1 =
         (timer_msec + C327_TIMER_INTERVAL - 1) / C327_TIMER_INTERVAL;
   }
   else
   {
      WRK.timer_type_2  = timer_type;
      WRK.timer_count_2 =
         (timer_msec + C327_TIMER_INTERVAL - 1) / C327_TIMER_INTERVAL;
   }
   return;

}
/*PAGE*/
/*
 * NAME: SetTimer()
 *                                                                  
 * FUNCTION: set a timer when interrupts 
 *           are enabled (used by all but Timer)
 *           
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

void SetTimer (DDS_DATA *dds_ptr, int timer_number, 
               TIMER_TYPE timer_type, int timer_msec)
{
   int saved_intr_level;

   DISABLE_INTERRUPTS(saved_intr_level);

   UnprotectedSetTimer (dds_ptr, timer_number, timer_type, timer_msec);

   RESTORE_INTERRUPTS(saved_intr_level);

   return;
}
/*PAGE*/
/*
 * NAME: BuildAsyncReq()
 *                                                                  
 * FUNCTION: build an async request que element
 *           
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

void BuildAsyncReq (uchar as_parm0, uchar as_parm1, uchar as_parm2, 
       uchar as_parm3, uchar as_event, uchar as_lta, ASYNC_REQ *async_req_adr)
{
   (*async_req_adr).as_parm[0]   = as_parm0;
   (*async_req_adr).as_parm[1]   = as_parm1;
   (*async_req_adr).as_parm[2]   = as_parm2;
   (*async_req_adr).as_parm[3]   = as_parm3;
   (*async_req_adr).as_event     = as_event;
   (*async_req_adr).as_lta       = as_lta;
   return;
}
/*PAGE*/
/*
 * NAME: CopyAsyncReq()
 *                                                                  
 * FUNCTION: copy an async request que element
 *           
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

void CopyAsyncReq (ASYNC_REQ *from_ptr, ASYNC_REQ *to_ptr)
{
   to_ptr->as_parm[0]   = from_ptr->as_parm[0];
   to_ptr->as_parm[1]   = from_ptr->as_parm[1];
   to_ptr->as_parm[2]   = from_ptr->as_parm[2];
   to_ptr->as_parm[3]   = from_ptr->as_parm[3];
   to_ptr->as_event     = from_ptr->as_event;
   to_ptr->as_lta       = from_ptr->as_lta;
   return;
}
/*PAGE*/
/*
 * NAME: InterruptDH()
 *                                                                  
 * FUNCTION: queue an interrupt for the device head
 *
 *   Interrupt the device head under three conditions:
 *     1) unsolicited interrupt
 *     2) start session complete
 *     3) write complete
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

void InterruptDH (DDS_DATA *dds_ptr, int session_index, 
                  INTR_TYPE intr_type, int return_code, int er_3270)
{

   C327TRACE4 ("QIDH", dds_ptr,  session_index, intr_type );
   C327TRACE5 ("QID2", return_code, er_3270,
                      TBL(session_index).network_id.session_code,
                      TBL(session_index).network_id.adapter_code);

   tcaintr ( TBL(session_index).network_id,
             intr_type, return_code, er_3270);
}
/*PAGE*/
/*
 * NAME: BroadcastUnSol()
 *                                                                 
 * FUNCTION: broadcast unsolicited status to the device head(s)
 *           
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

void BroadcastUnSol (DDS_DATA *dds_ptr, int return_code, int er_3270)
{

   int sessndx;

   C327TRACE4 ("BCST", dds_ptr, return_code, er_3270);

   for ( sessndx = 0; sessndx < WRK.num_sess; sessndx++)
   {
      if ( (int)TBL(sessndx).lt_state >= (int)LT_ONLINE )
         InterruptDH(dds_ptr,sessndx,INTR_UNSOL,return_code,er_3270);
   }

   return;
}
/*PAGE*/
/*
 * NAME: AckCmd()
 *                                                                  
 * FUNCTION: handle the completion of a command
 *           
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

void AckCmd (DDS_DATA *dds_ptr, int session_index, INTR_TYPE intr_type)
{
   C327TRACE4 ("AKCM", dds_ptr, session_index, intr_type);

   switch ( intr_type )
   {
      case INTR_SOL_WRITE:
         if ( TBL(session_index).cmd_write_pending == TRUE )
         {
            TBL(session_index).cmd_write_pending = FALSE;

            TBL(session_index).write_in_progress = FALSE;

            InterruptDH(dds_ptr, session_index,
                        INTR_SOL_WRITE, OR_NO_ERROR, 0);
         }
         break;

      case INTR_SOL_START:
         if ( TBL(session_index).cmd_start_pending == TRUE )
         {
            TBL(session_index).cmd_start_pending = FALSE;

            InterruptDH(dds_ptr, session_index,
                        INTR_SOL_START, OR_NO_ERROR, 0);

            if (WRK.WCUS_30_pending == TRUE) /* is como reminder is pending */
            {
               InterruptDH (dds_ptr, session_index, INTR_UNSOL,
                            OR_CHK_STAT_MSK, WRK.cu_error_o);
            }
         }
         break;

      default:
         break;

   } /* end switch   */

   return;
}
/*PAGE*/
/*
 * NAME: ProcessAsyncReq()
 *                                                                  
 * FUNCTION: process a que element passed from SendAS or SendNextAS.
 *           
 *           
 * EXECUTION ENVIRONMENT: Must be called with interrupts disabled to
 * 	c327_intr_level.
 *                        
 *                        
 * NOTES: This routine posts either AEEPs or AEDV asynchronous requests
 *	and sets flags appropriatly to indicate that acknowledgement is
 *	pending from the host.  If the element being processed is from the
 *	queue, the element is removed from the queue.  If the WRK.el.cnt is
 *	non zero, then the element being handled is from the queue, otherwise
 * 	the element was passed directly to this routine without being enqueued.
 *
 *
 * RETURNS: SEND_REQ_TCA_REJECT - if the the event was not posted to the adapter
 *		In this case, the element isn't dequeued if is was on the queue.
 *	    SEND_REQ_POLL_SENT, SEND_REQ_COMPLETE_OK - ok
 *          
 */  

SEND_REQ_RESULT ProcessAsyncReq(DDS_DATA *dds_ptr, ASYNC_REQ *async_req_ptr)
{
   SEND_REQ_RESULT  rc;
   int              sessndx;
   boolean          found_active_session;

   rc = SEND_REQ_COMPLETE_OK;                     /* initialize return code */

C327TRACE2("pasb",(async_req_ptr->as_event == AEDV));
   if (async_req_ptr->as_event == AEDV)           /* call to go on/offline */
   {
      if (async_req_ptr->as_parm[0] == AEDV_01)   /* is call to go online   */
         WRK.device_state = PENDING_ON_LINE;      /* state = pending online */

      if (async_req_ptr->as_parm[0] == AEDV_02)   /* is call to go offline  */
      {

         for (sessndx = 0; sessndx < WRK.num_sess; sessndx++)
         {  /* look for active sessions to be notified */

            if (TBL(sessndx).lt_bit_map ==       /* look for LT addr matchs */
                (TBL(sessndx).lt_bit_map & async_req_ptr->as_parm[1]) )
            {
               if ((TBL(sessndx).lt_state == LT_ONLINE) ||
                   (TBL(sessndx).lt_state == LT_READ_LOCK_DEF))
               {
                  WRK.device_state = PENDING_OFF_LINE;   /* AEDV  = offline */
                  break;
               }
               else
               {
C327TRACE3("pase",1, SEND_REQ_TCA_REJECT);
                  return (SEND_REQ_TCA_REJECT);     /* Reject Asyn request */
               }
            }
         }
      }
   }

   if (async_req_ptr->as_event == AEEP)              /* write data pending */
   {

      if ((WRK.non_sna == TRUE) && (WRK.aeep_outstanding == TRUE))
	 return (SEND_REQ_TCA_REJECT);		/* only one aeep at a time */

      if (WRK.WCUS_30_pending == TRUE)          /* which sessn is write for */
      {
         /* Do not put this async request in adapter mem */
         return (SEND_REQ_TCA_REJECT);              /* Return with pos rc */
      }

      found_active_session = FALSE;             /* Init act sessn found flg */

      if ((WRK.cu_type == CH_4361) || (WRK.cu_type == CH_3274) ||
          (WRK.cu_type == BSC_3274))           /* CU type is a 4361 or 3274 */
      {

         for (sessndx = 0; sessndx < WRK.num_sess; sessndx++)
         {                                      /* look for active sessions */

            if (((int)TBL(sessndx).lt_state >  (int)LT_WRT_CMD_REDO)  &&
                 (TBL(sessndx).lt_state != LT_READ_LOCK_DEF) &&
                 (TBL(sessndx).lt_state != LT_READ_LOCK_PRT) )
            {                                  /* is sessn active with host */
               found_active_session = TRUE;    /* Init act sessn found flg */
               break;
            }
         }
      }

      for (sessndx = 0; sessndx < WRK.num_sess; sessndx++)
      {  
         /*
         ** look for active sessions to be notified
         */
         if (TBL(sessndx).lt_id == async_req_ptr->as_lta) /* LT addr match */
         {
            if (TBL(sessndx).lt_state == LT_ONLINE)
            {

               if (TBL(sessndx).kbd_reset_pending == TRUE)
               {                                          /* reset keystrk  */
                  TBL(sessndx).kbd_reset_pending = FALSE; /* Reset the flag */

                  /* RAS.cnt_rst_key_p ++ Incr RAS Ctr   */

                  /* xxx - pathlength down AckCmd is pretty long */
                  AckCmd(dds_ptr,sessndx,INTR_SOL_WRITE); /* Build QE       */

                  return (SEND_REQ_COMPLETE_OK);  /* Take AEEP out of queue */
               }
               else
               {

                  if (found_active_session == TRUE)
                  {                       /* conditions for 4361/3274 patch */
                     /* AEEP waits until after CTCCS */
                     TBL(sessndx).lt_state = LT_WRT_CMD_REDO;
                     C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);

                     return(SEND_REQ_COMPLETE_OK);/* Take AEEP out of queue */
                  }
                  else
                  {
                     /* Update the LT state to write pending */
                     TBL(sessndx).lt_state = LT_WRT_CMD_PEND;
                     C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);

                     break;
                  }
               }
            }
            else
            {
               return(SEND_REQ_TCA_REJECT);      /* Return with positive rc */
            }
         }
      }                                                   /* end for */
   }

   if ((WRK.non_sna == TRUE) && (async_req_ptr->as_event == AEEP))
      WRK.aeep_outstanding = TRUE;

   WRK.dpastat_save = UNACK;                           /* Save Comp stat    */
   /* RAS.cnt_async_req ++                                Incr RAS counter  */

   /* Save the Async Event for processing at async Acknowledgement time */
   CopyAsyncReq (async_req_ptr, &(WRK.old_async_req));

   C327TRACE4 ("SDAS", dds_ptr, 
               async_req_ptr->as_event, async_req_ptr->as_lta );

   C327TRACE5 ("SDA2", async_req_ptr->as_parm[0],
                       async_req_ptr->as_parm[1], async_req_ptr->as_parm[2],
                       async_req_ptr->as_parm[3]);

   /* Set Asynchronous Status in the adapter mem */
   /* Set Event ID Code */
   PUTC_BUSMEM (dds_ptr, offsetof(tca_t, daev), async_req_ptr->as_event);

   /* Set LT address    */
   PUTC_BUSMEM (dds_ptr, offsetof(tca_t, daltad), async_req_ptr->as_lta);

   /* Set CC Parm 1     */
   PUTC_BUSMEM (dds_ptr, offsetof(tca_t, daep1), async_req_ptr->as_parm[0]);

   /* Set CC Parm 2     */
   PUTC_BUSMEM (dds_ptr, offsetof(tca_t, daep2), async_req_ptr->as_parm[1]);

   /* Set CC Parm 3     */
   PUTC_BUSMEM (dds_ptr, offsetof(tca_t, daep3), async_req_ptr->as_parm[2]);

   /* Set CC Parm 4     */
   PUTC_BUSMEM (dds_ptr, offsetof(tca_t, daep4), async_req_ptr->as_parm[3]);

   PUTC_BUSMEM (dds_ptr, offsetof(tca_t, dpastat), UNACK);

   PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg,        /* Set req poll bit  */
               (WRK.conn_ctrl_save | CONN_CTRL_POLL_REQ));

   rc = SEND_REQ_POLL_SENT;            /* Set rc so that a 2nd req not sent */

   if (WRK.el_cnt >= 1) {              /* Take el off the queue if we were */
				       /* processing from the queue */
            WRK.el_cnt--;
            WRK.out_ptr++;
            if (WRK.out_ptr >= ASYNC_QUE_SIZE)
               WRK.out_ptr = 0;
   }
C327TRACE4("pase",2, rc, WRK.el_cnt);

   C327PERF( 0x265 );

   return (rc);
}                                      /* end ProcessAsyncReq               */
/*PAGE*/
/*
 * NAME: EnqueAsyncRequest()
 *                                                                  
 * FUNCTION: add an async request to the 
 *           async request que -- there must be room!
 *           
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

void EnqueAsyncRequest (DDS_DATA *dds_ptr, ASYNC_REQ *async_req_ptr)
{
   int   saved_intr_level;

   CopyAsyncReq (async_req_ptr, &(WRK.async_req_que[WRK.in_ptr]));

   WRK.in_ptr++;

   if (WRK.in_ptr >= ASYNC_QUE_SIZE)
      WRK.in_ptr = 0;

   WRK.el_cnt++;
C327TRACE2("enqe", WRK.el_cnt);

   return;

} /* end EnqueAsyncRequest */
/*PAGE*/
/*
 * NAME: SendAS()
 *                                                                  
 * FUNCTION: send asynchronous status to the 
 *           control unit, queuing if necessary
 *           note this is the only place where 
 *           items are added to the async que
 *           
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

SEND_REQ_RESULT SendAS (DDS_DATA *dds_ptr, ASYNC_REQ *async_req_ptr)
{
   int               saved_intr_level;  
   SEND_REQ_RESULT   rc;          /* the status we will return              */
   SEND_REQ_RESULT   drc;         /* the status back from ProcessAsyncReq   */

   rc = SEND_REQ_COMPLETE_OK;     /* initialize return code                 */

   /* determine if some async request can be sent right now */
   DISABLE_INTERRUPTS(saved_intr_level);
   if ((WRK.interface_state         == CONNECTED_A_IDLE) &&
       (WRK.dpastat_save            == ACK             ))
   {

      /* if queue is empty, we can try to do it right now */
 C327TRACE2("sasb", WRK.el_cnt);
      if (WRK.el_cnt == 0)
      {
         drc = ProcessAsyncReq (dds_ptr, async_req_ptr);

         if (drc == SEND_REQ_TCA_REJECT)
            EnqueAsyncRequest (dds_ptr, async_req_ptr);
      }
      else
      {
         /* try to remove oldest element and then enqueue this one */
         drc = ProcessAsyncReq (dds_ptr, &(WRK.async_req_que[WRK.out_ptr]));

         if (drc != SEND_REQ_TCA_REJECT)
            rc = drc;

         if (WRK.el_cnt < ASYNC_QUE_SIZE)
            EnqueAsyncRequest (dds_ptr, async_req_ptr);
         else
            rc = SEND_REQ_QUE_FULL;
      }

   }
   else
   {
      if (WRK.el_cnt < ASYNC_QUE_SIZE)
         EnqueAsyncRequest (dds_ptr, async_req_ptr);
      else
         rc = SEND_REQ_QUE_FULL;
   }

 C327TRACE2("sase", rc);
   RESTORE_INTERRUPTS(saved_intr_level);
   return (rc);

} /* end SendAS */
/*PAGE*/
/*
 * NAME: SendNextAS()
 *                                                                  
 * FUNCTION: do next SendAS from que if there 
 *           are any to send
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

void SendNextAS (DDS_DATA *dds_ptr)
{
   int                saved_intr_level;
   SEND_REQ_RESULT    rc;

   /* setup to assume ProcessAsyncReq does not send poll */
   rc = SEND_REQ_COMPLETE_OK;

   DISABLE_INTERRUPTS(saved_intr_level);

   /* determine if some async request can be sent right now */
   if ((WRK.dpastat_save == ACK)&& (WRK.el_cnt >= 1)) {
      rc = ProcessAsyncReq (dds_ptr, &(WRK.async_req_que[WRK.out_ptr]));
   }

   /* if poll request not already sent, then send poll one */
   if (rc != SEND_REQ_POLL_SENT)
   {
      PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg,
                  (WRK.conn_ctrl_save | CONN_CTRL_POLL_REQ));
   }

   RESTORE_INTERRUPTS(saved_intr_level);

   return;

}                                                        /* SendNextAS */
/*PAGE*/
/*
 * NAME: SendSS()
 *                                                                  
 * FUNCTION: send synchronous status to 
 *           the control unit
 *           
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

void SendSS (DDS_DATA *dds_ptr, uchar ss_code, uchar ss_parm1, 
             uchar ss_parm2)
{
   C327TRACE5 ("SDSS", dds_ptr, ss_code,  ss_parm1, ss_parm2);

   /* disconnect the timer if it is doing device timing */
   if (WRK.timer_type_2 == TIMER_TYPE_DEVICE_TIMING)
      SetTimer (dds_ptr, 2, TIMER_TYPE_NONE, 0);

   /* set synchronous status values in adapter mem */
   PUTC_BUSMEM (dds_ptr, offsetof( tca_t, dpsstat), UNACK);
   PUTC_BUSMEM (dds_ptr, offsetof( tca_t, dssv),    ss_code);
   PUTC_BUSMEM (dds_ptr, offsetof( tca_t, dssp1),   ss_parm1);
   PUTC_BUSMEM (dds_ptr, offsetof( tca_t, dssp2),   ss_parm2);
   PUTC_BUSMEM (dds_ptr, offsetof( tca_t, dssp3),   0);

   /* RAS.cnt_sync_req++                              update RAS counter */

   WRK.interface_state = CONNECTED_A_IDLE;        /* change interface state */

   SendNextAS (dds_ptr);   /* if possible, send next async request from que */

   C327PERF( 0x260 );

   return;
}                                                  /* end SendSS */
/*PAGE*/
/*
 * NAME: XmitData()
 *                                                                  
 * FUNCTION: transfer transmit data to the adapter mem
 *           
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

void XmitData (DDS_DATA *dds_ptr, int session_index, boolean restart_data)
{
   int              tca_offset;
   int              tca_length;
   int              tca_test_request_incr;
   int              move_len;
   boolean          temp_write_in_progress;
   unsigned char    tca_flag_bits;

   C327TRACE4 ("XMTB", dds_ptr, session_index, restart_data);

   tca_offset =
                (GETC_BUSMEM (dds_ptr, offsetof(tca_t, cudp_msb)) << 8) |
                 GETC_BUSMEM (dds_ptr, offsetof(tca_t, cudp_lsb));

   tca_length =
                (GETC_BUSMEM (dds_ptr, offsetof(tca_t, cufrp3)) << 8) |
                 GETC_BUSMEM (dds_ptr, offsetof(tca_t, cufrp4));

   tca_flag_bits         = 0x00;
   tca_test_request_incr = 0;

   /* Test for halt session to be pending for this session */
   if (TBL(session_index).halt_pending == TRUE)
   {
      /* Put Halt session dummy data in adapter mem */
      PUTC_BUSMEM (dds_ptr, tca_offset+0, 0x00);
      PUTC_BUSMEM (dds_ptr, tca_offset+1, 0x07);
      PUTC_BUSMEM (dds_ptr, tca_offset+2, (F_O_M | L_O_M));
      PUTC_BUSMEM (dds_ptr, tca_offset+3, 0x00);
      PUTC_BUSMEM (dds_ptr, tca_offset+4, 0x60);
      PUTC_BUSMEM (dds_ptr, tca_offset+5, 0x40);
      PUTC_BUSMEM (dds_ptr, tca_offset+6, 0x40);

      return;
   }

   /* Test if the write data needs to started from the beginning */
   if (restart_data == TRUE)
   {
      TBL(session_index).write_data_addr =
         TBL(session_index).write_orig_data_addr;
      TBL(session_index).write_data_len =
         TBL(session_index).write_orig_data_len;
      TBL(session_index).write_in_progress = FALSE;
   }

   /* Set up local copy of the write in progress value */
   temp_write_in_progress = TBL(session_index).write_in_progress;

   /*
   ** Handle 'Test Request' key
   */

   /* check if first of data buffer & device head Test Request */
   if ((temp_write_in_progress == FALSE) &&
       (*TBL(session_index).write_data_addr == 0xF0))
   {
      /* skip the three chars of the device head test request */
      TBL(session_index).write_data_addr = (char *)
         (TBL(session_index).write_data_addr + 3);

      TBL(session_index).write_data_len  -= 3;

      tca_length -= 4;

      if (WRK.local_attach == FALSE)           /* check if BSC is attached */
      {
         /* Handle 'test request' for BSC */
         tca_flag_bits |= BSC_TRANSPAR | BSC_TEST_REQ;
      }
      else
      {
         /* Build 'test request' message in adapter mem buffer */
         PUTC_BUSMEM (dds_ptr, tca_offset+4, 0x01);
         PUTC_BUSMEM (dds_ptr, tca_offset+5, 0x6C);
         PUTC_BUSMEM (dds_ptr, tca_offset+6, 0x61);
         PUTC_BUSMEM (dds_ptr, tca_offset+7, 0x02);

         /* Set incrementer to jump over the 'test request' message */
         tca_test_request_incr = 4;
      }
   }

   /*
   ** Set up to move data from device head buffer to adapter mem buffer
   */

   /* check if BSC is attached and BSC transparency wanted */
   if ((WRK.local_attach == FALSE) && (WRK.non_sna == TRUE) &&
       (TBL(session_index).BSC_transparency == TRUE))
   {
      /* set transparency for BSC */
      tca_flag_bits |= BSC_TRANSPAR;
   }

   /* Decrement adapter mem buffer length to allow for transparency char */
   if (tca_flag_bits & BSC_TRANSPAR)
      tca_length -= 1;

   /* Determine which buffer length to use for data move */
   if (TBL(session_index).write_data_len > (tca_length-4))
   {
      /* data move length = (size of adptr mem buf) - (length & flag chars)*/
      move_len = tca_length - 4;

      /* Determine what flag bits to set in adapter mem buffer header */
      if (temp_write_in_progress == FALSE)
      {
         /* Set local 'write in progress' flag for Net ID Table */
         temp_write_in_progress = TRUE;

         /* Set local 'first of message' flag for adapter mem data header */
         tca_flag_bits |= F_O_M;
      }
   }
   else
   {
      /* Use the write data length */
      move_len = TBL(session_index).write_data_len;

      /* Set local 'last of message' flag for adapter mem data header */
      tca_flag_bits |= L_O_M;

      /* Determine what flag bits to set in adapter mem buffer header */
      if (temp_write_in_progress == TRUE)
      {
         /* Reset local 'write in progress' flag for Net ID Table */
         temp_write_in_progress = FALSE;
      }
      else
      {
         /* Set local 'first of message' flag for adapter mem buffer header */
         tca_flag_bits |= F_O_M;
      }
   }

   /* 
   ** Move data from device head buffer to adapter mem 
   */

   PUTC_BUSMEML (dds_ptr, tca_offset + 4 + tca_test_request_incr,
                 TBL(session_index).write_data_addr, move_len);

   /* update status and counts to reflect write */
   TBL(session_index).write_in_progress = temp_write_in_progress;

   TBL(session_index).write_data_addr = 
            (char *)(TBL(session_index).write_data_addr + move_len);

   TBL(session_index).write_data_len   -= move_len;

   /* Incr bufr length include length and flag chars plus the */
   /* 'test request' message if added. */
   
   move_len += (4 + tca_test_request_incr);

   /* Set length and flags */
   PUTC_BUSMEM (dds_ptr, tca_offset,   ((move_len >> 8) & 0xFF));
   PUTC_BUSMEM (dds_ptr, tca_offset+1, (move_len & 0xFF));
   PUTC_BUSMEM (dds_ptr, tca_offset+2, tca_flag_bits);
   PUTC_BUSMEM (dds_ptr, tca_offset+3, 0x00);

   C327TRACE4("XMTE", dds_ptr, move_len, tca_length);

   C327TRACE3("XMT2",tca_flag_bits,TBL(session_index).write_data_len);

   return;

}                                                           /* end XmitData */
/*PAGE*/

/*
 * NAME: xfer_data_SNA ()
 *                                                                  
 * FUNCTION: This routine transfers one segment of SNA data to the adapter
 *           card. It will build the link header, add the TH if not the first
 *           segment, and write the amount of user data requested.
 *           
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
 * RETURNS: none.
 *          
 */  

 void xfer_data_SNA (dds_ptr,sess_indx,length,tca_offset,header_type,th)

 DDS_DATA     *dds_ptr;      /* device data structure                 */
 int          sess_indx;     /* which session is sending data         */
 int          length;        /* length of the data to write           */
 int          *tca_offset;   /* address in tca buffer to put data     */
 uchar        header_type;   /* hex value for which segment. This can be
                                first,middle,last,or only.            */
 char         *th;           /* the six byte TH (if present)          */

{
   char    link_header[4];     /* The link header for control unit    */
   int     data_length=length; /* The actual length of the user data to xfer */

   /* build the link header */
   link_header[0] = (length >> 8);   /* MSB of length */
   link_header[1] = (length);        /* LSB of length */
   link_header[2] = header_type;     /* MSB of header - the segment type */
   link_header[3] = 0x00;            /* LSB of header */

   /* xmit header to adapter */
   PUTC_BUSMEML (dds_ptr,*tca_offset,link_header,HEADER_LENGTH);
   *tca_offset += HEADER_LENGTH;  /* update address */
   data_length -= HEADER_LENGTH;  /* less room for data now */

   /* check for need to xmit the TH */
   if (TBL(sess_indx).write_in_progress) {
      PUTC_BUSMEML (dds_ptr,*tca_offset,th,TH_LENGTH);
      *tca_offset += TH_LENGTH;
      data_length -= TH_LENGTH;  
   }

   /* now we can xmit the rest of the data */
   PUTC_BUSMEML (dds_ptr,*tca_offset,TBL(sess_indx).write_data_addr,
                 data_length);
   *tca_offset += data_length;  /* update address */

}  /* end of xfer_data_SNA */
/*PAGE*/
/*
 * NAME: segment_SNA_data ()
 *                                                                  
 * FUNCTION: This function will perform SNA segmenting for the user data
 *           until the TCA buffer is full or the write is complete. 
 *           There are four main cases:
 *           case 1: A PDAT was issued and we can fit all data in one segment.
 *           case 2: We can't fit all data in one segment, but there was a
 *                   PDAT, so we do a start segment,fill the seg. and keep going
 *           case 3: It is the last segment. We set last segment, use remaining
 *                   data, and signify that the write is complete.
 *           case 4: some middle segment. We set middle segment. We fill the
 *                   segment, and we keep on going.
 *           
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
 * RETURNS: none. 
 *          
 */  
void segment_SNA_data (dds_ptr, sess_indx)

DDS_DATA   *dds_ptr;   /* device data structure */
int        sess_indx;  /* the index of session that is doing the write */

{

   int     tca_offset;     /* the address of the tca buffer on the adapter */
   int     max_num_segs;   /* maximum number of segments in buffer */ 
   int     max_seg_length; /* maximum length that a segment can be */
   int     data_length;    /* length of the data to xmit           */
   short   segments_used;  /* number of segments used so far       */
   boolean length_odd;    /* If segment length is even do nothing special. If
                              it is odd, we increment to even boundary between
                              segments */


   /* get address of the tca data buffer */
   tca_offset = ((GETC_BUSMEM (dds_ptr,offsetof (tca_t,cudp_msb)) << 8) | 
                 GETC_BUSMEM (dds_ptr,offsetof (tca_t,cudp_lsb)));

   /* get maximum number of segments allowed */
   max_num_segs = ((GETC_BUSMEM (dds_ptr,offsetof (tca_t,cufrp1)) << 8) | 
                 GETC_BUSMEM (dds_ptr,offsetof (tca_t,cufrp2)));

   /* get maximum length allowed for each segment */
   max_seg_length = ((GETC_BUSMEM (dds_ptr,offsetof (tca_t,cufrp3)) << 8) | 
                 GETC_BUSMEM (dds_ptr,offsetof (tca_t,cufrp4)));

   /* determine if even or odd length */
   if ( (max_seg_length % 2) != 0) {
      length_odd = TRUE;
   }
   else {
      length_odd = FALSE;
   }

   /* Determine if this is the first inbound traffic for this session. If 
      this is the case, calculate the maximum allowable RU size. The size
      will be transmitted (and the flag reset) after the inbound data is
      sent to the adapter. */
   if (TBL(sess_indx).first_inbound) 
     /* max size formula with TH in all segments */
     TBL(sess_indx).MaxRuSize = max_num_segs * (max_seg_length - 
                                HEADER_LENGTH - TH_LENGTH) - RH_LENGTH;

   data_length = TBL(sess_indx).write_data_len;  /* length of user buffer */
   C327TRACE4 ("sgm2",max_num_segs,max_seg_length,data_length);

   /***********************************************************************
    *** CASE 1: This is the only segment. This is the case where we were***
    *** issued a PDAT and all of the data will fit in one segment.      ***
    ***********************************************************************/
    if (( max_seg_length >= (data_length + HEADER_LENGTH)) &&
        !(TBL(sess_indx).write_in_progress)) {

       C327TRACE1 ("pdt2");
       data_length += HEADER_LENGTH;  /* adjust for header */
       
       /* reset bits 4 and 5 in TH */
       *(TBL(sess_indx).write_data_addr) &= TH_MASK;
       /* set bits 4 and 5 in the TH for whole segment */
       *(TBL(sess_indx).write_data_addr) |= TH_WHOLE_SEG;

       /* write the data and header to adapter */
       xfer_data_SNA (dds_ptr,sess_indx,data_length,&tca_offset,HEAD_ONLY_SEG);
       
       /* if the length is odd, skip to even memory location */
       if (length_odd) {
          tca_offset++;
       }

       C327TRACE1 ("pdt3");

       /* we're done now */
       /* The function complete is done in RDAT/PDAT code.
          Since write_in_progress never set, The tca head  will then be 
          interrupted that the write command is complete */ 
       return;
    }
    else { /* The data will not be the only segment */

       segments_used = 0;  /* initialized segments used variable */

      /***********************************************************************
       *** CASE 2: This is the first segment. This is the case where we    ***
       *** were issued a PDAT and the data will need to be put in multiple ***
       *** segments.                                                       ***
       ***********************************************************************/
       if (!(TBL(sess_indx).write_in_progress)) {   /* PDAT */

          /* reset bits 4 & 5 in TH (first byte out of 6 in data stream) */
          *(TBL(sess_indx).write_data_addr) &= TH_MASK;
          /* store the TH in the NET_TBL */
          memcpy (TBL(sess_indx).trans_header,TBL(sess_indx).write_data_addr,
                  TH_LENGTH);
          /* set bits 4 and 5 in the TH for the Starting segment */
          *(TBL(sess_indx).write_data_addr) |= TH_FIRST_SEG;

          /* transfer header and data to first segment in adapter memory */
          xfer_data_SNA (dds_ptr,sess_indx,max_seg_length,&tca_offset,
                         HEAD_START_SEG);

          /* if the segment length is odd, skip to even mem. location */
          if (length_odd) {
             tca_offset++;
          }

          /* indicate that a write is in progress */
          TBL(sess_indx).write_in_progress = TRUE; 

          /* make all adjustments */
          data_length -= (max_seg_length - HEADER_LENGTH); /* data remaining */
          TBL(sess_indx).write_data_len = data_length;
          TBL(sess_indx).write_data_addr += (max_seg_length - HEADER_LENGTH);
          segments_used ++; /* one segment used */
       } /* end first segment */

       /* Now we must loop until either we run out of user data or we run
          out of segments. If it is tha latter, we will continue after we get
          the next RDAT. */
       while ((data_length > 0) && (segments_used < max_num_segs)) {
       
     /***********************************************************************
      *** CASE 3: This is a middle segment. This is the case where we     ***
      *** were issued an RDAT/PDAT and the remaining data (+ cu header    ***
      *** + TH) will not all fit in the current segment.                  ***
      ***********************************************************************/
          if (max_seg_length < (data_length + HEADER_LENGTH + TH_LENGTH)) {
             /* set bits 4 and 5 in TH for middle segment. we can just mask
                because the middle is zero anyway */
             TBL(sess_indx).trans_header[0] &= TH_MASK;
          
             /* transfer header,TH,and user data to next seg. on adapter card */
             xfer_data_SNA (dds_ptr,sess_indx,max_seg_length,&tca_offset,
                            HEAD_MIDDLE_SEG,TBL(sess_indx).trans_header);
          
             if (length_odd) {
                tca_offset++;
             }

             /* make all adjustments */
             data_length -= (max_seg_length - HEADER_LENGTH - TH_LENGTH);
             TBL(sess_indx).write_data_len = data_length;
             TBL(sess_indx).write_data_addr += (max_seg_length - HEADER_LENGTH -
                                                TH_LENGTH );
             segments_used ++;
          }
          else {
      /***********************************************************************
       *** CASE 4: This is the last segment. This is the case where we     ***
       *** were issued an RDAT/PDAT and the remaining data (+ cu header    ***
       ***  + TH) will all fit in the current segment.                     ***
       ***********************************************************************/
             /* set bits 4 and 5 in TH for last segment */
             TBL(sess_indx).trans_header[0] &= TH_MASK;
             TBL(sess_indx).trans_header[0] |= TH_LAST_SEG;
     
             /* adjust data length remaining for the cu header and TH */
             data_length += (HEADER_LENGTH + TH_LENGTH );
     
             /* transfer remaining data, header and TH to next seg. on card */
             xfer_data_SNA (dds_ptr,sess_indx,data_length,&tca_offset,
                            HEAD_END_SEG,TBL(sess_indx).trans_header);
             /* if odd segment length, skip to even mem. loc. */
             if (length_odd) {
                tca_offset++;
             }

             /* set data length to zero to indicate we are finished. We have
                already made the calculation that this is the case and there is
                is no need to waste two additions */
             data_length = 0;

             /* reset write_in_progress flag. The operation is complete. Again,
                the tca head will be interrupted after the function complete
                is issued in the PDAT/RDAT code */
             TBL(sess_indx).write_in_progress = FALSE;
          } /* end of last segment */
       } /* end of while loop */
       
    } /* end of else clause */
} /* end of function segment_SNA_data */
/*PAGE*/
/*
 * NAME: InitCard()
 *                                                                  
 * FUNCTION: initialize and start the adapter
 *           
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

void InitCard (DDS_DATA *dds_ptr)
{
   uchar           temp_intr_stat;
   int             ndx;

   C327TRACE2 ("INIT", dds_ptr);

   /***** CLEAN UP WORK AREA *****/
   WRK.el_cnt           = 0;              /* No. of Send_AS requests queued */
   WRK.in_ptr           = 0;              /* Input pointer to Send_AS Q     */
   WRK.out_ptr          = 0;              /* Output pointer to Send_AS Q    */
   WRK.cusyn_save       = 0;              /* CU Syncronization value        */
   WRK.dpastat_save     = ACK;            /* AS acknowledgement save char   */
   WRK.aeep_outstanding = FALSE;          /* aeep outstanding flag */
   WRK.cu_error_o       = 0;              /* WCUS Control Unit Error value  */
   WRK.interface_state  = CONNECTED_A_IDLE;/* Interface State Variable      */
   WRK.device_state     = INIT_IN_PROCESS; /* Device State Variable         */
   WRK.reset_in_process = TRUE;           /* ignore 1st Read ID cmd from cu */
   WRK.waiting_for_restart = FALSE;
   WRK.WCUS_30_pending  = FALSE;
   WRK.WCUS_10_received = FALSE;

   /* reset the net id table broadcast flags */
   for (ndx = 0; ndx < WRK.num_sess; ndx++)
   {
      TBL(ndx).ack_pending       = FALSE;/* not waiting on emul to ack data */
      TBL(ndx).program_check     = FALSE;/* cu has not issued program check */
      TBL(ndx).kbd_reset_pending = FALSE;/* reset not pressed not processed */
      TBL(ndx).cmd_chaining      = FALSE;/* cu is not chaining 3270 cmds    */
   }

   /* reset pending intr */
   temp_intr_stat = GETC_BUSIO (dds_ptr, adapter_intr_stat_reg);

   PUTC_BUSIO (dds_ptr, adapter_intr_stat_reg, temp_intr_stat);

   /* tell we are smart dev*/
   PUTC_BUSIO (dds_ptr, adapter_term_id_reg, ((~0x01) & 0xFF));

   C327_BZERO (dds_ptr, HDW.bus_mem_size);            /* init card memory */

   /* now set a few chars in adapter mem, first tell cu we are DFT with TCA */
   PUTC_BUSMEM (dds_ptr, offsetof( tca_t, dtid1), 0x02);

   PUTC_BUSMEM (dds_ptr, offsetof( tca_t, dbuf_msb),
                ((HDW.bus_mem_size >> 8) & 0xFF));

   PUTC_BUSMEM (dds_ptr, offsetof( tca_t, dbuf_lsb),
                (HDW.bus_mem_size & 0xFF));

   PUTC_BUSMEML (dds_ptr, (int)sizeof(tca_t), (char *)DEV.dev_info.dev_array,
                 (int)sizeof(DEV.dev_info.dev_array) );

   c327ConnectWait (dds_ptr, 5);

   /* start timer for this card to detect inactive cu */
   SetTimer (dds_ptr, 1, TIMER_TYPE_CU_INACTIVE, 30000);

   /* enable the COAX and interrupts and dft mode */
   WRK.conn_ctrl_save = CONN_CTRL_DFT_MODE | CONN_CTRL_ENABLE_COAX;

   PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg, WRK.conn_ctrl_save);

   WRK.restart_in_progress = FALSE;

   return;
}                                                       /* end InitCard */
/*PAGE*/
/*
 * NAME: ShutDown()
 *                                                                  
 * FUNCTION: clean up during close, open timeout, 
 *           cu dead timeout, or second restart
 *
 *                  CAUTION!!    CAUTION!!    CAUTION!!    
 *           this routine must be entered only with common locked
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

void ShutDown(DDS_DATA *dds_ptr)
{

   int         sessndx;

   C327TRACE2 ("SHUT", dds_ptr);

   /* force halt for all sessions */
   for (sessndx = 0; sessndx < WRK.num_sess; sessndx++)
   {
      TBL(sessndx).network_id.adapter_code = 0;
      TBL(sessndx).network_id.session_code = 0;

      if ( (int)TBL(sessndx).lt_state > (int)LT_NO_SESSION )
      {
         TBL(sessndx).lt_state = LT_OFFLINE;
         C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);
      }
   }

   /* last close for this adapter requires that card be shut down   */
   if (WRK.num_opens == 0)
   {
      c327Disconnect (dds_ptr);                 /* disconnect the adapter */

      /* reset all our flags, state machines, timers, etc. */
      bzero ((void *)&(WRK), sizeof (DDS_WRK_SECTION));

      WRK.num_sess = DEV.netid_table_entries; /* may be reduced by cu alloc */

      /* remove this adapter from being in use */
      CMNADP(HDR.minor_number).dds_ptr    = NULL;

      CMNMIS.adapters_in_use--;

      /* kill the timers for this adapter */
      SetTimer (dds_ptr, 1, TIMER_TYPE_NONE, 0);
      SetTimer (dds_ptr, 2, TIMER_TYPE_NONE, 0);

   } /* end of special activity for last close to this adapter */

   return;

} /* end ShutDown */
/*PAGE*/
/*
 * NAME: ValidNetID()
 *                                                                  
 * FUNCTION: local routine to validate the netid 
 *           passed to Write, Send, and Halt
 *           As a by-product, return the dds_ptr, 
 *           and session_index
 *           
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

int 
ValidNetID (NETWORK_ID network_id, DDS_DATA **dds_ptr_ptr, int *session_ptr)
{
   int saved_intr_level;
   DDS_DATA    *dds_ptr;

   DISABLE_INTERRUPTS(saved_intr_level);

   /* check the netid adapter code */
   if ((network_id.adapter_code<1) || (MAX_MINORS < network_id.adapter_code))
   {
      RESTORE_INTERRUPTS(saved_intr_level);
      return (1);
   }

   dds_ptr = CMNADP(network_id.adapter_code-1).dds_ptr;  /* get the DDS ptr */

   RESTORE_INTERRUPTS(saved_intr_level);

   if (dds_ptr == NULL)                         /* do we have a good dds   */
      return(2);                                /* NO, error return        */

   *dds_ptr_ptr = dds_ptr;                      /* YES return it to caller */

   *session_ptr = network_id.session_code;      /* check netid session code */

   if ((*session_ptr < 1) || (WRK.num_sess < *session_ptr))
      return (3);

   (*session_ptr)--;

   /* does session have a matching netid ? */
   if((TBL(*session_ptr).network_id.adapter_code != 
       network_id.adapter_code) || 
      (TBL(*session_ptr).network_id.session_code != network_id.session_code))
      return (4);

   return (0);                                   /* say that all is well    */

}                                                 /* end ValidNetID         */
/*PAGE*/
/*
** service a device head request to send status to the remote system
*/

int dftnsSend (NETWORK_ID network_id, int status_desc)
{
   DDS_DATA        *dds_ptr;
   int             session_index;
   int             validnetid_code;
   ASYNC_REQ       async_req;
   int             sessndx;

   C327PERF( 0x0300 );
   C327PERF( 0x0355 );

   C327TRACE4 ("SENB", network_id.adapter_code,
               network_id.session_code, status_desc);

   if ((validnetid_code = ValidNetID(network_id,&dds_ptr,&session_index)) != 0)
   {                                            /* check for invalid net id */
      C327PERF( 0x0301 );

      return (RC_INVAL_NETID);
   }

   if ((WRK.open_state != DD_OPENED ) || (WRK.device_state <= ON_LINE_TO_CU))
   {        /* adapter is not open and/or device is not on line to the host */
      C327TRACE3 ("sene", WRK.open_state, WRK.device_state);

      C327PERF( 0x0301 );

      return (RC_CMD_NOT_VALID);
   }
   
   /* If SNA read, just simply do a function complete */
   if (status_desc == TCA_FC) {
      /* If buffer management and a bind was received, do not send the FC */
      if (!(TBL(session_index).bind_recvd)) {
         SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);
      }
      return (RC_OK_NONE_PEND);
   }

   if ((TBL(session_index).ack_pending == FALSE) &&
       (status_desc != SEND_STAT_KBD_RESET))
   {            /* no ack pending, and this is not KBD reset, so do nothing */
      C327PERF( 0x0301 );

      return (RC_OK_NONE_PEND);
   }

   switch (status_desc)           /* validate and send the status requested */
   {

      case SEND_STAT_ACK:
         if (TBL(session_index).lt_state == LT_READ_LOCK)
         {
            TBL(session_index).ack_pending = FALSE;

            TBL(session_index).lt_state = LT_LOCK;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
            SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);
         }

         if (TBL(session_index).lt_state == LT_READ_LOCK_DEF)
         {
            TBL(session_index).ack_pending = FALSE;

            BuildAsyncReq((uchar)AESTAT_00, (uchar)0, (uchar)0,
                (uchar)0, (uchar)AESTAT,TBL(session_index).lt_id, &async_req);

            TBL(session_index).lt_state = LT_LOCK;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
            SendAS (dds_ptr, &async_req);

         }

         if (TBL(session_index).lt_state == LT_READ_LOCK_PRT)
         {

            TBL(session_index).lt_state = LT_READ_LOCK_DEF;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
            SendSS (dds_ptr, (uchar)SS_FCDEF, (uchar)FCDEF_01, (uchar)0);


            /* Done with current session */
            for (sessndx = 0; sessndx < WRK.num_sess; sessndx++)
            {      /* Determine which Net ID Table entry this CTCSS is for. */

               if (TBL(sessndx).lt_state == LT_WRT_CMD_REDO)
               {
                  TBL(sessndx).lt_state = LT_ONLINE;
                  C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);

                  TBL(sessndx).write_in_progress = FALSE; /* reset wrt flg */

                  /* Re-Request of control unit for Inbound Event Pending */
                  BuildAsyncReq ((uchar)0, (uchar)0, (uchar)0, (uchar)0,
                        (uchar)AEEP, (uchar)TBL(sessndx).lt_id, &async_req);

                  SendAS (dds_ptr, &async_req);

                  break;
               }
            }
         }
         break;

      case SEND_STAT_OKDATA:
         if (TBL(session_index).lt_state == LT_RM_LOCK)
         {
            TBL(session_index).ack_pending = FALSE;

            TBL(session_index).lt_state = LT_RM_LOCKED;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
            SendSS (dds_ptr, (uchar)SS_FCIR, (uchar)FCIR_00, (uchar)0);

         }
         break;

      case SEND_STAT_KBD_RESET:
         /* RAS.cnt_rst_key_i++ */

         if (TBL(session_index).cmd_write_pending == TRUE)
            TBL(session_index).kbd_reset_pending = TRUE;

         if (TBL(session_index).program_check == TRUE)
         {
            TBL(session_index).program_check = FALSE;
            InterruptDH (dds_ptr, session_index, INTR_UNSOL,
                         OR_CHK_STAT_MSK, 0);
         }
         break;

      case SEND_STAT_PRINT_COMPLETE:
         if (TBL(session_index).lt_state == LT_READ_LOCK_DEF)
         {
            TBL(session_index).ack_pending = FALSE;

            BuildAsyncReq((uchar)AESTAT_00, (uchar)0, (uchar)0, (uchar)0,
                          (uchar)AESTAT,TBL(session_index).lt_id, &async_req);

            TBL(session_index).lt_state = LT_LOCK;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
            SendAS (dds_ptr, &async_req);

         }
         break;

      case SEND_STAT_DEV_NOT_SEL:
         if ((TBL(session_index).lt_state == LT_READ_LOCK) ||
             (TBL(session_index).lt_state == LT_READ_LOCK_PRT))
         {
            TBL(session_index).ack_pending = FALSE;

            TBL(session_index).lt_state = LT_LOCK;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
            SendSS (dds_ptr, (uchar)SS_FCSE, (uchar)FCSE_04, (uchar)0);

         }

         if (TBL(session_index).lt_state == LT_READ_LOCK_DEF)
         {
            TBL(session_index).ack_pending = FALSE;

            BuildAsyncReq ( (uchar)AESTAT_04, (uchar)0, (uchar)0, (uchar)0,
               (uchar)AESTAT, (uchar)TBL(session_index).lt_id, &async_req);

            TBL(session_index).lt_state = LT_LOCK;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
            SendAS (dds_ptr, &async_req);

         }
         break;

      case SEND_STAT_BUFFER_ERR:
         if (TBL(session_index).lt_state == LT_READ_LOCK) 
         {
            TBL(session_index).ack_pending = FALSE;

            TBL(session_index).lt_state = LT_LOCK;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
            SendSS (dds_ptr, (uchar)SS_FCSE, (uchar)FCSE_06,
                    (uchar)SS_BUF_ERROR);

         }

         if ((TBL(session_index).lt_state == LT_READ_LOCK_DEF) ||
             (TBL(session_index).lt_state == LT_READ_LOCK_PRT))
         {
            TBL(session_index).ack_pending = FALSE;

            BuildAsyncReq ( (uchar)AESTAT_05, (uchar)SS_BUF_ERROR,
                            (uchar)0, (uchar)0, (uchar)AESTAT,
                            (uchar)TBL(session_index).lt_id, &async_req);

            TBL(session_index).lt_state = LT_LOCK;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
            SendAS (dds_ptr, &async_req);

         }
         break;

      case SEND_STAT_BAD_CMD:
         if (TBL(session_index).lt_state == LT_READ_LOCK) 
         {
            TBL(session_index).ack_pending = FALSE;

            TBL(session_index).lt_state = LT_LOCK;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
            SendSS (dds_ptr, (uchar)SS_FCSE, (uchar)FCSE_03,
                    (uchar)SS_BAD_CMD);

         }
         if ((TBL(session_index).lt_state == LT_READ_LOCK_DEF) ||
             (TBL(session_index).lt_state == LT_READ_LOCK_PRT))
         {
            TBL(session_index).ack_pending = FALSE;

            BuildAsyncReq ( (uchar)AESTAT_02, (uchar)SS_BAD_CMD,
                            (uchar)0, (uchar)0, (uchar)AESTAT,
                            (uchar)TBL(session_index).lt_id, &async_req);

            TBL(session_index).lt_state = LT_LOCK;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
            SendAS (dds_ptr, &async_req);

         }
         break;

      case SEND_STAT_UNSUPP_CMD:
         if ((TBL(session_index).lt_state == LT_READ_LOCK) ||
             (TBL(session_index).lt_state == LT_READ_LOCK_PRT))
         {
            TBL(session_index).ack_pending = FALSE;

            TBL(session_index).lt_state = LT_LOCK;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
            SendSS (dds_ptr, (uchar)SS_FCSE, (uchar)FCSE_06,
                    (uchar)SS_UNS_CMD);

         }

         if (TBL(session_index).lt_state == LT_READ_LOCK_DEF)
         {
            TBL(session_index).ack_pending = FALSE;

            BuildAsyncReq ( (uchar)AESTAT_06, (uchar)SS_UNS_CMD, (uchar)0,
                            (uchar)0, (uchar)AESTAT,
                            (uchar)TBL(session_index).lt_id, &async_req);

            TBL(session_index).lt_state = LT_LOCK;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
            SendAS (dds_ptr, &async_req);

         }
         break;

      default:
         C327PERF( 0x0301 );

         return (RC_INVAL_PARMS);
   }                                                          /* end switch */

   C327PERF( 0x0301 );

   return (RC_OK_NONE_PEND);
}                                                          /* end dftnsSend */
/*PAGE*/
/*
 * NAME: dftnsOpen()
 *                                                                  
 * FUNCTION: service device head request to 
 *           start communications with the control unit
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

int dftnsOpen (DDS_DATA *dds_ptr)
{
   unsigned char    temp_byte;
   int              ndx, rc = 0;
   int              maximum_waits;

   C327TRACE2 ("OPNB", dds_ptr );


   /*
   ** CAUTION! Code below is not re-entrant
   **
   ** 
   ** lock common area
   */

#ifdef _POWER_MP
   lock_write((complex_lock_t)&CMNMIS.common_locked);
#else
   lockl ( (lock_t *)&CMNMIS.common_locked, LOCK_SHORT );
#endif

   if (CMNADP(HDR.minor_number).dds_ptr == dds_ptr) /* this is NOT 1st open */
   {
      if (WRK.open_state == OPEN_PENDING)  /* is any other open in progress */
      {

#ifdef _POWER_MP
         lock_done((complex_lock_t)&CMNMIS.common_locked );
#else
         unlockl ( (lock_t *)&CMNMIS.common_locked ); /* unlock common area   */
#endif
         return (RC_OPEN_BUSY);
      }

#ifdef _POWER_MP
      lock_done((complex_lock_t)&CMNMIS.common_locked );
#else
      unlockl ( (lock_t *)&CMNMIS.common_locked );  /* unlock common area   */
#endif

      return (RC_OK_NONE_PEND);
   }

   /* check to see if the adapter is here */
   PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg, CONN_CTRL_INT_INH);

   temp_byte = GETC_BUSIO (dds_ptr, adapter_conn_ctrl_reg);

   if ( temp_byte != CONN_CTRL_INT_INH )
   {

#ifdef _POWER_MP
      lock_done((complex_lock_t)&CMNMIS.common_locked );
#else
      unlockl ( (lock_t *)&CMNMIS.common_locked );  /* unlock common area   */
#endif

      return (RC_ADAPTER_FAIL);
   }

   for (ndx = 0; ndx < HDW.bus_mem_size; ndx++)       /* is adapter mem ok? */
      PUTC_BUSMEM (dds_ptr, ndx, 0xAA);

   for (ndx = 0; ndx < HDW.bus_mem_size; ndx++)
   {
      if (GETC_BUSMEM (dds_ptr, ndx) != 0xAA)
      {

#ifdef _POWER_MP
         lock_done((complex_lock_t)&CMNMIS.common_locked );
#else
         unlockl ( (lock_t *)&CMNMIS.common_locked );  /* unlock common area */
#endif

         return (RC_ADAPTER_FAIL);
      }
   }

   for (ndx = 0; ndx < HDW.bus_mem_size; ndx++)
      PUTC_BUSMEM (dds_ptr, ndx, 0x55);

   for (ndx = 0; ndx < HDW.bus_mem_size; ndx++)
   {
      if (GETC_BUSMEM (dds_ptr, ndx) != 0x55)
      {

#ifdef _POWER_MP
         lock_done((complex_lock_t)&CMNMIS.common_locked );
#else
         unlockl ( (lock_t *)&CMNMIS.common_locked ); /* unlock common area  */
#endif

         return (RC_ADAPTER_FAIL);
      }
   }

   /*
   ** CAUTION! From this point on, we cannot take error exit without ShutDown
   */

   CMNADP(HDR.minor_number).dds_ptr = dds_ptr;   /* add DDS to common table */
   CMNMIS.adapters_in_use++;

   c327InitDDS (dds_ptr);                        /* initialize the DDS */
  
   WRK.num_opens = 1;

   WRK.num_sess = DEV.netid_table_entries;       /* determined by CU */
   WRK.open_state = OPEN_PENDING;

   /* 
   ** if first adapter, start timer routine 
   */
   if (CMNMIS.adapters_in_use == 1) 
   {
      timeout(dftnsTimer,0,(HZ/4));
   }

   InitCard (dds_ptr);                     /* start the adapter */

   maximum_waits = 300;                  /* wait until finished or time out */

   while ( (WRK.open_state == OPEN_PENDING) && (maximum_waits-- > 0))
   {
      delay ( (int)HZ/5 );               /* compute delay in clock ticks */
   }

   switch (WRK.open_state)
   {
      case DD_OPENED:
         rc = RC_OK_NONE_PEND;
         break;

      case OPEN_PENDING:
      case OPEN_TIMEOUT:
         rc = RC_OPEN_TIMEOUT;
         break;

      case NOT_OPENED:
      default:
         rc = RC_NON_SUPP_CU;
         break;
   }                                                          /* end switch */

   if (rc != RC_OK_NONE_PEND)
   {
      WRK.num_opens = 0;
      ShutDown (dds_ptr);
   }

#ifdef _POWER_MP
   lock_done((complex_lock_t)&CMNMIS.common_locked );
#else
   unlockl ( (lock_t *)&CMNMIS.common_locked );  /* unlock common area   */
#endif

   /*
   ** CAUTION! Code above is not re-entrant
   */

   return (rc);
}                                                          /* end dftnsOpen */
