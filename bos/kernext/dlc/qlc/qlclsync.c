static char sccsid[] = "@(#)11	1.18  src/bos/kernext/dlc/qlc/qlclsync.c, sysxdlcq, bos411, 9437C411a 9/15/94 09:42:25";

/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: qlm_start_ls, qlm_initialise_station,
 *            qlm_convert_sna_facs_to_cb_fac, qlm_halt_ls, qlm_query_ls,
 *            qlm_trace, qlm_test, qlm_alter, qlm_contact,
 *            qlm_enter_local_busy, qlm_exit_local_busy, qlm_write
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


#include "qlcg.h"     /* correlator_type                                     */
	              /* diag_tag_type                                       */
	              /* trace_channel_type                                  */
	              /* x25_address_type                                    */
	              /* lcn_type                                            */
	              /* ras_counter_type                                    */
#include "qlcq.h"     /* qllc_ls_type                                        */
#include "qlcv.h"     /* x25_vc_type                                         */
#include "qlcvfac.h"  /* facilities                                          */
#include "qlcb.h"     /* gen_buffer_type                                     */
#include "qlcp.h"
#include "qlcc.h"     /* for channel_id_type                                 */
#include "qlcs.h"     /* qsm_rc_type                                         */
#include "qlcl.h"
#include "qlclsync.h"
#include "qlcltime.h"
#include "qlclutil.h"
#include "qlcctrc.h"
#include "qlcvsess.h" /* Defect 110313 */

extern channel_list_type channel_list;

/*****************************************************************************/
/* Function     QLM_START_LS                                                 */
/*                                                                           */
/* Description  This procedure is called when the user issues a Start_LS     */
/*              ioctl. It checks that the SAP is able to support the LS, and */
/*              allocates resources for the LS. The LS is initialised.       */
/*              On successful completion the parm block will have been       */
/*              updated by the insertion of a qllc_ls_correlator. A new LS   */
/*              will have been added to the LS linked list for the SAP.      */
/*                                                                           */
/* Return       qlm_rc_type     rc;                                          */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*   channel_id_type  channel_id;                                            */
/*                                                                           */
/*   start_ls_ioctl_ext_type *parm_block_ptr;                                */
/*                                                                           */
/*****************************************************************************/
qlm_rc_type qlm_start_ls(

  channel_id_type channel_id,
  struct qlc_sls_arg *qlc_ext_ptr)
{
  struct dlc_sls_arg *ext_ptr;
  qsm_rc_type   qsm_rc;
  qvm_rc_type   qvm_rc;
  qlm_rc_type   rc;
  sap_type     *sap_ptr;
  station_type *station_ptr;
  station_type *station_list_ptr;
  station_type *contending_ls_ptr;
  station_type *last_station_in_list;
  init_rec_type init_rec;            /* from qlcv.h                          */
  char          tmp_addr[DLC_MAX_NAME];
  int           tmp_addr_length;
  int           i;
  bool		unlock_list;
  bool		unlock_sap;
  bool          unlock_cont;
/* defect 160635 */
  port_type     *port_ptr;
  char          x25_adapter_num;
  char          pvc_channel[DLC_MAX_NAME];
/* end defect 160635 */

  outputf("QLM_START_LS: ..has been called\n");
  outputf("QLM_START_LS: remote address =[%s] \n",qlc_ext_ptr->dlc.raddr_name);
  /***************************************************************************/
  /* Convert input pointers to convenient types                              */
  /***************************************************************************/
  ext_ptr = (struct dlc_sls_arg *)qlc_ext_ptr;

  /***************************************************************************/
  /* Get global list lock                                                    */
  /***************************************************************************/
  unlock_list = (lockl(&channel_list.lock, LOCK_SHORT) != LOCK_NEST);

  /***************************************************************************/
  /* Convert correlator to pointer and lock
  /***************************************************************************/
  sap_ptr = qsm_find_sap_given_correlator(ext_ptr->gdlc_sap_corr, &unlock_sap);
  if (sap_ptr == NULL)
  {
    if (unlock_list) unlockl(&channel_list.lock);
    return qlm_rc_user_interface_error;
  }
 
  /***************************************************************************/
  /* Check whether SAP is OK                                                 */
  /***************************************************************************/
  qsm_rc = qsm_check_sap(sap_ptr);
  if (qsm_rc != qsm_rc_ok)
  {
    outputf("QLM_START_LS: sap mgr is not happy....\n");
    if (unlock_sap) unlockl(&sap_ptr->lock);
    if (unlock_list) unlockl(&channel_list.lock);
    return qsm_rc;
  }
  outputf("QLM_START_LS: sap can accommodate start.\n");
  /***************************************************************************/
  /* SAP exists and can accommodate another station                          */
  /***************************************************************************/

  /***************************************************************************/
  /* If there is already a listening station the DH will pick up the         */
  /* duplication, and return a bad Start_Done. There is therefore no need to */
  /* to check here to see if there is already a listener started.            */
  /***************************************************************************/

  /***************************************************************************/
  /* Cannot allow a call to be made with an invalid remote id.               */
  /***************************************************************************/
  /* if LSVC (link station virtual call) is set we are CALLING */
  if ( (ext_ptr->flags & DLC_SLS_LSVC) == DLC_SLS_LSVC
    && qlm_remote_stn_id_is_valid(ext_ptr->raddr_name,ext_ptr->len_raddr_name)
    == FALSE
    )
  {
    outputf("QLM_START_LS: calling station with invalid remote id\n");
    qcm_make_result(
      channel_id,
      QSM_RETURN_USER_SAP_CORRELATOR(ext_ptr->gdlc_sap_corr),
      ext_ptr->user_ls_corr,
      station_halted,                /* enum result_indicators_type          */
      invalid_remote_name            /* enum result_code_type                */
      );
    if (unlock_sap) unlockl(&sap_ptr->lock);
    if (unlock_list) unlockl(&channel_list.lock);
    rc = qlm_rc_invalid_remote_name;
    return(rc);
  }

  /***************************************************************************/
  /* Cannot make a call to a remote node to which there is already a circuit */
  /* established                                                             */
  /***************************************************************************/

/* defect 160635 */
  /***************************************************************************/
  /* If this is a pvc, fudge the port id into the remote name.  This will    */
  /* allow the same PVC channel number on different ports to have unique     */
  /* names and avoid link station remote name contention.                    */
  /***************************************************************************/
  if ( ext_ptr->raddr_name[0] == 'P' || ext_ptr->raddr_name[0] == 'p')
  {
    port_ptr = QCM_RETURN_PORT_ID(channel_id);
    x25_adapter_num = port_ptr->xdh_pathname[strlen(port_ptr->xdh_pathname)-1];

    /* sanity check on adapter path name */
    if (x25_adapter_num < '0' || x25_adapter_num > '9') {
       outputf("QLM_START_LS: invalid adapter number [%c], pathname [%s]\n",
                 x25_adapter_num, port_ptr->xdh_pathname);
       rc = qlm_rc_port_error;
       if (unlock_sap) unlockl(&sap_ptr->lock);
       if (unlock_list) unlockl(&channel_list.lock);
       return (rc);
    }

    outputf("QLM_START_LS: PVC: adding adapter num [%c] into remote name [%s]",
              x25_adapter_num, ext_ptr->raddr_name);

    strncpy(pvc_channel, &ext_ptr->raddr_name[1], DLC_MAX_NAME);
    ext_ptr->raddr_name[1] = x25_adapter_num;
    strncpy(&ext_ptr->raddr_name[2], pvc_channel, DLC_MAX_NAME-2);
    ext_ptr->len_raddr_name += 1;
    outputf("QLM_START_LS: new remote name: %s, length: %d\n",
              ext_ptr->raddr_name, ext_ptr->len_raddr_name);
  }
/* defect 160635 */

  /***************************************************************************/
  /* Set correlator to NULL in next call, because the station is not in the  */
  /* station list yet, so that the function will report ANY station with a   */
  /* matching remote id.                                                     */
  /***************************************************************************/
  tmp_addr_length = ext_ptr->len_raddr_name;
  strncpy(tmp_addr,ext_ptr->raddr_name,DLC_MAX_NAME);
/* defect 156503 */
  contending_ls_ptr = qlm_find_contending_ls(sap_ptr, NULL,tmp_addr,
                                             &unlock_cont);
/* end defect 156503 */
  if (contending_ls_ptr != (station_type *)NULL)
  {
    outputf("QLM_START_LS: contention detected\n");
    qcm_make_contention_result(
      channel_id,
      QSM_RETURN_USER_SAP_CORRELATOR(ext_ptr->gdlc_sap_corr),
      ext_ptr->user_ls_corr,
      contending_ls_ptr->qllc_ls_correlator
      );

    if (unlock_cont) unlockl(&contending_ls_ptr->lock);
    if (unlock_sap) unlockl(&sap_ptr->lock);
    if (unlock_list) unlockl(&channel_list.lock);
    rc = qlm_rc_contention;
    return(rc);
  }

  /***************************************************************************/
  /* The attempt to start a station is valid and the QLM will try and        */
  /* allocate storage                                                        */
  /***************************************************************************/
  station_ptr = (station_type *)xmalloc(sizeof(station_type),WORD,pinned_heap);
  outputf("QLM_START_LS: station has been allocated\n");
  /***************************************************************************/
  /* Check whether malloc failed                                             */
  /***************************************************************************/
  if (station_ptr == (station_type *)NULL)
  {
    outputf("QLM_START_LS: ....but allocate failed\n");
    qcm_make_result (
      channel_id,
      QSM_RETURN_USER_SAP_CORRELATOR(ext_ptr->gdlc_sap_corr),
      ext_ptr->user_ls_corr,
      station_halted,
      sna_system_error
      );
    if (unlock_sap) unlockl(&sap_ptr->lock);
    if (unlock_list) unlockl(&channel_list.lock);
    rc = qlm_rc_alloc_failed;
    return(rc);
  }

