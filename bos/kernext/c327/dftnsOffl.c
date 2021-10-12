static char sccsid[] = "@(#)93	1.34  src/bos/kernext/c327/dftnsOffl.c, sysxc327, bos411, 9430C411a 7/27/94 09:32:40";

/*
 * COMPONENT_NAME: (SYSXC327) c327 dft Offlevel handler
 *
 * FUNCTIONS:    AckAllCmd(), AckAsync(), ForceClose(), ProcessCTCCS(), 
 *     ProcessWCUS(), RecvCmd(), RecvData(), Restart(), StartOp(), 
 *     TimerSendES(), dftnsPIOError(), dftnsProcessInterrupt(), 
 *     dftnsProcessTimeout(), dftnsTimer()
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
** dftnsOffl - off-level interrupt entry points and their local subroutines
**
**
** Local: RecvData
** Local: ProcessCTCCS
** Local: RecvCmd
** Local: ProcessWCUS
** Local: AckAsync
** Local: StartOp
** Local: AckAllCmd
** Local: ForceClose
** Local: Restart
** called from c327OffLevel: dftnsProcessInterrupt
** called from c327OffLevel: dftnsProcessTimeout  
**
** Local: TimerSendES
** Timer may queue work for dftnsProcessTimeout: dftnsTimer
**
*/

#include <stddef.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>
#include <sys/intr.h>
#include <sys/types.h>
#include <sys/timer.h>
#include <sys/param.h>

#ifdef _POWER_MP
#include <sys/lock_def.h>
#else
#include <sys/lockl.h>
#endif

#include <sys/io3270.h>
#include "c327dd.h"
#include "dftnsDcl.h"
#include "dftnsSubr.h"
#include "tcadecls.h"

