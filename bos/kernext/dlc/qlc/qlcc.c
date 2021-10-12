static char sccsid[] = "@(#)50	1.18  src/bos/kernext/dlc/qlc/qlcc.c, sysxdlcq, bos411, 9428A410j 6/27/94 18:36:36";
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: qcm_allocate_channel, qcm_free_channel, qcm_open_channel,
 *            qcm_close_channel, qcm_read_data_queue, qcm_read_exception_queue,
 *            qcm_receive_data, qcm_select, qcm_saps_using_port,
 *            qcm_check_channel, qcm_port_error
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
/* Include the QCM header file.                                              */
/*****************************************************************************/
#include "qlcg.h"
#include "qlcvfac.h"
#include "qlcv.h"
#include "qlcq.h"
#include "qlcb.h"
#include "qlcl.h"       /* station types                                     */
#include "qlcp.h"       /* port_name_type, port_type                         */
#include "qlcc.h"
#include "qlcs.h"       /* sap_type                                          */

/*****************************************************************************/
/* Declare Global Channel_List                                               */
/*****************************************************************************/
extern channel_list_type channel_list;

/*****************************************************************************/
/* Function     QCM_ALLOCATE_CHANNEL                                         */
/*                                                                           */
/* Description  This procedure allocates storage for a channel control block */
/*              adds it to the channel list and intialises it.               */
/*              Channel list, on successful completion has additional channel*/
/*              added.                                                       */
/*              A pointer to the allocated channel storage is returned as    */
/*              the only parameter. It is therefore passed by reference.     */
/*                                                                           */
/* Return       qcm_rc_type      rc                                          */
/*                                                                           */
/* Parameters   channel_id_type  *channel_id;  passed by reference...        */
/*                                                                           */
/*****************************************************************************/
qcm_rc_type qcm_allocate_channel(

  channel_id_type *channel_id)
{
  qcm_rc_type rc;
  channel_type *channel_ptr;
  int lock_rc;

  rc = qcm_rc_ok;
  /***************************************************************************/
  /* Allocate storage for a channel                                          */
  /***************************************************************************/
  *channel_id =(channel_id_type)xmalloc(sizeof(channel_type),WORD,pinned_heap);
  if ( *channel_id == NULL )
  {
    outputf("QCM_ALLOCATE_CHANNEL: allocate failed.\n");
    return(qcm_rc_alloc_failed);
  }
  bzero((*channel_id), sizeof(channel_type));
  (*channel_id)->lock = LOCK_AVAIL;
  /***************************************************************************/
  /* Channel has been successfully allocated, lock list to add it            */
  /***************************************************************************/
  lock_rc = lockl(&(channel_list.lock), LOCK_SHORT);
  /***************************************************************************/
  /* Find out if there are any channels already open                         */
  /***************************************************************************/
  if (channel_list.channel_ptr == (channel_type *)NULL)
  {
    outputf("QCM_ALLOCATE_CHANNEL: putting first channel in list...\n");
    /*************************************************************************/
    /* This is the first channel to be put in the list                       */
    /* Channel list is already locked so can modify channel list.            */
    /*************************************************************************/
    channel_list.channel_ptr = (*channel_id);
    (*channel_id)->next_channel_ptr = NULL;
    (*channel_id)->prev_channel_ptr = NULL;
  }
  else
  {
    outputf("QCM_ALLOCATE_CHANNEL: adding channel to existing list...\n");
    /*************************************************************************/
    /* There is already at least one channel in the list                     */
    /* Find start of list                                                    */
    /*************************************************************************/
    channel_ptr = channel_list.channel_ptr;
    /*************************************************************************/
    /* Move along list to end                                                */
    /*************************************************************************/
    while (channel_ptr->next_channel_ptr != NULL)
      channel_ptr = channel_ptr->next_channel_ptr;
    /*************************************************************************/
    /* Add new channel to end of list                                        */
    /*************************************************************************/
    channel_ptr->next_channel_ptr = (*channel_id);
    (*channel_id)->next_channel_ptr = NULL;
    (*channel_id)->prev_channel_ptr = channel_ptr;
  }
  /***************************************************************************/
  /* Channel is now added to end of list. Initialise it.                     */
  /***************************************************************************/
/* Defect 101380 - don't use u-area to get user process information.
  (*channel_id)->proc_id = u.u_procp;
  (*channel_id)->select_status.user_proc = (struct proc *)NULL;
  (*channel_id)->readsleep = (struct proc *)EVENT_NULL;
*/
/* <<< THREADS >>> */
/* <<< removed  (*channel_id)->proc_id = getpid (); >>> */
/* <<< end THREADS >>> */
  (*channel_id)->readsleep = EVENT_NULL;

/* End defect 101380 */

  (*channel_id)->read_status = NULL;
  (*channel_id)->data_queue.first = NULL;
  (*channel_id)->data_queue.last = NULL;
  (*channel_id)->exception_queue = (exception_queue_type)NULL;
  if (lock_rc != LOCK_NEST) unlockl(&(channel_list.lock));
  return(rc);
}

/*****************************************************************************/
/* Function     QCM_FREE_CHANNEL                                             */
/*                                                                           */
/* Description  This procedure frees the storage associated with a channel.  */
/*              On succesful completion the channel identified by the id     */
/*              passed as input parameter will have been removed from the    */
/*              linked list and any resources associated with it will have   */
/*              been freed.                                                  */
/*                                                                           */
/* Return       qcm_rc_type    rc;                                           */
/*                                                                           */
/* Parameters   channel_id_type  channel_id;                                 */
/*                                                                           */
/* Lock requirements: This function assumes all locks to be off on entry.    */
/*                    It exits with all locks off as well.                   */
/*****************************************************************************/
qcm_rc_type qcm_free_channel (

  channel_id_type channel_id)
{
  int                     lock_rc;
  channel_type           *channel_ptr;
  boolean                 channel_found;
  gen_buffer_type        *data_ptr;
  pending_exception_type *excep_ptr;
  qcm_rc_type             rc = qcm_rc_ok;

  outputf("QCM_FREE_CHANNEL: .....has been called\n");
  lock_rc = lockl(&(channel_list.lock),0);
  /***************************************************************************/
  /* Perform validity checks on channel_id passed to this procedure.         */
  /***************************************************************************/
  if (channel_id->channel_id != NULL)
  {
    outputf("QCM_FREE_CHANNEL: channel is in use\n");
    if (lock_rc != LOCK_NEST) unlockl(&(channel_list.lock));
    rc = qcm_rc_invalid_command;
    return (rc);
  }
  /***************************************************************************/
  /* The following algorithm is used:                                        */
  /***************************************************************************/
  if (channel_list.channel_ptr == NULL)
  {
    /*************************************************************************/
    /* Trivial case - list is empty                                          */
    /*************************************************************************/
    outputf("QCM_FREE_CHANNEL: channel_list is empty\n");
    /* Defect 101380 */
    if (lock_rc != LOCK_NEST) unlockl(&(channel_list.lock));
    return(qcm_rc_channel_not_found);
  }
  else
  {
    /*************************************************************************/
    /* List is not empty - that's a good start!                              */
    /*************************************************************************/
    channel_ptr = channel_list.channel_ptr;
    channel_found = FALSE;
    while (channel_ptr != NULL && channel_found == FALSE)
    {
      if (channel_ptr == channel_id)
      {
         /* If execptions exists on the exception queue, free it. */
         while( QCM_EXCEPTION_QUEUE_IS_NOT_EMPTY(channel_ptr) )
         {
            excep_ptr = channel_ptr->exception_queue;
            channel_ptr->exception_queue=excep_ptr->next_pending_exception_ptr;
            if( channel_ptr->exception_queue != NULL )
               channel_ptr->exception_queue->prev_pending_exception_ptr = NULL;
	    if( excep_ptr->exception_data != NULL )
	      xmfree( excep_ptr->exception_data, pinned_heap );
            xmfree( excep_ptr, pinned_heap );
         }
         /* If data exists on the data queue, free it. */
         while( channel_ptr->data_queue.first )
         {
            data_ptr = QBM_DEQUE_BUFFER( &(channel_ptr->data_queue) );
            if( data_ptr )
               QBM_FREE_BUFFER( data_ptr );
         }
         channel_found = TRUE;

	 /* fix up link for next channel */
         if (channel_ptr->next_channel_ptr != NULL)
	   channel_ptr->next_channel_ptr->prev_channel_ptr =
		channel_ptr->prev_channel_ptr;

	 /* fix up link for previous channel */
         if (channel_ptr->prev_channel_ptr != NULL)
	   channel_ptr->prev_channel_ptr->next_channel_ptr =
		channel_ptr->next_channel_ptr;
         else
	   channel_list.channel_ptr = channel_ptr->next_channel_ptr;

         if (xmfree((char *)channel_ptr, pinned_heap) != NULL)
         {
            if (lock_rc != LOCK_NEST) unlockl(&(channel_list.lock));
            return(qcm_rc_free_failed);
         }
      }
      else /* channel_ptr != channel_id */
      {
         channel_ptr = channel_ptr->next_channel_ptr;
      }
    }

    if (lock_rc != LOCK_NEST) unlockl(&channel_list.lock);
    if (channel_found == FALSE)
    {
       return(qcm_rc_channel_not_found);
    }
    else
    {
       outputf("QCM_FREE_CHANNEL: channel successfully freed\n");
       return(qcm_rc_ok);
    }
  }
}

