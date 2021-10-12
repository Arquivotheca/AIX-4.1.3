static char sccsid[] = "@(#)04  1.30.1.7  src/bos/kernext/dlc/lan/lansta1.c, sysxdlcg, bos411, 9439A411b 9/26/94 18:17:04";

/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: lansta1.c
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
                                                                      */

#include <fcntl.h>
#include <sys/types.h>
#include <net/spl.h>
#include <sys/mbuf.h>
#include <sys/file.h>
#include <sys/malloc.h>
#include <sys/errno.h>
#include "dlcadd.h"
#include <sys/gdlextcb.h>

/* <<< feature CDLI >>> */
#include <sys/ndd.h>
#include <sys/ndd_var.h>
#include <sys/cdli.h>
/* <<< end feature CDLI >>> */

#include <sys/trchkid.h>
#if defined(TRL) || defined(FDL)
#define TRLORFDDI
#endif

/* <<< feature CDLI >>> */
#ifdef   TRL
#include <sys/cdli_tokuser.h>
#include <sys/trlextcb.h>
#endif /* TRL */
#ifdef   FDL
#include <sys/cdli_fddiuser.h>
#include <sys/fdlextcb.h>
#endif /* FDDI */
#ifndef  TRLORFDDI
#include <sys/cdli_entuser.h>
#endif /* not TRLORFDDI */
/* <<< end feature CDLI >>> */

#include "lancomcb.h"
#include "lanstlst.h"
#include "lansplst.h"
#include "lanrstat.h"
#include "lanport.h"

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  lansta                                               */
/*                                                                    */
/* descriptive name:  link station                                    */
/*                                                                    */
/* function:  handles all link station protocol functions, as well as */
/*            the opening and closing of the logical link.            */
/*                                                                    */
/* input:  input station command                                      */
/*                                                                    */
/* output:  various send packets and result buffers                   */
/*                                                                    */
/*** end of specifications ********************************************/

lansta(p,input_command)
  register struct port_dcl *p;
  int      input_command;
{
  TRACE1(p, "STAb");

  /********************************************************************/
  /* get full addressability to the link station and associated sap.  */
  /********************************************************************/

  p->sta_ptr = (struct station_cb *)p->station_list[p->stano].sta_cb_addr;
  p->sapno = p->station_list[p->stano].sapnum;
  p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].sap_cb_addr;
 
