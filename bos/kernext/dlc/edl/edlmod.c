static char sccsid[] = "@(#)98  1.33  src/bos/kernext/dlc/edl/edlmod.c, sysxdlce, bos411, 9428A410j 6/2/94 10:20:16";

/**********************************************************************
 * COMPONENT_NAME: (SYSXDLCE) Standard Ethernet Data Link Control
 *
 * FUNCTIONS: edlmod.c
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 **********************************************************************/

#ifdef   EDL

/**********************************************************************/
/*  This module contains Standard Ethernet specific routines only.    */
/**********************************************************************/

/* <<< feature CDLI >>> */
#include <fcntl.h>
#include <net/spl.h>
#include <sys/types.h>
#include <sys/mbuf.h>
#include <sys/file.h>
#include <sys/malloc.h>
#include <sys/errno.h>
#include <sys/gdlextcb.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include "dlcadd.h"
#include "lancomcb.h"
#include "lanstlst.h"
#include "lansplst.h"
#include "lanrstat.h"
#include "lanport.h"
/* <<< end feature CDLI >>> */

int search_cache();

g_build_iframe(p)
  register struct port_dcl *p;
{
  struct mbuf *temp_buf_ptr;

  /********************************************************************/
  /* copy the small mbufs and double link the cluster so that there is*/
  /* no conflict with the device on m_free and retransmits            */
  /********************************************************************/

/* <<< defect 129276 >>> */
  /* set the current buffer length to maximum so that the m_copym will
   * not pull up "short" data into the datalink header mbuf.
   */
  p->m->m_len = MHLEN;
/* <<< end defect 129276 >>> */

  /* defect 109030 */
  p->m = (struct mbuf *)m_copym(p->m, 0, M_COPYALL, M_WAIT);
  /* end defect 109030 */

  if                                   /* the copy was successful     */
     (p->m != 0) {

    /******************************************************************/
    /* get addressability to the data area                            */
    /******************************************************************/

    p->d.data_ptr = MTOD(p->m, caddr_t);

    /******************************************************************/
    /* set the lpdu length to i-field length plus header (4-bytes).   */
    /******************************************************************/

    temp_buf_ptr = p->m->m_next;
    p->sta_ptr->lpdu_length = temp_buf_ptr->m_len+4;

    /******************************************************************/
    /* copy the prebuilt packet header from the station control block */
    /* into the buffer.                                               */
    /******************************************************************/

    bcopy(p->sta_ptr->raddr, p->d.send_data->raddr, NORM_HDR_LENGTH);
    if                                 /* the users data length plus
                                          full header is less than    */

      /****************************************************************/
      /* a minimum packet length.                                     */
      /****************************************************************/

       (p->m->m_next->m_len < (MIN_PACKET-NORM_HDR_LENGTH)) {

      /****************************************************************/
      /* set the users mbuf length to the minimum allowed.            */
      /****************************************************************/

      p->m->m_next->m_len = (MIN_PACKET-NORM_HDR_LENGTH);
    } 

      /****************************************************************/
      /* set the data link header's mbuf length                       */
      /****************************************************************/

    p->m->m_len = NORM_HDR_LENGTH;
  } 
  else                                 /* the m_copy was not
                                          successful                  */
    {

    /******************************************************************/
    /* permanent station error - out of station resources             */
    /******************************************************************/

    lanerrlg(p, ERRID_LAN8010, NON_ALERT, PERM_STA_ERR, DLC_SYS_ERR, 
       FILEN, LINEN);
  } 
}                                      /* end build_iframe;           */
tx_sta_cmd(p, in_ctl1, in_ctl2, format)
  register struct port_dcl *p;
  register u_char in_ctl1;
  register u_char in_ctl2;
  register u_char format;
{
  TRACE1(p, "TXSb");
  if (p->debug)
    printf("g_tx_sta_cmd %x\n", p->sta_ptr->last_cmd_1);

    /******************************************************************/
    /* save control byte-1 in the last command control sent.          */
    /******************************************************************/

  p->sta_ptr->last_cmd_1 = in_ctl1;
  p->m0 = (struct mbuf *)lanfetch(p);
  if                                   /* a buffer is available       */
     (p->m0 != (struct mbuf *)NO_BUF_AVAIL) {

    /******************************************************************/
    /* get addressability to the data area                            */
    /******************************************************************/

    p->sta_cmd_buf = MTOD(p->m0, struct STA_CMD_BUF *);
    if                                 /* there is only one control
                                          byte to transmit            */
       (format == 1) {

      /****************************************************************/
      /* set the lpdu length to 3.                                    */
      /****************************************************************/

      p->sta_cmd_buf->lpdu_length = 3;
    } 
    else                               /* there are two control bytes
                                          to transmit.                */
      {

      /****************************************************************/
      /* set the lpdu length to 4.                                    */
      /****************************************************************/

      p->sta_cmd_buf->lpdu_length = 4;

      /****************************************************************/
      /* load the 2nd control byte into the command.                  */
      /****************************************************************/

      p->sta_cmd_buf->ctl2 = in_ctl2;
    } 

      /****************************************************************/
      /* build the remainder of the station command.                  */
      /****************************************************************/

    bcopy(p->sta_ptr->raddr, p->sta_cmd_buf->raddr, 6);
    bcopy(p->sta_ptr->laddr, p->sta_cmd_buf->laddr, 6);
    p->sta_cmd_buf->llc_type = SNA_LLC_TYPE;
    p->sta_cmd_buf->lpad = 0;
    p->sta_cmd_buf->rsap = p->sta_ptr->ls_profile.rsap;
    p->sta_cmd_buf->lsap = (p->sap_ptr->sap_profile.local_sap&RESP_OFF
       );
    p->sta_cmd_buf->ctl1 = in_ctl1;
    p->m0->m_len = MIN_PACKET;

/* <<< feature CDLI >>> */
    /******************************************************************/
    /* call the write send command generator, with the station        */
    /* command buffer address.                                        */
    /******************************************************************/

    lanwrtsg(p, p->m0);
/* <<< end feature CDLI >>> */
    if                                 /* command polls counter not at
                                          maximum value               */
       (p->sta_ptr->ras_counters.counters.cmd_polls_sent != DLC_ERR)

      /****************************************************************/
      /* increment count of command polls sent                        */
      /****************************************************************/

      ++p->sta_ptr->ras_counters.counters.cmd_polls_sent;

      /****************************************************************/
      /* restart the t1 repoll timer.                                 */
      /****************************************************************/

    p->station_list[p->stano].t1_ena = TRUE;
    p->station_list[p->stano].t1_ctr = p->sta_ptr->resp_to_val;
  } 
}                                      /* end tx_sta_cmd;             */
tx_rr_rsp(p, in_ctl2)
  register struct port_dcl *p;
  register u_char in_ctl2;
{
  register struct mbuf *temp_buf_ptr;
  register caddr_t temp_data_ptr;
  TRACE1(p, "TXRb");
  if (p->debug)
    printf("tx_rr_rsp\n");

    /******************************************************************/
    /* save any receive buffer pointers.                              */
    /******************************************************************/

  temp_buf_ptr = p->m;
  temp_data_ptr = p->d.data_ptr;

  /********************************************************************/
  /* fetch a block i/o buffer from the pool.                          */
  /********************************************************************/

  p->m = (struct mbuf *)lanfetch(p);
  if                                   /* a buffer is available       */
     (p->m != (struct mbuf *)NO_BUF_AVAIL) {

    /******************************************************************/
    /* get addressability to the data area                            */
    /******************************************************************/

    p->d.data_ptr = MTOD(p->m, caddr_t);

    /******************************************************************/
    /* set the packet length to minimum size                          */
    /******************************************************************/

    p->m->m_len = MIN_PACKET;

    /******************************************************************/
    /* copy the prebuilt packet header from the station control block */
    /* into the buffer.                                               */
    /******************************************************************/

    bcopy(p->sta_ptr->raddr, p->d.send_data->raddr, 6);
    bcopy(p->sta_ptr->laddr, p->d.send_data->laddr, 6);
    p->d.send_data->llc_type = SNA_LLC_TYPE;
    p->d.send_data->lpdu_length = 4;
    p->d.send_data->lpad = 0;
    p->d.send_data->rsap = p->sta_ptr->rsap;
    p->d.send_data->lsap = (p->sta_ptr->lsap|RESP_ON);
    p->d.send_data->ctl1 = RR;
    p->d.send_data->ctl2 = in_ctl2;

/* <<< feature CDLI >>> */
    /******************************************************************/
    /* call the write send command generator, with the buffer         */
    /* address.                                                       */
    /******************************************************************/

    lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
  } 

    /******************************************************************/
    /* restore any receive buffer pointers.                           */
    /******************************************************************/

  p->m = temp_buf_ptr;
  p->d.data_ptr = temp_data_ptr;
}                                      /* end tx_rr_rsp;              */
tx_rnr_rsp(p, in_ctl2)
  register struct port_dcl *p;
  register u_char in_ctl2;
{
  TRACE1(p, "TXNb");
  if (p->debug)
    printf("tx_rnr_rsp\n");
  p->m0 = (struct mbuf *)lanfetch(p);
  if                                   /* a buffer is available       */
     (p->m0 != (struct mbuf *)NO_BUF_AVAIL) {

    /******************************************************************/
    /* get addressability to the data area                            */
    /******************************************************************/

    p->rnr_rsp_buf = MTOD(p->m0, struct STA_CMD_BUF *);

    /******************************************************************/
    /* build the remainder of the station response.                   */
    /******************************************************************/

    bcopy(p->sta_ptr->raddr, p->rnr_rsp_buf->raddr, 6);
    bcopy(p->sta_ptr->laddr, p->rnr_rsp_buf->laddr, 6);
    p->rnr_rsp_buf->llc_type = SNA_LLC_TYPE;
    p->rnr_rsp_buf->lpdu_length = 4;
    p->rnr_rsp_buf->lpad = 0;
    p->rnr_rsp_buf->rsap = p->sta_ptr->ls_profile.rsap;
    p->rnr_rsp_buf->lsap = (p->sap_ptr->sap_profile.local_sap|RESP_ON)
       ;
    p->rnr_rsp_buf->ctl1 = RNR;
    p->rnr_rsp_buf->ctl2 = in_ctl2;
    p->m0->m_len = MIN_PACKET;

/* <<< feature CDLI >>> */
    /******************************************************************/
    /* call the write send command generator, with the station        */
    /* response buffer address.                                       */
    /******************************************************************/

    lanwrtsg(p, p->m0);
/* <<< end feature CDLI >>> */
  } 
}                                      /* end tx_rnr_rsp;             */

