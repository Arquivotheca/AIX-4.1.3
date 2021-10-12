static char sccsid[] = "@(#)91	1.4  src/bos/kernext/dlc/qlc/qlcvsess.c, sysxdlcq, bos411, 9428A410j 11/2/93 10:33:03";
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: qvm_open_vc, qvm_close_vc, qvm_vc_opened, qvm_vc_closed
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************/
/* Description: This file contains the operations described in QLLC design   */
/*              dealing with establishment/closure of logical sessions with  */
/*              the Device Handler. Each logical session corresponds to an   */
/*              X.25 virtual circuit.                                        */
/*****************************************************************************/

#include "qlcg.h"                   /* correlator_type                       */
	                            /* diag_tag_type                         */
	                            /* trace_channel_type                    */
	                            /* x25_address_type                      */
	                            /* lcn_type                              */
	                            /* ras_counter_type                      */
#include "qlcv.h"
#include "qlcvfac.h"                /* facilities types                      */
#include "qlcq.h"
#include "qlcb.h"                   /* gen_buffer_type                       */
#include "qlcvbuf.h"
#include "qlcp.h"                   /* port_type                             */
#include "qlcvpkt.h"
#include "qlcvsess.h"


/*****************************************************************************/
/* Function:    qvm_open_vc                                                  */
/*                                                                           */
/* Description: This function is used to request establishment of a virtual  */
/*              circuit with a specified remote DTE. Only outgoing (i.e.     */
/*              locally-initiated) virtual circuits are handled by this      */
/*              function, but either an SVC or a PVC may be requested.       */
/*              Separate function is provided for accepting or rejecting     */
/*              incoming calls.                                              */
/*                                                                           */
/*              A virtual circuit instance is provided by the caller for     */
/*              initialisation by the function. The port address is passed   */
/*              as a parameter, as this is required to call the QPM.         */
/*                                                                           */
/*              The caller also supplies an initialisation record containing */
/*              additional parameters relating to the virtual circuit to be  */
/*              opened.                                                      */
/*                                                                           */
/* Parameters:  virt_circuit    - a pointer to the X.25 virtual circuit      */
/*                                instance which is to be initialised.       */
/*              x25_port        - a pointer to the X.25 port instance.       */
/*              init_rec        - an initialistaion record containing        */
/*                                additional parameters. See the             */
/*                                x25_init_rec_type definition in qlcxdefs.h.*/
/*                                                                           */
/* Return: qvm_rc_port_error - an error occurred while in the QPM.           */
/*         qvm_rc_system_error- an error was reported by a system function.  */
/*         qvm_rc_ok          - the function completed successfully.         */
/*                                                                           */
/*****************************************************************************/
qvm_rc_type qvm_open_vc (

  struct qlc_sls_arg *qlc_ext_ptr,
  x25_vc_type     *virt_circuit,
  port_type       *x25_port,
  init_rec_type   *init_rec)
{
  struct x25_start_data  start_data; /* start_data arg for DH start ioctl    */
  qvm_rc_type qvm_rc = qvm_rc_ok;  /* return code from this function.        */
  gen_buffer_type *buffer_ptr;     /* ptr to claimed buffer                  */
  qpm_rc_type qpm_rc;              /* return code from port enqueue fn       */

  outputf("QVM_OPEN_VC: called\n");
  /***************************************************************************/
  /* Find out whether the circuit type is a PVC or an SVC.                   */
  /***************************************************************************/
  if (init_rec->circuit == session_pvc)
  {
    outputf("QVM_OPEN_VC: this is a PVC\n");
    /*************************************************************************/
    /* Initialise the fields in the VC for a PVC.                            */
    /*************************************************************************/
    virt_circuit->correlator = init_rec->correlator;
    virt_circuit->netid = init_rec->netid;
    virt_circuit->circuit = session_pvc;
    virt_circuit->protocol = init_rec->protocol;
    virt_circuit->state = xs_opening;
    virt_circuit->logical_channel = init_rec->channel_num;
    (void)strncpy (
      virt_circuit->session_name,
      init_rec->session_name,
      DIAG_TAG_LENGTH
      );

    /*************************************************************************/
    /* Initialise packet statistic RAS counters                              */
    /*************************************************************************/
    virt_circuit->data_packets_tx = 0;
    virt_circuit->data_packets_rx = 0;
    virt_circuit->invalid_packets_rx = 0;
    virt_circuit->adapter_rx_errors = 0;
    virt_circuit->adapter_tx_errors = 0;

    /*************************************************************************/
    /* There isn't a command extension buffer for opening a PVC.             */
    /* All you need is a Start data structure                                */
    /* The netid was selected by the QLM when the user issued the Start to   */
    /* QLLC. The session name was passed by the user, for diagnostic use,    */
    /* in the Start_LS parameter block.                                      */
    /* The session_id is not initialised yet. It is returned by the DH,      */
    /* consequently by the QPM, and is stored in the VC for later use.       */
    /* It is therefore passed by reference.                                  */
    /* The logical channel was also selected by the user in the Start LS     */
    /* parameter block.                                                      */
    /*************************************************************************/
    qpm_make_pvc_start_data (
      &(start_data),
      virt_circuit->netid,
      virt_circuit->session_name,
      virt_circuit->protocol,
      virt_circuit->logical_channel
      );
    buffer_ptr = NULL;
  }

  else if (init_rec->circuit == session_svc_out)
  {
    outputf("QVM_OPEN_VC: this is an svc\n");
    /*************************************************************************/
    /* If the circuit type is SVC, claim a buffer in which to build the call */
    /* request packet.                                                       */
    /*************************************************************************/
    outputf("QVM_OPEN: getting buffer, size=%d\n",MIN_BUF_SIZE);
    buffer_ptr = QBM_GET_BUFFER(MIN_BUF_SIZE);
    outputf("QVM_OPEN_VC: back from buffer get, buffer_ptr=%d\n",buffer_ptr);
    if (buffer_ptr == (gen_buffer_type *)NULL)
    {
      outputf("QVM_OPEN_VC: couldn't get buffer\n");
      qvm_rc = qvm_rc_system_error;
    }
    else /* if buffer claimed successfully */
    {
      outputf("QVM_OPEN_VC: got buffer\n");
      /***********************************************************************/
      /* Initialise the fields in the VC for an SVC.                         */
      /***********************************************************************/
      virt_circuit->netid = init_rec->netid;
      virt_circuit->correlator = init_rec->correlator;
      virt_circuit->circuit = session_svc_out;
      virt_circuit->protocol = init_rec->protocol;
      virt_circuit->state = xs_opening;
      virt_circuit->locally_initiated = TRUE;
      /***********************************************************************/
      /* This is an SVC, so logical_channel has no meaning.                  */
      /***********************************************************************/
      virt_circuit->logical_channel = 0;
      (void)strncpy (virt_circuit->session_name,init_rec->session_name,
	DIAG_TAG_LENGTH
	);
      /***********************************************************************/
      /* Initialise packet statistics RAS counters to zero.                  */
      /***********************************************************************/
      virt_circuit->data_packets_tx = 0;
      virt_circuit->data_packets_rx = 0;
      virt_circuit->invalid_packets_rx = 0;
      virt_circuit->adapter_rx_errors = 0;
      virt_circuit->adapter_tx_errors = 0;
      /***********************************************************************/
      /* Make a call request packet extension for opening an SVC.            */
      /***********************************************************************/
      outputf("QVM_OPEN_VC: call make_call_request_buffer function\n");
      outputf("QVM_OPEN_VC: calling addr [%s]\n",init_rec->recipient_address);
      (void)qvm_make_call_req_buffer (
        qlc_ext_ptr,	/* Defect 110313 */
	buffer_ptr,
	QPM_RETURN_LOCAL_ADDRESS (x25_port),
	init_rec->recipient_address,
	&init_rec->facilities,                         /* now a cb_fac_t ptr */
	init_rec->protocol,
	&(x25_port->dh_devinfo)
	);
      /***********************************************************************/
      /* Create a Start data structure.                                      */
      /* The netid was selected by the QLM when the user issued the Start to */
      /* QLLC. The session name was passed by the user, for diagnostic use,  */
      /* in the Start_LS parameter block.                                    */
      /* The session_id is not initialised yet. It is returned by the DH,    */
      /* consequently by the QPM, and is stored in the VC for later use.     */
      /* It is therefore passed by reference.                                */
      /* The protocol is derived from the support level requested by the     */
      /* user in the Start_LS parameter block.                               */
      /***********************************************************************/
      outputf("QVM_OPEN_VC: call make_svc_start_data function\n");
      qpm_make_svc_start_data (
	&(start_data),
	virt_circuit->netid,
	virt_circuit->session_name,
	SESSION_SVC_OUT,
	virt_circuit->protocol
	);
    }
  }
  else
  {
    outputf("QVM_OPEN_VC: this is a listening station..\n");
    outputf("QVM_OPEN_VC: listen_name = %s\n",init_rec->listen_name);
    /*************************************************************************/
    /* The station is a listener.                                            */
    /*************************************************************************/
    virt_circuit->correlator = init_rec->correlator;
    virt_circuit->netid = init_rec->netid;
    virt_circuit->circuit = session_svc_listen;
    strncpy(virt_circuit->listen_name, init_rec->listen_name, 8);
    virt_circuit->protocol = init_rec->protocol;
    virt_circuit->state = xs_opening;
    virt_circuit->locally_initiated = FALSE;
    /*************************************************************************/
    /* This is an SVC, so logical_channel has no meaning.                    */
    /*************************************************************************/
    virt_circuit->logical_channel = 0;
    (void)strncpy (
      virt_circuit->session_name,
      init_rec->session_name,
      DLC_MAX_DIAG
      );
    /*************************************************************************/
    /* Initialise packet statistics RAS counters to zero.                    */
    /*************************************************************************/
    virt_circuit->data_packets_tx = 0;
    virt_circuit->data_packets_rx = 0;
    virt_circuit->invalid_packets_rx = 0;
    virt_circuit->adapter_rx_errors = 0;
    virt_circuit->adapter_tx_errors = 0;
    /*************************************************************************/
    /* Make start data block                                                 */
    /*************************************************************************/
    outputf("QVM_OPEN_VC: making start data block\n");
    qpm_make_listen_start_data(
      &(start_data),
      virt_circuit->netid,
      virt_circuit->session_name,
      virt_circuit->protocol,
      virt_circuit->listen_name
      );
    outputf("QVM_OPEN_VC: setting buffer ptr to NULL\n");
    buffer_ptr = NULL;
  }
  /***************************************************************************/
  /* The start buffer for an SVC and the start_data blocks for either        */
  /* a PVC or an SVC (Calling or Listening) have been constructed.           */
  /* Call the QPM.                                                           */
  /***************************************************************************/
  outputf("QVM_OPEN_VC: calling QPM START function\n");
  qpm_rc = qpm_start (x25_port, buffer_ptr, &start_data);
  outputf("QVM_OPEN_VC: qpm_rc = %d\n",qpm_rc);
  switch (qpm_rc)
  {
  case qpm_rc_ok :
    qvm_rc = qvm_rc_ok;
    /*************************************************************************/
    /* If the start was successful, the xdh will have returned a session_id  */
    /*************************************************************************/
    outputf("QVM_OPEN_VC: new session id = 0x%x\n", start_data.session_id);
    virt_circuit->session_id = start_data.session_id;
    break;
  case qpm_rc_no_name :
    qvm_rc = qvm_rc_no_name;
    if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
    break;
  default :
    qvm_rc = qvm_rc_port_error;
    if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
    break;
  }
  return (qvm_rc);
}


