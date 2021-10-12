static char sccsid[] = "@(#)68  1.28  src/bos/kernext/dlc/qlc/qlcp.c, sysxdlcq, bos411, 9437B411a 9/13/94 09:42:31";
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: qpm_receive_function, qpm_transmit_function,
 *            qpm_exception_function, qpm_port_initialise, qpm_port_terminate,
 *            qpm_query_device, qpm_start, qpm_write, qpm_halt,
 *            qpm_interrupt_handler, qpm_enter_local_busy, qpm_exit_local_busy,
 *            qpm_reject, qpm_init_address, listening_netid, rm_listen_netid,
 *            add_listen_netid

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


#include "qlcg.h"
#include "qlcq.h"
#include "qlcv.h"            /* for x25_address_type                         */
#include "qlcpcons.h"        /* QLLC Port Manager Constructors               */
#include "qlcb.h"
#include "qlcp.h"
#include "qlcc.h"            /* channel list type                            */

/*****************************************************************************/
/* Reference global channel list declared in channel manager                 */
/*****************************************************************************/
extern channel_list_type channel_list;

/*****************************************************************************/
/* Function     qpm_receive_function                                         */
/*                                                                           */
/* Description                                                               */
/*               The read extension contains the netid, session_id           */
/*               and call_id, the last being valid in the case of            */
/*               incoming call packets.                                      */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*****************************************************************************/
void  qpm_receive_function (

  unsigned int          open_id,
  struct x25_read_ext  *dh_read_ext,
  gen_buffer_type      *buffer_ptr)
{
  struct port_type *port_id;
  boolean queue_empty;
  int flag;
  int lock_rc;

  /***************************************************************************/
  /* Determine which port this data is for.                                  */
  /***************************************************************************/
  port_id = (port_type *)open_id;
  /***************************************************************************/
  /* Verify that port_id really is a ptr to a port.                          */
  /***************************************************************************/
  if (port_id->port_id != port_id)
  {
    outputf ("QPM_RECEIVE_FUNCTION port_id does not point to a port \n");
    return;
  }
  /***************************************************************************/
  /* Port is valid                                                           */
  /***************************************************************************/
/* Defect 101380
  outputf("QPM_RX_FUNC: pid       = %x\n",u.u_procp->p_pid);
*/
/* <<< THREADS >>> */
  outputf("QPM_RX_FUNC: tid       = %x\n",thread_self ());
/* <<< end THREADS >>> */
/* End defect 101380 */

  outputf("QPM_RX_FUNC: port lock = %x\n",port_id->lock);
  lock_rc = lockl(&(port_id->lock),0);
  outputf("QPM_RX_FUNC: port lock = %x\n",port_id->lock);
  outputf("QPM_RX_FUNC: port locked\n");
  /***************************************************************************/
  /* The QLLC Read Extension is built in the space at the top of the buffer  */
  /* that was occupied by the GP header when the buffer was in the DH.       */
  /* The QLLC extension is needed to note the following items:               */
  /*                                                                         */
  /*    1. overflow status                                                   */
  /*    2. call_id in the case of incoming calls                             */
  /*    3. netid for routing of incoming data                                */
  /*    4. session_id                                                        */
  /*                                                                         */
  /* Other fields are assigned as the buffer works its way up through the    */
  /* QLLC layers.                                                            */
  /*                                                                         */
  /* The call_id and overflow status are stored inside the buffer to         */
  /* minimise the number of parameters that need to be passed around the     */
  /* QLLC functions                                                          */
  /*                                                                         */
  /* The netid is stored inside the buffer as well, as it is needed for QLLC */
  /* routing.                                                                */
  /***************************************************************************/
  outputf("QPM_RX_FUNC: buffer_ptr = %x\n",buffer_ptr);
  print_x25_buffer((char *)buffer_ptr);
  /***************************************************************************/
  /* Status is derived from the read_ext read_extension field (re).          */
  /* We must note whether the status word indicates that there has been an   */
  /* OVF or not.                                                             */
  /***************************************************************************/
  
  /***************************************************************************/
  /* Start with a clean set of flags.                                        */
  /***************************************************************************/
  /* flag = QBM_RETURN_DLC_FLAGS(buffer_ptr); */
  flag = 0;
  if (dh_read_ext->re.status == CIO_BUF_OVFLW)
  {
    outputf("QPM_RX_FUNC: buffer overflow indication set\n");
    /*************************************************************************/
    /* set oflo flag in buffer header                                        */
    /*************************************************************************/
    flag = flag | OFLO;
  }
  QBM_SET_DLC_FLAGS(buffer_ptr,flag);
  /***************************************************************************/
  /* Now get the netid, session_id and call_id into the buffer.              */
  /***************************************************************************/
  outputf("QPM_RX_FUNC: set buffer netid=%d\n",dh_read_ext->re.netid);
  QBM_SET_NETID(buffer_ptr,dh_read_ext->re.netid);
  outputf("QPM_RX_FUNC: set buffer session_id=%d\n",dh_read_ext->re.sessid);
  QBM_SET_SESSION_ID(buffer_ptr,dh_read_ext->re.sessid);
  outputf("QPM_RX_FUNC: set buffer callid=%d\n",dh_read_ext->call_id);
  QBM_SET_CALL_ID(buffer_ptr,dh_read_ext->call_id);
  /***************************************************************************/
  /* Note whether queue is empty. If so you post int handler. Otherwise don't*/
  /***************************************************************************/
  if (port_id->receive_data_queue.first == NULL)
  {
    outputf("QPM_RX_FUNC: data queue previously empty.\n");
    queue_empty = TRUE;
  }
  /***************************************************************************/
  /* Receive_function must queue data buf in port queue identified by open_id*/
  /* which is port_id (ptr to port), and post int handler for the port.      */
  /***************************************************************************/
  /***************************************************************************/
  /* Chain mbuf to queue using m_next if queue empty, or                     */
  /* using m_act if there is already a buffer/s in queue                     */
  /***************************************************************************/
  outputf("QPM_RX_FUNC: enqueue buffer on data queue\n");
  QBM_ENQUE_BUFFER(buffer_ptr,&(port_id->receive_data_queue));
  outputf("QPM_RX_FUNC: buffer queued\n");
  /***************************************************************************/
  /* Unlock port                                                             */
  /***************************************************************************/
  outputf("QPM_RX_FUNC: port lock = %x\n",port_id->lock);
  if (lock_rc != LOCK_NEST) unlockl(&(port_id->lock));
  outputf("QPM_RX_FUNC: port lock = %x\n",port_id->lock);
  outputf("QPM_RX_FUNC: port unlocked\n");
  /***************************************************************************/
  /* And post kproc if queue was empty                                       */
  /***************************************************************************/
  if (queue_empty == TRUE)
  {
    outputf("QPM_RX_FUNC: posting kproc with DATA_RECEIVED mask\n");
/* <<< THREADS >>> */
    et_post(DATA_RECEIVED, port_id->int_tid);
/* <<< end THREADS >>> */
  }
  outputf("QPM_RX_FUNC: returning\n");
  return;
}

/*****************************************************************************/
/* Function     qpm_transmit_function                                        */
/*                                                                           */
/* SUPPORT FOR TRANSMIT FUNCTION REMOVED 19th August 1989                    */
/*                                                                           */
/* Description                                                               */
/*     This function accepts an open_id, converts it into a port_id, and     */
/*    checks the port is valid. If it isn't it returns having logged an      */
/*    error.                                                                 */
/*                                                                           */
/* This function is responsible for posting the int handler that services    */
/* the port identified by the open_id parameter. It does nothing more.       */
/*                                                                           */
/* Buffers are stored on the transmit queue in raw form.                     */
/*  i.e. their dlc_io_ext has been removed, and they are ready to be sent    */
/*  to the dh.                                                               */
/*                                                                           */
/* TRANSMIT_AVAIL is an event mask understood by int handler and this        */
/* function, defined in qlcp.h.                                              */
/*                                                                           */
/* <<< THREADS >>>                                                           */
/* Post the interrupt handler serving the port via et_post, as follows:      */
/*      et_post(TRANSMIT_AVAIL,port_id->int_tid);                            */
/* <<< end THREADS >>>                                                       */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*****************************************************************************/
void  qpm_transmit_function (

  unsigned int open_id)
{
  struct port_type *port_id;

  /***************************************************************************/
  /* Determine which port this data                                          */
  /* is for.                                                                 */
  /***************************************************************************/
  port_id = (struct port_type *)open_id;
  /***************************************************************************/
  /* Verify that port_id really is a                                         */
  /* ptr to a port.                                                          */
  /***************************************************************************/
  if (port_id->port_id != port_id)
  {
    return;
  }
  /***************************************************************************/
  /* Port is valid                                                           */
  /***************************************************************************/
  /***************************************************************************/
  /* This function is responsible for posting the int handler that services  */
  /* the port identified by the open_id parameter. It does nothing more      */
  /***************************************************************************/
  /***************************************************************************/
  /* buffers are stored on the transmit queue in raw form.                   */
  /*  i.e. their dlc_io_ext has been removed, and they are ready to be sent  */
  /*  to the dh                                                              */
  /***************************************************************************/
  /***************************************************************************/
  /* TRANSMIT_AVAIL is an event mask understood by int handler and this      */
  /* function, defined in qlcp.h                                             */
  /***************************************************************************/
/* <<< THREADS >>> */
  et_post(TRANSMIT_AVAIL,port_id->int_tid);
/* <<< end THREADS >>> */
  return;
}

/*****************************************************************************/
/* Function     qpm_exception_function                                       */
/*                                                                           */
/* Description                                                               */
/*              This function is called by the device handler when it has a  */
/*              a status block to pass to QLLC.                              */
/*              This function locks the port queue that holds the status     */
/*              blocks. It queues the data, and posts QLLC's interrupt       */
/*              handler. It is important that the path length of this        */
/*              function is kept to a minimum, because it is executed on the */
/*              device handler's kproc that serves the adapter in use.       */
/*                                                                           */
/* Return       none                                                         */
/*                                                                           */
/* Parameters   open_id to identify port on which to queue status block.     */
/*              status block address                                         */
/*****************************************************************************/
void  qpm_exception_function (

  unsigned int open_id,
  struct status_block *status_block_ptr)
{
  struct port_type          *port_id;
  bool                       queue_empty;
  pending_status_block_type *pending_status_block_ptr;
  pending_status_block_type *end_of_list_ptr;
  int lock_rc;
 
