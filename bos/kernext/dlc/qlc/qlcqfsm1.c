static char sccsid[] = "@(#)75  1.4.1.2  src/bos/kernext/dlc/qlc/qlcqfsm1.c, sysxdlcq, bos41J, 9507C 2/3/95 13:51:26";
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: qllc_repoll, qllc_cmd_poll, qllc_svc_cleared, qllc_lltox,
 *            qllc_lstrt, qllc_lstop, qllc_xchid, qllc_sdata, qllc_l3rdy,
 *            qllc_l3nop, qllc_rdata
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
** Description: This file contains code for QLLC finite state machine
**              procedures. The procedures included in this file are invoked
**              as a result of higher layer stimuli, such as a request from
**              SNA Services to exchange id information with the remote link
**              station, or notification that the underlying virtual circuit
**              has changed state. Procedures invoked as a result of received
**              Q packets are coded in qlcqfsm2.c
**
**              It's worthwhile noting some RAS aspects of the QLLC link
**              station manager code, at this point. Error handling is
**              done by a set of macros, dealing with PROGRAM errors, LOCAL
**              errors and REMOTE errors. LOCAL and REMOTE errors are protocol
**              problems, architected by the QLLC FSM. PROGRAM errors are
**              conditions detected by the code, which should never occur, eg
**              requests for a secondary link station to repoll a command.
**
**              The QLLC link station manager maintains three RAS counters:
**                  command_polls_sent         - a count of the number of
**                                               QLLC cmd packets sent
**                                               since the station was opened
**                  command_repolls_sent       - a count of the number of
**                                               times a QLLC cmd packet had
**                                               to be re-sent because it
**                                               was not acknowledged.
**                  command_contiguous_repolls - a count of the number of
**                                               times the current pending
**                                               cmd packet has been repolled
**
** Version:     2.0
**
**              Changes from version 1.0 include:
**            *  qllc_timer removed.
**            *  qllc_lltox no longer internally called. Now called by kernel,
**            *  using timeout expiry function call.
**            *  qvm_rc_type replaced by qvm_rc_type.
**            *  x25_port_type replaced by port_type.
**            *  Timers are now handled by QTM. Settings of timer counters are
**               therefore replaced by calls to qtm to start_timer,
**               cancel_timer,etc.
**
** Compiler:    <DOS>:  cc
**              <AIX>:  cc
** Options:     <DOS>:  cc /AS
**              <AIX>:  cc -DAIX
** Make File:
** Exported:    <functions defined for use within QLLC l.s. mgr>
**              enum qllc_rc_type qllc_repoll();
**              enum qllc_rc_type qllc_cmd_poll();
**              enum qllc_rc_type qllc_pvc_reset();
**              enum qllc_rc_type qllc_svc_cleared();
**
**              <functions defined for use outside QLLC l.s. mgr>
**              enum qllc_rc_type qllc_lltox();
**              enum qllc_rc_type qllc_lstrt();
**              enum qllc_rc_type qllc_lstop();
**              enum qllc_rc_type qllc_xchid();
**              enum qllc_rc_type qllc_ltest();
**              enum qllc_rc_type qllc_sdata();
**              enum qllc_rc_type qllc_l3rdy();
**              enum qllc_rc_type qllc_l3nop();
**              enum qllc_rc_type qllc_errpk();
**              enum qllc_rc_type qllc_clrst();
**              enum qllc_rc_type qllc_rdata();
**
** Imported:    <from qlcxproc.c>
**              enum qvm_rc_type qvm_send_qllu
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

/*****************************************************************************/
/* DEFINITIONS OF PROCS EXPORTED WITHIN, BUT NOT OUTSIDE OF QLLC L.S. MGR    */
/*****************************************************************************/

/******************************************************************************
** COMMAND RE-POLL
** Function:    qllc_repoll
**
** Description: Re-sends the last unacknowledged command packet sent by the
**              local primary station. Updates RAS counters and decrements the
**              running repoll counter
**              This should not be called if there is no unacknowledged
**              command, but if this is done, an error is logged.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_max_repolls_exceeded
**                              - a command packet was unacknowledged
**                                after the maximum number of repolls
**              qrc_local_error - the station was in an invalid state for the
**                                operation
**              qrc_x25_error   - an error occurred in an underlying X.25 proc
**              qrc_ok          - operation completed succesfully
**
******************************************************************************/
enum qllc_rc_type qllc_repoll (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port)
{
  enum qllc_rc_type qllc_rc;
  
  if (link_station->pending_cmd[0].used == FALSE) /* pending cmd set empty */
  {
    QLLC_PROGRAM_ERROR(Q_PERROR_REPOLL, link_station->correlator);
    qllc_rc = qrc_program_error;
  }
  else /* if pending cmd set is not empty */
  {
    if (link_station->counter > 0)
    {
      qllc_rc = qllc_cmd_poll (
	link_station,
	virt_circuit,
	x25_port,
	&(link_station->pending_cmd[0].qllu)
	);
    
      /* update the RAS counters */
      INC_RAS_COUNTER(link_station->command_repolls_sent);
      INC_RAS_COUNTER(link_station->command_contiguous_repolls);
      
      /* decrement the number of repolls remaining, unless it's already 0 */
      if (link_station->counter >= 1)
	link_station->counter--;
    }
    else
    {
      /***********************************************************************/
      /* max_repolls already sent                                            */
      /***********************************************************************/
      qllc_rc = qrc_max_repolls_exceeded;
    }
  }
  return (qllc_rc);
}

