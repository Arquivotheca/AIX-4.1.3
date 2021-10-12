static char sccsid[] = "@(#)00  1.18.1.4  src/bos/kernext/dlc/lan/landiscv.c, sysxdlcg, bos411, 9428A410j 1/20/94 17:48:43";

/**********************************************************************
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: landiscv.c
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
 **********************************************************************/

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
void sort_cache();
landiscv(p,input_command)
  register struct port_dcl *p;

/*** start of specifications *******************************************
 *
 * module name:  landiscv
 *
 * descriptive name:  discovery manager
 *
 * function:  handles all discovery protocol functions
 *
 * input:  input discovery command
 *
 * output:  various send packets and result buffers
 *
 *** end of specifications ********************************************/

  int      input_command;              /* command issued from manager */
{
  int      index;

  TRACE2(p, "DSVb", input_command);

  /********************************************************************/
  /* setup pointer to cache buffer                                    */
  /********************************************************************/

  p_to_cache = (struct cache *)p->p2_to_cache;
 
  switch                               /* input command from the link 
                                          manager                     */
     (input_command)
    {
      case                             /* the input command = add     
                                          local name.                 */
         (ADDNO) :

        /*------------------------------------------------------------*/
        /* add local name                                             */
        /*------------------------------------------------------------*/
        /* call add name qyery generator for each entry in the sap    */
        /* list.                                                      */
        /**************************************************************/
 

        for (index = 0; index <= 127; index++)
          {
 
            if                         /* a sap is ready for add name 
                                          proceedures                 */
               (p->sap_list[index].addn_ready == TRUE)
              {

                /******************************************************/
                /* reset the add name ready indicator.                */
                /******************************************************/

                p->sap_list[index].addn_ready = FALSE;

                /******************************************************/
                /* get addressability to the sap.                     */
                /******************************************************/

                p->sap_ptr = (struct sap_cb *)p->sap_list[index].sap_cb_addr;
                p->sapno = index;
 
                if                     /* if using RESOLVE            */
                   ((p->sap_ptr->sap_profile.flags&DLC_ESAP_ADDR) != 0
                   )
                  {

                    /**************************************************/
                    /* set the sap state = sap_open.                  */
                    /**************************************************/

                    p->sap_ptr->sap_state = SAP_OPEN_STATE;
                    p->sap_list[p->sapno].find_self_echo = FALSE;
                    add_name_cmpl(p);
                  } 
 
                else
                  {

                    /**************************************************/
                    /* set the sap state = adding name.               */
                    /**************************************************/

                    p->sap_ptr->sap_state = ADDING_NAME;

                    /**************************************************/
                    /* set the add name retries counter to maximum.   */
                    /**************************************************/

                    p->sap_list[index].addn_retries = MAX_ADDN_REPOLLS
                       ;

                    /**************************************************/
                    /* enable the sap's t1 timer for add name query   */
                    /* retries.                                       */
                    /**************************************************/

                    p->sap_list[index].t1_ctr = ADDN_RETRY_VAL;
                    p->sap_list[index].t1_ena = TRUE;

                    /**************************************************/
                    /* bump the common counter for SAPs adding their  */
                    /* name                                           */
                    /**************************************************/

                    p->common_cb.addn_ctr++;

                    /**************************************************/
                    /* indicate that an add name query is expected to */
                    /* echo back to the local station due to the      */
                    /* BROADCAST destination addr.                    */
                    /**************************************************/

                    p->sap_list[index].addn_echo = TRUE;
                    p->sap_list[index].addn_pend = TRUE;

                    /**************************************************/
                    /* call the find self generator (new discovery)   */
                    /**************************************************/

                    find_self_gen(p);
                  } 
              } 
          }                            /* end for index;              */
        break;
      case                             /* the input command = call.   */
         (CALLNO) :

        /*------------------------------------------------------------*/
        /* call remote                                                */
        /*------------------------------------------------------------*/

          {

            /**********************************************************/
            /* get addressability back to the calling station.        */
            /**********************************************************/

            p->sta_ptr = (struct station_cb *)p->station_list[p->stano
               ].sta_cb_addr;
 
            if                         /* the stations command retry  
                                          count less then 3           */
               (p->sta_ptr->ls_profile.max_repoll < 3)

              /********************************************************/
              /* set the call retries counter value to minimum of 3.  */
              /********************************************************/

              p->station_list[p->stano].call_retries = 3;
 
            else

              /********************************************************/
              /* set the call retries counter in the station list to  */
              /* maximum.                                             */
              /********************************************************/

              p->station_list[p->stano].call_retries = 
                 p->sta_ptr->ls_profile.max_repoll;

            /**********************************************************/
            /* enable the station's t1 timer.                         */
            /**********************************************************/

            p->station_list[p->stano].t1_ctr = p->sta_ptr->resp_to_val
               ;
            p->station_list[p->stano].t1_ena = TRUE;

            /**********************************************************/
            /* set the call pending indicator in the station list.    */
            /**********************************************************/

            p->station_list[p->stano].call_pend = TRUE;

            /**********************************************************/
            /* call the find remote generator (new discovery)         */
            /**********************************************************/

            find_remote_gen(p);
          } 
        break;
      case                             /* the input command = receive.*/
         (RCVNO) :

        /*------------------------------------------------------------*/
        /* receive                                                    */
        /*------------------------------------------------------------*/

          {

            /**********************************************************/
            /* call the receive discovery manager.                    */
            /**********************************************************/

            rcv_discovery_mgr(p);
          } 
        break;
      default  :                       /* link manager sent invalid   
                                          command.                    */

        /**************************************************************/
        /* call error log - coding error. plc shutdown = coding error.*/
        /**************************************************************/

        lanerrlg(p, ERRID_LAN8011, NON_ALERT, PERM_PLC_ERR, 
           DLC_ERR_CODE, FILEN, LINEN);
    } 
  TRACE1(p, "DSVe");

  /********************************************************************/
  /* return to the link manager.                                      */
  /********************************************************************/

  return (0);
} 

