static char sccsid[] = "@(#)62  1.9  src/bos/kernext/dlc/qlc/qlcltime.c, sysxdlcq, bos411, 9428A410j 5/2/94 17:24:52";
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: qlm_qllc_repoll_timeout, qlm_ls_contacted, qlm_halt_timeout,
 *            qlm_inactivity_timeout                                    
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


/*****************************************************************************/
/* QLLC Timers are used to monitor various timeouts.                         */
/* There is one timer which is used by the channel to measure the intervals  */
/* between retries of receive data calls to kernel users. This timer is the  */
/* Retry_Timer.                                                              */
/* There are also a number of timers which monitor the intervals between     */
/* repolling of commands by a primary station */
/*****************************************************************************/


#include "qlcg.h"   
#include "qlcq.h"    
#include "qlcqmisc.h"
#include "qlcv.h"    
#include "qlcvfac.h" 
#include "qlcb.h"    
#include "qlcp.h"
#include "qlcc.h"                              
#include "qlcs.h"    
#include "qlcl.h"
#include "qlcltime.h"
#include "qlclutil.h"
#include "qlcctrc.h"
/*****************************************************************************/
/* Function     QLM_REPOLL_TIMEOUT                                           */
/*                                                                           */
/* Description  This procedure is called when the repoll timer set by the    */
/*              QQM expires.                                                 */
/*              The correlator passed is actually the correlator from a QLLC */
/*              link station. However this is the same as the SNA link       */
/*              correlator for that station, and by indexing into the SNA    */
/*              station, some security is afforded.                          */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Parameters   correlator_type   correlator;                                */
/*****************************************************************************/
void qlm_repoll_timeout(
  struct watchdog *w)
{
  station_type *station_ptr;
  port_type    *port_id;
  char this_func[]="qlm_repoll_timeout";

  station_ptr = (station_type *)
    ((char *)w - OFFSETOF(link_station.repoll_dog,station_type));
  if (station_ptr != NULL)
  {
    station_ptr->link_station.repoll_due = TRUE;
    port_id = QCM_RETURN_PORT_ID(station_ptr->channel_id);
    if ((port_id != NULL) && (port_id->port_id == port_id)) 
/* <<< THREADS >>> */
      et_post(REPOLL_TIMEOUT,port_id->int_tid);
/* <<< end THREADS >>> */
  }
  return;
}

/*****************************************************************************/
/* Function     QLM_SERVICE_REPOLLS                                          */
/*                                                                           */
/* Description  This procedure is called when the repoll timer set by the    */
/*              QQM expires and posts the kproc.                             */
/*              A station whose repoll timer has expired is marked on the    */
/*              timeout() interrupt thread as having repoll_due, but the     */
/*              repolls are performed here on the kproc in order to avoid    */
/*              locking on the interrupt thread and to minimise the path     */
/*              length of the timeout function.                              */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Parameters   correlator_type   correlator;                                */
/*****************************************************************************/
void qlm_service_repolls(port_type *port_id)

