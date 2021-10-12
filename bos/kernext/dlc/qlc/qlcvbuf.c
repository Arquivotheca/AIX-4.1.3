static char sccsid[] = "@(#)84	1.6  src/bos/kernext/dlc/qlc/qlcvbuf.c, sysxdlcq, bos411, 9428A410j 11/2/93 10:28:07";
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: qvm_make_call_req_buffer, qvm_make_reset_buffer,
 *            qvm_make_clear_req_buffer, qvm_make_clear_conf_buffer,
 *            qvm_make_call_accept_buffer, qvm_make_write_buffer,
 *            qvm_return_cud, qvm_return_qllu
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
/* #include "qlcl.h" */

/******************************************************************************
** DATA BUFFER CONSTRUCTORS:
******************************************************************************/


/*****************************************************************************/
/* Function     QVM_MAKE_CALL_REQ_BUFFER                                     */
/*                                                                           */
/* Description  This procedure initialises a BUFFER to make a call_req       */
/*              so that it can be used to issue a start for an SVC via       */
/*              the QPM_Start procedure.                                     */
/*              The buffer is claimed by the calling procedure, but is only  */
/*              of size MIN_BUF_SIZE. It is enlarged if necessary by this    */
/*              function.                                                    */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Parameters   gen_buffer_type buffer_ptr                                   */
/*              others..                                                     */
/*              The cb_facs ptr is the address of the facs structure in the  */
/*              station                                                      */
/*                                                                           */
/*****************************************************************************/
 void    qvm_make_call_req_buffer (

  struct qlc_sls_arg   *qlc_ext_ptr,
  gen_buffer_type      *buffer_ptr,
  char                 *calling_address,
  char                 *called_address,
  cb_fac_t             *cb_facilities,
  unsigned int          protocol,
  x25_devinfo_t        *dh_devinfo_ptr)
{
  unsigned int fac_len;
  unsigned int ii;
  byte         fac_stream[X25_MAX_FACILITIES_LENGTH];
  int          index;
  outputf("QBM_MAKE_CALL_REQ: calling address [%s]\n",calling_address);
  outputf("QBM_MAKE_CALL_REQ: called address [%s]\n",called_address);

  /***************************************************************************/
  /* Initialise fac_stream                                                   */
  /***************************************************************************/
  for (index=0; index<X25_MAX_FACILITIES_LENGTH; index++)
    fac_stream[index]='\0';
  /***************************************************************************/
  /* Calculate fac_len                                                       */
  /***************************************************************************/
  fac_len = _x25_convert_cb_fac_to_byte_stream(
    fac_stream,
    cb_facilities,
    dh_devinfo_ptr
    );
  outputf("QVM_MAKE_CALL_REQ: fac_len = %d\n",fac_len);
  outputf("QVM_MAKE_CALL_REQ: facilities stream = ");
  for (index=0; index<fac_len; index++)
    outputf("%x",fac_stream[index]);
  outputf("\n");
  /***************************************************************************/
  /* Now guarantee size of buffer                                            */
  /***************************************************************************/
  if (qlc_ext_ptr->psd.facilities.cud == TRUE)
  {
     JSMBUF_GUARANTEE_SIZE(
      buffer_ptr,
      X25_OFFSETOF_FAC_CUD_DATA + fac_len + qlc_ext_ptr->psd.facilities.cud_length
    );
  }
  else
  {
     JSMBUF_GUARANTEE_SIZE(
      buffer_ptr,
      X25_OFFSETOF_FAC_CUD_DATA + fac_len + 1
    );
  }
  /***************************************************************************/
  /* Set up the packet data in the buffer                                    */
  /***************************************************************************/
  QBM_SET_PACKET_TYPE(buffer_ptr,PKT_CALL_REQ);
  QBM_SET_CAUSE(buffer_ptr,0);
  QBM_SET_DIAGNOSTIC(buffer_ptr,0);
  QBM_SET_FLAGS(buffer_ptr,0);

  /***************************************************************************/
  /* Set up the call data in the buffer                                      */
  /***************************************************************************/
  QBM_SET_CALLING_ADDRESS(buffer_ptr,calling_address);
  QBM_SET_CALLED_ADDRESS(buffer_ptr,called_address);
  /***************************************************************************/
  /* Copy the facilities into the buffer via a request to the generic X25    */
  /* facilities parser                                                       */
  /***************************************************************************/
  QBM_SET_BLOCK(buffer_ptr,X25_OFFSETOF_FAC_CUD_DATA,fac_stream,fac_len);
  QBM_SET_FACILITIES_LENGTH(buffer_ptr,fac_len);
  /***************************************************************************/
  /* Now copy cud in after facilities. Have to allow for fac_len to determine*/
  /* where the cud should start, as it follows the facs fields.              */
  /***************************************************************************/
 
  /***************************************************************************/
  /* There is an assumption here that QLLC CUD is always ONE BYTE LONG.      */
  /***************************************************************************/
  if (qlc_ext_ptr->psd.facilities.cud == TRUE) /* Defect 110313 */
  {
     outputf("QBM_MAKE_CALL_REQ: User Defined CUD. cud_length=%d\n",qlc_ext_ptr->psd.facilities.cud_length);
     QBM_SET_CUD_LENGTH(buffer_ptr,qlc_ext_ptr->psd.facilities.cud_length);
     for (ii=0;ii<qlc_ext_ptr->psd.facilities.cud_length;ii++)
         {
         QBM_SET_BYTE(
           buffer_ptr,
           X25_OFFSETOF_FAC_CUD_DATA+fac_len+ii,
           qlc_ext_ptr->psd.facilities.cud_data[ii]);
     outputf("QBM_MAKE_CALL_REQ: cud[%d]=%x \n",ii,qlc_ext_ptr->psd.facilities.cud_data[ii]);
         }
  }
  else
  /***************************************************************************/
  /* There is an assumption here that QLLC CUD is always ONE BYTE LONG.      */
  /***************************************************************************/
  {
     QBM_SET_CUD_LENGTH(buffer_ptr,QLLC_CUD_LENGTH);

     if (protocol == X25_PROTOCOL_QLLC_80)
     {
       outputf("QBM_MAKE_CALL_REQ: cud = X25_PROTOCOL_QLLC_80\n");
       QBM_SET_BYTE(
         buffer_ptr,
         X25_OFFSETOF_FAC_CUD_DATA + fac_len,
         QLLC_CUD_80
         );
     }
     else   /* protocol == X25_PROTOCOL_QLLC_84 */
     {
       outputf("QBM_MAKE_CALL_REQ: cud = X25_PROTOCOL_QLLC_84\n");
       QBM_SET_BYTE(
         buffer_ptr,
         X25_OFFSETOF_FAC_CUD_DATA + fac_len,
         QLLC_CUD_84
         );
     }
  }
  print_x25_buffer (buffer_ptr );
}