/*
** external data declarations
*/
extern COMMON_AREA dftnsCommon;
extern struct intr  c327offlstruct;
extern TIMR_QUE     timr_que;
/*PAGE*/
/*
 * NAME: RecvData()
 *                                                                  
 * FUNCTION: Process received data
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

void RecvData (DDS_DATA *dds_ptr, int session_index, RECV_CTRL recv_ctrl)
{

   unsigned char   temp_flag_bits;
   ASYNC_REQ       async_req;
   int             sessndx;
   int             tca_offset;
   int             tca_length;
   int             temp_buff_length;
   int             move_length;
   boolean         command_seg=FALSE; /* flag indicating First segment (SNA) */
   uchar           RU_command;        /* RU command byte (SNA only) */

   C327TRACE4 ("RECV", dds_ptr, session_index,recv_ctrl);

   if (TBL(session_index).halt_pending == TRUE)   /* is halt pending        */
   {
      /* This based on SEND_BYTE or READ_MODIFY */
      if ((recv_ctrl == 
          SEND_BYTE)&&(TBL(session_index).lt_state == LT_RM_LOCK)) 
         /* ack strt op ask 4 RDAT */
         SendSS (dds_ptr, (uchar)SS_FCIR, (uchar)FCIR_00, (uchar)0);
      else 
         /* Send Sync Status to cu */
         SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);

      return ;
   }

   if (TBL(session_index).apnd_to_buffer == FALSE) /* are we adding to bufr */
   {
      TBL(session_index).read_data_addr =          /* reset buffer ptr      */
          (char *) ((&TBL(session_index).read_buff_addr->buf_start) +
                    (TBL(session_index).read_buff_addr->dbhead.data_offset));

      TBL(session_index).read_buff_addr->dbhead.data_len = 0;
   
      TBL(session_index).read_buff_addr->dbhead.buf_ovrflw = FALSE; 

      if (recv_ctrl != JUST_DATA) /* is a command byte is being moved *******/
      {
         /* Move the 3270 command byte to the buffer */
         *TBL(session_index).read_data_addr = TBL(session_index).cmd_3270;

         TBL(session_index).read_data_addr++;

         TBL(session_index).read_buff_addr->dbhead.data_len = 1;

         TBL(session_index).apnd_to_buffer = TRUE;/* Set apnd_to_buffer flg */

         temp_flag_bits = 0x00;                 /* Reset F_O_M & L_O_M flgs */
      }
   }
   temp_buff_length = 
       TBL(session_index).read_buff_addr->dbhead.buff_size   -
       TBL(session_index).read_buff_addr->dbhead.data_offset -
       TBL(session_index).read_buff_addr->dbhead.data_len;

   if (recv_ctrl == JUST_DATA)          /* add to a partial or a new buffer */
   {
      /*
      ** Move data from adapter mem to Buffer
      */
      tca_offset =((int)GETC_BUSMEM(dds_ptr,(offsetof (tca_t, cudp_msb)))<<8) |
                   (int)GETC_BUSMEM(dds_ptr,(offsetof (tca_t, cudp_lsb)));

      tca_length = ((int)GETC_BUSMEM (dds_ptr, tca_offset)<<8) |
                    (int)GETC_BUSMEM (dds_ptr, tca_offset+1);

      tca_offset += 4;          /* Increment offset to bypass 4 byte header */

      tca_length -= 4;    /* Decrement adapter mem because of 4 byte header */
      
      /* 
      ** which buffer lngth for data move 
      */

      if (tca_length > temp_buff_length)
      {
         move_length = temp_buff_length;  /* Use dh bufr lngth for data move */

         /* Set overflow flag in buffer header */
         TBL(session_index).read_buff_addr->dbhead.buf_ovrflw = TRUE;
      }
      else
      {
         move_length = tca_length;      /* move length = size of adapter mem */

         /* clear overflow flag */
         TBL(session_index).read_buff_addr->dbhead.buf_ovrflw = FALSE;
      }

      GETC_BUSMEML (dds_ptr, tca_offset,
                    TBL(session_index).read_data_addr, move_length);

      TBL(session_index).read_data_addr = (char *)
                   (TBL(session_index).read_data_addr + move_length);

      TBL(session_index).read_buff_addr->dbhead.data_len += move_length;

      temp_flag_bits = GETC_BUSMEM (dds_ptr, tca_offset-2);

      /* 
       *  Set add_vrm flag
       */
      if ((WRK.non_sna == TRUE) && ((temp_flag_bits & L_O_M) == 0x00))
         TBL(session_index).apnd_to_buffer = TRUE;

   }

   if ( ((temp_flag_bits & L_O_M) != 0x00) || (recv_ctrl == SEND_BYTE) ||
         (TBL(session_index).read_buff_addr->dbhead.buf_ovrflw == TRUE) )
   {                                           /* do we hold or send data ? */
      TBL(session_index).apnd_to_buffer = FALSE;     /* Clear add_vrm flag */

      for (sessndx = 0; sessndx < WRK.num_sess; sessndx++)
      {
         if (TBL(sessndx).cmd_start_pending == TRUE)  /* any starts pending */
            AckCmd (dds_ptr, sessndx, INTR_SOL_START);

         if (TBL(sessndx).cmd_halt_pending  == TRUE)   /* any halts pending */
            TBL(sessndx).cmd_halt_pending = FALSE;
      }

      /* RAS.cnt_bytes_rec += */
      /*                 TBL(session_index).read_buff_addr->dbhead.data_len */
      /* RAS.cnt_blks_rec++                          Increment RAS Counters */

      TBL(session_index).ack_pending = TRUE;     /* ack Pending flag = true */

      if ( ( TBL(session_index).lt_state == LT_READ_LOCK)               &&
         ((GETC_BUSMEM (dds_ptr, (offsetof(tca_t, cuslvl_2))) & 0x01) != 0) &&
           ( TBL(session_index).sess_slow_dev == TRUE)                  &&
           ( TBL(session_index).cmd_3270 != WLCC_RM)                    &&
           ( TBL(session_index).cmd_3270 != WLCC_RD_BUFFER)             &&
           ( TBL(session_index).cmd_chaining == FALSE)        )

      {  /* is this a slow device response */

         if ( TBL(session_index).prt_bit_map
              && (TBL(session_index).start_print == TRUE)  )
         {
            /* check for printer session */
            TBL(session_index).lt_state = LT_READ_LOCK_PRT;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);

            /* Function complete Deffered print bit set */
            SendSS (dds_ptr, (uchar)SS_FCDEF, (uchar)FCDEF_01, (uchar)0);
         }
         else
         {
            TBL(session_index).lt_state = LT_READ_LOCK_DEF;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);

            /* Function complete Deffered - Print bit not set */
            SendSS (dds_ptr, (uchar)SS_FCDEF, (uchar)FCDEF_00, (uchar)0);
         }

         InterruptDH(dds_ptr, session_index, INTR_UNSOL,
                     OR_REC_DATA_AVAIL, TBL(session_index).cmd_chaining);

         for (sessndx = 0; sessndx < WRK.num_sess; sessndx++)
         {
            if (TBL(sessndx).lt_state == LT_WRT_CMD_REDO)
            {
               TBL(sessndx).lt_state = LT_ONLINE;/* Session State to online */
               C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);

               TBL(sessndx).write_in_progress = FALSE;        /* Reset flag */

               BuildAsyncReq((uchar)0, (uchar)0, (uchar)0, (uchar)0,
                           (uchar)AEEP, (uchar)TBL(sessndx).lt_id, &async_req);

               /* Re-Request of control unit for Inbound Event Pending */
               (void) SendAS (dds_ptr, &async_req);

               break;
            }
         }
      }
      else
      {
         InterruptDH(dds_ptr, session_index, INTR_UNSOL,
                     OR_REC_DATA_AVAIL, TBL(session_index).cmd_chaining);
      }
   }
   else
   {

     /*
      *  If a SNA Controller then want to send the data to HCON and send FC to 
      *  host every time we have data beause HCON is doing the segmenting for
      *  outbound data instead of us.
      */
     if (WRK.non_sna == FALSE) { 
         InterruptDH(dds_ptr, session_index, INTR_UNSOL,
                     OR_REC_DATA_AVAIL, 0);
        
        /* check if non-enhanced buffer management support. If not, we need to
           check for a bind or unbind. Bits 4 and 5 of the TH will be set to 
           whole segment and the command byte of the RU will be a 30 or 31.
           Also, The 2 and 3 bits of byte 0 of the RH will be non-zero and
           bit 0 of byte zero has to be zero (a request unit and not a response
           unit ) If this is the case, an FCIR will need to be issued instead of
           a function complete */

        if(((*(TBL(session_index).read_data_addr - move_length) & TH_WHOLE_SEG)
           == TH_WHOLE_SEG) && ((*(TBL(session_index).read_data_addr -
           move_length + 9) & 0x60) != 0x00) &&
           !(*(TBL(session_index).read_data_addr - move_length + TH_LENGTH)
           & 0x80)) {

           command_seg = TRUE; /* TH bits indicate that this is a whole seg. */
        }
        /* get the RU command byte */
        RU_command = *(TBL(session_index).read_data_addr - move_length + 
                       TH_LENGTH + RH_LENGTH);
        
        if ( !(WRK.enh_buffer_mgmt) && (command_seg) && 
             ((RU_command == BIND) || (RU_command == UNBIND))) {
           /* we must send an FCIR as a response to BIND or UNBIND */
           C327TRACE1("bind");
           SendSS(dds_ptr, (uchar)SS_FCIR, (uchar)0, (uchar)0);
           /* set the bind received flag */
           TBL(session_index).bind_recvd = TRUE;
        }
     }
     else {
           SendSS(dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0); 
     }
       
   }
   return;
}                                                      /* end RecvData */
/*PAGE*/
/*
 * NAME: ProcessCCTS()
 *                                                                  
 * FUNCTION: Process the CTCCS in either the 
 *           command chain WLCC or a regular CTCCS
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

void ProcessCTCCS (DDS_DATA *dds_ptr, int session_index)
{
   int             sessndx;
   ASYNC_REQ       async_req;

   C327TRACE4 ("CTWL", dds_ptr, session_index,
       TBL(session_index).lt_state);

   if ( ( (TBL(session_index).lt_state          == LT_WRT_CMD_ACK) ||
          (TBL(session_index).lt_state          == LT_WRT_LOCK_DA) ||
          (TBL(session_index).lt_state          == LT_WRT_LOCK_NA) ||
          (TBL(session_index).lt_state          == LT_WRT_RDAT_NA)   ) &&
          (TBL(session_index).cmd_write_pending == TRUE       )  )
   {
      /* 
      ** if this is a write cmd, deque it, ack it, 
      ** incr ras and reset flags  
      **
      ** RAS.cnt_bytes_xmit += TBL(session_index).write_orig_data_len
      ** RAS.cnt_blks_xmit++
      */

      AckCmd (dds_ptr, session_index, INTR_SOL_WRITE);

      TBL(session_index).kbd_reset_pending = FALSE;

      /* added for dual WLCC within one LOCK/CTCCS sequence */
      if ( ( TBL(session_index).lt_state == LT_WRT_RDAT_NA) &&
           ( GETC_BUSMEM (dds_ptr, ( offsetof (tca_t, cufrv) ) ) == WLCC ) )
      {
         TBL(session_index).lt_state = LT_LOCK;
         C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
         return;
      }
   }

   if ( ( (TBL(session_index).lt_state == LT_RM_LOCK    ) ||
      (TBL(session_index).lt_state     == LT_RM_LOCKED  ) ||
      (TBL(session_index).lt_state     == LT_RM_LOCKED_1) ||
      (TBL(session_index).lt_state     == LT_RM_LOCKED_2) ||
      (TBL(session_index).lt_state     == LT_RM_LOCKED_3)   ) &&
      (WRK.rm_cmd_write_pending        == TRUE                   )  )
   {
      /* 
      ** if this is a write cmd, deque it, ack it, 
      ** incr ras and reset flags
      **
      ** RAS.cnt_bytes_xmit += TBL(session_index).write_orig_data_len
      ** RAS.cnt_blks_xmit++ 
      */

      WRK.rm_write_in_progress = FALSE;

      WRK.rm_cmd_write_pending = FALSE;
   }

   TBL(session_index).lt_state = LT_ONLINE;        /* reset logical term st */
   C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);

   SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);

   if ( TBL(session_index).halt_pending == FALSE )
      InterruptDH(dds_ptr, session_index, INTR_UNSOL, OR_UNLOCK, 0);

   /* 
   ** lets tiptoe through the netid table looking to see if there
   ** is any session that has a write that needs restarting?
   */

   for ( sessndx = 0; sessndx < WRK.num_sess; sessndx++ )
   {
      if ( TBL(sessndx).lt_state == LT_WRT_CMD_REDO )
      {
         /*
         ** there is a write cmd to restart so, update state,
         ** reset buff len, reset buff offset, set write-in-progress flag,
         ** and re-request to cu for inbound event
         */

         TBL(sessndx).lt_state = LT_ONLINE;
         C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);
         TBL(sessndx).write_data_len  = TBL(sessndx).write_orig_data_len;
         TBL(sessndx).write_data_addr = TBL(sessndx).write_orig_data_addr;
         TBL(sessndx).write_in_progress = FALSE;

         BuildAsyncReq ((uchar)0, (uchar)0, (uchar)0, (uchar)0,
                        (uchar)AEEP, (uchar)TBL(sessndx).lt_id, &async_req);

         (void) SendAS (dds_ptr, &async_req);
      }
   }
   return;
} /* end ProcessCTCCS */
/*PAGE*/
/*
 * NAME: RecvCmd()
 *                                                                  
 * FUNCTION: Receive 3270 Command Byte
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

void RecvCmd (DDS_DATA *dds_ptr, int session_index, uchar type_cmd)
{
   C327TRACE5 ("RCMD", dds_ptr, session_index, type_cmd,
      TBL(session_index).cmd_3270 );

   if ( ( (TBL(session_index).lt_state == LT_RM_LOCK    ) ||
          (TBL(session_index).lt_state == LT_RM_LOCKED  ) ||
          (TBL(session_index).lt_state == LT_RM_LOCKED_1) ||
          (TBL(session_index).lt_state == LT_RM_LOCKED_2) ||
          (TBL(session_index).lt_state == LT_RM_LOCKED_3)   ) &&
          (WRK.rm_cmd_write_pending    == TRUE              )  )
   {

      WRK.rm_write_in_progress    = FALSE;   /* reset write in progress flg */

      WRK.rm_cmd_write_pending    = FALSE;   /* reset write pending flg     */

      TBL(session_index).lt_state = LT_LOCK;
      C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);

      /* 
      ** RAS.cnt_bytes_xmit += TBL(session_index).write_orig_data_len
      ** RAS.cnt_blks_xmit++ 
      */
   }

   switch ( TBL(session_index).cmd_3270 )
   {
      /*
      **     3270 command = Write - EWA or Write or WSF or EW or EAU
      **                             7E     F1       F3    F5     6F
      */
      case      WLCC_EWA       :
      case      WLCC_WRITE     :
      case      WLCC_WSF       :
      case      WLCC_EW        :
      case      WLCC_EAU       :
      {
         /* update logical terminal state to locked */
         if ( TBL(session_index).lt_state == LT_LOCK )
         {
            TBL(session_index).lt_state = LT_READ_LOCK;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
         }

         /* is there write cmd -- host write contention? */
         if ( (TBL(session_index).lt_state == LT_WRT_LOCK_DA) ||
              (TBL(session_index).lt_state == LT_WRT_LOCK_NA) )
         {
            /* yup so update state to read data lock and ack cmd */
            /* RAS.cnt_host_cont++ */
            TBL(session_index).lt_state = LT_READ_LOCK;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);

            InterruptDH (dds_ptr, session_index, INTR_UNSOL,
                         OR_WRITE_DISCARD, 0);

            AckCmd (dds_ptr, session_index, INTR_SOL_WRITE);
         }

         if ( type_cmd == WLCC )          /* is data available at this time */
         {
            if ( TBL(session_index).cmd_3270 == WLCC_EAU )
               /*
               ** send cmd to emulator one byte only, no more data to follow
               */
               RecvData (dds_ptr, session_index, SEND_BYTE);
            else
               /* send cmd to emulator as first byte, & wait for more */
               RecvData (dds_ptr, session_index, STORE_AND_WAIT);
         }
         else
         {
            /*
            ** send cmd to emulator as first byte, there is much more to come
            */
            RecvData (dds_ptr, session_index, JUST_DATA);
         }
         break;
      }

      /*
      **     3270 command = Read - read or read modified
      **                           F2      F6
      */
      case      WLCC_RD_BUFFER   :
      case      WLCC_RM          :
      {
         if (TBL(session_index).lt_state == LT_LOCK)
         {
            TBL(session_index).lt_state = LT_RM_LOCK;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);

            if ( type_cmd == WLCC )   /* is there data available? */
               RecvData (dds_ptr, session_index, SEND_BYTE); /* 1 byte only */ 
            else
               /* send cmd to emulator as first byte, & wait for more */
               RecvData (dds_ptr, session_index, JUST_DATA);
         }

         /* is there write cmd -- host write contention? */
         if ( (TBL(session_index).lt_state == LT_WRT_LOCK_DA) ||
              (TBL(session_index).lt_state == LT_WRT_LOCK_NA) )
         {
            if  ( type_cmd == WLCC )                 /* who sent this cmd ? */
            {
               if  ( TBL(session_index).cmd_3270 != WLCC_RM )
               {
                  /* RAS.cnt_host_cont++    assume Host wants all the data */

                  InterruptDH (dds_ptr, session_index, INTR_UNSOL,
                               OR_WRITE_DISCARD, 0);
                    
                  AckCmd (dds_ptr, session_index, INTR_SOL_WRITE);
                    
                  TBL(session_index).kbd_reset_pending = FALSE;

                  TBL(session_index).lt_state = LT_RM_LOCK; /* update state */
                  C327TRACE3 ("STAT", TBL(session_index).lt_state,
                              session_index);

                  /* send this cmd to the emulator as data, one byte only */
                  RecvData (dds_ptr, session_index, SEND_BYTE);

                  return;
               }
            }

            /* is there data available or not ? */
            if ( TBL(session_index).lt_state == LT_WRT_LOCK_DA )
               /*
               ** ack start op cmd
               */
               SendSS (dds_ptr, (uchar)SS_FCIR, (uchar)FCIR_01, (uchar)0);
            else
               /*
               ** ack start op cmd
               */
               SendSS (dds_ptr, (uchar)SS_FCIR, (uchar)FCIR_00, (uchar)0);

         }

         break;

      }

      /*
      ** OTHERWISE  default is non supported cu command
      */
      default   :
      {

         if (TBL(session_index).lt_state == LT_LOCK)  /* update term state */
         {
            TBL(session_index).lt_state = LT_READ_LOCK;
            C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
         }

         if (type_cmd == WLCC)                  /* is there data available */
            RecvData (dds_ptr, session_index, STORE_AND_WAIT);  /* one byte */
         else
            RecvData (dds_ptr, session_index, JUST_DATA ); /* > one byte */
            break;
      }
   }                                                       /* end switch */
}                                                          /* end RecvCmd */
/*PAGE*/
/*
 * NAME: ProcessWCUS (dds_ptr)
 *                                                                  
 * FUNCTION: Process start op WCUS command
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

void ProcessWCUS (DDS_DATA *dds_ptr)
{
   ASYNC_REQ       async_req;
   uchar           restart_pr_bit_map;
   uchar           restart_bit_map;
   int             cu_info;
   int             sessndx;
   int             cultandx;
   int             ddsndx;
   int             wcus_20_length;
   int             tca_ndx;
   int             wcus_20_address;
   uchar           wcus_type;
   boolean         slow_device;

   wcus_type = GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp1)));

   C327TRACE3 ("wcus", dds_ptr, wcus_type);

   switch (wcus_type)
   {
      /*********************************************************************/
      /*****          Start Operation  - WCUS - Machine Check          *****/
      /*********************************************************************/
      case MACHINE_CHECK   :

         C327TRACE1 ("mchk");

         /* prepare cu error to DD error */
         cu_info =
              ((int)GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp2))) <<8) |
               (int)GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp3)));

         cu_info = (cu_info & 0x0FFF) | MC_ERROR_MSK | BROADCAST_MSK;
         /*
         ** ack the start op command
         */
         SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);

         /* RAS.cnt_mach_chks++                    bump mach cks count      */

         BroadcastUnSol (dds_ptr,OR_CHK_STAT_MSK,cu_info);  /* acks to all */

         break;

      /*********************************************************************/
      /*****       Start Operation = WCUS = Communications Check       *****/
      /*********************************************************************/
      case COMO_CHECK  :

         C327TRACE1 ("cchk");

         /* prepare the cu error to DD error */
         cu_info =
               ((int)GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp2))) <<8) |
                (int)GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp3)));

         cu_info = (cu_info & 0x0FFF) | CC_ERROR_MSK | BROADCAST_MSK;
         /*
         ** ack the start op command
         */
         SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);

         /* RAS.cnt_como_chks++                         bump como cks count */

         BroadcastUnSol (dds_ptr,OR_CHK_STAT_MSK,cu_info);  /* acks to all */

         break;

      /*********************************************************************/
      /*****          START OPERATION  = wcus = Progam Check           *****/
      /*********************************************************************/
      case PROGRAM_CHECK  :

         C327TRACE1 ("pchk");

         for ( sessndx = 0; sessndx<WRK.num_sess; sessndx++)
         {
            /*
            ** find out which session had the program check
            */
            if ( TBL(sessndx).lt_id ==
               GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cultad))) )
            {
               break;
            }
         }

         /* prepare the cu error to DD error */
         cu_info =
               ((int)GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp2))) <<8) |
                (int)GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp3)));

         cu_info = (cu_info & 0x0FFF) | PC_ERROR_MSK;
         /*
         ** ack the start op command
         */
         SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);

         /* RAS.cnt_prog_chks++                        bump como cks count */

         TBL(sessndx).program_check = TRUE;
         /* pass the program check to the session */

         InterruptDH (dds_ptr, sessndx, INTR_UNSOL,
                      OR_CHK_STAT_MSK, cu_info);

         break;

      /*********************************************************************/
      /*****              START OPERATION = wcus = ready               *****/
      /*********************************************************************/
      case READY   :
         C327TRACE1 ("redy");
         WRK.WCUS_10_received = TRUE;

         /* is open in progress? */
         if ((WRK.open_state == OPEN_PENDING) || (WRK.restart_device == TRUE))
         {
            WRK.device_state = ON_LINE_TO_CU;

            /* save host attachment flag */
            DEV.cuat_dds = GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cuat)));

            WRK.cu_type  =  DEV.cuat_dds & 0xC0; /* first part of cu_type */

            if ((WRK.cu_type & 0x80) == 0)
               WRK.local_attach = FALSE;
            else
               WRK.local_attach = TRUE;

            if ((WRK.cu_type & 0x40) == 0) 
               WRK.non_sna = FALSE;
            else
               WRK.non_sna = TRUE;

            DEV.cuslvl_1_dds =
                  GETC_BUSMEM(dds_ptr,(offsetof(tca_t,cuslvl_1)));

            DEV.cuslvl_2_dds =
                  GETC_BUSMEM(dds_ptr,(offsetof(tca_t,cuslvl_2)));

            C327TRACE2 ("ebmt",(DEV.cuslvl_2_dds));
            /* Test for enhanced buffer management support */
            if (DEV.cuslvl_2_dds & EBM_MSK) {
               WRK.enh_buffer_mgmt = TRUE;
               C327TRACE2 ("ebms",1);
            }
            else {
               WRK.enh_buffer_mgmt = FALSE;
               C327TRACE2 ("ebms",0);
            }

            /* get the "cultas" out of the tca (8 of them) */
            for ( cultandx=0; cultandx<MAX_CULTA; cultandx++ )
            {
               DEV.culta_dds[cultandx] =
                       GETC_BUSMEM (dds_ptr,
                                    (offsetof(tca_t, culta[cultandx])));
            }
            if ( WRK.restart_device == FALSE )
            {
               /*
               ** assign the cu session(s) to net id table & init lt states
               ** This code sets WRK.num_sess to the smaller of what the
               ** application has allocated memory space for & what the CU
               ** has told us it has available.  The information comes from
               ** the number of valid culta's, not from a bytes value.
               */

               WRK.num_sess = 0;
               slow_device  = FALSE;

               for ( cultandx=0; cultandx < MAX_CULTA; cultandx++ )
               {
                  if ((DEV.culta_dds[cultandx] != 0xFF) &&
                      (WRK.num_sess < DEV.netid_table_entries) )
                  {
                     /* if lt is valid save stuff for the netid table */
                     TBL(WRK.num_sess).lt_id = DEV.culta_dds[cultandx];

                     TBL(WRK.num_sess).lt_bit_map =
                                (int)0x0000080 >> cultandx;

                     TBL(WRK.num_sess).prt_bit_map = 0;

                     TBL(WRK.num_sess).lt_state = LT_OFFLINE;
                     C327TRACE3 ("STAT", TBL(WRK.num_sess).lt_state,
                                 WRK.num_sess);

                     TBL(WRK.num_sess).sess_slow_dev = FALSE;

                     for ( ddsndx = 0; ddsndx < MAX_CULTA; ddsndx++ )
                     {
                        if (DEV.culta_dds[cultandx] ==
                            DEV.culta_prt[ddsndx])
                        {
                           TBL(WRK.num_sess).prt_bit_map =
                               TBL(WRK.num_sess).lt_bit_map;

                           TBL(WRK.num_sess).sess_slow_dev = TRUE;

                           slow_device  = TRUE;

                           break;
                        }
                     }

                     WRK.num_sess++;
                  }
               }
            }
            /*  test for either slow device supported or a single session
                that is not a printer session */
            if ( ( (DEV.cuslvl_2_dds & 0x01) != 0x00) ||
                 ( (WRK.num_sess     == 1) &&
                   (slow_device == FALSE) ) || (WRK.non_sna == FALSE) )
            {

               SendSS(dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);

               if (WRK.restart_device == TRUE)
               {
                  BroadcastUnSol (dds_ptr,
                                  OR_CHK_STAT_MSK, OR_RESTART_DONE);

                  WRK.restart_device = FALSE;  /* reset restart flag */

                  restart_bit_map = 0x00;      /* init restart bit maps */

                  restart_pr_bit_map = 0x00;

                  /* start through the netid table */
                  for ( sessndx=0; sessndx<WRK.num_sess; sessndx++)
                  {
                     /* look for session that has made it to online */
                     if ( (int)TBL(sessndx).lt_state >= (int)LT_ONLINE)
                     {

                        /* look for session that is busy */
                        if ( (int)TBL(sessndx).lt_state > (int)LT_ONLINE)
                        {
                           /* put session in ready state */
                           InterruptDH(dds_ptr, sessndx,
                                       INTR_UNSOL, OR_UNLOCK, 0);
                        }

                        TBL(sessndx).lt_state = LT_ONLINE_PEND;
                        C327TRACE3 ("STAT", TBL(sessndx).lt_state,
                                    sessndx);

                        /* save the bit maps for this session */
                        restart_bit_map =
                              restart_bit_map | TBL(sessndx).lt_bit_map;

                        restart_pr_bit_map =
                              restart_pr_bit_map | TBL(sessndx).prt_bit_map;
                     }
                     else
                     {
                        /* is session about to go offline ? */
                        if ( TBL(sessndx).halt_pending == TRUE )
                        {
                           /* reset state to offline */
                           TBL(sessndx).halt_pending = FALSE;
                           TBL(sessndx).lt_state = LT_OFFLINE;
                           C327TRACE3 ("STAT", TBL(sessndx).lt_state,
                                       sessndx);
                        }

                        /* is the session about to go online ? */
                        if ( TBL(sessndx).lt_state == LT_ONLINE_PEND)
                        {

                           /* save the bit maps for this session */
                           restart_bit_map =
                                 restart_bit_map | TBL(sessndx).lt_bit_map;

                           restart_pr_bit_map =
                              restart_pr_bit_map | TBL(sessndx).prt_bit_map;
                        }
                     }
                  }    /* end for */
                  if ( restart_bit_map )
                  {
                     /* send async req AEDV */
                     BuildAsyncReq ((uchar)AEDV_01, restart_bit_map,
                             restart_pr_bit_map, (uchar)0, (uchar)AEDV,
                             (uchar)AEDV_AD, &async_req);

                     (void) SendAS (dds_ptr, &async_req);
                  }
               }

               /* allow dftnsOpen to complete */
               WRK.open_state = DD_OPENED;
            }
            else
            {
               /* tell the world the DD must have Extended AEDV status */
               SendSS (dds_ptr, (uchar)SS_ERFR,
                       (uchar)ERFR_00, (uchar)0);

               /* allow dftnsOpen to complete */
               WRK.open_state = NOT_OPENED;
            }
         }
         else
         {
            SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);
         }
         break;

      /********************************************************************/
      /*            Start Operation = WCUS = Device ID status             */
      /********************************************************************/
      case   DEVICE_ID    :

         C327TRACE1 ("dvid");

         wcus_20_address = sizeof (tca_t);

         wcus_20_length  = GETC_BUSMEM (dds_ptr, wcus_20_address);

         if ( wcus_20_length > sizeof(DEV.cu_info.cu_array)  )
              wcus_20_length = sizeof(DEV.cu_info.cu_array);

         for (tca_ndx=0; tca_ndx < wcus_20_length; tca_ndx++)
         {
            DEV.cu_info.cu_array[tca_ndx] =
                   GETC_BUSMEM (dds_ptr, wcus_20_address + tca_ndx);
         }

         wcus_20_length  = 4 + sizeof( DEV.dev_info.dev_array );

         PUTC_BUSMEM (dds_ptr, wcus_20_address,   wcus_20_length);
         PUTC_BUSMEM (dds_ptr, wcus_20_address+1, 0x01);
         PUTC_BUSMEM (dds_ptr, wcus_20_address+2, 0x00);
         PUTC_BUSMEM (dds_ptr, wcus_20_address+3, 0x00);

         for (tca_ndx=0; tca_ndx < wcus_20_length; tca_ndx++)
         {
            PUTC_BUSMEM (dds_ptr, wcus_20_address + 4 + tca_ndx,
                         DEV.dev_info.dev_array[tca_ndx]);
         }

         SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);

         if ( ((DEV.cu_info.cu_array[4] == 0xF3) &&
               (DEV.cu_info.cu_array[6] == 0xF7) &&
               (DEV.cu_info.cu_array[7] == 0xF4)  ) ||
              ((DEV.cu_info.cu_array[4] == 0xF4) &&
               (DEV.cu_info.cu_array[5] == 0xF3) &&
               (DEV.cu_info.cu_array[6] == 0xF6) &&
               (DEV.cu_info.cu_array[7] == 0xF1)  ) )

            /*
            ** if we get here through that maze of conditions then we are
            ** either a 3x74 or a 4361 controller type !!!
            ** insert rest of bits into cu_type
            */

            WRK.cu_type = (WRK.cu_type | (DEV.cu_info.cu_array[7] & 0x03));
         {
            /*
            ** this BIG compound statement to trace device and control unit
            ** data, don't worry about it too much.
            */
            union
            {
               unsigned char trcbyte[16];
               int trcint[4];
            } trc;

            bzero ((void *)trc.trcint, sizeof(trc.trcint));

            bcopy ((void *)&(DEV.dev_info.dev.machine_type_number[0]),
                   (void *)&(trc.trcbyte[0]), (uint)4);

            bcopy ((void *)&(DEV.dev_info.dev.model_number[0]),
                   (void *)&(trc.trcbyte[4]), (uint)3);

            bcopy ((void *)&(DEV.dev_info.dev.serial_no[0]),
                   (void *)&(trc.trcbyte[8]), (uint)7);

            C327TRACE5 ("dv20",
                 trc.trcint[0], trc.trcint[1], trc.trcint[2], trc.trcint[3]);

            bzero ((void *)trc.trcint, sizeof(trc.trcint));

            bcopy ((void *)&(DEV.cu_info.cu.machine_type_number[0]),
                   (void *)&(trc.trcbyte[0]), (uint)4);

            bcopy ((void *)&(DEV.cu_info.cu.model_number[0]),
                   (void *)&(trc.trcbyte[4]), (uint)3);

            bcopy ((void *)&(DEV.cu_info.cu.serial_no[0]),
                   (void *)&(trc.trcbyte[8]), (uint)7);

            C327TRACE5 ("cu20",
                 trc.trcint[0], trc.trcint[1], trc.trcint[2], trc.trcint[3]);

            bzero ((void *)trc.trcint, sizeof(trc.trcint));

            bcopy ((void *)&(DEV.culta_dds[0]),
                   (void *)&(trc.trcbyte[0]), (uint)8);

            trc.trcbyte[ 9] = DEV.cuslvl_1_dds;
            trc.trcbyte[10] = DEV.cuslvl_2_dds;
            trc.trcbyte[11] = DEV.cuat_dds;

            C327TRACE4 ("mi20",
                        trc.trcint[0], trc.trcint[1], trc.trcint[2]);
         }
         break;

      /********************************************************************/
      /*       Start Operation = WCUS = Como Reminder Check Status        */
      /********************************************************************/

      case   COMO_CHK_MIND  :

         C327TRACE1 ("mind");

         /* prepare cu error for test */
         cu_info =
             ((int)GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp2))) <<8) |
              (int)GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp3)));

         cu_info = (cu_info & 0x0FFF) | CC_ERROR_MSK;

         /*
         ** ack the start op command
         */
         SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);

         if ( (WRK.WCUS_30_pending == FALSE) || (WRK.cu_error_o != cu_info) )
         {  /* if this is the first reminder - send it up */


            WRK.WCUS_30_pending = TRUE;

            WRK.cu_error_o = cu_info;

            BroadcastUnSol (dds_ptr, OR_SOFT_DSC, cu_info);
         }
         break;

      /********************************************************************/
      /*       Start Operation = WCUS = Como No Reminder Check Status     */
      /********************************************************************/

      case   COMO_CHK_N_MIND  :

         C327TRACE2 ("nmnd", WRK.WCUS_30_pending);
         /*
         ** ack the start op command
         */
         SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);

         if (WRK.WCUS_30_pending == TRUE)
         {  /* if we previously had reminder, clean up */

            WRK.WCUS_30_pending = FALSE;

            WRK.cu_error_o = 0;

            BroadcastUnSol (dds_ptr, OR_SOFT_DSC, 0);
         }
         break;


      /********************************************************************/
      /*       Start Operation = WCUS = Activate the LU                   */
      /********************************************************************/

      case   SNA_ACT  :


         for ( sessndx = 0; sessndx<WRK.num_sess; sessndx++)
         {
            /*
            ** find out which session had the WCUS 40
            */
            if ( TBL(sessndx).lt_id ==
               GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cultad))) )
            {
               break;
            }
         }

         /* prepare the cu info to DD info */
         cu_info =
               ((int)GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp1))) <<16) |
               ((int)GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp2))) <<8)  |
                (int)GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cultad)));

         cu_info = (cu_info & 0x00FFFFFF);

         C327TRACE2 ("aclu",cu_info);
         /*
         ** ack the WCUS 40 command
         */
         SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);

         /*
          *  Interrupt the Device Head to process WCUS 40
          */
         InterruptDH (dds_ptr, sessndx, INTR_UNSOL,
                      OR_SNA_INFO, cu_info);
         break;

      /********************************************************************/
      /*       Start Operation = WCUS = Deactivate the LU                 */
      /********************************************************************/

      case   SNA_DEACT  :
         
         /* prepare the cu info to DD info */
         cu_info =
               ((int)GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp1))) <<16) |
               ((int)GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp2))) <<8)  |
                (int)GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cultad)));

         cu_info = (cu_info & 0x00FFFFFF);

         C327TRACE2 ("dalu",cu_info);

         /*
         ** ack the start op command
         */
         SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);
        
         /* If the CULTAD = FF broadcast daclu to all sessions, else
            just send to appropriate session */
         if ((cu_info & 0x000000FF) == 0xFF) {
            for (sessndx=0;sessndx < WRK.num_sess; sessndx++) {
               if ((int)TBL(sessndx).lt_state >= (int)LT_ONLINE) {
                  InterruptDH (dds_ptr, sessndx, INTR_UNSOL,
                               OR_SNA_INFO, cu_info);
               }
            }
         }
         else { /* Broadcast to a single session */
            for ( sessndx = 0; sessndx<WRK.num_sess; sessndx++)
            {
               /*
               Match the LTID to a session number 
               */
               if ( TBL(sessndx).lt_id ==
                  GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cultad))) )
               {
                  break;
               }
            }

            InterruptDH (dds_ptr, sessndx, INTR_UNSOL,
                         OR_SNA_INFO, cu_info);
         }

         break;

      /********************************************************************/
      /*                Start Operation = WCUS = Otherwise                */
      /********************************************************************/

      default   :

         C327TRACE1 ("othr");

         SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);
   }                                              /* end switch      */
   return;
}                                                 /* end ProcessWCUS */
/*PAGE*/
/*
** Acknowledge async status
*/
void AckAsync (DDS_DATA *dds_ptr)
{
   unsigned char   temp_bit_map;
   int             sessndx;
   int             active_sess;
   boolean         ack_present=FALSE;
   unsigned char   command;

   C327TRACE3 ("AKAS", dds_ptr, WRK.old_async_req.as_parm[0]);
   C327TRACE3 ("AKAT", GETC_BUSMEM(dds_ptr, (offsetof (tca_t, dpastat))),
                WRK.dpastat_save);

   /* 
   ** If work area is unacked & adapter mem has ACK outstanding need to work 
   */
   if ( (WRK.dpastat_save == UNACK) &&
        (GETC_BUSMEM (dds_ptr, (offsetof( tca_t, dpastat))) == ACK) )
   {
      ack_present = TRUE;
      C327TRACE1 ("aka1");

      WRK.dpastat_save = ACK;              /* set WRK ack                  */
   }
   
   command = GETC_BUSMEM (dds_ptr, (offsetof(tca_t, cufrp1)));
   if ( (ack_present) || (command == SNA_ACT)  || (command == SNA_DEACT)) {
      /* get bit map from adapter mem */
      temp_bit_map = GETC_BUSMEM (dds_ptr, (offsetof( tca_t, daep2)));

      active_sess = 0;                      /* init counter                 */

      for (sessndx=0; sessndx<WRK.num_sess; sessndx++)  /* find who called  */
      {
         if (TBL(sessndx).lt_bit_map ==
             (temp_bit_map & TBL(sessndx).lt_bit_map))
         {
            if ( (TBL(sessndx).lt_state == LT_ONLINE_PEND) && /* is online  */
                 (WRK.old_async_req.as_parm[0] == AEDV_01) )  /* pending ?  */
            {
               TBL(sessndx).lt_state = LT_ONLINE;        /* comp online req */
               C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);

               if (TBL(sessndx).cmd_start_pending == TRUE )  /* start pending*/
                  AckCmd (dds_ptr, sessndx, INTR_SOL_START); /* ack it */
            }
            if ( WRK.old_async_req.as_parm[0] == AEDV_02 )
            {
               TBL(sessndx).lt_state = LT_OFFLINE;   /* complete online req */
               C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);

               TBL(sessndx).halt_pending = FALSE;
         
               if ( TBL(sessndx).cmd_halt_pending == TRUE ) /* pending cmd */ 
                    TBL(sessndx).cmd_halt_pending = FALSE;
            }
         }

         if ( (int)TBL(sessndx).lt_state >= (int)LT_ONLINE )
            active_sess++;       /* set the ras counter for max sess acive */
      }                                                         /* end for */

      /* if any session is on-line to host then device is on-line to host */
      if (active_sess > 0)
         WRK.device_state = ON_LINE_TO_HOST;
      else
         WRK.device_state = ON_LINE_TO_CU;

      /* 
      ** if ( active_sess >= RAS.max_active_ses )
      ** RAS.max_active_ses = active_sess
      ** update RAS counter 
      */
   
   }
   return;
}                                                           /* end AckAsync */
/*PAGE*/
/*
** Start up command processor
*/
void StartOp (DDS_DATA *dds_ptr)
{
   uchar           msb, lsb, temp_start_print;
   int             sessndx, xtra_sessndx;
   uint            wordptr;
   uchar           sync_fnreq;            /* type of function request   */
   uchar           ltid;                  /* logical terminal id        */
   int             offset;                /* offset into data area      */
   boolean         DONE;                  /* flag for while loop        */
   int             laNum;
   linkAddr        *laP;
   NETWORK_ID      netID;
   int             dev;

   C327TRACE5("STOP", dds_ptr,
                      GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrv))),
                      GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cultad))),
                      GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp1))) );

   C327TRACE4("STO2", GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp2))),
                      GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp3))),
                      GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp4))) );

   /* save some adapter mem information */
   WRK.dpastat_save = GETC_BUSMEM (dds_ptr, (offsetof( tca_t, dpastat)));

   WRK.cusyn_save   = GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cusyn)));

   WRK.interface_state = CONNECTED_A_ACTIVE;

   /* check if cu wants host timing */
   if (GETC_BUSMEM (dds_ptr, (offsetof( tca_t, extime))) == 0x01) 
   {
      /* if host timer is not already running, start it */
      if (WRK.timer_type_2 != TIMER_TYPE_HOST_TIMING)
         SetTimer (dds_ptr, 2, TIMER_TYPE_HOST_TIMING, 500);
   }
   else
   {
      /* restart the device timer whether it's running or not */
      SetTimer (dds_ptr, 2, TIMER_TYPE_DEVICE_TIMING, 500);
   }

   AckAsync (dds_ptr);                             /* send acknowledgement */

   /* what type of start op do we have ? */
   /* The variable sync_fnreq is used to hold this value so we won't waste 
      so much time getting the value from the bus again */
   switch ( sync_fnreq = GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrv))) )
   {
      /**************************************************************/
      /*****    LOCK - Non SNA host selection, device ready     *****/
      /**************************************************************/
      case  LOCK  :

         C327TRACE1 ("lock");

         if ( (int)WRK.device_state <= (int)ON_LINE_TO_CU )
         {  /* NO - send error to control unit */
            SendSS (dds_ptr, (uchar)SS_ERFR, (uchar)ERFR_01, (uchar)0);
         }
         else
         {  /*   yup - process the start up command */

            /* back to looking through the netid table again */
            for ( sessndx = 0; sessndx < WRK.num_sess; sessndx++ )
            {

               /* does bit map in adapter mem Async Req match */
               if ( TBL(sessndx).lt_id ==
                    GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cultad)) ))
               {

                  if ( TBL(sessndx).lt_state == LT_WRT_CMD_PEND )
                  {  /* session busy reject lock */

                     SendSS (dds_ptr, (uchar)SS_FCSE,
                             (uchar)FCSE_01, (uchar)0);

                     /* RAS.cnt_host_cont++ */

                     return;
                  }

                  /* yes, so now test for valid state condition */
                  if ( (int)TBL(sessndx).lt_state < (int)LT_ONLINE )
                  {
                     /* oops session not on line yet, report error */
                     SendSS (dds_ptr, (uchar)SS_ERFR, (uchar)ERFR_07,
                             (uchar)0);

                     return;
                  }

                  /* Are we in contention with host for cmd processing ? */
                  if ( TBL(sessndx).lt_state == LT_WRT_CMD_REDO )
                  {
                     /* yep, host wins, so kill write command */
                     /* RAS.cnt_host_cont++ increment ras counter */

                     TBL(sessndx).lt_state = LT_ONLINE; /* reset current st */
                     C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);

                     InterruptDH (dds_ptr, sessndx, INTR_UNSOL,
                                  OR_WRITE_DISCARD, 0);

                     /*
                     ** ack wrt cmd
                     */
                     AckCmd (dds_ptr,sessndx,INTR_SOL_WRITE);
                  }

                  /*  Test for BSC contention scenarios */
                  if ( (WRK.local_attach == FALSE)  && (WRK.non_sna == TRUE) )
                  {
                     /*
                     ** now lets start looking through the netid table again
                     */
                     for( xtra_sessndx=0; xtra_sessndx < WRK.num_sess;
                          xtra_sessndx++ )
                     {
                        /* test for lt_wrt_cmd_ack */
                        if ( TBL(xtra_sessndx).lt_state == LT_WRT_CMD_ACK )
                        {

                           /* if so what are we to do with current session */
                           /* RAS.cnt_host_cont++         bump ras counter */

                           /* test if this lock is for the current session */
                           if ( sessndx == xtra_sessndx )
                           {
                              /*  reset current state to online */
                              TBL(sessndx).lt_state = LT_ONLINE;
                              C327TRACE3 ("STAT", TBL(sessndx).lt_state,
                                          sessndx);

                              /* do we free the buffer? */
                              if ( TBL(sessndx).cmd_write_pending == TRUE )
                                 /*  ack write command and free buffer */
                              {
                                 InterruptDH (dds_ptr, sessndx,
                                              INTR_UNSOL,
                                              OR_WRITE_DISCARD, 0);

                                 AckCmd (dds_ptr,sessndx,INTR_SOL_WRITE);
                              }
                           }
                           else
                           {
                              /* set current state to restart again later */
                              TBL(sessndx).lt_state = LT_WRT_CMD_REDO;
                              C327TRACE3 ("STAT", TBL(sessndx).lt_state,
                                          sessndx);
                           }
                           break;
                        }
                     }        /*  end xtra_sessndx */
                  }

                  if ( TBL(sessndx).lt_state == LT_ONLINE )
                  {

                     TBL(sessndx).lt_state = LT_LOCK;  /* lock write cmd */
                     C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);
                     /* send sync stats to cu */

                     SendSS(dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);

                     if ( (TBL(sessndx).program_check == TRUE ) &&
                          (TBL(sessndx).halt_pending == FALSE ) )
                     {
                        /* reset pgm ck flg */
                        TBL(sessndx).program_check = FALSE;

                        /* tell emulator program check is over */
                        InterruptDH (dds_ptr, sessndx, INTR_UNSOL,
                                     OR_CHK_STAT_MSK, 0);
                     }

                     /* is halt not pending for this session */
                     if ( TBL(sessndx).halt_pending == FALSE )
                     {
                        /*
                        ** tell LLC to put session in an INHIBIT INPUT state
                        */
                        InterruptDH (dds_ptr, sessndx,
                                     INTR_UNSOL, OR_LOCK, 0);
                     }
                  }

                  /* is a write data command pending? */
                  netID = TBL(sessndx).network_id;
                  dev = (int)(netID.adapter_code - 1);
                  netId2La(dev, netID, &laP, &laNum);
                  if ( (TBL(sessndx).lt_state == LT_WRT_CMD_ACK ) ||
                       ( ! laP->writeBuffer_used && WRK.non_sna == TRUE ))
                  {
                     /* is data still in adapter mem buffer from PDAT  */
                     /* adapter mem is bit mapped with bit 3 (0x08)    */
                     /* being turned on to indicate enhanced           */
                     /* adapter mem buf management                     */

                     if ( (GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cuslvl_2)))
                          & 0x08) == 0x08 )
                     {
                        /* now check for LOCK parm   */
                       if ( GETC_BUSMEM(dds_ptr,offsetof(tca_t,cufrp1))== 0x01)
                       {
                          TBL(sessndx).lt_state = LT_WRT_LOCK_DA;
                          C327TRACE3 ("STAT", TBL(sessndx).lt_state,
                                      sessndx);
                       }
                       else
                       {
                          TBL(sessndx).lt_state = LT_WRT_LOCK_NA;
                          C327TRACE3 ("STAT", TBL(sessndx).lt_state,
                                      sessndx);
                       }

                     }
                     else
                     {
                        TBL(sessndx).lt_state = LT_WRT_LOCK_NA;
                        C327TRACE3 ("STAT", TBL(sessndx).lt_state,
                                    sessndx);
                     }

                     /* send sync stats to cu */
                     SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);

                     /* is HALT pending ? */
                     if ( TBL(sessndx).halt_pending == FALSE )
                          InterruptDH (dds_ptr, sessndx,
                                       INTR_UNSOL, OR_LOCK, 0);

                  }
                  break;
               }
            }  /* end for loop (sessndx)  */
         }
         break;

      /**************************************************************/
      /*****     CTCCS - Terminate chained command sequence     *****/
      /**************************************************************/
      case  CTCCS :

         C327TRACE1 ("ctcc");

         if ( (int)WRK.device_state <= (int)ON_LINE_TO_CU )
         {
            /* NO - send error to control unit */
            SendSS(dds_ptr, (uchar)SS_ERFR, (uchar)ERFR_01, (uchar)0);
         }
         else     /*   yup - process the CTCCS instruction */
         {
            for ( sessndx = 0; sessndx < WRK.num_sess; sessndx++ )
            {

               /* does bit map in adapter mem Async Req match */
               if ( TBL(sessndx).lt_id ==
                    GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cultad)) ))
               {

                  if ( (int)TBL(sessndx).lt_state <  (int)LT_ONLINE )
                  {
                     /* NO - send error to control unit */
                     SendSS (dds_ptr, (uchar)SS_ERFR,
                             (uchar)ERFR_07, (uchar)0);

                     return;
                  }

                  /* reset chaining in netid table */
                  TBL(sessndx).cmd_chaining = FALSE;

                   break;
               }
            }  /* end for */

            if ( TBL(sessndx).lt_id !=
                 GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cultad)) ))
            {
               /* NO match  - send error to control unit */
               SendSS (dds_ptr,(uchar)SS_ERFR,
                       (uchar)ERFR_07,(uchar)0);

               return;
            }

            /* is write command restart condition set / */
            if ( TBL(sessndx).lt_state ==  LT_WRT_CMD_ACK )
            {
               /* RAS.cnt_host_cont++ */

               /* update session state to restart the write */
               TBL(sessndx).lt_state =  LT_WRT_CMD_REDO;
               C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);

               /* NO - send error ot control unit */
               SendSS (dds_ptr,(uchar)SS_FC, (uchar)0, (uchar)0);

               /* is HALT pending ? */
               if (TBL(sessndx).halt_pending == FALSE )
                  InterruptDH(dds_ptr,sessndx,INTR_UNSOL,OR_UNLOCK,0);

            }
            else
               ProcessCTCCS (dds_ptr,sessndx);
         }
         break;

      /**************************************************************/
      /*****         WLCC - Write Local Channel Command         *****/
      /**************************************************************/
      case  WLCC :

         C327TRACE1 ("wlcc");

         if ( (int)WRK.device_state <= (int)ON_LINE_TO_CU )
         {

            /* NO - send error ot control unit */
            SendSS(dds_ptr,(uchar)SS_ERFR,(uchar)ERFR_01,(uchar)0);
         }
         else     /*   yup - process the Write Local Channel Command */
         {
            for ( sessndx = 0; sessndx < WRK.num_sess; sessndx++ )
            {
               /* does bit map in adapter mem Async Req match */
               if (TBL(sessndx).lt_id ==
                   GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cultad))))
               {
                  if ( (int)TBL(sessndx).lt_state <  (int)LT_ONLINE )
                  {
                     /* NO - send error ot control unit */
                     SendSS (dds_ptr, (uchar)SS_ERFR,
                             (uchar)ERFR_07, (uchar)0);

                     return;
                  }

                  /* save the WLCC command in the adapter mem */
                  TBL(sessndx).cmd_3270 =
                         GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp1)));

                  break;
               }
            }
            /* is chaining bit on ?   ... this is bit 2 in cufrp2 */
            if (((((GETC_BUSMEM(dds_ptr,
                               (offsetof(tca_t,cufrp2))) & 0x04)== 0x04)&&
                  ( TBL(sessndx).cmd_chaining == TRUE ) ) &&
                  (  WRK.non_sna == FALSE ))||
                ( ( TBL(sessndx).lt_state == LT_WRT_RDAT_NA) &&
                  ( TBL(sessndx).cmd_chaining == FALSE) ) ) 
            {
               /*
               ** yep chaining is on, so call ProcessCTCCS to finish old WDAT
               */
               ProcessCTCCS (dds_ptr,sessndx);
            }
            RecvCmd (dds_ptr, sessndx, (uchar)WLCC);
         }
         break;

      /**************************************************************/
      /*****            WDAT - Write Data From Host             *****/
      /**************************************************************/

      case  WDAT :

         C327TRACE1 ("wdat");

         if ( (int)WRK.device_state <= (int)ON_LINE_TO_CU )
         {
            /* NO - send error to control unit */
            SendSS(dds_ptr,(uchar)SS_ERFR,(uchar)ERFR_01,(uchar)0);
         }
         else     /*   yup - process the Write Data From Host */
         {
            sessndx = 0;
            DONE = FALSE;
            /* get ltid from adapter buffer */
            /* If non-SNA, this will be done by getting the CULTAD byte.
               For SNA, we will get the DAF value to work around a 3274
               CU bug that will never be fixed. */
            if (WRK.non_sna) 
               ltid  = GETC_BUSMEM (dds_ptr, (offsetof(tca_t, cultad)));
            else {
               offset = ((int) GETC_BUSMEM (dds_ptr, (offsetof (tca_t, 
                        cudp_msb))) << 8) | (int) GETC_BUSMEM (dds_ptr, 
                        (offsetof (tca_t, cudp_lsb)));
               ltid = GETC_BUSMEM (dds_ptr, offset + 6);
             }
              
            /* make sure that we have a session record with matching ltid */
            while ((sessndx < WRK.num_sess) && (!DONE)) {
               if (TBL(sessndx).lt_id == ltid)
                  DONE = TRUE;
               else sessndx ++;
            }
            if (DONE)
               {
                  if ( (int)TBL(sessndx).lt_state <  (int)LT_ONLINE )
                  {

                     /* NO - send error to control unit */
                     SendSS(dds_ptr,(uchar)SS_ERFR,
                            (uchar)ERFR_07,(uchar)0);

                     return;
                  }
               

                  /*
                   *  Are we attached to a SNA controller.
                   */
                   if (WRK.non_sna == FALSE) {

                      C327TRACE1 ("snrd");
                      C327TRACE3 ("wdlt",ltid,sessndx);

                      TBL(sessndx).apnd_to_buffer = FALSE;

                      /* call rec data to  */
                      RecvData (dds_ptr, sessndx, JUST_DATA);

                      break;
                   }
                   else {
                      /* is chaining bit on ?   ... this is bit 2 in cufrp2 */
                      if ( (GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cufrp2)))
                           & 0x04 ) == 0x04)
                      {
                         TBL(sessndx).cmd_chaining = TRUE;
                      } else {
                         TBL(sessndx).cmd_chaining = FALSE;
                      }
                      C327TRACE2 ("chn0",TBL(sessndx).cmd_chaining);
    
                      /* check if this is first WDAT in a channel series */
                      if ( (WRK.local_attach      == TRUE   ) &&
                           (TBL(sessndx).lt_state == LT_LOCK)   )
                      {

                         /* get address of first byte data in tca data area */
                         msb = GETC_BUSMEM(dds_ptr, offsetof(tca_t, cudp_msb));
                         lsb = GETC_BUSMEM(dds_ptr, offsetof(tca_t, cudp_lsb));
                         wordptr = (uint)(((uint)msb << 8) | ((uint)lsb + 4));

                         /*  test first byte of data from tca data area  */
                         temp_start_print =  GETC_BUSMEM (dds_ptr, wordptr);

                         if ( (temp_start_print & START_PRT_MASK) ==
                                                  START_PRT_MASK)
                            TBL(sessndx).start_print = TRUE;
                         else
                            TBL(sessndx).start_print = FALSE;

                         /* call rec data to determine what the 3270 means */
                         RecvCmd (dds_ptr, sessndx, (uchar)JUST_DATA);

                      }

                      /* check if this is first WDAT in a bi-sync(BSC) series */
                      if ( (WRK.local_attach      == FALSE  ) &&
                           (TBL(sessndx).lt_state == LT_LOCK)   )
                      {
                         /*  save first byte of data from tca data area  */
                         TBL(sessndx).cmd_3270 =
                              GETC_BUSMEM (dds_ptr,  (uint)(
                                           ((int)GETC_BUSMEM(dds_ptr,
                                            (offsetof(tca_t,cudp_msb)))<< 8) |
                                            (int)GETC_BUSMEM(dds_ptr,
                                            (offsetof(tca_t,cudp_lsb)))) + 4);

                         /*  test second byte of data from tca data area  */
                         temp_start_print =
                              GETC_BUSMEM (dds_ptr,  (
                                           ((int)GETC_BUSMEM (dds_ptr,
                                            (offsetof( tca_t,cudp_msb))) << 8) |
                                            (int)GETC_BUSMEM (dds_ptr,
                                            (offsetof( tca_t,cudp_lsb))) ) + 5);

                         if ((temp_start_print & START_PRT_MASK)==
                                                    START_PRT_MASK)
                            TBL(sessndx).start_print = TRUE;
                         else
                            TBL(sessndx).start_print = FALSE;

                         /* call rec data to determine what the 3270 means */
                         RecvCmd (dds_ptr, sessndx, (uchar)WDAT);
                      }
                      else /* not first WDAT, assume more data */
                      {
                         if ( TBL(sessndx).halt_pending == TRUE )
                            SendSS (dds_ptr, (uchar)SS_FC,
                                    (uchar)0, (uchar)0);
                         else
                            RecvData (dds_ptr, sessndx, JUST_DATA);
                      }

                      break;
                  }
               }


         }

         break;

      /**************************************************************/
      /*****            RDAT - Write Data From Host             *****/
      /*****       PDAT - Prepare Read Data Prior to Host       *****/
      /**************************************************************/
      case  RDAT :
      case  PDAT :

         if (sync_fnreq == RDAT )
            C327TRACE1 ("rdat");
         else  {
            C327TRACE1 ("pdat");
	    if ((WRK.non_sna == TRUE) && (WRK.aeep_outstanding == TRUE))
		WRK.aeep_outstanding = FALSE;
	 }

         if ( (int)WRK.device_state <= (int)ON_LINE_TO_CU )
         {

            /* NO - send error ot control unit */
            SendSS (dds_ptr, (uchar)SS_ERFR, (uchar)ERFR_01, (uchar)0);

         }
         else     /*   yup - process the Write Data From Host */
         {
            sessndx = 0;
            DONE = FALSE;
            /* get ltid from adapter buffer */ 
            ltid = GETC_BUSMEM (dds_ptr, (offsetof( tca_t, cultad))); 

            /* make sure that we have a session record with matching ltid */
            while ((sessndx < WRK.num_sess) && (!DONE)) {
               if (TBL(sessndx).lt_id == ltid) 
                  DONE = TRUE;  /* we have found the proper ltid */
               else sessndx ++;
            }


               /* does bit map in adapter mem Async Req match */
               if (DONE) 
               {

                  if ( (int)TBL(sessndx).lt_state <  (int)LT_ONLINE )
                  {

                     /* NO - send error to control unit */
                     SendSS (dds_ptr, (uchar)SS_ERFR, (uchar)ERFR_07, 
                             (uchar)0);

                     return;
                  }


                  /* Halt pending ? */
                  if ( TBL(sessndx).halt_pending == TRUE )
                  {

                     /* if SNA, always just do the abort            */
                     if ((sync_fnreq == PDAT) || (WRK.non_sna == FALSE))
                     {

                        TBL(sessndx).lt_state =  LT_ONLINE;
                        C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);
                        SendSS (dds_ptr, (uchar)SS_FRA,
                                (uchar)0, (uchar)0);


                        return;
                     }
                     else
                     {

                        /*
                        ** is it BSC attached and first RDAT in a WRITE seq?
                        */
                        if ( (TBL(sessndx).write_data_len != 0           ) &&
                             (WRK.local_attach == FALSE                  ) &&
                             (WRK.non_sna == TRUE                        ) &&
                             ( (TBL(sessndx).lt_state == LT_WRT_CMD_ACK) ||
                               (TBL(sessndx).lt_state == LT_WRT_LOCK_DA) ||
                               (TBL(sessndx).lt_state == LT_WRT_LOCK_NA) ||
                               (TBL(sessndx).lt_state == LT_WRT_RDAT_NA) )  )
                        {

                           /* send a frame */
                           SendSS (dds_ptr, (uchar)SS_FRA,
                                   (uchar)0, (uchar)0);

                           return;
                        }
                     }
                  }

                  /* The SNA case is very simple. Just call the segmenting and
                     transfer routine */
                  if (WRK.non_sna == FALSE) {
                     C327TRACE1 ("sgm1");
                     /* First we need to check if we are non EBMS and a 
                        bind/unbind was received.If this is so and there was
                        no write yet, we need to set the state as locked and 
                        return */
                     if ((TBL(sessndx).bind_recvd == TRUE) && 
                        (TBL(sessndx).cmd_write_pending == FALSE)) 
                     {
                        TBL(sessndx).lt_state = LT_RM_LOCKED_1;
                        return;
                     }
                     else {
                        /* segment and xmit the data */
                        segment_SNA_data (dds_ptr,sessndx);
                     }
                  }
                  else {

                  if ( TBL(sessndx).kbd_reset_pending == TRUE )
                  {
                     TBL(sessndx).kbd_reset_pending = FALSE;

                     if (
                          (sync_fnreq == PDAT)   ||
                         ((sync_fnreq == RDAT ) &&
                           ( TBL(sessndx).write_data_len != 0) &&
                           ( WRK.local_attach == FALSE       ) &&
                           ( WRK.non_sna == TRUE             )   )   )
                     {

                        SendSS (dds_ptr, (uchar)SS_FRA,
                                (uchar)0, (uchar)0);

                        /* RAS.cnt_rst_key_p++              bump ras counter */

                        TBL(sessndx).lt_state = LT_ONLINE;   /* update state */
                        C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);

                        AckCmd (dds_ptr, sessndx, INTR_SOL_WRITE);/* ack wrt */

                        /* have llc put session in ready st */
                        InterruptDH (dds_ptr, sessndx, INTR_UNSOL,
                                     OR_UNLOCK, 0);

                        return;
                     }
                  }

                  if (sync_fnreq == PDAT)
                  {

                     TBL(sessndx).lt_state = LT_WRT_CMD_ACK;/* update state */
                     C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);

                     /* transfer data to tca */
                     XmitData (dds_ptr, sessndx, (boolean)FALSE);

                  }
                  else
                  {

                     /* is there an unsolictied read modify out there ? */
                     if ( (TBL(sessndx).lt_state == LT_RM_LOCKED)   ||
                          (TBL(sessndx).lt_state == LT_RM_LOCKED_1) ||
                          (TBL(sessndx).lt_state == LT_RM_LOCKED_2) ||
                          (TBL(sessndx).lt_state == LT_RM_LOCKED_3) )
                     {

                        /*  is there data to send to tca ? */
                        if ( (TBL(sessndx).lt_state == LT_RM_LOCKED_2) ||
                             (TBL(sessndx).lt_state == LT_RM_LOCKED_3) )
                        {
                           /* update session state */
                           TBL(sessndx).lt_state = LT_RM_LOCKED_3;
                           C327TRACE3 ("STAT", TBL(sessndx).lt_state,
                                       sessndx);

                           /* transfer data to tca */
                           XmitData (dds_ptr, sessndx, (boolean)FALSE);

                        }
                        else
                        {
                           /* update session state & wait for write cmd */
                           TBL(sessndx).lt_state = LT_RM_LOCKED_1;
                           C327TRACE3 ("STAT", TBL(sessndx).lt_state,
                                       sessndx);
                           return;
                        }
                     }

                     if ( TBL(sessndx).lt_state == LT_WRT_RDAT_NA )
                        /* transfer data to tca */
                        XmitData (dds_ptr, sessndx, (boolean)FALSE);

                     if ( TBL(sessndx).lt_state == LT_WRT_LOCK_NA )
                     {
                        /* transfer data to tca */
                        XmitData (dds_ptr, sessndx, (boolean)TRUE );

                        /* update for next RDAT that DATA is not available */
                        TBL(sessndx).lt_state = LT_WRT_RDAT_NA;
                        C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);
                     }

                     if (TBL(sessndx).lt_state == LT_WRT_LOCK_DA)
                     {
                        /* transfer data to tca */
                        XmitData (dds_ptr, sessndx, (boolean)FALSE );

                        TBL(sessndx).lt_state = LT_WRT_RDAT_NA;
                        C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);
                     }

                     if (TBL(sessndx).lt_state == LT_WRT_CMD_ACK)
                     {
                        TBL(sessndx).lt_state = LT_WRT_RDAT_NA;
                        C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);
                     }

                  }

               }  /* end else clause for non_sna */
            }  /* end if LTID matches */


            /* to get here inning is complete; no hits, no runs, no errors */
            /* send a function complete to the controller. */
            SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);

           /* Check if the write is complete. If SNA, this is the only logical
              place to interrupt the tca header */
           if ((WRK.non_sna == FALSE) && !(TBL(sessndx).write_in_progress)) {
              AckCmd (dds_ptr,sessndx,INTR_SOL_WRITE);  /* write complete */
              C327TRACE1 ("pdt1");
              TBL(sessndx).lt_state = LT_ONLINE;        /* update state   */
              TBL(sessndx).bind_recvd = FALSE;   /* doesn't hurt to reset */

              /* if first inbound send a broadcast message */
              if (TBL(sessndx).first_inbound) {
                 TBL(sessndx).first_inbound = FALSE;
                 InterruptDH (dds_ptr, sessndx, INTR_UNSOL, OR_SNA_RUSIZE, 
                              TBL(sessndx).MaxRuSize);
              }  
           }
         }
         break;

      /**************************************************************/
      /*****          WCUS - Write Control Unit Status          *****/
      /**************************************************************/
      case  WCUS :

         ProcessWCUS (dds_ptr);

         break;

      /**************************************************************/
      /*****          CNOP - Cause Interrupt on Device          *****/
      /**************************************************************/
      case  CNOP :

         C327TRACE1 ("cnop");

         SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);

         break;

      /**************************************************************/
      /*****   default means invalid or non supported command   *****/
      /**************************************************************/
      default    :

         C327TRACE1 ("cerr");

         /* send error to interface */
         SendSS (dds_ptr, (uchar)SS_ERFR, (uchar)0, (uchar)0);

         break;

   }  /* end switch */

   return;

} /* end StartOp */
/*PAGE*/
/*
** acknowledge completion of all commands on all sessions
*/
void AckAllCmd (DDS_DATA *dds_ptr)
{
   int sessndx;

   C327TRACE2 ("AKAL", dds_ptr);

   /* Check for rm write in process */
   if (WRK.rm_cmd_write_pending == TRUE)
   {
      WRK.rm_cmd_write_pending = FALSE;
      WRK.rm_write_in_progress = FALSE;
   }

   for (sessndx = 0; sessndx < WRK.num_sess; sessndx++)
   {

      /* Reset or Initialize session variables */
      TBL(sessndx).cmd_chaining       = FALSE;
      TBL(sessndx).kbd_reset_pending  = FALSE;
      TBL(sessndx).ack_pending        = FALSE;

      /* Check for session about to go online */
      if (TBL(sessndx).cmd_start_pending == TRUE)
      {
         TBL(sessndx).cmd_start_pending = FALSE;

         /* do we need to set state flags? */
         InterruptDH(dds_ptr, sessndx,
                     INTR_SOL_START, OR_NO_ERROR, 0);
      }

      /* Check for session write in process */
      if (TBL(sessndx).cmd_write_pending == TRUE)
      {
         TBL(sessndx).cmd_write_pending = FALSE;

         TBL(sessndx).write_in_progress = FALSE;

         /* do we need to set state flags? */
         InterruptDH(dds_ptr, sessndx,
                     INTR_SOL_WRITE, OR_NO_ERROR, 0);
      }

      /* Test if a halt command is pending */
      if (TBL(sessndx).cmd_halt_pending == TRUE)
      {
         TBL(sessndx).cmd_halt_pending = FALSE;
         /* do we need to set state flags? */
      }

   } /* end for */

   return;

} /* end AckAllCmd */
/*PAGE*/
/*
** got PIO errors so force a simulated close
*/
void ForceClose (DDS_DATA *dds_ptr)
{
   C327TRACE2 ("FORC", dds_ptr);

   if (WRK.num_opens == 0) /* make sure there's an open device head to kill */
      return;

   BroadcastUnSol (dds_ptr, OR_CU_DOWN, 0);

   BroadcastUnSol (dds_ptr, OR_CHK_STAT_MSK,
                   (BROADCAST_MSK | SS_6_MASK | SS_PIO_ERROR));

   AckAllCmd (dds_ptr);

   /* must lock common so when we find right place, it doesn't change on us */
#ifdef _POWER_MP
   lock_write((complex_lock_t)&CMNMIS.common_locked);
#else
   lockl ( (lock_t *)&CMNMIS.common_locked, LOCK_SHORT );
#endif

   ShutDown (dds_ptr);                                 /* force close */

#ifdef _POWER_MP
      lock_done((complex_lock_t)&CMNMIS.common_locked );
#else
      unlockl ( (lock_t *)&CMNMIS.common_locked );  /* unlock common area   */
#endif

   return;

} /* end ForceClose */
/*PAGE*/
/*
** restart the DFT card and resume sessions after a COAX disconnect
*/
void Restart (DDS_DATA *dds_ptr)
{
   int                 sessndx;

   C327TRACE2 ("REST", dds_ptr );

   /* make sure this card still hooked up */
   if (WRK.interface_state == NOT_CONNECTED)
      return;

   WRK.restart_in_progress = TRUE;

   c327Disconnect (dds_ptr);    /* disconnect the coax */

   WRK.interface_state     = NOT_CONNECTED;

   WRK.device_state        = OFF_LINE_TO_CU;

   /* kill the timers */
   SetTimer (dds_ptr, 1, TIMER_TYPE_NONE, 0);
   SetTimer (dds_ptr, 2, TIMER_TYPE_NONE, 0);

   AckAllCmd (dds_ptr);         /* acknowledge any unacknowledged commands */

   /* allow any pending halts to complete */
   for (sessndx = 0; sessndx < WRK.num_sess; sessndx++)
   {
      if (TBL(sessndx).halt_pending == TRUE)
      {
         TBL(sessndx).halt_pending = FALSE;
         TBL(sessndx).lt_state     = LT_OFFLINE;
         C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);
      }
   }

   /* RAS.cnt_restarts++ */

   WRK.restart_device = TRUE;

   WRK.coax_reset_received = FALSE;

   SetTimer (dds_ptr, 1, TIMER_TYPE_RESTART, 7000);   /* initialize the card */

} /* end Restart */
/*PAGE*/
/*
** handle the nitty-gritty details of processing an interrupt from the card
*/
void dftnsProcessInterrupt (DDS_DATA *dds_ptr)
{
#  define INTR_READ_ID    0x08
#  define INTR_DIAG_RESET 0x04
#  define INTR_RESET      0x02
#  define INTR_START_OP   0x01

   unsigned int   cu_error;


   C327TRACE3 ("PINT", dds_ptr, WRK.intr_reg_save);

   if (WRK.interface_state == NOT_CONNECTED)     /* is card still hooked up */
      return;

   /* reset inactivity timer for this cu.  can now get away with not
   talking to this cu for another 30 seconds */
   SetTimer (dds_ptr, 1, TIMER_TYPE_CU_INACTIVE, 30000);

   /* now find out why interrupt happened, looking at most important first */
   if ((WRK.intr_reg_save & INTR_RESET) != 0) /* is interrupt a "Reset" cmd */
   {
      C327TRACE2 ("rset", dds_ptr);

      WRK.intr_reg_save = 0;         /* clear previous saved interrupt stat */

      if (WRK.coax_reset_received == FALSE)
      {
         /* disconnect coax and inhibit interrupts */
         WRK.conn_ctrl_save = CONN_CTRL_DFT_MODE | CONN_CTRL_INT_INH;

         PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg, WRK.conn_ctrl_save);

         /* clear pndng interrupts */
         PUTC_BUSIO (dds_ptr, adapter_intr_stat_reg, 0x3F);

         /* we are a smart dev   */
         PUTC_BUSIO (dds_ptr, adapter_term_id_reg, ((~0x01) & 0xFF));

         WRK.coax_reset_received = TRUE;         /* we've processed a reset */

         /* enable the COAX and interrupts and dft mode */
         WRK.conn_ctrl_save = CONN_CTRL_DFT_MODE | CONN_CTRL_ENABLE_COAX;

         PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg, WRK.conn_ctrl_save);

         return;                            /* don't process anything else */
      }
      else
      {
         /* simple reset didn't work, so restart from scratch */
         if (WRK.open_state == DD_OPENED)
            BroadcastUnSol (dds_ptr, OR_CU_DOWN, 0);

         Restart(dds_ptr);

         return;                             /* don't process anything else */
      }

   } /* end of "Reset" processing */

   /* check if interrupt due to "Read Terminal ID" command */
   if ((WRK.intr_reg_save & INTR_READ_ID) != 0)
   {
      C327TRACE2 ("rdid", dds_ptr);
      C327PERF( 0x0250 );

      /* check if because of reset or alternate start operation */
      if (WRK.reset_in_process == TRUE)
      {
         WRK.reset_in_process = FALSE;
      }
      else
      {
         /* check if cu wants host timing */

         if (GETC_BUSMEM (dds_ptr, (offsetof( tca_t, extime))) == 0x01)
         {
            /*
            ** if not currently doing host timing, then start it in host mode
            */
            if (WRK.timer_type_2 != TIMER_TYPE_HOST_TIMING)
               SetTimer (dds_ptr, 2, TIMER_TYPE_HOST_TIMING, 500);
         }
         else
         {
            /* if currently doing host timing, then change type */
            if (WRK.timer_type_2 == TIMER_TYPE_HOST_TIMING)
               WRK.timer_type_2 = TIMER_TYPE_DEVICE_TIMING;
         }
      }
   } /* end of "Read Terminal ID" processing */

   /* is interrupt due to either "Read Terminal ID" or "Start Operation" */
   if (((WRK.intr_reg_save & INTR_READ_ID) != 0) ||
       ((WRK.intr_reg_save & INTR_START_OP) != 0))
   {
      AckAsync (dds_ptr);                             /* send ack        */
   }

   /* check if interrupt due to "Start Operation" command */
   if ((WRK.intr_reg_save & INTR_START_OP) != 0)
   {
      /* check if cu detected serious error */
      cu_error = (
                  ((int)GETC_BUSMEM(dds_ptr,
                                    (offsetof( tca_t, cudser_msb))) <<8) |
                  (int)GETC_BUSMEM(dds_ptr,
                                   (offsetof( tca_t, cudser_lsb)))) & 0x0FFF;

      if (cu_error != 0)
      {
         cu_error = cu_error | MC_ERROR_MSK | BROADCAST_MSK;

         BroadcastUnSol (dds_ptr, OR_CHK_STAT_MSK, (int)cu_error);

         Restart (dds_ptr);
      }
      else
      {
         /* check that this is valid start op, non-duplicate, non-error */
         if ( (GETC_BUSMEM (dds_ptr,
                            (offsetof( tca_t, cusyn))) != WRK.cusyn_save) &&
              (GETC_BUSMEM (dds_ptr,
                            (offsetof( tca_t, dpsstat))) == ACK) &&
              (WRK.interface_state != CONNECTED_A_ACTIVE))
         {
            StartOp (dds_ptr);             /* handle new and valid start op */
         }
         else
         {
            /* check for out of sync (non-duplicate) start op */
            if ( GETC_BUSMEM (dds_ptr,
                (offsetof( tca_t, cusyn))) != WRK.cusyn_save )
            {
               SendSS (dds_ptr, (uchar)SS_ERFR, (uchar)ERFR_04, (uchar)0);
            }
            /* note that duplicate start op gets ignored */
         }
      }
   } /* end "Start Operation" processing */

   return;

} /* end dftnsProcessInterrupt */
/*PAGE*/
/*
** handle the nitty-gritty details of processing a time out
*/
void dftnsProcessTimeout (DDS_DATA *dds_ptr)
{

   BroadcastUnSol (dds_ptr, OR_CU_DOWN, 0);

   BroadcastUnSol (dds_ptr, OR_CHK_STAT_MSK,
      (BROADCAST_MSK | SS_6_MASK | SS_CU_TIMEOUT));

   Restart (dds_ptr);

   return;

} /* end dftnsProcessTimeout */
/*PAGE*/
/*
** got a permanent PIO error so force a simulated close
*/
void dftnsPIOError (DDS_DATA *dds_ptr)
{
   int                 sessndx;

   C327TRACE2 ("PIOE", dds_ptr );

   /* make sure this card still hooked up */
   if (WRK.interface_state == NOT_CONNECTED)
      return;

   WRK.interface_state     = NOT_CONNECTED;

   WRK.device_state        = OFF_LINE_TO_CU;

   /* kill the timers */
   SetTimer (dds_ptr, 1, TIMER_TYPE_NONE, 0);
   SetTimer (dds_ptr, 2, TIMER_TYPE_NONE, 0);

   ForceClose (dds_ptr);

   /* acknowlege any unacknowledged commands */
   AckAllCmd (dds_ptr);

   /* allow any pending halts to complete */
   for (sessndx = 0; sessndx < WRK.num_sess; sessndx++)
   {
      if (TBL(sessndx).halt_pending == TRUE)
      {
         TBL(sessndx).halt_pending = FALSE;
         TBL(sessndx).lt_state     = LT_OFFLINE;
         C327TRACE3 ("STAT", TBL(sessndx).lt_state, sessndx);
      }
   }

   return;

} /* end dftnsPIOError */

