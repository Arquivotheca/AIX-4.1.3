static char sccsid[] = "@(#)59	1.29  src/bos/kernext/dlc/qlc/qlclasyn.c, sysxdlcq, bos411, 9437B411a 9/14/94 11:29:46";
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: qlm_station_started, qlm_station_halted, qlm_station_reset,
 *            qlm_incoming_call, qlm_incoming_call_rejected, qlm_receive_data,
 *            qlm_invalid_packet_rx, qlm_receive_qllu,
 *            qlm_receive_inactivity_handler, qlm_write_error,
 *            qlm_make_netd_buffer, qlm_station_remote_discontact
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

#include "qlcg.h"   
#include "qlcq.h"  
#include "qlcqmisc.h"
#include "qlcv.h"  
#include "qlcvfac.h"  
#include "qlcb.h"  
#include "qlcp.h"
#include "qlcc.h"  
#include "qlcs.h"  
#include "qlcl.h"
#include "qlclasyn.h"
#include "qlclutil.h"
#include "qlcltime.h"
#include "qlcctrc.h"

extern channel_list_type channel_list;

/*****************************************************************************/
/* Function     qlm_station_started                                          */
/*                                                                           */
/* Description  This procedure gets called by the Interrupt Handler defined  */
/*              in the QPM. The QPM gets a START_DONE status block from the  */
/*              DH, which contains the following;                            */
/*                                                                           */
/*             code- type of status      (CIO_STATUS)                        */
/*             opt0- exception_code      (CIO_EXCEPT)                        */
/*             opt1- session_id                                              */
/*             opt2- netid                                                   */
/*             opt3- buffer_ptr  (iff session_type is svc_outgoing for start */
/*                                done status blocks).                       */
/*                                                                           */
/*             This procedure is passed the opt0-3   as separate parameters, */
/*             thereby using the QPM as a layer of isolation between the DH  */
/*             and the QLM, and limiting the effect of any changes to the DH */
/*             to just the QPM.                                              */
/*                                                                           */
/*             Note that Start commands are issued for:                      */
/*                  Listening stations                                       */
/*                    Just registered a listener                             */
/*                    Issue a STAS result to the user.                       */
/*                  Calling PVC stations                                     */
/*                    Good or bad result - issue STAS to user.               */
/*                  Calling SVC stations                                     */
/*                    Call Conn/Clear Ind (i.e. good/bad)->STAS result.      */
/*     Must now check the station state. If it is not OPENING, it means that */
/*     the User issued a Halt whilst QLLC was waiting for the start_done     */
/*     status block.                                                         */
/*                                                                           */
/*     Under these circumstances, the state was changed by the Halt_LS       */
/*     procedure. A Halt will have already been issued, and there is no      */
/*     guarantee that a start done will be received, but in this case it     */
/*     has. QLLC is not concerned with the result of the start, as it is     */
/*     going to close down the vc anyway.                                    */
/*     When the halt done status block comes back from the dh, it            */
/*     will generate a Halted result to the user, so no started result is    */
/*     issued from here. (No error is logged).                               */
/*                                                                           */
/*     If this is a start done in response to a calling start, there will be */
/*     a buffer attached to the start done, which will be a Clear Indication */
/*     or Call Connected.                                                    */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
void  qlm_station_started(
 
  unsigned long result,
  unsigned short netid,
  unsigned short session_id,
  gen_buffer_type *buffer_ptr)
{
  station_type         *station_ptr;
  station_type         *cont_ls;
  port_type            *port_id;
  x25_vc_type          *vc_ptr;
  qllc_ls_type         *ls_ptr;
  sap_type	       *sap_ptr;
  channel_type         *channel_id;
  correlator_type       qllc_sap_correlator;
  struct x25_halt_data  halt_data;
  lock_t                lock_list_rc;
  qpm_rc_type           qpm_rc = qpm_rc_ok;
  bool                  unlock;
  bool                  unlock_cont;
 
  /***************************************************************************/
  /* Area for analysis of facilities sent back with call_connected packet,   */
  /* if the call was accepted by the remote station:                         */
  /***************************************************************************/
  cb_fac_t         returned_facs;
  byte             fac_stream[X25_MAX_FACILITIES_LENGTH];
 
  /***************************************************************************/
  /* Find and lock the station to which this result applies.                 */
  /***************************************************************************/
  station_ptr = qlm_find_ls_given_netid(netid, &unlock);
  if (station_ptr == (station_type *)NULL)
  {
    outputf("QLM_STATION_STARTED: station with netid %d not found\n",netid);
    if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
    return;
  }
  /***************************************************************************/
  /* The station to which this result is associated exists, and is pointed   */
  /* to by station_ptr.                                                      */
  /* Set up local variables                                                  */
  /***************************************************************************/
  channel_id = station_ptr->channel_id;
  vc_ptr = &(station_ptr->virt_circuit);
  ls_ptr = &(station_ptr->link_station);
  port_id = QCM_RETURN_PORT_ID(channel_id);
  qllc_sap_correlator = station_ptr->qllc_sap_correlator;
  sap_ptr = (sap_type *) qllc_sap_correlator;
 
  /***************************************************************************/
  /* We need to have the sap lock as well since we may end up searching      */
  /* the station list below, and we have to drop the station lock to get     */
  /* the sap lock to avoid deadlock.                                         */
  /***************************************************************************/
  lock_list_rc = qlm_lock_list_and_ls(station_ptr);
  if (lock_list_rc == LOCK_FAIL)
  {
    outputf("QLM_STATION_STARTED: station with netid %d got deleted\n", netid);
    if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
    return;
  }

  /***************************************************************************/
  /* Find out if this is a listener who is just getting its first start done */
  /* and will be waiting for an incoming call, or whether it is a listener   */
  /* which has already accepted an incoming call, and is about to go fully   */
  /* opened.                                                                 */
  /***************************************************************************/
  if (station_ptr->listen_accepted_pdg_start_done == TRUE)
  {
    /*************************************************************************/
    /* This is a listener which has already had a call and has accepted it.  */
    /* This is the Start Done from the Call Accept Start. Note that the      */
    /* session_id does not have to match that found in the station's virtual */
    /* circuit.                                                              */
    /* Remember what the new session_id is.                                  */
    /*************************************************************************/
    outputf("QLM_STATION_STARTED: was a listener, update session_id\n");
 
    /*************************************************************************/
    /* Before updating the session_id, you must halt the original device     */
    /* handler session, and when the halt done comes in you must handle that.*/
    /* This is because the DH spawns a new session to pass the call up on,   */
    /* so the listening session still exists. However, as QLLC we do not     */
    /* want to keep the listener open.                                       */
    /*************************************************************************/
    qpm_make_halt_data(&halt_data,netid,station_ptr->virt_circuit.session_id);
    qpm_rc = qpm_halt(port_id, &halt_data, NULL);
 
    /************************************************************************/
    /* Now update the session_id                                            */
    /************************************************************************/
    station_ptr->virt_circuit.session_id = session_id;
 
    if ((result != CIO_OK) || (qpm_rc != qpm_rc_ok))
    {
      qvm_close_vc(&(station_ptr->virt_circuit),
	QCM_RETURN_PORT_ID(station_ptr->channel_id),
	link_station_unusual_network_condition,
	FALSE);
      if (unlock) unlockl(&(station_ptr->lock));
      if (lock_list_rc != LOCK_NEST) unlockl(&channel_list.lock);
      return;
    }
    else
    {
      /***********************************************************************/
      /* Look for link station contention                                    */
      /***********************************************************************/
/* defect 156503 */
      cont_ls = qlm_find_contending_ls(sap_ptr, station_ptr, NULL,
                                       &unlock_cont);
/* end defect 156503 */
      if (cont_ls != NULL)
      {
	if (unlock_cont) unlockl(&cont_ls->lock);
	outputf("QLM_STATION_STARTED: contention\n");
	/*********************************************************************/
	/* A contending station has been found. The station with the higher  */
	/* address clears                                                    */
	/* may want a better length ?? max. */
	/*********************************************************************/
	if (strncmp(QPM_RETURN_LOCAL_ADDRESS(port_id),
	  station_ptr->remote_addr,
	  sizeof(station_ptr->remote_addr)
	  ) > 0)
	{
	  /*******************************************************************/
	  /* local station must clear                                        */
	  /*******************************************************************/
	  outputf("QLM_STATION_STARTED: local station clearing\n");
	  qvm_close_vc (vc_ptr,
	    port_id,
	    LOCAL_PROCEDURE_ERROR_GENERAL,
	    FALSE);
	  station_ptr->station_state = (byte)closing;
	  station_ptr->reason_for_closure = (int)remote_name_already_connected;
	}
	/*********************************************************************/
	/* Otherwise remote station will clear, so do nothing at this end.   */
	/*********************************************************************/
	if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
	if (unlock) unlockl(&(station_ptr->lock));
        if (lock_list_rc != LOCK_NEST) unlockl(&channel_list.lock);
	return;
      }
      /***********************************************************************/
      /* The call accept was successful, and you now have a fully established*/
      /* session.                                                            */
      /* Reset the pending call accept flag, and drop through to the VC and  */
      /* FSM checking, and result generation at the end of the function.     */
      /***********************************************************************/
      station_ptr->listen_accepted_pdg_start_done = FALSE;
    }
  }
  else 
  {
    outputf("QLM_STATION_STARTED: station_flags = %x\n",station_ptr->flags);
    /*************************************************************************/
    /* Either a basic listener or a caller. Either way the session id should */
    /* match that in the virtual circuit.                                    */
    /* Check the session id is OK. This ensures the dh isn't still running a */
    /* session relating to an old link station whose netid has been used     */
    /* again                                                                 */
    /*************************************************************************/
    if (station_ptr->virt_circuit.session_id != session_id)
    {
      outputf("QLM_STATION_STARTED: station found, but session_id wrong\n");
      outputf("QLM_STATION_STARTED: session_id in stn =%d\n",
	station_ptr->virt_circuit.session_id);
      outputf("QLM_STATION_STARTED: session_id from xdh =%d\n",session_id);
      if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
      qlm_delete_station(station_ptr,NULL,NULL);
      if (lock_list_rc != LOCK_NEST) unlockl(&channel_list.lock);
      return;
    }
    else
    {
      /***********************************************************************/
      /* The basic listener/caller has the correct session_id.               */
      /***********************************************************************/
      if (station_ptr->station_state != (byte)opening)
      {
	outputf("QLM_STATION_STARTED: state != opening\n");
	if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
	/*********************************************************************/
	/* Halt has already been issued, so just wait for the Halt Done      */
	/*********************************************************************/
	if (unlock) unlockl(&(station_ptr->lock));
        if (lock_list_rc != LOCK_NEST) unlockl(&channel_list.lock);
	return;
      }
      /***********************************************************************/
      /* The station state is opening, so the result must be checked.        */
      /***********************************************************************/
      /***********************************************************************/
      /* If the result indicates that there has been a local error then send */
      /* a Station Started result to the user with diagnostic                */
      /* Link_Station_Unusual_Network_Condition, and issue a silent halt     */
      /* using QVM_Close_VC and setting the silent field.                    */
      /* Don't delete the station until the Halt Done is received.           */
      /* Note that for LOCAL ERRORS there is no buffer.                      */
      /***********************************************************************/
      else if (result != CIO_OK)
      {
	outputf("QLM_STATION_STARTED: local error occurred\n");
	qcm_make_result (
	  channel_id,
	  QSM_RETURN_USER_SAP_CORRELATOR(qllc_sap_correlator),
	  station_ptr->user_ls_correlator,
	  station_started,
	  (int)link_station_unusual_network_condition
	  );
	station_ptr->silent_halt = TRUE;
	outputf("QLM_STATION_STARTED: calling qvm_close_vc\n");
	qvm_close_vc(vc_ptr,
	  port_id,
	  link_station_unusual_network_condition,
	  FALSE);
	if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
	if (unlock) unlockl(&(station_ptr->lock));
        if (lock_list_rc != LOCK_NEST) unlockl(&channel_list.lock);
	return;
      }
      else if ((station_ptr->flags & DLC_SLS_LSVC) == FALSE)
      {
	outputf("QLM_STATION_STARTED: listening station waiting for call\n");
	/*********************************************************************/
	/* Don't do anything. You need to wait for an incoming call before   */
	/* sending a STAS result to the user.                                */
	/*********************************************************************/
	if (unlock) unlockl(&(station_ptr->lock));
        if (lock_list_rc != LOCK_NEST) unlockl(&channel_list.lock);
	return;
      }
      else
      {
	/*********************************************************************/
	/* This station is a caller. The start done contains the result of   */
	/* the call which the dh has made.                                   */
	/*********************************************************************/
	outputf("QLM_STATION_STARTED: station is a caller\n");
	/*********************************************************************/
	/* This station could apply to either a PVC or an SVC.               */
	/*********************************************************************/
	if (station_ptr->virt_circuit.circuit == session_pvc)
	{
	  /*******************************************************************/
	  /* Virtual Circuit is a PVC                                        */
	  /*******************************************************************/
	  ;
	}
	else
	{
	  /*******************************************************************/
	  /* Virtual Circuit is an SVC                                       */
	  /*******************************************************************/
	  /*******************************************************************/
	  /* The only valid buffers which may accompany a Start Done         */
	  /* are Clear Indication or a Call Connected.                       */
	  /*******************************************************************/
/* defect 149350 - check buffer_ptr first */
	  if (( buffer_ptr != NULL )
	    && (QBM_RETURN_PACKET_TYPE(buffer_ptr)!=PKT_CLEAR_IND && 
	        QBM_RETURN_PACKET_TYPE(buffer_ptr)!=PKT_CALL_CONNECTED))
/* end defect 149350 */
	  {
	    outputf("QLM_STATION_STARTED: Packet type is invalid. (= %d)\n",
	      QBM_RETURN_PACKET_TYPE(buffer_ptr));
	    QBM_FREE_BUFFER(buffer_ptr);
	    if (unlock) unlockl(&(station_ptr->lock));
            if (lock_list_rc != LOCK_NEST) unlockl(&channel_list.lock);
	    return;
	  }
	  else
	  {
	    /*****************************************************************/
	    /* The station must determine whether the call                   */
	    /* was successful or not.                                        */
	    /*                                                               */
	    /* If the call was successful, then the station will be started. */
	    /*                                                               */
	    /* There are two types of problem that could have occurred.      */
	    /*  1. the call encountered a local problem, which will be       */
	    /*     indicated by the result variable.                         */
	    /*                                                               */
	    /* 2.  the call was sent out successfully, but there was some    */
	    /*     remotely detected error (eg. bad called address), which   */
	    /*     would be indicated by a Clear Indication.                 */
	    /*     A Call Connected indicates that there was no error.       */
	    /*****************************************************************/
/* defect 149350 - check buffer_ptr first */
	    if (( buffer_ptr != NULL )
	      && QBM_RETURN_PACKET_TYPE(buffer_ptr) == PKT_CLEAR_IND
	      && QVM_RETURN_SESSION_TYPE(vc_ptr) == session_svc_out)
/* end defect 149350 */
	    {
	      outputf("QLM_STATION_STARTED: received Clear Ind on o/g SVC\n");
	      /***************************************************************/
	      /* The call was not accepted, because of some remote error.    */
	      /* The buffer contains a clear indication packet               */
	      /* Have received a Clear Ind in response to a Call Request.    */
	      /* Send a Clear Confirm, so set remote_clear parm to TRUE.     */
	      /* Issue a Halt to the DH, and wait for the Halt Done.         */
	      /***************************************************************/
	      QLLC_DIAGNOSTIC(
		(QBM_RETURN_CAUSE(buffer_ptr)),
		(QBM_RETURN_DIAGNOSTIC(buffer_ptr)),
		(station_ptr->qllc_ls_correlator)
		);
	      qcm_make_result (
		channel_id,
		QSM_RETURN_USER_SAP_CORRELATOR(qllc_sap_correlator),
		station_ptr->user_ls_correlator,
		station_started,
		(int)DLC_NO_FIND
		);
	      station_ptr->silent_halt = TRUE;
	      outputf("QLM_STATION_STARTED: calling qvm_close_vc\n");
	      qvm_close_vc(vc_ptr,port_id,DLC_NO_FIND,TRUE);
	      if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
	      if (unlock) unlockl(&(station_ptr->lock));
              if (lock_list_rc != LOCK_NEST) unlockl(&channel_list.lock);
	      return;
	    }
	    else
	    {
	      /***************************************************************/
	      /* Otherwise, buffer must contain a CALL CONNECTED packet.     */
	      /* If there was no remote error then the call has been         */
	      /* successful                                                  */
	      /***************************************************************/
	      outputf("QLM_STATION_STARTED: call request was successful\n");
 
	      /***************************************************************/
	      /* Check the facilities                                        */
	      /***************************************************************/
 
	      /***************************************************************/
	      /* Get the facilities parser to build a new cb_fac structure,  */
	      /* so that you can check the facilities that have been returned*/
	      /* on the call_connected packet.                               */
	      /***************************************************************/
	      /***************************************************************/
	      /* First get the byte stream out the buffer using read macro   */
	      /* Read it into the byte array given by fac_stream.            */
	      /***************************************************************/
	      JSMBUF_READ_BLOCK(
		buffer_ptr,
		X25_OFFSETOF_FAC_CUD_DATA,
		fac_stream,
		(unsigned)QBM_RETURN_FACILITIES_LENGTH(buffer_ptr)
	      );
	      /***************************************************************/
	      /* Now convert the fac_stream array into a cb_fac struture in  */
	      /* space returned_facs                                         */
	      /***************************************************************/
	      bzero(fac_stream,X25_MAX_FACILITIES_LENGTH);
	      
	      _x25_convert_byte_stream_to_cb_fac(
		&(returned_facs),
		NULL,
		fac_stream,
		(unsigned)QBM_RETURN_FACILITIES_LENGTH(buffer_ptr)
		);
	      
	      /***************************************************************/
	      /* Check facilities are OK.                                    */
	      /* Update station if necessary, e.g. if packet size has been   */
	      /* reduced remote station. Same for window sizes.              */
	      /***************************************************************/
	      if (returned_facs.flags & X25FLG_PSIZ)
	      {
		if (station_ptr->facilities.psiz_clg > returned_facs.psiz_clg)
		  station_ptr->facilities.psiz_clg = returned_facs.psiz_clg; 
		if (station_ptr->facilities.psiz_cld > returned_facs.psiz_cld)
		  station_ptr->facilities.psiz_cld = returned_facs.psiz_cld; 
	      }
	      if (returned_facs.flags & X25FLG_WSIZ)
	      {
		if (station_ptr->facilities.wsiz_clg > returned_facs.wsiz_clg)
		  station_ptr->facilities.wsiz_clg = returned_facs.wsiz_clg; 
		if (station_ptr->facilities.wsiz_cld > returned_facs.wsiz_cld)
		  station_ptr->facilities.wsiz_cld = returned_facs.wsiz_cld; 
	      }
	    }
	  }
	}	    
 
	/*****************************************************************/
	/* Regardless of whether the circuit is a pvc or an svc, look    */
	/* for link station contention                                   */
        /*****************************************************************/
/* defect 156503 */
        cont_ls = qlm_find_contending_ls(sap_ptr, station_ptr, NULL,
                                         &unlock_cont);
/* end defect 156503 */
        if (cont_ls != NULL)
	{
	  if (unlock_cont) unlockl(&cont_ls->lock);
	  outputf("QLM_STATION_STARTED: contention\n");
	  /***************************************************************/
	  /* A contending station has been found.                        */
	  /* The station with the higher address clears                  */
	  /* may want a better length ?? max.                            */
	  /***************************************************************/
	  if (strncmp(
	    QPM_RETURN_LOCAL_ADDRESS(port_id),
	    station_ptr->remote_addr,
	    sizeof(station_ptr->remote_addr)
	    ) > 0)
	  {
	    /*************************************************************/
	    /* local station must clear                                  */
	    /*************************************************************/
	    outputf("QLM_STATION_STARTED: local station clearing\n");
	    qvm_close_vc (vc_ptr,
	      port_id,
	      LOCAL_PROCEDURE_ERROR_GENERAL,
	      FALSE);
	    station_ptr->station_state = (byte)closing;
	    station_ptr->reason_for_closure =
	      (int)remote_name_already_connected;
	  }
	  /***************************************************************/
	  /* Otherwise remote station will clear, so do nothing this end */
	  /***************************************************************/
	  if (buffer_ptr != NULL)
	    QBM_FREE_BUFFER(buffer_ptr);
	  if (unlock) unlockl(&(station_ptr->lock));
          if (lock_list_rc != LOCK_NEST) unlockl(&channel_list.lock);
	  return;
	}
	/*****************************************************************/
	/* There is no contention.                                       */
	/*****************************************************************/
	outputf("QLM_STATION_STARTED: no contention\n"); 
      }
    }
  }
 
  /**********************************************************************/
  /* No more searching for stations to be done, unlock SAP now.         */
  /**********************************************************************/
  if (lock_list_rc != LOCK_NEST) unlockl(&channel_list.lock);

  /***************************************************************************/
  /* Call QVM_VC_Opened() to register fact that circuit is now opened.       */
  /***************************************************************************/
  outputf("QLM_STATION_STARTED: calling qvm_vc_opened\n");
  if (qvm_vc_opened (vc_ptr,buffer_ptr) != qvm_rc_ok)     
  {
    outputf("QLM_STATION_STARTED: qvm_vc_opened failed, force close\n");
    qvm_close_vc(vc_ptr,port_id,LOCAL_PROCEDURE_ERROR_GENERAL,FALSE);
    station_ptr->station_state = (byte)closing;
    station_ptr->reason_for_closure = sna_system_error;
    /*************************************************************************/
    /* send result to user                                                   */
    /*************************************************************************/
    qcm_make_result (
      channel_id,
      QSM_RETURN_USER_SAP_CORRELATOR(station_ptr->qllc_sap_correlator),
      station_ptr->user_ls_correlator,
      station_started,
      (int)link_station_unusual_network_condition
      );
    if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
    if (unlock) unlockl(&(station_ptr->lock));
    return;
  }
  /***************************************************************************/
  /* qvm_vc_opened worked ok                                                 */
  /***************************************************************************/
  outputf("QLM_STATION_STARTED: qvm_vc_opened ok calling fsm l3rdy\n");
  if (qllc_l3rdy(&(station_ptr->link_station)) != qrc_ok)
  {
    /*************************************************************************/
    /* qfsm_l3rdy failed                                                     */
    /*************************************************************************/
    qvm_close_vc(vc_ptr, port_id, LOCAL_PROCEDURE_ERROR_GENERAL,FALSE);
    station_ptr->station_state = (byte)closing;
    station_ptr->reason_for_closure = protocol_error;
    qcm_make_result (
      channel_id,
      QSM_RETURN_USER_SAP_CORRELATOR(station_ptr->qllc_sap_correlator),
      station_ptr->user_ls_correlator,
      station_halted,
      (int)link_station_unusual_network_condition
      );
    if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
    if (unlock) unlockl(&(station_ptr->lock));
    return;
  }
  /***************************************************************************/
  /* qfsm worked                                                             */
  /* station can be converted into opened state                              */
  /***************************************************************************/
  outputf("QLM_STATION_STARTED: convert station into opened state\n");
  if (QVM_VC_IS_PVC(vc_ptr) == TRUE )
  {
    /*************************************************************************/
    /* There are two cases to consider. One is that the PVC which has just   */
    /* been started was responsible for bringing the link up. If this is the */
    /* case, then a reset will have been sent and a reset confirmation will  */
    /* be received, which will trigger the transfer into started state.      */
    /* The second case is that the link was already up. It is then           */
    /* considered necessary to send a reset with cause and diag of 0x00, in  */
    /* order that a confirmation will be received which will trigger the     */
    /* state change.                                                         */
    /* This is implemented by always sending a Reset from here, which means  */
    /* that in the first case, there will be two resets. It is not           */
    /* anticipated that this will be a problem, but this will need to be     */
    /* reviewed after NPSI testing, which has unusual requirements regarding */
    /* resets.                                                               */
    /*************************************************************************/
    outputf("QLM_STATION_STARTED: send out Reset Indication\n");
    qvm_clrst(vc_ptr,port_id,0x00);              
  }
  /***************************************************************************/
  /* Regardless of whether the circuit is a PVC or an SVC, stimulate the     */
  /* station if secondary, and start the inactivity timer.                   */
  /***************************************************************************/
  if (QLLC_LINK_STATION_IS_PRIMARY(ls_ptr) == FALSE)
  {
    outputf("QLM_STATION_STARTED: secondary station\n");
    /*************************************************************************/
    /* The station is secondary.                                             */
    /* Secondary stations need a higher layer stimulus before they can be    */
    /* contacted. This is not provided by the user, so it is provided        */
    /* implicitly whenever a secondary station becomes operational.          */
    /*************************************************************************/
    outputf("QLM_STATION_STARTED: stimulate fsm\n");
    (void)qllc_lstrt(ls_ptr,vc_ptr,port_id);
    /*************************************************************************/
    /* We must also start monitoring for inactivity as soon as a secondary   */
    /* is opened. Start timer.                                               */
    /*************************************************************************/
    outputf("QLM_STATION_STARTED: start inactivity timer\n");
    w_start(&(station_ptr->inact_dog));
  }
  /***************************************************************************/
  /* For either PRIMARY or SECONDARY stations, the following applies.        */
  /***************************************************************************/
 
  /***************************************************************************/
  /* Only put non-PVC stations into opened state, as PVC stations will go    */
  /* into opened state on receipt of reset confirm from reset indicate just  */
  /* sent.                                                                   */
  /***************************************************************************/
  if (QVM_VC_IS_PVC(vc_ptr) == FALSE)
  {
    outputf("QLM_STATION_STARTED: SVC state becomes OPENED\n");
    station_ptr->station_state = (byte)opened;
    outputf("QLM_STATION_STARTED: station opened,send result\n");
    qcm_make_stas_result(
      channel_id,
      QSM_RETURN_USER_SAP_CORRELATOR(station_ptr->qllc_sap_correlator),
      station_ptr->user_ls_correlator,
      station_ptr->max_i_field,
      station_ptr->remote_addr_len,
      station_ptr->remote_addr
      );
    outputf("CALLING TRACE_START MACRO\n");
    TRACE_START(station_ptr);
  }
  /***************************************************************************/
  /* No matter what the result of any of the previous if's, free the buffer  */
  /***************************************************************************/
  if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
  if (unlock) unlockl(&(station_ptr->lock));
  return;
}