/******************************************************************************
** COMMAND POLL
** Function:    qllc_cmd_poll
**
** Description: This function performs all the necessary steps involved in
**              sending a Q command packet to the remote link station. The
**              X.25 Send QLLU operation is called to send the packet. The RAS
**              counters are updated. The link station pending command is set
**              to the command sent. The repoll timer is restarted to run the
**              full repoll time.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**              qllu            - the command packet to send
**
** Return Code: qrc_x25_error   - an error occurred in an underlying X.25 proc
**              qrc_ok          - operation completed succesfully
******************************************************************************/
enum qllc_rc_type qllc_cmd_poll (

  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port,
  qllc_qllu_type                    *qllu)
{
  qvm_rc_type   qvm_rc;
  qllc_rc_type  qllc_rc;
  
  qvm_rc = qvm_send_qllu (virt_circuit,x25_port,qllu);

  if (qvm_rc == qvm_rc_ok)
  {
    /*************************************************************************/
    /* Start the repoll timer for this link station.                         */
    /*************************************************************************/
    outputf("QLLC_CMD_POLL: repoll timer = %d\n",link_station->repoll_time);
    w_start(&(link_station->repoll_dog));

    /* update the RAS counter for polls sent */
    INC_RAS_COUNTER(link_station->command_polls_sent);
    link_station->pending_cmd[0].used = TRUE;
    link_station->pending_cmd[0].qllu = *qllu;
    qllc_rc = qrc_ok;
  }
  else
  {
    /*************************************************************************/
    /* No need to do anything with the timer.                                */
    /*************************************************************************/
    /* Defect 103642 - Remove the buffer after using it */
    if (link_station->pending_cmd[0].qllu.info_field)
    {
        QBM_FREE_BUFFER((struct mbuf *)link_station->pending_cmd[0].qllu.info_field);
        link_station->pending_cmd[0].qllu.info_field = NULL;
    }

    link_station->pending_cmd[0].used = FALSE;
    qllc_rc = qrc_x25_error;
  }
  return (qllc_rc);
}

/******************************************************************************
** PERMANENT VIRTUAL CIRCUIT RESET
** Function:    qllc_pvc_reset
**
** Description: This function is called each time a Reset Packet is received
**              on a PVC.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**              cause           - the cause code field from the reset packet
**              diagnostic      - the diagnostic code from the reset packet
**
** Return Code: qrc_x25_error   - an error occurred in an underlying X.25 proc
**              qrc_local_error - a QLLC local protocol error was detected
**              qrc_ok          - operation completed succesfully
******************************************************************************/
enum qllc_rc_type qllc_pvc_reset (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port,
  q_cause_code_type                 cause,
  q_diag_code_type                  diagnostic)
{
  enum qllc_rc_type qllc_rc;
  qllc_qllu_type qsm;
  
  
  switch (link_station->state)
  {
  case qs_closed:
    QLLC_DIAGNOSTIC (cause, diagnostic, link_station->correlator);
    /* reset the predicate if secondary - defect 171065 */
    if (link_station->role == qr_secondary)
      link_station->predicate = qp_null;
    qllc_rc = qrc_ok;
    break;
    
  case qs_opening:
    if (link_station->role == qr_primary)
    {
      qllc_rc = qrc_ok;
      /********  removed for defect 171065 ************
      if (link_station->predicate == qp_null)
      {
	QLLC_DIAGNOSTIC(cause,diagnostic,link_station->correlator);
	link_station->state = qs_closed;
	qllc_rc = qrc_ok;
      }
      else ** if (link_station->predicate != qp_null) **
      {
	link_station->predicate = qp_null;
	qsm.control_field = (byte)qsm_cmd;
	qsm.address_field = (byte)command;
	qsm.info_field = NULL;
	qsm.already_sent = FALSE;
	qllc_rc = qllc_cmd_poll (
	  link_station,
	  virt_circuit,
	  x25_port,
	  &qsm
	  );
	link_station->counter = link_station->max_repoll;
      }
      **************** end of delete *************/
    }
    else /* if (link_station->role == qr_secondary) */
    {
      QLLC_DIAGNOSTIC (cause, diagnostic, link_station->correlator);
      link_station->state = qs_closed;
      link_station->predicate = qp_null;
      qllc_rc = qrc_ok;
    }
    break;
    
  case qs_closing:
  case qs_recovery:
  case qs_opened:
    QLLC_DIAGNOSTIC (cause, diagnostic, link_station->correlator);
    link_station->state = qs_closed;
    link_station->predicate = qp_null;
    qllc_rc = qrc_ok;
    break;
    
  case qs_inop:
    QLLC_LOCAL_ERROR(Q_LERROR_PVC_RESET, link_station->correlator);
    qllc_rc = qrc_local_error;
    break;
  }
  return (qllc_rc);
}

