static char sccsid[] = "@(#)89  1.7  src/bos/kernext/dlc/qlc/qlcvpkt.c, sysxdlcq, bos411, 9428A410j 2/26/94 14:07:21";
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: qvm_clrst, qvm_confirm_reset, qvm_send_dllu, qvm_send_qllu
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
#include "qlcg.h"                   /* correlator_type                       */
	                            /* diag_tag_type                         */
	                            /* trace_channel_type                    */
	                            /* x25_address_type                      */
	                            /* lcn_type                              */
	                            /* ras_counter_type                      */
#include "qlcq.h"
#include "qlcv.h"
#include "qlcvfac.h"                /* facilities types                      */
#include "qlcb.h"                   /* gen_buffer_type                       */
#include "qlcvbuf.h"
#include "qlcp.h"                   /* port_type                             */
#include "qlcvpkt.h"

/*****************************************************************************/
/* X.25 CLEAR SVC / RESET PVC                                                */
/* Function:    qvm_clrst                                                    */
/*                                                                           */
/* Description: This function is used to send a Clear Request on an SVC or a */
/*              Reset Request on a PVC. The caller supplies a virt circuit   */
/*              instance and port instance for modification by the function. */
/*              A diagnostic code specified by the caller is placed in the   */
/*              Clear/Reset packet.                                          */
/* Return    qvm_rc_port_error  - an error occurred during execution by the  */
/*                                QPM of the Write/Halt command.             */
/*           qvm_rc_system_error- an error was reported by a system function */
/*                                For example an attempt to claim a buffer   */
/*           qvm_rc_ok          - the function completed successfully.       */
/*                                                                           */
/* Parameters:  virt_circuit    - a pointer to the X.25 virtual circuit      */
/*                                instance on which the data is to be sent.  */
/*              x25_port        - a pointer to the X.25 port instance.       */
/*              diagnostic      - the diagnostic code to be placed in the    */
/*                                clear / reset packet.                      */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
qvm_rc_type qvm_clrst (

  x25_vc_type                *virt_circuit,
  port_type                  *x25_port,
  diag_code_type             diagnostic)
{
  gen_buffer_type *buffer_ptr;         /* pointer to a buffer from the pool  */
  struct x25_write_ext  write_ext;
  struct x25_halt_data  halt_data;     /* used for Clears on SVCs            */
  qvm_rc_type qvm_rc;                  /* return code from this function     */
  qpm_rc_type qpm_rc;                  /* return code from port function     */

  qvm_rc = qvm_rc_ok;
  /***************************************************************************/
  /* Claim the buffer which will be used to hold the Clear / Reset packet    */
  /***************************************************************************/
  buffer_ptr = QBM_GET_BUFFER(sizeof(gen_buffer_type));
  if (buffer_ptr == (gen_buffer_type *)NULL)
  {
    qvm_rc = qvm_rc_system_error;
  }
  else
  {
    /*************************************************************************/
    /* At this stage we know that QBM_GET_BUFFER was ok. We have to build a  */
    /* command and a data buffer. The content of the buffer, and the call to */
    /* the QPM depend on whether this is a PVC or SVC.                       */
    /*************************************************************************/
    if (virt_circuit->circuit == session_pvc)
    {
      /***********************************************************************/
      /* For a PVC, we will be sending a Reset, by calling QPM_Write, since  */
      /* a RESET does not halt the session.                                  */
      /***********************************************************************/
      buffer_ptr = qvm_make_reset_buffer (
	buffer_ptr,
	diagnostic
	);
      (void)qpm_make_write_ext (
	&(write_ext),
	virt_circuit->session_id,
	virt_circuit->netid
	);
       outputf ("QVM_CLRST  CALLING qpm_write \n");
       qpm_rc = qpm_write(x25_port, buffer_ptr, &write_ext);
    }
    else /* if (virt_circuit->circuit == session_svc_in/out) */
    {
      /***********************************************************************/
      /* For an SVC, we will be sending a Clear, which can only be done      */
      /* by issuing a Halt via QPM_Halt().                                   */
      /***********************************************************************/
      buffer_ptr = qvm_make_clear_req_buffer (
	buffer_ptr,
	diagnostic
	);
      (void)qpm_make_halt_data (
	&(halt_data),
	virt_circuit->netid,
	virt_circuit->session_id
	);
      outputf ("QVM_CLRST  CALLING qpm_halt \n");
      qpm_rc = qpm_halt(x25_port, &(halt_data), buffer_ptr);
    }
    /*************************************************************************/
    /* Whichever call to QPM was made, if it failed, then the return code    */
    /* from this function is set to qvm_rc_port_error. The buffer which was  */
    /* claimed and initialised is freed by the Device Handler.               */
    /*************************************************************************/
    if (qpm_rc != qpm_rc_ok)
    {
      qvm_rc = qvm_rc_port_error;
      /* QBM_FREE_BUFFER(buffer_ptr); */
    }
  }
  return (qvm_rc);
}