/*****************************************************************************/
/* Function     QCM_OPEN_CHANNEL                                             */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* Return       qcm_rc_type     rc;                                          */
/*                                                                           */
/* Parameters   channel_id_type      channel_id;                             */
/*              open_extension_type *open_extension;                         */
/*              unsigned int         open_flag;                              */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
qcm_rc_type qcm_open_channel(

  channel_id_type        channel_id,
  struct dlc_open_ext   *open_extension,
  unsigned long          open_flag)
{
  qcm_rc_type rc;
  int lock_rc;

  outputf("QCM_OPEN_CHANNEL: open_ext ptr = %d\n",open_extension);
  outputf("channel_id == %x\n", channel_id );
  /***************************************************************************/
  /* Lock channel                                                            */
  /***************************************************************************/
  lock_rc = lockl(&(channel_id->lock), LOCK_SHORT);

  /***************************************************************************/
  /* Initialise channel_id field in channel. This serves as an indication    */
  /* that the channel has been properly opened.                              */
  /***************************************************************************/
  channel_id->channel_id = channel_id;

  /***************************************************************************/
  /* Remember open_flag, so you know whether DNDELAY is set or not..         */
  /***************************************************************************/
  channel_id->devflag = open_flag;

  /***************************************************************************/
  /* If the caller has not provided a non-zero value of max_saps then select */
  /* the default value.                                                      */
  /***************************************************************************/
  if (open_extension->maxsaps == 0)
  {
    outputf("QCM_OPEN_CHANNEL: no maxsaps selected by user at open\n");
    outputf("QCM_OPEN_CHANNEL: ..allowing maxsaps to default to %d\n",
      MAXIMUM_SAPS_DEFAULT);
    channel_id->maximum_saps = MAXIMUM_SAPS_DEFAULT;
  }
  else
  {
    /*************************************************************************/
    /* Probably want some more concrete value                                */
    /* checking. i.e. 0 < max_saps < "LIMIT"                                 */
    /*************************************************************************/
    outputf("QCM_OPEN_CHANNEL: maxsaps selected by user = %d\n",
      open_extension->maxsaps);
    channel_id->maximum_saps = open_extension->maxsaps;
  }
  /***************************************************************************/
  /* Initialise other channel fields                                         */
  /***************************************************************************/
  channel_id->sap_list_ptr = NULL;
  channel_id->exception_queue = NULL;
  channel_id->data_queue.first = NULL;
  channel_id->data_queue.last = NULL;
  channel_id->read_status = NULL;
  /***************************************************************************/
  /* Find out whether the user is a kernel user or an application user.      */
  /***************************************************************************/
  if ( (open_flag & DKERNEL) == DKERNEL)
  {
    /*************************************************************************/
    /* The user is a Kernel User.                                            */
    /*************************************************************************/
    channel_id->user_nature = kernel_user;
    /*************************************************************************/
    /* A Kernel user passes in the open ext the addresses of their functions */
    /* to called when data/exceptions are to be posted to the user.          */
    /* Initialise function addresses in channel from open extension          */
    /*************************************************************************/
    channel_id->rx_normal_data_fn_addr= open_extension->rcvi_fa;
    channel_id->rx_xid_data_fn_addr   = open_extension->rcvx_fa;
    channel_id->rx_netd_data_fn_addr  = open_extension->rcvn_fa;
    channel_id->exception_fn_addr     = open_extension->excp_fa;
    /*************************************************************************/
    /* Initialise any select fields to NULL as they are for Application      */
    /* Users only                                                            */
    /*************************************************************************/
/* Defect 101380 
    channel_id->select_status.user_proc = NULL;
*/
    channel_id->select_status.selected_events &= ~POLLIN;
    channel_id->select_status.selected_events &= ~POLLPRI;
    channel_id->select_status.selected_events &= ~POLLOUT;
  }
  else
  {
    /*************************************************************************/
    /* The user is an application user.                                      */
    /*************************************************************************/
    channel_id->user_nature = application_user;
    /*************************************************************************/
    /* Set the function addresses to NULL, as they are or kernel users only  */
    /*************************************************************************/
    channel_id->rx_normal_data_fn_addr = (function_address_type)NULL;
    channel_id->rx_xid_data_fn_addr = (function_address_type)NULL;
    channel_id->rx_netd_data_fn_addr = (function_address_type)NULL;
    channel_id->exception_fn_addr = (function_address_type)NULL;

    /*************************************************************************/
    /* If an Application User, the exception, normal, xid,                   */
    /* and network data queues must be initialised to empty.                 */
    /*************************************************************************/
    channel_id->exception_queue = (pending_exception_type *)NULL;
    channel_id->data_queue.first = NULL;
    channel_id->data_queue.last = NULL;
    /*************************************************************************/
    /* initialise any select info                                            */
    /*************************************************************************/
/* Defect 101380
    channel_id->select_status.user_proc = NULL;
*/
    channel_id->select_status.selected_events &= ~POLLIN;
    channel_id->select_status.selected_events &= ~POLLPRI;
    channel_id->select_status.selected_events &= ~POLLOUT;
  }
  rc = qcm_rc_ok;
  if (lock_rc != LOCK_NEST) unlockl(&(channel_id->lock));
  return (rc);
}