/*****************************************************************************/
/* Function:    qvm_close_vc                                                 */
/*                                                                           */
/* Description: This function is used to initiate closure of a virt circuit  */
/*              with a specified remote DTE. The operation is valid for both */
/*              SVCs and PVCs.                                               */
/*                                                                           */
/*              The caller supplies the virt circuit instance to be closed,  */
/*              and the X.25 port, allowing a Halt command to be sent to the */
/*              Device Handler.                                              */
/*                                                                           */
/* Parameters:  virt_circuit    - a pointer to the X.25 virtual circuit      */
/*                                instance which is to be closed.            */
/*              x25_port        - a pointer to the X.25 port instance.       */
/*              diagnostic      - X.25 diagnsotic code to be inserted into   */
/*                                the clear req packet generated when this   */
/*                                function is invoked for an SVC.            */
/* Return Code: qvm_rc_port_error  - an error occurred in the QPM            */
/*              qvm_rc_system_error- an error was reported by a system func  */
/*              qvm_rc_ok          - the function completed successfully.    */
/*                                                                           */
/*****************************************************************************/
qvm_rc_type qvm_close_vc (

  x25_vc_type               *virt_circuit,
  port_type                 *x25_port,
  diag_code_type            diagnostic,
  boolean                   remote_clear)
{
  qvm_rc_type qvm_rc = qvm_rc_ok;    /* return code from this function.      */
  gen_buffer_type *buffer_ptr;        /* ptr to claimed buffer               */
  qpm_rc_type qpm_rc;                /* return code from port enqueue fn     */
  struct x25_halt_data halt_data;    /* halt data arg for the DH halt ioctl  */

  outputf("QVM_CLOSE_VC: called\n");
  /***************************************************************************/
  /* Find out whether the VC is a PVC, an SVC, or a listener.                */
  /***************************************************************************/
  if (virt_circuit->circuit == session_pvc 
    || virt_circuit->circuit == session_svc_listen)
  {
    outputf("QVM_CLOSE_VC: pvc or svc&listener\n");
    /*************************************************************************/
    /* There is no command extension buffer for a Halt command for a         */
    /* PVC or a listening station on an SVC.                                 */
    /* But there is Halt Data.                                               */
    /*************************************************************************/
    outputf("QVM_CLOSE_VC: make halt data\n");
    qpm_make_halt_data(
      &halt_data,
      virt_circuit->netid,
      virt_circuit->session_id
      );
    outputf("QVM_CLOSE_VC: buffer_ptr = NULL\n");
    buffer_ptr = NULL;
    /*************************************************************************/
    /* Change state to CLOSING.                                              */
    /*************************************************************************/
    virt_circuit->state = xs_closing;
  }
  else if (virt_circuit->circuit == session_svc_out 
    || virt_circuit->circuit == session_svc_in)
  {
    outputf("QVM_CLOSE_VC: session_svc_out/session_svc_in\n");
    /*************************************************************************/
    /* If the virtual circuit type is an SVC (in/out), claim a buf to build  */
    /* the Clear Request, or Clear Confirm packet in.                        */
    /*************************************************************************/
    buffer_ptr = QBM_GET_BUFFER(MIN_BUF_SIZE);
    if (buffer_ptr == (gen_buffer_type *)NULL)
    {
      outputf("QVM_CLOSE_VC: no buffer\n");
      qvm_rc = qvm_rc_system_error;
      virt_circuit->state = xs_closed;
    }
    else /* if buffer claimed successfully */
    {
      if (remote_clear == FALSE)
      {
	outputf("QVM_CLOSE_VC: local clear\n");
	/*********************************************************************/
	/* The call to qvm_close_vc was as a result of a requirement to      */
	/* issue a clear request from this DTE.                              */
	/*********************************************************************/
	/*********************************************************************/
	/* Build a command extension buffer for a Halt command on an         */
	/* an SVC.                                                           */
	/* Use the diagnostic passed in by the calling routine.              */
	/*********************************************************************/
	outputf("QVM_CLOSE_VC: make a clear_request\n");
 	(void)qvm_make_clear_req_buffer(buffer_ptr,diagnostic);
	virt_circuit->remote_clear = FALSE;
      }
      else
      {
	outputf("QVM_CLOSE_VC: remote clear\n");
	/*********************************************************************/
	/* The call to qvm_close_vc was as a result of a requirement to      */
	/* issue a clear confirm from this DTE in response to an incoming    */
	/* clear from the remote, either when the vc was started or in       */
	/* immediate response to the Call Request.                           */
	/*********************************************************************/
	/*********************************************************************/
	/* Build a command extension buffer for a Halt command on an         */
	/* an SVC.                                                           */
	/* Use the diagnostic passed in by the calling routine.              */
	/*********************************************************************/
	outputf("QVM_CLOSE_VC: make a clear_confirm\n");
	(void)qvm_make_clear_conf_buffer(buffer_ptr);
	virt_circuit->remote_clear = TRUE;
      }
      /***********************************************************************/
      /* Build Halt Data structure, too.                                     */
      /***********************************************************************/
      outputf("QVM_CLOSE_VC: make halt_data\n");
      qpm_make_halt_data(
	&halt_data,
	virt_circuit->netid,
	virt_circuit->session_id
	);
      /***********************************************************************/
      /* Change state to CLOSING.                                            */
      /***********************************************************************/
      virt_circuit->state = xs_closing;
    }
  }
  /***************************************************************************/
  /* The halt data blocks for SVC/PVC and in the case of an SVC, the         */
  /* buffer have been built. Call the QPM.                                   */
  /***************************************************************************/
  outputf("QVM_CLOSE_VC: call qpm_halt()\n");
  qpm_rc = qpm_halt(x25_port, &halt_data, buffer_ptr);
  if (qpm_rc != qpm_rc_ok)
  {
    outputf("QVM_CLOSE_VC: qpm_halt_rc = %d\n",qpm_rc);
    if (buffer_ptr != NULL) 
      QBM_FREE_BUFFER(buffer_ptr);
    qvm_rc = qvm_rc_port_error;
    virt_circuit->state = xs_closed;
  }
  return (qvm_rc);
}