/*****************************************************************************/
/* X.25 CONFIRM RESET                                                        */
/* Function:    x25_confirm_reset                                            */
/*                                                                           */
/* Description: This function is used to send a Reset Confirmation packet.   */
/*              This will happen when a Reset Indication is received on a    */
/*              PVC.                                                         */
/*              Resets are not allowed by SNA on SVCs, but this function is  */
/*              still called for an SVC, so that a confirm can be sent to    */
/*              the Device Handler, before issuing a halt.  This is to avoid */
/*              protocol errors.                                             */
/*              The buffer used to send the packet to the Device Handler     */
/*              is the same one in which the Reset indication was received,  */
/*              so only minor changes are needed.                            */
/*                                                                           */
/* Parameters:  virt_circuit    - a pointer to the X.25 virtual circuit      */
/*                                instance on which the packet is to be sent.*/
/*              x25_port        - a pointer to the X.25 port instance.       */
/*              buffer_ptr      - a pointer to the buffer in which the reset */
/*                                indication was received.                   */
/*                                                                           */
/* Return    qvm_rc_port_error  - an error occurred during execution by the  */
/*                                QPM of the Write command.                  */
/*           qvm_rc_ok          - the function completed successfully.       */
/*                                                                           */
/*****************************************************************************/
qvm_rc_type qvm_confirm_reset (

  x25_vc_type     *virt_circuit,
  port_type       *x25_port,
  gen_buffer_type *buffer_ptr)
{
  struct x25_write_ext   write_ext;
  qvm_rc_type qvm_rc;             /* return code from this function   */
  qpm_rc_type qpm_rc;             /* return code from port function   */

  qvm_rc = qvm_rc_ok;

  /***************************************************************************/
  /* In order to send the reset confirmation, we need a buffer and a write   */
  /* extension. The only change needed to convert a reset indication buffer  */
  /* into a reset confirmation is to set the Packet_Type field in the buffer */
  /* packet_data area to PKT_RESET_CONFIRM.                                  */
  /***************************************************************************/
  QBM_SET_PACKET_TYPE(buffer_ptr,PKT_RESET_CONFIRM);

  (void)qpm_make_write_ext (
    &(write_ext),
    virt_circuit->session_id,
    virt_circuit->netid
    );

  /***************************************************************************/
  /* Now the write extension and buffer have been constructed for the reset  */
  /* confirmation. The command must now be sent to the QPM using qpm_write.  */
  /* If QPM fails, then the return code from this function is set to         */
  /* qvm_rc_port_error. The buffer which was supplied as a parameter must be */
  /* freed.                                                                  */
  /***************************************************************************/
  outputf ("QVM_CONFIRM_RESET: calling qpm_write \n");
  qpm_rc = qpm_write (x25_port, buffer_ptr, &write_ext);
  if (qpm_rc != qpm_rc_ok)
  {
    qvm_rc = qvm_rc_port_error;
    QBM_FREE_BUFFER(buffer_ptr);
  }
  return (qvm_rc);
}