/* LEHb defect 44499 */
/* moved 24 lines to lansta2 */
/* LEHe */

  switch                               /* input command from the link 
                                          manager                     */
     (input_command)
    {
      case                             /* the input command = write   
                                          command                     */
         (WRITENO) :

        /*------------------------------------------------------------*/
        /* write                                                      */
        /*------------------------------------------------------------*/

          {

            /**********************************************************/
            /* call the write command routine.                        */
            /**********************************************************/

            write_command(p);

            /**********************************************************/
            /* expect a wakeup from the dlc manager when the ring     */
            /* queue has been emptied, or a wakeup count has been     */
            /* reached.                                               */
            /**********************************************************/

          } 
        break;
      case                             /* the input command = receive 
                                          completion.                 */
         (RCV_CMPLNO) :

        /*------------------------------------------------------------*/
        /* receive completion                                         */
        /*------------------------------------------------------------*/

          {

            /**********************************************************/
            /* call the receive completion from device handler routine*/
            /**********************************************************/

            sta_rcv_cmpl(p);
          } 
        break;

/* <<< feature CDLI >>> */
/* <<< removed case (WRT_SINGLE_CMPLNO) >>> */
/* <<< end feature CDLI >>> */

      case                             /* the input command = t1 timer
                                          completion.                 */
         (T1_TIMEOUT_CMPLNO) :

        /*------------------------------------------------------------*/
        /* t1 timeout completion                                      */
        /*------------------------------------------------------------*/

          {

            /**********************************************************/
            /* call the repoll timeout routine.                       */
            /**********************************************************/

            t1_timeout_cmpl(p);
          } 
        break;
      case                             /* the input command = t2 timer
                                          completion.                 */
         (T2_TIMEOUT_CMPLNO) :

        /*------------------------------------------------------------*/
        /* t2 timeout completion                                      */
        /*------------------------------------------------------------*/

          {

            /**********************************************************/
            /* call the acknowledgement timeout routine.              */
            /**********************************************************/

            t2_timeout_cmpl(p);
          } 
        break;
      case                             /* the input command =         
                                          inact/abort timer           
                                          completion.                 */
         (T3_TIMEOUT_CMPLNO) :

        /*------------------------------------------------------------*/
        /* t3 timeout completion                                      */
        /*------------------------------------------------------------*/

          {

            /**********************************************************/
            /* call the inact/abort timeout routine.                  */
            /**********************************************************/

            t3_timeout_cmpl(p);
          } 
        break;
/* LEHb defect 43788 */
/* delete LOW_ON_BUFFERSNO case */
/* delete BUFFERS_NOW_AVAILNO case */
/*LEHe */
      case                             /* the input command = open    */
         (OPENNO) :

        /*------------------------------------------------------------*/
        /* open                                                       */
        /*------------------------------------------------------------*/

          {

            /**********************************************************/
            /* call the open link station routine.                    */
            /**********************************************************/

            open_link_station(p);
          } 
        break;
      case                             /* the input command = call    
                                          completion                  */
         (CALL_CMPLNO) :

        /*------------------------------------------------------------*/
        /* call completion                                            */
        /*------------------------------------------------------------*/

          {

            /**********************************************************/
            /* call the call completion from device handler routine   */
            /**********************************************************/

            call_completion(p);
          } 
        break;
      case                             /* the input command = listen  
                                          completion                  */
         (LISTEN_CMPLNO) :

        /*------------------------------------------------------------*/
        /* listen completion                                          */
        /*------------------------------------------------------------*/

          {

            /**********************************************************/
            /* call the listen completion from device handler routine */
            /**********************************************************/

            listen_completion(p);
          } 
        break;
      case                             /* the input command = close   */
         (CLOSENO) :

        /*------------------------------------------------------------*/
        /* close                                                      */
        /*------------------------------------------------------------*/

          {

            /**********************************************************/
            /* call the close link station routine.                   */
            /**********************************************************/

            close_link_station(p);
          } 
        break;
      case                             /* the input command = local   
                                          busy wakeup                 */
         (LBUSY_WAKEUPNO) :

        /*------------------------------------------------------------*/
        /* wakeup                                                     */
        /*------------------------------------------------------------*/

          {
/* LEHb  defect 43788 */
/* delete 2 lines */
	  /************************************************************/
	  /* call the user receive wakeup routine.                    */
	  /************************************************************/

	  user_rcv_wakeup(p);
	  }
/* LEHe */
        break;
      case                             /* the input command = query   
                                          link station                */
         (QUERYNO) :

        /*------------------------------------------------------------*/
        /* query                                                      */
        /*------------------------------------------------------------*/

          {

            /**********************************************************/
            /* call the query command routine.                        */
            /**********************************************************/

            query_cmd(p);
          } 
        break;
      default  :                       /* link manager sent invalid   
                                          command.                    */
          {

            /**********************************************************/
            /* call error log - link station invalid command, plc     */
            /* return code = error detected in code.                  */
            /**********************************************************/

            lanerrlg(p, ERRID_LAN8011, NON_ALERT, PERM_PLC_ERR, 
               DLC_ERR_CODE, FILEN, LINEN);
          } 
    }                                  /* end switch                  */
 
  if                                   /* the station control block is
                                          still in use                */
     (p->station_list[p->stano].in_use == TRUE)
    {
 
      if                               /* able to transmit i-frames   */
         (p->sta_ptr->iframes_ena == TRUE)
        {

          /************************************************************/
          /* call the transmit i-frames routine.                      */
          /************************************************************/

          tx_iframes(p);
        } 
    } 

  /********************************************************************/
  /* return to the link manager.                                      */
  /********************************************************************/

  TRACE1(p, "STAe");
}                                      /* end of lansta               */
tx_iframes(p)
  register struct port_dcl *p;
{

  /********************************************************************/
  /* calculate the previously transmitted window as (vs minus va)     */
  /* modulo 256, divided by 2.                                        */
  /********************************************************************/

  p->sta_ptr->txw_ct = (((p->sta_ptr->vs-p->sta_ptr->va)%256)/2);

  /********************************************************************/
  /* calculate the remaining transmit window as (max transmit window  */
  /* minus previous transmit window).                                 */
  /********************************************************************/

  p->sta_ptr->txw_ct = (p->sta_ptr->ls_profile.xmit_wind-p->sta_ptr->txw_ct);

  /********************************************************************/
  /* not at max transmit count, and there are more buffers in in the  */
  /* transmit queue.                                                  */
  /********************************************************************/
 

  while ((p->sta_ptr->txw_ct != 0) && (p->sta_ptr->vs != 
     (p->sta_ptr->txq_input *2)))
    {

      /****************************************************************/
      /* call the handle i-frame routine.                             */
      /****************************************************************/

      handle_iframe(p);
 
      if                               /* an i-frame build failed     */
         (p->m == 0)
        break;
    } 
}                                      /* end tx_iframes;             */
handle_iframe(p)
  register struct port_dcl *p;
{
  ulong    dlctype;                    /* lan type                    */
  ulong_t  port_sta;                   /* station number and port     
                                          number                      */

  TRACE2(p, "HIFb", p->sta_ptr->vs);

  /********************************************************************/
  /* fetch the next i-frame from the transmit queue                   */
  /********************************************************************/

  p->m = (struct mbuf *)p->sta_ptr->transmit_queue[p->sta_ptr->vs/2].buf;

  /********************************************************************/
  /* call the protocol specific build iframe routine                  */
  /********************************************************************/

  g_build_iframe(p);
 
  if                                   /* the build was successful    */
     (p->m != 0)
    {
 
      if                               /* data frames transmitted     
                                          counter not at maximum value*/
	 (p->sta_ptr->ras_counters.counters.data_pkt_sent != MINUS_ONE)

        /**************************************************************/
        /* increment count of data frames transmitted                 */
        /**************************************************************/

        ++p->sta_ptr->ras_counters.counters.data_pkt_sent;
 
      if                               /* this is a retransmission of 
                                          the same I-frame            */
         (p->sta_ptr->vs != p->sta_ptr->xmit_vs)
        {
 
          if                           /* data frames retransmitted   
                                          counter not at maximum value*/
             (p->sta_ptr->ras_counters.counters.data_pkt_resent != 
             MINUS_ONE)

            /**********************************************************/
            /* increment count of data frames retransmitted           */
            /**********************************************************/

            ++p->sta_ptr->ras_counters.counters.data_pkt_resent;
        } 

      /****************************************************************/
      /* terminate any acknowledgement delay (t2) timeout             */
      /****************************************************************/

      p->station_list[p->stano].t2_ctr = -1;
      p->station_list[p->stano].t2_ena = FALSE;
 
      if                               /* t3 timer state = inactivity */
         (p->sta_ptr->t3_state == T3_INACT)
        {

          /************************************************************/
          /* terminate any inactivity timeout in progress.            */
          /************************************************************/

          p->station_list[p->stano].t3_ctr = -2;
          p->station_list[p->stano].t3_ena = FALSE;
        } 

      /****************************************************************/
      /* restart the t1 reply timeout.                                */
      /****************************************************************/

      p->station_list[p->stano].t1_ctr = p->sta_ptr->resp_to_val;
      p->station_list[p->stano].t1_ena = TRUE;

      /****************************************************************/
      /* set the acknowledgement delay count (ir_ct) to maximum (n3). */
      /****************************************************************/

      p->sta_ptr->ir_ct = p->sta_ptr->ls_profile.rcv_wind;
      TRACE1(p, "BIFe");
#ifdef   DEBUG
 
      if (p->debug)
        {
          printf("index=%d\n", p->sta_ptr->vs/2);
          printf("m=%x word=%x\n", p->sta_ptr->transmit_queue
             [p->sta_ptr->vs/2].buf);
        } 
#endif

/* setup lan and monitor types                                        */

      dlctype = DLC_TRACE_SNDC<<8;
#ifdef   TRL
      dlctype |= DLC_TOKEN_RING;
#endif
#ifdef   FDL
      dlctype |= DLC_FDDI;
#endif
#ifdef   EDL
      dlctype |= DLC_ETHERNET;
#endif
#ifdef   E3L
      dlctype |= DLC_IEEE_802_3;
#endif

      /****************************************************************/
      /* get station number in upper half word and get number from    */
      /* port name in lower half word                                 */
      /****************************************************************/
/* <<< feature CDLI >>> */
#ifndef FDL
      port_sta = ((p->stano<<16)|p->dlc_port.namestr[3]);
#elif FDL
      port_sta = ((p->stano<<16)|p->dlc_port.namestr[4]);
#endif
/* <<< feature CDLI >>> */

/* call trchkgt to record monitor trace                               */

      trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, 0x08, p->sta_ptr->vr, 
         p->sta_ptr->vs, 0, port_sta);

      /****************************************************************/
      /* setup hook id and dlc types for performance trace            */
      /****************************************************************/

      dlctype = HKWD_SYSX_DLC_PERF|DLC_TRACE_SNDIF;
#ifdef   TRL
      dlctype |= DLC_TOKEN_RING;
#endif
#ifdef   FDL
      dlctype |= DLC_FDDI;
#endif
#ifdef   EDL
      dlctype |= DLC_ETHERNET;
#endif
#ifdef   E3L
      dlctype |= DLC_IEEE_802_3;
#endif

/* call trchklt to record entry in preformance trace                  */

      trchklt(dlctype, ((p->sta_ptr->vr<<16)|p->sta_ptr->vs));

/* <<< feature CDLI >>> */
      /****************************************************************/
      /* send the i-frame to the device handler with the i-frame      */
      /* command buffer address.                                      */
      /****************************************************************/

      lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
 
      if                               /* the send state variable is  
                                          equal to the already        
                                          transmitted                 */

      /****************************************************************/
      /* send state variable, ie. all caught up                       */
      /****************************************************************/

         (p->sta_ptr->vs == p->sta_ptr->xmit_vs)
        {

          /************************************************************/
          /* increment the transmitted send state variable (xmit_vs)  */
          /* note - vs bumped +2 modulo 256 to match nr/ns counts.    */
          /************************************************************/

          p->sta_ptr->xmit_vs = (p->sta_ptr->xmit_vs+2)%256;
        } 

      /****************************************************************/
      /* increment the send state variable (vs) and its associated    */
      /* transmit queue index. note - vs bumped +2 modulo 256 to match*/
      /* nr/ns counts.                                                */
      /****************************************************************/

      p->sta_ptr->vs = (p->sta_ptr->vs+2)%256;

      /****************************************************************/
      /* decrement the transmit window count.                         */
      /****************************************************************/

      p->sta_ptr->txw_ct = (p->sta_ptr->txw_ct-1);
    } 
  TRACE1(p, "HIFe");
}                                      /* end handle_iframe;          */

