/* @(#)78  1.4  src/bos/kernext/dlc/qlc/qlcqfsm2.h, sysxdlcq, bos411, 9428A410j 3/22/94 15:12:34 */
#ifndef _H_QLCQFSM2
#define _H_QLCQFSM2
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


#ifdef _NO_PROTO
/******************************************************************************
*******************************************************************************
** DEFINITIONS OF PROCEDURES EXPORTED WITHIN, BUT NOT OUTSIDE OF QLLC L.S. MGR
*******************************************************************************
******************************************************************************/

/******************************************************************************
** COMMON COMMAND PRIMARY
** enum qllc_rc_type qllc_common_cmd_pri (link_station, virt_circuit, x25_port)
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
******************************************************************************/
enum qllc_rc_type qllc_common_cmd_pri();

/******************************************************************************
** COMMON RESPONSE SECONDARY
** enum qllc_rc_type qllc_common_rsp_sec (
**     link_station,
**     virt_circuit,
**     x25_port,
**     response
** )
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
**     qllc_qllu_type                    *response;
******************************************************************************/
enum qllc_rc_type qllc_common_rsp_sec();

/******************************************************************************
** Q_SET_MODE (SECONDARY)
** enum qllc_rc_type qllc_qsm_sec (link_station, virt_circuit, x25_port)
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
******************************************************************************/
enum qllc_rc_type qllc_qsm_sec();

/******************************************************************************
** Q_DISCONNECT (SECONDARY)
** enum qllc_rc_type qllc_qdisc_sec (link_station, virt_circuit, x25_port
**                                 stn_deleted)
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
**     bool                              *stn_deleted;
******************************************************************************/
enum qllc_rc_type qllc_qdisc_sec();

/******************************************************************************
** Q_XID_COMMAND (SECONDARY)
** enum qllc_rc_type qllc_qxid_cmd_sec (link_station, virt_circuit, x25_port)
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
******************************************************************************/
enum qllc_rc_type qllc_qxid_cmd_sec();

/******************************************************************************
** Q_TEST_COMMAND (SECONDARY)
** enum qllc_rc_type qllc_qtest_cmd_sec (link_station, virt_circuit, x25_port)
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
******************************************************************************/
enum qllc_rc_type qllc_qtest_cmd_sec();

/******************************************************************************
** Q_XID_RESPONSE (PRIMARY)
** enum qllc_rc_type qllc_qxid_rsp_pri (link_station, virt_circuit, x25_port)
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
******************************************************************************/
enum qllc_rc_type qllc_qxid_rsp_pri();

/******************************************************************************
** Q_TEST_RESPONSE (PRIMARY)
** enum qllc_rc_type qllc_qtest_rsp_pri (link_station, virt_circuit, x25_port)
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
******************************************************************************/
enum qllc_rc_type qllc_qtest_rsp_pri();

/******************************************************************************
** CLEAR PENDING COMMAND
** void qllc_clear_pending_cmd (link_station)
**     qllc_ls_type                      *link_station;
******************************************************************************/
void qllc_clear_pending_cmd();




/******************************************************************************
*******************************************************************************
** DEFINITIONS OF PROCEDURES EXPORTED OUTSIDE OF QLLC LINK STATION MANAGER
*******************************************************************************
******************************************************************************/

/******************************************************************************
** Q_RECEIVER_READY
** enum qllc_rc_type qllc_qrr (link_station)
**     qllc_ls_type                      *link_station;
******************************************************************************/
enum qllc_rc_type qllc_qrr();

/******************************************************************************
** Q_SET_MODE
** enum qllc_rc_type qllc_qsm (link_station, virt_circuit, x25_port)
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
******************************************************************************/
enum qllc_rc_type qllc_qsm();

/******************************************************************************
** Q_DISCONNECT
** enum qllc_rc_type qllc_qdisc (link_station, virt_circuit, x25_port,
**                                 stn_deleted)
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
**     bool                              *stn_deleted;
******************************************************************************/
enum qllc_rc_type qllc_qdisc();

/******************************************************************************
** Q_XID_COMMAND
** enum qllc_rc_type qllc_qxid_cmd (link_station, virt_circuit, x25_port)
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
******************************************************************************/
enum qllc_rc_type qllc_qxid_cmd();

/******************************************************************************
** Q_TEST_COMMAND
** enum qllc_rc_type qllc_qtest_cmd (link_station, virt_circuit, x25_port)
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
******************************************************************************/
enum qllc_rc_type qllc_qtest_cmd();

/******************************************************************************
** Q_UNNUMBERED_ACKNOWLEDGMENT
** enum qllc_rc_type qllc_qua (link_station, virt_circuit, x25_port)
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
******************************************************************************/
enum qllc_rc_type qllc_qua();