/*****************************************************************************/
/* X.25 SEND PACKET BUFFER                                                   */
/* Function:    qvm_send_buffer                                              */
/*                                                                           */
/* Description: This function is used to send a buffer of data on the X.25   */
/*              virtual circuit. The caller supplies an instance of an X.25  */
/*              virtual circuit and an X.25 port, which will be modified.    */
/*              The caller also supplies the data to be sent. This function  */
/*              provides a common means of access for the send_qllu and send */
/*              dllu functions. Either kind of data can be sent              */
/*              using this routine. The function is only used internally     */
/*              within the X.25 Virtual Circuit Manager.                     */
/*                                                                           */
/*              The QLLC FSM treats buffer_ptr as a byte *, but since the    */
/*              QLM and QVM agree to pass the whole buffer through the FSM,  */
/*              it never gets to realise that what it actually has is a ptr  */
/*              to the whole buffer                                          */
/*                                                                           */
/* Parameters:  virt_circuit    - a pointer to the X.25 virtual circuit      */
/*                                instance on which the data is to be sent.  */
/*              x25_port        - a pointer to the X.25 port instance.       */
/*              data            - a ptr to a buffer containing the data to   */
/*                                be sent.                                   */
/*              q_bit           - a boolean value indicating whether  Q bit  */
/*                                should be set                              */
/*              free_buffer     - a boolean value indicating whether the     */
/*                                supplied buffer should be freed on compl.  */
/*                                of this routine (or by the device handler  */
/*                                after it has written the data).            */
/*                                                                           */
/* Return    qvm_rc_port_error  - an error occurred while in the QPM.        */
/*           qvm_rc_system_error- an error was reported by a system function */
/*           qvm_rc_ok          - the function completed successfully.       */
/*                                                                           */
/*****************************************************************************/
qvm_rc_type qvm_send_buffer (

  x25_vc_type            *virt_circuit,
  port_type              *x25_port,
  gen_buffer_type        *buffer_ptr,
  bool                    q_bit,
  bool                    free_buffer)
{
  /***************************************************************************/
  /* The QPM_Write function is called to request transmission of the data    */
  /* to the DH.                                                              */
  /***************************************************************************/
  qvm_rc_type qvm_rc;          /* return code from this function.       */
  qpm_rc_type qpm_rc;          /* return code from X.25 Port function.  */
  struct x25_write_ext  write_ext;

  qpm_rc = qpm_rc_ok;
  qvm_rc = qvm_rc_ok;
  /***************************************************************************/
  /* Initialise the write extension using qpm_make_write_ext().              */
  /***************************************************************************/
  (void)qpm_make_write_ext(
    &write_ext,
    virt_circuit->session_id,
    virt_circuit->netid
    );
  /***************************************************************************/
  /* The buffer supplied by the caller will be used, so we don't need to     */
  /* claim a new buffer.  But some alteration of the header is needed.       */
  /* This is done by qvm_make_write_buffer(), which modifies the Q-bit in    */
  /* buffer if appropriate.                                                  */
  /***************************************************************************/
  (void)qvm_make_write_buffer(buffer_ptr,q_bit);

  /***************************************************************************/
  /* If caller requested freeing of the buffer on completion then free it    */
  /* then let the XDH free it. If the caller requested that the buffer not   */
  /* be freed, then make sure the XDH will not free it.                      */
  /* This facility is provided to support the repolling of QLLC commands,    */
  /* which are held in the link station's pending command queue until the    */
  /* station decides to not repoll them any more times, or until the         */
  /* response is received.                                                   */
  /***************************************************************************/
  if (free_buffer == TRUE)
    write_ext.we.flag &= ~CIO_NOFREE_MBUF;
  else
    write_ext.we.flag |= CIO_NOFREE_MBUF;

  outputf("QVM_SEND_BUFFER: calling qpm_write \n");
  qpm_rc = qpm_write(x25_port,buffer_ptr,&write_ext);

  if (qpm_rc != qpm_rc_ok)
  {
    qvm_rc = qvm_rc_port_error;
    outputf("QVM_SEND_BUFFER: qpm_write return code = %d\n",qpm_rc);
  }
  else
  {
    /*************************************************************************/
    /* The RAS counters should be updated here to reflect the packets that   */
    /* have just been sent. However the mechanism for getting this info from */
    /* the DH is not yet known.                                              */
    /*************************************************************************/
  }
  return (qvm_rc);
}