/* LEHb  defect 43788 */
/* This was re-written to support rcvd, rcvx and rcvi independently   */
/* There should not be support for rcvn in lansta1.                   */


user_rcv_wakeup(p)
  register struct port_dcl *p;
{
  struct dlc_chan *c_ptr;

  TRACE1(p, "RQWb");
  if                               /* the user is waiting for a
					  local busy wakeup          */
     (p->station_list[p->stano].wakeup_needed = TRUE)
    {
					/* get the user's channel    */
    c_ptr = p->sap_ptr->user_sap_channel;
    if                                  /* an XID is pending wakeup  */
       (p->sta_ptr->retry_rcvx_buf != 0)
      {

      /****************************************************************/
      /* call the user's receive XID handler                          */
      /****************************************************************/
      p->rc = (*c_ptr->rcvx_fa)(p->sta_ptr->retry_rcvx_buf,
				   &(p->sta_ptr->retry_rcvx_ext),c_ptr);

      if (p->rc == DLC_FUNC_BUSY)       /* not supported              */
					/* set rc arbitrarily invalid so
					   that it will errlog correctly */
	p->rc = -10;

      /****************************************************************/
      /* call wakeup_rc to test return code from function handler     */
      /****************************************************************/
      wakeup_rc(p,p->rc,&(p->sta_ptr->retry_rcvx_buf),ERRID_LAN8084);
      }

    if                                  /* a DGRM is pending wakeup  */
       (p->sta_ptr->retry_rcvd_buf != 0)
      {

      /****************************************************************/
      /* call the user's receive datagram handler                     */
      /****************************************************************/
      p->rc = (*c_ptr->rcvd_fa)(p->sta_ptr->retry_rcvd_buf,
				   &(p->sta_ptr->retry_rcvd_ext),c_ptr);

      if (p->rc == DLC_FUNC_BUSY)       /* not supported              */
					/* set rc arbitrarily invalid so
					   that it will errlog correctly */
	p->rc = -10;

      /****************************************************************/
      /* call wakeup_rc to test return code from function handler     */
      /****************************************************************/
      wakeup_rc(p,p->rc,&(p->sta_ptr->retry_rcvd_buf),ERRID_LAN8085);

      }

    if                                /* an I-frame is pending wakeup,
					   and not already in user
					    initiated local busy mode */
       ((p->sta_ptr->retry_rcvi_buf != 0) &&
	    (p->sta_ptr->us_local_busy == FALSE))
      {

      /****************************************************************/
      /* call the user's receive I-frame handler                      */
      /****************************************************************/
      p->rc = (*c_ptr->rcvi_fa)(p->sta_ptr->retry_rcvi_buf,
				   &(p->sta_ptr->retry_rcvi_ext),c_ptr);

      /****************************************************************/
      /* call wakeup_rc to test return code from function handler     */
      /****************************************************************/
      wakeup_rc(p,p->rc,&(p->sta_ptr->retry_rcvi_buf),ERRID_LAN8083);
      }
    } /* endif wakeup is pending */
  TRACE1(p, "RQWe");
}                                        /* end user_rcv_wakeup       */


wakeup_rc(p,input_rc,retry_ptr,errid)
  register struct port_dcl *p;
  register int    input_rc;
  register int    *retry_ptr;
  register int    errid;
{
  switch (input_rc)
  {
    case (DLC_FUNC_OK) :
				       /* reset the save buffer address  */

      *retry_ptr = 0;

      if                               /* all function retries completed
					  for xid, datagram, and iframes */
	 ((p->sta_ptr->retry_rcvx_buf == 0) &&
	  (p->sta_ptr->retry_rcvd_buf == 0) &&
	  (p->sta_ptr->retry_rcvi_buf == 0))
	 {
				       /* then clear wakeup_needed flag  */
	 p->station_list[p->stano].wakeup_needed = FALSE;

	 if                            /* not also in user initiated     */
				       /*   local busy mode              */
	    (p->sta_ptr->us_local_busy == FALSE)
	   {
				       /* then call exit local busy      */
	   exit_local_busy(p);
	   }
	 }
      break;

    case (DLC_FUNC_BUSY) :             /* <<< I-frame only >>>           */
				       /* the user has initiated lbusy   */
      p->sta_ptr->us_local_busy == TRUE;
				       /* note: leave the save buffer
						address as is            */
				       /* clear wakeup_needed flag       */
      p->station_list[p->stano].wakeup_needed = FALSE;

      break;

    case (DLC_FUNC_RETRY) :
				       /* the user needs another retry,
					  so leave everything as is     */
      break;

    default  :
				       /* invalid return code from user */
      lanerrlg(p, errid, NON_ALERT, PERM_SAP_ERR, DLC_USR_INTRF,
							    FILEN, LINEN);
      break;
  }
}                                      /* end wakeup_rc               */

/* delete low_on_buffers routine */
/* delete buffers_now_avail routine */
/* LEHe */