  /***************************************************************************/
  /* Malloc was successful, start with zeroed station_ptr                    */
  /***************************************************************************/
  outputf("QLM_START_LS: alloc was successful\n");
  bzero(station_ptr, sizeof(*station_ptr));

  /***************************************************************************/
  /* Set the lock word on the newly allocated station control block          */
  /***************************************************************************/
  station_ptr->lock = LOCK_AVAIL;
  /***************************************************************************/
  /* Chain station into linked list                                          */
  /***************************************************************************/
  /***************************************************************************/
  /* Lock station struct for list manipulation                               */
  /***************************************************************************/
  lockl(&station_ptr->lock, LOCK_SHORT);
  /***************************************************************************/
  /* If qsm_return_station_list_ptr returned a null value it means the list  */
  /* is empty. This is the first station to be put into it.                  */
  /***************************************************************************/
  station_list_ptr = sap_ptr->station_list_ptr;
  if (station_list_ptr == NULL)
  {
    outputf("QLM_START_LS: this is the first station\n");
    sap_ptr->station_list_ptr = station_ptr;
    station_ptr->next_station_ptr = NULL;
    station_ptr->prev_station_ptr = NULL;
  }
  else
  {
    outputf("QLM_START_LS: adding station to list with others\n");
    /*************************************************************************/
    /* There are already other station/s in the list. We must find the end of*/
    /* the list so the new station can be added to it.                       */
    /*************************************************************************/
    last_station_in_list = station_list_ptr;
    while (last_station_in_list->next_station_ptr != NULL)
    {
      last_station_in_list = last_station_in_list->next_station_ptr;
    }
    /*************************************************************************/
    /* last_station_in_list now pts to last station in list.                 */
    /* Add the new station to the end of the list                            */
    /*************************************************************************/
    last_station_in_list->next_station_ptr = station_ptr;
    station_ptr->prev_station_ptr = last_station_in_list;
    station_ptr->next_station_ptr = NULL;
  }

  /***************************************************************************/
  /* Copy ext into station                                                   */
  /***************************************************************************/
  outputf("QLM_START_LS: calling init station\n");
  qlm_initialise_station(station_ptr,channel_id,qlc_ext_ptr);

  /*************************************************************************/
  /* unlock sap now that station is all initialised                        */
  /*************************************************************************/
  if (unlock_sap) unlockl(&sap_ptr->lock);
  if (unlock_list) unlockl(&channel_list.lock);

  /***************************************************************************/
  /* Get 5 timers                                                            */
  /***************************************************************************/
/*  outputf("QLM_START_STATION: get 5 timers\n"); */
/*  (void)timeoutcf(+5); */
/*  outputf("QLM_START_STATION: got 5 timers\n"); */
  /***************************************************************************/
  /* We must now decide whether this is a listening station, or a calling    */
  /* station, as the x25 vc is only opened when calling. Listeners have      */
  /* their x25 vc opened when an incoming call arrives                       */
  /***************************************************************************/
  if ( (station_ptr->flags & DLC_SLS_LSVC) == FALSE)
  {
    outputf("QLM_START_LS: listening station\n");
    /*************************************************************************/
    /* Initialise the QLLC link station. This is a void function.            */
    /*************************************************************************/
    (void)qllc_init_station(
      &(station_ptr->link_station),
      ((((station_ptr->flags) & DLC_SLS_STAT) == DLC_SLS_STAT) ?
	qr_primary : qr_secondary),
      station_ptr->max_repoll,
      station_ptr->repoll_time,
      station_ptr->qllc_ls_correlator,
      (station_ptr->flags & DLC_SLS_NEGO)    /* defect 52838 */
      );
    /*************************************************************************/
    /* Issue a start listen to the DH                                        */
    /*************************************************************************/
    init_rec.netid = station_ptr->netid;
    init_rec.circuit = session_svc_listen;
    (void)strncpy(
      init_rec.session_name,
      station_ptr->station_tag,
      DLC_MAX_DIAG
      );
    init_rec.correlator = (correlator_type)station_ptr;
    switch (station_ptr->support_level)
    {
    case 1980:
      init_rec.protocol = X25_PROTOCOL_QLLC_80;
      break;
    case 1984:
      init_rec.protocol = X25_PROTOCOL_QLLC_84;
      break;
    default:
      init_rec.protocol = X25_PROTOCOL_QLLC_84;
      break;
    }
    outputf("QLM_START_LS: copying %s into listen name\n",
      station_ptr->listen_name);
    strncpy(init_rec.listen_name,station_ptr->listen_name,8);
    /*************************************************************************/
    /* Hand over to QVM                                                      */
    /*************************************************************************/
    qvm_rc = qvm_open_vc (
      qlc_ext_ptr,	/* Defect 110313 */
      &(station_ptr->virt_circuit),
      QCM_RETURN_PORT_ID(channel_id),
      &(init_rec)
      );
    /*************************************************************************/
    /* And check all went OK                                                 */
    /*************************************************************************/
    if (qvm_rc != qvm_rc_ok)
    {
      outputf("QLM_START_LS: open vc was a failure....\n");
      qlm_delete_station(station_ptr, NULL, NULL);
      switch (qvm_rc)
      {
      case qvm_rc_no_name :
	rc = qlm_rc_bad_listen_name;
	break;
      default :
	rc = qlm_rc_open_vc_failed;
	break;
      }
    }
    else
    {
      unlockl(&(station_ptr->lock));
      rc = qlm_rc_ok;
    }
    return (rc);
  }

  /***************************************************************************/
  /* This is a request to start a calling station.                           */
  /***************************************************************************/

  /***************************************************************************/
  /* Find out whether this request is for a call over an SVC or a PVC        */
  /***************************************************************************/
  if ( ext_ptr->raddr_name[0] == 'P' || ext_ptr->raddr_name[0] == 'p')
  {
    /*************************************************************************/
    /* This is a PVC                                                         */
    /*************************************************************************/
    outputf("QLM_START_LS: this is a PVC");
    outputf("QLM_START_LS: calling fsm to init\n");
    (void)qllc_init_station (
      &(station_ptr->link_station),
      ((station_ptr->flags & DLC_SLS_STAT) == DLC_SLS_STAT ?
	qr_primary : qr_secondary),
      station_ptr->max_repoll,
      station_ptr->repoll_time,
      station_ptr->qllc_ls_correlator,
      (station_ptr->flags & DLC_SLS_NEGO)    /* defect 52838 */
      );
    /*************************************************************************/
    /* Fill in init_rec                                                      */
    /*************************************************************************/
    init_rec.netid = station_ptr->netid;
    init_rec.circuit = session_pvc;
    (void)strncpy(
      init_rec.session_name,
      station_ptr->station_tag,
      DIAG_TAG_LENGTH
      );

    switch (station_ptr->support_level)
    {
    case 1980:
      init_rec.protocol = X25_PROTOCOL_QLLC_80;
      break;
    case 1984:
      init_rec.protocol = X25_PROTOCOL_QLLC_84;
      break;
    default:
      init_rec.protocol = X25_PROTOCOL_QLLC_84;
      break;
    }

    init_rec.correlator = (correlator_type)station_ptr;
/* defect 160635 */
    /* PVC Channel Number is remote station id with "P/p" removed */
    init_rec.channel_num = atoi(&(station_ptr->remote_addr[2]));
/* end defect 160635 */

    outputf("QLM_START_LS: calling qvm_open_vc\n");
    qvm_rc = qvm_open_vc (
      qlc_ext_ptr,	/* Defect 110313 */
      &(station_ptr->virt_circuit),
      QCM_RETURN_PORT_ID(channel_id),
      &(init_rec)
      );
    if (qvm_rc != qvm_rc_ok)
    {
      outputf("QLM_START_LS: open vc failed\n");
      /***********************************************************************/
      /* Open VC failed                                                      */
      /* There are two cases to consider. One is that there was a system     */
      /* error which would arise from failure to get a buffer. In that case  */
      /* case the station should be closed down.                             */
      /* The other case is when there was a port error, and under these      */
      /* circumstances, the station is still closed down, and a port_error   */
      /* rc passed to QDH, which can then close the channels and port if     */
      /* necessary                                                           */
      /***********************************************************************/
      station_ptr->qllc_sap_correlator = ext_ptr->gdlc_sap_corr;
      station_ptr->user_ls_correlator = ext_ptr->user_ls_corr;
      station_ptr->reason_for_closure = link_station_resource_outage;
      qlm_delete_station(station_ptr,NULL,NULL);
      if (qvm_rc == qvm_rc_system_error)
	rc = qlm_rc_open_vc_failed;
      else  /* qvm_rc == qvm_rc_port_error */
	rc = qlm_rc_port_error;
      return(rc);
    }
    else
    {
      outputf("QLM_START_LS: open vc was successful\n");
      rc = qlm_rc_ok;
    }
    unlockl(&(station_ptr->lock));
    return (rc);
  }
  /***************************************************************************/
  /* This is an SVC                                                          */
  /***************************************************************************/
  outputf("QLM_START_LS: this is an SVC\n");
  (void)qllc_init_station (
    &(station_ptr->link_station),
    ((station_ptr->flags & DLC_SLS_STAT) == DLC_SLS_STAT ?
      qr_primary : qr_secondary),
    station_ptr->max_repoll,
    station_ptr->repoll_time,
    station_ptr->qllc_ls_correlator,
    (station_ptr->flags & DLC_SLS_NEGO)    /* defect 52838 */
    );
  /***************************************************************************/
  /* Fill in init_rec                                                        */
  /***************************************************************************/
  init_rec.netid = station_ptr->netid;
  init_rec.circuit = session_svc_out;
  (void)strncpy(
    init_rec.session_name,
    station_ptr->station_tag,
    DIAG_TAG_LENGTH
    );

