/* @(#)86	1.3  src/bos/kernext/dlc/qlc/qlcvbuf.h, sysxdlcq, bos411, 9428A410j 11/2/93 10:38:28 */
#ifndef _H_QLCVBUF
#define _H_QLCVBUF
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

/*****************************************************************************/
/* MACRO FUNCTIONS:                                                          */
/*****************************************************************************/
/* all macros deleted                                                        */

/*****************************************************************************/
/* C FUNCTION HEADERS                                                        */
/*****************************************************************************/

/* Start of declarations for qlcvbuf.c                                       */
#ifdef _NO_PROTO
void                   qvm_make_call_req_buffer();
gen_buffer_type       *qvm_make_reset_buffer();
gen_buffer_type       *qvm_make_clear_buffer();
gen_buffer_type       *qvm_make_call_accept_buffer();
gen_buffer_type       *qvm_make_write_buffer();
struct x25_cud_type   *qvm_return_cud ();
struct qllc_qllu_type *qvm_return_qllu ();
#else
extern  void    qvm_make_call_req_buffer (
  struct qlc_sls_arg   *qlc_ext_ptr,	/* Defect 110313 */
  gen_buffer_type      *buffer_ptr,
  char                 *calling_address,
  char                 *called_address,
  cb_fac_t             *cb_facilities,
  unsigned int          protocol,
  x25_devinfo_t *dh_devinfo_ptr);

extern gen_buffer_type *qvm_make_reset_buffer (
  gen_buffer_type *buffer_ptr,
  diag_code_type diagnostic);

extern gen_buffer_type *qvm_make_clear_buffer (
  gen_buffer_type *buffer_ptr,
  diag_code_type diagnostic);

extern gen_buffer_type *qvm_make_call_accept_buffer(
  gen_buffer_type *buffer_ptr);

extern gen_buffer_type *qvm_make_write_buffer(
  gen_buffer_type *buffer_ptr,
  bool q_bit);

extern x25_cud_type *qvm_return_cud (
  x25_cud_type    *cud_ptr,
  gen_buffer_type *buffer_ptr);

extern qllc_qllu_type *qvm_return_qllu (
  qllc_qllu_type  *qllu_ptr,
  gen_buffer_type *buffer_ptr);

#endif /* _NO_PROTO */
/* End of declarations for qlcvbuf.c                                         */

#endif



