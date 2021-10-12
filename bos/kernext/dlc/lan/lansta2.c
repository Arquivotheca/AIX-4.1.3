static char sccsid[] = "@(#)05  1.25.1.11  src/bos/kernext/dlc/lan/lansta2.c, sysxdlcg, bos411, 9437C411a 9/14/94 16:10:59";

/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: lansta2.c
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
sta_rcv_cmpl(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  sta_rcv_cmpl                                         */
/*                                                                    */
/* descriptive name:  station receive completion                      */
/*                                                                    */
/* function:  processes the frames received to determine what to      */
/*            transmit on the line next.                              */
/*                                                                    */
/* input:  received buffer pointer                                    */
/*                                                                    */
/* output:  none                                                      */
/*                                                                    */
/*** end of specifications ********************************************/

{
  ulong    dlctype;                    /* lan type                    */
  ulong_t  port_sta;                   /* station number and port     
                                          number                      */
/* LEHb defect 44499 */
  int      bridge_buf;
/* LEHe */
  if                                   /* link trace enabled.         */

     ((p->sta_ptr->ls_profile.flags&DLC_TRCO) == DLC_TRCO)
    {

      /****************************************************************/
      /* call the receive link trace routine.                         */
      /****************************************************************/

      lanrcvtr(p);
    } 
 
  if (p->debug)
    printf("sta_rcv_cmpl ls=%x p->sta_ptr=%x ctl1=%x\n", 
       p->sta_ptr->ls, p->sta_ptr, p->rcv_data.ctl1);

  /********************************************************************/
  /* preset the "ignore frame" to the default state = "FALSE", to     */
  /* indicate that the buffer is not to be returned to the pool.      */
  /********************************************************************/

  p->sta_ptr->ignore = FALSE;
 
  if                                   /* in a valid receive state    */
     (TSTBIT(p->sta_ptr->ls, LS_RCV_VALID) == TRUE)
    {
 
      if                               /* at least 3-bytes of lpdu    
                                          were received,              */

      /****************************************************************/
      /* ie. the first control byte was received.                     */
      /****************************************************************/

         (p->lpdu_length >= 3)
        {

/* LEHb defect 44499 */
#ifdef   TRLORFDDI
	  /************************************************************/
	  /* call set station route routine                           */
	  /************************************************************/

	  set_sta_route (p);
#endif
/* LEHe */

          /************************************************************/
          /* get station number in upper half word and get number from*/
          /* port name in lower half word                             */
          /************************************************************/
/* <<< feature CDLI >>> */
#ifndef FDL
          port_sta = ((p->stano<<16)|p->dlc_port.namestr[3]);
#elif FDL
          port_sta = ((p->stano<<16)|p->dlc_port.namestr[4]);
#endif
/* <<< end feature CDLI >>> */
 
          if                           /* the type of frame received =
                                          information                 */
             (TSTBIT(p->rcv_data.ctl1, I_TYPE) == INFO)
            {

              /********************************************************/
              /* setup lan and monitor types                          */
              /********************************************************/

              dlctype = DLC_TRACE_RCVI<<8;
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

/* call trchkgt to record monitor trace                               */

              trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, 
              (p->sta_ptr->rcvd_nr<<16)|p->sta_ptr->rcvd_ns, p->m, 
              p->m->m_len, ((p->rcv_data.lsap<<16)|p->rcv_data.rsap), 
              port_sta);

              /********************************************************/
              /* call the information frame received routine          */
              /********************************************************/

              i_rcvd(p);
            } 
 
          else                         /* check for other frame types */
            {

              /********************************************************/
              /* setup values for monitor trace                       */
              /********************************************************/

              dlctype = DLC_TRACE_RNONI<<8;
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

/* call trchkgt to record monitor trace                               */

              trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, p->rcv_data.ctl1,
              p->m, p->m->m_len, ((p->rcv_data.lsap<<16)|
              p->rcv_data.rsap), port_sta);
 
              switch                   /* type of frame received      */
                 (p->rcv_data.ctl1&0x03)
                {
                  case                 /* the type of frame received =
                                          SUPERVISORY                 */
                     (SUPERVISORY) :
                      {

                        /**********************************************/
                        /* call the SUPERVISORY frame received routine*/
                        /**********************************************/

                        super_rcvd(p);
                      } 
                    break;
                  case                 /* the type of frame received =
                                          UNNUMBERED                  */
                     (UNNUMBERED) :
                      {

                        /**********************************************/
                        /* call the UNNUMBERED frame received routine */
                        /**********************************************/

                        unnum_rcvd(p);
                      } 
                    break;
                  default  :           /* unknown frame type.         */
                      {
 
                        if             /* invalid frames received     
                                          counter not at maximum value*/
			   (p->sta_ptr->ras_counters.counters.inv_pkt_rec
			     != MINUS_ONE)

                          /********************************************/
                          /* increment count of invalid frames        */
                          /* received                                 */
                          /********************************************/

			  ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

                        /**********************************************/
                        /* call error log - unknown command received. */
                        /**********************************************/

                        lanerrlg(p, ERRID_LAN0015, NON_ALERT, 
                           TEMP_ERR, 0, FILEN, LINEN);

                        /**********************************************/
                        /* ignore the frame.                          */
                        /**********************************************/

                        p->sta_ptr->ignore = TRUE;
                      } 
                }                      /* end switch;                 */
            } 
        } 
 
      else                             /* there was not enough data   
                                          for a data link header.     */
        {
 
          if                           /* invalid frames received     
                                          counter not at maximum value*/
             (p->sta_ptr->ras_counters.counters.inv_pkt_rec != 
             MINUS_ONE)

            /**********************************************************/
            /* increment count of invalid frames received             */
            /**********************************************************/

            ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

          /************************************************************/
          /* call error log - received frame < 3-bytes                */
          /************************************************************/

          lanerrlg(p, ERRID_LAN0006, NON_ALERT, TEMP_ERR, 0, FILEN, 
             LINEN);

          /************************************************************/
          /* ignore the receive frame                                 */
          /************************************************************/

          p->sta_ptr->ignore = TRUE;
        } 
    } 
 
  else                                 /* the station is in a         
                                          non-receive state           */
    {

      /****************************************************************/
      /* ignore the receive frame.                                    */
      /****************************************************************/

      p->sta_ptr->ignore = TRUE;
    } 