  outputf("QPM_STAT_FUNC: called\n");
  /***************************************************************************/
  /* Determine which port this data is for.                                  */
  /***************************************************************************/
  port_id = (struct port_type *)open_id;
  /***************************************************************************/
  /* Verify that port_id really is a ptr to a port.                          */
  /***************************************************************************/
  if (port_id->port_id != port_id)
  {
    outputf("QLLC_STAT_FN: port unknown\n");
    return;
  }
  /***************************************************************************/
  /* Port is valid. Lock it.                                                 */
  /***************************************************************************/
/* Defect 101380
  outputf("QPM_STAT_FUNC: pid       = %x\n",u.u_procp->p_pid);
*/
/* <<< THREADS >>> */
  outputf("QPM_STAT_FUNC: tid       = %x\n",thread_self ());
/* <<< end THREADS >>> */
/* End defect 101380 */

  outputf("QPM_STAT_FUNC: port lock = %x\n",port_id->lock);
  lock_rc = lockl(&(port_id->lock),0);
  outputf("QPM_STAT_FUNC: port lock = %x\n",port_id->lock);
  outputf("QPM_STAT_FUNC: port locked\n");

  /***************************************************************************/
  /* Provide debug dump of status block contents                             */
  /***************************************************************************/
  outputf("...dump status block contents\n");
  (void)print_stat_block((char *)status_block_ptr);
  outputf("...done dump of status block contents\n");

  /***************************************************************************/
  /* Note whether the queue is empty. If so you'll have to post int handler. */
  /* Otherwise don't, because it will find the new status block as it        */
  /* empties the queue.                                                      */
  /***************************************************************************/
  outputf("QPM_STAT_FUNC: test for queue empty condition\n");

  if (port_id->exception_queue == NULL )
  {
    queue_empty = TRUE;
    outputf("QLLC_STAT_FUNC: queue was empty, will post\n");
  }
  /***************************************************************************/
  /* This function must take the status block and place it on the exception  */
  /* queue of the port identified by open_id parameter.                      */
  /* It does this by building a pending_status block, and intialising it,    */
  /* and chaining it into the queue.                                         */
  /***************************************************************************/
  outputf("...malloc pdg block\n");
  pending_status_block_ptr = (struct pending_status_block_type *)
    xmalloc(
      (unsigned int)sizeof(struct pending_status_block_type),
      WORD, 
      (caddr_t)pinned_heap);
  outputf("...malloc'd pdg block\n");

  if (pending_status_block_ptr == NULL)
  {
    /*************************************************************************/
    /* could not allocate a pending status block structure                   */
    /*************************************************************************/
    outputf("QLLC_STAT_FUNC: couldn't malloc space for pdg stat blk\n");
    if (lock_rc != LOCK_NEST) unlockl(&(port_id->lock));
    outputf("2a) port_id->lock = 0x%x, unlocked\n", port_id->lock);
    return;
  }

  /***************************************************************************/
  /* Initialise pending block                                                */
  /***************************************************************************/
  outputf("QLLC_STAT_FUNC: initialising pdg stat blk\n");
  pending_status_block_ptr->block = *status_block_ptr;
  outputf("QLLC_STAT_FUNC: initialised pdg stat blk\n");

  if (queue_empty == TRUE )
  {
    outputf("QLLC_STAT_FUNC: queue was empty\n");
    /*************************************************************************/
    /* Queue is empty. Now queue new block, and post int handler.            */
    /* EXCEPTION_ARRIVED is an event mask understood by int handler          */
    /*************************************************************************/
    port_id->exception_queue = pending_status_block_ptr;
    pending_status_block_ptr->next_block_ptr = NULL;
    outputf("QPM_STAT_FUNC: port lock = %x\n",port_id->lock);
    if (lock_rc != LOCK_NEST) unlockl(&(port_id->lock));
    outputf("QPM_STAT_FUNC: port lock = %x\n",port_id->lock);
/* <<< THREADS >>> */
    outputf("QPM_STAT_FUNC: post qllc kproc, tid=%d\n",port_id->int_tid);
    et_post(EXCEPTION_ARRIVED, port_id->int_tid);
/* <<< end THREADS >>> */
    return;
  }

  outputf("QLLC_STAT_FUNC: Queue not empty, adding blk, no post\n");
  /***************************************************************************/
  /* Queue is not empty. Find end of exception queue.                        */
  /***************************************************************************/
  end_of_list_ptr = port_id->exception_queue;
  while (end_of_list_ptr->next_block_ptr != NULL)
    end_of_list_ptr = end_of_list_ptr->next_block_ptr;
  /***************************************************************************/
  /* end_of_list_ptr now points to pending status block on end of exception  */
  /* queue.                                                                  */
  /* Add the new pending block                                               */
  /***************************************************************************/
  outputf("QLLC_STAT_FUNC: have found end of list\n");
  end_of_list_ptr->next_block_ptr = pending_status_block_ptr;
  pending_status_block_ptr->prev_block_ptr = end_of_list_ptr;
  pending_status_block_ptr->next_block_ptr = NULL;
  outputf("QPM_STAT_FUNC: port lock = %x\n",port_id->lock);
  if (lock_rc != LOCK_NEST) unlockl(&(port_id->lock));
  outputf("QPM_STAT_FUNC: port lock = %x\n",port_id->lock);
  return;
}