find_self_gen(p)
  register struct port_dcl *p;
{
  int      i;
  int      vec_size;

  TRACE1(p, "FSGo");

  /********************************************************************/
  /* fetch a buffer from the pool for the "find self" packet, and save*/
  /* the "find self" buffer address in the sap list.                  */
  /********************************************************************/

  p->sap_list[p->sapno].find_self_addr = 0;
  p->m = (struct mbuf *)lanfetch(p);
 
  if                                   /* a buffer is available       */
     ((ulong_t)p->m != NO_BUF_AVAIL)
    {
      g_find_self_gen(p);
    } 
}                                      /* end find_self_gen;          */
find_remote_gen(p)
  register struct port_dcl *p;
{
  TRACE1(p, "FRGo");

  /********************************************************************/
  /* fetch a buffer from the pool for the "find name" packet          */
  /********************************************************************/

  p->m = (struct mbuf *)lanfetch(p);
 
  if                                   /* a buffer is available       */
     ((ulong_t)p->m != NO_BUF_AVAIL)
    {
      g_find_remote_gen(p);
    }                                  /* else error - no buffer      
                                          available - already error   
                                          logged                      */
}                                      /* end find_remote_gen;        */
rcv_discovery_mgr(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  rcv_discovery_mgr                                    */
/*                                                                    */
/* descriptive name:  receive discovery manager.                      */
/*                                                                    */
/* function:  checks the validity of the discovery packet and routes  */
/*            to the specified packet type handler:  add name query,  */
/*            add name response, find name, name found.               */
/*                                                                    */
/* input:  the discovery packet for lsap fc hex.                      */
/*                                                                    */
/* output:  the appropriate handler is called.                        */
/*                                                                    */
/*** end of specifications ********************************************/

{
  ushort   first_hw;

  TRACE1(p, "RDMo");

  if                                    /* this packet is for the LS/X
					  Remote IPL group address    */
/* <<< feature CDLI >>> */
/* <<< removed #ifdef  TRL >>> */

     (bcmp(&p->rcv_data.laddr[0], &p->common_cb.ripl_grp_addr[0], 6) == 0)
/* <<< end feature CDLI >>> */

    {
      /****************************************************************/
      /* call the Remote IPL handler routine                          */
      /****************************************************************/

      ripl_handler(p);
    }
  else
    /******************************************************************/
    /* it's not for LS/X Remote IPL, so it's for DLC Discovery        */
    /******************************************************************/
    {
      /****************************************************************/
      /* get addressability to the i-field.                           */
      /****************************************************************/

/* <<< feature CDLI >>> */
#ifndef TRLORFDDI
      p->i.i_field_ptr = &(p->d.rcv_data->ctl2);   /* un_info        */
#endif
/* <<< end feature CDLI >>> */

      bcopy(p->i.i_field_ptr, &first_hw, 2);
 
      if                                /* the first half word of the
					   i-field is greater than 4,
					   ie. the packet indicates that
					   the new discovery format was
					   used                      */
	 (first_hw > 4)
	{

	/*************************************************************/
	/* call the new discovery handler routine                    */
	/*************************************************************/

	new_discovery(p);
	}
        else
    	   lanfree(p, p->m);
    }
}                                      /* end rcv_discovery_mgr;      */

ripl_handler(p)
  register struct port_dcl *p;

/*** start of specifications *******************************************
 *
 * module name:  ripl_handler
 *
 * descriptive name:  Remote IPL handler.
 *
 * function:  checks to see if the LS/X application for OS/2 Remote IPL
 *            has been enabled and passes the receive packet to them
 *            if enabled.
 *
 * input:  the received Sap 0xFC packet
 *
 * output: the proper routine is called to service the received packet
 *         or the mbuf is returned
 *
 *** end of specifications ********************************************/
{
  struct dlc_chan *c_ptr;
  int discard_buf;                     /* discard buffer indicator    */
  ulong    dlctype;                    /* lan type for trace          */
  ulong    port_sta;                   /* port/station for trace      */

				       /* preset to discard the mbuf  */
  discard_buf = 1;

  if                                   /* sap 0xFC RIPL is "in-use"   */
     (p->sap_list[DISCOVERY_SAP/2].in_use == TRUE)
    {
				       /* get addressability to the
					  LS/X RIPL sap control block */
    p->sapno = DISCOVERY_SAP/2;
    p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].sap_cb_addr;

#ifdef   TRLORFDDI
#ifdef   DEBUG
    if (p->debug)
      printf("routing_info=%d\n", p->routing_info);
#endif

    if                                 /* routing information present */
       (p->routing_info == 1)
      {
				       /* set routing information flag
					  in the token ring header    */
      SETBIT(p->rcv_data.laddr[0], RI_PRESENT);
      }
#endif /* TRLORFDDI */

				       /* set results to network data
					  received                    */
    p->dlc_io_ext.flags = DLC_NETD;

				       /* return sap user correlator  */
    p->dlc_io_ext.sap_corr = p->sap_ptr->sap_profile.user_sap_corr;


				       /* setup data header length    */
/* <<< feature CDLI >>> */
#ifndef TRLORFDDI
    p->dlc_io_ext.dlh_len = UN_HDR_LENGTH;
#endif /* not TRLORFDDI */
#ifdef TRLORFDDI
    p->dlc_io_ext.dlh_len = UN_HDR_LENGTH + (p->rcv_data.ri_field[0]&0x1f);
#endif
/* <<< end feature CDLI >>> */

    p->m->m_data += UN_HDR_LENGTH;

/* <<< feature CDLI >>> */
#ifdef   TRLORFDDI
    p->m->m_len -= (UN_HDR_LENGTH + (p->rcv_data.ri_field[0]&0x1f));
#endif /* TRLORFDDI */
/* <<< end feature CDLI >>> */

#ifndef  TRLORFDDI
    p->m->m_len = p->lpdu_length-3;
#endif /* not TRLORFDDI */

				       /* setup channel pointer       */
    c_ptr = p->sap_ptr->user_sap_channel;

    /******************************************************************/
    /* setup values to record monitor trace                           */
    /******************************************************************/

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

    /******************************************************************/
    /* get station number in upper half word and get                  */
    /* number from port name in lower half word                       */
    /* Note: no station number for RIPL (set to 0)                    */
    /******************************************************************/

    #ifndef FDL
    port_sta = (p->dlc_port.namestr[3]);
	 #elif FDL
    port_sta = (p->dlc_port.namestr[4]);
	 #endif

    /******************************************************************/
    /* call trchkgt to record monitor trace data                      */
    /******************************************************************/

    trchkgt(HKWD_SYSX_DLC_MONITOR|dlctype, p->m,
	    p->m->m_len, p->dlc_io_ext.sap_corr, 0,
	    port_sta);

    /******************************************************************/
    /* setup values for performance trace                             */
    /******************************************************************/

    dlctype = HKWD_SYSX_DLC_PERF|DLC_TRACE_RDIDA;
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

    /******************************************************************/
    /* call trchklt to record performance data                        */
    /******************************************************************/

    trchklt(dlctype, p->m->m_len);

    /******************************************************************/
    /* call the user RCV network data routine                         */
    /******************************************************************/

    p->rc = (*c_ptr->rcvn_fa)(p->m, &(p->dlc_io_ext),c_ptr);

    switch (p->rc)
      {
      case  DLC_FUNC_OK :              /* only case supported, ie.    */
	discard_buf = 0;               /* no local busy function      */
	break;

      default  :

      /****************************************************************/
      /* call error log - user network data routine failed            */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN8086, NON_ALERT, INFO_ERR, 0, FILEN, LINEN);

      }                                /* end switch                  */
    }                                  /* endif: RIPL sap 0xFC in use */

  if                                   /* the buffer is to be
					  discarded                   */
     (discard_buf == 1)
    {
    /******************************************************************/
    /* return the buffer to the pool                                  */
    /******************************************************************/

    lanfree(p, p->m);
    }                                  /* endif:  discard buffer      */
}                                      /* end ripl_handler;           */

new_discovery(p)
  register struct port_dcl *p;

/*** start of specifications *******************************************
 *
 * module name:  new_discovery
 *
 * descriptive name:  new discovery handler.
 *
 * function:  handles the tr lan fap version of name discovery packets
 *
 * input:  the received discovery packet
 *
 * output: the proper routine is called to service the received packet
 *
 *** end of specifications ********************************************/

