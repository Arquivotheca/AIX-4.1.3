static char sccsid[] = "@(#)77  1.7.1.1  src/bos/kernext/dlc/qlc/qlcqfsm2.c, sysxdlcq, bos411, 9435D411a 9/2/94 10:02:30";
/*
 * COMPONENT_NAME: (SYSXDLCQ X.25 QLLC module
 *
 * FUNCTIONS: qllc_common_cmd_pri, qllc_qsm_sec, qllc_qdisc_sec,
 *            qllc_qxid_cmd_sec, qllc_qtest_cmd_sec, qllc_qxid_rsp_pri,
 *            qllc_qtest_rsp_pri, qllc_clear_pending_cmd, qllc_qrr, qllc_qsm,
 *            qllc_qdisc, qllc_qxid_cmd, qllc_qtest_cmd, qllc_qua, qllc_qrd,
 *            qllc_qdm, qllc_qfrmr, qllc_qxid_rsp, qllc_qtest_rsp
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

/****************************************************************************
** Description: This file contains code for QLLC finite state machine
**              procedures. The procedures included in this file are invoked
**              as a result of Q packets received from the QLLC link station
**              in the adjacent SNA node.
**
** Exported:    <functions defined for use within QLLC l.s. mgr>
**              enum qllc_rc_type qllc_common_cmd_pri();
**              enum qllc_rc_type qllc_common_rsp_sec();
**              enum qllc_rc_type qllc_qsm_sec();
**              enum qllc_rc_type qllc_qdisc_sec();
**              enum qllc_rc_type qllc_qxid_cmd_sec();
**              enum qllc_rc_type qllc_qtest_cmd_sec();
**              enum qllc_rc_type qllc_qxid_rsp_pri();
**              enum qllc_rc_type qllc_qtest_rsp_pri();
**              void qllc_clear_pending_cmd();
**
**              <functions defined for use outside QLLC l.s. mgr>
**              enum qllc_rc_type qllc_qrr();
**              enum qllc_rc_type qllc_qsm();
**              enum qllc_rc_type qllc_qdisc();
**              enum qllc_rc_type qllc_qxid_cmd();
**              enum qllc_rc_type qllc_qtest_cmd();
**              enum qllc_rc_type qllc_qua();
**              enum qllc_rc_type qllc_qrd();
**              enum qllc_rc_type qllc_qdm();
**              enum qllc_rc_type qllc_qfrmr();
**              enum qllc_rc_type qllc_qxid_rsp();
**              enum qllc_rc_type qllc_qtest_rsp();
** Imported:
**
*****************************************************************************/
#include "qlcg.h"
#include "qlcq.h"
#include "qlcv.h"
#include "qlcb.h"
#include "qlcp.h"
#include "qlcqfsm1.h"
#include "qlcqfsm2.h"
#include "qlcqmisc.h"
#include "qlcqerrs.h"
#include "qlcltime.h"

/******************************************************************************
** COMMON COMMAND PRIMARY
** Function:    qllc_common_cmd_pri
**
** Description: The behaviour of a QLLC primary link station is the same when
**              any QLLC command packet (except QRR) is received. A common
**              function is provided to handle this event. It is called on
**              receipt by a primary station of one of the following QLLUs:
**              QSM_Cmd, QDISC_Cmd, QXID_Cmd, QTEST_Cmd.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_remote_error- a protocol error caused by the remote QLLC
**                                link station was detected.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_common_cmd_pri (
  qllc_ls_type   *link_station,
  x25_vc_type    *virt_circuit,
  port_type      *x25_port)
{
  qvm_rc_type      qvm_rc;    /* used to hold return code from an X.25 fn */
  enum qllc_rc_type qllc_rc;  /* return code from this function */
  
  qvm_rc = qvm_rc_ok; 
  
  switch (link_station->state)
  {
  case qs_closed:
  case qs_opening:
    qllc_rc = qrc_remote_error;
    QLLC_REMOTE_ERROR (Q_RERROR_COM_CMD_PRI, link_station->correlator);
    break;
    
  case qs_closing:
  case qs_opened:
    link_station->state = qs_closed;
    link_station->predicate = qp_null;
    qvm_rc = qvm_clrst (
      virt_circuit,
      x25_port,
      UNEXPECTED_C_FIELD              /* diagnostic code */
      );
    qllc_rc = (qvm_rc == qvm_rc_ok) ? qrc_ok : qrc_x25_error;
    /* some implementations may need a status report on state change */
    QLLC_REPORT_STATUS (
      qllc_rc,
      qsr_entered_closed_state,
      link_station->correlator
      );
    break;
    
  case qs_inop:
    QLLC_LOCAL_ERROR (Q_LERROR_COM_CMD_PRI, link_station->correlator);
    qllc_rc = qrc_local_error;
    break;
  }
  return (qllc_rc);
}

