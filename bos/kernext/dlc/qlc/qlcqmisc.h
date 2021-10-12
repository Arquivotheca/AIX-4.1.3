/* @(#)80  1.3.1.1  src/bos/kernext/dlc/qlc/qlcqmisc.h, sysxdlcq, bos411, 9435D411a 9/2/94 10:02:04 */
#ifndef _H_QLCQMISC
#define _H_QLCQMISC
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

/******************************************************************************
** MACROS EXPORTED FROM qlcqmisc.h
******************************************************************************/

/******************************************************************************
** RETURN "COMMAND POLLS" RAS COUNTER
** ras_counter_type QLLC_RETURN_COMMAND_POLLS_SENT (link_station)
**     qllc_ls_type                      *link_station;
******************************************************************************/
#define QLLC_RETURN_COMMAND_POLLS_SENT(ls) (ls)->command_polls_sent

/******************************************************************************
** RETURN "COMMAND REPOLLS" RAS COUNTER
** ras_counter_type QLLC_RETURN_COMMAND_REPOLLS_SENT (link_station)
**     qllc_ls_type                      *link_station;
******************************************************************************/
#define QLLC_RETURN_COMMAND_REPOLLS_SENT(ls) (ls)->command_repolls_sent

/******************************************************************************
** RETURN "COMMAND CONTIGUOUS REPOLLS" RAS COUNTER
** ras_counter_type QLLC_RETURN_COMMAND_CONTIGUOUS_REPOLLS (link_station)
**     qllc_ls_type                      *link_station;
******************************************************************************/
#define QLLC_RETURN_COMMAND_CONTIGUOUS_REPOLLS(ls) \
    (ls)->command_contiguous_repolls

/******************************************************************************
** QLLC LINK STATION IS PRIMARY ?
** bool QLLC_LINK_STATION_IS_PRIMARY (link_station)
**     qllc_ls_type                      *link_station;
******************************************************************************/
#define QLLC_LINK_STATION_IS_PRIMARY(ls) ((ls)->role == qr_primary)

/******************************************************************************
** QLLC LINK STATION IS CLOSED ?
** bool QLLC_LINK_STATION_IS_CLOSED (link_station)
**     qllc_ls_type                      *link_station;
******************************************************************************/
#define QLLC_LINK_STATION_IS_CLOSED(ls) ((ls)->state == qs_closed)

/******************************************************************************
** QLLC LINK STATION IS OPENED ?
** bool QLLC_LINK_STATION_IS_OPENED (link_station)
**     qllc_ls_type                      *link_station;
******************************************************************************/
#define QLLC_LINK_STATION_IS_OPENED(ls) ((ls)->state == qs_opened)

/******************************************************************************
** QLLC OUTGOING XID RESPONSE PENDING ?
** bool QLLC_XID_RESPONSE_PENDING (link_station)
**     qllc_ls_type                      *link_station;
******************************************************************************/
#define QLLC_XID_RESPONSE_PENDING(ls) ((ls)->predicate == qp_oxrp)

/******************************************************************************
** QLLC REPORT STATUS
** The QLLC finite state machine architecture instructs us that certain state
** changes in a QLLC link station should be reported to a higher layer. In fact
** in the AIX 2 environment, these status reports were not utilised.
** Under AIX 3 however, status reports are required whenever a link station
** goes into the qs_opened state (i.e. contacted).
** QLLC_REPORT_STATUS macro is called each time the architecture requires a
** status report.
**
** If status reports are turned off altogether, (i.e. neither of the constants
** QLLC_STATUS_REPORTS or QLLC_OPENED_STATUS_REPORTS are #define'd) then the
** macro will do nothing.
**
** If "opened" status reports are turned on, (i.e. the constant
** QLLC_OPENED_STATUS_REPORTS is #defined), then the status value will be ANDed** into the return code, iff it's a qsr_entered_opened_state status value.
**
** If status reports are turned on, (i.e. QLLC_STATUS_REPORTS is #defined),
** the status value will always be ANDed into the supplied return code.
**
** The definition of the status and return code types, should ensure that each
** is accessible separately, by bit mask operations.
******************************************************************************/

/******************************************************************************
** qllc_status_type enumerates the status reports which can be generated
** by QLLC finite state machine functions. These status reports can be
** suppressed by commenting out the lines defined below.
******************************************************************************/

/* Uncomment the appropriate line to enable desired status reports */
/* Following line enables all reports */
/* #define QLLC_STATUS_REPORTS */
/* Following line enables only "opened" reports */
#define QLLC_OPENED_STATUS_REPORTS
/* Keep both lines commented if you want NO reports */

enum qllc_status_type {
    qsr_entered_closed_state = 0x0100,
    qsr_entered_opening_state = 0x0200,
    qsr_entered_opened_state = 0x0300,
    qsr_entered_closing_state = 0x0400,
    qsr_entered_recovery_state = 0x0500,
    qsr_entered_inop_state = 0x0600,
    qsr_xid_cmd_received = 0x0700,
    qsr_test_cmd_received = 0x0800,
    qsr_rd_rsp_received = 0x0900
};

#if defined(QLLC_STATUS_REPORTS)
#define QLLC_REPORT_STATUS(rc,status,corr) \
    rc = rc & (enum qllc_rc_type)(status)

#elseif defined (QLLC_OPENED_STATUS_REPORTS)
#define QLLC_REPORT_STATUS(rc,status,corr) \
  if ((enum qllc_status_type)(status) == qsr_entered_opened_state) \
  (rc) = ((rc) & (enum qllc_rc_type)(status))
/* if ((status) == qsr_entered_opened_state) */

#else
#define QLLC_REPORT_STATUS(rc,status,corr)
#endif

/* Start of declarations for qlcqmisc.c                                      */
#ifdef _NO_PROTO
void qllc_init_station();
enum qllc_rc_type qllc_alter_role();
void qllc_alter_repoll_timeout();
void qllc_alter_max_repoll();
#else
void qllc_init_station (
      qllc_ls_type                      *link_station,
      enum qllc_role_type               role,
      repoll_counter_type               max_repoll,
      repoll_timer_type                 repoll_time,
      correlator_type                   correlator,
      ulong_t                           negotiable);   /* defect 52838 */

enum qllc_rc_type qllc_alter_role (
      qllc_ls_type                      *link_station,
      enum qllc_role_type               new_role);

extern void qllc_alter_repoll_timeout (
      qllc_ls_type                      *link_station,
      bool                              alter_repoll,
      repoll_timer_type                 new_repoll_time);

extern void qllc_alter_max_repoll (
      qllc_ls_type                      *link_station,
      unsigned int                       new_max_repoll);

#endif /* _NO_PROTO */
/* End of declarations for qlcqmisc.c                                        */


#endif