{
  int      retc;
#ifdef   TRLORFDDI
  struct ri_control_field ri_control;
  int      ri_length;
#endif

  TRACE1(p, "NDIo");

  /********************************************************************/
  /* preset the error indicator FALSE.                                */
  /********************************************************************/

  retc = DLC_OK;

  /********************************************************************/
  /* call the verify discovery vector routine.                        */
  /********************************************************************/

  retc = verify_vector(p, p->i.i_field_ptr);
 
  if                                   /* vector checks out ok        */
     (retc == DLC_OK)
    {
/* LEHb defect 44499 */
/* deleted 63 lines */
/* LEHe */
      if                               /* it's a find vector          */
         (p->vector_header.key == FIND_VECTOR_KEY)
        {

          /************************************************************/
          /* call the received find vector handler routine.           */
          /************************************************************/

          retc = rcvd_find_vector(p);
        } 
 
      else                             /* not a find vector so must be
                                          found vector,               */

        /**************************************************************/
        /* since already checked in the vector verify routine.        */
        /**************************************************************/

        {

          /************************************************************/
          /* call the received found vector handler routine.          */
          /************************************************************/

          retc = rcvd_found_vector(p);
        } 
    } 
 
  if                                   /* a loggable error has        
                                          occurred                    */
     ((retc != DLC_OK) && (retc != NO_MATCH))
    {
      lanerrlg(p, retc, NON_ALERT, INFO_ERR, DLC_REMOTE_CONN, FILEN, 
         LINEN);
    } 
 
  if                                   /* the buffer needs to be      
                                          returned                    */
     (retc != DLC_OK)

    /******************************************************************/
    /* return the buffer to the pool.                                 */
    /******************************************************************/

    lanfree(p, p->m);
}                                      /* end new_discovery;          */
rcvd_find_vector(p)
  register struct port_dcl *p;
{
  u_char   match_found;
  int      retc;                       /* return code                 */
  int      index;

  TRACE1(p, "RFVo");

  /*------------------------------------------------------------------*/
  /* SEARCH thru all active saps for one who's local name matches the */
  /* source name on the find. if a match is found, it's either an add */
  /* name echo or a remote add name that duplicates a local sap's name*/
  /* or a call echo.                                                  */
  /*------------------------------------------------------------------*/
  /* preset the "match found" indicator FALSE.                        */
  /********************************************************************/

  match_found = FALSE;

  /********************************************************************/
  /* clear return code                                                */
  /********************************************************************/

  retc = DLC_OK;

  /********************************************************************/
  /* for each of the possible saps                                    */
  /********************************************************************/
 

  for (index = 0; index <= 127; index++)
    {
 
      if                               /* a local name match is found */
         (match_found == TRUE)
        break;
 
      if                               /* the indexed sap is "in use" */
         (p->sap_list[index].in_use == TRUE)
        {

          /************************************************************/
          /* get addressability to the sap.                           */
          /************************************************************/

          p->sap_ptr = (struct sap_cb *)p->sap_list[index].sap_cb_addr
             ;
 
          if                           /* the received source name    
                                          matches the local name      */
             ((bcmp(p->source_name, 
             p->sap_ptr->sap_profile.laddr_name, p->source_name_len) 
             == 0) && (p->source_name_len == 
             p->sap_ptr->sap_profile.len_laddr_name))
            {

              /********************************************************/
              /* indicate that a match was found.                     */
              /********************************************************/

              match_found = TRUE;

              /********************************************************/
              /* set the sap index to the index found.                */
              /********************************************************/

              p->sapno = index;
            } 
        } 
    }                                  /* end do index;               */
 
  if                                   /* a match was found           */
     (match_found == TRUE)
    {
 
      if                               /* this is an echo of a local  
                                          add name query, ie. the sap 
                                          has                         */

      /****************************************************************/
      /* an add name pending, an add name echo is expected, and the   */
      /* received mac address matches the local mac address           */
      /****************************************************************/

         (((p->sap_list[p->sapno].addn_pend == TRUE) && (p->sap_list
         [p->sapno].addn_echo == TRUE)) && (bcmp(p->mac_vector.value, 
         p->common_cb.local_addr, 6) == 0))

        /**************************************************************/
        /* it's an echo of a local add name query (find self).        */
        /**************************************************************/

        add_name_vector_echo(p);
 
      else                             /* it's a remote station       
                                          attempting to add a         
                                          duplicate name, or a call   
                                          echo                        */
        {
 
          if                           /* the source name equals the  
                                          target name                 */
             ((bcmp(p->source_name, p->target_name, p->source_name_len
             ) == 0) && (p->source_name_len == p->target_name_len))

            /**********************************************************/
            /* it's a add name query that duplicates the local sap    */
            /* name.                                                  */
            /**********************************************************/

            {

              /********************************************************/
              /* call the duplicate add name vector routine.          */
              /********************************************************/

              dup_add_name_vector(p);
            } 
 
          else                         /* it's a call echo from a     
                                          local station.              */
            {

              /********************************************************/
              /* set the return code to "no match" to force return of */
              /* the buf.                                             */
              /********************************************************/

              retc = NO_MATCH;
            } 
        } 
    } 
 
  else                                 /* the received source name    
                                          does not equal any local sap
                                          name,                       */

    /******************************************************************/
    /* so it may be a call in.                                        */
    /******************************************************************/

    {

      /*--------------------------------------------------------------*/
      /* SEARCH thru all active saps for one who's local name matches */
      /* the target name on the find. if a match is found, it's an    */
      /* incomming remote call.                                       */
      /*--------------------------------------------------------------*/
      /* preset the "match found" indicator FALSE.                    */
      /****************************************************************/

      match_found = FALSE;

      /****************************************************************/
      /* for each of the possible saps                                */
      /****************************************************************/
 

      for (index = 0; index <= 127; index++)
        {
 
          if                           /* a local name match is found */
             (match_found == TRUE)
            break;
 
          if                           /* the indexed sap is "in use" */
             (p->sap_list[index].in_use == TRUE)
            {

              /********************************************************/
              /* get addressability to the sap.                       */
              /********************************************************/

              p->sap_ptr = (struct sap_cb *)p->sap_list[index].sap_cb_addr;
 
              if                       /* the received target name    
                                          matches the local name      */
                 ((bcmp(p->target_name, 
                 p->sap_ptr->sap_profile.laddr_name, 
                 p->sap_ptr->sap_profile.len_laddr_name) == 0) && 
                 (p->target_name_len == 
                 p->sap_ptr->sap_profile.len_laddr_name))
                {

                  /****************************************************/
                  /* indicate that a match was found.                 */
                  /****************************************************/

                  match_found = TRUE;

                  /****************************************************/
                  /* set the sap index to the index found.            */
                  /****************************************************/

                  p->sapno = index;
                } 
            } 
        }                              /* end do index;               */
 
      if                               /* a match was found           */
         (match_found == TRUE)

        /**************************************************************/
        /* it's an incomming remote call                              */
        /**************************************************************/

        {

          /************************************************************/
          /* call the incomming call vector handler routine.          */
          /************************************************************/

          retc = incomming_call_vector(p);
        } 
 
      else                             /* the target name does not    
                                          equal any local sap name, so*/

        /**************************************************************/
        /* it is not destined for this port.                          */
        /**************************************************************/

        {

          /************************************************************/
          /* set the return code to "no match".                       */
          /************************************************************/

          retc = NO_MATCH;
        } 
    } 
  return (retc);
}                                      /* end rcvd_find_vector;       */
rcvd_found_vector(p)
  register struct port_dcl *p;
{
  u_char   match_found;
  int      retc;

  TRACE1(p, "RFOo");

  /*------------------------------------------------------------------*/
  /* the CORRELATOR on a find (self) (add name query) is set to the   */
  /* sapno ored with sap_mask_corr. thus, if the found's CORRELATOR is*/
  /* for a sap which is adding its name, then call add name response. */
  /* otherwise, the found is not a remote response to add name, so it */
  /* may be a call response.                                          */
  /*------------------------------------------------------------------*/
  /* clear return code                                                */
  /********************************************************************/

  retc = DLC_OK;
 
  if                                   /* the received CORRELATOR     
                                          indicates that it's a sap   
                                          CORRELATOR                  */
     ((p->correlator_vector.value&SAP_CORR_MASK) != 0)

    /******************************************************************/
    /* it may be a remote response to a local add name (find self).   */
    /******************************************************************/

    {
 
      if                               /* the masked CORRELATOR value 
                                          is within a valid sap range */
         (((p->correlator_vector.value&~(SAP_CORR_MASK)) > 0) && 
         ((p->correlator_vector.value&~(SAP_CORR_MASK)) < 128))
        {

          /************************************************************/
          /* get addressability to the sap from the received          */
          /* CORRELATOR.                                              */
          /************************************************************/

          p->sapno = (p->correlator_vector.value&~(SAP_CORR_MASK));
          p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].
             sap_cb_addr;
 
          if                           /* the identified sap is in use
                                          and adding its name         */
             ((p->sap_list[p->sapno].in_use == TRUE) && (p->sap_list
             [p->sapno].addn_pend == TRUE))
            {

              /********************************************************/
              /* notify the user that the sap failed with a duplicate */
              /* name.                                                */
              /********************************************************/

              p->sap_ptr->sap_retcode = DLC_NAME_IN_USE;
              p->common_cb.sap_retcode = DLC_NAME_IN_USE;

              /********************************************************/
              /* call error log - add name -- local name in use. sap  */
              /* shutdown = user interface error.                     */
              /********************************************************/

              lanerrlg(p, ERRID_LAN8001, NON_ALERT, PERM_SAP_ERR, 
                 DLC_NAME_IN_USE, FILEN, LINEN);

              /********************************************************/
              /* set the return code to "no match" to return the      */
              /* buffer.                                              */
              /********************************************************/

              retc = NO_MATCH;
            } 
 
          else                         /* not adding name or not in   
                                          use, so return code =       */

            /**********************************************************/
            /* "no match".                                            */
            /**********************************************************/

            retc = NO_MATCH;
        } 
 
      else                             /* the received CORRELATOR is  
                                          not within range.           */
        retc = NO_MATCH;
    } 
 
  else                                 /* the received CORRELATOR does
                                          not indicate a sap, so it is*/

    /******************************************************************/
    /* possibly a remote response to a call.                          */
    /******************************************************************/

    {
 
      if                               /* the received CORRELATOR     
                                          value is within a valid     
                                          range                       */

      /****************************************************************/
      /* for a calling link station                                   */
      /****************************************************************/

         ((p->correlator_vector.value > 0) && 
         (p->correlator_vector.value <= MAX_SESSIONS))
        {

          /************************************************************/
          /* get addressability to the link station and its sap from  */
          /* the received CORRELATOR.                                 */
          /************************************************************/

          p->stano = (p->correlator_vector.value);
          p->sta_ptr = (struct station_cb *)p->station_list[p->stano].
             sta_cb_addr;
          p->sapno = p->station_list[p->stano].sapnum;
          p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].
             sap_cb_addr;
 
          if                           /* the identified station is in
                                          use and calling a remote    */
             ((p->station_list[p->stano].in_use == TRUE) && 
             (p->station_list[p->stano].call_pend == TRUE))
            {

              /********************************************************/
              /* call the call response for new discovery.            */
              /********************************************************/

              new_call_response(p);

              /********************************************************/
              /* free the received buffer                             */
              /********************************************************/

              lanfree(p, p->m);
            } 
 
          else                         /* station is not calling or   
                                          not in use, so return code =*/

            /**********************************************************/
            /* "no match".                                            */
            /**********************************************************/

            retc = NO_MATCH;
        } 
 
      else                             /* the received CORRELATOR is  
                                          not within range.           */
        retc = NO_MATCH;
    } 
  return (retc);
}                                      /* end rcvd_found_vector;      */
verify_vector(p,vect_addr)
  register struct port_dcl *p;