/******************************************************************************
** COMMON RESPONSE SECONDARY
** Function:    qllc_common_rsp_sec
**
** Description: The behaviour of a QLLC secondary link station is the same when
**              any QLLC response packet is received. A common function is
**              provided to handle this event. The function is called when a
**              secondary station receives one of the following QLLUs:
**              QRD_Rsp, QXID_Rsp, QTEST_Rsp, QUA_Rsp, QDM_Rsp, QFRMR_Rsp
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**              qrsp            - the QLLU response received
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_remote_error- a protocol error caused by the remote QLLC
**                                link station was detected.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_common_rsp_sec (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port,
  qllc_qllu_type                    *qrsp)
{
  qvm_rc_type      qvm_rc;    /* used to hold return code from an X.25 fn */
  enum qllc_rc_type qllc_rc;  /* return code from this function */

  qvm_rc = qvm_rc_ok;     
  
  switch (link_station->state)
  {
  case qs_closed:
  case qs_opening:
  case qs_closing:
  case qs_opened:
    link_station->state = qs_recovery;
    qvm_rc = qvm_send_qfrmr (
      virt_circuit,
      x25_port,
      qrsp->control_field,
      qfr_response_received_by_secondary_station
      );
    qllc_rc = qrc_ok;
    /* some implementations need a status report on state change */
    QLLC_REPORT_STATUS (
      qllc_rc,
      qsr_entered_recovery_state,
      link_station->correlator
      );
    break;
    
  case qs_recovery:
    qvm_rc = qvm_send_qfrmr (
      virt_circuit,
      x25_port,
      qrsp->control_field,
      qfr_response_received_by_secondary_station
      );
    QLLC_REMOTE_ERROR (Q_RERROR_COM_RSP_SEC, link_station->correlator);
    qllc_rc = qrc_remote_error;
    break;
    
  case qs_inop:
    QLLC_LOCAL_ERROR (Q_LERROR_COM_RSP_SEC, link_station->correlator);
    qllc_rc = qrc_local_error;
    break;
  }
  if (qvm_rc != qvm_rc_ok)
    qllc_rc = qrc_x25_error;
  return (qllc_rc);
}

/******************************************************************************
** Q_SET_MODE (SECONDARY)
** Function:    qllc_qsm_sec
**
** Description: This function is invoked when a secondary link station receives
**              QSM_Cmd packet.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_remote_error- a protocol error caused by the remote QLLC
**                                link station was detected.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_qsm_sec (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port)
{
  qvm_rc_type      qvm_rc;    /* used to hold return code from an X.25 fn */
  enum qllc_rc_type qllc_rc;  /* return code from this function */
  qllc_qllu_type qdm;         /* structure to hold a QDM_Rsp */
  qllc_qllu_type qua;         /* structure to hold a QUA_Rsp */
  qllc_qllu_type qrd;         /* structure to hold a QRD_Rsp */
  
  register enum qllc_state_type state = link_station->state;
  register enum qllc_pred_type predicate = link_station->predicate;
  
  qvm_rc = qvm_rc_ok; 
  
  switch (state)
  {
  case qs_closed:
    if ((predicate == qp_oxrp) || (predicate == qp_otrp))
    {
      link_station->predicate = qp_null;
      qvm_rc = qvm_clrst (
	virt_circuit,
	x25_port,
	UNEXPECTED_C_FIELD          /* diagnostic code */
	);
      QLLC_REMOTE_ERROR (Q_RERROR_QSM_SEC, link_station->correlator);
      qllc_rc = qrc_remote_error;
    }
    else /* predicate not in {qp_oxrp, qp_otrp} */
    {
      qdm.control_field = (byte)qdm_rsp;
      qdm.address_field = (byte)response;
      qdm.info_field = NULL;
      qdm.already_sent = FALSE;
      qvm_rc = qvm_send_qllu (
	virt_circuit,
	x25_port,
	&qdm
	);
      qllc_rc = qrc_ok;
    }
    break;
    
  case qs_opening:
    if ((predicate == qp_oxrp) || (predicate == qp_otrp))
    {
      link_station->state = qs_closed;
      link_station->predicate = qp_null;
      qvm_rc = qvm_clrst (
	virt_circuit,
	x25_port,
	UNEXPECTED_C_FIELD          /* diagnostic code */
	);
      qllc_rc = qrc_ok;
      /* some implementations need a status report on state change */
      QLLC_REPORT_STATUS (
	qllc_rc,
	qsr_entered_recovery_state,
	link_station->correlator
	);
    }
    else /* predicate not in {qp_oxrp, qp_otrp} */
    {
      link_station->state = qs_opened;
      qua.control_field = (byte)qua_rsp;
      qua.address_field = (byte)response;
      qua.info_field = NULL;
      qua.already_sent = FALSE;
      qvm_rc = qvm_send_qllu (
	virt_circuit,
	x25_port,
	&qua
	);
      qllc_rc = qrc_ok;
      /* some implementations need a status report on state change */
      qlm_ls_contacted(link_station->correlator);
      QLLC_REPORT_STATUS (
	qllc_rc,
	qsr_entered_opened_state,
	link_station->correlator
	);
    }
    break;
    
  case qs_closing:
    qrd.control_field = (byte)qrd_rsp;
    qrd.address_field = (byte)response;
    qrd.info_field = NULL;
    qrd.already_sent = FALSE;
    qvm_rc = qvm_send_qllu (
      virt_circuit,
      x25_port,
      &qrd
      );
    qllc_rc = qrc_ok;
    break;
    
  case qs_recovery:
  case qs_opened:
    link_station->state = qs_closed;
    link_station->predicate = qp_null;
    qdm.control_field = (byte)qdm_rsp;
    qdm.address_field = (byte)response;
    qdm.info_field = NULL;
    qdm.already_sent = FALSE;
    qvm_rc = qvm_send_qllu (
      virt_circuit,
      x25_port,
      &qdm
      );
    qllc_rc = qrc_ok;
    /* some implementations need a status report on state change */
    QLLC_REPORT_STATUS (
      qllc_rc,
      qsr_entered_closed_state,
      link_station->correlator
      );
    break;
    
  case qs_inop:
    QLLC_LOCAL_ERROR (Q_LERROR_QSM_SEC, link_station->correlator);
    qllc_rc = qrc_local_error;
    break;
  }
  if (qvm_rc != qvm_rc_ok)
    qllc_rc = qrc_x25_error;
  return (qllc_rc);
  
}