/*
** ALL CODE ABOVE THIS POINT OPERATES AT THE OFFLEVEL INTERRUPT LEVEL
*/
/*PAGE*/
/*
** ALL CODE BELOW THIS POINT OPERATES AT THE HARDWARE OR CLOCK INTERRUPT LEV
**
** Send expedited status to the control unit.
** CAUTION! Callable ONLY by dftnsTimer and operates at clock interrupt level
*/

void TimerSendES (DDS_DATA *dds_ptr)
{

   C327UNPTRACE2 ("SNES", dds_ptr);

   /* Set the Expedited Status Post/Acknowledgement flag byte */
   PUTC_BUSMEM (dds_ptr, offsetof( tca_t, exfak), UNACK);

   /* Set the Expedited Status value in the adapter mem to BUSY */
   PUTC_BUSMEM (dds_ptr, offsetof( tca_t, exfrq), ES_BUSY);

   /* Set the Logical Terminal Address byte to physical device */
   PUTC_BUSMEM (dds_ptr, offsetof( tca_t, exflt), EXFLT_FF);

   /* Set the Expedited Status PARMs in the adapter mem to zero */
   PUTC_BUSMEM (dds_ptr, offsetof( tca_t, exfp1), 0);
   PUTC_BUSMEM (dds_ptr, offsetof( tca_t, exfp2), 0);
   PUTC_BUSMEM (dds_ptr, offsetof( tca_t, exfp3), 0);
   PUTC_BUSMEM (dds_ptr, offsetof( tca_t, exfp4), 0);

   /* RAS.cnt_exp_req ++                Increment the expedited RAS counter */

   /* 
   ** Set request poll bit 
   */
   PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg,
               (WRK.conn_ctrl_save | CONN_CTRL_POLL_REQ) );

   C327PERF( 0x255 );

   return;

} /* end TimerSendES */
/*PAGE*/
/*
** Main timer entry entered periodically to update timer counts and check
** for timeouts.  Some are handled immediately, but int ones are queued for
** for later processing by OffLevel.
** Caution! This code operates at a very high priority, so keep it short!
*/
int dftnsTimer(void)
{
   DDS_DATA            *dds_ptr;
   unsigned char       temp_exfak;
   boolean             need_OffLevel_to_run;
   int                 comndx;

   /* C327TRACE1 ("TIMS"); */

   need_OffLevel_to_run = FALSE;                      /* initialize */

   for (comndx = 0; comndx < MAX_MINORS; comndx++)
   {

      if (CMNADP(comndx).dds_ptr != NULL)
      {

         dds_ptr = CMNADP(comndx).dds_ptr;

         if (WRK.timer_type_1 != TIMER_TYPE_NONE)
         {

            WRK.timer_count_1--;

            if (WRK.timer_count_1 <= 0)
            {

               switch (WRK.timer_type_1)
               {
                  case TIMER_TYPE_CU_INACTIVE:
                     if (WRK.WCUS_10_received == TRUE)
                     {
                        /*
                        ** we think cu has died, but try to elicit a response
                        */
                        TimerSendES (dds_ptr);
                     }

                     UnprotectedSetTimer(dds_ptr, 1,
                                         TIMER_TYPE_CU_DEAD, 10000);

                     break;

                  case TIMER_TYPE_CU_DEAD:
                     C327UNPTRACE2 ("timd", dds_ptr);

                     /* kill the timers */
                     UnprotectedSetTimer (dds_ptr, 1, TIMER_TYPE_NONE, 0);
                     UnprotectedSetTimer (dds_ptr, 2, TIMER_TYPE_NONE, 0);

                     /* see if we are in middle of open */
                     if (WRK.open_state == OPEN_PENDING)
                     {
                        /* this allows wait loop in dftnsOpen to proceed */
                        WRK.open_state = OPEN_TIMEOUT;
                     }
                     else
                     {
                        /* we need to do a restart, but there's not enough */
                        /* time to do needed stuff here at clock intr level */
                        /* insert dds_ptr in c327OffLevel queue */
                        timr_que.dds_ptr[timr_que.ndx_in++] = dds_ptr;

                        if (timr_que.ndx_in >= TIMR_QUE_SIZE)
                           timr_que.ndx_in = 0;

                        /* remember we may need to start OffLevel */
                        need_OffLevel_to_run = TRUE;
                    }
                    break;

                 case TIMER_TYPE_RESTART:
                    C327UNPTRACE2 ("timi", (int)dds_ptr);

                    /* kill the timers */
                    UnprotectedSetTimer (dds_ptr, 1, TIMER_TYPE_NONE, 0);
                    UnprotectedSetTimer (dds_ptr, 2, TIMER_TYPE_NONE, 0);

                    WRK.waiting_for_restart = TRUE;
                    /* we need to do a InitCard, but there's not enough */
                    /* time to do needed stuff here at clock intr level */
                    /* insert dds_ptr in c327OffLevel queue */
                    timr_que.dds_ptr[timr_que.ndx_in++] = dds_ptr;

                    if (timr_que.ndx_in >= TIMR_QUE_SIZE)
                       timr_que.ndx_in = 0;

                    /* remember we need to start OffLevel */
                    need_OffLevel_to_run = TRUE;

                    break;

                 default:
                    /* we shouldn't get here */
                    WRK.timer_type_1 = TIMER_TYPE_NONE;

                    break;

               } /* end switch */
            }
         }

         if (WRK.timer_type_2 != TIMER_TYPE_NONE)
         {
            WRK.timer_count_2--;
            if (WRK.timer_count_2 <= 0)
            {

               C327UNPTRACE2 ("tim2", dds_ptr);

               temp_exfak = GETC_BUSMEM (dds_ptr, (offsetof( tca_t, exfak)));

               switch (WRK.timer_type_2)
               {
                  case TIMER_TYPE_HOST_TIMING:

                     if (temp_exfak == UNACK)
                     {

                        C327UNPTRACE2 ("timt", dds_ptr);

                        /* give it one more timer tick to get acknowledge */
                        UnprotectedSetTimer(dds_ptr, 2, TIMER_TYPE_HOST_TIMING,
                                            C327_TIMER_INTERVAL);
                     }
                     else
                     {

                        C327UNPTRACE2 ("timr", dds_ptr);

                        TimerSendES (dds_ptr);

                        /* now restart ourselves */
                        UnprotectedSetTimer(dds_ptr, 2,
                                            TIMER_TYPE_HOST_TIMING, 500);
                     }
                     break;

                  case TIMER_TYPE_DEVICE_TIMING:

                     if (temp_exfak == UNACK)
                     {
                        C327UNPTRACE2 ("timm", dds_ptr);

                        /* give it one more timer tick to get acknowledge */
                        UnprotectedSetTimer(dds_ptr, 2,
                               TIMER_TYPE_DEVICE_TIMING, C327_TIMER_INTERVAL);
                     }
                     else
                     {
                        C327UNPTRACE2 ("time", dds_ptr);

                        TimerSendES (dds_ptr);

                        /* now restart ourselves */
                        UnprotectedSetTimer (dds_ptr, 2,
                                             TIMER_TYPE_DEVICE_TIMING, 500);
                     }
                     break;

                  default:
                     /* we shouldn't get here */
                     C327TRACE1 ("TIMC");

                     WRK.timer_type_2 = TIMER_TYPE_NONE;

                     break;

               } /* end switch */
            }
         }
      } /* end if */
   }

   if ( need_OffLevel_to_run == TRUE )
      i_sched( &c327offlstruct );

   /* unless there are no cards active, re-schedule ourselves to run again */
   if (CMNMIS.adapters_in_use >= 1)
      timeout((int (*)(void))dftnsTimer,0,C327_TIMEOUT_INTERVAL);

   /* C327TRACE1 ("TIME"); */

   return(0);

}  /* end dftnsTimer */
