static char sccsid[] = "@(#)26	1.23  src/bos/kernext/c327/dftnsCmd.c, sysxc327, bos411, 9430C411a 7/27/94 09:32:13";

/*
 * COMPONENT_NAME: (SYSXC327) c327 dft cmd device head entry points
 *
 * FUNCTIONS:    dftnsClose(), dftnsHalt(), dftnsStart(), dftnsWrite(), 
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
** dftnsCmd - command entry points from the device head and their local subr
**
**
** For device head:      dftnsOpen 
** For device head:      dftnsStart
** For device head:      dftnsClose
** Local:                ValidNetID
** For device head:      dftnsWrite
** For device head:      dftnsSend 
** For device head:      dftnsHalt
*/
 
#include <stddef.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>
#include <sys/intr.h>

#ifdef _POWER_MP
#include <sys/lock_def.h>
#else
#include <sys/lockl.h>
#endif

#include <sys/param.h>
#include <sys/io3270.h>
#include "c327dd.h"
#include "dftnsDcl.h"
#include "dftnsSubr.h"

extern    COMMON_AREA dftnsCommon;

/*PAGE*/
/*
 * NAME: dftnsStart()
 *                                                                  
 * FUNCTION: service device head request to 
 *           start a session on an open adapter
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

int dftnsStart(DDS_DATA *dds_ptr, char sess_type, 
               char prnt_addr, Data_Buffer *buffer_ptr, boolean *non_sna)
{
   int             session_index;
   int             sessndx;
   boolean         foundit;
   ASYNC_REQ       async_req;

   C327TRACE5 ("STRB", dds_ptr,   sess_type,
               prnt_addr, buffer_ptr );

   if (CMNADP(HDR.minor_number).dds_ptr != dds_ptr) /* is DDS in common TBL */
   {
      C327TRACE3 ("stre", dds_ptr, RC_NOT_OPEN);
      return (RC_NOT_OPEN);
   }

   if ((buffer_ptr->dbhead.buff_size < 1024) ||   /* check buf for */
       (buffer_ptr->dbhead.data_offset > 3))  /* invalid parms */
   {
      C327TRACE3 ("stre", dds_ptr, RC_INVAL_PARMS);
      return (RC_INVAL_PARMS);
   }

   if (WRK.open_state == OPEN_PENDING)    /* insure open is not in progress */
   {
      C327TRACE3 ("stre", dds_ptr, RC_OPEN_BUSY);
      return (RC_OPEN_BUSY);
   }

   if (WRK.open_state != DD_OPENED)       /* check that device head is open */
   {
      C327TRACE3 ("stre", dds_ptr, RC_NOT_OPEN);
      return (RC_NOT_OPEN); /* is this necessary ????? */
   }

   session_index = -1;          /* search for a valid and available session */

   for (sessndx = 0; sessndx < WRK.num_sess; sessndx++)
   {
      /* is there an unassigned, valid, and available session */
      if ( TBL(sessndx).lt_state==LT_OFFLINE )
      {
         if (sess_type == SESS_TYPE_TERMINAL) /* app wants terminal session*/
         {
            if (!TBL(sessndx).prt_bit_map )
            {
               session_index = sessndx;
               break;
            }
         }

         if (sess_type == SESS_TYPE_PRINTER)   /* app wants prnter session */
         {
            if (TBL(sessndx).prt_bit_map )
            {
               if (prnt_addr == TBL(sessndx).lt_id)
               {
                   session_index = sessndx;
                   break;
               }
            }
         }
      }
   }

   if (session_index < 0)       /* have we found a valid session to start */
   {
      C327TRACE3 ("stre", dds_ptr, RC_NO_SESS_AVAIL);
      return (RC_NO_SESS_AVAIL);
   }

   /***** at this point, this device head is doing a valid start *****/

   /* record the network_id code in the netid table */
   TBL(session_index).network_id.adapter_code = HDR.minor_number + 1;

   TBL(session_index).network_id.session_code = session_index + 1;

   /* record where the read buffer is and how much data we've read */
   TBL(session_index).read_buff_addr = buffer_ptr;

   TBL(session_index).read_data_addr = (char *)
       ((&buffer_ptr->buf_start) + (buffer_ptr->dbhead.data_offset));

   buffer_ptr->dbhead.data_len = 0;

   TBL(session_index).lt_state = LT_ONLINE_PEND;  /* chg to on-line pending */
   C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
