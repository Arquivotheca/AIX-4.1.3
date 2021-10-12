static char sccsid[] = "@(#)99	1.60.1.23  src/bos/kernext/dlc/lan/lan.c, sysxdlcg, bos412, 9446B 11/15/94 16:42:55";

/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
                                                                      */

#include <sys/fp_io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <net/spl.h>
#include <sys/mbuf.h>
#include <sys/file.h>
#include <sys/malloc.h>
#include <sys/errno.h>
#include <sys/trchkid.h>
#include <sys/devinfo.h>
#include "dlcadd.h"
#include <sys/gdlextcb.h>

/* defect 122577 */
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
/* end defect 122577 */

/* <<< feature CDLI >>> */
#include <sys/ndd.h>
#include <sys/ndd_var.h>
#include <sys/cdli.h>
/* <<< end feature CDLI >>> */

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
#include "lanmem.h"
#include "lanstlst.h"
#include "lansplst.h"
#include "lanrstat.h"
#include "lanport.h"
#define  NORMAL        0x00
init_cb(p)
  register struct port_dcl *p;
{
  int found,index,procno,rtn;
  int rc;

  /********************************************************************/
  /* set monitor trace as enabled for visibility                      */
  /********************************************************************/

  p->common_cb.ena_word = M_TRC_ENA;   /* enable trace                */
  p->common_cb.top = &p->common_cb.trace[0];
  p->common_cb.tend = &p->common_cb.trace[TMAX];
  p->common_cb.tptr = p->common_cb.top;

  /********************************************************************/
  /* set the monitor trace pointers. set the state of the physical    */
  /* link = closed.                                                   */
  /********************************************************************/

  p->common_cb.plc_state = PLC_CLOSED;

  /********************************************************************/
  /* set any plc return code = NORMAL.                                */
  /********************************************************************/

  p->common_cb.plc_retcode = DLC_OK;

  /********************************************************************/
  /* set the station local busy counter = 0.                          */
  /********************************************************************/

  p->common_cb.lbusy_ctr = 0;

  /********************************************************************/
  /* reset the sap list.                                              */
  /********************************************************************/

  bzero(&p->sap_list[0], sizeof(p->sap_list));

  /********************************************************************/
  /* reset all sap timers. for index = 0 to 127 possible saps         */
  /********************************************************************/

  for (index = 0; index < 128; index++)
    {

      /****************************************************************/
      /* reset the manager's t1 (add name) timeout enabled indicators.*/
      /****************************************************************/

      p->sap_list[index].t1_ena = FALSE;

      /****************************************************************/
      /* set the manager's t1 timeout counters to a -1                */
      /****************************************************************/

      p->sap_list[index].t1_ctr = -1;
    } 

  /********************************************************************/
  /* reset the station list.                                          */
  /********************************************************************/

  bzero(&p->station_list[0], sizeof(p->station_list));

  /********************************************************************/
  /* reset all link station timers. for index = 0 to maximum number of*/
  /* stations                                                         */
  /********************************************************************/

  for (index = 0; index <= MAX_SESSIONS; index++)
    {

      /****************************************************************/
      /* reset the station's t1 thru t3 timeout enabled indicators.   */
      /****************************************************************/

      p->station_list[index].t1_ena = FALSE;
      p->station_list[index].t2_ena = FALSE;
      p->station_list[index].t3_ena = FALSE;

      /****************************************************************/
      /* set the station's t1 thru t3 timeout counters to a -1        */
      /****************************************************************/

      p->station_list[index].t1_ctr = -1;
      p->station_list[index].t2_ctr = -1;
      p->station_list[index].t3_ctr = -1;
    } 

  /********************************************************************/
  /* reset the manager's local busy timeout.                          */
  /********************************************************************/
/* LEHb defect 43788 */
/* delete 2 lines */
/* LEHe */

#ifdef   TRLORFDDI
  bzero(&(p->common_cb.ri_field), sizeof(p->common_cb.ri_field));
  p->common_cb.ri_length = 0;
#endif                                  /* TRLORFDDI                         */

  /********************************************************************/
  /* create receive ring queue and save pointer to ring queue         */
  /********************************************************************/

  p->dlc_port.rcv_ringq = (struct ring_queue *)dlc_rqcreate();
  p->loopback.lb_ring_addr = (char *)dlc_rqcreate();
 
  /* allocate the ring queue locks  - defect 127690 */
  lock_alloc (&(p->dlc_port.ringq_lock), LOCK_ALLOC_PIN, PORT_LOCK,
                                                       PORT_RINGQ_LOCK);
  simple_lock_init (&(p->dlc_port.ringq_lock));

  /* end defect 127690 */
  
} 

len_port_cb()
{
  return (sizeof(struct port_dcl));
} 

max_opens()
{
  return (254);
} 

pr_open(p)
  register struct port_dcl *p;
{

  int stat;

/* defect 122577 */
  simple_lock(&p->dlc_port.lock);
/* end defect 122577 */
  TRACE1(p, "PROb");     /* defect 167068 */

  /********************************************************************/
  /* open the physical link                                           */
  /********************************************************************/

  stat = open_plc(p);
  TRACE2(p, "PROe", stat);  /* defect 167068 */
/* defect 122577  */
  simple_unlock(&p->dlc_port.lock);
/* end defect 122577 */

  return (stat);
} 

pr_ioctl(devno,op,arg,devflag,cid)
  register dev_t devno;
  register int op;
  register ulong arg;
  register ulong devflag;
  register struct dlc_chan *cid;
{
  int status = 0;
  register struct port_dcl *p;


  ulong dlctype;                       /* lan type                    */
  ulong_t port_sta;                    /* station number and port     
                                          number                      */
  ulong_t *p_to_arg;                   /* pointer to input argument   
                                          values                      */

  p = (struct port_dcl *)cid->cb;

/* defect 122577 */
  simple_lock(&p->dlc_port.lock);
/* end defect 122577 */

  TRACE2(p, "PRIb", op);  /* defect 167068 */

  p->dlc_port.cid = cid;

  /********************************************************************/
  /* setup lan type and monitor trace type                            */
  /********************************************************************/

  dlctype = DLC_TRACE_ISND<<8;
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
  /* get link station number from input arg values                    */
  /********************************************************************/

  p_to_arg = (ulong_t *)arg;
  ++p_to_arg;
  p->stano = *p_to_arg;
  if                                   /* requested task doesn't need 
                                          a station number            */
     ((op == DLC_ENABLE_SAP) || (op == DLC_DISABLE_SAP) || (op == 
     DLC_START_LS) || (op == DLC_QUERY_SAP) || (op == DLC_ADD_GRP) ||
     (op ==DLC_DEL_GRP))

    /******************************************************************/
    /* indicate station not present                                   */
    /******************************************************************/

    p->stano = NO_MATCH;

  /********************************************************************/
  /* get station number in upper half word and get number from port   */
  /* name in lower half word                                          */
  /********************************************************************/

  #ifndef FDL
  port_sta = ((p->stano<<16)|p->dlc_port.namestr[3]);
  #elif FDL
  port_sta = ((p->stano<<16)|p->dlc_port.namestr[4]);
  #endif

/* call trchkgt to put input command entry in monitor trace           */

  trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, op, 0, 0, 0, port_sta);
  switch (op)
    {
      case  DLC_ENABLE_SAP :
        p->l.sap_config = (struct dlc_esap_arg *)arg;
        status = open_sap_cmd(cid->cb);
        break;
      case  DLC_DISABLE_SAP :
        p->l.sap_config = (struct dlc_esap_arg *)arg;
        status = close_sap_cmd(cid->cb);
        break;
      case  DLC_START_LS :
        p->l.ls_config = (struct dlc_sls_arg *)arg;
        status = enable_link_sta(cid->cb);
        break;
      case  DLC_HALT_LS :
        p->l.ls_config = (struct dlc_sls_arg *)arg;
        status = close_link_sta(cid->cb, arg);
        break;
      case  DLC_QUERY_SAP :
        status = query_sap_cmd(cid->cb, arg);
        break;
      case  DLC_QUERY_LS :
        status = query_link_sta(cid->cb, arg);
        break;
      case  DLC_TRACE :
      case  DLC_CONTACT :
      case  DLC_TEST :
      case  DLC_ALTER :
      case  DLC_ENTER_LBUSY :
      case  DLC_EXIT_LBUSY :
        status = control(op, cid->cb, arg);
        break;
#if defined(EDL) || defined(E3L) || defined(FDL) || defined(TRL)
      case  DLC_ADD_GRP :
        status = add_grp(cid->cb, arg, op);
        break;
#endif /* EDL, E3L, FDL, or TRL */

#if defined(EDL) || defined(E3L) || defined(FDL) || defined(TRL)
      case  DLC_DEL_GRP :
        status = add_grp(cid->cb, arg, op);
        break;
#endif /* EDL, E3L, FDL, or TRL */
#ifdef TRL
      case DLC_ADD_FUNC_ADDR :
/* <<< feature CDLI >>> */
	status = lan_setfa (cid->cb, arg, DLC_ADD_FUNC_ADDR);
	break;
      case DLC_DEL_FUNC_ADDR :
	status = lan_setfa (cid->cb, arg, DLC_DEL_FUNC_ADDR);
/* <<< end feature CDLI >>> */
	break;
#endif /* TRL */
      case  IOCINFO :
	status = get_iocinfo(cid->cb, arg);
	break;
      default  :

        /**************************************************************/
        /* log error - invalid ioctl operation                        */
        /**************************************************************/

        lanerrlg(p, ERRID_LAN0036, NON_ALERT, INFO_ERR, 0, FILEN, 
           LINEN);
        status = EINVAL;
        break;
    } 
  TRACE2(p, "PRIe", status);

/* defect 122577 */
  simple_unlock(&p->dlc_port.lock);
/* end defect 122577 */

    return (status);
} 

pr_write(uiop,cid,m,ext)
  register struct uio *uiop;
  register struct dlc_chan *cid;
  register struct mbuf *m;
  register struct dlc_io_ext *ext;
{
  register struct port_dcl *p;
    int stat = 0;

/* <<< THREADS >>> */
  tid_t tid;
/* <<< end THREADS >>> */


  p = (struct port_dcl *)cid->cb;

  /********************************************************************/
  /* check to see if this is the kproc doing the write                */
  /********************************************************************/
/* <<< THREADS >>> */
  if ((tid = thread_self()) != p->dlc_port.kproc_tid)
/* <<< end THREADS >>> */

    {
      
/* defect 122577 */
  simple_lock(&p->dlc_port.lock);
/* end defect 122577 */

      
      /****************************************************************/
      /* get addressability to the link stationno from the buffer.    */
      /****************************************************************/

      p->stano = ext->ls_corr;
    } 
  TRACE2(p, "PRWb", m);
  bcopy(ext, &(p->dlc_io_ext), sizeof(struct dlc_io_ext));
/* <<< THREADS >>> */
  if (tid == p->dlc_port.kproc_tid)
/* <<< end THREADS >>> */

    {

      /****************************************************************/
      /* Force the sap and link station correlators so that the user  */
      /* doesn't have to look them up or remember them.               */
      /****************************************************************/

      p->dlc_io_ext.ls_corr = p->stano;
      p->dlc_io_ext.sap_corr = p->sapno;
    } 
  if (ext->flags == DLC_NETD)
    {

      /****************************************************************/
      /* call test network data packet to test for valid sap and      */
      /* control byte                                                 */
      /****************************************************************/

      stat = test_netd(p, m);
    } 
  else
    {
      if                               /* the specified link station  
                                          CORRELATOR is valid         */
         ((p->stano > 0) && (p->stano < MAX_SESSIONS))
        {
          if                           /* the station index from the  
                                          buffer points to a valid    */

          /************************************************************/
          /* station, ie. the station control block exists            */
          /************************************************************/

             (p->station_list[p->stano].sta_cb_addr != 0)
            {
              if (uiop->uio_fmode&FNDELAY == FNDELAY) /* Defect 115926 */
                {
/* <<< feature CDLI >>> */
		  p->m = (struct mbuf *)m_gethdr(M_DONTWAIT, MT_HEADER);
                }
              else
                {
		  p->m = (struct mbuf *)m_gethdr(M_WAIT, MT_HEADER);
/* <<< end feature CDLI >>> */
                }
              if (p->m != NULL)
                {

                  /****************************************************/
                  /* concatenate user buffer and DLC buffer not using */
                  /* m_cat because it bcopies the data                */
                  /****************************************************/

                  p->m->m_next = m;

                  /****************************************************/
                  /* call the link station with command=write so that */
                  /* the buffer can be queued internally in the       */
                  /* station's retransmit queue.                      */
                  /****************************************************/

                  lansta(p, WRITENO);

                  /****************************************************/
                  /* see if the internal que is full                  */
                  /****************************************************/

		  if (p->sta_ptr->que_flag == TRUE &&
		      p->sta_ptr->ls       == LS_ABME) /* Defect 119467 */
                    {

                      /************************************************/
                      /* free only the dlc mbuf                       */
                      /************************************************/

                      m_free(p->m);
                      stat = EAGAIN;
                    } 
                } 
	      else                     /* no buffer available, hit the
					  wall                        */
		{
		lanerrlg(p, ERRID_LAN8010, NON_ALERT, INFO_ERR, DLC_SYS_ERR,
			 FILEN, LINEN);
		if (uiop->uio_fmode&FNDELAY == FNDELAY)
		  {
		  stat = EAGAIN;
		  }
		else
		  {
		  stat = ENOMEM;
		  }
		}
	    }
	  else                         /* error - the station control
					  block does not exist.       */
	    {
	      stat = EINVAL;
	    }
	}
      else                             /* error - the link station
					  CORRELATOR is not valid.    */
	{
	  stat = EINVAL;
	}
    }
  TRACE2(p, "PRWe", stat);
  return (stat);
}

test_netd(p,m)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  test_netd                                            */
/*                                                                    */
/* descriptive name:  test write network data packet                  */
/*                                                                    */
/*  function: checks the data packet for a valid sap and the control  */
/*            byte set to ui. transmits the data packet if those      */
/*            conditions are met; otherwise logs an error.            */
/*                                                                    */
/* input:  write network data packet                                  */
/*                                                                    */
/* output:  transmit packet.                                          */
/*          returns an errno or zero                                  */
/*                                                                    */
/*** end of specifications ********************************************/

  struct mbuf *m;
{
  int status = 0;
  int length_ri = 0;
  ulong dlctype;                       /* lan type                    */
#ifdef   TRLORFDDI
  struct ri_control_field ri_control;
#endif
#ifdef   DLC_DEBUG

  if (p->debug)
    printf("test_netd\n");
#endif
  p->m = m;

  /********************************************************************/
  /* setup pointer to start of data packet                            */
  /********************************************************************/

  p->d.data_ptr = MTOD(p->m, caddr_t);
#ifdef   TRLORFDDI
  if                                   /* routing information present */
     (TSTBIT(p->d.send_data->laddr[0], RI_PRESENT) == TRUE)
    {
/* LEHb defect 44499 */
      bcopy(&(p->d.send_data->ri_field[0]), &ri_control, 2);
/* LEHe */
      length_ri = ri_control.ri_lth;
    } 

  /********************************************************************/
  /* setup pointer to saps in data packet                             */
  /********************************************************************/

