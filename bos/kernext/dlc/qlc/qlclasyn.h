/* @(#)60  1.6  src/bos/kernext/dlc/qlc/qlclasyn.h, sysxdlcq, bos411, 9428A410j 3/22/94 15:11:31 */
#ifndef _H_QLCLASYN
#define _H_QLCLASYN
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

/* Start of declarations for qlclasyn.c                                      */
#ifdef _NO_PROTO


void qlm_station_started();
void qlm_station_halted();
void qlm_incoming_clear();
void qlm_station_reset();
void qlm_incoming_call();
void qlm_incoming_call_rejected();
void qlm_receive_data ();
void qlm_invalid_packet_rx();
enum qllc_rc_type  qlm_receive_qllu();
void qlm_receive_inactivity_handler();
void qlm_write_error ();
enum qlm_rc_type qlm_make_netd_buffer();
void qlm_station_remote_discontact();

#else

extern void  qlm_station_started(
  unsigned long result,
  unsigned short netid,
  unsigned short session_id,
  gen_buffer_type *buffer_ptr);

extern void qlm_station_halted(
  unsigned long    result,
  unsigned short   netid,
  unsigned short   session_id,
  gen_buffer_type *buffer_ptr);

extern void qlm_incoming_clear(
  unsigned short   netid,
  unsigned short   session_id,
  gen_buffer_type *buffer_ptr);

extern void qlm_station_reset(
  gen_buffer_type  *buffer_ptr,
  unsigned short    netid,
  unsigned short    session_id);

extern void qlm_incoming_call(
  port_type       *port_id,
  unsigned short   netid,
  gen_buffer_type *buffer_ptr);

extern void qlm_incoming_call_rejected(
  unsigned long   result,
  unsigned short  netid,
  unsigned short  session_id);

extern void qlm_receive_data (
  gen_buffer_type  *buffer_ptr,
  unsigned short    netid,
  unsigned short    session_id);

extern void qlm_invalid_packet_rx (
  gen_buffer_type *buffer_ptr,
  unsigned short   netid,
  unsigned short   session_id,
  diag_code_type   diagnostic);

extern qllc_rc_type  qlm_receive_qllu(
  station_type    *station_ptr,
  port_type       *port_id,
  gen_buffer_type *buffer_ptr,
  bool *stn_deleted);

extern void qlm_receive_inactivity_handler(
  station_type *station_ptr);

extern void qlm_write_error (
  unsigned short netid,
  unsigned short session_id,
  gen_buffer_type *buffer_ptr);

extern qlm_rc_type qlm_make_netd_buffer(
  station_type *station_ptr,
  type_of_netd_t  netd_type,
  byte *data_ptr);

extern void qlm_retry_receive(
  port_type *port_id);

extern void  qlm_qfrmr_alert_gen(
  station_type    *station_ptr,
  gen_buffer_type *buffer_ptr);

extern void  qlm_qllu_i_field_check(
  qllc_qllu_type *qllu,
  station_type *station_ptr);

extern void qlm_station_remote_discontact(
  correlator_type corr,
  bool *stn_deleted);

#endif /* _NO_PROTO */
/* End of declarations for qlclasyn.c                                        */

#endif

