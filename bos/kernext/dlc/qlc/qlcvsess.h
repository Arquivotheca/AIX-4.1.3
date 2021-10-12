/* @(#)92	1.3  src/bos/kernext/dlc/qlc/qlcvsess.h, sysxdlcq, bos411, 9428A410j 11/2/93 10:40:33 */
#ifndef _H_QLCVSESS
#define _H_QLCVSESS
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


/* Start of declarations for qlcvsess.c                                      */
#ifdef _NO_PROTO
enum qvm_rc_type qvm_open_vc();
enum qvm_rc_type qvm_close_vc();
enum qvm_rc_type qvm_accept_incoming_call();
enum qvm_rc_type qvm_reject_incoming_call();
enum qvm_rc_type qvm_vc_opened();
enum qvm_rc_type qvm_vc_closed();
#else
extern qvm_rc_type qvm_open_vc (
  struct qlc_sls_arg *qlc_ext_ptr,	/* Defect 110313 */
  x25_vc_type        *virt_circuit,
  port_type          *x25_port,
  init_rec_type      *init_rec);

extern qvm_rc_type qvm_close_vc (
  x25_vc_type               *virt_circuit,
  port_type                 *x25_port,
  diag_code_type            diagnostic,
  boolean                   remote_clear);

extern qvm_rc_type qvm_accept_incoming_call (
  x25_vc_type     *virt_circuit,
  port_type       *x25_port,
  gen_buffer_type *buffer_ptr,
  diag_tag_type    session_name,
  correlator_type  correlator);

extern qvm_rc_type qvm_reject_incoming_call (
  x25_vc_type          *virt_circuit,
  port_type            *x25_port,
  gen_buffer_type      *buffer_ptr,
  diag_code_type        diagnostic);

extern qvm_rc_type qvm_vc_opened (
  x25_vc_type        *virt_circuit);

extern qvm_rc_type qvm_vc_closed (
  x25_vc_type           *virt_circuit,
  gen_buffer_type       *buffer_ptr);

#endif /* _NO_PROTO */
/* End of declarations for qlcvsess.c                                        */

#endif