/*** start of specifications *******************************************
 *
 * module name:  verify_vector
 *
 * descriptive name: verify discovery vector
 *
 * function:          check discovery vector for correctness and set
 *                     pointers to major and minor subvectors if
 *                     vector is ok.
 *
 * input:  address of vector
 *** end of specifications ********************************************/

  char     *vect_addr;
{
  int      retc;                       /* return code                 */
  char     *ptr_end;                   /* vector pointer              */
  char     *ptr_begin;
  char     *anchor;                    /* vector pointer              */
  int      mode;                       /* major or minor vector scan  */
  int      total_len;                  /* sum of vector length fields */
  struct vector_header anchor_vector_header;
  ushort   vect_key;

  TRACE1(p, "VRVb");

  /********************************************************************/
  /* clear return code                                                */
  /********************************************************************/

  retc = DLC_OK;

  /********************************************************************/
  /* clear vector status, and length counter                          */
  /********************************************************************/

  total_len = p->common_cb.vector_status = 0;

  /********************************************************************/
  /* set up pointer to start of vector                                */
  /********************************************************************/

  anchor = vect_addr;
  ptr_begin = vect_addr;
  bcopy(vect_addr, &anchor_vector_header, SIZE_VECTOR_HEADER);
 
  if                                   /* vector is a find or found   
                                          vector                      */
     ((anchor_vector_header.key == FIND_VECTOR_KEY) || 
     (anchor_vector_header.key == FOUND_VECTOR_KEY))
    {
 
      if                               /* length of vector in range   */
         (anchor_vector_header.length < 100)
        {

          /************************************************************/
          /* save find/found vector key                               */
          /************************************************************/

          vect_key = anchor_vector_header.key;

          /************************************************************/
          /* setup end pointer                                        */
          /************************************************************/

          ptr_end = anchor_vector_header.length+ptr_begin;

          /************************************************************/
          /* save size of header in sum of vectors                    */
          /************************************************************/

          total_len += SIZE_VECTOR_HEADER;

          /************************************************************/
          /* move to start of subvector                               */
          /************************************************************/

          ptr_begin += SIZE_VECTOR_HEADER;

          /************************************************************/
          /* check each subvector                                     */
          /************************************************************/
 

          while (ptr_begin < ptr_end)
            {
              bcopy(ptr_begin, &p->vector_header, SIZE_VECTOR_HEADER);
 
              if                       /* current vector is a major   
                                          vector                      */
                 ((p->vector_header.key&MASK_MAJOR_VECTOR) == MAJOR_VECTOR)
                {

                  /****************************************************/
                  /* call MAJOR_VECTOR to check syntax of either      */
                  /* SEARCH id or ORIGIN id                           */
                  /****************************************************/

                  retc = major_vec(p, &p->vector_header, ptr_begin);
                } 
 
              else                     /* check minor vector          */
                {

                  /****************************************************/
                  /* set minor vector mode                            */
                  /****************************************************/

                  mode = MINOR_VECTOR_MODE;

                  /****************************************************/
                  /* call minor_vector to check syntax of minor       */
                  /* vectors                                          */
                  /****************************************************/

                  retc = minor_vector(p, &p->vector_header, mode, ptr_begin);
                } 
 
              if                       /* check of vector ok          */
                 (retc == DLC_OK)

                /******************************************************/
                /* move to next vector                                */
                /******************************************************/

                {

                  /****************************************************/
                  /* add length of vector to sum count                */
                  /****************************************************/

                  total_len += p->vector_header.length;
                  ptr_begin += p->vector_header.length;
                } 
 
              else                     /* error in vector             */
                {

                  /****************************************************/
                  /* move pointer to end of vector to halt checking   */
                  /****************************************************/

                  ptr_begin = ptr_end;
                } 
            }                          /* end do until;               */
 
          if                           /* SEARCH successful           */
             (retc == DLC_OK)

            /**********************************************************/
            /* check vector for completeness                          */
            /**********************************************************/

            {
 
              if                       /* length of subvectors =      
                                          overall vector              */

              /********************************************************/
              /* length                                               */
              /********************************************************/

                 (total_len == anchor_vector_header.length)
                {
 
                  if                   /* find vector                 */
                     (vect_key == FIND_VECTOR_KEY)

                    /**************************************************/
                    /* check vector for required vectors              */
                    /**************************************************/

                    {
 
                      if               /* all required vectors not    
                                          present                     */
                         (p->common_cb.vector_status != (CORRELATOR|
                         MAC|LLSAP|ORIGIN_NSA|SEARCH_NSA|ORIGIN|SEARCH
                         |ORIGIN_NAME|SEARCH_NAME))

                        /**********************************************/
                        /* set error code                             */
                        /**********************************************/

                        retc = ERRID_LAN0030;
                    } 
 
                  else                 /* check found vector for      
                                          required vectors            */
                    {
 
                      if               /* all required vectors not    
                                          present                     */
                         (p->common_cb.vector_status != (CORRELATOR|
                         MAC|LLSAP|RESP))

                        /**********************************************/
                        /* set error code                             */
                        /**********************************************/

                        retc = ERRID_LAN0030;
                    } 
                } 
 
              else                     /* set length error due to     
                                          subvectors length           */

                /******************************************************/
                /* not equal total length                             */
                /******************************************************/

                retc = ERRID_LAN002C;
            } 
        } 
 
      else                             /* vector length not in range  */
        {

          /************************************************************/
          /* set out of range error                                   */
          /************************************************************/

          retc = ERRID_LAN002C;
        } 
    } 
 
  else                                 /* not found or find vector    */

    /******************************************************************/
    /* set not found/find vector                                      */
    /******************************************************************/

    retc = ERRID_LAN002F;
  bcopy(&anchor_vector_header, &p->vector_header, SIZE_VECTOR_HEADER);

  /********************************************************************/
  /* return vector ok or error found                                  */
  /********************************************************************/

  TRACE2(p, "VRVe", retc);
  return (retc);
}                                      /* end verify_vector;          */
major_vec(p,vect_addr,orig_data)
  register struct port_dcl *p;

/*** start of specifications *******************************************
 *
 * module name:  major_vec
 *
 * descriptive name: verify discovery major vector
 *
 * function:          check discovery major vector for correctness
 *                     and set pointers to minor vectors if vector is
 *                     ok.
 *
 * input:  address of vector
 *
 *** end of specifications ********************************************/

  struct vector_header *vect_addr;     /* pointer to word aligned     */
  char     *orig_data;                 /* pointer to orig data        */
{
  int      retc;                       /* return code                 */

  /********************************************************************/
  /* clear return code                                                */
  /********************************************************************/

  retc = DLC_OK;

  /********************************************************************/
  /* clear count of nsa and object vectors                            */
  /********************************************************************/

  p->common_cb.nsa_count = p->common_cb.object_count = 0;

/* defect 96697 */
  /********************************************************************/
  /* initialize nsa and object pointers for source/target             */
  /********************************************************************/

  p->ptr_object = 0;
  p->ptr_nsa = 0;
/* end defect 96697 */
 
  switch                               /* major vector type           */
     (vect_addr->key)
    {
      case                             /* SEARCH id major vector      */
         (SEARCH_ID_KEY) :
 
        if                             /* not duplicate vector        */
           ((p->common_cb.vector_status&SEARCH) == FALSE)

          /************************************************************/
          /* check vector                                             */
          /************************************************************/

          {

            /**********************************************************/
            /* set vector found indicator                             */
            /**********************************************************/

            p->common_cb.vector_status |= SEARCH;

            /**********************************************************/
            /* call check to check SEARCH id vector                   */
            /**********************************************************/

            retc = check(p, vect_addr, orig_data);
 
            if                         /* vector ok                   */
               (retc == DLC_OK)
              {

                /******************************************************/
                /* set pointer to SEARCH id vector                    */
                /******************************************************/

                p->ptr_tid = (char *)vect_addr;
 
                if                     /* nsa vector present          */
                   (p->ptr_nsa != 0)

                  /****************************************************/
                  /* set nsa pointer                                  */
                  /****************************************************/

                  {
                    p->ptr_t_nsa = p->ptr_nsa;

                    /**************************************************/
                    /* set found nsa indicator                        */
                    /**************************************************/

                    p->common_cb.vector_status |= SEARCH_NSA;
                  } 
 
                else                   /* set missing vector error    
                                          message                     */
                  retc = ERRID_LAN0030;
 
                if                     /* object vector present       */
                   (p->ptr_object != 0)
                  {

                    /**************************************************/
                    /* set SEARCH object pointer and save target      */
                    /* name/length                                    */
                    /**************************************************/

                    p->target_name_len = p->object_vector.length-
                       SIZE_VECTOR_HEADER;
                    bcopy(p->object_vector.value, p->target_name, 
                       p->target_name_len);
                    p->common_cb.vector_status |= SEARCH_NAME;
                  } 
 
                else                   /* set missing vector error    
                                          message                     */
                  retc = ERRID_LAN0030;
              } 
          } 
 
        else                           /* duplicate vector            */
          retc = ERRID_LAN002E;
        break;
      case                             /* ORIGIN id major vector      */
         (ORIGIN_ID_KEY) :
 
        if                             /* not duplicate vector        */
           ((p->common_cb.vector_status&ORIGIN) == FALSE)

          /************************************************************/
          /* check vector                                             */
          /************************************************************/

          {

            /**********************************************************/
            /* set vector found indicator                             */
            /**********************************************************/

            p->common_cb.vector_status |= ORIGIN;

            /**********************************************************/
            /* call check to check ORIGIN id vector                   */
            /**********************************************************/

            retc = check(p, vect_addr, orig_data);
 
            if                         /* vector ok                   */
               (retc == DLC_OK)

              /********************************************************/
              /* set pointer to ORIGIN id vector                      */
              /********************************************************/

              {
                p->ptr_sid = (char *)vect_addr;
 
                if                     /* nsa vector present          */
                   (p->ptr_nsa != 0)

                  /****************************************************/
                  /* set nsa pointer                                  */
                  /****************************************************/

                  {
                    p->ptr_s_nsa = p->ptr_nsa;

                    /**************************************************/
                    /* set found nsa indicator                        */
                    /**************************************************/

                    p->common_cb.vector_status |= ORIGIN_NSA;
                  } 
 
                else                   /* set missing vector error    
                                          message                     */
                  retc = ERRID_LAN0030;
 
                if                     /* object vector present       */
                   (p->ptr_object != 0)
                  {

                    /**************************************************/
                    /* set ORIGIN object pointer                      */
                    /**************************************************/

                    p->source_name_len = p->object_vector.length-
                       SIZE_VECTOR_HEADER;
                    bcopy(p->object_vector.value, p->source_name, 
                       p->source_name_len);
                    p->common_cb.vector_status |= ORIGIN_NAME;
                  } 
 
                else                   /* set missing vector error    
                                          message                     */
                  retc = ERRID_LAN0030;
              } 
          } 
 
        else                           /* duplicate vector            */
          retc = ERRID_LAN002E;
        break;
      default  :                       /* set unknown vector error    */
        retc = ERRID_LAN002F;
    } 
  TRACE2(p, "MJVe", retc);

  /********************************************************************/
  /* return success or vector eror                                    */
  /********************************************************************/

  return (retc);
}                                      /* end major_vec;              */
check(p,vect_addr,orig_data)
  register struct port_dcl *p;