/* 
** send async status to cu to request on-line 
*/
   TBL(session_index).bind_recvd = FALSE;  /* Just make sure state ok */
   TBL(session_index).first_inbound = TRUE; /* State is first inbound */
   *non_sna = WRK.non_sna;  /* pass flag back to calling routine */
   if (WRK.non_sna == TRUE) {
       BuildAsyncReq ( (uchar)AEDV_01, (uchar)TBL(session_index).lt_bit_map, 
                       (uchar)TBL(session_index).prt_bit_map, (uchar)0,
                       (uchar)AEDV, (uchar)AEDV_AD, &async_req);
   }
   else {
       BuildAsyncReq ( (uchar)AEDV_01, (uchar)TBL(session_index).lt_bit_map, 
                       (uchar)0,(uchar)0,(uchar)AEDV,(uchar)AEDV_AD,&async_req);
   }

   C327TRACE2("jsas",session_index);
   SendAS (dds_ptr, &async_req);
   C327TRACE2("jsae",session_index);

   foundit = FALSE; /* search for any ack pending for this dev head */

   for (sessndx = 0; sessndx < WRK.num_sess; sessndx++)
   {
      if ( TBL(sessndx).ack_pending == TRUE )
      {
         foundit = TRUE;
         break;
      }
   }

   if (foundit == TRUE)  /* if ack pending, do ack */
   {
      InterruptDH (dds_ptr, session_index, INTR_SOL_START, 
                   OR_NO_ERROR, 0);

      if (WRK.WCUS_30_pending == TRUE)
      {
         InterruptDH (dds_ptr, session_index, INTR_UNSOL,
                      OR_CHK_STAT_MSK, WRK.cu_error_o);
      }

      C327TRACE3 ("stre", dds_ptr, RC_OK_NONE_PEND);

      return (RC_OK_NONE_PEND);
   }
   else
   {
      TBL(session_index).cmd_start_pending = TRUE;

      C327TRACE3 ("stre", dds_ptr, RC_OK_INTR_PEND);

      return (RC_OK_INTR_PEND);
   }
}                                                        /* end dftnsStart */

/*PAGE*/
/*
 * NAME: dftnsClose()
 *                                                                  
 * FUNCTION: service a device head request to 
 *           close the comm link to the control unit
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

int dftnsClose(DDS_DATA *dds_ptr)
{
   int maximum_waits;

   C327TRACE2 ("CLOB", dds_ptr );

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

   if (CMNADP(HDR.minor_number).dds_ptr != dds_ptr) /* are we for sure open */
   {

#ifdef _POWER_MP
      lock_done((complex_lock_t)&CMNMIS.common_locked );
#else
      unlockl ( (lock_t *)&CMNMIS.common_locked );  /* unlock common area   */
#endif

      C327TRACE3 ("cloe", dds_ptr, RC_NOT_OPEN);

      return (RC_NOT_OPEN);
   }

   maximum_waits = 300;                           /* are we doing an open ? */

   while ( (WRK.open_state == OPEN_PENDING) && (maximum_waits-- > 0))
      delay ( (int)HZ/5 );                  /* compute delay in clock ticks */

   if (WRK.open_state == OPEN_PENDING)
   {

#ifdef _POWER_MP
      lock_done((complex_lock_t)&CMNMIS.common_locked );
#else
      unlockl ( (lock_t *)&CMNMIS.common_locked );  /* unlock common area   */
#endif

      C327TRACE3 ("cloe", dds_ptr, RC_OPEN_BUSY);

      return (RC_OPEN_BUSY);
   }

   WRK.num_opens = 0;

   ShutDown ( dds_ptr );                       /* halt all sessions & close */