/*****************************************************************************/
/* Function     qlm_station_halted                                           */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* This procedure is called when a Halt Done status block is                 */
/* received from the DH, or when an unsolicited Clear Indication is received */
/* by the DH.                                                                */
/*                                                                           */
/* A Halt Done status block can arrive for a number of different reasons.    */
/*                                                                           */
/* 1) One is that this is the response to a locally initiated clear.         */
/* 2) It could also be a response to the removal of a listener.              */
/* 3) It can also be a response to a Halt command issued when a remote       */
/*    station has cleared, so that a clear confirm was sent.                 */
/* 4) It can also be because a Halt was issued on a PVC.                     */
/*                                                                           */
/* On only one of the above (the local clear of an SVC) is a clear           */
/* request packet issued, and that is the only case when the buffer          */
/* ptr passed to this function points to a valid buffer. The buffer          */
/* contains a Clear Confirm, which may contain useful info, depending        */
/* on the facilities that were requested on this call. If not a Clear        */
/* Confirm, then it may be a Clear Inidication, which could result           */
/* for a clear collision taking place. In this case, the same clear          */
/* information is present.                                                   */
/*                                                                           */
/* The remote_cleared parameter indicates whether this function has          */
/* been called as a result of the Interrupt Handler receiving a Halt         */
/* Done status block (remote_cleared == FALSE) or whether it                 */
/* received an unsolicited Clear_Indication from the remote node             */
/* (i.e. remote_cleared == TRUE).                                            */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*****************************************************************************/
void qlm_station_halted(

  unsigned long result,
  unsigned short netid,
  unsigned short session_id,
  gen_buffer_type *buffer_ptr)
{
  station_type *station_ptr;
  sap_type     *sap_ptr;
  lock_t        lock_list_rc;
  port_type *port_id;
  x25_vc_type *vc_ptr;
  qllc_ls_type *ls_ptr;
  channel_type *channel_id;
  station_type *contending_station_ptr;
  bool          unlock;
  bool          unlock_cont;

  outputf("QLM_STATION_HALTED: called\n");
  /***************************************************************************/
  /* Find and lock the station to which this result applies.                 */
  /***************************************************************************/
  station_ptr = qlm_find_ls_given_netid(netid, &unlock);
  if (station_ptr == NULL)
  {
    outputf("QLM_STATION_HALTED: buffer ptr nulled %d \n", netid);
    if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
    return;
  }
 
  /***************************************************************************/
  /* We need to have the sap lock as well since we may end up searching      */
  /* the station list below, and we have to drop the station lock to get     */
  /* the sap lock to avoid deadlock.                                         */
  /***************************************************************************/
  sap_ptr = (sap_type *) station_ptr->qllc_sap_correlator;
  lock_list_rc = qlm_lock_list_and_ls(station_ptr);
  if (lock_list_rc == LOCK_FAIL)
  {
    outputf("QLM_STATION_HALTED: station with netid %d got deleted\n", netid);
    if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
    return;
  }

  /***************************************************************************/
  /* Check the session id is OK. This ensures the dh isn't still running a   */
  /* session relating to an old link station whose netid has been used again */
  /***************************************************************************/
  if (station_ptr->virt_circuit.session_id != session_id)
  {
    /************************************************************************/
    /* You don't want to delete the station in the case of a mismatched     */
    /* session_id if the session type is SESSION_SVC_IN, because this is    */
    /* the halt_done from the call_accepted halt of the original session    */
    /* set up as the listener. When the halt done relating to the eventual  */
    /* closure of the station is received, the session_id should match OK.  */
    /************************************************************************/
    if (station_ptr->virt_circuit.circuit == session_svc_in)
    {
      outputf("QLM_STATION_HALTED: discard halt done for dh listen session\n");
      if (unlock) unlockl(&(station_ptr->lock));
      if (lock_list_rc != LOCK_NEST) unlockl(&channel_list.lock);
      return;
    }
    else
    {
      outputf("QLM_STATION_HALTED: session_id mismatch\n");
      qlm_delete_station(station_ptr,NULL,NULL);
      if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
      if (lock_list_rc != LOCK_NEST) unlockl(&channel_list.lock);
      return;
    }
  }
  /***************************************************************************/
  /* The station to which this result is associated exists, and is pointed   */
  /* to by station_ptr, and relates to the same session as the xdh does.     */
  /* Set up local variables                                                  */
  /***************************************************************************/
  channel_id = station_ptr->channel_id;
  vc_ptr = &(station_ptr->virt_circuit);
  ls_ptr = &(station_ptr->link_station);
  port_id = channel_id->port_id;

  outputf("QLM_STATION_HALTED: check result\n");
  /***************************************************************************/
  /* Whether the result indicates that the Halt was successful or not, QLLC  */
  /* will delete the station.                                                */
  /* If result indicates that halt was unsuccessful then log an error, and   */
  /* set the reason for closure to DLC_SYS_ERR. Otherwise use the reason     */
  /* given in the station.                                                   */
  /***************************************************************************/
  if (result != CIO_OK)
  {
    outputf("QLM_STATION_HALTED: result was not CIO_OK\n");
    station_ptr->reason_for_closure = DLC_SYS_ERR;
  }
  if (vc_ptr->remote_clear == TRUE)
  {
    /***********************************************************************/
    /* There will be no buffer to free.                                    */
    /***********************************************************************/
    qlm_delete_station(
      station_ptr,
      (correlator_type)NULL,
      (gen_buffer_type *)NULL); 
  }
  else
  {
    /*************************************************************************/
    /* The call was cleared locally. If the clear was due to link station    */
    /* contention, you need to tell delete_station() the correlator of the   */
    /* contending station. Otherwise pass NULL.                              */
    /*                                                                       */
    /* You will have to free the buffer that the XDH passed back in the      */
    /* status block.                                                         */
    /*************************************************************************/
    if (station_ptr->reason_for_closure == (int)remote_name_already_connected)
    {
      outputf("QLM_STATION_HALTED: local clear was due to contention\n");
      /***********************************************************************/
      /* find the conflicting correlator                                     */
      /***********************************************************************/
/* defect 156503 */
      contending_station_ptr = 
	qlm_find_contending_ls(sap_ptr, station_ptr, NULL, &unlock_cont);
/* end defect 156503 */
      if (contending_station_ptr != NULL && unlock_cont)
	unlockl(&contending_station_ptr->lock);

      station_ptr->reason_for_closure = (int)remote_name_already_connected;
      qlm_delete_station(
	station_ptr,
	(correlator_type)contending_station_ptr,
	buffer_ptr);
    }
    else
    {
      /***********************************************************************/
      /* The locally initiated clear was not as a result of a link station   */
      /* contention. You should use the reason for closure in the station,   */
      /* and pass a NULL correlator to delete station as "additional_info".  */
      /***********************************************************************/
      qlm_delete_station(
	station_ptr,
	(correlator_type)NULL,
	(gen_buffer_type *)buffer_ptr); 
    }
  }

  if (lock_list_rc != LOCK_NEST) unlockl(&channel_list.lock);
  return;
}

