static char sccsid[] = "@(#)06  1.38.3.5  src/bos/kernext/dlc/lan/lansta3.c, sysxdlcg, bos41J, 9516B_all 4/14/95 18:02:06";

/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS:  lansta3.c
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
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
trace_cmd(p,t_ext)
  register struct port_dcl *p;
  register struct dlc_trace_arg *t_ext;

/*--------------------------------------------------------------------*/
/* trace command routine                                              */
/*--------------------------------------------------------------------*/

{
  ulong    dlctype;                    /* lan type                    */
 
  if                                   /* trace to be turned on       */

     ((t_ext->flags&DLC_TRCO) == DLC_TRCO)
    {

      /****************************************************************/
      /* setup short trace flag as default in station control block   */
      /****************************************************************/

      CLRBIT(p->sta_ptr->ls_profile.flags, DLC_TRCL);
 
      if                               /* long trace requested        */
         ((t_ext->flags&DLC_TRCL) == DLC_TRCL)
        {

          /************************************************************/
          /* indicate current station is using trace long flag        */
          /************************************************************/

          p->sta_ptr->ls_profile.flags |= DLC_TRCL;
        } 

      /****************************************************************/
      /* save channel number and trace status in link profile         */
      /****************************************************************/

      p->sta_ptr->ls_profile.trace_chan = t_ext->trace_chan;
      p->sta_ptr->ls_profile.flags |= DLC_TRCO;
    } 
 
  else
    {

      /****************************************************************/
      /* turn trace off                                               */
      /****************************************************************/

      p->sta_ptr->ls_profile.flags ^= DLC_TRCO;
    } 
  return (0);
}                                      /* end trace_cmd;              */
contact_cmd(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  contact_cmd                                          */
/*                                                                    */
/* descriptive name:  contact command from the user.                  */
/*                                                                    */
/* function:  handles the contact command from the user, by           */
/*            sending a SABME to the remote, in order to get into     */
/*            NORMAL data phase.                                      */
/*                                                                    */
/* input:  "contact" command.                                         */
/*                                                                    */
/* output:  transmits a SABME command.                                */
/*                                                                    */
/*** end of specifications ********************************************/

{
  TRACE2(p, "SCCo", p->sta_ptr->ls);
 
  if                                   /* link station state = adm    
                                          mode                        */
     (p->sta_ptr->ls == LS_ADM)
    {
 
      if (p->debug)
        printf("test=%d xid=%d\n", p->sta_ptr->test_ir_pend, 
           p->sta_ptr->xid_ir_pend);
 
      if                               /* no test or xid command is   
                                          pending a response          */
         ((p->sta_ptr->test_ir_pend == FALSE) && 
         (p->sta_ptr->xid_ir_pend == FALSE))
        {

          /************************************************************/
          /* set the link station state = contacting                  */
          /************************************************************/

          p->sta_ptr->ls = LS_CONTACTING;

          /************************************************************/
          /* build a SABME in the station command buffer.             */
          /************************************************************/

          tx_sta_cmd(p, SABME, DLC_NULL, 1);
 
          if                           /* t3 timer state = inactivity */
             (p->sta_ptr->t3_state == T3_INACT)
            {

              /********************************************************/
              /* terminate any inactivity timeout.                    */
              /********************************************************/

              p->station_list[p->stano].t3_ena = FALSE;
              p->station_list[p->stano].t3_ctr = -1;
            } 

          /************************************************************/
          /* set the poll retry count to the station's max retries.   */
          /************************************************************/

          p->sta_ptr->p_ct = p->sta_ptr->ls_profile.max_repoll;
        } 
 
      else                             /* error - an xid or test      
                                          response is pending.        */
        {

          /************************************************************/
          /* call error log - command already in progress. link       */
          /* station return code = user interface error.              */
          /************************************************************/
/* LEHb defect 56420 */
	  /* 56420 makes this error log an informational error instead
	     of a permanent station error so that SNA FSM's can issue
	     a contact command prior to receiving a response to an
	     outstanding XID command.                                 */

	  lanerrlg(p, ERRID_LAN801E, NON_ALERT, INFO_ERR,
	     DLC_USR_INTRF, FILEN, LINEN);
/* LEHe */
          return (EINVAL);
        } 
    } 
 
  else                                 /* error - not in adm mode (may
                                          happen due to remote        
                                          initiated                   */

    /******************************************************************/
    /* discontact).                                                   */
    /******************************************************************/

    {

      /****************************************************************/
      /* call error log - invalid mode for contact.                   */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN801F, NON_ALERT, INFO_ERR, DLC_USR_INTRF, 
         FILEN, LINEN);
      return (EINVAL);
    } 
  return (0);
}                                      /* end contact_cmd;            */
test_link_cmd(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  test_link_cmd                                        */
/*                                                                    */
/* descriptive name:  test link command from pu services.             */
/*                                                                    */
/* function:  handles the test link command from pu services, by      */
/*            sending a test command frame to the remote, inorder to  */
/*            solicit an echo of the frame.                           */
/*                                                                    */
/* input:  "test link" command.                                       */
/*                                                                    */
/* output:  transmits a test command frame with a maximum length      */
/*          i-field of hex 00-ff repeated.                            */
/*                                                                    */
/*** end of specifications ********************************************/

{
  int      stat,index,if_size;
 
  if                                   /* no test, xid, or contact    
                                          commands are already in     
                                          progress                    */

     ((p->sta_ptr->test_ir_pend == FALSE) && (p->sta_ptr->xid_ir_pend 
     == FALSE) && (p->sta_ptr->ls != LS_CONTACTING))
    {
 
      if                               /* the link station state =    
                                          adm, abme pending, or abme  */

      /****************************************************************/
      /* and no UNNUMBERED commands are stacked on vc (checkpointing) */
      /****************************************************************/

         ((p->sta_ptr->vc == NONE_ENA) && ((p->sta_ptr->ls == LS_ADM) 
         || (p->sta_ptr->ls == LS_ABME_PEND) || (p->sta_ptr->ls == 
         LS_ABME)))
        {

          /************************************************************/
          /* call buffer management to fetch a buffer, and save the   */
          /* address of the buffer in the "test command address".     */
          /************************************************************/

          p->sta_ptr->test_cmd_addr = lanfetch(p);
          p->m = (struct mbuf *)p->sta_ptr->test_cmd_addr;
 
          if                           /* an mbuf is available        */
             ((ulong_t)p->m != NO_BUF_AVAIL)
            {

              /********************************************************/
              /* get cluster buffer to hold test data                 */
              /********************************************************/

              m_clget(p->m);
 
              if                       /* the cluster fetch was       
                                          successful                  */
                 ((ulong_t)p->m != NO_BUF_AVAIL)
                {

                  /****************************************************/
                  /* call the protocol specific test link cmd to      */
                  /* generate the test packet                         */
                  /****************************************************/

                  g_test_link_cmd(p);
                  stat = RC_GOOD;
                } 
 
              else                     /* can't get an mbuf cluster   
                                          for the test packet         */
                {

                  /****************************************************/
                  /* call error log - out of communications memory    */
                  /* buffers                                          */
                  /****************************************************/

                  lanerrlg(p, ERRID_LAN8010, NON_ALERT, PERM_STA_ERR, 
                     DLC_LS_ROUT, FILEN, LINEN);
                  stat = ENOMEM;
                } 
            } 
 
          else                         /* error - no mbufs are        
                                          available                   */

            /**********************************************************/
            /* (already error logged in the fetch routine)            */
            /**********************************************************/

            stat = ENOMEM;
        } 
 
      else                             /* error - invalid state for   
                                          test (may happen due to     */

        /**************************************************************/
        /* remote initiated discontact.                               */
        /**************************************************************/

        {

          /************************************************************/
          /* call error log - invalid state for test.                 */
          /************************************************************/

          lanerrlg(p, ERRID_LAN8021, NON_ALERT, INFO_ERR, 
             DLC_USR_INTRF, FILEN, LINEN);
          stat = EINVAL;
        } 
    } 
 
  else                                 /* error - test commmand       
                                          already in progress.        */
    {

      /****************************************************************/
      /* call error log - test already enabled, link station return   */
      /* code = user interface error.                                 */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN8020, NON_ALERT, INFO_ERR, DLC_USR_INTRF, 
         FILEN, LINEN);
      stat = EINVAL;
    } 
  return (stat);
}                                      /* end test_link_cmd;          */
send_test_cmd(p)
  register struct port_dcl *p;
{

  /********************************************************************/
  /* set the buffer pointer to the test buffer's address.             */
  /********************************************************************/

  p->m = (struct mbuf *)p->sta_ptr->test_cmd_addr;

  /********************************************************************/
  /* save the last command sent to the remote.                        */
  /********************************************************************/

  p->sta_ptr->last_cmd_1 = TEST;

  /********************************************************************/
  /* set the test state = incomming response pending.                 */
  /********************************************************************/

  p->sta_ptr->test_ir_pend = TRUE;
 
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
  /* set the poll retry count (p_ct) = 1 test packet only             */
  /********************************************************************/

  p->sta_ptr->p_ct = 1;

  /********************************************************************/
  /* start a repoll timeout to insure inactivity failsafe             */
  /********************************************************************/

  p->station_list[p->stano].t1_ena = TRUE;
  p->station_list[p->stano].t1_ctr = p->sta_ptr->resp_to_val;

  /********************************************************************/
  /* disable sending any i-frames.                                    */
  /********************************************************************/

  p->sta_ptr->iframes_ena = FALSE;

/* <<< feature CDLI >>> */
  /********************************************************************/
  /* call the write send command generator, with the test command     */
  /* buffer address.                                                  */
  /********************************************************************/

  if (p->debug)
    printf("send_test %x\n", p->m);

  lanwrtsg(p, p->m);
/* <<< end feature CDLI >>> */

  p->sta_ptr->test_cmd_addr = 0;
 
  if                                   /* test commands sent counter  
                                          not at maximum value        */
     (p->sta_ptr->ras_counters.counters.test_cmds_sent != MINUS_ONE)

    /******************************************************************/
    /* increment count of test commands sent                          */
    /******************************************************************/

    ++p->sta_ptr->ras_counters.counters.test_cmds_sent;
}                                      /* end send_test_cmd;          */
alter_cmd(p, ext)
  register struct port_dcl *p;
  register struct dlc_alter_arg *ext;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  alter_cmd                                            */
