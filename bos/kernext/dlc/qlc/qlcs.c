static char sccsid[] = "@(#)81  1.9  src/bos/kernext/dlc/qlc/qlcs.c, sysxdlcq, bos411, 9428A410j 11/2/93 09:21:51";
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: qsm_enable_sap, qsm_disable_sap, qsm_query_sap, qsm_check_sap,
 *            qsm_find_sap_given_correlator
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

#include "qlcg.h"      /* for correlator_type, diag_tag_type, etc.           */
#include "qlcvfac.h"
#include "qlcv.h"
#include "qlcq.h"
#include "qlcb.h"
#include "qlcl.h"      /* for station_type, SILENT                           */
#include "qlcp.h"      /* for port_name_type                                 */
#include "qlcc.h"      /* for channel_type                                   */
#include "qlcs.h"

/*****************************************************************************/
/* Function     QSM_ENABLE_SAP                                               */
/*                                                                           */
/* Description  This procedure is called by the QDH when an Enable_SAP ioctl */
/*              is issued by the user. It accepts an configuration area,     */
/*              pointed to by the Ext_Ptr, and allocates storage for a SAP,  */
/*              initialises it, and links the new SAP into the list of SAPs  */
/*              enabled on the channel on which the ioctl was issued.        */
/*                                                                           */
/*              On the first sap to be enabled on a port the call to the     */
/*              qpm_init_address function will register the local address in */
/*              the port.                                                    */
/*                                                                           */
/*          On successful completion there will be a new SAP in the channel  */
/*          linked list of SAPs. The new SAP will be fully initialised and   */
/*          ready to have link stations started on it.                       */
/*                                                                           */
/*                                                                           */
/* Return   qsm_rc_type                                                      */
/*                                                                           */
/* Parameters channel_id_type      channel_id                                */
/*            struct dlc_esap_arg *ext_ptr                                   */
/*****************************************************************************/
qsm_rc_type qsm_enable_sap(

  channel_id_type channel_id,
  struct dlc_esap_arg *ext_ptr)
{
  qsm_rc_type rc = qsm_rc_ok;
  qcm_rc_type qcm_rc;
  sap_type *sap_ptr;
  sap_type *last_sap_ptr;
  int lock_rc;

  outputf("QSM_ENABLE_SAP: ...has been called\n");
  /***************************************************************************/
  /* Check how many SAPs are enabled on the channel                          */
  /* the sap is to be enabled on.                                            */
  /***************************************************************************/
  qcm_rc = qcm_check_channel(channel_id);
  if (qcm_rc != qcm_rc_ok)
  {
    switch (qcm_rc)
    {
    case qcm_rc_channel_not_found:
      rc = qsm_rc_system_error;
      break;
    case qcm_rc_sap_limit_reached:
      rc = qsm_rc_sap_limit_reached;
      break;
    default:
      rc = qsm_rc_system_error;
      break;
    }
    return(rc);
  }
  /***************************************************************************/
  /* The channel can accommodate another SAP. Check the fields in the config */
  /* extension passed to the ioctl.                                          */
  /***************************************************************************/
  if ( ext_ptr->max_ls > 255 || ext_ptr->max_ls < 1 )
  {
    outputf("QSM_ENABLE_SAP: invalid max_ls = %d\n",ext_ptr->max_ls);
    rc = qsm_rc_max_link_stations_invalid;
    return(rc);
  }
  /***************************************************************************/
  /* SAP can be alloc'd and enabled                                          */
  /***************************************************************************/
  sap_ptr = (sap_type *)xmalloc(sizeof(sap_type),WORD,pinned_heap);
  if (sap_ptr == (sap_type *)NULL)
  {
    /*************************************************************************/
    /* malloc failed                                                         */
    /*************************************************************************/
    rc = qsm_rc_alloc_failed;
    return (rc);
  }
  /***************************************************************************/
  /* malloc was successful                                                   */
  /***************************************************************************/
  outputf("QSM_ENABLE_SAP: ...sap has been allocated.\n");
  sap_ptr->lock = LOCK_AVAIL;
  lockl(&(sap_ptr->lock), LOCK_SHORT);
  /***************************************************************************/
  /* copy returned qllc_sap_correlator into extension block                  */
  /***************************************************************************/
  ext_ptr->gdlc_sap_corr = (correlator_type)sap_ptr;
  /***************************************************************************/
  /* copy config area into sap record (including new correlator)             */
  /***************************************************************************/
  outputf("QSM_ENABLE_SAP: new sap at %d\n",sap_ptr);
  outputf("QSM_ENABLE_SAP: begin initialisation\n");
  sap_ptr->qllc_sap_correlator = (correlator_type)sap_ptr;
  sap_ptr->user_sap_correlator = ext_ptr->user_sap_corr;
  /***************************************************************************/
  /* There is no default value for max_link_stations. The user provides a    */
  /* valid value. The range is checked in one of the above partitions.       */
  /***************************************************************************/
  sap_ptr->max_link_stations = ext_ptr->max_ls;
  bzero (sap_ptr->local_x25_address,DLC_MAX_NAME);
  strcpy(sap_ptr->local_x25_address,ext_ptr->laddr_name);
  /***************************************************************************/
  /* Ensure that local address is NULL terinated */
  /***************************************************************************/
  sap_ptr->local_x25_address[ext_ptr->len_laddr_name] = '\0';
  /***************************************************************************/
  /* Notify port of the local address                                        */
  /***************************************************************************/
  qpm_init_address(
    QCM_RETURN_PORT_ID(channel_id),
    sap_ptr->local_x25_address
    );
  /***************************************************************************/
  /* Initialise other SAP fields not initialised by config copy, except the  */
  /* linked list fields, next_sap_ptr and prev_sap_ptr, which will be        */
  /* initialised when the linked list is dealt with further on.              */
  /***************************************************************************/
  sap_ptr->channel_id = channel_id;
  sap_ptr->max_write_netd_length = MAX_WRITE_NETD_LENGTH;
  sap_ptr->station_list_ptr = NULL;
  /***************************************************************************/
  /* Set sap_state straight to opened rather than to opening, as there is no */
  /* longer any dh involvement                                               */
  /***************************************************************************/
  sap_ptr->sap_state = (unsigned int)sap_opened;
  outputf("QSM_ENABLE_SAP: initialisation complete\n");
  /***************************************************************************/
  /* Add SAP to linked list                                                  */
  /* First you'll need to lock the channel                                   */
  /***************************************************************************/
  outputf("QSM_ENABLE_SAP: link management\n");
  lock_rc = lockl(&(channel_id->lock),0);
  outputf("QSM_ENABLE_SAP: channel locked\n");