/*****************************************************************************/
/* Function:    qvm_accept_incoming_call                                     */
/*                                                                           */
/* Description: This function is used to accept an incoming call             */
/*              notified to QLLC by the Device Handler. The caller supplies  */
/*              an uninitialised virtual circuit for initialisation by       */
/*              this function. The X.25 port must also be supplied to allow a*/
/*              Start command to be sent to the Device Handler.              */
/*              Incoming Calls are accepted by issuing a Start with the      */
/*              temporary call_id the DH attributed to the incoming call,    */
/*              on the session_id attributed at listen.                      */
/*              The session type is SVC IN, so the session data contains the */
/*              call_id, and on return the session_id will be a new permanent*/
/*              value (overwriting the session_id in the start data).        */
/*                                                                           */
/* Parameters:  virt_circuit    - a pointer to the X.25 virtual circuit      */
/*                                instance which is to be initialised.       */
/*              x25_port        - a pointer to the X.25 port instance.       */
/*              buff_ptr        - a pointer to the data buffer containing the*/
/*                                incoming call packet.                      */
/*              session_name    - a diagnostic tag by which this session will*/
/*                                known in error and trace log entries.      */
/*              correlator      - a copy of the QLLC link station correlator,*/
/*                                needed to identify this station to error   */
/*                                logging functions.                         */
/*                                                                           */
/* Return Code: qvm_rc_port_error  - an error occurred in the QPM.           */
/*              qvm_rc_system_error- an error was reported by a system       */
/*                                   function                                */
/*              qvm_rc_ok          - the function completed successfully.    */
/*                                                                           */
/*****************************************************************************/
qvm_rc_type qvm_accept_incoming_call (

  x25_vc_type     *virt_circuit,
  port_type       *x25_port,
  gen_buffer_type *buffer_ptr,
  diag_tag_type    session_name,
  correlator_type  correlator)
{
  qvm_rc_type  qvm_rc = qvm_rc_ok;    /* return code from this function.     */
  qpm_rc_type  qpm_rc;                /* return code from port enqueue fn    */
  cb_fac_t     cb_fac;
  byte        *cb_fac_ovf_ptr=NULL;
  byte         fac_stream[X25_MAX_FACILITIES_LENGTH];
  unsigned int fac_len;
  int          i;
  struct x25_start_data start_data;  /* start data for DH Start              */

  if (QBM_RETURN_FACILITIES_LENGTH(buffer_ptr) > 0)
  {
    /************************************************************************/
    /* Initialise the fac_stream, which is a contiguous work area for facs. */
    /************************************************************************/
    for (i=0; i<X25_MAX_FACILITIES_LENGTH; i++)
    fac_stream[i] = '\0';

    /*************************************************************************/
    /* Copy the byte stream from the buffer into the contiguous work area.   */
    /*************************************************************************/
    QBM_RETURN_BLOCK(
      buffer_ptr,
      X25_OFFSETOF_FAC_CUD_DATA,
      fac_stream,
      (unsigned int)QBM_RETURN_FACILITIES_LENGTH(buffer_ptr)
      );
    
    /*************************************************************************/
    /* The fac stream derived from the incoming call buffer is converted into*/
    /* a cb_fac structure by calling the facilities parser, so that it can be*/
    /* easily.                                                               */
    /*************************************************************************/
    (void)_x25_convert_byte_stream_to_cb_fac(
      &cb_fac,
      cb_fac_ovf_ptr,
      fac_stream,
      (unsigned int)QBM_RETURN_FACILITIES_LENGTH(buffer_ptr)
      );
    /*************************************************************************/
    /* The cb_fac structure can now be used to examine the facilities if     */
    /* required. Presently no requirement is seen for negotiation of facs,   */
    /* by this procedure, so none is done.....any future requirements should */
    /* be incorporated here.                                                 */
    /* Something that QLLC must do is remove any non-negotiable facilities   */
    /* from the set requested in the incoming call buffer.                   */
    /* Only the window size and packet size are negotiable, so clear out all */
    /* the other fields.                                                     */
    /*************************************************************************/
    cb_fac.flags = 0;
    cb_fac.flags |= X25FLG_PSIZ;
    cb_fac.flags |= X25FLG_WSIZ;
    /*************************************************************************/
    /* Now put the facilities structure back into the buffer.                */
    /* Initialise the fac_stream, which is a contiguous work area for facs.  */
    /*************************************************************************/
    for (i=0; i<X25_MAX_FACILITIES_LENGTH; i++)
      fac_stream[i] = '\0';
    fac_len = _x25_convert_cb_fac_to_byte_stream(
      fac_stream,
      &cb_fac,
      &(x25_port->dh_devinfo)
      );
    QBM_SET_BLOCK(
      buffer_ptr,
      (unsigned int)X25_OFFSETOF_FAC_CUD_DATA,
      fac_stream,
      fac_len
      );
  }
  /*************************************************************************/
  /* The virtual circuit instance must be initialised from the info in     */
  /* the parameter list.                                                   */
  /*************************************************************************/
  virt_circuit->correlator = correlator;
  virt_circuit->circuit = session_svc_in;
  virt_circuit->state = xs_opening;
  (void)strncpy (
    virt_circuit->session_name,
    session_name,
    DIAG_TAG_LENGTH
    );
  virt_circuit->data_packets_tx = 0;
  virt_circuit->data_packets_rx = 0;
  virt_circuit->invalid_packets_rx = 0;
  virt_circuit->adapter_rx_errors = 0;
  virt_circuit->adapter_tx_errors = 0;
  virt_circuit->locally_initiated = FALSE;
  /***************************************************************************/
  /* A Start Data block is constructed and passed with the                   */
  /* buffer to the QPM_Start function.                                       */
  /***************************************************************************/
  (void)qpm_make_svc_start_data (
    &start_data,
    virt_circuit->netid,
    virt_circuit->session_name,
    SESSION_SVC_IN,
    virt_circuit->protocol,
    QBM_RETURN_CALL_ID(buffer_ptr)
    );
  /***************************************************************************/
  /* The incoming call buffer is modified for use as a command ext to the    */
  /* Accept Incoming Call command.                                           */
  /* Note that it's facilities have already been "negotiated".               */
  /***************************************************************************/
  (void)qvm_make_call_accept_buffer(buffer_ptr);
  /***************************************************************************/
  /* The buffer contains a Call Accepted packet.                             */
  /***************************************************************************/
  qpm_rc = qpm_start (x25_port, buffer_ptr, &start_data);
  if (qpm_rc != qpm_rc_ok)
  {
    QBM_FREE_BUFFER(buffer_ptr);
    qvm_rc = qvm_rc_port_error;
  }
  return (qvm_rc);
}