/*****************************************************************************/
/* Function     QCM_CLOSE_CHANNEL                                            */
/*                                                                           */
/* Description  This procedure is called when the user issues a              */
/*              Close command, or when the port experiences a condition      */
/*              that causes it to call QCM_Port_Error, which finds all       */
/*              the channels on the port in trouble, and closes them, which  */
/*              in turn closes the port.                                     */
/*                                                                           */
/* Return       qcm_rc_type      rc;                                         */
/*                                                                           */
/* Parameters   channel_id_type  channel_id;                                 */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
qcm_rc_type  qcm_close_channel (

  channel_id_type channel_id)
{
  qcm_rc_type rc;
  int lock_rc;
  qsm_rc_type qsm_rc;

  outputf("QCM_CLOSE_CHANNEL: has been called...\n");
  outputf("channel_id == %x\n", channel_id );

  /***************************************************************************/
  /* Make sure the channel exists.                                           */
  /***************************************************************************/
  if (channel_id->channel_id != channel_id)
  {
    rc = qcm_rc_channel_not_found; 
    return(rc);
  }
  /***************************************************************************/
  /* First ensure that all SAPs enabled on this channel are disabled.        */
  /***************************************************************************/
  qsm_rc = qsm_rc_ok;
  lock_rc = lockl(&channel_list.lock, LOCK_SHORT);
  while (channel_id->sap_list_ptr != NULL && qsm_rc == qsm_rc_ok)
  {
    outputf("QCM_CLOSE_CHANNEL: there are saps enabled...\n");
    /*************************************************************************/
    /* Set the silent indicator (last arg to disable) in order to suppress   */
    /* sap disabled result.                                                  */
    /*************************************************************************/
    qsm_rc = qsm_disable_sap(
      channel_id,
      (correlator_type)(channel_id->sap_list_ptr),
      SILENT);
  }
  if (lock_rc != LOCK_NEST) unlockl(&channel_list.lock);

  if (qsm_rc != qsm_rc_ok)
  {
    outputf("QCM_CLOSE_CHANNEL: bad rc from qsm_disable_sap = %d\n",qsm_rc);
    rc = qcm_rc_system_error;
    return(rc);
  }
  /***************************************************************************/
  /* Try and terminate the port. (This doesn't free the port, it merely      */
  /* disconnects this channel from the port. If the channel is the only one  */
  /* using the port, then the QPM will decide to close and free the port,    */
  /* otherwise it will leave it for the other channels that are using it.    */
  /***************************************************************************/
  outputf("QCM_CLOSE_CHANNEL: calling port terminate....\n");
  if ( qpm_port_terminate(channel_id->port_id) != qpm_rc_ok )
  {
    outputf("QCM_CLOSE_CHANNEL: bad rc from qpm\n");
    rc = qcm_rc_port_error;
  }
  else /* good return from qpm_terminate_port()                              */
  {
    outputf("QCM_CLOSE_CHANNEL: good rc from qpm, closing channel...\n");
    /*************************************************************************/
    /* Lock the channel                                                      */
    /*************************************************************************/
    outputf("QCM_CLOSE_CHANNEL: locking channel\n");
    outputf("QCM_CLOSE_CHANNEL: state of chan lock = %x\n",channel_id->lock);
    lock_rc = lockl(&(channel_id->lock),0);
    outputf("QCM_CLOSE_CHANNEL: state of chan lock = %x\n",channel_id->lock);
    /*************************************************************************/
    /* The channel could be "disabled" by setting a state field in the       */
    /* channel to CLOSING. This would prevent any other functions from       */
    /* making use of it before qlcmpx gets to free it and tidy everything up.*/
    /* I will implement this if there proves to be a problem.                */
    /*************************************************************************/
    /* note that state and CLOSING are not defined in qlcc.h yet. */
    /* channel_id->state = CLOSING; */
    channel_id->channel_id = NULL;
    outputf("QCM_CLOSE_CHANNEL: unlock channel\n");
    outputf("QCM_CLOSE_CHANNEL: state of chan lock = %x\n",channel_id->lock);
    if (lock_rc != LOCK_NEST) unlockl(&(channel_id->lock));
    outputf("QCM_CLOSE_CHANNEL: state of chan lock = %x\n",channel_id->lock);
    rc = qcm_rc_ok;
  }
  return(rc);
}


/*****************************************************************************/
/* Function     QCM_READ_DATA_QUEUE                                          */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/*    This procedure doesn't do any clever buffer translation or             */
/*  iomove type things. It simply checks the read queue and returns          */
/*  the ptr to the whole buffer, with header info included as per the        */
/*  buffer definition given in QBM.                                          */
/*    It does separate the read_ext (or rather copy it, as it is             */
/*  still in the buffer), and pass that back as a separate parameter.        */
/*    Note that the buffer_ptr is passed by reference.                       */
/*                                                                           */
/* Return   qcm_rc_type    rc;                                               */
/*                                                                           */
/* Parameters                                                                */
/*          channel_id_type channel_id;                                      */
/*          gen_buffer_type **buffer_ptr;   passed by reference...           */
/*          read_ext_type   *read_ext_ptr;                                   */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
qcm_rc_type  qcm_read_data_queue(
  channel_id_type    channel_id,
  gen_buffer_type  **buffer_ptr,
  read_ext_type     *read_ext_ptr)
{
  qcm_rc_type  rc;
  int          lock_rc;

  outputf("QCM_READ_DATA_Q: called\n");
  if (channel_id->channel_id != channel_id)
  {
    outputf("QCM_READ_DATA_Q: no such channel\n");
    rc = qcm_rc_channel_not_found;
    return (rc);
  }
  if (channel_id->user_nature == kernel_user)
  {
    outputf("QCM_READ_DATA_Q: kernel user\n");
    rc = qcm_rc_invalid_command;
  }
  else
  {
    outputf("QCM_READ_DATA_Q: application user\n");
    lock_rc = lockl(&(channel_id->lock),0);
    /*************************************************************************/
    /* Buffers are queued in channel by chaining them together by m_act      */
    /* field. Each buffer can actually consist of a number of individual     */
    /* m_bufs chained together by m_next. The use of X25 macros enables      */
    /* QLLC to treat m_next chains as single entities.                       */
    /* These macros are called by the QBM to provide isolation from changes  */
    /* to the buffer handling scheme.                                        */
    /*************************************************************************/

    /*************************************************************************/
    /* find first buffer in data queue                                       */
    /*************************************************************************/
    outputf("QCM_READ_DATA_Q: dequeue first buffer\n");
    *buffer_ptr = QBM_DEQUE_BUFFER(&(channel_id->data_queue));
    if (*buffer_ptr == NULL)
    {
      outputf("QCM_READ_DATA_Q: queue was empty\n");
      /***********************************************************************/
      /* Queue is empty                                                      */
      /* Do nothing. This is not considered to be an error condition. User's */
      /* are at liberty to read the channel queues even if there is no data. */
      /***********************************************************************/
      rc= qcm_rc_ok;
    }
    else /* there is data                                                    */
    {
      outputf("QCM_READ_DATA_Q: data found on queue\n");
      outputf("QCM_READ_DATA_Q: buffer_ptr = %x\n",*buffer_ptr);
      print_x25_buffer(*buffer_ptr);
      /***********************************************************************/
      /* The QLM builds completes the building of the read extension held in */
      /* the top part of the buffer before passing a buffer to the QCM. The  */
      /* QCM can therefore simply copy the fields from the ext in the buffer */
      /* to the fields in the read_ext supplied by the caller.               */
      /***********************************************************************/
      outputf("QCM_READ_DATA_Q: build read extension\n");
      read_ext_ptr->user_sap_correlator =
	QBM_RETURN_USER_SAP_CORRELATOR(*buffer_ptr);
      read_ext_ptr->user_ls_correlator =
	QBM_RETURN_USER_LS_CORRELATOR(*buffer_ptr);
      read_ext_ptr->dlc_flags = QBM_RETURN_DLC_FLAGS(*buffer_ptr);
      /***********************************************************************/
      /* Regardless of the value of header.qllc_header.dlh_length, return    */
      /* the whole buffer, including header. The QDH will separate the QLLC  */
      /* header information if necessary.                                    */
      /***********************************************************************/
    }
    rc = qcm_rc_ok;
    outputf("QCM_READ_DATA_Q: unlock channel\n");
    if (lock_rc != LOCK_NEST) unlockl(&(channel_id->lock));
  }
  return(rc);
}


/*****************************************************************************/
/* Function     QCM_READ_EXCEPTION_QUEUE                                     */
/*                                                                           */
/* Description  This procedure gets the first exception off the              */
/*              exception queue if it is not empty.                          */
/*              On succesful completion the top exception on the exception   */
/*              queue will have been dequeued, if the queue was not empty.   */
/*                                                                           */
/* Return       qcm_rc_type      rc;                                         */
/*                                                                           */
/* Parameters   channel_id_type  channel_id;                                 */
/*              get_exceptio...  exc_ptr   passed by reference..             */
/*                                                                           */
/*****************************************************************************/
qcm_rc_type qcm_read_exception_queue(
  channel_id_type         channel_id,
  struct dlc_getx_arg   **exc_ptr)
{
  pending_exception_type *tmp;

  outputf("QCM_READ_EXCP_Q: called\n");
  if (channel_id->channel_id != channel_id)
  {
    /*************************************************************************/
    /* The channel id passed to this procedure does not point to a valid     */
    /* channel control block.                                                */
    /*************************************************************************/
    outputf("QCM_READ_EXCP_Q: channel not found\n");
    return (qcm_rc_channel_not_found);
  }
  else
  {
    outputf("QCM_READ_EXCP_Q: channel found\n");
    if (channel_id->user_nature == kernel_user)
    {
      outputf("QCM_READ_EXCP_Q: kernel user\n");
      return (qcm_rc_invalid_command);
    }
    else
    {
      outputf("QCM_READ_EXCP_Q: application user\n");
      outputf("QCM_READ_EXCP_Q: exception_queue = %x\n",
	channel_id->exception_queue);
      if (QCM_EXCEPTION_QUEUE_IS_NOT_EMPTY(channel_id) == TRUE)
      {
	outputf("QCM_READ_EXCP_Q: queue is not empty\n");
	/*********************************************************************/
	/* There is at least one exception on the queue. Dequeue it and      */
	/* return it to the caller.                                          */
	/*********************************************************************/
	outputf("QCM_READ_EXCP_Q: lock channel\n");
	(void)lockl(&(channel_id->lock),0);
	*exc_ptr = channel_id->exception_queue->exception_data;
	tmp = channel_id->exception_queue;
	channel_id->exception_queue=
	  channel_id->exception_queue->next_pending_exception_ptr;
	if (channel_id->exception_queue != NULL)
	  channel_id->exception_queue->prev_pending_exception_ptr= NULL;
	xmfree(tmp, pinned_heap);
	outputf("QCM_READ_EXCP_Q: unlock channel\n");
	unlockl(&(channel_id->lock));
	return(qcm_rc_ok);
      }
      else
      {
	outputf("QCM_READ_EXCP_Q: there are no exceptions\n");
	/*********************************************************************/
	/* There is no exception for the user, simply return. Note that this */
	/* is not an error condition.                                        */
	/*********************************************************************/
	*exc_ptr = NULL;
	return(qcm_rc_ok);
      }
    }
  }
}