  if (channel_id->sap_list_ptr == NULL)
  {
    outputf("QSM_ENABLE_SAP: no other saps on channel\n");
    /*************************************************************************/
    /* If sap list for this channel is empty, then this is the first SAP.    */
    /*************************************************************************/
    channel_id->sap_list_ptr = sap_ptr;
    sap_ptr->next_sap_ptr = NULL;
    sap_ptr->prev_sap_ptr = NULL;
  }
  else
  {
    outputf("QSM_ENABLE_SAP: there are already other saps on channel\n");
    /*************************************************************************/
    /* There is at least one SAP already in the linked list.                 */
    /* Find the end of the list.                                             */
    /*************************************************************************/
    last_sap_ptr = channel_id->sap_list_ptr;
    while (last_sap_ptr->next_sap_ptr != NULL)
    {
      outputf("QSM_ENABLE_SAP: move once along sap list\n");
      last_sap_ptr = last_sap_ptr->next_sap_ptr;
    }
    outputf("QSM_ENABLE_SAP: found end of sap list for this channel\n");
    outputf("QSM_ENABLE_SAP: last sap ptr = %x\n",last_sap_ptr);
    /*************************************************************************/
    /* last_sap_ptr now points to the last sap in the list.                  */
    /* Add new SAP to end of list.                                           */
    /*************************************************************************/
    last_sap_ptr->next_sap_ptr = sap_ptr;
    sap_ptr->next_sap_ptr = NULL;
    sap_ptr->prev_sap_ptr = last_sap_ptr;
  }
  /*************************************************************************/
  /* Now unlock channel                                                    */
  /*  NOTE THAT SAP IS STILL LOCKED */
  /*************************************************************************/
  if (lock_rc != LOCK_NEST) unlockl(&(channel_id->lock));

  /***************************************************************************/
  /* SAP has been added to the end of the list. Generate the result.         */
  /* Note that SAP is still locked from when it was allocated                */
  /***************************************************************************/
  outputf("QSM_ENABLE_SAP: calling qcm_make_sape_result\n");
  qcm_rc = qcm_make_sape_result (
    sap_ptr->channel_id,
    sap_ptr->user_sap_correlator,
    (correlator_type)NULL,
    sap_ptr->max_write_netd_length
    );
  outputf("QSM_ENABLE_SAP: qcm make result func rc=%d\n",qcm_rc);
  if (qcm_rc != qcm_rc_ok)
  {
    rc = qsm_rc_system_error;
  }
  outputf("QSM_ENABLE_SAP: unlocking sap\n");
  unlockl(&(sap_ptr->lock));
  outputf("QSM_ENABLE_SAP: returning\n");
  return(rc);
}