/*****************************************************************************/
/* X.25 REJECT INCOMING CALL                                                 */
/* Function:    qvm_reject_incoming_call                                     */
/*                                                                           */
/* Description: This function is used to reject an incoming call which has   */
/*              been notified to QLLC by the Device Handler.                 */
/*              The caller supplies a pointer to the buffer containing the   */
/*              incoming call indication, and a diagnostic code to be        */
/*              placed in the clear request generated. The X.25 port must    */
/*              also be supplied to allow a command to be sent to the        */
/*              Device Handler.                                              */
/*                                                                           */
/* Parameters:  x25_port        - a pointer to the X.25 port instance.       */
/*              buff_ptr        - a pointer to the data buffer containing the*/
/*                                incoming call packet.                      */
/*              diagnostic      - the diagnostic code to be placed in the    */
/*                                clear request packet generated as a result */
/*                                of this function.                          */
/*                                                                           */
/* Return Code: qvm_rc_port_error  - an error occurred in the QPM            */
/*              qvm_rc_ok          - the function completed successfully.    */
/*                                                                           */
/*****************************************************************************/
qvm_rc_type qvm_reject_incoming_call (

  x25_vc_type          *virt_circuit,
  port_type            *x25_port,
  gen_buffer_type      *buffer_ptr,
  diag_code_type        diagnostic)
{
  /***************************************************************************/
  /* The following steps are necessary to reject an incoming call:           */
  /* 1. Construct a Reject Data area for use as a DH arg.                    */
  /* 2. Alter the incoming call buffer to make it suitable for use as a      */
  /*    clear request                                                        */
  /* 3. Call the QPM                                                         */
  /***************************************************************************/
  struct x25_reject_data  reject_data; /* used as dh arg                     */
  qvm_rc_type qvm_rc = qvm_rc_ok;    /* return code from this function.      */
  qpm_rc_type qpm_rc;                /* return code from port enqueue fn     */

  /***************************************************************************/
  /* Must make reject data BEFORE adjusting buffer, because you need the     */
  /* call_id out of the buffer, and so must get it before making the buffer  */
  /* a Clear Request                                                         */
  /***************************************************************************/
  outputf("QVM_REJECT_INCOMING_CALL: make reject data\n");
  (void)qpm_make_reject_data (
    &reject_data,
    virt_circuit->netid,
    virt_circuit->session_id,
    QBM_RETURN_CALL_ID(buffer_ptr)
    );

  outputf("QVM_REJECT_INCOMING_CALL: make clear request buffer\n");
  (void)qvm_make_clear_req_buffer (
    buffer_ptr,
    diagnostic
    );

  outputf("QVM_REJECT_INCOMING_CALL: call qpm reject\n");
  qpm_rc = qpm_reject (x25_port, &reject_data, buffer_ptr);
  if (qpm_rc != qpm_rc_ok)
  {
    outputf("QVM_REJECT_INCOMING_CALL: qpm reject failed\n");
    QBM_FREE_BUFFER(buffer_ptr);
    qvm_rc = qvm_rc_port_error;
  }
  return (qvm_rc);
}