enter_local_busy(p)
  register struct port_dcl *p;
{
  TRACE1(p, "ELBb");
 
  if                                   /* not already in the local    
                                          busy state                  */
     (p->sta_ptr->local_busy == FALSE)
    {

      /****************************************************************/
      /* set the station control block local busy indicator.          */
      /****************************************************************/

      p->sta_ptr->local_busy = TRUE;
 
      if                               /* the opportunity exists to   
                                          send an rnr response (ie.   
                                          abme)                       */
         (p->sta_ptr->ls == LS_ABME)
        {

          /************************************************************/
          /* disable the t2 acknowledgement timer.                    */
          /************************************************************/

          p->station_list[p->stano].t2_ctr = -1;
          p->station_list[p->stano].t2_ena = FALSE;

          /************************************************************/
          /* set the rcvd i-frames count (ir_ct) to maximum (n3).     */
          /************************************************************/

          p->sta_ptr->ir_ct = p->sta_ptr->ls_profile.rcv_wind;

          /************************************************************/
          /* call the transmit unsolicited response routine, to send a*/
          /* rnr response with no final bit.                          */
          /************************************************************/

          tx_rnr_rsp(p, PF2_OFF_MASK&p->sta_ptr->vr);
        } 
    } 
  TRACE1(p, "ELBe");
  return (0);
}                                      /* end enter_local_busy;       */
exit_local_busy(p)
  register struct port_dcl *p;
{
  TRACE1(p, "XLBb");
 
  if                                   /* currently in the local busy 
                                          state                       */
     (p->sta_ptr->local_busy == TRUE)
    {

      /****************************************************************/
      /* reset the local busy indicator.                              */
      /****************************************************************/

      p->sta_ptr->local_busy = FALSE;
 
      if                               /* the opportunity exists to   
                                          send an rr command or       
                                          response                    */

      /****************************************************************/
      /* (ie. abme mode)                                              */
      /****************************************************************/

         (p->sta_ptr->ls == LS_ABME)
        {
 
          if                           /* the opportunity exists to   
                                          send an rr command          */

          /************************************************************/
          /* (ie. not checkpointing)                                  */
          /************************************************************/

             (p->sta_ptr->checkpointing == FALSE)
            {

              /********************************************************/
              /* set the checkpointing indicator.                     */
              /********************************************************/

              p->sta_ptr->checkpointing = TRUE;
 
              if                       /* t3 timer state = inactivity */
                 (p->sta_ptr->t3_state == T3_INACT)
                {

                  /****************************************************/
                  /* disable the t3 inactivity timer.                 */
                  /****************************************************/

                  p->station_list[p->stano].t3_ctr = -1;
                  p->station_list[p->stano].t3_ena = FALSE;
                } 

              /********************************************************/
              /* set the poll retry count (p_ct) to maximum (n2).     */
              /********************************************************/

              p->sta_ptr->p_ct = p->sta_ptr->ls_profile.max_repoll;

              /********************************************************/
              /* disable the sending of i-frames.                     */
              /********************************************************/

              p->sta_ptr->iframes_ena = FALSE;

              /********************************************************/
              /* call the transmit station command routine, to send a */
              /* rr command with poll bit.                            */
              /********************************************************/

              tx_sta_cmd(p, RR, (PF2_MASK|p->sta_ptr->vr), 2);
            } 
 
          else                         /* checkpointing, so not able  
                                          to send an rr command.      */
            {

              /********************************************************/
              /* set the local busy clearing indicator.               */
              /********************************************************/

              p->sta_ptr->clearing = TRUE;

              /********************************************************/
              /* call the transmit unsolicited response routine, to   */
              /* send a rr response with no final bit.                */
              /********************************************************/

              tx_rr_rsp(p, PF2_OFF_MASK&p->sta_ptr->vr);
            } 
        } 
    } 
  TRACE1(p, "XLBe");
  return (0);
}                                      /* end exit_local_busy;        */
open_link_station(p)
  register struct port_dcl *p;
{
  int      n;
  char     *temp_sap_ptr;
  int      error;

  TRACE1(p, "OLSb");
 
  if                                   /* the station is currently in 
                                          the "closed" state          */
     (p->sta_ptr->ls == LS_CLOSE_STATE)
    {
#ifdef   TRLORFDDI
 
      if                               /* resolve procedures          */
         ((p->sta_ptr->ls_profile.flags&DLC_SLS_ADDR) != 0)
        {

          /************************************************************/
          /* set the max i-field value to the max packet value.       */
          /************************************************************/

          p->common_cb.maxif = (MAX_PACKET-NORM_HDR_LENGTH);
        } 
#endif                                  /* TRLORFDDI                         */
 
      if                               /* the max i-field size        
                                          specified by the user is    
                                          greater than                */

      /****************************************************************/
      /* the actual buffer capability determined by the link manager  */
      /****************************************************************/

         (p->sta_ptr->ls_profile.maxif > p->common_cb.maxif)
        {

          /************************************************************/
          /* set the max i-field value in the station profile to the  */
          /* value specified by the link manager.                     */
          /************************************************************/

          p->sta_ptr->ls_profile.maxif = p->common_cb.maxif;
        } 

      /****************************************************************/
      /* buffer, so leave the user specified value as is. set the     */
      /* inactivity timeout value and abort timeout values at (2n)    */
      /* since there are 2 timer ticks per second, and add 1 to insure*/
      /* minimum value at tick.                                       */
      /****************************************************************/

      p->sta_ptr->inact_to_val = ((p->sta_ptr->ls_profile.inact_time*2)+1);
      p->sta_ptr->force_to_val = ((p->sta_ptr->ls_profile.force_time*2)+1);

      /****************************************************************/
      /* set the repoll and acknowledgement timeouts as given, and add*/
      /* 1 to insure minimum value at tick.                           */
      /****************************************************************/

      p->sta_ptr->resp_to_val = (p->sta_ptr->ls_profile.repoll_time+1);
      p->sta_ptr->ack_to_val = (p->sta_ptr->ls_profile.ack_time+1);

      /****************************************************************/
      /* set up addressability to the various transmit buffers.       */
      /* prebuild the transmit i-frame header area.                   */
      /****************************************************************/
 

      if ((p->sta_ptr->ls_profile.flags&DLC_SLS_ADDR) != 0)
        bcopy(p->sta_ptr->ls_profile.raddr_name, p->sta_ptr->raddr, 6);
      else
        bzero(p->sta_ptr->raddr, 6);   /* filled in at LS Started     */
      bcopy(p->common_cb.local_addr, p->sta_ptr->laddr, 6);
      p->sta_ptr->rsap = 0;            /* filled in at LS Started     */
      p->sta_ptr->lsap = (p->sap_ptr->sap_profile.local_sap&RESP_OFF);
      p->sta_ptr->vs = 0;
      p->sta_ptr->vr = 0;
#ifdef   EDL
      p->sta_ptr->dpad = 0;
      p->sta_ptr->llc_type = SNA_LLC_TYPE;
      p->sta_ptr->lpdu_length = 0;     /* filled in for each i-frame  */
      p->sta_ptr->lpad = 0;
#endif
#ifdef   E3L
      p->sta_ptr->dpad = 0;
      p->sta_ptr->lpdu_length = 0;     /* filled in for each i-frame  */
#endif
#ifdef   TRLORFDDI
/* LEHb defect 54638 */
/* <<< feature CDLI >>> */
#ifdef TRL
      p->sta_ptr->phy_ctl1 = (p->sta_ptr->trl_start_psd.pkt_prty << 5);
      p->sta_ptr->phy_ctl2 = 0x40;
#elif FDL
      p->sta_ptr->phy_ctl1 = (0x50 | p->sta_ptr->fdl_start_psd.pkt_prty);
#endif
/* <<< end feature CDLI >>> */
/* LEHe */

      /****************************************************************/
      /*  defect 29546 -- clear alter route in station control block  */
      /****************************************************************/
      p->sta_ptr->alter_route = 0;
      
      /****************************************************************/
      /* set dynamic window to max transmit value                     */
      /****************************************************************/

      p->sta_ptr->ww = p->sta_ptr->ls_profile.xmit_wind;

      /****************************************************************/
      /* set count of acknowledgements                                */
      /****************************************************************/

      p->sta_ptr->nw = 0x08;
#endif

/* LEHb defect 44499 */
      /***************************************************************/
      /* reset the i-frame or s-frame received indicator             */
      /***************************************************************/

      p->sta_ptr->iors_rcvd = FALSE;
/* LEHe */

      /****************************************************************/
      /* reset all the station internal states and indicators.        */
      /****************************************************************/

      bzero(&(p->sta_ptr->ras_counters.counters),
	      sizeof(struct dlc_ls_counters));

      if                               /* the open is a "connect-out" */
         ((p->sta_ptr->ls_profile.flags&DLC_SLS_LSVC) != 0)
        {

          /************************************************************/
          /* preset the error indicator FALSE.                        */
          /************************************************************/

          error = FALSE;
 
          if                           /* using discovery procedures  */
             ((p->sta_ptr->ls_profile.flags&DLC_SLS_ADDR) == 0)
            {

              /********************************************************/
              /* check to see if the specified name is already added  */
              /* as one of the local sap's name. save the current sap */
              /* pointer.                                             */
              /********************************************************/

              temp_sap_ptr = (char *)p->sap_ptr;

              /********************************************************/
              /* do for index = 0 to 127 possible saps                */
              /********************************************************/
 

              for (n = 0; n <= 127; n++)
                {
 
                  if                   /* the sap is in use           */
                     (p->sap_list[n].in_use == TRUE)
                    {

                      /************************************************/
                      /* get addressability to the sap control block. */
                      /************************************************/

                      p->sap_ptr = (struct sap_cb *)p->sap_list[n].
                         sap_cb_addr;
 
                      if               /* the local name matches the  
                                          specified remote name       */
                         ((bcmp(p->sap_ptr->sap_profile.laddr_name, 
                         p->sta_ptr->ls_profile.raddr_name, 
                         p->sap_ptr->sap_profile.len_laddr_name) == 0)
                         && (p->sap_ptr->sap_profile.len_laddr_name ==
                         p->sta_ptr->ls_profile.len_raddr_name))
                        {
                          error = TRUE;
                          break;
                        } 
                    } 
                }                      /* end do n;                   */

              /********************************************************/
              /* restore the current sap pointer.                     */
              /********************************************************/

              p->sap_ptr = (struct sap_cb *)temp_sap_ptr;
            }                          /* end if discovery            */
 
          else
            {
 
              if (!bcmp(p->sta_ptr->ls_profile.raddr_name, 
                 p->common_cb.local_addr, 6))
                error = TRUE;
            } 
 
          if                           /* the specified name is       
                                          already defined locally     */
             (error == TRUE)
            {
              p->sta_ptr->loopback = TRUE;
            } 
 
          else                         /* ok - not attempting to call 
                                          the local node via discovery*/
            p->sta_ptr->loopback = 0;

          /************************************************************/
          /* set the link state to "call pending".                    */
          /************************************************************/

          p->sta_ptr->ls = LS_CALL_PEND;
 
/* deleted migration raddr_8 */
        } 
 
      else                             /* the open is a "connect-in". */
        {

          /************************************************************/
          /* set the link state to "listen pending".                  */
          /************************************************************/

          p->sta_ptr->ls = LS_LISTEN_PEND;
        } 
    } 
 
  else                                 /* error - invalid state for   
                                          open.                       */
    lanerrlg(p, ERRID_LAN8011, NON_ALERT, PERM_STA_ERR, DLC_ERR_CODE, 
       FILEN, LINEN);

  /********************************************************************/
  /* panic("lansta1 798");                                            */
  /********************************************************************/

  TRACE1(p, "OLSe");
}                                      /* end open_link_station;      */
call_completion(p)
  register struct port_dcl *p;
{
  TRACE1(p, "CACb");
 
  if                                   /* waiting for a call          
                                          completion from the dlc     
                                          manager                     */
     (p->sta_ptr->ls == LS_CALL_PEND)
    {

      /****************************************************************/
      /* save the remote MAC address for this call in the station cb. */
      /****************************************************************/

      bcopy(p->rcv_data.raddr, p->sta_ptr->raddr, 6);

      /****************************************************************/
      /* save the remote sap for this call in the station profile and */
      /* the station control block. note - the rsap was stored in the */
      /* common control block.                                        */
      /****************************************************************/

      p->sta_ptr->ls_profile.rsap = p->sap_ptr->incomming_rsap;
      p->sta_ptr->rsap = p->sap_ptr->incomming_rsap;

      /****************************************************************/
      /* call station opened routine;                                 */
      /****************************************************************/

      station_opened(p);
    } 
 
  else                                 /* not a valid state to get    
                                          call completion.            */
    lanerrlg(p, ERRID_LAN8011, NON_ALERT, PERM_STA_ERR, DLC_ERR_CODE, 
       FILEN, LINEN);

  /********************************************************************/
  /* panic("lansta1 1078");                                           */
  /********************************************************************/

  TRACE1(p, "CACe");
}                                      /* end call_completion;        */
station_opened(p)
  register struct port_dcl *p;
{
  struct dlc_getx_arg dlc_getx_arg;
  struct dlc_chan *c_ptr;
  struct dlc_stas_res *q;
  char     *temp_buf_ptr;
/* LEHb defect 44499 */
/* deleted 1 line */

  TRACE1(p, "SOPb");

  /********************************************************************/
  /* set the link station state = opened in adm mode.                 */
  /********************************************************************/

  p->sta_ptr->ls = LS_ADM;

/* deleted 75 lines */
/* LEHe */

  dlc_getx_arg.result_ind = DLC_STAS_RES;
  dlc_getx_arg.user_sap_corr = p->sap_ptr->sap_profile.user_sap_corr;
  dlc_getx_arg.user_ls_corr = p->sta_ptr->ls_profile.user_ls_corr;
  dlc_getx_arg.result_code = 0;

  /********************************************************************/
  /* return the extension portion                                     */
  /********************************************************************/

  q = (struct dlc_stas_res *)&dlc_getx_arg.result_ext[0];
  q->maxif = p->sta_ptr->ls_profile.maxif;
  q->rport_addr_len = 6;
  bcopy(p->sta_ptr->raddr, q->rport_addr, q->rport_addr_len);
  q->rname_len = p->sta_ptr->ls_profile.len_raddr_name;
  bcopy(p->sta_ptr->ls_profile.raddr_name, q->rname, q->rname_len);
  q->rsap = p->sta_ptr->ls_profile.rsap;
  q->max_data_off = 0;

  /********************************************************************/
  /* call the exception handler to return data                        */
  /********************************************************************/

  c_ptr = p->sap_ptr->user_sap_channel;
  TRACE3(p, "XFAb", DLC_STAS_RES, 0);
  p->rc = (*c_ptr->excp_fa)(&dlc_getx_arg, c_ptr);
  TRACE1(p, "XFAe");
 
  if (p->rc != 0)
    {
 
      if (p->debug)
        printf("Start link station user exec = %x\n", p->rc);
      lanerrlg(p, ERRID_LAN8087, NON_ALERT, PERM_STA_ERR, 
         DLC_ERR_CODE, FILEN, LINEN);
      shutdown(p, HARD);

      /****************************************************************/
      /* panic("Start link station user exec\n");                     */
      /****************************************************************/

    } 
 
  if                                   /* link trace is enabled       */
     ((p->sta_ptr->ls_profile.flags&DLC_TRCO) == DLC_TRCO)

    /******************************************************************/
    /* call the session trace routine (open, length).                 */
    /******************************************************************/

    {
      session_trace(p, HKWD_SYSX_DLC_START, 0);
    } 
 
  if                                   /* t3 timer state = inactivity */
     (p->sta_ptr->t3_state == T3_INACT)
    {

      /****************************************************************/
      /* enable the inactivity timer.                                 */
      /****************************************************************/

      p->station_list[p->stano].t3_ctr = p->sta_ptr->inact_to_val;
      p->station_list[p->stano].t3_ena = TRUE;
    } 

/* LEHb defect XXX */
/* deleted phy_1 and phy_2 and associated pkt_prty */
/* LEHe */

/* LEHb defect 44499 */
/* deleted 12 lines */
/* LEHe */
TRACE1(p, "SOPe");
}                                      /* end station_opened;         */