/*                                                                    */
/* descriptive name:  alter link station configuration command        */
/*                                                                    */
/* function:  handles the altering of various link station            */
/*            configuration parameters by overlaying the profile      */
/*            values with the values supplied in the command.         */
/*                                                                    */
/* input:  "alter" input queue element.                               */
/*                                                                    */
/* output:  configuration profile updated.                            */
/*                                                                    */
/*** end of specifications ********************************************/

{
 
                                       /* copy new alter data */
  if (p->dlc_port.cid->state == KERN)
     bcopy(ext, &p->dlc_alter_buf, sizeof(struct dlc_alter_arg));
  else
     copyin(ext, &p->dlc_alter_buf, sizeof(struct dlc_alter_arg));

  if                                   /* the link station is not     
                                          closed                      */
     ((p->sta_ptr->ls != LS_CLOSE_STATE) && (p->sta_ptr->ls != 
     LS_DISCONTACTING) && (p->sta_ptr->ls != LS_CLOSE_PEND))
    {

      /*--------------------------------------------------------------*/
      /* max i-field                                                  */
      /*--------------------------------------------------------------*/
 
/* defect 109242 */
      if /* alter parameter "max i-field" is indicated */
	 ((p->dlc_alter_buf.flags & DLC_ALT_MIF) == DLC_ALT_MIF)
	{
/* <<< defect 125160 >>> */
	  if /* the specified max i-field is non-zero */
	     (p->dlc_alter_buf.maxif != 0)
/* end defect 109242 */
            {
	      if /* the specified max i-field is greater than the maximum
		    allowed value on the network */
		 (p->dlc_alter_buf.maxif > p->common_cb.maxif)

		{
		  /****************************************************/
		  /* set the station profile to the maximum allowed   */
		  /****************************************************/

		  p->sta_ptr->ls_profile.maxif = p->common_cb.maxif;
		}

	      else /* it's less than the maximum allowed */
		{
		  /****************************************************/
		  /* copy the input max i-field size to the station   */
		  /* profile.                                         */
		  /****************************************************/

		  p->sta_ptr->ls_profile.maxif = p->dlc_alter_buf.maxif;
		}
/* <<< end defect 125160 >>> */
            } 
 
          else                         /* error - invalid max i-filed 
                                          size.                       */
            {

              /********************************************************/
              /* call error log - input command - invalid parm link   */
              /* station return code = user interface error.          */
              /********************************************************/

              lanerrlg(p, ERRID_LAN8068, NON_ALERT, PERM_STA_ERR, 
                 DLC_USR_INTRF, FILEN, LINEN);
              return (EINVAL);
            } 
        } 

      /*--------------------------------------------------------------*/
      /* repoll timeout                                               */
      /*--------------------------------------------------------------*/
 
/* defect 109242 */
      if /* alter parameter "repoll time out" is indicated */
	 ((p->dlc_alter_buf.flags & DLC_ALT_RTO) == DLC_ALT_RTO)
	{
/* end defect 109242 */
/* defect 101404 */
	  if                           /* the input repoll timeout
					  value is valid (@ 500-msec) */
	     ((p->dlc_alter_buf.repoll_time > 0) &&
	      (p->dlc_alter_buf.repoll_time < 256))
	    {
	      /********************************************************/
	      /* copy the input repoll timer value to the station     */
	      /* T1 timeout.  Add 1 to insure a minimum of 500 ms.    */
	      /********************************************************/

	      p->sta_ptr->resp_to_val = p->dlc_alter_buf.repoll_time +1;
/* end defect 101404 */
            } 
 
          else                         /* error - invalid repoll timer
                                          value.                      */
            {

              /********************************************************/
              /* call error log - input command - invalid parm link   */
              /* station return code = user interface error.          */
              /********************************************************/

              lanerrlg(p, ERRID_LAN806D, NON_ALERT, PERM_STA_ERR, 
                 DLC_USR_INTRF, FILEN, LINEN);
              return (EINVAL);
            } 
        } 

      /*--------------------------------------------------------------*/
      /* transmit window                                              */
      /*--------------------------------------------------------------*/
 
/* defect 109242 */
      if /* alter parameter "transmit window" is indicated */
	 ((p->dlc_alter_buf.flags & DLC_ALT_XWIN) == DLC_ALT_XWIN)
/* end defect 109242 */
        {
 
          if                           /* the input transmit window   
                                          value is valid (1-127)      */
             ((p->dlc_alter_buf.xmit_wind > 0) && 
             (p->dlc_alter_buf.xmit_wind < 128))
            {

              /********************************************************/
              /* copy the input max transmit window to the station    */
              /* profile.                                             */
              /********************************************************/

              p->sta_ptr->ls_profile.xmit_wind = 
                 p->dlc_alter_buf.xmit_wind;
            } 
 
          else                         /* error - invalid transmit    
                                          window value.               */
            {

              /********************************************************/
              /* call error log - input command - invalid parm link   */
              /* station return code = user interface error.          */
              /********************************************************/

              lanerrlg(p, ERRID_LAN8070, NON_ALERT, PERM_STA_ERR, 
                 DLC_USR_INTRF, FILEN, LINEN);
              return (EINVAL);
            } 
        } 

      /*--------------------------------------------------------------*/
      /* acknowledge timeout                                          */
      /*--------------------------------------------------------------*/
 
/* defect 109242 */
      if /* alter parameter "acknowledge timeout" is indicated */
	 ((p->dlc_alter_buf.flags & DLC_ALT_AKT) == DLC_ALT_AKT)
	{
	  if                           /* the input value is valid    */
	     ((p->dlc_alter_buf.ack_time > 0) &&
	      (p->dlc_alter_buf.ack_time < 256))
	    {

	      /********************************************************/
	      /* copy the input ackonwledge timeout to the station    */
	      /* T2 timeout.  Add 1 to insure a minimum of 500 ms.    */
	      /********************************************************/

	      p->sta_ptr->ack_to_val = (p->dlc_alter_buf.ack_time +1);
	    }
/* end defect 109242 */
 
          else                         /* error - invalid acknowledge 
                                          timeout value               */
            {

              /********************************************************/
              /* call error log - input command - invalid parm link   */
              /* station return code = user interface error.          */
              /********************************************************/

              lanerrlg(p, ERRID_LAN8070, NON_ALERT, PERM_STA_ERR, 
                 DLC_USR_INTRF, FILEN, LINEN);
              return (EINVAL);
            } 
        } 

      /*--------------------------------------------------------------*/
      /* force timeout                                                */
      /*--------------------------------------------------------------*/
 
/* defect 109242 */
      if /* alter parameter "force timeout " is indicated */
	 ((p->dlc_alter_buf.flags & DLC_ALT_FHT) == DLC_ALT_FHT)
	{
	  if                           /* the input value is valid    */
	     ((p->dlc_alter_buf.force_time > 0) &&
	      (p->dlc_alter_buf.force_time < 16384))
	    {

	      /********************************************************/
	      /* copy the input force timeout to the station's force  */
	      /* timeout.  Multiply by 2 and add 1 to insure a        */
	      /* minimum of 1 sec (using 500 ms ticks).               */
	      /********************************************************/

	      p->sta_ptr->force_to_val =
				  ((p->dlc_alter_buf.force_time *2) +1);
	    }
/* end defect 109242 */
 
          else                         /* error - invalid force       
                                          timeout value               */
            {

              /********************************************************/
              /* call error log - input command - invalid parm link   */
              /* station return code = user interface error.          */
              /********************************************************/

              lanerrlg(p, ERRID_LAN8070, NON_ALERT, PERM_STA_ERR, 
                 DLC_USR_INTRF, FILEN, LINEN);
              return (EINVAL);
            } 
        } 

      /*--------------------------------------------------------------*/
      /* maximum repoll                                               */
      /*--------------------------------------------------------------*/
 
/* defect 109242 */
      if /* alter parameter "maximum repoll" is indicated */
	 ((p->dlc_alter_buf.flags & DLC_ALT_MXR) == DLC_ALT_MXR)
	{
	  if                           /* the input value is valid    */
	     ((p->dlc_alter_buf.max_repoll > 0) &&
	      (p->dlc_alter_buf.max_repoll < 256))
/* end defect 109242 */
            {

              /********************************************************/
              /* copy the input max repoll to the station profile.    */
              /********************************************************/

              p->sta_ptr->ls_profile.max_repoll = 
                 p->dlc_alter_buf.max_repoll;
            } 
 
          else                         /* error - invalid maximum     
                                          repoll value                */
            {

              /********************************************************/
              /* call error log - input command - invalid parm link   */
              /* station return code = user interface error.          */
              /********************************************************/

              lanerrlg(p, ERRID_LAN8070, NON_ALERT, PERM_STA_ERR, 
                 DLC_USR_INTRF, FILEN, LINEN);
              return (EINVAL);
            } 
        } 

      /*--------------------------------------------------------------*/
      /* notification on halt                                         */
      /*--------------------------------------------------------------*/
 

      if                               /* alter parameter             
                                          "notification on halt" is   
                                          indicated                   */
         ((p->dlc_alter_buf.flags&DLC_ALT_IT1) == DLC_ALT_IT1)
        {
 
          if                           /* no conflict with automatic  
                                          halt flag                   */
             ((p->dlc_alter_buf.flags&DLC_ALT_IT2) != DLC_ALT_IT2)

            /**********************************************************/
            /* indicate station should notify user on halt            */
            /**********************************************************/

            SETBIT(p->sta_ptr->ls_profile.flags, DLC_SLS_HOLD);
 
          else                         /* error - invalid maximum     
                                          repoll value                */
            {

              /********************************************************/
              /* call error log - input command - invalid parm link   */
              /* station return code = user interface error.          */
              /********************************************************/

              lanerrlg(p, ERRID_LAN8070, NON_ALERT, PERM_STA_ERR, 
                 DLC_USR_INTRF, FILEN, LINEN);
              return (EINVAL);
            } 
        } 

      /*--------------------------------------------------------------*/
      /* automatic halt                                               */
      /*--------------------------------------------------------------*/
 
/* defect 109242 */
      if /* alter parameter "automatic halt" is indicated */
	 ((p->dlc_alter_buf.flags & DLC_ALT_IT2) == DLC_ALT_IT2)
/* end defect 109242 */
        {
 
          if                           /* no conflict with            
                                          notification on halt flag   */
             ((p->dlc_alter_buf.flags&DLC_ALT_IT1) != DLC_ALT_IT1)

            /**********************************************************/
            /* indicate station should halt on inactivity             */
            /**********************************************************/

            CLRBIT(p->sta_ptr->ls_profile.flags, DLC_SLS_HOLD);
 
          else                         /* error - invalid maximum     
                                          repoll value                */
            {

              /********************************************************/
              /* call error log - input command - invalid parm link   */
              /* station return code = user interface error.          */
              /********************************************************/

              lanerrlg(p, ERRID_LAN8070, NON_ALERT, PERM_STA_ERR, 
                 DLC_USR_INTRF, FILEN, LINEN);
              return (EINVAL);
            } 
        } 
#ifdef   TRLORFDDI

      /*--------------------------------------------------------------*/
      /* routing                                                      */
      /*--------------------------------------------------------------*/
 
/* defect 109242 */
      if /* alter parameter "routing" is indicated */
	 ((p->dlc_alter_buf.flags & DLC_ALT_RTE) == DLC_ALT_RTE)
	{
	  if /* the input length is valid (ie. greater than zero, less
		than or equal to max route size, and an even number */
	     ((p->dlc_alter_buf.routing_len > 0) &&
	      (p->dlc_alter_buf.routing_len <= DLC_MAX_ROUT) &&
	      ((p->dlc_alter_buf.routing_len & 1) != 1))
/* end defect 109242 */
            {

              /********************************************************/
              /* copy the input routing to the station profile.       */
              /********************************************************/

/* LEHb defect 44499 */
	      bcopy(&(p->dlc_alter_buf.routing[0]),
		    &(p->sta_ptr->ri_field),
		    p->dlc_alter_buf.routing_len);
/* LEHe */

              /********************************************************/
              /* save length of new routing information               */
              /********************************************************/

              p->sta_ptr->ri_length = p->dlc_alter_buf.routing_len;
            
              /********************************************************/
              /* defect 29546 -- indicate route altered. disable      */
              /* saving route on each receive packet.                 */
              /********************************************************/
              p->sta_ptr->alter_route = 1;
              

            } 
 
          else                         /* error - invalid routing     
                                          length value                */
            {

              /********************************************************/
              /* call error log - input command - invalid parm link   */
              /* station return code = user interface error.          */
              /********************************************************/

              lanerrlg(p, ERRID_LAN8070, NON_ALERT, PERM_STA_ERR, 
                 DLC_USR_INTRF, FILEN, LINEN);
              return (EINVAL);
            } 
        } 
#endif

      /*--------------------------------------------------------------*/
      /* inactivity timeout                                           */
      /*--------------------------------------------------------------*/
 
/* defect 109242 */
      if /* alter parameter "inactivity timeout" is indicated */
	 ((p->dlc_alter_buf.flags & DLC_ALT_ITO) == DLC_ALT_ITO)
	{
	  if                           /* the input inactivity timeout
					  value is valid (@ 1-sec)    */
	     ((p->dlc_alter_buf.inact_time > 0) &&
	      (p->dlc_alter_buf.inact_time < 256))
	    {

	      /********************************************************/
	      /* copy the input inactivity timer value to the station */
	      /* T3 timeout.  Multiply by 2 and add 1 to insure a     */
	      /* minimum of 1 sec (using 500 ms ticks).               */
	      /********************************************************/

	      p->sta_ptr->inact_to_val =
				  ((p->dlc_alter_buf.inact_time *2) +1);
	    }
/* end defect 109242 */
 
          else                         /* error - invalid inactivity  
                                          timer value.                */
            {

              /********************************************************/
              /* call error log - input command - invalid parm link   */
              /* station return code = user interface error.          */
              /********************************************************/

              lanerrlg(p, ERRID_LAN8071, NON_ALERT, PERM_STA_ERR, 
                 DLC_USR_INTRF, FILEN, LINEN);
              return (EINVAL);
            } 
        } 

      /****************************************************************/
      /* call buffer management to return the extension buffer.       */
      /* lanfree (p,p->m);                                            */
      /****************************************************************/

    } 
 
  else                                 /* error - the link station is 
                                          closed.                     */
    {

      /****************************************************************/
      /* call error log - input command - ls not open link station    */
      /* return code = user interface error.                          */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN0039, NON_ALERT, INFO_ERR, DLC_USR_INTRF, 
         FILEN, LINEN);
      return (EINVAL);
    } 
  return (0);
}                                      /* end alter_cmd;              */
query_cmd(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  query_cmd                                            */
/*                                                                    */
/* descriptive name:  query link station statistics command           */
/*                                                                    */
/* function:  builds the ras statistics buffer for the user.          */
/*                                                                    */
/* input:  "query" input queue element.                               */
/*                                                                    */
/* output:  ras statistics buffer.                                    */
/*                                                                    */
/*** end of specifications ********************************************/

{
 
  if                                   /* the link station is not     
                                          closed                      */
     (p->sta_ptr->ls != LS_CLOSE_STATE)
    {
 
      if (p->debug)
        printf("===========>ls=%d\n", p->sta_ptr->ls);
      p->dlc_qls_arg.user_ls_corr = 
         p->sta_ptr->ls_profile.user_ls_corr;
      bcopy(p->sta_ptr->ls_profile.ls_diag, p->dlc_qls_arg.ls_diag, 16);
      p->dlc_qls_arg.ls_sub_state = 0; /* pre-zero the sub-state      */
 
      switch                           /* the current ls state.       */
         (p->sta_ptr->ls)
        {
          case                         /* call pending                */
             (LS_CALL_PEND) :
              {

                /******************************************************/
                /* set the "ls opening" and "ls calling" indicators.  */
                /******************************************************/

                p->dlc_qls_arg.ls_state = DLC_OPENING;
                p->dlc_qls_arg.ls_sub_state = DLC_CALLING;
              } 
            break;
          case                         /* listen pending              */
             (LS_LISTEN_PEND) :
              {

                /******************************************************/
                /* set the "ls opening" and "ls listening" indicators.*/
                /******************************************************/

                p->dlc_qls_arg.ls_state = DLC_OPENING;
                p->dlc_qls_arg.ls_sub_state = DLC_LISTENING;
              } 
            break;

            /**********************************************************/
            /* close pending or discontacting                         */
            /**********************************************************/

          case  LS_CLOSE_PEND :
          case  LS_DISCONTACTING :
              {

                /******************************************************/
                /* set the "ls closing" indicator, and clear the ls   */
                /* substate.                                          */
                /******************************************************/

                p->dlc_qls_arg.ls_state = DLC_CLOSING;
                p->dlc_qls_arg.ls_sub_state = 0;
              } 
            break;

            /**********************************************************/
            /* adm or abme modes                                      */
            /**********************************************************/

          case  LS_ADM :
          case  LS_CONTACTING :
          case  LS_ABME_PEND :
          case  LS_ABME :
              {
 
                if                     /* inactivity without          
                                          termination is pending      */
                   (p->sta_ptr->inact_without_pend == TRUE)

                  /****************************************************/
                  /* set the "ls inactive" indicator.                 */
                  /****************************************************/

                  p->dlc_qls_arg.ls_state = DLC_INACTIVE;
 
                else                   /* not inactive, so set the "ls
                  opened" indicator.                                  */
                  p->dlc_qls_arg.ls_state = DLC_OPENED;
 
                if                     /* contacted                   */
                   (p->sta_ptr->ls == LS_ABME)

                  /****************************************************/
                  /* set the "ls contacted" indicator.                */
                  /****************************************************/

                  p->dlc_qls_arg.ls_sub_state |= DLC_CONTACTED;
 
                if                     /* the local station is busy   */
                   (p->sta_ptr->local_busy == TRUE)

                  /****************************************************/
                  /* set the "local busy" indicator.                  */
                  /****************************************************/

                  p->dlc_qls_arg.ls_sub_state |= DLC_LOCAL_BUSY;
 
                if                     /* the remote station is busy  */
                   (p->sta_ptr->remote_busy == TRUE)

                  /****************************************************/
                  /* set the "remote busy" indicator.                 */
                  /****************************************************/

                  p->dlc_qls_arg.ls_sub_state |= DLC_REMOTE_BUSY;
              } 
            break;
          default  :                   /* error - invalid ls state.   */
            lanerrlg(p, ERRID_LAN003A, NON_ALERT, INFO_ERR, 
               DLC_USR_INTRF, FILEN, LINEN);
            return (EINVAL);

            /**********************************************************/
            /* panic("lansta3 581");                                  */
            /**********************************************************/

        } 

      /****************************************************************/
      /* copy the user's command CORRELATOR into the result buffer.   */
      /****************************************************************/

      p->dlc_qls_arg.user_ls_corr = 
         p->sta_ptr->ls_profile.user_ls_corr;

      /****************************************************************/
      /* copy the station's ras counters to the status buffer.        */
      /****************************************************************/

      bcopy(&(p->sta_ptr->ras_counters.counters), 
         &p->dlc_qls_arg.counters, sizeof(struct dlc_ls_counters));

      /****************************************************************/
      /* note: the LAN code does not support a protocol specific area */
      /****************************************************************/

      p->dlc_qls_arg.protodd_len = 0;
    } 
 
  else                                 /* error - the link station is 
                                          closed.                     */
    {

      /****************************************************************/
      /* call error log - input command - ls not open link station    */
      /* return code = user interface error.                          */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN003A, NON_ALERT, INFO_ERR, DLC_USR_INTRF, 
         FILEN, LINEN);
      return (EIO);
    } 
  return (0);
}                                      /* end query_cmd;              */
user_elb_cmd(p)
  register struct port_dcl *p;
{
  int      stat = 0;
 
  if                                   /* not already in user         
                                          initiated local busy mode   */

     (p->sta_ptr->us_local_busy == FALSE)
    {

      /****************************************************************/
      /* set the user local busy indicator.                           */
      /****************************************************************/

      p->sta_ptr->us_local_busy = TRUE;

      /****************************************************************/
      /* call the enter local busy routine.                           */
      /****************************************************************/

      stat = enter_local_busy(p);
    } 
 
  else                                 /* already in user initiated   
                                          local busy mode             */

    /******************************************************************/
    /* no error log or errno is needed                                */
    /******************************************************************/

    return (stat);
}                                      /* end user_elb_cmd;           */
user_xlb_cmd(p)
  register struct port_dcl *p;
{
  int      stat = 0;
 
  if                                   /* currently in the local busy 
                                          state                       */

     (p->sta_ptr->local_busy == TRUE)
    {

/* LEHb defect 43788 */
/* delete 5 lines */
/* LEHe */

      /****************************************************************/
      /* reset the user initiated local busy indicator.               */
      /****************************************************************/

      p->sta_ptr->us_local_busy = FALSE;
 
/* LEHb defect 43788 */
      if                               /* the user is waiting for a
					    local busy wakeup         */
	 (p->station_list[p->stano].wakeup_needed == TRUE)

	/**************************************************************/
	/* call the user receive wakeup routine.                      */
	/**************************************************************/
	user_rcv_wakeup(p);

      if                               /* the user is no longer waiting
					    for local busy wakeup     */
	 (p->station_list[p->stano].wakeup_needed == FALSE)

	/**************************************************************/
	/* call the exit local busy routine.                          */
	/**************************************************************/
	stat = exit_local_busy(p);
/* LEHe */
    }
 
  else                                 /* not in local busy state     */
    {

      /****************************************************************/
      /* FIX - need errlog: invalid state for exit local busy         */
      /****************************************************************/

      stat = EINVAL;
    } 
  return (stat);
}                                      /* end user_xlb_cmd;           */
t1_timeout_cmpl(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  t1_timeout_cmpl                                      */
/*                                                                    */
/* descriptive name:  t1 timeout completion                           */
/*                                                                    */
/* function:  handles the t1 (command repoll) timeout, by either      */
/*            repolling the same command frame, or by halting the     */
/*            logical link session due to inactivity.                 */
/*                                                                    */
/* input:  last command sent and the current repoll count.            */
/*                                                                    */
/* output:  last command sent is repolled, inactivity without         */
/*          termination result psb, or inactivity link station        */
/*          closed psb.                                               */
/*** end of specifications ********************************************/

{
  int      cont_polling;
  ulong    dlctype;                    /* lan type                    */
  ulong_t  port_sta;                   /* station number and port     
                                          number                      */
  struct ri_control_field *p_ri;       /* routing inrofmation control */

#define  STOP_POLL 0
#define  INACTIVE_POLL 1
#define  NORMAL_POLL 2
  TRACE1(p, "t1to");
 
  if                                   /* the station is not closed   */
     ((p->sta_ptr->ls != LS_CLOSE_STATE) && (p->sta_ptr->ls != 
     LS_CLOSE_PEND))
    {

      /****************************************************************/
      /* setup lan and monitor values                                 */
      /****************************************************************/

      dlctype = DLC_TRACE_TIMER<<8;
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
/* <<< end feature CDLI >>> */

/* call trchkgt to record monitor trace                               */

      trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, 0x04, 0, 0, 0, port_sta);

      /****************************************************************/
      /* setup hookid and dlc type for performance trace              */
      /****************************************************************/

      dlctype = HKWD_SYSX_DLC_PERF|DLC_TRACE_T1TO;
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

/* call trckhlt to record performance trace                           */

      trchklt(dlctype, 0);

/* DYNA                                                               */
      /* initialize the continue polling value                        */

      cont_polling = NORMAL_POLL;
 
      if                               /* link trace enabled.         */
         ((p->sta_ptr->ls_profile.flags&DLC_TRCO) == DLC_TRCO)
        {

          /************************************************************/
          /* call the timer link trace routine (t1 repoll timeout).   */
          /************************************************************/

          lantoltr(p, DLC_TO_REPOLL_T1);
        } 
 
      if                               /* in abme and not             
                                          checkpointing               */
         ((p->sta_ptr->ls == LS_ABME) && (p->sta_ptr->checkpointing ==
         FALSE))
        {
 
	  /* LEHb defect 38204 */
	  /* clear the last command sent variable so that normal supervisory
	     packets will be sent instead of some prior command type */
	  p->sta_ptr->last_cmd_1 = 0;
	  /* LEHe */

          if                           /* the i-lpdu retry count has  
                                          not decremented to zero     */
             (p->sta_ptr->is_ct != 0)
            {

              /********************************************************/
              /* set the poll count to the maximum poll retries value.*/
              /********************************************************/

              p->sta_ptr->p_ct = p->sta_ptr->ls_profile.max_repoll;

              /********************************************************/
              /* stop the sending of i-frames.                        */
              /********************************************************/

              p->sta_ptr->iframes_ena = FALSE;

              /********************************************************/
              /* set the abme substate for checkpointing.             */
              /********************************************************/

              p->sta_ptr->checkpointing = TRUE;

              /********************************************************/
              /* call the send short repoll routine.                  */
              /********************************************************/

              send_short_repoll(p);
            } 
 
          else                         /* the i-lpdu retry count has  
                                          reached zero, ie. the remote
                                          is                          */

            /**********************************************************/
            /* not accepting i-lpdus.                                 */
            /**********************************************************/

            {

              /********************************************************/
              /* call error log - remote not accepting i-frames. link */
              /* station return code = protocol error.                */
              /********************************************************/

              lanerrlg(p, ERRID_LAN8041, NON_ALERT, PERM_STA_ERR, 
                 DLC_PROT_ERR, FILEN, LINEN);

              /********************************************************/
              /* call shutdown (p,soft) to send a disc command.       */
              /********************************************************/

              shutdown(p, SOFT);
            } 
        } 
 
      else                             /* not in abme or already      
                                          checkpointing.              */
        {

          /************************************************************/
          /* decrement the poll count.                                */
          /************************************************************/

          p->sta_ptr->p_ct = (p->sta_ptr->p_ct-1);
 
          if                           /* the poll count has          
                                          decremented to zero.        */
             (p->sta_ptr->p_ct == 0)
            {
 
              if                       /* a test command is in        
                                          progress                    */
                 (p->sta_ptr->last_cmd_1 == TEST)
                {

                  /****************************************************/
                  /* only polling once without termination, so set    */
                  /* continue polling to NORMAL so that the poll      */
                  /* duration is not extended.                        */
                  /****************************************************/

                  cont_polling = NORMAL_POLL;
                } 
 
              else                     /* no test command is in       
                                          progress                    */
                {
#ifndef  TRLORFDDI                           /* indicate inactivity not     
                                          pending                     */
                  p->sta_ptr->inact_pend = 0;
#endif
#ifdef   TRLORFDDI
 
                  if                   /* routing information present */
                     (p->sta_ptr->ri_length > 0)
                    {

                      /************************************************/
                      /* indicate link is to be kept active using     */
                      /* normal polling                               */
                      /************************************************/

                      cont_polling = NORMAL_POLL;
 
                      if               /* inactivity not pending      */
                         (p->sta_ptr->inact_pend == 0)
                        {              /* set limited broadcast       
                                          inactivity pending          */
                          p->sta_ptr->inact_pend = INACT_LIMITED;/*   
                                          set limited broadcast bits  
                                          in packet header            */
                          p_ri = (struct ri_control_field *)
                             &p->sta_ptr->ri_field;
                          p_ri->all_route = TRUE;
                          p_ri->single_route = TRUE;/* set routing    
                                          length for single route     
                                          broadcast                   */
                          p->sta_ptr->ri_length = 2;
                          p_ri->ri_lth = 2;
                        
                        }
 
                      else
 
                        if             /* all limited broadcast       
                                          inactivity pending          */
                           (p->sta_ptr->inact_pend == INACT_LIMITED)
                          {            /* set all routes broadcast    
                                          inactivity pending          */
                            p->sta_ptr->inact_pend = INACT_ALL_ROUTE;/*
                                          set all route broadcast bits
                                          in packet header            */
                            p_ri = (struct ri_control_field *)
                               &p->sta_ptr->ri_field;
                            p_ri->all_route = TRUE;
                            p_ri->single_route = FALSE;/* set routing 
                                          length for all route        
                                          broadcast                   */
                            p->sta_ptr->ri_length = 2;
                          p_ri->ri_lth = 2;
                          } 
 
                        else
 
                          if (p->sta_ptr->inact_pend == 
                             INACT_ALL_ROUTE)
                            {          /* reset pending flags         */
                              p->sta_ptr->inact_pend = 0;
                            } 
                    } 
#endif
 
                  if                   /* inactivity not pending      */
                     (p->sta_ptr->inact_pend == 0)
                    {

                      /************************************************/
                      /* call the inactivity timeout routine to see if*/
                      /* the link station is to remain active or      */
                      /* continue polling                             */
                      /*----------------------------------------------*/
                      /* note: inact_timeout returns TRUE when it     */
                      /* should remain active (same as inactive) FALSE*/
                      /* when it will go inactive (same as stop)      */
                      /*----------------------------------------------*/

                      cont_polling = inact_timeout(p);
                    } 
                } 

              /********************************************************/
              /* set the poll count back to the maximum poll retries  */
              /* value.                                               */
              /********************************************************/
/* defect 124049 */
	      if (p->sta_ptr)
		  p->sta_ptr->p_ct = p->sta_ptr->ls_profile.max_repoll;
/* end defect 124049 */
            } 

          /************************************************************/
          /* the polling has been initialized to NORMAL already.      */
          /************************************************************/
 

          if                           /* polling is to be continued  */
             (cont_polling != STOP_POLL)
            {
 
              if                       /* command repolls counter not 
                                          at maximum value            */
                 (p->sta_ptr->ras_counters.counters.cmd_repolls_sent 
                 != MINUS_ONE)

                /******************************************************/
                /* increment count of command repolls sent            */
                /******************************************************/

                ++p->sta_ptr->ras_counters.counters.cmd_repolls_sent;
 
              if                       /* contiguous cmd repolls      
                                          counter not at maximum value*/
                 (p->sta_ptr->ras_counters.counters.cmd_cont_repolls 
                 != MINUS_ONE)

                /******************************************************/
                /* increment count of contiguous cmd repolls sent     */
                /******************************************************/

                ++p->sta_ptr->ras_counters.counters.cmd_cont_repolls;
 
              switch                   /* the last command packet sent
                                          (control byte no1)          */
                 (p->sta_ptr->last_cmd_1)
                {
                  case                 /* an xid was the last command 
                                          packet sent.                */
                     (XID) :
                      {

                        /**********************************************/
                        /* call the send xid command routine.         */
                        /**********************************************/

                        send_xid_cmd(p);
                      } 
                    break;
                  case                 /* a test was the last command 
                                          packet sent.                */
                     (TEST) :
                      {

                        /**********************************************/
                        /* call error log - inactivity without        */
                        /* termination (information only)             */
                        /**********************************************/

                        lanerrlg(p, ERRID_LAN0101, NON_ALERT, 
                           INFO_ERR, 0, FILEN, LINEN);

                        /**********************************************/
                        /* set the test psb result to "inactivity with
                        out termination.                              */
                        /**********************************************/

                        p->operation_result = DLC_INACT_TO;

                        /**********************************************/
                        /* call test complete with the inactivity     */
                        /* without termination result.                */
                        /**********************************************/

                        test_completion(p, DLC_IWOT_RES);
 
                        if             /* in ABME                     */
                           (p->sta_ptr->ls == LS_ABME)
                          {
                          p->sta_ptr->checkpointing = TRUE;

                          /**********************************************/
                          /* disable sending of i-frames.               */
                          /**********************************************/

                          p->sta_ptr->iframes_ena = FALSE;
   

 
                            if         /* not local busy              */
                               (p->sta_ptr->local_busy == FALSE)
                              {

                                /**************************************/
                                /* transmit an rr command with the    */
                                /* final bit set                      */
                                /**************************************/

                                tx_sta_cmd(p, RR, (PF2_MASK|
                                   p->sta_ptr->vr), 2);
                              } 
 
                            else       /* local busy.                 */
                              {

                                /**************************************/
                                /* transmit an rnr command with the   */
                                /* final bit set                      */
                                /**************************************/

                                tx_sta_cmd(p, RNR, (PF2_MASK|
                                   p->sta_ptr->vr), 2);
                              } 
                          } 
 
                        else           /* not in ABME                 */
                          {

                            /******************************************/
                            /* reset the t1 timer.                    */
                            /******************************************/

                            p->station_list[p->stano].t1_ctr = -1;
                            p->station_list[p->stano].t1_ena = FALSE;
                            /******************************************/
                            /* reset inactivity pending flags to      */
                            /* indicate packet has been received      */
                            /******************************************/
                            p->sta_ptr->inact_pend = 0;

                            /******************************************/
                            /* start the t3 inactivity timer          */
                            /******************************************/

                            p->station_list[p->stano].t3_ctr = 
                               p->sta_ptr->inact_to_val;
                            p->station_list[p->stano].t3_ena = TRUE;
                          } 
                      } 
                    break;
                  default  :           /* the last command packet sent
                                          was rr, rnr, SABME,         */

                    /**************************************************/
                    /* or disc.                                       */
                    /**************************************************/

                      {

                        /**********************************************/
                        /* call the send short repoll routine.        */
                        /**********************************************/

                        send_short_repoll(p);
                      } 
                } 
 
              if                       /* polling is to be extended   
                                          due to inactivity           */
                 (cont_polling == INACTIVE_POLL)
                {

                  /*--------------------------------------------------*/
                  /* reset t1 count from the value set in send_xid_cnd*/
                  /* or send_short_repoll to the t3 inactivity timeout*/
                  /* value in the station control block. this will    */
                  /* cause a longer wait until the next poll command  */
                  /* is sent and will reduce the excessive re-polling */
                  /* to an inactive station.                          */
                  /*--------------------------------------------------*/

                  p->station_list[p->stano].t1_ctr = 
                     p->sta_ptr->inact_to_val;
                } 
            }                          /* contine polling             */

          /************************************************************/
          /* note - station already closed in the inactivity routine. */
          /************************************************************/

        }                              /* not abme or already         
                                          checkpointing               */
    }                                  /* station open                */
}                                      /* end t1_timeout_cmpl;        */
send_short_repoll(p)
  register struct port_dcl *p;
{
 
  if                                   /* last command sent was       
                                          i-frame or s-frame          */
     ((p->sta_ptr->last_cmd_1&0x03) != UNNUMBERED)
    {
 
      if                               /* in local busy mode          */
         (p->sta_ptr->local_busy == TRUE)
        {

          /************************************************************/
          /* send rnr.                                                */
          /************************************************************/

          tx_sta_cmd(p, RNR, (PF2_MASK|p->sta_ptr->vr), 2);
        } 
 
      else                             /* not local busy.             */
        {

          /************************************************************/
          /* send rr.                                                 */
          /************************************************************/

          tx_sta_cmd(p, RR, (PF2_MASK|p->sta_ptr->vr), 2);
        } 
    } 
 
  else                                 /* the last command sent was   
                                          UNNUMBERED.                 */
    {

      /****************************************************************/
      /* re-send the same UNNUMBERED command last sent.               */
      /****************************************************************/

      tx_sta_cmd(p, p->sta_ptr->last_cmd_1, DLC_NULL, 1);
 
      if                               /* the control being sent =    
                                          test                        */
         (p->sta_ptr->last_cmd_1 == TEST)
        {
 
          if                           /* test command sent counter   
                                          not at maximum value        */
             (p->sta_ptr->ras_counters.counters.test_cmds_sent != 
             MINUS_ONE)

            /**********************************************************/
            /* increment count of test commands sent                  */
            /**********************************************************/

            ++p->sta_ptr->ras_counters.counters.test_cmds_sent;
        } 
    } 
}                                      /* end send_short_repoll;      */
t2_timeout_cmpl(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  t2_timeout_cmpl                                      */
/*                                                                    */
/* descriptive name:  t2 timeout completion                           */
/*                                                                    */
/* function:  handles the t2 (acknowledgement) timeout, by sending    */
/*            an acknowledgement to the i-frames already received.    */
/*                                                                    */
/* input:  current receive state variable.                            */
/*                                                                    */
/* output:  acknowledgement sent to the remote.                       */
/*                                                                    */
/*** end of specifications ********************************************/

{
  ulong    dlctype;                    /* lan type                    */
  ulong_t  port_sta;                   /* station number and port     
                                          number                      */

  TRACE1(p, "t2to");
 
  if                                   /* the station is not closed   */
     ((p->sta_ptr->ls != LS_CLOSE_STATE) && (p->sta_ptr->ls != 
     LS_CLOSE_PEND))
    {

/* DYNA                                                               */
/* setup lan and monitor trace types                                  */

      dlctype = DLC_TRACE_TIMER<<8;
#ifdef   TRL
      dlctype |= DLC_TOKEN_RING;
#endif
#ifdef   FDDI
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
/* <<< end feature CDLI >>> */

/* call trchkgt to record monitor trace                               */

      trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, 0x05, 0, 0, 0, port_sta);

      /****************************************************************/
      /* setup hook id and type for performance trace                 */
      /****************************************************************/

      dlctype = HKWD_SYSX_DLC_PERF|DLC_TRACE_T2TO;
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

/* call trchklt tp record performance trace                           */

      trchklt(dlctype, 0);
 
      if                               /* any abme substate local     
                                          busy, rejection, or clearing
                                          is active                   */
         ((p->sta_ptr->local_busy == TRUE) || (p->sta_ptr->rejection 
         == TRUE) || (p->sta_ptr->clearing == TRUE))
        {

          /************************************************************/
          /* error - invalid abme substate for t2 timeout.            */
          /************************************************************/

          lanerrlg(p, ERRID_LAN8011, NON_ALERT, PERM_STA_ERR, 
             DLC_USR_INTRF, FILEN, LINEN);

          /************************************************************/
          /* panic("lansta3 934");                                    */
          /************************************************************/

        } 
 
      else                             /* valid abme substate for t2  
                                          timeout.                    */
        {
 
          if                           /* link trace enabled.         */
             ((p->sta_ptr->ls_profile.flags&DLC_TRCO) == DLC_TRCO)
            {

              /********************************************************/
              /* call the timer link trace routine (t2 ack timeout).  */
              /********************************************************/

              lantoltr(p, DLC_TO_ACK_T2);
            } 

          /************************************************************/
          /* terminate the t2 timer.                                  */
          /************************************************************/

          p->station_list[p->stano].t2_ena = FALSE;
          p->station_list[p->stano].t2_ctr = -1;

          /************************************************************/
          /* set the received i-lpdu count to n3 (max number of       */
          /* i-frames between acknowledgements).                      */
          /************************************************************/

          p->sta_ptr->ir_ct = p->sta_ptr->ls_profile.rcv_wind;

          /************************************************************/
          /* call the transmit rr response routine to send an rr      */
          /* response with the final bit turned off.                  */
          /************************************************************/

          tx_rr_rsp(p, p->sta_ptr->vr);
        } 
    } 
}                                      /* end t2_timeout_cmpl;        */
t3_timeout_cmpl(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  t3_timeout_cmpl                                      */
/*                                                                    */
/* descriptive name:  t3 timeout completion                           */
/*                                                                    */
/* function:  handles the t3 (inactiivity or abort) timeout, by       */
/*            halting the logical link session (if optioned) and      */
/*            notifying pu services with a psb.                       */
/*                                                                    */
/* input:  inactivity without termination option.                     */
/*                                                                    */
/* output:  psb for pu services.                                      */
/*                                                                    */
/*** end of specifications ********************************************/

{
  ulong    dlctype;                    /* lan type                    */
  ulong_t  port_sta;                   /* station number and port     
                                          number                      */

  TRACE1(p, "t3to");
 
  if                                   /* the station is not closed   */
     (p->sta_ptr->ls != LS_CLOSE_STATE)
    {
 
      switch                           /* the t3 timer state.         */
         (p->sta_ptr->t3_state)
        {
          case                         /* popped timer = inactivity.  */
             (T3_INACT) :
              {

/* DYNA                                                               */
/* setup lan and monitor types                                        */

                dlctype = DLC_TRACE_TIMER<<8;
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

                /******************************************************/
                /* get station number in upper half word and get      */
                /* number from port name in lower half word           */
                /******************************************************/
/* <<< feature CDLI >>> */
#ifndef FDL
                port_sta = ((p->stano<<16)|p->dlc_port.namestr[3]);
#elif FDL
                port_sta = ((p->stano<<16)|p->dlc_port.namestr[4]);
#endif
/* <<< end feature CDLI >>> */

/* call trchkgt to record monitor trace                               */

                trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, 0x06, 0, 0, 0, 
                   port_sta);

                /******************************************************/
                /* setup hook id and dlc type for performance trace   */
                /******************************************************/

                dlctype = HKWD_SYSX_DLC_PERF|DLC_TRACE_T3TO;   /* 175640 */
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

/* call trchklt to record performance dadta                           */

                trchklt(dlctype, 0);
 
                if                     /* the link state is adm, abme 
                                          pending, or abme            */
                   ((p->sta_ptr->ls == LS_ADM) || (p->sta_ptr->ls == 
                   LS_ABME_PEND) || (p->sta_ptr->ls == LS_ABME))

                  /****************************************************/
                  /* valid state for inctivity to occur.              */
                  /****************************************************/

                  {
 
                    if                 /* link trace enabled.         */
                       ((p->sta_ptr->ls_profile.flags&DLC_TRCO) == 
                       DLC_TRCO)
                      {

                        /**********************************************/
                        /* call the timer link trace routine (t3 inact*/
                        /* timeout).                                  */
                        /**********************************************/

                        lantoltr(p, DLC_TO_INACT);
                      } 
 
                    if                 /* in adm mode                 */
                       (p->sta_ptr->ls == LS_ADM)
                      {

                        /**********************************************/
                        /* call the inactivity timeout routine to     */
                        /* notify pu services, and to determine if the*/
                        /* link station is to remain active.          */
                        /**********************************************/

                        p->rc = inact_timeout(p);
 
                        if             /* the link station is to      
                                          remain active               */
                           (p->rc == TRUE)
                          {

                            /******************************************/
                            /* restart the t3 timer.                  */
                            /******************************************/

                            p->station_list[p->stano].t3_ena = TRUE;
                            p->station_list[p->stano].t3_ctr = 
                               p->sta_ptr->inact_to_val;
                          } 
                      } 
 
                    else               /* in abme pending or abme     
                                          mode.                       */
                      {
 
                        if             /* currently in abme pending   
                                          state                       */
                           (p->sta_ptr->ls == LS_ABME_PEND)
                          {

                            /******************************************/
                            /* set link state (ls) = LS_ABME.         */
                            /******************************************/

                            p->sta_ptr->ls = LS_ABME;

                            /******************************************/
                            /* inform the higher layer - "contacted"  */
                            /******************************************/

                            station_contacted(p);
                          } 

                        /**********************************************/
                        /* terminate any inactivity or acknowledgement*/
                        /* timers.                                    */
                        /**********************************************/

                        p->station_list[p->stano].t2_ctr = -1;
                        p->station_list[p->stano].t2_ena = FALSE;
                        p->station_list[p->stano].t3_ctr = -2;
                        p->station_list[p->stano].t3_ena = FALSE;

                        /**********************************************/
                        /* set the poll retry count (p_ct) = max (n2).*/
                        /**********************************************/

                        p->sta_ptr->p_ct = 
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
 
                        else           /* local busy.                 */
                          {

                            /******************************************/
                            /* transmit an rnr command with the final */
                            /* bit set.                               */
                            /******************************************/

                            tx_sta_cmd(p, RNR, (PF2_MASK|
                               p->sta_ptr->vr), 2);
                          } 
                      } 
                  } 
              } 
            break;
          case                         /* popped timer = discontact   
                                          abort timeout.              */
             (T3_ABORT) :
              {

                /******************************************************/
                /* setup lan and monitor types                        */
                /******************************************************/

                dlctype = DLC_TRACE_TIMER<<8;
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

                /******************************************************/
                /* get station number in upper half word and get      */
                /* number from port name in lower half word           */
                /******************************************************/
/* <<< feature CDLI >>> */
#ifndef FDL
                port_sta = ((p->stano<<16)|p->dlc_port.namestr[3]);
#elif FDL
                port_sta = ((p->stano<<16)|p->dlc_port.namestr[4]);
#endif
/* <<< end feature CDLI >>> */

/* call trchkgt to record monitor trace                               */

                trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, 0x07, 0, 0, 0, 
                   port_sta);

                /******************************************************/
                /* setup hook id and dlc type for performance trace   */
                /******************************************************/

                dlctype = HKWD_SYSX_DLC_PERF|DLC_TRACE_T3ABORT;
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

/* call trchklt to record performance trace                           */

                trchklt(dlctype, 0);
 
                if                     /* link trace enabled.         */
                   ((p->sta_ptr->ls_profile.flags&DLC_TRCO) != 0)
                  {

                    /**************************************************/
                    /* call the timer link trace routine (t3 abort    */
                    /* timeout).                                      */
                    /**************************************************/

                    lantoltr(p, DLC_TO_ABORT);
                  } 

                /******************************************************/
                /* set closing reason code = discontact abort timeout.*/
                /******************************************************/

                p->sta_ptr->closing_reason = DLC_DISC_TO;

/* <<< feature CDLI >>> */
/* <<< removed p->sta_ptr->close_pend_non_buf >>> */
/* <<< end feature CDLI >>> */

                /******************************************************/
                /* call the shutdown routine - hard.                  */
                /******************************************************/

                shutdown(p, HARD);
              } 
            break;
          default  :                   /* error - unknown t3 state.   */
            lanerrlg(p, ERRID_LAN8011, NON_ALERT, PERM_STA_ERR, 
               DLC_SYS_ERR, FILEN, LINEN);

            /**********************************************************/
            /* panic("lansta3 1130");                                 */
            /**********************************************************/

        } 
    } 
}                                      /* end t3_timeout_cmpl;        */
inact_timeout(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  inact_timeout                                        */
/*                                                                    */
/* descriptive name:  inactivity timeout                              */
/*                                                                    */
/* function:  handles the inactivity timeout, by                      */
/*            halting the logical link session (if optioned) and      */
/*            notifying pu services with a psb.                       */
/*                                                                    */
/* input:  inactivity without termination option.                     */
/*                                                                    */
/* output:  psb for pu services.                                      */
/*                                                                    */
/*** end of specifications ********************************************/

{
  int      active;
 
  if                                   /* the user has optioned       
                                          inactivity without          
                                          termination                 */

     ((p->sta_ptr->ls_profile.flags & DLC_SLS_HOLD) != 0)
    {

      /****************************************************************/
      /* set the return code for "still active".                      */
      /****************************************************************/

      active = TRUE;
 
      if                               /* not already in the          
                                          inactivity without          
                                          termination mode            */
         (p->sta_ptr->inact_without_pend == FALSE)
        {

          /************************************************************/
          /* set the inactivity without termination mode.             */
          /************************************************************/

          p->sta_ptr->inact_without_pend = TRUE;

          /************************************************************/
          /* call error log - inactivity without termination (info    */
          /* only).                                                   */
          /************************************************************/

          lanerrlg(p, ERRID_LAN0101, NON_ALERT, INFO_ERR, 0, FILEN, 
             LINEN);

          /************************************************************/
          /* call the short result generator routine. with the        */
          /* "inactivity without termination" result.                 */
          /************************************************************/

          lansrslt(p, DLC_INACT_TO, DLC_IWOT_RES, 
             p->sap_ptr->sap_profile.user_sap_corr, 
             p->sta_ptr->ls_profile.user_ls_corr);
        } 
    } 
 
  else                                 /* terminate on inactivity     
                                          optioned.                   */
    {
 
      if                               /* receive incativity timeout  
                                          counter not at maximum value*/
         (p->sta_ptr->ras_counters.counters.rec_inact_to != MINUS_ONE)

        /**************************************************************/
        /* increment count of receive inactivity timeout              */
        /**************************************************************/

        ++p->sta_ptr->ras_counters.counters.rec_inact_to;

      /****************************************************************/
      /* call error log - receive inactivity timeout, link station    */
      /* return code = inactivity timeout.                            */
      /****************************************************************/

/* defect 86180 */
      lanerrlg(p, ERRID_LAN_ALERT1, ALERT, PERM_STA_ERR, DLC_INACT_TO,
         FILEN, LINEN);
      lanerrlg(p, ERRID_LAN8004, NON_ALERT, PERM_STA_ERR, DLC_INACT_TO,
         FILEN, LINEN);
/* ens defect 86180 */

      /****************************************************************/
      /* call the shutdown routine - hard.                            */
      /****************************************************************/

      shutdown(p, HARD);

      /****************************************************************/
      /* set the return code for "not active".                        */
      /****************************************************************/

      active = FALSE;
    } 
  return (active);
}                                      /* end inact_timeout;          */
close_link_station(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  close_link_station                                   */
/*                                                                    */
/* descriptive name:  close link station command handler              */
/*                                                                    */
/* function:  prepares the link station for discontact.               */
/*                                                                    */
/* input:  current link station and transmit queue state.             */
/*                                                                    */
/* output:  possible disc command sent to the remote.                 */
/*                                                                    */
/*** end of specifications ********************************************/

{
#define  NORMAL 0x00
  TRACE2(p, "CLSo", p->sap_ptr->sap_retcode);
 
  if                                   /* this close is due to a sap  
                                          shutdown                    */
     (p->sap_ptr->sap_retcode != NORMAL)
    {

      /****************************************************************/
      /* call shutdown routine - hard.                                */
      /****************************************************************/

      shutdown(p, HARD);
    } 
 
  else                                 /* not a sap shutdown.         */
    {
 
      if                               /* the link station state is   
                                          not already                 */

      /****************************************************************/
      /* discontacting, closing, or closed, and is not in an abort    */
      /* timeout condition                                            */
      /****************************************************************/

         ((p->sta_ptr->ls != LS_DISCONTACTING) && (p->sta_ptr->ls != 
         LS_CLOSE_PEND) && (p->sta_ptr->ls != LS_CLOSE_STATE) && 
         (p->sta_ptr->t3_state != T3_ABORT))
        {
 
          if                           /* the link station state is in
                                          abme and the transmit queue */

          /************************************************************/
          /* is not empty                                             */
          /************************************************************/

             ((p->sta_ptr->ls == LS_ABME) && (p->sta_ptr->txq_input !=
             p->sta_ptr->txq_output))
            {

              /********************************************************/
              /* set the t3 timer state = abort, in order to give the */
              /* transmit queue time to empty.                        */
              /********************************************************/

              p->sta_ptr->t3_state = T3_ABORT;

              /********************************************************/
              /* start the discontact abort timer with the specified  */
              /* value.                                               */
              /********************************************************/

              p->station_list[p->stano].t3_ctr = 
                 p->sta_ptr->force_to_val;
              p->station_list[p->stano].t3_ena = TRUE;
            } 
 
          else                         /* not in abme or the transmit 
                                          queue is empty.             */
            {

              /********************************************************/
              /* call shutdown routine - soft.                        */
              /********************************************************/

              shutdown(p, SOFT);
            } 
        } 
    } 
}                                      /* end close_link_station;     */
shutdown(p,type)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  shutdown                                             */
/*                                                                    */
/* descriptive name:  shutdown link station                           */
/*                                                                    */
/* function:  sends a disc command to the remote to terminate the     */
/*            logical link session "soft", or calls llc close         */
/*            completion if the shutdown is "hard".                   */
/*                                                                    */
/* input:  type of shutdown: soft, hard.                              */
/*                                                                    */
/* output:  possible disc command sent to the remote.                 */
/*                                                                    */
/*** end of specifications ********************************************/

  register int type;                   /* type of shutdown: 0=soft    
                                          1=hard 3=frmr sent          */
{
  TRACE2(p, "SHUo", type);
 
  if                                   /* the link station is not     
                                          already closed              */
     (p->sta_ptr->ls != LS_CLOSE_STATE)
    {
 
/* LEHb defect 44499 */
      if                               /* a test command is in progress
					  and not aborting the link   */
	 ((p->sta_ptr->test_ir_pend == TRUE) &&
	  (p->sap_ptr->sap_retcode != ABORTED))
/* LEHe */
        {

          /************************************************************/
          /* reset the test command state.                            */
          /************************************************************/

          p->sta_ptr->ts = TEST_RESET;
          p->sta_ptr->test_ir_pend = TEST_RESET;
 
          if                           /* a sap error occurred        */
             (p->sap_ptr->sap_retcode != NORMAL)
            {

              /********************************************************/
              /* preset the test psb result field to the plc failure  */
              /* code.                                                */
              /********************************************************/

              p->operation_result = p->sap_ptr->sap_retcode;
            } 
 
          else                         /* not a sap error.            */
            {

              /********************************************************/
              /* set the test psb result field to any link station    */
              /* failure code.                                        */
              /********************************************************/

              p->operation_result = p->sta_ptr->closing_reason;
            } 

          /************************************************************/
          /* call test complete with no extra result indicators.      */
          /************************************************************/

          test_completion(p, 0);
        } 
 
      if                               /* t3 timer state = inactivity */
         (p->sta_ptr->t3_state == T3_INACT)
        {

          /************************************************************/
          /* stop any inactivity timer in progress by setting the t3  */
          /* timer state = abort (insures that the link will close).  */
          /************************************************************/

          p->sta_ptr->t3_state = T3_ABORT;

          /************************************************************/
          /* start the discontact abort timer with the specified      */
          /* value.                                                   */
          /************************************************************/

          p->station_list[p->stano].t3_ctr = p->sta_ptr->force_to_val;
          p->station_list[p->stano].t3_ena = TRUE;
        } 
 
      if                               /* the type of shutdown is     
                                          soft, and                   */

      /****************************************************************/
      /* the link station is in abme mode, and not checkpointing; or  */
      /* case a frame reject has just been sent                       */
      /****************************************************************/

         (((type == SOFT) && (p->sta_ptr->ls == LS_ABME) && 
         (p->sta_ptr->checkpointing == FALSE)) || (type == FRMR_SENT))
        {

          /************************************************************/
          /* set the link station state to discontacting.             */
          /************************************************************/

          p->sta_ptr->ls = LS_DISCONTACTING;

          /************************************************************/
          /* stop any acknowledgement timer in progress.              */
          /************************************************************/

          p->station_list[p->stano].t2_ctr = -1;
          p->station_list[p->stano].t2_ena = FALSE;

          /************************************************************/
          /* set the poll retry count to the station's max retries.   */
          /************************************************************/

          p->sta_ptr->p_ct = p->sta_ptr->ls_profile.max_repoll;

          /************************************************************/
          /* call transmit station command with a disc.               */
          /************************************************************/

          tx_sta_cmd(p, DISC, DLC_NULL, 1);
        } 
 
      else
 
        if                             /* the type of shutdown is     
                                          soft, and                   */

        /**************************************************************/
        /* the link station is in abme mode, and checkpointing        */
        /**************************************************************/

           ((type == SOFT) && (p->sta_ptr->ls == LS_ABME) && 
           (p->sta_ptr->checkpointing == TRUE))
          {

            /**********************************************************/
            /* set the stacked command variable (vc) to disc.         */
            /**********************************************************/

            p->sta_ptr->vc = DISC_ENA;
          } 
 
        else                           /* shutdown will not send disc 
                                          to the remote.              */
          {

            /**********************************************************/
            /* call the llc close completion routine.                 */
            /**********************************************************/

            llc_close_completion(p);
          } 
    } 
}                                      /* end shutdown;               */
llc_close_completion(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  llc_close_completion                                 */
/*                                                                    */
/* descriptive name:  llc close completion                            */
/*                                                                    */
/* function:  returns any xid or test command buffers to the pool,    */
/*            purges the transmit queue, and clears the internal      */
/*            station states and indicators.  a psb issued to the     */
/*            user for link station closed.                           */
/*                                                                    */
/* input:  none                                                       */
/*                                                                    */
/* output:  link station closed psb to the user.                      */
/*                                                                    */
/*** end of specifications ********************************************/

{
  struct dlc_getx_arg dlc_getx_arg;
  struct dlc_chan *c_ptr;
  struct dlc_stah_res *stah;

/* <<< feature CDLI >>> */
/* <<< removed p->sta_ptr->close_pend_non_buf >>> */
  TRACE1(p, "LLCo");
/* <<< end feature CDLI >>> */

  /*------------------------------------------------------------------*/
  /* this routine is called when the station is ready to close. a     */
  /* check is made to see if there are any outstanding requests in use*/
  /* by the device handler. if not, the station control block is      */
  /* freed. if yes, then the indicator is set to cause any xmit       */
  /* completes to call this routine again. the close pend indicator   */
  /* will remain in effect until all outstanding buffers are freed.   */
  /*------------------------------------------------------------------*/
 
/* defect 149350 */
  if                                   /* a receive buffer needs to
                                          be returned                 */
     (p->sta_ptr->ignore == TRUE)
    {

      /****************************************************************/
      /* call buffer management to return the buffer.                 */
      /****************************************************************/

      lanfree(p, p->m);
    } 
/* end defect 149350 */
 
  if                                   /* an xid command buffer is not
                                          yet returned                */
     (p->sta_ptr->xid_cmd_addr != 0)
    {

      /****************************************************************/
      /* call buffer management to return the buffer.                 */
      /****************************************************************/

      lanfree(p, p->sta_ptr->xid_cmd_addr);
    } 
 
  if                                   /* an test command buffer is   
                                          not yet returned            */
     (p->sta_ptr->test_cmd_addr != 0)
    {

      /****************************************************************/
      /* call buffer management to return the buffer.                 */
      /****************************************************************/

      lanfree(p, p->sta_ptr->test_cmd_addr);
    } 

  /********************************************************************/
  /* purge the transmit queue.                                        */
  /********************************************************************/
 

  while (p->sta_ptr->txq_input != p->sta_ptr->txq_output)
    {

      /****************************************************************/
      /* call buffer management to return the buffer.                 */
      /****************************************************************/

      lanfree(p, p->sta_ptr->transmit_queue[p->sta_ptr->txq_output].
         buf);

      /****************************************************************/
      /* bump the transmit queue output index modulo the queue size.  */
      /****************************************************************/

      p->sta_ptr->txq_output = (p->sta_ptr->txq_output+1)%TXQ_SIZE;
    }                                  /* end do while;               */
 
  if                                   /* the sap is not being closed */
     (p->sap_ptr->sap_retcode == NORMAL)
    {
 
      if                               /* closing due to a conflicting
                                          station                     */
         (p->sta_ptr->closing_reason == DLC_REMOTE_CONN)
        {
 
          if (p->debug)
            printf("DLC_REMOTE_CONN\n");

          /************************************************************/
          /* wakeup any read or write processes                       */
          /************************************************************/

          e_wakeup((int *)&p->sap_ptr->user_sap_channel->readsleep);
          e_wakeup((int *)&p->sap_ptr->user_sap_channel->writesleep);
          dlc_getx_arg.result_ind = DLC_STAH_RES;
          dlc_getx_arg.user_sap_corr = 
             p->sap_ptr->sap_profile.user_sap_corr;
          dlc_getx_arg.user_ls_corr = 
             p->sta_ptr->ls_profile.user_ls_corr;
          dlc_getx_arg.result_code = p->sta_ptr->closing_reason;
          stah = (struct dlc_stah_res *)dlc_getx_arg.result_ext;
          stah->conf_ls_corr = p->sap_ptr->conflict_user_corr;
          c_ptr = p->sap_ptr->user_sap_channel;
          TRACE3(p, "XFAb", DLC_STAH_RES, p->sta_ptr->closing_reason);
          p->rc = (*c_ptr->excp_fa)(&dlc_getx_arg, c_ptr);
          TRACE2(p, "XFAe", p->rc);
 
          if (p->rc != 0)

            /**********************************************************/
            /* error - Bad rc from user's exception routine           */
            /**********************************************************/

            lanerrlg(p, ERRID_LAN8087, NON_ALERT, PERM_SAP_ERR, 
               DLC_SYS_ERR, FILEN, LINEN);
        } 
 
      else                             /* not closing due to an       
                                          attachment conflict.        */
        {
 
          if                           /* this station is in "limbo"  
                                          mode, ie. the device handler*/

          /************************************************************/
          /* has not been able to complete a write to the adapter     */
          /************************************************************/

             (p->sta_ptr->sta_limbo != 0)
            {

              /********************************************************/
              /* override closing reason with unusual network         */
              /* condition                                            */
              /********************************************************/

              p->sta_ptr->closing_reason = DLC_LS_NT_COND;
            } 

          /************************************************************/
          /* wakeup any read or write processes                       */
          /************************************************************/

          e_wakeup((int *)&p->sap_ptr->user_sap_channel->readsleep);
          e_wakeup((int *)&p->sap_ptr->user_sap_channel->writesleep);

          /************************************************************/
          /* call the short result generator routine.                 */
          /************************************************************/

          lansrslt(p, p->sta_ptr->closing_reason, DLC_STAH_RES, 
             p->sap_ptr->sap_profile.user_sap_corr, 
             p->sta_ptr->ls_profile.user_ls_corr);
        } 
 
      if                               /* link trace is enabled       */
         ((p->sta_ptr->ls_profile.flags&DLC_TRCO) != 0)

        /**************************************************************/
        /* call the session link trace routine                        */
        /**************************************************************/

        {
          session_trace(p, HKWD_SYSX_DLC_HALT, 0);
        } 
    }

/* LEHb defect 43788 */
/* delete 11 lines */

  if                                   /* xid receive buffer is pending   */
     (p->sta_ptr->retry_rcvx_buf != 0)
				       /* call buffer management to return
					  the buffer.                     */
    lanfree (p, p->sta_ptr->retry_rcvx_buf);

  if                                   /* dgrm receive buffer is pending  */
     (p->sta_ptr->retry_rcvd_buf != 0)
				       /* call buffer management to return
					  the buffer.                     */
    lanfree (p, p->sta_ptr->retry_rcvd_buf);

  if                                   /* iframe receive buffer is pending*/
     (p->sta_ptr->retry_rcvi_buf != 0)
				       /* call buffer management to return
					  the buffer.                     */
    lanfree (p, p->sta_ptr->retry_rcvi_buf);
/* LEHe */

  if (p->sap_ptr->loop_sta_ptr == p->sta_ptr)
    p->sap_ptr->loop_sta_ptr = 0;

  /********************************************************************/
  /* call the delete station from hash table routine. note - must     */
  /* preceed zeroing the receive station table entry.                 */
  /********************************************************************/

  landelsh(p);

  /********************************************************************/
  /* stop all timers in the station list.                             */
  /********************************************************************/

  p->station_list[p->stano].t1_ctr = -1;
  p->station_list[p->stano].t1_ena = FALSE;
  p->station_list[p->stano].t2_ctr = -2;
  p->station_list[p->stano].t2_ena = FALSE;
  p->station_list[p->stano].t3_ctr = -3;
  p->station_list[p->stano].t3_ena = FALSE;

  /********************************************************************/
  /* reset any flag indicators in the station list.                   */
  /********************************************************************/

  p->station_list[p->stano].sta_active = 0;
  p->station_list[p->stano].wakeup_needed = 0;
  p->station_list[p->stano].call_pend = 0;
  p->station_list[p->stano].discv_allr = 0;
 
  if                                   /* a listen was pending for    
                                          this station                */
     ((p->sap_list[p->sapno].listen_pend == TRUE) && 
     (p->sap_ptr->listen_stano == p->stano))

    /******************************************************************/
    /* reset the listen pending indicator.                            */
    /******************************************************************/

    p->sap_list[p->sapno].listen_pend = FALSE;

  /********************************************************************/
  /* free the station control block area.                             */
  /********************************************************************/

/* <<< feature CDLI >>> */
  assert(xmfree((caddr_t)p->station_list[p->stano].sta_cb_addr, (caddr_t)
	 kernel_heap) == 0);
/* <<< end feature CDLI >>> */

  /********************************************************************/
  /* set the station cb pointer in the station list to zero.          */
  /********************************************************************/

/* defect 124049 */
  if (p->sta_ptr == p->station_list[p->stano].sta_cb_addr)
      p->sta_ptr = 0;
/* end defect 124049 */

  p->station_list[p->stano].sta_cb_addr = 0;

  /********************************************************************/
  /* indicate that this station list entry is not in use.             */
  /********************************************************************/

  p->station_list[p->stano].in_use = FALSE;

  /********************************************************************/
  /* decrement the number of stations counter.                        */
  /********************************************************************/

  p->sap_ptr->num_sta_opened = (p->sap_ptr->num_sta_opened-1);
}                                      /* end llc_close_completion;   */
session_trace(p,session_type,d_length)
  register struct port_dcl *p;
  register ulong session_type;         /* session type                */
  register ulong d_length;             /* data length                 */
{
  struct   tr_data
    {
      uchar    diag_tag[16];
      uchar    remote_name[20];
      uchar    remote_address[6];
    } 
  tr_data;
  ulong    dlctype;                    /* lan type                    */

  /********************************************************************/
  /* set remote name and address to blanks                            */
  /********************************************************************/

  bcopy(" ", tr_data.diag_tag, 16);
  bcopy(" ", tr_data.remote_name, 20);

  /********************************************************************/
  /* copy attachment name to trace data area                          */
  /********************************************************************/

  bcopy(p->sta_ptr->ls_profile.ls_diag, tr_data.diag_tag, 16);

  /********************************************************************/
  /* copy remote name                                                 */
  /********************************************************************/

  bcopy(p->sta_ptr->ls_profile.raddr_name, tr_data.remote_name, 20);

  /********************************************************************/
  /* copy remote address                                              */
  /********************************************************************/

  bcopy(p->sta_ptr->raddr, tr_data.remote_address, 6);
#ifdef   TRL
  dlctype = (DLC_TOKEN_RING<<16)|0x08;
#endif
#ifdef   FDL
  dlctype = (DLC_FDDI<<16)|0x08;
#endif
#ifdef   EDL
  dlctype = (DLC_ETHERNET<<16)|0x04;
#endif
#ifdef   E3L
  dlctype = (DLC_IEEE_802_3<<16)|0x06;
#endif

/* call trcgenkt to record link trace                                 */

  trcgenkt(p->sta_ptr->ls_profile.trace_chan, session_type, dlctype, 
  sizeof(tr_data), &tr_data);
}                                      /* end session_trace;          */