  p->ri.ptr_ri = (p->d.send_data->ri_field)+length_ri;
  if                                   /* control byte is ui          */
     ((p->ri.ri_sbp->ctl1 == UI) || (p->ri.ri_sbp->ctl1 == UI_NO_PF))
    {

      /****************************************************************/
      /* get sap index of local sap                                   */
      /****************************************************************/

      p->sapno = p->ri.ri_sbp->lsap/2;
#endif                                  /* TRLORFDDI                         */
#ifndef  TRLORFDDI
      if                               /* control byte is ui          */
         ((p->d.send_data->ctl1 == UI) || (p->d.send_data->ctl1 == 
         UI_NO_PF))
        {

          /************************************************************/
          /* get sap index of local sap                               */
          /************************************************************/

          p->sapno = p->d.send_data->lsap/2;
#endif                                  /* not TRLORFDDI                     */
if                                     /* sap is in use, and the local
                                          sap index                   */

  /********************************************************************/
  /* matches the input sap                                            */
  /********************************************************************/

   ((p->sap_list[p->sapno].in_use == TRUE) && (p->dlc_io_ext.sap_corr 
   == p->sapno))
            {

              /********************************************************/
              /* get addressability to sap control block              */
              /********************************************************/

              p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].sap_cb_addr;
              if                       /* the in use sap is open      */
                 (p->sap_ptr->sap_state == SAP_OPEN_STATE)
                {

 /* setup values to record a performance hook                         */

                  dlctype = HKWD_SYSX_DLC_PERF|DLC_TRACE_SNDND;
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
                  trchklt(dlctype, p->m->m_len);

                  /****************************************************/
                  /* disable any station access such as link trace or */
                  /* station limbo                                    */
                  /****************************************************/

                  p->stano = NO_MATCH;

/* <<< feature CDLI >>> */
		  /****************************************************/
		  /* insure that any broadcast or multicast packet    */
		  /* sets the appropriate mbuf flag (M_BCAST, M_MCAST)*/
		  /****************************************************/
		  if /* transmitting to a group address */
		     (p->d.send_data->raddr[0] & GROUP_ADDR_MASK)
		    {
		      if /* all ones broadcast */
			 (((ulong)(p->d.send_data->raddr[0]) == 0xffffffff)
			 && ((ushort)(p->d.send_data->raddr[4]) == 0xffff))
			{
			  /* set the broadcast mbuf flag */
			  p->m->m_flags |= M_BCAST;
			}
		      else /* not all ones broadcast */
			{
			  /* set the multicast mbuf flag */
			  p->m->m_flags |= M_MCAST;
			}
		    }

		  /****************************************************/
                  /* call write send command generator to transmit    */
                  /* data packet                                      */
                  /****************************************************/
		  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */
                } 
              else                     /* sap is not open             */
                {

                  /****************************************************/
                  /* log error and free buffer - invalid state        */
                  /****************************************************/

                  lanerrlg(p, ERRID_LAN0032, NON_ALERT, INFO_ERR, 0, 
                     FILEN, LINEN);
                  status = EINVAL;
                } 
            } 
          else                         /* sap is not in use or sap    
                                          correlator does not match   */
            {

              /********************************************************/
              /* log error - ioctl failed - invalid sap correlator    */
              /********************************************************/

              lanerrlg(p, ERRID_LAN0033, NON_ALERT, INFO_ERR, 0, 
                 FILEN, LINEN);
              status = EINVAL;
            } 
        } 
      else                             /* the control byte is not UI  */
	{
/* LEHb defect 43788 */
	  /************************************************************/
	  /* log error - write cmd failed - invalid write option      */
	  /************************************************************/

	  lanerrlg(p, ERRID_LAN8019, NON_ALERT, INFO_ERR, 0, FILEN,
	     LINEN);
/* LEHe */
          status = EINVAL;
        } 
      return (status);

}                                      /* end test_netd;              */
control(op,p,ext)
  register int op;
  register struct port_dcl *p;
  register struct dlc_corr_arg *ext;
{
  int err_code;
  int status;

  TRACE2(p, "CONb", ext);
#ifdef   DLC_DEBUG
  if (p->debug)
    printf("control plc_state=%d\n", p->common_cb.plc_state);
#endif
  if                                   /* the plc is currently open   */
     (p->common_cb.plc_state == PLC_OPENED)
    {

      /****************************************************************/
      /* get the link station CORRELATOR from the input               */
      /****************************************************************/

      p->stano = ext->gdlc_ls_corr;
      if                               /* the specified link station  
                                          CORRELATOR is valid         */
         ((p->stano > 0) && (p->stano < MAX_SESSIONS))
        {
          if                           /* the specified link station  
                                          is currently in use.        */
             (p->station_list[p->stano].in_use == TRUE)
            {

              /********************************************************/
              /* get addressability to the link station.              */
              /********************************************************/

              p->sta_ptr = (struct station_cb *)p->station_list
                 [p->stano].sta_cb_addr;

              /********************************************************/
              /* get addressability to the sap.                       */
              /********************************************************/

              p->sapno = p->station_list[p->stano].sapnum;
              p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].
                 sap_cb_addr;
              if                       /* the specified sap is        
                                          currently open.             */
                 (p->sap_ptr->sap_state == SAP_OPEN_STATE)
                {
                  switch (op)
                    {
                      case  DLC_TRACE :
                        status = trace_cmd(p, ext);
                        break;
                      case  DLC_CONTACT :
                        status = contact_cmd(p);
                        break;
                      case  DLC_TEST :
                        status = test_link_cmd(p);
                        break;
                      case  DLC_ALTER :
                        status = alter_cmd(p, ext);
                        break;
                      case  DLC_ENTER_LBUSY :
                        status = user_elb_cmd(p);
                        break;
                      case  DLC_EXIT_LBUSY :
                        status = user_xlb_cmd(p);
                        break;
                    } 
                } 
              else                     /* the specified sap is not    
                                          currently open.             */
                {

                  /****************************************************/
                  /* call error log - ioctl failed...sap not open.    */
                  /****************************************************/

                  lanerrlg(p, ERRID_LAN0033, NON_ALERT, INFO_ERR, 0, 
                     FILEN, LINEN);

                  /****************************************************/
                  /* indicate user interface error.                   */
                  /****************************************************/

                  err_code = DLC_USR_INTRF;
                  status = EINVAL;
                } 
            } 
          else                         /* the specified link station  
                                          is not currently in use.    */
            {

              /********************************************************/
              /* call error log - ioctl failed...invalid LS correlator*/
              /********************************************************/

              lanerrlg(p, ERRID_LAN0034, NON_ALERT, INFO_ERR, 0, 
                 FILEN, LINEN);

              /********************************************************/
              /* indicate user interface error.                       */
              /********************************************************/

              err_code = DLC_USR_INTRF;
              status = EINVAL;
            } 
        } 
      else                             /* the specified link station  
                                          CORRELATOR is not valid     */
        {

          /************************************************************/
          /* call error log - ioctl failed...invalid LS correlator    */
          /************************************************************/

          lanerrlg(p, ERRID_LAN0034, NON_ALERT, INFO_ERR, 0, FILEN, 
             LINEN);

          /************************************************************/
          /* indicate user interface error.                           */
          /************************************************************/

          err_code = DLC_USR_INTRF;
          status = EINVAL;
        } 
    } 
  else                                 /* the plc is not currently    
                                          open                        */
    {

      /****************************************************************/
      /* call error log - ioctl failed...plc not open.                */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN0035, NON_ALERT, INFO_ERR, 0, FILEN, LINEN);

      /****************************************************************/
      /* indicate user interface error.                               */
      /****************************************************************/

      err_code = DLC_USR_INTRF;
      status = EINVAL;
    } 
  TRACE2(p, "CONe", status);
  return (status);
} 