/*****************************************************************************/
/* Function     qpm_port_initialise                                          */
/*                                                                           */
/* Description  Manages the addition of a port to the port list, or the      */
/*              addition of a user to an existing port.                      */
/*                                                                           */
/*                                                                           */
/* Return       qpm_rc_ok               - satisfactory completion            */
/*              qpm_rc_alloc_failed     - could not get control block        */
/*              qpm_rc_creatp_failed    - could not start int handler        */
/*              qpm_rc_initp_failed     - could not init int handler         */
/*              qpm_rc_open_failed      - open to device handler failed      */
/*              qpm_rc_ioctl_failed     - iocinfo ioctl after open failed    */
/*              qpm_rc_pin_failed       - pincode failed before open         */
/*                                                                           */
/* Parameters   port_type **port_id   ptr to port returned on allocation     */
/*              char      *path       name of dh port                        */
/*                                                                           */
/*****************************************************************************/
qpm_rc_type qpm_port_initialise(

  struct port_type **port_id,                                   /* returned  */
  char              *path)
{
  qpm_rc_type                  rc;
  port_type                   *port_ptr;     /* temporary port list ptr      */
  struct kopen_ext             open_ext;     /* structure to use on devopen  */
  pid_t                        int_pid;      /* proc id of int handler       */
  qlc_intrpt_hndlr_init_data_t init_parms;
  int                          dh_rc;
  int port_lock_rc, list_lock_rc;
/* <<< defect THREAD 2 >>> */
  int kproc_failed;
/* <<< end defect THREAD 2 >>> */

  /***************************************************************************/
  /* Lock channel, and keep it locked until init procedure is complete. If   */
  /* the open goes bad half-way, this procedure needs to back out of the     */
  /* changes it makes to the list.                                           */
  /***************************************************************************/
  outputf("init start .. state of list lock is %x\n",channel_list.lock);
  list_lock_rc = lockl(&(channel_list.lock),0);
  outputf("init start .. state of list lock is %x\n",channel_list.lock);

  /***************************************************************************/
  /* Find out whether the port is already open.                              */
  /* This is determined by searching the list and looking for a port         */
  /* instance with the same device no as that passed as arg.                 */
  /***************************************************************************/

  port_ptr = channel_list.port_ptr;      /* port_list_ptr global port anchor */
  outputf("init start .. state of port ptr is %x\n",channel_list.port_ptr);

  if (port_ptr != NULL)
  {
    /*************************************************************************/
    /* The list has at least one port on it.                                 */
    /* Look for wanted port. We need a copy with at least 1 user else its    */
    /* in the process of being deleted.                                      */
    /*************************************************************************/
    while (
      port_ptr->next_port_ptr != NULL && 
      (strcmp(path,port_ptr->xdh_pathname) != 0 ||
       port_ptr->user_count < 1))
      port_ptr = port_ptr->next_port_ptr;
  }

  /***************************************************************************/
  /* port_ptr now pts to end of port list (NULL => empty list), or port that */
  /* is wanted. If devno /= devno then this isn't the port that is wanted,   */
  /* so it must be the last port in the list.                                */
  /* Add a new port to the list                                              */
  /***************************************************************************/

  if (port_ptr != NULL && strcmp(path,port_ptr->xdh_pathname) == 0 &&
	port_ptr->user_count > 0)
  {
    /*************************************************************************/
    /* Port already exists                                                   */
    /*************************************************************************/
    outputf("QPM_PORT_INIT: existing port being modified...\n");

    /*************************************************************************/
    /* Add one to the user_count for this port                               */
    /*************************************************************************/
    port_ptr->user_count++;

    /*************************************************************************/
    /* Return address of port as port_id                                     */
    /*************************************************************************/
    *port_id = port_ptr;

    /*************************************************************************/
    /* Unlock list                                                           */
    /*************************************************************************/
    if (list_lock_rc != LOCK_NEST) unlockl(&(channel_list.lock));
    rc = qpm_rc_ok;
    return (rc);
  }

  /***************************************************************************/
  /* The port does not already exist.  Allocate storage for a port.          */
  /***************************************************************************/
  outputf("QPM_PORT_INIT: allocate a new control block\n");

  *port_id = (port_type *)xmalloc(
    (unsigned int)sizeof(port_type),
    WORD,
    (caddr_t)pinned_heap);

  if ( *port_id == NULL )
  {
    rc = qpm_rc_alloc_failed;
    if (list_lock_rc != LOCK_NEST) unlockl(&(channel_list.lock));
    return (rc);
  }
  /***************************************************************************/
  /* Port has been successfully allocated.                                   */
  /***************************************************************************/
  outputf("QPM_PORT_INIT: port allocated.\n");
  (*port_id)->lock = LOCK_AVAIL;

  /***************************************************************************/
  /* Add it to the end of the port list. Remember that port_ptr pts to end   */
  /* of list.                                                                */
  /***************************************************************************/
  if ( port_ptr == NULL )              /* This is the first port in the list */
  {
    channel_list.port_ptr = *port_id;
    (*port_id)->prev_port_ptr = NULL;
  }
  else    
  {
    /*************************************************************************/
    /* Otherwise, add it on to the end of the list                           */
    /*************************************************************************/
    port_ptr->next_port_ptr = *port_id;
    (*port_id)->prev_port_ptr = port_ptr;
  }
  (*port_id)->next_port_ptr = NULL;

  /***************************************************************************/
  /* Port is now added to end of list                                        */
  /* port_ptr still is either NULL (=> one new port in list) or is !NULL (=> */
  /* there is more than one port in the list (the new one has been added to  */
  /* the end).                                                               */
  /***************************************************************************/

  /***************************************************************************/
  /* Perform preliminary initialisation so that qpm_port_terminate can be    */
  /* used if the creatp, initp, open, etc. go wrong.                         */
  /***************************************************************************/
  (*port_id)->port_id = (*port_id);
  (*port_id)->user_count = 0;

  /***************************************************************************/
  /* And perform further initialisation                                      */
  /***************************************************************************/
  strcpy((*port_id)->xdh_pathname,path);
  
  /***************************************************************************/
  /* We must now create the Interrupt Handler which will service this port.  */
  /***************************************************************************/

  int_pid = creatp();

  outputf("QPM_INIT: created qllc kproc with pid=%d\n",int_pid);

  (*port_id)->int_pid = int_pid;

  if ( int_pid == (pid_t)(-1) )
  {
    /*************************************************************************/
    /* Remove port from list, and free it                                    */
    /*************************************************************************/
    /*************************************************************************/
    /* As the port has just been newly created you know it's at the end of   */
    /* the list, and that port_ptr still points to the port which was the    */
    /* last before you added the new one, or is NULL if only one port.       */
    /*************************************************************************/
    if (port_ptr == NULL)
    {
      /***********************************************************************/
      /* there is only this one port in the list                             */
      /***********************************************************************/
      channel_list.port_ptr = NULL;
    }
    else
    {
      /***********************************************************************/
      /* there is more than one port in the list...                          */
      /***********************************************************************/
      port_ptr->next_port_ptr = NULL;
    }
    if (list_lock_rc != LOCK_NEST) unlockl(&(channel_list.lock));
    (void)xmfree((caddr_t)*port_id, (caddr_t)pinned_heap);
    rc = qpm_rc_creatp_failed;
    return (rc);
  }

  /***************************************************************************/
  /* int_pid is process_id of int handler process.                           */
  /***************************************************************************/
  /***************************************************************************/
  /* The init parms passed to the int handler contain the port_id of the     */
  /* port being opened. The value 4 is the byte length of the port_id        */
  /***************************************************************************/

  /***************************************************************************/
  /* Invent a process name for this kproc                                    */
  /***************************************************************************/
  strncpy((*port_id)->kproc_process_name,"qllc",4);
  sprintf(&((*port_id)->kproc_process_name[4]),"%c",
    (*port_id)->xdh_pathname[9]);
  sprintf(&((*port_id)->kproc_process_name[5]),"%c",'\0');

  outputf("QPM_INIT_PORT: name of kproc = %s\n",
    (*port_id)->kproc_process_name);

  /***************************************************************************/
  /* And initialise kproc, set priority to 38                                */
  /***************************************************************************/
  init_parms.port_id = *port_id;

/* <<< THREADS 2 >>> */
  kproc_failed = FALSE;

  if (initp( int_pid, qpm_interrupt_handler, &init_parms,4,
		(*port_id)->kproc_process_name) != 0)
  {
    kproc_failed = TRUE;
  }
  else
  {
    /* initp worked ok, so force a reschedule so that the thread id can
       be obtained by the kproc */
    delay (10);

    /* attempt to set the kproc's priority */

    if /* setpri failed */
       (setpri( int_pid, 38 ) < 0)
    {
      kproc_failed = TRUE;

      /***********************************************************************/
      /* Kill process you just created                                       */
      /***********************************************************************/
      et_post(TERMINATION,(*port_id)->int_tid);
    }
  } 

  if /* initp or setpri failed */
     (kproc_failed == TRUE)
  { 
/* <<< end THREADS 2 >>> */

    /*************************************************************************/
    /* As the port has just been newly created you know it's at the end of   */
    /* the list, and that port_ptr still points to the port which was the    */
    /* last before you added the new one, or is NULL if only one port.       */
    /*************************************************************************/
    if (port_ptr == NULL)
    {
      /***********************************************************************/
      /* there is only this one port in the list                             */
      /***********************************************************************/
      channel_list.port_ptr = NULL;
    }
    else
    {
      /***********************************************************************/
      /* there is more than one port in the list...                          */
      /***********************************************************************/
      port_ptr->next_port_ptr = NULL;
    }
    if (list_lock_rc != LOCK_NEST) unlockl(&(channel_list.lock));
    (void)xmfree((caddr_t)*port_id, (caddr_t)pinned_heap);
    rc = qpm_rc_initp_failed;
    return (rc);
  }

  /***************************************************************************/
  /* Now issue open to DH                                                    */
  /* Start by initialising open_ext                                          */
  /***************************************************************************/
  open_ext.rx_fn = qpm_receive_function;
  open_ext.tx_fn = qpm_receive_function;
  open_ext.stat_fn = qpm_exception_function;
  /***************************************************************************/
  /* port_id is used as open_id which is correlator passed to receive_fn,    */
  /* and exception_fn, so that they know which port a call applies to.       */
  /***************************************************************************/
  open_ext.open_id = (unsigned long)*port_id;

  outputf("QPM_PORT_INIT: about to open dh....\n");
  outputf("path = %s\n",path);
  
  dh_rc = fp_open(
    path,
    (DKERNEL | DNDELAY | O_RDWR),
    NULL,
    &open_ext,
    FP_SYS,
    &((*port_id)->fp)
    );

  outputf("QPM_PORT_INIT: x25sopen rc = %d\n",dh_rc);
  outputf("QPM_PORT_INIT: fp returned = %x\n",(*port_id)->fp);

  if (dh_rc != 0)
  {
    /*************************************************************************/
    /* Kill process you just created, tidy up list, and free port.           */
    /*************************************************************************/
/* <<< THREADS >>> */
    et_post(TERMINATION,(*port_id)->int_tid);
/* <<< end THREADS >>> */
    /*************************************************************************/
    /* As the port has just been newly created you know it's at the end of   */
    /* the list, and that port_ptr still points to the port which was the    */
    /* last before you added the new one, or is NULL if only one port.       */
    /*************************************************************************/
    if (port_ptr == NULL)
    {
      /***********************************************************************/
      /* there is only this one port in the list                             */
      /***********************************************************************/
      channel_list.port_ptr = NULL;
    }
    else
    {
      /***********************************************************************/
      /* there is more than one port in the list...                          */
      /***********************************************************************/
      port_ptr->next_port_ptr = NULL;
    }
    if (list_lock_rc != LOCK_NEST) unlockl(&(channel_list.lock));
    (void)xmfree((caddr_t)*port_id, (caddr_t)pinned_heap);
    rc = qpm_rc_open_failed;
    return (rc);
  }

  /***************************************************************************/
  /* Open to DH was successful                                               */
  /* Initialise the new port                                                 */
  /***************************************************************************/
  /* (*port_id)->port_id = *port_id; */
  /* (*port_id)->dh_devno = dh_devno; */

  (*port_id)->receive_data_queue.first = NULL;
  (*port_id)->receive_data_queue.last = NULL;
  (*port_id)->exception_queue = NULL;
  (*port_id)->listen_netids = (struct listen_netid_type *)NULL; /*def 156503 */


  /***************************************************************************/
  /* Set user_count to 1, as port has only just been opened                  */
  /***************************************************************************/
  (*port_id)->user_count = 1;

  /***************************************************************************/
  /* As this is a new port, we must initialise the dh ddi defined fields.    */
  /***************************************************************************/

  /***************************************************************************/
  /* Initialise devinfo structure held in port                               */
  /***************************************************************************/
  /*  (*port_id)->dh_devinfo.un.x25.support_level = 1984; */

  dh_rc = fp_ioctl(
    (*port_id)->fp,
    IOCINFO,
    &((*port_id)->dh_devinfo),
    NULL
    );

  outputf("DH iocinfo ioctl rc is %d\n",dh_rc);
  if (dh_rc != 0)
  {
    /*************************************************************************/
    /* Kill process you just created, tidy up list, and free port.           */
    /*************************************************************************/
/* <<< THREADS >>> */
    et_post(TERMINATION,(*port_id)->int_tid);
/* <<< end THREADS >>> */
    /*************************************************************************/
    /* As the port has just been newly created you know it's at the end of   */
    /* the list, and that port_ptr still points to the port which was the    */
    /* last before you added the new one, or is NULL if only one port.       */
    /*************************************************************************/
    if (port_ptr == NULL)
    {
      /***********************************************************************/
      /* there is only this one port in the list                             */
      /***********************************************************************/
      channel_list.port_ptr = NULL;
    }
    else
    {
      /***********************************************************************/
      /* there is more than one port in the list...                          */
      /***********************************************************************/
      port_ptr->next_port_ptr = NULL;
    }
    if (list_lock_rc != LOCK_NEST) unlockl(&(channel_list.lock));
    (void)xmfree((caddr_t)*port_id, (caddr_t)pinned_heap);
    return(qpm_rc_ioctl_failed);
  }

  /***************************************************************************/
  /* And finally unlock channel list                                         */
  /***************************************************************************/
  outputf("state of list lock is %x\n",channel_list.lock);
  if (list_lock_rc != LOCK_NEST) unlockl(&(channel_list.lock));
  outputf("state of list lock is %x\n",channel_list.lock);
  rc = qpm_rc_ok;
  return(rc);
}