/******************************************************************************
** SWITCHED VIRTUAL CIRCUIT CLEARED
** Function:    qllc_svc_cleared
**
** Description: This function is called each time a Clear Request Packet is
**              received on an SVC.
**
** Parameters:  link_station    - the link station instance
**              cause           - the cause code field from the reset packet
**              diagnostic      - the diagnostic code from the reset packet
**
** Return Code: qrc_ok          - operation completed succesfully
******************************************************************************/
enum qllc_rc_type qllc_svc_cleared (
  qllc_ls_type                      *link_station,
  q_cause_code_type                 cause,
  q_diag_code_type                  diagnostic)
{
  QLLC_DIAGNOSTIC (cause, diagnostic, link_station->correlator);
  link_station->state = qs_inop;
  return qrc_ok;
}


/******************************************************************************
*******************************************************************************
** DEFINITIONS OF PROCEDURES EXPORTED OUTSIDE OF QLLC LINK STATION MANAGER
*******************************************************************************
******************************************************************************/


/******************************************************************************
** LINK TIMEOUT EXPIRY
** Function:    qllc_lltox
**
** Description: Called each time a primary repoll timeout occurs. If the
**              station role is primary, and the primary repoll counter has not
**              expired, the pending command is repolled, the repoll counter
**              is decremented, the repoll timer is reset, and the return code
**              is set to "ok".
**              If the repoll counter has expired, the return code is set to
**              "max repolls exceeded", but polling continues.
**              This function should never be called for a secondary station,
**              or one which is in INOP state, but if this happens, an error
**              is logged.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              port            - the X.25 port instance
**
** Return Code: qrc_max_repolls_exceeded
**                              - a command packet was unacknowledged
**                                after the maximum number of repolls
**              qrc_local_error - the station role or state was invalid for the
**                                operation
**              qrc_x25_error   - an error occured in an underlying X.25 proc
**              qrc_ok          - operation completed succesfully
**
******************************************************************************/
enum qllc_rc_type qllc_lltox (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port)
{
  enum qllc_rc_type qllc_rc;
  
  if (link_station->role != qr_primary)        /* this should never happen */
  {
    QLLC_PROGRAM_ERROR(Q_PERROR_LLTOX, link_station->correlator);
    qllc_rc = qrc_program_error;
  }
  else if (link_station->state == qs_inop)
  {
    QLLC_LOCAL_ERROR(Q_LERROR_LLTOX, link_station->correlator);
    qllc_rc = qrc_local_error;
  }
  else /* if ((role == qr_primary) && (state != qs_inop)) */
  {
    qllc_rc = qllc_repoll (link_station, virt_circuit, x25_port);
    if ((qllc_rc == qrc_ok) && (link_station->counter == 0))
      qllc_rc = qrc_max_repolls_exceeded;
  }
  return (qllc_rc);
}

/******************************************************************************
** LINK START
** Function:    qllc_lstrt
**
** Description: This function is called to initiate the logical link control
**              set-up procedure for the link connection. It corresponds to a
**              Contact command from SNA Services to the Gladstone QLLC
**              component.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_x25_error   - an error occurred in an underlying X.25 proc
**              qrc_local_error - a QLLC local protocol error was detected
**              qrc_ok          - operation completed succesfully
******************************************************************************/
enum qllc_rc_type qllc_lstrt (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port)
{
  enum qllc_rc_type qllc_rc;
  qllc_qllu_type qsm;
  
  switch (link_station->state)
  {
  case qs_closed:
    if (link_station->role == qr_primary)
    {
      link_station->negotiable = 0;         /* defect 52838 */
      qsm.control_field = (byte)qsm_cmd;
      qsm.address_field = (byte)command;
      qsm.info_field = NULL;
      qsm.already_sent = FALSE;
      qllc_rc = qllc_cmd_poll (
	link_station,
	virt_circuit,
	x25_port,
	&qsm
	);
      link_station->counter = link_station->max_repoll;
      link_station->state = qs_opening;
    }
    else /* if (link_station->role == qr_secondary) */
    {
      link_station->state = qs_opening;
      qllc_rc = qrc_ok;
    }
    break;
    
  case qs_recovery:
    if (link_station->role == qr_primary)
    {
      QLLC_PROGRAM_ERROR(Q_PERROR_LSTRT, link_station->correlator);
      qllc_rc = qrc_program_error;
    }
    else /* if (link_station->role == qr_secondary) */
    {
      if (link_station->state == qs_recovery)
	link_station->predicate = qp_null;
      link_station->state = qs_opening;
      qllc_rc = qrc_ok;
    }
    break;
    
  case qs_inop:
  case qs_closing:
  case qs_opening:
  case qs_opened:
    QLLC_LOCAL_ERROR(Q_LERROR_LSTRT, link_station->correlator);
    qllc_rc = qrc_local_error;
    break;
  }
  return (qllc_rc);
}