/*****************************************************************************/
/* Function     QCM_RECEIVE_DATA                                             */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
qcm_rc_type qcm_receive_data(

  channel_id_type   channel_id,
  gen_buffer_type  *buffer_ptr)
{
  qcm_rc_type    rc;
  int            user_rc;
  read_ext_type *ext_ptr;
  read_ext_type  ext_data;		/* defect 152196 */
  int            lock_rc;

  /***************************************************************************/
  /* Extra variables needed for special single-mbuf calls to kernel user     */
  /***************************************************************************/
  int               buf_data_size;
  gen_buffer_type  *new_buf;
  int               rdto;
  int               flags = 0;

  outputf("QCM_RECEIVE_DATA: called\n");
  if (channel_id->user_nature == kernel_user)
  {
    outputf("QCM_RECEIVE_DATA: kernel user\n");
    /*************************************************************************/
    /* Channel belongs to a kernel mode user.                                */
    /* Build the read extension from the qllc_header in the buffer.          */
    /*************************************************************************/
    ext_ptr=&ext_data;			/* defect 152196 */
    outputf("QCM_RECEIVE_DATA: building read extension from buffer\n");
    ext_ptr->user_sap_correlator = QBM_RETURN_USER_SAP_CORRELATOR(buffer_ptr);
    ext_ptr->user_ls_correlator = QBM_RETURN_USER_LS_CORRELATOR(buffer_ptr);
    ext_ptr->dlc_flags = QBM_RETURN_DLC_FLAGS(buffer_ptr);
    ext_ptr->dlh_length = QBM_RETURN_DLH_LENGTH(buffer_ptr);
    outputf("QCM_RECEIVE_DATA: dlh_length = %d\n",ext_ptr->dlh_length);
    /*************************************************************************/
    /* There is a special requirement (at least for Release 1) which is that */
    /* only a single mbuf can be passed up to the SNA code.                  */
    /* The next section of code gets a single mbuf big enough to take all    */
    /* the data and contiguously copies the chained buffers' data into the   */
    /* big buffer.                                                           */
    /* It then frees the chain of buffers and replaces them with the single  */
    /* buffer.                                                               */
    /*************************************************************************/
    outputf("QCM_RECEIVE_DATA: determine buffer size and rdto\n");
    if ((ext_ptr->dlc_flags & DLC_INFO) == DLC_INFO)
	buf_data_size = JSMBUF_LENGTH(buffer_ptr) - X25_OFFSETOF_USER_DATA;
    else
	buf_data_size = JSMBUF_LENGTH(buffer_ptr) - 
			OFFSETOF(body.qllc_body.user_data[0],x25_mbuf_t) + 
			ext_ptr->dlh_length;

    rdto = channel_id->port_id->dh_devinfo.un.x25.rdto;
    outputf("QCM_RECEIVE_DATA: buf data size = %d\n",buf_data_size);
    outputf("QCM_RECEIVE_DATA: rdto     = %d\n",rdto);
    /*************************************************************************/
    /* If there is more data than we can fit into a single big buffer, then  */
    /* truncate it and set the overflow flag in the read extension.          */
    /*************************************************************************/
    if (buf_data_size + rdto > CLBYTES)
    {
      outputf("QCM_RECEIVE_DATA: overflow occurred, truncating\n");
      buf_data_size = CLBYTES - rdto;
      ext_ptr->dlc_flags |= OFLO;
    }
    /*************************************************************************/
    /* Get a 4K page buffer                                                  */
    /*************************************************************************/
    outputf("QCM_RECEIVE_DATA: getting 4K page buffer\n");
    new_buf = QBM_GET_BUFFER((int)CLBYTES);
    if ( new_buf == (gen_buffer_type *)NULL)
    {
      QBM_FREE_BUFFER(buffer_ptr);
      return(qcm_rc_alloc_failed);
    }

    /*************************************************************************/
    /* Copy data from original buffer chain into new buffer                  */
    /*************************************************************************/
    outputf("QCM_RECEIVE_DATA: copying data into new 4K buffer\n");
    /*************************************************************************/
    /* There are two cases to consider when transferring data into the 4K    */
    /* buffer for a kernel user. One is the case of "normal" data, known as  */
    /* sequenced I frames, which do not have address and control fields, and */
    /* in which the data is aligned as for a user_data type of buffer body.  */
    /* This means that the data starts at the first byte after the packet    */
    /* data.                                                                 */
    /* The other case is XIDD data which is preceded by address and control  */
    /* fields so that the data is aligned at the third byte after the packet */
    /* data. It is therefore necessary to skip over the address and control  */
    /* fields and copy the data starting at qllc_body.user_data[0],unless the*/
    /* the user wants the dlc headers (address and control fields). Since    */
    /* there is currently no mechanism for a kernel user to request that the */
    /* headers are supplied, the data will be copied from the                */ 
    /* qllc_body.user_data[0] field, since ext_ptr->dlh_length is 0 because  */
    /* the QLM initialises it to 0. However, the second is treated in full   */
    /* so that if a mechanism is added at a later date, only the dlh_length  */
    /* field need be set, and the JSMBUF_READ_BLOCK statement will not need  */
    /* to be modified.                                                       */
    /*************************************************************************/
    if ((ext_ptr->dlc_flags & DLC_INFO) == DLC_INFO)
    {
      JSMBUF_READ_BLOCK(
        buffer_ptr,
        OFFSETOF(body.user_data[0],x25_mbuf_t),
        x25smbuf_address(new_buf,0) + rdto,
        buf_data_size
      );
    }
    else
    {
      /************************************************************************/
      /* This domain will correctly copy the data with or without dlc headers */
      /* dependent on the setting of dlh_length. A value of 0 will cause it   */
      /* skip the headers, and a value of 2 will make it include the headers. */
      /* Any value other than 0 or 2 is illegal.                              */
      /************************************************************************/
      JSMBUF_READ_BLOCK(
        buffer_ptr,
        OFFSETOF(body.qllc_body.user_data[0],x25_mbuf_t) - ext_ptr->dlh_length,
        x25smbuf_address(new_buf,0) + rdto,
        buf_data_size
      );
    }
    /*************************************************************************/
    /* Now set length and offset fields to skip over rdto.                   */
    /*************************************************************************/
    JSMBUF_ADJUST_FORWARD(new_buf,rdto);
    /*************************************************************************/
    /* Set length field on new buffer                                        */
    /*************************************************************************/
    outputf("QCM_RECEIVE_DATA: trimming dead space in buffer\n");
    JSMBUF_TRIM(new_buf,(int)(CLBYTES - (rdto + buf_data_size)));
    flags = QBM_RETURN_DLC_FLAGS(buffer_ptr);
    outputf("QCM_RECEIVE_DATA: flags = %x\n",flags);
    /*************************************************************************/
    /* Can now free old buffer chain                                         */
    /*************************************************************************/
    outputf("QCM_RECEIVE_DATA: free old buffer chain\n");
    QBM_FREE_BUFFER(buffer_ptr);
    /*************************************************************************/
    /* And replace the old buffers with the new one.                         */
    /*************************************************************************/
    buffer_ptr = new_buf;
    /*************************************************************************/
    /* So from now on the code is as if there was still a chain of mbufs.... */
    /*************************************************************************/

    /*************************************************************************/
    /* Decide what sort of data it is, and call the appropriate kernel func  */
    /*************************************************************************/
    if ((flags & NORM) == NORM)
    {
      outputf("QCM_RECEIVE_DATA: normal flag is set\n");
      /***********************************************************************/
      /* This is normal data. Call the user's rx_normal data function using  */
      /* the address held in the channel control block.                      */
      /***********************************************************************/
      ext_ptr->dlc_flags |= NORM;
      ext_ptr->dlc_flags &= ~XIDD;
      ext_ptr->dlc_flags &= ~NETD;
      /***********************************************************************/
      /* Now call the user's rx normal data function                         */
      /***********************************************************************/
      outputf("QCM_RECEIVE_DATA: call user's receive normal function\n");
      user_rc = (channel_id->rx_normal_data_fn_addr)(buffer_ptr,ext_ptr);
    }
    else if ( (flags & XIDD) == XIDD)
    {
      outputf("QCM_RECEIVE_DATA: xid data flag is set\n");
      /***********************************************************************/
      /* This is xid data. Call the user's rx_xid data function using        */
      /* the address held in the channel control block.                      */
      /***********************************************************************/
      ext_ptr->dlc_flags &= ~NORM;
      ext_ptr->dlc_flags |= XIDD;
      ext_ptr->dlc_flags &= ~NETD;
      /***********************************************************************/
      /* Now call the user's rx function                                     */
      /***********************************************************************/
      outputf("QCM_RECEIVE_DATA: call user's receive xidd function\n");
      user_rc = (channel_id->rx_xid_data_fn_addr)(buffer_ptr,ext_ptr);
    }
    else if ( (flags & NETD) == NETD)
    {
      outputf("QCM_RECEIVE_DATA: netd data flag is set\n");
      /***********************************************************************/
      /* This is network data. Call the user's rx_netd data function using   */
      /* the address held in the channel control block.                      */
      /***********************************************************************/
      ext_ptr->dlc_flags &= ~NORM;
      ext_ptr->dlc_flags &= ~XIDD;
      ext_ptr->dlc_flags |= NETD;
      /***********************************************************************/
      /* Now call the user's rx function                                     */
      /***********************************************************************/
      outputf("QCM_RECEIVE_DATA: call user's receive netd function\n");
      user_rc = (channel_id->rx_netd_data_fn_addr)(buffer_ptr,ext_ptr);
    }
    else
    {
      outputf("QCM_RECEIVE_DATA: no valid data flag is set\n");
      /***********************************************************************/
      /* None of the data indicator bits were set.                           */
      /***********************************************************************/
      QBM_FREE_BUFFER(buffer_ptr);
      rc = qcm_rc_invalid_flags;
      return (rc);
    }
    if (user_rc == DLC_FUNC_BUSY)
    {
      outputf("QCM_RECEIVE_DATA: user rc => busy\n");
      rc = qcm_rc_local_busy;
      return(rc);
    }
    else if (user_rc == DLC_FUNC_RETRY)
    {
      outputf("QCM_RECEIVE_DATA: user rc => retry\n");
      rc = qcm_rc_retry;
      return(rc);
    }
    else
    {
      outputf("QCM_RECEIVE_DATA: user rc => ok\n");
      /***********************************************************************/
      /* The rc from user is considered to have been OK                      */
      /***********************************************************************/
      rc = qcm_rc_ok;
      return(rc);
    }
  }
  else
  {
    outputf("QCM_RECEIVE_DATA: application mode user\n");
    /*************************************************************************/
    /* Channel belongs to an application mode user.                          */
    /* Queue the data in the channel, and if there was an outstanding select */
    /* selnotify the user.                                                   */
    /*************************************************************************/
    outputf("QCM_RECEIVE_DATA: lock channel\n");
    lock_rc = lockl(&(channel_id->lock),0);
    outputf("QCM_RECEIVE_DATA: queue data\n");
    QBM_ENQUE_BUFFER(buffer_ptr, &(channel_id->data_queue));
    /*************************************************************************/
    /* Provide general wakeup for sleeping (blocked) reads.                  */
    /*************************************************************************/
    e_wakeup((int *)&(channel_id->readsleep));
    /*************************************************************************/
    /* Check for outstanding selects on receive data. If you find one, then  */
    /* selnotify the user.                                                   */
    /*************************************************************************/
    if ((channel_id->select_status.selected_events & POLLIN) == POLLIN)
    {
      outputf("QCM_RECEIVE_DATA: selected events awaiting read data\n");
      /***********************************************************************/
      /* selnotify user                                                      */
      /***********************************************************************/
      outputf("QCM_RECEIVE_DATA: selnotify\n");
      selnotify(channel_id->devno, channel_id, POLLIN);
    }
    outputf("QCM_RECEIVE_DATA: unlock channel\n");
    unlockl(&(channel_id->lock));
    outputf("QCM_RECEIVE_DATA: return\n");
    return (qcm_rc_ok);
  }
}