/*****************************************************************************/
/* Function     qpm_port_terminate                                           */
/*                                                                           */
/* Description                                                               */
/*              Normally used when a channel is being closed, this procedure */
/*              takes care of the port. It may close the port if there was   */
/*              only one user, or it will leave the port intact and simply   */
/*              disassociate the user from the port if there are still other */
/*              users of the port.                                           */
/*                                                                           */
/*              This procedure may be used to remove a port that has         */
/*              encountered an error during initialisation. However, if the  */
/*              error occurs BEFORE the port has been validated (i.e. had    */
/*              its port_id field initialised), or BEFORE the user_count     */
/*              has been initialised to ZERO, then this procedure should     */
/*              not be used. This is because it has to check validity and    */
/*              and user_count <= 1 in order to determine whether it should  */
/*              shutdown a port or merely disassociate a user, when called   */
/*              under a normal close situation.                              */
/*              Therefore if port initialisation fails at such an early      */
/*              stage the port must be shutdown manually.                    */
/*                                                                           */
/* Return       qpm_rc_port_not_found      - validity check failed           */
/*                                           or list has been corrupted      */
/*              qpm_rc_ok                  - port closed or user             */
/*                                           disassociated from port if      */
/*                                           there were other user/s         */
/*              qpm_rc_close_failed        - dev_close call failed           */
/*              qpm_rc_free_failed         - could not free storage          */
/*                                                                           */
/* Parameters   address of port to be terminated                             */
/*                                                                           */
/*****************************************************************************/
qpm_rc_type qpm_port_terminate (

  port_type *port_id)
{
  qpm_rc_type rc = qpm_rc_ok;
  port_type  *port_ptr;
  boolean     port_found;
  int         list_lock_rc, port_lock_rc;
  pending_status_block_type *tmp_exc_ptr;
/* defect 151120 */
  struct status_block *exception_ptr;
/* end defect 151120 */
  gen_buffer_type *buffer_ptr;
  int dh_rc;

  outputf("QPM_PORT_TERMINATE: entering\n");
  outputf("QPM_PORT_TERMINATE: port_id = %x\n",port_id);
  /***************************************************************************/
  /* Perform validity checks on port_id passed to this procedure.            */
  /***************************************************************************/
  if (port_id->port_id != port_id)
  {
    outputf("QPM_PORT_TERMN: port_id invalid\n");
    rc = qpm_rc_port_not_found;
    return (rc);
  }
  /***************************************************************************/
  /* The port is valid. Lock it.                                             */
  /***************************************************************************/
  outputf("QPM_PORT_TERMINATE port_id->lock =   %x\n", port_id->lock);
  port_lock_rc = lockl(&(port_id->lock),0);
  outputf("QPM_PORT_TERMINATE: port_id->lock = %x\n", port_id->lock);
  /***************************************************************************/
  /* Must find out if any other users are on this port. If there are, then   */
  /* don't terminate it. If there is only one user, then you can terminate   */
  /* it. Test user count.                                                    */
  /***************************************************************************/
  port_id->user_count--;
  if (port_id->user_count > 0)
  {
    outputf("QPM_PORT_TERMN: decrementing user count: was %d, now %d\n",
      port_id->user_count,port_id->user_count-1);
    rc = qpm_rc_ok;
    if (port_lock_rc != LOCK_NEST) unlockl(&(port_id->lock));
    outputf("3a) port_id->lock = 0x%x, unlocked\n", port_id->lock);
    return (rc);
  }
  /***************************************************************************/
  /* There is only one user, or the port was never fully opened.             */
  /* The port can be shutdown                                                */
  /***************************************************************************/
  /***************************************************************************/
  /* Issue close to device handler                                           */
  /* Whether close is successful or not free up the resources, as they will  */
  /* be of no further value. On close error, the device handler does all it  */
  /* can to clean up its resources, and there is nothing more the user can   */
  /* do to clean up, so the port is freed whether close is good or bad.      */
  /***************************************************************************/
  outputf("QPM_PORT_TERMN: port_id->fp = %d\n", port_id->fp);
  /***************************************************************************/
  /* Unlock port before issuing close because the close will flush out any   */
  /* pending stat blocks, so stat function must be able to get the lock.     */
  /***************************************************************************/
  unlockl(&(port_id->lock));
  outputf("QPM_PORT_TERMINATE: port_id->lock = %x\n", port_id->lock);
  outputf("fp used for close = %x\n",port_id->fp);
  dh_rc = fp_close(port_id->fp);
  if (dh_rc != 0)
  {
    outputf("QPM_PORT_TERMN: devclose failed, rc=%d\n",dh_rc);
    rc = qpm_rc_close_failed;
    /*************************************************************************/
    /* ..don't return, continue with the port termination.                   */
    /*************************************************************************/
  }
  (void) lockl(&(port_id->lock),0);
  outputf("4.1) port_id->lock = 0x%x, locked\n", port_id->lock);
  /***************************************************************************/
  /* Kill interrupt handler serving the port.                                */
  /***************************************************************************/
/* <<< THREADS >>> */
  et_post(TERMINATION,port_id->int_tid);
/* <<< end THREADS >>> */
  /* psignal (port_id->int_pid, SIGKILL); */
  /* outputf("psignal = SIGKILL\n");      */

  /***************************************************************************/
  /* Free up any resources owned by the port.                                */
  /*  -- buffers on receive queue                                            */
  /*  -- exceptions on status queue                                          */
  /*                                                                         */
  /* The whole port is locked throughout this cleaning up operation.         */
  /***************************************************************************/
  outputf("QPM_PORT_TERMINATE: free up receive data buffers queued in port\n");
  while (port_id->receive_data_queue.first != NULL)
  {
    buffer_ptr = QBM_DEQUE_BUFFER(&(port_id->receive_data_queue));
    QBM_FREE_BUFFER(buffer_ptr);
  }
  outputf("port_id->receive_data_queue is now empty\n");

  outputf("QPM_PORT_TERMINATE: free up stat blocks queued in port\n");
  while (port_id->exception_queue != NULL)
  {
    tmp_exc_ptr = port_id->exception_queue;
    port_id->exception_queue = port_id->exception_queue->next_block_ptr;
    if (port_id->exception_queue != NULL)
      port_id->exception_queue->prev_block_ptr = NULL;

/* defect 151120 */
    exception_ptr = &(tmp_exc_ptr->block);
    switch (exception_ptr->code)
        {
        case CIO_START_DONE :
          outputf("QPM_PORT_TERMINATE: START_DONE status\n");
          /* if an mbuf is indicated in option[2] then free it */
          if (exception_ptr->option[2] != NULL)
            outputf("QPM_PORT_TERMINATE: Free buffer %8x\n",exception_ptr->option[2]);
            QBM_FREE_BUFFER(exception_ptr->option[2]);
          break;
        case CIO_HALT_DONE :
          outputf("QPM_PORT_TERMINATE: HALT_DONE status\n");
          /* if an mbuf is indicated in option[2] then free it */
          if (exception_ptr->option[2] != NULL)
            outputf("QPM_PORT_TERMINATE: Free buffer %8x\n",exception_ptr->option[2]);
            QBM_FREE_BUFFER(exception_ptr->option[2]);
          break;
        default :
          /* just fall through since no other exceptions should carry mbufs */
          break;
        } /* end of switch */
/* end defect 151120 */

    xmfree((caddr_t)tmp_exc_ptr,(caddr_t)pinned_heap);
  }
  outputf("port_id->exception_queue == NULL\n");
  /***************************************************************************/
  /* Must note position of port in linked port list, so that list can be     */
  /* repaired.                                                               */
  /***************************************************************************/
  /***************************************************************************/
  /* There are three conditions which must be catered for.                   */
  /*   1) The port is at the start of the list.                              */
  /*   2) The port is in the middle of the list.                             */
  /*   3) The port is at the end of the list.                                */
  /*                                                                         */
  /* The algorithm used is as follows:                                       */
  /***************************************************************************/
  if (port_lock_rc != LOCK_NEST) unlockl(&(port_id->lock));
  outputf("4?a) port_id->lock = 0x%x, unlocked\n", port_id->lock);
  /***************************************************************************/
  /* Lock list                                                               */
  /***************************************************************************/
  list_lock_rc = lockl(&(channel_list.lock),0);
  if (channel_list.port_ptr == NULL)
  {
    outputf("QPM_PORT_TERMN: port list is empty\n");
    /*************************************************************************/
    /* Trivial case - list is empty                                          */
    /*************************************************************************/
    if (list_lock_rc != LOCK_NEST) unlockl(&(channel_list.lock));
    return(qpm_rc_port_not_found);
  }
  else
  {
    outputf("QPM_PORT_TERMINATE: remove port from list\n");
    /*************************************************************************/
    /* List is not empty - that's a good start!                              */
    /*************************************************************************/
    port_ptr = channel_list.port_ptr;
    port_found = FALSE;
    while (port_ptr != NULL && port_found == FALSE)
    {
      if (port_ptr == port_id)
      {
	outputf("QPM_PORT_TERMN: found port in list. remove it\n");

	/* lock port just to make sure no one else has it */
	/* (they can't get it without getting list lock, which we have, but */
	/* someone might already have been holding its lock) */
        lockl(&(port_id->lock),0);
	unlockl(&(port_id->lock));
	port_found = TRUE;
	if (port_ptr->prev_port_ptr == NULL)
	{
	  channel_list.port_ptr = port_ptr->next_port_ptr;
	}
	else /* prev_port != NULL */
	{
	  port_ptr->prev_port_ptr->next_port_ptr = port_ptr->next_port_ptr;
	}
        if (port_ptr->next_port_ptr != NULL)
	  port_ptr->next_port_ptr->prev_port_ptr = port_ptr->prev_port_ptr;

	outputf("QPM_PORT_TERMINATE: free port\n");
	if (xmfree((caddr_t)port_ptr, (caddr_t)pinned_heap) != NULL)
	{
	  if (list_lock_rc != LOCK_NEST) unlockl(&(channel_list.lock));
	  return(qpm_rc_free_failed);
	}
      }
      else /* port_ptr != port_id */
      {
	port_ptr = port_ptr->next_port_ptr;
      }
    }
    if (port_found == FALSE)
    {
      rc = qpm_rc_port_not_found;
    }
    else
    {
      rc = qpm_rc_ok;
    }
  }
  if (list_lock_rc != LOCK_NEST) unlockl(&(channel_list.lock));
  return(rc);
}