/*****************************************************************************/
/* X.25 SEND UNQUALIFIED DATA                                                */
/* Function:    qvm_send_dllu                                                */
/*                                                                           */
/* Description: This function is used to send unqualified data on the X.25   */
/*              virtual circuit. The caller supplies an instance of an X.25  */
/*              virtual circuit and an X.25 port, which will be modified.    */
/*              The caller also supplies the data to be sent. In the design  */
/*              the data supplied is modelled as a vstring. In the code, the */
/*              data will be supplied as a buffer pointer, since this is the */
/*              way unqualified data will be received from SNA.              */
/*                                                                           */
/*              The virtual circuit should be in the OPENED state, as the    */
/*              QLLC link station which invoked this operation must have     */
/*              been in the OPENED state. However, if the virtual            */
/*              circuit has been cleared since the command from SNA was      */
/*              accepted for processing, then the data should not be issued. */
/*              This is because the DH can re-use session_id's and may have  */
/*              established a new session with the same id, in which case the*/
/*              data would be transmitted on the wrong session, but there    */
/*              would be no way of knowing this.                             */
/*                                                                           */
/* Parameters:  virt_circuit    - a pointer to the X.25 virtual circuit      */
/*                                instance on which the data is to be sent.  */
/*              x25_port        - a pointer to the X.25 port instance.       */
/*              data            - a ptr to a buffer containing the data to   */
/*                                be sent. This is declared as a (byte *) in */
/*                                the formal parameter list, so that the QLLC*/
/*                                link station manager need not know about   */
/*                                VRM buffers. It will be cast to a buffer   */
/*                                before use.                                */
/*                                                                           */
/* Return    qvm_rc_port_error  - an error occurred in the QPM               */
/*           qvm_rc_system_error- an error was reported by a system function */
/*                                for example an attempt to claim buffers.   */
/*           qvm_rc_ok          - the function completed successfully.       */
/*                                                                           */
/*****************************************************************************/
qvm_rc_type qvm_send_dllu (

  x25_vc_type            *virt_circuit,
  port_type              *x25_port,
  gen_buffer_type        *buffer_ptr)
{
  qvm_rc_type qvm_rc;        /* return code from this function        */

  /*************************************************************************/
  /* If the virt_circuit is not OPENED do not send the data.               */
  /*************************************************************************/
  if (virt_circuit->state != xs_opened)
  {
    return(qvm_rc_circuit_not_opened);
  }

  /***************************************************************************/
  /* Set QLLC Address and Control fields to zero.                            */
  /***************************************************************************/
/*
  QBM_SET_QLLC_ADDRESS_FIELD(buffer_ptr,0);
  QBM_SET_QLLC_CONTROL_FIELD(buffer_ptr,0);
*/
  outputf ("QVM_SEND_DLLU  CALLING qvm_send_buffer  \n");
  qvm_rc = qvm_send_buffer (
    virt_circuit,
    x25_port,
    buffer_ptr,
    (bool)FALSE,               /* setting of Q bit                          */
    (bool)TRUE                 /* don't free the buffer                     */
    );
  return (qvm_rc);
}