/*****************************************************************************/
/* Function     qlm_incoming_clear                                           */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* This procedure is called when a Clear Indication buffer is                */
/* received from the DH.                                                     */
/*                                                                           */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*****************************************************************************/
void qlm_incoming_clear(

  unsigned short netid,
  unsigned short session_id,
  gen_buffer_type *buffer_ptr)
{
  qvm_rc_type     qvm_rc;
  station_type   *station_ptr;
  port_type      *port_id;
  x25_vc_type    *vc_ptr;
  qllc_ls_type   *ls_ptr;
  channel_type   *channel_id;
  bool            unlock;
  q_cause_code_type  cause;

  outputf("QLM_INCOMING_CLEAR: called\n");
  /***************************************************************************/
  /* Find the station to which this result applies.                          */
  /***************************************************************************/
  station_ptr = qlm_find_ls_given_netid(netid, &unlock);
  if (station_ptr == NULL)
  {
    outputf("QLM_INCOMING_CLEAR: buffer_freed %d\n", netid);
    if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
    return;
  }
  /***************************************************************************/
  /* Check the session id is OK. This ensures the dh isn't still running a   */
  /* session relating to an old link station whose netid has been used again */
  /***************************************************************************/
  if (station_ptr->virt_circuit.session_id != session_id)
  {
    outputf("QLM_INCOMING_CLEAR: session_id mismatch\n");
    /* maybe delete station ?? */
    if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
    if (unlock) unlockl(&(station_ptr->lock));
    return;
  }
  /***************************************************************************/
  /* The station to which this result is associated exists, and is pointed   */
  /* to by station_ptr, and relates to the same session as the xdh does.     */
  /* Set up local variables                                                  */
  /***************************************************************************/
  channel_id = station_ptr->channel_id;
  vc_ptr = &(station_ptr->virt_circuit);
  ls_ptr = &(station_ptr->link_station);
  port_id = channel_id->port_id;

  /*
   * if station already closing, just forget this clear
   */
  if (station_ptr->station_state == closing)
  {
    if (unlock) unlockl(&(station_ptr->lock));
    return;
  }

  /***************************************************************************/
  /* Clear Indication has been received.                                     */
  /* Issue a Clear Confirm via the FSM, and QVM.                             */
  /***************************************************************************/
  outputf("QLM_INCOMING_CLEAR: call qllc_clrst\n");
  (void)qllc_clrst(
    ls_ptr,vc_ptr,port_id,FALSE,
    QBM_RETURN_CAUSE(buffer_ptr),
    QBM_RETURN_DIAGNOSTIC(buffer_ptr)
    );
  
  /***************************************************************************/
  /* Issue a Close to the QVM.                                               */
  /***************************************************************************/
  qvm_rc = qvm_close_vc(vc_ptr, port_id, NORMAL_TERMINATION,TRUE);
  if (qvm_rc != qvm_rc_ok)
  {
    station_ptr->reason_for_closure = (int)DLC_SYS_ERR;
    outputf("QLM_INCOMING_CLEAR: call delete station\n");
    qlm_delete_station(
      station_ptr,
      (correlator_type)NULL,
      buffer_ptr);
  }
  else
  {
/* defect 149350 */
   outputf("QLM_INCOMING_CLEAR: buffer_freed %d\n", netid);
   if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
/* end defect 149350 */

   cause =  QBM_RETURN_CAUSE(buffer_ptr);
   if ( cause == 9)
   {
     outputf("QLM_INCOMING_CLEAR: unusal network condition \n");
/* defect 159838 */
     station_ptr->reason_for_closure = link_station_unusual_network_condition;
/* end defect 159838 */
   }
   else
        station_ptr->reason_for_closure = remote_initiated_discontact;

    station_ptr->station_state = closing;
    if (unlock) unlockl(&(station_ptr->lock));
  }
  return;
}