tx_buf_rsp(p, in_ctl1, in_ctl2, format)
  register struct port_dcl *p;
  register u_char in_ctl1;
  register u_char in_ctl2;
  register u_char format;              /* format of the frame 1 or 2
                                          control bytes               */
{
  TRACE1(p, "TXBb");
  if (p->debug)
    printf("tx_buf_rsp %x\n", in_ctl1);

    /******************************************************************/
    /* insure that the current buffer is not returned to the pool by  */
    /* dlc.                                                           */
    /******************************************************************/

  p->sta_ptr->ignore = FALSE;
  p->d.data_ptr = MTOD(p->m, caddr_t);

  /********************************************************************/
  /* set the packet length to minimum size                            */
  /********************************************************************/

  p->m->m_len = MIN_PACKET;
  if                                   /* there is only one control
                                          byte to transmit            */
     (format == 1) {

    /******************************************************************/
    /* set the lpdu length to 3.                                      */
    /******************************************************************/

    p->d.send_data->lpdu_length = 3;
  } 
  else                                 /* there are two control bytes
                                          to transmit.                */
    {

    /******************************************************************/
    /* set the lpdu length to 4.                                      */
    /******************************************************************/

    p->d.send_data->lpdu_length = 4;

    /******************************************************************/
    /* load the 2nd control byte into the command.                    */
    /******************************************************************/

    p->d.send_data->ctl2 = in_ctl2;
  } 

    /******************************************************************/
    /* copy the prebuilt packet header from the station control block */
    /* into the buffer.                                               */
    /******************************************************************/

  bcopy(p->sta_ptr->raddr, p->d.send_data->raddr, 6);
  bcopy(p->sta_ptr->laddr, p->d.send_data->laddr, 6);
  p->d.send_data->llc_type = SNA_LLC_TYPE;
  p->d.send_data->lpad = 0;
  p->d.send_data->rsap = p->sta_ptr->rsap;
  p->d.send_data->lsap = (p->sta_ptr->lsap|RESP_ON);
  p->d.send_data->ctl1 = in_ctl1;

/* <<< feature CDLI >>> */
  /********************************************************************/
  /* call the write send command generator, with the buffer address   */
  /********************************************************************/

  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
}                                      /* end tx_buf_rsp;             */