  init_rec.correlator = (correlator_type)station_ptr;

  switch (station_ptr->support_level)
  {
  case 1980:
    init_rec.protocol = X25_PROTOCOL_QLLC_80;
    break;
  case 1984:
    init_rec.protocol = X25_PROTOCOL_QLLC_84;
    break;
  default:
    init_rec.protocol = X25_PROTOCOL_QLLC_84;
    break;
  }
  /***************************************************************************/
  /* Copy the facilities structure from the station to the init_rec.         */
  /***************************************************************************/
  outputf("Copy facilities from station into init_rec.\n");
  init_rec.facilities = station_ptr->facilities;
  outputf("facilities in init_rec.....\n");
  print_cb_fac((char *)&(init_rec.facilities));
  strcpy(init_rec.recipient_address,station_ptr->remote_addr);
  outputf("QLM_START_LS: addr in init_rec [%s]\n",init_rec.recipient_address);

  /***************************************************************************/
  /* Open the VC                                                             */
  /***************************************************************************/
  qvm_rc = qvm_open_vc (
    qlc_ext_ptr,	/* Defect 110313 */
    &(station_ptr->virt_circuit),
    QCM_RETURN_PORT_ID(channel_id),
    &init_rec
    );
  if (qvm_rc != qvm_rc_ok)
  {
    outputf("QLM_START_LS: open vc failed\n");
    /*************************************************************************/
    /* Open VC failed                                                        */
    /*************************************************************************/
    station_ptr->qllc_sap_correlator = ext_ptr->gdlc_sap_corr;
    station_ptr->user_ls_correlator = ext_ptr->user_ls_corr;
    station_ptr->reason_for_closure = link_station_resource_outage;
    qlm_delete_station(station_ptr,NULL,NULL);
    rc = qlm_rc_open_vc_failed;
    return(rc);
  }
  else
  {
    outputf("QLM_START_LS: open vc was successful\n");
    rc = qlm_rc_ok;
    unlockl(&(station_ptr->lock));
    return (rc);
  }
}

/*****************************************************************************/
/* Function     QLM_INITIALISE_STATION                                       */
/*                                                                           */
/* Description  This procedure is used by the QLM_Start_LS procedure to      */
/*              initialise the fields in the station record.                 */
/*              On successful completion the station record will have been   */
/*              initialised from the config information in the Start_LS ext  */
/*                                                                           */
/* Return       qlm_rc_type  rc;                                             */
/*                                                                           */
/* Parameters   station_type *station_ptr;                                   */
/*              channel_id_type channel_id;                                  */
/*              struct dlc_sls_arg *ext_ptr;                                 */
/*                                                                           */
/*        NOTE THAT STATION AND SAP ARE ALREADY LOCKED BY CALLING PROCEDURE  */
/*****************************************************************************/
void qlm_initialise_station(

  station_type *station_ptr,
  channel_type *channel_id,
  struct qlc_sls_arg *qlc_ext_ptr)
{
  struct dlc_sls_arg *ext_ptr;
  int i;
  /***************************************************************************/
  /* Initialise local pointers                                               */
  /***************************************************************************/
  ext_ptr = (struct dlc_sls_arg *)qlc_ext_ptr;
  outputf("QLM_INIT_STATION: ext_ptr = %d\n",ext_ptr);
  outputf("QLM_INIT_STATION: station_ptr = %d\n",station_ptr);
  /***************************************************************************/
  /* This procedure copies the station configuration info                    */
  /* from the ext passed to the start entry point into the                   */
  /* station struct allocated in Start_LS.                                   */
  /* It also copies the station_ptr into the returned correlator             */
  /* field in the configuration extension, which is returned to              */
  /* the user.                                                               */
  /***************************************************************************/
  /***************************************************************************/
  /* Put station ptr into config area                                        */
  /***************************************************************************/
  ext_ptr->gdlc_ls_corr = (correlator_type)station_ptr;
  /***************************************************************************/
  /* Also put station_ptr into correlator field in station                   */
  /***************************************************************************/
  station_ptr->qllc_ls_correlator = (correlator_type)station_ptr;
  /***************************************************************************/
  /* Copy config information to station                                      */
  /***************************************************************************/
  outputf("QLM_INIT_STATION: start to copy config info into station\n");
  strncpy(station_ptr->station_tag,ext_ptr->ls_diag,DLC_MAX_DIAG);
  /***************************************************************************/
  /* The session name is no longer needed for routing so there is no need to */
  /* customise it. This means however that stations using the same logical   */
  /* configuration profile will have the same name, which may be undesirable */
  /* when used in RAS context.                                               */
  /***************************************************************************/
  station_ptr->qllc_sap_correlator  = ext_ptr->gdlc_sap_corr;
  station_ptr->user_ls_correlator   = ext_ptr->user_ls_corr;

  station_ptr->flags                = ext_ptr->flags;
  station_ptr->trace_channel        = ext_ptr->trace_chan;

  if (((ext_ptr->flags) & DLC_SLS_LSVC) == DLC_SLS_LSVC)
  {
    outputf("QLM_INIT_STATION: this is a calling station\n");
    /*************************************************************************/
    /* This is a calling station. Copy the remote station id into the        */
    /* station record                                                        */
    /*************************************************************************/
    strncpy(station_ptr->remote_addr,ext_ptr->raddr_name,DLC_MAX_NAME);
    station_ptr->remote_addr_len=ext_ptr->len_raddr_name;
    /*************************************************************************/
    /* Now ensure NULL termination of address                                */
    /*************************************************************************/
    station_ptr->remote_addr[station_ptr->remote_addr_len] = '\0';
  }
  else
  {
    outputf("QLM_INIT_STATION: this is not a calling station\n");
    /*************************************************************************/
    /* This is a listening station and has no remote station id              */
    /*************************************************************************/
    station_ptr->remote_addr_len = 0;
  }
  /***************************************************************************/
  /* QLLC imposes no limit on the amount of information that can be          */
  /* transferred in an I-frame. This is because additional mbufs             */
  /* will be chained together to make enough space for any size I-field      */
  /***************************************************************************/
  station_ptr->max_i_field = ext_ptr->maxif;
  /***************************************************************************/
  /* Following two fields are not used by QLLC. They are copied anyway       */
  /***************************************************************************/
  station_ptr->receive_window_size  = ext_ptr->rcv_wind;
  station_ptr->transmit_window_size = ext_ptr->xmit_wind;
  /***************************************************************************/
  /* Copy repoll counter and timers from config info                         */
  /***************************************************************************/
  station_ptr->max_repoll   = ext_ptr->max_repoll;
  station_ptr->repoll_time  = ext_ptr->repoll_time;
  station_ptr->inact_time   = ext_ptr->inact_time;
  station_ptr->force_halt_time = ext_ptr->force_time;

  strncpy(station_ptr->listen_name,qlc_ext_ptr->psd.listen_name,8);
  station_ptr->support_level = qlc_ext_ptr->psd.support_level;

  /***************************************************************************/
  /* If this is a calling SVC, copy facilities structure                     */
  /* This is not quite as straight forward as it might have been as the link */
  /* station now stores its facilities as a cb_fac structure, which contains */
  /* a superset of the facilities available presently to the sna user. This  */
  /* allows use of the generic facs parser, and also will mean any new facs  */
  /* (e.g. 1988) should be easier to incorporate.                            */
  /***************************************************************************/
  if (((ext_ptr->flags) & DLC_SLS_LSVC) == DLC_SLS_LSVC &&
    ((ext_ptr->raddr_name[0] != 'P') && (ext_ptr->raddr_name[0] != 'p')) )
  {
    /*************************************************************************/
    /* First initialise facilities structure.                                */
    /*************************************************************************/
    station_ptr->facilities.flags = 0;
    station_ptr->facilities.fac_ext_len = 0;
    station_ptr->facilities.fac_ext = 0;
    station_ptr->facilities.psiz_clg = 0;
    station_ptr->facilities.psiz_cld = 0;
    station_ptr->facilities.wsiz_clg = 0;
    station_ptr->facilities.wsiz_cld = 0;
    station_ptr->facilities.tcls_clg = 0;
    station_ptr->facilities.tcls_cld = 0;
    station_ptr->facilities.rpoa_id_len = 0;
    station_ptr->facilities.rpoa_id = 0;
    station_ptr->facilities.cug_id = 0;
    station_ptr->facilities.nui_data_len = 0;
    station_ptr->facilities.ci_seg_cnt_len = 0;
    station_ptr->facilities.ci_mon_unt_len = 0;
    station_ptr->facilities.ci_call_dur_len = 0;
    station_ptr->facilities.clamn = 0;
    *(station_ptr->facilities.call_redr_addr) = '\0';
    station_ptr->facilities.call_redr_reason = '\0';
    station_ptr->facilities.min_tcls_clg = 0;
    station_ptr->facilities.min_tcls_cld = 0;
    /*************************************************************************/
    /* The following call is to a function that copies the ext facs to the   */
    /* sna station, converting them into cb_fac_t format in the process      */
    /*************************************************************************/
    outputf("QLM_INIT_STATION: calling facs function..\n");
    qvm_convert_sna_facs_to_cb_fac(
      &(qlc_ext_ptr->psd.facilities),
      &(station_ptr->facilities)
      );
    outputf("QLM_INIT_STATION: station now has facilities...\n");
    print_cb_fac((char *)&(station_ptr->facilities));

    /*************************************************************************/
    /* and additionally copy the window sizes from their GDLC defined        */
    /* fields into the facilities fields defined by QLLC.                    */
    /*************************************************************************/
    station_ptr->facilities.wsiz_cld = ext_ptr->rcv_wind;
    station_ptr->facilities.wsiz_clg = ext_ptr->xmit_wind;
  }
  /***************************************************************************/
  /* Initialise fields in station not given in configuration                 */
  /***************************************************************************/
  station_ptr->channel_id = channel_id;
  station_ptr->netid = qlm_select_netid();
  station_ptr->station_state = opening;    /* enum station_state_type        */
  /***************************************************************************/
  /* Station_Sub_State                                                       */
  /***************************************************************************/
  if (((station_ptr->flags) & DLC_SLS_LSVC) == DLC_SLS_LSVC)
  {
    station_ptr->station_sub_state &= ~DLC_LISTENING;
    station_ptr->station_sub_state |= DLC_CALLING;
  }
  else
  {
    /*************************************************************************/
    /* This is a listening station                                           */
    /*************************************************************************/
    station_ptr->station_sub_state |= DLC_LISTENING;
    station_ptr->station_sub_state &= ~DLC_CALLING;
  }
  station_ptr->station_sub_state &= ~DLC_CONTACTED;
  station_ptr->station_sub_state &= ~DLC_LOCAL_BUSY;
  station_ptr->station_sub_state &= ~DLC_REMOTE_BUSY;