/*****************************************************************************/
/* Function     QSM_DISABLE_SAP                                              */
/*                                                                           */
/* Description  This procedure calls the qlm to halt all the stations        */
/*              started on this sap. It also frees the storage assoc         */
/*              with the sap                                                 */
/*              It calls the QLM_Halt with the "silent" flag set to          */
/*              suppress the Halt Done results as the user would not         */
/*              expect to receive STAS results to every station that         */
/*              is halted in response to a Disable_SAP ioctl.                */
/*              On output the sap identified for disabling will have         */
/*              been removed from the sap list for the channel. Any          */
/*              resources belonging to any stations that were started        */
/*              on the SAP will have been freed up.                          */
/*                                                                           */
/*              The parameter silent indicates whether this function should  */
/*              produce a result or not. If it was called in response to a   */
/*              user issued disable ioctl, a result is needed, so silent is  */
/*              passed as FALSE. If in response to a QLLC internal error     */
/*              condition, it is TRUE, and no result should be generated.    */
/*                                                                           */
/* Return       qsm_rc_type   rc;                                            */
/*                                                                           */
/* Parameters   channel_id;                                                  */
/*                                                                           */
/*****************************************************************************/
qsm_rc_type  qsm_disable_sap (

  channel_id_type       channel_id,
  correlator_type       qllc_sap_correlator,
  boolean               silent)
{
  qsm_rc_type     rc = qsm_rc_ok;
  qcm_rc_type     qcm_rc;
  correlator_type user_sap_correlator;
  sap_type       *sap_ptr;
  sap_type       *sap_id;
  int             lock_rc;
  boolean	  unlock_sap;
  boolean	  chan_waslocked;
  boolean         sap_found;
  station_type   *station_ptr;

  outputf("QSM_DISABLE_SAP: ...has been called\n");

  /***************************************************************************/
  /*  we need to get the channel lock before getting SAP lock                */
  /***************************************************************************/
  outputf("QSM_DISABLE_SAP: locking channel\n");
  chan_waslocked = (lockl(&(channel_id->lock), LOCK_SHORT) == LOCK_NEST);

  /***************************************************************************/
  /* start off by verifying that correlator is genuine                       */
  /***************************************************************************/
  sap_id = qsm_find_sap_given_correlator(qllc_sap_correlator, &unlock_sap);
  if (sap_id == NULL)
  {
    /*************************************************************************/
    /* SAP cannot be found                                                   */
    /*************************************************************************/
    outputf("QSM_DISABLE_SAP: sap cannot be found\n");
    if (!chan_waslocked) unlockl(&(channel_id->lock));
    rc = qsm_rc_no_such_sap;
    return (rc);
  }
  /***************************************************************************/
  /* The SAP can be disabled.                                                */
  /* Remember the user's correlator.                                         */
  /***************************************************************************/
  user_sap_correlator = sap_id->user_sap_correlator;
  /***************************************************************************/
  /* Call QLM to halt all the LSs; QLM frees storage, and mends stn list     */
  /***************************************************************************/
  while ((station_ptr = sap_id->station_list_ptr) != NULL)
  {
    /*************************************************************************/
    /* Halt any stations that are started.                                   */
    /* Note that the qlm_halt_ls function is void.                           */
    /*************************************************************************/
    outputf("QSM_DISABLE_SAP: calling halt_ls\n");
    (void)qlm_halt_ls(station_ptr, SILENT, FALSE);
  }

  /***************************************************************************/
  /* unlock sap now - channel is locked so no one else can be waiting        */
  /* From this point, we will either delete it or do nothing with it         */
  /***************************************************************************/
  unlockl(&sap_id->lock);

  /***************************************************************************/
  /* Can now take SAP out of list and free it.                               */
  /***************************************************************************/
  outputf("QSM_DISABLE_SAP: removing sap from list...\n");
  /***************************************************************************/
  /* Order in which you scan list is crucial to avoid deadlocks.             */
  /* Channel is already locked by QCM                                        */
  /***************************************************************************/
  if (channel_id->sap_list_ptr == NULL)
  {
    /*************************************************************************/
    /* Trivial case - list is empty                                          */
    /*************************************************************************/
    if (!chan_waslocked) unlockl(&(channel_id->lock));
    return(qsm_rc_no_such_sap);
  }
  else
  {
    /*************************************************************************/
    /* List is not empty                                                     */
    /*************************************************************************/
    sap_ptr = channel_id->sap_list_ptr;
    sap_found = FALSE;
    while (sap_ptr != NULL && sap_found == FALSE)
    {
      if (sap_ptr == sap_id)
      {
	sap_found = TRUE;
	outputf("QSM_DISABLE_SAP: sap's position in list has been found\n");

	/* unlink this SAP from the list */
	if (sap_ptr->prev_sap_ptr != NULL)
		sap_ptr->prev_sap_ptr->next_sap_ptr = sap_ptr->next_sap_ptr;
	else
		channel_id->sap_list_ptr = sap_ptr->next_sap_ptr;

	if (sap_ptr->next_sap_ptr != NULL)
		sap_ptr->next_sap_ptr->prev_sap_ptr = sap_ptr->prev_sap_ptr;
	
	/* release associated memory */
	if (xmfree((char *)sap_ptr, pinned_heap) != NULL)
	{
	  if (!chan_waslocked) unlockl(&(channel_id->lock));
	  return(qsm_rc_system_error);
	}
      }
      else /* sap_ptr != sap_id */
      {
	sap_ptr = sap_ptr->next_sap_ptr;
      }
    }
    if (sap_found == FALSE)
    {
      rc = qsm_rc_no_such_sap;
    }
    else
    {
      outputf("QSM_DISABLE_SAP: sap has been removed from list and freed\n");
      rc = qsm_rc_ok;
    }
  }

  /* all done with lists now, unlock channel */
  if (!chan_waslocked) unlockl(&(channel_id->lock));

  /***************************************************************************/
  /* Generate SAP_Disabled result, if silent indication is not set.          */
  /***************************************************************************/
  if (silent == FALSE)
  {
    qcm_rc = qcm_make_result (
      channel_id,
      user_sap_correlator,
      (correlator_type)NULL,                 /* user_ls_correlator           */
      sap_disabled,
      successful
      );
    if (qcm_rc != qcm_rc_ok)
    {
      rc = qsm_rc_system_error;
    }
  }
  outputf("QSM_DISABLE_SAP: state of channel lock = %x\n",channel_id->lock);
  return(rc);
}