{
  qllc_rc_type qrc;
  station_type *station_ptr;
  channel_type *channel_id;
  qvm_rc_type qvm_rc;
  bool unlock;

  outputf("QLM_REPOLL_TIMEOUT: called\n");
  /***************************************************************************/
  /* Find all stations which have got repolls pending, and service them.     */
  /***************************************************************************/
  while ( (station_ptr = qlm_find_repoll_ls_on_port(port_id,&unlock))!= NULL)
  {
    station_ptr->link_station.repoll_due = FALSE;
    channel_id = station_ptr->channel_id;
    /***********************************************************************/
    /* Call QLLC Repoll function                                           */
    /* This will reset the timer when the command is polled.               */
    /***********************************************************************/
    outputf("QLM_REPOLL_TIMEOUT: call qllc_repoll()\n");
    qrc = qllc_repoll(
      &(station_ptr->link_station),
      &(station_ptr->virt_circuit),
      port_id
      );
    switch (qrc)
    {
    case qrc_ok :
      if (unlock) unlockl(&(station_ptr->lock));
      outputf("QLM_REPOLL_TIMEOUT: OK\n");
      break;
    case qrc_max_repolls_exceeded :
      /*********************************************************************/
      /* Generate Alert 3                                                  */
      /* Alert 3 is defined to indicate when max_repolls has been exceeded */
      /* & the station is disconnecting.                                   */
      /* There is some debate about whether it should be generated when    */
      /* the station HOLD flag is set. In the code which follows it is     */
      /* generated regardless of the setting of the HOLD flag.             */
      /* If this needs to be changed, then simply move the MACRO reference */
      /* into the second domain.                                           */
      /*********************************************************************/
      outputf("QLM_REPOLL_TIMEOUT: max repoll exceeded\n");
      qlcerrlog(
	ERRID_QLLC_ALERT_3,
	"qllc",
	station_ptr->user_ls_correlator,
	NULL,
	NULL
	);
      if ((station_ptr->flags & DLC_SLS_HOLD) == DLC_SLS_HOLD)
      {
	/*******************************************************************/
        /* Hold on Inactivity is TRUE                                      */
	/*******************************************************************/
	outputf("QLM_REPOLL_TIMEOUT: do not halt station\n");
	station_ptr->station_state = (byte)inactive;
	qcm_make_result (
	  channel_id,
	  QSM_RETURN_USER_SAP_CORRELATOR(station_ptr->qllc_sap_correlator),
	  station_ptr->user_ls_correlator,
	  inactivity_without_termination,
	  successful
	  );
	INC_RAS_COUNTER(station_ptr->ras_counters.inactivity_timeouts);
	if (unlock) unlockl(&(station_ptr->lock));
      }
      else
      {
	/*******************************************************************/
	/* Hold on Inact is FALSE                                          */
	/*******************************************************************/
	outputf("QLM_REPOLL_TIMEOUT: halt station\n");
	station_ptr->station_state = (byte)closing;
	station_ptr->reason_for_closure = (int)DLC_INACT_TO;
	qvm_rc = qvm_close_vc(
	  &(station_ptr->virt_circuit),
	  port_id,
	  TIMEOUT_CONDITION,
	  FALSE /* this is a local clear */
	  );
	outputf("QLM_REPOLL: qvm_close_vc rc = %d\n",qvm_rc);
	if (qvm_rc != qvm_rc_ok)
	{
	  outputf("QVM_CLOSE_VC: delete station\n");
	  station_ptr->reason_for_closure = sna_system_error;
	  qlm_delete_station(station_ptr,NULL,NULL);
	}
	else
	{
	  if (unlock) unlockl(&(station_ptr->lock));
	}
      }
      break;
    case qrc_x25_error :
      outputf("QLM_REPOLL_TIMEOUT: x25_error\n");
      qvm_rc = qvm_close_vc(
	&(station_ptr->virt_circuit),
	port_id,
	LOCAL_PROCEDURE_ERROR_GENERAL,
	FALSE /* this is a local clear */
	);
      if (qvm_rc != qvm_rc_ok)
      {
	station_ptr->reason_for_closure = sna_system_error;
	qlm_delete_station(station_ptr,NULL,NULL);
      }
      else
      {
	station_ptr->station_state = (byte)closing;
	station_ptr->reason_for_closure = (int)sna_system_error;
	if (unlock) unlockl(&(station_ptr->lock));
      }
      break;
    default :
      if (unlock) unlockl(&(station_ptr->lock));
      outputf("QLM_REPOLL_TIMEOUT: unknown rc from qllc fsm\n");
      break;
    }
  }
}

/*****************************************************************************/
/* Function     QLM_LS_CONTACTED                                             */
/*                                                                           */
/* Description  This procedure is called when the QLLC Link Station goes     */
/*              contacted, enters the OPENED state. Whenever a QLLC Status   */
/*              Report occurs that indicates a transition to the OPENED      */
/*              state, the QLM procedure which gets the status report calls  */
/*              this function.                                               */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Parameters   correlator_type   correlator;                                */
/*****************************************************************************/
void qlm_ls_contacted(

  correlator_type   correlator)
{
  station_type *station_ptr;
  channel_type *channel_id;
  bool unlock;

  /***************************************************************************/
  /* Initialise pointers to control blocks                                   */
  /***************************************************************************/
  station_ptr = qlm_find_ls_given_correlator(correlator,&unlock);
  if (station_ptr == (station_type *)NULL)
  {
    outputf("QLM_LS_CONTACTED: station not found\n");
    return;
  }
  channel_id = station_ptr->channel_id;
  /***************************************************************************/
  /* Check the station states (SNA and QLLC stations) as if either is not in */
  /* the OPENED state, then this procedure should do nothing.                */
  /***************************************************************************/
  outputf("QLM_LS_CONTACTED: station state = %d\n",station_ptr->station_state);
  outputf("QLM_LS_CONTACTED: qllc opened = %d\n",
    QLLC_LINK_STATION_IS_OPENED(&(station_ptr->link_station)));

  if (station_ptr->station_state == (byte)opened
    && QLLC_LINK_STATION_IS_OPENED(&(station_ptr->link_station))
    )
  {
    /*************************************************************************/
    /* It is OK to go contacted, as both the SNA and QLLC stations are in    */
    /* the OPENED state.                                                     */
    /*************************************************************************/
    outputf("QLM_LS_CONTACTED: going contacted\n");
    station_ptr->station_sub_state |= DLC_CONTACTED;
    /*************************************************************************/
    /* Stop the inactivity timer for this station.                           */
    /*************************************************************************/
    w_stop(&(station_ptr->inact_dog));
    qcm_make_result(
      channel_id,
      QSM_RETURN_USER_SAP_CORRELATOR(station_ptr->qllc_sap_correlator),
      station_ptr->user_ls_correlator,
      contacted,
      successful
    );
  }
  if (unlock) unlockl(&(station_ptr->lock));
}