open_sap_cmd(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  open_sap_cmd                                         */
/*                                                                    */
/* descriptive name:  open service access point command.              */
/*                                                                    */
/* function:  adds the sap to the sap table, and copies in the sap    */
/*            profile, and issues the "open device" command to the    */
/*            block i/o manager if it's the first sap on the port.    */
/*                                                                    */
/* input:  the sap configuration data.                                */
/*                                                                    */
/* output:  "open device" is enqueued to the block i/o manager.       */
/*                                                                    */
/*** end of specifications ********************************************/

{
  struct dlc_esap_arg esap;

#define  NO_ERROR      0               /* no error, needs no backout  */
#define  L1_ERROR      1               /* level-1 error, needs no       
                                        backout                      */
#define  L2_ERROR      2               /* level-2 error, needs sap cb 
                                          deallocated                 */
int op_res = RC_GOOD;                  /* error operation result code */
  int error = NO_ERROR;                /* default no errors           */

  TRACE1(p, "OPSb");

  /********************************************************************/
  /* preset the p->sapno and stationno to "no match".                 */
  /********************************************************************/

  p->sapno = NO_MATCH;
  p->stano = NO_MATCH;
  if                                   /* the device handler is not   
                                          currently open              */
/* <<< feature CDLI >>> */
     (p->nddp == 0)
/* <<< end feature CDLI >>> */

    /******************************************************************/
    /* then open the physical link                                    */
    /******************************************************************/

    op_res = open_plc(p);
  if                                   /* the open was successful or  
                                          already opened              */
     (op_res == RC_GOOD)
    {
      if                               /* the maximum number of saps  
                                          is not being exceeded       */
         ((p->dlc_port.cid->saps) < p->dlc_port.cid->maxsaps)
        {

          /************************************************************/
          /* copy the user's sap configuration into temp sap profile  */
          /* area.                                                    */
          /************************************************************/

          if (p->dlc_port.cid->state == KERN)
	    bcopy(p->l.sap_config, &esap, sizeof(struct dlc_esap_arg));
          else
	    copyin(p->l.sap_config, &esap, sizeof(struct dlc_esap_arg));

          /************************************************************/
          /* set the current sap index from the input profile's lsap  */
          /* value.                                                   */
          /************************************************************/

          p->sapno = esap.local_sap/2;
          if                           /* the input sap is not already
                                          in use in the sap list      */
             (p->sap_list[p->sapno].in_use == FALSE)
            {

              /********************************************************/
              /* allocate the sap profile and control block area from */
              /* the heap, word aligned.                              */
              /********************************************************/

              p->sap_ptr = (struct sap_cb *)xmalloc(sizeof(struct 
                 sap_cb), 2, kernel_heap);
              if                       /* there was storage available */
                 (p->sap_ptr != (struct sap_cb *)RC_NONE)
                {

                  /****************************************************/
                  /* clear the sap profile and sap control block area.*/
                  /****************************************************/

                  bzero(p->sap_ptr, sizeof(struct sap_cb));

                  /****************************************************/
                  /* copy the user's sap configuration into the sap   */
                  /* profile area.                                    */
                  /****************************************************/

                  bcopy(&esap, &(p->sap_ptr->sap_profile), sizeof
                     (struct dlc_esap_arg));

                  /****************************************************/
                  /* validate the input configuration area. returns   */
                  /* any errno or RC_GOOD                             */
		  /****************************************************/

		  p->rc = validate_sap_config(p);
                  if                   /* the input configuration is  
                                          valid                       */
		     (p->rc == RC_GOOD)
                    {

                      /************************************************/
                      /* indicate the sap is in use in the sap list.  */
                      /************************************************/

                      p->sap_list[p->sapno].in_use = TRUE;

                      /************************************************/
                      /* put the address of the sap control block in  */
                      /* the sap list.                                */
		      /************************************************/

		      p->sap_list[p->sapno].sap_cb_addr = (int)
			 p->sap_ptr;
		      p->sap_ptr->user_sap_channel = p->dlc_port.cid;


		     
                        {

                          /********************************************/
                          /* bump the number of saps open             */
                          /********************************************/

                          p->dlc_port.cid->saps++;



/* defect 82006 */
                          /********************************************/
                          /* call llinit to setup multicast address   */
                          /*  list anchor                             */
                          /********************************************/

                         (void) llinit(p);

/* end defect 82006 */ 
                          
                          
                        } 
                       
                    } 
                  else                 /* the input configuration is  
                                          not valid.                  */
                    {

                      /************************************************/
                      /* indicate that level-2 backout is needed.     */
                      /************************************************/

                      error = L2_ERROR;

                      /************************************************/
                      /* call error log - invalid input command       */
                      /* parameter. with the return code from         */
                      /* validation.                                  */
                      /************************************************/

		      lanerrlg(p, p->rc, NON_ALERT, 0, 0, FILEN, LINEN);
                      op_res = EINVAL;
                    } 
                } 
              else                     /* no storage for SAP control  
                                          block                       */
                {

                  /****************************************************/
                  /* indicate that level-1 backout is needed.         */
                  /****************************************************/

                  error = L1_ERROR;

                  /****************************************************/
                  /* call error log - memory allocation failure       */
                  /****************************************************/

                  lanerrlg(p, ERRID_LAN8007, NON_ALERT, 0, 0, FILEN, 
                     LINEN);
                  op_res = ENOSPC;
                } 
            } 
          else                         /* target sap is already in use*/
            {

              /********************************************************/
              /* indicate that level-1 backout is needed.             */
              /********************************************************/

              error = L1_ERROR;

              /********************************************************/
              /* call error log - sap already in use                  */
              /********************************************************/

	      lanerrlg(p, ERRID_LAN8003, NON_ALERT, 0, 0, FILEN, LINEN);
              op_res = EINVAL;
            } 
        } 
      else                             /* maximum saps have been      
                                          exceeded                    */
        {

          /************************************************************/
          /* indicate that level-1 backout is needed.                 */
          /************************************************************/

          error = L1_ERROR;

          /************************************************************/
          /* call error log - max saps exceeded                       */
          /************************************************************/

          lanerrlg(p, ERRID_LAN8005, NON_ALERT, 0, 0, FILEN, LINEN);
          op_res = EINVAL;
        } 
    } 
  else                                 /* open to the device handler  
                                          failed                      */
    {

      /****************************************************************/
      /* indicate that level-1 backout is needed.                     */
      /****************************************************************/

      error = L1_ERROR;

      /****************************************************************/
      /* already error logged in open_plc and op_res holds the errno  */
      /****************************************************************/

    } 
  if                                   /* no error has occurred       */
     (error == NO_ERROR)
    {

      /****************************************************************/
      /* call the sap setup routine to check the local name, open the */
      /* port, and initiate add name procedures. Note: any errors will*/
      /* cause asynchronous sap_disabled status                       */
      /****************************************************************/

      sap_setup(p);

      /****************************************************************/
      /* write the gdlc sap correlator to the user's ioctl extension  */
      /****************************************************************/

      if (p->dlc_port.cid->state == KERN)
        p->l.sap_config->gdlc_sap_corr = p->sapno;
      else
        copyout(&p->sapno, &p->l.sap_config->gdlc_sap_corr, 4);
    } 
  else                                 /* an error has occurred,      
                                          enable sap return code      
                                          already set up              */
    {
      if                               /* a level-2 error occurred    */
         (error == L2_ERROR)
        {

          /************************************************************/
          /* a backout of the sap control block is needed.            */
          /************************************************************/
/* <<< feature CDLI >>> */
	  assert(xmfree(p->sap_ptr, kernel_heap) == 0);
/* <<< end feature CDLI >>> */
        } 
    } 

 /* return with the prebuilt return code.                             */

  return (op_res);
}                                      /* end open_sap_cmd;           */
validate_sap_config(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  validate_sap_config                                  */
/*                                                                    */
/* descriptive name:  validate input sap configuration                */
/*                                                                    */
/* function:  checks pertinent input parameters within the sap        */
/*            configuration area for proper ranges, values, etc.      */
/*            the correct length has already been checked and the     */
/*            configuration area has been copied to the sap control   */
/*            block prior to calling this routine.                    */
/*                                                                    */
/* input:  the sap control block.                                     */
/*                                                                    */
/*** end of specifications ********************************************/

{
/* <<< feature CDLI >>> */
#ifdef   TRL
  /********************************************************************/
  /* if length of functional address is not zero, 4-bytes, or 6-bytes */
  /* Note:  the COMIO version of functional address is 4-bytes and    */
  /*        the CDLI version is the full 6-bytes.                     */
  /********************************************************************/

  if ((p->sap_ptr->sap_profile.len_func_addr_mask != 0) && 
      (p->sap_ptr->sap_profile.len_func_addr_mask != 6))
    {
      if /* it's a COMIO 4-byte functional address */
	 (p->sap_ptr->sap_profile.len_func_addr_mask == 4)
	{
	   /* convert it to a 6-byte functional address */
	   p->sap_ptr->sap_profile.func_addr_mask[5] =
		  p->sap_ptr->sap_profile.func_addr_mask[3];
	   p->sap_ptr->sap_profile.func_addr_mask[4] =
		  p->sap_ptr->sap_profile.func_addr_mask[2];
	   p->sap_ptr->sap_profile.func_addr_mask[3] =
		  p->sap_ptr->sap_profile.func_addr_mask[1];
	   p->sap_ptr->sap_profile.func_addr_mask[2] =
		  p->sap_ptr->sap_profile.func_addr_mask[0];
	   p->sap_ptr->sap_profile.func_addr_mask[1] = 0x00;
	   p->sap_ptr->sap_profile.func_addr_mask[0] = 0xc0;

	   p->sap_ptr->sap_profile.len_func_addr_mask = 6;
	}
      else /* not 0, 4, or 6 bytes */
	{
	  /************************************************************/
	  /* set the error code to "invalid functional address"       */
	  /************************************************************/

	  return (ERRID_LAN8044);
	}
    }

  /********************************************************************/
  /* if length of group address is not zero, 4-bytes, or 6-bytes      */
  /* Note:  the COMIO version of group address is 4-bytes and the     */
  /*        CDLI version is the full 6-bytes.                         */
  /********************************************************************/

  if ((p->sap_ptr->sap_profile.len_grp_addr != 0) &&
      (p->sap_ptr->sap_profile.len_grp_addr != 6))
    {
      if /* it's a COMIO 4-byte group address */
	 (p->sap_ptr->sap_profile.len_grp_addr == 4)
	{
	   /* convert it to a 6-byte group address */
	   p->sap_ptr->sap_profile.grp_addr[5] =
			 p->sap_ptr->sap_profile.grp_addr[3];
	   p->sap_ptr->sap_profile.grp_addr[4] =
			 p->sap_ptr->sap_profile.grp_addr[2];
	   p->sap_ptr->sap_profile.grp_addr[3] =
			 p->sap_ptr->sap_profile.grp_addr[1];
	   p->sap_ptr->sap_profile.grp_addr[2] =
			 p->sap_ptr->sap_profile.grp_addr[0];
	   p->sap_ptr->sap_profile.grp_addr[1] = 0x00;
	   p->sap_ptr->sap_profile.grp_addr[0] = 0xc0;

	   p->sap_ptr->sap_profile.len_grp_addr = 6;
	}
      else /* not 0, 4, or 6 bytes */
	{
	  /************************************************************/
	  /* set the error code to "invalid group address"            */
	  /************************************************************/

	  return (ERRID_LAN8045);
	}
    }
#endif                                  /* TRL                       */
/* <<< end feature CDLI >>> */

#if defined(EDL) || defined(E3L) || defined(FDL)

  /********************************************************************/
  /* if length of functional address is not zero                      */
  /********************************************************************/

  if (p->sap_ptr->sap_profile.len_func_addr_mask != 0)

    /******************************************************************/
    /* set the error code to "invalid functional address"             */
    /******************************************************************/

    return (ERRID_LAN8044);

  /********************************************************************/
  /* if length of group address is not zero or 6-bytes                */
  /********************************************************************/

  if ((p->sap_ptr->sap_profile.len_grp_addr != 0) && 
     (p->sap_ptr->sap_profile.len_grp_addr != 6))

    /******************************************************************/
    /* set the error code to "invalid group address"                  */
    /******************************************************************/

    return (ERRID_LAN8045);
#endif  /* EDL or E3L or FDL */

  /********************************************************************/
  /* if max_ls is 0 or greater than the maximum allowed               */
  /********************************************************************/

if ((p->sap_ptr->sap_profile.max_ls == 0) || (
p->sap_ptr->sap_profile.max_ls >= MAX_SESSIONS))

    /******************************************************************/
    /* set the error code to "invalid number of link stations".       */
    /******************************************************************/

    return (ERRID_LAN8046);

  /********************************************************************/
  /* if the local name length is 0 or exceeds 20 characters and this  */
  /* is a "discovery" sap (ie. not "resolve")                         */
  /********************************************************************/

  if (((p->sap_ptr->sap_profile.len_laddr_name == 0) || 
     (p->sap_ptr->sap_profile.len_laddr_name > sizeof
     (p->sap_ptr->sap_profile.laddr_name))) && 
     ((p->sap_ptr->sap_profile.flags&DLC_ESAP_ADDR) == 0))

    /******************************************************************/
    /* set the error code to "invalid local name length".             */
    /******************************************************************/

    return (ERRID_LAN8049);

  /********************************************************************/
  /* if the number of group saps is anything other than zero          */
  /********************************************************************/

  if (p->sap_ptr->sap_profile.num_grp_saps != 0)

    /******************************************************************/
    /* set the error code to "invalid number of groups"               */
    /******************************************************************/

    return (ERRID_LAN8050);

  /********************************************************************/
  /* if the local sap is zero or odd                                  */
  /********************************************************************/

  if ((p->sap_ptr->sap_profile.local_sap == 0) ||
     ((p->sap_ptr->sap_profile.local_sap&1) != 0))

    /******************************************************************/
    /* set the error code to "invalid local sap".                     */
    /******************************************************************/

    return (ERRID_LAN8051);

  /********************************************************************/
  /* if the local sap is 0xFC (LS/X RIPL or Discovery)                */
  /********************************************************************/
  /** Normally this sap value is not allowed since DLC uses 0xFC for **/
  /** its discovery protocol.  The LS/X (OS/2) Remote IPL Server     **/
  /** (RIPL) application also uses this value, however, so the RIPL  **/
  /** functional/multicast address is used as an identifier at enable**/
  /** sap time.  Once enabled, all receive packets that have a       **/
  /** destination sap of 0xFC and the RIPL functional/multicast      **/
  /** destination address are routed to the RIPL application.        **/
  /********************************************************************/

  if (p->sap_ptr->sap_profile.local_sap == DISCOVERY_SAP)
    {
/* <<< feature CDLI >>> */
#ifdef  TRL
				      /* the LS/X RIPL functional
					 address is NOT specified     */
    if ((p->sap_ptr->sap_profile.len_func_addr_mask != 6) ||
       (bcmp(p->sap_ptr->sap_profile.func_addr_mask,
	     p->common_cb.ripl_grp_addr, 6) != 0))
#endif /* TRL */
/* <<< end feature CDLI >>> */

#if defined(EDL) || defined(E3L) || defined(FDL)
				      /* the LS/X RIPL multicast
					 address is NOT specified     */
    if ((p->sap_ptr->sap_profile.len_grp_addr != 6) ||
       (bcmp(p->sap_ptr->sap_profile.grp_addr,
	     p->common_cb.ripl_grp_addr, 6) != 0))
#endif /* not TRL */

       /***************************************************************/
       /* set the error code to "invalid local sap".                  */
       /***************************************************************/

       return (ERRID_LAN8051);
    }                                  /* endif not LS/X RIPL         */
  return (RC_GOOD);
}                                      /* end validate_sap_config;    */

/* <<< feature CDLI >>> */
extern void lan_rcv();
/* <<< end feature CDLI >>> */
sap_setup(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  sap_setup                                            */
/*                                                                    */
/* descriptive name:  sap add name setup routine.                     */
/*                                                                    */
/* function:  checks to see if the desired local name is already      */
/*            added to the NETWORK and initiates the add name sequence*/
/*            if it's not already added.                              */
/*                                                                    */
/* input:  the sap profile.                                           */
/*                                                                    */
/* output:  "add name" is sent to the NETWORK.                        */
/*                                                                    */
/*** end of specifications ********************************************/

{
  char *temp_sap_ptr;
  char temp_lname[20];
  int temp_lname_length,match_found,index;
/* <<< feature CDLI >>> */
  struct ns_user  ns_user;
  struct ns_8022  ns_filter;
/* <<< end feature CDLI >>> */
  struct dlc_add_grp add_group;
  ulong dlctype;                       /* lan type                    */
  int rc = 0;

  TRACE2(p, "SPSb", p->sapno);

  /********************************************************************/
  /* save the input local name and length in temporary storage.       */
  /********************************************************************/

  bcopy(p->sap_ptr->sap_profile.laddr_name, temp_lname, 
     p->sap_ptr->sap_profile.len_laddr_name);
  temp_lname_length = p->sap_ptr->sap_profile.len_laddr_name;

  /********************************************************************/
  /* save the current sap pointer.                                    */
  /********************************************************************/

  temp_sap_ptr = (char *)p->sap_ptr;

  /********************************************************************/
  /* preset the match found indicator = FALSE.                        */
  /********************************************************************/

  match_found = FALSE;
  if                                   /* name discovery              */
     (!(p->sap_ptr->sap_profile.flags & DLC_ESAP_ADDR))

    {

      /****************************************************************/
      /* for each of the saps already in use                          */
      /****************************************************************/

      for (index = 0; index <= 127; index++)
        {
          if (match_found == TRUE)
            break;
          if                           /* the indexed sap is in use,  
                                          and it's not the sap being  */

          /************************************************************/
          /* opened                                                   */
	  /************************************************************/

	     ((p->sap_list[index].in_use == TRUE) && (p->sapno !=
	     index))
            {

              /********************************************************/
              /* get addressability to the sap control block          */
	      /********************************************************/

	      p->sap_ptr = (struct sap_cb *)p->sap_list[index].
		 sap_cb_addr;
	      if                       /* the given local name matches
					  the indexed local name      */
		 ((bcmp(p->sap_ptr->sap_profile.laddr_name, temp_lname,
			temp_lname_length) == 0) &&
		 (temp_lname_length ==
				p->sap_ptr->sap_profile.len_laddr_name))

                /******************************************************/
                /* indicate that a match was found.                   */
                /******************************************************/

                match_found = TRUE;
            } 
        }                              /* end do index;               */
    } 

  /********************************************************************/
  /* restore the current sap pointer.                                 */
  /********************************************************************/

  p->sap_ptr = (struct sap_cb *)temp_sap_ptr;
  if                                   /* the specified local name is 
                                          not already added to the    
                                          network                     */
     (match_found == FALSE)
    {
    /******************************************************************/
    /* set the add name ready indicator in the sap list, so that the  */
    /* add name query generator will send the add name packet.        */
    /******************************************************************/

    p->sap_list[p->sapno].addn_ready = TRUE;

#ifndef  EDL
/* <<< feature CDLI >>> */
    if                                 /* the physical link is open   */
       (p->common_cb.plc_state == PLC_OPENED)
      {

      if                               /* this is normal sap setup, ie.
					  not LS/X RIPL enabling 0xFC */
	 (p->sap_ptr->sap_profile.local_sap != DISCOVERY_SAP)
	{

	/**************************************************************/
	/* preset ns_user and ns_filter structures                    */
	/**************************************************************/
	bzero(&ns_filter, sizeof(ns_filter));
	bzero(&ns_user, sizeof(ns_user));
	ns_user.isr = lan_rcv;                /* rcv entry point   */
	ns_user.isr_data = (ulong_t)p;        /* pointer to port   */
     /* ns_user.protoq = NULL;                ** no input queue    */
     /* ns_user.netisr = NULL;                ** no schednetisr    */
	ns_user.pkt_format = NS_INCLUDE_MAC;  /* get entire packet */
     /* ns_user.ifp = NULL;                   ** non-socket user   */

	/**************************************************************/
	/* issue ns add filter to DH for user's local sap             */
	/**************************************************************/
	ns_filter.filtertype = NS_8022_LLC_DSAP; /* DSAP only   */
	ns_filter.dsap = p->l.sap_config->local_sap;

	p->rc = ns_add_filter(p->nddp, &ns_filter, sizeof(ns_filter), &ns_user);

	if (p->rc != 0)
	  {
#ifdef   DEBUG
	    printf("User Sap ns_add_filter Failed rc=%d\n", p->rc);
#endif /* DEBUG */

	    /************************************************************/
	    /* call error log - ns_add_filter failed                    */
	    /* sap shutdown = Unusual network condition                 */
	    /************************************************************/

	    lanerrlg(p, ERRID_LAN802C, NON_ALERT, PERM_SAP_ERR,
		     DLC_SAP_NT_COND, FILEN, LINEN);
	    rc = -1;
	  }

	/**********************************************************/
	/* call filter_started to add the local name              */
	/**********************************************************/

	filter_started(p,p->l.sap_config->local_sap);

/* <<< end feature CDLI >>> */
	}                              /* endif: not LS/X RIPL        */
      }                                /* endif: PL starting or opened*/


    if                                 /* this is LS/X RIPL enabling 0xFC */

       (p->sap_ptr->sap_profile.local_sap == DISCOVERY_SAP)
      {
      if                               /* the physical link is already
					  open, ie. 0xFC started       */
	 (p->common_cb.plc_state == PLC_OPENED)
	{
				       /* set sap state = open        */
		p->sap_ptr->sap_state = SAP_OPEN_STATE;

				       /* bump the add name counter just
					  because it will be decremented */
		p->common_cb.addn_ctr++;
				       /* call add name completion to set
					  multicast addresses and send
					  result to user              */
		add_name_cmpl(p);
	}                              /* endif: PL opened            */
      }                                /* endif: RIPL sap             */


#endif /* Not EDL */

  /********************************************************************/
  /* setup values to record start performance hook                    */
  /********************************************************************/

dlctype = HKWD_SYSX_DLC_PERF|DLC_TRACE_STDH;
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
      trchklt(dlctype, 0);

#ifdef   EDL
#ifdef   DEBUG
      printf("PLC-STATE = %d\n", p->common_cb.plc_state);
#endif /* DEBUG */
    if                                 /* the physical link is already
					  fully open (not just
					  starting)                    */
       (p->common_cb.plc_state == PLC_OPENED)
      {

      /************************************************************/
      /* call discovery to issue an add name                      */
      /************************************************************/

      landiscv(p, ADDNO);

      }                                /* endif: PLC fully opened     */
#endif /* EDL */
    } 
  else                                 /* the specified local name is 
                                          already in use.             */
    {

      /****************************************************************/
      /* call error log - duplicate local name. sap shutdown = user   */
      /* interface error.                                             */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN8001, NON_ALERT, PERM_SAP_ERR, 
         DLC_NAME_IN_USE, FILEN, LINEN);
      rc = -3;
    } 
  TRACE3(p, "SPSe", rc, p->rc);
}                                      /* end sap_setup;              */

extern void lan_status();
/* <<< feature CDLI >>> */
/* removed extern void lan_rcv(); */
/* removed extern void lan_xmit();*/
/* <<< end feature CDLI >>> */
open_plc(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  open_plc                                             */
/*                                                                    */
/* descriptive name:  open physical link control.                     */
/*                                                                    */
/* function:                                                          */
/*                                                                    */
/* input:                                                             */
/*                                                                    */
/* output:                                                            */
/*                                                                    */
/*** end of specifications ********************************************/

{
/* <<< feature CDLI >>> */
  struct ns_user        ns_user;
  struct ns_statuser    ns_statuser;
  struct ns_8022        ns_filter;
  struct ns_com_status  ns_statfilter;
/* <<< end feature CDLI >>> */
  int    rc;                            /* fast write return code     */
  ulong dlctype;                       /* lan type                    */
  char path[80];

  TRACE1(p, "OPPb");
  if                                   /* the state of the physical   
                                          link is not closed          */
     (p->common_cb.plc_state != PLC_CLOSED)
    {

      /****************************************************************/
      /* call error log - invalid program execution sap shutdown =    */
      /* coding error                                                 */
      /****************************************************************/
/* <<< defect 131020 >>> */
      lanerrlg(p, ERRID_LAN8011, NON_ALERT, PERM_PLC_ERR, 
         DLC_ERR_CODE, FILEN, LINEN);
/* <<< end defect 131020 >>> */

      /****************************************************************/
      /* return error code                                            */
      /****************************************************************/

      return (EINVAL);
    } 
  else                                 /* the physical link is in the 
                                          proper state                */
    {

      /****************************************************************/
      /* set the state of the physical link = opening device.         */
      /****************************************************************/

      p->common_cb.plc_state = OPENING_DEVICE;

      /****************************************************************/
      /* pre-set the plc return code to NORMAL.                       */
      /****************************************************************/

      p->common_cb.plc_retcode = NORMAL;
      if (p->dlc_port.cid->saps == 0)
        {
/* <<< feature CDLI >>> */
/* defect 155341 */
	  /************************************************************/
	  /* allocate the ndd device                                  */
	  /************************************************************/
	  rc = ns_alloc(p->dlc_port.namestr, &p->nddp);

	  if /* bad return code */
	     (rc != RC_GOOD)
	    {
	      /********************************************************/
	      /* set the state of the physical link = phys link closed*/
	      /********************************************************/

	      p->common_cb.plc_state = PLC_CLOSED;

	      /********************************************************/
	      /* call error log - ns_allocate failed, plc shutdown =  */
	      /* system error                                         */
	      /********************************************************/
	      /* NEEDS NEW ERRID ??????????????????? */

	      lanerrlg(p, ERRID_LAN0035, NON_ALERT, PERM_PLC_ERR,
		 DLC_SYS_ERR, FILEN, LINEN);

	      return (rc);
	    }

	  /********************************************************/
	  /* set up the device for asyncronous status             */
	  /* preset ns_statuser and ns_statfilter structures      */
	  /********************************************************/
	  bzero(&ns_statfilter, sizeof(ns_statfilter));
	  bzero(&ns_statuser, sizeof(ns_statuser));
	  ns_statuser.isr = lan_status;      /* status entry point*/
	  ns_statuser.isr_data = (ulong_t)p; /* pointer to port   */
	  ns_statfilter.filtertype = NS_STATUS_MASK;
	  ns_statfilter.mask = (NDD_HARD_FAIL | NDD_CONNECTED);
       /* ns_statfilter.sid = 0;              * already zeroed    */

	  /********************************************************/
	  /* issue ns add status to DH                            */
	  /********************************************************/
	  rc = ns_add_status(p->nddp, &ns_statfilter,
				sizeof(ns_statfilter), &ns_statuser);

	  if /* bad return code */
	     (rc != RC_GOOD)
	    {
	      /********************************************************/
	      /* call error log - ns_add_status failed, plc shutdown  */
	      /* system error                                         */
	      /********************************************************/
	      /* NEEDS NEW ERRID ??????????????????? */

	      lanerrlg(p, ERRID_LAN0035, NON_ALERT, PERM_PLC_ERR,
		 DLC_SYS_ERR, FILEN, LINEN);

	      return (rc);
	    }
/* end defect 155341 */

	  /* save the status id (sid) in the port cb */
	  p->statfilter_sid = ns_statfilter.sid;

	  /* Call start_filters to attempt to add the sap filters.
	     The device may still be inserting into the network, in
	     which case the kproc will be notified with NDD_CONNECTED
	     once it is ready for business */

	  start_filters(p);

/* <<< end feature CDLI >>> */
        } 
    } 
  TRACE1(p, "OPPe");
  return (0);
} /* end open_plc */

/* <<< feature CDLI >>> */
extern void lan_rcv();
start_filters(p)
  register struct port_dcl *p;
{
  ulong dlctype;                       /* trace: lan type */
  struct ns_user        ns_user;
  struct ns_8022        ns_filter;
  int i;

  /********************************************************/
  /* setup values to record filter started performance    */
  /* hook                                                 */
  /********************************************************/

  dlctype = HKWD_SYSX_DLC_PERF|DLC_TRACE_STDH;
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
  trchklt(dlctype, 0);

  if /* the device is running, ie. ready for data traffic */
     (p->nddp->ndd_flags & NDD_RUNNING)
    {
      /****************************************************/
      /* get the local mac address                        */
      /****************************************************/
      bcopy(*(&p->nddp->ndd_physaddr), p->common_cb.local_addr, 6);


      /****************************************************/
      /* set the state of the physical link =             */
      /* starting device.                                 */
      /****************************************************/

      p->common_cb.plc_state = STARTING_DEVICE;
      p->common_cb.resolve_started = FALSE;

      /****************************************************/
      /* preset ns_user and ns_filter structures          */
      /****************************************************/
      bzero(&ns_filter, sizeof(ns_filter));
      bzero(&ns_user, sizeof(ns_user));
      ns_user.isr = lan_rcv;           /* rcv entry point */
      ns_user.isr_data = (ulong_t)p;   /* pointer to port */
   /* ns_user.protoq = NULL;           ** no input queue  */
   /* ns_user.netisr = NULL;           ** no schednetisr  */
      ns_user.pkt_format = NS_INCLUDE_MAC; /* get full packet */
   /* ns_user.ifp = NULL;              ** non-socket user */

#ifndef  EDL

     /********************************************************/
     /* issue ns add filter to DH for SAP=0x00               */
     /********************************************************/

     ns_filter.filtertype = NS_8022_LLC_DSAP; /* DSAP only   */
  /* ns_filter.dsap = 0x00;               ** enable sap 0x00 */

     p->rc = ns_add_filter(p->nddp, &ns_filter, sizeof(ns_filter), &ns_user);

     if (p->rc != 0)
       {

	 /****************************************************/
	 /* call error log - ns_add_filter failed. plc       */
	 /* shutdown = system error                          */
	 /****************************************************/

	 lanerrlg(p, ERRID_LAN802C, NON_ALERT, PERM_PLC_ERR,
	    DLC_SYS_ERR, FILEN, LINEN);

	 /****************************************************/
	 /* return error code                                */
	 /****************************************************/

	 return (p->rc);
       }

     /********************************************************/
     /* call the filter_started routine to handle the        */
     /* specific filter started (0x00).                      */
     /********************************************************/

     filter_started(p, 0x00);

     /********************************************************/
     /* issue ns add filter to DH for SAP=0xFC               */
     /********************************************************/

     ns_filter.filtertype = NS_8022_LLC_DSAP; /* DSAP only   */
     ns_filter.dsap = 0xFC;               /* enable sap 0xFC */

     p->rc = ns_add_filter(p->nddp, &ns_filter, sizeof(ns_filter), &ns_user);

     if (p->rc != 0)
       {

	 /****************************************************/
	 /* call error log - ns_add_filter failed. plc       */
	 /* shutdown = system error                          */
	 /****************************************************/

	 lanerrlg(p, ERRID_LAN802C, NON_ALERT, PERM_PLC_ERR,
	    DLC_SYS_ERR, FILEN, LINEN);

	 /****************************************************/
	 /* return error code                                */
	 /****************************************************/

	 return (p->rc);
       }

     /********************************************************/
     /* call the filter_started routine to handle the        */
     /* specific filter started (0xFC).                      */
     /********************************************************/

     filter_started(p,0xFC);

     /********************************************************/
     /* loop and issue ns add filter to DH for any user saps */
     /********************************************************/

      for /* each sap already enabled on the port */
	  (i = 1; i < 128; i++)
	 {
	   if /* sap is active */
	      (p->sap_list[i].in_use == TRUE)
	     {
/* <<< defect 128609 >>> */
               if /* it's the RIPL sap */
                  (i == DISCOVERY_SAP/2)
                 {
                   /* default the add filter return code to zero */
                   p->rc = 0;
                 }
               else /* not the RIPL sap */
                 {
	           ns_filter.filtertype = NS_8022_LLC_DSAP; /* DSAP only */
	           ns_filter.dsap = (i*2);             /* user sap value */

	           p->rc = ns_add_filter(p->nddp, &ns_filter,
					 sizeof(ns_filter), &ns_user);
                 }
/* <<< end defect 128609 >>> */
	       if (p->rc != 0) 
		 {
		   /****************************************************/
		   /* call error log - ns_add_filter failed            */
		   /* sap shutdown = Unusual network condition         */
		   /****************************************************/

		   lanerrlg(p, ERRID_LAN802C, NON_ALERT, PERM_SAP_ERR,
			    DLC_SAP_NT_COND, FILEN, LINEN);
		 }
	       else
		 {
		   /****************************************************/
		   /* call the filter_started routine to handle the    */
		   /* specific user filter started.                    */
		   /****************************************************/

		   filter_started(p,(i*2));
		 }

	     } /* endif sap active */
	 } /* end loop */
#endif /* not EDL */

#ifdef   EDL

     /********************************************************/
     /* issue ns add filter to DH for ethertype 0x80D5       */
     /********************************************************/

     ns_filter.filtertype = NS_ETHERTYPE;  /* ethertype only */
     ns_filter.ethertype = 0x80d5;         /* enable 0x80D5  */

     p->rc = ns_add_filter(p->nddp, &ns_filter, sizeof(ns_filter), &ns_user);

     if (p->rc != 0)
       {

	 /****************************************************/
	 /* call error log - ns_add_filter failed. plc       */
	 /* shutdown = system error                          */
	 /****************************************************/

	 lanerrlg(p, ERRID_LAN802C, NON_ALERT, PERM_PLC_ERR,
	    DLC_SYS_ERR, FILEN, LINEN);

	 /****************************************************/
	 /* return error code                                */
	 /****************************************************/

	 return (p->rc);
       }

     /********************************************************/
     /* call the filter_started routine to handle the        */
     /* specific filter started (0x80D5).                    */
     /********************************************************/

     filter_started(p,NULL);

#endif                                  /* EDL                         */
     } /* endif ndd is running */
}
/* <<< end feature CDLI >>> */

/* <<< feature CDLI >>> */
 /********************************************************************
 * NAME: filter_started
 *
 * FUNCTION: starts the add name query procedure.
 *           Note: the input sap filter is only used for token ring,
 *                 fddi, and IEEE 802.3 links.
 *
 **********************************************************************/

filter_started(p,filter)
  register struct port_dcl *p;
  uchar    filter;                     /* filter value started        */
{
  ulong    dlctype;                    /* lan type                    */
  ulong_t  port_sta;                   /* station number and port
					  number                      */

  TRACE1(p, "KDSb");

#ifdef   DEBUG
  if (p->debug)
    printf("filter_started %d\n", p->common_cb.plc_state);
#endif

  if                                   /* the state of the physical
					  link is starting device or
					  opened                      */
     (p->common_cb.plc_state == STARTING_DEVICE ||
      p->common_cb.plc_state == PLC_OPENED)
    {
				       /* save the local mac address  */
      bcopy(p->nddp->ndd_physaddr, p->common_cb.local_addr, 6);

#ifdef   DEBUG
      if (p->debug)
	{
	  printf("input filter = %x\n", filter);
	  dump(p->common_cb.local_addr, 6);
	}
#endif /* DEBUG */

#if defined(E3L) || defined(TRL) || defined(FDL)
      switch                           /* on the sap started          */
	(filter)
	{

	  case RESOLVE_SAP:
				       /* set resolve sap started     */
	    p->common_cb.resolve_started = TRUE;
	    break;

	  case DISCOVERY_SAP:
 /* <<< defect 128609 >>> */
	    if                         /* resolve already started     */
	       (p->common_cb.resolve_started == TRUE)
	      {
		if                     /* this is discovery starting,
					  ie. 0xFC for the first time */
		   (p->common_cb.discovery_started == FALSE)
		  {
		    /* indicate that discovery has started */
		    p->common_cb.discovery_started = TRUE;

		    /* set the state of the physical link = opened.   */
		    p->common_cb.plc_state = PLC_OPENED;
		  }
		else /* it may be RIPL sap starting */
		  {
		    if                 /* this sap is in-use (ie, RIPL)*/
				       /* NOTE: discovery sap 0xFC does
						not set "in-use".  LS/X
						RIPL does set "in-use" */
		       (p->sap_list[filter/2].in_use == TRUE)
		      {
				       /* get addressability to the sap
					  control block               */
			p->sapno = filter/2;
			p->sap_ptr =
			   (struct sap_cb *)p->sap_list[p->sapno].sap_cb_addr;

				       /* set sap state = open        */
			p->sap_ptr->sap_state = SAP_OPEN_STATE;

				       /* bump the add name counter just
					  because it will be decremented */
			p->common_cb.addn_ctr++;
				       /* call add name completion to set
					  multicast addresses and send
					  result to user              */
			add_name_cmpl(p);
		      }                /* endif: RIPL sap in-use      */
		  }                    /* endif: 0xFC sap             */
	      }
 /* <<< end defect 128609 >>> */

	    else /* resolve sap 0x00 did not start properly */
	      /********************************************************/
	      /* call error log - unexpected return code (start       */
	      /* device), plc shutdown = unusual NETWORK condition.   */
	      /********************************************************/

	      lanerrlg(p, ERRID_LAN802C, NON_ALERT, PERM_PLC_ERR,
		       DLC_SAP_NT_COND, FILEN, LINEN);
	    break;

	  default:                     /* all other saps              */
	    if                         /* this sap is in-use          */
	       (p->sap_list[filter/2].in_use == TRUE)
	      {

	      /********************************************************/
	      /* get addressability to the sap control block          */
	      /********************************************************/

	      p->sapno = filter/2;
	      p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].
			   sap_cb_addr;
	      /********************************************************/
	      /* call the discovery manager with command =            */
	      /* add local name.                                      */
	      /********************************************************/

	      landiscv(p, ADDNO);
	      }                        /* endif: sap in-use           */
	}                              /* end select: sap started     */
#endif /* E3L or TRL or FDL */

#if defined(EDL)
      /**************************************************************/
      /* set the state of the physical link = opened.               */
      /**************************************************************/

      p->common_cb.type_80d5_started = TRUE;       /* defect 155401 */
      p->common_cb.plc_state = PLC_OPENED;

      /**************************************************************/
      /* call the discovery manager with command = add local name.  */
      /**************************************************************/

      landiscv(p, ADDNO);

#endif /* EDL */

/* setup values to record performance trace                           */

dlctype = HKWD_SYSX_DLC_PERF|DLC_TRACE_DHST;
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
      dlctype|+DLC_IEEE_802_3;
#endif

      /************************************************************/
      /* call trchklt to record performance data                  */
      /************************************************************/

      trchklt(dlctype, 0);

      /************************************************************/
      /* setup values to record monitor trace                     */
      /************************************************************/

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

      /************************************************************/
      /* get station number in upper half word and get number from*/
      /* port name in lower half word                             */
      /* Note: no station number here (set to 0)                  */
      /************************************************************/

#ifndef FDL
      port_sta = p->dlc_port.namestr[3];
#elif FDL
      port_sta = p->dlc_port.namestr[4];
#endif

      /************************************************************/
      /* call trchkgt to record monitor data                      */
      /************************************************************/

      trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, 0x07, 0, 0, 0, port_sta);

    }                                  /* endif physical link starting
					  device or opened            */
  TRACE1(p, "KDSe");
}                                      /* end filter_started          */



enable_link_sta(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  enable_link_sta                                      */
/*                                                                    */
/* descriptive name:  enable link station.                            */
/*                                                                    */
/* function:  locates an empty station entry in the station list,     */
/*            allocates station storage, and calls the station to     */
/*            initialize itself.                                      */
/*                                                                    */
/* input:  the input configuration and queue element.                 */
/*                                                                    */
/* output:  the station list is updated with the new station entry.   */
/*                                                                    */
/*** end of specifications ********************************************/

{
  int error,index,match_found;
  ushort op_res;
  int loca,locb;
  struct
    {
      struct dlc_sls_arg dlc_sls_arg;
#ifdef   TRL
      struct trl_start_psd trl_start_psd;
#endif
#ifdef   FDL
      struct fdl_start_psd fdl_start_psd;
#endif
    } 
  ls_config;
  ulong_t sta_size;

  TRACE1(p, "ELSb");

  /********************************************************************/
  /* preset the error indicator.                                      */
  /********************************************************************/

  error = TRUE;
  if                                   /* the plc is currently open   */
     (p->common_cb.plc_state == PLC_OPENED)
    {

      /****************************************************************/
      /* copy user's link station configuration data in temp area     */
      /****************************************************************/

      if (p->dlc_port.cid->state == KERN)
        bcopy(p->l.ls_config, &ls_config, sizeof(struct dlc_sls_arg));
      else
	copyin(p->l.ls_config, &ls_config, sizeof(struct dlc_sls_arg));
#ifdef   TRL

      /****************************************************************/
      /* copy user's link station configuration data in temp area     */
      /****************************************************************/

      if (p->dlc_port.cid->state == KERN)
	bcopy((char *)p->l.ls_config+sizeof(struct dlc_sls_arg),
	   (char*)&ls_config+sizeof(struct dlc_sls_arg), sizeof
	   (struct trl_start_psd));
      else
        copyin((char *)p->l.ls_config+sizeof(struct dlc_sls_arg), 
           (char *)&ls_config+sizeof(struct dlc_sls_arg), sizeof
           (struct trl_start_psd));
#endif
#ifdef   FDL

      /****************************************************************/
      /* copy user's link station configuration data in temp area     */
      /****************************************************************/

      if (p->dlc_port.cid->state == KERN)
	bcopy((char *)p->l.ls_config+sizeof(struct dlc_sls_arg),
	   (char*)&ls_config+sizeof(struct dlc_sls_arg), sizeof
	   (struct fdl_start_psd));
      else
        copyin((char *)p->l.ls_config+sizeof(struct dlc_sls_arg), 
           (char *)&ls_config+sizeof(struct dlc_sls_arg), sizeof
           (struct fdl_start_psd));
#endif
      if                               /* the specified sap is valid  
                                          (between 1 and 127)         */
         ((ls_config.dlc_sls_arg.gdlc_sap_corr > 0) && 
         (ls_config.dlc_sls_arg.gdlc_sap_corr < 128))
        {
          if                           /* the specified sap is        
                                          currently in use.           */
             (p->sap_list[ls_config.dlc_sls_arg.gdlc_sap_corr].in_use 
             == TRUE)
            {

              /********************************************************/
              /* get addressability to the sap.                       */
              /********************************************************/

              p->sap_ptr = (struct sap_cb *)p->sap_list
                 [ls_config.dlc_sls_arg.gdlc_sap_corr].sap_cb_addr;
              p->sapno = ls_config.dlc_sls_arg.gdlc_sap_corr;
              if                       /* the specified sap is        
                                          currently open.             */
                 (p->sap_ptr->sap_state == SAP_OPEN_STATE)
                {
                  if                   /* less than the specified     
                                          maximum stations are
                                          currently opened or opening 
                                          on the specified sap        */
                     (p->sap_ptr->sap_profile.max_ls > 
                     p->sap_ptr->num_sta_opened)
                    {

                      /************************************************/
                      /* validate the input configuration area.       */
                      /************************************************/

                      p->rc = validate_ls_config(&ls_config, p);
                      TRACE2(p, "VLSe", p->rc);
                      if               /* the input configuration is  
                                          valid                       */
                         (p->rc == RC_GOOD)
                        {

                          /********************************************/
                          /* preset the "match found" indicator FALSE.*/
                          /********************************************/

                          match_found = FALSE;

                          /********************************************/
                          /* for each of the station list entries     */
                          /********************************************/

                          for (index = 1; index < MAX_SESSIONS; index
                             ++)
                            {
                              if (match_found == TRUE)
				break;
			      if       /* the slot is not in use      */
				 (p->station_list[index].in_use ==
				 FALSE)
                                {

                                  /************************************/
                                  /* indicate that an empty slot was  */
                                  /* found.                           */
                                  /************************************/

                                  match_found = TRUE;

                                  /************************************/
                                  /* set the stationno to the index   */
                                  /* value found.                     */
                                  /************************************/

                                  p->stano = index;
                                } 
                            }          /* end for index;              */
                          if           /* an empty slot has been found
                                          in the station list         */
                             (match_found == TRUE)
                            {

                              /****************************************/
                              /* copy station correlator to user area */
                              /****************************************/

                              if (p->dlc_port.cid->state == KERN)
                                bcopy(&p->stano, 
                                   &p->l.ls_config->gdlc_ls_corr, 4);
                              else
                                copyout(&p->stano, 
                                   &p->l.ls_config->gdlc_ls_corr, 4);

                              /****************************************/
                              /* allocate the station control block   */
                              /* from the heap, word aligned.         */
                              /****************************************/

                              p->sta_ptr = (struct station_cb *)
                                 xmalloc(sizeof(struct station_cb), 2,
                                 kernel_heap);
                              if       /* there was storage available */
                                 (p->sta_ptr != (struct station_cb *)
                                 RC_NONE)
                                {
                                  bzero(p->sta_ptr, sizeof(struct 
                                     station_cb));

                                  /************************************/
                                  /* indicate that no errors occurred.*/
                                  /************************************/

                                  error = FALSE;

                                  /************************************/
                                  /* save the address of this control */
                                  /* block in the station list.       */
                                  /************************************/

                                  p->station_list[p->stano].
                                     sta_cb_addr = (int)p->sta_ptr;

                                  /************************************/
                                  /* indicate that this station list  */
                                  /* entry is in use.                 */
				  /************************************/

				  p->station_list[p->stano].in_use =
				     TRUE;

                                  /************************************/
                                  /* save the station's sap CORRELATOR*/
                                  /* in the station list.             */
                                  /************************************/

                                  p->station_list[p->stano].sapnum = 
                                     p->sapno;

                                  /************************************/
                                  /* increment the number of stations */
                                  /* counter.                         */
                                  /************************************/

                                  p->sap_ptr->num_sta_opened++;

                                  /************************************/
                                  /* reset all the station's internal */
                                  /* states and indicators.           */
                                  /************************************/

                                  loca = (int)
                                     &(p->sta_ptr->test_ir_pend);
				  locb = (int)&(p->sta_ptr->
				     ras_counters.counters.test_cmds_sent);

                                  /************************************/
                                  /* sta_clear_parameters             */
                                  /************************************/

                                  bzero(locb, (loca-locb));

                                  /************************************/
                                  /* copy the user's link station     */
                                  /* configuration into the station   */
                                  /* control block.                   */
				  /************************************/
/* LEHb defect 46592 */
				  bcopy(&ls_config,
					&(p->sta_ptr->ls_profile),
					sizeof(ls_config));
/* deleted 12 lines */
/* LEHe */
				  /************************************/
                                  /* call the link station with       */
                                  /* command=open.                    */
                                  /************************************/

                                  lansta(p, OPENNO);
                                  switch /* the resultant link station
                                          state.                      */
                                     (p->sta_ptr->ls)
                                    {
                                      case /* the open indicates a    
                                          connect-out (call pending)  */
					 (LS_CALL_PEND) :
					  {
					    if (p->sta_ptr->loopback
					       != 0)
                                              complete_local(p);
                                            else
                                              connect_out(p);
                                          } 
                                        break;
                                      case /* the open indicates a    
                                          connect-in                  */

                                      /********************************/
                                      /* (listen pending)             */
                                      /********************************/

                                         (LS_LISTEN_PEND) :
                                          {

/* <<< feature CDLI >>> */
                                        /******************************/
                                        /* call the "connect-in"      */
                                        /* routine to initiate the    */
                                        /* listen.                    */
					/******************************/
/* <<< end feature CDLI >>> */

                                            connect_in(p);
                                          } 
                                        break;
                                      default  :/* a failure occurred 
                                          in the station open.        */
                                          {

/* <<< feature CDLI >>> */
                                        /******************************/
                                        /* set error flag             */
					/******************************/
/* <<< end feature CDLI >>> */

                                            error = TRUE;
                                            lanerrlg(p, ERRID_LAN8015,
                                               NON_ALERT, 
                                               PERM_STA_ERR, 
                                               DLC_INV_RNAME, FILEN, 
                                               LINEN);

/* <<< feature CDLI >>> */
                                        /******************************/
                                        /* free station control block */
					/******************************/

					    assert(xmfree(p->sta_ptr,
						   kernel_heap) == 0);
/* <<< end feature CDLI >>> */
                                          } 
                                    } 
                                } 
                              else     /* station control block       
                                          storage is not available.   */
                                lanerrlg(p, ERRID_LAN8008, NON_ALERT, 
				   TEMP_ERR, DLC_LS_ROUT, FILEN, LINEN);
                            } 
                          else         /* no empty slot was found in  
                                          the station list.           */
                            lanerrlg(p, ERRID_LAN0029, NON_ALERT, 
                               TEMP_ERR, DLC_LS_ROUT, FILEN, LINEN);
                        } 
                      else             /* the input configuration is  
                                          not valid.                  */

                        /**********************************************/
                        /* note - the errid was returned from         */
                        /* validation                                 */
                        /**********************************************/

                        lanerrlg(p, p->rc, NON_ALERT, TEMP_ERR, 
                           DLC_USR_INTRF, FILEN, LINEN);
                    } 
                  else                 /* the maximum stations are    
                                          currently open.             */
                    lanerrlg(p, ERRID_LAN0029, NON_ALERT, TEMP_ERR, 
                       DLC_SESS_LIM, FILEN, LINEN);
                } 
              else                     /* the specified sap is not    
                                          currently open.             */
                lanerrlg(p, ERRID_LAN805A, NON_ALERT, TEMP_ERR, 
                   DLC_USR_INTRF, FILEN, LINEN);
            } 
          else                         /* the specified sap is not    
                                          currently in use.           */
            lanerrlg(p, ERRID_LAN805A, NON_ALERT, TEMP_ERR, 
               DLC_USR_INTRF, FILEN, LINEN);
        } 
      else                             /* the specified sap is not    
                                          valid (between 1 and 127)   */
        lanerrlg(p, ERRID_LAN805A, NON_ALERT, TEMP_ERR, DLC_USR_INTRF,
           FILEN, LINEN);
    } 
  else                                 /* the plc is not currently    
                                          open.                       */
    lanerrlg(p, ERRID_LAN0035, NON_ALERT, TEMP_ERR, DLC_USR_INTRF, 
       FILEN, LINEN);
  if                                   /* an error has occurred.      */
     (error == TRUE)
    op_res = EINVAL;
  else                                 /* no error occurred.          */

    /******************************************************************/
    /* set the open return code = ok.                                 */
    /******************************************************************/

    op_res = RC_GOOD;
  TRACE2(p, "ELSe", op_res);

  /********************************************************************/
  /* return with the open return code.                                */
  /********************************************************************/

  return (op_res);
}                                      /* end enable_link_sta;        */
validate_ls_config(p_to_ls,p)
  register struct dlc_sls_arg *p_to_ls;
  register struct port_dcl *p;
{
  struct p_to_lan
    {
      struct dlc_sls_arg dlc_sls_arg;
#ifdef   TRL
      struct trl_start_psd trl_start_psd;
#endif
#ifdef   FDL
      struct fdl_start_psd fdl_start_psd;
#endif
    } 

  *p_to_lan;
  TRACE1(p, "VLSb");
  if                                   /* this is an outgoing call,   
                                          and                         */

  /********************************************************************/
  /* the remote name length is 0 or exceeds 20 characters             */
  /********************************************************************/

     (((p_to_ls->flags&DLC_SLS_LSVC) != 0) && 
     ((p_to_ls->len_raddr_name == 0) || (p_to_ls->len_raddr_name > 
     sizeof(p_to_ls->raddr_name))))

    /******************************************************************/
    /* set the error code to "invalid remote address".                */
    /******************************************************************/

    return (ERRID_LAN8054);
  if                                   /* the maximum i-field is equal
                                          to zero.                    */
     (p_to_ls->maxif == 0)

    /******************************************************************/
    /* set the error code to "invalid maximum i-field length".        */
    /******************************************************************/

    return (ERRID_LAN8055);
  if                                   /* the receive window is not   
                                          between 1 and 127.          */
     ((p_to_ls->rcv_wind < 1) || (p_to_ls->rcv_wind > 127))

    /******************************************************************/
    /* set the error code to "invalid receive window".                */
    /******************************************************************/

    return (ERRID_LAN8056);
  if                                   /* the transmit window is not  
                                          between 1 and 127.          */
     ((p_to_ls->xmit_wind < 1) || (p_to_ls->xmit_wind > 127))

    /******************************************************************/
    /* set the error code to "invalid transmit window".               */
    /******************************************************************/

    return (ERRID_LAN8057);

/**** deleted ifdef TRL ****/

  if                                   /* calling and using resolve   
                                          procedures instead of       */

  /********************************************************************/
  /* name discovery                                                   */
  /********************************************************************/
/* LEH fixed DLC_SLS_ADDR check */
  (((p_to_ls->flags&DLC_SLS_LSVC) == DLC_SLS_LSVC) && ((p_to_ls->flags
  &DLC_SLS_ADDR) == DLC_SLS_ADDR))
    {
      if                               /* the remote sap is zero or   
                                          odd                         */
         ((p_to_ls->rsap == 0) || ((p_to_ls->rsap&1) != 0))

        /**************************************************************/
        /* set the error code to "invalid remote sap".                */
        /**************************************************************/

        return (ERRID_LAN8058);
     }
  if                      /* for openning as listen, check rsap range */
  ((p_to_ls->flags&DLC_SLS_LSVC) != DLC_SLS_LSVC)
     {
      if                               /* remote sap low is null sap, 
                                          or discovery sap, or        */

      /****************************************************************/
      /* has low bit set on                                           */
      /****************************************************************/

         ((p_to_ls->rsap_low == 0) || ((p_to_ls->rsap_low&1) != 0) || 
         (p_to_ls->rsap_low == 0xfc))

        /**************************************************************/
        /* set the error code to "invalid rsap low value".            */
        /**************************************************************/

        return (ERRID_LAN8064);
      if                               /* remote sap high is null sap,
                                          or discovery sap, or        */

      /****************************************************************/
      /* has low bit set on                                           */
      /****************************************************************/

         ((p_to_ls->rsap_high == 0) || ((p_to_ls->rsap_high&1) != 0) 
         || (p_to_ls->rsap_high == 0xfc))

        /**************************************************************/
        /* set the error code to "invalid rsap high value".           */
        /**************************************************************/

        return (ERRID_LAN8065);
      if                               /* remote sap range not valid  */
         (p_to_ls->rsap_low > p_to_ls->rsap_high)

        /**************************************************************/
        /* set the error code to "invalid rsap high value".           */
        /**************************************************************/

        return (ERRID_LAN8065);
    } 

/**** deleted endif TRL ****/

if                                     /* the maximum repoll is zero  */
   (p_to_ls->max_repoll == 0)

    /******************************************************************/
    /* set the error code to "invalid maximum repoll".                */
    /******************************************************************/

    return (ERRID_LAN8059);
  if                                   /* the repoll time is zero     */
     (p_to_ls->repoll_time == 0)

    /******************************************************************/
    /* set the error code to "invalid repoll time".                   */
    /******************************************************************/

    return (ERRID_LAN8060);
  if                                   /* the acknowledge time is zero*/
     (p_to_ls->ack_time == 0)

    /******************************************************************/
    /* set the error code to "invalid acknowledge time".              */
    /******************************************************************/

    return (ERRID_LAN8061);
  if                                   /* the inactivity time is zero */
     (p_to_ls->inact_time == 0)

    /******************************************************************/
    /* set the error code to "invalid inactivity time".               */
    /******************************************************************/

    return (ERRID_LAN8062);
  if                                   /* the force close time is zero*/
     (p_to_ls->force_time == 0)

    /******************************************************************/
    /* set the error code to "invalid force close time".              */
    /******************************************************************/

    return (ERRID_LAN8063);

#ifdef   TRLORFDDI
  /********************************************************************/
  /* setup pointer to token ring station data                         */
  /********************************************************************/

  p_to_lan = (struct p_to_lan *)p_to_ls;
  if                                   /* priority value not valid    */
     #ifdef TRL
     (p_to_lan->trl_start_psd.pkt_prty > 3)
     #elif FDL
     (p_to_lan->fdl_start_psd.pkt_prty > 7)
     #endif
     return (ERRID_LAN8066);

  if                                   /* dynamic window not valid    */
     #ifdef TRL
     (p_to_lan->trl_start_psd.dyna_wnd == 0)
     #elif FDL
     (p_to_lan->fdl_start_psd.dyna_wnd == 0)
     #endif
     return (ERRID_LAN8067);

#endif                                 /* TRLORFDDI                   */
return (RC_GOOD);
}                                      /* end validate_ls_config;     */

connect_out(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  connect_out                                          */
/*                                                                    */
/* descriptive name:  connect-out (call) routine.                     */
/*                                                                    */
/* function:  checks for remote name conflicts and closes the calling */
/*            station if a conflict occurs, or builds and transmits   */
/*            the discovery "find" packet to remote sap fc if no      */
/*            conflicts were found.                                   */
/*                                                                    */
/* input:  remote NETWORK name.                                       */
/*                                                                    */
/* output:  "write" is enqueued to the device handler, or a close     */
/*          of the calling station occurs.                            */
/*                                                                    */
/*** end of specifications ********************************************/

{
  char target_name[20];
  int target_length,index;
  char match_found;

  TRACE2(p, "COOb", p->stano);

/*--------------------------------------------------------------------*/
/* check all other stations (active or pending call completion)       */
/* that might conflict via an identical remote station name.          */
/*--------------------------------------------------------------------*/
   /* preset the match found indicator FALSE.                         */

  match_found = FALSE;

  /********************************************************************/
  /* get the call's target name and length.                           */
  /********************************************************************/

  target_length = p->sta_ptr->ls_profile.len_raddr_name;
  bcopy(p->sta_ptr->ls_profile.raddr_name, target_name, 
     p->sta_ptr->ls_profile.len_raddr_name);

  /********************************************************************/
  /* for each station in the station list                             */
  /********************************************************************/

  for (index = 1; index < MAX_SESSIONS; index++)
    {

      /****************************************************************/
      /* a match is found for the remote name                         */
      /****************************************************************/

      if (match_found == TRUE)
        break;
      if                               /* the station is fully active 
                                          or pending a call completion*/
         ((p->station_list[index].sta_active == TRUE) || 
         (p->station_list[index].call_pend == TRUE))
        {

          /************************************************************/
          /* get addressability to the station's control block.       */
          /************************************************************/

          p->sta_ptr = (struct station_cb *)p->station_list[index].
             sta_cb_addr;
          if                           /* the station's remote name   
                                          equals the target name      */

          /************************************************************/
          /* and not the current station and not for a different sap  */
          /************************************************************/

	     ((bcmp(p->sta_ptr->ls_profile.raddr_name, target_name,
						 target_length) == 0) &&
	     (p->sta_ptr->ls_profile.len_raddr_name == target_length) &&
	     (index != p->stano) &&
	     (p->l.ls_config->gdlc_sap_corr ==
				  p->sta_ptr->ls_profile.gdlc_sap_corr))
	    {

              /********************************************************/
              /* set the match found indicator.                       */
              /********************************************************/

              match_found = TRUE;

              /********************************************************/
              /* save the user's station CORRELATOR of the conflicting*/
              /* station.                                             */
              /********************************************************/

              p->sap_ptr->conflict_user_corr = 
                 p->sta_ptr->ls_profile.user_ls_corr;
              if (p->station_list[index].call_pend != TRUE)
                {
                  if (p->station_list[index].t1_ena == TRUE)
                    p->station_list[index].t1_ctr = 
                       p->sta_ptr->resp_to_val;
                  if (p->station_list[index].t3_ena == TRUE)
                    p->station_list[index].t3_ctr = 
                       p->sta_ptr->inact_to_val;
                } 
              p->sta_ptr = (struct station_cb *)p->station_list
                 [p->stano].sta_cb_addr;

              /********************************************************/
              /* call error log - attach conflict                     */
              /********************************************************/

              lanerrlg(p, ERRID_LAN805D, NON_ALERT, PERM_STA_ERR, 
                 DLC_REMOTE_CONN, FILEN, LINEN);

              /********************************************************/
              /* call the link station with command = close.          */
              /********************************************************/

              lansta(p, CLOSENO);
            } 
        } 
    }                                  /* end do index;               */
  if                                   /* no conflicting stations were
                                          found                       */
     (match_found == FALSE)
    {

      /****************************************************************/
      /* call the discover manager with command = call.               */
      /****************************************************************/

      landiscv(p, CALLNO);
    } 
}                                      /* end connect_out;            */
connect_in(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  connect_in                                           */
/*                                                                    */
/* descriptive name:  connect-in (listen) routine.                    */
/*                                                                    */
/* function:  saves the local station sap and p->stano                */
/*            in a special "listen pending" area for any subsequent   */
/*            incomming call.                                         */
/*                                                                    */
/* input:  local station sap and p->stano.                            */
/*                                                                    */
/* output:  the input is saved.                                       */
/*                                                                    */
/*** end of specifications ********************************************/

{
  TRACE2(p, "COIe", p->stano);
  if                                   /* a listen is not already in  
                                          progress                    */
     (p->sap_list[p->sapno].listen_pend == FALSE)
    {

      /****************************************************************/
      /* save the station index that is pending listen completion.    */
      /****************************************************************/

      p->sap_ptr->listen_stano = p->stano;

      /****************************************************************/
      /* indicate that a listen is now pending an incomming call.     */
      /****************************************************************/

      p->sap_list[p->sapno].listen_pend = TRUE;
    } 
  else                                 /* error - cannot accept more  
                                          than 1 listen at a time.    */
    {

      /****************************************************************/
      /* call error log - user interface error, link station shutdown */
      /* = listen already in progress.                                */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN0018, NON_ALERT, PERM_STA_ERR, 
         DLC_LST_IN_PRGS, FILEN, LINEN);

      /****************************************************************/
      /* call the link station with command = close.                  */
      /****************************************************************/

      lansta(p, CLOSENO);
    } 
}                                      /* end connect_in;             */
close_sap_cmd(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  close_sap_cmd                                        */
/*                                                                    */
/* descriptive name:  close service access point command              */
/*                                                                    */
/* function:  checks for a valid input sap CORRELATOR and calls the   */
/*            sap shutdown routine.                                   */
/*                                                                    */
/* input:  close command input queue element                          */
/*                                                                    */
/* output:  sap shutdown is called                                    */
/*                                                                    */
/*** end of specifications ********************************************/

{
  int error,index;

  TRACE1(p, "CSCb");

  /********************************************************************/
  /* preset the error indicator = TRUE.                               */
  /********************************************************************/

  error = TRUE;
  if                                   /* the state of the physical   
                                          link is not closing or      
                                          closed                      */
     ((p->common_cb.plc_state != PLC_CLOSING) && 
     (p->common_cb.plc_state != PLC_CLOSED))
    {

      /****************************************************************/
      /* get the sap CORRELATOR from the input queue element.         */
      /****************************************************************/

      p->sapno = p->l.sap_config->gdlc_sap_corr;
      if                               /* the specified sap CORRELATOR
                                          is within range             */
         ((p->sapno > 0) && (p->sapno < 128))
        {
          if                           /* the specified sap is        
                                          currently in use.           */
             (p->sap_list[p->sapno].in_use == TRUE)
            {

              /********************************************************/
              /* get addressability to the sap control block.         */
              /********************************************************/

              p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].
                 sap_cb_addr;

              /********************************************************/
              /* call the sap shutdown routine.                       */
              /********************************************************/

              sap_shutdown(p);
            } 
          else                         /* the specified sap is not    
                                          currently in use.           */
            {

              /********************************************************/
              /* call error log - close sap command - invalid sap     */
              /* correlator                                           */
              /********************************************************/

              lanerrlg(p, ERRID_LAN8052, NON_ALERT, INFO_ERR, 0, 
                 FILEN, LINEN);
              return (EINVAL);
            } 
        } 
      else                             /* the specified sap CORRELATOR
                                          is not valid                */
        {

          /************************************************************/
          /* call error log - close sap command - invalid sap         */
          /* correlator.                                              */
          /************************************************************/

          lanerrlg(p, ERRID_LAN8052, NON_ALERT, INFO_ERR, 0, FILEN, 
             LINEN);
          return (EINVAL);
        } 
    } 
  else                                 /* the plc is not currently    
                                          open                        */
    {

      /****************************************************************/
      /* call error log - close sap command - plc already closed.     */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN8035, NON_ALERT, INFO_ERR, 0, FILEN, LINEN);
      return (EIO);
    } 
  return (0);
}                                      /* end close_sap_cmd;          */
close_link_sta(p,ext)
  register struct port_dcl *p;
  register struct dlc_io_ext *ext;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  close_link_sta                                       */
/*                                                                    */
/* descriptive name:  close link station                              */
/*                                                                    */
/* function:  calls the link station to be closed, who then removes   */
/*            itself from the station list and free's its             */
/*            storage area.                                           */
/*                                                                    */
/* input:  close command input queue element                          */
/*                                                                    */
/* output:  link station is called.                                   */
/*                                                                    */
/*** end of specifications ********************************************/

{
  int error,match_found;

  TRACE1(p, "CLSb");
  bcopy(ext, &(p->dlc_io_ext), sizeof(struct dlc_io_ext));

  /********************************************************************/
  /* preset the error indicator = TRUE.                               */
  /********************************************************************/

  error = TRUE;
  if                                   /* the plc is currently open   */
     (p->common_cb.plc_state == PLC_OPENED)
    {

      /****************************************************************/
      /* get the link station correlator from the input queue element.*/
      /****************************************************************/

      p->stano = p->dlc_io_ext.ls_corr;
      if                               /* the specified link station  
                                          correlator is within range  */
         ((p->stano > 0) && (p->stano <= MAX_SESSIONS))
        {
          if                           /* the specified link station  
                                          is currently in use.        */
             (p->station_list[p->stano].in_use == TRUE)
            {

              /********************************************************/
              /* get addressability to the link station.              */
              /********************************************************/

              p->sta_ptr = (struct station_cb *)p->station_list
                 [p->stano].sta_cb_addr;

              /********************************************************/
              /* get addressability to the station's sap.             */
              /********************************************************/

              p->sapno = p->station_list[p->stano].sapnum;
              p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].
                 sap_cb_addr;
              if                       /* the input sap correlator is 
                                          not valid, or               */

              /********************************************************/
              /* the sap is not in use                                */
              /********************************************************/

                 ((p->sap_ptr->sap_profile.local_sap != 
                 (p->dlc_io_ext.sap_corr *2)) || (p->sap_list[p->sapno
                 ].in_use == FALSE))
                {

                  /****************************************************/
                  /* call error log - invalid sap correlator          */
                  /****************************************************/

                  lanerrlg(p, ERRID_LAN8091, NON_ALERT, 0, 0, FILEN, 
                     LINEN);
                  return (EINVAL);
                } 
              else                     /* addressability is ok        */
                {

                  /****************************************************/
                  /* call the link station with command = close.      */
                  /****************************************************/

                  lansta(p, CLOSENO);
                } 
            } 
          else                         /* the specified link station  
                                          is not currently in use.    */
            {

              /********************************************************/
              /* call error log - invalid ls correlator               */
              /********************************************************/

	      lanerrlg(p, ERRID_LAN8092, NON_ALERT, 0, 0, FILEN, LINEN);
              return (EINVAL);
            } 
        } 
      else                             /* the specified link station  
                                          correlator is not valid     */
        {

          /************************************************************/
          /* call error log - invalid ls correlator                   */
          /************************************************************/

          lanerrlg(p, ERRID_LAN8092, NON_ALERT, 0, 0, FILEN, LINEN);
          return (EINVAL);
        } 
    } 
  else                                 /* the plc is not currently    
                                          open                        */
    {

      /****************************************************************/
      /* call error log - close link station - plc not open.          */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN8035, NON_ALERT, 0, 0, FILEN, LINEN);
      return (EINVAL);
    } 
  return (0);
}                                      /* end close_link_sta;         */
sap_shutdown(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  sap_shutdown                                         */
/*                                                                    */
/* descriptive name:  service access point shutdown                   */
/*                                                                    */
/* function:  closes any remaining logical links on the sap and       */
/*            notifies the user when the sap is closed.               */
/*                                                                    */
/* input:  none.                                                      */
/*                                                                    */
/* output: "sap closed" psb                                           */
/*                                                                    */
/*** end of specifications ********************************************/

{
/* <<< feature CDLI >>> */
  struct ns_8022  ns_filter;
/* <<< end feature CDLI >>> */
/* defect 142249 */
  char tempaddr[6];
/* end defect 142249 */
  int index;

  TRACE1(p, "SSHb");
  if                                   /* the sap is still in use     */
     (p->sap_list[p->sapno].in_use == TRUE)
    {
/* LEHb defect 43788 */
      if                               /* sap local busy mode due to
					  network data                */
	 (p->sap_ptr->retry_rcvn_buf != 0)
	{
				       /* then decrement the common sap
					  local busy counter.         */
	p->common_cb.lbusy_ctr--;
				       /* indicate that a sap wakeup is
					  no longer needed */
	p->sap_list[p->sapno].wakeup_needed = FALSE;
				       /* return the buffer to the
					  system buffer pool.         */
	m_freem (p->sap_ptr->retry_rcvn_buf);
				       /* clear the netdata buffer ptr */
	p->sap_ptr->retry_rcvn_buf = 0;
	}
/* LEHe */

      if                               /* no sap return code has      
                                          already been established    */
         (p->sap_ptr->sap_retcode == 0)
        {

          /************************************************************/
          /* set the sap return code = ABORTED by user, to keep the   */
          /* individual stations from issuing "ls closed" results.    */
          /************************************************************/

          p->sap_ptr->sap_retcode = ABORTED;
        } 

      /****************************************************************/
      /* for index = 1 to maximum number of stations on this sap      */
      /****************************************************************/

      for (index = 1; index <= MAX_SESSIONS; index++)
        {
          if                           /* the lsap matches, and the   
                                          station is in use           */
             ((p->station_list[index].sapnum == p->sapno) && 
             (p->station_list[index].in_use == TRUE))
            {

              /********************************************************/
              /* get addressability to the station.                   */
              /********************************************************/

              p->stano = index;
              p->sta_ptr = (struct station_cb *)p->station_list
                 [p->stano].sta_cb_addr;

              /********************************************************/
              /* call the link station with command = close.          */
              /********************************************************/

              lansta(p, CLOSENO);
            } 
        }                              /* end do index;               */
      if                               /* the sap return code =       
                                          ABORTED by user             */
         (p->sap_ptr->sap_retcode == ABORTED)
        {

          /************************************************************/
          /* set the sap return code back to NORMAL.                  */
          /************************************************************/

          p->sap_ptr->sap_retcode = NORMAL;
        } 
#ifndef  EDL
	if                             /* the sap shutdown is not due
					  to a plc failure and it's
					  not the LS/X RIPL sap 0xFC  */
	 ((p->common_cb.plc_retcode == DLC_OK)  &&
	  (p->sapno != DISCOVERY_SAP/2))
        {
          p->sap_ptr->sap_state = SAP_CLOSE_PEND;

/* <<< feature CDLI >>> */

	  /********************************************************/
	  /* issue ns del filter to DH for user sap               */
	  /********************************************************/

	  TRACE2(p, "DELF", p->sapno *2);
	  bzero(&ns_filter, sizeof(ns_filter));
	  ns_filter.filtertype = NS_8022_LLC_DSAP;
	  ns_filter.dsap = p->sapno *2;

	  p->rc = ns_del_filter(p->nddp, &ns_filter, sizeof(ns_filter));
	  if /* bad return code */
             (p->rc != 0)
	    {
              /* defect 167068 */
              TRACE4(p, "DELF", 0xBADBAD00 , p->rc , (p->sapno *2));

	      /**********************************************************/
	      /* call error log - unexpected return code from           */
	      /* ns_del_filter                                          */
	      /**********************************************************/

	      lanerrlg(p, ERRID_LAN802D, NON_ALERT, PERM_PLC_ERR,
		       DLC_SAP_ROUT, FILEN, LINEN);
	    }
	  else /* del_filter worked ok */
	    {
	      /**********************************************************/
	      /* call user_filter_halted to cleanup sap and notify user */
	      /**********************************************************/

	      user_filter_halted(p);
	    }
/* <<< end feature CDLI >>> */
        } 
      else                             /* don't issue the halt, the   
                                          device will be closed       */

        /**************************************************************/
        /* just notify the user that the sap is disabled              */
        /**************************************************************/

        {
#endif                                  /* not EDL                     */
          /************************************************************/
	  /*   wakeup any read or write processes                     */
          /************************************************************/
          e_wakeup((int *) &p->sap_ptr->user_sap_channel->readsleep);
          e_wakeup((int *) &p->sap_ptr->user_sap_channel->writesleep);
          
/* defect 142249 */
          lansrslt(p, p->sap_ptr->sap_retcode, DLC_SAPD_RES, 
                   p->sap_ptr->sap_profile.user_sap_corr, 0);

	  /**************************************************/
	  /* get the channel id that enabled this sap       */
	  /**************************************************/
          p->dlc_port.cid = p->sap_ptr->user_sap_channel;

          /************************************************************/
          /* call llget to fetch address and release storage used in  */
          /* multicast (group) address list                           */
          /************************************************************/
   
          while /* there are group addresses in the sap's list */
                (llget (&tempaddr[0],p))
            { 
              /* issue ndd disable address to the device driver */
              p->rc = (*p->nddp->ndd_ctl)(p->nddp,
                                NDD_DISABLE_ADDRESS, &tempaddr[0], 6);
            }
/* end defect 142249 */
               
          /************************************************************/
          /* deallocate the sap control block.                        */
          /************************************************************/
/* <<< feature CDLI >>> */
	  assert(xmfree(p->sap_ptr, kernel_heap) == 0);
/* <<< end feature CDLI >>> */

          /************************************************************/
          /* clear this sap entry from the sap list.                  */
          /************************************************************/

	  bzero(&p->sap_list[p->sapno], sizeof(p->sap_list[p->sapno]));

          /************************************************************/
          /* set the sap's t1 counter value to a large countdown      */
          /* value.                                                   */
          /************************************************************/

          p->sap_list[p->sapno].t1_ctr = -1;

/* defect 122577 */
  simple_lock(&p->dlc_port.cid->lock);
/* end defect 122577 */


                     {

              /********************************************************/
              /* decrement the number of sap's open counter.          */
              /********************************************************/

              p->dlc_port.cid->saps--;
              if                       /* the number decremented past 
                                          zero                        */
                 ((long)p->dlc_port.cid->saps < 0)

                /******************************************************/
                /* call error log - coding error                      */
                /******************************************************/

                lanerrlg(p, ERRID_LAN8011, NON_ALERT, PERM_PLC_ERR, 
                   DLC_ERR_CODE, FILEN, LINEN);

/* defect 122577 */
  simple_unlock(&p->dlc_port.cid->lock);
/* end defect 122577 */

             

            }
 

#ifndef  EDL
        } 
#endif                                  /* not EDL                     */
    }                                  /* endif sap in use            */
  TRACE1(p, "SSHe");
}                                      /* end sap_shutdown;           */

plc_blowout(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  plc_blowout                                          */
/*                                                                    */
/* descriptive name:  physical link blowout                           */
/*                                                                    */
/* function:  closes all saps which shuts down the entire plc.        */
/*                                                                    */
/* input:  none.                                                      */
/*                                                                    */
/* output:  all saps are closed                                       */
/*                                                                    */
/*** end of specifications ********************************************/

{
  int index;
  int rtn;

  TRACE1(p, "PLCb");
  if                                   /* the state of the physical   
                                          link is not closing or      
                                          closed                      */
     ((p->common_cb.plc_state != PLC_CLOSING) && 
     (p->common_cb.plc_state != PLC_CLOSED))
    {

      /****************************************************************/
      /* for index = 0 to 127 possible saps                           */
      /****************************************************************/

      for (index = 0; index <= 127; index++)
        {
          if                           /* the sap is in use           */
             (p->sap_list[index].in_use == TRUE)
            {

              /********************************************************/
              /* get addressability to the sap control block          */
              /********************************************************/

              p->sapno = index;
              p->sap_ptr = (struct sap_cb *)p->sap_list[index].
                 sap_cb_addr;

              /********************************************************/
              /* copy the plc shutdown reason code to the sap control */
              /* block.                                               */
              /********************************************************/

              p->sap_ptr->sap_retcode = p->common_cb.plc_retcode;

              /********************************************************/
              /* call the sap shutdown routine.                       */
              /********************************************************/

              sap_shutdown(p);
            } 
        }                              /* end for index;              */
/* <<< feature CDLI >>> */

      /********************************************************/
      /* call halt_ndd to delete the dlc filters for saps     */
      /* 0x00 and 0xFC and ethertype 0x80D5 and async status. */
      /* Also frees the ndd.                                  */
      /********************************************************/

      halt_ndd(p);

/* <<< end feature CDLI >>> */

      /****************************************************************/
      /* reset plc return code to prevent looping in lankproc         */
      /****************************************************************/

      p->common_cb.plc_retcode = DLC_OK;
    }                                  /* endif plc already closing or
                                          closed                      */
  TRACE1(p, "PLCe");
}                                      /* end plc_blowout;            */

/* <<< feature CDLI >>> */
halt_ndd(p)
  register struct port_dcl *p;         /* port control block          */
{
  struct ns_8022        ns_filter;
  struct ns_com_status  ns_statfilter;
  ulong    dlctype;                    /* lan type                    */
  ulong_t  port_sta;                   /* station and port numbers    */

  TRACE1(p, "CLOb");

  if /* the device handler still exists */
     (p->nddp != 0)
    {
      /********************************************************/
      /* setup lan and monitor types for trace                */
      /********************************************************/

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

      /********************************************************/
      /* get station number in upper half word and get number */
      /* from port name in lower half word                    */
      /********************************************************/

#ifndef FDL
      port_sta = ((p->stano<<16)|p->dlc_port.namestr[3]);
#elif FDL
      port_sta = ((p->stano<<16)|p->dlc_port.namestr[4]);
#endif

      /****************************************************/
      /* call trchkgt to record monitor data              */
      /****************************************************/

      trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, DLC_TRACE_ISND, 0, 0, 0,
	      port_sta);

#ifndef EDL
/* defect 155401 */
      if /* sap 0x00 was activated */
	(p->common_cb.resolve_started == TRUE)
      {
	/* reset the resolve_started indicator */
	p->common_cb.resolve_started = FALSE;
/* end defect 155401 */
	/********************************************************/
	/* issue ns del filter to DH for the 0x00 sap           */
	/********************************************************/

	TRACE2(p, "DELF", NULL_SAP);

	bzero(&ns_filter, sizeof(ns_filter));
	ns_filter.filtertype = NS_8022_LLC_DSAP;
	ns_filter.dsap = NULL_SAP;
	p->rc = ns_del_filter(p->nddp, &ns_filter, sizeof(ns_filter));
/*      assert (p->rc == 0)     ** assert if bad return from ns_del_filter */
/* defect 155401 */
        /* defect 167068 */
        if /* bad return code */
           (p->rc != 0)
           TRACE4(p, "DELF", 0xBADBAD00 , p->rc , NULL_SAP);
      }

      if /* sap 0xFC was activated */
	(p->common_cb.discovery_started == TRUE)
      {
	/* reset the discovery_started indicator */
	p->common_cb.discovery_started = FALSE;
/* end defect 155401 */
	/********************************************************/
	/* issue ns del filter to DH for the 0xFC sap           */
	/********************************************************/

	TRACE2(p, "DELF", DISCOVERY_SAP);
	ns_filter.dsap = DISCOVERY_SAP;
	p->rc = ns_del_filter(p->nddp, &ns_filter, sizeof(ns_filter));
/*      assert (p->rc == 0)     ** assert if bad return from ns_del_filter */
      } /* defect 155401 */
        /* defect 167068 */
        if /* bad return code */
           (p->rc != 0)
           TRACE4(p, "DELF", 0xBADBAD00 , p->rc , DISCOVERY_SAP);
#endif /* not EDL */

#ifdef EDL
      if /* type field 0x80D5 was activated */
	(p->common_cb.type_80d5_started == TRUE)
      {
	/* reset the type_80d5_started indicator */
	p->common_cb.type_80d5_started = FALSE;
/* end defect 155401 */
	/********************************************************/
	/* issue ns del filter to DH for the 0x80D5 ethertype   */
	/********************************************************/

	TRACE2(p, "DELF", 0x80D5);  /* defect 167068 */
	bzero(&ns_filter, sizeof(ns_filter));
	ns_filter.filtertype = NS_ETHERTYPE;  /* ethertype only */
	ns_filter.ethertype = 0x80d5;         /* enable 0x80D5  */
	p->rc = ns_del_filter(p->nddp, &ns_filter, sizeof(ns_filter));
/*      assert (p->rc == 0)     ** assert if bad return from ns_del_filter */
      } /* defect 155401 */
        if /* bad return code */
           (p->rc != 0)
           TRACE4(p, "DELF", 0xBADBAD00 , p->rc , 0x80d5);
#endif /* EDL */

	/********************************************************/
	/* remove asyncronous status capability from the device */
	/* preset ns_statfilter structure                       */
	/********************************************************/
	TRACE1(p, "DELS");
	bzero(&ns_statfilter, sizeof(ns_statfilter));
	ns_statfilter.filtertype = NS_STATUS_MASK;
	ns_statfilter.mask = (NDD_HARD_FAIL | NDD_CONNECTED);

	/* pick up the status id (sid) from the port cb */
	ns_statfilter.sid = p->statfilter_sid;

	/********************************************************/
	/* issue ns delete status to DH                         */
	/* ignore any bad return code                           */
	/********************************************************/
	p->rc = ns_del_status(p->nddp, &ns_statfilter,
					   sizeof(ns_statfilter));

	/************************************************************/
	/* remove access to the device with ns_free                 */
	/************************************************************/
	TRACE1(p, "NSFR");
	ns_free(p->nddp);

	/************************************************************/
	/* reset the device handler's ndd pointer so that we don't  */
	/* try to close this device again.                          */
	/************************************************************/

	p->nddp = 0;

	/****************************************************************/
	/* indicate that the physical link is closed                    */
	/****************************************************************/

	p->common_cb.plc_state = PLC_CLOSED;

	/****************************************************/
	/* call trchkgt to record monitor data              */
	/****************************************************/

	trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, DLC_TRACE_ISND, 0, 0, 0, 0);

    }

  TRACE1(p, "CLOe");

} /* end halt_ndd */