/*****************************************************************************/
/* Function     QSM_QUERY_SAP                                                */
/*                                                                           */
/* Description  This procedure is called by the QDH when a Query_SAP ioctl   */
/*              is issued by the user. It accepts a Query_SAP_Extension,     */
/*              for initialisation.                                          */
/*              On successful completion the query sap extension will be     */
/*              filled with the results from the query.                      */
/*                                                                           */
/*              The query saps data returned by QLLC is the cio_stats struct */
/*              defined in comio.h.  There is no additional protocol data    */
/*              as portrayed by the example in comio.h.                      */
/*                                                                           */
/* Return       qsm_rc_type                                                  */
/*                                                                           */
/* Parameters   channel_type        *channel_id                              */
/*              struct qlc_qsap_arg *qlc_ext_ptr                             */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
qsm_rc_type qsm_query_sap (

  channel_type        *channel_id,
  struct qlc_qsap_arg *qlc_ext_ptr)
{
  sap_type             *sap_ptr;
  port_type            *port_id;
  qpm_rc_type           qpm_rc;
  struct dlc_qsap_arg  *ext_ptr;
  boolean		unlock_sap;

  int                   i; /* debug only */

  outputf("QSM_QUERY_SAP: ...has been called\n");
  /***************************************************************************/
  /* Set up internal pointer to GDLC structure sub-area of QLLC structure.   */
  /***************************************************************************/
  ext_ptr = (struct dlc_qsap_arg *)qlc_ext_ptr;
  /***************************************************************************/
  /* Find SAP                                                                */
  /***************************************************************************/
  sap_ptr = qsm_find_sap_given_correlator(ext_ptr->gdlc_sap_corr, &unlock_sap);
  if (sap_ptr == NULL)
  {
    outputf("QSM_QUERY_SAP: sap not found, returning\n");
    return(qsm_rc_no_such_sap);
  }
  else
  {
    port_id = QCM_RETURN_PORT_ID(sap_ptr->channel_id);
    /*************************************************************************/
    /* Update returned fields in the query sap argument block.               */
    /*************************************************************************/
    ext_ptr->user_sap_corr = sap_ptr->user_sap_correlator;
    ext_ptr->sap_state = sap_ptr->sap_state;
    strcpy(ext_ptr->dev, port_id->xdh_pathname);
    ext_ptr->devdd_len = sizeof(cio_stats_t);

    /*************************************************************************/
    /* unlock SAP now, we are done with it                                   */
    /*************************************************************************/
    if (unlock_sap) unlockl(&sap_ptr->lock);

    /*************************************************************************/
    /* And issue query to DH via the qpm.                                    */
    /*************************************************************************/
    outputf("QSM_QUERY_SAP: calling qpm_query_device() \n");
    qpm_rc = qpm_query_device(port_id,qlc_ext_ptr->psd.x25_data);
    outputf("QSM_QUERY_SAP: query completed. data is as follows...\n");
    for(i=0; i<sizeof(cio_stats_t); i++)
      outputf("%d",qlc_ext_ptr->psd.x25_data[i]);
    outputf("\n");

    outputf("QSM_QUERY_SAP: returning, rc = %d\n",qpm_rc);
    if (qpm_rc != qpm_rc_ok)
      return(qsm_rc_system_error);
    else
      return(qsm_rc_ok);
  }
}