#ifdef _POWER_MP
      lock_done((complex_lock_t)&CMNMIS.common_locked );
#else
      unlockl ( (lock_t *)&CMNMIS.common_locked );  /* unlock common area   */
#endif

/*
** CAUTION! Code above is not re-entrant
*/

   C327TRACE3 ("cloe", dds_ptr, RC_OK_NONE_PEND);

   return (RC_OK_NONE_PEND);

} /* end dftnsClose */
/*PAGE*/
/*
 * NAME: dftnsWrite()
 *                                                                  
 * FUNCTION: service device head request to 
 *           send a buffer of data to the 3270
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

int dftnsWrite (NETWORK_ID network_id, 
                Data_Buffer *buffer_ptr, uchar write_option)
{
   DDS_DATA        *dds_ptr;         /* pointer to appropriate DDS_DATA */
   int             session_index;    /* index into TBL for this session */
   int             validnetid_code;  /* saved return from ValidNetID */
   ASYNC_REQ       async_req;

   C327TRACE5 ("WRIB", network_id.adapter_code,
       network_id.session_code, buffer_ptr, write_option);

   /* decode network_id, get dds_ptr, & session_index */
   if ((validnetid_code=ValidNetID(network_id,&dds_ptr,&session_index)) != 0)
   {
      C327TRACE4 ("wrie", network_id.adapter_code,
                  network_id.session_code, RC_CMD_NOT_VALID);
      C327TRACE2 ("wri1",validnetid_code);

      return (RC_INVAL_NETID);
   }

   /* make sure adapter is open and device is on line to the host */
   if ( (WRK.open_state != DD_OPENED ) || (WRK.device_state <= ON_LINE_TO_CU) )
   {
      C327TRACE4 ("wrie", network_id.adapter_code,
                  network_id.session_code, RC_CMD_NOT_VALID);

      C327TRACE3 ("wri2", WRK.open_state, WRK.device_state );

      return (RC_CMD_NOT_VALID);
   }

   if (TBL(session_index).program_check == TRUE) /* clear active prog check */
   {

      TBL(session_index).program_check = FALSE;

      /* Tell the emulator the program check is over */
      InterruptDH (dds_ptr, session_index, 
                   INTR_UNSOL, OR_CHK_STAT_MSK, 0);
   }

   /*
   ** We should be here under only two conditions:
   ** 1) the session is partly thru a host read, and the dh has the data ready
   ** 2) this session is idle, and the device head has data to send
   **  In the first case, we may or may not have already gotten the RDAT
   **    from the control unit.  If we have, we can send the data.  Otherwise,
   **    we can only record the information and let OffLevel send the data.
   **  In the second case, we can tell the CU we have data, but it won't be
   **    sent until OffLevel receives the PDAT.
   */

   C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);

   /* check if we should be here at all */
   if ( ( (TBL(session_index).lt_state == LT_RM_LOCKED)     ||
          (TBL(session_index).lt_state == LT_RM_LOCKED_1)   ||
          (TBL(session_index).lt_state == LT_RM_LOCKED_2)   ||
          (TBL(session_index).lt_state == LT_RM_LOCKED_3)   ||
          (TBL(session_index).lt_state == LT_ONLINE)          ) &&
          (TBL(session_index).cmd_write_pending == FALSE      ) &&
          (WRK.rm_cmd_write_pending == FALSE                  ) )
   {
    /* set up the addresses */

    /* write command buffer header address */
    TBL(session_index).write_buff_addr = buffer_ptr;

    /* starting address of data buffer from user */
    TBL(session_index).write_orig_data_addr = (char *)
       (&buffer_ptr->buf_start + buffer_ptr->dbhead.data_offset);

    /* The length of the whole data buffer from user */
    TBL(session_index).write_orig_data_len = buffer_ptr->dbhead.data_len;

    /* Pointer to the current place in data buffer. Initially this points
       to the beginning */
    TBL(session_index).write_data_addr = 
        TBL(session_index).write_orig_data_addr;

    /* The current length of the data. This is used as a remaining length if
       the TCA buffer is filled and there is still data to write. Initially
       this is set to the original length */
    TBL(session_index).write_data_len = 
        TBL(session_index).write_orig_data_len;

    /* Flag indicating that there is a write in progress. Since this is a
       brand-new write, we want to start from the beginning */
    TBL(session_index).write_in_progress = FALSE;

    if ((write_option == 0) || (WRK.non_sna == FALSE))
       TBL(session_index).BSC_transparency = FALSE;
    else
       TBL(session_index).BSC_transparency = TRUE;
   }
   else
   {
      C327TRACE4 ("wrie", network_id.adapter_code,
                  network_id.session_code, RC_HOST_CONTEN );

      return (RC_HOST_CONTEN);
   }

