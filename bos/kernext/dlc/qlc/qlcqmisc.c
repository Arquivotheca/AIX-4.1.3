static char sccsid[] = "@(#)79  1.7  src/bos/kernext/dlc/qlc/qlcqmisc.c, sysxdlcq, bos411, 9435D411a 9/2/94 10:02:15";
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: qllc_alter_role, qllc_alter_repoll_timeout
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

/**************************************************************************
** Description: This file contains code for the miscellaneous QLLC procedures
**              which are not defined by the QLLC finite state machine
**              architecture.
**
** Exported:    <functions defined>
**              void qllc_init_station();
**              enum qllc_rc_type qllc_timer();
**              enum qllc_rc_type qllc_alter_role();
**              void qllc_alter_repoll_timeout();
**              enum qllc_rc_type qllc_alter_role();
**
** Imported:    <functions>
**              enum qllc_rc_type qllc_lltox();
**
*****************************************************************************/
#include "qlcg.h"
#include "qlcv.h"
#include "qlcvfac.h"
#include "qlcq.h"
#include "qlcb.h"
#include "qlcp.h"
#include "qlcqfsm1.h"
#include "qlcqfsm2.h"
#include "qlcqmisc.h"
#include "qlcqerrs.h"

/******************************************************************************
** INITIALISE A LINK STATION INSTANCE:
** Function:    qllc_init_station
**
** Description: Takes an uninitialised qllc link station instance, and
**              initialises the fields within the structure. No global
**              variables are modified.
**
** Parameters:  link_station    - the unintialised link station instance
**              role            - the station role, primary or secondary
**              max_repoll      - number of times an unacknowledged command
**                                will be re-transmitted
**              repoll_time     - time (**              correlator      - a copy of the QLLC link station correlator,
**                                which will be needed for error logging.
**
******************************************************************************/
void qllc_init_station (
  qllc_ls_type                      *link_station,
  enum qllc_role_type               role,
  repoll_counter_type               max_repoll,
  repoll_timer_type                 repoll_time,
  correlator_type                   correlator,
  ulong_t                           ls_negotiable)    /* defect 52838 */
{
  outputf("QLLC_INIT_STATION: has been called\n");
  outputf(" ARGS PASSED IN TO FSM ARE: \n");
  outputf("  link_station ptr = %d\n",link_station);
  outputf("  role             = %d\n",role);
  outputf("  repoll_counter   = %d\n",max_repoll);
  outputf("  repoll_timer     = %d\n",repoll_time);
  outputf("  correlator       = %d\n",correlator);
/* defect 52838 */
  outputf("  ls_negotiable    = %d\n",ls_negotiable);

  link_station->negotiable = ls_negotiable;
  if (link_station->negotiable)
     role = qr_primary;
/* end defect 52838 */
  link_station->correlator = correlator;
  link_station->role = role;
  link_station->state = qs_inop;
  link_station->predicate = qp_null;
  link_station->command_polls_sent = 0;
  link_station->command_repolls_sent = 0;
  link_station->command_contiguous_repolls = 0;
  link_station->repoll_time = repoll_time;
  link_station->max_repoll = max_repoll;
  link_station->counter = 0;
  link_station->pending_cmd[0].used = FALSE;

  outputf("QLLC_INIT_STATION: returning\n");
}


/******************************************************************************
** ALTER STATION ROLE
** Function:    qllc_alter_role
**
** Description: This function attempts to change the link station role
**              
**              Services. The station role may only be changed if the station
**              is not already contacted into normal response mode, and no
**              Q_XID, Q_Test or Q_Set_Mode is in progress.
**
** Parameters:  link_station    - a link station instance
**              new_role        - the new role for the station
**
** Return Code: qrc_alter_role_failed
**                              - the station role could not be changed
**                                because the station was in the wrong state
**              qrc_ok          - station role was changed successfully
******************************************************************************/
enum qllc_rc_type qllc_alter_role (
  qllc_ls_type                      *link_station,
  enum qllc_role_type               new_role)
{
  enum qllc_rc_type qllc_rc;

  outputf("QLLC_LTEST: station role = %d\n",link_station->role);
  outputf("QLLC_LTEST: station_state = %d\n",link_station->state);
  outputf("QLLC_LTEST: station_predicate = %d\n",link_station->predicate);