/*****************************************************************************/
/* Function     qlm_station_reset                                            */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*****************************************************************************/
void qlm_station_reset(
 
  gen_buffer_type  *buffer_ptr,
  unsigned short    netid,
  unsigned short    session_id)
{
  x25_vc_type *vc_ptr;
  port_type *port_id;
  station_type *station_ptr;
  qllc_ls_type *ls_ptr;
  channel_id_type channel_id;
  bool unlock;
  q_cause_code_type  cause; 

  /***************************************************************************/
  /* This procedure gets called when a buffer is received by the port        */
  /* which contains a reset indication or a reset confirmation.              */
  /*                                                                         */
  /* If the vc is an SVC, on which any form of reset is not permitted by     */
  /* QLLC, then the vc is closed, an error is logged.                        */
  /*                                                                         */
  /* If the vc is a PVC, then we can expect to get resets.                   */
  /* However, there are certain conditions under which it is considered      */
  /* abnormal to get a reset confirmation or indication.                     */
  /* RESET INDICATION:                                                       */
  /*   confirm reset                                                         */
  /*   inform fsm that reset was received                                    */
  /*   stimulate if secondary                                                */
  /*   OPENING:                                                              */
  /*     turn off primary inact timer                                        */
  /*     stimulate if secondary                                              */
  /*     state = opened                                                      */
  /*     result, trace,                                                      */
  /*     start inact if secondary                                            */
  /*   NOT OPENING:                                                          */
  /*     CONTACTED:                                                          */
  /*       close vc                                                          */
  /*       state = closing                                                   */
  /*     NOT CONTACTED:                                                      */
  /*       do nothing. situation is recoverable.                             */
  /*                                                                         */
  /* RESET CONFIRMATION:                                                     */
  /*   free buffer                                                           */
  /*   OPENING:                                                              */
  /*     if primary turn off inact timer                                     */
  /*     secondary needs stumulus                                            */
  /*     state = opened                                                      */
  /*     result, trace,                                                      */
  /*     start inact timer if secondary                                      */
  /*   NOT OPENING:                                                          */
  /*     close vc                                                            */
  /*     state to closing                                                    */
  /*                                                                         */
  /* In short, if the PVC is in opening state, then either a reset           */
  /* indication or reset confirmation is considered ok.                      */
  /* If the PVC is not in the opening state then a reset indication is       */
  /* considered to be recoverable, so long as the station is not             */
  /* contacted.                                                              */
  /* If the PVC is not in opening state and a reset confirmation is          */
  /* received, then that is not allowed.                                     */
  /***************************************************************************/
 
  /***************************************************************************/
  /* First let's find the station the                                        */
  /* is intended for.                                                        */
  /***************************************************************************/
  station_ptr = qlm_find_ls_given_netid(netid, &unlock);
  if (station_ptr == NULL)
  {
    outputf("QLM_STATION_RESET: station with netid %d got reset\n", netid);
    /*************************************************************************/
    /* station to which this buffer should be routed cannot be found         */
    /*************************************************************************/
    QBM_FREE_BUFFER(buffer_ptr);
    return;
  }
 
  /***************************************************************************/
  /* Check the session id is OK. This ensures the dh isn't still running a   */
  /* session relating to an old link station whose netid has been used again */
  /***************************************************************************/
  if (station_ptr->virt_circuit.session_id != session_id)
  {
    outputf("QLM_STATION_RESET: station found, but session_id wrong\n");
    outputf("QLM_STATION_RESET: session_id in stn =%d\n",
      station_ptr->virt_circuit.session_id);
    outputf("QLM_STATION_RESET: session_id from xdh =%d\n",session_id);
    if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
    if (unlock) unlockl(&(station_ptr->lock));
    return;
  }
 
  /***************************************************************************/
  /* Initialise local variables                                              */
  /***************************************************************************/
  vc_ptr = &(station_ptr->virt_circuit);
  port_id = QCM_RETURN_PORT_ID(station_ptr->channel_id);
  ls_ptr = &(station_ptr->link_station);
  channel_id = station_ptr->channel_id;
 
  /***************************************************************************/
  /* Find out if this is an SVC or a PVC                                     */
  /***************************************************************************/
  if (QVM_VC_IS_PVC(vc_ptr) == FALSE)
  {
    /*************************************************************************/
    /* This is an SVC                                                        */
    /* In order to observe the correct protocol for clearing the vc, a reset */
    /* confirmation is sent immediately before the clear request is issued.  */
    /*************************************************************************/
    qvm_confirm_reset(vc_ptr,port_id,buffer_ptr);
    qvm_close_vc(vc_ptr,port_id,RESET_INDICATION_ON_VIRTUAL_CALL,FALSE);
    station_ptr->station_state = closing;
    station_ptr->reason_for_closure =
      (int)link_station_unusual_network_condition;
    if (unlock) unlockl(&(station_ptr->lock));
    return;
  }
  /***************************************************************************/
  /*                     This is a PVC.                                      */
  /***************************************************************************/
  /***************************************************************************/
  /* determine whether this is a reset indication                            */
  /* or a reset confirmation                                                 */
  /***************************************************************************/
  if (QBM_RETURN_PACKET_TYPE (buffer_ptr) == PKT_RESET_IND)
  {
    /*************************************************************************/
    /* PVC and RESET INDICATION                                              */
    /*                                                                       */
    /*************************************************************************/
    /* A Reset cause code of 1 indicate the phyical layer went away.         */
    /* Normally, we should sent back a qvm_confirm_reset but currently due   */
    /* to a bug in microcode this would cause the microcode to hang.         */
    /* Since we are going to issue a CIO_HALT anyway, so not to send a       */
    /* confirm_reset can avoid the hang                                      */
    cause =  QBM_RETURN_CAUSE(buffer_ptr);
    if ( cause != 1)
      {
	outputf("QLM_STATION_RESET: confirm sent\n");
        qvm_confirm_reset(vc_ptr,port_id,buffer_ptr);
      } 
    outputf("QLM_STATION_RESET: call qllc_clrst()\n");
    (void)qllc_clrst(
      ls_ptr,
      vc_ptr,
      port_id,
      TRUE,
      QBM_RETURN_CAUSE(buffer_ptr),
      QBM_RETURN_DIAGNOSTIC(buffer_ptr)
      );
    outputf("QLM_STATION_RESET: back from qllc_clrst()\n");
    if (station_ptr->station_state == opening)
    {
      outputf("QLM_STATION_RESET: the station is in opening state\n");
      if (QLLC_LINK_STATION_IS_PRIMARY(ls_ptr) == TRUE)
      {
	outputf("QLM_STATION_RESET: primary\n");
	/*********************************************************************/
	/* Timer shouldn't be running if primary, so superfluous             */
	/*********************************************************************/
	/* w_stop(&(station_ptr->inact_dog)); */
      }
      else /* (QLLC_LINK_STATION_IS_PRIMARY (ls_ptr) == FALSE)               */
      {
	outputf("QLM_STATION_RESET: secondary\n");
	/*********************************************************************/
	/* Station is a secondary and so needs a higher level                */
	/* stimulus to go contacted.                                         */
	/*********************************************************************/
	qllc_lstrt(ls_ptr, vc_ptr, port_id);
      }
      /***********************************************************************/
      /* Whether the station is primary or secondary, do the following.      */
      /***********************************************************************/
      outputf("QLM_STATION_RESET: set state to opened, and send result\n");
      station_ptr->station_state = opened;
      qcm_make_stas_result (
	channel_id,
	QSM_RETURN_USER_SAP_CORRELATOR(station_ptr->qllc_sap_correlator),
	station_ptr->user_ls_correlator,
	station_ptr->max_i_field,
	station_ptr->remote_addr_len,
	station_ptr->remote_addr
	);
      TRACE_START(station_ptr);
      if (QLLC_LINK_STATION_IS_PRIMARY (ls_ptr) == FALSE)
      {
	w_start(&(station_ptr->inact_dog));
      }
    }
    else
    {
      outputf("QLM_STATION_RESET: state not opening\n");
      /***********************************************************************/
      /* The station is not in opening state. The only way that we can       */
      /* process the reset indication is if the station has not yet gone     */
      /* contacted.                                                          */
      /***********************************************************************/
      if ((station_ptr->station_sub_state & DLC_CONTACTED) == DLC_CONTACTED)
      {
	outputf("QLM_STATION_RESET: station contacted\n");
       	if ( cause == 1)
      	   {
              outputf("QLM_STATION_RESET: sap_unusual_network_condition \n");
	      station_ptr->reason_for_closure = sap_unusual_network_condition;

      	   }
	qvm_close_vc(vc_ptr,port_id,NORMAL_TERMINATION,FALSE);
	station_ptr->station_state = closing;
      }
      else
      {
	outputf("QLM_STATION_RESET: station not contacted\n");
	/*********************************************************************/
	/* Do very little; the fact that the reset has been confirmed, and   */
	/* the fsm has been updated, means you only need to stimulate the    */
	/* station (if it's secondary).                                      */
	/*********************************************************************/
	if (QLLC_LINK_STATION_IS_PRIMARY (ls_ptr) == FALSE)
	{
	  qllc_lstrt(ls_ptr, vc_ptr, port_id);
	}
      }
    }
  }
  else
  {
    /*************************************************************************/
    /* RESET CONFIRMATION                                                    */
    /*************************************************************************/
    QBM_FREE_BUFFER(buffer_ptr);
    if (station_ptr->station_state == opening)
    {
      if (QLLC_LINK_STATION_IS_PRIMARY (ls_ptr) == TRUE)
      {
	/*********************************************************************/
	/* stop inact timer                                                  */
	/*********************************************************************/
	w_stop(&(station_ptr->inact_dog));
      }
      else
      {
	/*********************************************************************/
	/* Secondary station requiring higher level stimulus                 */
	/*********************************************************************/
	qllc_lstrt (ls_ptr, vc_ptr, port_id);
      }
      station_ptr->station_state = (byte)opened;
      qcm_make_stas_result (
	channel_id,
	QSM_RETURN_USER_SAP_CORRELATOR(station_ptr->qllc_sap_correlator),
	station_ptr->user_ls_correlator,
	station_ptr->max_i_field,
	station_ptr->remote_addr_len,
	station_ptr->remote_addr
	);
      TRACE_START(station_ptr);
      if (QLLC_LINK_STATION_IS_PRIMARY (ls_ptr) == FALSE)
      {
	/*********************************************************************/
	/* start inact timer                                                 */
	/*********************************************************************/
	w_start(&(station_ptr->inact_dog));
      }
    }
    else
    {
      /***********************************************************************/
      /* Not OPENING                                                         */
      /***********************************************************************/
      qvm_close_vc(vc_ptr,port_id,NORMAL_TERMINATION,FALSE);
      station_ptr->station_state = closing;
    }
  }
  if (unlock) unlockl(&(station_ptr->lock));
  return;
}

/*****************************************************************************/
/* Function     qlm_incoming_call                                            */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/*              This procedure gets called when a buffer is received         */
/*              which contains an incoming call indication.                  */
/*              The incoming call carries with it a temporary call_id        */
/*              which must be used on the start command issued to the        */
/*              dh if the call is to be accepted, or the reject ioctl        */
/*              if the call is to be rejected.                               */
/*                                                                           */
/*              Note that the call_id is appended to the bottom of the       */
/*              read ext that will be sent to SNA. Read it from the buffer   */
/*              and pass it to the virt_circuit, on the reject/accept calls. */
/*              There is no need to store it, as it is only a temporary id   */
/*              that lasts while the call is being accepted/rejected. After  */
/*              that, if QLLC accepted the call, it will be passed a new     */
/*              session_id on the start done result.                         */
/*                                                                           */
/*                                                                           */
/* Return       none                                                         */
/*                                                                           */
/* Parameters   the address of the port on which the call was received       */
/*              the address of the incoming call buffer                      */
/*                                                                           */
/*****************************************************************************/
void qlm_incoming_call(
  port_type       *port_id,
  unsigned short   netid,
  gen_buffer_type *buffer_ptr)
{
  station_type  *station_ptr;
  x25_vc_type   *vc_ptr;
  cb_fac_t       incoming_facs; 
  struct x25_reject_data  reject_data; /* used as dh arg                     */
  qvm_rc_type    qvm_rc;
  qpm_rc_type    qpm_rc;
  bool           unlock;

  /***************************************************************************/
  /* There is only one listening station on a port. Identify which station   */
  /* is listening on this port, and initialise the local variables.          */
  /***************************************************************************/
  outputf("QLM_INCOMING_CALL: find the listening station netid=%d\n", netid);
  station_ptr = qlm_find_ls_given_netid(netid, &unlock);
  if (station_ptr == (station_type *)NULL)
  {
    outputf("QLM_INCOMING_CALL: station_ptr is null %d\n",station_ptr);
    /*************************************************************************/
    /* We got a call for someone we don't know, just reject it		     */
    /*************************************************************************/
    outputf("QLM_INCOMING_CALL: rejecting call for unknown netid\n");
    (void) qpm_make_reject_data(
	&reject_data,
	netid,
	QBM_RETURN_SESSION_ID(buffer_ptr),
	QBM_RETURN_CALL_ID(buffer_ptr));

    (void) qvm_make_clear_req_buffer(buffer_ptr, NORMAL_TERMINATION);

    qpm_rc = qpm_reject (port_id, &reject_data, buffer_ptr);
    if (qpm_rc != qpm_rc_ok)
    {
      outputf("QLM_INCOMING_CALL: qpm reject failed\n");
      QBM_FREE_BUFFER(buffer_ptr);
    }
    return;
  }

  outputf("QLM_INCOMING_CALL: station found\n");
  vc_ptr = &(station_ptr->virt_circuit);

  /***************************************************************************/
  /* Check that the Call User Data shows that the call is for QLLC.          */
  /***************************************************************************/
  outputf("QLM_INCOMING_CALL: buffer_ptr = %x\n",buffer_ptr);
  print_x25_buffer(buffer_ptr);

  /* check for listening station in the listen list, defect 156503 */
  if (!listening_netid(netid, port_id)
    || station_ptr->listen_accepted_pdg_start_done
    || JSMBUF_LENGTH(buffer_ptr) <= X25_OFFSETOF_FAC_CUD_DATA
    || (QBM_RETURN_CUD(buffer_ptr) != 0xC3
    &&  QBM_RETURN_CUD(buffer_ptr) != 0xCB))
  {
    int rejcode;	/* rejection diagnostic code */

    /*
     * if the netid doesn't match here, its probably a legitimate call
     * that snuck in between when we got the first call for a listener, and
     * when we will halt the listening session. This is not an error, we must
     * just politely reject the call
     */
    /* check for listening station in the listen list, defect 156503 */
    if (!listening_netid(netid, port_id)
      || station_ptr->listen_accepted_pdg_start_done)
    {
      outputf("QLM_INCOMING_CALL: rejecting sneaker call\n");
      rejcode = NORMAL_TERMINATION;
    }
    else
    {
      outputf("QLM_INCOMING_CALL: invalid CUD for QLLC\n");
      rejcode = INVALID_LLC_TYPE;
    }
    /*************************************************************************/
    /* This call is not for QLLC                                             */
    /*************************************************************************/
    outputf("QLM_INCOMING_CALL: call qvm_reject_incoming_call\n");
    qvm_rc = qvm_reject_incoming_call(
      vc_ptr,port_id,buffer_ptr, rejcode);
    outputf("QLM_INCOMING_CALL: qvm_rc from reject function = %d\n",qvm_rc);
    if (qvm_rc != qvm_rc_ok)
    {
      station_ptr->reason_for_closure = sna_system_error;
      qlm_delete_station(station_ptr,NULL,NULL);
      return;
    }
    else
      if (unlock) unlockl(&(station_ptr->lock));
    return;
  }
  /***************************************************************************/
  /* The X.25 D-bit acknowledgement scheme is not supported by SNA. If the   */
  /* D-bit is set, reject the call.                                          */
  /***************************************************************************/
  if (QBM_RETURN_D_BIT(buffer_ptr) != FALSE)
  {
    outputf("QLM_INCOMING_CALL: d bit set. Invalid\n");
    qvm_rc = qvm_reject_incoming_call(
      vc_ptr,port_id,buffer_ptr,INVALID_D_BIT_REQUEST);
    if (qvm_rc != qvm_rc_ok)
    {
      station_ptr->reason_for_closure = sna_system_error;
      qlm_delete_station(station_ptr,NULL,NULL);
      return;
    }
    if (unlock) unlockl(&(station_ptr->lock));
    return;
  }
  /***************************************************************************/
  /* If the D-bit wasn't set, and the CUD is one of the ones QLLC uses, then */
  /* QLLC will accept the incoming call.                                     */
  /* Note that any negotiation of facilities is done by the qvm.             */
  /***************************************************************************/
  /***************************************************************************/
  /* We must first ensure that the station has a remote_addr.                */
  /* If the id is provided by the caller, then use that id                   */
  /* If the caller doesn't provide an id, use the "remotenn"                 */
  /* where "nn" is a pair of bytes generated by splitting the netid of       */
  /* the station into its high and low bytes.                                */
  /***************************************************************************/
  outputf("QLM_INCOMING_CALL: ensure station gets a remote address\n");
  QBM_RETURN_CALLING_ADDRESS(buffer_ptr,station_ptr->remote_addr);
  /***************************************************************************/
  /* Test the length field returned by the function, to see whether a        */
  /* calling address was provided or not.                                    */
  /* If the length field is not zero, which indicates that the caller and/   */
  /* or the network provided the calling address in the incoming call        */
  /* indication packet.                                                      */
  /* If the length field is zero it indicates that the address was not       */
  /* present.                                                                */
  /***************************************************************************/
  if (station_ptr->remote_addr_len == 0)
  {
    /*************************************************************************/
    /* The caller has not provided the calling address, or it was stripped   */
    /* out by the network. Simulate a calling address, using remotenn, as    */
    /* described above.                                                      */
    /*************************************************************************/
    (void)strncpy(station_ptr->remote_addr,"remote",6);
    /*************************************************************************/
    /* Now to make the id unique convert the station's netid into a pair     */
    /* of bytes formed from the high-byte/low-byte parts of the netid.       */
    /*************************************************************************/
    station_ptr->remote_addr[6] = ((station_ptr->netid & 0xFF00) >> 8);
    station_ptr->remote_addr[7] =  (station_ptr->netid & 0x00FF);
    station_ptr->remote_addr[8] = '\0';
  }
  outputf("QLM_INCOMING_CALL: remote address=%s\n",station_ptr->remote_addr);
    
  outputf("QLM_INCOMING_CALL: accept incoming call\n");
  if (qvm_accept_incoming_call(
    vc_ptr,
    port_id,
    buffer_ptr,
    station_ptr->station_tag,
    station_ptr->qllc_ls_correlator
    ) != qvm_rc_ok)
  {
    outputf("QLM_INCOMING_CALL: failed to accept incoming call\n");
    station_ptr->reason_for_closure = sna_system_error;
    qlm_delete_station(
      station_ptr,
      (correlator_type)NULL,
      (gen_buffer_type *)NULL);
    return;
  }
  else
  {
    outputf("QLM_INCOMING_CALL: call accepted\n");
    /*************************************************************************/
    /* Set the listening flag in the sub state to OFF.                       */
    /* And turn off the listening flag, so that the Station Started expects  */
    /* the station to be in opening mode.                                    */
    /*************************************************************************/
    station_ptr->station_sub_state &= ~DLC_LISTENING;
    station_ptr->flags & ~DLC_SLS_LSVC;
    /*************************************************************************/
    /* It is necessary to remeber that this was a listener, just until the   */
    /* Start Done arrives. This is because the Start Done will give a        */
    /* session_id which is not necessarily the same as the original value    */
    /* that was returned by the Device Handler at Start Listen.              */
    /*************************************************************************/
    station_ptr->listen_accepted_pdg_start_done = TRUE;
    if (unlock) unlockl(&(station_ptr->lock));
    return;
  }
}