/*****************************************************************************/
/* X.25 SEND QUALIFIED LOGICAL LINK UNIT                                     */
/* Function:    qvm_send_qllu                                                */
/*                                                                           */
/* Description: This function is used to send qualified data on the X.25     */
/*              virtual circuit. The caller supplies an instance of an X.25  */
/*              virtual circuit and an X.25 port, which will be modified.    */
/*              The caller also supplies the QLLU to be sent. This is held in*/
/*              a structure of type qllc_qllu_type. If the QLLU to be sent   */
/*              includes an info field (QXID, QTEST) then the info field of  */
/*              the QLLU structure holds a pointer to a data buffer. If the  */
/*              QLLU does not include an info field, then the info field     */
/*              pointer in the structure is set to NULL. In this case, a     */
/*              block I/O buffer must be claimed from the pool in order to   */
/*              send the qllu.                                               */
/*                                                                           */
/*              The virtual circuit should be in the OPENED state, as the    */
/*              QLLC link station which invoked this operation must have     */
/*              been in the OPENED state. However, if the virtual            */
/*              circuit has been cleared since the command from SNA was      */
/*              accepted for processing, then the data should not be issued. */
/*              This is because the DH can re-use session_id's and may have  */
/*              established a new session with the same id, in which case the*/
/*              data would be transmitted on the wrong session, but there    */
/*              would be no way of knowing this.                             */
/*                                                                           */
/*                                                                           */
/*              The function also preserves command polls where possible, so */
/*              that they do not need to be reconstructed if a repoll is     */
/*              necessary. This involves setting the 'already_sent' flag in  */
/*              the qllu, and saving the command buffer (compl with two byte */
/*              qllc header) in the info field of the qllu.                  */
/*                                                                           */
/* Parameters:  virt_circuit    - a pointer to the X.25 virtual circuit      */
/*                                instance on which the data is to be sent.  */
/*              x25_port        - a pointer to the X.25 port instance.       */
/*              qllu            - a pointer to a structure of qllc_qllu_type,*/
/*                                indicating the QLLU to be sent.            */
/*                                                                           */
/* Return    qvm_rc_port_error  - an error occurred in the QPM               */
/*           qvm_rc_system_error- an error was reported by a system function */
/*                                for example an attempt to claim buffers    */
/*           qvm_rc_ok          - the function completed successfully.       */
/*                                                                           */
/*****************************************************************************/
qvm_rc_type qvm_send_qllu (

  x25_vc_type            *virt_circuit,
  port_type              *x25_port,
  qllc_qllu_type         *qllu)
{
#define M_COPYM(To,From) \
    { \
	register struct mbuf *m = From, *n, **np; \
	struct mbuf *top; \
	\
	np = &top; \
	top = 0; \
	do { \
		n = QBM_GET_BUFFER(m->m_len); \
		if (n == 0) { \
			if (top) \
				m_freem(top); \
			qvm_rc = qvm_rc_system_error; \
			return (qvm_rc); \
		} \
		*np = n; \
		n->m_len = m->m_len; \
		bcopy(mtod(m, caddr_t), mtod(n, caddr_t), (unsigned)n->m_len); \
		np = &n->m_next; \
	} while (m = m->m_next); \
	To = top; \
    }

  qvm_rc_type      qvm_rc;          /* return code from this function.       */
  gen_buffer_type *buffer_ptr;      /* a buffer pointer                      */
  gen_buffer_type *DupBufferPtr;    /* a copy of above buffer pointer        */
  gen_buffer_type *chained_mbuf_ptr;/* a copy of chained mbuf                */


  qvm_rc = qvm_rc_ok;              /* initialise this function's return code */

  /***************************************************************************/
  /* There are two cases to consider. If the qllu.already_sent flag is TRUE  */
  /* (indicating that this is a repoll) then we simply need to re-submit     */
  /* the buffer attached as the info field of the qllu. Otherwise, we have   */
  /* to build a new buffer.                                                  */
  /***************************************************************************/
  if (qllu->already_sent == TRUE)
  {
    /*********************************************************************/
    /* Defect 103650 - Make a copy of the buffer associated to this QLLU */
    /* and call qvm_send_buffer with this new buffer with the option to  */
    /* free the buffer after sending it.	 			 */
    /*********************************************************************/

    M_COPYM (DupBufferPtr, qllu->info_field);

    outputf ("QVM_SEND_QLLU: calling qvm_send_buffer to repoll a command.\n");

    qvm_rc = qvm_send_buffer (
      virt_circuit,
      x25_port,
      DupBufferPtr,
      (bool)TRUE,                /* setting of Q bit                      */
      (bool)TRUE                 /* free the buffer                 	  */
      );

    /* End of Defect 103650 */
  }
  else /* if (qllu->already_sent == FALSE) */
  {
    /*************************************************************************/
    /* If the info field is not NULL, then this is an XID or TEST command    */
    /* and there is already a buffer associated with the QLLU.               */
    /* If the info_field is NULL, then there is no buffer associated with    */
    /* this QLLU. In this case a buffer must be claimed from the pool.       */
    /*************************************************************************/
    if (qllu->info_field != NULL)
    {
      buffer_ptr = (gen_buffer_type *)(qllu->info_field);
      outputf("QVM_SEND_QLLU: buffer associated with first time poll.\n");
      outputf("QVM_SEND_QLLU: buffer_ptr = %x\n",buffer_ptr);
    }
    else /* if (qllu->info_field == NULL) */
    {
      /***********************************************************************/
      /* Get a buffer which is big enough to contain the qllc address and    */
      /* control fields.                                                     */
      /***********************************************************************/
      buffer_ptr = 
	QBM_GET_BUFFER(OFFSETOF(body.qllc_body.user_data[0],x25_mbuf_t));

      if (buffer_ptr == (gen_buffer_type *)NULL)
      {
	qvm_rc = qvm_rc_system_error;
	return (qvm_rc);
      }
    }
    /*************************************************************************/
    /* At this stage, data pointer points either to a SNA supplied           */
    /* buffer containing XID or TEST data, or a newly-claimed, empty         */
    /* buffer. In either case, the two byte QLLC header must be added.       */
    /*************************************************************************/
    /*************************************************************************/
    /* Set address field in buffer from address field in qllu.               */
    /*************************************************************************/
    QBM_SET_QLLC_ADDRESS_FIELD(buffer_ptr,(qllu->address_field));
    /*************************************************************************/
    /* Set control field in buffer from control field in qllu.               */
    /*************************************************************************/
    QBM_SET_QLLC_CONTROL_FIELD(buffer_ptr,(qllu->control_field));
    /*************************************************************************/
    /* We now have a buffer which contains the data to be sent as X.25       */
    /* packet(s) to the remote station. The function qvm_send_buffer() is    */
    /* used to transmit the data. The data will be sent with the Q bit       */
    /* turned on as it is a QLLU.                                            */
    /* If the QLLU is a response, we can assume that the buffer should be    */
    /* freed. If it is a command, then the buffer is not freed. A pointer    */
    /* to the buffer is saved in the qllu->info_field, and the already_sent  */
    /* flag is set to TRUE.                                                  */
    /*************************************************************************/
    if (qllu->address_field == command)
    {

    /*********************************************************************/
    /* Defect 103650 - Make a copy of the buffer associated to this QLLU */
    /* and call qvm_send_buffer with this new buffer with the option to  */
    /* free the buffer after sending it.	 			 */
    /*********************************************************************/

      M_COPYM (DupBufferPtr, buffer_ptr);

      qllu->already_sent = TRUE;
      qllu->info_field = (byte *)buffer_ptr;
      outputf("QVM_SEND_QLLU: calling qvm_send_buffer to send command\n");
      outputf("QVM_SEND_QLLU: buffer_ptr = %x\n",buffer_ptr);
      qvm_rc = qvm_send_buffer (
	virt_circuit,
	x25_port,
	DupBufferPtr,
	(bool)TRUE,          /* setting of Q bit                      */
	(bool)TRUE           /* free the buffer                       */
	);

    /* End of Defect 103650 */

    }
    else /* if (qllu->address_field == response) */
    {
      outputf ("QVM_SEND_QLLU: calling qvm_send_buffer to send response \n");
      qvm_rc = qvm_send_buffer (
	virt_circuit,
	x25_port,
	buffer_ptr,
	(bool)TRUE,             /* setting of Q bit                      */
	(bool)TRUE              /* free the buffer                       */
	);
    }
  }
  return (qvm_rc);
}