user_filter_halted(p)
  register struct port_dcl *p;         /* port control block          */
{
  ulong    rtn;                        /* function handler return code*/
  struct dlc_chan *c_ptr;              /* pointer to channel          */
  struct dlc_getx_arg st_arg;          /* function handler return
					  values                      */
/* defect 142249 */
  char tempaddr[6];
/* end defect 142249 */
#ifdef   DEBUG

  if (p->debug)
     printf("Filter_Halted sapno=%d state=%x\n", p->sapno,
						 p->sap_ptr->sap_state);
#endif

  if /* this sap is in-use */
     (p->sap_list[p->sapno].in_use == TRUE)
    {

      /******************************************************/
      /* get addressability to the sap control block        */
      /******************************************************/

      p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].
	 sap_cb_addr;

      if /* sap state is "closing" */
	 ((p->sap_ptr->sap_state == SAP_CLOSE_PEND) ||
	 (p->sap_ptr->sap_state == SAP_ABORTED))
	{

	  if /* it was a normal halt */
	     (p->sap_ptr->sap_state != SAP_ABORTED)
	    {

	      if /* the sap is in "limbo" mode, ie. the device handler
		    could not complete a write to the adapter  */
		 (p->sap_ptr->sap_limbo != 0)
		{
		  /********************************************/
		  /* override the sap_retcode with unusual    */
		  /* network cond                             */
		  /********************************************/

		  p->sap_ptr->sap_retcode = DLC_SAP_NT_COND;
		}

	      /**********************************************/
	      /* fill in the exception handler return values*/
	      /**********************************************/

	      st_arg.result_code = p->sap_ptr->sap_retcode;
	      st_arg.result_ind = DLC_SAPD_RES;
	      st_arg.user_sap_corr =
		 p->sap_ptr->sap_profile.user_sap_corr;
	      st_arg.user_ls_corr = 0;

	      /**********************************************/
	      /* setup pointer to channel to get functional */
	      /* address                                    */
	      /**********************************************/

	      c_ptr = p->sap_ptr->user_sap_channel;

	      /****************************************************/
	      /* call user's exception handler with "sap disabled */
	      /****************************************************/

	      TRACE3(p, "XFAb", DLC_SAPD_RES, p->sap_ptr->sap_retcode);
	      rtn = (*c_ptr->excp_fa)(&st_arg, c_ptr);
	      TRACE1(p, "XFAe");
	    } /* endif normal halt */

	  /**************************************************/
	  /* get the channel id that enabled this sap       */
	  /**************************************************/
	  p->dlc_port.cid = p->sap_ptr->user_sap_channel;

/* defect 142249 */
          /************************************************************/
          /* call llget to fetch address and release storage used in  */
          /* multicast (group) address list                           */
          /************************************************************/
   
          while /* there are group addresses in the sap's list */
                (llget (&tempaddr[0],p))
            { 
              /* issue ndd disable address to the device driver */
              p->rc = (*p->nddp->ndd_ctl)(p->nddp,
                                NDD_DISABLE_ADDRESS, &tempaddr[0], 6);
            }
/* end defect 142249 */
               
	  /**************************************************/
	  /* free the sap control block                     */
	  /**************************************************/
/* <<< feature CDLI >>> */
	  assert(xmfree(p->sap_ptr, kernel_heap) == 0);
/* <<< end feature CDLI >>> */
	  /**************************************************/
	  /* clear this sap entry from the sap list.        */
	  /**************************************************/

	  bzero(&p->sap_list[p->sapno], sizeof(p->sap_list[p->sapno]));

	  /**************************************************/
	  /* set the sap's t1 counter value to a large      */
	  /* countdown value.                               */
	  /**************************************************/

	  p->sap_list[p->sapno].t1_ctr = -1;

	  /**************************************************/
	  /* lock the channel                               */
	  /**************************************************/

/* defect 122577 */
  simple_lock(&p->dlc_port.cid->lock);
/* end defect 122577 */


	  /**************************************************/
	  /* decrement the number of saps enabled           */
	  /**************************************************/

	  p->dlc_port.cid->saps--;

	  if ((long)p->dlc_port.cid->saps < 0)

	     /************************************************/
	     /* ERROR: saps decremented below zero           */
	     /************************************************/

	     lanerrlg(p, ERRID_LAN8011, NON_ALERT,
		      PERM_PLC_ERR, 0, FILEN, LINEN);

	  /**************************************************/
	  /* unlock the channel                             */
	  /**************************************************/

/* defect 122577 */
  simple_unlock(&p->dlc_port.cid->lock);
/* end defect 122577 */





	} /* endif sap is closing */
    } /* endif sap is in use */

} /* end user_filter_halted */
/* <<< end feature CDLI >>> */