/*****************************************************************************/
/* Function     QVM_MAKE_RESET_BUFFER                                        */
/*                                                                           */
/* Description  This procedure initialises a buffer claimed by the caller    */
/*              so that it can be used to issue a reset to the network via   */
/*              the QPM_Write procedure.                                     */
/*                                                                           */
/*                                                                           */
/* Return       gen_buffer_type *buffer_ptr                                  */
/*                                                                           */
/* Parameters   gen_buffer_type *buffer_ptr                                  */
/*              diag_code_type diagnostic                                    */
/*                                                                           */
/*****************************************************************************/
gen_buffer_type *qvm_make_reset_buffer (

  gen_buffer_type *buffer_ptr,
  diag_code_type diagnostic)
{
  /***************************************************************************/
  /* Guarantee size of buffer is big enough for a Clear Request              */
  /***************************************************************************/
  JSMBUF_GUARANTEE_SIZE(
    buffer_ptr,
    X25_OFFSETOF_FAC_CUD_DATA
    );
  /***************************************************************************/
  /* Set up the packet data in the buffer                                    */
  /***************************************************************************/
  QBM_SET_PACKET_TYPE(buffer_ptr,PKT_RESET_REQ);
  QBM_SET_CAUSE(buffer_ptr,DTE_GENERATED);
  QBM_SET_DIAGNOSTIC(buffer_ptr,diagnostic);
  QBM_SET_FLAGS(buffer_ptr,0);
  /***************************************************************************/
  /* There is no data in a reset request packet.                             */
  /* This is indicated by the fact that the length field (m_len) in the      */
  /* mbuf header is only as long as the packet_data.                         */
  /***************************************************************************/
  return (buffer_ptr);
}