/******************************************************************************
** LINK STOP
** Function:    qllc_lstop
**
** Description: This function is called to initiate the logical link control
**              disconnection procedure for the link connection. It will be
**              invoked as a result of a Close Link Station command from SNA
**              Services.
**
**              Unlike most of the finite state machine functions, a switch
**              statement is not used to separate the partitions described
**              in the QLLC design. This is because the predicates being
**              tested are unusually complicated, and especially because the
**              final "TRUE" predicate embodies many different state and
**              predicate combinations.
**
**              Therefore a series of "if ... else if ..."  tests are
**              used to separate the partitions. Since the link station
**              state, role and predicate are repetedly tested, they are
**              declared as register variables. This just might give a
**              performance improvement.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**
** Return Code: qrc_x25_error   - an error occurred in an underlying X.25 proc
**              qrc_local_error - a QLLC local protocol error was detected
**              qrc_ok     - operation completed succesfully
******************************************************************************/
enum qllc_rc_type qllc_lstop (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port)
{
  enum qvm_rc_type qvm_rc;
  enum qllc_rc_type qllc_rc;
  qllc_qllu_type qrd;
  qllc_qllu_type qdisc;

  register enum qllc_state_type state = link_station->state;
  register enum qllc_role_type role = link_station->role;
  register enum qllc_pred_type predicate = link_station->predicate;
  
  if ((state == qs_opening) && (role == qr_secondary))
  {
    link_station->state = qs_closed;
    link_station->predicate = qp_null;
    qllc_rc = qrc_ok;
    /* some implementations may need to report state change */
    QLLC_REPORT_STATUS(
      qllc_rc,
      qsr_entered_closed_state,
      link_station->correlator
      );
  }
  else if (state == qs_recovery)
  {
    if (role == qr_primary) /* primary should never be in recovery state */
    {
      QLLC_PROGRAM_ERROR(Q_PERROR_LSTOP, link_station->correlator);
      qllc_rc = qrc_program_error;
    }
    else /* if (role == qr_secondary) */
    {
      link_station->state = qs_closing;
      qrd.control_field = (byte)qrd_rsp;
      qrd.address_field = (byte)response;
      qrd.info_field = NULL;
      qrd.already_sent = FALSE;
      qvm_rc = qvm_send_qllu (
	virt_circuit,
	x25_port,
	&qrd
	);
      qllc_rc = (qvm_rc == qvm_rc_ok) ? qrc_ok : qrc_x25_error;
    }
  }
  /*************************************************************************/
  /* Defect 103642 - If the primary station is opened and its FSM is in    */
  /* closed state, allow it to do an immediate halt.			   */
  /*************************************************************************/
  else if ((state == qs_closed) && (role == qr_primary))
  {
      qllc_rc = qvm_close_vc(
        virt_circuit,
        x25_port,
        NORMAL_TERMINATION,
        FALSE
        );
  }
  /*************************************************************************/
  /* End of defect 103642.						   */
  /*************************************************************************/
  else if (state == qs_opened)
  {
    if (role == qr_primary)
    {
      if ((predicate != qp_ixrp) && (predicate != qp_itrp))
      {
	link_station->state = qs_closing;
	link_station->counter = link_station->max_repoll;
	qdisc.control_field = (byte)qdisc_cmd;
	qdisc.address_field = (byte)command;
	qdisc.info_field = NULL;
	qdisc.already_sent = FALSE;
	qllc_rc = qllc_cmd_poll (
	  link_station,
	  virt_circuit,
	  x25_port,
	  &qdisc
	  );
	/* some implementations may need to report state change */
	QLLC_REPORT_STATUS(
	  qllc_rc,
	  qsr_entered_closing_state,
	  link_station->correlator
	  );
      }
      else /* if ((predicate == qp_ixrp) || (predicate == qp_itrp)) */
      {
	QLLC_LOCAL_ERROR(Q_LERROR_LSTOP_1, link_station->correlator);
	qllc_rc = qrc_local_error;
      }
    }
    else /* if (role == qr_secondary) */
    {
      link_station->state = qs_closing;
      qrd.control_field = (byte)qrd_rsp;
      qrd.address_field = (byte)response;
      qrd.info_field = NULL;
      qrd.already_sent = FALSE;
      qvm_rc = qvm_send_qllu (
	virt_circuit,
	x25_port,
	&qrd
	);
      qllc_rc = (qvm_rc == qvm_rc_ok) ? qrc_ok : qrc_x25_error;
      /* some implementations may need to report state change */
      QLLC_REPORT_STATUS(
	qllc_rc,
	qsr_entered_closing_state,
	link_station->correlator
	);
    }
  }
  else /* none of the above cases */
  {
    QLLC_LOCAL_ERROR(Q_LERROR_LSTOP_2, link_station->correlator);
    qllc_rc = qrc_local_error;
  }
  return (qllc_rc);
}