listen_completion(p)
  register struct port_dcl *p;
{
  TRACE1(p, "LCOb");
 
  if                                   /* the station is still waiting
                                          for listen completion       */
     (p->sta_ptr->ls == LS_LISTEN_PEND)
    {

      /****************************************************************/
      /* save the incomming remote name, NETWORK address, and sap     */
      /* values.                                                      */
      /****************************************************************/

      bcopy(p->sap_ptr->listen_raddr, p->sta_ptr->raddr, 6);
      p->sta_ptr->ls_profile.len_raddr_name = p->sap_ptr->listen_rname_length;
      bcopy(p->sap_ptr->listen_rname, 
         p->sta_ptr->ls_profile.raddr_name, 
         p->sta_ptr->ls_profile.len_raddr_name);
      p->sta_ptr->ls_profile.rsap = p->sap_ptr->incomming_rsap;

      /****************************************************************/
      /* save the remote sap address in the station cb.               */
      /****************************************************************/

      p->sta_ptr->rsap = p->sta_ptr->ls_profile.rsap;

      /****************************************************************/
      /* call station opened routine;                                 */
      /****************************************************************/

      station_opened(p);
    } 
  TRACE1(p, "LCOe");
}                                      /* end listen_completion;      */

/* LEHb defect 44499  - new routine */
#ifdef   TRLORFDDI
set_sta_route(p)
  register struct port_dcl *p;
{
  int      bridge_buf;
  struct ri_control_field ri_control;

  TRACE1(p, "SSRb");

/* defect 162732 */
  if /* the user has not altered routing, ie. we are allowed to change
        the station's route dynamically */
     (p->sta_ptr->alter_route == 0)
  {
    if /* this receive packet does not have routing information */
      (p->routing_info == 0)
    {
      /****************************************************************/
      /* zero the station's routing information length                */
      /****************************************************************/

      p->sta_ptr->ri_length = 0;
    }
    else /* this receive packet has routing information */
    {
/* end defect 162732 */

    /******************************************************************/
    /* get routing control information from received packet           */
    /******************************************************************/

    bcopy(&(p->d.rcv_data->ri_field[0]), &ri_control, 2);

    /******************************************************************/
    /* toggle direction indicator                                     */
    /******************************************************************/

    ri_control.direction ^= 1;

    /******************************************************************/
    /* default the single route broadcast information length to zero  */
    /******************************************************************/

    p->sta_ptr->sri_length = 0;

    if                                 /* routing information present
					    in the received packet    */
       (ri_control.ri_lth > 2)
    {
      /****************************************************************/
      /* check this in the received packet since any single route     */
      /* broadcast has been converted to 2-byte all routes broadcast  */
      /* in the common control block.                                 */
      /****************************************************************/

      /****************************************************************/
      /* save common routing information length in station cb         */
      /****************************************************************/

      p->sta_ptr->ri_length = p->common_cb.ri_length;

      /****************************************************************/
      /* save common routing information field in station cb          */
      /****************************************************************/

      bcopy(&(p->common_cb.ri_field), &(p->sta_ptr->ri_field),
	 p->common_cb.ri_length);

      if                               /* receive packet indicates
					  single route broadcast      */
	 ((ri_control.single_route == TRUE) &&
	  (ri_control.all_route == TRUE))
	{

	TRACE1(p, "SSRS");

	/**************************************************************/
	/* reset the single route broadcast indicator bits            */
	/**************************************************************/

	ri_control.single_route = FALSE;
	ri_control.all_route = FALSE;

	/**************************************************************/
	/* save single route information length in station cb         */
	/**************************************************************/

	p->sta_ptr->sri_length = ri_control.ri_lth;

	/**************************************************************/
	/* save single route information field in station cb          */
	/**************************************************************/

	bcopy(&(p->d.rcv_data->ri_field[0]), &(p->sta_ptr->sri_field[0]),
	   ri_control.ri_lth);
	bcopy(&ri_control, &(p->sta_ptr->sri_field[0]), 2);
	}

      /****************************************************************/
      /* set smallest buffer size as default bridge buffer            */
      /****************************************************************/


      switch                           /* largest field size          */
	 (ri_control.largest_field)
	{
/* fix for pmr3753x */
	  case                         /* bridge buffer = 516         */
	     (BRIDGE_BUF_SIZE0) :
	      {
		bridge_buf = 516;
	      }
	    break;
/* fix for pmr3753x */
	  case                         /* bridge buffer = 1500        */
	     (BRIDGE_BUF_SIZE1) :
	      {
		bridge_buf = 1500;
	      }
	    break;
	  case                         /* bridge buffer = 2052        */
	     (BRIDGE_BUF_SIZE2) :
	      {
		bridge_buf = 2052;
	      }
	    break;
	  case                         /* bridge buffer = 4472        */
	     (BRIDGE_BUF_SIZE3) :
	      {
		bridge_buf = 4472;
	      }
	    break;
	  case                         /* bridge buffer = 8144        */
	     (BRIDGE_BUF_SIZE4) :
	      {
		bridge_buf = 8144;
	      }
	    break;
	  case                         /* bridge buffer = 11407       */
	     (BRIDGE_BUF_SIZE5) :
	      {
		bridge_buf = 11407;
	      }
	    break;
	  case                         /* bridge buffer = 17800       */
	     (BRIDGE_BUF_SIZE6) :
	      {
		bridge_buf = 17800;
	      }
	    break;
/* fix for pmr3753x */
 	  default :                    /* size7 is for all route      */
	    break;                     /* broadcast                   */ 
/* fix for pmr3753x */
	  }                            /* end switch                  */

      if                               /* station's maxif greater than
					  bridge buffer size          */
	 (p->sta_ptr->ls_profile.maxif > bridge_buf)
	{

	  /************************************************************/
	  /* use bridge buffer value for station's maxif.             */
	  /************************************************************/
	  /* This should notify the user of any changes but there is  */
	  /* no mechanism to notify once the station is started.      */
	  /************************************************************/

	  p->sta_ptr->ls_profile.maxif = bridge_buf;
	}
    }
/* defect 162732 */
  else /* route is too short */
    {
      /****************************************************************/
      /* zero the station's routing information length                */
      /****************************************************************/

      p->sta_ptr->ri_length = 0;
    }                                  /* endif routing too short     */
   }                                   /* endif routing present       */
/* end defect 162732 */
  }                                    /* endif able to change route  */
  TRACE1(p, "SSRe");
}                                      /* end set_sta_route           */
#endif /* TRLORFDDI */
/* LEHe */