tx_disc_rsp(p, in_ctl1)
  register struct port_dcl *p;
  register u_char in_ctl1;
{
/* <<< feature CDLI >>> */
/* removed disc_check_word >>> */
/* <<< end feature CDLI >>> */
/* <<< defect 129507 >>> */
  ulong_t disc_rsp_pend;
/* <<< end defect 129507 >>> */

  TRACE1(p, "TXDb");
#ifdef   DEBUG
  if (p->debug)
    printf("tx_disc_rsp\n");
#endif

  /********************************************************************/
  /* indicate that the current receive buffer is being used for the   */
  /* response buffer and should not be returned to the pool           */
  /********************************************************************/

  p->sta_ptr->ignore = FALSE;
  p->disc_rsp_buf = MTOD(p->m, struct STA_CMD_BUF *);
  if                                   /* discontacting and sending
                                          response to DISC            */
     ((p->sta_ptr->ls == LS_DISCONTACTING) && ((p->rcv_data.ctl1|
     PF1_MASK) == DISC))
/* <<< defect 129507 >>> */
    {
    /* indicate that a disconnect response is still pending */
    disc_rsp_pend = TRUE;
    }
  else                                 /* not already discontacting
					  the remote                  */
    {
    /* indicate that no disconnect response is pending */
    disc_rsp_pend = FALSE;
/* <<< end defect 129507 >>> */

    /******************************************************************/
    /* stop any reply and acknowledgement timers.                     */
    /******************************************************************/

    p->station_list[p->stano].t1_ctr = -1;
    p->station_list[p->stano].t1_ena = FALSE;
    p->station_list[p->stano].t2_ctr = -1;
    p->station_list[p->stano].t2_ena = FALSE;

    /******************************************************************/
    /* start the disconnect abort timer.                              */
    /******************************************************************/

    p->sta_ptr->t3_state = T3_ABORT;
    p->station_list[p->stano].t3_ctr = p->sta_ptr->force_to_val;
    p->station_list[p->stano].t3_ena = TRUE;

    /******************************************************************/
    /* NOTE: ls set to LS_CLOSED upon send completion. set link state */
    /* (ls) = LS_CLOSE_PEND                                           */
    /******************************************************************/

    p->sta_ptr->ls = LS_CLOSE_PEND;

/* <<< feature CDLI >>> */
/* <<< removed p->sta_ptr->close_pend_non_buf >>> */
/* <<< end feature CDLI >>> */
  }

    /******************************************************************/
    /* build the disconnect response in the same mbuf                 */
    /******************************************************************/

  bcopy(p->sta_ptr->raddr, p->disc_rsp_buf->raddr, 6);
  bcopy(p->sta_ptr->laddr, p->disc_rsp_buf->laddr, 6);
  p->disc_rsp_buf->llc_type = SNA_LLC_TYPE;
  p->disc_rsp_buf->lpdu_length = 3;
  p->disc_rsp_buf->lpad = 0;
  p->disc_rsp_buf->rsap = p->sta_ptr->ls_profile.rsap;
  p->disc_rsp_buf->lsap = (p->sap_ptr->sap_profile.local_sap|RESP_ON);
  p->disc_rsp_buf->ctl1 = in_ctl1;
  p->m->m_len = MIN_PACKET;

/* <<< feature CDLI >>> */
  /********************************************************************/
  /* call the write send command generator with the disconnect        */
  /* response buffer address.                                         */
  /********************************************************************/

/* <<< removed disc_check_word >>> */
  lanwrtsg(p, p->m);

/* <<< defect 129507 >>> */
  if /* no disconnect response is pending at this local station */
     (disc_rsp_pend == FALSE)
    {
      /****************************************************************/
      /* call llc close completion                                    */
      /****************************************************************/

      llc_close_completion(p);
    }
/* <<< end defect 129507 >>> */
/* <<< end feature CDLI >>> */

  TRACE1(p, "TXDe");
}                                      /* end tx_disc_rsp;            */