/******************************************************************************
** EXCHANGE IDENTIFICATION
** Function:    qllc_xchid
**
** Description: This function is called to request transmission of logical link
**              identification information. This will occur in response to a
**              Write XID Data command from SNA Services.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**              xid_info        - a pointer to the XID information.
**                                In the AIX3 environment, this is really a
**                                pointer to a block I/O buffer, but the QLLC
**                                link station manager need not know this. It
**                                is defined as a char * to enable easier
**                                porting to other environments.
**
** Return Code: qrc_x25_error   - an error occurred in an underlying X.25 proc
**              qrc_local_error - a QLLC local protocol error was detected
**              qrc_ok          - operation completed succesfully
******************************************************************************/
enum qllc_rc_type qllc_xchid (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port,
  byte                              *xid_info)
{
  enum qvm_rc_type qvm_rc;
  enum qllc_rc_type qllc_rc;
  qllc_qllu_type qxid;

  /* Again, register variables are declared to hold role, state and        */
  /* predicate, since there are multiple comparisons against these values, */
  /* and a performance improvement might result                            */
  register enum qllc_state_type state = link_station->state;
  register enum qllc_role_type role = link_station->role;
  register enum qllc_pred_type predicate = link_station->predicate;
  
  if (role == qr_primary)
  {
    if (((state == qs_opened) || (state == qs_closed)) &&
      (predicate != qp_ixrp) && (predicate != qp_itrp))
    {
      link_station->predicate = qp_ixrp;
      link_station->counter = link_station->max_repoll;
      qxid.control_field = (byte)qxid_cmd;
      qxid.address_field = (byte)command;
      qxid.info_field = xid_info;
      qxid.already_sent = FALSE;
      qllc_rc = qllc_cmd_poll (
	link_station,
	virt_circuit,
	x25_port,
	&qxid
	);
    }
    else
    {
      QLLC_LOCAL_ERROR(Q_LERROR_XCHID_1, link_station->correlator);
      qllc_rc = qrc_local_error;
    }
  }
  else /* if (role == qr_secondary) */
  {
    if (((state == qs_opened) || (state == qs_opening) ||
      (state == qs_closed) || (state == qs_closing)) &&
      (predicate == qp_oxrp))
    {
      link_station->predicate = qp_null;
      qxid.control_field = (byte)qxid_rsp;
      qxid.address_field = (byte)response;
      qxid.info_field = xid_info;
      qxid.already_sent = FALSE;
      qvm_rc = qvm_send_qllu (
	virt_circuit,
	x25_port,
	&qxid
	);
      qllc_rc = (qvm_rc == qvm_rc_ok) ? qrc_ok : qrc_x25_error;
    }
    else
    {
      QLLC_LOCAL_ERROR(Q_LERROR_XCHID_2, link_station->correlator);
      qllc_rc = qrc_local_error;
    }
  }
  return (qllc_rc);
}

/******************************************************************************
** LINK TEST
** Function:    qllc_ltest
**
** Description: This function is called to request transmission of link test
**              data. For a primary link station, this corresponds to a Test
**              command from SNA Services. For a secondary link station, this
**              is used to echo test data, without involvement from SNA
**              Services.
**
**              Note that, according to the QLLC architecture, test commands
**              may be sent by primary stations in OPENED state. The AIX GDLCi
**              specification requires that an error be generated if a test
**              command is issued for a link station in contacted state, but
**              this is policed by the QLLC Link Station Manager, not at this
**              level.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**              test_info       - a pointer to the TEST information.
**                                In the AIX 3 environment, this is really a
**                                pointer to a gen_buffer_type, but the QLLC
**                                link station manager need not know this. It
**                                is defined as a char * to allow easier 
**                                porting to other environments.
**
** Return Code: qrc_x25_error   - an error occurred in an underlying X.25 proc
**              qrc_local_error - a QLLC local protocol error was detected
**              qrc_ok          - operation completed succesfully
******************************************************************************/
enum qllc_rc_type qllc_ltest (
      qllc_ls_type                      *link_station,
      x25_vc_type                       *virt_circuit,
      port_type                         *x25_port,
      byte                              *test_info)
{
  enum qvm_rc_type  qvm_rc;    /* used to hold return code from an X.25 fn */
  enum qllc_rc_type qllc_rc;   /* return code from this function           */
  qllc_qllu_type    qtest;     /* holds the Qtest_Cmd which may be sent    */
  /*************************************************************************/
  /* This function has unusually complex logic to separate the partitions  */
  /* described in the design. Register variables are used                  */
  /* hold the role, state and predicate of the link station, just in case  */
  /* a performance advantage may result.                                   */
  /*************************************************************************/
  register enum qllc_state_type state = link_station->state;
  register enum qllc_role_type role = link_station->role;
  register enum qllc_pred_type predicate = link_station->predicate;
  /*************************************************************************/
  /* Initialise X.25 return code. This will be used at end of function to  */
  /* see if X.25 errors were detected.                                     */
  /*************************************************************************/
  qvm_rc = qvm_rc_ok;
  
  outputf("QLLC_LTEST: station role = %d\n",role);
  outputf("QLLC_LTEST: station_state = %d\n",state);
  outputf("QLLC_LTEST: station_predicate = %d\n",predicate);

  switch (role)
  {
  case qr_primary:
    outputf("QLLC_LTEST: station found to be primary\n");
    switch (state)
    {
    case qs_opened:
    case qs_closed:
      if ((predicate == qp_ixrp) || (predicate == qp_itrp))
      {
	qllc_rc = qrc_local_error;
	QLLC_LOCAL_ERROR(
	  Q_LERROR_LTEST_PRI,
	  link_station->correlator
	  );
      }
      else
      {
	link_station->predicate = qp_itrp;
	link_station->counter = link_station->max_repoll;
	outputf("QLLC_LTEST: setting qllu control_field to qtest_cmd\n");
	qtest.control_field = (byte)qtest_cmd;
	outputf("QLLC_LTEST: setting qllu address_field to command\n");
	qtest.address_field = (byte)command;
	qtest.info_field = test_info;
	qtest.already_sent = FALSE;
	qllc_rc = qllc_cmd_poll (
	  link_station,
	  virt_circuit,
	  x25_port,
	  &qtest
	  );
      }
      break;
      
    case qs_opening:
    case qs_closing:
    case qs_inop:
      qllc_rc = qrc_local_error;
      QLLC_LOCAL_ERROR(
	Q_LERROR_LTEST_PRI,
	link_station->correlator
	);
      break;
      
    case qs_recovery:
      qllc_rc = qrc_program_error;
      QLLC_PROGRAM_ERROR(
	Q_PERROR_LTEST,
	link_station->correlator
	);
      break;
    }
    break;
    
  case qr_secondary:
    outputf("QLLC_LTEST: station found to be secondary\n");
    switch (state)
    {
    case qs_opened:
    case qs_closed:
    case qs_closing:
      if (predicate == qp_otrp)
      {
	link_station->predicate = qp_null;
	qtest.control_field = (byte)qtest_rsp;
	qtest.address_field = (byte)response;
	qtest.info_field = test_info;
	qtest.already_sent = FALSE;
	qvm_rc = qvm_send_qllu (
	  virt_circuit,
	  x25_port,
	  &qtest
	  );
	qllc_rc = qrc_ok;
      }
      else /* if (predicate != qp_otrp) */
      {
	qllc_rc = qrc_local_error;
	QLLC_LOCAL_ERROR(
	  Q_LERROR_LTEST_SEC,
	  link_station->correlator
	  );
	}
      break;
      
    case qs_opening:
      if ((predicate == qp_otrp) ||
	(predicate ==qp_iotrp) ||
	(predicate == qp_ixotrp))
      {
	link_station->predicate = qp_null;
	qtest.control_field = (byte)qtest_rsp;
	qtest.address_field = (byte)response;
	qtest.info_field = test_info;
	qtest.already_sent = FALSE;
	qvm_rc = qvm_send_qllu (
	  virt_circuit,
	  x25_port,
	  &qtest
	  );
	qllc_rc = qrc_ok;
      }
      else /* if (predicate != qp_otrp) */
      {
	qllc_rc = qrc_local_error;
	QLLC_LOCAL_ERROR(
	  Q_LERROR_LTEST_SEC,
	  link_station->correlator
	  );
      }
      break;
      
    case qs_inop:
    case qs_recovery:
    {
      qllc_rc = qrc_local_error;
      QLLC_LOCAL_ERROR(
	Q_LERROR_LTEST_SEC,
	link_station->correlator
	);
    }
      break;
    }
    break;
  }
  if (qvm_rc != qvm_rc_ok)
      qllc_rc = qrc_x25_error;
  return (qllc_rc);
}