/******************************************************************************
** Q_DISCONNECT (SECONDARY)
** Function:    qllc_qdisc_sec
**
** Description: This function is called when a secondary station receives a
**              QDISC_Cmd packet.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_qdisc_sec (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port,
  bool                              *stn_deleted)
{
  qvm_rc_type      qvm_rc;    /* used to hold return code from an X.25 fn */
  enum qllc_rc_type qllc_rc;  /* return code from this function */
  qllc_qllu_type qdm;         /* structure to hold a QDM_Rsp */
  qllc_qllu_type qua;         /* structure to hold a QUA_Rsp */
  
  qvm_rc = qvm_rc_ok;   

  switch (link_station->state)
  {
  case qs_closed:
    qdm.control_field = (byte)qdm_rsp;
    qdm.address_field = (byte)response;
    qdm.info_field = NULL;
    qdm.already_sent = FALSE;
    qvm_rc = qvm_send_qllu (
      virt_circuit,
      x25_port,
      &qdm
      );
    qllc_rc = qrc_ok;
    break;
    
  case qs_opening:
    link_station->state = qs_closed;
    qdm.control_field = (byte)qdm_rsp;
    qdm.address_field = (byte)response;
    qdm.info_field = NULL;
    qdm.already_sent = FALSE;
    qvm_rc = qvm_send_qllu (
      virt_circuit,
      x25_port,
      &qdm
      );
    qllc_rc = qrc_ok;
    /* some implementations need a status report on state change */
    QLLC_REPORT_STATUS (
      qllc_rc,
      qsr_entered_closed_state,
      link_station->correlator
      );
    break;
    
  case qs_closing:
  case qs_recovery:
  case qs_opened:
    link_station->state = qs_closing;
    link_station->predicate = qp_null;
    qua.control_field = (byte)qua_rsp;
    qua.address_field = (byte)response;
    qua.info_field = NULL;
    qua.already_sent = FALSE;
    qvm_rc = qvm_send_qllu (
      virt_circuit,
      x25_port,
      &qua
      );
    qllc_rc = qrc_ok;

    /* handle remote discontact */
    qlm_station_remote_discontact(link_station->correlator, stn_deleted);

    /* some implementations need a status report on state change */
    QLLC_REPORT_STATUS (
      qllc_rc,
      qsr_entered_closing_state,
      link_station->correlator
      );
    break;
    
  case qs_inop:
    QLLC_LOCAL_ERROR (Q_LERROR_QDISC_SEC, link_station->correlator);
    qllc_rc = qrc_local_error;
    break;
  }
  if (qvm_rc != qvm_rc_ok)
    qllc_rc = qrc_x25_error;
  return (qllc_rc);
  
}

/******************************************************************************
** Q_XID_COMMAND (SECONDARY)
** Function:    qllc_qxid_cmd_sec
**
** Description: This function is called when a secondary station receives a
**              QXID_Cmd packet.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_remote_error- a protocol error caused by the remote QLLC
**                                link station was detected.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
**
******************************************************************************/
enum qllc_rc_type qllc_qxid_cmd_sec (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port)
{
  qvm_rc_type      qvm_rc;    /* used to hold return code from an X.25 fn */
  enum qllc_rc_type qllc_rc;  /* return code from this function */
  qllc_qllu_type qdm;         /* structure to hold a QDM_Rsp */
  
  register enum qllc_state_type state = link_station->state;
  register enum qllc_pred_type predicate = link_station->predicate;
  
  qvm_rc = qvm_rc_ok;    

  switch (state)
  {
  case qs_opening:
  case qs_opened:
  case qs_closed:
    switch (predicate)
    {
    case qp_oxrp:
    case qp_otrp:
      if (state == qs_closed)
      {
	link_station->state = qs_recovery;
	qvm_rc = qvm_send_qfrmr (
	  virt_circuit,
	  x25_port,
	  (byte)qxid_cmd,
	  qfr_qxid_qtest_cmd_received_in_invalid_state
	  );
	qllc_rc = qrc_ok;
	/* may need status report */
	QLLC_REPORT_STATUS (
	  qllc_rc,
	  qsr_entered_recovery_state,
	  link_station->correlator
	  );
      }
      else if (state == qs_opened)
      {
	link_station->state = qs_closed;
	link_station->predicate = qp_null;
	qdm.control_field = (byte)qdm_rsp;
	qdm.address_field = (byte)response;
	qdm.info_field = NULL;
	qdm.already_sent = FALSE;
	qvm_rc = qvm_send_qllu (
	  virt_circuit,
	  x25_port,
	  &qdm
	  );
	qllc_rc = qrc_ok;
	/* some implemetations need a status report */
	QLLC_REPORT_STATUS (
	  qllc_rc,
	  qsr_entered_closed_state,
	  link_station->correlator
	  );
      }
      else /* if (state == qs_opening) */
      {
	link_station->predicate = qp_oxrp;
	qllc_rc = qrc_ok;
	/* some implementations need a status report */
	QLLC_REPORT_STATUS (
	  qllc_rc,
	  qsr_xid_cmd_received,
	  link_station->correlator);
      }
      break;
      
    default:
      link_station->predicate = qp_oxrp;
      qllc_rc = qrc_ok;
      /* some implementations need a status report */
      QLLC_REPORT_STATUS (
	qllc_rc,
	qsr_xid_cmd_received,
	link_station->correlator
	);
      break;
    }
    break;
    
  case qs_closing:
    qllc_rc = qrc_remote_error;
    QLLC_REMOTE_ERROR (
	        Q_RERROR_XID_CMD_SEC_1,
      link_station->correlator
      );
    break;
    
  case qs_recovery:
    qvm_rc = qvm_send_qfrmr (
      virt_circuit,
      x25_port,
      (byte)qxid_cmd,
      qfr_qxid_qtest_cmd_received_in_invalid_state
      );
    qllc_rc = qrc_remote_error;
    QLLC_REMOTE_ERROR (
      Q_RERROR_XID_CMD_SEC_2,
      link_station->correlator
      );
    break;
    
  case qs_inop:
    QLLC_LOCAL_ERROR (Q_LERROR_QXID_CMD_SEC, link_station->correlator);
    qllc_rc = qrc_local_error;
    break;
  }
  if (qvm_rc != qvm_rc_ok)
    qllc_rc = qrc_x25_error;
  return (qllc_rc);
}