write_command(p)
  register struct port_dcl *p;
{
  ulong    dlctype;                    /* lan type                    */
  ulong    dlctype1;                   /* lan type                    */
  ulong_t  port_sta;                   /* station number and port     
                                          number                      */

  TRACE1(p, "WRCb");

/* setup hook id sna dlc type for performance trace                   */

  dlctype = HKWD_SYSX_DLC_PERF;
#ifdef   TRL
  dlctype |= DLC_TOKEN_RING;
#endif
#ifdef   FDL
  dlctype |= DLC_FDDI;
#endif
#ifdef   EDL
  dlctype |= DLC_ETHERNET;
#endif
#ifdef   E3L
  dlctype |= DLC_IEEE_802_3;
#endif

  /********************************************************************/
  /* get addressability to the data area of the dlc header mbuf       */
  /********************************************************************/

  p->d.data_ptr = MTOD(p->m, caddr_t);

/* setup lan and monitor types                                        */

  dlctype1 = DLC_TRACE_WRTC<<8;
#ifdef   TRL
  dlctype1 |= DLC_TOKEN_RING;
#endif
#ifdef   FDL
  dlctype1 |= DLC_FDDI;
#endif
#ifdef   EDL
  dlctype1 |= DLC_ETHERNET;
#endif
#ifdef   E3L
  dlctype1 |= DLC_IEEE_802_3;
#endif

  /********************************************************************/
  /* get station number in upper half word and get number from port   */
  /* name in lower half word                                          */
  /********************************************************************/
/* <<< feature CDLI >>> */
#ifndef FDL
  port_sta = ((p->stano<<16)|p->dlc_port.namestr[3]);
#elif FDL
  port_sta = ((p->stano<<16)|p->dlc_port.namestr[4]);
#endif
/* <<< end feature CDLI >>> */

  /********************************************************************/
  /* call trchkgt to record monitor trace                             */
  /********************************************************************/

  trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype1, p->dlc_io_ext.flags, 
     p->stano, p->m, p->m->m_len, port_sta);
 
  switch                               /* the write data type         */
     (p->dlc_io_ext.flags)
    {
      case                             /* NORMAL data                 */
         (DLC_INFO) :
          {

            /**********************************************************/
            /* call trchklt to record performance trace entry         */
            /**********************************************************/

            trchklt(dlctype|DLC_TRACE_SNDIF, p->m->m_len);
 
            if (p->debug)
	      printf("push m=%x txq=%d\n", p->m, p->sta_ptr->txq_input);
 
            if                         /* the link station state =    
                                          abme                        */
               (p->sta_ptr->ls == LS_ABME)
              {

                /******************************************************/
                /* insure that the dummy dlc header mbuf has a proper */
                /* offset and length for the m_copy later, even though*/
                /* the dlc header is not actually built here.         */
                /******************************************************/

                p->m->m_len = NORM_HDR_LENGTH;

                /******************************************************/
                /* push the data buffer address on the transmit queue.*/
                /******************************************************/

                p->sta_ptr->transmit_queue[p->sta_ptr->txq_input].buf 
                   = (char *)p->m;

                /******************************************************/
                /* bump the transmit queue input index modulo queue   */
                /* size.                                              */
                /******************************************************/

                p->sta_ptr->txq_input = (p->sta_ptr->txq_input+1)%TXQ_SIZE;
 
                if                     /* the transmit queue is now   
                                          full                        */
                   (p->sta_ptr->txq_input == p->sta_ptr->txq_output)
                  {
                    p->sta_ptr->que_flag = TRUE;

                    /**************************************************/
                    /* decrement the transmit queue input index modulo*/
                    /* queue size.                                    */
                    /**************************************************/

                    p->sta_ptr->txq_input = (p->sta_ptr->txq_input-1)%TXQ_SIZE;

                    /**************************************************/
                    /* call error log - transmit queue exceeded, link */
                    /* station return code = resource outage.         */
                    /**************************************************/

                    lanerrlg(p, ERRID_LAN8010, NON_ALERT, 
                       PERM_STA_ERR, DLC_LS_ROUT, FILEN, LINEN);
/* LEHb defect 43788 */
		    /**************************************************/
		    /* The buffer is returned to buffer management in */
		    /* pr_write in module lan.c                       */
		    /**************************************************/
/* LEHe */
                  } 
 
                else
                  {
                    p->sta_ptr->que_flag = FALSE;
                  } 
              } 
 
            else                       /* not in abme mode.           */
              {

                /******************************************************/
                /* call error log - user interface error.             */
                /******************************************************/

                lanerrlg(p, ERRID_LAN002A, NON_ALERT, INFO_ERR, 0, 
                   FILEN, LINEN);

                /******************************************************/
                /* call buffer management to return the data buffer.  */
                /******************************************************/

                lanfree(p, p->m);
              } 
          } 
        break;
      case                             /* xid data                    */
         (DLC_XIDD) :
          {

            /**********************************************************/
            /* call trchklt to record performance trace data          */
            /**********************************************************/

            trchklt(dlctype|DLC_TRACE_XMITX, p->m->m_len);
 
            if                         /* the link station state = adm
                                          or abme mode                */
               ((p->sta_ptr->ls == LS_ADM) || (p->sta_ptr->ls == 
               LS_ABME))
              {

                /******************************************************/
                /* call build xid packet routine.                     */
                /******************************************************/

                build_xid(p);
                p->sta_ptr->xs = p->sta_ptr->xid_ir_pend+
                   p->sta_ptr->xid_or_pend;
 
                if                     /* the xid state = reset       */
                   (p->sta_ptr->xs == XID_RESET)
                  {

                    /**************************************************/
                    /* save the xid command buffer address for        */
                    /* repolling.                                     */
                    /**************************************************/

                    p->sta_ptr->xid_cmd_addr = (int)p->m;
 
                    if                 /* in abme checkpointing state */
                       (p->sta_ptr->checkpointing == TRUE)
                      {

                        /**********************************************/
                        /* stack the xid command for future           */
                        /* transmission.                              */
                        /**********************************************/

                        p->sta_ptr->vc = XID_ENA;
                      } 
 
                    else               /* not abme checkpointing.     */
                      {

                        /**********************************************/
                        /* set the poll retry count to the station's  */
                        /* max retries.                               */
                        /**********************************************/

                        p->sta_ptr->p_ct = p->sta_ptr->ls_profile.max_repoll;

                        /**********************************************/
                        /* call the send xid command routine.         */
                        /**********************************************/

                        send_xid_cmd(p);
                      } 
                  } 
 
                else
 
                  if                   /* the xid state = outgoing    
                                          response pending            */
                     (p->sta_ptr->xid_or_pend == TRUE)
                    {

                      /************************************************/
                      /* reset the xid state = outgoing response      */
                      /* pending.                                     */
                      /************************************************/

                      p->sta_ptr->xid_or_pend = FALSE;

                      /************************************************/
                      /* call send xid response routine.              */
                      /************************************************/

                      send_xid_rsp(p);
                    } 
 
                  else                 /* error - invalid xid state.  */
                    {

                      /************************************************/
                      /* call error log - invalid xid state. station  */
                      /* return code = user interface error.          */
                      /************************************************/

                      p->sta_ptr->xs = p->sta_ptr->xid_ir_pend+
                         p->sta_ptr->xid_or_pend;
                      lanerrlg(p, ERRID_LAN0025, NON_ALERT, 
                         PERM_STA_ERR, DLC_USR_INTRF, FILEN, LINEN);
/* LEHb defect 43788 */
		      /************************************************/
		      /* call buffer management to return the buffer. */
		      /************************************************/
		      lanfree(p, p->m);
/* LEHe */

                      /************************************************/
                      /* call shutdown routine - HARD.                */
                      /************************************************/

                      shutdown(p, HARD);
                    } 
              } 
 
            else                       /* not in the proper mode for  
                                          an xid.                     */
              {

                /******************************************************/
                /* call error log - user interface error.             */
                /******************************************************/

                lanerrlg(p, ERRID_LAN0037, NON_ALERT, INFO_ERR, 0, 
                   FILEN, LINEN);

                /******************************************************/
                /* call buffer management to return the data buffer.  */
                /******************************************************/

                lanfree(p, p->m);
              } 
          } 
        break;
      case                             /* datagram data               */
         (DLC_DGRM) :
          {

            /**********************************************************/
            /* call trchklt to record performance hook data           */
            /**********************************************************/

            trchklt(dlctype|DLC_TRACE_SNDDG, p->m->m_len);
 
            if                         /* the link station state = adm
                                          or abme mode                */
               ((p->sta_ptr->ls == LS_ADM) || (p->sta_ptr->ls == 
               LS_ABME))
              {
 
                if                     /* t3 timer stat = inactivity  */
                   (p->sta_ptr->t3_state == T3_INACT)
                  {

                    /**************************************************/
                    /* restart inactivity timer                       */
                    /**************************************************/

                    p->station_list[p->stano].t3_ctr = p->sta_ptr->inact_to_val;
                    p->station_list[p->stano].t3_ena = TRUE;
                  } 

                /******************************************************/
                /* call build datagram packet routine.                */
                /******************************************************/

                build_datagram(p);
              } 
 
            else                       /* not in the proper mode for a
                                          datagram.                   */
              {

                /******************************************************/
                /* call error log - user interface error.             */
                /******************************************************/

                lanerrlg(p, ERRID_LAN002B, NON_ALERT, INFO_ERR, 0, 
                   FILEN, LINEN);

                /******************************************************/
                /* call buffer management to return the data buffer.  */
                /******************************************************/

                lanfree(p, p->m);
              } 
          } 
        break;
      default  :                       /* not NORMAL, xid, or datagram
                                          data.                       */
          {

            /**********************************************************/
            /* call error log - invalid write option, station return  */
            /* code = user interface error.                           */
            /**********************************************************/

            lanerrlg(p, ERRID_LAN8019, NON_ALERT, PERM_STA_ERR, 
               DLC_USR_INTRF, FILEN, LINEN);
/* LEHb defect 43788 */
	    /*********************************************************/
	    /* call buffer management to return the buffer.          */
	    /*********************************************************/
	    lanfree(p, p->m);
/* LEHe */

            /**********************************************************/
            /* call shutdown routine - HARD.                          */
            /**********************************************************/

            shutdown(p, HARD);
          } 
    }                                  /* end select;                 */
  TRACE1(p, "WRCe");
}                                      /* end write_command;          */
build_xid(p)
  register struct port_dcl *p;
{

  /********************************************************************/
  /* call the protocol specific build_xid                             */
  /********************************************************************/

  g_build_xid(p);
}                                      /* end build_xid;              */
send_xid_cmd(p)
  register struct port_dcl *p;
{
  TRACE1(p, "SXCb");

  /********************************************************************/
  /* set the buffer pointer to the xid buffer's address.              */
  /********************************************************************/

  p->m = (struct mbuf *)p->sta_ptr->xid_cmd_addr;
  p->d.data_ptr = MTOD(p->m, caddr_t);
#ifdef   TRLORFDDI

  /********************************************************************/
  /* set routing pointer                                              */
  /********************************************************************/

  p->ri.ptr_ri = (p->d.send_data->ri_field)+p->sta_ptr->ri_length;

  /********************************************************************/
  /* reset the lsap response indicator in the buffer.                 */
  /********************************************************************/

  p->ri.ri_sd->lsap &= RESP_OFF;
#endif
#ifndef  TRLORFDDI

  /********************************************************************/
  /* reset the lsap response indicator in the buffer.                 */
  /********************************************************************/

  p->d.send_data->lsap &= RESP_OFF;
#endif

  /********************************************************************/
  /* save the last command sent to the remote.                        */
  /********************************************************************/
  /* LEHb defect 38204 */
  p->sta_ptr->last_cmd_1 = XID;
  /* LEHe */

  /********************************************************************/
  /* set the xid state = incomming response pending.                  */
  /********************************************************************/

  p->sta_ptr->xid_ir_pend = TRUE;
 
  if                                   /* t3 timer state = inactivity */
     (p->sta_ptr->t3_state == T3_INACT)
    {

      /****************************************************************/
      /* terminate any inactivity timeout.                            */
      /****************************************************************/

      p->station_list[p->stano].t3_ena = FALSE;
      p->station_list[p->stano].t3_ctr = -1;
    } 

  /********************************************************************/
  /* start a repoll timeout;                                          */
  /********************************************************************/

  p->station_list[p->stano].t1_ena = TRUE;
  p->station_list[p->stano].t1_ctr = p->sta_ptr->resp_to_val;

  /********************************************************************/
  /* disable sending any i-frames.                                    */
  /********************************************************************/

  p->sta_ptr->iframes_ena = FALSE;
 
  if                                   /* link station state = abme   
                                          mode                        */
     (p->sta_ptr->ls == LS_ABME)
    {

      /****************************************************************/
      /* set the abme substate "checkpointing".                       */
      /****************************************************************/

      p->sta_ptr->checkpointing = TRUE;
    } 

  /********************************************************************/
  /* copy the small mbufs and double link the cluster so that there is*/
  /* no conflict with the device on m_free and retransmits            */
  /********************************************************************/

/* defect 109030 */
  p->m = (struct mbuf *)m_copym(p->m, 0, M_COPYALL, M_WAIT);
/* end defect 109030 */
 
  if                                   /* the copy was successful     */
     (p->m != 0)
    {

/* <<< feature CDLI >>> */
      /****************************************************************/
      /* call the write send command generator, with the xid command  */
      /* buffer address.                                              */
      /****************************************************************/

      lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
    } 
 
  else                                 /* the m_copy was not          
                                          successful                  */
    {

      /****************************************************************/
      /* permanent station error - out of station resources           */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN8010, NON_ALERT, PERM_STA_ERR, DLC_SYS_ERR,
         FILEN, LINEN);
    } 
  TRACE1(p, "SXCe");
}                                      /* end send_xid_cmd;           */
send_xid_rsp(p)
  register struct port_dcl *p;
{
  TRACE1(p, "SXRb");

  /********************************************************************/
  /* call protocol specific send_xid_rsp                              */
  /********************************************************************/

  g_send_xid_rsp(p);
  TRACE1(p, "SXRe");
}                                      /* end send_xid_rsp;           */
build_datagram(p)
  register struct port_dcl *p;
{
  TRACE1(p, "BDGb");

  /********************************************************************/
  /* call the protocol specific build datagram routine                */
  /********************************************************************/

  g_build_datagram(p);
  TRACE1(p, "BDGe");
}                                      /* end build_datagram;         */