/******************************************************************************
** SEND DATA
** Function:    qllc_sdata
**
** Description: This function is called to request transmission of normal
**              sequenced data to the link station in the adjacent node. It
**              corresponds to a Write Normal Data command from SNA Services.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**              data            - a pointer to the data.
**                                In the AIX3 environment, this is really a
**                                pointer to a block I/O buffer, but the QLLC
**                                link station manager need not know this. It
**                                is defined as a char * to enable easier
**                                porting to other environments.
**
** Return Code: qrc_x25_error   - an error occurred in an underlying X.25 proc
**              qrc_local_error - a QLLC local protocol error was detected
**              qrc_ok          - operation completed succesfully
******************************************************************************/
enum qllc_rc_type qllc_sdata (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port,
  byte                              *data)
{
  enum qvm_rc_type qvm_rc;    /* used to hold return code from an X.25 fn */
  enum qllc_rc_type qllc_rc;  /* return code from this function */

  qvm_rc = qvm_rc_ok;  

  if (link_station->state == qs_opened)
  {
    qvm_rc = qvm_send_dllu (
      virt_circuit,
      x25_port,
      data
      );
    qllc_rc = qrc_ok;
  }
  else /* if (link_station->state != qs_opened) */
  {
    qllc_rc = qrc_local_error;
    QLLC_LOCAL_ERROR(Q_LERROR_SDATA, link_station->correlator);
  }
  if (qvm_rc != qvm_rc_ok)
    qllc_rc = qrc_x25_error;
  return (qllc_rc);
}

/******************************************************************************
** PACKET LEVEL READY
** Function:    qllc_l3rdy
**
** Description: This function is called to notify the QLLC link station that
**              the underlying virtual circuit is in the ready state. This
**              corresponds to a succesful acknowledgment of an Open Logical
**              Command to the X25 Device Handler.
**
** Parameters:  link_station    - the link station instance
**
** Return Code: qrc_local_error - a QLLC local protocol error was detected
**              qrc_ok          - operation completed succesfully
******************************************************************************/
enum qllc_rc_type qllc_l3rdy (
  qllc_ls_type                      *link_station)
{
  enum qllc_rc_type qllc_rc;       /* holds return code from this function */
  
  switch (link_station->state)
  {
  case qs_inop:
    link_station->state = qs_closed;
    link_station->predicate = qp_null;
    qllc_rc = qrc_ok;
    break;
    
  case qs_opening:
  case qs_opened:
  case qs_closing:
  case qs_recovery:
    link_station->state = qs_closed;
    /* NULL is assumed predicate in OPENED, CLOSING, RECOVERY        */
    /* it's not mentioned in the architecture                        */
    link_station->predicate = qp_null;
    qllc_rc = qrc_ok;
    /* some implementations need a status report */
    QLLC_REPORT_STATUS(
      qllc_rc,
      qsr_entered_closed_state,
      link_station->correlator
      );
    break;
    
  case qs_closed:
    qllc_rc = qrc_local_error;
    QLLC_LOCAL_ERROR(Q_LERROR_L3RDY, link_station->correlator);
    break;
  }
  return (qllc_rc);
}