/*****************************************************************************/
/* Function     qpm_query_device                                             */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
qpm_rc_type qpm_query_device(

  port_type   *port_id,
  char        *data_area)
{
  int dh_rc;
  cio_query_blk_t query_arg;

  query_arg.status = NULL;
  query_arg.bufptr = data_area;
  query_arg.buflen = sizeof(cio_stats_t);
  query_arg.clearall = NULL;

  /***************************************************************************/
  /* DBUG _ print out fp                                                     */
  /***************************************************************************/
  outputf("CIO_QUERY will be passed fp = %d\n",port_id->fp);
  dh_rc = fp_ioctl(port_id->fp, CIO_QUERY, &query_arg, NULL);
  outputf("QPM_QUERY_DEVICE: dh_rc = %d\n",dh_rc);
  /***************************************************************************/
  /* Because QLLC does not support the query of X25 specific data, and only  */
  /* provides a cio_stats_t structure, the DH is likely to return EMSGSIZE,  */
  /* which is a warning that it filled the area provided and truncated the   */
  /* statistics. All this means to QLLC is that the cio data is intact and   */
  /* that the truncated data was the X25 specific info QLLC doesn't want.    */
  /***************************************************************************/
  if (dh_rc != 0 && dh_rc != EMSGSIZE)
  {
    /*************************************************************************/
    /* There has been an error condition                                     */
    /*************************************************************************/
    outputf("QPM_QUERY_DEVICE: dh error from CIO_QUERY. dh_rc=%d\n",dh_rc);
    outputf("...status value is %d\n",query_arg.status);
    return(qpm_rc_system_error);
  }
  else
    return(qpm_rc_ok);
}

/*****************************************************************************/
/* Function     qpm_start                                                    */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* This procedure accepts a fully initialised x25_start_data structure.      */
/* The structure is storage owned by the calling procedure, and was          */
/* initialised by the QPM constructor functions in qlcpcons.c module.        */
/* The calling procedure also got an mbuf for the packet_data and any        */
/* call_data, or user_data.  The mbuf has been completely initialised by     */
/* the QVM qlcvbuf.c module.                                                 */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
qpm_rc_type qpm_start(

  port_type             *port_id,
  gen_buffer_type       *start_buffer,
  struct x25_start_data *start_data)
{
  int dh_rc;
  qpm_rc_type qpm_rc;

  /***************************************************************************/
  /* Call the Device Handler's IOCTL entry point.                            */
  /***************************************************************************/
  print_x25_buffer(start_buffer);
  print_start_data(start_data);

  /***************************************************************************/
  /* The Port Manager needs to remember the netid of the listening station   */
  /* on each port. Only one listener is allowed per port.                    */
  /***************************************************************************/
  if (start_data->session_type == SESSION_SVC_LISTEN)
  {
/* defect 156503 */
    outputf("QPM_START: SESSION_SVC_LISTEN, netid = %d\n",start_data->sb.netid);
    if (!add_listen_netid(start_data->sb.netid, port_id)) {
      outputf("QPM_START: listen_netid %d already exists!\n", 
		start_data->sb.netid);
      qpm_rc = qpm_rc_start_failed;
      return(qpm_rc);
      }
/* end defect 156503 */
  }
  /***************************************************************************/
  /* The netid of a listener is used when an incoming call is received.      */
  /***************************************************************************/
  if (start_data->session_type == SESSION_SVC_IN)
  {
    outputf("QPM_CALL_ID_CHECK: call_id = %d\n",
      start_data->session_type_data.call_id);
/* defect 156503 */
    outputf("QPM_START: SESSION_SVC_IN, netid = %d\n",start_data->sb.netid);
    if (!rm_listen_netid(start_data->sb.netid, port_id)) {
      outputf("QPM_START: listen_netid %d wasn't on the listener list!\n",
		start_data->sb.netid);
      qpm_rc = qpm_rc_start_failed;
      return(qpm_rc);
    }
/* end defect 156503 */
  }
  /***************************************************************************/
  /* The netid has now been cleared from the port's listener field, as the   */
  /* listener will be converted into a fully established session.            */
  /***************************************************************************/
  outputf("QPM_START: ...issuing start ioctl to dh\n");
  dh_rc = fp_ioctl(port_id->fp,CIO_START,start_data,start_buffer);
  outputf("QPM_START: ...dh_rc = %d\n",dh_rc);
  print_start_data(start_data);

  switch (dh_rc)
  {
  case EIO :
    outputf("QPM_START: case EIO = %d\n", EIO);
    outputf("QPM_START: STATUS WORD RETURNED=%d\n",start_data->sb.status);
    switch (start_data->sb.status)
    {
    case X25_NO_NAME :
      qpm_rc = qpm_rc_no_name;
      break;
    default :
      qpm_rc = qpm_rc_start_failed;
      break;
    }
    break;
  case 0 :
    outputf("QPM_START: case 0 \n");
    qpm_rc = qpm_rc_ok;
    break;
  default :
    outputf("QPM_START: default\n");
    qpm_rc = qpm_rc_start_failed;
      outputf("   qpm_rc_start_failed = %d\n", qpm_rc);
    break;
  }
  return(qpm_rc);
}

/*****************************************************************************/
/* Function     qpm_write                                                    */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* This procedure accepts a fully initialised x25_write_ext structure.       */
/* The structure is storage owned by the calling procedure, and was          */
/* initialised by the QPM constructor functions in qlcpcons.c module.        */
/* The calling procedure also got an mbuf for the data to be transmitted.    */
/* The mbuf has been completely initialised by the QVM qlcvbuf.c module.     */
/* This function must use a uio structure to pass the address of the mbuf    */
/* to the device handler. The uio handling is performed by the fp_write      */
/* system call.                                                              */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
qpm_rc_type  qpm_write (

  port_type             *port_id,
  gen_buffer_type       *buffer_ptr,
  struct x25_write_ext  *write_ext)
{
  qpm_rc_type        qpm_rc;
  int                dh_rc;
  int                counter = 0;
  struct uio         write_uio;
  struct iovec       write_iovec;

  /***************************************************************************/
  /* Put together a uio structure for use on devwrite call.                  */
  /***************************************************************************/
  write_iovec.iov_base = (char *)buffer_ptr;
  write_iovec.iov_len = 4;          
  write_uio.uio_iov = &write_iovec;
  write_uio.uio_iovcnt = 1;  
  write_uio.uio_resid = 4;             
  write_uio.uio_offset = 0;              
  write_uio.uio_segflg = UIO_SYSSPACE;
  write_uio.uio_fmode = O_RDWR;            /* r/w and blocked, defect 160105 */

  outputf("QPM_WRITE: buffer_ptr = %x\n",buffer_ptr);
  print_x25_buffer(buffer_ptr);
  print_write_ext(write_ext);
  
  outputf("QPM_WRITE: calling devwrite port_id = %d \n",port_id );

  dh_rc = fp_rwuio(
    port_id->fp,
    UIO_WRITE,
    &write_uio,
    write_ext
    );

  outputf("QPM_WRITE: return code from devwrite = %d\n",dh_rc);

  /***************************************************************************/
  /* Pass fp_write a value of 0 for nbytes, because the length is not        */
  /* relevant as we are a kernel user of the device handler. A non-negative  */
  /* value (e.g. 0) is sufficient to allow fp_write to work.                 */
  /* The countp paramater is not relevant either, again because we are a     */
  /* kernel user. Just pass a pointer to a count location but don't use the  */
  /* counter for anything. It will always be 0.                              */
  /* The countp would normally be decremented from nbytes each time an       */
  /* iomove took place for an application user.                              */
  /***************************************************************************/
/*
*  dh_rc = fp_write(
*    port_id->fp,
*    buffer_ptr,
*    0,
*    write_ext,
*    FP_SYS,
*    &counter
*    );
*
*  outputf("QPM_WRITE: return code from fp_write = %d\n",dh_rc);
*/

  if (dh_rc != 0)
  {
    outputf("QPM_WRITE: devwrite rc = %d\n",dh_rc);
    /*************************************************************************/
    /* There has been some local error, and the buffer could not be sent.    */
    /*************************************************************************/
    qpm_rc = qpm_rc_write_error;
  }
  else
  {
    qpm_rc = qpm_rc_ok;
  }
  return(qpm_rc);
}