/* special case for SNA with non Enhanced buffer management support.
   We need to check if a bind/unbind was recieved, since this is treated
   special. If it was then we need to see if an RDAT was already done */
   
   if (TBL(session_index).bind_recvd == TRUE) {
      TBL(session_index).cmd_write_pending = TRUE;
      if (TBL(session_index).lt_state == LT_ONLINE) {
      /* In this case, we have not received an RDAT yet. We will simply
         wait for the interrupt to arrive. We do not issue an AEEP in this
         case because an FCIR was alraedy issued */
         
         C327TRACE1("bnd1");
         return (RC_OK_INTR_PEND);  /* tell tca to wait for interrupt */
      }
      else if (TBL(session_index).lt_state == LT_RM_LOCKED_1) {
      /* In this case, the RDAT has already arrived. We need to call the
         SNA data xfer routine and reset our flags */
         C327TRACE1("bnd2");
    
         segment_SNA_data (dds_ptr,session_index);
         TBL(session_index).lt_state = LT_ONLINE;
         TBL(session_index).bind_recvd = FALSE;

         /* send function complete */
         SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);

         /* ack the command to stay in-state */
         AckCmd (dds_ptr, session_index, INTR_SOL_WRITE);

         return (RC_OK_NONE_PEND);  /* tell tca that write finished */
      }
      else {
         return (RC_CMD_NOT_VALID);
      }
   }
      
/* 
** check if this is a normal write (not in response to a host read) 
*/
   /* added non-sna check just to make sure that we always start by sending
      AEEP if running SNA. */
   if ((TBL(session_index).lt_state == LT_ONLINE) || (WRK.non_sna == FALSE))
   {
      C327PERF( 0x0400 );            /* dftio start */
      C327PERF( 0x0450 );            /* write data  */
      TBL(session_index).cmd_write_pending = TRUE;

      BuildAsyncReq ((uchar)0, (uchar)0, (uchar)0, 
           (uchar)0, (uchar)AEEP, (uchar)TBL(session_index).lt_id, &async_req);

/* 
** Req of ctrl unt Inbnd Evnt Pend 
*/
      SendAS (dds_ptr, &async_req);

      C327TRACE4 ("wrie", network_id.adapter_code,
                  network_id.session_code, RC_OK_INTR_PEND);

      C327PERF( 0x0401 );            /* dftio end   */
      return (RC_OK_INTR_PEND);
   }
   else
   {
/* 
** this must be write in response to host read 
*/
      C327PERF( 0x0300 );            /* DFT chkp start */
      C327PERF( 0x0350 );            /* write RM cmd   */

      WRK.rm_cmd_write_pending = TRUE;

/* 
** Test if the cu is waiting for this data (we received RDAT) 
*/
      if ((TBL(session_index).lt_state == LT_RM_LOCKED_1) ||
          (TBL(session_index).lt_state == LT_RM_LOCKED_3))
      {
         TBL(session_index).lt_state = LT_RM_LOCKED_3;  /* Update the state */
         C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);

/* 
** Move data to the adapter 
*/
         XmitData (dds_ptr, session_index, (boolean)TRUE);
/* 
** Send Sync Stat to CU 
*/
         SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);

      }
      else
      {
/* 
** we need RDAT before we can send data 
*/
         TBL(session_index).lt_state = LT_RM_LOCKED_2; /* Update session st */
         C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);
      }

      C327TRACE4 ("wrie", network_id.adapter_code,
                  network_id.session_code, RC_OK_NONE_PEND);

      C327PERF( 0x0301 );                  /* chkp end */

      return (RC_OK_NONE_PEND);
   }
} /* end dftnsWrite */
/*PAGE*/
/*
** service a device head request to terminate an active session
*/