/*****************************************************************************/
/* X.25 VIRTUAL CIRCUIT OPENED                                               */
/* Function:    qvm_vc_opened                                                */
/*                                                                           */
/* Description: This function is called when a Start Done status block is    */
/*              received from the Device Handler.                            */
/*                                                                           */
/* Parameters:  virt_circuit    - a pointer to the X.25 virtual circuit      */
/*                                instance which has been opened.            */
/*              buff_ptr        - a pointer to the data buffer containing the*/
/*                                Call Accepted packet.                      */
/*                                                                           */
/* Return Code: x25_program_error-an error was detected which must have been */
/*                                caused by a programming error, e.g. a      */
/*                                virtual circuit was in an invalid state for*/
/*                                the operation.                             */
/*              qvm_rc_ok        - the function completed successfully.      */
/*                                                                           */
/*****************************************************************************/
qvm_rc_type qvm_vc_opened (
  x25_vc_type        *virt_circuit)
{
  qvm_rc_type qvm_rc = qvm_rc_ok;

  if (virt_circuit->state != xs_opening)
  {
    outputf("QVM_VC_OPENED: state not opening\n");
    /*************************************************************************/
    /* virtual circuit is in the wrong state, report a program error         */
    /*************************************************************************/
    qvm_rc = qvm_rc_program_error;
  }
  else /* if (virt_circuit->state == xs_opening)                             */
  {
    outputf("QVM_VC_OPENED: state opening\n");
    /*************************************************************************/
    /* Virtual circuit is in the correct state. Change its state to OPENED.  */
    /*************************************************************************/
    virt_circuit->state = xs_opened;
  }
  return (qvm_rc);
}