/*****************************************************************************/
/* Function     qcm_select                                                   */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                available events is assume to be initialised. This is so   */
/*                that the calling routine can set any additional select     */
/*                events prior to calling channel, and they are not          */
/*                over-written                                               */
/*****************************************************************************/
qcm_rc_type qcm_select(
  channel_id_type        channel_id,
  unsigned short         selected_events,
  unsigned short        *available_events)
{
  outputf("QCM_SELECT: called\n");
  /***************************************************************************/
  /* Make sure the user is not a kernel user.                                */
  /***************************************************************************/
  if (channel_id->user_nature == kernel_user)
  {
    outputf("QCM_SELECT: kernel user\n");
    return (qcm_rc_invalid_user_type);
  }
  /***************************************************************************/
  /* May need to check for disastrous factors, such as Port being out of     */
  /* action, and return "available" to each of the select criteria so as to  */
  /* avoid hangups                                                           */
  /***************************************************************************/
  /***************************************************************************/
  /* Write select (POLLOUT) is checked by QDH.                               */
  /* Check Read select (POLLIN).                                             */
  /***************************************************************************/
  if ( (selected_events & POLLIN) == POLLIN)
  {
    outputf("QCM_SELECT: read selected\n");
    /*************************************************************************/
    /* User is interested in receive data availability                       */
    /*************************************************************************/
    if (channel_id->data_queue.first != NULL)
    {
      outputf("QCM_SELECT: data_queue isn't empty\n");
      /***********************************************************************/
      /* There is received data available on the channel queue, indicate its */
      /* availability by setting POLLOUT in the returned parameter.          */
      /***********************************************************************/
      outputf("QCM_SELECT: setting available events POLLIN\n");
      *(available_events) |= POLLIN;
    }
  }
  if ( (selected_events & POLLPRI) == POLLPRI)
  {
    outputf("QCM_SELECT: exceptions selected\n");
    /*************************************************************************/
    /* User interested in exceptions                                         */
    /*************************************************************************/
    if (QCM_EXCEPTION_QUEUE_IS_NOT_EMPTY(channel_id))
    {
      outputf("QCM_SELECT: exception_queue isn't empty\n");
      /***********************************************************************/
      /* set exception available bit in returned paramater                   */
      /***********************************************************************/
      *(available_events) |= POLLPRI;
    }
  }
  /***************************************************************************/
  /* Do nothing further. Device Head select code will take care of sync,etc. */
  /***************************************************************************/
  outputf("QCM_SELECT: returning\n");
  return(qcm_rc_ok);
}