/*****************************************************************************/
/* Function     QLM_HALT_TIMEOUT                                             */
/*                                                                           */
/* Description  This procedure is called on expiry of the timer set when a   */
/*              Halt_LS is issued, either as a direct response to a Halt     */
/*              Command from the user, or in response to a Disable_SAP call  */
/*              from the user or from the Close (Channel) function.          */
/*              If the Halt Done has not been received by the time this      */
/*              timer expires, the station is forced to close, and a halted  */
/*              result is sent to the user.                                  */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Parameters   correlator_type   correlator                                 */
/*****************************************************************************/
void qlm_halt_timeout(
  struct watchdog *w)
{
  station_type *station_ptr;
  port_type    *port_id;
  char this_func[]="qlm_halt_timeout";

  station_ptr = (station_type *)((char *)w - OFFSETOF(halt_dog,station_type));
  /***************************************************************************/
  /* The station ptr may be NULL, this is not an error condition, it simply  */
  /* means that the station did manage to halt within the time specified by  */
  /* the user.                                                               */
  /***************************************************************************/
  if (station_ptr != NULL)
  {
    station_ptr->forced_halt_due = TRUE;
    port_id = QCM_RETURN_PORT_ID(station_ptr->channel_id);
    if ((port_id != NULL) && (port_id->port_id == port_id)) 
/* <<< THREADS >>> */
      et_post(HALT_TIMEOUT,port_id->int_tid);
/* <<< end THREADS >>> */
  }
  return;
}

/*****************************************************************************/
/* Function     QLM_SERVICE_HALT_TIMEOUTS                                    */
/*                                                                           */
/* Description  This procedure is called on expiry of the force halt timer   */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Parameters   port_type *port_id                                           */
/*****************************************************************************/
void qlm_service_halt_timeouts(port_type *port_id)
{
  station_type *station_ptr;
  bool unlock;

  /***************************************************************************/
  /* Find all stations which have got forced halts pending, and service them.*/
  /***************************************************************************/
  while ( (station_ptr = qlm_find_forced_ls_on_port(port_id,&unlock))!= NULL)
  {
    station_ptr->forced_halt_due = FALSE;
    /***********************************************************************/
    /* Call qlm_delete_station()                                           */
    /* If you need to suppress the result generated by qlm_delete_station, */
    /* then set station_ptr->silent_halt to TRUE.                          */
    /***********************************************************************/
    outputf("QLM_DELETE_STATION: call qlm_delete_station()\n");
    (void)qlm_delete_station(station_ptr,NULL,NULL);
  }
}

/*****************************************************************************/
/* Function     QLM_INACTIVITY_TIMEOUT                                       */
/*                                                                           */
/* Description  This procedure is called on expiry of the inactivity timer   */
/*              which is regularly reset (whenever activity is detected).    */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* This is called on the interrupt level, and the fact that the timer was    */
/* is sufficient to guarantee that the station_ptr is valid.                 */
/*                                                                           */
/* Parameters   correlator_type   correlator                                 */
/*****************************************************************************/
void qlm_inactivity_timeout(
  struct watchdog *w)
{
  station_type *station_ptr;
  channel_type *channel_id;
  port_type    *port_id;
  char this_func[]="qlm_inactivity_timeout";

  station_ptr = (station_type *)((char *)w - OFFSETOF(inact_dog,station_type));
  channel_id = station_ptr->channel_id;

  if (station_ptr != NULL)
  {
    station_ptr->inactivity_detected = TRUE;
    port_id = QCM_RETURN_PORT_ID(station_ptr->channel_id);
    if ((port_id != NULL) && (port_id->port_id == port_id)) 
/* <<< THREADS >>> */
      et_post(INACT_TIMEOUT,port_id->int_tid);
/* <<< end THREADS >>> */
  }
  return;
}