/******************************************************************************
** Q_TEST_COMMAND (SECONDARY)
** Function:    qllc_qtest_cmd_sec
**
** Description: This function is called each time a secondary station receives
**              a QTEST_Cmd packet.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_remote_error- a protocol error caused by the remote QLLC
**                                link station was detected.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_qtest_cmd_sec (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port)
{
  qvm_rc_type      qvm_rc;    /* used to hold return code from an X.25 fn */
  enum qllc_rc_type qllc_rc;  /* return code from this function */
  qllc_qllu_type qdm;         /* structure to hold a QDM_Rsp */
  
  register enum qllc_state_type state = link_station->state;
  register enum qllc_pred_type predicate = link_station->predicate;
  
  qvm_rc = qvm_rc_ok; 

  switch (state)
  {
  case qs_opening:
  case qs_opened:
  case qs_closed:
    switch (predicate)
    {
    case qp_oxrp:
    case qp_otrp:
      if (state == qs_closed)
      {
	link_station->state = qs_recovery;
	qvm_rc = qvm_send_qfrmr (
	  virt_circuit,
	  x25_port,
	  (byte)qtest_cmd,
	  qfr_qxid_qtest_cmd_received_in_invalid_state
	  );
	qllc_rc = qrc_remote_error;
	QLLC_REMOTE_ERROR (
	  Q_RERROR_TEST_CMD_SEC_1,
	  link_station->correlator
	  );
      }
      else if (state == qs_opened)
      {
	link_station->state = qs_closed;
	link_station->predicate = qp_null;
	qdm.control_field = (byte)qdm_rsp;
	qdm.address_field = (byte)response;
	qdm.info_field = NULL;
	qdm.already_sent = FALSE;
	qvm_rc = qvm_send_qllu (
	  virt_circuit,
	  x25_port,
	  &qdm
	  );
	qllc_rc = qrc_ok;
	/* some implementations may need status report */
	QLLC_REPORT_STATUS (
	  qllc_rc,
	  qsr_entered_closed_state,
	  link_station->correlator
	  );
      }
      else /* if (state == qs_opening) */
      {
	link_station->predicate = qp_otrp;
	qllc_rc = qrc_ok;
	/* some implementations need status report */
	QLLC_REPORT_STATUS (
	  qllc_rc,
	  qsr_test_cmd_received,
	  link_station->correlator
	  );
      }
      break;
      
    default:
      link_station->predicate = qp_otrp;
      qllc_rc = qrc_ok;
      /* some implementations need a status report */
      QLLC_REPORT_STATUS (
	qllc_rc,
	qsr_test_cmd_received,
	link_station->correlator
	);
      break;
    }
    break;
    
  case qs_closing:
    qllc_rc = qrc_remote_error;
    QLLC_REMOTE_ERROR (
      Q_RERROR_TEST_CMD_SEC_2,
      link_station->correlator
      );
    break;
    
  case qs_recovery:
    qvm_rc = qvm_send_qfrmr (
      virt_circuit,
      x25_port,
      (byte)qtest_cmd,
      qfr_qxid_qtest_cmd_received_in_invalid_state
      );
    qllc_rc = qrc_remote_error;
    QLLC_REMOTE_ERROR (
      Q_RERROR_TEST_CMD_SEC_3,
      link_station->correlator
      );
    break;
    
  case qs_inop:
    QLLC_LOCAL_ERROR (Q_LERROR_QXID_CMD_SEC, link_station->correlator);
    qllc_rc = qrc_local_error;
    break;
  }
  if (qvm_rc != qvm_rc_ok)
    qllc_rc = qrc_x25_error;
  return (qllc_rc);
}

/******************************************************************************
** Q_XID_RESPONSE (PRIMARY)
** Function:    qllc_qxid_rsp_pri
**
** Description: This function is called each time a primary station receives
**              a QXID_Rsp packet.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_remote_error- a protocol error caused by the remote QLLC
**                                link station was detected.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_qxid_rsp_pri (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port)
{
  qvm_rc_type      qvm_rc;    /* used to hold return code from an X.25 fn */
  enum qllc_rc_type qllc_rc;  /* return code from this function */
  
  register enum qllc_state_type state = link_station->state;
  register enum qllc_pred_type predicate = link_station->predicate;
  
  qvm_rc = qvm_rc_ok; 
  
  switch (state)
  {
  case qs_opening:
  case qs_closing:
    qllc_rc = qrc_remote_error;
    QLLC_REMOTE_ERROR (
      Q_RERROR_XID_RSP_PRI_1,
      link_station->correlator
      );
    break;
    
  case qs_closed:
/* defect 52838 */
    if /* an incomming xid response is pending, or this station is negotiable */
       ((predicate == qp_ixrp) || (link_station->negotiable))
    {
      link_station->predicate = qp_null;
/* end defect 52838 */
      qllc_rc = qrc_ok;
    }
    else /* if (predicate != qp_ixrp) */
    {
      link_station->predicate = qp_null;
      qllc_rc = qrc_remote_error;
      QLLC_REMOTE_ERROR (
	Q_RERROR_XID_RSP_PRI_2,
	link_station->correlator
	);
    }
    break;
    
  case qs_opened:
    if (predicate == qp_ixrp)
    {
      link_station->predicate = qp_null;
      qllc_rc = qrc_ok;
    }
    else /* if (predicate != qp_ixrp) */
    {
      link_station->state = qs_closed;
      link_station->predicate = qp_null;
      qllc_rc = qrc_ok;
      /* some implementations need a status report */
      QLLC_REPORT_STATUS (
	qllc_rc,
	qsr_entered_closed_state,
	link_station->correlator
	);
      qvm_rc = qvm_clrst (
	virt_circuit,
	x25_port,
	UNEXPECTED_C_FIELD
	);
    }
    break;
    
  case qs_inop:
    QLLC_LOCAL_ERROR (Q_LERROR_QXID_RSP_PRI, link_station->correlator);
    qllc_rc = qrc_local_error;
    break;
    
  case qs_recovery:
    QLLC_PROGRAM_ERROR (
      Q_PERROR_QXID_RSP_PRI,
      link_station->correlator
      );
    qllc_rc = qrc_program_error;
    break;
  }
  if (qvm_rc != qvm_rc_ok)
    qllc_rc = qrc_x25_error;
  return (qllc_rc);
}