query_link_sta(p,ls_result)
  register struct port_dcl *p;
  char *ls_result;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  query_link_sta                                       */
/*                                                                    */
/* descriptive name:  query link station                              */
/*                                                                    */
/* function:  calls the link station to be queried, who then builds   */
/*            the statistics buffer.                                  */
/*                                                                    */
/* input:  query command input queue element                          */
/*                                                                    */
/* output:  link station is called.                                   */
/*                                                                    */
/*** end of specifications ********************************************/

{
  TRACE1(p, "QLSb");
  if                                   /* the plc is currently open   */
     (p->common_cb.plc_state == PLC_OPENED)
    {

      /****************************************************************/
      /* get the query input parameters from the user's space and put */
      /* them in the port control block so that the link station can  */
      /* get at them easily.                                          */
      /****************************************************************/

      if (p->dlc_port.cid->state == KERN)
	bcopy(ls_result, &(p->dlc_qls_arg), sizeof(struct dlc_qls_arg));
      else
	copyin(ls_result, &(p->dlc_qls_arg), sizeof(struct dlc_qls_arg));

      /****************************************************************/
      /* get the link station CORRELATOR from the input query parms   */
      /****************************************************************/

      p->stano = p->dlc_qls_arg.gdlc_ls_corr;
      if                               /* the specified link station  
                                          CORRELATOR is within range  */
         ((p->stano > 0) && (p->stano <= MAX_SESSIONS))
        {
          if                           /* the specified link station  
                                          is currently in use.        */
             (p->station_list[p->stano].in_use == TRUE)
            {

              /********************************************************/
              /* get addressability to the link station.              */
              /********************************************************/

              p->sta_ptr = (struct station_cb *)p->station_list
                 [p->stano].sta_cb_addr;

              /********************************************************/
              /* get the sap CORRELATOR from the input query parms    */
              /********************************************************/

              p->sapno = p->dlc_qls_arg.gdlc_sap_corr;
              if                       /* the specified sap CORRELATOR
                                          is within range             */

              /********************************************************/
              /* and the sap is currently in use.                     */
              /********************************************************/

                 ((p->sapno > 0) && (p->sapno < 128) && (p->sap_list
                 [p->sapno].in_use == TRUE))
                {

                  /****************************************************/
                  /* get addressability to the sap control block.     */
                  /****************************************************/

                  p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].
                     sap_cb_addr;

                  /****************************************************/
                  /* put the user's sap correlator in the status block*/
                  /****************************************************/

                  p->dlc_qls_arg.user_sap_corr = 
                     p->sap_ptr->sap_profile.user_sap_corr;

                  /****************************************************/
                  /* call the link station with command = query.      */
                  /****************************************************/

                  lansta(p, QUERYNO);

                  /****************************************************/
                  /* write out the query status to the user's space   */
                  /****************************************************/

                  if                   /* kernel space                */
                     (p->dlc_port.cid->state == KERN)
                    bcopy(&(p->dlc_qls_arg), ls_result, sizeof(struct 
                       dlc_qls_arg));
                  else                 /* application space           */
                    copyout(&(p->dlc_qls_arg), ls_result, sizeof
                       (struct dlc_qls_arg));
                } 
              else                     /* sap addressability is bad   */
                {

                  /****************************************************/
                  /* call error log - invalid sap correlator          */
                  /****************************************************/

                  lanerrlg(p, ERRID_LAN8074, NON_ALERT, 0, 0, FILEN, 
                     LINEN);
                  return (EINVAL);
                } 
            } 
          else                         /* the specified link station  
                                          is not currently in use.    */
            {

              /********************************************************/
              /* call error log - invalid ls correlator               */
              /********************************************************/

	      lanerrlg(p, ERRID_LAN8075, NON_ALERT, 0, 0, FILEN, LINEN);
              return (EINVAL);
            } 
        } 
      else                             /* the specified link station  
                                          CORRELATOR is not valid     */
        {

          /************************************************************/
          /* call error log - invalid ls correlator.                  */
          /************************************************************/

          lanerrlg(p, ERRID_LAN8075, NON_ALERT, 0, 0, FILEN, LINEN);
          return (EINVAL);
        } 
    } 
  else                                 /* the plc is not currently    
                                          open                        */
    {

      /****************************************************************/
      /* call error log - plc not open.                               */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN0035, NON_ALERT, 0, 0, FILEN, LINEN);
      return (EIO);
    } 
  return (0);
}                                      /* end query_link_sta;         */
query_sap_cmd(p,sap_result)
  register struct port_dcl *p;
  char *sap_result;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  query_sap_cmd                                        */