/*****************************************************************************/
/* Function     qlm_incoming_call_rejected                                   */
/*                                                                           */
/* Description                                                               */
/*       This procedure is called when a Reject Done status block is         */
/*       received from the DH. Such a status block can only result           */
/*       from a previous call to the DH to reject an incoming call.          */
/*       The call would have been rejected for one of the reasons            */
/*       given in qlm_incoming_call() procedure.                             */
/*       I can't see that any more needs to be done here, but this           */
/*       procedure will give us the opportunity to make any changes          */
/*       to the station if the need arises.                                  */
/*       (The status block is freed by the port manager).                    */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*****************************************************************************/
void qlm_incoming_call_rejected(

  unsigned long   result,
  unsigned short  netid,
  unsigned short  session_id)
{
  return;
}

/*****************************************************************************/
/* Function     qlm_receive_data                                             */
/*                                                                           */
/* Description                                                               */
/*              This function is called by the port interrupt handler when   */
/*              data is received that is destined for a known station.       */
/*              The QLM will pass it up to the relevant channel, which will  */
/*              either queue it for an application user, or send it directly */
/*              to a kernel user.                                            */
/*                                                                           */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Parameters   buffer_ptr                                                   */
/*              netid                                                        */
/*                                                                           */
/*****************************************************************************/
void qlm_receive_data (
 
  gen_buffer_type  *buffer_ptr,
  unsigned short    netid,
  unsigned short    session_id)
 
{
  qllc_rc_type   qrc;
  qvm_rc_type    vrc;
  qcm_rc_type    qcm_rc = qcm_rc_ok;
  qpm_rc_type    qpm_rc = qpm_rc_ok;
  station_type  *station_ptr;
  x25_vc_type   *vc_ptr;
  qllc_ls_type  *ls_ptr;
  port_type     *port_id;
  channel_type  *channel_id;
  int            flag;
  bool           unlock;
  bool           stn_deleted = FALSE;
 
  /***************************************************************************/
  /* This procedure is called every time that a data buffer is received from */
  /* the dh. It has to determine the type of data being received and route   */
  /* it to the correct station. It routes the buffer based on netid.         */
  /***************************************************************************/
  station_ptr = qlm_find_ls_given_netid(netid, &unlock);
  if (station_ptr == (station_type *)NULL)
  {
    outputf("QLM_RECEIVE_DATA: station not found %d\n",station_ptr);
    if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
    return;
  }
  /***************************************************************************/
  /* Station has been found. Initialise local variables                      */
  /***************************************************************************/
  if (station_ptr->virt_circuit.session_id != session_id)
  {
    outputf("QLM_RECEIVE_DATA: station found, but session_id wrong\n");
    outputf("QLM_RECEIVE_DATA: session_id in stn =%d\n",
      station_ptr->virt_circuit.session_id);
    outputf("QLM_RECEIVE_DATA: session_id from xdh =%d\n",session_id);
    if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
    if (unlock) unlockl(&(station_ptr->lock));
    return;
  }
 
  outputf("QLM_RECEIVE_DATA: station_sub_state = %x\n",
    station_ptr->station_sub_state);
 
  vc_ptr = &(station_ptr->virt_circuit);
  ls_ptr = &(station_ptr->link_station);
  channel_id = station_ptr->channel_id;
  port_id = channel_id->port_id;
  /***************************************************************************/
  /* Call the Receive_Inactivity_Handler procedure, to register the activity */
  /***************************************************************************/
  qlm_receive_inactivity_handler(station_ptr);
 
  /***************************************************************************/
  /* Trace the arrival of data                                               */
  /***************************************************************************/
  TRACE_RECV(station_ptr,buffer_ptr);
 
  /***************************************************************************/
  /* Update the correlator fields in the read ext stored in the top of the   */
  /* buffer.                                                                 */
  /***************************************************************************/
  QBM_SET_USER_SAP_CORRELATOR(buffer_ptr,
    QSM_RETURN_USER_SAP_CORRELATOR(station_ptr->qllc_sap_correlator));
  QBM_SET_USER_LS_CORRELATOR(buffer_ptr,station_ptr->user_ls_correlator);
  QBM_SET_DLH_LENGTH(buffer_ptr,0);

  /***************************************************************************/
  /* If the Q bit in the buffer is not set, call the FSM to notify it that   */
  /* data has been received.                                                 */
  /***************************************************************************/
  if (QBM_RETURN_Q_BIT(buffer_ptr) == FALSE)
  {
    /*************************************************************************/
    /* Set the read ext flag to NORM                                         */
    /*************************************************************************/
    flag = QBM_RETURN_DLC_FLAGS(buffer_ptr);
    flag = flag | DLC_INFO;
    QBM_SET_DLC_FLAGS(buffer_ptr,flag);
 
    outputf("QLM_RECEIVE_DATA: Q bit not set\n");
    outputf("QLM_RECEIVE_DATA: call FSM to register data\n");
    qrc = qllc_rdata(ls_ptr);
    if (qrc != qrc_ok)
    {
      outputf("QLM_RECEIVE_DATA: FSM reported error\n");
      vrc = qvm_close_vc(vc_ptr,port_id,LOCAL_PROCEDURE_ERROR_GENERAL,FALSE);
      if (vrc != qvm_rc_ok)
      {
	outputf("QLM_RECEIVE_DATA: qvm_close_vc reported error\n");
	station_ptr->reason_for_closure = sna_system_error;
	qlm_delete_station(
	  station_ptr,
	  (correlator_type)NULL,
	  (gen_buffer_type *)NULL);
	return;
      }
      else
      {
	outputf("QLM_RECEIVE_DATA: qvm_close_vc successful\n");
	station_ptr->station_state = closing;
	if (qrc == qrc_x25_error)
	  station_ptr->reason_for_closure = (int)sna_system_error;
	else
	  station_ptr->reason_for_closure = (int)protocol_error;
      }
    }
    else
    {
      outputf("QLM_RECEIVE_DATA: FSM registered data successfully\n");
      /***********************************************************************/
      /* Normal received data and QLLC function was ok                       */
      /***********************************************************************/
      /***********************************************************************/
      /* Normal data has been received and the fsm has been notified.        */
      /* Build a dlc_io_ext in the top of the buffer.                        */
      /* This will be used for routing the data to the user. Parts of it     */
      /* will be passed to the user in the read ext.                         */
      /*                                                                     */
      /* There is no need to get an extra mbuf to build the dlc ext          */
      /* as it is built where the gp hdr was, in the reserved space          */
      /* at the top of the buffer.                                           */
      /***********************************************************************/
      /***********************************************************************/
      /* Before sending data to the qcm check whether the channel is in      */
      /* local busy mode. If it is the data is queued here in the station    */
      /***********************************************************************/
      if ((station_ptr->station_sub_state & DLC_LOCAL_BUSY) == DLC_LOCAL_BUSY)
      {
	outputf("QLM_RECEIVE_DATA: station is local busy\n");
	/*********************************************************************/
	/* Queue data buffer in station if in local busy mode.               */
	/*********************************************************************/
	QBM_ENQUE_BUFFER(buffer_ptr,&(station_ptr->receive_data_queue));
      }
      else
      {
	outputf("QLM_RECEIVE_DATA: station is not local busy\n");
	/*********************************************************************/
	/* Station is not local busy                                         */
	/*********************************************************************/
	qcm_rc = qcm_receive_data (channel_id,buffer_ptr);
	/*********************************************************************/
	/* ..but it might be local busy now..check the rc and if the QCM has */
	/* requested that the session be put into busy/retry state, take the */
	/* appropriate actions...                                            */
	/*********************************************************************/
	if (qcm_rc == qcm_rc_local_busy)
	{
	  outputf("QLM_RECEIVE_DATA: station going into local busy\n");
	  station_ptr->station_sub_state |= DLC_LOCAL_BUSY;
	  QBM_ENQUE_BUFFER(buffer_ptr, &(station_ptr->receive_data_queue));
	  /*******************************************************************/
	  /* Ask device handler to set session to busy. qpm_rc is checked in */
	  /* a moment, so that rc checking can be shared between this domain */
	  /* and the retry domain below.                                     */
	  /*******************************************************************/
	  qpm_rc = qpm_enter_local_busy(port_id,vc_ptr->session_id);
	}
	else if (qcm_rc == qcm_rc_retry)
	{
	  outputf("QLM_RECEIVE_DATA: station going into retry state\n");
	  station_ptr->station_sub_state |= DLC_LOCAL_BUSY;
	  QBM_ENQUE_BUFFER(buffer_ptr, &(station_ptr->receive_data_queue));
	  /*******************************************************************/
	  /* Ask device handler to set session to busy. qpm_rc is checked in */
	  /* a moment, so that rc checking can be shared between this domain */
	  /* and the retry domain below.                                     */
	  /*******************************************************************/
	  qpm_rc = qpm_enter_local_busy(port_id,vc_ptr->session_id);
	  w_start(&(station_ptr->retry_dog));
	}
	if ( (qcm_rc != qcm_rc_ok) || (qpm_rc != qpm_rc_ok) )
	{
	  outputf("QLM_RECEIVE_DATA: qcm or qpm failed, closing station\n");
	  /*******************************************************************/
	  /* If the qcm/qpm function went wrong it will have freed the buffer*/
	  /*******************************************************************/
	  vrc=qvm_close_vc(vc_ptr,port_id,LOCAL_PROCEDURE_ERROR_GENERAL,FALSE);
	  if (vrc != qvm_rc_ok)
	  {
	    outputf("QLM_RECEIVE_DATA: qvm_close_vc failed\n");
	    station_ptr->reason_for_closure = sna_system_error;
	    qlm_delete_station(
	      station_ptr,
	      (correlator_type)NULL,
	      (gen_buffer_type *)NULL);
	    return;
	  }
	  else
	  {
	    station_ptr->reason_for_closure = (int)sna_system_error;
	    station_ptr->station_state = (byte)closing;
	  }
	}
      }
    }
  }
  else
  {
    outputf("QLM_RECEIVE_DATA: QLLC cmd or rsp received\n");
    /*************************************************************************/
    /*   The data buffer received contains a QLLC command                    */
    /*   or response. It is passed to qlm_receive_qllu()                     */
    /*   There is a chance the station will be deleted. If so, stn_deleted   */
    /*   will be set to TRUE.                                                */
    /*************************************************************************/
    qrc = qlm_receive_qllu(station_ptr,port_id,buffer_ptr, &stn_deleted);
    if (qrc == qrc_x25_error)
    {
      outputf("QLM_RECEIVE_DATA: qlm_receive_qllu reported error\n");
      vrc = qvm_close_vc(vc_ptr,port_id,LOCAL_PROCEDURE_ERROR_GENERAL,FALSE);
      if (vrc != qvm_rc_ok)
      {
	outputf("QLM_RECEIVE_DATA: qvm_close_vc failed\n");
	station_ptr->reason_for_closure = sna_system_error;
	qlm_delete_station(
	  station_ptr,
	  (correlator_type)NULL,
	  (gen_buffer_type *)NULL);
	return;
      }
      else
      {
	outputf("QLM_RECEIVE_DATA: close was successful\n");
	station_ptr->reason_for_closure = (int)sna_system_error;
	station_ptr->station_state = (byte)closing;
      }
    }
  }

  /* if we get here, unlock station */

  if (unlock && !stn_deleted) 
  {
      outputf("QLM_RECEIVE_DATA: unlock station.\n");
      unlockl(&(station_ptr->lock));
  }
  else
      outputf("QLM_RECEIVE_DATA: station was already deleted.\n");

  return;
}