/******************************************************************************
** Q_TEST_RESPONSE (PRIMARY)
** Function:    qllc_qtest_rsp_pri
**
** Description: This function is called each time a primary link station
**              receives a QTEST_Rsp.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_remote_error- a protocol error caused by the remote QLLC
**                                link station was detected.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_qtest_rsp_pri (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port)
{
  qvm_rc_type      qvm_rc;    /* used to hold return code from an X.25 fn */
  enum qllc_rc_type qllc_rc;  /* return code from this function */
  
  register enum qllc_state_type state = link_station->state;
  register enum qllc_pred_type predicate = link_station->predicate;
  
  qvm_rc = qvm_rc_ok;  

  switch (state)
  {
  case qs_opening:
  case qs_closing:
    qllc_rc = qrc_remote_error;
    QLLC_REMOTE_ERROR (
      Q_RERROR_TEST_RSP_PRI_1,
      link_station->correlator
      );
    break;
    
  case qs_closed:
  case qs_opened:
    switch (predicate)
    {
    case qp_itrp:
      link_station->predicate = qp_null;
      qllc_rc = qrc_ok;
      break;
      
    default:
      if (state == qs_closed)
      {
	qllc_rc = qrc_remote_error;
	QLLC_REMOTE_ERROR (
	  Q_RERROR_TEST_RSP_PRI_2,
	  link_station->correlator
	  );
      }
      else /* if (state = qs_opened) */
      {
	link_station->state = qs_closed;
	link_station->predicate = qp_null;
	qllc_rc = qrc_ok;
	/* some implementations need a status report */
	QLLC_REPORT_STATUS (
	  qllc_rc,
	  qsr_entered_closed_state,
	  link_station->correlator
	  );
	qvm_rc = qvm_clrst (
	  virt_circuit,
	  x25_port,
	  UNEXPECTED_C_FIELD
	  );
      }
      break;
    }
    break;
    
  case qs_inop:
    QLLC_LOCAL_ERROR (
      Q_LERROR_QTEST_RSP_PRI,
      link_station->correlator
      );
    qllc_rc = qrc_local_error;
    break;
    
  case qs_recovery:
    QLLC_PROGRAM_ERROR (
      Q_PERROR_QTEST_RSP_PRI,
      link_station->correlator
      );
    qllc_rc = qrc_program_error;
    break;
  }
  if (qvm_rc != qvm_rc_ok)
    qllc_rc = qrc_x25_error;
  return (qllc_rc);
}

/******************************************************************************
** CLEAR PENDING COMMAND
** Function:    qllc_clear_pending_cmd
**
** Description: This function is called when a response is received by a QLLC
**              primary link station, in order to clear any record of a QLLC
**              command which is awaiting acknowledgment.
**
** Parameters:  link_station    - the link station instance
**
******************************************************************************/
void qllc_clear_pending_cmd (
  qllc_ls_type                     *link_station)
{
  if (link_station->role == qr_primary)
  {
    /*********************************************************************/
    /* Cancel repoll timer                                               */
    /*********************************************************************/
    w_stop(&(link_station->repoll_dog));
    /**********************************************************************
     ** There's a nasty dependency here, which we tried hard to avoid
     ** in the QLLC link station manager. But only QLLC knows when a
     ** command has been acknowledged, and so the pending buffer can be
     ** freed.
     **********************************************************************/
    if (link_station->pending_cmd[0].used == TRUE)
    {
      QBM_FREE_BUFFER(link_station->pending_cmd[0].qllu.info_field);
    }
    link_station->pending_cmd[0].used = FALSE;
    link_station->command_contiguous_repolls = 0;
    /**********************************************************************/
    /* Force the repoll_due indicator to False in case the timer pinged   */
    /**********************************************************************/
    link_station->repoll_due = FALSE;
  }
  else /* if (link_station->role = qr_secondary) */
  {
    QLLC_PROGRAM_ERROR (Q_PERROR_CLR_PEND, link_station->correlator);
  }
}

/******************************************************************************
** Q_RECEIVER_READY
** Function:    qllc_qrr
**
** Description: This function is called each time a QLLC link station receives
**              a QRR_Cmd packet.
**
** Parameters:  link_station    - the link station instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_qrr (
  qllc_ls_type                      *link_station)
{
  enum qllc_rc_type qllc_rc;  /* return code from this function */
  
  if (   (link_station->state == qs_opening)
    && (link_station->role == qr_secondary)
    && (link_station->predicate == qp_ctp))
  {
    link_station->state = qs_opened;
    link_station->predicate = qp_null;
    qllc_rc = qrc_ok;
    /* some implementations need a status report on state change */
    qlm_ls_contacted(link_station->correlator);
    QLLC_REPORT_STATUS (
      qllc_rc,
      qsr_entered_opened_state,
      link_station->correlator
      );
  }
  else if (link_station->state == qs_inop)
  {
    QLLC_LOCAL_ERROR (Q_LERROR_QRR, link_station->correlator);
    qllc_rc = qrc_local_error;
  }
  else
  {
    /* ignore logical link unit */
    qllc_rc = qrc_ok;
  }
  return (qllc_rc);
}