/* <<< feature CDLI >>> */
/* <<< removed wrt_single_cmpl(p) >>> */
/* <<< end feature CDLI >>> */

station_contacted(p)
  register struct port_dcl *p;

/*** start of specifications **************************************** */
/*                                                                    */
/* function name  = station_contacted                                 */
/*                                                                    */
/* descriptive name = station contacted routine                       */
/*                                                                    */
/* function =  issues the "contacted" status to pu services to        */
/*             indicate that the logical link is in normal data       */
/*             mode.                                                  */
/*                                                                    */
/* input = n/a                                                        */
/*                                                                    */
/* output = send command psb (result) to pu services                  */
/*                                                                    */
/*** end of specifications ****************************************** */

{
 
  if (p->debug)
    printf("===========>station_contacted\n");

  /********************************************************************/
  /* set the link station state = abme.                               */
  /********************************************************************/

  p->sta_ptr->ls = LS_ABME;
 
  if                                   /* t3 timer state = inactivity */
     (p->sta_ptr->t3_state == T3_INACT)
    {

      /****************************************************************/
      /* restart the t3 inactivity timer                              */
      /****************************************************************/

      p->station_list[p->stano].t3_ctr = p->sta_ptr->inact_to_val;
      p->station_list[p->stano].t3_ena = TRUE;
    } 
  lansrslt(p, RC_GOOD, DLC_CONT_RES, 
     p->sap_ptr->sap_profile.user_sap_corr, 
     p->sta_ptr->ls_profile.user_ls_corr);
}                                      /* end station_contacted;      */