/*****************************************************************************/
/* Function     qpm_halt                                                     */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* This procedure accepts a fully initialised x25_halt_data structure.       */
/* The structure is storage owned by the calling procedure, and was          */
/* initialised by the QPM constructor functions in qlcpcons.c module.        */
/* The calling procedure also got an mbuf for the packet_data and any        */
/* call_data, or user_data.  The mbuf has been completely initialised by     */
/* the QVM qlcvbuf.c module.                                                 */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
qpm_rc_type   qpm_halt(

  port_type             *port_id,
  struct x25_halt_data  *halt_data,
  gen_buffer_type       *buffer_ptr)
{
  qpm_rc_type qpm_rc = qpm_rc_ok;
  int dh_rc;

  outputf("QPM_HALT: printing out contents of buffer\n");
  print_x25_buffer(buffer_ptr);
  print_halt_data(halt_data);
  /***************************************************************************/
  /* If the request is to halt a listener, then take the netid out of the    */
  /* port's listener_netid list. (defect 156503)                             */
  /***************************************************************************/
  if (!rm_listen_netid(halt_data->sb.netid, port_id))
    outputf("QPM_HALT tried to remove netid %d, but it didnt exist\n",
		halt_data->sb.netid);
  else
    outputf("QPM_HALT removed netid %d from listen list\n",halt_data->sb.netid);

  outputf("QPM_HALT calling devioctl, port_id = %d\n",port_id);
  dh_rc = fp_ioctl(port_id->fp,CIO_HALT,halt_data,buffer_ptr);

  outputf("QPM_HALT return code from devioctl = %d\n",dh_rc);
  if (dh_rc != 0)
  {
    /* qcm_port_error(port_id); */
    qpm_rc = qpm_rc_halt_failed;
  }
  return (qpm_rc);
}