g_build_xid(p)
  register struct port_dcl *p;
{
  if (p->debug)
    printf("g_build_xid %x\n", p->m);
  p->d.data_ptr = MTOD(p->m, caddr_t);

  /********************************************************************/
  /* set the lpdu length to i-field length plus header (3-bytes).     */
  /********************************************************************/

  p->d.send_data->lpdu_length = (p->m->m_next->m_len+3);
  if (p->m->m_next->m_len < (MIN_PACKET-UN_HDR_LENGTH)) { /* Defect 123702 */

    /******************************************************************/
    /* the packet length is less than minimum packet size. set the    */
    /* packet length to the minimum allowed.                          */
    /******************************************************************/

    p->m->m_next->m_len = (MIN_PACKET-UN_HDR_LENGTH); /* End Defect 123702 */

  } 
  p->m->m_len = UN_HDR_LENGTH;

  /********************************************************************/
  /* load the xid control byte.                                       */
  /********************************************************************/

  p->d.send_data->ctl1 = XID;

  /********************************************************************/
  /* build the remainder of the xid command.                          */
  /********************************************************************/

  bcopy(p->sta_ptr->raddr, p->d.send_data->raddr, 6);
  bcopy(p->sta_ptr->laddr, p->d.send_data->laddr, 6);
  p->d.send_data->llc_type = SNA_LLC_TYPE;
  p->d.send_data->lpad = 0;
  p->d.send_data->rsap = p->sta_ptr->ls_profile.rsap;
  p->d.send_data->lsap = p->sap_ptr->sap_profile.local_sap;

  /********************************************************************/
  /* lsap cmd/rsp bit must be set by caller                           */
  /********************************************************************/

}                                      /* end g_build_xid;            */
g_send_xid_rsp(p)
  register struct port_dcl *p;
{
  if (p->debug)
    printf("g_send_xid_rsp %x\n", p->m);
  p->d.data_ptr = MTOD(p->m, caddr_t);

  /********************************************************************/
  /* set the lsap response indicator in the buffer.                   */
  /********************************************************************/

  p->d.send_data->lsap = (p->d.send_data->lsap|RESP_ON);

  /********************************************************************/
  /* set the final bit to equal the received poll bit setting.        */
  /********************************************************************/

  p->d.send_data->ctl1 = (XID_NO_PF|p->sta_ptr->px);

/* <<< feature CDLI >>> */
  /********************************************************************/
  /* call the write send command generator, with the xid buffer       */
  /* address.                                                         */
  /********************************************************************/

  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
}                                      /* end g_send_xid_rsp;         */

g_build_datagram(p)
  register struct port_dcl *p;
{
  if (p->debug)
    printf("g_build_datagram\n");
  p->d.data_ptr = MTOD(p->m, caddr_t);

  /********************************************************************/
  /* set the lpdu length to i-field length plus header (3-bytes).     */
  /********************************************************************/

  p->d.send_data->lpdu_length = (p->m->m_next->m_len+3);
  if (p->m->m_next->m_len < (MIN_PACKET-UN_HDR_LENGTH)) { /* Defect 123702 */

    /******************************************************************/
    /* the packet length is less than minimum packet size. set the    */
    /* packet length to the minimum allowed.                          */
    /******************************************************************/

    p->m->m_next->m_len = (MIN_PACKET-UN_HDR_LENGTH); /* End Defect 123702 */
  } 
  p->m->m_len = UN_HDR_LENGTH;

  /********************************************************************/
  /* load the ui control byte.                                        */
  /********************************************************************/

  p->d.send_data->ctl1 = UI_NO_PF;

  /********************************************************************/
  /* build the remainder of the xid command.                          */
  /********************************************************************/

  bcopy(p->sta_ptr->raddr, p->d.send_data->raddr, 6);
  bcopy(p->sta_ptr->laddr, p->d.send_data->laddr, 6);
  p->d.send_data->llc_type = SNA_LLC_TYPE;
  p->d.send_data->lpad = 0;
  p->d.send_data->rsap = p->sta_ptr->ls_profile.rsap;
  p->d.send_data->lsap = (p->sap_ptr->sap_profile.local_sap&RESP_OFF);

/* <<< feature CDLI >>> */
  /********************************************************************/
  /* call the write send command generator, with the ui command       */
  /* buffer address.                                                  */
  /********************************************************************/

  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
}                                      /* end g_build_datagram;       */