/*** start of specifications *******************************************
 *
 * module name:  check
 *
 * descriptive name: verify discovery major vector
 *
 * function:          check discovery major vector for correctness
 *                     and set pointers to minor vectors if vector is
 *                     ok.
 *
 * input:  address of vector
 *
 *** end of specifications ********************************************/

  struct vector_header *vect_addr;
  char     *orig_data;
{
  int      retc;                       /* return code                 */
  char     *ptr_end;                   /* vector pointer              */
  char     *ptr_begin;
  char     *anchor;                    /* vector pointer              */
  int      mode;                       /* major or minor vector scan  */
  struct vector_header m_vector_header;

  TRACE1(p, "CMVo");

  /********************************************************************/
  /* clear return code                                                */
  /********************************************************************/

  retc = DLC_OK;

  /********************************************************************/
  /* set up pointer to start of vector                                */
  /********************************************************************/

  ptr_begin = (char *)orig_data;

  /********************************************************************/
  /* set major vector mode                                            */
  /********************************************************************/

  mode = MAJOR_VECTOR_MODE;
 
  if                                   /* length of vector in range   */
     (vect_addr->length < 100)

    /******************************************************************/
    /* check subvectors                                               */
    /******************************************************************/

    {

      /****************************************************************/
      /* setup end pointer                                            */
      /****************************************************************/

      ptr_end = vect_addr->length+ptr_begin;

      /****************************************************************/
      /* move to start of subvector                                   */
      /****************************************************************/

      ptr_begin += SIZE_VECTOR_HEADER;

      /****************************************************************/
      /* check each subvector                                         */
      /****************************************************************/
 

      while (ptr_begin < ptr_end)
        {
          bcopy(ptr_begin, &m_vector_header, SIZE_VECTOR_HEADER);
 
          if                           /* current vector is a minor   
                                          vector                      */
             ((m_vector_header.key&MASK_MAJOR_VECTOR) != MAJOR_VECTOR)

            /**********************************************************/
            /* check minor vector                                     */
            /**********************************************************/

            {

              /********************************************************/
              /* call minor_vector to check syntax of vector          */
              /********************************************************/

              retc = minor_vector(p, &m_vector_header, mode, ptr_begin
                 );
            } 
 
          else                         /* major or unknown vector     */
            {

              /********************************************************/
              /* set error return code                                */
              /********************************************************/

              retc = ERRID_LAN002F;
            } 
 
          if                           /* check of vector ok          */
             (retc == DLC_OK)

            /**********************************************************/
            /* move to next vector                                    */
            /**********************************************************/

            ptr_begin += m_vector_header.length;
 
          else                         /* error in vector             */
            {

              /********************************************************/
              /* move pointer to end of vector to halt checking       */
              /********************************************************/

              ptr_begin = ptr_end;
            } 
        }                              /* end do until;               */
    } 
 
  else                                 /* vector length not in range  */
    {

      /****************************************************************/
      /* set out of range error                                       */
      /****************************************************************/

      retc = ERRID_LAN002C;
    } 

  /********************************************************************/
  /* return vector ok or error found                                  */
  /********************************************************************/

  return (retc);
}                                      /* end check;                  */
minor_vector(p,vect_addr,vect_mode,orig_data)
  register struct port_dcl *p;

/*** start of specifications *******************************************
 *
 * module name:  minor_vector
 *
 * descriptive name: verify discovery minor vector
 *
 * function:          check discovery minor vector for correctness
 *                     and set pointers to minor vectors if vector is
 *                     ok.
 *
 * input:  address of vector
 *
 * output:
 *** end of specifications ********************************************/

  struct vector_header *vect_addr;
  int      vect_mode;                  /* major or minor vector       */
  char     *orig_data;
{
  int      retc;                       /* return code                 */

  /********************************************************************/
  /* clear return code                                                */
  /********************************************************************/

  retc = DLC_OK;
 
  switch                               /* minor vector type           */
     (vect_addr->key)
    {
      case                             /* CORRELATOR minor vector     */
         (CORRELATOR_KEY) :
 
        if                             /* not duplicate vector and    
                                          valid mode                  */
           (((p->common_cb.vector_status&CORRELATOR) == FALSE) && 
           (vect_mode == MINOR_VECTOR_MODE))

          /************************************************************/
          /* check vector                                             */
          /************************************************************/

          {

            /**********************************************************/
            /* set vector found indicator                             */
            /**********************************************************/

            p->common_cb.vector_status |= CORRELATOR;
 
            if                         /* valid length                */
               (vect_addr->length == SIZE_CORRELATOR_VECTOR)

              /********************************************************/
              /* set pointer to CORRELATOR                            */
              /********************************************************/

              {
                bcopy(orig_data, &p->correlator_vector, 
                   vect_addr->length);
                p->ptr_corr = (char *)vect_addr;
              } 
 
            else                       /* length error                */
              retc = ERRID_LAN002C;
          } 
 
        else                           /* duplicate vector or mode    
                                          error                       */
          retc = ERRID_LAN002E;
        break;
      case                             /* name structure architecture 
                                          (nsa) vector                */
         (NSA_KEY) :
 
        if                             /* not duplicate vector and    
                                          valid mode                  */
           ((p->common_cb.nsa_count < 1) && (vect_mode == 
           MAJOR_VECTOR_MODE))

          /************************************************************/
          /* check vector                                             */
          /************************************************************/

          {

            /**********************************************************/
            /* increment vector count                                 */
            /**********************************************************/

            p->common_cb.nsa_count++;
 
            if                         /* valid length                */
               (vect_addr->length == SIZE_NSA_VECTOR)

              /********************************************************/
              /* set pointer to CORRELATOR                            */
              /********************************************************/

              {
#ifdef   lehall
                bcopy(orig_data, &p->nsa_vector, SIZE_NSA_VECTOR);
 
                if                     /* local sna supported name    */
                   (p->nsa_vector.value == LOCAL_NSA_ID)

                  /****************************************************/
                  /* save pointer                                     */
                  /****************************************************/

                  p->ptr_nsa = (char *)vect_addr;
#endif
                p->ptr_nsa = (char *)vect_addr;
              } 
 
            else                       /* length error                */
              retc = ERRID_LAN002C;
          } 
 
        else                           /* duplicate vector or mode    
                                          error                       */
          retc = ERRID_LAN002E;
        break;
      case                             /* designated mac vector       */
         (MAC_KEY) :
 
        if                             /* not duplicate vector and    
                                          valid mode                  */
           (((p->common_cb.vector_status&MAC) == FALSE) && (vect_mode 
           == MINOR_VECTOR_MODE))

          /************************************************************/
          /* check vector                                             */
          /************************************************************/

          {

            /**********************************************************/
            /* set vector found indicator                             */
            /**********************************************************/

            p->common_cb.vector_status |= MAC;
 
            if                         /* valid length                */
               (vect_addr->length == SIZE_MAC_VECTOR)

              /********************************************************/
              /* set pointer to designated mac vector                 */
              /********************************************************/

              {
                bcopy(orig_data, &p->mac_vector, vect_addr->length);
                p->ptr_mac = (char *)vect_addr;
              } 
 
            else                       /* length error                */
              retc = ERRID_LAN002C;
          } 
 
        else                           /* duplicate vector or mode    
                                          error                       */
          retc = ERRID_LAN002E;
        break;
      case                             /* designated lsap vector      */
         (LSAP_KEY) :
 
        if                             /* not duplicate vector and    
                                          valid mode                  */
           (((p->common_cb.vector_status&LLSAP) == FALSE) && 
           (vect_mode == MINOR_VECTOR_MODE))

          /************************************************************/
          /* check vector                                             */
          /************************************************************/

          {

            /**********************************************************/
            /* set vector found indicator                             */
            /**********************************************************/

            p->common_cb.vector_status |= LLSAP;
 
            if                         /* valid length                */
               (vect_addr->length == SIZE_LSAP_VECTOR)

              /********************************************************/
              /* set pointer to designated lsap vector                */
              /********************************************************/

              {
                bcopy(orig_data, &p->lsap_vector, vect_addr->length);
                p->ptr_lsap = (char *)vect_addr;
              } 
 
            else                       /* length error                */
              retc = ERRID_LAN002C;
          } 
 
        else                           /* duplicate vector or mode    
                                          error                       */
          retc = ERRID_LAN002E;
        break;
      case                             /* response code vector        */
         (RESP_KEY) :
 
        if                             /* not duplicate vector and    
                                          valid mode                  */
           (((p->common_cb.vector_status&RESP) == FALSE) && (vect_mode
           == MINOR_VECTOR_MODE))

          /************************************************************/
          /* check vector                                             */
          /************************************************************/

          {

            /**********************************************************/
            /* set vector found indicator                             */
            /**********************************************************/

            p->common_cb.vector_status |= RESP;
 
            if                         /* valid length                */
               (vect_addr->length == SIZE_RESPONSE_VECTOR)

              /********************************************************/
              /* set pointer to designated lsap vector                */
              /********************************************************/

              {
                bcopy(orig_data, &p->response_vector, 
                   vect_addr->length);
                p->ptr_resp = (char *)vect_addr;
              } 
 
            else                       /* length error                */
              retc = ERRID_LAN002C;
          } 
 
        else                           /* duplicate vector or mode    
                                          error                       */
          retc = ERRID_LAN002E;
        break;
      case                             /* object name vector          */
         (OBJECT_KEY) :
 
        if                             /* not duplicate vector and    
                                          valid mode                  */
           ((p->common_cb.object_count < 1) && (vect_mode == 
           MAJOR_VECTOR_MODE))

          /************************************************************/
          /* check vector                                             */
          /************************************************************/

          {

            /**********************************************************/
            /* increment vector count                                 */
            /**********************************************************/

            p->common_cb.object_count++;
 
            if                         /* valid length                */
               (vect_addr->length <= 28)

              /********************************************************/
              /* set pointer to object name vector                    */
              /********************************************************/

              {
                bcopy(orig_data, &p->object_vector, vect_addr->length)
                   ;
                p->ptr_object = (char *)vect_addr;
              } 
 
            else                       /* length error                */
              retc = ERRID_LAN002C;
          } 
 
        else                           /* duplicate vector or mode    
                                          error                       */
          retc = ERRID_LAN002E;
        break;
      default  :                       /* unknown vector              */

        /**************************************************************/
        /* set error code                                             */
        /**************************************************************/

        retc = ERRID_LAN002F;
    } 

  /********************************************************************/
  /* return vector ok or error found                                  */
  /********************************************************************/

  return (retc);
}                                      /* end minor_vector;           */
add_name_vector_echo(p)
  register struct port_dcl *p;
{
  TRACE1(p, "ANEo");
 
  if                                   /* the "find self" was expected
                                          to echo                     */
     (p->sap_list[p->sapno].find_self_echo == TRUE)
    {

      /****************************************************************/
      /* reset the add name echo indicator.                           */
      /****************************************************************/

      p->sap_list[p->sapno].find_self_echo = FALSE;
    } 

  /********************************************************************/
  /* ignore the name query and return the buffer to the pool.         */
  /********************************************************************/

  lanfree(p, p->m);
}                                      /* end add_name_vector_echo;   */
dup_add_name_vector(p)
  register struct port_dcl *p;
{
  g_dup_add_name_vector(p);
}                                      /* end dup_add_name_vector;    */
found_vector_gen(p,input_sap)
  register struct port_dcl *p;

