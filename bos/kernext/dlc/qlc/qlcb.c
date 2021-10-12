static char sccsid[] = "@(#)48  1.4  src/bos/kernext/dlc/qlc/qlcb.c, sysxdlcq, bos411, 9428A410j 11/2/93 08:57:37";
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: qbm_return_qllu, qbm_return_cud
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

#include "qlcg.h"
#include "qlcvfac.h"
#include "qlcv.h"
#include "qlcq.h"
#include "qlcb.h"

/*****************************************************************************/
/* Function     qbm_return_qllu                                              */
/*                                                                           */
/* Description  This procedure gets the qllu out of a buffer                 */
/*              and fills in a structure of type qllc_qllu_type              */
/*              which is provided by the caller (address passed as           */
/*              input parameter).                                            */
/*              The info field does nothing more than hold a copy of the     */
/*              buffer_ptr (suitably cast).                                  */
/*                                                                           */
/* Return       none                                                         */
/*                                                                           */
/* Parameters   address of buffer                                            */
/*              address of empty qllc_qllu_type structure.                   */
/*                                                                           */
/*****************************************************************************/
void qbm_return_qllu(

  gen_buffer_type *buffer_ptr,
  struct qllc_qllu_type *qllu_ptr)
{
  /***************************************************************************/
  /* The fields of interest are stored in the mbuf in the QLLC_Body section  */
  /* The QBM_RETURN_QLLC_ADDRESS_CONTROL_FIELD macros are supplied by the    */
  /* to call the necessary X25_READ macros.                                  */
  /***************************************************************************/
  qllu_ptr->address_field = QBM_RETURN_QLLC_ADDRESS_FIELD(buffer_ptr);
  qllu_ptr->control_field = QBM_RETURN_QLLC_CONTROL_FIELD(buffer_ptr);
  qllu_ptr->info_field = (byte *)buffer_ptr;
}

/*****************************************************************************/
/* Function     qbm_return_cud                                               */
/*                                                                           */
/* Description  This procedure gets the cud out of a buffer                  */
/*              and fills in a structure of type x25_cud_type                */
/*              which is provided by the caller (address passed as           */
/*              input parameter).                                            */
/*                                                                           */
/* Return       none                                                         */
/*                                                                           */
/* Parameters   address of buffer                                            */
/*              address of empty x25_cud_type structure.                     */
/*                                                                           */
/*****************************************************************************/
void qbm_return_cud(

  gen_buffer_type *buffer_ptr,
  x25_cud_type *cud_ptr)
{
  unsigned short fac_length;
  /***************************************************************************/
  /* The Call user data is stored after the facilities in the optional_data  */
  /* string in the call_data area of the buffer                              */
  /* The address of the start of the CUD is therefore given by:              */
  /*       buffer_ptr->body.cd.optional_data[                                */
  /*           buffer_ptr->body.cd.facilities_length]                        */
  /* The length of the CUD is given by:                                      */
  /*       buffer_ptr->body.cd.user_data_length;                             */
  /*                                                                         */
  /*  X25_SET/READ macros are used to conceal the actual placement of fields */
  /*  in the mbuf.                                                           */
  /***************************************************************************/
/*
  fac_length =  QBM_RETURN_FACILITIES_LENGTH(buffer_ptr);
  if (fac_length != QLLC_CUD_LENGTH)
  {
    cud_ptr->length = 0;
    *(cud_ptr->cud) = '\0';
    return;
  }
*/
  cud_ptr->length = QBM_RETURN_CUD_LENGTH(buffer_ptr);
  *(cud_ptr->cud) = QBM_RETURN_CUD(buffer_ptr);
}