/*                                                                    */
/* descriptive name:  query service access point command              */
/*                                                                    */
/* function:  checks for a valid input sap CORRELATOR and calls the   */
/*            sap shutdown routine.                                   */
/*                                                                    */
/* input:  close command input queue element                          */
/*                                                                    */
/* output:  sap shutdown is called                                    */
/*                                                                    */
/*** end of specifications ********************************************/

{
/* <<< feature CDLI >>> */
#ifdef  TRL
#define MAX_DH_STATS (sizeof(tok_ndd_stats_t))
#endif

#ifdef  FDL
#define MAX_DH_STATS (sizeof(fddi_ndd_stats_t))
#endif

#ifndef TRLORFDDI
#define MAX_DH_STATS (sizeof(ent_ndd_stats_t))
#endif
/* <<< end feature CDLI >>> */

struct qsap_total
    {
      struct dlc_qsap_arg qsap_arg;
      uchar_t qsap_dh[MAX_DH_STATS];
    } 
  qsap_total;

  TRACE1(p, "QSCb");

  /********************************************************************/
  /* get the query input parameters from the user's space             */
  /********************************************************************/

  if (p->dlc_port.cid->state == KERN)
    bcopy(sap_result, &qsap_total.qsap_arg, sizeof(struct dlc_qsap_arg));
  else
    copyin(sap_result, &qsap_total.qsap_arg, sizeof(struct 
       dlc_qsap_arg));
  if                                   /* the plc is currently open   */
     (p->common_cb.plc_state == PLC_OPENED)
    {

      /****************************************************************/
      /* get the sap CORRELATOR from the input queue element.         */
      /****************************************************************/

      p->sapno = qsap_total.qsap_arg.gdlc_sap_corr;
      if                               /* the specified sap CORRELATOR
                                          is within range             */
         ((p->sapno > 0) && (p->sapno < 128))
        {
          if                           /* the specified sap is        
                                          currently in use.           */
             (p->sap_list[p->sapno].in_use == TRUE)
            {

              /********************************************************/
              /* get addressability to the sap control block.         */
              /********************************************************/

              p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].
                 sap_cb_addr;

              /********************************************************/
              /* check the length of the user's status area           */
              /********************************************************/

              if                       /* greater than the maximum    
                                          device handler status       
                                          allowed                     */
                 (qsap_total.qsap_arg.devdd_len > MAX_DH_STATS)

                /******************************************************/
                /* set the length to maximum                          */
                /******************************************************/

                qsap_total.qsap_arg.devdd_len = MAX_DH_STATS;

              /********************************************************/
              /* call the build sap status routine                    */
              /********************************************************/

              p->rc = build_sap_cmd(p, &qsap_total);
              if                       /* no error occured            */
                 (p->rc == RC_GOOD)

                /******************************************************/
                /* write out the query status to the user's space     */
                /******************************************************/

                if                     /* kernel space                */
                   (p->dlc_port.cid->state == KERN)
                  {

                    /**************************************************/
                    /* copy dlc qsap values                           */
                    /**************************************************/

                    bcopy(&qsap_total, sap_result, sizeof(struct 
                       dlc_qsap_arg));

                    /**************************************************/
                    /* copy device specific values                    */
                    /**************************************************/
/* <<< feature CDLI >>> */
                    bcopy(&qsap_total.qsap_dh[0],
                          sap_result+sizeof(struct dlc_qsap_arg),
                          qsap_total.qsap_arg.devdd_len);
/* <<< end feature CDLI >>> */
                  } 
                else                   /* application space           */
                  {

                    /**************************************************/
                    /* copy dlc qsap arg values                       */
                    /**************************************************/

                    copyout(&qsap_total, sap_result, sizeof(struct 
                       dlc_qsap_arg));

                    /**************************************************/
                    /* copy device specific values                    */
                    /**************************************************/
/* <<< feature CDLI >>> */
                    copyout(&qsap_total.qsap_dh[0],
                       sap_result+sizeof(struct dlc_qsap_arg),
                       qsap_total.qsap_arg.devdd_len);
/* <<< end feature CDLI >>> */
                  } 
              else                     /* something failed in         
                                          build_sap_cmd               */
                return (p->rc);
            } 
          else                         /* the specified sap is not    
                                          currently in use.           */
            {

              /********************************************************/
              /* call error log - invalid sap correlator.             */
              /********************************************************/

	      lanerrlg(p, ERRID_LAN8073, NON_ALERT, 0, 0, FILEN, LINEN);
              return (EINVAL);
            } 
        } 
      else                             /* the specified sap correlator
                                          is not valid                */
        {

          /************************************************************/
          /* call error log - invalid sap correlator.                 */
          /************************************************************/

          lanerrlg(p, ERRID_LAN8073, NON_ALERT, 0, 0, FILEN, LINEN);
          return (EINVAL);
        } 
    } 
  else                                 /* the plc is not currently    
                                          open                        */
    {

      /****************************************************************/
      /* call error log - plc already closed.                         */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN0035, NON_ALERT, 0, 0, FILEN, LINEN);
      return (EIO);
    } 
  return (0);
}                                      /* end query_sap_cmd;          */