/*****************************************************************************/
/* Function     qpm_interrupt_handler                                        */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* The Port Manager Interrupt Handler is initialised by the initp() call     */
/* made by the QPM on Port Initialisation (qpm_port_initialise). When the    */
/* the QPM_Interrupt_Handler is called with the INIT_PROC value set, it      */
/* initialises itself, and copies the initialisation data it is passed.      */
/* The init. data consists of the port_id of the port which is starting the  */
/* the Interrupt Handler. (One Handler is started for each port which is     */
/* initialised).                                                             */
/* The INIT_PROC flag value is the only one that the QPM_Interrupt Handler   */
/* will act upon. Any other values of flag will be ignored, while an error   */
/* is logged.                                                                */
/* Having done the initialisation, the Interrupt Handler will enter an       */
/* infinite loop, which does the following:                                  */
/*    a) Tests the queues to make sure there is nothing on them.             */
/*    b) If any queue has something on it, a bit is set in the event mask.   */
/*    c) The state of the port is checked.                                   */
/*    d) If all the bits in the event mask indicate empty queues, and the    */
/*       port is in a valid state, the Handler enters a wait......           */
/*    e) ....when it gets woken it checks that the wait was ok.              */
/*    f) If not OK, it will log an error, and terminate.                     */
/*    g) If it was OK, then it checks each bit in the events mask returned   */
/*       from the wait, and services each of the queues in turn.             */
/*                                                                           */
/*    There are some points to note.                                         */
/*      1) If a queue has anything on it, it is serviced till empty.         */
/*      2) The order in which the queues are checked may be important.       */
/*      3) ONLY QUEUES WHICH WERE EMPTY RESULT IN A POST TO THE INT HANDLER. */
/*         IT IS THEREFORE IMPERATIVE THAT THE INT HANDLER DOES EMPTY THEM.  */
/*                                                                           */
/* Return    none                                                            */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
void qpm_interrupt_handler (

  unsigned int                   flag,
  qlc_intrpt_hndlr_init_data_t  *init_parms,
  int                            parms_length)
{
  unsigned int         events;
  gen_buffer_type     *buffer_ptr;
  struct pending_status_block_type *pending_exception_ptr;
  struct status_block *exception_ptr;
  port_type           *port_id;
  unsigned short       netid;
  unsigned short       session_id;
  int lock_rc;

  outputf( "IN qpm_interrupt_handler.\n" );
  outputf("QPM_INT_HND: initialising....\n");
  /***************************************************************************/
  /* This routine has been called due to an initp call.                      */
  /*                                                                         */
  /* The init parms consist of the port_id of the port that is starting the  */
  /* int handler.                                                            */
  /***************************************************************************/
  port_id = init_parms->port_id;
  outputf("QPM_INT_HND: port_id=%d\n",port_id);
  outputf("QPM_INT_HND: port_id->lock=%x\n",port_id->lock);
  /***************************************************************************/
  /* Disconnect the process from the caller.  This means that the parent     */
  /* can die without waiting for this one to die!                            */
  /***************************************************************************/
  setpgid(0,0);
  setpinit();

/* <<< THREADS >>> */
  /********************************************************************/
  /* call thread_self to get the current thread id of this kproc      */
  /********************************************************************/
  assert((port_id->int_tid = thread_self()) != -1)
/* <<< end THREADS >>> */

  /***************************************************************************/
  /* There is a QLLC QPM Interrupt Handler for each port that is initialised.*/
  /* The purpose of the handler is to wait until something happens on the    */
  /* port which means that the port queues need servicing. At such times the */
  /* <<< THREADS >>>                                                         */
  /* handler is woken by an et_post and will be passed an events parm which  */
  /* indicates what has happened.                                            */
  /* The handler must then service the queue until the queue is empty, as no */
  /* et_posts are generated on queues that are already non-empty.            */
  /* <<< end THREADS >>>                                                     */
  /*                                                                         */
  /* The three things the handler must do are:                               */
  /*  1. Service receive_data_queue                                          */
  /*  2. Service exception_queue                                             */
  /*  3. Service transmit_queue            --support removed 17th Aug 89--   */
  /*                                                                         */
  /***************************************************************************/

  /***************************************************************************/
  /* The DH currently does not support the use of tx_fn, and as a result the */
  /* code that supports tx_fn activity here in the qpm_interrupt_handler has */
  /* been removed. It is possible that the DH will re-support tx_fn, and this*/
  /* comment indicates what is needed to provide such support.               */
  /*                                                                         */
  /* The support for tx_fn has been removed (17th August 1989). This used to */
  /* consist of checking the TANSMIT_AVAIL mask as well as the DATA_RECEIVED */
  /* and EXCEPTION_ARRIVED masks in the et_wait call.                        */
  /* Upon wakeup, there used to be a partition which was invoked if the      */
  /* events returned by the wait had the TRANSMIT_AVAIL bit set.             */
  /* This partition would then deque buffers and issue qpm_write calls with  */
  /* the buffer pointers derived from the dequeueing macro. This was repeated*/
  /* until the queue was empty.                                              */
  /***************************************************************************/
  while (TRUE)
  {
    outputf("QPM_INT_HND: sleeping................\n");
/* defect THREADS 3 */
    events = et_wait(
      (DATA_RECEIVED | EXCEPTION_ARRIVED | TERMINATION | TIMER_SERVICE),
      (DATA_RECEIVED | EXCEPTION_ARRIVED | TERMINATION | TIMER_SERVICE),
      EVENT_SIGRET
      );
/* end defect THREADS 3 */
    outputf("QPM_INT_HND: ...............awake\n");
    /*************************************************************************/
    /* Upon wakeup from the above wait, do the following.....                */
    /*************************************************************************/
    /*************************************************************************/
    /* Look and termination before doing any port service for the data and   */
    /* exception queues. QPM_Terminate will take care of cleaning up the     */
    /* port control block, this is just to kill off the kproc gracefully.    */
    /*************************************************************************/
    if ( (events & TERMINATION) == TERMINATION )
    {
      outputf("QPM_KPROC: terminating!\n");
      return;
    }
    if ( (events & DATA_RECEIVED) == DATA_RECEIVED )
    {
      outputf("QPM_INT_HND: data received\n");
      /***********************************************************************/
      /* Service receive_queue until empty or a blockage occurs              */
      /***********************************************************************/
      while (port_id->receive_data_queue.first != NULL)
      {
	/*********************************************************************/
	/* Get the first buffer off the queue and call QLM.                  */
	/* The buffer was stored on the receive queue by the                 */
	/* qpm_receive_function, and noted whether there had been an o/flow  */
	/* condition                                                         */
	/*                                                                   */
	/* Note that there are a number of things that the data buffer could */
	/* represent.                                                        */
	/*                                                                   */
	/*   1) Incoming Call (on listening sessions only)                   */
	/*   2) Data (on all types of session)                               */
	/*   3) Interrupt or Interrupt Confirm (illegal)                     */
	/*   4) Reset Indication or Reset Confirm (on PVCs only)             */
	/*   5) A D-bit acknowledgement (illegal)                            */
	/*   6) A Clear Inidication (only for SVCs)                          */
	/*********************************************************************/
/* Defect 101380
	outputf("QLLC_KPROC: pid       = %x\n",u.u_procp->p_pid);
*/
/* <<< THREADS >>> */
	outputf("QLLC_KPROC: tid       = %x\n",thread_self ());
/* <<< end THREADS >>> */
/* End defect 101380 */

	outputf("QLLC_KPROC: port lock = %x\n",port_id->lock);
	lock_rc = lockl(&(port_id->lock),0);
	outputf("QLLC_KPROC: port lock = %x\n",port_id->lock);
	buffer_ptr = QBM_DEQUE_BUFFER(&(port_id->receive_data_queue));
	outputf("QPM_KPROC: got first buffer from queue. ptr=%x\n",buffer_ptr);
	outputf("QLLC_KPROC: port lock = %x\n",port_id->lock);
	if (lock_rc != LOCK_NEST) unlockl(&(port_id->lock));
	outputf("QLLC_KPROC: port lock = %x\n",port_id->lock);
	/*********************************************************************/
	/* Now you have the buffer, decide what to do with it.               */
	/*********************************************************************/
	/*********************************************************************/
	/* Get the netid out of the buffer before you do anything.           */
	/*********************************************************************/
	netid = QBM_RETURN_NETID(buffer_ptr);
	outputf("QPM_KPROC: netid from buffer = %d\n",netid);
        session_id = QBM_RETURN_SESSION_ID(buffer_ptr);        
	outputf("QPM_KPROC: session_id from buffer = %d\n",session_id);

	outputf("QPM_KPROC: rx data pkt type = %x\n",
	  QBM_RETURN_PACKET_TYPE(buffer_ptr));
	print_x25_buffer(buffer_ptr);
	/*********************************************************************/
	/* If the D-Bit is set the station must be closed down, as follows   */
	/*********************************************************************/
	if (QBM_RETURN_D_BIT(buffer_ptr) == TRUE)
	{
	  outputf("QPM_KPROC: D bit set\n");
	  qlm_invalid_packet_rx(buffer_ptr, netid, session_id,
	    INVALID_D_BIT_REQUEST);
	}
	/*********************************************************************/
	/* Check now to see if it's data, because you want to emphasise the  */
	/* performance aspects of normal data, whereas other packets are not */
	/* quite so critical.                                                */
	/*********************************************************************/
	else if (QBM_RETURN_PACKET_TYPE(buffer_ptr) == PKT_DATA)
	{
	  outputf("QPM_KPROC: data packet received on queue\n");
	  qlm_receive_data(buffer_ptr,netid,session_id);
	}
	/*********************************************************************/
	/* Check to see if it's an incoming call                             */
	/*********************************************************************/
	else if (QBM_RETURN_PACKET_TYPE(buffer_ptr) == PKT_INCOMING_CALL)
	{
	  outputf("QPM_KPROC: incoming call received on queue\n");
	  qlm_incoming_call(port_id, netid, buffer_ptr);
	}
	/*********************************************************************/
	/* Check to see if the packet contains a RESET IND or RESET CONFIRM  */
	/*********************************************************************/
	else if (QBM_RETURN_PACKET_TYPE(buffer_ptr) == PKT_RESET_IND
	  || QBM_RETURN_PACKET_TYPE(buffer_ptr) == PKT_RESET_CONFIRM
	  )
	{
	  outputf("QPM_KPROC: reset ind/confirm received on queue\n");
	  qlm_station_reset(buffer_ptr,netid,session_id);
	}
	/*********************************************************************/
	/* And finally the packet might be a Clear Indication (for SVCs)     */
	/*********************************************************************/
	else if (QBM_RETURN_PACKET_TYPE(buffer_ptr) == PKT_CLEAR_IND)
	{
	  outputf("QPM_KPROC: clear indication received on queue\n");
	  /*******************************************************************/
	  /* Since the station is being halted due to an unsolicited Clear   */
	  /* Indication being received, the result parameter to the qlm      */
	  /* function is set to NULL, and the remote_cleared parameter is set*/
	  /* to TRUE.                                                        */
	  /*******************************************************************/
	  qlm_incoming_clear(netid, session_id, buffer_ptr);
	}
	/*********************************************************************/
	/* If the packet was none of the above, and didn't have its D bit    */
	/* set then the VC is closed.                                        */
	/* Note that PKT_INT and PKT_INT_CONFIRM packets drop through to here*/
	/********************************************************************/
	else
	{
	  outputf("QPM_KPROC: illegal packet detected on queue\n");
	  /*******************************************************************/
	  /* The packet is either an interrupt packet or some other packet   */
	  /* that the DH should never pass to you.                           */
	  /* Interrupt packets are not supported by QLLC. The VC should be   */
	  /* closed. If this isn't an Interrupt packet then you should also  */
	  /* close the VC.                                                   */
	  /*******************************************************************/
	  qlm_invalid_packet_rx(
	    buffer_ptr,
	    netid,
	    session_id,
	    link_station_unusual_network_condition
	    );
	}
	outputf("QPM_KPROC: looping back to check data queue\n");
	outputf("QPM_KPROC: port_id->receive_data_queue.first = %x\n",
	  port_id->receive_data_queue.first);
      }
    }
    if ( (events & EXCEPTION_ARRIVED) == EXCEPTION_ARRIVED)
    {
      outputf("QPM_INT_HND: exception arrived\n");
      /***********************************************************************/
      /* Service exception_queue until empty or a blockage occurs            */
      /***********************************************************************/
      while (port_id->exception_queue != NULL)
      {
	/*********************************************************************/
	/* Get the first exception off the queue and call QLM.               */
	/* The elements in the queue are pending status blocks and are in a  */
	/* linked list. The list is managed here.                            */
	/*********************************************************************/
	lock_rc = lockl(&(port_id->lock),0);
	outputf("QPM_INT_HAND: reading exception queue\n");
	pending_exception_ptr = port_id->exception_queue;
	outputf("QPM_INT_HAND: removing first block\n");
	port_id->exception_queue = pending_exception_ptr->next_block_ptr;
	if (port_id->exception_queue != NULL)
	  port_id->exception_queue->prev_block_ptr = NULL;
	if (lock_rc != LOCK_NEST) unlockl(&(port_id->lock));
	exception_ptr = &(pending_exception_ptr->block);
	/*********************************************************************/
	/* Determine the nature of the status_block.                         */
	/*********************************************************************/
	switch (exception_ptr->code)
	{
	case CIO_START_DONE :
	  /*******************************************************************/
	  /* This is a start done status block. The link station this status */
	  /* block applies to is indicated by the net_id in the option[1]    */
	  /* lower 2 bytes. Note that buffer_ptr is only valid if            */
	  /* session_type is SESSION_SVC_OUT. This will be checked by QLM.   */
	  /*******************************************************************/
	  outputf("QPM_KPROC: START_DONE\n");
	  outputf( "call qlm_station_started\n" );
	  qlm_station_started(
	    exception_ptr->option[0],                    /* result           */
	    ((exception_ptr->option[1])&0x0000FFFF),     /* net_id           */
	    ((exception_ptr->option[1])>>16),            /* session_id       */
	    exception_ptr->option[2]                     /* buffer_ptr       */
	    );
	  outputf( "left call on qlm_station_started\n" );
	  break;
	case CIO_HALT_DONE :
	  /*******************************************************************/
	  /* This is a halt done status block.  The link station this status */
	  /* block is for is indicated by the net_id in the lower 2 bytes of */
	  /* option[1].                                                      */
	  /*                                                                 */
	  /* Note that buffer_ptr is only valid if the halt was used to      */
	  /* issue a clear request. This will be the case as follows..       */
	  /*                                                                 */
	  /* I issue a halt under the following circumstances:               */
	  /*   1. When local clearing.                                       */
	  /*      This issues a clear request. The halt done passes back the */
	  /*      buffer you gave it as Call Req, but has filled it in with  */
	  /*      the contents of the Clear Confirm packet, which you may    */
	  /*      need if you have Charging Info faciility selected.         */
	  /*   2. When a remote clear comes in it arrives as a Clear         */
	  /*      Indication.                                                */
	  /*      This issues a Clear Confirm not a clear request.           */
	  /*      No clear request.                                          */
	  /*   3. When I want to stop a listen.                              */
	  /*      No clear request.                                          */
	  /*   4. When halting a PVC. No clear request.                      */
	  /*                                                                 */
	  /* So the buffer_ptr is only valid in the case of a local clear.   */
	  /* The qlm proc needs to be told whether the clear was remotely    */
	  /* initiated or locally initiated. The port manager doesn't know   */
	  /* which is the case, so it uses the presence of a non-NULL buffer */
	  /* ptr to indicate local clearing. A NULL buffer_ptr indicates that*/
	  /* the halt was issued in response to an incoming Clear Indication */
	  /*******************************************************************/
	  outputf("QPM_KPROC: HALT DONE\n");
	  qlm_station_halted(
	    exception_ptr->option[0],                    /* result           */
	    ((exception_ptr->option[1])&0x0000FFFF),     /* net_id           */
	    ((exception_ptr->option[1])>>16),            /* session_id       */
	    exception_ptr->option[2]                     /* buffer_ptr       */
	    );
	  break;
	case CIO_TX_DONE :
	  /*******************************************************************/
	  /* This is a transmit done status block.  The link station this    */
	  /* status block applies to is indicated by the net_id in the       */
	  /* option[1] lower 2 bytes. If the status block contains a result  */
	  /* that indicates the tx was sucecssful then the status block is   */
	  /* trashed. If the status block contains a result that indicates   */
	  /* the tx was unsuccessful then the qlm_write_error function is    */
	  /* called. This will close the vc.                                 */
	  /*******************************************************************/
	  outputf("QPM_KPROC: TX DONE\n");
	  if (exception_ptr->option[0] != CIO_OK)
	  {
	    qlm_write_error(
	      ((exception_ptr->option[1])&0x0000FFFF),     /* net_id         */
	      ((exception_ptr->option[1])>>16),            /* session_id     */
	      exception_ptr->option[2]                     /* buffer_ptr     */
	      );
	  }
	  break;
	case X25_REJECT_DONE :
	  /*******************************************************************/
	  /* This is a reject done status block. The link station this status*/
	  /* block applies to is indicated by the net_id in the option[1]    */
	  /* lower 2 bytes. If the status block contains a result that       */
	  /* indicates the tx was sucecssful then the status block is        */
	  /* trashed. If the status block contains a result that indicates   */
	  /* the tx was unsuccessful then the qlm_write_error function is    */
	  /* called. This will close the vc.                                 */
	  /*******************************************************************/
	  outputf("QPM_KPROC: REJECT DONE\n");
	  qlm_incoming_call_rejected(
	    exception_ptr->option[0],
	    ((exception_ptr->option[1])&0x0000FFFF),    /* net_id            */
	    ((exception_ptr->option[1])>>16)            /* session_id        */
	    );
	  break;
	default :
	  /*******************************************************************/
	  /* The DH has passed you an invalid status block, close the port.  */
	  /*******************************************************************/
	  outputf("5a) port_id->lock = 0x%x, unlocked\n", port_id->lock);
	  (void)qpm_port_terminate(port_id);
	  break;
	} /* end of switch on exc type */
	(void)xmfree((caddr_t)pending_exception_ptr, (caddr_t)pinned_heap);
      } /* loop till exc queue empty */
    } /* end of if exception condition */
    if (events & TIMER_SERVICE)
    {
      /******************************************************************/
      /* Defect 103650 - Error in testing the event.                    */
      /******************************************************************/
      outputf("QLLC_KPROC: timer service post received\n");
      if ((events & REPOLL_TIMEOUT) == REPOLL_TIMEOUT)
      {
	outputf("QLLC_KPROC: repoll timer service post received\n");
	qlm_service_repolls(port_id);
	outputf("QLLC_KPROC: back from qlm_service_repolls()\n");
      }
      if ((events & INACT_TIMEOUT) == INACT_TIMEOUT)
      {
	outputf("QLLC_KPROC: inactivity timeout being serviced\n");
	qlm_service_inactivity_timeouts(port_id);
      }
      if ((events & HALT_TIMEOUT) == HALT_TIMEOUT)
      {
	outputf("QLLC_KPROC: halt timeout being serviced\n");
	qlm_service_halt_timeouts(port_id);
      }
      if ((events & RETRY_TIMEOUT) == RETRY_TIMEOUT)
      {
	outputf("QLLC_KPROC: retry service\n");
	qlm_retry_receive(port_id);
      }
      /********************************************************************/
      /* End of defect 103650.						  */
      /********************************************************************/
    }
  } /* loop back to sleep */
}