/*****************************************************************************/
/* Function     qcm_make_result                                              */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
qcm_rc_type qcm_make_result (

  channel_id_type        channel_id,
  correlator_type        user_sap_correlator,
  correlator_type        user_ls_correlator,
  result_indicators_type result_indicators,
  result_code_type       result_code)
{

  qcm_rc_type rc;
  pending_exception_type *new_exc_ptr;
  pending_exception_type *pending_exc_ptr;
  struct dlc_getx_arg *result_block_ptr;

  /***************************************************************************/
  /* allocate memory for a result block                                      */
  /***************************************************************************/
  result_block_ptr = (struct dlc_getx_arg *)
    xmalloc(sizeof(struct dlc_getx_arg),WORD,pinned_heap);
  if (result_block_ptr == (struct dlc_getx_arg *)NULL)
  {
    rc = qcm_rc_alloc_failed;
  }
  else
  {
    result_block_ptr->user_sap_corr = user_sap_correlator;
    result_block_ptr->user_ls_corr = user_ls_correlator;
    result_block_ptr->result_ind = (unsigned int)result_indicators;
    result_block_ptr->result_code = (unsigned int)result_code;

    outputf("QCM_MAKE_RESULT: result_indicators = %x\n",result_indicators);
    outputf("QCM_MAKE_RESULT: result_code       = %d\n",result_code);
    /*************************************************************************/
    /* only valid for kernel users                                           */
    /*************************************************************************/
    if (channel_id->user_nature == kernel_user)
    {
      /***********************************************************************/
      /* The following call has been cast to void as the call is to kernel   */
      /* kernel user's exception function handler, which cannot return busy  */
      /* or retry indications, so is presumed to always be "OK".             */
      /***********************************************************************/
      (void)(* channel_id->exception_fn_addr)(result_block_ptr);
      rc = qcm_rc_ok;
      /***********************************************************************/
      /* Before being cast to a void, the rc from the user was checked, and  */
      /* return code from qcm to the qlm was set to either                   */
      /*   qcm_rc_local_busy,  qcm_rc_retry, or qcm_rc_ok.                   */
      /***********************************************************************/
    }
    else
    {
      /***********************************************************************/
      /* Channel belongs to an application mode user.                        */
      /***********************************************************************/
      (void)lockl(&(channel_id->lock),0);
      /***********************************************************************/
      /* queue exception structure in channel, from where it will be picked  */
      /* up by user via select.                                              */
      /***********************************************************************/
      /* first, find end of queue                                            */
      pending_exc_ptr = channel_id->exception_queue;
      if (pending_exc_ptr != NULL)
      {
	while (pending_exc_ptr->next_pending_exception_ptr != NULL)
	{
	  pending_exc_ptr = pending_exc_ptr->next_pending_exception_ptr;
	}
      }
      /***********************************************************************/
      /* pending_exc_ptr now pts to last exception in channel's exception    */
      /* queue alloc storage for a new pending_exception and add new exceptn */
      /* to end of queue                                                     */
      /***********************************************************************/
      new_exc_ptr = (pending_exception_type *)
	xmalloc(sizeof(struct pending_exception_type),WORD,pinned_heap);
      if (new_exc_ptr == NULL)
      {
	xmfree( result_block_ptr, pinned_heap );
	rc = qcm_rc_alloc_failed;
      }
      else /* allocate was successful                                        */
      {
	if (pending_exc_ptr == NULL)
	{
	  channel_id->exception_queue = new_exc_ptr;
	  new_exc_ptr->prev_pending_exception_ptr = NULL;
	}
	else
	{
	  pending_exc_ptr->next_pending_exception_ptr = new_exc_ptr;
	  new_exc_ptr->prev_pending_exception_ptr = pending_exc_ptr;
	}
	new_exc_ptr->next_pending_exception_ptr = NULL;
	new_exc_ptr->exception_data = result_block_ptr;
	rc = qcm_rc_ok;
      }

      /***********************************************************************/
      /* Unlock channel                                                      */
      /***********************************************************************/
      unlockl(&(channel_id->lock));
      /***********************************************************************/
      /* Check for outstanding selects on receive data. If you find one,     */
      /* selnotify the user and queue the data                               */
      /* If you don't find one, queue the data.                              */
      /***********************************************************************/
      if ((channel_id->select_status.selected_events & POLLPRI) == POLLPRI)
      {
	/*********************************************************************/
	/* selnotify user                                                    */
	/*********************************************************************/
	outputf("QCM_MAKE_RESULT: selnotify user\n");
	selnotify(channel_id->devno, channel_id, POLLPRI);
      }
    }
  }
  return(rc);
}

/*****************************************************************************/
/* Function     QCM_MAKE_SAPE_RESULT                                         */
/*                                                                           */
/* Description  This procedure allocates storage for an exception block that */
/*              will be used to send a SAP_Enabled (SAPE) result to the user */
/*              Depending on the type of user, there may be an additional    */
/*              pending_exception on the channel's exception queue on        */
/*              completion.                                                  */
/*                                                                           */
/* Return       There is no return code from this procedure                  */
/*                                                                           */
/* Parameters   channel_id_type  channel_id                                  */
/*              correlator_type  user_sap_correlator;                        */
/*              correlator_type  user_ls_correlator;                         */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
qcm_rc_type  qcm_make_sape_result (

  channel_id_type  channel_id,
  correlator_type  user_sap_correlator,
  correlator_type  user_ls_correlator,
  unsigned int     max_write_netd_length)
{
  qcm_rc_type              rc;
  pending_exception_type  *new_exc_ptr;
  pending_exception_type  *pending_exc_ptr;
  struct dlc_getx_arg     *result_block_ptr;

  /***************************************************************************/
  /* allocate memory for a result block                                      */
  /***************************************************************************/
  result_block_ptr = (struct dlc_getx_arg *)
    xmalloc(sizeof(struct dlc_getx_arg),WORD,pinned_heap);

  if (result_block_ptr == (struct dlc_getx_arg *)NULL)
  {
    rc = qcm_rc_alloc_failed;
  }
  else
  {

    result_block_ptr->user_sap_corr = user_sap_correlator;
    result_block_ptr->user_ls_corr = user_ls_correlator;
    result_block_ptr->result_ind = (unsigned int)sap_enabled;
    result_block_ptr->result_code = 0;
    ((struct dlc_sape_res *)result_block_ptr->result_ext)->max_net_send =
      max_write_netd_length;
    ((struct dlc_sape_res *)result_block_ptr->result_ext)->lport_addr_len = 0;
    /*************************************************************************/
    /* check whether applic or kernel user                                   */
    /*************************************************************************/
    if (channel_id->user_nature == kernel_user)
    {
      /***********************************************************************/
      /* The following call is cast to void as the call is to the kernel     */
      /* kernel user's exception function handler, which can't return busy or*/
      /* retry indications, so is presumed to always be "OK".                */
      /***********************************************************************/
      (void)(* channel_id->exception_fn_addr)(result_block_ptr);
      rc = qcm_rc_ok;
    }
    else
    {
      /***********************************************************************/
      /* Application user                                                    */
      /* Queue exception structure in channel, from where it will be picked  */
      /* up by kernel process managing port.                                 */
      /* this processing is the same whether the user is a kernel user or an */
      /* application user                                                    */
      /***********************************************************************/
      outputf("QCM_MAKE_SAPE_RESULT: lock channel\n");
      (void)lockl(&(channel_id->lock),0);
      outputf("QCM_MAKE_SAPE_RESULT: channel locked\n");
      /***********************************************************************/
      /* First, find end of queue                                            */
      /***********************************************************************/
      pending_exc_ptr = channel_id->exception_queue;

      if (pending_exc_ptr != NULL)
      {
	/*********************************************************************/
	/* List is already occupied, scan along to the end.                  */
	/*********************************************************************/
	while (pending_exc_ptr->next_pending_exception_ptr != NULL)
	{
	  pending_exc_ptr = pending_exc_ptr->next_pending_exception_ptr;
	}
      }
      /***********************************************************************/
      /* pending_exception_ptr now pts to last exception in chan's exception */
      /* queue, or queue itself if there are no elements already on it.      */
      /* Alloc storage for a new pending_exception and add exception to end  */
      /* of queue                                                            */
      /***********************************************************************/
      new_exc_ptr = (pending_exception_type *)
	xmalloc(sizeof(struct pending_exception_type),WORD,pinned_heap);
      if (new_exc_ptr == NULL)
      {
	xmfree( result_block_ptr, pinned_heap );
	rc = qcm_rc_alloc_failed;
      }
      else
      {
	/*********************************************************************/
	/* Allocate was successful                                           */
	/*********************************************************************/
	if (pending_exc_ptr == NULL)
	{
	  channel_id->exception_queue = new_exc_ptr;
	  new_exc_ptr->prev_pending_exception_ptr = NULL;
	}
	else
	{
	  pending_exc_ptr->next_pending_exception_ptr = new_exc_ptr;
	  new_exc_ptr->prev_pending_exception_ptr = pending_exc_ptr;
	}
	new_exc_ptr->next_pending_exception_ptr = NULL;
	new_exc_ptr->exception_data = result_block_ptr;
	rc = qcm_rc_ok;
      }
      /***********************************************************************/
      /* Unlock channel                                                      */
      /***********************************************************************/
      outputf("QCM_MAKE_SAPE_RESULT: unlock channel\n");
      unlockl(&(channel_id->lock));
      /***********************************************************************/
      /* Check for outstanding selects on receive data. If you find one,     */
      /* selnotify the user and queue the data                               */
      /* If you don't find one, queue the data.                              */
      /***********************************************************************/
      if ((channel_id->select_status.selected_events & POLLPRI) == POLLPRI)
      {
	outputf("QCM_MAKE_SAPE_RESULT: select pending\n");
	/*********************************************************************/
	/* selnotify user                                                    */
	/*********************************************************************/
	selnotify(channel_id->devno, channel_id, POLLPRI);
      }
    }
  }
  return(rc);
}