/*** start of specifications *******************************************
 *
 * module name:  found_vector_gen
 *
 * descriptive name:  name found response vector generator
 *
 * function:  transmits the name found response back to the remote in
 *            the same buffer that the find name query was received in
 *
 * input:  the find name query packet
 *
 * output: the name found response packet is transmitted to the remote
 *
 *** end of specifications ********************************************/

  u_char   input_sap;
{
  g_found_vector_gen(p, input_sap);
}                                      /* end found_vector_gen;       */
incomming_call_vector(p)
  register struct port_dcl *p;

/*** start of specifications *******************************************
 *
 * module name:  incomming_call_vector
 *
 * descriptive name: incomming call vector
 *
 * function:  Checks the validity of the find name query received for
 *            local SAP-0 , and completes any "listen" in progress
 *            if the received query is valid and if the attachment
 *            doesn't already exist as an active or a call pending
 *            station.
 *
 * input:  The received command packet for LSAP-0  hex.
 *
 * output:  The associated link station is notified of "listen
 *          completion".
 *
 *** end of specifications ********************************************/

{
  int      match_found,index,retc;

  TRACE1(p, "ICVb");

  /*------------------------------------------------------------------*/
  /* check for a match in the active stations or in the stations that */
  /* have calls that are pending "name found" responses.              */
  /*------------------------------------------------------------------*/
  /* preset the "match found" indicator to FALSE again.               */
  /********************************************************************/

  match_found = FALSE;

  /********************************************************************/
  /* preset the return code as unsuccessful.                          */
  /********************************************************************/

  retc = NO_MATCH;

  /********************************************************************/
  /* for each station in the station list                             */
  /********************************************************************/
 

  for (index = 1; index < MAX_SESSIONS; index++)
    {
 
      if                               /* a match is found for the    
                                          remote name and local sap.  */
         (match_found == TRUE)
        break;
 
      if                               /* the indexed station operates
                                          under the found local sap,  */

      /****************************************************************/
      /* and it is either fully active or pending a call completion   */
      /****************************************************************/

         ((p->station_list[index].sapnum == p->sapno) && 
         ((p->station_list[index].sta_active == TRUE) || 
         (p->station_list[index].call_pend == TRUE)))
        {

          /************************************************************/
          /* get addressability to the station's control block.       */
          /************************************************************/

          p->sta_ptr = (struct station_cb *)p->station_list[index].
             sta_cb_addr;
          p->stano = index;
 
          if                           /* the station's remote name   
                                          equals the incomming source 
                                          name                        */
             ((bcmp(p->sta_ptr->ls_profile.raddr_name, p->source_name,
             p->source_name_len) == 0) && 
             (p->sta_ptr->ls_profile.len_raddr_name == 
             p->source_name_len))
            {

              /********************************************************/
              /* set the match found indicator.                       */
              /********************************************************/

              match_found = TRUE;

              /********************************************************/
              /* set the return code = succesful.                     */
              /********************************************************/

              retc = DLC_OK;

              /********************************************************/
              /* set the station index to the index found.            */
              /********************************************************/

              p->stano = index;
              p->sta_ptr = (struct station_cb *)p->station_list
                 [p->stano].sta_cb_addr;

              /********************************************************/
              /* send the "name found" response vector.               */
              /********************************************************/

              found_vector_gen(p, p->sap_ptr->sap_profile.local_sap);
            } 
        } 

      /****************************************************************/
      /* call completion, so fall thru.                               */
      /****************************************************************/

    }                                  /* end for index;              */
 
  if                                   /* no match has been found yet */
     (match_found == FALSE)
    {

      /*--------------------------------------------------------------*/
      /* check for a listen pending condition                         */
      /*--------------------------------------------------------------*/
 

      if                               /* a listen is pending a       
                                          connection on the found lsap*/
         (p->sap_list[p->sapno].listen_pend == TRUE)
        {

          /************************************************************/
          /* reset the listen pending indicator.                      */
          /************************************************************/

          p->sap_list[p->sapno].listen_pend = FALSE;

          /************************************************************/
          /* save the incomming remote's address, sap, and name       */
          /* values.                                                  */
          /************************************************************/

          bcopy(p->rcv_data.raddr, p->sap_ptr->listen_raddr, 6);
          p->sap_ptr->incomming_rsap = ((p->lsap_vector.value)
             &RESP_OFF);
          p->sap_ptr->listen_rname_length = (p->object_vector.length-
             SIZE_VECTOR_HEADER);
          bcopy(p->object_vector.value, p->sap_ptr->listen_rname, 
             p->sap_ptr->listen_rname_length);

          /************************************************************/
          /* set the match found indicator.                           */
          /************************************************************/

          match_found = TRUE;

          /************************************************************/
          /* set the return code = succesful.                         */
          /************************************************************/

          retc = DLC_OK;

          /************************************************************/
          /* retrieve the station index pending the listen completion.*/
          /************************************************************/

          p->stano = p->sap_ptr->listen_stano;

          /************************************************************/
          /* get addressability to the station's control block.       */
          /************************************************************/

          p->sta_ptr = (struct station_cb *)p->station_list[p->stano].
             sta_cb_addr;

/* LEHb defect 44499 */
#ifdef   TRLORFDDI
	  /************************************************************/
	  /* call set station route routine to save any bridge        */
	  /* routing that was received in the find packet             */
	  /************************************************************/

	  set_sta_route (p);

#endif /* TRLORFDDI */
/* LEHe */
          /************************************************************/
          /* send the "name found" response vector->                  */
          /************************************************************/

          found_vector_gen(p, p->sap_ptr->sap_profile.local_sap);

          /************************************************************/
          /* build hashing string for the remote station address, the */
          /* local sap, and the remote sap values.                    */
          /************************************************************/

          bcopy(p->sap_ptr->listen_raddr, 
             p->common_cb.u_h.s_h.hash_string_raddr, 6);
          p->common_cb.u_h.s_h.hash_string_rsap = 
             p->sap_ptr->incomming_rsap;
          p->common_cb.u_h.s_h.hash_string_lsap = 
             p->sap_ptr->sap_profile.local_sap;

          /************************************************************/
          /* call the "add station to receive hash table" routine,    */
          /* with the hashing string and stationno.                   */
          /************************************************************/

          add_sta_to_hash(p);

          /************************************************************/
          /* call the link station with command =listen completion.   */
          /************************************************************/

          lansta(p, LISTEN_CMPLNO);
        } 
    } 
  TRACE2(p, "ICVe", retc);
  return (retc);
}                                      /* end incomming_call_vector;  */
build_vector(p,vect_addr,vect_type,vect_corr)
  register struct port_dcl *p;