g_trgen(p)
  register struct port_dcl *p;
{
/* <<< feature CDLI >>> */
/* <<< removed temp_addr[6] >>> */
/* <<< feature CDLI >>> */
  u_char   temp_sap;
  if (p->debug)
    printf("g_trgen %x %d\n", p->sta_ptr, p->sta_ptr->ignore);

    /******************************************************************/
    /* build and send the test response in the same buffer that the   */
    /* test command was received in, and bump test ras counts.        */
    /******************************************************************/

  if                                   /* not a response for the null
                                          sap, ie. a station exists   */
     (p->rcv_data.lsap != NULL_SAP) {
    if                                 /* test commands received
                                          counter not at maximum value*/
       (p->sta_ptr->ras_counters.counters.test_cmds_rec != DLC_ERR)
       {

      /****************************************************************/
      /* increment count of test commands received                    */
      /****************************************************************/

      ++p->sta_ptr->ras_counters.counters.test_cmds_rec;
    } 
  } 

/* <<< feature CDLI >>> */
/* <<< this is a rework of token-ring apar IX34430 defect 82097 >>> */
  /********************************************************************/
  /* move the received source address to the send destination address */
  /********************************************************************/

  bcopy(&p->rcv_data.raddr[0], &p->d.send_buf_rsp->raddr[0], 6);

  /********************************************************************/
  /* force the send source address to be the local card address       */
  /********************************************************************/

  bcopy(&p->common_cb.local_addr[0], &p->d.send_buf_rsp->laddr[0], 6);

/* <<< end feature CDLI >>> */

  /********************************************************************/
  /* swap the remote and destination sap fields.                      */
  /********************************************************************/

  temp_sap = p->rcv_data.rsap;
  p->d.send_buf_rsp->lsap = (p->rcv_data.lsap|RESP_ON);
  p->d.send_buf_rsp->rsap = temp_sap;

  /********************************************************************/
  /* build the remainder of the data link header.                     */
  /* p->d.send_buf_rsp->ctl1 = p->rcv_data.ctl1; already there        */
  /********************************************************************/

  p->d.send_buf_rsp->llc_type = SNA_LLC_TYPE;/* was overlayed w/saps  */
  if                                   /* the received buffer
                                          indicates overflow          */
     (((int)p->m->m_nextpkt & DLC_OFLO) == DLC_OFLO) {

    /******************************************************************/
    /* set the response lpdu length to include only the data link     */
    /* header fields.                                                 */
    /******************************************************************/

    p->d.send_buf_rsp->lpdu_length = 3;

    /******************************************************************/
    /* set the buffer's data length to the minimum packet length.     */
    /******************************************************************/

    p->m->m_len = MIN_PACKET;
  } 
  else                                 /* no overflow of the buffer
                                          occurred.                   */
    {

    /******************************************************************/
    /* leave the lpdu length as received.                             */
    /* p->d.send_buf_rsp->lpdu_length = ? already there leave the lpdu*/
    /* length as received. p->m->m_len = ? already there              */
    /******************************************************************/

  } 

/* <<< feature CDLI >>> */
    /******************************************************************/
    /* call the write send command generator, with the test command   */
    /* buffer address.                                                */
    /******************************************************************/

  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
}                                      /* end g_trgen                 */

g_send_frmr(p, fr_reason, fr_pf)
  register struct port_dcl *p;
  u_char   fr_reason;
  u_char   fr_pf;
{
  u_char   temp_sap;
  char     temp_addr[6];
  if (p->debug)
    printf("g_send_frmr\n");

    /******************************************************************/
    /* get addressability to the i-field.                             */
    /******************************************************************/

  p->i.i_field_ptr = &(p->d.rcv_data->ctl2);/* un_info                */

  /********************************************************************/
  /* build the frmr i-field.                                          */
  /********************************************************************/

  {
    if                                 /* the received packet was an
                                          UNNUMBERED frame            */
       ((p->rcv_data.ctl1&0x03) == UNNUMBERED)

      /****************************************************************/
      /* set the frmr control byte-2 to zero.                         */
      /****************************************************************/

      p->i.i_field_frmr->frmr_ctl2 = 0;
    else                               /* set the frmr control byte-2
                                          to the received control
                                          byte-2.                     */
      p->i.i_field_frmr->frmr_ctl2 = p->rcv_data.ctl2;

      /****************************************************************/
      /* set the frmr control byte-1 to the received control byte-1.  */
      /****************************************************************/

    p->i.i_field_frmr->frmr_ctl1 = p->rcv_data.ctl1;

    /******************************************************************/
    /* set the frmr returned vs and vr to the current station vs and  */
    /* vr.                                                            */
    /******************************************************************/

    p->i.i_field_frmr->frmr_vs = p->sta_ptr->vs;
    p->i.i_field_frmr->frmr_vr = p->sta_ptr->vr;
    if                                 /* the received packet was a
                                          response                    */
       (TSTBIT(p->rcv_data.rsap, RESPONSE) == TRUE)

      /****************************************************************/
      /* set the frmr c/r bit in the returned vr field.               */
      /****************************************************************/

      SETBIT(p->i.i_field_frmr->frmr_vr, FRMR_RESPONSE);
    else                               /* reset the frmr c/r bit in
                                          the returned vr field.      */
      CLRBIT(p->i.i_field_frmr->frmr_vr, FRMR_RESPONSE);

      /****************************************************************/
      /* set the frmr reason code to the supplied value.              */
      /****************************************************************/

    p->i.i_field_frmr->frmr_reason = fr_reason;
  } 

  /********************************************************************/
  /* swap the remote and destination address fields.                  */
  /********************************************************************/

  bcopy(p->rcv_data.raddr, temp_addr, 6);
  bcopy(p->rcv_data.laddr, p->d.send_data->laddr, 6);
  bcopy(temp_addr, p->d.send_data->raddr, 6);

  /********************************************************************/
  /* swap the remote and destination sap fields.                      */
  /********************************************************************/

  temp_sap = (p->rcv_data.rsap&RESP_OFF);
  p->d.send_data->lsap = (p->rcv_data.lsap|RESP_ON);
  p->d.send_data->rsap = temp_sap;

  /********************************************************************/
  /* build the remainder of the data link header.                     */
  /********************************************************************/

  p->d.send_data->ctl1 = (FRMR_NO_PF|fr_pf);
  p->d.send_data->llc_type = SNA_LLC_TYPE;
  p->d.send_data->lpdu_length = 8;

  /********************************************************************/
  /* set the packet length to minimum packet size.                    */
  /********************************************************************/

  p->m->m_len = MIN_PACKET;

/* <<< feature CDLI >>> */
  /********************************************************************/
  /* call the write send command generator, with the invalid receive  */
  /* buffer address.                                                  */
  /********************************************************************/

  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
}                                      /* end g_send_frmr             */