/*****************************************************************************/
/* Function     qlm_invalid_packet_rx                                        */
/*                                                                           */
/* Description                                                               */
/*              Called on off-level whenever interrupt handler detects an    */
/*              illegal packet, such as one with the d-bit set, or an int    */
/*              or int confirm.                                              */
/*                                                                           */
/*              diagnostic parm is INVALID_D_BIT_REQUEST, or link_station_   */
/*              unusual_network_condition.                                   */
/*                                                                           */
/* Return   void                                                             */
/*                                                                           */
/* Parameters   buffer_ptr                                                   */
/*              netid                                                        */
/*              diagnostic                                                   */
/*****************************************************************************/
void qlm_invalid_packet_rx (

  gen_buffer_type *buffer_ptr,
  unsigned short   netid,
  unsigned short   session_id,
  diag_code_type   diagnostic)
{
  station_type *station_ptr;
  x25_vc_type *vc_ptr;
  port_type *port_id;
  channel_id_type channel_id;
  bool            unlock;

  /***************************************************************************/
  /* The interrupt handler has detected a packet received from the DH which  */
  /* has an invalid type or in which the D-bit is set.                       */
  /***************************************************************************/
  /***************************************************************************/
  /* Find the station to which the buffer relates.                           */
  /***************************************************************************/
  station_ptr = qlm_find_ls_given_netid(netid, &unlock);
  if (station_ptr == (station_type *)NULL)
  {
    outputf("QLM_INVALID_PACKET_RX: station not found %d\n",station_ptr);
    QBM_FREE_BUFFER(buffer_ptr);
    return;
  }
  /***************************************************************************/
  /* station has been found.                                                 */
  /***************************************************************************/
  if (station_ptr->virt_circuit.session_id != session_id)
  {
    outputf("QLM_INV_PKT: station found, but session_id wrong\n");
    outputf("QLM_INV_PKT: session_id in stn =%d\n",
      station_ptr->virt_circuit.session_id);
    outputf("QLM_INV_PKT: session_id from xdh =%d\n",session_id);
    if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
    if (unlock) unlockl(&(station_ptr->lock));
    return;
  }

  /***************************************************************************/
  /* Initialise local variables                                              */
  /***************************************************************************/
  channel_id = station_ptr->channel_id;
  vc_ptr = &(station_ptr->virt_circuit);
  port_id = QCM_RETURN_PORT_ID(channel_id);

  /*************************************************************************/
  /* An invalid packet has been received from the DH. The VC is closed.    */
  /*************************************************************************/
  if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
  if (qvm_close_vc(vc_ptr, port_id, diagnostic,FALSE) != qvm_rc_ok)
  {
    station_ptr->reason_for_closure = sna_system_error;
    qlm_delete_station(
      station_ptr,
      (correlator_type)NULL,
      (gen_buffer_type *)NULL);
  }
  else
  {
    station_ptr->reason_for_closure =
      (int)link_station_unusual_network_condition;
    station_ptr->station_state = closing;
    if (unlock) unlockl(&(station_ptr->lock));
    /*************************************************************************/
    /* stop all timers associated with this station                          */
    /*************************************************************************/
  }
  return;
}

/*****************************************************************************/
/* Function     qlm_receive_qllu                                             */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
qllc_rc_type  qlm_receive_qllu(

  station_type    *station_ptr,
  port_type       *port_id,
  gen_buffer_type *buffer_ptr,
  bool             *stn_deleted)
{

  x25_vc_type     *vc_ptr;
  qllc_ls_type    *ls_ptr;
  qllc_rc_type     rc;
  channel_id_type  channel_id;
  result_code_type test_result;
  int              i;
  qllc_qllu_type   qllu;
  bool             xid_response_pending;
  char             test_data[TEST_STRING_LENGTH];
  int              flag;

  /***************************************************************************/
  /* Initialise local variables                                              */
  /***************************************************************************/
  ls_ptr = &(station_ptr->link_station);
  vc_ptr = &(station_ptr->virt_circuit);
  channel_id = station_ptr->channel_id;
  (void)qbm_return_qllu(buffer_ptr,&qllu);

  qlm_qllu_i_field_check(&qllu,station_ptr);

/* defect 52838 */
  outputf("QLM_RECEIVE_QLLU: addr=%x, ctl=%x, nego=%x\n",
          qllu.address_field, qllu.control_field,
          (station_ptr->flags & DLC_SLS_NEGO));
  if /* the received packet is an xid, and this station is a primary */
     ((qllu.control_field == qxid_cmd) &&
                        (QLLC_LINK_STATION_IS_PRIMARY(ls_ptr) == TRUE))
  {
    /* note: this is to insure that negotiable/negotiable and
             negotiable/primary xids will be accepted since they always
             come in as command frames instead of response frames */
    /* force the packet to be a response packet */
    qllu.address_field = 0x00;
  }
/* end defect 52838 */

  if (qllu.address_field == 0xFF)
  {
    outputf("QLM_RECEIVE_QLLU: command Q-packet received\n");
    /*************************************************************************/
    /* This is a command Q-Packet                                            */
    /*************************************************************************/
    switch (qllu.control_field)
    {
    case (qsm_cmd) :
      outputf("QLM_RECEIVE_QLLU: qsm_cmd received\n");
      /* For a PVC, it has to receive a RESET  Confirm to put  */
      /* the PVC in the opened state to receive any packet     */
      if ((station_ptr -> station_state == opened ) ||
           (QVM_VC_IS_PVC(vc_ptr) == FALSE))
         {
          rc = qllc_qsm(ls_ptr,vc_ptr,port_id);
          outputf("QLM_RECEIVE_QLLU: qsm_cmd accepted\n");
  	 }
      outputf("QLM_RECEIVE_QLLU: free qsm_cmd, ptr = %x\n",buffer_ptr);
      QBM_FREE_BUFFER(buffer_ptr);
      break;
    case (qdisc_cmd) :
      outputf("QLM_RECEIVE_QLLU: qdisc_cmd received\n");
      rc = qllc_qdisc(ls_ptr,vc_ptr,port_id, stn_deleted);
      outputf("QLM_RECEIVE_QLLU: free qdisc_cmd, ptr = %x\n",buffer_ptr);
      QBM_FREE_BUFFER(buffer_ptr);
      break;
    case (qxid_cmd) :
      outputf("QLM_RECEIVE_QLLU: qxid_cmd received\n");
      /***********************************************************************/
      /* find out from QLLC FSM whether we've already passed an XID command  */
      /* to SNA.                                                             */
      /***********************************************************************/
      xid_response_pending = QLLC_XID_RESPONSE_PENDING(ls_ptr);
      rc = qllc_qxid_cmd(ls_ptr,vc_ptr,port_id);
     /* if ((rc == qrc_ok) && (!xid_response_pending)) */
      if (rc == qrc_ok)
      {
	/*********************************************************************/
	/* The building of the read extension started in the port receive    */
	/* function, which set the oflo flag if necessary. The               */
	/* qlm_receive_data function put the correlators in, and this        */
	/* function now updates the flag to show that this is xid data.      */
	/*********************************************************************/
	flag = QBM_RETURN_DLC_FLAGS(buffer_ptr);
	flag = flag | DLC_XIDD | DLC_RSPP;
	QBM_SET_DLC_FLAGS(buffer_ptr,flag);
	qcm_receive_data(channel_id,buffer_ptr);
      }
      else /* if (rc != qrc_ok || xid_response_pending)                      */
      {
	outputf("QLM_RECEIVE_QLLU: free qxid_cmd, ptr = %x\n",buffer_ptr);
	QBM_FREE_BUFFER(buffer_ptr);
      }
      break;
    case (qtest_cmd) :
      outputf("QLM_RECEIVE_QLLU: qtest_cmd received\n");
      INC_RAS_COUNTER(station_ptr->ras_counters.test_commands_received);
      rc = qllc_qtest_cmd(ls_ptr, vc_ptr, port_id);
      if (rc == qrc_ok)
      {
	/*********************************************************************/
	/*  Adjust data length and offset                                    */
	/*  so that QLLC header is not                                       */
	/*  included.                                                        */
	/*********************************************************************/
	rc = qllc_ltest(ls_ptr,vc_ptr,port_id, buffer_ptr);
      }
      else
      {
	/*********************************************************************/
	/* return code was not ok                                            */
	/*********************************************************************/
	outputf("QLM_RECEIVE_QLLU: free qtest_cmd, ptr = %x\n",buffer_ptr);
	QBM_FREE_BUFFER(buffer_ptr);
      }
      break;
    case (qrr_cmd) :
      outputf("QLM_RECEIVE_QLLU: qrr_cmd received\n");
      rc = qllc_qrr(ls_ptr);
      outputf("QLM_RECEIVE_QLLU: free qrr_cmd, ptr = %x\n",buffer_ptr);
      QBM_FREE_BUFFER(buffer_ptr);
      break;
    default :
      outputf("QLM_RECEIVE_QLLU: erroneous Q-command packet received\n");
      /***********************************************************************/
      /* erroneous pkt received                                              */
      /***********************************************************************/
      /*********************************************************************/
      /* Pass errpk() the QLLC Address and Control fields from the QLLU    */
      /*********************************************************************/
      rc = qllc_errpk(
	ls_ptr,vc_ptr,port_id,
	QBM_RETURN_QLLC_ADDRESS_FIELD(buffer_ptr),
	QBM_RETURN_QLLC_CONTROL_FIELD(buffer_ptr)
	);
      outputf("QLM_RECEIVE_QLLU: free q_errpk, ptr = %x\n",buffer_ptr);
      QBM_FREE_BUFFER(buffer_ptr);
      break;
    }
  }
  else
  {
    outputf("QLM_RECEIVE_QLLU: Response Q-packet received\n");
    /*************************************************************************/
    /* address_field is not 0xFF so response                                 */
    /*************************************************************************/
    switch (qllu.control_field)
    {
    case (qrd_rsp) :
      outputf("QLM_RECEIVE_QLLU: qrd_rsp received\n");
      rc = qllc_qrd(ls_ptr,vc_ptr,port_id);
      outputf("QLM_RECEIVE_QLLU: free qrd_rsp, ptr = %x\n",buffer_ptr);
      QBM_FREE_BUFFER(buffer_ptr);
      break;
    case (qxid_rsp) :
      /***********************************************************************/
      /* Don't free the buffer in this case, as the channel manager will     */
      /* handle it from here, and pass it up to the user.                    */
      /***********************************************************************/
      outputf("QLM_RECEIVE_QLLU: qxid_rsp received\n");
      rc = qllc_qxid_rsp(ls_ptr,vc_ptr,port_id);
      if (rc == qrc_ok)
      {
	/*********************************************************************/
	/* The building of the read extension started in the port receive    */
	/* function, which set the oflo flag if necessary. The               */
	/* qlm_receive_data function put the correlators in, and this        */
	/* function now updates the flag to show that this is xid data.      */
	/*********************************************************************/
	flag = QBM_RETURN_DLC_FLAGS(buffer_ptr);
	flag = flag | DLC_XIDD;
	QBM_SET_DLC_FLAGS(buffer_ptr,flag);
	qcm_receive_data(channel_id,buffer_ptr);
      }
      break;
    case (qtest_rsp) :
      outputf("QLM_RECEIVE_QLLU: qtest_rsp received\n");
      rc = qllc_qtest_rsp(ls_ptr,vc_ptr,port_id);
      outputf("QLM_RECEIVE_QLLU: rc from qllc_qtest_rsp = %d\n",rc);
      /***********************************************************************/
      /* Don't proceed unless the rc was OK, and the station is primary.     */
      /* This is because if the station is secondary the FSM will have       */
      /* issued a qfrmr, and will then return OK.                            */
      /***********************************************************************/
      if ((rc == qrc_ok) && (QLLC_LINK_STATION_IS_PRIMARY(ls_ptr) == TRUE))
      {
	/*********************************************************************/
	/* Find out if test pattern matches.                                 */
	/*********************************************************************/
        outputf("QLM_RECEIVE_QLLU: getting test resp data from buffer\n");
        QBM_RETURN_BLOCK(
	  buffer_ptr,
	  OFFSETOF(body.qllc_body.user_data[0], x25_mbuf_t),
	  test_data,
	  JSMBUF_LENGTH(buffer_ptr)-2                 /* TEST_STRING_LENGTH */
	  );
      
        i = 0;
	while (i<TEST_STRING_LENGTH && (test_data[i] == i))
	  i++;
	if (i == TEST_STRING_LENGTH)
	{
	  outputf("QLM_RECEIVE_QLLU: good compare of test data\n");
          test_result = test_completed_ok;
        }
	else
	{
	  outputf("QLM_RECEIVE_QLLU: bad compare of test data\n");
	  INC_RAS_COUNTER(station_ptr->ras_counters.test_command_failures);
	  test_result = bad_data_compare_on_test;
	}
        outputf("QLM_RECEIVE_QLLU: send result to QCM\n");
	qcm_make_result (
	  channel_id,
	  QSM_RETURN_USER_SAP_CORRELATOR(station_ptr->qllc_sap_correlator),
	  station_ptr->user_ls_correlator,
	  test_completed,
	  test_result
	);
      }
      outputf("QLM_RECEIVE_QLLU: free qtest_cmd, ptr = %x\n",buffer_ptr);
      QBM_FREE_BUFFER(buffer_ptr);
      break;
    case (qua_rsp) :
      outputf("QLM_RECEIVE_QLLU: qua_rsp received\n");
      rc = qllc_qua(ls_ptr,vc_ptr,port_id);
      outputf("QLM_RECEIVE_QLLU: qllc_qua rc = %d\n",rc);
      outputf("QLM_RECEIVE_QLLU: free qua_rsp, ptr = %x\n",buffer_ptr);
      QBM_FREE_BUFFER(buffer_ptr);
      break;
    case (qdm_rsp) :
      outputf("QLM_RECEIVE_QLLU: qdm_rsp received\n");
      rc = qllc_qdm(ls_ptr, vc_ptr, port_id);
      outputf("QLM_RECEIVE_QLLU: free qdm_rsp, ptr = %x\n",buffer_ptr);
      QBM_FREE_BUFFER(buffer_ptr);
      break;
    case (qfrmr_rsp) :
      outputf("QLM_RECEIVE_QLLU: qfrmr_rsp received\n");
      qlm_qfrmr_alert_gen(station_ptr,buffer_ptr);
      rc = qllc_qfrmr(ls_ptr,vc_ptr,port_id);
      outputf("QLM_RECEIVE_QLLU: free qfrmr_rsp, ptr = %x\n",buffer_ptr);
      QBM_FREE_BUFFER(buffer_ptr);
      break;
    default :  /* erroneous pkt received                                     */
      outputf("QLM_RECEIVE_QLLU: erroneous Q-Response packet received\n");
      rc = qllc_errpk(
	 ls_ptr,
	 vc_ptr,
	 port_id,
	/*********************************************************************/
	/* Pass errpk() the QLLC Address and Control fields from the QLLU    */
	/*********************************************************************/
	 QBM_RETURN_QLLC_ADDRESS_FIELD(buffer_ptr),
	 QBM_RETURN_QLLC_CONTROL_FIELD(buffer_ptr)
	);
      outputf("QLM_RECEIVE_QLLU: free q_errpk, ptr = %x\n",buffer_ptr);
      QBM_FREE_BUFFER(buffer_ptr);
      break;
    }
  }
  return(rc);
}