/*****************************************************************************/
/* X.25 SEND QLLC FRAME REJECT PACKET                                        */
/* Function:    qvm_send_qfrmr                                               */
/*                                                                           */
/* Description: This function is provided to allow the QLLC link station     */
/*              manager to request transmission of Frame Reject Responses,   */
/*              without having to know the detailed format of the information*/
/*              field contained within the packet.                           */
/*                                                                           */
/* Parameters:  virt_circuit    - a pointer to the X.25 virtual circuit      */
/*                                instance on which the data is to be sent.  */
/*              x25_port        - a pointer to the X.25 port instance.       */
/*              control_byte    - value of the control field from the QLLU   */
/*                                which caused the Frame Rej to be generated */
/*              reason          - enumerated code indicating the reason for  */
/*                                the Frame Reject.                          */
/*                                                                           */
/* Return    qvm_rc_port_error  - an error occurred in the QPM.              */
/*           qvm_rc_system_error- an error was reported by a system function */
/*           qvm_rc_ok          - the function completed successfully.       */
/*                                                                           */
/*****************************************************************************/
qvm_rc_type qvm_send_qfrmr (

  x25_vc_type                *virt_circuit,
  port_type                  *x25_port,
  byte                       control_byte,
  enum qfrmr_reason_type     reason)
{
  gen_buffer_type *buffer_ptr;         /* pointer to a buffer from the pool*/
  struct x25_write_ext write_ext;
  qvm_rc_type qvm_rc;                  /* return code from this function   */
  qpm_rc_type qpm_rc;                  /* return code from port function   */