g_test_link_cmd(p)
  register struct port_dcl *p;
{
  int      index,if_size;
  struct mbuf *pt;
  if (p->debug)
    printf("g_test_link_cmd p->sta_ptr=%x\n", p->sta_ptr, 
       p->sta_ptr->ls_profile.maxif);

    /******************************************************************/
    /* save address of header buffer                                  */
    /******************************************************************/

  pt = p->m;

  /********************************************************************/
  /* get address of start of cluster                                  */
  /********************************************************************/

  MTOCL(pt);
  p->d.data_ptr = MTOD(pt, char *);
  p->i.i_field_ptr = &(p->d.send_data->ctl2);/* un_info               */
  if                                   /* maximum i-field size is
                                          greater than 256            */
     (p->sta_ptr->ls_profile.maxif > 256) {

    /******************************************************************/
    /* set the i-field size to 256.                                   */
    /******************************************************************/

    if_size = 256;
  } 
  else                                 /* the max i-field is less than
                                          or equal to 256.            */
    {

    /******************************************************************/
    /* set the i-field size to the current max i-field size.          */
    /******************************************************************/

    if_size = p->sta_ptr->ls_profile.maxif;
  } 
  if (p->debug)
    printf("if_size=%d\n", if_size);

    /******************************************************************/
    /* set the buffer data length to the maximum i-field length plus  */
    /* the header length of an UNNUMBERED command packet,             */
    /******************************************************************/

/* <<< feature CDLI >>> */
  p->m->m_len = (if_size + UN_HDR_LENGTH);

  if /* the transmition length is still less that minimum packet size */
     (p->m->m_len < MIN_PACKET)
    {
      /* set m_len to minimum packet size */
      p->m->m_len = MIN_PACKET;
    }
/* <<< end feature CDLI >>> */

  /********************************************************************/
  /* build the header area of the test packet.                        */
  /********************************************************************/

  bcopy(p->sta_ptr->raddr, p->d.send_data->raddr, 6);
  bcopy(p->sta_ptr->laddr, p->d.send_data->laddr, 6);
  p->d.send_data->llc_type = SNA_LLC_TYPE;
  p->d.send_data->lpdu_length = (if_size+3);
  p->d.send_data->lpad = 0;
  p->d.send_data->rsap = p->sta_ptr->ls_profile.rsap;
  p->d.send_data->lsap = (p->sap_ptr->sap_profile.local_sap&RESP_OFF);
  p->d.send_data->ctl1 = TEST;

  /********************************************************************/
  /* for index = 1 to the i-field size, build a test i-field of x'00' */
  /* through x'ff' repeating.                                         */
  /********************************************************************/

  for (index = 1; index <= if_size; index++) {

    /******************************************************************/
    /* set the data equal to the index value modulo 256.              */
    /******************************************************************/

    *(p->i.i_field_ptr)++ = (index-1)%256;
  }                                    /* end for index;              */
  if                                   /* in abme checkpointing state */
     (p->sta_ptr->checkpointing == TRUE) {

    /******************************************************************/
    /* stack the test command for future transmission.                */
    /******************************************************************/

    p->sta_ptr->vc = TEST_ENA;
  } 
  else                                 /* not abme checkpointing.     */
    {

    /******************************************************************/
    /* call the send test command routine.                            */
    /******************************************************************/

    send_test_cmd(p);
  } 
} 
g_find_self_gen(p)
  register struct port_dcl *p;
{
  int      i;
  int      vec_size;
  if (p->debug)
    printf("g_find_self_gen\n");
  TRACE1(p, "GFSb");
  p->d.data_ptr = MTOD(p->m, caddr_t);

  /********************************************************************/
  /* build a discovery "find self" command to remote sap fc.          */
  /********************************************************************/

  for (i = 0; i < 6; i++) {
    p->d.send_data->raddr[i] = 0xff;
  } 
  bcopy(p->common_cb.local_addr, p->d.send_data->laddr, 6);
  p->d.send_data->llc_type = SNA_LLC_TYPE;
  p->d.send_data->lpad = 0;
  p->d.send_data->rsap = DISCOVERY_SAP;
  p->d.send_data->lsap = DISCOVERY_SAP;
  p->d.send_data->ctl1 = UI_NO_PF;
  vec_size = build_vector(p, (p->d.data_ptr+UN_HDR_LENGTH), 
     FIND_VECTOR_KEY, (p->sapno|SAP_CORR_MASK));
  p->d.send_data->lpdu_length = vec_size+3;
  if                                   /* the data length is less than
                                          a minimum packet size       */
     ((vec_size+UN_HDR_LENGTH) < MIN_PACKET)

    /******************************************************************/
    /* set the buffer data length to minimum packet length.           */
    /******************************************************************/

    p->m->m_len = MIN_PACKET;
  else                                 /* set the buffer data length
                                          to actual packet length.    */
    p->m->m_len = (vec_size+UN_HDR_LENGTH);

/* <<< feature CDLI >>> */
  /********************************************************************/
  /* set the M_BCAST mbuf flag to indicate that this is a broadcast   */
  /* packet being sent.                                               */
  /********************************************************************/
  p->m->m_flags |= M_BCAST;

  /********************************************************************/
  /* call the write send command generator, with the "call" buffer    */
  /* address, disabling link trace.                                   */
  /********************************************************************/
  p->stano = NO_MATCH;                 /* disables trace              */
  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
  TRACE1(p, "GFSe");
}                                      /* end g_find_self_gen         */

