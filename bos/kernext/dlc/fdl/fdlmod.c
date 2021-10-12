static char sccsid[] = "@(#)70  1.7  src/bos/kernext/dlc/fdl/fdlmod.c, sysxdlcf, bos411, 9439A411b 9/26/94 18:01:48";

/**********************************************************************
 * COMPONENT_NAME: (SYSXDLCF) FDDI Data Link Control
 *
 * FUNCTIONS: fdlmod.c
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 **********************************************************************/

/* <<< feature CDLI >>> */
#ifdef FDL

/**********************************************************************/
/*  This module contains FDDI specific routines only.                 */
/**********************************************************************/

#include <fcntl.h>
#include <sys/types.h>
#include <sys/fp_io.h>
#include <net/spl.h>
#include <sys/mbuf.h>
#include <sys/file.h>
#include <sys/malloc.h>
#include <sys/errno.h>
#include "dlcadd.h"
#include <sys/gdlextcb.h>  
#include <sys/ndd.h>
#include <sys/ndd_var.h>
#include <sys/cdli.h>
#include <sys/cdli_fddiuser.h>
#include <sys/fdlextcb.h>
#include "lancomcb.h"
#include "lanstlst.h"
#include "lansplst.h"
#include "lanrstat.h"
#include "lanport.h"
/* <<< end feature CDLI >>> */

struct   rip
      {
        u_char   rsap;
        u_char   lsap;
        u_char   ctl1;
        u_char   ctl2;
        u_char   ack_slot;
      } ;