  outputf("QLM_INITIALISE_STATION: station_sub_state = %x\n",
    station_ptr->station_sub_state);
  /***************************************************************************/
  /* The reason_for_closure field takes values of sna_operations_result_type */
  /* Initialise it to successful.                                            */
  /***************************************************************************/
  station_ptr->reason_for_closure = successful;
  /***************************************************************************/
  /* Silent Halt is a boolean field used to suppress STAH results when the   */
  /* station is halted in response to a Close Channel, or Disable SAP        */
  /* command.                                                                */
  /* As the Close/Disable_SAP procedures explicitly set the field to TRUE    */
  /* when they require the result to be suppressed, it is initialised to     */
  /* FALSE, thereby providing results unless otherwise indicated upon Halt   */
  /***************************************************************************/
  station_ptr->silent_halt = FALSE;

  /***************************************************************************/
  /* Initialise watchdog timers.                                             */
  /***************************************************************************/
  qlm_init_watchdog(
    &(station_ptr->link_station.repoll_dog),
    qlm_repoll_timeout,
    station_ptr->repoll_time
    );

  qlm_init_watchdog(
    &(station_ptr->inact_dog),
    qlm_inactivity_timeout,
    station_ptr->inact_time
    );

  qlm_init_watchdog(
    &(station_ptr->halt_dog),
    qlm_halt_timeout,
    station_ptr->force_halt_time
    );

  qlm_init_watchdog(
    &(station_ptr->retry_dog),
    qlm_retry_timeout,
    1                             /* hard coded to 1 second. Min res. of dog */
    );

  /***************************************************************************/
  /* Set listen_accepted_pdg_start_done to FALSE.                            */
  /***************************************************************************/
  station_ptr->listen_accepted_pdg_start_done = FALSE;
  /***************************************************************************/
  /* RAS Counters                                                            */
  /* Initialise all the RAS counters to zero.                                */
  /***************************************************************************/
  station_ptr->ras_counters.test_commands_sent = 0;
  station_ptr->ras_counters.test_command_failures = 0;
  station_ptr->ras_counters.test_commands_received = 0;
  station_ptr->ras_counters.inactivity_timeouts = 0;

  /***************************************************************************/
  /*                                                                         */
  /* These aren't RAS counters from the station, they are generated only     */
  /* when a query_ls is issued, because they come up from the DH.            */
  /*                                                                         */
  /* station_ptr->ras_counters.seq_data_packets_transmitted = 0;             */
  /* station_ptr->ras_counters.seq_data_packets_received = 0;                */
  /* station_ptr->ras_counters.invalid_packets_received = 0;                 */
  /* station_ptr->ras_counters.adapter_detected_rx_errors = 0;               */
  /* station_ptr->ras_counters.adapter_detected_tx_errors = 0;               */
  /* station_ptr->ras_counters.receive_inactivity_timeouts = 0;              */
  /* station_ptr->ras_counters.command_polls_sent = 0;                       */
  /* station_ptr->ras_counters.command_repolls_sent = 0;                     */
  /* station_ptr->ras_counters.command_contiguous_repolls = 0;               */
  /***************************************************************************/

  /***************************************************************************/
  /* Data is queued in the station on the receive_data_queue.                */
  /***************************************************************************/
  station_ptr->receive_data_queue.first = NULL;
  station_ptr->receive_data_queue.last = NULL;

  /***************************************************************************/
  /* There is also a retry_indicators structure in the station, to indicate  */
  /* whether there are currently any retries pending.                        */
  /***************************************************************************/
  station_ptr->retry_indicators.normal_data_retry_pending = FALSE;
  station_ptr->retry_indicators.xid_data_retry_pending = FALSE;
  station_ptr->retry_indicators.netd_data_retry_pending = FALSE;
}


/*****************************************************************************/
/* Function     QLM_HALT_LS                                                  */
/*                                                                           */
/* Description  This procedure gets called when the user issues a Halt       */
/*              ioctl and also when they issue a Close (Channel) or a        */
/*              Disable_SAP, which both require the halting of one or more   */
/*              link stations.                                               */
/*              On successful completion this procedure will issue a halt    */
/*              to the DH, but the station will stay in the list until the   */
/*              halt done status block is received.                          */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*    channel_id_type channel_id;                                            */
/*                                                                           */
/*    halt_ls_ioctl_ext_type *ext                                            */
/*                                                                           */
/*****************************************************************************/
qlm_rc_type  qlm_halt_ls(

  correlator_type qllc_ls_correlator,
  boolean silent,
  boolean send_disc)
{
  struct station_type *station_ptr;
  qvm_rc_type qvm_rc = qvm_rc_ok;
  qllc_rc_type qllc_rc;
  bool unlock;

  /***************************************************************************/
  /* Find and lock the station.                                              */
  /***************************************************************************/
  station_ptr = qlm_find_ls_given_correlator(qllc_ls_correlator,&unlock);
  outputf("QLM_HALT_LS: station ptr = %d\n", station_ptr);
  if (station_ptr == (struct station_type *)NULL)
  {
    outputf("QLM_HALT_LS: station ptr is NULL\n");
    return(qlm_rc_station_not_found);
  }

  /***************************************************************************/
  /* Note whether the caller wants a result or not, and update station field */
  /***************************************************************************/
  outputf("QLM_HALT_LS: station has just been locked, check if silent\n");
  if (silent == TRUE)
  {
    station_ptr->silent_halt = TRUE;
    qlm_delete_station(station_ptr,NULL,NULL);
    return(qlm_rc_ok);
  }

  /***************************************************************************/
  /* The close procedure must complete within Force_Halt_Time, so a timer is */
  /* set which will call the qlm_halt_timeout function when it expires.      */
  /* This timer is not stopped. If the station successfully halts within the */
  /* allotted time, then the halt timeout function will be called and there  */
  /* will be no station left. The function is capable of detecting this and  */
  /* does not do anything further.                                           */
  /* This is considered to be the easiest way, if not the most efficient, to */
  /* manage the halt timer.                                                  */
  /***************************************************************************/
  w_start(&(station_ptr->halt_dog));

  /***************************************************************************/
  /* Check whether the station is in opening state. If it is, then we are in */
  /* one of two conditions:                                                  */
  /*                                                                         */
  /* Either the station is a listener, and can be halted "immediately"....   */
  /*   i.e. issue a close to vc and that will in turn issue a halt to dh     */
  /*                                                                         */
  /* Or the station is calling, a start has been issued but the start        */
  /* done status block has not yet been received from the DH.                */
  /*                                                                         */
  /* Whether the station is calling or listening, the state is changed from  */
  /* opening to closing, so when the status block is received, the check of  */
  /* whether the state is opening will fail and the station will be halted.  */
  /*                                                                         */
  /***************************************************************************/
  if (station_ptr->station_state == opening)
  {
    outputf("QLM_HALT_LS: station state is opening\n");

    /*************************************************************************/
    /* Defect 103642 - If the station is either a calling or a listening     */
    /* station and it is in opening state, allow it to do an immediate halt. */
    /* Remove differentiation between calling and listening station.         */
    /*************************************************************************/

    qvm_rc = qvm_close_vc(
        &(station_ptr->virt_circuit),
        QCM_RETURN_PORT_ID(station_ptr->channel_id),
        NULL, /* diagnostic */
        FALSE
        );

    if (qvm_rc != qvm_rc_ok)
    {
        outputf("QLM_HALT_LS: qvm_close_vc failed\n");
        station_ptr->reason_for_closure = sna_system_error;
        qlm_delete_station(station_ptr,NULL,NULL);
        return(qlm_rc_close_vc_failed);
    }
    else
    {
        outputf("QLM_HALT_LS: qvm_close_vc successful\n");
        station_ptr->station_state = closing;
        station_ptr->reason_for_closure = successful;
        if (unlock) unlockl(&(station_ptr->lock));
        return(qlm_rc_ok);
    }
    /*************************************************************************/
    /* End of defect 103642.						   */
    /*************************************************************************/

  }

  else if (station_ptr->station_state == opened 
        || station_ptr->station_state == inactive)
  {
    outputf("QLM_HALT_LS: station state = opened or inactive\n");
    /*************************************************************************/
    /* If the station state is opened or inactive, then the start done status*/
    /* block has been received.                                              */
    /* In this case the vc must be closed and a halt done status block       */
    /* awaited.                                                              */
    /*************************************************************************/
    if (send_disc)
    {
      qllc_rc = qllc_lstop(
	&(station_ptr->link_station),
	&(station_ptr->virt_circuit),
	station_ptr->channel_id->port_id);
      if (qllc_rc != qrc_ok)
      {
	outputf("QLM_HALT_LS: qllc_lstop() failed\n");
        station_ptr->reason_for_closure = sna_system_error;
        qlm_delete_station(station_ptr,NULL,NULL);
        return(qlm_rc_close_vc_failed);
      }

      outputf("QLM_HALT_LS: qllc_lstop() successful\n");
    }
    else
    {
      qvm_rc = qvm_close_vc(
        &(station_ptr->virt_circuit),
        QCM_RETURN_PORT_ID (station_ptr->channel_id),
        NORMAL_TERMINATION,
        FALSE
        );
      if (qvm_rc != qvm_rc_ok)
      {
        outputf("QLM_HALT_LS: qvm_close_vc() failed\n");
        station_ptr->reason_for_closure = sna_system_error;
        qlm_delete_station(station_ptr,NULL,NULL);
        return(qlm_rc_close_vc_failed);
      }

      outputf("QLM_HALT_LS: qvm_close_vc() successful\n");
    }

    station_ptr->station_state = closing;
    station_ptr->reason_for_closure = successful;
    if (unlock) unlockl(&(station_ptr->lock));
    return(qlm_rc_ok);
   
  }
  else
  {
    /*************************************************************************/
    /* Only state not catered for already is closing, which is OK because the*/
    /* station will close anyway.                                            */
    /* If the station is in closing/closed state the ioctl is ignored        */
    /*************************************************************************/
    if (unlock) unlockl(&(station_ptr->lock));
    return(qlm_rc_ok);
  }
}