/*****************************************************************************/
/* X.25 VIRTUAL CIRCUIT CLOSED                                               */
/* Function:    qvm_vc_closed                                                */
/*                                                                           */
/* Description: This function is called when an acknowledgment of an Close   */
/*              Logical Session command or an unsolicited Session Closed     */
/*              indication is received from the VRM device driver.           */
/*                                                                           */
/* Parameters:  virt_circuit    - a pointer to the X.25 virtual circuit      */
/*                                instance which has been opened.            */
/*              buff_ptr        - a pointer to the data buffer containing the*/
/*                                Session Closed indication.                 */
/*                                                                           */
/* Return Code: qvm_rc_ok          - the function completed successfully.    */
/*                                                                           */
/*****************************************************************************/
qvm_rc_type qvm_vc_closed (

  x25_vc_type           *virt_circuit,
  gen_buffer_type       *buff_ptr)
{
  qvm_rc_type qvm_rc = qvm_rc_ok;

  if ( (virt_circuit->state == xs_closing)
    && (virt_circuit->remote_clear == FALSE))
  {
    /*************************************************************************/
    /* The halt was issued as a result of a local clear condition. A Clear   */
    /* Request was sent and the buffer contains the Clear Confirm. Free it.  */
    /*************************************************************************/
    if (buff_ptr != NULL) QBM_FREE_BUFFER(buff_ptr);
  }
  virt_circuit->state = xs_closed;
  return (qvm_rc);
}
