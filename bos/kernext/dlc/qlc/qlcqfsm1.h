/* @(#)76  1.2  src/bos/kernext/dlc/qlc/qlcqfsm1.h, sysxdlcq, bos411, 9428A410j 11/2/93 09:18:26 */
#ifndef _H_QLCQFSM1
#define _H_QLCQFSM1
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

/* Start of declarations for qlcqfsm1.c                                      */
#ifdef _NO_PROTO
enum qllc_rc_type qllc_repoll ();
enum qllc_rc_type qllc_cmd_poll ();
enum qllc_rc_type qllc_pvc_reset ();
enum qllc_rc_type qllc_svc_cleared ();
enum qllc_rc_type qllc_lltox ();
enum qllc_rc_type qllc_lstrt ();
enum qllc_rc_type qllc_lstop ();
enum qllc_rc_type qllc_xchid ();
enum qllc_rc_type qllc_ltest ();
enum qllc_rc_type qllc_sdata ();
enum qllc_rc_type qllc_l3rdy ();
enum qllc_rc_type qllc_l3nop ();
enum qllc_rc_type qllc_errpk ();
enum qllc_rc_type qllc_clrst ();
enum qllc_rc_type qllc_rdata ();
#else
enum qllc_rc_type qllc_repoll (
      qllc_ls_type                      *link_station,
      x25_vc_type                       *virt_circuit,
      port_type                         *x25_port);

extern enum qllc_rc_type qllc_cmd_poll (
      qllc_ls_type                      *link_station,
      x25_vc_type                       *virt_circuit,
      port_type                         *x25_port,
      qllc_qllu_type                    *qllu);

extern enum qllc_rc_type qllc_pvc_reset (
      qllc_ls_type                      *link_station,
      x25_vc_type                       *virt_circuit,
      port_type                         *x25_port,
      q_cause_code_type                 cause,
      q_diag_code_type                  diagnostic);

extern enum qllc_rc_type qllc_svc_cleared (
      qllc_ls_type                      *link_station,
      q_cause_code_type                 cause,
      q_diag_code_type                  diagnostic);

extern enum qllc_rc_type qllc_lltox (
      qllc_ls_type                      *link_station,
      x25_vc_type                       *virt_circuit,
      port_type                         *x25_port);

extern enum qllc_rc_type qllc_lstrt (
      qllc_ls_type                      *link_station,
      x25_vc_type                       *virt_circuit,
      port_type                         *x25_port);

extern enum qllc_rc_type qllc_lstop (
      qllc_ls_type                      *link_station,
      x25_vc_type                       *virt_circuit,
      port_type                         *x25_port);

enum qllc_rc_type qllc_xchid (
      qllc_ls_type                      *link_station,
      x25_vc_type                       *virt_circuit,
      port_type                         *x25_port,
      byte                              *xid_info);

enum qllc_rc_type qllc_ltest (
      qllc_ls_type                      *link_station,
      x25_vc_type                       *virt_circuit,
      port_type                         *x25_port,
      byte                              *test_info);

enum qllc_rc_type qllc_sdata (
      qllc_ls_type                      *link_station,
      x25_vc_type                       *virt_circuit,
      port_type                         *x25_port,
      byte                              *data);

extern enum qllc_rc_type qllc_l3rdy (
      qllc_ls_type                      *link_station);

extern enum qllc_rc_type qllc_l3nop (
      qllc_ls_type                      *link_station);

enum qllc_rc_type qllc_errpk (
      qllc_ls_type                      *link_station,
      x25_vc_type                       *virt_circuit,
      port_type                         *x25_port,
      byte                              address_byte,
      byte                              control_byte);

enum qllc_rc_type qllc_clrst (
      qllc_ls_type                      *link_station,
      x25_vc_type                       *virt_circuit,
      port_type                         *x25_port,
      bool                              pvc_flag,
      q_cause_code_type                 cause,
      q_diag_code_type                  diagnostic);

extern enum qllc_rc_type qllc_rdata (
      qllc_ls_type                      *link_station);

#endif /* _NO_PROTO */
/* End of declarations for qlcqfsm1.c                                        */
#endif