/*****************************************************************************/
/* Function     QSM_CHECK_SAP                                                */
/*                                                                           */
/* Description  This procedure provides the QLM with an indication as to     */
/*              whether it a SAP is in a state that will allow a further     */
/*              link station to be started.                                  */
/*                                                                           */
/* Return       qsm_rc_type    rc;                                           */
/*                                                                           */
/* Parameters   sap_type *     sap_ptr                                       */
/*                                                                           */
/*****************************************************************************/
qsm_rc_type qsm_check_sap(
  sap_type *sap_ptr)
{
  qsm_rc_type rc;
  station_type *station_ptr;
  int station_count;

  rc = qsm_rc_ok;

  /* count the number of stations started on the sap                       */
  station_ptr = sap_ptr->station_list_ptr;
  station_count = 0;
  while (station_ptr != NULL)
  {
    station_ptr = station_ptr->next_station_ptr;
    station_count++;
  }
  if (station_count >= sap_ptr->max_link_stations)
  {
    rc = qsm_rc_station_limit_reached;
  }

  return(rc);
}

/*****************************************************************************/
/* Function     qsm_find_sap_given_correlator                                */
/*                                                                           */
/* Description  This procedure verifies the existence of a sap with          */
/*              the correlator passed as an argument to it.                  */
/*                                                                           */
/* Return       sap address if sap found                                     */
/*              null if sap not found                                        */
/*                                                                           */
/* Parameters                                                                */
/*              correlator                                                   */
/*                                                                           */
/*****************************************************************************/
sap_type *qsm_find_sap_given_correlator(

  correlator_type sap_correlator,
  boolean *unlock)
{
  sap_type *sap_ptr;
  boolean was_locked;
  channel_type *channel_ptr;
  extern channel_list_type channel_list;

  if ( (channel_ptr=channel_list.channel_ptr)==NULL )
    return(NULL);

  /***************************************************************************/
  /* Search each channel in the channel list for the SAP                     */
  /***************************************************************************/
  sap_ptr = NULL;
  while (channel_ptr != NULL && sap_ptr == NULL)
  {
	/* lock list of SAPs in channel */
    was_locked = (lockl(&channel_ptr->lock, LOCK_SHORT) == LOCK_NEST);

    /*************************************************************************/
    /* compare correlator with each SAP in this channel's list               */
    /*************************************************************************/
    if ( channel_ptr->sap_list_ptr == NULL )
    {
      sap_ptr = NULL;
    }
    else
    {
      for ( sap_ptr = channel_ptr->sap_list_ptr;
	    sap_ptr != NULL;
	    sap_ptr = sap_ptr->next_sap_ptr)
      {
	/*
	 * If SAP is found, lock it and return code from lockl()
	 */
        if (sap_ptr->qllc_sap_correlator == sap_correlator)
        {
	  *unlock = (lockl(&sap_ptr->lock, LOCK_SHORT) != LOCK_NEST);
	  break;
        }
      }
    }

    if (!was_locked) unlockl(&channel_ptr->lock);
    channel_ptr = channel_ptr->next_channel_ptr;
  }
  
  return sap_ptr;
}