g_find_remote_gen(p)
  register struct port_dcl *p;
{
  int      i;
  ulong_t  arc;                        /* cache index                 */
  int      vec_size;

  /********************************************************************/
  /* get address of cache                                             */
  /********************************************************************/

  p_to_cache = (struct cache *)p->p2_to_cache;

  /********************************************************************/
  /* get addressability back to calling station                       */
  /********************************************************************/

  p->sta_ptr = (struct station_cb *)p->station_list[p->stano].
     sta_cb_addr;

  /********************************************************************/
  /* set up send data pointer                                         */
  /********************************************************************/

  p->d.data_ptr = MTOD(p->m, caddr_t);

  /********************************************************************/
  /* build the common part of the DLC header: ie. local address, type */
  /* field, and pad                                                   */
  /********************************************************************/

  bcopy(p->common_cb.local_addr, p->d.send_data->laddr, 6);
  p->d.send_data->llc_type = SNA_LLC_TYPE;
  p->d.send_data->lpad = 0;
  if                                   /* station using resolve       */
     ((p->sta_ptr->ls_profile.flags&DLC_SLS_ADDR) != 0) {
#ifdef   DEBUG
    if (p->debug)
      printf("edlmod:using resolve\n");
#endif

    /******************************************************************/
    /* build the remainder of the DLC headet for a test command       */
    /******************************************************************/

    bcopy(p->sta_ptr->ls_profile.raddr_name, p->d.send_data->raddr, 6)
       ;
    p->d.send_data->lpdu_length = 3;
    p->d.send_data->rsap = RESOLVE_SAP;
    p->d.send_data->lsap = p->sap_ptr->sap_profile.local_sap;
    p->d.send_data->ctl1 = TEST;

    /******************************************************************/
    /* set the mbuf data length to minimum packet length.             */
    /******************************************************************/

    p->m->m_len = MIN_PACKET;
  } 
  else {                               /* using discovery             */
#ifdef   DEBUG
    if (p->debug)
      printf("edlmod:using discovery\n");
#endif

    /******************************************************************/
    /* build a discovery "find" command to remote sap fc.             */
    /******************************************************************/

    for (i = 0; i < 6; i++) {
      p->d.send_data->raddr[i] = 0xff;
      }
      
      if                               /* name found in cache from
                                          previous search             */
         (p->sta_ptr->sta_cache == CACHE_NAME) {

        /**************************************************************/
        /* use index from previous search to copy address into packet */
        /**************************************************************/

        bcopy(p_to_cache->cache_data[p_to_cache->name_index[
           p->sta_ptr->cache_pindex]].address, p->d.send_data->raddr, 
           6);
      } 
      else
        if                             /* cache not previously
                                          searched                    */
           (p->sta_ptr->sta_cache != CACHE_WRONG_NAME) {

          /************************************************************/
          /* call search cache to find card address of remote name    */
          /************************************************************/

          arc = search_cache(p_to_cache->name_index, 
             p_to_cache->n_entries, p->sta_ptr->ls_profile.raddr_name,
             p_to_cache->cache_data);
          if                           /* remote name is in cache     */
             (arc != -1) {

            /**********************************************************/
            /* replace broadcast address in packet with the actual    */
            /* card address                                           */
            /**********************************************************/

            bcopy(p_to_cache->cache_data[p_to_cache->name_index[arc]].
               address, p->d.send_data->raddr, 6);

            /**********************************************************/
            /* set flag indicating packet contains name from cache    */
            /**********************************************************/

            p->sta_ptr->sta_cache = CACHE_NAME;

            /**********************************************************/
            /* save name index                                        */
            /**********************************************************/

            p->sta_ptr->cache_pindex = arc;
          } 
        } 

    vec_size = build_vector(p, p->d.data_ptr+UN_HDR_LENGTH, 
       FIND_VECTOR_KEY, p->stano);
    p->d.send_data->lpdu_length = (vec_size+3);
    p->d.send_data->rsap = DISCOVERY_SAP;
    p->d.send_data->lsap = DISCOVERY_SAP;
    p->d.send_data->ctl1 = UI_NO_PF;
    if                                 /* the data length is less than
                                          a minimum packet size       */
       ((vec_size+UN_HDR_LENGTH) < MIN_PACKET)

      /****************************************************************/
      /* set the buffer data length to minimum packet length.         */
      /****************************************************************/

      p->m->m_len = MIN_PACKET;
    else                               /* set the buffer data length
                                          to actual packet length.    */
      p->m->m_len = (vec_size+UN_HDR_LENGTH);
  } 

/* <<< feature CDLI >>> */
  /********************************************************************/
  /* set the M_BCAST mbuf flag to indicate that this is a broadcast   */
  /* packet being sent.                                               */
  /********************************************************************/
  p->m->m_flags |= M_BCAST;

  /********************************************************************/
  /* call the write send command generator, with the find remote      */
  /* command buffer address.                                          */
  /********************************************************************/
  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
}                                      /* end g_find_remote_gen       */