/*****************************************************************************/
/* Function     QLM_SERVICE_INACTIVITY_TIMEOUTS                              */
/*                                                                           */
/* Description  This procedure is called on expiry of the inactivity timer   */
/*              which is regularly reset (whenever activity is detected).    */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Parameters   correlator_type   correlator                                 */
/*****************************************************************************/
void qlm_service_inactivity_timeouts(
  port_type *port_id)
{
  station_type *station_ptr;
  channel_type *channel_id;
  qvm_rc_type   qvm_rc;
  bool          unlock;
 
  /***************************************************************************/
  /* Find all stations which have detected inactivity, and service them.     */
  /***************************************************************************/
  while ( (station_ptr = qlm_find_inact_ls_on_port(port_id,&unlock))!= NULL)
  {
    station_ptr->inactivity_detected = FALSE;
    channel_id = station_ptr->channel_id;
    INC_RAS_COUNTER(station_ptr->ras_counters.inactivity_timeouts);
    if (station_ptr->flags & DLC_SLS_HOLD)
    {
      /*********************************************************************/
      /* Restart timer                                                     */
      /*********************************************************************/
      /* w_start(&(station_ptr->inact_dog)); */
      /*********************************************************************/
      /* Should the station go to inactive state (regardless of role)??    */
      /*********************************************************************/
      station_ptr->station_state = inactive; 
      /*********************************************************************/
      /* Don't halt the link station, just report inactivity               */
      /*********************************************************************/
      qcm_make_result(
	channel_id,
      	QSM_RETURN_USER_SAP_CORRELATOR(station_ptr->qllc_sap_correlator),
	station_ptr->qllc_ls_correlator,
	DLC_IWOT_RES,
	DLC_SUCCESS
	);
      TRACE_TIMER(station_ptr,DLC_TO_INACT);
      if (unlock) unlockl(&(station_ptr->lock));
    }
    else
    {
      /*********************************************************************/
      /* Halt the link station as it has been inactive for too long.       */
      /*********************************************************************/
      TRACE_TIMER(station_ptr,DLC_TO_INACT);
      outputf("QLM_INACT_TIMEOUT: halt station\n");
      qvm_rc = qvm_close_vc(
	&(station_ptr->virt_circuit),
	port_id,
	TIMEOUT_CONDITION,
	FALSE /* this is a local clear */
	);
      if (qvm_rc != qvm_rc_ok)
      {
	station_ptr->reason_for_closure = sna_system_error;
	qlm_delete_station(station_ptr,NULL,NULL);
      }
      else
      {
	station_ptr->station_state = (byte)closing;
	station_ptr->reason_for_closure = DLC_INACT_TO;
	if (unlock) unlockl(&(station_ptr->lock));
      }
    }
  }
}

/*****************************************************************************/
/* Function     QLM_RETRY_TIMEOUT                                            */
/*                                                                           */
/* Description  This procedure is called when the retry timer set by the     */
/*              QLM expires.                                                 */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* This is called on the interrupt level, and the fact that the timer was    */
/* is sufficient to guarantee that the station_ptr is valid.                 */
/*                                                                           */
/* Parameters   struct watchdog *w                                           */
/*****************************************************************************/
void qlm_retry_timeout(
  struct watchdog *w)
{
  station_type *station_ptr;
  port_type    *port_id;
  char this_func[]="qlm_retry_timeout";

  station_ptr = (station_type *)
    ((char *)w - OFFSETOF(retry_dog,station_type));
  if (station_ptr != NULL)
  {
    station_ptr->retry_pending = TRUE;
    port_id = QCM_RETURN_PORT_ID(station_ptr->channel_id);
    if ((port_id != NULL) && (port_id->port_id == port_id)) 
/* <<< THREADS >>> */
      et_post(RETRY_TIMEOUT,port_id->int_tid);
/* <<< end THREADS >>> */
  }
  return;
}


/*****************************************************************************/
/* Function:    Initialise a watchdog timer.                                 */
/*                                                                           */
/* Description  This procedure sets up a watchdog timer which the caller     */
/*              supplies, and registers it with the kernel.                  */
/*                                                                           */
/* Return        void                                                        */
/*                                                                           */
/* Parameters:   w    is ptr to watchdog to be initialised.                  */
/*               dur  is duration in seconds.                                */
/*****************************************************************************/
void qlm_init_watchdog(
  struct watchdog  *w,
  void            (*func)(),
  unsigned int      dur)
{
  bool bibble=TRUE;

  /***************************************************************************/
  /* This may look strange code, but it is to ensure that there is a check_  */
  /* point in a function in the timer module which will be called before the */
  /* handlers running on the interrupt thread get called. Otherwise the      */
  /* check_point on the int thread will be the first, and will try and alloc */
  /***************************************************************************/
  if (bibble)
  {
    bibble=FALSE;				  /* No bibbles allowed      */
  }
  w->next = w->prev = NULL;
  w->func = func;
  w->count = 0;
  w->restart = dur;
/* defect 111172 */
  while (w_init(w));
/* end defect 111172 */
  return;
}
