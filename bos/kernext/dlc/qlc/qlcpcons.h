/* @(#)71  1.3  src/bos/kernext/dlc/qlc/qlcpcons.h, sysxdlcq, bos411, 9428A410j 11/2/93 09:11:52 */
#ifndef _H_QLCPCONS
#define _H_QLCPCONS
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

/* Start of declarations for qlcpcons.c                                      */
#ifdef _NO_PROTO
void qpm_make_pvc_start_data ();
void qpm_make_svc_start_data ();
void qpm_make_halt_data ();
void qpm_make_write_ext ();

#else

extern void qpm_make_pvc_start_data (
  struct x25_start_data *start_data,
  unsigned short netid,
  diag_tag_type session_name,
  int protocol,
  int logical_channel);

extern void qpm_make_svc_start_data (
  struct x25_start_data *start_data,
  unsigned short         netid,
  diag_tag_type          session_name,
  int                    session_typ,
  int                    protocol,
  unsigned short         call_id);

extern void qpm_make_listen_start_data (
  struct x25_start_data *start_data,
  unsigned short         netid,
  diag_tag_type          session_name,
  int                    protocol,
  char                  *listen_name);

extern void qpm_make_halt_data (
  struct x25_halt_data *halt_data,
  unsigned short        netid,
  unsigned short        session_id);

extern void qpm_make_write_ext (
  struct x25_write_ext *write_ext,
  unsigned short        session_id,
  unsigned short        netid);

extern void qpm_make_reject_data (
  struct x25_reject_data *reject_data,
  unsigned short          netid,
  unsigned short          session_id,
  unsigned short call_id);

#endif /* _NO_PROTO */
/* End of declarations for qlcpcons.c                                        */

#endif
