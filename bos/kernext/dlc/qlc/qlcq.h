/* @(#)72  1.3.1.1  src/bos/kernext/dlc/qlc/qlcq.h, sysxdlcq, bos411, 9435D411a 9/2/94 10:02:57 */
#ifndef _H_QLCQ
#define _H_QLCQ
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

#include "qlcqerrs.h"

/******************************************************************************
** qllc_role_type enumerates the roles which a link station may assume
******************************************************************************/
enum qllc_role_type {qr_primary, qr_secondary};

/******************************************************************************
** qllc_role_type enumerates the states which a link station may be in
******************************************************************************/
enum qllc_state_type {
    qs_closed,
    qs_closing,
    qs_inop,
    qs_opened,
    qs_opening,
    qs_recovery
};

/******************************************************************************
** qllc_pred_type enumerates the valid predicate states
******************************************************************************/
enum qllc_pred_type {
    qp_ctp,
    qp_iotrp,
    qp_ioxrp,
    qp_itrp,
    qp_ixotrp,
    qp_itoxrp,
    qp_ixrp,
    qp_null,
    qp_otrp,
    qp_oxrp,
    qp_rrp,
    qp_smp
};

struct qllc_qllu_type
{
  byte address_field;
  byte control_field;
  byte *info_field;
  bool already_sent;
};
typedef struct qllc_qllu_type qllc_qllu_type;


/******************************************************************************
** We need to keep a record of any pending command. This is defined in SEDL as
** a set with maximum cardinality of one. qllc_qllu_element_type describes one
** element of such a set. A set is defined as an array of these elements. In
** this case, the cardinality is one, and so an array with one element is
** defined.
******************************************************************************/
typedef struct {
    bool                        used;
    struct qllc_qllu_type       qllu;
}   qllc_qllu_element_type;

/******************************************************************************
** Specify the storage classes of values defined in the design as Natural or
** Positive.
******************************************************************************/
typedef byte repoll_timer_type;
typedef byte repoll_counter_type;

/******************************************************************************
** QLLC l.s. manager has to have its own local definition of cause and diag
** code type, to remain independent of VRM.
******************************************************************************/
typedef byte q_cause_code_type;
typedef byte q_diag_code_type;

/******************************************************************************
** qllc_ls_type defines all the data recorded for each QLLC link station.
**
** The QLLC link station manager maintains three RAS counters:
**     command_polls_sent         - a count of the number of QLLC cmd packets
**                                  sent since the station was opened
**     command_repolls_sent       - a count of the number of times a QLLC cmd
**                                  packet has been to be re-sent because it
**                                  was not acknowledged.
**     command_contiguous_repolls - a count of the number of times the current
**                                  pending cmd packet has been repolled
******************************************************************************/
struct qllc_ls_type
{
  ulong_t                     negotiable;   /* defect 52838 */
  correlator_type             correlator;
  enum qllc_role_type         role;
  enum qllc_state_type        state;
  enum qllc_pred_type         predicate;
  ras_counter_type            command_polls_sent;
  ras_counter_type            command_repolls_sent;
  ras_counter_type            command_contiguous_repolls;
  struct watchdog             repoll_dog;
  repoll_timer_type           repoll_time;
  unsigned int                repoll_due;
  repoll_counter_type         max_repoll;
  repoll_counter_type         counter;
  qllc_qllu_element_type      pending_cmd [1];
};
typedef struct qllc_ls_type qllc_ls_type;

/******************************************************************************
** qllc_rc_type enumerates the return codes from qllc procedures
******************************************************************************/
enum qllc_rc_type {
    qrc_ok = 0,
    qrc_max_repolls_exceeded,
    qrc_local_error,
    qrc_remote_error,
    qrc_program_error,
    qrc_alter_role_failed,
    qrc_x25_error,
    qrc_discard_data,
    qrc_qrd_received_in_opening_state,
    qrc_qfrmr_received_in_opening_or_closed_state
};
typedef enum qllc_rc_type qllc_rc_type;


#endif