/*****************************************************************************/
/* Function     QLM_QUERY_LS                                                 */
/*                                                                           */
/* Description  This procedure queries the statistics relevant to a link     */
/*              station. It is synchronous, and fills in the stats block     */
/*              provided by the user.                                        */
/*              It is called by the QDH.                                     */
/*                                                                           */
/* Return       qlm_rc_station_not_found                                     */
/*              qlm_rc_ok                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*     ext_ptr                                                               */
/*                                                                           */
/*****************************************************************************/
qlm_rc_type qlm_query_ls(

  struct dlc_qls_arg *ext_ptr)
{
  station_type *station_ptr;
  x25_vc_type  *vc_ptr;
  qllc_ls_type *ls_ptr;
  bool          unlock;

  outputf("QLM_QUERY: has been called\n");
  station_ptr = qlm_find_ls_given_correlator(ext_ptr->gdlc_ls_corr, &unlock);
  if (station_ptr == NULL)
  {
    outputf("QLM_QUERY: station not found\n");
    return(qlm_rc_station_not_found);
  }
  outputf("QLM_QUERY: station locked\n");
  vc_ptr = &(station_ptr->virt_circuit);
  ls_ptr = &(station_ptr->link_station);
  /***************************************************************************/
  /* The station has been found. Initialise the statistics block and return  */
  /* it to the caller.                                                       */
  /***************************************************************************/
  outputf("QLM_QUERY: initialising stats block\n");
  ext_ptr->user_sap_corr =
    QSM_RETURN_USER_SAP_CORRELATOR(station_ptr->qllc_sap_correlator);
  ext_ptr->user_ls_corr = station_ptr->user_ls_correlator;
  strncpy(ext_ptr->ls_diag,station_ptr->station_tag,DLC_MAX_DIAG);
  ext_ptr->ls_state = station_ptr->station_state;
  ext_ptr->ls_sub_state = station_ptr->station_sub_state;
  ext_ptr->counters.test_cmds_sent = 
    station_ptr->ras_counters.test_commands_sent;
  ext_ptr->counters.test_cmds_fail = 
    station_ptr->ras_counters.test_command_failures;
  ext_ptr->counters.test_cmds_rec = 
    station_ptr->ras_counters.test_commands_received;
  ext_ptr->counters.data_pkt_sent = 
    vc_ptr->data_packets_tx;
  ext_ptr->counters.data_pkt_resent = 0;
  ext_ptr->counters.max_cont_resent = 0;
  ext_ptr->counters.data_pkt_rec = 
    vc_ptr->data_packets_rx;
  ext_ptr->counters.inv_pkt_rec = 
    vc_ptr->invalid_packets_rx;
  ext_ptr->counters.adp_rec_err =
    vc_ptr->adapter_rx_errors;
  ext_ptr->counters.adp_send_err = 
    vc_ptr->adapter_tx_errors;
  ext_ptr->counters.rec_inact_to =
    station_ptr->ras_counters.inactivity_timeouts;
  ext_ptr->counters.cmd_polls_sent = ls_ptr->command_polls_sent;
  ext_ptr->counters.cmd_repolls_sent = ls_ptr->command_repolls_sent;
  ext_ptr->counters.cmd_cont_repolls = ls_ptr->command_contiguous_repolls;
  ext_ptr->protodd_len = 0;
  if (unlock) unlockl(&(station_ptr->lock));
  outputf("QLM_QUERY: returning\n");
  return(qlm_rc_ok);
}

/*****************************************************************************/
/* Function     QLM_TRACE                                                    */
/*                                                                           */
/* Description  This procedure changes the settings of the station           */
/*              trace parameters, upon a Trace Ioctl being issued by         */
/*              the user.                                                    */
/*              It is called by the QDH.                                     */
/*                                                                           */
/* Return       qlm_rc_station_not_found                                     */
/*              qlm_rc_ok                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*     ext_ptr                                                               */
/*                                                                           */
/*****************************************************************************/
qlm_rc_type qlm_trace(

  struct dlc_trace_arg  *ext_ptr)
{
  correlator_type correlator;
  station_type   *station_ptr;
  qlm_rc_type     rc;
  bool            unlock;

  outputf("QLM_TRACE: has been called\n");
  correlator = ext_ptr->gdlc_ls_corr;
  station_ptr = qlm_find_ls_given_correlator(correlator,&unlock);

  if ( station_ptr == (station_type *)NULL )
  {
    outputf("QLM_TRACE: station not found\n");
    rc = qlm_rc_station_not_found;
  }
  else
  {
    outputf("QLM_TRACE: station found and locked\n");
    /*************************************************************************/
    /* update station's trace fields from parameter block                    */
    /*************************************************************************/
    (ext_ptr->flags & DLC_TRCO) ?
      (station_ptr->flags |= DLC_TRCO) : (station_ptr->flags &= ~DLC_TRCO);

    (ext_ptr->flags & DLC_TRCL) ? 
      (station_ptr->flags |= DLC_TRCL) : (station_ptr->flags &= ~DLC_TRCL); 

    if (ext_ptr->flags & DLC_TRCO)
      outputf("QLM_TRACE: turning trace ON\n");
    else
      outputf("QLM_TRACE: turning trace OFF\n");
    if (ext_ptr->flags & DLC_TRCL)
      outputf("QLM_TRACE: setting trace length to LONG\n");
    else
      outputf("QLM_TRACE: setting trace length to SHORT\n");

    station_ptr->trace_channel = ext_ptr->trace_chan;
    rc = qlm_rc_ok;
    if (unlock) unlockl(&(station_ptr->lock));
  }
  outputf("QLM_TRACE: returning %d\n",rc);
  return(rc);
}

/*****************************************************************************/
/* Function     QLM_TEST                                                     */
/*                                                                           */
/* Description  This procedure is invoked when the user issues a             */
/*              Test Ioctl. It is called by the QDH, checks the              */
/*              station state, and calls the QFSM, to send test data         */
/*              if appropriate.                                              */
/*              Note that q packets can be linked by the m-bit, so           */
/*              there is no constraint to use only one packet. The           */
/*              test string is of fixed length, and so an mbuf of            */
/*              sufficient is requested, and then the test data is copied    */
/*              into it.                                                     */
/*                                                                           */
/* Return       qlm_rc_station_not_found                                     */
/*              qlm_rc_protocol_error                                        */
/*              qlm_rc_ok                                                    */
/*                                                                           */
/* Parameters                                                                */
/*              ext_ptr                                                      */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
qlm_rc_type qlm_test(

  struct dlc_corr_arg *ext_ptr)
{
  qlm_rc_type            rc;
  correlator_type        correlator;
  station_address_type   station_ptr;
  gen_buffer_type       *buffer_ptr;
  int                    i;
  char                   test_data[TEST_STRING_LENGTH];
  bool                   unlock;