build_sap_cmd(p,stat_ptr)
  register struct port_dcl *p;
/* <<< feature CDLI >>> */
  struct qsap_total {
    struct dlc_qsap_arg qsap_arg;
#ifdef TRL
    tok_ndd_stats_t ndd_stats;
#endif
#ifdef FDL
    fddi_ndd_stats_t ndd_stats;
#endif
#ifndef  TRLORFDDI
    ent_ndd_stats_t ndd_stats;
#endif
  } *stat_ptr;
/* <<< end feature CDLI >>> */
    {
      int error;

      /****************************************************************/
      /* preset the error indicator = FALSE.                          */
      /****************************************************************/

      error = FALSE;

      /****************************************************************/
      /* build the "sap status" common result fields.                 */
      /****************************************************************/

      stat_ptr->qsap_arg.gdlc_sap_corr = p->sapno;
      stat_ptr->qsap_arg.user_sap_corr = 
         p->sap_ptr->sap_profile.user_sap_corr;
      strcpy(stat_ptr->qsap_arg.dev, p->dlc_port.namestr);
      switch                           /* the current sap state.      */
	  (p->sap_ptr->sap_state)
          {
            case                       /* opening                     */
	      (ADDING_NAME):
              stat_ptr->qsap_arg.sap_state = DLC_OPENING;
              break;

            case                       /* open                        */
	      (SAP_OPEN_STATE):
              stat_ptr->qsap_arg.sap_state = DLC_OPENED;
              break;

            case                       /* closing                     */
	      (SAP_CLOSE_PEND):
              stat_ptr->qsap_arg.sap_state = DLC_CLOSING;
              break;

              /********************************************************/
              /* note - the closed state doesn't exist                */
              /********************************************************/

            default  :                 /* error - invalid sap state.  */

              /********************************************************/
              /* call error log - invalid sap state.                  */
              /********************************************************/

	      lanerrlg(p, ERRID_LAN8011, NON_ALERT, 0, 0, FILEN, LINEN);
              return (EIO);
          } 
/* <<< feature CDLI >>> */
      /****************************************************************/
      /* call the device handler's ndd get status ioctl               */
      /****************************************************************/

      p->rc = (*p->nddp->ndd_ctl)(p->nddp, NDD_GET_STATS,
                                  &(stat_ptr->ndd_stats),
                                  sizeof(stat_ptr->ndd_stats));
/* <<< end feature CDLI >>> */

      if                               /* query device handler failed */
	  (p->rc != RC_GOOD)
          {

            /**********************************************************/
            /* call error log - unnusual return code from query device*/
            /**********************************************************/

            lanerrlg(p, ERRID_LAN8082, NON_ALERT, 0, 0, FILEN, LINEN);
            return (EIO);
	  }
	  return (RC_GOOD);
    }                                  /* end build_sap_cmd */