/*****************************************************************************/
/* Function     QCM_MAKE_STAS_RESULT                                         */
/*                                                                           */
/* Description  This procedure allocates storage for an exception block that */
/*              will be used to send a Station_Started (STAS) result to the  */
/*              user.                                                        */
/*              Depending on the type of user, there may be an additional    */
/*              pending_exception on the channel's exception queue on        */
/*              completion.                                                  */
/*                                                                           */
/* Return       There is no return code from this procedure                  */
/*                                                                           */
/* Parameters   channel_id_type channel_id                                   */
/*              correlator_type user_sap_correlator;                         */
/*              correlator_type user_ls_correlator;                          */
/*              unsigned int    maximum_i_field_size;                        */
/*              unsigned int    remote_station_id_length;                    */
/*              char           *remote_station_id;                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
qcm_rc_type  qcm_make_stas_result (

  channel_id_type  channel_id,
  correlator_type  user_sap_correlator,
  correlator_type  user_ls_correlator,
  unsigned int     maximum_i_field_size,
  unsigned int     remote_station_id_length,
  char            *remote_station_id)
{
  qcm_rc_type             rc;
  pending_exception_type *new_exception_ptr;
  pending_exception_type *pending_exception_ptr;
  struct dlc_getx_arg    *result_block_ptr;
  int                     lock_rc;

  /***************************************************************************/
  /* allocate memory for a result block                                      */
  /***************************************************************************/
  result_block_ptr = (struct dlc_getx_arg *)
    xmalloc(sizeof(struct dlc_getx_arg),WORD,pinned_heap);
  if (result_block_ptr == NULL)
  {
    rc = qcm_rc_alloc_failed;
  }
  else
  {
    result_block_ptr->user_sap_corr = user_sap_correlator;
    result_block_ptr->user_ls_corr = user_ls_correlator;
    result_block_ptr->result_ind = (unsigned int)station_started;
    result_block_ptr->result_code = 0;
    ((struct dlc_stas_res *)result_block_ptr->result_ext)->maxif =
      maximum_i_field_size;
    ((struct dlc_stas_res *)result_block_ptr->result_ext)->rport_addr_len = 0;
    ((struct dlc_stas_res *)result_block_ptr->result_ext)->rname_len =
      remote_station_id_length;
    strncpy (
      ((struct dlc_stas_res *)result_block_ptr->result_ext)->rname,
      remote_station_id,
      DLC_MAX_NAME
      );

    if (channel_id->user_nature == kernel_user)
    {
      /***********************************************************************/
      /* Use an offset of zero, and then xidd data can be accommodated by    */
      /* chaining in a new buffer as header (plus a and c fields).           */
      /***********************************************************************/
      ((struct dlc_stas_res *)result_block_ptr->result_ext)->max_data_off = 0;
    }
    else
      ((struct dlc_stas_res *)result_block_ptr->result_ext)->max_data_off = 0;

    /*************************************************************************/
    /* find out if user is applic or kernel                                  */
    /*************************************************************************/
    if (channel_id->user_nature == kernel_user)
    {
      /***********************************************************************/
      /* The following call has been cast to void as the call is to kernel   */
      /* kernel user's exception function handler, which can't return busy or*/
      /* retry indications, so is presumed to always be "OK".                */
      /***********************************************************************/
      (void)(* channel_id->exception_fn_addr)(result_block_ptr);
      /***********************************************************************/
      /* Before being cast to a void, the rc from the user was checked, and  */
      /* return code from qcm to the qlm was set to either                   */
      /*   qcm_rc_local_busy,  qcm_rc_retry, or qcm_rc_ok.                   */
      /***********************************************************************/
      rc = qcm_rc_ok;
    }
    else /* applic user                                                      */
    {
      /***********************************************************************/
      /* queue exception structure in channel, where it will be picked up    */
      /* by kernel process managing port.                                    */
      /* this processing is the same whether the user is a kernel user or an */
      /* application user                                                    */
      /***********************************************************************/
      /***********************************************************************/
      /* Lock channel                                                        */
      /***********************************************************************/
      (void)lockl(&(channel_id->lock),0);

      /* first, find end of queue                                            */
      pending_exception_ptr = channel_id->exception_queue;
      if (pending_exception_ptr != NULL)
      {
	while (pending_exception_ptr->next_pending_exception_ptr != NULL)
	{
	  pending_exception_ptr = 
	    pending_exception_ptr->next_pending_exception_ptr;
	}
      }
      /***********************************************************************/
      /* pending_exception_ptr now pts to last exception in chan's exception */
      /* queue alloc storage for pending_exception and add new exception     */
      /* to end of queue                                                     */
      /***********************************************************************/
      new_exception_ptr = (pending_exception_type *)
	xmalloc(sizeof(struct pending_exception_type),WORD, pinned_heap);
      if (new_exception_ptr == NULL)
      {
	xmfree( result_block_ptr, pinned_heap );
	rc = qcm_rc_alloc_failed;
      }
      else /* allocate was successful                                        */
      {
	if (pending_exception_ptr == NULL)
	{
	  channel_id->exception_queue = new_exception_ptr;
	  new_exception_ptr->prev_pending_exception_ptr=NULL;
	}
	else
	{
	  pending_exception_ptr->next_pending_exception_ptr=new_exception_ptr;
	  new_exception_ptr->prev_pending_exception_ptr=pending_exception_ptr;
	}
	new_exception_ptr->next_pending_exception_ptr = NULL;
	new_exception_ptr->exception_data = result_block_ptr;
	rc = qcm_rc_ok;
      }
      /***********************************************************************/
      /* Unlock channel                                                      */
      /***********************************************************************/
      unlockl(&(channel_id->lock));
      /***********************************************************************/
      /* Check for outstanding selects on receive data. If you find one,     */
      /* selnotify the user and queue the data                               */
      /* If you don't find one, queue the data.                              */
      /***********************************************************************/
      if ((channel_id->select_status.selected_events & POLLPRI) == POLLPRI)
      {
	/*********************************************************************/
	/* selnotify user                                                    */
	/*********************************************************************/
	outputf("QCM_MAKE_STAS_RESULT: selnotify to user\n");
	selnotify(channel_id->devno, channel_id, POLLPRI);
      }
    }
  }
  return (rc);
}