/******************************************************************************
** Q_SET_MODE
** Function:    qllc_qsm
**
** Description: This function is called each time a QSM_Cmd packet is received
**              by a QLLC link station. The function simply performs a branch
**              according to station role, as the real protocol machinery is
**              contained in funcs qllc_qsm_sec() and qllc_common_cmd_pri().
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_remote_error- a protocol error caused by the remote QLLC
**                                link station was detected.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_qsm (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port)
{
  enum qllc_rc_type qllc_rc;  /* the return code from this function */
  
  if (link_station->role == qr_primary)
    qllc_rc = qllc_common_cmd_pri (
      link_station,
      virt_circuit,
      x25_port
      );
  else /* if (link_station->role == qr_secondary) */
    qllc_rc = qllc_qsm_sec (
      link_station,
      virt_circuit,
      x25_port
      );
  return (qllc_rc);
}

/******************************************************************************
** Q_DISCONNECT
** Function:    qllc_qdisc
**
** Description: This function is called each time a QLLC link station receives
**              a QDISC_Cmd packet. The function simply performs a branch
**              according to station role, as the real protocol machinery is
**              contained in the functions qllc_common_cmd_pri() and
**              qllc_qdisc_sec().
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_remote_error- a protocol error caused by the remote QLLC
**                                link station was detected.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_qdisc (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port,
  bool                              *stn_deleted)
{
  enum qllc_rc_type qllc_rc;  /* the return code from this function */
  
  if (link_station->role == qr_primary)
    qllc_rc = qllc_common_cmd_pri (
      link_station,
      virt_circuit,
      x25_port
      );
  else /* if (link_station->role == qr_secondary) */
    qllc_rc = qllc_qdisc_sec (
      link_station,
      virt_circuit,
      x25_port,
      stn_deleted
      );
  return (qllc_rc);
}

/******************************************************************************
** Q_XID_COMMAND
** Function:    qllc_qxid_cmd
**
** Description: This function is called each time a QLLC link station receives
**              a QXID_Cmd packet. The function simply performs a branch
**              according to station role, as the real protocol machinery is
**              contained in the functions qllc_common_cmd_pri() and
**              qllc_qxid_cmd_sec().
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_remote_error- a protocol error caused by the remote QLLC
**                                link station was detected.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_qxid_cmd (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port)
{
  enum qllc_rc_type qllc_rc;  /* the return code from this function */

  if (link_station->role == qr_primary)
    qllc_rc = qllc_common_cmd_pri (
      link_station,
      virt_circuit,
      x25_port
      );
  else /* if (link_station->role == qr_secondary) */
    qllc_rc = qllc_qxid_cmd_sec (
      link_station,
      virt_circuit,
      x25_port
      );
  return (qllc_rc);
}

/******************************************************************************
** Q_TEST_COMMAND
** Function:    qllc_qtest_cmd
**
** Description: This function is called each time a QTEST_Cmd packet is
**              received by a QLLC link station. The function simply performs
**              a branch according to station role, as the real protocol
**              machinery is contained in the functions qllc_common_cmd_pri()
**              and qllc_qtest_cmd_sec().
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_remote_error- a protocol error caused by the remote QLLC
**                                link station was detected.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_qtest_cmd (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port)
{
  enum qllc_rc_type qllc_rc;  /* the return code from this function */
  
  if (link_station->role == qr_primary)
    qllc_rc = qllc_common_cmd_pri (
      link_station,
      virt_circuit,
      x25_port
      );
  else /* if (link_station->role == qr_secondary) */
    qllc_rc = qllc_qtest_cmd_sec (
      link_station,
      virt_circuit,
      x25_port
      );
  return (qllc_rc);
}

/******************************************************************************
** Q_UNNUMBERED_ACKNOWLEDGMENT
** Function:    qllc_qua
**
** Description: This function is called each time a QUA_Rsp packet is received
**              by a QLLC link station
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_remote_error- a protocol error caused by the remote QLLC
**                                link station was detected.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_qua (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port)
{
  enum qllc_rc_type qllc_rc;  /* the return code from this function */
  qllc_qllu_type qua;         /* structure to hold a qua response */
  qvm_rc_type qvm_rc;
  
  if (link_station->role == qr_primary)
  {
    switch (link_station->state)
    {
    case qs_opened:
    case qs_closed:
      qllc_rc = qrc_remote_error;
      QLLC_REMOTE_ERROR (Q_RERROR_UA, link_station->correlator);
      break;
      
    case qs_opening:
      link_station->state = qs_opened;
      qllc_rc = qrc_ok;
      /* some implementations need a status report */
      qlm_ls_contacted(link_station->correlator);
      QLLC_REPORT_STATUS (
	qllc_rc,
	qsr_entered_opened_state,
	link_station->correlator
	);
      break;
      
    case qs_closing:
      qvm_rc = qvm_close_vc(
        virt_circuit,
        x25_port, 
        NORMAL_TERMINATION,
        FALSE
        );

      qllc_rc = (qvm_rc == qvm_rc_ok) ? qrc_ok : qrc_local_error;
      link_station->state = qs_closed;
      link_station->predicate = qp_null;

      /* some implementations need a status report */
      QLLC_REPORT_STATUS (
	qllc_rc,
	qsr_entered_closed_state,
	link_station->correlator
	);
      break;
      
    case qs_inop:
      QLLC_LOCAL_ERROR (Q_LERROR_QUA, link_station->correlator);
      qllc_rc = qrc_local_error;
      break;
    }
    qllc_clear_pending_cmd (link_station);
  }
  else /* if (link_station->role == qr_secondary) */
  {
    /* we build a full qllu structure here, even though it's just the    */
    /* control field which is needed by qllc_common_rsp_sec().           */
    /* there's no good reason for this except that it matches the design */
    /* better, since in SEDL qllc_qllu_type does not distinguish control */
    /* and address fields.                                               */
    qua.control_field = (byte)qua_rsp;
    qua.address_field = (byte)response;
    qua.info_field = NULL;
    qua.already_sent = FALSE;
    qllc_rc = qllc_common_rsp_sec (
      link_station,
      virt_circuit,
      x25_port,
      &qua
      );
  }
  return (qllc_rc);
}