/******************************************************************************
** Q_REQUEST_DISCONNECT
** enum qllc_rc_type qllc_qrd (link_station, virt_circuit, x25_port)
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
******************************************************************************/
enum qllc_rc_type qllc_qrd();

/******************************************************************************
** Q_DISCONNECTED_MODE
** enum qllc_rc_type qllc_qdm (link_station, virt_circuit, x25_port)
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
******************************************************************************/
enum qllc_rc_type qllc_qdm();

/******************************************************************************
** Q_FRAME_REJECT
** enum qllc_rc_type qllc_qfrmr (link_station, virt_circuit, x25_port)
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
******************************************************************************/
enum qllc_rc_type qllc_qfrmr();

/******************************************************************************
** Q_XID_RESPONSE
** enum qllc_rc_type qllc_qxid_rsp (link_station, virt_circuit, x25_port)
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
******************************************************************************/
enum qllc_rc_type qllc_qxid_rsp();

/******************************************************************************
** Q_TEST_RESPONSE
** enum qllc_rc_type qllc_qtest_rsp (link_station, virt_circuit, x25_port)
**     qllc_ls_type                      *link_station;
**     x25_vc_type                       *virt_circuit;
**     x25_port_type                     *x25_port;
******************************************************************************/
enum qllc_rc_type qllc_qtest_rsp();
#else
/* PROTO DECLARATIONS */
enum qllc_rc_type qllc_common_cmd_pri (
  qllc_ls_type   *link_station,
  x25_vc_type    *virt_circuit,
  port_type      *x25_port);
enum qllc_rc_type qllc_common_rsp_sec (
    qllc_ls_type                      *link_station,
    x25_vc_type                       *virt_circuit,
    port_type                         *x25_port,
    qllc_qllu_type                    *qrsp);
enum qllc_rc_type qllc_qsm_sec (
    qllc_ls_type                      *link_station,
    x25_vc_type                       *virt_circuit,
    port_type                         *x25_port);
enum qllc_rc_type qllc_qdisc_sec (
    qllc_ls_type                      *link_station,
    x25_vc_type                       *virt_circuit,
    port_type                         *x25_port,
    bool                              *stn_deleted);
enum qllc_rc_type qllc_qxid_cmd_sec (
    qllc_ls_type                      *link_station,
    x25_vc_type                       *virt_circuit,
    port_type                         *x25_port);
enum qllc_rc_type qllc_qtest_cmd_sec (
    qllc_ls_type                      *link_station,
    x25_vc_type                       *virt_circuit,
    port_type                         *x25_port);
enum qllc_rc_type qllc_qxid_rsp_pri (
    qllc_ls_type                      *link_station,
    x25_vc_type                       *virt_circuit,
    port_type                         *x25_port);
enum qllc_rc_type qllc_qtest_rsp_pri (
    qllc_ls_type                      *link_station,
    x25_vc_type                       *virt_circuit,
    port_type                         *x25_port);
void qllc_clear_pending_cmd (
    qllc_ls_type                     *link_station);
enum qllc_rc_type qllc_qrr (
    qllc_ls_type                      *link_station);
enum qllc_rc_type qllc_qsm (
    qllc_ls_type                      *link_station,
    x25_vc_type                       *virt_circuit,
    port_type                         *x25_port);
enum qllc_rc_type qllc_qdisc (
    qllc_ls_type                      *link_station,
    x25_vc_type                       *virt_circuit,
    port_type                         *x25_port,
    bool                              *stn_deleted);
enum qllc_rc_type qllc_qxid_cmd (
    qllc_ls_type                      *link_station,
    x25_vc_type                       *virt_circuit,
    port_type                         *x25_port);
enum qllc_rc_type qllc_qtest_cmd (
    qllc_ls_type                      *link_station,
    x25_vc_type                       *virt_circuit,
    port_type                         *x25_port);
enum qllc_rc_type qllc_qua (
    qllc_ls_type                      *link_station,
    x25_vc_type                       *virt_circuit,
    port_type                         *x25_port);
enum qllc_rc_type qllc_qrd (
    qllc_ls_type                      *link_station,
    x25_vc_type                       *virt_circuit,
    port_type                         *x25_port);
enum qllc_rc_type qllc_qdm (
    qllc_ls_type                      *link_station,
    x25_vc_type                       *virt_circuit,
    port_type                         *x25_port);
enum qllc_rc_type qllc_qfrmr (
    qllc_ls_type                      *link_station,
    x25_vc_type                       *virt_circuit, 
    port_type                         *x25_port);
enum qllc_rc_type qllc_qxid_rsp (
    qllc_ls_type                      *link_station,
    x25_vc_type                       *virt_circuit,
    port_type                         *x25_port);
enum qllc_rc_type qllc_qtest_rsp (
    qllc_ls_type                      *link_station,
    x25_vc_type                       *virt_circuit,
    port_type                         *x25_port);
#endif


#endif