/* <<< feature CDLI >>> */
add_grp(p, arg, grp_code)
register struct port_dcl *p;
register struct dlc_add_grp *arg;
register int grp_code;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  add_grp                                              */
/*                                                                    */
/* descriptive name:  add a group address                             */
/*                                                                    */
/*  function: checks that the port is open, the sap is active, and    */
/*            verifies the group address length.  If OK, it           */
/*            passes the ndd_ctl to the device driver.                */
/*                                                                    */
/* input:  port control block pointer                                 */
/*         dlc_add_grp ioctl argument pointer                         */
/*         grp_code indicates whether it's an add or a delete         */
/*                                                                    */
/* output:  returns an errno or the return code                       */
/*             from the device ioctl                                  */
/*                                                                    */
/*** end of specifications ********************************************/

{
  struct dlc_add_grp tmp_add_grp;
  int rc;
  char add_grp_arg[6];

  /* get the input parameters from the user's space                   */

  if (p->dlc_port.cid->state == KERN)
    bcopy(arg, &tmp_add_grp, sizeof(struct dlc_add_grp));
  else
    copyin(arg, &tmp_add_grp, sizeof(struct dlc_add_grp));

  if /* the state of the physical link is opened */
     (p->common_cb.plc_state == PLC_OPENED)
    {
      /* get the sap CORRELATOR from the input add_grp_arg */
      p->sapno = tmp_add_grp.gdlc_sap_corr;

      if /* the input sap correlator is within range */
	 ((p->sapno > 0) && (p->sapno < 128))
        {
	  if /* the sap is in use */
	     (p->sap_list[p->sapno].in_use == TRUE)
            {
	      /* get addressability to the sap control block. */
              p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].
                 sap_cb_addr;

/* <<< feature CDLI >>> */
#ifdef   TRL
	      if /* the input group address length is correct */
		 ((tmp_add_grp.grp_addr_len == 4) ||
		  (tmp_add_grp.grp_addr_len == 6))
		{
		  if /* it's a COMIO 4-byte address */
		     (tmp_add_grp.grp_addr_len == 4)
		    {
		      /* build the last 4-bytes from the input address */
		      bcopy(&tmp_add_grp.grp_addr, &add_grp_arg[2], 4);
		    }

		  else /* it's a 6-byte CDLI address */
		    {
		      /* copy in all 6-bytes from the input address */
		      bcopy(&tmp_add_grp.grp_addr, &add_grp_arg[0], 6);
		    }

		  /* force the top 2-bytes to 0xC000 */
		  add_grp_arg[0] = 0xc0;
		  add_grp_arg[1] = 0x00;

		  /* set group address bit on (ie. non-functional addr) */
		  add_grp_arg[2] |= 0x80;

#endif /* TRL */
#ifndef TRL
	      if /* the input group address length is correct */
		 (tmp_add_grp.grp_addr_len == 6)
		{
		  /* build the 6-byte add_grp_arg for the ndd_ctl call */
		  bcopy(&tmp_add_grp.grp_addr, &add_grp_arg[0], 6);
#endif /* not TRL */

		  if /* it's a delete group address command */
		     (grp_code == DLC_DEL_GRP)
		    {
		      /*************************************************/
		      /* DELETE GROUP ADDRESS                          */
		      /*************************************************/

		      if /* group address is in this sap's address list */
			 (llcheck(&add_grp_arg, p))
			{
			  /* issue ndd disable address to the device driver */
			  rc = (*p->nddp->ndd_ctl)(p->nddp,
				    NDD_DISABLE_ADDRESS, &add_grp_arg, 6);

			  if /* device driver able to delete it */
			     (rc == 0)
			    {
			      /* remove address from sap's address list */
			      lldelete(p);
			    }

			  else /* ndd disable group address failed */
			      return (rc);
			}

                      else /* can't find group address in sap's list */
			  return (EINVAL);
		    }

		  else /* it's an add group address command */
		    {
		      /*************************************************/
		      /* ADD GROUP ADDRESS                             */
		      /*************************************************/

		      /* issue ndd enable address to the device driver */
		      rc = (*p->nddp->ndd_ctl)(p->nddp,
				 NDD_ENABLE_ADDRESS, &add_grp_arg, 6);

		      if /* device driver able to add it */
			 (rc == 0)
			{
			  /* call lladd to add address to list of valid
			     group addresses for this port  */
			  rc = lladd(&add_grp_arg, p);

			  if /* the add address to list failed */
			     (rc != 0)
			    {
			      /* Note: already error logged in lladd */

			      /* issue ndd disable address to the device
				 driver and ignore any return code */
			      rc = (*p->nddp->ndd_ctl)(p->nddp,
				    NDD_DISABLE_ADDRESS, &add_grp_arg, 6);

			      return (rc);
			    }
			}

		      else /* ndd enable address failed */
			  return (rc);
		    }

		  /* everything worked ok */
		  return (RC_GOOD);

		}

	      else /* invalid group address length */
		  return (EINVAL);
	    }

	  else /* the sap is not in use */
	      return (EINVAL);
	}

      else /* the sap correlator is not within range */
	  return (EINVAL);
    }

  else /* the port is not open */
    return (EIO);
}                                      /* end add_grp                 */
/* <<< end feature CDLI >>> */

#ifdef TRL
lan_setfa ( p, arg, opcode )
    register struct port_dcl *p;
    register struct dlc_func_addr *arg;
    int opcode;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  lan_setfa                                            */
/*                                                                    */
/* descriptive name:  sets or resets a functional address             */
/*                                                                    */
/*  function: checks that the pcb is open and that the sap is active  */
/*            also verifies the address length                        */
/*            then passes the ioctl to the device driver              */
/*                                                                    */
/* input:  port control block                                         */
/*         dlc_add_func_addr ioctl argument pointer                   */
/*         operation code (add/delete)                                */
/*                                                                    */
/* output:  returns an errno or the return code                       */
/*             from the device ioctl                                  */
/*                                                                    */
/*** end of specifications ********************************************/

{
struct dlc_func_addr tmp_func_addr;

   /* get the input parameters from the user's space */
   if (p->dlc_port.cid->state == KERN)
      bcopy(arg, &tmp_func_addr, sizeof(struct dlc_func_addr));
   else
      copyin(arg, &tmp_func_addr, sizeof(struct dlc_func_addr));

   if /* the state of the physical link is opened */
   (p->common_cb.plc_state == PLC_OPENED)
   {
      /* get the sap CORRELATOR from the input arg. */
      p->sapno = tmp_func_addr.gdlc_sap_corr;
      /* validate sap correlator */
      if ((p->sapno > 0) && (p->sapno < 128))
      {
	 /* if sap is in use */
	 if (p->sap_list[p->sapno].in_use == TRUE)
	 {
	    /* get addressability to the sap control block. */
	    p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].sap_cb_addr;

	    /* send the functional address to the device handler */
	    return (trl_setfa (p, opcode, tmp_func_addr.len_func_addr_mask,
					     tmp_func_addr.func_addr_mask));
	 }
	 else /* the sap is not in use */
	    return (EINVAL);
      }
      else /* the sap correlator is not within range */
	 return (EINVAL);
   }
   else /* the port is not open */
      return(EIO);
} /* end lan_setfa */
#endif /*TRL */


get_iocinfo(p,arg)
  register struct port_dcl *p;
  register struct devinfo *arg;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  get_iocinfo                                          */
/*                                                                    */
/* descriptive name:  get the iocinfo data                            */
/*                                                                    */
/*  function: returns the proper iocinfo type and subtype as defined  */
/*            in sys/devinfo.h.                                       */
/*                                                                    */
/* input:  port control block pointer                                 */
/*         iocinfo ioctl argument pointer                             */
/*                                                                    */
/* output:  returns a zero return code                                */
/*          fills in the top 3-bytes of the iocinfo structure         */
/*                                                                    */
/*** end of specifications ********************************************/

{
  struct devinfo laninfo;

  /********************************************************************/
  /* load the device class                                            */
  /********************************************************************/

  laninfo.devtype = DD_DLC;

  /********************************************************************/
  /* clear the flags                                                  */
  /********************************************************************/

  laninfo.flags = 0;

  /********************************************************************/
  /* load the device subclass based on protocol                       */
  /********************************************************************/

#ifdef   TRL
  laninfo.devsubtype = DS_DLCTOKEN;
#endif
#ifdef   FDL
  laninfo.devsubtype = DS_DLCFDDI;
#endif
#ifdef   EDL
  laninfo.devsubtype = DS_DLCETHER;
#endif
#ifdef   E3L
  laninfo.devsubtype = DS_DLC8023;
#endif

  /********************************************************************/
  /* write out the iocinfo to the user's space                        */
  /********************************************************************/

  if                                   /* kernel space                */
  (p->dlc_port.cid->state == KERN)
    bcopy(&laninfo, arg, 3);
  else                                 /* application space           */
    copyout(&laninfo, arg, 3);
  return (0);
}                                      /* end get_iocinfo             */
complete_local(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  complete_local                                       */
/*                                                                    */
/* discriptive name: complete local loopback connect out              */
/*                                                                    */
/* function:       this function is called when a link station        */
/*                 attempts to connect to a link station on the       */
/*                 same machine.  it sets the appropriate             */
/*                 states for both of the link stations and           */
/*                 posts the link manager to indicate that            */
/*                 the link stations have connected.                  */
/*                                                                    */
/*** end of specifications ********************************************/

{
  int match_found = FALSE;
  uchar_t dest_sapno,call_sapno;
  struct sap_cb *call_sapptr,*lsn_sapptr;
  struct station_cb *call_staptr,*lsn_staptr;
  int lsn_stano;
  int temp_stano;

  TRACE2(p, "CmLb", p->stano);
  if (p->sap_ptr->loop_sta_ptr == 0)
    {
#ifdef   TRLORFDDI
/* <<< feature CDLI >>> */
#ifdef TRL
      p->sta_ptr->phy_ctl1 = 0x00;
      p->sta_ptr->phy_ctl2 = 0x40;
#elif FDL
      p->sta_ptr->phy_ctl1 = 0x50;
#endif
/* <<< end feature CDLI >>> */
      p->common_cb.ri_length = 0;
#endif
      p->sap_ptr->loop_sta_ptr = p->sta_ptr;
      p->sap_ptr->loop_stano = p->stano;
      p->sta_ptr->ls = LS_ADM;
      p->sta_ptr->ls_profile.len_raddr_name =
	 p->sap_ptr->sap_profile.len_laddr_name;
      bcopy(p->sap_ptr->sap_profile.laddr_name, 
         p->sta_ptr->ls_profile.raddr_name, 
         p->sap_ptr->sap_profile.len_laddr_name);
      p->sta_ptr->ls_profile.rsap = p->sap_ptr->sap_profile.local_sap;
      bcopy(p->common_cb.local_addr, p->sta_ptr->laddr, 6);
      bcopy(p->common_cb.local_addr, p->sta_ptr->raddr, 6);
      p->sta_ptr->rsap = p->sap_ptr->sap_profile.local_sap;
      p->sta_ptr->lsap = p->sap_ptr->sap_profile.local_sap;
      bcopy(p->common_cb.local_addr,
	 p->common_cb.u_h.s_h.hash_string_raddr, 6);
      p->common_cb.u_h.s_h.hash_string_rsap =
	 p->sap_ptr->sap_profile.local_sap;
      p->common_cb.u_h.s_h.hash_string_lsap =
	 p->sap_ptr->sap_profile.local_sap;

      /****************************************************************/
      /* call the "add station to receive hash table" routine, with   */
      /* the hashing string and stationno.                            */
      /****************************************************************/

      add_sta_to_hash(p);
      post_stares(p);
      p->station_list[p->stano].t1_ctr = -1;
      p->station_list[p->stano].t1_ena = FALSE;
    } 
  else
    {
      if (p->station_list[p->sap_ptr->loop_stano].t1_ena == TRUE)
        p->station_list[p->sap_ptr->loop_stano].t1_ctr = 
           p->sap_ptr->loop_sta_ptr->resp_to_val;
      if (p->station_list[p->sap_ptr->loop_stano].t3_ena == TRUE)
        p->station_list[p->sap_ptr->loop_stano].t3_ctr = 
           p->sap_ptr->loop_sta_ptr->inact_to_val;

      /****************************************************************/
      /* save the user's station CORRELATOR of the conflicting        */
      /* station.                                                     */
      /****************************************************************/

      p->sap_ptr->conflict_user_corr = 
         p->sap_ptr->loop_sta_ptr->ls_profile.user_ls_corr;

      /****************************************************************/
      /* call error log - attach conflict                             */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN805D, NON_ALERT, PERM_STA_ERR, 
         DLC_REMOTE_CONN, FILEN, LINEN);

      /****************************************************************/
      /* call the link station with command = close.                  */
      /****************************************************************/

      lansta(p, CLOSENO);
    } 
  TRACE2(p, "CmLe", p->stano);
} 