  correlator = ext_ptr->gdlc_ls_corr;
  station_ptr = qlm_find_ls_given_correlator(correlator,&unlock);
  if (station_ptr == NULL)
  {
    outputf ("QLM_TEST Station could not be found. \n");
    rc = qlm_rc_station_not_found;
    return (rc);
  }
  else
  {
    buffer_ptr = QBM_GET_BUFFER(
      OFFSETOF(body.qllc_body.user_data[0],x25_mbuf_t)+TEST_STRING_LENGTH);

    outputf("QLM_TEST: buffer_length = %d\n",
      OFFSETOF(body.qllc_body.user_data[0],x25_mbuf_t)+TEST_STRING_LENGTH);
    outputf("QLM_TEST: TEST_STRING_LENGTH = %d\n",TEST_STRING_LENGTH);

    if (buffer_ptr == NULL)
    {
      outputf ("QLM_TEST: Could not get buffer. \n");
      rc = qlm_rc_buffer_unavailable;
    }
    else
    {
      /***********************************************************************/
      /*  Initialise test string into buffer, allowing two bytes for QLLC    */
      /*  header (address and control fields).                               */
      /*  Test data is simply the digits 0 through 255.                      */
      /***********************************************************************/
      outputf("QLM_TEST: initialising test data\n");
      for (i=0; i<TEST_STRING_LENGTH; i++)
      {
	outputf("%x",i);
	test_data[i]= i;
      }
      outputf("\n");

      JSMBUF_SET_BLOCK(
	buffer_ptr,
	OFFSETOF(body.qllc_body.user_data[0], x25_mbuf_t),
	test_data,
	TEST_STRING_LENGTH
	);
      /***********************************************************************/
      /* Buffer is initialised and ready to be sent                          */
      /***********************************************************************/
      outputf("QLM_TEST Buffer initialised and ready to send.\n");

      if ( (station_ptr->station_state != (byte)opened)
	|| (station_ptr->station_sub_state & DLC_CONTACTED) == DLC_CONTACTED)
      {
	outputf ("station is in CONTACTED state, or station is not OPENED\n");
	rc = qlm_rc_protocol_error;
	outputf("QLM_TEST: protocol error\n");
/* defect 154624 */
	QBM_FREE_BUFFER(buffer_ptr);
/* end defect 154624 */
      }
      else if (qllc_ltest(
	&(station_ptr->link_station),
	&(station_ptr->virt_circuit),
	station_ptr->channel_id->port_id,
	buffer_ptr
	) != qrc_ok)
      {
	outputf ("QLM_TEST: qllc_ltest failed\n");
	rc = qlm_rc_protocol_error;
	outputf("QLM_TEST: protocol error\n");
/* defect 154624 */
	QBM_FREE_BUFFER(buffer_ptr);
/* end defect 154624 */
      }
      else
      {
	INC_RAS_COUNTER(station_ptr->ras_counters.test_commands_sent);
	outputf ("QLM_TEST test_link went ok\n");
	rc = qlm_rc_ok;
      }
    }
    if (unlock) unlockl(&(station_ptr->lock));
    outputf ("QLM_TEST: returning %d\n",rc);
    return(rc);
  }
}

/*****************************************************************************/
/* Function     QLM_ALTER                                                    */
/*                                                                           */
/* Description  This procedure is invoked when the user issues an            */
/*              Alter Ioctl. It is called by the QDH, and updates            */
/*              the station state, and calls the QFSM.                       */
/*                                                                           */
/* Return       qlm_rc_station_not_found                                     */
/*              qlm_rc_protocol_error                                        */
/*              qlm_rc_ok                                                    */
/*                                                                           */
/* Parameters                                                                */
/*              ext_ptr                                                      */
/*                                                                           */
/*****************************************************************************/
qlm_rc_type qlm_alter(
 
  struct dlc_alter_arg *ext_ptr)
{
 
  qlm_rc_type           rc;
  correlator_type       correlator;
  station_type         *station_ptr;
  bool                  inact_timer_running = FALSE;
  bool                  force_timer_running = FALSE;
  bool                  unlock;
 
  outputf("QLM_ALTER: has been called\n");
  outputf("QLM_ALTER: value of flags parameter = %x\n",ext_ptr->flags);
  /***************************************************************************/
  /* Find the station                                                        */
  /***************************************************************************/
  correlator = ext_ptr->gdlc_ls_corr;
  station_ptr = qlm_find_ls_given_correlator(correlator,&unlock);
  if ( station_ptr == (station_type *)NULL )
  {
    outputf("QLM_ALTER: station not found\n");
    rc = qlm_rc_station_not_found;
    return (rc);
  }
  else
  {
    outputf("QLM_ALTER: station found and locked\n");
    /*************************************************************************/
    /* Initialise the result indicators to zero. The relevant bits are later */
    /* set according to the result that is achieved.                         */
    /*************************************************************************/
    ext_ptr->result_flags = 0;
 
    /*************************************************************************/
    /* Alter Repoll Timeout if requested.                                    */
    /* There is no error checking on next function call.                     */
    /* The QLLC function only deals with the actual link_station itself.     */
    /* The repoll time held in the station config in the station record      */
    /* is only of interest on link startup, and can be ignored               */
    /* from then on. It is updated explicitly here in case you               */
    /* want to look at it in the debugger, or if any future calls            */
    /* reference the station_cfg value by mistake.                           */
    /*************************************************************************/
    outputf("QLM_ALTER: calling qllc_alter_repoll_timeout\n");
    qllc_alter_repoll_timeout(
      &(station_ptr->link_station),
      ((ext_ptr->flags & DLC_ALT_RTO) ? TRUE : FALSE),
      ext_ptr->repoll_time
      );
    station_ptr->repoll_time = ext_ptr->repoll_time;
    /*************************************************************************/
    /* Alter Inactivity Timeout if requested.                                */
    /*************************************************************************/
    if ((ext_ptr->flags & DLC_ALT_ITO) == DLC_ALT_ITO)
    {
      outputf("QLM_ALTER: altering inact time to %d\n",ext_ptr->inact_time);
      if (station_ptr->inact_dog.count != 0)
      {
	/*********************************************************************/
	/* timer is running. it will be restarted when modified.             */
	/*********************************************************************/
	inact_timer_running = TRUE;
      }
      w_stop(&(station_ptr->inact_dog));
/* defect 111172 */
      while (w_clear(&(station_ptr->inact_dog)));
      station_ptr->inact_dog.restart = ext_ptr->inact_time;
      while (w_init(&(station_ptr->inact_dog)));
/* end defect 111172 */
      station_ptr->inact_time = ext_ptr->inact_time;
      if (inact_timer_running == TRUE)
      {
	inact_timer_running = FALSE;
	w_start(&(station_ptr->inact_dog));
      }
    }
    /*************************************************************************/
    /* Alter Force Halt Timeout if requested.                                */
    /*************************************************************************/
    if ((ext_ptr->flags & DLC_ALT_FHT) == DLC_ALT_FHT)
    {
      if (station_ptr->halt_dog.count != 0)
      {
	/*********************************************************************/
	/* timer is running. it will be restarted when modified.             */
	/*********************************************************************/
	force_timer_running = TRUE;
      }
      outputf("QLM_ALTER: alter force halt time to %d\n",ext_ptr->force_time);
      w_stop(&(station_ptr->halt_dog));
/* defect 111172 */
      while (w_clear(&(station_ptr->halt_dog)));
      station_ptr->halt_dog.restart = ext_ptr->force_time;
      while (w_init(&(station_ptr->halt_dog)));
/* end defect 111172 */
      if (force_timer_running == TRUE)
      {
	force_timer_running = FALSE;
	w_start(&(station_ptr->halt_dog));
      }
    }
    /*************************************************************************/
    /* Alter Max I Field if requested.                                       */
    /*************************************************************************/
    if ((ext_ptr->flags & DLC_ALT_MIF) == DLC_ALT_MIF)
    {
      outputf("QLM_ALTER: altering max i field to %d\n",ext_ptr->maxif);
      station_ptr->max_i_field = ext_ptr->maxif;
    }
    /*************************************************************************/
    /* Alter Max Repoll Count.                                               */
    /*************************************************************************/
    if ((ext_ptr->flags & DLC_ALT_MXR) == DLC_ALT_MXR)
    {
      outputf("QLM_ALTER: alter max repoll count to %d\n",ext_ptr->max_repoll);
      station_ptr->max_repoll = ext_ptr->max_repoll;
      qllc_alter_max_repoll(
	&(station_ptr->link_station),
	ext_ptr->max_repoll
	);
    }
    /*************************************************************************/
    /* The alter_mode_1/2 bits have the following meanings:                  */
    /*                                                                       */
    /*    alter_mode_1            alter_mode_2           meaning             */
    /*    ------------            ------------           -------             */
    /*         FALSE                FALSE                don't alter         */
    /*         FALSE                TRUE                 set secondary       */
    /*         TRUE                 FALSE                set primary         */
    /*         TRUE                 TRUE                 invalid             */
    /*                                                                       */
    /*************************************************************************/
    outputf("QLM_ALTER: alter flags are as follows:\n");
    outputf("QLM_ALTER:   SM1 = %x\n",ext_ptr->flags & DLC_ALT_SM1);
    outputf("QLM_ALTER:   SM2 = %x\n",ext_ptr->flags & DLC_ALT_SM2);
 
    if ((ext_ptr->flags & DLC_ALT_SM1) == DLC_ALT_SM1)
    {
      if ((ext_ptr->flags & DLC_ALT_SM2) == DLC_ALT_SM2)
      {
	outputf("QLM_ALTER: invalid set mode request\n");
	rc = qlm_rc_invalid_set_mode_request;
	if (unlock) unlockl(&(station_ptr->lock));
	return (rc);
      }
      else
      {
	/*********************************************************************/
	/* The alter command contains a request to set the station role to   */
	/* primary. If this is successful, the result indiactor is set to    */
	/* mode_set_primary. Since the station is now primary, the inactivity*/
	/* timer is not needed, and so it is stopped by calling w_stop.      */
	/* It is likely that the station state wil be inactive, so this must */
	/* be set to opened.                                                 */
	/* If we were unable to change the role, the result indicator is set */
	/* to mode_set_primary_failed.                                       */
	/*********************************************************************/
	outputf("QLM_ALTER: set mode to primary request\n");
	if (qllc_alter_role(
	  &(station_ptr->link_station),
	  qr_primary
	  ) == qrc_ok)
	{
	  outputf("QLM_ALTER: qllc_alter_role call successful\n");
	  ext_ptr->result_flags |= DLC_MSP_RES;
	  w_stop(&(station_ptr->inact_dog));
	  station_ptr->station_state = (byte)opened;
	}
	else
	{
	  outputf("QLM_ALTER: qllc_alter_role call unsuccessful\n");
	  /*******************************************************************/
	  /* mode set primary failed                                         */
	  /*******************************************************************/
	  ext_ptr->result_flags |= DLC_MSPF_RES;
	}
      }
    }
    else
    {
      /***********************************************************************/
      /* The alter_mode_1 bit is known to be FALSE                           */
      /***********************************************************************/
      if ((ext_ptr->flags & DLC_ALT_SM2) == DLC_ALT_SM2)
      {
	/*********************************************************************/
	/* The alter command contains a request to set the station role to   */
	/* secondary. If this is successful, the result indicator is set to  */
	/* mode_set_secondary. Since the station is now primary, the inact   */
	/* timer must be started, by calling timeout() with the value        */
	/* of inactivity timer in the config area.                           */
	/* If we were unable to change the role, the result indicator is set */
	/* to mode_set_secondary_failed.                                     */
	/*********************************************************************/
	outputf("QLM_ALTER: set mode secondary request\n");
	if (qllc_alter_role(
	  &(station_ptr->link_station),
	  qr_secondary
	  ) == qrc_ok)
	{
	  outputf("QLM_ALTER: qllc_alter_role call successful\n");
	  ext_ptr->result_flags |= DLC_MSS_RES;
	  w_start(&(station_ptr->inact_dog));
	}
	else
	{
	  outputf("QLM_ALTER: qllc_alter_role call unsuccessful\n");
	  /*******************************************************************/
	  /* mode_set_secondary failed                                       */
	  /*******************************************************************/
	  ext_ptr->result_flags |= DLC_MSSF_RES;
	}
      }
    }
 
    /*************************************************************************/
    /* The alter_inactivity_1/2 bits alter the inactivity timeout mode.      */
    /* The alter_inactivity bits have the following meaning:                 */
    /*                                                                       */
    /*    alter_inactivity_1    alter_inactivity_2         meaning           */
    /*    ------------------    ------------------       -----------         */
    /*         FALSE                FALSE                don't alter         */
    /*         FALSE                TRUE                 notification        */
    /*         TRUE                 FALSE                auto halt           */
    /*         TRUE                 TRUE                 invalid             */
    /*************************************************************************/
    outputf("QLM_ALTER: change the inactivity action\n");
    outputf("QLM_ALTER:   IT1 = %x\n",ext_ptr->flags & DLC_ALT_IT1);
    outputf("QLM_ALTER:   IT2 = %x\n",ext_ptr->flags & DLC_ALT_IT2);
 
    if ((ext_ptr->flags & DLC_ALT_IT1) == DLC_ALT_IT1)
    {
      if ((ext_ptr->flags & DLC_ALT_IT2) == DLC_ALT_IT2)
      {
	outputf("QLM_ALTER: invalid alter inact action request\n");
	rc = qlm_rc_invalid_inact_bits;
	if (unlock) unlockl(&(station_ptr->lock));
	return (rc);
      }
      else
      {
	outputf("QLM_ALTER: altering inact action to NO HOLD\n");
	station_ptr->flags &= !DLC_SLS_HOLD;
      }
    }
    else
    {
      /***********************************************************************/
      /* alter_inactivity_1 is known to be FALSE                             */
      /***********************************************************************/
      if ((ext_ptr->flags & DLC_ALT_IT2) == DLC_ALT_IT2)
      {
	outputf("QLM_ALTER: altering inact action to HOLD\n");
	station_ptr->flags |= DLC_SLS_HOLD;
      }
    }
    if (unlock) unlockl(&(station_ptr->lock));
    outputf("QLM_ALTER: returning OK\n");
    return(qlm_rc_ok);
  }
}


