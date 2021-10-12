/* @(#)90  1.2  src/bos/kernext/dlc/qlc/qlcvpkt.h, sysxdlcq, bos411, 9428A410j 11/2/93 09:25:18 */
#ifndef _H_QLCVPKT
#define _H_QLCVPKT
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS:
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

/* Start of declarations for qlcvpkt.c                                       */
#ifdef _NO_PROTO

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
enum qvm_rc_type qvm_clrst();

/*****************************************************************************/
/* X.25 CONFIRM RESET                                                        */
/* Function:    qvm_confirm_reset                                            */
/*                                                                           */
/* Description: This function is used to send a Reset Confirmation packet.   */
/*              This will happen when a Reset Indication is received on a    */
/*              PVC. Since Resets are not allowed by SNA on SVCs, this       */
/*              function should never be used on an SVC.                     */
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
enum qvm_rc_type qvm_confirm_reset();

/*****************************************************************************/
/* X.25 SEND PACKET SEQUENCE                                                 */
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
enum qvm_rc_type qvm_send_buffer();

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
enum qvm_rc_type qvm_send_dllu();


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
/*              necessary. this involves setting the 'already_sent' flag in  */
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
enum qvm_rc_type qvm_send_qllu();

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
enum qvm_rc_type qvm_send_qfrmr();




#else

extern qvm_rc_type qvm_clrst (
  x25_vc_type                *virt_circuit,
  port_type                  *x25_port,
  diag_code_type             diagnostic);

extern qvm_rc_type qvm_confirm_reset (
  x25_vc_type     *virt_circuit,
  port_type       *x25_port,
  gen_buffer_type *buffer_ptr);

extern qvm_rc_type qvm_send_buffer (
  x25_vc_type            *virt_circuit,
  port_type              *x25_port,
  gen_buffer_type        *buffer_ptr,
  bool                    q_bit,
  bool                    free_buffer);

extern qvm_rc_type qvm_send_dllu (
  x25_vc_type            *virt_circuit,
  port_type              *x25_port,
  gen_buffer_type        *buffer_ptr);

extern qvm_rc_type qvm_send_qllu (
  x25_vc_type            *virt_circuit,
  port_type              *x25_port,
  qllc_qllu_type         *qllu);

extern qvm_rc_type qvm_send_qfrmr (
  x25_vc_type                *virt_circuit,
  port_type                  *x25_port,
  byte                       control_byte,
  enum qfrmr_reason_type     reason);

#endif /* _NO_PROTO */
/* End of declarations for qlcvpkt.c                                         */



#endif