/*****************************************************************************/
/* Function     QVM_MAKE_CLEAR_REQ_BUFFER                                    */
/*                                                                           */
/* Description  This procedure initialises a buffer claimed by the caller    */
/*              so that it can be used to issue a clear request to the xdh   */
/*              the QPM_Halt or QPM_Reject procedures.                       */
/*                                                                           */
/*                                                                           */
/* Return       gen_buffer_type *buffer_ptr                                  */
/*                                                                           */
/* Parameters   gen_buffer_type *buffer_ptr                                  */
/*              diag_code_type diagnostic                                    */
/*                                                                           */
/*****************************************************************************/
gen_buffer_type *qvm_make_clear_req_buffer (

  gen_buffer_type *buffer_ptr,
  diag_code_type diagnostic)
{
  char null_x25_addr[X25_MAX_ASCII_ADDRESS_LENGTH] = {""};
  /***************************************************************************/
  /* If buffer has been used as an incoming call indication, then it will be */
  /* longer than is needed. It is therefore trimmed.                         */
  /* If on the other hand, the buffer is a new one, for a locally initiated  */
  /* clear, it will need to be guaranteed to be long enough.                 */
  /***************************************************************************/
  if (JSMBUF_LENGTH(buffer_ptr) > X25_OFFSETOF_FAC_CUD_DATA)
  {
    JSMBUF_TRIM(buffer_ptr,
      JSMBUF_LENGTH(buffer_ptr)-X25_OFFSETOF_FAC_CUD_DATA);
  }
  else
  {
    /*************************************************************************/
    /* Guarantee size of buffer is big enough for a Clear Request            */
    /*************************************************************************/
    JSMBUF_GUARANTEE_SIZE(
      buffer_ptr,
      X25_OFFSETOF_FAC_CUD_DATA
      );
  }
  /***************************************************************************/
  /* Set up the packet data in the buffer                                    */
  /***************************************************************************/
  QBM_SET_PACKET_TYPE(buffer_ptr,PKT_CLEAR_REQ);
  QBM_SET_CAUSE(buffer_ptr,DTE_GENERATED);
  QBM_SET_DIAGNOSTIC(buffer_ptr,diagnostic);
  QBM_SET_FLAGS(buffer_ptr,0);
  /***************************************************************************/
  /* The Call Data is left empty for Clear Request packets. The adapter      */
  /* decides whether or not to include addresses, and QLLC has no use of     */
  /* the facilities and user_data fields as it does not support fast_select  */
  /* and the only facilities relevant to a clear (in QLLC's repertoire of    */
  /* support) are generated by the network. i.e. transit delay, charging.    */
  /***************************************************************************/
  /***************************************************************************/
  /* Ensure that the address fields are NULL                                 */
  /***************************************************************************/
  QBM_SET_CALLING_ADDRESS(buffer_ptr,null_x25_addr);
  QBM_SET_CALLED_ADDRESS(buffer_ptr,null_x25_addr);
  QBM_SET_CUD_LENGTH(buffer_ptr,0);
  QBM_SET_FACILITIES_LENGTH(buffer_ptr,0);
  return (buffer_ptr);
}

/*****************************************************************************/
/* Function     QVM_MAKE_CLEAR_CONF_BUFFER                                   */
/*                                                                           */
/* Description  This procedure initialises a buffer claimed by the caller    */
/*              so that it can be used to issue a clear confirm to the xdh.  */
/*                                                                           */
/*                                                                           */
/* Return       gen_buffer_type *buffer_ptr                                  */
/*                                                                           */
/* Parameters   gen_buffer_type *buffer_ptr                                  */
/*              diag_code_type diagnostic                                    */
/*                                                                           */
/*****************************************************************************/
gen_buffer_type *qvm_make_clear_conf_buffer (
 
  gen_buffer_type *buffer_ptr)
{
  char null_x25_addr[X25_MAX_ASCII_ADDRESS_LENGTH] = {""};
  /***************************************************************************/
  /* Guarantee size of buffer is big enough for a Clear Request              */
  /***************************************************************************/
  JSMBUF_GUARANTEE_SIZE(
    buffer_ptr,
    X25_OFFSETOF_FAC_CUD_DATA
    );
  /***************************************************************************/
  /* Set up the packet data in the buffer                                    */
  /***************************************************************************/
  QBM_SET_PACKET_TYPE(buffer_ptr,PKT_CLEAR_CONFIRM);
  QBM_SET_CAUSE(buffer_ptr,DTE_GENERATED);
  QBM_SET_DIAGNOSTIC(buffer_ptr,0);
  QBM_SET_FLAGS(buffer_ptr,0);
  /***************************************************************************/
  /* The Clear Data is left empty for Clear Confirm packets. The adapter     */
  /* decides whether or not to include addresses, and QLLC has no use of     */
  /* the facilities and user_data fields as it does not support fast_select  */
  /* and the only facilities relevant to a clear (in QLLC's repertoire of    */
  /* support) are generated by the network. i.e. transit delay, charging.    */
  /***************************************************************************/
  /***************************************************************************/
  /* Ensure that the address fields are NULL                                 */
  /***************************************************************************/
  QBM_SET_CALLING_ADDRESS(buffer_ptr,null_x25_addr);
  QBM_SET_CALLED_ADDRESS(buffer_ptr,null_x25_addr);
  QBM_SET_CUD_LENGTH(buffer_ptr,0);
  QBM_SET_FACILITIES_LENGTH(buffer_ptr,0);
  return (buffer_ptr);
}