/*****************************************************************************/
/* Function     QLM_CONTACT                                                  */
/*                                                                           */
/* Description  This procedure is invoked when the user issues a             */
/*              Contact Ioctl. It is called by the QDH, and updates          */
/*              the station state, and calls the QFSM.                       */
/*                                                                           */
/* Return       qlm_rc_station_not_found                                     */
/*              qlm_rc_protocol_error                                        */
/*              qlm_rc_ok                                                    */
/*                                                                           */
/* Parameters                                                                */
/*              ext_ptr                                                      */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
qlm_rc_type qlm_contact(

  struct dlc_corr_arg  *ext_ptr)
{
  qlm_rc_type       rc;
  correlator_type   correlator;
  station_type     *station_ptr;
  port_type        *port_ptr;
  bool              unlock;

  outputf("QLM_CONTACT: has been called\n");
  outputf("..dlc corr arg block contains:\n");
  outputf("QLLC SAP Correlator=%x\n",ext_ptr->gdlc_sap_corr);
  outputf("QLLC LS Correlator=%x\n",ext_ptr->gdlc_ls_corr);
  correlator = ext_ptr->gdlc_ls_corr;
  station_ptr = qlm_find_ls_given_correlator(correlator,&unlock);
  outputf("Station Address after util interrogation =%x\n",station_ptr);
  /***************************************************************************/
  /* Check whether the station exists. If so, issue a Link Start call to the */
  /* QLLC FSM. If that goes wrong then advise the user, but if it works      */
  /* alright just return OK. The contacted result will be issued later.      */
  /***************************************************************************/
  if (station_ptr == NULL)
  {
    outputf("QLM_CONTACT: station not found\n");
    rc = qlm_rc_station_not_found;
    return (rc);
  }
  outputf("QLM_CONTACT: station locked\n");

  port_ptr = QCM_RETURN_PORT_ID(station_ptr->channel_id);
  /***************************************************************************/
  /* Check states.                                                           */
  /***************************************************************************/
  if ( station_ptr->station_state != (byte)opened
    || (station_ptr->station_sub_state & DLC_CONTACTED) == DLC_CONTACTED)
  {
    outputf("QLM_CONTACT: invalid state for qllc_lstrt\n");
    rc = qlm_rc_protocol_error;
  }
  else if ( qllc_lstrt(
    &(station_ptr->link_station),
    &(station_ptr->virt_circuit),
    port_ptr
    ) != qrc_ok)
  {
    outputf("QLM_CONTACT: qllc_lstrt failed\n");
    rc = qlm_rc_protocol_error;
  }
  else
  {
    outputf("QLM_CONTACT: qllc_lstrt succeeded\n");
    rc = qlm_rc_ok;
  }
  if (unlock) unlockl(&(station_ptr->lock));
  outputf("QLM_CONTACT: returning %d\n",rc);
  return(rc);
}


/*****************************************************************************/
/* Function     QLM_ENTER_LOCAL_BUSY                                         */
/*                                                                           */
/* Description  This procedure is invoked when the user issues an Enter      */
/*              Local Busy Ioctl. It is called by the QDH.                   */
/*                                                                           */
/* Return       qlm_rc_type  rc;                                             */
/*                                                                           */
/* Parameters   enter_local_busy_ioctl_ext_type *ext;                        */
/*****************************************************************************/
qlm_rc_type qlm_enter_local_busy(

  struct dlc_corr_arg *ext)
{
  station_type *station_ptr;
  qpm_rc_type   qpm_rc = qpm_rc_ok;
  bool          unlock;

  /***************************************************************************/
  /* Find Station                                                            */
  /***************************************************************************/
  station_ptr = qlm_find_ls_given_correlator(ext->gdlc_ls_corr,&unlock);
  if (station_ptr == (station_type *)NULL)
  {
    outputf("QLM_ENTER_LBUSY: station not found\n");
    return(qlm_rc_station_not_found);
  }
  /***************************************************************************/
  /* If the station is not contacted then it cannot be put into local busy   */
  /* mode                                                                    */
  /***************************************************************************/
  if ((station_ptr->station_sub_state & DLC_CONTACTED) != DLC_CONTACTED)
  {
    outputf("QLM_ENTER_LBUSY: station not contacted\n");
    if (unlock) unlockl(&(station_ptr->lock));
    return (qlm_rc_station_not_contacted);
  }
  /***************************************************************************/
  /* If the station is contacted, but is already in local busy mode, then an */
  /* error is logged                                                         */
  /***************************************************************************/
  if ((station_ptr->station_sub_state & DLC_LOCAL_BUSY) == DLC_LOCAL_BUSY)
  {
    outputf("QLM_ENTER_LBUSY: station contacted but already busy\n");
    if (unlock) unlockl(&(station_ptr->lock));
    return (qlm_rc_station_already_local_busy);
  }
  /***************************************************************************/
  /* If the station is contacted, then it can be put into local busy mode.   */
  /***************************************************************************/
  outputf("QLM_ENTER_LBUSY: station entering busy mode\n");
  station_ptr->station_sub_state |= DLC_LOCAL_BUSY;
  qpm_rc = qpm_enter_local_busy(
    QCM_RETURN_PORT_ID(station_ptr->channel_id),
    QVM_RETURN_SESSION_ID(&(station_ptr->virt_circuit))
    );
  if (qpm_rc != qpm_rc_ok)
  {
    outputf("QLM_ENTER_BUSY: qpm_enter_busy failed\n");
    station_ptr->reason_for_closure = sna_system_error;
    qlm_delete_station(station_ptr,NULL,NULL);
    return(qlm_rc_system_error);
  }
  if (unlock) unlockl(&(station_ptr->lock));
  return (qlm_rc_ok);
}

/*****************************************************************************/
/* Function     QLM_EXIT_LOCAL_BUSY                                          */
/*                                                                           */
/* Description  This procedure is invoked when the user issues an            */
/*              Exit Local Busy Ioctl. It is called by the QDH.              */
/*                                                                           */
/* Return       qlm_rc_type rc;                                              */
/*                                                                           */
/* Parameters   exit_local_busy_ioctl_ext_type *ext;                         */
/*****************************************************************************/
qlm_rc_type  qlm_exit_local_busy(

  struct dlc_corr_arg *ext)
{
  station_type    *station_ptr;
  gen_buffer_type *buffer_ptr;
  qpm_rc_type      qpm_rc = qpm_rc_ok;
  bool             unlock;