/*****************************************************************************/
/* Function     qpm_enter_local_busy                                         */
/*                                                                           */
/* Description                                                               */
/*              This function is used to issue an enter local busy command   */
/*              to the X25 Device Handler.                                   */
/*                                                                           */
/* Return       none                                                         */
/*                                                                           */
/* Parameters   port identifier and session identifier                       */
/*****************************************************************************/
qpm_rc_type  qpm_enter_local_busy (

  port_type      *port_id,
  unsigned short  session_id)
{
  int dh_rc;
  struct x25_local_busy x25_local_busy;

  x25_local_busy.busy_mode = ENTER_LOCAL_BUSY;
  x25_local_busy.session_id = session_id;

  dh_rc = fp_ioctl(port_id->fp,X25_LOCAL_BUSY,&(x25_local_busy),NULL);
  if (dh_rc != 0)
  {
    /* qcm_port_error(port_id); */
    return(qpm_rc_enter_busy_failed);
  }
  return(qpm_rc_ok);
}

/*****************************************************************************/
/* Function     qpm_exit_local_busy                                          */
/*                                                                           */
/* Description                                                               */
/*              This function is used to issue an exit local busy command    */
/*              to the X25 Device Handler.                                   */
/*                                                                           */
/* Return       none                                                         */
/*                                                                           */
/* Parameters   port identifier and session identifier                       */
/*****************************************************************************/
qpm_rc_type  qpm_exit_local_busy (

  port_type      *port_id,
  unsigned short  session_id)
{
  int dh_rc;
  struct x25_local_busy x25_local_busy;

  x25_local_busy.busy_mode = EXIT_LOCAL_BUSY;
  x25_local_busy.session_id = session_id;

  dh_rc = fp_ioctl(port_id->fp,X25_LOCAL_BUSY,&(x25_local_busy),NULL);
  if (dh_rc != 0)
  {
    /* qcm_port_error(port_id); */
    return(qpm_rc_exit_busy_failed);
  }
  return(qpm_rc_ok);
}


/*****************************************************************************/
/* Function     qpm_reject                                                   */
/*                                                                           */
/* Description                                                               */
/*              This function is used to reject an incoming call. It issues  */
/*              an ioctl call to the X25 Device Handler, requesting that the */
/*              call is rejected.                                            */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters   port identifier                                              */
/*              reject data structure as defined in XDH spec.                */
/*              address of buffer containing clear data                      */
/*****************************************************************************/
qpm_rc_type  qpm_reject (
  
  port_type              *port_id,
  struct x25_reject_data *reject_data,
  gen_buffer_type        *buffer_ptr)
{
  int dh_rc;

  outputf("QPM_REJECT: called\n");
  print_x25_buffer(buffer_ptr);
  
  dh_rc = fp_ioctl(port_id->fp,X25_REJECT_CALL,reject_data,buffer_ptr);

  outputf("QPM_REJECT: dh reject ioctl rc = %d\n",dh_rc);
  outputf("QPM_REJECT: status returned    = %d\n",reject_data->sb.status);
  
  if (dh_rc != 0)
  {
    /* qcm_port_error(port_id); */
    return(qpm_rc_reject_failed);
  }
  return(qpm_rc_ok);
}

/*****************************************************************************/
/* Function     qpm_init_address                                             */
/*                                                                           */
/* Description                                                               */
/*              This function is used to update the address field in the     */
/*              port on the first enable. The way the port mgr detects that  */
/*              this is the first enable is that this function is always     */
/*              called on enable, but the address, which is initialised to   */
/*              NULL on port_init, is not NULL after the first enable has    */
/*              been issued.                                                 */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters   port identifier                                              */
/*              address                                                      */
/*****************************************************************************/
qpm_rc_type  qpm_init_address (

  port_type         *port_id,
  char              *address)
{
  if (strlen(address) != 0)
  {
    strcpy(port_id->local_address,address);
    outputf("qpm_init_address = %s\n",address);
  }
}

/* defect 156503 */
/*****************************************************************************/
/* Debug routine for printing out the list of listeners on a port.           */
/*****************************************************************************/
static int print_list( port_type *port_id)
{
  struct listen_netid_type *lptr;

  lptr = port_id->listen_netids;

  if (lptr == (struct listen_netid_type *)NULL)
        outputf("print_list: list is empty\n");

  while (lptr != (struct listen_netid_type *)NULL) {
        outputf("print_list: next:%d, listener_netid:%d\n",(int)lptr->next,
		 lptr->listener_netid);
        lptr=lptr->next;
  }
}

/*****************************************************************************/
/* If netid exists on port_id->listen_netids list, TRUE (-1)                 */
/* else return FALSE (0).                                                    */
/*                                                                           */
/*****************************************************************************/
boolean listening_netid(
  unsigned    short netid,
  port_type   *port_id)
{
  struct listen_netid_type *lptr;
  boolean found = FALSE;
  int was_locked;

  /* get port lock */
  was_locked = (lockl(&port_id->lock, LOCK_SHORT) == LOCK_NEST);

  outputf("listening_netid: looking for netid= %d\n",netid);

  lptr = port_id->listen_netids;

  while (lptr != (struct listen_netid_type *)NULL) {
        outputf("listening_netid: found listener_netid = %d\n",
		lptr->listener_netid);
	if (lptr->listener_netid == netid) {
		found=TRUE;
		break;
	}
	lptr=lptr->next;
  }

  print_list(port_id);
  outputf("listening_netid: return=%d\n", found);

  /* release port lock */
  if (!was_locked) unlockl(&port_id->lock);

  return(found);
}


/*****************************************************************************/
/* If the netid passed in is already on the port_id->listen_netids list,     */
/* return FALSE, else add it to the end of the list and return TRUE.         */
/*                                                                           */
/*****************************************************************************/
boolean add_listen_netid(
  unsigned    short netid,
  port_type   *port_id)
{
  struct listen_netid_type *lptr;
  struct listen_netid_type *last_lptr;
  boolean found = FALSE;
  boolean was_locked;

  /* get port lock */
  was_locked = (lockl(&port_id->lock, LOCK_SHORT) == LOCK_NEST);

  outputf("add_listen_netid: request to add netid= %d\n",netid);

  lptr = last_lptr = port_id->listen_netids;

  if (lptr == (struct listen_netids_type *)NULL) {
	/* first guy on list */

  	outputf("add_listen_netid: adding netid first in list\n");

	/* should check for malloc failure */
	 port_id->listen_netids = (struct listen_netid_type *)xmalloc(
	    (unsigned int)sizeof(struct listen_netid_type),
	    WORD,
	    (caddr_t)pinned_heap);
	port_id->listen_netids->next = (struct listen_netids_type *)NULL;
	port_id->listen_netids->listener_netid = netid;
  } else {
	/* at least one guy on listen list */
  	while (lptr != (struct listen_netids_type *)NULL) {
        	outputf("add_listen_netid: found listener_netid = %d\n",
			lptr->listener_netid);
		if (lptr->listener_netid == netid) {
			found=TRUE;
			break;
		}
		last_lptr = lptr;
		lptr = lptr->next;
	}

	if (!found) {
		/* allocate and put new struct on list */
		last_lptr->next = (struct listen_netid_type *)xmalloc(
		    (unsigned int)sizeof(struct listen_netid_type),
		    WORD,
		    (caddr_t)pinned_heap);
		last_lptr->next->next = (struct listen_netids_type *)NULL;
		last_lptr->next->listener_netid = netid;
	}
  }

  /* if the listen netid was already found on the list, another isn't added */
  print_list(port_id);
  outputf("add_listen_netid: return=%d\n", !found);

  /* release port lock */
  if (!was_locked) unlockl(&port_id->lock);
  return(!found);
}

/*****************************************************************************/
/* If the netid passed in can't be found, return FALSE,  else remove it from */ 
/* the list and return TRUE.                                                 */
/*****************************************************************************/
boolean rm_listen_netid(
  unsigned    short netid,
  port_type   *port_id)
{
  struct listen_netid_type *lptr;
  struct listen_netid_type *tmp_lptr;
  struct listen_netid_type *last_lptr;
  boolean found = FALSE;
  boolean was_locked;

  /* get port lock */
  was_locked = (lockl(&port_id->lock, LOCK_SHORT) == LOCK_NEST);

  outputf("rm_listen_netid: request to rm netid= %d\n",netid);

  lptr = last_lptr = port_id->listen_netids;

  while (lptr != (struct listen_netids_type *)NULL) {
        outputf("rm_listen_netid: found listener_netid = %d\n",
		lptr->listener_netid);
	if (lptr->listener_netid == netid) {
		found=TRUE;
		break;
	}
	last_lptr = lptr;
	lptr = lptr->next;
  }

  if (found) {
	if (lptr == port_id->listen_netids)
		/* special case for removing the first netid from the list */
		port_id->listen_netids = lptr->next;
	else
		last_lptr->next = lptr->next;
	xmfree(lptr, (caddr_t)pinned_heap);
  }

  print_list(port_id);
  outputf("rm_listen_netid: return=%d\n", found);

  /* release port lock */
  if (!was_locked) unlockl(&port_id->lock);
  return(found);
}
/* end defect 156503 */