/*****************************************************************************/
/* Function     QVM_MAKE_CALL_ACCEPT_BUFFER                                  */
/*                                                                           */
/* Description  This procedure initialises a buffer claimed by the caller    */
/*              so that it can issue a Call_Accept pkt to the network via    */
/*              the QPM_Start procedure.                                     */
/*                                                                           */
/*                                                                           */
/* Return       gen_buffer_type *buffer_ptr                                  */
/*                                                                           */
/* Parameters   gen_buffer_type *buffer_ptr                                  */
/*                                                                           */
/*****************************************************************************/
gen_buffer_type *qvm_make_call_accept_buffer(

  gen_buffer_type *buffer_ptr)
{
  char null_x25_addr[X25_MAX_ASCII_ADDRESS_LENGTH] = {""};

  /***************************************************************************/
  /* Trim buffer down from Call Request to Call Accept size.                 */
  /***************************************************************************/
  JSMBUF_TRIM(buffer_ptr,
    JSMBUF_LENGTH(buffer_ptr)-X25_OFFSETOF_FAC_CUD_DATA);

  /***************************************************************************/
  /* Guarantee size of buffer is big enough for a Clear Request              */
  /***************************************************************************/
  JSMBUF_GUARANTEE_SIZE(
    buffer_ptr,
    X25_OFFSETOF_FAC_CUD_DATA
    );
  /***************************************************************************/
  /* Set up the packet data in the buffer                                    */
  /***************************************************************************/
  QBM_SET_PACKET_TYPE(buffer_ptr,PKT_CALL_ACCEPT);
  QBM_SET_CAUSE(buffer_ptr,0);
  QBM_SET_DIAGNOSTIC(buffer_ptr,0);
  QBM_SET_FLAGS(buffer_ptr,0);
  /***************************************************************************/
  /* Initialise the Call Data structure                                      */
  /*                                                                         */
  /* The Call Data is left empty for Call Accept packets. The adapter        */
  /* decides whether or not to include addresses, and QLLC has no use of     */
  /* the user_data fields as it does not support fast_select.                */
  /*                                                                         */
  /* Facilities are a different matter. They have been modified already,     */
  /* by the qvm_accept_incoming_call function.                               */
  /***************************************************************************/
  /***************************************************************************/
  /* Ensure that the address fields are NULL                                 */
  /***************************************************************************/
  QBM_SET_CALLING_ADDRESS(buffer_ptr,null_x25_addr);
  QBM_SET_CALLED_ADDRESS(buffer_ptr,null_x25_addr);
  QBM_SET_CUD_LENGTH(buffer_ptr,0);
  QBM_SET_FACILITIES_LENGTH(buffer_ptr,0);
  return (buffer_ptr);
}