  /***************************************************************************/
  /* Find Station                                                            */
  /***************************************************************************/
  station_ptr = qlm_find_ls_given_correlator(ext->gdlc_ls_corr, &unlock);
  if (station_ptr == (station_type *)NULL)
  {
    outputf("QLM_EXIT_LBUSY: station not found\n");
    return(qlm_rc_station_not_found);
  }

  /***************************************************************************/
  /* Check to see if the station is in busy mode                             */
  /***************************************************************************/
  if ((station_ptr->station_sub_state & DLC_LOCAL_BUSY) != DLC_LOCAL_BUSY)
  {
    outputf("QLM_EXIT_LBUSY: station not local busy\n");
    if (unlock) unlockl(&(station_ptr->lock));
    return (qlm_rc_station_not_local_busy);
  }
  /***************************************************************************/
  /* The following code attempts to bring the station out of busy mode.      */
  /* This involves sending any data that has been queued in the station,     */
  /* whilst in busy mode, to the channel manager. The channel mgr may manage */
  /* to send it to the user, or may decide to enter local busy mode once     */
  /* again, or force a dlc retry.                                            */
  /***************************************************************************/
  outputf("QLM_EXIT_LBUSY: station being brought out of busy\n");
  /***************************************************************************/
  /* QBM_DEQUE_BUFFER() gets the first buffer off the queue.                 */
  /***************************************************************************/
  buffer_ptr = QBM_DEQUE_BUFFER(&(station_ptr->receive_data_queue));
  /***************************************************************************/
  /* The loop below checks whether there was anything on the queue, and if   */
  /* so it passes it to the QCM. So long as the qcm does not report any      */
  /* error, another buffer is got from the queue, and so long as there's on  */
  /* there to get off, the loop repeats. If ever the queue runs out of       */
  /* buffers, or the qcm reports some failure, then the loop breaks out.     */
  /***************************************************************************/
  while ( buffer_ptr != (gen_buffer_type *)NULL
    && qcm_receive_data (station_ptr->channel_id,buffer_ptr) == qcm_rc_ok)
  {
    outputf("QLM_EXIT_LBUSY: getting another buffer off station queue\n");
    buffer_ptr = QBM_DEQUE_BUFFER(&(station_ptr->receive_data_queue));
  }
  /***************************************************************************/
  /* If it was the QCM giving a bad return code that stopped the loop then   */
  /* you must re-queue the buffer which was taken from the queue by the qbm. */
  /* You must also stay in local busy mode as you have not managed to send   */
  /* up all the data that was on the receive_q.                              */
  /***************************************************************************/
  if (buffer_ptr != NULL)
  {
    outputf("QLM_EXIT_LBUSY: channel gave bad rc to receive\n");
    QBM_REQUE_BUFFER(buffer_ptr, &(station_ptr->receive_data_queue));
    if (unlock) unlockl(&(station_ptr->lock));
    return(qlm_rc_ok);
    /*************************************************************************/
    /* This is a strange rc, but the user forced the busy so even though     */
    /* we're in exit func, the user should realise what's going on.          */
    /*************************************************************************/
  }
  else
  {
    outputf("QLM_EXIT_LBUSY: managed to pass up all the data\n");
    /*************************************************************************/
    /* The data has all been sent to the user. Reset the station state, and  */
    /* tell the DH that the busy condition has ended                         */
    /*************************************************************************/
    qpm_rc = qpm_exit_local_busy(
      QCM_RETURN_PORT_ID(station_ptr->channel_id),
      QVM_RETURN_SESSION_ID(&(station_ptr->virt_circuit))
      );
    if (qpm_rc != qpm_rc_ok)
    {
      outputf("QLM_ENTER_BUSY: qpm_exit_busy failed\n");
      station_ptr->reason_for_closure = sna_system_error;
      qlm_delete_station(station_ptr,NULL,NULL);
      return(qlm_rc_system_error);
    }
    station_ptr->station_sub_state &= ~DLC_LOCAL_BUSY;
    if (unlock) unlockl(&(station_ptr->lock));
    return(qlm_rc_ok);
  }
}

/*****************************************************************************/
/* Function     QLM_WRITE                                                    */
/*                                                                           */
/* Description  This procedure is invoked when the user issues a             */
/*              Write Command.                                               */
/*              Currently writes are not blocked. i.e. the DH provides       */
/*              the buffering needed to ensure it never returns EAGAIN.      */
/*              If EAGAIN, and the corresponding calls to tx_fn in the QPM   */
/*              are ever introduced, this function will need to look at      */
/*              uio_fmode to tell whether the write is synchronous or not.   */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Parameters   write_ext_type *ext;                                         */
/*              gen_buffer_type *buffer_ptr;                                 */
/*                                                                           */
/*****************************************************************************/
qlm_rc_type qlm_write(
  write_ext_type   *ext,
  gen_buffer_type  *buffer_ptr)
{
  qlm_rc_type          qlm_rc = qlm_rc_ok;
  struct station_type *station_ptr;
  qllc_rc_type         qrc;
  qvm_rc_type          vrc;
  x25_vc_type         *vc_ptr;
  port_type           *port_id;
  gen_buffer_type     *tbuf; 
  bool                 unlock;

  char                 temp_data[32];

  outputf("QLM_WRITE: called\n");

  outputf("QLM_WRITE: looking for %x\n",ext->qllc_ls_correlator);
  station_ptr = qlm_find_ls_given_correlator(ext->qllc_ls_correlator, &unlock);
  if (station_ptr == (station_type *)NULL)
  {
    outputf("QLM_WRITE: station not found\n");
    return(qlm_rc_station_not_found);
  }
  outputf("QLM_WRITE: station found\n");

  /***************************************************************************/
  /* Station exists, initialise local variables                              */
  /***************************************************************************/
  vc_ptr = &(station_ptr->virt_circuit);
  port_id = station_ptr->channel_id->port_id;

  /***************************************************************************/
  /* Test the data flags in the write extension                              */
  /***************************************************************************/
  if ((ext->write_flags & DLC_INFO) == DLC_INFO)
  {

    TRACE_DATA_XMIT(station_ptr, buffer_ptr);

    outputf("QLM_WRITE: write norm data\n");
    /*************************************************************************/
    /* To send normal data, the following steps are taken.                   */
    /*************************************************************************/
    outputf("QLM_WRITE: buffer_ptr = %x\n",buffer_ptr);
    outputf("QLM_WRITE: call qllc_sdata()\n");
    qrc = qllc_sdata(
      &(station_ptr->link_station),
      vc_ptr,
      port_id,
      buffer_ptr
      );
    outputf("QLM_WRITE: qllc_sdata() rc = %d\n",qrc);
    if (qrc != qrc_ok)
    {
      vrc = qvm_close_vc(
	vc_ptr,
	port_id,
	LOCAL_PROCEDURE_ERROR_GENERAL,
	FALSE
	);
      if (vrc != qvm_rc_ok)
      {
	outputf("QLM_WRITE: close vc failed\n");
	station_ptr->reason_for_closure = sna_system_error;
	qlm_delete_station(station_ptr,NULL,NULL);
	return(qlm_rc_close_vc_failed);
      }
      else
      {
	outputf("QLM_WRITE: close vc succeeded\n");
	if (qrc == qrc_x25_error)
	{
	  outputf("QLM_WRITE: qllc_sdata returned qrc_x25_error\n");
	  station_ptr->reason_for_closure = sna_system_error;
	  qlm_rc = qlm_rc_system_error;
	}
	else
	{
	  outputf("QLM_WRITE: qllc_sdata did not return qrc_x25_error\n");
	  station_ptr->reason_for_closure = protocol_error;
	  qlm_rc = qlm_rc_protocol_error;
	}
	station_ptr->station_state = closing;
      }
    }
    outputf("QLM_WRITE: unlock station and return\n");
    if (unlock) unlockl(&(station_ptr->lock));
    return(qlm_rc);
  }
  else if ((ext->write_flags & DLC_XIDD) == DLC_XIDD)
  {

    TRACE_XID_XMIT(station_ptr, buffer_ptr);

    outputf("QLM_WRITE: sending xid data\n");
    /*************************************************************************/
    /* To send xid data, the following steps are taken.                      */
    /*************************************************************************/
    qrc = qllc_xchid(
      &(station_ptr->link_station),
      vc_ptr,
      port_id,
      buffer_ptr
      );
    outputf("QLM_WRITE: qllc_xchid() rc = %d\n",qrc);
    if (qrc != qrc_ok)
    {
      vrc = qvm_close_vc(
	vc_ptr,
	port_id,
	LOCAL_PROCEDURE_ERROR_GENERAL,
	FALSE
	);
      if (vrc != qvm_rc_ok)
      {
	station_ptr->reason_for_closure = sna_system_error;
	qlm_delete_station(station_ptr,NULL,NULL);
	return(qlm_rc_close_vc_failed);
      }
      else
      {
	if (qrc == qrc_x25_error)
	{
	  station_ptr->reason_for_closure = sna_system_error;
	  qlm_rc = qlm_rc_system_error;
	}
	else
	{
	  station_ptr->reason_for_closure = protocol_error;
	  qlm_rc = qlm_rc_protocol_error;
	}
	station_ptr->station_state = closing;
      }
    }
    if (unlock) unlockl(&(station_ptr->lock));
    return(qlm_rc);
  }
  /***************************************************************************/
  /* If you get this far it's because the write flags are for an unsupported */
  /* type of data, e.g. netd or datagram. In this case, log an error and     */
  /* return                                                                  */
  /***************************************************************************/
  outputf("QLM_WRITE: write flags are unsupported\n");
  if (unlock) unlockl(&(station_ptr->lock));
  return(qlm_rc_user_interface_error);
}