/* <<< feature CDLI >>> */ 
/* defect 119996   adds in_use check */
/* <<< feature CDLI >>> */ 
  if                                   /* the frame is to be ignored  */
     ((p->station_list[p->stano].in_use == TRUE) &&
     (p->sta_ptr->ignore == TRUE))
    {

/* defect 149350 */
      /* reset the ignore mbuf indicator so that any subsequent link
         station close completion will not double free it.            */
      p->sta_ptr->ignore = FALSE;
/* end defect 149350 */

      if (p->debug)
        printf("*****************IGNORE == TRUE\n");

      /****************************************************************/
      /* call buffer management to return the receive buffer.         */
      /****************************************************************/

      lanfree(p, p->m);
    } 
 
  if                                   /* the station control block   
                                          still exists                */
     (p->station_list[p->stano].in_use == TRUE)
    {
 
      if                               /* currently aborting the link */
         (p->sta_ptr->t3_state == T3_ABORT)

        /**************************************************************/
        /* complete the checks to determine whether a disc can be     */
        /* sent.                                                      */
        /**************************************************************/

        {
 
          if                           /* in abme mode, the transmit  
                                          queue is empty, and we're   
                                          not                         */

          /************************************************************/
          /* already checkpointing the remote                         */
          /************************************************************/

             ((p->sta_ptr->ls == LS_ABME) && (p->sta_ptr->txq_input ==
             p->sta_ptr->txq_output) && (p->sta_ptr->checkpointing == 
             FALSE))

            /**********************************************************/
            /* call the shutdown routine to send a disc command.      */
            /**********************************************************/

            shutdown(p, SOFT);
        } 
    } 
}                                      /* end sta_rcv_cmpl            */
i_rcvd(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  i_rcvd                                               */
/*                                                                    */
/* descriptive name:  i frame received                                */
/*                                                                    */
/* function:  processes the i frame received and determines what      */
/*            to transmit on the NETWORK next.                        */
/*                                                                    */
/* input:  received buffer pointer                                    */
/*                                                                    */
/* output:  none                                                      */
/*                                                                    */
/*** end of specifications ********************************************/

{
  u_char   temp_pf;
  int      temp_nr,temp_vs;
  u_char   abme_ss;
 

  if (p->debug)
    printf("=====================>i_rcvd\n");
 
  if                                   /* at least 4-bytes of lpdu    
                                          were received,              */

  /********************************************************************/
  /* ie. the first and second control bytes were received.            */
  /********************************************************************/

     (p->lpdu_length >= 4)
    {

      /****************************************************************/
      /* save nr and ns counts from the received control bytes.       */
      /****************************************************************/

      p->sta_ptr->rcvd_nr = (p->rcv_data.ctl2&PF2_OFF_MASK);
      p->sta_ptr->rcvd_ns = p->rcv_data.ctl1;

      /****************************************************************/
      /* ** do the nr range check inline for best performance ***     */
      /* pre-adjust the vs and nr counts for wrap-around.             */
      /****************************************************************/

        {
 
          if                           /* vs is less than va          */
             (p->sta_ptr->vs < p->sta_ptr->va)

            /**********************************************************/
            /* bump vs by 256.                                        */
            /**********************************************************/

            temp_vs = (p->sta_ptr->vs+256);
 
          else                         /* leave vs the same.          */
            temp_vs = p->sta_ptr->vs;
 
          if                           /* the received nr is less than
                                          va                          */
             (p->sta_ptr->rcvd_nr < p->sta_ptr->va)

            /**********************************************************/
            /* bump nr by 256.                                        */
            /**********************************************************/

            temp_nr = (p->sta_ptr->rcvd_nr+256);
 
          else                         /* leave vs the same.          */
            temp_nr = p->sta_ptr->rcvd_nr;
        } 
 
      if                               /* va is less than or equal to 
                                          the adjusted nr, and        */

      /****************************************************************/
      /* the adjusted nr is less than or equal to the adjusted vs     */
      /****************************************************************/

         ((p->sta_ptr->va <= temp_nr) && (temp_nr <= temp_vs))

        /**************************************************************/
        /* the nr range is valid.                                     */
        /**************************************************************/

        {
 
          switch                       /* link state                  */
             (p->sta_ptr->ls)
            {
              case                     /* state = abme                */
                 (LS_ABME) :
                  {
 
                    if                 /* inactivity without          
                                          termination is indicated    */
                       (p->sta_ptr->inact_without_pend == TRUE)
                      {

                        /**********************************************/
                        /* call the check inactivity routine.         */
                        /**********************************************/

                        valid_rcv(p);
                      } 
/* LEHb defect 44499 */
		    /**************************************************/
		    /* set the i-frame or s-frame received indicator  */
		    /**************************************************/

		    p->sta_ptr->iors_rcvd = TRUE;
/* LEHe */
                    abme_ss = p->sta_ptr->local_busy+
                       p->sta_ptr->remote_busy+p->sta_ptr->rejection+
                       p->sta_ptr->checkpointing+p->sta_ptr->clearing;
 
                    if                 /* no p->sta_ptr->indicators   
                                          set - NORMAL fast path      */
                       (abme_ss == 0)
                      {

                        /**********************************************/
                        /* update the va variable with nr recieved    */
                        /**********************************************/

                        update_va(p);

                        /**********************************************/
                        /* call i-frame received, no checkpointing, no*/
                        /* local busy.                                */
                        /**********************************************/

                        i_nck_nlb(p);
                      } 
 
                    else               /* at least one                
                                          p->sta_ptr->indicator is    
                                          set.                        */
                      {
 
                        if             /* checkpointing bit set on in 
                                          p->sta_ptr->                */
                           (p->sta_ptr->checkpointing == TRUE)
                          {
 
                            if ((TSTBIT(p->rcv_data.rsap, RESPONSE) ==
                               TRUE) && (TSTBIT(p->rcv_data.ctl2, 
                               POLL_FINAL_2) == TRUE))
                              i_ck_rsp(p);
 
                            else       /* i-frame command, or i-frame 
                                          response with p/f=0.        */
                              {

                                /**************************************/
                                /* call i-frame command received,     */
                                /* checkpointing.                     */
                                /**************************************/

                                i_ck_cmd(p);
                              } 
                          } 
 
                        else           /* no checkpointing bit on in  
                                          p->sta_ptr->                */
                          {

                            /******************************************/
                            /* update the va variable with nr recieved*/
                            /******************************************/

                            update_va(p);
 
                            if         /* local busy bit set on in    
                                          p->sta_ptr->                */
                               (p->sta_ptr->local_busy == TRUE)
                              {

                                /**************************************/
                                /* call i-frame received, no          */
                                /* checkpointing, local busy.         */
                                /**************************************/

                                i_nck_lb(p);
                              } 
 
                            else       /* local busy not set, but may 
                                          be remote busy or           */

                              /****************************************/
                              /* rejection.                           */
                              /****************************************/

                              {

                                /**************************************/
                                /* call i-frame received, no          */
                                /* checkpointing, no local busy.      */
                                /**************************************/

                                i_nck_nlb(p);
                              } 
                          } 
                      } 
                  } 
                break;
              case                     /* state = abme pending mode   */
                 (LS_ABME_PEND) :
                  {
 
                    if                 /* the received frame is a     
                                          response with the final bit 
                                          set on                      */
                       ((TSTBIT(p->rcv_data.rsap, RESPONSE) == TRUE) 
                       && (TSTBIT(p->rcv_data.ctl2, POLL_FINAL_2) == 
                       TRUE))

                      /************************************************/
                      /* its not a valid contact completion frame.    */
                      /************************************************/

                      {

                        /**********************************************/
                        /* call the send frmr routine with final bit  */
                        /* set off, for an invalid control byte       */
                        /* received.                                  */
                        /**********************************************/

                        send_frmr(p, FRMR_INV_CTL_RCVD, FRMR_PF_OFF);
                      } 
 
                    else               /* received a valid contact    
                                          completion frame.           */
                      {
 
/* LEHb defect 44499 */
			/**********************************************/
			/* set the i-frame or s-frame received        */
			/**********************************************/

			p->sta_ptr->iors_rcvd = TRUE;
/* LEHe */
                        if             /* t3 timer state = inactivity */
                           (p->sta_ptr->t3_state == T3_INACT)
                          {

                            /******************************************/
                            /* restart the inactivity timer (t3).     */
                            /******************************************/

                            p->station_list[p->stano].t3_ctr = 
                               p->sta_ptr->inact_to_val;
                            p->station_list[p->stano].t3_ena = TRUE;
                          } 

                        /**********************************************/
                        /* enable sending of i-frames.                */
                        /**********************************************/

                        p->sta_ptr->iframes_ena = TRUE;

                        /**********************************************/
                        /* call the valid receive routine to check    */
                        /* inactivity.                                */
                        /**********************************************/

                        valid_rcv(p);

                        /**********************************************/
                        /* inform the user - llc contacted            */
                        /**********************************************/

                        station_contacted(p);
 
                        if             /* local busy bit set in       
                                          p->sta_ptr->                */
                           (p->sta_ptr->local_busy == TRUE)

                          /********************************************/
                          /* discard i field and send rnr             */
                          /********************************************/

                          {

                            /******************************************/
                            /* send a rnr response with final bit =   */
                            /* poll bit received. note - must be in   */
                            /* two steps for the compiler.            */
                            /******************************************/

                            temp_pf = (p->rcv_data.ctl2&PF2_MASK);
			    tx_buf_rsp(p, RNR, (temp_pf|p->sta_ptr->vr), 2);

                          } 
 
                        else           /* not local busy.             */
                          {
 
                            if         /* i-frame is out of sequence  */
                               (p->sta_ptr->rcvd_ns != p->sta_ptr->vr)

                              /****************************************/
                              /* reject frame and i field             */
                              /****************************************/

                              {

                                /**************************************/
                                /* send a rej response with final bit */
                                /* = poll bit received. note - must be*/
                                /* in two steps for the compiler.     */
                                /**************************************/

                                temp_pf = (p->rcv_data.ctl2&PF2_MASK);
                                tx_buf_rsp(p, REJ, (temp_pf|
                                   p->sta_ptr->vr), 2);

                                /**************************************/
                                /* set on rejection bit in            */
                                /* p->sta_ptr->                       */
                                /**************************************/

                                p->sta_ptr->rejection = TRUE;
                              } 
 
                            else       /* i-frame is in sequence.     */
                              {

                                /**************************************/
                                /* increment the receive state        */
                                /* variable vr (bits 7-1 only).       */
                                /**************************************/

                                p->sta_ptr->vr = (p->sta_ptr->vr+2);
 
                                if     /* a command frame was received
                                          with poll bit set.          */
                                   ((TSTBIT(p->rcv_data.rsap, RESPONSE
                                   ) == FALSE) && (TSTBIT
                                   (p->rcv_data.ctl2, POLL_FINAL_2) ==
                                   TRUE))
                                  {
                                    abme_cmd_poll(p);
                                  } 
 
                                else   /* not a command poll          */
                                  {
                                    send_ack(p);
                                  } 

                                /**************************************/
                                /* accept i-field and send to path    */
                                /* control in.                        */
                                /**************************************/

                                rcv_i_frame(p);
                              } 
                          } 
                      } 
                  } 
                break;
              case                     /* state = disconnected mode   */
                 (LS_ADM) :
                  {

                    /**************************************************/
                    /* call error log - invalid state to receive      */
                    /* i_frames                                       */
                    /**************************************************/

                    lanerrlg(p, ERRID_LAN0007, NON_ALERT, TEMP_ERR, 0,
                       FILEN, LINEN);

                    /**************************************************/
                    /* call invalid receive in disconnected mode.     */
                    /**************************************************/

                    invalid_rcv_adm(p);
                  } 
                break;

                /******************************************************/
                /* state = contacting or discontacting mode           */
                /******************************************************/

              case  LS_CONTACTING :
              case  LS_DISCONTACTING :
                  {

                    /**************************************************/
                    /* ignore the frame                               */
                    /**************************************************/

                    p->sta_ptr->ignore = TRUE;
                  } 
                break;
              default  :               /* not a valid state for i     
                                          command.                    */
                  {
 
                    if                 /* invalid frames received     
                                          counter not at maximum value*/
                       (p->sta_ptr->ras_counters.counters.inv_pkt_rec 
                       != MINUS_ONE)

                      /************************************************/
                      /* increment count of invalid frames received   */
                      /************************************************/

                      ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

                    /**************************************************/
                    /* call error log - invalid state to receive      */
                    /* i_frames                                       */
                    /**************************************************/

                    lanerrlg(p, ERRID_LAN0007, NON_ALERT, TEMP_ERR, 0,
                       FILEN, LINEN);

                    /**************************************************/
                    /* ignore the frame.                              */
                    /**************************************************/

                    p->sta_ptr->ignore = TRUE;
                  } 
            }                          /* end switch;                 */
        } 
 
      else                             /* error - the nr count is out 
                                          of range.                   */
        {

          /************************************************************/
          /* call invalid lpdu received routine to generate a possible*/
          /* frame reject.                                            */
          /************************************************************/

          inv_lpdu_rcvd(p, FRMR_INV_SEQ_NUM, TSTBIT(p->rcv_data.ctl2, 
             POLL_FINAL_2));
        } 
    } 
 
  else                                 /* data link header is too     
                                          short for information frame.*/
    {
 
      if                               /* invalid frames received     
                                          counter not at maximum value*/
         (p->sta_ptr->ras_counters.counters.inv_pkt_rec != MINUS_ONE)

        /**************************************************************/
        /* increment count of invalid frames received                 */
        /**************************************************************/

        ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

      /****************************************************************/
      /* call error log - received i-frame < 4-bytes.                 */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN0008, NON_ALERT, TEMP_ERR, 0, FILEN, LINEN);

      /****************************************************************/
      /* ignore the frame.                                            */
      /****************************************************************/

      p->sta_ptr->ignore = TRUE;
    } 
}                                      /* end i_rcvd;                 */
i_ck_rsp(p)
  register struct port_dcl *p;
{
 
  if (p->debug)
    printf("=========================>i_ck_rsp\n");

  /********************************************************************/
  /* update the va while in checkpointing                             */
  /********************************************************************/

  update_va_chkpt(p);
 
  switch                               /* on stacked command variable 
                                          (vc).                       */
     (p->sta_ptr->vc)
    {
      case                             /* nothing is stacked on vc    */
         (NONE_ENA) :
          {
 
            if                         /* local busy bit set on in    
                                          p->sta_ptr->or out of       
                                          sequence                    */
               ((p->sta_ptr->local_busy == TRUE) || 
               (p->sta_ptr->rcvd_ns != p->sta_ptr->vr))
              {
 
                if                     /* local busy or rejection bits
                                          on in p->sta_ptr->          */
                   ((p->sta_ptr->local_busy == TRUE) || 
                   (p->sta_ptr->rejection == TRUE))
                  {
 
                    if                 /* in sequence i frame         */
                       (p->sta_ptr->rcvd_ns == p->sta_ptr->vr)
                      {

                        /**********************************************/
                        /* send rnr response with the final bit reset.*/
                        /**********************************************/

                        tx_buf_rsp(p, RNR, (PF2_OFF_MASK
                           &p->sta_ptr->vr), 2);
                      } 
 
                    else               /* do nothing                  */
                      {

                        /**********************************************/
                        /* discard the received buffer.               */
                        /**********************************************/

                        p->sta_ptr->ignore = TRUE;
                      } 
                  } 
 
                else                   /* local not busy, and not in  
                                          rejection, and out of       */

                  /****************************************************/
                  /* sequence.                                        */
                  /****************************************************/

                  {
 
                    if                 /* not clearing                */
                       (p->sta_ptr->clearing == FALSE)
                      {

                        /**********************************************/
                        /* terminate timer t2                         */
                        /**********************************************/

                        p->station_list[p->stano].t2_ctr = -1;
                        p->station_list[p->stano].t2_ena = FALSE;

                        /**********************************************/
                        /* reset the received i lpdu count to n3      */
                        /**********************************************/

                        p->sta_ptr->ir_ct = 
                           p->sta_ptr->ls_profile.rcv_wind;
                      } 
 
                    else               /* clearing                    */
                      {

                        /**********************************************/
                        /* pass through                               */
                        /**********************************************/

                      } 

                    /**************************************************/
                    /* send rej response with the final bit reset.    */
                    /**************************************************/

                    tx_buf_rsp(p, REJ, (PF2_OFF_MASK&p->sta_ptr->vr), 
                       2);

                    /**************************************************/
                    /* set on rejection bit in p->sta_ptr->           */
                    /**************************************************/

                    p->sta_ptr->rejection = TRUE;
                  } 
              } 
 
            else                       /* local not busy, and in      
                                          sequence.                   */
              {

                /******************************************************/
                /* increment the receive state variable vr (bits 7-1  */
                /* only).                                             */
                /******************************************************/

                p->sta_ptr->vr = (p->sta_ptr->vr+2);

                /******************************************************/
                /* set off rejection bit in p->sta_ptr->              */
                /******************************************************/

                p->sta_ptr->rejection = FALSE;

                /******************************************************/
                /* call send acknowledgement routine                  */
                /******************************************************/

                send_ack(p);

                /******************************************************/
                /* receive the i frame and pass to higher layer       */
                /******************************************************/

                rcv_i_frame(p);
              } 

            /**********************************************************/
            /* turn off checkpointing bit in p->sta_ptr->             */
            /**********************************************************/

            p->sta_ptr->checkpointing = FALSE;

            /**********************************************************/
            /* turn off clearing bit in p->sta_ptr->                  */
            /**********************************************************/

            p->sta_ptr->clearing = FALSE;

            /**********************************************************/
            /* start send_proc (set on bit)                           */
            /**********************************************************/

            p->sta_ptr->iframes_ena = TRUE;
          } 
        break;
      case                             /* vc = disc command           */
         (DISC_ENA) :
          {

            /**********************************************************/
            /* discard information field                              */
            /**********************************************************/

            p->sta_ptr->ignore = TRUE;

            /**********************************************************/
            /* call process vc routine.                               */
            /**********************************************************/

            proc_vc(p);
          } 
        break;

        /**************************************************************/
        /* vc = xid or test command                                   */
        /**************************************************************/

      case  XID_ENA :
      case  TEST_ENA :
          {
 
            if                         /* local busy bit set on       
                                          p->sta_ptr->or out of       
                                          sequence                    */
               ((p->sta_ptr->local_busy == TRUE) || 
               (p->sta_ptr->rcvd_ns != p->sta_ptr->vr))
              {

                /******************************************************/
                /* discard information field                          */
                /******************************************************/

                p->sta_ptr->ignore = TRUE;
              } 
 
            else                       /* local not busy, and in      
                                          sequence.                   */
              {

                /******************************************************/
                /* increment the receive state variable vr (bits 7-1  */
                /* only).                                             */
                /******************************************************/

                p->sta_ptr->vr = (p->sta_ptr->vr+2);

                /******************************************************/
                /* set off rejection bit                              */
                /******************************************************/

                p->sta_ptr->rejection = FALSE;

                /******************************************************/
                /* receive the i frame and pass to higher layer       */
                /******************************************************/

                rcv_i_frame(p);
              } 

            /**********************************************************/
            /* set off clearing bit in p->sta_ptr->                   */
            /**********************************************************/

            p->sta_ptr->clearing = FALSE;

            /**********************************************************/
            /* call process vc routine.                               */
            /**********************************************************/

            proc_vc(p);
          } 
    }                                  /* end switch;                 */
}                                      /* end i_ck_rsp;               */
i_ck_cmd(p)
  register struct port_dcl *p;
{
 
  if (p->debug)
    printf("=================>i_ck_cmd lb=%d\n", 
       p->sta_ptr->local_busy);
 
  if                                   /* any of the transmitted      
                                          i-frames are being          
                                          acknowledged                */
     (p->sta_ptr->va != p->sta_ptr->rcvd_nr)
    {
#ifdef   TRLORFDDI
      adjust_window(p);
#endif

      /****************************************************************/
      /* update va = nr.                                              */
      /****************************************************************/

      p->sta_ptr->va = p->sta_ptr->rcvd_nr;

      /****************************************************************/
      /* return all acknowledged transmit buffers to the pool. note - */
      /* must run after updating va.                                  */
      /****************************************************************/

      free_tx_acked(p);

      /****************************************************************/
      /* reset i-lpdu count (is_ct) to n2                             */
      /****************************************************************/

      p->sta_ptr->is_ct = p->sta_ptr->ls_profile.max_repoll;
    } 
 
  if                                   /* in sequence                 */
     (p->sta_ptr->rcvd_ns == p->sta_ptr->vr)
    {
 
      if                               /* local busy set on in        
                                          p->sta_ptr->                */
         (p->sta_ptr->local_busy == TRUE)
        {
 
          if                           /* i frame is a command with   
                                          the poll bit set            */
             ((TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE) && TSTBIT
             (p->rcv_data.ctl2, POLL_FINAL_2) == TRUE)
            {

              /********************************************************/
              /* send rnr response with the final bit set.            */
              /********************************************************/

              tx_buf_rsp(p, RNR, (PF2_MASK|p->sta_ptr->vr), 2);
            } 
 
          else                         /* i-frame is not a command    
                                          with the poll bit set.      */
            {

              /********************************************************/
              /* send rnr response with the final bit reset.          */
              /********************************************************/

              tx_buf_rsp(p, RNR, (PF2_OFF_MASK&p->sta_ptr->vr), 2);
            } 
        } 
 
      else                             /* not in local busy.          */
        {

          /************************************************************/
          /* increment the receive state variable vr (bits 7-1 only). */
          /************************************************************/

          p->sta_ptr->vr = (p->sta_ptr->vr+2);
 
          if                           /* i frame is a command with   
                                          the poll bit set            */
             ((TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE) && (TSTBIT
             (p->rcv_data.ctl2, POLL_FINAL_2) == TRUE))
            {

              /********************************************************/
              /* send rr response with the final bit set.             */
              /********************************************************/

              tx_rr_rsp(p, PF2_MASK|p->sta_ptr->vr);
 
              if                       /* not in rejection, and not   
                                          clearing local busy         */
                 ((p->sta_ptr->rejection == FALSE) && 
                 (p->sta_ptr->clearing == FALSE))
                {

                  /****************************************************/
                  /* terminate timer t2                               */
                  /****************************************************/

                  p->station_list[p->stano].t2_ctr = -1;
                  p->station_list[p->stano].t2_ena = FALSE;

                  /****************************************************/
                  /* reset i-lpdu count, ir_ct, to n3.                */
                  /****************************************************/

                  p->sta_ptr->ir_ct = p->sta_ptr->ls_profile.rcv_wind;
                } 
 
              else                     /* rejection or clearing.      */
                {

                  /****************************************************/
                  /* reset clearing.                                  */
                  /****************************************************/

                  p->sta_ptr->clearing = FALSE;

                  /****************************************************/
                  /* reset rejection.                                 */
                  /****************************************************/

                  p->sta_ptr->rejection = FALSE;
                } 
            } 
 
          else                         /* i-frame is not a command    
                                          with the poll bit set.      */
            {

              /********************************************************/
              /* call send acknowledgement routine                    */
              /********************************************************/

              send_ack(p);

              /********************************************************/
              /* reset clearing.                                      */
              /********************************************************/

              p->sta_ptr->clearing = FALSE;

              /********************************************************/
              /* reset rejection.                                     */
              /********************************************************/

              p->sta_ptr->rejection = FALSE;
            } 

          /************************************************************/
          /* accept the i-frame and pass to higher layer.             */
          /************************************************************/

          rcv_i_frame(p);
        } 
    } 
 
  else                                 /* not in sequence.            */
    {
 
      if                               /* local busy set on in        
                                          p->sta_ptr->                */
         (p->sta_ptr->local_busy == TRUE)
        {
 
          if                           /* i frame is a command with   
                                          the poll bit set            */
             ((TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE) && (TSTBIT
             (p->rcv_data.ctl2, POLL_FINAL_2) == TRUE))
            {

              /********************************************************/
              /* send rnr response with the final bit set.            */
              /********************************************************/

              tx_buf_rsp(p, RNR, (PF2_MASK|p->sta_ptr->vr), 2);
            } 
 
          else                         /* i-frame is not a command    
                                          with the poll bit set.      */
            {
 
              if                       /* in rejection                */
                 (p->sta_ptr->rejection == TRUE)
                {

                  /****************************************************/
                  /* discard information field                        */
                  /****************************************************/

                  p->sta_ptr->ignore = TRUE;
                } 
 
              else                     /* not in rejection.           */
                {

                  /****************************************************/
                  /* send rnr response with the final bit reset.      */
                  /****************************************************/

                  tx_buf_rsp(p, RNR, (PF2_OFF_MASK&p->sta_ptr->vr), 2)
                     ;
                } 
            } 
        } 
 
      else                             /* not in local busy.          */
        {
 
          if                           /* in rejection                */
             (p->sta_ptr->rejection == TRUE)
            {

              /********************************************************/
              /* reset clearing.                                      */
              /********************************************************/

              p->sta_ptr->clearing = FALSE;
 
              if                       /* i frame is a command with   
                                          the poll bit set            */
                 ((TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE) && 
                 (TSTBIT(p->rcv_data.ctl2, POLL_FINAL_2) == TRUE))
                {

                  /****************************************************/
                  /* send rr response with the final bit set.         */
                  /****************************************************/

                  tx_buf_rsp(p, RR, (PF2_MASK|p->sta_ptr->vr), 2);
                } 
 
              else                     /* i-frame is not a command    
                                          with the poll bit set.      */
                {

                  /****************************************************/
                  /* discard information field                        */
                  /****************************************************/

                  p->sta_ptr->ignore = TRUE;
                } 
            } 
 
          else                         /* not in rejection.           */
            {

              /********************************************************/
              /* terminate timer t2                                   */
              /********************************************************/

              p->station_list[p->stano].t2_ctr = -1;
              p->station_list[p->stano].t2_ena = FALSE;

              /********************************************************/
              /* reset i-lpdu count, ir_ct, to n3.                    */
              /********************************************************/

              p->sta_ptr->ir_ct = p->sta_ptr->ls_profile.rcv_wind;

              /********************************************************/
              /* set rejection.                                       */
              /********************************************************/

              p->sta_ptr->rejection = TRUE;
 
              if                       /* i frame is command with the 
                                          poll bit set.               */
                 ((TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE) && 
                 (TSTBIT(p->rcv_data.ctl2, POLL_FINAL_2) == TRUE))
                {

                  /****************************************************/
                  /* send rej response with the final bit set.        */
                  /****************************************************/

                  tx_buf_rsp(p, REJ, (PF2_MASK|p->sta_ptr->vr), 2);
                } 
 
              else                     /* i-frame is not a command    
                                          with the poll bit set.      */
                {

                  /****************************************************/
                  /* send rej response with the final bit reset.      */
                  /****************************************************/

                  tx_buf_rsp(p, REJ, (PF2_OFF_MASK&p->sta_ptr->vr), 2);
                } 
            } 
        } 
    } 
}                                      /* end i_ck_cmd;               */
i_nck_lb(p)
  register struct port_dcl *p;
{
 
  if (p->debug)
    printf("======================>i_nck_lb\n");
 
  if                                   /* i frame is in sequence      */
     (p->sta_ptr->rcvd_ns == p->sta_ptr->vr)
    {

      /****************************************************************/
      /* set off rejection bit in p->sta_ptr->                        */
      /****************************************************************/

      p->sta_ptr->rejection = FALSE;

      /****************************************************************/
      /* discard information field                                    */
      /****************************************************************/

      p->sta_ptr->ignore = FALSE;
 
      if                               /* a command was received with 
                                          the poll bit set            */
         ((TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE) && (TSTBIT
         (p->rcv_data.ctl2, POLL_FINAL_2) == TRUE))
        {

          /************************************************************/
          /* send rnr response with the final bit set.                */
          /************************************************************/

          tx_buf_rsp(p, RNR, (PF2_MASK|p->sta_ptr->vr), 2);
        } 
 
      else                             /* not a command poll received.*/
        {

          /************************************************************/
          /* send rnr response with the final bit reset.              */
          /************************************************************/

          tx_buf_rsp(p, RNR, (PF2_OFF_MASK&p->sta_ptr->vr), 2);
        } 
    } 
 
  else                                 /* (i frame not in sequence)   */
    {
 
      if                               /* a command was received with 
                                          the poll bit set.           */
         ((TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE) && (TSTBIT
         (p->rcv_data.ctl2, POLL_FINAL_2) == TRUE))
        {

          /************************************************************/
          /* send rnr response with the final bit set.                */
          /************************************************************/

          tx_buf_rsp(p, RNR, (PF2_MASK|p->sta_ptr->vr), 2);
        } 
 
      else                             /* not a command poll received.*/
        {
 
          if                           /* rejection bit set off in    
                                          p->sta_ptr->                */
             (p->sta_ptr->rejection == FALSE)
            {

              /********************************************************/
              /* send rnr response with the final bit reset.          */
              /********************************************************/

              tx_buf_rsp(p, RNR, (PF2_OFF_MASK&p->sta_ptr->vr), 2);
            } 
 
          else                         /* rejection bit is on in      
                                          p->sta_ptr->                */
            {

              /********************************************************/
              /* discard information field                            */
              /********************************************************/

              p->sta_ptr->ignore = TRUE;
            } 
        } 
    } 
}                                      /* end i_nck_lb;               */
i_nck_nlb(p)
  register struct port_dcl *p;
{
 
  if (p->debug)
    printf("=========================>i_nck_nlb\n");
 
  if                                   /* i frame is in sequence      */
     (p->sta_ptr->rcvd_ns == p->sta_ptr->vr)
    {

      /****************************************************************/
      /* increment the receive state variable vr (bits 7-1 only).     */
      /****************************************************************/

      p->sta_ptr->vr = (p->sta_ptr->vr+2);
 
      if                               /* a command was received with 
                                          the poll bit set            */
         ((TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE) && (TSTBIT
         (p->rcv_data.ctl2, POLL_FINAL_2) == TRUE))
        {

          /************************************************************/
          /* send rr response with the final bit set.                 */
          /************************************************************/

          tx_rr_rsp(p, PF2_MASK|p->sta_ptr->vr);
 
          if                           /* rejection bit set off in    
                                          p->sta_ptr->                */
             (p->sta_ptr->rejection == TRUE)
            {

              /********************************************************/
              /* terminate timer t2                                   */
              /********************************************************/

              p->station_list[p->stano].t2_ctr = -1;
              p->station_list[p->stano].t2_ena = FALSE;

              /********************************************************/
              /* reset i lpdu retry counter, ir_ct, to n3             */
              /********************************************************/

              p->sta_ptr->ir_ct = p->sta_ptr->ls_profile.rcv_wind;
            } 
 
          else                         /* rejection bit is on in      
                                          p->sta_ptr->                */
            {

              /********************************************************/
              /* turn off rejection bit in p->sta_ptr->               */
              /********************************************************/

              p->sta_ptr->rejection = FALSE;
            } 
        } 
 
      else                             /* not a command poll received.*/
        {

          /************************************************************/
          /* call send acknowledgement routine                        */
          /************************************************************/

          send_ack(p);

          /************************************************************/
          /* turn off rejection bit in p->sta_ptr->                   */
          /************************************************************/

          p->sta_ptr->rejection = FALSE;
        } 

      /****************************************************************/
      /* receive i frame and pass to higher layer                     */
      /****************************************************************/

      rcv_i_frame(p);
    } 
 
  else                                 /* not in sequence.            */
    {
 
      if                               /* rejection bit set on in     
                                          p->sta_ptr->                */
         (p->sta_ptr->rejection == TRUE)
        {
 
          if                           /* a command was received with 
                                          the poll bit set.           */
             ((TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE) && (TSTBIT
             (p->rcv_data.ctl2, POLL_FINAL_2) == TRUE))
            {

              /********************************************************/
              /* send rr response with the final bit set.             */
              /********************************************************/

              tx_buf_rsp(p, RR, (PF2_MASK|p->sta_ptr->vr), 2);
            } 
 
          else                         /* not a command poll received.*/
            {

              /********************************************************/
              /* discard information field                            */
              /********************************************************/

              p->sta_ptr->ignore = TRUE;
            } 
        } 
 
      else                             /* rejection bit is off in     
                                          p->sta_ptr->                */
        {

          /************************************************************/
          /* terminate timer t2                                       */
          /************************************************************/

          p->station_list[p->stano].t2_ctr = -1;
          p->station_list[p->stano].t2_ena = FALSE;

          /************************************************************/
          /* reset i lpdu counter, ir_ct, to n3                       */
          /************************************************************/

          p->sta_ptr->ir_ct = p->sta_ptr->ls_profile.rcv_wind;

          /************************************************************/
          /* set on rejection bit in p->sta_ptr->                     */
          /************************************************************/

          p->sta_ptr->rejection = TRUE;
 
          if                           /* a command was received with 
                                          the poll bit set.           */
             ((TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE) && (TSTBIT
             (p->rcv_data.ctl2, POLL_FINAL_2) == TRUE))
            {

              /********************************************************/
              /* send rej response with the final bit set.            */
              /********************************************************/

              tx_buf_rsp(p, REJ, (PF2_MASK|p->sta_ptr->vr), 2);
            } 
 
          else                         /* not a command poll received.*/
            {

              /********************************************************/
              /* send rej response with the final bit reset.          */
              /********************************************************/

              tx_buf_rsp(p, REJ, (PF2_OFF_MASK&p->sta_ptr->vr), 2);
            } 
        } 
    } 
}                                      /* end i_nck_nlb;              */
abme_cmd_poll(p)
  register struct port_dcl *p;
{
 
  if (p->debug)
    printf("=================>abme_cmd_poll\n");
 
  if                                   /* this is an i-frame (which   
                                          will be in sequence)        */
     (TSTBIT(p->rcv_data.ctl1, I_TYPE) == INFO)
    {

/*--------------------------------------------------------------------*/
/*   use tx_rnr_rsp or tx_rr_rsp rather than tx_buf_rsp because       */
/*  this buffer is an i-frame in-sequence and will be sent to a       */
/*  higher layer.                                                     */
/*--------------------------------------------------------------------*/

      if                               /* local busy                  */
         (p->sta_ptr->local_busy == TRUE)
        {

          /************************************************************/
          /* send rnr response with final bit set.                    */
          /************************************************************/

          tx_rnr_rsp(p, PF2_MASK|p->sta_ptr->vr);
        } 
 
      else                             /* not local busy.             */
        {

          /************************************************************/
          /* send rr response with final bit set.                     */
          /************************************************************/

          tx_rr_rsp(p, PF2_MASK|p->sta_ptr->vr);
 
          if                           /* not in rejection and not    
                                          clearing local busy         */
             ((p->sta_ptr->rejection == FALSE) && 
             (p->sta_ptr->clearing == FALSE))
            {

              /********************************************************/
              /* terminate acknowledgement delay timer t2             */
              /********************************************************/

              p->station_list[p->stano].t2_ctr = -1;
              p->station_list[p->stano].t2_ena = FALSE;

              /********************************************************/
              /* set the acknowledgement delay count (ir_ct) = max    */
              /* (n3).                                                */
              /********************************************************/

              p->sta_ptr->ir_ct = p->sta_ptr->ls_profile.rcv_wind;
            } 
        } 
    } 
 
  else                                 /* this is a SUPERVISORY frame */
    {

/*--------------------------------------------------------------------*/
/*   use tx_buf_rsp rather than tx_rnr_rsp ortx_rr_rsp because        */
/*  this buffer is not an i-frame in-sequence and will not be         */
/*  sent to a higher layer.                                           */
/*--------------------------------------------------------------------*/
      /* set ignore to not free the buffer                            */

      p->sta_ptr->ignore = FALSE;
 
      if                               /* local busy                  */
         (p->sta_ptr->local_busy == TRUE)
        {

          /************************************************************/
          /* send rnr response with final bit set.                    */
          /************************************************************/

          tx_buf_rsp(p, RNR, (PF2_MASK|p->sta_ptr->vr), 2);
        } 
 
      else                             /* not local busy.             */
        {

          /************************************************************/
          /* send rr response with final bit set.                     */
          /************************************************************/

          tx_buf_rsp(p, RR, (PF2_MASK|p->sta_ptr->vr), 2);
 
          if                           /* not in rejection and not    
                                          clearing local busy         */
             ((p->sta_ptr->rejection == FALSE) && 
             (p->sta_ptr->clearing == FALSE))
            {

              /********************************************************/
              /* terminate acknowledgement delay timer t2             */
              /********************************************************/

              p->station_list[p->stano].t2_ctr = -1;
              p->station_list[p->stano].t2_ena = FALSE;

              /********************************************************/
              /* set the acknowledgement delay count (ir_ct) = max    */
              /* (n3).                                                */
              /********************************************************/

              p->sta_ptr->ir_ct = p->sta_ptr->ls_profile.rcv_wind;
            } 
        } 
    } 
}                                      /* end abme_cmd_poll;          */
super_rcvd(p)
  register struct port_dcl *p;
{
 
  if (p->debug)
    printf("=================>super_rcvd\n");

  /********************************************************************/
  /* preset the "ignore frame" to the default state = "TRUE", to      */
  /* indicate that the buffer will be returned to the pool. note -    */
  /* this will be TRUE for all cases except when a frmr is generated  */
  /* in the same buffer received.                                     */
  /********************************************************************/

  p->sta_ptr->ignore = TRUE;
 
  if                                   /* it's the correct length of  
                                          4-bytes                     */
     (p->lpdu_length == 4)
    {

      /****************************************************************/
      /* save the received nr count by masking off the p/f bit.       */
      /****************************************************************/

      p->sta_ptr->rcvd_nr = (p->rcv_data.ctl2&PF2_OFF_MASK);

      /****************************************************************/
      /* call the nr range check routine to see if nr is between va   */
      /* and vs.                                                      */
      /****************************************************************/

      p->rc = nr_range_ck(p);
 
      if                               /* the range of nr is valid    */
         (p->rc == TRUE)
        {
 
          if                           /* the type of frame received =
                                          receive ready, receive not  */

          /************************************************************/
          /* ready, or reject.                                        */
          /************************************************************/

             ((p->rcv_data.ctl1 == RR) || (p->rcv_data.ctl1 == RNR) ||
             (p->rcv_data.ctl1 == REJ))
            {
 
              switch                   /* link state                  */
                 (p->sta_ptr->ls)
                {
                  case                 /* state = abme                */
                     (LS_ABME) :
                      {

                        /**********************************************/
                        /* call reveived SUPERVISORY in abme mode     */
                        /* routine.                                   */
                        /**********************************************/

                        rcvd_super_abme(p);
                      } 
                    break;
                  case                 /* state = abme pending mode   */
                     (LS_ABME_PEND) :
                      {

                        /**********************************************/
                        /* call reveived SUPERVISORY in abme pending  */
                        /* mode routine.                              */
                        /**********************************************/

                        rcvd_super_abme_pend(p);
                      } 
                    break;
                  case                 /* state = disconnected mode   */
                     (LS_ADM) :
                      {

                        /**********************************************/
                        /* call error log - invalid state to receive  */
                        /* SUPERVISORY frame.                         */
                        /**********************************************/

                        lanerrlg(p, ERRID_LAN000B, NON_ALERT, 
                           TEMP_ERR, 0, FILEN, LINEN);

                        /**********************************************/
                        /* call received i,rr,rnr,rej in disconnected */
                        /* mode                                       */
                        /**********************************************/

                        invalid_rcv_adm(p);
                      } 
                    break;

                    /**************************************************/
                    /* state = contacting or discontacting mode       */
                    /**************************************************/

                  case  LS_CONTACTING :
                  case  LS_DISCONTACTING :
                      {

                        /**********************************************/
                        /* ignore the frame                           */
                        /**********************************************/

                      } 
                    break;
                  default  :           /* not a valid state for       
                                          SUPERVISORY frame.          */
                      {
 
                        if             /* invalid frames received     
                                          counter not at maximum value*/
			   (p->sta_ptr->ras_counters.counters.inv_pkt_rec
			     != MINUS_ONE)

                          /********************************************/
                          /* increment count of invalid frames        */
                          /* received                                 */
                          /********************************************/

			  ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

                        /**********************************************/
                        /* call error log - invalid state to receive  */
                        /* SUPERVISORY frame.                         */
                        /**********************************************/

                        lanerrlg(p, ERRID_LAN000B, NON_ALERT, 
                           TEMP_ERR, 0, FILEN, LINEN);
                      } 
                }                      /* end switch;                 */
            } 
 
          else                         /* error - unknown SUPERVISORY 
                                          frame received.             */
            {

              /********************************************************/
              /* call error log - unknown SUPERVISORY frame received. */
              /********************************************************/

/* defect 86180 */
              lanerrlg(p, ERRID_LAN_ALERT8, ALERT, TEMP_ERR, 0, FILEN,
                 LINEN);
              lanerrlg(p, ERRID_LAN0004, NON_ALERT, TEMP_ERR, 0, FILEN,
                 LINEN);
/* end defect 86180 */

              /********************************************************/
              /* call invalid lpdu received routine to generate a     */
              /* possible frame reject.                               */
              /********************************************************/

              inv_lpdu_rcvd(p, FRMR_INV_CTL_RCVD, TSTBIT
                 (p->rcv_data.ctl2, POLL_FINAL_2));
            } 
        } 
    } 
 
  else                                 /* error - invalid lpdu length 
                                          (ie. too long, since length 
                                          known                       */

    /******************************************************************/
    /* to be greater than 3 and not equal to 4).                      */
    /******************************************************************/

    {

      /****************************************************************/
      /* call error log - invalid i-field received.                   */
      /****************************************************************/

/* defect 86180 */
      lanerrlg(p, ERRID_LAN_ALERT9, ALERT, TEMP_ERR, 0, FILEN, LINEN);
      lanerrlg(p, ERRID_LAN000C, NON_ALERT, TEMP_ERR, 0, FILEN, LINEN);
/* end defect 86180 */

      /****************************************************************/
      /* call invalid lpdu received routine to generate a possible    */
      /* frame reject.                                                */
      /****************************************************************/

      inv_lpdu_rcvd(p, FRMR_INV_IFIELD_RCVD, TSTBIT(p->rcv_data.ctl2, 
         POLL_FINAL_2));
    } 
}                                      /* end super_rcvd;             */