/*****************************************************************************/
/* Function     qlm_receive_inactivity_handler                               */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
void qlm_receive_inactivity_handler(

  station_type *station_ptr)
{

  outputf("QLM_RECEIVE_INACT_HNDLR: called\n");
  /***************************************************************************/
  /* Data has been received. If the inactivity timer is running, then it     */
  /* should be stopped, and restarted at its full value.                     */ 
  /***************************************************************************/
  if (station_ptr->inact_dog.count != 0)
  { 
    w_stop(&(station_ptr->inact_dog));
    w_start(&(station_ptr->inact_dog));
  }
  else
  {
    /**************************************************************************/
    /* If the timer isn't running, then either that is because the station is */
    /* already inactive, and an inactivity ended result should be made, or it */
    /* because the station is primary (and hence not checking for inactivity) */
    /* or is already contacted, (and therefore no longer checking for         */
    /* inactivity).                                                           */
    /**************************************************************************/
    if ( station_ptr->station_state == (byte)inactive
      && (station_ptr->station_sub_state & DLC_CONTACTED) != DLC_CONTACTED
      && QLLC_LINK_STATION_IS_PRIMARY(&(station_ptr->link_station)) == FALSE
      )
    {
      station_ptr->station_state = (byte)opened;
      qcm_make_result (
        station_ptr->channel_id,
        QSM_RETURN_USER_SAP_CORRELATOR(station_ptr->qllc_sap_correlator),
        station_ptr->user_ls_correlator,
        inact_ended,
        successful
        );
    }
    outputf("QLM_RECEIVE_INACT_HNDLR: returning. NO TIMEOUTS WERE SET\n");
    return;
  }
} 


/*****************************************************************************/
/* Function     qlm_write_error                                              */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*****************************************************************************/
void qlm_write_error (

  unsigned short netid,
  unsigned short session_id,
  gen_buffer_type *buffer_ptr)
{
  station_type *station_ptr;
  bool          unlock;

  outputf("QLM_WRITE_ERROR: called\n");
  /***************************************************************************/
  /* This procedure is called by the interrupt handler when it gets a        */
  /* tx_done status block from the DH which indicates that an error occurred */
  /* during transmission.                                                    */
  /* It is passed the netid, from the status block. This identifies the      */
  /* station which issued the write to the DH. It is also passed a ptr to    */
  /* the buffer which it was attempting to send.                             */
  /* The buffer is freed, an error is logged, and the vc is closed.          */
  /***************************************************************************/
  QBM_FREE_BUFFER(buffer_ptr);
  station_ptr = qlm_find_ls_given_netid(netid, &unlock);
  if (station_ptr == (struct station_type *)NULL)
  {
    outputf("QLM_WRITE_ERROR: station not found %d\n",station_ptr);
    return;
  }
  /***************************************************************************/
  /* Make sure you haven't got hold of a netid that was used before for a    */
  /* previous session                                                        */
  /***************************************************************************/
  if (station_ptr->virt_circuit.session_id != session_id)
  {
    if (unlock) unlockl(&station_ptr->lock);
    return;
  }
  /***************************************************************************/
  /* Request closure of the vc by the qvm                                    */
  /***************************************************************************/
  if (qvm_close_vc(
    &(station_ptr->virt_circuit),
    station_ptr->channel_id->port_id,
    NORMAL_TERMINATION,
    FALSE
    ) != qvm_rc_ok)
  {
    /*************************************************************************/
    /* the close attempt failed, so force the station to shutdown.           */
    /*************************************************************************/
    station_ptr->reason_for_closure = sna_system_error;
    qlm_delete_station(
      station_ptr,
      (correlator_type)NULL,
      (gen_buffer_type *)NULL);
  }
  else
  {
    /*************************************************************************/
    /* Otherwise, the close vc succeeded, and when the halt done             */
    /* comes back from the DH, the station will be cleaned up and            */
    /* a result sent to the user                                             */
    /*************************************************************************/
    station_ptr->station_state = closing;
    station_ptr->reason_for_closure = successful;
    if (unlock) unlockl(&station_ptr->lock);
  }
  return;
}


/*****************************************************************************/
/* Function     QLM_MAKE_NETD_BUFFER                                         */
/*                                                                           */
/* Description  This procedure is called by the QLM when it has Network Data */
/*              to send to the user. It builds a netd buffer and copies the  */
/*              data contained in the data_area ptd to by the data_ptr parm  */
/*              into the buffer.  The type  param indicates the type of      */
/*              netd that is being sent to the user.                         */
/*              The buffer is built with the read ext in it. The QCM receive */
/*              function will then extract the read ext and pass it up to    */
/*              the user separately.                                         */
/*                                                                           */
/* Return   qcm_rc_type                                                      */
/*                                                                           */
/* Parameters station_ptr                                                    */
/*            netd_type                                                      */
/*            data_ptr                                                       */
/*****************************************************************************/
qlm_rc_type qlm_make_netd_buffer(

  station_type *station_ptr,
  type_of_netd_t  netd_type,
  byte *data_ptr)

/*****************************************************************************/
/* The data ptr parameter is passed in as a void ptr, since its use depends  */
/* on the type of netd buffer being built. The type is indicated by the      */
/* netd_type parameter.                                                      */
/*****************************************************************************/

{
  gen_buffer_type *buffer_ptr;
  qcm_rc_type qcm_rc;
  qlm_rc_type rc;
  channel_id_type channel_id;
  correlator_type user_sap_correlator;
  unsigned short fac_length;
  char fac_array[X25_MAX_FACILITIES_LENGTH];  /* much too big actually       */
  unsigned int netd_type_flags = 0;

  /***************************************************************************/
  /* Initialise local variables                                              */
  /***************************************************************************/
  channel_id = station_ptr->channel_id;
  user_sap_correlator =
    QSM_RETURN_USER_SAP_CORRELATOR(station_ptr->qllc_sap_correlator);

  /***************************************************************************/
  /* Get a buffer.                                                           */
  /***************************************************************************/
  buffer_ptr = QBM_GET_BUFFER(sizeof(gen_buffer_type));

  /***************************************************************************/
  /* Initialise buffer.                                                      */
  /***************************************************************************/

  /***************************************************************************/
  /* First, the read extension details, in the top of the buffer.            */
  /***************************************************************************/
  QBM_SET_USER_SAP_CORRELATOR(buffer_ptr,user_sap_correlator);
  QBM_SET_USER_LS_CORRELATOR(buffer_ptr,station_ptr->user_ls_correlator);
  QBM_SET_DLC_FLAGS(buffer_ptr,NETD_DATA);
  QBM_SET_DLH_LENGTH(buffer_ptr,0);

  /***************************************************************************/
  /* Fill in data area......                                                 */
  /*                                                                         */
  /* The information to be put into the netd buffer is derived from the      */
  /* cb_fac structure, which was filled in when the packet was received by   */
  /* the QLM.                                                                */
  /* The info is CCITT encoded, and relates to Charging Info, or Transit     */
  /* Delay (others may be added later).                                      */
  /*                                                                         */
  /* The cb_fac struct stores the info in X25 CCITT format, and it can be    */
  /* copied by memcpy directly into the netd buffer using the length field   */
  /* given in the cb_fac structure which is known to relate to the facility  */
  /* field being copied.                                                     */
  /*                                                                         */
  /* The netd_type parm indicates the facility from which to copy the data.  */
  /*                                                                         */
  /* The data is copied to fac_array, and THEN to buffer to avoid problems   */
  /* with contiguous space (or otherwise) in mbuf chains.                    */
  /*                                                                         */
  /* The filling in of the buffer data area depends on type of netd that     */
  /* the function is building a buffer for.                                  */
  /***************************************************************************/
  switch(netd_type)
  {
  case transit_delay :
    /*************************************************************************/
    /* Transit Delay info field is a short, so occupies two bytes.           */
    /*************************************************************************/
    fac_length = 2;
    netd_type_flags = QLLC_CINF;
    break;
  case charging_info :
    /*************************************************************************/
    /* Length of charging info structure will depend on network provider, so */
    /* is calculated from lengths in cb_fac structure.                       */
    /*************************************************************************/
    fac_length = station_ptr->facilities.ci_seg_cnt_len +
      station_ptr->facilities.ci_mon_unt_len +
	station_ptr->facilities.ci_call_dur_len;
    netd_type_flags = QLLC_TDSI;
    break;
  }
  bcopy( fac_array, data_ptr, fac_length);

  /***************************************************************************/
  /* Now copy netd type, fac length and fac data into buffer.                */
  /***************************************************************************/
  QBM_SET_NETD_FLAGS_FIELD(buffer_ptr,netd_type_flags);

  QBM_SET_NETD_DATA_LENGTH(buffer_ptr,fac_length);

  QBM_SET_BLOCK(
    buffer_ptr,
    OFFSETOF(body.qllc_netd.data[0],x25_mbuf_t),
    fac_array,
    fac_length);

  /***************************************************************************/
  /* Call channel receive function                                           */
  /***************************************************************************/
  qcm_rc = qcm_receive_data(channel_id, buffer_ptr);

  /***************************************************************************/
  /* The qcm_receive_data function can return OK, or RETRY.                  */
  /* Netd cannot be held in a busy condition.                                */
  /***************************************************************************/
  if (qcm_rc == DLC_FUNC_OK)
  {
    /*************************************************************************/
    /* Buffer was successfully passed to user, you can forget it now.        */
    /*************************************************************************/
    rc = qlm_rc_ok;
  }
  else if (qcm_rc == DLC_FUNC_RETRY)
  {
    /*************************************************************************/
    /* If a Retry has been requested, you must hold onto the buffer and try  */
    /* again in a "short time" (see below).                                  */
    /* The Retry Timer is set to repoll time held in station.                */
    /* The actual time delay is arbitrary, but QLLC will use repoll_time     */
    /* unless that proves to be unacceptable.                                */
    /*************************************************************************/
    /*************************************************************************/
    /* Queue the buffer in the station                                       */
    /*************************************************************************/
    QBM_ENQUE_BUFFER(buffer_ptr, &(station_ptr->receive_data_queue));
    
    /* timeout(retry_timer, station_ptr->repoll_time); ..?? */
    station_ptr->station_sub_state |= DLC_LOCAL_BUSY;
    rc = qlm_rc_ok;
  }
  else
  {
    /*************************************************************************/
    /* Illegal return                                                        */
    /*************************************************************************/
    rc = qlm_rc_system_error;
  }
  return (rc);
}