int dftnsHalt (NETWORK_ID network_id)
{
   unsigned char   temp_cuslvl_2;
   DDS_DATA        *dds_ptr;
   ASYNC_REQ       async_req;
   int             session_index;
   int             validnetid_code;
   int             maximum_waits;

  C327TRACE3("HALB",network_id.adapter_code,network_id.session_code);

   if ((validnetid_code = 
        ValidNetID(network_id,&dds_ptr,&session_index)) != 0)
   {                                           /* decode & check network_id */

      C327TRACE2 ("hale", validnetid_code);

      return (RC_INVAL_NETID);
   }

   if ( (WRK.open_state != DD_OPENED) || (WRK.device_state <= ON_LINE_TO_CU) )
   {                                  /* is adapter open and device on line */

      C327TRACE3 ("hal2", WRK.open_state, WRK.device_state);

      return (RC_CMD_NOT_VALID);
   }

   /* set flag */
   TBL(session_index).halt_pending = TRUE;

   if (TBL(session_index).cmd_start_pending == TRUE) /* ack strt cmd pendng */
      AckCmd (dds_ptr, session_index, INTR_SOL_START);

   if (TBL(session_index).cmd_write_pending == TRUE) /* ack write cmd pendng*/
      AckCmd (dds_ptr, session_index, INTR_SOL_WRITE);

   if (TBL(session_index).ack_pending == TRUE)       /* test for ack pending*/
   {
      TBL(session_index).ack_pending = FALSE;

      if (TBL(session_index).lt_state == LT_RM_LOCK)
         SendSS (dds_ptr, (uchar)SS_FCIR, (uchar)FCIR_00, (uchar)0);
      else if (TBL(session_index).lt_state != LT_READ_LOCK_DEF)
         SendSS (dds_ptr, (uchar)SS_FC, (uchar)0, (uchar)0);
   }

   temp_cuslvl_2 = GETC_BUSMEM (dds_ptr, ( offsetof( tca_t, cuslvl_2) ) );

   if ((temp_cuslvl_2 & 0x01) != 0)         /* does cu support AEDV stat ?  */
   {
      BuildAsyncReq((uchar)AEDV_02,(uchar)TBL(session_index).lt_bit_map,
                    (uchar)0,(uchar)0,(uchar)AEDV,(uchar)AEDV_AD, &async_req);

      SendAS (dds_ptr, &async_req);
   }
   else
   {
      BuildAsyncReq ( (uchar)AEDV_02, (uchar)0, (uchar)0, (uchar)0, 
                      (uchar)AEDV, (uchar)AEDV_AD, &async_req );

      SendAS (dds_ptr, &async_req);
   }

   maximum_waits = 300;

   while ( ((int)TBL(session_index).lt_state > (int)LT_OFFLINE ) &&
           (maximum_waits-- > 0))            /* wait until done or time out */
   {
      delay ( (int)HZ/5 );                  /* compute delay in clock ticks */
   }

   TBL(session_index).network_id.adapter_code = 0;

   TBL(session_index).network_id.session_code = 0;
/* 
** clr session for next start
*/
   if ( (int)TBL(session_index).lt_state > (int)LT_NO_SESSION )
      TBL(session_index).lt_state = LT_OFFLINE;
   C327TRACE3 ("STAT", TBL(session_index).lt_state, session_index);

   return (RC_OK_NONE_PEND);
}                                                          /* end dftnsHalt */