  qvm_rc = qvm_rc_ok;

  /***************************************************************************/
  /* Claim the buffer which will be used to hold the QFRMR packet            */
  /***************************************************************************/
  buffer_ptr = QBM_GET_BUFFER(
    OFFSETOF(body.qllc_body.user_data[3],x25_mbuf_t));
  if (buffer_ptr == (gen_buffer_type *)NULL)
  {
    qvm_rc = qvm_rc_system_error;
    return(qvm_rc);
  }
  else
  {
    /*************************************************************************/
    /* We know that QBM_GET_BUFFER succeeded. We have to build a command ext */
    /* and a data buffer.                                                    */
    /*************************************************************************/
    (void)qpm_make_write_ext (
      &write_ext,
      virt_circuit->session_id,
      virt_circuit->netid
      );

    /*************************************************************************/
    /* Initialise the packet data fields in the buffer.                      */
    /*************************************************************************/
    QBM_SET_PACKET_TYPE(buffer_ptr,PKT_DATA);
    QBM_SET_FLAGS(buffer_ptr,0);
    QBM_SET_Q_BIT(buffer_ptr);
    QBM_SET_CAUSE(buffer_ptr,0);
    QBM_SET_DIAGNOSTIC(buffer_ptr,0);
    /*************************************************************************/
    /* QFRMR packets always contain 5 bytes. The first is the address field  */
    /* of the QFRMR packet (always set to 'response'). The second is the     */
    /* QFRMR control field (identifies this as a QFRMR packet). The third    */
    /* is the control field                                                  */
    /* from the packet which caused the QFRMR to be generated. The fourth    */
    /* and fifth are dependent on the reason code supplied.                  */
    /*************************************************************************/
    QBM_SET_QLLC_ADDRESS_FIELD(buffer_ptr,(byte)response);
    QBM_SET_QLLC_CONTROL_FIELD(buffer_ptr,(byte)qfrmr_rsp);

    switch (reason)
    {
    case (byte)qfr_erroneous_qllu_command:
    case (byte)qfr_qxid_qtest_cmd_received_in_invalid_state:
      QBM_SET_BYTE(buffer_ptr,
	OFFSETOF(body.qllc_body.user_data[0],x25_mbuf_t),control_byte);
      QBM_SET_BYTE(buffer_ptr,
	OFFSETOF(body.qllc_body.user_data[1],x25_mbuf_t),0x00);
      QBM_SET_BYTE(buffer_ptr,
	OFFSETOF(body.qllc_body.user_data[2],x25_mbuf_t),0x80);
      break;

    case (byte)qfr_erroneous_qllu_response:
    case (byte)qfr_response_received_by_secondary_station:
      /***********************************************************************/
      /* Use proper macros to get the right offsets                          */
      /***********************************************************************/
      QBM_SET_BYTE(buffer_ptr,
	OFFSETOF(body.qllc_body.user_data[0],x25_mbuf_t),control_byte);
      QBM_SET_BYTE(buffer_ptr,
	OFFSETOF(body.qllc_body.user_data[1],x25_mbuf_t),0x08);
      QBM_SET_BYTE(buffer_ptr,
	OFFSETOF(body.qllc_body.user_data[2],x25_mbuf_t),0x80);
      break;
    }

    /*************************************************************************/
    /* Now the queue element and buffer have been constructed for the        */
    /* QFRMR packet. Next, the command must be enqueued. If the              */
    /* enqueue fails, then the return code from this function is set to      */
    /* x25_port_error. The buffer which was claimed and initialised must be  */
    /* freed.                                                                */
    /*************************************************************************/
    outputf ("QVM_SEND_QFRMR  CALLING qpm_write \n");
    qpm_rc = qpm_write (
      x25_port,
      buffer_ptr,
      &(write_ext)
      );
    if (qpm_rc != qpm_rc_ok)
    {
      qvm_rc = qvm_rc_port_error;
      QBM_FREE_BUFFER(buffer_ptr);
    }
    else
    {
      /***********************************************************************/
      /* The RAS counters should be updated here to reflect the packets that */
      /* have just been sent. However the mechanism for getting this info    */
      /* from the DH is not yet known.                          ??           */
      /***********************************************************************/
    }
    return (qvm_rc);
  }
}