/******************************************************************************
** PACKET LEVEL INOPERATIVE
** Function:    qllc_l3nop
**
** Description: This function is called to notify the QLLC link station that
**              underlying virtual circuit is inoperative. This corresponds to
**              acknowledgment of a Close Logical Session command to the X25
**              Device Handler.
**
** Parameters:  link_station    - the link station instance
**
** Return Code: qrc_local_error - a QLLC local protocol error was detected
**              qrc_ok          - operation completed succesfully
******************************************************************************/
enum qllc_rc_type qllc_l3nop (
  qllc_ls_type                      *link_station)
{
  enum qllc_rc_type qllc_rc;       /* holds return code from this function */
  
  if (link_station-> state == qs_inop)
  {
    qllc_rc = qrc_local_error;
    QLLC_LOCAL_ERROR(Q_LERROR_L3NOP, link_station->correlator);
  }
  else /* if (link_station->state != qs_inop) */
  {
    link_station->state = qs_inop;
    w_stop(&(link_station->repoll_dog));
    /* NULL predicate is assumed, it's not mentioned in architecture */
    link_station->predicate = qp_null;
    qllc_rc = qrc_ok;
    /* some implementations need a status report */
    QLLC_REPORT_STATUS(
      qllc_rc,
      qsr_entered_inop_state,
      link_station->correlator
      );
  }
  return (qllc_rc);
}

/******************************************************************************
** ERRONEOUS QLLU
** Function:    qllc_errpk
**
** Description: This function is called to notify the QLLC link station that
**              an erroneous qualified logical link unit has been
**              received, for example a Q packet containing an unidentifiable
**              command or response.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**              address_byte    - the address field from the invalid packet
**              control_byte    - the control field from the invalid packet
**
** Return Code: qrc_x25_error   - an error occurred in an underlying X.25 proc
**              qrc_remote_error- a QLLC remote protocol error was detected
**              qrc_local_error - a QLLC local protocol error was detected
**              qrc_ok          - operation completed succesfully
******************************************************************************/
enum qllc_rc_type qllc_errpk (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port,
  byte                              address_byte,
  byte                              control_byte)
{
  enum qvm_rc_type qvm_rc;    /* used to hold return code from an X.25 fn */
  enum qllc_rc_type qllc_rc;  /* return code from this function           */
  
  qvm_rc = qvm_rc_ok;

  /***************************************************************************/
  /* Regardless of whether the station is primary or secondary, Alert 8 will */
  /* be generated.                                                           */
  /***************************************************************************/
  qlcerrlog(
    ERRID_QLLC_ALERT_8,
    "qllc",
    0,  /* sorry, but the FSM doesn't know what a SNA station is */
    NULL,
    NULL
    );
  
  if (link_station->role == qr_primary)
  {
    switch (link_station->state)
    {
    case qs_closing:
    case qs_opened:
      link_station->state = qs_closed;
      qvm_rc = qvm_clrst (
	virt_circuit,
	x25_port,
	UNDEFINED_C_FIELD
	);
      qllc_rc = qrc_ok;
      /* some implementations require a status report */
      QLLC_REPORT_STATUS(
	qllc_rc,
	qsr_entered_closed_state,
	link_station->correlator
	);
      break;
      
    case qs_opening:
      link_station->state = qs_closed;
      link_station->predicate = qp_null;
      qllc_rc = qrc_ok;
      /* some implementations require a status report */
      QLLC_REPORT_STATUS(
	qllc_rc,
	qsr_entered_closed_state,
	link_station->correlator
	);
      break;
      
    case qs_closed:
      qllc_rc = qrc_remote_error;
      QLLC_REMOTE_ERROR(Q_RERROR_ERRPK, link_station->correlator);
      break;
      
    case qs_recovery:
      qllc_rc = qrc_program_error;
      QLLC_PROGRAM_ERROR(Q_PERROR_ERRPK, link_station->correlator);
      break;
      
    case qs_inop:
      qllc_rc = qrc_local_error;
      QLLC_LOCAL_ERROR(Q_LERROR_ERRPK, link_station->correlator);
      break;
    }
  }
  else /* if (link_station->role = qr_secondary) */
  {
    switch (link_station->state)
    {
    case qs_closing:
    case qs_closed:
    case qs_opening:
    case qs_opened:
      link_station->state = qs_recovery;
      qvm_rc = qvm_send_qfrmr (
	virt_circuit,
	x25_port,
	control_byte,
	((address_byte == (byte)command) ?
	  qfr_erroneous_qllu_command :
	  qfr_erroneous_qllu_response)
	);
      qllc_rc = qrc_ok;
      /* some implementations need a status report */
      QLLC_REPORT_STATUS(
	qllc_rc,
	qsr_entered_recovery_state,
	link_station->correlator
	);
      break;
      
    case qs_recovery:
      qvm_rc = qvm_send_qfrmr (
	virt_circuit,
	x25_port,
	control_byte,
	((address_byte == (byte)command) ?
	  qfr_erroneous_qllu_command :
	  qfr_erroneous_qllu_response)
	);
      qllc_rc = qrc_remote_error;
      QLLC_REMOTE_ERROR(Q_RERROR_ERRPK, link_station->correlator);
      break;
      
    case qs_inop:
      qllc_rc = qrc_local_error;
      QLLC_LOCAL_ERROR(Q_LERROR_ERRPK, link_station->correlator);
      break;
    }
  }
  if (qvm_rc != qvm_rc_ok)
    qllc_rc = qrc_x25_error;
  return (qllc_rc);
}