g_dup_add_name_vector(p)
  register struct port_dcl *p;
{
  short    vec_size;
  if (p->debug)
    printf("g_dup_add_name_vector\n");

    /*----------------------------------------------------------------*/
    /* alter the find name query i-field into a name found response   */
    /* packet (everything already set up except the key).             */
    /*----------------------------------------------------------------*/
    /* move the remote source address to the destination address      */
    /* field.                                                         */
    /******************************************************************/

  bcopy(p->rcv_data.raddr, p->d.send_buf_rsp->raddr, 6);

  /********************************************************************/
  /* fill in the local address in the source address field.           */
  /********************************************************************/

  bcopy(p->common_cb.local_addr, p->d.send_buf_rsp->laddr, 6);
  p->d.send_buf_rsp->llc_type = SNA_LLC_TYPE;/* already there         */
  p->d.send_buf_rsp->lpad = 0;         /* already there               */
  p->d.send_buf_rsp->rsap = DISCOVERY_SAP;/* already there            */
  p->d.send_buf_rsp->lsap = DISCOVERY_SAP;/* already there            */
  p->d.send_buf_rsp->ctl1 = UI_NO_PF;  /* already there               */
  vec_size = build_vector(p, (p->d.data_ptr+UN_HDR_LENGTH), 
     FOUND_VECTOR_KEY, p->correlator_vector.value);
  vec_size = vec_size+3;
  bcopy(&vec_size, &(p->d.send_buf_rsp->lpdu_length), 2);
  if                                   /* the data length is less than
                                          a minimum packet size       */
     ((vec_size+UN_HDR_LENGTH) < MIN_PACKET)

    /******************************************************************/
    /* set the buffer data length to minimum packet length.           */
    /******************************************************************/

    p->m->m_len = MIN_PACKET;
  else                                 /* set the buffer data length
                                          to actual packet length.    */
    p->m->m_len = (vec_size+UN_HDR_LENGTH);

/* <<< feature CDLI >>> */
    /******************************************************************/
    /* call the write send command generator, with the add name       */
    /* response buffer address, disabling link trace.                 */
    /******************************************************************/

  p->stano = NO_MATCH;                 /* disables trace              */
  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
}                                      /* end dup_add_name_vector;    */

g_found_vector_gen(p)
  register struct port_dcl *p;
{
  short    vec_size;
  if (p->debug)
    printf("g_found_vector_gen\n");
  if                                   /* link trace enabled.         */
     ((p->sta_ptr->ls_profile.flags&DLC_TRCO) == DLC_TRCO) {

    /******************************************************************/
    /* call the receive link trace routine.                           */
    /******************************************************************/

    lanrcvtr(p);
  } 

    /*----------------------------------------------------------------*/
    /* alter the find name vector into a name found vector packet     */
    /* (everything already set up except the key).                    */
    /*----------------------------------------------------------------*/
    /* move the remote source address to the destination address      */
    /* field.                                                         */
    /******************************************************************/

  bcopy(p->rcv_data.raddr, p->d.send_buf_rsp->raddr, 6);

  /********************************************************************/
  /* fill in the local address in the source address field.           */
  /********************************************************************/

  bcopy(p->common_cb.local_addr, p->d.send_buf_rsp->laddr, 6);

  /********************************************************************/
  /* build the remainder of the data link header.                     */
  /********************************************************************/

  p->d.send_buf_rsp->llc_type = SNA_LLC_TYPE;/* already there         */
  p->d.send_buf_rsp->lpad = 0;         /* already there               */
  p->d.send_buf_rsp->rsap = DISCOVERY_SAP;/* already there            */
  p->d.send_buf_rsp->lsap = DISCOVERY_SAP;/* already there            */
  p->d.send_buf_rsp->ctl1 = UI_NO_PF;  /* already there               */
  vec_size = build_vector(p, (p->d.data_ptr+UN_HDR_LENGTH), 
     FOUND_VECTOR_KEY, p->correlator_vector.value);
  vec_size = vec_size+3;
  bcopy(&vec_size, &(p->d.send_buf_rsp->lpdu_length), 2);
  if                                   /* the data length is less than
                                          a minimum packet size       */
     ((vec_size+UN_HDR_LENGTH) < MIN_PACKET)

    /******************************************************************/
    /* set the buffer data length to minimum packet length.           */
    /******************************************************************/

    p->m->m_len = MIN_PACKET;
  else                                 /* set the buffer data length
                                          to actual packet length.    */
    p->m->m_len = (vec_size+UN_HDR_LENGTH);

/* <<< feature CDLI >>> */
    /******************************************************************/
    /* call the write send command generator, with the call response  */
    /* buffer address.                                                */
    /******************************************************************/

  if (p->debug)
    printf("m=%x data_ptr=%x\n", p->m, p->d.data_ptr);
  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
}                                      /* end g_found_vector_gen      */
#endif                                 /* EDL                         */