  /*************************************************************************
   ** Reasons for failing to change role are:
   ** - the predicate is not NULL,
   ** - the station is contacted into normal response mode (OPENED state),
   ** - the station is primary and has sent a QSM (OPENING state).
   *************************************************************************/
  if ((link_station->predicate != qp_null)
    || (link_station->state == qs_opened)
    || ((link_station->state == qs_opening)
      && (link_station->role == qr_primary))
    )
    qllc_rc = qrc_alter_role_failed;
  else
  {
    /**********************************************************************
     ** The normal case if station role has to be changed is to simply set
     ** the link station role to the new role specified as a parameter to
     ** the function. A special case is a secondary station in OPENING
     ** state. Secondary stations have to be faked into OPENING state by
     ** QLLC (there is no higher layer stimulus to allow a
     ** secondary to accept set mode). Therefore, when a station is altered
     ** from secondary/opening to primary, we must set its state to CLOSED.
     ** Likewise, a primary station in closed state must be converted to
     ** opening state.
     **********************************************************************/
    if ((link_station->role==qr_secondary)&&(link_station->state==qs_opening))
      link_station->state = qs_closed;
    else if((link_station->role==qr_primary)&&(link_station->state==qs_closed))
      link_station->state = qs_opening;
    qllc_rc = qrc_ok;
    link_station->role = new_role;
/* defect 52838 */
    if (link_station->role == qr_secondary)
      link_station->negotiable = 0;
/* end defect 52838 */
  }
  return (qllc_rc);
}

/******************************************************************************
** ALTER STATION REPOLL TIMEOUT
** Function:    qllc_alter_repoll_timeout
**
** Description: This function changes the link station repoll timeout in
**              response to a request from SNA Services.  There is no
**              restriction on the link station states in which this change is
**              allowed.
**
** Parameters:  alter_repoll    - a flag showing whether repoll_time should be
**                                altered
**              new_repoll_time - the new repoll_time for the station.
******************************************************************************/
void qllc_alter_repoll_timeout (
  qllc_ls_type                      *link_station,
  bool                              alter_repoll,
  repoll_timer_type                 new_repoll_time)
{
  boolean timer_running = FALSE;
 
  outputf("qllc_alter_repoll_timeout\n");
  if (alter_repoll != FALSE)
  {
    outputf("qllc_alter_repoll_timeout: flag is set\n");
    /**************************************************************************/
    /* Check to see whether timer is running, because if it is, then restart  */
    /* it when the value has been modified.                                   */
    /**************************************************************************/
    if (link_station->repoll_dog.count != 0)
    {
      outputf("qllc_alter_repoll_timeout: timer running\n");
      timer_running = TRUE;
    }
 
    outputf("qllc_alter_repoll_timeout: stop timer\n");
    w_stop(&(link_station->repoll_dog));
    outputf("qllc_alter_repoll_timeout: clear timer\n");
/* defect 111172 */
    while (w_clear(&(link_station->repoll_dog)));
 
    link_station->repoll_time = new_repoll_time;
    link_station->repoll_dog.restart = new_repoll_time;
    outputf("qllc_alter_repoll_timeout: init timer with new value=%d\n",
      new_repoll_time);
    while (w_init(&(link_station->repoll_dog)));
/* end defect 111172 */
    /**************************************************************************/
    /* Start the timer running if it was running before alter was requested   */
    /**************************************************************************/
    if (timer_running == TRUE)
    {
      outputf("qllc_alter_repoll_timeout: restart timer\n");
      w_start(&(link_station->repoll_dog));
    }
  }
}

/******************************************************************************
** ALTER STATION MAX REPOLL COUNT
** Function:    qllc_alter_max_repoll
**
** Description: This function changes the link station max repoll count in
**              response to a request from SNA Services.  There is no
**              restriction on the link station states in which this change is
**              allowed.
**
** Parameters:  link_station    - address of link station to be altered       
**              new_max_repoll - the new max_repoll for the station.
******************************************************************************/
void qllc_alter_max_repoll (
  qllc_ls_type          *link_station,
  unsigned int           new_max_repoll)
{
  link_station->max_repoll = new_max_repoll;
}