/******************************************************************************
** Q_REQUEST_DISCONNECT
** Function:    qllc_qrd
**
** Description: This function is called each time a QLLC receives a QRD_Rsp
**              packet.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_remote_error- a protocol error caused by the remote QLLC
**                                link station was detected.
**              qrc_qrd_received_in_opening_state
**                              - this event must be reported to a higher layer
**                                according to the architecture.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_qrd (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port)
{
  enum qllc_rc_type qllc_rc;  /* the return code from this function */
  qllc_qllu_type qdisc;       /* structure to hold a qdisc command */
  qllc_qllu_type qrd;         /* structure to hold a qrd response */
  
  outputf("QLLC_QRD:\n");
  if (link_station->role == qr_primary)
  {
    outputf("QLLC_QRD: primary\n");
    qllc_clear_pending_cmd (link_station);
    switch (link_station->state)
    {
    case qs_closed:
      outputf("QLLC_QRD: closed\n");
      qdisc.control_field = (byte)qdisc_cmd;
      qdisc.address_field = (byte)command;
      qdisc.info_field = NULL;
      qdisc.already_sent = FALSE;
      link_station->counter = link_station->max_repoll;
      qllc_rc = qllc_cmd_poll (
	link_station,
	virt_circuit,
	x25_port,
	&qdisc
	);
      QLLC_REMOTE_ERROR (Q_RERROR_RD, link_station->correlator);
      if (qllc_rc != qrc_x25_error)
	qllc_rc = qrc_remote_error;
      break;
      
    case qs_opening:
      outputf("QLLC_QRD: opening\n");
      /* architecture says 'inform higher layer'. */
      /* this is the best we can manage.          */
      qllc_rc = qrc_qrd_received_in_opening_state;
      break;
      
    case qs_closing:
      outputf("QLLC_QRD: closing\n");
      qllc_rc = qrc_ok;
      /* some implementations need a status report */
      QLLC_REPORT_STATUS (
	qllc_rc,
	qsr_rd_rsp_received,
	link_station->correlator
	);
      break;
      
    case qs_opened:
      outputf("QLLC_QRD: opened\n");
      qdisc.control_field = (byte)qdisc_cmd;
      qdisc.address_field = (byte)command;
      qdisc.info_field = NULL;
      qdisc.already_sent = FALSE;
      link_station->state = qs_closing;
      link_station->counter = link_station->max_repoll;
      qllc_rc = qllc_cmd_poll (
	link_station,
	virt_circuit,
	x25_port,
	&qdisc
	);
      /* some implementations need a status report */
      QLLC_REPORT_STATUS (
	qllc_rc,
	qsr_entered_closing_state,
	link_station->correlator
	);
      break;
      
    case qs_inop:
      outputf("QLLC_QRD: inop\n");
      QLLC_LOCAL_ERROR (Q_LERROR_QRD, link_station->correlator);
      qllc_rc = qrc_local_error;
      break;
    }
  }
  else /* if (link_station->role == secondary) */
  {
    outputf("QLLC_QRD: secondary\n");
    qrd.control_field = (byte)qrd_rsp;
    qrd.address_field = (byte)response;
    qrd.info_field = NULL;
    qrd.already_sent = FALSE;
    qllc_rc = qllc_common_rsp_sec (
      link_station,
      virt_circuit,
      x25_port,
      &qrd
      );
  }
  return (qllc_rc);
}

/******************************************************************************
** Q_DISCONNECTED_MODE
** Function:    qllc_qdm
**
** Description: This function is called each time a QLLC link station receives
**              a QDM_Rsp packet.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_remote_error- a protocol error caused by the remote QLLC
**                                link station was detected.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_qdm (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port)
{
  enum qllc_rc_type qllc_rc;  /* the return code from this function */
  qvm_rc_type      qvm_rc;    /* holds return code from X.25 functions */
  qllc_qllu_type qdm;         /* structure to hold a qdm response */
  
  qvm_rc = qvm_rc_ok;   

  if (link_station->role == qr_primary)
  {
    qllc_clear_pending_cmd (link_station);
    switch (link_station->state)
    {
    case qs_closed:
      link_station->predicate = qp_null;
      qllc_rc = qrc_ok;
      break;
      
    case qs_opened:
      /* for OPENED link stations, clear/reset the VC    */
      /* and then act as for OPENING or CLOSING stations */
      qvm_rc = qvm_clrst (
	virt_circuit,
	x25_port,
	UNEXPECTED_C_FIELD
	);
      /* no break ! */
      
    case qs_opening:
    case qs_closing:
      link_station->state = qs_closed;
      link_station->predicate = qp_null;
      qllc_rc = qrc_ok;
      /* some implementations need a status report on state change */
      QLLC_REPORT_STATUS (
	qllc_rc,
	qsr_entered_closed_state,
	link_station->correlator
	);
      break;

      
    case qs_inop:
      QLLC_LOCAL_ERROR (Q_LERROR_QDM, link_station->correlator);
      qllc_rc = qrc_local_error;
      break;
    }
  }
  else /* if (link_station->role == secondary) */
  {
    qdm.control_field = (byte)qdm_rsp;
    qdm.address_field = (byte)response;
    qdm.info_field = NULL;
    qdm.already_sent = FALSE;
    qllc_rc = qllc_common_rsp_sec (
      link_station,
      virt_circuit,
      x25_port,
      &qdm
      );
  }
  if (qvm_rc != qvm_rc_ok)
    qllc_rc = qrc_x25_error;
  return (qllc_rc);
}