/*** start of specifications *******************************************
 *
 * module name:  build_vector
 *
 * descriptive name: build discovery find and found vectors
 *
 * function: build major vectors and minor vectors for discovery
 *                     find vector and builds minor vectors for
 *                     discovery found vector->
 *
 * input:  address of vector, type of vector
 *
 * output:
 *** end of specifications ********************************************/

  char     *vect_addr;                 /* start addr                  */
  int      vect_type;                  /* type of vector              */
  int      vect_corr;                  /* CORRELATOR value            */
{
  char     *ptr_begin;
  char     *anchor_ptr;                /* vector pointer              */
  char     *name_ptr;                  /* name vector pointer         */
  int      name_len;                   /* length of name              */
  int      len;

  TRACE4(p, "BVCo", vect_addr, vect_type, vect_corr);

  /********************************************************************/
  /* clear return code retc = DLC_OK; set up pointers to start of     */
  /* vector                                                           */
  /********************************************************************/

  anchor_ptr = vect_addr;
  ptr_begin = vect_addr;

  /********************************************************************/
  /* move past vector header                                          */
  /********************************************************************/

  ptr_begin = ptr_begin+SIZE_VECTOR_HEADER;

  /*------------------------------------------------------------------*/
  /* build CORRELATOR vector                                          */
  /*------------------------------------------------------------------*/

    {

      /****************************************************************/
      /* set vector type to CORRELATOR vector                         */
      /****************************************************************/

      p->correlator_vector.key = CORRELATOR_KEY;

      /****************************************************************/
      /* set length of CORRELATOR vector                              */
      /****************************************************************/

      p->correlator_vector.length = SIZE_CORRELATOR_VECTOR;

      /****************************************************************/
      /* set CORRELATOR value                                         */
      /****************************************************************/

      p->correlator_vector.value = vect_corr;
    } 

  /********************************************************************/
  /* move past CORRELATOR vector                                      */
  /********************************************************************/

  bcopy(&p->correlator_vector, ptr_begin, SIZE_CORRELATOR_VECTOR);
  ptr_begin = ptr_begin+p->correlator_vector.length;

  /*------------------------------------------------------------------*/
  /* build designated mac vector                                      */
  /*------------------------------------------------------------------*/

    {

      /****************************************************************/
      /* set vector type to mac vector                                */
      /****************************************************************/

      p->mac_vector.key = MAC_KEY;

      /****************************************************************/
      /* set length of mac vector                                     */
      /****************************************************************/

      p->mac_vector.length = 6+SIZE_VECTOR_HEADER;

      /****************************************************************/
      /* set mac name value                                           */
      /****************************************************************/

      bcopy(p->common_cb.local_addr, p->mac_vector.value, 6);
    } 

  /********************************************************************/
  /* move past mac vector                                             */
  /********************************************************************/

  bcopy(&p->mac_vector, ptr_begin, p->mac_vector.length);
  ptr_begin = ptr_begin+p->mac_vector.length;

  /*------------------------------------------------------------------*/
  /* build lsap vector                                                */
  /*------------------------------------------------------------------*/

    {

      /****************************************************************/
      /* set vector type to lsap vector                               */
      /****************************************************************/

      p->lsap_vector.key = LSAP_KEY;

      /****************************************************************/
      /* set length of lsap vector                                    */
      /****************************************************************/

      p->lsap_vector.length = SIZE_LSAP_VECTOR;

      /****************************************************************/
      /* set lsap value                                               */
      /****************************************************************/

      p->lsap_vector.value = p->sap_ptr->sap_profile.local_sap;
    } 

  /********************************************************************/
  /* move past lsap vector                                            */
  /********************************************************************/

  bcopy(&p->lsap_vector, ptr_begin, p->lsap_vector.length);
  ptr_begin = ptr_begin+p->lsap_vector.length;
 
  if                                   /* found vector being built    */
     (vect_type == FOUND_VECTOR_KEY)

    /******************************************************************/
    /* complete found vector                                          */
    /******************************************************************/

    {

      /****************************************************************/
      /* set vector type to response vector                           */
      /****************************************************************/

      p->response_vector.key = RESP_KEY;

      /****************************************************************/
      /* set length of response vector                                */
      /****************************************************************/

      p->response_vector.length = SIZE_RESPONSE_VECTOR;

      /****************************************************************/
      /* set response value                                           */
      /****************************************************************/

      p->response_vector.value = RESOURCE_AVAIL;

      /****************************************************************/
      /* move past response vector                                    */
      /****************************************************************/

      bcopy(&p->response_vector, ptr_begin, p->response_vector.length)
         ;
      ptr_begin = ptr_begin+p->response_vector.length;
    } 
 
  else                                 /* complete find vector        */
    {

      /*--------------------------------------------------------------*/
      /* p->sap_ptr is always set prior to call p->sta_ptr is always  */
      /* set for a connect_out call                                   */
      /*--------------------------------------------------------------*/
      /*--------------------------------------------------------------*/
      /* build SEARCH id vector                                       */
      /*--------------------------------------------------------------*/
 

      if                               /* adding name                 */
         (p->sap_ptr->sap_state == ADDING_NAME)

        /**************************************************************/
        /* find self                                                  */
        /**************************************************************/

        {

          /************************************************************/
          /* name = p->sap_ptr->sap_profile.laddr_name                */
          /************************************************************/

          name_ptr = p->sap_ptr->sap_profile.laddr_name;
          name_len = p->sap_ptr->sap_profile.len_laddr_name;
        } 
 
      else                             /* find remote                 */
        {

          /************************************************************/
          /* name = p->sta_ptr->ls_profile.raddr_name                 */
          /************************************************************/

          name_ptr = p->sta_ptr->ls_profile.raddr_name;
          name_len = p->sta_ptr->ls_profile.len_raddr_name;
        } 
      len = build_id(p, ptr_begin, name_ptr, name_len, SEARCH_ID_KEY);

      /****************************************************************/
      /* update pointer vector                                        */
      /****************************************************************/

      ptr_begin = ptr_begin+len;

      /*--------------------------------------------------------------*/
      /* build ORIGIN id vector                                       */
      /*--------------------------------------------------------------*/

      len = build_id(p, ptr_begin, p->sap_ptr->sap_profile.laddr_name,
         p->sap_ptr->sap_profile.len_laddr_name, ORIGIN_ID_KEY);

      /****************************************************************/
      /* update pointer vector                                        */
      /****************************************************************/

      ptr_begin = ptr_begin+len;
    }                                  /* find                        */

  /********************************************************************/
  /* set length of find/found vector return length of vector          */
  /********************************************************************/

  len = ptr_begin-anchor_ptr;

  /********************************************************************/
  /* set type of vector to build                                      */
  /********************************************************************/

  p->vector_header.key = vect_type;
  p->vector_header.length = len;
  bcopy(&p->vector_header, anchor_ptr, SIZE_VECTOR_HEADER);
  return (len);
}                                      /* end build_vector;           */
build_id(p,vect_addr,name_addr,name_length,vect_type)
  register struct port_dcl *p;