/******************************************************************************
** VIRTUAL CIRCUIT CLEARED/RESET
** Function:    qllc_clrst
**
** Description: This function is called to notify the QLLC link station that
**              the underlying switched virtual circuit has been cleared, or
**              that the underlying permenent virtual circuit has been reset.
**
** Parameters:  link_station    - the link station instance
**              virt_circuit    - the corresponding virtual circuit instance
**              x25_port        - the X.25 port instance
**              pvc             - a flag indicating whether the underlying
**                                virtual circuit is a PVC or an SVC 
**              cause           - the cause code field from the clear/reset pkt
**              diagnostic      - the diag code field from the clear/reset pkt
**
** Return Code: qrc_x25_error   - an error occurred in an underlying X.25 proc
**              qrc_local_error - a QLLC local protocol error was detected
**              qrc_ok          - operation completed succesfully
******************************************************************************/
enum qllc_rc_type qllc_clrst (
  qllc_ls_type                      *link_station,
  x25_vc_type                       *virt_circuit,
  port_type                         *x25_port,
  bool                              pvc_flag,
  q_cause_code_type                 cause,
  q_diag_code_type                  diagnostic)
{
  enum qllc_rc_type qllc_rc;   /* holds the return code from this function */
  
  if (pvc_flag == TRUE)
  {
    qllc_rc = qllc_pvc_reset (
      link_station,
      virt_circuit,
      x25_port,
      cause,
      diagnostic
      );
  }
  else /* if (pvc_flag == FALSE) - i.e. this is an SVC */
  {
    qllc_rc = qllc_svc_cleared (
      link_station,
      cause,
      diagnostic
      );
  }
  return (qllc_rc);
}

/******************************************************************************
** RECEIVE DATA
** Function:    qllc_rdata
**
** Description: This function is called to inform the QLLC link station that
**              unqualified data has been received on the underlying virtual
**              circuit.
**
** Parameters:  link_station    - the link station instance
**
** Return Code: qrc_local_error - a QLLC local protocol error was detected
**              qrc_remote_error- a QLLC remote protocol error
**              qrc_discard_data- a protocol error occurred which does not fall
**                                into either of the above categories, but
**                                indicates that the received data should be
**                                discarded.
**              qrc_ok          - operation completed succesfully
******************************************************************************/
enum qllc_rc_type qllc_rdata (
  qllc_ls_type                      *link_station)
{
  enum qllc_rc_type qllc_rc;   /* holds the return code from this function */
  
  switch (link_station->state)
  {
  case qs_opened:
  case qs_closing:
    /* everything is fine, pass data to higher layer */
    qllc_rc = qrc_ok;
    break;
    
  case qs_closed:
    /* protocol error, therefore discard data */
    qllc_rc = qrc_remote_error;
    QLLC_REMOTE_ERROR(Q_RERROR_RDATA, link_station->correlator);
    break;
    
  case qs_opening:
    if (link_station->role == qr_primary)
    {
      /* protocol error, therefore discard data */
      qllc_rc = qrc_remote_error;
      QLLC_REMOTE_ERROR(Q_RERROR_RDATA, link_station->correlator);
    }
    else /* if (link_station->role == qr_secondary) */
    {
      if (link_station->predicate == qp_ctp)
      {
	/* for a secondary in OPENING/CTp state, receipt of data */
	/* completes transition into OPENED state. Data is       */
	/* accepted.                                             */
	link_station->state = qs_opened;
	link_station->predicate = qp_null;
	qllc_rc = qrc_ok;
	/* some implementations need a status report */
	qlm_ls_contacted(link_station->correlator);
	QLLC_REPORT_STATUS(
	  qllc_rc,
	  qsr_entered_opened_state,
	  link_station->correlator
	  );
      }
      else /* if (link_station->predicate != qp_ctp) */
      {
	/* invalid state for data receipt, therefore discard data*/
	qllc_rc = qrc_discard_data;
      }
    }
    break;
    
  case qs_recovery:
    /* invalid state for data receipt, therefore discard data*/
    qllc_rc = qrc_discard_data;
    break;
    
  case qs_inop:
    /* local protocol error, therefore discard data */
    qllc_rc = qrc_local_error;
    QLLC_LOCAL_ERROR(Q_LERROR_RDATA, link_station->correlator);
    break;
  }
  return (qllc_rc);
}