/*****************************************************************************/
/* Function     QVM_MAKE_WRITE_BUFFER                                        */
/*                                                                           */
/* Description  This procedure initialises a buffer claimed by the caller    */
/*              so that it can be used to issue a DATA BUFFER using the      */
/*              the QPM_Write procedure.                                     */
/*                                                                           */
/* Return       gen_buffer_type *buffer_ptr                                  */
/*                                                                           */
/* Parameters   gen_buffer_type *buffer_ptr                                  */
/*                                                                           */
/*****************************************************************************/
gen_buffer_type *qvm_make_write_buffer(

  gen_buffer_type *buffer_ptr,
  bool q_bit)
{

  QBM_SET_CAUSE(buffer_ptr,0);
  QBM_SET_DIAGNOSTIC(buffer_ptr,0);
  QBM_SET_FLAGS(buffer_ptr,0);
  if (q_bit == TRUE)
  {
    QBM_SET_Q_BIT(buffer_ptr);
  }
  else
  {
    QBM_CLEAR_Q_BIT(buffer_ptr);
  }
  QBM_SET_PACKET_TYPE(buffer_ptr,PKT_DATA);

  return (buffer_ptr);
}

/*****************************************************************************/
/* RETURN CUD                                                                */
/* Function:    qvm_return_cud                                               */
/*                                                                           */
/* Description: This function takes a pointer to a data buffer, and a ptr    */
/*              to a call_user_data structure. The call_user_data struct is  */
/*              initialised from the contents of the buffer.                 */
/*                                                                           */
/*                                                                           */
/* Parameters:  cud_ptr         - a pointer to the cud structure to          */
/*                                be initialised.                            */
/*              buffer_ptr      - pointer to the buffer to be used by the    */
/*                                function.                                  */
/*                                                                           */
/* Return Code: a pointer to the initialised cud structure                   */
/*                                                                           */
/*****************************************************************************/
x25_cud_type *qvm_return_cud (

  x25_cud_type    *cud_ptr,
  gen_buffer_type *buffer_ptr)
{
  byte fac_len;

  /***************************************************************************/
  /* The Call_Data structure defined in jsxbuf.h is used                     */
  /* Need to use proper jsmbuf macros to get correct offsets                 */
  /***************************************************************************/
  fac_len = QBM_RETURN_FACILITIES_LENGTH(buffer_ptr);
  cud_ptr->length = QBM_RETURN_CUD_LENGTH(buffer_ptr);
  QBM_RETURN_BLOCK(
    buffer_ptr,
    X25_OFFSETOF_FAC_CUD_DATA + fac_len,
    cud_ptr->cud,
    cud_ptr->length
    );
  return(cud_ptr);
}

/*****************************************************************************/
/* RETURN QLLU                                                               */
/* Function:    qvm_return_qllu                                              */
/*                                                                           */
/* Description: This function takes a pointer to a data buffer, and a ptr    */
/*              to a qllc_qllu_type structure. The qllc_qllu_type struct is  */
/*              initialised from the contents of the buffer. The control and */
/*              address field are read from the first two bytes of the packet*/
/*              user data. The info field pointer is initialised with the    */
/*              buffer pointer. The code to find the info from within the    */
/*              buffer is not implemented here.                              */
/*                                                                           */
/*              Note that this function replaces the two functions originally*/
/*              described in the QLLC design as vdb_return_q_pkt_type() and  */
/*              vdb_return_i_field().                                        */
/*                                                                           */
/* Parameters:  qllu_ptr        - a pointer to the qllu structure to         */
/*                                be initialised.                            */
/*              buffer_ptr      - pointer to the buffer to be used by the    */
/*                                function.                                  */
/*                                                                           */
/* Return Code: a pointer to the initialised qllu structure                  */
/*                                                                           */
/*****************************************************************************/
qllc_qllu_type *qvm_return_qllu (

  qllc_qllu_type  *qllu_ptr,
  gen_buffer_type *buffer_ptr)
{
  /***************************************************************************/
  /* address field is the first byte of data in the packet                   */
  /* Use proper macros to get the offsets right.                             */
  /***************************************************************************/
  qllu_ptr->address_field = QBM_RETURN_QLLC_ADDRESS_FIELD(buffer_ptr);

  /***************************************************************************/
  /* control field is the second byte of data in the packet                  */
  /* Use proper macros to get the offsets right.                             */
  /***************************************************************************/
  qllu_ptr->control_field = QBM_RETURN_QLLC_CONTROL_FIELD(buffer_ptr);

  /***************************************************************************/
  /* info field is simply the buffer pointer at this stage                   */
  /***************************************************************************/
  qllu_ptr->info_field = (byte *)buffer_ptr;

  return (qllu_ptr);
}