int search_cache();
g_build_iframe(p)
  register struct port_dcl *p;
{

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
     (p->m != 0)
    {

      /****************************************************************/
      /* setup data and routing pointers                              */
      /****************************************************************/

      p->d.data_ptr = MTOD(p->m, caddr_t);

		if( p->sta_ptr->ri_length < 3 )
			p->sta_ptr->ri_length = 0;

     	p->ri.ptr_ri = ((p->d.send_data->ri_field)+p->sta_ptr->ri_length);

      /****************************************************************/
      /* copy the prebuilt dlc header (1st 16 bytes) from the station */
      /* cb                                                           */
      /****************************************************************/

      bcopy(&(p->sta_ptr->phy_ctl1), &(p->d.send_data->phy_ctl_1), FDL_ROUTING_OFFSET);
 
      if                               /* routing information field   
                                          present                     */
         (p->sta_ptr->ri_length > 0)
        {

          /************************************************************/
          /* move routing information                                 */
          /************************************************************/

/* LEHb defect 44499 */
	  bcopy(&(p->sta_ptr->ri_field), &(p->d.send_data->ri_field[0]),
	     p->sta_ptr->ri_length);
/* LEHe */

          /************************************************************/
          /* set routing information present flag                     */
          /************************************************************/

          SETBIT(p->d.send_data->laddr[0], RI_PRESENT);
        } 

      /****************************************************************/
      /* copy the prebuilt dlc header (last 4 bytes) from the station */
      /* cb                                                           */
      /****************************************************************/

      bcopy(&(p->sta_ptr->rsap), &(p->ri.ri_sd->rsap), 4);

      /****************************************************************/
      /* set frame length                                             */
      /****************************************************************/

      p->m->m_len = FDL_ROUTING_OFFSET+4+p->sta_ptr->ri_length;
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
}                                      /* end build_iframe;           */
tx_sta_cmd(p,in_ctl1,in_ctl2,format)
  register struct port_dcl *p;
  register u_char in_ctl1;
  register u_char in_ctl2;
  register u_char format;
{
  int      cmd_size;
  struct rip *tptr_cmd_sta;

  TRACE1(p, "TXSb");

  /********************************************************************/
  /* save control byte-1 in the last command control sent.            */
  /********************************************************************/

  p->sta_ptr->last_cmd_1 = in_ctl1;
  p->m0 = (struct mbuf *)lanfetch(p);
 
  if                                   /* buffer is available         */
     (p->m0 != (struct mbuf *)NO_BUF_AVAIL)
    {

      /****************************************************************/
      /* get addressability to the data area                          */
      /****************************************************************/

      p->sta_cmd_buf = MTOD(p->m0, struct STA_CMD_BUF *);

      /****************************************************************/
      /* setup temporary station command pointer to actual start of   */
      /* routing information                                          */
      /****************************************************************/

		if( p->sta_ptr->ri_length < 3 )
			p->sta_ptr->ri_length = 0;

      tptr_cmd_sta = (struct rip *)((&p->sta_cmd_buf->ri_field[0])+
         p->sta_ptr->ri_length);

      /****************************************************************/
      /* calculate frame size                                         */
      /****************************************************************/

      cmd_size = FDL_ROUTING_OFFSET+3+p->sta_ptr->ri_length;

      /****************************************************************/
      /* build the remainder of the station command.                  */
      /****************************************************************/
/* LEHb defect XXX */
      p->sta_cmd_buf->phy_ctl_1 = p->sta_ptr->phy_ctl1;
/* LEHe */
      bcopy(p->sta_ptr->raddr, p->sta_cmd_buf->raddr, 6);
      bcopy(p->sta_ptr->laddr, p->sta_cmd_buf->laddr, 6);
      tptr_cmd_sta->rsap = p->sta_ptr->ls_profile.rsap;
      tptr_cmd_sta->lsap = (p->sap_ptr->sap_profile.local_sap&RESP_OFF
         );
      tptr_cmd_sta->ctl1 = in_ctl1;
 
      if                               /* there is only one control   
                                          byte to transmit            */
         (format == 1)
        {
          ;                            /* set the lpdu length to 3.   */
        } 
 
      else                             /* there are two control bytes 
                                          to transmit.                */
        {

          /************************************************************/
          /* load the 2nd control byte into the command.              */
          /************************************************************/

          tptr_cmd_sta->ctl2 = in_ctl2;

          /************************************************************/
          /* increment command size                                   */
          /************************************************************/

          cmd_size = cmd_size+1;
        } 
 
      if                               /* routing information field   
                                          present                     */
         (p->sta_ptr->ri_length > 0)
        {

          /************************************************************/
          /* move routing information                                 */
          /************************************************************/

/* LEHb defect 44499 */
	  bcopy(&(p->sta_ptr->ri_field), &(p->sta_cmd_buf->ri_field[0]),
	     p->sta_ptr->ri_length);
/* LEHe */

          /************************************************************/
          /* set routing information present flag                     */
          /************************************************************/

          SETBIT(p->sta_cmd_buf->laddr[0], RI_PRESENT);
        } 
      p->m0->m_len = cmd_size;

      /****************************************************************/
      /* save station index number                                    */
      /****************************************************************/

      tptr_cmd_sta->ack_slot = p->stano;

/* <<< feature CDLI >>> */
      /****************************************************************/
      /* call the write send command generator, with the station      */
      /* command buffer address.                                      */
      /****************************************************************/

      lanwrtsg(p, p->m0);
/* <<< end feature CDLI >>> */
 
      if                               /* command polls counter not at
                                          maximum value               */
         (p->sta_ptr->ras_counters.counters.cmd_polls_sent != DLC_ERR)

        /**************************************************************/
        /* increment count of command polls sent                      */
        /**************************************************************/

        ++p->sta_ptr->ras_counters.counters.cmd_polls_sent;

      /****************************************************************/
      /* restart t1 repoll timer                                      */
      /****************************************************************/

      p->station_list[p->stano].t1_ena = TRUE;
      p->station_list[p->stano].t1_ctr = p->sta_ptr->resp_to_val;
    } 
}                                      /* end tx_sta_cmd;             */
tx_rr_rsp(p,in_ctl2)
  register struct port_dcl *p;
  register u_char in_ctl2;
{
  int      cmd_size;
  struct mbuf *temp_buf_ptr;
  char     *temp_data_ptr;
  char     *tptr_rsp_sta;

  TRACE1(p, "TXRb");

  /********************************************************************/
  /* save receive buffer pointer                                      */
  /********************************************************************/

  temp_buf_ptr = p->m;
  temp_data_ptr = p->d.data_ptr;

  /********************************************************************/
  /* get a buffer                                                     */
  /********************************************************************/

  p->m = (struct mbuf *)lanfetch(p);
 
  if                                   /* buffer is available         */
     (p->m != (struct mbuf *)NO_BUF_AVAIL)
    {

      /****************************************************************/
      /* adjust data pointer to routing info                          */
      /****************************************************************/

		if( p->sta_ptr->ri_length < 3 )
			p->sta_ptr->ri_length = 0;

      p->m->m_data += p->sta_ptr->ri_length;

      /****************************************************************/
      /* set the buffer data offset= offset of data area).            */
      /****************************************************************/

      p->d.data_ptr = MTOD(p->m, caddr_t);

      /****************************************************************/
      /* build the header area of the packet.                         */
      /****************************************************************/

      bcopy(p->sta_ptr->raddr, p->d.send_data->raddr, 6);
      bcopy(p->sta_ptr->laddr, p->d.send_data->laddr, 6);
 
      if                               /* routing information present */
         (p->sta_ptr->ri_length > 0)
        {

          /************************************************************/
          /* copy routing info data;                                  */
          /************************************************************/

/* LEHb defect 44499 */
	  bcopy(&(p->sta_ptr->ri_field), &(p->d.send_data->ri_field[0]),
	     p->sta_ptr->ri_length);
/* LEHe */

          /************************************************************/
          /* set routing information present                          */
          /************************************************************/

          SETBIT(p->d.send_data->laddr[0], RI_PRESENT);
        } 

      /****************************************************************/
      /* setup routing information pointer                            */
      /****************************************************************/

      p->ri.ptr_ri = (p->d.send_data->ri_field)+p->sta_ptr->ri_length;

      /****************************************************************/
      /* calculate frame size                                         */
      /****************************************************************/

      cmd_size = p->sta_ptr->ri_length+FDL_ROUTING_OFFSET+4;

      /****************************************************************/
      /* load the 2nd control byte into the response.                 */
      /****************************************************************/

      p->ri.ri_sd->ctl2 = in_ctl2;
      p->ri.ri_sd->rsap = p->sta_ptr->ls_profile.rsap;
      p->ri.ri_sd->lsap = (p->sap_ptr->sap_profile.local_sap|RESP_ON);
      p->ri.ri_sd->ctl1 = RR;

      /****************************************************************/
      /* set up access an control packet values                       */
      /****************************************************************/

      p->d.send_data->phy_ctl_1 = p->sta_ptr->phy_ctl1;

      /****************************************************************/
      /* set length of packet                                         */
      /****************************************************************/

      p->m->m_len = cmd_size;

/* <<< feature CDLI >>> */
      /****************************************************************/
      /* call the write send command generator, with the station      */
      /* response buffer address.                                     */
      /****************************************************************/

      lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
    } 

  /********************************************************************/
  /* restore receive buffers                                          */
  /********************************************************************/

  p->m = temp_buf_ptr;
  p->d.data_ptr = temp_data_ptr;
}                                      /* end tx_rr_rsp;              */
tx_rnr_rsp(p,in_ctl2)
  register struct port_dcl *p;
  register u_char in_ctl2;
{
  int      cmd_size;
  struct rip *tptr_unsol;

  TRACE1(p, "TXNb");
  p->m0 = (struct mbuf *)lanfetch(p);
 
  if                                   /* buffer is available         */
     (p->m0 != (struct mbuf *)NO_BUF_AVAIL)
    {

      /****************************************************************/
      /* get addressability to the data area                          */
      /****************************************************************/

      p->rnr_rsp_buf = MTOD(p->m0, struct STA_CMD_BUF *);

      /****************************************************************/
      /* setup temporary station command pointer to actual start of   */
      /* routing information tptr_unsol = ptr_unsol + p->sta_ptr->    */
      /* ri_length;                                                   */
      /****************************************************************/

		if( p->sta_ptr->ri_length < 3 )
			p->sta_ptr->ri_length = 0;

      tptr_unsol = (struct rip *)((&p->rnr_rsp_buf->ri_field[0])+
         p->sta_ptr->ri_length);

      /****************************************************************/
      /* calculate size of frame                                      */
      /****************************************************************/

      cmd_size = p->sta_ptr->ri_length+FDL_ROUTING_OFFSET+4;
        {

          /************************************************************/
          /* load the 2nd control byte into the response.             */
          /************************************************************/

          tptr_unsol->ctl2 = in_ctl2;
        } 

      /****************************************************************/
      /* build the remainder of the unsolicited response.             */
      /****************************************************************/
/* LEHb defect XXX */
      p->rnr_rsp_buf->phy_ctl_1 = p->sta_ptr->phy_ctl1;
/* LEHe */
      bcopy(p->sta_ptr->raddr, p->rnr_rsp_buf->raddr, 6);
      bcopy(p->sta_ptr->laddr, p->rnr_rsp_buf->laddr, 6);
      tptr_unsol->rsap = p->sta_ptr->ls_profile.rsap;
      tptr_unsol->lsap = (p->sap_ptr->sap_profile.local_sap|RESP_ON);
      tptr_unsol->ctl1 = RNR;
 
      if                               /* routing information field   
                                          present                     */
         (p->sta_ptr->ri_length > 0)

        /**************************************************************/
        /* move routing information                                   */
        /**************************************************************/

        {
/* LEHb defect 44499 */
	  bcopy(&(p->sta_ptr->ri_field), &(p->rnr_rsp_buf->ri_field[0]),
	     p->sta_ptr->ri_length);
/* LEHe */

          /************************************************************/
          /* set routing information present flag                     */
          /************************************************************/

          SETBIT(p->rnr_rsp_buf->laddr[0], RI_PRESENT);
        } 
      p->m0->m_len = cmd_size;

      /****************************************************************/
      /* save station index number                                    */
      /****************************************************************/

      tptr_unsol->ack_slot = p->stano;

/* <<< feature CDLI >>> */
      /****************************************************************/
      /* call the write send command generator, with the unsolicited  */
      /* response buffer address.                                     */
      /****************************************************************/

      lanwrtsg(p, p->m0);
/* <<< end feature CDLI >>> */
    } 
}                                      /* end tx_rnr_rsp;             */

tx_buf_rsp(p,in_ctl1,in_ctl2,format)
  register struct port_dcl *p;
  register u_char in_ctl1;
  register u_char in_ctl2;
  register u_char format;
{
  int      cmd_size;
  char     *tptr_unsol;

  TRACE1(p, "TXBb");
#ifdef   DEBUG
 
  if (p->debug)
    printf("tx_buf_rsp\n");
#endif

	if( p->sta_ptr->ri_length < 3 )
		p->sta_ptr->ri_length = 0;

  /********************************************************************/
  /* insure that current buffer is not returned to pool by dlc        */
  /********************************************************************/

  p->sta_ptr->ignore = FALSE;

  /********************************************************************/
  /* setup data and routing information pointers                      */
  /********************************************************************/

  p->d.data_ptr = MTOD(p->m, caddr_t);

  /********************************************************************/
  /* adjust data pointer to routing info                              */
  /********************************************************************/

  p->d.data_ptr = p->d.data_ptr+p->sta_ptr->ri_length;

  /********************************************************************/
  /* build the header area of the packet.                             */
  /********************************************************************/

  bcopy(p->sta_ptr->raddr, p->d.send_data->raddr, 6);
  bcopy(p->sta_ptr->laddr, p->d.send_data->laddr, 6);
 
  if                                   /* routing information present */
     (p->sta_ptr->ri_length > (ulong_t)0)
    {

      /****************************************************************/
      /* copy routing info data;                                      */
      /****************************************************************/

/* LEHb defect 44499 */
      bcopy(&(p->sta_ptr->ri_field), &(p->d.send_data->ri_field[0]),
	 p->sta_ptr->ri_length);
/* LEHe */

      /****************************************************************/
      /* set routing information present                              */
      /****************************************************************/

      SETBIT(p->d.send_data->laddr[0], RI_PRESENT);
    } 

  /********************************************************************/
  /* set length and offset                                            */
  /********************************************************************/

  p->m->m_data += p->sta_ptr->ri_length;
  p->m->m_len = p->sta_ptr->ri_length;

  /********************************************************************/
  /* setup routing information pointer                                */
  /********************************************************************/

  p->ri.ptr_ri = (p->d.send_data->ri_field)+p->sta_ptr->ri_length;

  /********************************************************************/
  /* calculate size of frame                                          */
  /********************************************************************/

  cmd_size = FDL_ROUTING_OFFSET+3;
 
  if                                   /* only one control byte       */
     (format == 1)
    {
      ;                                /* length already set          */
    } 
 
  else                                 /* two control bytes           */
    {

      /****************************************************************/
      /* load the 2nd control byte into the response.                 */
      /****************************************************************/

      p->ri.ri_sd->ctl2 = in_ctl2;

      /****************************************************************/
      /* increment frame size                                         */
      /****************************************************************/

      cmd_size = cmd_size+1;
    } 

  /********************************************************************/
  /* set length of frame                                              */
  /********************************************************************/

  p->m->m_len += cmd_size;

  /********************************************************************/
  /* load control byte no1                                            */
  /********************************************************************/

  p->ri.ri_sd->ctl1 = in_ctl1;
  p->ri.ri_sd->rsap = p->sta_ptr->rsap;
  p->ri.ri_sd->lsap = (p->sta_ptr->lsap|RESP_ON);
  p->d.send_data->phy_ctl_1 = p->sta_ptr->phy_ctl1;

/* <<< feature CDLI >>> */
  /********************************************************************/
  /* call the write send command generator, with the buffer address   */
  /********************************************************************/

  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
}                                      /* end tx_buf_rsp;             */

tx_disc_rsp(p,in_ctl1)
  register struct port_dcl *p;
  register u_char in_ctl1;
{
/* <<< feature CDLI >>> */
/* <<< removed disc_check_word >>> */
/* <<< end feature CDLI >>> */
  struct rip *tptr_disc;
/* <<< defect 129507 >>> */
  ulong_t disc_rsp_pend;
/* <<< end defect 129507 >>> */

  TRACE1(p, "TXDb");

  /********************************************************************/
  /* indicate that the current receive buffer is being used for the   */
  /* response buffer and should not be returned to the pool           */
  /********************************************************************/

  p->sta_ptr->ignore = FALSE;
  p->disc_rsp_buf = MTOD(p->m, struct STA_CMD_BUF *);
 
  if                                   /* discontacting and sending   
                                          response to disc            */
     ((p->sta_ptr->ls == LS_DISCONTACTING) && ((p->rcv_data.ctl1|
     PF1_MASK) == DISC))

    /******************************************************************/
    /* in discontacting mode so wait for ua_rsp                       */
    /******************************************************************/

    {
/* <<< defect 129507 >>> */
      /* indicate that a disconnect response is still pending */
      disc_rsp_pend = TRUE;
    }
  else
    {
      /* indicate that no disconnect response is pending */
      disc_rsp_pend = FALSE;
/* <<< end defect 129507 >>> */
      /****************************************************************/
      /* stop any reply and acknowledgement timers.                   */
      /****************************************************************/

      p->station_list[p->stano].t1_ctr = -1;
      p->station_list[p->stano].t1_ena = FALSE;
      p->station_list[p->stano].t2_ctr = -2;
      p->station_list[p->stano].t2_ena = FALSE;

      /****************************************************************/
      /* start the disconnect abort timer.                            */
      /****************************************************************/

      p->sta_ptr->t3_state = T3_ABORT;
      p->station_list[p->stano].t3_ctr = p->sta_ptr->force_to_val;
      p->station_list[p->stano].t3_ena = TRUE;

      /****************************************************************/
      /* set link state (ls) = ls_close_pend                          */
      /****************************************************************/

      p->sta_ptr->ls = LS_CLOSE_PEND;

/* <<< feature CDLI >>> */
/* <<< removed p->sta_ptr->close_pend_non_buf >>> */
/* <<< end feature CDLI >>> */

      /****************************************************************/
      /* note: ls set to ls_closed upon send completion.              */
      /****************************************************************/

    } 

  /********************************************************************/
  /* build the disconnect response.                                   */
  /********************************************************************/
/* LEHb defect XXX */
  p->disc_rsp_buf->phy_ctl_1 = p->sta_ptr->phy_ctl1;
/* LEHe */
  bcopy(p->sta_ptr->raddr, p->disc_rsp_buf->raddr, 6);
  bcopy(p->sta_ptr->laddr, p->disc_rsp_buf->laddr, 6);

  /********************************************************************/
  /* setup temporary station command pointer to actual start of       */
  /* routing information                                              */
  /********************************************************************/

	if( p->sta_ptr->ri_length < 3 )
		p->sta_ptr->ri_length = 0;

  tptr_disc = (struct rip *)((&p->disc_rsp_buf->ri_field[0])+
     p->sta_ptr->ri_length);
 
  if                                   /* routing information field   
                                          present                     */
     (p->sta_ptr->ri_length > 0)

    /******************************************************************/
    /* move routing information                                       */
    /******************************************************************/

    {
/* LEHb defect 44499 */
      bcopy(&(p->sta_ptr->ri_field), &(p->disc_rsp_buf->ri_field[0]),
	 p->sta_ptr->ri_length);
/* LEHe */

      /****************************************************************/
      /* set routing information present flag                         */
      /****************************************************************/

      SETBIT(p->disc_rsp_buf->laddr[0], RI_PRESENT);
    } 
  tptr_disc->rsap = p->sta_ptr->ls_profile.rsap;
  tptr_disc->lsap = (p->sap_ptr->sap_profile.local_sap|RESP_ON);
  tptr_disc->ctl1 = in_ctl1;

  /********************************************************************/
  /* store slot number in packet                                      */
  /********************************************************************/

  tptr_disc->ack_slot = p->stano;

  /********************************************************************/
  /* calculate frame size                                             */
  /********************************************************************/

  p->m->m_len = p->sta_ptr->ri_length+FDL_ROUTING_OFFSET+3;

/* <<< feature CDLI >>> */
  /********************************************************************/
  /* write the disconnect response to the device handler, with the    */
  /* disconnect response buffer address.                              */
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
  int      p_length;

  /********************************************************************/
  /* setup data and routing pointers                                  */
  /********************************************************************/

  p->d.data_ptr = MTOD(p->m, caddr_t);

  if( p->sta_ptr->ri_length < 3 )
		p->sta_ptr->ri_length = 0;

  p->ri.ptr_ri = ((p->d.send_data->ri_field)+p->sta_ptr->ri_length);

  /********************************************************************/
  /* build the dlc header of the xid                                  */
  /********************************************************************/

  p->d.send_data->phy_ctl_1 = p->sta_ptr->phy_ctl1;
  bcopy(p->sta_ptr->raddr, p->d.send_data->raddr, 6);
  bcopy(p->sta_ptr->laddr, p->d.send_data->laddr, 6);
 
  if                                   /* routing information field   
                                          present                     */
     (p->sta_ptr->ri_length > 0)
    {

      /****************************************************************/
      /* move routing information                                     */
      /****************************************************************/

/* LEHb defect 44499 */
      bcopy(&(p->sta_ptr->ri_field), &(p->d.send_data->ri_field[0]),
	 p->sta_ptr->ri_length);
/* LEHe */

      /****************************************************************/
      /* set routing information present flag                         */
      /****************************************************************/

      SETBIT(p->d.send_data->laddr[0], RI_PRESENT);
    } 
  p->ri.ri_sd->rsap = p->sta_ptr->ls_profile.rsap;
  p->ri.ri_sd->lsap = p->sap_ptr->sap_profile.local_sap;
  p->ri.ri_sd->ctl1 = XID;

  /********************************************************************/
  /* set length of dlc header mbuf                                    */
  /********************************************************************/

  p->m->m_len = p->sta_ptr->ri_length+FDL_ROUTING_OFFSET+3;
}                                      /* end build_xid;              */
g_send_xid_rsp(p)
  register struct port_dcl *p;
{

	if( p->sta_ptr->ri_length < 3 )
		p->sta_ptr->ri_length = 0;

  /********************************************************************/
  /* set the lsap response indicator in the buffer.                   */
  /********************************************************************/

  p->ri.ri_sd->lsap = (p->ri.ri_sd->lsap|RESP_ON);

  /********************************************************************/
  /* set the final bit to equal the received poll bit setting.        */
  /********************************************************************/

  p->ri.ri_sd->ctl1 = (XID_NO_PF|p->sta_ptr->px);

  /********************************************************************/
  /* setup value of physical control byte 1                           */
  /********************************************************************/
/* LEHb defect XXX */
  p->d.send_data->phy_ctl_1 = p->sta_ptr->phy_ctl1;
/* LEHe */

/* <<< feature CDLI >>> */
  /********************************************************************/
  /* call the write send command generator, with the xid command      */
  /* buffer address.                                                  */
  /********************************************************************/

  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
}                                      /* end g_send_xid_rsp;         */

g_build_datagram(p)
  register struct port_dcl *p;
{
  int      p_length;

  /********************************************************************/
  /* setup data and routing pointers                                  */
  /********************************************************************/

	if( p->sta_ptr->ri_length < 3 )
		p->sta_ptr->ri_length = 0;

  p->d.data_ptr = MTOD(p->m, caddr_t);
  p->ri.ptr_ri = ((p->d.send_data->ri_field)+p->sta_ptr->ri_length);

  /********************************************************************/
  /* build the dlc header of the xid                                  */
  /********************************************************************/

  p->d.send_data->phy_ctl_1 = p->sta_ptr->phy_ctl1;
  bcopy(p->sta_ptr->raddr, p->d.send_data->raddr, 6);
  bcopy(p->sta_ptr->laddr, p->d.send_data->laddr, 6);
 
  if                                   /* routing information field   
                                          present                     */
     (p->sta_ptr->ri_length > 0)
    {

      /****************************************************************/
      /* move routing information                                     */
      /****************************************************************/

/* LEHb defect 44499 */
      bcopy(&(p->sta_ptr->ri_field), &(p->d.send_data->ri_field[0]),
	 p->sta_ptr->ri_length);
/* LEHe */

      /****************************************************************/
      /* set routing information present flag                         */
      /****************************************************************/

      SETBIT(p->d.send_data->laddr[0], RI_PRESENT);
    } 
  p->ri.ri_sd->rsap = p->sta_ptr->ls_profile.rsap;
  p->ri.ri_sd->lsap = p->sap_ptr->sap_profile.local_sap;
  p->ri.ri_sd->ctl1 = UI_NO_PF;

  /********************************************************************/
  /* set length of dlc header mbuf                                    */
  /********************************************************************/

  p->m->m_len = p->sta_ptr->ri_length+FDL_ROUTING_OFFSET+3;

/* <<< feature CDLI >>> */
  /********************************************************************/
  /* call the write send command generator, with the ui command       */
  /* buffer address.                                                  */
  /********************************************************************/

  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
}                                      /* end g_build_datagram;       */

/* LEHb defect XXX */
/* deleted fdl_buf_build */
/* LEHe */

/* <<< feature CDLI >>> */
/* removed fdl_setfa routine */
/* <<< end feature CDLI >>> */

g_trgen(p)
  register struct port_dcl *p;
{
  u_char   tphy1;
/* <<< feature CDLI >>> */
/* <<< removed traddr[6] >>> */
/* <<< removed tladdr[6] >>> */
/* <<< end feature CDLI >>> */
  u_char   temp_rsap;

/* LEHb defect XXX */
  /********************************************************************/
  /* save frame control byte (masking off invalid bits)               */
  /********************************************************************/

  tphy1 = ((p->rcv_data.phy_ctl_1 & 0x07 ) | 0x50);
/* LEHe */

/* defect 162732 */
  /********************************************************************/
  /* The received packet may have routing inserted, but the           */
  /* test_routing routine may have reset the routing length to zero.  */
  /* So, calculate the start of data pointer based on the i-field.    */
  /********************************************************************/
  p->d.data_ptr = (p->i.i_field_ptr - p->common_cb.ri_length
                                    - FDL_ROUTING_OFFSET -3);

  /* re-adjust the mbuf's data pointer and length */
  p->m->m_data = (caddr_t)p->d.data_ptr;
  p->m->m_len = p->lpdu_length + FDL_ROUTING_OFFSET + p->common_cb.ri_length;
 
/* end defect 162732 */

/* <<< feature CDLI >>> */
/* <<< removed ((p->rcv_data.ri_field[0] & 0x1f) == 2) >>> */
/* <<< (it may be a 2-byte ARB due to a SRB command):
/* <<< end feature CDLI >>> */

#ifdef   DEBUG
  if (p->debug)
    printf("ri_length = %d\n", p->common_cb.ri_length);
    printf("data ptr=%x\n", p->d.data_ptr);
#endif

  /********************************************************************/
  /* update routing information pointer                               */
  /********************************************************************/

  p->ri.ptr_ri = p->d.send_data->ri_field + p->common_cb.ri_length;

#ifdef   DEBUG
  if (p->debug)
    printf("ptr_ri=%x\n", p->ri.ptr_ri);
#endif
 
  if                                   /* not a response for the null 
                                          sap, ie. a station exists   */
     (p->rcv_data.lsap != NULL_SAP)   /* defect 162732 */
    {

/* LEHb defect XXX */
				       /* set frame control to the station's
					  asynchronous frame priority */
      tphy1 = p->sta_ptr->phy_ctl1;
/* LEHe */

      if                               /* test commands received      
                                          counter not at maximum value*/
         (p->sta_ptr->ras_counters.counters.test_cmds_rec != DLC_ERR)
        {

          /************************************************************/
          /* increment count of test commands received                */
          /************************************************************/

          ++p->sta_ptr->ras_counters.counters.test_cmds_rec;
        } 
    } 

  /********************************************************************/
  /* setup frame control field                                        */
  /********************************************************************/

  bzero (p->d.send_buf_rsp, 3);
  p->d.send_buf_rsp->phy_ctl_1 = tphy1;

/* <<< feature CDLI >>> */
/* <<< this is a rework of token-ring apar IX34430 defect 82097 >>> */
  /********************************************************************/
  /* move the received source address to the send destination address */
  /* note: any routing flag was already reset in kproc rcv_completion */
  /********************************************************************/

  bcopy(&p->rcv_data.raddr[0], &p->d.send_buf_rsp->raddr[0], 6);

  /********************************************************************/
  /* force the send source address to be the local card address       */
  /********************************************************************/

  bcopy(&p->common_cb.local_addr[0], &p->d.send_buf_rsp->laddr[0], 6);

/* <<< end feature CDLI >>> */

  /********************************************************************/
  /* swap the remote and destination sap fields. temp_sap =           */
  /* rcv_data.rsap;                                                   */
  /********************************************************************/

  temp_rsap = p->ri.ri_rcv->rsap;
  p->ri.ri_sbp->lsap = (p->ri.ri_rcv->lsap|RESP_ON);
  p->ri.ri_sbp->rsap = temp_rsap;

  /********************************************************************/
  /* build the remainder of the data link header.                     */
  /********************************************************************/

  if                                   /* the received buffer         
                                          indicates overflow          */
     (((int)p->m->m_nextpkt & DLC_OFLO) == DLC_OFLO)
    {

      /****************************************************************/
      /* set the buffer's data length to the packet length.           */
      /****************************************************************/

      p->m->m_len = p->common_cb.ri_length+FDL_ROUTING_OFFSET+3;
    } 
 
  if                                   /* routing information field   
                                          present                     */
     (p->common_cb.ri_length > 0)
    {

      /****************************************************************/
      /* move routing information to correct place                    */
      /****************************************************************/

/* LEHb defect 44499 */
      bcopy(&(p->common_cb.ri_field), &(p->d.send_data->ri_field[0]),
	 p->common_cb.ri_length);
/* LEHe */

      /****************************************************************/
      /* indicate packet contains routing info                        */
      /****************************************************************/

      SETBIT(p->d.send_data->laddr[0], RI_PRESENT);
    } 

/* <<< feature CDLI >>> */
  /********************************************************************/
  /* call the write send command generator, with the test command     */
  /* buffer address.                                                  */
  /********************************************************************/

  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
}                                      /* end g_trgen                 */

g_send_frmr(p,fr_reason,fr_pf)
  register struct port_dcl *p;
  u_char   fr_reason;
  u_char   fr_pf;
{
  u_char   temp_sap;
  u_char   temp_addr[6];
  int      a;
  int      i;
  int      j;

  /********************************************************************/
  /* setup routing information pointer using data pointer             */
  /********************************************************************/

  if( p->common_cb.ri_length < 3 )
		p->common_cb.ri_length = 0;

  p->ri.ptr_ri = (p->d.send_data->ri_field)+p->sta_ptr->ri_length;

  /********************************************************************/
  /* swap the remote and destination address fields.                  */
  /********************************************************************/

  bcopy(p->rcv_data.raddr, temp_addr, 6);
  bcopy(p->rcv_data.laddr, p->d.send_buf_rsp->laddr, 6);
  bcopy(temp_addr, p->d.send_buf_rsp->raddr, 6);

  /********************************************************************/
  /* swap the remote and destination sap fields.                      */
  /********************************************************************/

  temp_sap = (p->rcv_data.rsap&RESP_OFF);
  p->ri.ri_sbp->lsap = (p->rcv_data.lsap|RESP_ON);
  p->ri.ri_sbp->rsap = temp_sap;

  /********************************************************************/
  /* build the remainder of the data link header.                     */
  /********************************************************************/

  p->ri.ri_sbp->ctl1 = (FRMR_NO_PF|fr_pf);

  /********************************************************************/
  /* build the frmr i-field.                                          */
  /********************************************************************/

  p->i.i_field_ptr = &(p->ri.ri_sbp->ctl2);/* un_info                 */
 
  if                                   /* the received packet was an  
                                          unnumbered frame            */
     ((p->rcv_data.ctl1&0x03) == UNNUMBERED)

    /******************************************************************/
    /* set the frmr control byte-2 to zero.                           */
    /******************************************************************/

    p->i.i_field_frmr->frmr_ctl2 = 0;
 
  else                                 /* set the frmr control byte-2 
                                          to received control         */

    /******************************************************************/
    /* byte-2.                                                        */
    /******************************************************************/

    p->i.i_field_frmr->frmr_ctl2 = p->rcv_data.ctl2;

  /********************************************************************/
  /* set the frmr control byte-1 to the received control byte-1.      */
  /********************************************************************/

  p->i.i_field_frmr->frmr_ctl1 = p->rcv_data.ctl1;

  /********************************************************************/
  /* set the frmr returned vs and vr to the current station vs and vr.*/
  /********************************************************************/

  p->i.i_field_frmr->frmr_vs = p->sta_ptr->vs;
  p->i.i_field_frmr->frmr_vr = p->sta_ptr->vr;
 
  if                                   /* the received packet was a   
                                          response                    */
     (TSTBIT(p->rcv_data.rsap, RESPONSE) == TRUE)

    /******************************************************************/
    /* set the frmr c/r bit in the returned vr field.                 */
    /******************************************************************/

    SETBIT(p->i.i_field_frmr->frmr_vr, FRMR_RESPONSE);
 
  else                                 /* reset the frmr c/r bit in   
                                          the returned vr field.      */
    CLRBIT(p->i.i_field_frmr->frmr_vr, FRMR_RESPONSE);

  /********************************************************************/
  /* set the frmr reason code to the supplied value.                  */
  /********************************************************************/

  p->i.i_field_frmr->frmr_reason = fr_reason;

  /********************************************************************/
  /* set the packet length to minimum packet size.                    */
  /********************************************************************/

  p->m->m_len = FDL_ROUTING_OFFSET+3+p->sta_ptr->ri_length+sizeof(struct i_field_frmr)
     ;
 
  if                                   /* routing information present */
     (p->sta_ptr->ri_length > 0)
    {

      /****************************************************************/
      /* set routing information present flag                         */
      /****************************************************************/

      CLRBIT(p->d.send_data->laddr[0], RI_PRESENT);
    } 

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
  int      if_size;
  int      index;
  struct ri_control_field *p_tri;      /* routing information control */
  struct mbuf *pt;

  /********************************************************************/
  /* setup data and routing information pointers save address of head */
  /* of buffer                                                        */
  /********************************************************************/

  pt = p->m;

  /********************************************************************/
  /* get address of start of cluster                                  */
  /********************************************************************/

  MTOCL(pt);
  p->d.data_ptr = MTOD(pt, char *);

  /********************************************************************/
  /* build the header area of the test packet.                        */
  /********************************************************************/

  bcopy(p->sta_ptr->raddr, p->d.send_data->raddr, 6);
  bcopy(p->sta_ptr->laddr, p->d.send_data->laddr, 6);
 
	if( p->common_cb.ri_length < 3 )
			p->common_cb.ri_length = 0;

  if                                   /* routing information present */
     (p->sta_ptr->ri_length > 0)
    {

      /****************************************************************/
      /* copy routing info data;                                      */
      /****************************************************************/

/* LEHb defect 44499 */
      bcopy(&(p->sta_ptr->ri_field), &(p->d.send_data->ri_field[0]),
	 p->sta_ptr->ri_length);
/* LEHe */

      /****************************************************************/
      /* set route in routing information control ot all route        */
      /* broadcast and aet length to 2 to accumulate new routing data */
      /****************************************************************/

      p->sta_ptr->ri_length = 2;
      p_tri = (struct ri_control_field *)&p->d.send_data->ri_field;
      p_tri->all_route = TRUE;
      p_tri->single_route = FALSE;
/* LEHb defect 44499 */
      p_tri->direction = 0;
      p_tri->largest_field = 7;
/* LEHe */

      /****************************************************************/
      /* set routing length in packet to indicate route will be       */
      /*  be determined thru packet passing thru bridges              */
      /****************************************************************/
         p_tri->ri_lth = 2;
         
      /****************************************************************/
      /* set routing information present                              */
      /****************************************************************/

      SETBIT(p->d.send_data->laddr[0], RI_PRESENT);
    } 

  /********************************************************************/
  /* setup routing information pointer                                */
  /********************************************************************/

  p->ri.ptr_ri = (p->d.send_data->ri_field)+p->sta_ptr->ri_length;

  /********************************************************************/
  /* get addressability to the i-field area of the buffer.            */
  /********************************************************************/

  p->i.i_field_ptr = &(p->ri.ri_sd->ctl2);/* un_info                  */
 
  if                                   /* maximum i-field size is     
                                          greater than 256            */
     (p->sta_ptr->ls_profile.maxif > 256)
    if_size = 256;
 
  else                                 /* the max i-field is less than
                                          or equal to 256.            */

    /******************************************************************/
    /* set the i-field size to the current max i-field size.          */
    /******************************************************************/

    if_size = p->sta_ptr->ls_profile.maxif;

  /********************************************************************/
  /* copy physical control bytes to buffer                            */
  /********************************************************************/

  p->d.send_data->phy_ctl_1 = p->sta_ptr->phy_ctl1;

  /********************************************************************/
  /* move rsap and lsap                                               */
  /********************************************************************/

  p->ri.ri_sd->rsap = p->sta_ptr->rsap;
  p->ri.ri_sd->lsap = (p->sta_ptr->lsap&RESP_OFF);
  p->ri.ri_sd->ctl1 = TEST;

/* <<< feature CDLI >>> */
  /********************************************************************/
  /* set the buffer data length to the maximum i-field length plus the*/
  /* header length of an unnumbered command packet.                   */
  /********************************************************************/
/* <<< removed "round up to next full word">>> */
/* <<< end feature CDLI >>> */

  p->m->m_len = (if_size+FDL_ROUTING_OFFSET+3+p->sta_ptr->ri_length);

  /********************************************************************/
  /* for index = 1 to the i-field size, build a test i-field of x'00' */
  /* through x'ff' repeating.                                         */
  /********************************************************************/
 

  for (index = 1; index <= if_size; index++)
    {

      /****************************************************************/
      /* set the data equal to the index value modulo 256.            */
      /****************************************************************/

      *(p->i.i_field_ptr)++ = (index-1)%256;
    }                                  /* end for index;              */
 
  if                                   /* in abme checkpointing state */
     (p->sta_ptr->checkpointing == TRUE)
    {

      /****************************************************************/
      /* stack the test command for future transmission.              */
      /****************************************************************/

      p->sta_ptr->vc = TEST_ENA;
    } 
 
  else                                 /* not abme checkpointing.     */
    {

      /****************************************************************/
      /* call the send test command routine.                          */
      /****************************************************************/

      send_test_cmd(p);
    } 
}                                      /* end g_test_link_cmd         */

g_find_self_gen(p)
  register struct port_dcl *p;
{
  int      i;
/* <<< feature CDLI >>> */
/* <<< removed cmd_size >>> */
/* <<< end feature CDLI >>> */
  int      vec_size;
  int      ipri;
  struct ri_control_field ri_control;
#ifdef   DEBUG
 

  if (p->debug)
    printf("g_find_self_gen\n");
#endif
 
  if                                   /* resolve procedures          */
     ((p->sap_ptr->sap_profile.flags&DLC_ESAP_ADDR) != 0)
    {
#ifdef   DEBUG
 
      if (p->debug)
        printf("resolve\n");
#endif

      /****************************************************************/
      /* release add name query buffer                                */
      /****************************************************************/

      lanfree(p, p->sap_list[p->sapno].find_self_addr);

      /****************************************************************/
      /* clear buffer pointer                                         */
      /****************************************************************/

      p->sap_list[p->sapno].find_self_addr = 0;

      /****************************************************************/
      /* set the sap state = adding name.                             */
      /****************************************************************/

      p->sap_ptr->sap_state = SAP_OPEN_STATE;

      /****************************************************************/
      /* call add_name_cmpl to notify user                            */
      /****************************************************************/

      add_name_cmpl(p);
    } 
 
  else
    {

      /****************************************************************/
      /* set up send data pointer                                     */
      /****************************************************************/

      p->d.data_ptr = MTOD(p->m, caddr_t);

      /****************************************************************/
      /* setup value of physical control byte 1 and 2                 */
      /****************************************************************/

      p->d.send_data->phy_ctl_1 = 0x50;

/* LEHb defect XXX */
/* deleted phy_1 */
/* LEHe */

      /****************************************************************/
      /* build the source address                                     */
      /****************************************************************/

      bcopy(p->common_cb.local_addr, p->d.send_data->laddr, 6);
 
      if                               /* first call packet to be sent
                                          (local ring only)           */
         (p->sap_list[p->sapno].addn_retries == MAX_ADDN_REPOLLS)
        {

          /************************************************************/
          /* reset length of routing information field                */
          /************************************************************/

          p->common_cb.ri_length = 0;

          /************************************************************/
          /* setup routing information pointer                        */
          /************************************************************/

          p->ri.ptr_ri = (p->d.send_data->ri_field);

          /************************************************************/
          /* indicate no routing info present                         */
          /************************************************************/

          CLRBIT(p->d.send_data->laddr[0], RI_PRESENT);
        } 
 
      else                             /* broadcasting to all rings   */
        {

          /************************************************************/
          /* set length of routing information field to 2 bytes       */
          /************************************************************/

          p->common_cb.ri_length = 2;

          /************************************************************/
          /* setup ptr_ri to point 2-bytes past start of routing field*/
          /************************************************************/

          p->ri.ptr_ri = (p->d.send_data->ri_field+2);

          /************************************************************/
          /* clear routing control field                              */
          /************************************************************/

          bzero(&ri_control, 2);

          /************************************************************/
          /* build the routing field in the stack set all-rings       */
          /* broadcast in routing information control field           */
          /************************************************************/

          ri_control.all_route = TRUE;
          ri_control.single_route = FALSE;

          /************************************************************/
          /* set forward scan of ri field                             */
          /************************************************************/

          ri_control.direction = 0;

          /************************************************************/
          /* set length of routing information field to 2             */
          /************************************************************/

          ri_control.ri_lth = 2;

          /************************************************************/
          /* set largest field to default value for all rings         */
          /* broadcast                                                */
          /************************************************************/
/* <<< feature CDLI >>> */
          ri_control.largest_field = 3;
/* <<< end feature CDLI >>> */

          /************************************************************/
          /* end build routing field in the stack copy routing into   */
          /* the send buffer                                          */
          /************************************************************/

/* LEHb defect 44499 */
	  bcopy(&ri_control, &(p->d.send_data->ri_field[0]), 2);
/* LEHe */

          /************************************************************/
          /* indicate routing info is present                         */
          /************************************************************/

          SETBIT(p->d.send_data->laddr[0], RI_PRESENT);
        } 
 
      for (i = 0; i < 6; i++)
        {
          p->d.send_data->raddr[i] = 0xff;
        } 

      /****************************************************************/
      /* set dsap and ssap to use discovery value                     */
      /****************************************************************/

      p->ri.ri_sd->lsap = DISCOVERY_SAP;
      p->ri.ri_sd->rsap = DISCOVERY_SAP;

      /****************************************************************/
      /* set command field to unnumbered information command          */
      /****************************************************************/

      p->ri.ri_sd->ctl1 = UI_NO_PF;

      /****************************************************************/
      /* call fdlbuild to build find vector                           */
      /****************************************************************/

      vec_size = build_vector(p, &(p->ri.ri_sd->ctl2), 
         FIND_VECTOR_KEY, (p->sapno|SAP_CORR_MASK), DISCOVERY_SAP);

      /****************************************************************/
      /* set the data length to discovery packet length plus routing  */
      /****************************************************************/

      p->m->m_len = p->common_cb.ri_length+vec_size+FDL_ROUTING_OFFSET+3;

/* <<< feature CDLI >>> */
      /****************************************************************/
      /* set the M_BCAST mbuf flag to indicate that this is a         */
      /* broadcast packet being sent.                                 */
      /****************************************************************/
      p->m->m_flags |= M_BCAST;

      /****************************************************************/
      /* call the write send command generator, with the find remote  */
      /* command buffer address.                                      */
      /****************************************************************/
      lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
    } 
}                                      /* end g_find_self_gen         */

g_find_remote_gen(p)
  register struct port_dcl *p;
{
  int      i;
  ulong_t  arc;
  int      vec_size = 0;
  struct ri_control_field ri_control;

  /********************************************************************/
  /* get addressability back to calling station                       */
  /********************************************************************/

  p->sta_ptr = (struct station_cb *)p->station_list[p->stano].sta_cb_addr;

  /********************************************************************/
  /* set up send data pointer                                         */
  /********************************************************************/

  p->d.data_ptr = MTOD(p->m, caddr_t);
  bzero( p->d.data_ptr, 50 );

  /********************************************************************/
  /* setup value of physical control byte 1 and 2                     */
  /********************************************************************/
/* LEHb defect XXX */
  p->d.send_data->phy_ctl_1 = p->sta_ptr->phy_ctl1;
/* LEHe */

  /********************************************************************/
  /* build the source address                                         */
  /********************************************************************/

  bcopy(p->common_cb.local_addr, p->d.send_data->laddr, 6);
 
  if                                   /* first call packet to be sent
                                          (local ring only)           */
     (p->station_list[p->stano].call_retries == p->sta_ptr->ls_profile.max_repoll)
    {

      /****************************************************************/
      /* reset length of routing information field                    */
      /****************************************************************/

      p->common_cb.ri_length = 0;

      /****************************************************************/
      /* setup routing information pointer                            */
      /****************************************************************/

      p->ri.ptr_ri = (p->d.send_data->ri_field);

      /****************************************************************/
      /* indicate no routing info present                             */
      /****************************************************************/

      CLRBIT(p->d.send_data->laddr[0], RI_PRESENT);
    } 
 
  else                                 /* broadcasting to all rings   */
    {

      /****************************************************************/
      /* set length of routing information field to 2 bytes           */
      /****************************************************************/

      p->common_cb.ri_length = 2;

      /****************************************************************/
      /* setup ptr_ri to point 2-bytes past start of routing field    */
      /****************************************************************/

      p->ri.ptr_ri = (p->d.send_data->ri_field+2);

      /****************************************************************/
      /* clear routing control field                                  */
      /****************************************************************/

      bzero(&ri_control, 2);

      /****************************************************************/
      /* build the routing field in the stack set single route        */
      /* broadcast in the routin control field                        */
      /****************************************************************/

      ri_control.all_route = TRUE;
      ri_control.single_route = TRUE;
 
      if                               /* using all route broadcast to
                                          find remote station         */
         (p->station_list[p->stano].discv_allr == TRUE)
        {

          /************************************************************/
          /* use all route broadcast in packet to find remote station */
          /************************************************************/

          ri_control.all_route = TRUE;
          ri_control.single_route = FALSE;
        } 

      /****************************************************************/
      /* set forward scan of ri field                                 */
      /****************************************************************/

      ri_control.direction = 0;

      /****************************************************************/
      /* set length of routing information field to 2                 */
      /****************************************************************/

      ri_control.ri_lth = 2;

      /****************************************************************/
      /* set largest field to default value for all rings broadcast   */
      /****************************************************************/
/* <<< feature CDLI >>> */
      ri_control.largest_field = 3;
/* <<< end feature CDLI >>> */

      /****************************************************************/
      /* end build routing field in the stack copy routing into the   */
      /* send buffer                                                  */
      /****************************************************************/

/* LEHb defect 44499 */
      bcopy(&ri_control, &(p->d.send_data->ri_field[0]), 2);
/* LEHe */

      /****************************************************************/
      /* indicate routing info is present                             */
      /****************************************************************/

      SETBIT(p->d.send_data->laddr[0], RI_PRESENT);
    } 
 
  if                                   /* station using resolve       */
     ((p->sta_ptr->ls_profile.flags&DLC_SLS_ADDR) != 0)
    {
#ifdef   DEBUG
 
      if (p->debug)
        printf("fdlmod:using resolve\n");
#endif

      /****************************************************************/
      /* set command field to test command                            */
      /****************************************************************/

      p->ri.ri_sd->ctl1 = TEST;

      /****************************************************************/
      /* set remote sap value to resolve sap                          */
      /****************************************************************/

      p->ri.ri_sd->rsap = RESOLVE_SAP;

      /****************************************************************/
      /* get local sap value from sap profile                         */
      /****************************************************************/

      p->ri.ri_sd->lsap = p->sap_ptr->sap_profile.local_sap;

      /****************************************************************/
      /* get the remote address from station profile                  */
      /****************************************************************/

      bcopy(p->sta_ptr->ls_profile.raddr_name, p->d.send_data->raddr, 6);

      /****************************************************************/
      /* set the data length to unnumbered packet length plus routing */
      /****************************************************************/

      p->m->m_len = p->common_cb.ri_length+FDL_ROUTING_OFFSET+3;
    } 
 
  else
    {                                  /* using discovery             */
#ifdef   DEBUG
 
      if (p->debug)
        printf("fdlmod:using discovery\n");
#endif
 
      for (i = 0; i < 6; i++)
        {
          p->d.send_data->raddr[i] = 0xff;
        } 
 
      if                               /* name found in cache from    
                                          previous search             */
         (p->sta_ptr->sta_cache == CACHE_NAME)
        {

          /************************************************************/
          /* use index from previous search to copy address into      */
          /* packet                                                   */
          /************************************************************/

          bcopy(p_to_cache->cache_data[p_to_cache->name_index
             [p->sta_ptr->cache_pindex]].address, 
             p->d.send_data->raddr, 6);
        } 
 
      else
 
        if                             /* cache not previously        
                                          searched                    */
           (p->sta_ptr->sta_cache != CACHE_WRONG_NAME)
          {

            /**********************************************************/
            /* call search cache to find card address of remote name  */
            /**********************************************************/

            arc = search_cache(p_to_cache->name_index, 
               p_to_cache->n_entries, 
               p->sta_ptr->ls_profile.raddr_name, 
               p_to_cache->cache_data);
 
            if                         /* remote name is in cache     */
               (arc != -1)
              {

                /******************************************************/
                /* replace broadcast address in packet with the acutal*/
                /* card address                                       */
                /******************************************************/

                bcopy(p_to_cache->cache_data[p_to_cache->name_index
                   [arc]].address, p->d.send_data->raddr, 6);

                /******************************************************/
                /* set flag indicating packet contains name from cache*/
                /******************************************************/

                p->sta_ptr->sta_cache = CACHE_NAME;

                /******************************************************/
                /* save name index                                    */
                /******************************************************/

                p->sta_ptr->cache_pindex = arc;
              } 
          } 

        /**************************************************************/
        /* set dsap and ssap to use discovery value                   */
        /**************************************************************/

      p->ri.ri_sd->lsap = DISCOVERY_SAP;
      p->ri.ri_sd->rsap = DISCOVERY_SAP;

      /****************************************************************/
      /* set command field to unnumbered information command          */
      /****************************************************************/

      p->ri.ri_sd->ctl1 = UI_NO_PF;

      /****************************************************************/
      /* call fdlbuild to build find vector                           */
      /****************************************************************/

      vec_size = build_vector(p, &(p->ri.ri_sd->ctl2), 
         FIND_VECTOR_KEY, p->stano, p->sap_ptr->sap_profile.local_sap)
         ;

      /****************************************************************/
      /* set the data length to discovery packet length plus routing  */
      /****************************************************************/

      p->m->m_len = p->common_cb.ri_length+vec_size+FDL_ROUTING_OFFSET+3;
    }

/* <<< feature CDLI >>> */
  /********************************************************************/
  /* set the M_BCAST mbuf flag to indicate that this is a             */
  /* broadcast packet being sent.                                     */
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
  ulong    vec_size;
#ifdef   DEBUG
 

  if (p->debug)
    printf("g_dup_add_name_vector\n");
#endif

  /********************************************************************/
  /* set up i-field data pointer                                      */
  /********************************************************************/

  p->d.data_ptr = MTOD(p->m, caddr_t);

  /********************************************************************/
  /* setup routing information pointer                                */
  /********************************************************************/

  p->ri.ptr_ri = (p->d.send_data->ri_field)+p->common_cb.ri_length;

  /********************************************************************/
  /* setup value of physical control byte 1                           */
  /********************************************************************/

  p->d.send_data->phy_ctl_1 = 0x50;

  /********************************************************************/
  /* setup value of physical control byte 2                           */
  /********************************************************************/

  /*------------------------------------------------------------------*/
  /* alter the find name query i-field into a name found response     */
  /* packet (everything already set up except the key).               */
  /*------------------------------------------------------------------*/
  /* move the remote source address to the destination address field. */
  /********************************************************************/

  bcopy(p->rcv_data.raddr, p->d.send_buf_rsp->raddr, 6);

  /********************************************************************/
  /* fill in the local address in the source address field.           */
  /********************************************************************/

  bcopy(p->common_cb.local_addr, p->d.send_buf_rsp->laddr, 6);
 
  if                                   /* routing information field   
                                          present                     */
     (p->common_cb.ri_length > 0)
    {

      /****************************************************************/
      /* move routing information to correct place                    */
      /****************************************************************/

/* LEHb defect 44499 */
      bcopy(&(p->common_cb.ri_field), &(p->d.send_data->ri_field[0]),
	 p->common_cb.ri_length);
/* LEHe */

      /****************************************************************/
      /* indicate packet contains routing info                        */
      /****************************************************************/

      SETBIT(p->d.send_data->laddr[0], RI_PRESENT);
    } 

  /********************************************************************/
  /* call fdlbuild to build found vector                              */
  /********************************************************************/

  vec_size = build_vector(p, &(p->ri.ri_sd->ctl2), FOUND_VECTOR_KEY, 
     p->correlator_vector.value, DISCOVERY_SAP);

  /********************************************************************/
  /* set total buffer data length                                     */
  /********************************************************************/

  p->m->m_len = FDL_ROUTING_OFFSET+3+vec_size+p->common_cb.ri_length;

  /********************************************************************/
  /* set dsap and ssap to use discovery value                         */
  /********************************************************************/

  p->ri.ri_sd->lsap = DISCOVERY_SAP;
  p->ri.ri_sd->rsap = DISCOVERY_SAP;

  /********************************************************************/
  /* set command field to unnumbered information command              */
  /********************************************************************/

  p->ri.ri_sd->ctl1 = UI_NO_PF;

/* <<< feature CDLI >>> */
  /********************************************************************/
  /* call the write send command generator, with the add name         */
  /* response buffer address, disabling link trace.                   */
  /********************************************************************/

  p->stano = NO_MATCH;                 /* disables trace              */
  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
}                                      /* end g_dup_add_name_vector   */

static u_char ieee_xid[] = 
      {
        0x81, 0x03, 0xfe
      } ;

g_found_vector_gen(p,input_lsap)
  register struct port_dcl *p;
  u_char   input_lsap;
{
/* <<< feature CDLI >>> */
/* <<< removed cmd_size >>> */
/* <<< end feature CDLI >>> */
  int      shift_size;
  int      ret;
/* <<< defect 128100 >>> */
/* <<< removed tphy1 >>> */
/* <<< removed tphy2 >>> */
/* <<< end defect 128100 >>> */
  u_char   trsap;
  u_char   tlsap;
  u_char   traddr[6];
  u_char   tctl1;
 
/* <<< defect 128100 >>> */
/* <<< removed  p->sta_ptr = p->sta_ptr; >>> */
/* <<< end defect 128100 >>> */
  if                                   /* link trace activated        */

     ((p->sta_ptr->ls_profile.flags&DLC_TRCO) == DLC_TRCO)

    /******************************************************************/
    /* put an entry into the link trace                               */
    /******************************************************************/

    lanrcvtr(p);

/* <<< defect 128100 >>> */
/* <<< removed tphy1 = p->rcv_data.phy_ctl_1; >>> */
/* <<< end defect 128100 >>> */

  bcopy(p->rcv_data.raddr, traddr, 6);

  /********************************************************************/
  /* save source and destination saps                                 */
  /********************************************************************/

  trsap = p->rcv_data.rsap;
  tlsap = p->rcv_data.lsap;
  tctl1 = p->rcv_data.ctl1;

  /********************************************************************/
  /* update buffer offset value                                       */
  /********************************************************************/

  shift_size = p->common_cb.ri_length;
  p->m->m_data += shift_size;

  /********************************************************************/
  /* update data pointer to include routing information offset        */
  /********************************************************************/

  p->d.data_ptr = p->d.data_ptr+shift_size;

  /********************************************************************/
  /* update routing information pointer                               */
  /********************************************************************/

/* <<< defect 128100 >>> */
  p->ri.ptr_ri = (char *)&(p->d.send_data->ri_field) + p->common_cb.ri_length;

  /********************************************************************/
  /* setup header control fields                                      */
  /********************************************************************/

  p->d.send_buf_rsp->phy_ctl_1 = p->sta_ptr->phy_ctl1;
/* <<< end defect 128100 >>> */

  /********************************************************************/
  /* move the remote source address to destination address field.     */
  /********************************************************************/

  bcopy(traddr, p->d.send_buf_rsp->raddr, 6);

  /********************************************************************/
  /* fill in the local address in the source address field.           */
  /********************************************************************/

  bcopy(p->common_cb.local_addr, p->d.send_buf_rsp->laddr, 6);

/* <<< defect 128100 >>> */
  /********************************************************************/
  /* retrieve the dlh control byte.                                   */
  /********************************************************************/

  p->ri.ri_sd->ctl1 = tctl1;
/* <<< end defect 128100 >>> */

  /********************************************************************/
  /* retrieve the pending station's lsap.                             */
  /********************************************************************/

  p->ri.ri_sd->lsap = input_lsap;

  /********************************************************************/
  /* turn response bit on in lsap byte to indicate response           */
  /********************************************************************/

  p->ri.ri_sd->lsap = p->ri.ri_sd->lsap|RESP_ON;
 
  if                                   /* routing information field   
                                          present                     */
     (p->common_cb.ri_length > 0)
    {

      /****************************************************************/
      /* move routing to send packet                                  */
      /****************************************************************/

/* LEHb defect 44499 */
      bcopy(&(p->common_cb.ri_field), &(p->d.send_buf_rsp->ri_field[0]),
	 p->common_cb.ri_length);
/* LEHe */

      /****************************************************************/
      /* set routing present flag in remote addr                      */
      /****************************************************************/

      SETBIT(p->d.send_buf_rsp->laddr[0], RI_PRESENT);
    } 
 
  if                                   /* not ieee xid data           */
     (p->ri.ri_sd->ctl1 != XID)
    {
      ret = 0;
 
      if                               /* discovery procedures        */
         (tlsap == DISCOVERY_SAP)
        {
          trsap = DISCOVERY_SAP;

          /************************************************************/
          /* turn response bit off in lsap byte                       */
          /************************************************************/

          p->ri.ri_sd->lsap = p->ri.ri_sd->lsap&RESP_OFF;

          /************************************************************/
          /* call fdlbuild to build found vector                      */
          /************************************************************/

          ret = build_vector(p, &(p->ri.ri_sd->ctl2), 
             FOUND_VECTOR_KEY, p->correlator_vector.value, 
             DISCOVERY_SAP);
        } 
 
      else                             /* restore command value       */
        {
          p->ri.ri_sd->ctl1 = tctl1;

          /************************************************************/
          /* force packet to go to null sap                           */
          /************************************************************/

          p->ri.ri_sd->lsap = 0x01;
        } 

      /****************************************************************/
      /* set total buffer data length                                 */
      /****************************************************************/

      p->m->m_len = FDL_ROUTING_OFFSET+3+ret+p->common_cb.ri_length;
    } 
 
  else                                 /* set size of xid total buffer
                                          length                      */
    {

      /****************************************************************/
      /* sizeof ieee_xid = 3                                          */
      /****************************************************************/

      p->m->m_len = p->common_cb.ri_length+FDL_ROUTING_OFFSET+3+3;

      /****************************************************************/
      /* move ieee xid data to ifield                                 */
      /****************************************************************/

      bcopy(ieee_xid, p->ri.ri_sd->ctl2, 3);
 
      if                               /* packet for sna sap          */
         ((trsap == 0x04) && (input_lsap == 0x04))

        /**************************************************************/
        /* force packet to go to null sap                             */
        /**************************************************************/

        p->ri.ri_sd->lsap = 0x01;
    } 

  /********************************************************************/
  /* build the remainder of the data link header.                     */
  /********************************************************************/

  p->ri.ri_sd->rsap = trsap;

/* <<< feature CDLI >>> */
  /********************************************************************/
  /* call the write send command generator, with the add name         */
  /* response buffer address.                                         */
  /********************************************************************/

/* <<< feature CDLI >>> */
/* <<< removed cmd_size >>> */
/* <<< end feature CDLI >>> */
  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
}                                      /* end g_found_vector_gen      */
#endif                                  /* TRLORFDDI                         */