/* called for rr, rnr, or rej received in abme mode, to check         */
/* whether the received nr is in the range va <= nr <= vs.            */

nr_range_ck(p)
  register struct port_dcl *p;
{
  int      valid_nr,temp_nr,temp_vs;
 

  if (p->debug)
    printf("=================>nr_range_ck\n");

  /********************************************************************/
  /* pre-adjust the vs and nr counts for wrap-around.                 */
  /********************************************************************/

    {
 
      if                               /* vs is less than va          */
         (p->sta_ptr->vs < p->sta_ptr->va)

        /**************************************************************/
        /* bump vs by 256.                                            */
        /**************************************************************/

        temp_vs = (p->sta_ptr->vs+256);
 
      else                             /* leave vs the same.          */
        temp_vs = p->sta_ptr->vs;
 
      if                               /* the received nr is less than
                                          va                          */
         (p->sta_ptr->rcvd_nr < p->sta_ptr->va)

        /**************************************************************/
        /* bump nr by 256.                                            */
        /**************************************************************/

        temp_nr = (p->sta_ptr->rcvd_nr+256);
 
      else                             /* leave vs the same.          */
        temp_nr = p->sta_ptr->rcvd_nr;
    } 
 
  if                                   /* va is less than or equal to 
                                          the adjusted nr, and        */

  /********************************************************************/
  /* the adjusted nr is less than or equal to the adjusted vs         */
  /********************************************************************/

     ((p->sta_ptr->va <= temp_nr) && (temp_nr <= temp_vs))

    /******************************************************************/
    /* the nr range is valid.                                         */
    /******************************************************************/

    {
 
      if                               /* a reject was received       */
         (p->rcv_data.ctl1 == REJ)
        {
 
          if                           /* none of the i-frames sent   
                                          were accepted by the remote,*/

          /************************************************************/
          /* ie., va equals the adjusted nr which is less than the    */
          /* adjusted vs.                                             */
          /************************************************************/

	     ((p->sta_ptr->va == temp_nr) && (p->sta_ptr->va < temp_vs))
            {
 
              if                       /* contiguous i_frame counter  
                                          not at maximum value        */
                 (p->sta_ptr->ras_counters.counters.max_cont_resent !=
                 MINUS_ONE)

                /******************************************************/
                /* increment count of contiguous i_frame retransmitted*/
                /******************************************************/

                ++p->sta_ptr->ras_counters.counters.max_cont_resent;
            } 
 
          else                         /* at least one i-frame was    
                                          accepted.                   */
            {

              /********************************************************/
              /* reset the contiguous i-frame bursts retransmitted ras*/
              /* counter.                                             */
              /********************************************************/

              p->sta_ptr->ras_counters.counters.max_cont_resent = 0;
            } 
        } 

      /****************************************************************/
      /* return a valid return code.                                  */
      /****************************************************************/

      valid_nr = TRUE;
    } 
 
  else                                 /* error - received nr not in  
                                          range va<=nr<=vs            */
    {

      /****************************************************************/
      /* return an invalid return code.                               */
      /****************************************************************/

      valid_nr = FALSE;

      /****************************************************************/
      /* call invalid lpdu received routine to generate a possible    */
      /* frame reject.                                                */
      /****************************************************************/

      inv_lpdu_rcvd(p, FRMR_INV_SEQ_NUM, TSTBIT(p->rcv_data.ctl2, 
         POLL_FINAL_2));
    } 
  return (valid_nr);
}                                      /* end nr_range_ck;            */
rcvd_super_abme(p)
  register struct port_dcl *p;
{
  int      remote_flag;
 

  if (p->debug)
    printf("=================>rcvd_sap_abme\n");

  /********************************************************************/
  /* receive is valid, call the check inactivity routine.             */
  /********************************************************************/

  valid_rcv(p);
 
/* LEHb defect 44499 */
  /********************************************************************/
  /* set the i-frame or s-frame received indicator                    */
  /********************************************************************/

  p->sta_ptr->iors_rcvd = TRUE;
/* LEHe */

  if                                   /* an rnr was received.        */
     (p->rcv_data.ctl1 == RNR)
    {

      /****************************************************************/
      /* indicate that the remote is busy.                            */
      /****************************************************************/

      p->sta_ptr->remote_busy = TRUE;

      /****************************************************************/
      /* disable the sending of i-frames.                             */
      /****************************************************************/

      p->sta_ptr->iframes_ena = FALSE;
    } 
 
  else                                 /* not an rnr.                 */
    {

      /****************************************************************/
      /* save the remote busy mode                                    */
      /****************************************************************/

      remote_flag = p->sta_ptr->remote_busy;

      /****************************************************************/
      /* indicate that the remote is not busy.                        */
      /****************************************************************/

      p->sta_ptr->remote_busy = FALSE;

      /****************************************************************/
      /* enable the sending of i-frames. note - may be disabled again */
      /* if still checkpointing.                                      */
      /****************************************************************/

      p->sta_ptr->iframes_ena = TRUE;
    } 
 
  if                                   /* checkpointing               */
     (p->sta_ptr->checkpointing == TRUE)
    {
 
      if                               /* a response was received with
                                          the final bit set.          */
         ((TSTBIT(p->rcv_data.rsap, RESPONSE) == TRUE) && (TSTBIT
         (p->rcv_data.ctl2, POLL_FINAL_2) == TRUE))
        {

          /************************************************************/
          /* update the acknowledge state variable (va) while in      */
          /* checkpointing.                                           */
          /************************************************************/

          update_va_chkpt(p);
 
          if                           /* the stacked command variable
                                          (vc) is empty               */
             (p->sta_ptr->vc == 0)
            {
 
              if                       /* clearing local busy         */
                 (p->sta_ptr->clearing == TRUE)
                {

                  /****************************************************/
                  /* turn off clearing bit in p->sta_ptr->            */
                  /****************************************************/

                  p->sta_ptr->clearing = FALSE;
 
                  if                   /* t3 timer state = inactivity */
                     (p->sta_ptr->t3_state == T3_INACT)
                    {

                      /************************************************/
                      /* terminate inactivity timer (t3)              */
                      /************************************************/

                      p->station_list[p->stano].t3_ctr = -1;
                      p->station_list[p->stano].t3_ena = FALSE;
                    } 
 
                  if                   /* a reject was received       */
                     (p->rcv_data.ctl1 == REJ)
                    {

                      /************************************************/
                      /* set the send state variable (vs) to the      */
                      /* received nr.                                 */
                      /************************************************/

                      p->sta_ptr->vs = p->sta_ptr->rcvd_nr;
                    } 

                  /****************************************************/
                  /* send an rr command with the poll bit set.        */
                  /****************************************************/

                  tx_sta_cmd(p, RR, (PF2_MASK|p->sta_ptr->vr), 2);
        	  /* disable sending of i-frames because of checkpoting */
                  p->sta_ptr->iframes_ena = FALSE;

                } 
 
              else                     /* not clearing local busy.    */
                {

                  /****************************************************/
                  /* set off checkpointing bit in p->sta_ptr->        */
                  /****************************************************/

                  p->sta_ptr->checkpointing = FALSE;
                } 
            } 
 
          else                         /* the stacked control (vc)    
                                          must be disc, xid, or test. */
            {

              /********************************************************/
              /* process the stacked command state variable (vc).     */
              /********************************************************/

              proc_vc(p);
            } 
        } 
 
      else                             /* not a response with final   
                                          bit set.                    */
        {

          /************************************************************/
          /* still checkpointing - disable the sending of i-frames.   */
          /************************************************************/

          p->sta_ptr->iframes_ena = FALSE;
 
          if                           /* acknowledge state variable  
                                          (va) < the received nr      */
             (p->sta_ptr->va != p->sta_ptr->rcvd_nr)
            {
#ifdef   TRLORFDDI
 
              if                       /* reject command              */
                 (p->rcv_data.ctl1 == REJ)
                {
                  p->sta_ptr->ww = 1;
                  p->sta_ptr->ia_ct = 0;
                } 
 
              else
                adjust_window(p);
#endif

              /********************************************************/
              /* set the acknowledge state variable (va) = the        */
              /* received nr.                                         */
              /********************************************************/

              p->sta_ptr->va = p->sta_ptr->rcvd_nr;

              /********************************************************/
              /* return all acknowledged transmit buffers to the pool.*/
              /* note - must run after updating va.                   */
              /********************************************************/

              free_tx_acked(p);

              /********************************************************/
              /* set the i-frame retry count (is_ct) = max retries    */
              /* (n2).                                                */
              /********************************************************/

              p->sta_ptr->is_ct = p->sta_ptr->ls_profile.max_repoll;
            } 
 
          if                           /* a command frame was received
                                          with poll bit set.          */
             ((TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE) && (TSTBIT
             (p->rcv_data.ctl2, POLL_FINAL_2) == TRUE))
            {

              /********************************************************/
              /* call the abme command poll received routine, in order*/
              /* to generate the proper response.                     */
              /********************************************************/

              abme_cmd_poll(p);
            } 
        } 
    } 
 
  else                                 /* not checkpointing.          */
    {

      /****************************************************************/
      /* update the acknowledge state variable (va).                  */
      /****************************************************************/

      update_va(p);
 
      if                               /* a reject was received       */
         (p->rcv_data.ctl1 == REJ)
        {

          /************************************************************/
          /* set the send state variable (vs) to the received nr.     */
          /************************************************************/

          p->sta_ptr->vs = p->sta_ptr->rcvd_nr;
        } 
 
      if                               /* a command was received with 
                                          the poll bit set            */
         ((TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE) && (TSTBIT
         (p->rcv_data.ctl2, POLL_FINAL_2) == TRUE))
        {
 
          if                           /* not local busy, and not     
                                          rejection                   */
             ((p->sta_ptr->local_busy == FALSE) && 
             (p->sta_ptr->rejection == FALSE))
            {

              /********************************************************/
              /* terminate the acknowledgement timer (t2).            */
              /********************************************************/

              p->station_list[p->stano].t2_ctr = -1;
              p->station_list[p->stano].t2_ena = FALSE;

              /********************************************************/
              /* set the acknowledgement delay count (ir_ct) = max    */
              /* (n3)                                                 */
              /********************************************************/

              p->sta_ptr->ir_ct = p->sta_ptr->ls_profile.rcv_wind;
            } 
 
          if                           /* local busy                  */
             (p->sta_ptr->local_busy == TRUE)
            {

              /********************************************************/
              /* send rnr response with the final bit set.            */
              /********************************************************/

              tx_buf_rsp(p, RNR, (PF2_MASK|p->sta_ptr->vr), 2);
            } 
 
          else                         /* not in local busy.          */
            {

              /********************************************************/
              /* send rr response with the final bit set.             */
              /********************************************************/

              tx_buf_rsp(p, RR, (PF2_MASK|p->sta_ptr->vr), 2);
            } 
 
          if                           /* in remote busy and with     
                                          outstanding acknowledgment  */
             ((remote_flag == TRUE) && (p->sta_ptr->vs != 
             p->sta_ptr->rcvd_nr))
            {

              /********************************************************/
              /* set checkpointing                                    */
              /********************************************************/

              p->sta_ptr->checkpointing = TRUE;

              /********************************************************/
              /* send an rr command with the poll bit set.            */
              /********************************************************/

              tx_sta_cmd(p, RR, (PF2_MASK|p->sta_ptr->vr), 2);
              /* disable sending of i-frames because of checkpointing */
              p->sta_ptr->iframes_ena = FALSE;

            } 

          /************************************************************/
          /* don't free the buffer                                    */
          /************************************************************/

          p->sta_ptr->ignore = FALSE;
        } 
    } 
}                                      /* end rcvd_super_abme;        */
rcvd_super_abme_pend(p)
  register struct port_dcl *p;
{
 
  if (p->debug)
    printf("==================>rcvd_super_abme_pend\n");

  /********************************************************************/
  /* receive is valid, call the check inactivity routine.             */
  /********************************************************************/

  valid_rcv(p);
 
  if                                   /* the received frame is a     
                                          response with the final bit 
                                          set on                      */
     ((TSTBIT(p->rcv_data.rsap, RESPONSE) == TRUE) && (TSTBIT
     (p->rcv_data.ctl2, POLL_FINAL_2) == TRUE))
    {

      /****************************************************************/
      /* call the send frmr routine with final bit set off, for an    */
      /* invalid control byte received.                               */
      /****************************************************************/

      send_frmr(p, FRMR_INV_CTL_RCVD, FRMR_PF_OFF);

      /****************************************************************/
      /* set the llc closing reason code = protocol error.            */
      /****************************************************************/

      p->sta_ptr->closing_reason = DLC_PROT_ERR;
    } 
 
/* LEHb defect 44499 */
  else                                 /* valid contact completion
					  frame (ie. not a response
					  with final bit set).       */
    {
      /****************************************************************/
      /* set the i-frame or s-frame received indicator                */
      /****************************************************************/

      p->sta_ptr->iors_rcvd = TRUE;
/* LEHe */

      if                               /* an rr frame was received    */
         (p->rcv_data.ctl1 == RR)
        {

          /************************************************************/
          /* call the station contacted routine.                      */
          /************************************************************/

          station_contacted(p);

          /************************************************************/
          /* enable sending of i-frames.                              */
          /************************************************************/

          p->sta_ptr->iframes_ena = TRUE;
        } 
 
      if                               /* command received with poll  
                                          bit set                     */
         ((TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE) && (TSTBIT
         (p->rcv_data.ctl2, POLL_FINAL_2) == TRUE))
        {
 
          if                           /* local busy bit set on in    
                                          p->sta_ptr->                */
             (p->sta_ptr->local_busy == TRUE)
            {

              /********************************************************/
              /* send a rnr response with the final bit set.          */
              /********************************************************/

              tx_buf_rsp(p, RNR, (PF2_MASK|p->sta_ptr->vr), 2);
            } 
 
          else
            {

              /********************************************************/
              /* send rr response with the final bit set.             */
              /********************************************************/

              tx_buf_rsp(p, RR, (PF2_MASK|p->sta_ptr->vr), 2);
            } 

          /************************************************************/
          /* don't free the buffer                                    */
          /************************************************************/

          p->sta_ptr->ignore = FALSE;
        } 
    } 
}                                      /* end rcvd_super_abme_pend;   */
unnum_rcvd(p)
  register struct port_dcl *p;
{
  u_char   ctl1_masked;
 
  if                                   /* unnumbered command received 
                                          in adm mode when an outgoing
                                          xid is pending              */

     ((p->sta_ptr->ls == LS_ADM) && (p->sta_ptr->xid_or_pend == TRUE) 
     && (TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE))
    {

      /****************************************************************/
      /* ignore packet and return to pool. This is a fix to the       */
      /* protocol to prevent losing the xid response                  */
      /****************************************************************/

      p->sta_ptr->ignore = TRUE;
    } 
 
  else
    {

      /****************************************************************/
      /* build a copy of control byte no1 with the poll/final bit     */
      /* masked on for faster compare.                                */
      /****************************************************************/

      ctl1_masked = (p->rcv_data.ctl1|PF1_MASK);
 
      if (p->debug)
        printf("=================>unnum_rcvd %d\n", ctl1_masked);
 
      switch                           /* type of UNNUMBERED frame    
                                          received                    */
         (ctl1_masked)
        {
          case                         /* the type of frame received =
                                          xid                         */
             (XID) :
              {

                /******************************************************/
                /* call the xid frame received routine                */
                /******************************************************/

                xid_rcvd(p);
              } 
            break;
          case                         /* the type of frame received =
                                          test                        */
             (TEST) :
              {

                /******************************************************/
                /* call the test frame received routine               */
                /******************************************************/

                test_rcvd(p);
              } 
            break;
          case                         /* the type of frame received =
                                          disc                        */
             (DISC) :
              {

                /******************************************************/
                /* call the disconnect command received routine       */
                /******************************************************/

                disc_cmd_rcvd(p);
              } 
            break;
          case                         /* the type of frame received =
                                          set abme                    */
             (SABME) :
              {

                /******************************************************/
                /* call the set mode command received routine         */
                /******************************************************/

                sabme_cmd_rcvd(p);
              } 
            break;
          case                         /* the type of frame received =
                                          ua                          */
             (UA) :
              {

                /******************************************************/
                /* call the ua response received routine              */
                /******************************************************/

                ua_rsp_rcvd(p);
              } 
            break;
          case                         /* the type of frame received =
                                          frame reject                */
             (FRMR) :
              {

                /******************************************************/
                /* call the frame reject response received routine    */
                /******************************************************/

                frmr_rsp_rcvd(p);
              } 
            break;
          case                         /* the type of frame received =
                                          disconnected mode           */
             (DM) :
              {

                /******************************************************/
                /* call the disconect mode response routine           */
                /******************************************************/

                dm_rsp_rcvd(p);
              } 
            break;
          case                         /* the type of frame received =
                                          datagram                    */
             (UI) :
              {

                /******************************************************/
                /* call the datagram received routine                 */
                /******************************************************/

                ui_rcvd(p);
              } 
            break;
          default  :                   /* unknown UNNUMBERED frame.   */
              {

                /******************************************************/
                /* call error log - unknown UNNUMBERED frame received */
                /******************************************************/

/* defect 86180 */
                lanerrlg(p, ERRID_LAN_ALERT8, ALERT, TEMP_ERR, 0, FILEN,
                   LINEN);
                lanerrlg(p, ERRID_LAN0005, NON_ALERT, TEMP_ERR, 0, FILEN,
                   LINEN);
/* end defect 86180 */

                /******************************************************/
                /* call invalid lpdu received routine to generate a   */
                /* possible frame reject.                             */
                /******************************************************/

                inv_lpdu_rcvd(p, FRMR_INV_CTL_RCVD, TSTBIT
                   (p->rcv_data.ctl1, POLL_FINAL_1));
              } 
        }                              /* end switch;                 */
    }                                  /* end unnumbered received     
                                          during xid pending          */
}                                      /* end unnum_rcvd;             */
proc_vc(p)
  register struct port_dcl *p;
{
  ulong_t  m_cmd;
 

  if (p->debug)
    printf("======>proc_vc\n");

  /********************************************************************/
  /* save the current mbuf pointer                                    */
  /********************************************************************/

  m_cmd = (ulong_t)p->m;
 
  if                                   /* t3 timer state = inactivity */
     (p->sta_ptr->t3_state == T3_INACT)
    {

      /****************************************************************/
      /* terminate inactivity timer (t3)                              */
      /****************************************************************/

      p->station_list[p->stano].t3_ctr = -1;
      p->station_list[p->stano].t3_ena = FALSE;
    } 

  /********************************************************************/
  /* restart timer t1                                                 */
  /********************************************************************/

  p->station_list[p->stano].t1_ctr = p->sta_ptr->resp_to_val;
  p->station_list[p->stano].t1_ena = TRUE;

  /********************************************************************/
  /* reset the contiguous command repolls sent ras counter.           */
  /********************************************************************/

  p->sta_ptr->ras_counters.counters.cmd_cont_repolls = 0;

  /********************************************************************/
  /* set the repoll count to maximum (p_ct = n2).                     */
  /********************************************************************/

  p->sta_ptr->p_ct = p->sta_ptr->ls_profile.max_repoll;
 
  switch                               /* on command stack (vc)       */
     (p->sta_ptr->vc)
    {
      case                             /* command stack has disc      
                                          enabled                     */
         (DISC_ENA) :
          {

            /**********************************************************/
            /* shutdown the link with a disc command.                 */
            /**********************************************************/

            shutdown(p, SOFT);
          } 
        break;
      case                             /* command stack has xid       
                                          enabled                     */
         (XID_ENA) :
          {

            /**********************************************************/
            /* call the send xid command routine.                     */
            /**********************************************************/

            send_xid_cmd(p);
          } 
        break;
      case                             /* command stack has test      
                                          enabled                     */
         (TEST_ENA) :
          {

            /**********************************************************/
            /* call the send test command routine.                    */
            /**********************************************************/

            send_test_cmd(p);
          } 
        break;
      default  :                       /* error - no command in vc to 
                                          process.                    */
          {
            lanerrlg(p, ERRID_LAN8011, NON_ALERT, PERM_STA_ERR, 0, 
               FILEN, LINEN);
            shutdown(p, HARD);

            /**********************************************************/
            /* panic("lansta2 1387");                                 */
            /**********************************************************/

          } 
    }                                  /* end switch;                 */

  /********************************************************************/
  /* reset the stacked command variable (vc).                         */
  /********************************************************************/

  p->sta_ptr->vc = NONE_ENA;

  /********************************************************************/
  /* restore the current mbuf pointer                                 */
  /********************************************************************/

  p->m = (struct mbuf *)m_cmd;
}                                      /* end proc_vc;                */
xid_rcvd(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  xid_rcvd                                             */
/*                                                                    */
/* descriptive name:  xid frame received                              */
/*                                                                    */
/* function:  processes the xid frame received and determines what    */
/*            to transmit on the NETWORK next.                        */
/*                                                                    */
/* input:  received buffer pointer                                    */
/*                                                                    */
/* output:  none                                                      */
/*                                                                    */
/*** end of specifications ********************************************/

{
 
  if (p->debug)
    printf("===================>xid_rcvd\n");

  /********************************************************************/
  /* receive is valid, call the check inactivity routine.             */
  /********************************************************************/

  valid_rcv(p);

/* LEHb defect 43788 */
  /* if the user does not already have a busy condition on rcv XID's  */
  if (p->sta_ptr->retry_rcvx_buf == 0)
  {
/* LEHe */
 
  switch                               /* link state                  */
     (p->sta_ptr->ls)
    {
      case                             /* state = disconnected mode   */
         (LS_ADM) :
          {
 
            if                         /* xid is a command            */
               (TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE)
              {

                /******************************************************/
                /* call the receive xid command routine.              */
                /******************************************************/

                rcv_xid_cmd(p);
              } 
 
            else                       /* xid was a response          */
              {

                /******************************************************/
                /* call the receive xid response routine.             */
                /******************************************************/

                p->rc = rcv_xid_rsp(p);
              } 
          } 
        break;
      case                             /* state = abme                */
         (LS_ABME) :
          {
 
            if                         /* xid is a command            */
               (TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE)
              {

                /******************************************************/
                /* call the receive xid command routine.              */
                /******************************************************/

                rcv_xid_cmd(p);
              } 
 
            else                       /* xid was a response          */
              {

                /******************************************************/
                /* call the receive xid response routine.             */
                /******************************************************/

                p->rc = rcv_xid_rsp(p);
 
                if                     /* the xid response was        
                                          expected.                   */
                   (p->rc == 0)
                  {

                    /**************************************************/
                    /* reset checkpointing indicated in p->sta_ptr->  */
                    /**************************************************/

                    p->sta_ptr->checkpointing = FALSE;
 
                    if                 /* no commands are stacked on  
                                          vc                          */
                       (p->sta_ptr->vc == NONE_ENA)
                      {
 
                        if             /* not remote busy             */
                           (p->sta_ptr->remote_busy == FALSE)

                          /********************************************/
                          /* enable sending of i-frames               */
                          /********************************************/

                          p->sta_ptr->iframes_ena = TRUE;
 
                        if             /* clearing local busy in      
                                          p->sta_ptr->                */
                           (p->sta_ptr->clearing == TRUE)
                          {

                            /******************************************/
                            /* reset clearing local busy in           */
                            /* p->sta_ptr->                           */
                            /******************************************/

                            p->sta_ptr->clearing = FALSE;

                            /******************************************/
                            /* send rr command with p/f on.           */
                            /******************************************/

			    tx_sta_cmd(p, RR, (p->sta_ptr->vr|PF2_MASK), 2);
                            /* set checkpointing back on.   */
                            p->sta_ptr->checkpointing = TRUE;
                            /* disable sending of i-frames. */
                            p->sta_ptr->iframes_ena = FALSE;


			  }
                      } 
 
                    else               /* a disc, xid, or test command
                                          is stacked in vc            */
                      {

                        /**********************************************/
                        /* call the process vc routine                */
                        /**********************************************/

                        proc_vc(p);
                      } 
                  } 
              } 
          } 
        break;

        /**************************************************************/
        /* state = abme pending, contacting or discontacting mode     */
        /**************************************************************/

      case  LS_ABME_PEND :
      case  LS_CONTACTING :
      case  LS_DISCONTACTING :
          {

            /**********************************************************/
            /* ignore the frame                                       */
            /**********************************************************/

            p->sta_ptr->ignore = TRUE;
          } 
        break;
      default  :                       /* not a valid state for xid   
                                          frame .                     */
          {

            /**********************************************************/
            /* ignore the frame                                       */
            /**********************************************************/

            p->sta_ptr->ignore = TRUE;
 
            if                         /* invalid frames received     
                                          counter not at maximum value*/
               (p->sta_ptr->ras_counters.counters.inv_pkt_rec != 
               MINUS_ONE)

              /********************************************************/
              /* increment count of invalid frames received           */
              /********************************************************/

              ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

            /**********************************************************/
            /* call error log - invalid state to receive xid frame    */
            /**********************************************************/

            lanerrlg(p, ERRID_LAN0009, NON_ALERT, TEMP_ERR, 0, FILEN, 
               LINEN);
          } 
    }                                  /* end switch;                 */
/* LEHb defect 43788 */
  }
  else                                 /* the user already has a busy
					  condition on receive XIDs
					  so ignore the received XID  */
    p->sta_ptr->ignore = TRUE;
/* LEHe */
}                                      /* end xid_rcvd;               */
rcv_xid_cmd(p)
  register struct port_dcl *p;
{
 
  if (p->debug)
    printf("===========>rcv_xid_cmd\n");
 
  if                                   /* already waiting on an xid   
                                          response from pu services   */
     (p->sta_ptr->xid_or_pend == TRUE)

    /******************************************************************/
    /* cannot receive additional xid command until response sent down */
    /* from the user.                                                 */
    /******************************************************************/

    {

      /****************************************************************/
      /* ignore the frame.                                            */
      /****************************************************************/

      p->sta_ptr->ignore = TRUE;
    } 
 
  else                                 /* ok to receive xid command.  */
    {

      /****************************************************************/
      /* set the xid state variable for outgoing response pending.    */
      /****************************************************************/

      p->sta_ptr->xid_or_pend = TRUE;

      /****************************************************************/
      /* pass the xid command buffer to pu services with response     */
      /* pending indicated.                                           */
      /****************************************************************/

      pass_rcv_xid(p, DLC_RSPP);

      /****************************************************************/
      /* save the command poll bit setting for the pending response.  */
      /****************************************************************/

      p->sta_ptr->px = p->rcv_data.ctl1&PF1_MASK;
 
      if                               /* t3 timer state = inactivity */
         (p->sta_ptr->t3_state == T3_INACT)
        {

          /************************************************************/
          /* restart the inactivity timer (t3)                        */
          /************************************************************/

          p->station_list[p->stano].t3_ctr = p->sta_ptr->inact_to_val;
          p->station_list[p->stano].t3_ena = TRUE;
        } 
    } 
}                                      /* end rcv_xid_cmd;            */
rcv_xid_rsp(p)
  register struct port_dcl *p;
{
  int      xid_not_expected;
 

  if (p->debug)
    printf("===========>rcv_xid_rsp\n");
 
  if                                   /* the final bit is set.       */
     (TSTBIT(p->rcv_data.ctl1, POLL_FINAL_1) == TRUE)
    {
 
      if                               /* looking for an incoming xid 
                                          response                    */
         (p->sta_ptr->xid_ir_pend == TRUE)
        {

          /************************************************************/
          /* indicate that the xid response was expected.             */
          /************************************************************/

          xid_not_expected = FALSE;

          /************************************************************/
          /* turn off looking for an xid indicator                    */
          /************************************************************/

          p->sta_ptr->xid_ir_pend = FALSE;

          /************************************************************/
          /* return the xid command buffer to the pool.               */
          /************************************************************/

          lanfree(p, p->sta_ptr->xid_cmd_addr);

          /************************************************************/
          /* reset the xid command buffer address to indicate that the*/
          /* command buffer is no longer in use.                      */
          /************************************************************/

          p->sta_ptr->xid_cmd_addr = 0;

          /************************************************************/
          /* pass the xid response buffer to pu services with response*/
          /* pending indication off.                                  */
          /************************************************************/

          pass_rcv_xid(p, 0);

          /************************************************************/
          /* reset the contiguous command repolls sent ras counter.   */
          /************************************************************/

          p->sta_ptr->ras_counters.counters.cmd_cont_repolls = 0;

          /************************************************************/
          /* terminate timer t1                                       */
          /************************************************************/

          p->station_list[p->stano].t1_ctr = -1;
          p->station_list[p->stano].t1_ena = FALSE;
	  /************************************************************/
	  /* reset inact pending flags to indicate packet received    */
	  /************************************************************/
          p->sta_ptr->inact_pend = 0;
 
          if                           /* t3 timer state = inactivity */
             (p->sta_ptr->t3_state == T3_INACT)
            {

              /********************************************************/
              /* initiate inactivity timer (t3).                      */
              /********************************************************/

              p->station_list[p->stano].t3_ctr = 
                 p->sta_ptr->inact_to_val;
              p->station_list[p->stano].t3_ena = TRUE;
            } 
        } 
 
      else                             /* no xid response expected    */
        {

          /************************************************************/
          /* indicate that the xid response was not expected.         */
          /************************************************************/

          xid_not_expected = TRUE;
 
          if                           /* invalid frames received     
                                          counter not at maximum value*/
             (p->sta_ptr->ras_counters.counters.inv_pkt_rec != 
             MINUS_ONE)

            /**********************************************************/
            /* increment count of invalid frames received             */
            /**********************************************************/

            ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

          /************************************************************/
          /* call error log - extraneous xid response received.       */
          /************************************************************/

          lanerrlg(p, ERRID_LAN000A, NON_ALERT, TEMP_ERR, 0, FILEN, 
             LINEN);

          /************************************************************/
          /* ignore the xid response.                                 */
          /************************************************************/

          p->sta_ptr->ignore = TRUE;
        } 
    } 
 
  else                                 /* invalid xid response (final 
                                          bit was reset).             */
    {

      /****************************************************************/
      /* indicate that the xid response was not expected.             */
      /****************************************************************/

      xid_not_expected = TRUE;
 
      if                               /* invalid frames received     
                                          counter not at maximum value*/
         (p->sta_ptr->ras_counters.counters.inv_pkt_rec != MINUS_ONE)

        /**************************************************************/
        /* increment count of invalid frames received                 */
        /**************************************************************/

        ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

      /****************************************************************/
      /* call error log - extraneous xid command received.            */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN000D, NON_ALERT, TEMP_ERR, 0, FILEN, LINEN)
         ;

      /****************************************************************/
      /* ignore the xid response and wait for the t1 repoll timeout   */
      /* before re-sending the command.                               */
      /****************************************************************/

      p->sta_ptr->ignore = TRUE;
    } 

  /********************************************************************/
  /* return indication whether the xid response was expected.         */
  /********************************************************************/

  return (xid_not_expected);
}                                      /* end rcv_xid_rsp;            */
pass_rcv_xid(p,rsp_is_pend)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  pass_rcv_xid                                         */
/*                                                                    */
/* descriptive name:  pass the received xid to the user               */
/*                                                                    */
/* function:  processes the xid received and queues it for            */
/*            the user via the user's ring queue.                     */
/*                                                                    */
/* input:  received buffer pointer                                    */
/*                                                                    */
/* output:  none                                                      */
/*                                                                    */
/*** end of specifications ********************************************/

  register int rsp_is_pend;
{
  struct dlc_io_ext dlc_io_ext;
  struct dlc_chan *c_ptr;

  /********************************************************************/
  /* build the "xid received - response pending" result buffer.       */
  /********************************************************************/

  dlc_io_ext.flags = (DLC_XIDD|rsp_is_pend);
  dlc_io_ext.sap_corr = p->sap_ptr->sap_profile.user_sap_corr;
  dlc_io_ext.ls_corr = p->sta_ptr->ls_profile.user_ls_corr;

/* <<< feature CDLI >>> */
#ifndef TRLORFDDI
  dlc_io_ext.dlh_len = UN_HDR_LENGTH;
#endif /* not TRLORFDDI */
#ifdef TRLORFDDI
  dlc_io_ext.dlh_len = UN_HDR_LENGTH + (p->rcv_data.ri_field[0]&0x1f);
#endif /* TRLORFDDI */
/* <<< end feature CDLI >>> */

/* <<< defect 129185 >>> */
  p->m->m_data += dlc_io_ext.dlh_len;
/* <<< end defect 129185 >>> */
  p->m->m_len = p->lpdu_length-3;
 
  if                                   /* overflow occurred           */
     (p->m->m_len > p->sta_ptr->ls_profile.maxif)
    {

      /****************************************************************/
      /* set the result data length to the max i-field length.        */
      /****************************************************************/

      p->m->m_len = p->sta_ptr->ls_profile.maxif;

      /****************************************************************/
      /* set the overflow result flag.                                */
      /****************************************************************/

      dlc_io_ext.flags |= DLC_OFLO;
    } 
  c_ptr = p->sap_ptr->user_sap_channel;

  /********************************************************************/
  /* call the user RCV XID function                                   */
  /********************************************************************/

  p->rc = (*c_ptr->rcvx_fa)(p->m, &dlc_io_ext, c_ptr);
 
  switch (p->rc)
    {
      case  DLC_FUNC_OK :              /* normal case                 */
        break;
      case  DLC_FUNC_RETRY :
/* LEHb defect 43788 */
/* delete 1 line */
				       /* a wakeup is needed */
	p->station_list[p->stano].wakeup_needed = TRUE;
				       /* save the xid buffer address
					  and its extension           */
	p->sta_ptr->retry_rcvx_buf = p->m;
	bcopy (&(dlc_io_ext),
	       &(p->sta_ptr->retry_rcvx_ext),
	       sizeof(p->dlc_io_ext));
	break;
      default  :
				       /* invalid rc - ignore the xid */
	p->sta_ptr->ignore = TRUE;

	lanerrlg(p, ERRID_LAN8084, NON_ALERT, PERM_SAP_ERR, 0, FILEN,
	   LINEN);
/* LEHe */

    } 
}                                      /* end pass_rcv_xid;           */
test_rcvd(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  test_rcvd                                            */
/*                                                                    */
/* descriptive name:  test frame received                             */
/*                                                                    */
/* function:  processes the test frame received and either            */
/*            completes the test or sends a test response on the      */
/*            NETWORK.                                                */
/*                                                                    */
/* input:  received buffer pointer                                    */
/*                                                                    */
/* output:  none                                                      */
/*                                                                    */
/*** end of specifications ********************************************/

{
 
  if (p->debug)
    printf("===================test_rcvd %x %d\n", p->sta_ptr, 
       p->sta_ptr->ignore);

  /********************************************************************/
  /* receive is valid, call the check inactivity routine.             */
  /********************************************************************/

  valid_rcv(p);
 
  switch                               /* link state                  */
     (p->sta_ptr->ls)
    {
      case                             /* state = disconnected mode   */
         (LS_ADM) :
          {
 
            if                         /* the test is a command packet*/
               (TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE)
              {
 
                if                     /* t3 timer state = inactivity */
                   (p->sta_ptr->t3_state == T3_INACT)
                  {

                    /**************************************************/
                    /* start inactivity timer (t3).                   */
                    /**************************************************/

                    p->station_list[p->stano].t3_ctr = p->sta_ptr->inact_to_val;
                    p->station_list[p->stano].t3_ena = TRUE;
                  } 

                /******************************************************/
                /* call the test response generator.                  */
                /******************************************************/

                lantrgen(p);
              } 
 
            else                       /* test was a response         */
              {

                /******************************************************/
                /* call the test response received routine.           */
                /******************************************************/

                p->rc = test_rsp_rcvd(p);
              } 
          } 
        break;
      case                             /* state = abme                */
         (LS_ABME) :
          {
 
            if                         /* test is a command           */
               (TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE)
              {

                /******************************************************/
                /* call the test response generator.                  */
                /******************************************************/

                lantrgen(p);
              } 
 
            else                       /* test was a response         */
              {

                /******************************************************/
                /* call the test response received routine.           */
                /******************************************************/

                p->rc = test_rsp_rcvd(p);
 
                if                     /* the test response was       
                                          expected                    */
                   (p->rc == 0)
                  {

                    /**************************************************/
                    /* reset checkpointing indicated in p->sta_ptr->  */
                    /**************************************************/

                    p->sta_ptr->checkpointing = FALSE;
 
                    if                 /* no commands are stacked on  
                                          vc                          */
                       (p->sta_ptr->vc == NONE_ENA)
                      {
 
                        if             /* not remote busy             */
                           (p->sta_ptr->remote_busy == FALSE)

                          /********************************************/
                          /* enable sending of i-frames               */
                          /********************************************/

                          p->sta_ptr->iframes_ena = TRUE;
 
                        if             /* clearing local busy in      
                                          p->sta_ptr->                */
                           (p->sta_ptr->clearing == TRUE)
                          {

                            /******************************************/
                            /* reset clearing local busy in           */
                            /* p->sta_ptr->                           */
                            /******************************************/

                            p->sta_ptr->clearing = FALSE;

                            /******************************************/
                            /* send rr command with p/f on.           */
                            /******************************************/

			    tx_sta_cmd(p, RR, (p->sta_ptr->vr|PF2_MASK), 2);
                            /* set checkpointing back on.   */
                            p->sta_ptr->checkpointing = TRUE;
                            /* disable sending of i-frames. */
                            p->sta_ptr->iframes_ena = FALSE;


                          } 
                      } 
 
                    else               /* a disc, xid, or test command
                                          is stacked in vc            */
                      {

                        /**********************************************/
                        /* call the process vc routine                */
                        /**********************************************/

                        proc_vc(p);
                      } 
                  } 
              } 
          } 
        break;

        /**************************************************************/
        /* state = abme pending, contacting or discontacting mode     */
        /**************************************************************/

      case  LS_ABME_PEND :
      case  LS_CONTACTING :
      case  LS_DISCONTACTING :
          {

            /**********************************************************/
            /* ignore the frame                                       */
            /**********************************************************/

            p->sta_ptr->ignore = TRUE;
          } 
        break;
      default  :                       /* not a valid state for test  
                                          frame .                     */
          {

            /**********************************************************/
            /* ignore the frame                                       */
            /**********************************************************/

            p->sta_ptr->ignore = TRUE;
 
            if                         /* invalid frames received     
                                          counter not at maximum value*/
               (p->sta_ptr->ras_counters.counters.inv_pkt_rec != 
               MINUS_ONE)

              /********************************************************/
              /* increment count of invalid frames received           */
              /********************************************************/

              ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

            /**********************************************************/
            /* call error log - invalid state to receive test frame   */
            /**********************************************************/

            lanerrlg(p, ERRID_LAN000E, NON_ALERT, TEMP_ERR, 0, FILEN, 
               LINEN);
          } 
    }                                  /* end switch;                 */
}                                      /* end test_rcvd;              */
sabme_cmd_rcvd(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  sabme_cmd_rcvd                                       */
/*                                                                    */
/* descriptive name:  SABME command received                          */
/*                                                                    */
/* function:  processes the SABME command frame received and          */
/*            determines what to transmit over the NETWORK next.      */
/*                                                                    */
/* input:  received buffer pointer                                    */
/*                                                                    */
/* output:  none                                                      */
/*                                                                    */
/*** end of specifications ********************************************/

{
  u_char   temp_pf;
 

  if (p->debug)
    printf("================>sabme_cmd_rcvd\n");
 
  if                                   /* there is no i-field attached*/
     (p->lpdu_length == 3)
    {
 
      if                               /* it is a command frame       */
         (TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE)
        {

          /************************************************************/
          /* receive is valid, call the check inactivity routine.     */
          /************************************************************/

          valid_rcv(p);
 
          switch                       /* link state                  */
             (p->sta_ptr->ls)
            {

              /********************************************************/
              /* state = disconnected mode or abme pending mode       */
              /********************************************************/

              case  LS_ADM :
              case  LS_ABME_PEND :
                  {

                    /**************************************************/
                    /* transmit a ua response, p/f = SABME note - must*/
                    /* be in two steps for the compiler.              */
                    /**************************************************/

                    temp_pf = (p->rcv_data.ctl1&PF1_MASK);
                    tx_buf_rsp(p, (UA_NO_PF|temp_pf), DLC_NULL, 1);
 
                    if                 /* t3 timer state = inactivity */
                       (p->sta_ptr->t3_state == T3_INACT)
                      {

                        /**********************************************/
                        /* start inactivity timer (t3).               */
                        /**********************************************/

                        p->station_list[p->stano].t3_ctr = 
                           p->sta_ptr->inact_to_val;
                        p->station_list[p->stano].t3_ena = TRUE;
                      } 

                    /**************************************************/
                    /* set link state (ls) = LS_ABME_PEND             */
                    /**************************************************/

                    p->sta_ptr->ls = LS_ABME_PEND;
 
                    if                 /* an xid response is pending  */
                       (p->sta_ptr->xid_ir_pend == TRUE)
                      {

                        /**********************************************/
                        /* terminate timer t1                         */
                        /**********************************************/

                        p->station_list[p->stano].t1_ctr = -1;
                        p->station_list[p->stano].t1_ena = FALSE;
	  		/**********************************************/
	  		/* reset inact pend flgs - indicate pkt rcvd  */
	  		/**********************************************/
            		p->sta_ptr->inact_pend = 0;
                      } 

                    /**************************************************/
                    /* set xid state (xs) = 0                         */
                    /**************************************************/

                    p->sta_ptr->xs = XID_RESET;
                    p->sta_ptr->xid_ir_pend = XID_RESET;
                    p->sta_ptr->xid_or_pend = XID_RESET;

                    /**************************************************/
                    /* set va = vs = vr = 0.                          */
                    /**************************************************/

                    p->sta_ptr->va = 0;
                    p->sta_ptr->vs = 0;
                    p->sta_ptr->vr = 0;

                    /**************************************************/
                    /* set is_ct = n2                                 */
                    /**************************************************/

                    p->sta_ptr->is_ct = 
                       p->sta_ptr->ls_profile.max_repoll;

                    /**************************************************/
                    /* set ir_ct = n3                                 */
                    /**************************************************/

                    p->sta_ptr->ir_ct = 
                       p->sta_ptr->ls_profile.rcv_wind;
                  } 
                break;
              case                     /* state = contacting          */
                 (LS_CONTACTING) :
                  {

                    /**************************************************/
                    /* transmit a ua response, p/f = SABME note - must*/
                    /* be in two steps for the compiler.              */
                    /**************************************************/

                    temp_pf = (p->rcv_data.ctl1&PF1_MASK);
                    tx_buf_rsp(p, (UA_NO_PF|temp_pf), DLC_NULL, 1);

                    /**************************************************/
                    /* note: t1 should still be running               */
                    /**************************************************/

                  } 
                break;

                /******************************************************/
                /* state = abme or discontacting mode                 */
                /******************************************************/

              case  LS_ABME :
              case  LS_DISCONTACTING :
                  {

                    /**************************************************/
                    /* set reason code - mid session reset            */
                    /**************************************************/

                    p->sta_ptr->closing_reason = DLC_MSESS_RE;

                    /**************************************************/
                    /* send a dm response with final bit = received   */
                    /* poll bit. note - must be in two steps for the  */
                    /* compiler.                                      */
                    /**************************************************/

                    temp_pf = (p->rcv_data.ctl1&PF1_MASK);
                    tx_disc_rsp(p, DM_NO_PF|temp_pf);
                  } 
                break;
              default  :               /* not a valid state for SABME 
                                          command.                    */
                  {
 
                    if                 /* invalid frames received     
                                          counter not at maximum value*/
                       (p->sta_ptr->ras_counters.counters.inv_pkt_rec 
                       != MINUS_ONE)

                      /************************************************/
                      /* increment count of invalid frames received   */
                      /************************************************/

                      ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

                    /**************************************************/
                    /* call error log - invalid state to receive SABME*/
                    /* command                                        */
                    /**************************************************/

/* defect 86180 */
                    lanerrlg(p, ERRID_LAN_ALERT3, ALERT, TEMP_ERR, 0,
                       FILEN, LINEN);
                    lanerrlg(p, ERRID_LAN000F, NON_ALERT, TEMP_ERR, 0,
                       FILEN, LINEN);
/* end defect 86180 */

                    /**************************************************/
                    /* indicate that the buffer is to be returned to  */
                    /* the pool.                                      */
                    /**************************************************/

                    p->sta_ptr->ignore = TRUE;
                  } 
            }                          /* end switch;                 */
        } 
 
      else                             /* it's not a command frame    */
        {

          /************************************************************/
          /* call error log - received SABME not command.             */
          /************************************************************/

          lanerrlg(p, ERRID_LAN0010, NON_ALERT, TEMP_ERR, 0, FILEN, 
             LINEN);

          /************************************************************/
          /* call invalid lpdu received routine to generate a possible*/
          /* frame reject.                                            */
          /************************************************************/

          inv_lpdu_rcvd(p, FRMR_INV_CTL_RCVD, TSTBIT(p->rcv_data.ctl1,
             POLL_FINAL_1));
        } 
    } 
 
  else                                 /* error - invalid i-field     
                                          received.                   */
    {

      /****************************************************************/
      /* call error log - received SABME > 3-bytes.                   */
      /****************************************************************/

/* defect 86180 */
      lanerrlg(p, ERRID_LAN_ALERT9, ALERT, TEMP_ERR, 0, FILEN, LINEN);
      lanerrlg(p, ERRID_LAN0011, NON_ALERT, TEMP_ERR, 0, FILEN, LINEN);
/* end defect 86180 */

      /****************************************************************/
      /* call invalid lpdu received routine to generate a possible    */
      /* frame reject.                                                */
      /****************************************************************/

      inv_lpdu_rcvd(p, FRMR_INV_IFIELD_RCVD, TSTBIT(p->rcv_data.ctl1, 
         POLL_FINAL_1));
    } 
}                                      /* end sabme_cmd_rcvd;         */
disc_cmd_rcvd(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  disc_cmd_rcvd                                        */
/*                                                                    */
/* descriptive name:  disc command received                           */
/*                                                                    */
/* function:  processes the disc command frame received and           */
/*            determines what to transmit over the NETWORK next.      */
/*                                                                    */
/* input:  received buffer pointer                                    */
/*                                                                    */
/* output:  none                                                      */
/*                                                                    */
/*** end of specifications ********************************************/

{
  int      temp_pf;
 

  if (p->debug)
    printf("================>disc_cmd_rcvd\n");
 
  if                                   /* there is no i-field attached*/
     (p->lpdu_length == 3)
    {
 
      if                               /* it is a command frame       */
         (TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE)
        {
 
          switch                       /* link state                  */
             (p->sta_ptr->ls)
            {

              /********************************************************/
              /* state = disconnected, abme pending, or contacting    */
              /* mode                                                 */
              /********************************************************/

              case  LS_ADM :
              case  LS_ABME_PEND :
              case  LS_CONTACTING :
                  {

                    /**************************************************/
                    /* set the closing reason code = remote initiated */
                    /* discontact.                                    */
                    /**************************************************/

                    p->sta_ptr->closing_reason = DLC_RDISC;

                    /**************************************************/
                    /* transmit a dm response with final bit = disc   */
                    /* poll bit. note - must be in two steps for the  */
                    /* compiler.                                      */
                    /**************************************************/

                    temp_pf = (p->rcv_data.ctl1&PF1_MASK);
                    tx_disc_rsp(p, DM_NO_PF|temp_pf);
                  } 
                break;
              case                     /* state = abme                */
                 (LS_ABME) :
                  {

                    /**************************************************/
                    /* set the closing reason code = remote initiated */
                    /* discontact.                                    */
                    /**************************************************/

                    p->sta_ptr->closing_reason = DLC_RDISC;

                    /**************************************************/
                    /* transmit a ua response with final bit = disc   */
                    /* poll bit. note - must be in two steps for the  */
                    /* compiler.                                      */
                    /**************************************************/

                    temp_pf = (p->rcv_data.ctl1&PF1_MASK);
                    tx_disc_rsp(p, UA_NO_PF|temp_pf);
                  } 
                break;
              case                     /* state = discontacting mode  */
                 (LS_DISCONTACTING) :
                  {

                    /**************************************************/
                    /* transmit a ua response with final bit = disc   */
                    /* poll bit. note1 - must be in two steps for the */
                    /* compiler. note2 - must not leave discontacting */
                    /* mode.                                          */
                    /**************************************************/

                    temp_pf = (p->rcv_data.ctl1&PF1_MASK);
                    tx_disc_rsp(p, UA_NO_PF|temp_pf);
                  } 
                break;
              default  :               /* not a valid state for disc  
                                          command.                    */
                  {
 
                    if                 /* invalid frames received     
                                          counter not at maximum value*/
                       (p->sta_ptr->ras_counters.counters.inv_pkt_rec 
                       != MINUS_ONE)

                      /************************************************/
                      /* increment count of invalid frames received   */
                      /************************************************/

                      ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

                    /**************************************************/
                    /* call error log - invalid state to receive disc */
                    /* command                                        */
                    /**************************************************/

                    lanerrlg(p, ERRID_LAN0016, NON_ALERT, TEMP_ERR, 0,
                       FILEN, LINEN);

                    /**************************************************/
                    /* indicate that the buffer is to be returned to  */
                    /* the pool.                                      */
                    /**************************************************/

                    p->sta_ptr->ignore = TRUE;
                  } 
            }                          /* end switch;                 */
        } 
 
      else                             /* it's not a command frame    */
        {

          /************************************************************/
          /* call error log - received disc not command.              */
          /************************************************************/

          lanerrlg(p, ERRID_LAN0017, NON_ALERT, TEMP_ERR, 0, FILEN, 
             LINEN);

          /************************************************************/
          /* call invalid lpdu received routine to generate a possible*/
          /* frame reject.                                            */
          /************************************************************/

          inv_lpdu_rcvd(p, FRMR_INV_CTL_RCVD, TSTBIT(p->rcv_data.ctl1,
             POLL_FINAL_1));
        } 
    } 
 
  else                                 /* error - invalid i-field     
                                          received.                   */
    {

      /****************************************************************/
      /* call error log - received disc > 3-bytes.                    */
      /****************************************************************/

/* defect 86180 */
      lanerrlg(p, ERRID_LAN_ALERT9, ALERT, TEMP_ERR, 0, FILEN, LINEN);
      lanerrlg(p, ERRID_LAN0012, NON_ALERT, TEMP_ERR, 0, FILEN, LINEN);
/* end defect 86180 */

      /****************************************************************/
      /* call invalid lpdu received routine to generate a possible    */
      /* frame reject.                                                */
      /****************************************************************/

      inv_lpdu_rcvd(p, FRMR_INV_IFIELD_RCVD, TSTBIT(p->rcv_data.ctl1, 
         POLL_FINAL_1));
    } 
}                                      /* end disc_cmd_rcvd;          */
ua_rsp_rcvd(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  ua_rsp_rcvd                                          */
/*                                                                    */
/* descriptive name:  ua response received                            */
/*                                                                    */
/* function:  processes the ua response frame received and            */
/*            determines what to transmit over the NETWORK next.      */
/*                                                                    */
/* input:  received buffer pointer                                    */
/*                                                                    */
/* output:  none                                                      */
/*                                                                    */
/*** end of specifications ********************************************/

{
  struct mbuf *temp_buf_ptr;
 

  if (p->debug)
    printf("===========>ua_rsp_rcvd\n");
 
  if                                   /* there is no i-field attached*/
     (p->lpdu_length == 3)
    {
 
      if                               /* it is a response frame      */
         (TSTBIT(p->rcv_data.rsap, RESPONSE) == TRUE)
        {

          /************************************************************/
          /* indicate that the buffer is to be returned to the pool.  */
          /************************************************************/

          p->sta_ptr->ignore = TRUE;

          /************************************************************/
          /* stack the current buffer pointer in order to free it     */
          /* later.                                                   */
          /************************************************************/

          temp_buf_ptr = p->m;
 
          switch                       /* link state                  */
             (p->sta_ptr->ls)
            {
              case                     /* state = contacting mode     */
                 (LS_CONTACTING) :
                  {
 
                    if                 /* the final bit is set.       */
		       (TSTBIT(p->rcv_data.ctl1, POLL_FINAL_1) == TRUE)
                      {

                        /**********************************************/
                        /* inform the higher layer - "contacted"      */
                        /**********************************************/

                        station_contacted(p);

                        /**********************************************/
                        /* reset the state variables (va = vs = vr =  */
                        /* 0).                                        */
                        /**********************************************/

                        p->sta_ptr->va = 0;
                        p->sta_ptr->vs = 0;
                        p->sta_ptr->vr = 0;

                        /**********************************************/
                        /* reset the test, xid and stacked control    */
                        /* state variables (ts = xs = vc = 0).        */
                        /**********************************************/

                        p->sta_ptr->xs = XID_RESET;
                        p->sta_ptr->xid_ir_pend = XID_RESET;
                        p->sta_ptr->xid_or_pend = XID_RESET;
                        p->sta_ptr->ts = TEST_RESET;
                        p->sta_ptr->test_ir_pend = TEST_RESET;
                        p->sta_ptr->vc = NONE_ENA;

                        /**********************************************/
                        /* initialize and start response timer t1     */
                        /**********************************************/

                        p->station_list[p->stano].t1_ctr = 
                           p->sta_ptr->resp_to_val;
                        p->station_list[p->stano].t1_ena = TRUE;

                        /**********************************************/
                        /* reset inactivity timer (t3) that was set in*/
                        /* station_contacted. t3 should not be active */
                        /* at the same time as t1.                    */
                        /**********************************************/

                        p->station_list[p->stano].t3_ctr = -1;
                        p->station_list[p->stano].t3_ena = FALSE;

                        /**********************************************/
                        /* reset the contiguous command repolls sent  */
                        /* ras counter.                               */
                        /**********************************************/

                        p->sta_ptr->ras_counters.counters.cmd_cont_repolls
                           = 0;

                        /**********************************************/
                        /* set the poll retry count (p_ct) = max (n2).*/
                        /**********************************************/

                        p->sta_ptr->p_ct = 
                           p->sta_ptr->ls_profile.max_repoll;

                        /**********************************************/
                        /* set the i-frame retry count (is_ct) = max  */
                        /* (n2).                                      */
                        /**********************************************/

                        p->sta_ptr->is_ct = 
                           p->sta_ptr->ls_profile.max_repoll;

                        /**********************************************/
                        /* set the acknowledgement delay count (ir_ct)*/
                        /* = max (n3).                                */
                        /**********************************************/

                        p->sta_ptr->ir_ct = 
                           p->sta_ptr->ls_profile.rcv_wind;

                        /**********************************************/
                        /* go into checkpointing.                     */
                        /**********************************************/

                        p->sta_ptr->checkpointing = TRUE;

                        /**********************************************/
                        /* disable sending of i-frames.               */
                        /**********************************************/

                        p->sta_ptr->iframes_ena = FALSE;
 
                        if             /* not local busy              */
                           (p->sta_ptr->local_busy == FALSE)
                          {

                            /******************************************/
                            /* transmit an rr command with the final  */
                            /* bit set.                               */
                            /******************************************/

			    tx_sta_cmd(p, RR, (PF2_MASK|p->sta_ptr->vr), 2);

                          } 
 
                        else
                          {

                            /******************************************/
                            /* transmit an rnr command with the final */
                            /* bit set.                               */
                            /******************************************/

                            tx_sta_cmd(p, RNR, (PF2_MASK|
                               p->sta_ptr->vr), 2);
                          } 
                      } 
 
                    else               /* error - ignore the lpdu -   
                                          final bit not equal to 1    */
                      {
 
                        if             /* invalid frames received     
                                          counter not at maximum value*/
                           (
                          p->sta_ptr->ras_counters.counters.inv_pkt_rec
                           != MINUS_ONE)

                          /********************************************/
                          /* increment count of invalid frames        */
                          /* received                                 */
                          /********************************************/

			  ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

                        /**********************************************/
                        /* call error log - unknown command received. */
                        /**********************************************/

                        lanerrlg(p, ERRID_LAN0019, NON_ALERT, 
                           TEMP_ERR, 0, FILEN, LINEN);
                      } 
                  } 
                break;
              case                     /* state = abme                */
                 (LS_ABME) :
                  {

/* LEHb defect 44499 */
		    if                 /* Info or supervisory packets
					  have already been received  */
		       (p->sta_ptr->iors_rcvd == TRUE)
		      {
		      /************************************************/
		      /* set closing reason code - protocol error.    */
		      /************************************************/

		      p->sta_ptr->closing_reason = DLC_PROT_ERR;

		      /************************************************/
		      /* call the send frmr routine with final bit set*/
		      /* off, for an invalid control byte received.   */
		      /************************************************/

		      send_frmr(p, FRMR_INV_CTL_RCVD, FRMR_PF_OFF);
		      }
		    else               /* possible multiple ua responses
					  during contact proceedure   */
		      {
		      /************************************************/
		      /* just fall through and ignore the packet      */
		      /************************************************/
		      }
/* LEHe */
                  } 
                break;
              case                     /* state = discontacting mode  */
                 (LS_DISCONTACTING) :
                  {
 
                    if                 /* final bit set.              */
		       (TSTBIT(p->rcv_data.ctl1, POLL_FINAL_1) == TRUE)
                      {

                        /**********************************************/
                        /* call shutdown to close the logical link.   */
                        /**********************************************/

                        shutdown(p, HARD);
                      } 
 
                    else               /* error - ignore the lpdu -   
                                          final bit not equal to 1    */
                      {
 
                        if             /* invalid frames received     
                                          counter not at maximum value*/
                           (
                          p->sta_ptr->ras_counters.counters.inv_pkt_rec
                           != MINUS_ONE)

                          /********************************************/
                          /* increment count of invalid frames        */
                          /* received                                 */
                          /********************************************/

			  ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

                        /**********************************************/
                        /* call error log - unknown command received. */
                        /**********************************************/

                        lanerrlg(p, ERRID_LAN001A, NON_ALERT, 
                           TEMP_ERR, 0, FILEN, LINEN);
                      } 
                  } 
                break;

                /******************************************************/
                /* state = discontacted mode or abme pending mode     */
                /******************************************************/

              case  LS_ADM :
              case  LS_ABME_PEND :
                  {

                    /**************************************************/
                    /* ignore the frame                               */
                    /**************************************************/

                  } 
                break;
              default  :               /* not a valid state for ua    
                                          response.                   */
                  {
 
                    if                 /* invalid frames received     
                                          counter not at maximum value*/
                       (p->sta_ptr->ras_counters.counters.inv_pkt_rec 
                       != MINUS_ONE)

                      /************************************************/
                      /* increment count of invalid frames received   */
                      /************************************************/

                      ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

                    /**************************************************/
                    /* call error log - invalid state to receive ua   */
                    /* response                                       */
                    /**************************************************/

/* defect 86180 */
                    lanerrlg(p, ERRID_LAN_ALERT8, ALERT, TEMP_ERR, 0,
                       FILEN, LINEN);
                    lanerrlg(p, ERRID_LAN001B, NON_ALERT, TEMP_ERR, 0,
                       FILEN, LINEN);
/* end defect 86180 */
                  } 
            }                          /* end switch;                 */

          /************************************************************/
          /* restore the response buffer pointer in order to free it. */
          /************************************************************/

          p->m = temp_buf_ptr;
        } 
 
      else                             /* it's not a response frame   */
        {

          /************************************************************/
          /* call error log - received ua not a response.             */
          /************************************************************/

          lanerrlg(p, ERRID_LAN001C, NON_ALERT, TEMP_ERR, 0, FILEN, 
             LINEN);

          /************************************************************/
          /* call invalid lpdu received routine to generate a possible*/
          /* frame reject.                                            */
          /************************************************************/

          inv_lpdu_rcvd(p, FRMR_INV_CTL_RCVD, TSTBIT(p->rcv_data.ctl1,
             POLL_FINAL_1));
        } 
    } 
 
  else                                 /* error - invalid i-field     
                                          received.                   */
    {

      /****************************************************************/
      /* call error log - received ua > 3-bytes.                      */
      /****************************************************************/

/* defect 86180 */
      lanerrlg(p, ERRID_LAN_ALERT9, ALERT, TEMP_ERR, 0, FILEN, LINEN);
      lanerrlg(p, ERRID_LAN001D, NON_ALERT, TEMP_ERR, 0, FILEN, LINEN);
/* end defect 86180 */

      /****************************************************************/
      /* call invalid lpdu received routine to generate a possible    */
      /* frame reject.                                                */
      /****************************************************************/

      inv_lpdu_rcvd(p, FRMR_INV_IFIELD_RCVD, TSTBIT(p->rcv_data.ctl1, 
         POLL_FINAL_1));
    } 
}                                      /* end ua_rsp_rcvd;            */
dm_rsp_rcvd(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  dm_rsp_rcvd                                          */
/*                                                                    */
/* descriptive name:  dm response received                            */
/*                                                                    */
/* function:  processes the dm response frame received and            */
/*            determines what to transmit over the NETWORK next.      */
/*                                                                    */
/* input:  received buffer pointer                                    */
/*                                                                    */
/* output:  none                                                      */
/*                                                                    */
/*** end of specifications ********************************************/

{
 
  if (p->debug)
    printf("============dm_rsp_rcvd\n");
 
  if                                   /* there is no i-field attached*/
     (p->lpdu_length == 3)
    {
 
      if                               /* it is a response frame      */
         (TSTBIT(p->rcv_data.rsap, RESPONSE) == TRUE)
        {
 
          switch                       /* link state                  */
             (p->sta_ptr->ls)
            {

              /********************************************************/
              /* state = contacting,discontacting,abme_pend, or abme  */
              /* mode                                                 */
              /********************************************************/

              case  LS_CONTACTING :
              case  LS_DISCONTACTING :
              case  LS_ABME_PEND :
              case  LS_ABME :
                  {

                    /**************************************************/
                    /* set the closing reason code = remote initiated */
                    /* discontact.                                    */
                    /**************************************************/

                    p->sta_ptr->closing_reason = DLC_RDISC;

                    /**************************************************/
                    /* call the shutdown routine (hard).              */
                    /**************************************************/

                    shutdown(p, HARD);
                  } 
                break;
              case                     /* state = discontacted mode   */
                 (LS_ADM) :
                  {

                    /**************************************************/
                    /* ignore the frame                               */
                    /**************************************************/

                  } 
                break;
              default  :               /* not a valid state for dm    
                                          response.                   */
                  {
 
                    if                 /* invalid frames received     
                                          counter not at maximum value*/
                       (p->sta_ptr->ras_counters.counters.inv_pkt_rec 
                       != MINUS_ONE)

                      /************************************************/
                      /* increment count of invalid frames received   */
                      /************************************************/

                      ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

                    /**************************************************/
                    /* call error log - invalid state to receive dm   */
                    /* response                                       */
                    /**************************************************/

                    lanerrlg(p, ERRID_LAN001E, NON_ALERT, TEMP_ERR, 0,
                       FILEN, LINEN);
                  } 
            }                          /* end switch;                 */
/* defect 142033 */
          /************************************************************/
          /* call buffer management to return the receive buffer.     */
          /* Note: buffer must be freed here because the station may  */
          /*       already be shutdown.                               */
          /************************************************************/

          lanfree(p, p->m);
/* end defect 142033 */
        } 
 
      else                             /* it's not a response frame   */
        {

          /************************************************************/
          /* call error log - received dm not a response.             */
          /************************************************************/

          lanerrlg(p, ERRID_LAN001F, NON_ALERT, TEMP_ERR, 0, FILEN, 
             LINEN);

          /************************************************************/
          /* call invalid lpdu received routine to generate a possible*/
          /* frame reject.                                            */
          /************************************************************/

          inv_lpdu_rcvd(p, FRMR_INV_CTL_RCVD, TSTBIT(p->rcv_data.ctl1,
             POLL_FINAL_1));
        } 
    } 
 
  else                                 /* error - invalid i-field     
                                          received.                   */
    {

      /****************************************************************/
      /* call error log - received dm > 3-bytes.                      */
      /****************************************************************/

/* defect 86180 */
      lanerrlg(p, ERRID_LAN_ALERT9, ALERT, TEMP_ERR, 0, FILEN, LINEN);
      lanerrlg(p, ERRID_LAN0020, NON_ALERT, TEMP_ERR, 0, FILEN, LINEN);
/* end defect 86180 */

      /****************************************************************/
      /* call invalid lpdu received routine to generate a possible    */
      /* frame reject.                                                */
      /****************************************************************/

      inv_lpdu_rcvd(p, FRMR_INV_IFIELD_RCVD, TSTBIT(p->rcv_data.ctl1, 
         POLL_FINAL_1));
    } 
}                                      /* end dm_rsp_rcvd;            */
ui_rcvd(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  ui_rcvd                                              */
/*                                                                    */
/* descriptive name:  UNNUMBERED information received                 */
/*                                                                    */
/* function:  processes the datagram and sends it up to the user.     */
/*                                                                    */
/* input:  received buffer pointer                                    */
/*                                                                    */
/* output:  datagram set to the user                                  */
/*                                                                    */
/*** end of specifications ********************************************/

{
  struct dlc_io_ext dlc_io_ext;
  struct dlc_chan *c_ptr;
 

  if (p->debug)
    printf("=============ui_rcvd\n");

  /********************************************************************/
  /* receive is valid, call the check inactivity routine.             */
  /********************************************************************/

  valid_rcv(p);

/* LEHb defect 43788 */
  /* if the user does not already have a busy condition on receive
							    datagrams */
  if (p->sta_ptr->retry_rcvd_buf == 0)
  {
/* LEHe */
 
  if                                   /* t3 timer state = inactivity */
     (p->sta_ptr->t3_state == T3_INACT)
    {

      /****************************************************************/
      /* restart inactivity timer (t3)                                */
      /****************************************************************/

      p->station_list[p->stano].t3_ctr = p->sta_ptr->inact_to_val;
      p->station_list[p->stano].t3_ena = TRUE;
    } 

  /********************************************************************/
  /* build the "Datagram" result buffer.                              */
  /********************************************************************/

  dlc_io_ext.flags = DLC_DGRM;
  dlc_io_ext.sap_corr = p->sap_ptr->sap_profile.user_sap_corr;
  dlc_io_ext.ls_corr = p->sta_ptr->ls_profile.user_ls_corr;

/* <<< feature CDLI >>> */
#ifndef TRLORFDDI
  dlc_io_ext.dlh_len = UN_HDR_LENGTH;
#endif /* not TRLORFDDI */
#ifdef TRLORFDDI
  dlc_io_ext.dlh_len = UN_HDR_LENGTH + (p->rcv_data.ri_field[0]&0x1f);
#endif /* TRLORFDDI */
/* <<< end feature CDLI >>> */

/* <<< defect 129185 >>> */
  p->m->m_data += dlc_io_ext.dlh_len;
/* <<< end defect 129185 >>> */
  p->m->m_len = p->lpdu_length-3;
 
  if                                   /* overflow occurred           */
     (p->m->m_len > p->sta_ptr->ls_profile.maxif)
    {

      /****************************************************************/
      /* set the result data length to the max i-field length.        */
      /****************************************************************/

      p->m->m_len = p->sta_ptr->ls_profile.maxif;

      /****************************************************************/
      /* set the overflow result flag.                                */
      /****************************************************************/

      dlc_io_ext.flags |= DLC_OFLO;
    } 
  c_ptr = p->sap_ptr->user_sap_channel;

  /********************************************************************/
  /* call the user RCV datagram function                              */
  /********************************************************************/

  p->rc = (*c_ptr->rcvd_fa)(p->m, &dlc_io_ext, c_ptr);
 
  switch (p->rc)
    {
      case  DLC_FUNC_OK :              /* normal case                 */
        break;
      case  DLC_FUNC_RETRY :
/* LEHb defect 43788 */
/* delete 1 line */
				       /* a wakeup is needed */
	p->station_list[p->stano].wakeup_needed = TRUE;
				       /* save the datagram buffer address
					  and its extension           */
	p->sta_ptr->retry_rcvd_buf = p->m;
	bcopy (&(dlc_io_ext),
	       &(p->sta_ptr->retry_rcvd_ext),
	       sizeof(p->dlc_io_ext));
	break;
      default  :
				       /* invalid rc - ignore the xid */
	p->sta_ptr->ignore = TRUE;

	lanerrlg(p, ERRID_LAN8085, NON_ALERT, PERM_SAP_ERR, 0, FILEN,
	   LINEN);
    }
  }
  else
				       /* the user already has a busy
					  condition on receive datagrams
					  so ignore the received XID  */
    p->sta_ptr->ignore = TRUE;
/* LEHe */
}                                      /* end ui_rcvd;                */
frmr_rsp_rcvd(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  frmr_rsp_rcvd                                        */
/*                                                                    */
/* descriptive name:  frmr response received                          */
/*                                                                    */
/* function:  processes the frmr response frame received and          */
/*            starts to terminate the logical link.                   */
/*                                                                    */
/* input:  received buffer pointer                                    */
/*                                                                    */
/* output:  none                                                      */
/*                                                                    */
/*** end of specifications ********************************************/

{
 
  if (p->debug)
    printf("==================>frmr_rsp_rcvd\n");
 
  if                                   /* the lpdu is exactly 8 bytes 
                                          in length                   */
     (p->lpdu_length == 8)
    {
 
      if                               /* it is a response frame      */
         (TSTBIT(p->rcv_data.rsap, RESPONSE) == TRUE)
        {
 
          switch                       /* link state                  */
             (p->sta_ptr->ls)
            {
              case                     /* state = abme mode           */
                 (LS_ABME) :
                  {

                    /**************************************************/
                    /* set closing reason code - protocol error.      */
                    /**************************************************/

                    p->sta_ptr->closing_reason = DLC_PROT_ERR;

                    /**************************************************/
                    /* call the shutdown routine inorder to send      */
                    /* disconnect.                                    */
                    /**************************************************/

                    shutdown(p, FRMR_SENT);
                  } 
                break;

                /******************************************************/
                /* state = contacting,discontacting,abme_pend, or adm */
                /* mode                                               */
                /******************************************************/

              case  LS_CONTACTING :
              case  LS_DISCONTACTING :
              case  LS_ABME_PEND :
              case  LS_ADM :
                  {

                    /**************************************************/
                    /* ignore the frame                               */
                    /**************************************************/

                  } 
                break;
              default  :               /* not a valid state for frmr  
                                          response.                   */
                  {
 
                    if                 /* invalid frames received     
                                          counter not at maximum value*/
                       (p->sta_ptr->ras_counters.counters.inv_pkt_rec 
                       != MINUS_ONE)

                      /************************************************/
                      /* increment count of invalid frames received   */
                      /************************************************/

                      ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

                    /**************************************************/
                    /* call error log - invalid state to receive frmr */
                    /* response                                       */
                    /**************************************************/

                    lanerrlg(p, ERRID_LAN0021, NON_ALERT, TEMP_ERR, 0,
                       FILEN, LINEN);
                  } 
            }                          /* end switch;                 */

          /************************************************************/
          /* return buffer to the pool.                 defect 156952 */
          /************************************************************/

          lanfree(p, p->m);
        } 
 
      else                             /* it's not a response frame   */
        {

          /************************************************************/
          /* call error log - received frmr not a response.           */
          /************************************************************/

          lanerrlg(p, ERRID_LAN0022, NON_ALERT, TEMP_ERR, 0, FILEN, 
             LINEN);

          /************************************************************/
          /* call invalid lpdu received routine to generate a possible*/
          /* frame reject.                                            */
          /************************************************************/

          inv_lpdu_rcvd(p, FRMR_INV_CTL_RCVD, TSTBIT(p->rcv_data.ctl1,
             POLL_FINAL_1));
        } 
    } 
 
  else                                 /* error - invalid i-field     
                                          received.                   */
    {

      /****************************************************************/
      /* call error log - received frmr i-field != 5-bytes.           */
      /****************************************************************/

/* defect 86180 */
      lanerrlg(p, ERRID_LAN_ALERT9, ALERT, TEMP_ERR, 0, FILEN, LINEN);
      lanerrlg(p, ERRID_LAN0023, NON_ALERT, TEMP_ERR, 0, FILEN, LINEN);
/* end defect 86180 */

      /****************************************************************/
      /* call invalid lpdu received routine to generate a possible    */
      /* frame reject.                                                */
      /****************************************************************/

      inv_lpdu_rcvd(p, FRMR_INV_IFIELD_RCVD, TSTBIT(p->rcv_data.ctl1, 
         POLL_FINAL_1));
    } 
}                                      /* end frmr_rsp_rcvd;          */
test_rsp_rcvd(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  test_rsp_rcvd                                        */
/*                                                                    */
/* descriptive name:  test response received                          */
/*                                                                    */
/* function:  verifies the test pattern received in the test          */
/*            response.                                               */
/*                                                                    */
/* input:  received buffer pointer                                    */
/*                                                                    */
/* output:  good or bad test response indication.                     */
/*                                                                    */
/*** end of specifications ********************************************/

{
  int      index,test_not_expected;
  u_char   test_data_index;
  u_char   test_data_match;
  int      if_size;
 

  if (p->debug)
    printf("=============>test_rsp_rcvd\n");
 
  if (p->debug)
    printf("test_ir=%d\n", p->sta_ptr->test_ir_pend);
 
  if                                   /* a test response was expected
                                          from a previous command sent*/
     (p->sta_ptr->test_ir_pend == TRUE)
    {

      /****************************************************************/
      /* indicate that the test response was expected.                */
      /****************************************************************/

      test_not_expected = FALSE;

      /****************************************************************/
      /* pre-set the psb operation result = "successful" (NORMAL      */
      /* case).                                                       */
      /****************************************************************/

      p->operation_result = DLC_SUCCESS;

      /****************************************************************/
      /* reset the contiguous command repolls sent ras counter.       */
      /****************************************************************/

      p->sta_ptr->ras_counters.counters.cmd_cont_repolls = 0;

      /****************************************************************/
      /* terminate timer t1                                           */
      /****************************************************************/

      p->station_list[p->stano].t1_ctr = -1;
      p->station_list[p->stano].t1_ena = FALSE;
      /****************************************************************/
      /* reset inactivity pending flags to indicate packet received   */
      /****************************************************************/
      p->sta_ptr->inact_pend = 0;
 
      if                               /* t3 timer state = inactivity */
         (p->sta_ptr->t3_state == T3_INACT)
        {

          /************************************************************/
          /* initiate inactivity timer (t3).                          */
          /************************************************************/

          p->station_list[p->stano].t3_ctr = p->sta_ptr->inact_to_val;
          p->station_list[p->stano].t3_ena = TRUE;
        } 
 
      if                               /* test was received with final
                                          bit on                      */
         (p->rcv_data.ctl1 == TEST)
        {

          /************************************************************/
          /* receive is valid, call the check inactivity routine.     */
          /************************************************************/

          valid_rcv(p);
 
          if                           /* maximum i-field size is     
                                          greater than 256            */
             (p->sta_ptr->ls_profile.maxif > 256)
            {

              /********************************************************/
              /* set the i-field size to 256.                         */
              /********************************************************/

              if_size = 256;
            } 
 
          else                         /* the max i-field is less than
                                          or equal to 256.            */
            {

              /********************************************************/
              /* set the i-field size to the current max i-field size.*/
              /********************************************************/

              if_size = p->sta_ptr->ls_profile.maxif;
            } 
 
          if                           /* an i-field of full length   
                                          was received (if_size bytes 
                                          of                          */

          /************************************************************/
          /* 0x00 to 0xff plus the 3-byte test header.                */
          /************************************************************/

             (p->lpdu_length == if_size+3)
            {

              /********************************************************/
              /* get addressability to the i-field.                   */
              /********************************************************/
/* <<< feature CDLI >>> */
#ifndef TRLORFDDI
              p->i.i_field_ptr = &(p->d.rcv_data->ctl2);/* un_info    */
#endif /* not TRLORFDDI */
#ifdef TRLORFDDI
	      p->i.i_field_ptr = &(p->d.rcv_data->ri_field[0]) +
					     p->common_cb.ri_length + 3;
#endif /* TRLORFDDI */
/* <<< end feature CDLI >>> */

              /********************************************************/
              /* pre-set the data match indicator.                    */
              /********************************************************/

              test_data_match = TRUE;

              /********************************************************/
              /* pre-set the data match index to 0.                   */
              /********************************************************/

              test_data_index = 0;

              /********************************************************/
              /* compare the i-field to the x'00' to x'ff' repeating  */
              /* pattern up to the i-field size. do scan the receive  */
              /* i-field.                                             */
              /********************************************************/
 

              for (index = 0; index <= (if_size-1); index++)
                {

                  /****************************************************/
                  /* while the data matches.                          */
                  /****************************************************/
 

                  if (test_data_match != TRUE)
                    break;
 
                  if                   /* the data received equals the
                                          index value.                */
                     (*(p->i.i_field_ptr)++ == test_data_index)

                    /**************************************************/
                    /* increment the data match index modulo 256.     */
                    /**************************************************/

                    {
                      test_data_index = (test_data_index+1)%256;
                    } 
 
                  else                 /* indicate that the match     
                                          failed.                     */
                    {
                      test_data_match = FALSE;
                    } 
                }                      /* end do index;               */
 
              if (p->debug)
                printf("test_data_match=%d\n", test_data_match);
 
              if                       /* bad data compare            */
                 (test_data_match == FALSE)
                {
 
                  if                   /* invalid frames received     
                                          counter not at maximum value*/
                     (p->sta_ptr->ras_counters.counters.inv_pkt_rec !=
                     MINUS_ONE)

                    /**************************************************/
                    /* increment count of invalid frames received     */
                    /**************************************************/

                    ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

                  /****************************************************/
                  /* call error log - invalid test response i-field   */
                  /* received.                                        */
                  /****************************************************/

                  lanerrlg(p, ERRID_LAN0014, NON_ALERT, TEMP_ERR, 0, 
                     FILEN, LINEN);

                  /****************************************************/
                  /* set the psb operation result = "bad data compare"
                  .                                                   */
                  /****************************************************/

                  p->operation_result = DLC_BAD_DATA;
                } 
            } 
 
          else                         /* i-field too short or too    
                                          long.                       */
            {
 
              if                       /* invalid frames received     
                                          counter not at maximum value*/
                 (p->sta_ptr->ras_counters.counters.inv_pkt_rec != 
                 MINUS_ONE)

                /******************************************************/
                /* increment count of invalid frames received         */
                /******************************************************/

                ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

              /********************************************************/
              /* call error log - invalid test response i-field       */
              /* received.                                            */
              /********************************************************/

              lanerrlg(p, ERRID_LAN0014, NON_ALERT, TEMP_ERR, 0, 
                 FILEN, LINEN);
 
              if                       /* there is no i-field attached*/
                 (p->lpdu_length == 3)
                {

                  /****************************************************/
                  /* set the psb operation result = "no remote bufferi
                  ng".                                                */
                  /****************************************************/

                  p->operation_result = DLC_NO_RBUF;
                } 
 
              else                     /* there is an i-field, but    
                                          it's too short or too long. */
                {

                  /****************************************************/
                  /* set the psb operation result = "bad data compare"
                  .                                                   */
                  /****************************************************/

                  p->operation_result = DLC_BAD_DATA;
                } 
            } 
        } 
 
      else                             /* invalid response to a test  
                                          command.                    */
        {
 
          if                           /* invalid frames received     
                                          counter not at maximum value*/
             (p->sta_ptr->ras_counters.counters.inv_pkt_rec != 
             MINUS_ONE)

            /**********************************************************/
            /* increment count of invalid frames received             */
            /**********************************************************/

            ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

          /************************************************************/
          /* call error log - invalid test response received.         */
          /************************************************************/

          lanerrlg(p, ERRID_LAN0013, NON_ALERT, TEMP_ERR, 0, FILEN, 
             LINEN);

          /************************************************************/
          /* set the psb operation result = "invalid control received"
          .                                                           */
          /************************************************************/

          p->operation_result = DLC_PROT_ERR;
        } 

      /****************************************************************/
      /* call test complete with test response buffer for result no   */
      /* extra result indicators.                                     */
      /****************************************************************/

      test_completion(p, 0);
    } 
 
  else                                 /* no test response was        
                                          expected.                   */
    {

      /****************************************************************/
      /* indicate that the test response was not expected.            */
      /****************************************************************/

      test_not_expected = TRUE;
 
      if                               /* invalid frames received     
                                          counter not at maximum value*/
         (p->sta_ptr->ras_counters.counters.inv_pkt_rec != MINUS_ONE)

        /**************************************************************/
        /* increment count of invalid frames received                 */
        /**************************************************************/

        ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;

      /****************************************************************/
      /* call error log - invalid state to receive test response.     */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN0024, NON_ALERT, TEMP_ERR, 0, FILEN, LINEN)
         ;
    } 
 
  if (p->debug)
    printf("test_not=%d\n", test_not_expected);

  /********************************************************************/
  /* free buffer containing test response packet                      */
  /********************************************************************/

  lanfree(p, p->m);
  return (test_not_expected);
}                                      /* end test_rsp_rcvd;          */
test_completion(p,or_result)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  test_completion                                      */
/*                                                                    */
/* descriptive name:  test complete                                   */
/*                                                                    */
/* function:  creates the psb to indicate test complete to            */
/*            the user                                                */
/*                                                                    */
/* input:  result mask to "or" with the "test completion" indicator.  */
/*                                                                    */
/* output:  "test complete" enqueued to the user                      */
/*                                                                    */
/*** end of specifications ********************************************/

  register int or_result;              /* results to "or" with test   
                                          complete                    */
{
 
  if (p->debug)
    printf("test_complete %d\n", or_result);

  /********************************************************************/
  /* set the test command state = reset.                              */
  /********************************************************************/

  p->sta_ptr->ts = TEST_RESET;
  p->sta_ptr->test_ir_pend = TEST_RESET;
 
  if                                   /* the test command failed     */
     (p->operation_result != DLC_SUCCESS)
    {
 
      if                               /* test command failures       
                                          counter not at maximum value*/
         (p->sta_ptr->ras_counters.counters.test_cmds_fail != 
         MINUS_ONE)

        /**************************************************************/
        /* increment count of test command failures                   */
        /**************************************************************/

        ++p->sta_ptr->ras_counters.counters.test_cmds_fail;
    } 

  /********************************************************************/
  /* call the short psb result generator routine.                     */
  /********************************************************************/

  lansrslt(p, p->operation_result, (DLC_TEST_RES|or_result), 
     p->sap_ptr->sap_profile.user_sap_corr, 
     p->sta_ptr->ls_profile.user_ls_corr);
}                                      /* end test_completion;        */
valid_rcv(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  valid_rcv                                            */
/*                                                                    */
/* descriptive name:  valid receive                                   */
/*                                                                    */
/* function:  resets the inactivity without completion indication     */
/*            and send psb to the user if required.                   */
/*                                                                    */
/* input:  none                                                       */
/*                                                                    */
/* output:  "inactivity ended" enqueued to the user                   */
/*                                                                    */
/*** end of specifications ********************************************/

{
#define  NORMAL 0x00
 
  if (p->debug)
    printf("===========>valid_rcv %d\n", p->sta_ptr->ignore);
 
  if                                   /* inactivity without          
                                          termination is indicated    */
     (p->sta_ptr->inact_without_pend == TRUE)
    {

      /****************************************************************/
      /* reset the inactivity without termination indicator.          */
      /****************************************************************/

      p->sta_ptr->inact_without_pend = FALSE;

      /****************************************************************/
      /* call error log - inactivity has ended (info only).           */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN0102, NON_ALERT, INFO_ERR, 0, FILEN, LINEN)
         ;

      /****************************************************************/
      /* call the short result generator routine.                     */
      /****************************************************************/

      lansrslt(p, NORMAL, DLC_IEND_RES, 
         p->sap_ptr->sap_profile.user_sap_corr, 
         p->sta_ptr->ls_profile.user_ls_corr);
    } 
}                                      /* end valid_rcv;              */
rcv_i_frame(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  rcv_i_frame                                          */
/*                                                                    */
/* descriptive name:  receive i frame                                 */
/*                                                                    */
/* function:  builds result buffer to pass received data to either    */
/*            the user's receive ring queue or sna path control's     */
/*            luxpcrd run time entry.                                 */
/*                                                                    */
/* input:  received buffer pointer                                    */
/*                                                                    */
/* output:  received data passed to the user.                         */
/*                                                                    */
/*** end of specifications ********************************************/

{
  struct dlc_io_ext dlc_io_ext;
  struct dlc_chan *c_ptr;
 

  if (p->debug)
    printf("==============>rcv_i_frame\n");
 
  if                                   /* total data frames received  
                                          counter not at maximum value*/
     (p->sta_ptr->ras_counters.counters.data_pkt_rec != MINUS_ONE)

    /******************************************************************/
    /* increment count of total data frames received                  */
    /******************************************************************/

    ++p->sta_ptr->ras_counters.counters.data_pkt_rec;

  /********************************************************************/
  /* build the "Normal Data" result buffer.                           */
  /********************************************************************/

  dlc_io_ext.flags = DLC_INFO;
  dlc_io_ext.sap_corr = p->sap_ptr->sap_profile.user_sap_corr;
  dlc_io_ext.ls_corr = p->sta_ptr->ls_profile.user_ls_corr;
/* <<< defect 129185 >>> */
#ifndef TRLORFDDI
  dlc_io_ext.dlh_len = NORM_HDR_LENGTH;
#endif /* not TRLORFDDI */
#ifdef TRLORFDDI
  dlc_io_ext.dlh_len = NORM_HDR_LENGTH + (p->rcv_data.ri_field[0]&0x1f);
#endif /* TRLORFDDI */
  p->m->m_data += dlc_io_ext.dlh_len;
/* <<< end defect 129185 >>> */
  p->m->m_len = p->lpdu_length-4;
 
  if                                   /* overflow occurred           */
     (p->m->m_len > p->sta_ptr->ls_profile.maxif)
    {

      /****************************************************************/
      /* set the result data length to the max i-field length.        */
      /****************************************************************/

      p->m->m_len = p->sta_ptr->ls_profile.maxif;

      /****************************************************************/
      /* set the overflow result flag.                                */
      /****************************************************************/

      dlc_io_ext.flags |= DLC_OFLO;
    } 
  c_ptr = p->sap_ptr->user_sap_channel;
  TRACE2(p, "RxIb", p->m);
  p->rc = (*c_ptr->rcvi_fa)(p->m, &dlc_io_ext, c_ptr);
  TRACE2(p, "RxIe", p->rc);
 
  switch (p->rc)
    {
      case  DLC_FUNC_OK :              /* normal case                 */
        break;
      case  DLC_FUNC_BUSY :
/* LEHb defect 43788 */
				       /* save the iframe buffer address
					  and its extension           */
	p->sta_ptr->retry_rcvi_buf = p->m;
	bcopy (&(dlc_io_ext),
	       &(p->sta_ptr->retry_rcvi_ext.sap_corr),
	       sizeof(p->dlc_io_ext));
	enter_local_busy(p);
	break;
      case  DLC_FUNC_RETRY :
/* delete 1 line */
				       /* save the iframe buffer address
					  and its extension           */
	p->sta_ptr->retry_rcvi_buf = p->m;
	bcopy (&dlc_io_ext, &p->sta_ptr->retry_rcvi_ext,
						    sizeof(dlc_io_ext));
				       /* a wakeup is needed */
	p->station_list[p->stano].wakeup_needed = TRUE;
				       /* enter local busy mode */
	enter_local_busy(p);
/* LEHe */
	break;
      default  :
        lanerrlg(p, ERRID_LAN8083, NON_ALERT, PERM_STA_ERR, 0, FILEN, 
           LINEN);
        shutdown(p, HARD);

        /**************************************************************/
        /* panic("lansta2: Normal Data routine failed");              */
        /**************************************************************/

    } 
}                                      /* end rcv_i_frame;            */
inv_lpdu_rcvd(p,reason_code,pf_bit)
  register struct port_dcl *p;
  register u_char reason_code;         /* possible frmr reason code   */
  register u_char pf_bit;
{
 
  if                                   /* invalid frames received     
                                          counter not at maximum value*/
     (p->sta_ptr->ras_counters.counters.inv_pkt_rec != MINUS_ONE)

    /******************************************************************/
    /* increment count of invalid frames received                     */
    /******************************************************************/

    ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;
 
  if                                   /* in abme or pending abme     
                                          modes                       */
     ((p->sta_ptr->ls == LS_ABME) || (p->sta_ptr->ls == LS_ABME_PEND))
    {
 
      if                               /* a command was received with 
                                          the poll bit set            */
         ((TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE) && (pf_bit == 
         TRUE))
        {

          /************************************************************/
          /* call the send frmr routine with final bit set on.        */
          /************************************************************/

          send_frmr(p, reason_code, FRMR_PF_ON);
        } 
 
      else                             /* not a command poll received.*/
        {

          /************************************************************/
          /* call the send frmr routine with final bit set off.       */
          /************************************************************/

          send_frmr(p, reason_code, FRMR_PF_OFF);
        } 
    } 
 
  else                                 /* not in abme or abme pending 
                                          mode.                       */
    {

      /****************************************************************/
      /* ignore the receive buffer.                                   */
      /****************************************************************/

      p->sta_ptr->ignore = TRUE;
    } 
}                                      /* end inv_lpdu_rcvd;          */
send_frmr(p,fr_reason,fr_pf)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  send_frmr                                            */
/*                                                                    */
/* descriptive name:  send frmr response frame                        */
/*                                                                    */
/* function:  transmits a frmr response frame followed by a disc      */
/*            command.  going to discontacting state.                 */
/*                                                                    */
/* input:  received buffer pointer.                                   */
/*         variable vwxyz set for frmr response                       */
/*                                                                    */
/* output:  frmr response and disc command queued for transmit.       */
/*                                                                    */
/*** end of specifications ********************************************/

  register u_char fr_reason;
  register u_char fr_pf;
{
  u_char   temp_sap;
  char     temp_addr[6];
 

  if (p->debug)
    printf("============send_frmr\n");

  /********************************************************************/
  /* build and send the frmr response in the same buffer that the     */
  /* invalid receive occurred, followed by a disc command. call error */
  /* log - unknown command received.                                  */
  /********************************************************************/
 

  switch (fr_reason)
    {
      case (FRMR_INV_CTL_RCVD) :
/* defect 86180 */
        lanerrlg(p, ERRID_LAN_ALERT4, ALERT, PERM_STA_ERR, DLC_PROT_ERR,
           FILEN, LINEN);
        break;
      case (FRMR_INV_IFIELD_RCVD) :
        lanerrlg(p, ERRID_LAN_ALERT5, ALERT, PERM_STA_ERR, DLC_PROT_ERR,
           FILEN, LINEN);
        break;
      case (FRMR_INV_SEQ_NUM) :
        lanerrlg(p, ERRID_LAN_ALERT6, ALERT, PERM_STA_ERR, DLC_PROT_ERR,
           FILEN, LINEN);
        break;
/* end defect 86180 */
    }                                  /* end switch;                 */

  /********************************************************************/
  /* indicate that the receive buffer is not to be returned to the    */
  /* pool. call protocol specific send frame response                 */
  /********************************************************************/

  g_send_frmr(p, fr_reason, fr_pf);
  p->sta_ptr->ignore = FALSE;

  /********************************************************************/
  /* set the llc closing reason code = protocol error.                */
  /********************************************************************/

  p->sta_ptr->closing_reason = DLC_PROT_ERR;

  /********************************************************************/
  /* shutdown the logical link by sending a disc command.             */
  /********************************************************************/

  shutdown(p, FRMR_SENT);
}                                      /* end send_frmr;              */
invalid_rcv_adm(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  invalid_rcv_adm                                      */
/*                                                                    */
/* descriptive name:  invalid receive in disconnected mode            */
/*                                                                    */
/* function:  called case receive a i, rr, rnr or rej frame while     */
/*            in adm mode. determines if a dm response needs to       */
/*            be transmitted.                                         */
/*                                                                    */
/* input:  received buffer pointer                                    */
/*                                                                    */
/* output:  dm response transmitted if required.                      */
/*                                                                    */
/*** end of specifications ********************************************/

{
 
  if                                   /* invalid frames received     
                                          counter not at maximum value*/
     (p->sta_ptr->ras_counters.counters.inv_pkt_rec != MINUS_ONE)

    /******************************************************************/
    /* increment count of invalid frames received                     */
    /******************************************************************/

    ++p->sta_ptr->ras_counters.counters.inv_pkt_rec;
 
  if                                   /* frame received is a command 
                                          with poll bit set           */
     ((TSTBIT(p->rcv_data.rsap, RESPONSE) == FALSE) && (TSTBIT
     (p->rcv_data.ctl2, POLL_FINAL_2) == TRUE))
    {

      /****************************************************************/
      /* set the llc closing reason code = protocol error.            */
      /****************************************************************/

      p->sta_ptr->closing_reason = DLC_PROT_ERR;

      /****************************************************************/
      /* send a dm response with the final bit set.                   */
      /****************************************************************/

      tx_disc_rsp(p, DM);
    } 
/* defect 142033 */
  else /* no response will be sent */
    {
      /* insure that the received mbuf is returned to the buffer pool */
      p->sta_ptr->ignore = TRUE;
    }
/* end defect 142033 */
}                                      /* end invalid_rcv_adm;        */
update_va(p)
  register struct port_dcl *p;
{

  /********************************************************************/
  /* process va based on nr and vs.                                   */
  /********************************************************************/
 

  if                                   /* there was nothing sent to   
                                          acknowledge                 */
     (p->sta_ptr->va == p->sta_ptr->vs)/* nr already checked for range*/

    /******************************************************************/
    /* response was received, so assume line is active.               */
    /******************************************************************/

    {
 
      if                               /* t3 timer state = inactivity */
         (p->sta_ptr->t3_state == T3_INACT)
        {

          /************************************************************/
          /* restart inactivity timer (t3).                           */
          /************************************************************/

          p->station_list[p->stano].t3_ctr = p->sta_ptr->inact_to_val;
          p->station_list[p->stano].t3_ena = TRUE;
        } 
    } 
 
  else                                 /* check other combinations    */
    {
 
      if                               /* some but not all i lpdus    
                                          acknowledged                */
         ((p->sta_ptr->va != p->sta_ptr->rcvd_nr) && 
         (p->sta_ptr->rcvd_nr != p->sta_ptr->vs))

        /**************************************************************/
        /* adjust va and reset counters                               */
        /**************************************************************/

        {
#ifdef   TRLORFDDI
          adjust_window(p);
#endif

          /************************************************************/
          /* set va to received nr count.                             */
          /************************************************************/

          p->sta_ptr->va = p->sta_ptr->rcvd_nr;

          /************************************************************/
          /* return all acknowledged transmit buffers to the pool.    */
          /* note - must run after updating va                        */
          /************************************************************/

          free_tx_acked(p);

          /************************************************************/
          /* restart repoll timer, t1                                 */
          /************************************************************/

          p->station_list[p->stano].t1_ctr = p->sta_ptr->resp_to_val;
          p->station_list[p->stano].t1_ena = TRUE;

          /************************************************************/
          /* reset send retry counter, is_ct                          */
          /************************************************************/

          p->sta_ptr->is_ct = p->sta_ptr->ls_profile.max_repoll;
        } 
 
      else                             /* check other combinations    */
        {
 
          if                           /* all outstanding i lpdus     
                                          acknowledged                */
             ((p->sta_ptr->va != p->sta_ptr->rcvd_nr) && 
             (p->sta_ptr->rcvd_nr == p->sta_ptr->vs))

            /**********************************************************/
            /* set up for no more activity                            */
            /**********************************************************/

            {
#ifdef   TRLORFDDI
              adjust_window(p);
#endif

              /********************************************************/
              /* update va to current received count                  */
              /********************************************************/

              p->sta_ptr->va = p->sta_ptr->rcvd_nr;

              /********************************************************/
              /* return all acknowledged transmit buffers to the pool.*/
              /* note - must run after updating va                    */
              /********************************************************/

              free_tx_acked(p);

              /********************************************************/
              /* stop repoll timer, t1                                */
              /********************************************************/

              p->station_list[p->stano].t1_ctr = -1;
              p->station_list[p->stano].t1_ena = FALSE;
	      /********************************************************/
	      /* reset inact pending flags - indicate packet received */
	      /********************************************************/
              p->sta_ptr->inact_pend = 0;
 
              if                       /* t3 timer state = inactivity */
                 (p->sta_ptr->t3_state == T3_INACT)
                {

                  /****************************************************/
                  /* start inactivity timer (t3).                     */
                  /****************************************************/

                  p->station_list[p->stano].t3_ctr = 
                     p->sta_ptr->inact_to_val;
                  p->station_list[p->stano].t3_ena = TRUE;
                } 

              /********************************************************/
              /* reset send retry counter, is_ct                      */
              /********************************************************/

              p->sta_ptr->is_ct = p->sta_ptr->ls_profile.max_repoll;
            } 
 
          else                         /* no i lpdus acked and t1     
                                          running or anything else    */
            {

              /********************************************************/
              /* fall through                                         */
              /********************************************************/

            } 
        } 
    } 
}                                      /* end update_va;              */
free_tx_acked(p)
  register struct port_dcl *p;
{
 
  while                                /* transmit queue output index 
                                          does not equal the va index.*/
     (p->sta_ptr->txq_output != (p->sta_ptr->va/2))
    {
      lanfree(p, p->sta_ptr->transmit_queue[p->sta_ptr->txq_output].
         buf);

      /****************************************************************/
      /* increment the transmit queue output index by 1, modulo the   */
      /* queue size.                                                  */
      /****************************************************************/

      p->sta_ptr->txq_output = (p->sta_ptr->txq_output+1)%TXQ_SIZE;
    }                                  /* end while;                  */

  /********************************************************************/
  /* if the head code is sleeping wake it up                          */
  /********************************************************************/
 

  if (p->sap_ptr->user_sap_channel->writesleep != EVENT_NULL)
    e_wakeup(&p->sap_ptr->user_sap_channel->writesleep);
}                                      /* end free_tx_acked;          */
update_va_chkpt(p)
  register struct port_dcl *p;
{
 
  if (p->debug)
    printf("============>update_va_chkptr\n");

  /********************************************************************/
  /* process va based on nr and vs.                                   */
  /********************************************************************/
 

  if                                   /* all outstanding i lpdus     
                                          acked                       */
     ((p->sta_ptr->va != p->sta_ptr->rcvd_nr) && (p->sta_ptr->rcvd_nr 
     == p->sta_ptr->vs))
    {
#ifdef   TRLORFDDI
      adjust_window(p);
#endif

      /****************************************************************/
      /* update va to current received count                          */
      /****************************************************************/

      p->sta_ptr->va = p->sta_ptr->rcvd_nr;

      /****************************************************************/
      /* stop repoll timer                                            */
      /****************************************************************/

      p->station_list[p->stano].t1_ctr = -1;
      p->station_list[p->stano].t1_ena = FALSE;
      /****************************************************************/
      /* reset inactivity pending flags to indicate packet received   */
      /****************************************************************/
      p->sta_ptr->inact_pend = 0;
 
      if                               /* t3 timer state = inactivity */
         (p->sta_ptr->t3_state == T3_INACT)
        {

          /************************************************************/
          /* restart inactivity timer (t3).                           */
          /************************************************************/

          p->station_list[p->stano].t3_ctr = p->sta_ptr->inact_to_val;
          p->station_list[p->stano].t3_ena = TRUE;
        } 

      /****************************************************************/
      /* return all acknowledged transmit buffers to the pool. note - */
      /* must run after updating va                                   */
      /****************************************************************/

      free_tx_acked(p);

      /****************************************************************/
      /* reset send retry counter, is_ct                              */
      /****************************************************************/

      p->sta_ptr->is_ct = p->sta_ptr->ls_profile.max_repoll;
    } 
 
  else
 
    if                                 /* some but not all i lpdus    
                                          acked                       */

       ((p->sta_ptr->va != p->sta_ptr->rcvd_nr) && 
       (p->sta_ptr->rcvd_nr != p->sta_ptr->vs))
      {

        /**************************************************************/
        /* restart repoll timer                                       */
        /**************************************************************/

        p->station_list[p->stano].t1_ctr = p->sta_ptr->resp_to_val;
        p->station_list[p->stano].t1_ena = TRUE;

        /**************************************************************/
        /* update vs to current received count                        */
        /**************************************************************/

        p->sta_ptr->vs = p->sta_ptr->rcvd_nr;

        /**************************************************************/
        /* update va to current received count                        */
        /**************************************************************/

        p->sta_ptr->va = p->sta_ptr->rcvd_nr;

        /**************************************************************/
        /* return all acknowledged transmit buffers to the pool. note */
        /* - must run after updating va                               */
        /**************************************************************/

        free_tx_acked(p);

        /**************************************************************/
        /* reset and decrement send retry counter, is_ct              */
        /**************************************************************/

        p->sta_ptr->is_ct = p->sta_ptr->ls_profile.max_repoll-1;
      } 
 
    else
 
      if                               /* no transmitted i-lpdus are  
                                          being acknowledged          */

         ((p->sta_ptr->va == p->sta_ptr->rcvd_nr) && 
         (p->sta_ptr->rcvd_nr != p->sta_ptr->vs))
        {

          /************************************************************/
          /* restart repoll timer, t1                                 */
          /************************************************************/

          p->station_list[p->stano].t1_ctr = p->sta_ptr->resp_to_val;
          p->station_list[p->stano].t1_ena = TRUE;

          /************************************************************/
          /* update vs to received nr count for retransmission.       */
          /************************************************************/

          p->sta_ptr->vs = p->sta_ptr->rcvd_nr;
 
          if                           /* control received was not a  
                                          rnr                         */
             (p->rcv_data.ctl1 != RNR)
            {

              /********************************************************/
              /* decrement send retry counter, is_ct                  */
              /********************************************************/

              p->sta_ptr->is_ct = (p->sta_ptr->is_ct-1);
            } 
        } 
 
      else
 
        if                             /* no i-lpdus are outstanding  
                                          that require                
                                          acknowledgement.            */

           ((p->sta_ptr->va == p->sta_ptr->rcvd_nr) && 
           (p->sta_ptr->rcvd_nr == p->sta_ptr->vs))
          {

            /**********************************************************/
            /* stop repoll timer                                      */
            /**********************************************************/

            p->station_list[p->stano].t1_ctr = -1;
            p->station_list[p->stano].t1_ena = FALSE;
	    /**********************************************************/
	    /* reset inact pending flags to indicate packet received  */
	    /**********************************************************/
            p->sta_ptr->inact_pend = 0;
 
            if                         /* t3 timer state = inactivity */
               (p->sta_ptr->t3_state == T3_INACT)
              {

                /******************************************************/
                /* start inactivity timer (t3).                       */
                /******************************************************/

                p->station_list[p->stano].t3_ctr = 
                   p->sta_ptr->inact_to_val;
                p->station_list[p->stano].t3_ena = TRUE;
              } 
          } 
 
        else                           /* error in state variables.   */
          {
            lanerrlg(p, ERRID_LAN8085, NON_ALERT, PERM_STA_ERR, 0, 
               FILEN, LINEN);
            shutdown(p, HARD);

            /**********************************************************/
            /* panic("lansta2 3180");                                 */
            /**********************************************************/

          } 

  /********************************************************************/
  /* reset the contiguous command repolls sent ras counter.           */
  /********************************************************************/

  p->sta_ptr->ras_counters.counters.cmd_cont_repolls = 0;
}                                      /* end update_va_chkpt;        */
send_ack(p)
  register struct port_dcl *p;
{
 
  if (p->debug)
    printf("=================>send_ack\n");

  /********************************************************************/
  /* control receive retry counter and send rr if required decrement  */
  /* ack delay counter, ir_ct                                         */
  /********************************************************************/

  p->sta_ptr->ir_ct = (p->sta_ptr->ir_ct-1);
 
  if                                   /* acknowledgement not required*/
     ((int)p->sta_ptr->ir_ct > 0)
    {

      /****************************************************************/
      /* start ack timer, t2, if not already running                  */
      /****************************************************************/
 

      if                               /* ack timer, t2, is not       
                                          already running             */
         (p->station_list[p->stano].t2_ena == FALSE)

        /**************************************************************/
        /* start the timer                                            */
        /**************************************************************/

        {

          /************************************************************/
          /* start ack timer, t2                                      */
          /************************************************************/

          p->station_list[p->stano].t2_ctr = p->sta_ptr->ack_to_val;
          p->station_list[p->stano].t2_ena = TRUE;
        } 
    } 
 
  else                                 /* acknowledgement required    */
    {

      /****************************************************************/
      /* send a rr response, p/f = 0                                  */
      /****************************************************************/

      tx_rr_rsp(p, PF2_OFF_MASK&p->sta_ptr->vr);

      /****************************************************************/
      /* reset ack counter                                            */
      /****************************************************************/

      p->sta_ptr->ir_ct = p->sta_ptr->ls_profile.rcv_wind;

      /****************************************************************/
      /* stop timer t2                                                */
      /****************************************************************/

      p->station_list[p->stano].t2_ctr = -1;
      p->station_list[p->stano].t2_ena = FALSE;
    } 
}                                      /* end send_ack;               */
adjust_window(p)
  register struct port_dcl *p;
{
  int      quotient;                   /* quotient result             */
  int      remainder;                  /* remainder result            */
 

  if (p->debug)
    printf("=================>adjust_window\n");
 
  if                                   /* dynamic window algorithm    
                                          active                      */
     (p->sta_ptr->ww != p->sta_ptr->ls_profile.xmit_wind)
    {

      /****************************************************************/
      /* update count of acknowledgements                             */
      /****************************************************************/

      p->sta_ptr->ia_ct = p->sta_ptr->ia_ct+((p->sta_ptr->rcvd_nr-
         p->sta_ptr->va)%128);
 
      if                               /* acknowledgements greater    
                                          than count of sequential    */

      /****************************************************************/
      /* acknowledgement since last increment of working window       */
      /****************************************************************/

         (p->sta_ptr->ia_ct >= p->sta_ptr->nw)
        {

          /************************************************************/
          /* get quotient of ia_ct/nw                                 */
          /************************************************************/

          quotient = p->sta_ptr->ia_ct/p->sta_ptr->nw;

          /************************************************************/
          /* get remainder of ia_ct/nw                                */
          /************************************************************/

          remainder = (p->sta_ptr->ia_ct%p->sta_ptr->nw);

          /************************************************************/
          /* update acknowledgement count                             */
          /************************************************************/

          p->sta_ptr->ia_ct = remainder;

          /************************************************************/
          /* update working window count                              */
          /************************************************************/

          p->sta_ptr->ww = ((p->sta_ptr->ww+quotient) < 
             p->sta_ptr->txw_ct)?(p->sta_ptr->ww+quotient):
                p->sta_ptr->txw_ct;
        } 
    } 
}                                      /* end adjust_window;          */