/*****************************************************************************/
/* Function     qlm_retry_receive                                            */
/*                                                                           */
/* Description                                                               */
/*              This function is called when the retry timer (started by     */
/*              qlm_receive_data) expires.                                   */
/*              The QLM will pass the data up to the relevant channel,       */
/*              which will either queue it for an application user, or send  */
/*              it directly to a kernel user.                                */
/*                                                                           */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Parameters   buffer_ptr                                                   */
/*              netid                                                        */
/*                                                                           */
/*****************************************************************************/
void qlm_retry_receive (
  port_type *port_id)
{
  station_type     *station_ptr;
  channel_type     *channel_id;
  gen_buffer_type  *buffer_ptr;
  qcm_rc_type       qcm_rc;
  qvm_rc_type       qvm_rc;
  x25_vc_type      *vc_ptr;
  bool		    unlock;
  /***************************************************************************/
  /* This function must service all the stations that have retries pending.  */
  /***************************************************************************/
  while ((station_ptr = qlm_find_retry_ls_on_port(port_id, &unlock)) != NULL)
  {
    channel_id = station_ptr->channel_id;
    port_id = QCM_RETURN_PORT_ID(station_ptr->channel_id);
    vc_ptr = &(station_ptr->virt_circuit);
    /***********************************************************************/
    /* Before sending data to the qcm check whether the channel is in local*/
    /* busy mode. If it is, nothing further need be done, as the user will */
    /* issue an exit lbusy command.                                        */
    /***********************************************************************/
    if ((station_ptr->station_sub_state & DLC_LOCAL_BUSY) != DLC_LOCAL_BUSY)
    {
      if (unlock) unlockl(&(station_ptr->lock));
      outputf("QLM_RETRY: station is not in local busy mode\n");
    }
    else
    {
      outputf("QLM_RETRY: station is still local busy\n");
      /*********************************************************************/
      /* Station is still local busy after the retry was instigated.       */
      /* Get first buffer off station data queue.                          */
      /*********************************************************************/
      buffer_ptr = QBM_DEQUE_BUFFER(&(station_ptr->receive_data_queue));
      qcm_rc = qcm_receive_data (channel_id,buffer_ptr);
      switch (qcm_rc)
      {
      case qcm_rc_ok:
        qpm_exit_local_busy(port_id,station_ptr->virt_circuit.session_id);
	station_ptr->station_sub_state &= ~DLC_LOCAL_BUSY;
	break;
      case qcm_rc_local_busy:
	QBM_REQUE_BUFFER(buffer_ptr,&(station_ptr->receive_data_queue));
	break;
      case qcm_rc_retry:
	QBM_REQUE_BUFFER(buffer_ptr,&(station_ptr->receive_data_queue));
	w_start(&(station_ptr->retry_dog));
	break;
      default:
	outputf("QLM_RECEIVE_DATA: qcm receive failed, closing station\n");
	/*******************************************************************/
	/* If the qcm function went wrong it will have freed the buffer    */
	/*******************************************************************/
	qvm_rc=qvm_close_vc(
	  vc_ptr,
	  port_id,
	  LOCAL_PROCEDURE_ERROR_GENERAL,
	  FALSE);
	if (qvm_rc != qvm_rc_ok)
	{
	  outputf("QLM_RECEIVE_DATA: qvm_close_vc failed\n");
	  station_ptr->reason_for_closure = sna_system_error;
	  qlm_delete_station(
	    station_ptr,
	    (correlator_type)NULL,
	    (gen_buffer_type *)NULL);
	  unlock = FALSE;
	}
	else
	{
	  station_ptr->reason_for_closure = (int)sna_system_error;
	  station_ptr->station_state = (byte)closing;
	}
	break;
      }
      if (unlock) unlockl(&(station_ptr->lock));
      return;
    }
  }
}
/*****************************************************************************/
/* Function QLM_QFRMR_ALERT_GEN                                              */
/*                                                                           */
/* Description This function is called whenever a qfrmr_rsp is received, and */
/*             it determines the reason for the reject and generates an      */
/*             appropriate alert from the QLLC alert vocabulary.             */
/*****************************************************************************/
void  qlm_qfrmr_alert_gen(
  station_type    *station_ptr,
  gen_buffer_type *buffer_ptr)
{

  byte   rejected_llu_control_byte;
  bool   rejected_llu_response;
  byte   wxyz_byte;
  /***********************************************************************/
  /* Generate Alerts 4,5,6                                               */
  /* Depending on the setting of the W,X, & Y bits in the QFRMR response */
  /* generate one of the above alerts.                                   */
  /* Note the station must be contacted.                                 */
  /***********************************************************************/
  if (QLLC_LINK_STATION_IS_OPENED(&(station_ptr->link_station)) == TRUE)
  {
    /*********************************************************************/
    /* Read the first three bytes of the QFRMR.                          */
    /*********************************************************************/
    rejected_llu_control_byte = JSMBUF_READ_BYTE(
      buffer_ptr,
      OFFSETOF(body.qllc_body.user_data[0],x25_mbuf_t),
      byte);  
    /*********************************************************************/
    /* Bit 4 of the second byte indicates whether the q_packet that was  */
    /* rejected was a command (0) or a response (1)                      */
    /*********************************************************************/
    rejected_llu_response = 0x08 & JSMBUF_READ_BYTE(
      buffer_ptr,
      OFFSETOF(body.qllc_body.user_data[1],x25_mbuf_t),
      byte);  
    wxyz_byte = JSMBUF_READ_BYTE(
      buffer_ptr,
      OFFSETOF(body.qllc_body.user_data[2],x25_mbuf_t),
      byte);  

    /*************************************************************************/
    /* Decide which alert to generate                                        */
    /*************************************************************************/
    if (wxyz_byte & 0x80)
    {
      if (wxyz_byte & 0x40)
	qlcerrlog(
	  ERRID_QLLC_ALERT_5,
	  "qllc",
	  station_ptr->user_ls_correlator,
	  NULL,
	  NULL
	  );
      else
	qlcerrlog(
	  ERRID_QLLC_ALERT_4,
	  "qllc",
	  station_ptr->user_ls_correlator,
	  NULL,
	  NULL
	  );
    }
    else if (wxyz_byte & 0x20)
	qlcerrlog(
	  ERRID_QLLC_ALERT_6,
	  "qllc",
	  station_ptr->user_ls_correlator,
	  NULL,
	  NULL
	  );
  }
  return;
}

/*****************************************************************************/
/* Function QLM_QLLU_I_FIELD_CHECK                                           */
/*                                                                           */
/* Description This function is called whenever a qllu is received, and      */
/*             it verifies that the i-field is valid for the type of qllu.   */
/*             If it finds an i-field on a qllu that shouldn't have one, it  */
/*             will generate Alert_9, and if it finds an I-field that is     */
/*             too long for the type of qllu, it will generate Alert_1.      */
/*                                                                           */
/*             Implementation note: Alert 1 is not checked on XID/TEST as it */
/*             would restrict the freedom of the remote DTE to send as much  */
/*             XID or TEST data as it requires on a QTEST_CMD or QXID_CMD.   */
/*             It is only checked for on qfrmr packets.                      */
/*****************************************************************************/
void  qlm_qllu_i_field_check(
  qllc_qllu_type *qllu,
  station_type   *station_ptr)
{
  if (qllu->address_field == 0xFF)
  {
    /*************************************************************************/
    /* There is no default domain on the next switch because Alerts 1 and 9  */
    /* only apply to the cases of known QLLUs. If the QLLU is of unknown type*/
    /* then Alert_8 will be generated by the qllc_errpk function.            */
    /*************************************************************************/
    switch (qllu->control_field)
    {
    case (qsm_cmd) :
    case (qdisc_cmd) :
    case (qrr_cmd) :
/* defect 108708 */
      if ((JSMBUF_LENGTH(qllu->info_field) - X25_OFFSETOF_USER_DATA) > 2)
/* end defect 108708 */
      {
	/*********************************************************************/
	/* The QLLU has an unwanted I-field associated with it.              */
	/*********************************************************************/
	qlcerrlog(
	  ERRID_QLLC_ALERT_9,
	  "qllc",
	  station_ptr->user_ls_correlator,
	  NULL,
	  NULL
	  );
      }
      break;
    }
  }
  else /* address_field != 0xFF, so this is a response */
  {
    switch (qllu->control_field)
    {
    case (qrd_rsp) :
    case (qua_rsp) :
    case (qdm_rsp) :
/* defect 108708 */
      if ((JSMBUF_LENGTH(qllu->info_field) - X25_OFFSETOF_USER_DATA) > 2)
/* end defect 108708 */
      {
	/*********************************************************************/
	/* The QLLU has an unwanted I-field associated with it.              */
	/*********************************************************************/
	qlcerrlog(
	  ERRID_QLLC_ALERT_9,
	  "qllc",
	  station_ptr->user_ls_correlator,
	  NULL,
	  NULL
	  );
      }
      break;
    case (qfrmr_rsp) :
/* defect 108708 */
      if ((JSMBUF_LENGTH(qllu->info_field) - X25_OFFSETOF_USER_DATA) > 5)
/* end defect 108708 */
      {
	/*********************************************************************/
	/* The QLLU has more than the 3 bytes allowed in a qfrmr, in addition*/
        /* to the address and control bytes.                                 */
	/*********************************************************************/
	qlcerrlog(
	  ERRID_QLLC_ALERT_1,
	  "qllc",
	  station_ptr->user_ls_correlator,
	  NULL,
	  NULL
	  );
      }
      break;
    }     
  }
  return;
}

/***************************************************************************/
/*   QLM_STATION_REMOTE_DISCONTACT -                                       */
/*        called when a remote station sends a discontact command          */
/*                                                                         */
/*        closes the virtual circuit and send reason_for_closure to        */
/*        remote_initiated_discontact                                      */
/*                                                                         */
/***************************************************************************/
void
qlm_station_remote_discontact(corr, stn_deleted)
correlator_type corr;
bool            *stn_deleted;
{

  station_type *station_ptr;
  boolean unlock;
  qvm_rc_type qvm_rc;

  outputf("QLM_STATION_REMOTE_DISCONTACT: corr = %x\n");

  /*
   * get and lock link station struct
   */
  station_ptr = qlm_find_ls_given_correlator(corr, &unlock);
  if (station_ptr == NULL)
  {
    outputf("QLM_STATION_REMOTE_DISCONTACT: no such station\n");
    return;
  }


  /***************************************************************************/
  /* Issue a Close to the QVM.                                               */
  /***************************************************************************/
/* defect 129081 */
  qvm_rc = qvm_close_vc(
	&(station_ptr->virt_circuit),
	QCM_RETURN_PORT_ID(station_ptr->channel_id),
	NORMAL_TERMINATION, /* diagnostic */
	FALSE);
/* end defect 129081 */
  if (qvm_rc != qvm_rc_ok)
  {
    station_ptr->reason_for_closure = (int)DLC_SYS_ERR;
    outputf("QLM_STATION_REMOTE_DISCONTACT: call delete station\n");
    qlm_delete_station(
      station_ptr,
      (correlator_type)NULL,
      NULL);
    *stn_deleted = TRUE;
  }
  else
  {
    outputf("QLM_STATION_REMOTE_DISCONTACT: station is closing\n");
    station_ptr->station_state = closing;
    station_ptr->reason_for_closure = remote_initiated_discontact;
    if (unlock) unlockl(&(station_ptr->lock));
  }

  return;
}