/*** start of specifications *******************************************
 *
 * module name:  build_id
 *
 * descriptive name: build discovery ORIGIN and SEARCH id vectors
 *
 * function: build ORIGIN and SEARCH id vectors.
 *
 * input:  address of vector, type of vector
 *
 * output: none
 *
 *** end of specifications ********************************************/

  char     *vect_addr;                 /* start addr                  */
  char     *name_addr;                 /* name addr                   */
  int      name_length;
  int      vect_type;                  /* type of vector              */
{
  char     *ptr_begin;
  char     *ptr_temp;
  char     *name_ptr;                  /* name vector pointer         */

  /********************************************************************/
  /* char name[6]; length of name set up pointer to start of vector   */
  /********************************************************************/

  ptr_temp = vect_addr;
  ptr_begin = vect_addr;

  /********************************************************************/
  /* setup pointer to name value                                      */
  /********************************************************************/

  name_ptr = name_addr;

  /********************************************************************/
  /* move past vector header                                          */
  /********************************************************************/

  ptr_begin = ptr_begin+SIZE_VECTOR_HEADER;

  /********************************************************************/
  /* build nsa id vector                                              */
  /********************************************************************/

    {

      /****************************************************************/
      /* set vector type to nsa id vector                             */
      /****************************************************************/

      p->nsa_vector.key = NSA_KEY;

      /****************************************************************/
      /* set length of nsa id vector                                  */
      /****************************************************************/

      p->nsa_vector.length = SIZE_NSA_VECTOR;

      /****************************************************************/
      /* set CORRELATOR value                                         */
      /****************************************************************/

      p->nsa_vector.value = LOCAL_NSA_ID;
    } 

  /********************************************************************/
  /* move past nsa id vector                                          */
  /********************************************************************/

  bcopy(&p->nsa_vector, ptr_begin, p->nsa_vector.length);
  ptr_begin = ptr_begin+p->nsa_vector.length;

  /********************************************************************/
  /* build ORIGIN or SEARCH id vector                                 */
  /********************************************************************/

    {

      /****************************************************************/
      /* set vector type                                              */
      /****************************************************************/

      p->object_vector.key = OBJECT_KEY;

      /****************************************************************/
      /* set length of SEARCH or ORIGIN id vector                     */
      /****************************************************************/

      p->object_vector.length = SIZE_OBJECT_VECTOR+name_length;

      /****************************************************************/
      /* set name value                                               */
      /****************************************************************/

      bcopy(name_ptr, p->object_vector.value, name_length);
    } 

  /********************************************************************/
  /* move past id vector                                              */
  /********************************************************************/

  bcopy(&p->object_vector, ptr_begin, p->object_vector.length);
  ptr_begin = ptr_begin+p->object_vector.length;

  /********************************************************************/
  /* set type of vector to build                                      */
  /********************************************************************/

  p->vector_header.key = vect_type;

  /********************************************************************/
  /* set overall length of vector                                     */
  /********************************************************************/

  p->vector_header.length = ptr_begin-ptr_temp;
  bcopy(&p->vector_header, ptr_temp, SIZE_VECTOR_HEADER);
  return (p->vector_header.length);
}                                      /* end build_id;               */
new_call_response(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*
 * module name:  new_call_response
 *
 * descriptive name:  new call response handler.
 *
 * function:  checks the validity of the "found" response received to
 *            a local "find" request, and notifies the link
 *            station of call completion.
 *
 * input:  the "found" response packet for 0xFC or any non-discovery
 *         packet received for an in-use/calling station.
 *
 * output:  the associated link station is notified of "call
 *          completion"
 *
                                                                      */
/*** end of specifications ********************************************/

{
#define  NEG_MASK 0x80
  int      ret1 = DLC_OK;
  int      i;
  u_char   *p_to_name;                 /* pointer to name in cache    */

  TRACE1(p, "NCRo");
 
  if                                   /* link trace enabled.         */
     ((p->sta_ptr->ls_profile.flags&DLC_TRCO) == DLC_TRCO)
    {

      /****************************************************************/
      /* call the receive link trace routine.                         */
      /****************************************************************/

      lanrcvtr(p);
    } 
  p->sap_ptr = (struct sap_cb *)p->sap_list[p->sapno].sap_cb_addr;
 
  if (p->rcv_data.lsap == DISCOVERY_SAP)
    {
 
      if                               /* the response code indicates 
                                          a positive response         */
         ((p->response_vector.value&NEG_MASK) == 0)
        {

          /************************************************************/
          /* save the remote's sap in the sap control block.          */
          /************************************************************/

          p->sap_ptr->incomming_rsap = ((p->lsap_vector.value)
             &RESP_OFF);
 
          if                           /* remote name not in cache    */
             (p->sta_ptr->sta_cache == CACHE_NO_NAME)
            {
 
              if                       /* space available in cache    */
                 (p_to_cache->n_entries < 100)
                {

                  /****************************************************/
                  /* save future index of name to be added to cache in*/
                  /* names array                                      */
                  /****************************************************/

                  p_to_cache->name_index[p_to_cache->n_entries] = 
                     (u_char)p_to_cache->n_entries;

                  /****************************************************/
                  /* update count of entries in cache                 */
                  /****************************************************/

                  p_to_cache->n_entries++;

                  /****************************************************/
                  /* clear name entry in cache                        */
                  /****************************************************/

                  p_to_name = (char *)p_to_cache->cache_data
                     [p_to_cache->n_entries-1].name;
 
                  for (i = 0; i < 20; i++)
                    {
                      *p_to_name = 0;
                      ++p_to_name;
                    } 

                  /****************************************************/
                  /* setup entry into cache copy remote name into     */
                  /* cache                                            */
                  /****************************************************/

                  bcopy(p->sta_ptr->ls_profile.raddr_name, 
                     p_to_cache->cache_data[p_to_cache->n_entries-1].
                     name, p->sta_ptr->ls_profile.len_raddr_name);

                  /****************************************************/
                  /* copy remote address into cache                   */
                  /****************************************************/

                  bcopy(p->rcv_data.raddr, p_to_cache->cache_data
                     [p_to_cache->n_entries-1].address, 6);

                  /****************************************************/
                  /* set name and address lengths in cache            */
                  /****************************************************/

                  p_to_cache->cache_data[p_to_cache->n_entries-1].
                     len_name = p->sta_ptr->ls_profile.len_raddr_name;
                  p_to_cache->cache_data[p_to_cache->n_entries-1].
                     len_address = 6;

                  /****************************************************/
                  /* call sort to sort name cache                     */
                  /****************************************************/

                  sort_cache(p_to_cache->name_index, 0, 
                     p_to_cache->n_entries-1, p_to_cache->cache_data);
                } 
            } 
 
          else
            {
 
              if                       /* name in cache has wrong     
                                          address                     */
                 (p->sta_ptr->sta_cache == CACHE_WRONG_NAME)
                {

                  /****************************************************/
                  /* use index saved in station control block to get  */
                  /* access and replace address in cache              */
                  /****************************************************/

                  bcopy(p->rcv_data.raddr, p_to_cache->cache_data
                     [p_to_cache->name_index[p->sta_ptr->cache_pindex]
                     ].address, 6);

                  /****************************************************/
                  /* reset cache flag                                 */
                  /****************************************************/

                  p->sta_ptr->sta_cache = CACHE_NO_NAME;
                  p->sta_ptr->cache_pindex = 0;
                } 
            } 
        } 
 
      else                             /* it's a negative response -  
                                          remote busy                 */
        {
          ret1 = NO_MATCH;

          /************************************************************/
          /* reset the call pending indicator in the station list and */
          /* the receive station table.                               */
          /************************************************************/

          p->station_list[p->stano].call_pend = FALSE;
          p->common_cb.hashno = p->station_list[p->stano].sta_hash;

          /************************************************************/
          /* call error log - negative response to call               */
          /************************************************************/

          lanerrlg(p, ERRID_LAN0027, NON_ALERT, PERM_STA_ERR, 
             DLC_RBUSY, FILEN, LINEN);

          /************************************************************/
          /* call the link station with command = close.              */
          /************************************************************/

          lansta(p, CLOSENO);
        } 
    } 
 
  if                                   /* discovery response is ok, or
                                          not discovery               */
     (ret1 == DLC_OK)
    {

      /****************************************************************/
      /* reset call pending flag in station list                      */
      /****************************************************************/

      p->station_list[p->stano].call_pend = FALSE;

      /****************************************************************/
      /* disable the station's t1 repoll timer.                       */
      /****************************************************************/

      p->station_list[p->stano].t1_ctr = -1;
      p->station_list[p->stano].t1_ena = FALSE;

      /****************************************************************/
      /* build hashing string for the remote station address, the     */
      /* local sap, and the remote sap values.                        */
      /****************************************************************/

      bcopy(p->rcv_data.raddr, p->common_cb.u_h.s_h.hash_string_raddr,
         sizeof(p->rcv_data.raddr));
      p->common_cb.u_h.s_h.hash_string_rsap = p->sap_ptr->incomming_rsap;
      p->common_cb.u_h.s_h.hash_string_lsap = p->sap_ptr->sap_profile.local_sap;

      /****************************************************************/
      /* call the "add station to receive hash table" routine, with   */
      /* the hashing string and stationno.                            */
      /****************************************************************/

      add_sta_to_hash(p);

/* LEHb defect 44499 */
#ifdef   TRLORFDDI
      /************************************************************/
      /* call set station route routine to save any bridge        */
      /* routing that was received in the found packet            */
      /************************************************************/

      set_sta_route (p);

#endif /* TRLORFDDI */
/* LEHe */
      /****************************************************************/
      /* call the link station with command =call completion.         */
      /****************************************************************/

      lansta(p, CALL_CMPLNO);
    } 
}                                      /* end new_call_response;      */