/*****************************************************************************/
/* Function     qcm_make_contention_result                                   */
/*                                                                           */
/* Description  This procedure allocates storage for an exception block that */
/*              will be used to send a Station_Halted (STAH) result to the   */
/*              user.                                                        */
/*              Contention is the only instance in which an extended STAH    */
/*              result is issued.                                            */
/*              Depending on the type of user, there may be an additional    */
/*              pending_exception on the channel's exception queue on        */
/*              completion.                                                  */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
qcm_rc_type qcm_make_contention_result (

  channel_id_type   channel_id,
  correlator_type   user_sap_correlator,
  correlator_type   user_ls_correlator,
  correlator_type   conflicting_user_ls_correlator)
{
  qcm_rc_type             rc;
  pending_exception_type  *new_exception_ptr;
  pending_exception_type *pending_exception_ptr;
  struct dlc_getx_arg    *result_block_ptr;

  /***************************************************************************/
  /* allocate memory for a result block                                      */
  /***************************************************************************/
  result_block_ptr = (struct dlc_getx_arg *)
    xmalloc(sizeof(struct dlc_getx_arg),WORD,pinned_heap);
  if (result_block_ptr == NULL)
  {
    rc = qcm_rc_alloc_failed;
  }
  else
  {
    result_block_ptr->user_sap_corr = user_sap_correlator;
    result_block_ptr->user_ls_corr = user_ls_correlator;
    result_block_ptr->result_ind = (unsigned int)station_halted;
    result_block_ptr->result_code
      = (unsigned long)remote_name_already_connected;               /* -936  */
    ((struct dlc_stah_res *)result_block_ptr->result_ext)->conf_ls_corr =
      conflicting_user_ls_correlator;

    /*************************************************************************/
    /* find out if user is applic/kernel                                     */
    /*************************************************************************/
    if (channel_id->user_nature == kernel_user)
    {
      /***********************************************************************/
      /* The following call has been cast to void as the call is to kernel   */
      /* kernel user's exception function handler, which can't return busy or*/
      /* retry indications, so is presumed to always be "OK".                */
      /***********************************************************************/
      (void)(* channel_id->exception_fn_addr)(result_block_ptr);
      /***********************************************************************/
      /* Before being cast to a void, the rc from the user was checked, and  */
      /* return code from qcm to the qlm was set to either                   */
      /*   qcm_rc_local_busy,  qcm_rc_retry, or qcm_rc_ok.                   */
      /***********************************************************************/
      rc = qcm_rc_ok;
    }
    else 
    {
      /***********************************************************************/
      /* User is an application mode user.                                   */
      /* Queue exception structure in channel, from where it will be pickedup*/
      /* by kernel process managing port.                                    */
      /***********************************************************************/
      (void)lockl(&(channel_id->lock),0);
      /***********************************************************************/
      /* first, find end of queue                                            */
      /***********************************************************************/
      pending_exception_ptr = channel_id->exception_queue;
      if (pending_exception_ptr != NULL)
      {
	while (pending_exception_ptr->next_pending_exception_ptr != NULL)
	{
	  pending_exception_ptr =
	    pending_exception_ptr->next_pending_exception_ptr;
	}
      }
      /***********************************************************************/
      /* pending_exception_ptr now pts to last exception in channel's exceptn*/
      /* queue alloc storage for a new pending_exception and add new exceptn */
      /* to end of queue                                                     */
      /***********************************************************************/
      new_exception_ptr = (pending_exception_type *)
	xmalloc(sizeof(struct pending_exception_type), WORD, pinned_heap);
      if (new_exception_ptr == NULL)
      {
	xmfree( result_block_ptr, pinned_heap );
	rc = qcm_rc_alloc_failed;
      }
      else /* allocate was successful                                        */
      {
	if (pending_exception_ptr == NULL)
	{
	  channel_id->exception_queue = new_exception_ptr;
	  new_exception_ptr->prev_pending_exception_ptr=NULL;
	}
	else
	{
	  pending_exception_ptr->next_pending_exception_ptr=new_exception_ptr;
	  new_exception_ptr->prev_pending_exception_ptr=pending_exception_ptr;
	}
	new_exception_ptr->next_pending_exception_ptr = NULL;
	new_exception_ptr->exception_data = result_block_ptr;
	rc = qcm_rc_ok;
      }
      /***********************************************************************/
      /* Unlock channel                                                      */
      /***********************************************************************/
      unlockl(&(channel_id->lock));
      /***********************************************************************/
      /* Check for outstanding selects on receive data. If you find one,     */
      /* selnotify the user and queue the data                               */
      /* If you don't find one, queue the data.                              */
      /***********************************************************************/
      if ((channel_id->select_status.selected_events & POLLPRI) == POLLPRI)
      {
	/*********************************************************************/
	/* selnotify user                                                    */
	/*********************************************************************/
	selnotify(channel_id->devno, channel_id, POLLPRI);
      }
    }
  }
  return (rc);
}



/*****************************************************************************/
/* Function     QCM_CHECK_CHANNEL                                            */
/*                                                                           */
/* Description  This procedure is called by the QSM when an Enable_SAP ioctl */
/*              is issued by the user. Its purpose is to provide the QSM     */
/*              with a Go/No-Go return code that indicates whether the       */
/*              channel can support a further SAP.                           */
/*                                                                           */
/*                                                                           */
/* Return   qcm_rc_type                                                      */
/*                                                                           */
/* Parameters channel_id_type           channel_id                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
qcm_rc_type qcm_check_channel(

  channel_id_type channel_id)
{
  unsigned short sap_count;
  sap_type *sap_ptr;
  bool was_locked;

  /***************************************************************************/
  /* Ensure that the channel_id is indeed the address of a channel.          */
  /***************************************************************************/
  if (channel_id->channel_id != channel_id)
  {
    outputf("QCM_CHECK_CHANNEL: invalid channel_id\n");
    return (qcm_rc_channel_not_found);
  }

  /***************************************************************************/
  /* Count the number of SAPs that are enabled on the channel.               */
  /***************************************************************************/
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);
  sap_ptr = channel_id->sap_list_ptr;
  if (sap_ptr == (sap_type *)NULL)
  {
    /*************************************************************************/
    /* trivial case, no saps are yet enabled                                 */
    /*************************************************************************/
    if (!was_locked) unlockl(&channel_list.lock);
    outputf("QCM_CHECK_CHANNEL: there are no saps yet.\n");
    return (qcm_rc_ok);
  }
  /***************************************************************************/
  /* We know that there is at least one sap already enabled on this channel. */
  /***************************************************************************/
  sap_count = 1;
  while (sap_ptr->next_sap_ptr != (sap_type *)NULL)
  {
    sap_count++;
    sap_ptr = sap_ptr->next_sap_ptr;
  }
  if (!was_locked) unlockl(&channel_list.lock);
  /***************************************************************************/
  /* sap_count now contains a count of the number of SAPs that are already   */
  /* enabled on the channel. If this is equal to the limit given by the value*/
  /* of max_saps in the channel, then return a bad return code. Else pass    */
  /* back a good return code.                                                */
  /***************************************************************************/
  outputf("QCM_CHECK_CHANNEL: existing sap_count is %d\n",sap_count);
  if (sap_count >= channel_id->maximum_saps)
  {
    outputf("QCM_CHECK_CHANNEL: channel's sap limit has been reached\n");
    return (qcm_rc_sap_limit_reached);
  }
  else
  {
    outputf("QCM_CHECK_CHANNEL: the new sap can be enabled\n");
    return (qcm_rc_ok);
  }
}

/*****************************************************************************/
/* Function    QCM_PORT_ERROR                                                */
/*                                                                           */
/* Description This function is for when the Port Mgr detects an error on a  */
/*             port due to the dh reporting some error condition. If the     */
/*             port's in trouble, all the channels saps and stations using   */
/*             that port must be shutdown. This will eventually close the    */
/*             port - when the last channel using it is closed.              */
/*****************************************************************************/
void qcm_port_error(
  port_type *port_id)
{
  int lock_rc;
  channel_type *channel_ptr;
  bool was_locked;

  outputf("QCM_PORT_ERROR: ...has been called\n");
  /***************************************************************************/
  /* Find all the channels that are using the port, and issue QCM_Close call */
  /* for each channel.                                                       */
  /* This function is only called in cases of severe error, when the port is */
  /* no longer in a useable condition. It is therefore not considered        */
  /* necessary to proceed along the channel list locking out a window of     */
  /* channels, to prevent interference with actions pending on channels.     */
  /* Instead, the list is locked and not released until the function has     */
  /* completed. It currently waits for locks on the channels it encounters   */
  /* along the list, but this may lead to deadlocks if those channels are    */
  /* waiting on events that can no longer occur because the port has gone    */
  /* down. If this proves to be a problem, this function may be made more    */
  /* drastic.                                                                */
  /***************************************************************************/
  was_locked = (lockl(&(channel_list.lock),0) == LOCK_NEST);
  channel_ptr = channel_list.channel_ptr;
  if (channel_ptr != NULL)
  {
    do
    {
      if (channel_ptr->port_id == port_id)
      {
	(void)qcm_close_channel(channel_ptr);
      }
    } while ((channel_ptr = channel_ptr->next_channel_ptr) != NULL);
  }
  outputf("QCM_PORT_ERROR: unlock channel list\n");
  outputf("QCM_PORT_ERROR: chan list lock = %x\n",channel_list.lock);
  if (!was_locked) unlockl(&(channel_list.lock));
  outputf("QCM_PORT_ERROR: chan list lock = %x\n",channel_list.lock);
  return;
}