/******************************************************************************
** Q_FRAME_REJECT
** Function:    qllc_qfrmr
**
** Description: This function is invoked each time a QLLC link station receives
**              a QFRMR_Rsp packet.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_remote_error- a protocol error caused by the remote QLLC
**                                link station was detected.
**              qrc_qfrmr_received_in_opening_or_closed_state
**                              - this event must be reported to a higher layer
**                                according to the architecture.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_qfrmr (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit, 
  port_type                         *x25_port)
{
  enum qllc_rc_type qllc_rc;  /* the return code from this function */
  qvm_rc_type      qvm_rc;    /* holds return code from X.25 functions */
  qllc_qllu_type qfrmr;       /* structure to hold a qfrmr response */
  
  qvm_rc = qvm_rc_ok;

  if (link_station->role == qr_primary)
  {
    qllc_clear_pending_cmd (link_station);
    switch (link_station->state)
    {
    case qs_closed:
    case qs_opening:
      /* architecture says 'inform higher layer'. */
      /* this is the best we can manage.          */
      qllc_rc = qrc_qfrmr_received_in_opening_or_closed_state;
      break;
      
    case qs_opened:
    case qs_closing:
      link_station->state = qs_closed;
      link_station->predicate = qp_null;
      qvm_rc = qvm_clrst (
	virt_circuit,
	x25_port,
	UNEXPECTED_C_FIELD
	);
      qllc_rc = qrc_ok;
      /* some implementations need a status report on state change */
      QLLC_REPORT_STATUS (
	qllc_rc,
	qsr_entered_closed_state,
	link_station->correlator
	);
      break;
      
    case qs_inop:
      QLLC_LOCAL_ERROR (Q_LERROR_QFRMR, link_station->correlator);
      qllc_rc = qrc_local_error;
      break;
    }
  }
  else /* if (link_station->role == secondary) */
  {
    qfrmr.control_field = (byte)qfrmr_rsp;
    qfrmr.address_field = (byte)response;
    qfrmr.info_field = NULL;
    qfrmr.already_sent = FALSE;
    qllc_rc = qllc_common_rsp_sec (
      link_station,
      virt_circuit,
      x25_port,
      &qfrmr
      );
  }
  if (qvm_rc != qvm_rc_ok)
    qllc_rc = qrc_x25_error;
  return (qllc_rc);
  
}

/******************************************************************************
** Q_XID_RESPONSE
** Function:    qllc_qxid_rsp
**
** Description: This function is called each time a QLLC link station receives
**              a QXID_Rsp packet. It performs a branch according to station
**              role, as most of the protocol machinery needed to handle the
**              event is contained within the functions qllc_common_rsp_sec()
**              and qllc_qxid_rsp_pri();
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_remote_error- a protocol error caused by the remote QLLC
**                                link station was detected.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_qxid_rsp (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port)
{
  enum qllc_rc_type qllc_rc;  /* the return code from this function */
  qllc_qllu_type qxid;        /* structure to build a qxid_rsp */
  
  if (link_station->role == qr_primary)
  {
    qllc_clear_pending_cmd (link_station);
    qllc_rc = qllc_qxid_rsp_pri (
      link_station,
      virt_circuit,
      x25_port
      );
  }
  else /* if (link_station->role = qr_secondary) */
  {
    qxid.control_field = (byte)qxid_rsp;
    qxid.address_field = (byte)response;
    qxid.info_field = NULL;
    qxid.already_sent = FALSE;
    qllc_rc = qllc_common_rsp_sec (
      link_station,
      virt_circuit,
      x25_port,
      &qxid
      );
  }
  return (qllc_rc);
}

/******************************************************************************
** Q_TEST_RESPONSE
** Function:    qllc_qtest_rsp
**
** Description: This function is called each time a QLLC link station receives
**              a QTEST_Rsp packet. It performs a branch according to station
**              role, as most of the protocol machinery needed to handle the
**              event is contained within the functions qllc_common_rsp_sec()
**              and qllc_qtest_rsp_pri();
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_local_error - a protocol error caused by the local QLLC
**                                link station was detected.
**              qrc_remote_error- a protocol error caused by the remote QLLC
**                                link station was detected.
**              qrc_x25_error   - an error was detected in an underlying X.25
**                                virtual circuit operation.
**              qrc_ok          - the operation completed withot error.
**
******************************************************************************/
enum qllc_rc_type qllc_qtest_rsp (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port)
{
  enum qllc_rc_type qllc_rc;  /* the return code from this function */
  qllc_qllu_type qtest;       /* structure to build a qtest_rsp */
  
  if (link_station->role == qr_primary)
  {
    qllc_clear_pending_cmd (link_station);
    qllc_rc = qllc_qtest_rsp_pri (
      link_station,
      virt_circuit,
      x25_port
      );
  }
  else /* if (link_station->role = qr_secondary) */
  {
    qtest.control_field = (byte)qtest_rsp;
    qtest.address_field = (byte)response;
    qtest.info_field = NULL;
    qtest.already_sent = FALSE;
    qllc_rc = qllc_common_rsp_sec (
      link_station,
      virt_circuit,
      x25_port,
      &qtest
      );
  }
  return (qllc_rc);
}
