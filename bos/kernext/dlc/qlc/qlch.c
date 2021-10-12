static char sccsid[] = "@(#)10	1.21  src/bos/kernext/dlc/qlc/qlch.c, sysxdlcq, bos411, 9428A410j 10/5/93 16:04:40";
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: qlcconfig, qlcmpx, qlcopen, qlcclose, qlcread, qlcwrite,
 *            qlcioctl, qlcselect
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

#include "qlcg.h"             /* for general defs                            */
#include "qlcvfac.h"
#include "qlcv.h"
#include "qlcq.h"
#include "qlcb.h"
#include "qlcl.h"             /* for station types                           */
#include "qlcp.h"
#include "qlcc.h"
#include "qlcs.h"             /* for sap types                               */
#include "qlch.h"

#include <sys/trchkid.h>

extern char *buffer;
/*****************************************************************************/
/* Declare global channel list anchor. This is the only global data in QLLC  */
/* and is the starting point for the channel and port lists.                 */
/*****************************************************************************/
channel_list_type channel_list;

/*****************************************************************************/
/* Function     qlcconfig                                                    */
/*                                                                           */
/* Description  CFG_INIT:                                                    */
/*              This procedure simply initialises the channel_list           */
/*                                                                           */
/*              CFG_TERM:                                                    */
/*              This procedure forces channels closed freeing all            */
/*              resources associated with them.                              */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters   Device Number.                                               */
/*              Operation to perform. VPD is not supported.                  */
/*              Data area. Ignored by QLLC.                                  */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
int qlcconfig(
  dev_t    devno,
  int      op,
  struct   uio *uio_ptr)
{
  int            rc;
  channel_type  *channel_ptr;
  int lock_rc;


  switch (op)
  {
  case CFG_INIT :
    rc = config_init(uio_ptr);
    if (rc == 0)
    {
      /***********************************************************************/
      /* On receipt of an INIT option to qlcconfig apply lock                */
      /* to channel list                                                     */
      /***********************************************************************/
      channel_list.lock = LOCK_AVAIL;
      lock_rc = lockl(&(channel_list.lock),0);
      /***********************************************************************/
      /* Initialise linked lists of channels & ports to empty                */
      /***********************************************************************/
      channel_list.channel_ptr = NULL;
      channel_list.port_ptr = NULL;
      /***********************************************************************/
      /* Set the default value of debug control to OFF (0)                   */
      /***********************************************************************/
      channel_list.debug_control = 0;
      
      if (lock_rc != LOCK_NEST) unlockl(&(channel_list.lock));
      rc = 0;
    }
    break;

  case (CFG_TERM) :
    outputf("CFG_TERM: called\n");
    /*************************************************************************/
    /* On receipt of a TERM option to the qlcconfig function, all channels   */
    /* are closed with all their associated sap and link station resources   */
    /* being freed. All ports are cleared and freed.                         */
    /* Buffer queues in the channels and ports are purged. The channel list  */
    /* is emptied, and left unlocked. There is not much checking of return   */
    /* codes here as if things go wrong there's not really much you can do,  */
    /* as you are freeing all the resources you can anyway.                  */
    /*************************************************************************/
    lock_rc = lockl(&(channel_list.lock),0);
    /*************************************************************************/
    /* loop along channel list removing and freeing channels                 */
    /*************************************************************************/
    if (channel_list.channel_ptr != (channel_type *)NULL)
    {
       if (lock_rc != LOCK_NEST) unlockl(&(channel_list.lock));
       return(EBUSY);
    }
    (void) devswdel(devno);
    if (lock_rc != LOCK_NEST) unlockl(&(channel_list.lock));
    (void) unpincode( (void_ptr_t) qpm_receive_function );
    (void) unpincode( (void_ptr_t) qpm_exception_function );
    (void) unpincode( (void *) qlcconfig );
    rc = 0;
    break;

  default :
    /*************************************************************************/
    /* QLLC only supports CFG_TERM and CFG_INIT options to qlcconfig. Any    */
    /* other options cause qlcconfig to return EINVAL.                       */
    /*************************************************************************/
    rc = EINVAL;
    break;
  }
  return (rc);
}

/*****************************************************************************/
/* Function     qlcmpx                                                       */
/*                                                                           */
/* Description                                                               */
/*              This function is called prior to qlcopen being called. It    */
/*              requests allocation of a channel control block, and          */
/*              initialises a port, allocating memory for the port only if   */
/*              the port doesn't already exist.                              */
/*                                                                           */
/*              There are the following cases to consider.                   */
/*              If a pathname for the DH given by port_name is found to      */
/*              exist, then qlcmpx attempts to allocate a channel control    */
/*              block.                                                       */
/*              It returns a pointer to the channel, if the allocation was   */
/*              successful, in the channel_id parameter.                     */
/*              It returns a NULL if the allocation fails.                   */
/*              If the path_name parameter is NULL, this indicates that the  */
/*              channel is to be deallocated. Channel is indicated by        */
/*              channel_id parameter.                                        */
/*                                                                           */
/* Return       successful allocation => pointer to channel                  */
/*              failed allocation => null                                    */
/*                                                                           */
/* Parameters   dev  - major and minor device numbers for QLLC.              */
/*              channel_id - returned ptr to channel control block.          */
/*                           notice that this parm is passed by reference.   */
/*              port_name - name of dev handler.                             */
/*                                                                           */
/*****************************************************************************/
int  qlcmpx(
  dev_t  dev,
  int *channel_id,
  char *port_name)
{
  int         qdh_rc = 0; /* return code from this function                  */
  dev_t       dh_devno;   /* Device Handler device number                    */
  int         rc;         /* return code from system calls made by qlcmpx    */
  qpm_rc_type qpm_rc;     /* return code from calls to QPM made by qlcmpx    */
  qcm_rc_type qcm_rc;     /* return code from calls to QCM made by qlcmpx    */
  struct file *fp;
  char        path[50];
  dev_t       tempdev;
  struct fullstat   qlc_stat;
  struct port_type  *returned_port_id;


  outputf("QLCMPX: port_name is %s\n",port_name);
  if (*port_name == '\0')
  {
    outputf("QLCMPX: channel is to be de-allocated.\n");
    /*************************************************************************/
    /* The device handler pathname extension is NULL, which indicates that   */
    /* the channel is to be deallocated.                                     */
    /* Don't term the port here - it must be done by QCM because of asynch   */
    /* error conditions from XDH, which rely on close channel. The port has  */
    /* therefore already been closed by qlcclose calling qpm_port_terminate  */
    /* Free the channel                                                      */
    /*************************************************************************/
    qcm_rc = qcm_free_channel((channel_type *)*channel_id);
    switch (qcm_rc)
    {
    case qcm_rc_ok:
      qdh_rc = 0;
      break;
    case qcm_rc_channel_not_found:
      qdh_rc = EINVAL;
      break;
    case qcm_rc_free_failed:
      qdh_rc = EFAULT;
      break;
    case qcm_rc_invalid_command:
      qdh_rc = EINVAL;
      break;
    default:
      qdh_rc = EFAULT;
	break;
    }
    outputf("QLCMPX returning: \n");
    qllc_state();
    return(qdh_rc);
  }
  /***************************************************************************/
  /* If the port_name is not null, the user wishes to allocate a channel     */
  /* Note that the channel_id parm is passed by reference to the QCM.        */
  /***************************************************************************/
  outputf("QLCMPX: channel is to be allocated.\n");

  /***************************************************************************/
  /* Build xdh path name                                                     */
  /***************************************************************************/
  strcpy(path,"/dev/");
  strcpy(&path[5], port_name);
  outputf("path_name for xdh port = %s\n",path);

  /***************************************************************************/
  /* If the device handler pathname extension was found and resolved, then   */
  /* proceed with the allocation of the channel.                             */
  /* Note that channel_id is passed to the QCM by reference.                 */
  /***************************************************************************/
  if (qcm_allocate_channel((channel_type *)channel_id) != qcm_rc_ok)
  {
    outputf("QLCMPX: channel allocation failed\n");
    *channel_id = (-1);
    return(ENOMEM);
  }
  else
  {
    /*************************************************************************/
    /* Channel Manager has set up channel, initialised it, &                 */
    /* and has returned channel_id, which is in fact a ptr to the channel,   */
    /* but is cast to an int to conform to the interface specification.      */
    /*************************************************************************/
    /*************************************************************************/
    /* Initialise Port                                                       */
    /* qpm_port_initialise expects all resources to be UNLOCKED.             */
    /*************************************************************************/
    qpm_rc = qpm_port_initialise(&returned_port_id,path);
    if (qpm_rc != qpm_rc_ok)
    {
      outputf("QLCMPX: port_initialisation failed.\n");
      qdh_rc = EFAULT;
    }
    else
    {
      (void)lockl(&(((channel_type *)(*channel_id))->lock),0);
      ((channel_type *)(*channel_id))->port_id = returned_port_id;
      (void)unlockl(&(((channel_type *)(*channel_id)))->lock);
      qdh_rc = 0;
    }
    return(qdh_rc);
  }
}


/*****************************************************************************/
/* Function     qlcopen                                                      */
/*                                                                           */
/* Description  Initialises a channel previously allocated by qlcmpx         */
/*              Sets the parameters to default, unless overridden by         */
/*              user.                                                        */
/*              Note that the port has already been initialised by the       */
/*              qlcmpx routine. The kernel process catching events on the    */
/*              port has also already been created and started.              */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters   devno     - major and minor QLLC device numbers              */
/*              openflag  - indicates whether kernel user and no delay apply */
/*              chan      - id of channel being opened.                      */
/*              ext       - open extension as defined in GDLC/QLLC specs.    */
/*****************************************************************************/
int qlcopen(

  dev_t          devno,
  unsigned long  devflag,
  int            chan,
  int            ext)
{
  int qdh_rc = 0;
  /***************************************************************************/
  /* Declare local variables of corret type to avoid casts                   */
  /***************************************************************************/
  channel_id_type      channel_id;
  struct dlc_open_ext  open_extension;
  struct dlc_open_ext *open_ext_ptr;
  
  outputf("QLCOPEN: called\n");
  outputf( "chan == %x\n", chan );
  /***************************************************************************/
  /* If the user is not in the kernel, copyin the open extension from user   */
  /* space into kernel space.                                                */
  /***************************************************************************/
  if ((devflag & DKERNEL) != DKERNEL)
  {
    outputf("QLCOPEN: copyin open ext\n");
    qdh_rc = copyin(ext,&open_extension,sizeof(struct dlc_open_ext));
    if (qdh_rc != 0)
    {
      return(qdh_rc);
    }
    open_ext_ptr = &open_extension;
  }
  else
  {
    /*************************************************************************/
    /* Kernel User                                                           */
    /*************************************************************************/
    open_ext_ptr = (struct dlc_open_ext *)ext;
  }
  /***************************************************************************/
  /* TRACE TEST                                                              */
  /***************************************************************************/
/*
  outputf("QLCOPEN: HKWD_DD_X25QLLC = %d\n",HKWD_DD_X25QLLC);
  outputf("QLCOPEN: DD_ENTRY_OPEN = %d\n",DD_ENTRY_OPEN);
  DDHKWD5(HKWD_DD_X25QLLC,DD_ENTRY_OPEN,0,devflag,chan,ext,0,0);
*/
  /***************************************************************************/
  /* ERROR LOGGING TEST                                                      */
  /***************************************************************************/
  /* errsave(ERRID_QLCOPEN_TESTRUN,ERR_REC_SIZE); */

  /***************************************************************************/
  /* Initialise local variables                                              */
  /***************************************************************************/
  channel_id = (channel_id_type)chan;
  /***************************************************************************/
  /* Save devno and devflag in channel                                       */
  /***************************************************************************/
  channel_id->devno = devno;
  /***************************************************************************/
  /* Call the QCM which will open the channel, if possible.                  */
  /***************************************************************************/
  if (qcm_open_channel(channel_id,open_ext_ptr,devflag) != qcm_rc_ok)
  {
    outputf("QLCOPEN: open channel failed.\n");
    qdh_rc = EFAULT;
  }
  else
  {
    outputf("QLCOPEN: open channel OK.\n");
    qdh_rc = 0;
  }
  outputf("QLCOPEN: Returning\n");
  qllc_state();
  outputf("QLCOPEN: returning %d\n",qdh_rc);
  return(qdh_rc);
}

/*****************************************************************************/
/* Function     qlcclose                                                     */
/*                                                                           */
/* Description  Closes a channel at the request of the user.                 */
/*              qlcclose is called prior to qlcmpx being called to           */
/*              deallocate the channel.                                      */
/*              If this is the last channel using the port identified in the */
/*              channel by port_id, then the port is terminated, and the     */
/*              kernel process managing the port is terminated.              */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters  devno  - major and minor QLLC device numbers                  */
/*             chan   - ID of channel to be closed                           */
/*             ext    - ignored by QLLC                                      */
/*****************************************************************************/
int qlcclose(
  dev_t   devno,
  int     chan,
  int     ext)
{
  int              qdh_rc = 0;
  channel_id_type  channel_id;
  qcm_rc_type      qcm_rc;
  /***************************************************************************/
  /* Initialise local variables                                              */
  /***************************************************************************/
  channel_id = (channel_id_type)chan;
  /***************************************************************************/
  /* Call QCM to close channel.                                              */
  /***************************************************************************/
  qcm_rc = qcm_close_channel(channel_id);
  switch (qcm_rc)
  {
  case qcm_rc_ok:
    qdh_rc = 0;
    break;
  case qcm_rc_channel_not_found:
    qdh_rc = EINVAL;
    break;
  default:
    qdh_rc = EFAULT;
    break;
  }
  return(qdh_rc);
}

/*****************************************************************************/
/* Function     qlcread                                                      */
/*                                                                           */
/* Description                                                               */
/*              This procedure is called by application users to get         */
/*              receive data from the channel queue. The read ext            */
/*              specifies whether it is normal, xid, or netd data.           */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters   devno - major and minor device numbers of QLLC               */
/*              uiop  - address of uio structure to hold data                */
/*              chan  - id of channel to be read from                        */
/*              ext   - address of extended i/o structure                    */
/*****************************************************************************/
int  qlcread(
 
  dev_t         devno,
  struct uio   *uiop,
  int           chan,
  int           ext)
{
  int                sys_rc;
  int                qdh_rc = 0;
  struct dlc_io_ext  read_ext;
  channel_id_type    channel_id;
  gen_buffer_type   *buf_ptr;
  int                iomove_rc;
  qcm_rc_type        qcm_rc;
  int                buf_length,uio_length;
 
  /***************************************************************************/
  /* Get Read Extension from User Space to Kernel Space.                     */
  /* (Kernel Users don't issue reads).                                       */
  /***************************************************************************/
  sys_rc = copyin(ext, &read_ext, sizeof(struct dlc_io_ext));
  if (sys_rc != 0)
  {
    return(sys_rc);
  }
  /***************************************************************************/
  /* Initialise local variables                                              */
  /***************************************************************************/
  channel_id = (channel_id_type)chan;
  lockl(&(channel_id->lock),0);
  /***************************************************************************/
  /* Read calls should only be received from Application mode users          */
  /***************************************************************************/
  if (channel_id->user_nature == kernel_user)
  {
    qdh_rc = EINVAL;
    unlockl(&(channel_id->lock));
  }
  else
  {
    /*************************************************************************/
    /* User is an application user so call is valid. Call Channel Manager    */
    /* which will return data from channel queue, in an mbuf.                */
    /* QCM builds read extension from that in buffer so there is no need to  */
    /* concern this proc with any fields other than the dlh_length field.    */
    /* Decide whether to pass header or not. Then QLLC Device Head issues    */
    /* iomove to transfer data out to user space.                            */
    /* Note that an empty queue is not considered to be an error condition,  */
    /* and that the QCM rc is 0 if a read is issued to an empty queue        */
    /*                                                                       */
    /* Note that the buf_ptr is initialised by the QCM so it is passed by    */
    /* reference.                                                            */
    /*************************************************************************/
    unlockl(&(channel_id->lock));
    qcm_rc = 
      qcm_read_data_queue(channel_id, &buf_ptr, (read_ext_type *)&read_ext);
    lockl(&(channel_id->lock),0);
    if (qcm_rc != qcm_rc_ok)
    {
      /***********************************************************************/
      /* QCM will have logged any error, so no need to log another here.     */
      /***********************************************************************/
      switch (qcm_rc)
      {
      case qcm_rc_channel_not_found:
	unlockl(&(channel_id->lock));
	qdh_rc = EINVAL;
	break;
      default:
	unlockl(&(channel_id->lock));
	qdh_rc = EFAULT;
	break;
      }
    }
    else
    {
      /***********************************************************************/
      /* Either queue was empty, or data has been returned in buffer_ptr.    */
      /* Find out if queue was empty. If so, simply return. It is not an     */
      /* error condition, as the user can freely issue reads, whether there  */
      /* is data waiting or not.                                             */
      /***********************************************************************/
      if (buf_ptr == NULL)
      {
	/*********************************************************************/
	/* The queue is empty.                                               */
	/* You must determine whether the read is blocking or non-blocking   */
	/* This is dependent on channel devflag.                             */
	/*********************************************************************/
	outputf("QLCREAD: channel manager has no rx data\n");
	if ((channel_id->devflag & DNDELAY))
	{
	  outputf("QLCREAD: channel open in non-blocking mode\n");
	  /*******************************************************************/
	  /* The device is open in non-blocking mode. You must return        */
	  /* immediately.                                                    */
	  /*******************************************************************/
	  unlockl(&(channel_id->lock));
	  return(0);
	}
	else
	{
	  outputf("QLCREAD: channel open in blocking mode\n");
	  /*******************************************************************/
	  /* Device is open in blocking mode....you must make the user sleep */
	  /* until data arrives.                                             */
	  /*******************************************************************/
	  outputf("QLCREAD: unlock channel\n");
          unlockl(&(channel_id->lock));
	  outputf("QLCREAD: channel unlocked, sleep...\n");
	  if (e_sleep(&(channel_id->readsleep),0)!= EVENT_SUCC)
	  {
	    /*****************************************************************/
	    /* The sleep was interrupted                                     */
	    /*****************************************************************/
	    outputf("QLCREAD: sleep was interrupted\n");
	    return(EINTR);
	  }
	  outputf("QLCREAD: re-lock channel\n");
	  lockl(&(channel_id->lock),0);
	}
       	/*********************************************************************/
	/* Re-test queue by calling QCM.                                     */
	/*********************************************************************/
	outputf("QLCREAD: unlock channel\n");
        unlockl(&(channel_id->lock));	
	outputf("QLCREAD: channel unlocked\n");
	qcm_rc = 
	  qcm_read_data_queue(channel_id,&buf_ptr,(read_ext_type *)&read_ext);
	outputf("QLCREAD: re-lock channel\n");
	lockl(&(channel_id->lock),0);
	if (qcm_rc != qcm_rc_ok)
	{
	  /*******************************************************************/
	  /* Channel Manager experienced error                               */
	  /* QCM will have logged any error, so no need to log another here. */
	  /*******************************************************************/
	  switch (qcm_rc)
	  {
	  case qcm_rc_channel_not_found:
	    unlockl(&(channel_id->lock));
	    qdh_rc = EINVAL;
	    break;
	  default:
	    unlockl(&(channel_id->lock));
	    qdh_rc = EFAULT;
	    break;
	  }
	}
	else
	{
	  /*******************************************************************/
	  /* Channel Mgr returned successfully.                              */
	  /*******************************************************************/
	  if (buf_ptr == NULL)
	  {
	    /*****************************************************************/
	    /* Queue was empty. This is not possible since the head process  */
	    /* has just been woken due to a data receipt, therefore report   */
	    /* that a system error has occurred.                             */
	    /*****************************************************************/
	    unlockl(&(channel_id->lock));
	    return(EFAULT);
	  }
	  else
	  {
	    /*****************************************************************/
	    /* Data returned by channel manager.                             */
	    /* Drop through to domain that handles data receipt.             */
	    /*****************************************************************/
	    ;
	  }
	}
      }
      outputf("QLCREAD: channel manager found data\n");
      outputf("QLCREAD: read extension flags = %x\n",read_ext.flags);
      /***********************************************************************/
      /* There was data on the queue. It must now be moved to user space.    */
      /* Check that the uio structure the user has passed is big enough for  */
      /* the data. If not set the OFLO bit in the dlc_io_ext.                */
      /* Note that DH to QLLC overflows will have been taken care of already */
      /* buf_length is the amount of data to be got from the mbuf.           */
      /* uio_length is the amount of available sapce in the uio.             */
      /***********************************************************************/
      if (read_ext.flags & DLC_XIDD)
      {
	outputf("QLCREAD: reading XIDD\n");
	/*********************************************************************/
	/* See if the user wants DLC header info                             */
	/*********************************************************************/
	if (read_ext.dlh_len != 0)
	{
	  outputf("QLCREAD: user has requested dlc header info\n");
	  /*******************************************************************/
	  /* The user wants QLLC Header info pre-pended to data.             */
	  /*******************************************************************/
	  read_ext.dlh_len = 2;
	}
	/*********************************************************************/
	/* Set buf_length to length of data only. i.e.exclude the length of  */
	/* dlc header info.                                                  */
	/*********************************************************************/
	buf_length = JSMBUF_LENGTH(buf_ptr) - 
	  OFFSETOF(body.qllc_body.user_data[0],x25_mbuf_t);
	uio_length = uiop->uio_resid;
	outputf("QLCREAD: buf_length = %d\n",buf_length);
	outputf("QLCREAD: dlh_length = %d\n",read_ext.dlh_len);
	outputf("QLCREAD: uio_length = %d\n",uio_length);
	if (buf_length+read_ext.dlh_len > uio_length)
	{
	  outputf("QLCREAD: truncating buffer\n");
	  read_ext.flags |= DLC_OFLO;
	  buf_length = uio_length-read_ext.dlh_len;
	}
	outputf("QLCREAD: iomove buffer into user space uio structure\n");
	outputf("QLCREAD: read_ext.dlh_len = %d\n",read_ext.dlh_len);
	print_x25_buffer(buf_ptr);
	outputf("QLCREAD: amt of data to be copied = %d\n",
	  buf_length+read_ext.dlh_len);
	iomove_rc = JSMBUF_IOMOVEOUT(
	  buf_ptr,
	  (unsigned)
	   (OFFSETOF(body.qllc_body.user_data[0],x25_mbuf_t)-read_ext.dlh_len),
	  uiop,
	  (unsigned)(buf_length + read_ext.dlh_len)
	  );
      }
      else
      {
	outputf("QLCREAD: reading NORM\n");
	/*********************************************************************/
	/* This is normal data, just send the data, and don't pass any dlc   */
	/* header fields.                                                    */
	/*********************************************************************/
	read_ext.dlh_len = 0;
	buf_length = JSMBUF_LENGTH(buf_ptr) - X25_OFFSETOF_USER_DATA;
	uio_length = uiop->uio_resid;
	outputf("QLCREAD: buf_length = %d\n",buf_length);
	outputf("QLCREAD: uio_length = %d\n",uio_length);
	if (buf_length > uio_length)
	{
	  outputf("QLCREAD: truncating buffer\n");
	  read_ext.flags |= DLC_OFLO;
	  buf_length = uio_length;
	}
	outputf("QLCREAD: iomove buffer into user space uio structure\n");
	outputf("QLCREAD: read_ext.dlh_len = %d\n",read_ext.dlh_len);
	print_x25_buffer(buf_ptr);
	outputf("QLCREAD: amt of data to be copied = %d\n",buf_length);
	iomove_rc = JSMBUF_IOMOVEOUT(
	  buf_ptr,
	  (unsigned)X25_OFFSETOF_USER_DATA,
	  uiop,
	  (unsigned)buf_length
	  );
      }
      /***********************************************************************/
      /* Copyout read extension back to user space.                          */
      /***********************************************************************/
      outputf("QLCREAD: copyout read ext\n");
      sys_rc = copyout(&read_ext,ext,sizeof(struct dlc_io_ext));
      if (sys_rc != 0)
      {
	unlockl(&(channel_id->lock));
	return(sys_rc);
      }
      if (iomove_rc != 0)
      {
	outputf("QLCREAD: iomoveout failed\n");
	qdh_rc = EFAULT;
      }
      /***********************************************************************/
      /* Whether the iomove was successful or not, the buffer that was used  */
      /* to queue the data in the channel must be freed.                     */
      /***********************************************************************/
      QBM_FREE_BUFFER(buf_ptr);
      unlockl(&(channel_id->lock));
    }
  }
  return(qdh_rc);
}

/*****************************************************************************/
/* Function     qlcwrite                                                     */
/*                                                                           */
/* Description                                                               */
/*             This procedure is called when a user wishes to transmit       */
/*             data to a remote node.                                        */
/*                                                                           */
/*             The write ext indicates whether the data is normal or         */
/*             xid data.                                                     */
/*                                                                           */
/*             Note that QLLC does not support writes of network data        */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
int  qlcwrite (
  dev_t       devno,
  struct uio *uiop,
  int         chan,
  int         ext)
{
  int                  qdh_rc;
  channel_type        *channel_id;
  struct dlc_io_ext    write_ext;
  struct dlc_io_ext   *write_ext_ptr;
  gen_buffer_type     *buf_ptr;
  gen_buffer_type     *new_buf;
  qlm_rc_type          qlm_rc;

  channel_id = (channel_type *)chan;
  /***************************************************************************/
  /* If the user is not in the kernel, copyin the write extension from user  */
  /* space into kernel space, get an mbuf and uiomove the data from the uio  */
  /* structure the user passed, into the mbuf.                               */
  /***************************************************************************/
  if ((channel_id->devflag & DKERNEL) != DKERNEL)
  {
    outputf("QLCWRITE: user is an appl user.\n");
    qdh_rc = copyin(ext,&write_ext,sizeof(struct dlc_io_ext));
    if (qdh_rc != 0)
    {
      outputf("QLCWRITE: copyin failed\n");
      return(qdh_rc);
    }
    write_ext_ptr = &write_ext;
    /*************************************************************************/
    /* User's data is in uio structure. Get an mbuf/cluster, and iomove      */
    /* data into it to pass it to QLM                                        */
    /*************************************************************************/
    /*************************************************************************/
    /* The data length will be uiop->uio_resid, and the allocated buffer must*/
    /* be able to accommodate resid + WRITE_DATA_OFFSET.  The offset allows  */
    /* enough space for mbuf header, qllc header, packet data and qllc addr  */
    /* and control fields.                                                   */
    /*************************************************************************/
    outputf("QLCWRITE: get a buffer to copy appl space data into.\n");
    outputf("QLCWRITE: uiop->uio_resid   = %d\n",uiop->uio_resid);
    outputf("QLCWRITE: write_data_offset = %d\n",WRITE_DATA_OFFSET);

    if ( write_ext_ptr->flags & DLC_XIDD )
    {
      outputf("QLCWRITE: writing XIDD\n");
      /***********************************************************************/
      /* Get a buffer big enough to hold the data and the qllc address and   */
      /* control fields that will be needed.                                 */
      /***********************************************************************/
      buf_ptr = (gen_buffer_type *)QBM_GET_BUFFER(uiop->uio_resid + 2);
      if ( buf_ptr == (gen_buffer_type *)NULL)
	return(EAGAIN);
      /***********************************************************************/
      /* Iomove the data into the mbuf, starting at the qllc_body data area  */
      /* offset, hence leaving space for address and control fields.         */
      /***********************************************************************/
      outputf("QLCWRITE: iomovein the data\n");
      qdh_rc = QBM_IOMOVEIN(
	buf_ptr,
	(unsigned)(OFFSETOF(body.qllc_body.user_data[0],x25_mbuf_t)),
	uiop,
	(unsigned)(uiop->uio_resid));
    }
    else 
    {
      /***********************************************************************/
      /* Get a buffer big enough for the data. There will be no QLLC address */
      /* and control fields to accommodate.                                  */
      /***********************************************************************/
      buf_ptr = (gen_buffer_type *)QBM_GET_BUFFER(uiop->uio_resid);
      if ( buf_ptr == (gen_buffer_type *)NULL)
	return(EAGAIN);
      /***********************************************************************/
      /* Use MBUF macro to copy data from user space into the mbuf.          */
      /***********************************************************************/
      outputf("QLCWRITE: iomovein the data\n");
      qdh_rc = QBM_IOMOVEIN(
	buf_ptr,
	(unsigned)(X25_OFFSETOF_USER_DATA),
	uiop,
	(unsigned)(uiop->uio_resid));
    }
    if (qdh_rc != 0)
    {
      outputf("QLCWRITE: iomovein failed\n");
      return(EINVAL);
    }
    print_x25_buffer(buf_ptr);
  }
  else
  {
    outputf("QLCWRITE: user is a kernel user.\n");
    /*************************************************************************/
    /* The user is a KERNEL USER. The uiop contains a pointer to an mbuf.    */
    /* The data can be either XIDD data or NORM data.                        */
    /* It will begin at an offset of 0 bytes from the start of the data area */
    /* because QLLC will have returned a Write_Data_Offset of 0 on the       */
    /* STATION_STARTED result.                                               */
    /* This is so that the data for normal I-frames, which does not have any */
    /* qllc address and control fields does not have to be copied before     */
    /* passing it to the xdh.                                                */
    /* Therefore if it's XIDD, QLLC gets an additional mbuf to chain to the  */
    /* front of the data, so that the a and c fields can be accommodated.    */
    /*************************************************************************/
    write_ext_ptr = (struct dlc_io_ext *)ext;
    outputf("QLCWRITE: get buffer_ptr from uio into buf_ptr.\n");

    /*************************************************************************/
    /* This used to be a uiomove, but just pull ptr out directly now         */
    /*************************************************************************/
    buf_ptr = (gen_buffer_type *)uiop->uio_iov->iov_base;

    /*************************************************************************/
    /* Allocate new buffer to form correct buffer header.                    */
    /*************************************************************************/
    if ((write_ext_ptr->flags & DLC_XIDD) == DLC_XIDD)
      new_buf=QBM_GET_BUFFER(OFFSETOF(body.qllc_body.user_data[0],x25_mbuf_t));
    else
      new_buf=QBM_GET_BUFFER(OFFSETOF(body.user_data[0],x25_mbuf_t));

    if ( new_buf == (gen_buffer_type *)NULL)
	return(EFAULT);
    JSMBUF_CAT(new_buf,buf_ptr);
    buf_ptr = new_buf;
  }
  /***************************************************************************/
  /* Call the QLM to send the data.                                          */
  /* TRUE indicates to qlm_write that the write is blocking                  */
  /***************************************************************************/
  outputf("QLCWRITE: calling qlm_write\n");
  qlm_rc = qlm_write(write_ext_ptr,buf_ptr);
  outputf("QLCWRITE: qlm_write rc = %d\n",qlm_rc);

  /****************************/
  /* free the buffer on error */
  /****************************/
  if (qlm_rc != qlm_rc_ok)
  {
     if ((channel_id->devflag & DKERNEL) == DKERNEL)
     {
	/**************************************************/
	/* just get rid of mbuf buffer header QLLC added. */
	/**************************************************/
	buf_ptr->m_next = (gen_buffer_type *)NULL;
     }
     QBM_FREE_BUFFER(buf_ptr);
  }

  switch (qlm_rc)
  {
  case qlm_rc_station_not_found:
    qdh_rc = EINVAL;
    break;
  case qlm_rc_close_vc_failed:
    qdh_rc = EFAULT;
    break;
  case qlm_rc_system_error:
    qdh_rc = EFAULT;
    break;
  case qlm_rc_protocol_error:
    qdh_rc = EINVAL;
    break;
  case qlm_rc_user_interface_error:
    qdh_rc = EINVAL;
    break;
  case qlm_rc_ok:
    qdh_rc = 0;
    break;
  default:
    qdh_rc = EFAULT;
    break;
  }
  return (qdh_rc);
}


/*****************************************************************************/
/* Function     qlcioctl                                                     */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/*             This is the ioctl entry point for QLLC.                       */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
int  qlcioctl(
  dev_t         devno,
  int           op,
  int           arg,
  unsigned long devflag,
  int           chan,
  int           ext)
{
  bool             appl_user;
  int              rc;
  int              qdh_rc;
  int              coverage_points;
  qsm_rc_type      qsm_rc;
  qlm_rc_type      qlm_rc;
  qcm_rc_type      qcm_rc;
  ioctl_ext_type   ioctl_ext;
  ioctl_ext_type   *ioctl_ext_ptr;
  channel_id_type  channel_id;
  struct dlc_getx_arg *excep_ptr;

  /***************************************************************************/
  /* Initialise local variables.                                             */
  /***************************************************************************/
  channel_id = (channel_type *)chan;
  if (channel_id->user_nature == application_user)
  {
    appl_user = TRUE;
    outputf("QLCIOCTL: application user\n");
  }
  else
  {
    appl_user = FALSE;
    outputf("QLCIOCTL: kernel user\n");
  }
  /***************************************************************************/
  /* Decide which of the sub-component functions should be called based on   */
  /* the operation selected by the user.                                     */
  /***************************************************************************/
  outputf("QLCIOCTL: ioctl op = %d\n",op);
  switch (op)
  {
  case (DLC_ENABLE_SAP) :
    /*************************************************************************/
    /* Ioctl is for ENABLE_SAP                                               */
    /* Exception blocks are sent by QSM so there is no need                  */
    /* for this procedure to send a good or bad result.                      */
    /*************************************************************************/
    if (appl_user)
    {
      outputf("QLCIOCTL: copyin enable_sap arg from user space\n");
      outputf("QLCIOCTL: size of arg block =%d\n",sizeof(struct dlc_esap_arg));
      qdh_rc = copyin(arg,&ioctl_ext,sizeof(struct dlc_esap_arg));
      outputf("QLCIOCTL: copyin rc = %d\n",qdh_rc);
      if (qdh_rc != 0)
      {
	return(qdh_rc);
      }
      ioctl_ext_ptr = &ioctl_ext;
    }
    else
    {
      outputf("QLCIOCTL: assign ptr to ext arg\n");
      ioctl_ext_ptr = (ioctl_ext_type *)arg;
    }
    /*************************************************************************/
    /* For either type of user                                               */
    /*************************************************************************/
    outputf("..enable sap arg block contains:\n");
    outputf("Returned QLLC SAP Correlator=%x\n",
      ioctl_ext_ptr->enable_sap.gdlc_sap_corr);
    outputf("User SAP Correlator         =%x\n",
      ioctl_ext_ptr->enable_sap.user_sap_corr);
    outputf("Maximum Link Stations       =%d\n",
      ioctl_ext_ptr->enable_sap.max_ls);
    outputf("Local Address Length        =%d\n",
      ioctl_ext_ptr->enable_sap.len_laddr_name);
    outputf("Local Address               =%s\n",
      ioctl_ext_ptr->enable_sap.laddr_name);
    qsm_rc = qsm_enable_sap(channel_id,(struct dlc_esap_arg *)ioctl_ext_ptr);
    outputf("rc = %d\n",qsm_rc);
    switch (qsm_rc)
    {
    case qsm_rc_ok:
      qdh_rc = 0;
      break;
    case qsm_rc_sap_limit_reached :
    case qsm_rc_max_link_stations_invalid :
      qdh_rc = EINVAL;
      break;
    case qsm_rc_alloc_failed :
    case qsm_rc_system_error :
    default :
      qdh_rc = EFAULT;
    }
    if (appl_user)
    {
      rc = copyout(&ioctl_ext,arg,sizeof(struct dlc_esap_arg));
      if (rc != 0)
      {
	return(EFAULT);
      }
    }
    break;

  case (DLC_DISABLE_SAP) :
    /*************************************************************************/
    /* Ioctl is for DISABLE_SAP                                              */
    /* Exception blocks are sent by QSM so there is no need for this         */
    /* procedure to send a good or bad result.                               */
    /* Ensure silent arg (last arg to disable) is et to FALSE so result is   */
    /* generated                                                             */
    /*************************************************************************/
    if (appl_user)
    {
      qdh_rc = copyin(arg,&ioctl_ext,sizeof(struct dlc_corr_arg));
      if (qdh_rc != 0)
      {
	return(qdh_rc);
      }
      ioctl_ext_ptr = &ioctl_ext;
    }
    else
    {
      ioctl_ext_ptr = (ioctl_ext_type *)arg;
    }
    qsm_rc = qsm_disable_sap(
      channel_id,
      (correlator_type)(ioctl_ext_ptr->disable_sap.gdlc_sap_corr),
      FALSE);
    switch (qsm_rc)
    {
    case qsm_rc_ok :
      qdh_rc = 0;
      break;
    case qsm_rc_no_such_sap :
      qdh_rc = EINVAL;
      break;
    case qsm_rc_system_error :
    default :
      qdh_rc = EFAULT;

    }
    break;

  case (DLC_QUERY_SAP) :
    /*************************************************************************/
    /* Ioctl is for QUERY_SAP                                                */
    /* This procedure is synchronous. That is, the statistics                */
    /* passed in as the ioctl_ext must be used to pass back                  */
    /* the statistics to the user.                                           */
    /* The QSM fills in some of the ext structure, and the                   */
    /* DH fills in the rest.                                                 */
    /*************************************************************************/
    if (appl_user)
    {
      qdh_rc = copyin(arg,&ioctl_ext,sizeof(struct qlc_qsap_arg));
      if (qdh_rc != 0)
      {
	return(qdh_rc);
      }
      ioctl_ext_ptr = &ioctl_ext;
    }
    else
    {
      ioctl_ext_ptr = (ioctl_ext_type *)arg;
    }
    qsm_rc = qsm_query_sap(channel_id,(struct dlc_qsap_arg *)ioctl_ext_ptr);
    switch (qsm_rc)
    {
    case qsm_rc_ok :
      qdh_rc = 0;
      break;
    case qsm_rc_no_such_sap :
      qdh_rc = EINVAL;
      break;
    case qsm_rc_system_error :
    default :
      qdh_rc = EFAULT;
    }
    if (appl_user)
    {
      rc = copyout(&ioctl_ext,arg,sizeof(struct qlc_qsap_arg));
      if (rc != 0)
      {
	return(EFAULT);
      }
    }
    break;

  case (DLC_START_LS) :
    /*************************************************************************/
    /* Ioctl is for START_LS                                                 */
    /* Exception blocks are sent by QLM so there is no need                  */
    /* for this procedure to send a good or bad result.                      */
    /*************************************************************************/
    if (appl_user)
    {
      qdh_rc = copyin(arg,&ioctl_ext,sizeof(struct qlc_sls_arg));
      if (qdh_rc != 0)
      {
	return(qdh_rc);
      }
      ioctl_ext_ptr = &ioctl_ext;
    }
    else
    {
      ioctl_ext_ptr = (ioctl_ext_type *)arg;
    }
    qlm_rc = qlm_start_ls(channel_id,ioctl_ext_ptr);
    outputf("QLCIOCTL: qlm_start_ls rc = %d\n",qlm_rc);
    switch (qlm_rc)
    {
    case qlm_rc_ok:
      qdh_rc = 0;
      break;
    case qlm_rc_station_limit_reached :
    case qlm_rc_user_interface_error :
    case qlm_rc_invalid_remote_name :
    case qlm_rc_contention :
    case qlm_rc_bad_listen_name :
      qdh_rc = EINVAL;
      break;
    case qlm_rc_alloc_failed :
    case qlm_rc_port_error :
    case qlm_rc_open_vc_failed :
    case qlm_rc_system_error :
    default :
      qdh_rc = EFAULT;
      break;
    }
    if (appl_user)
    {
      rc = copyout(&ioctl_ext,arg,sizeof(struct qlc_sls_arg));
      if (rc != 0)
      {
	return(EFAULT);
      }
    }
    break;

  case (DLC_HALT_LS) :
    /*************************************************************************/
    /* Ioctl is for HALT_LS                                                  */
    /* Exception blocks are sent by QLM so there is no need                  */
    /* for this procedure to send a good or bad result.                      */
    /*************************************************************************/
    if (appl_user)
    {
      qdh_rc = copyin(arg,&ioctl_ext,sizeof(struct dlc_corr_arg));
      if (qdh_rc != 0)
      {
	return(qdh_rc);
      }
      ioctl_ext_ptr = &ioctl_ext;
    }
    else
    {
      ioctl_ext_ptr = (ioctl_ext_type *)arg;
    }
    qlm_rc = qlm_halt_ls(
      (station_type *)(ioctl_ext_ptr->halt_ls.gdlc_ls_corr),
      FALSE,                  /* not SILENT - expect normal result generation */
      TRUE                    /* send a discontact message */
      );
    switch (qlm_rc)
    {
    case qlm_rc_ok:
      qdh_rc = 0;
      break;
    case qlm_rc_station_not_found:
      qdh_rc = EINVAL;
      break;
    case qlm_rc_close_vc_failed:
      qdh_rc = EFAULT;
      break;
    default:
      qdh_rc = EFAULT;
      break;
    }
    break;

  case (DLC_QUERY_LS) :
    /*************************************************************************/
    /* Ioctl is for QUERY_LS                                                 */
    /* This procedure is synchronous. That is, the statistics                */
    /* passed in as the ioctl_ext must be used to pass back                  */
    /* the statistics to the user.                                           */
    /* The QLM fills in some of the ext structure, and the                   */
    /* DH fills in the rest.                                                 */
    /*************************************************************************/
    if (appl_user)
    {
      qdh_rc = copyin(arg,&ioctl_ext,sizeof(struct dlc_qls_arg));
      if (qdh_rc != 0)
      {
	return(qdh_rc);
      }
      ioctl_ext_ptr = &ioctl_ext;
    }
    else
    {
      ioctl_ext_ptr = (ioctl_ext_type *)arg;
    }
    qlm_rc = qlm_query_ls(ioctl_ext_ptr);
    outputf("QLCIOCTL: qlm_query_ls rc = %d\n",qlm_rc);
    switch (qlm_rc)
    {
    case qlm_rc_ok:
      qdh_rc = 0;
      break;
    case qlm_rc_station_not_found:
      qdh_rc = EINVAL;
      break;
    case qlm_rc_system_error :
    default :
      qdh_rc = EFAULT;                          
      break;
    }
    if (appl_user)
    {
      rc = copyout(&ioctl_ext,arg,sizeof(struct dlc_qls_arg));
      if (rc != 0)
      {
	return(EFAULT);
      }
    }
    break;

  case (DLC_TRACE) :
    /*************************************************************************/
    /* Ioctl is for TRACE_LS                                                 */
    /* There are no asynchronous results connected with this                 */
    /* procedure. Only a good or bad return code                             */
    /*************************************************************************/
    if (appl_user)
    {
      qdh_rc = copyin(arg,&ioctl_ext,sizeof(struct dlc_trace_arg));
      if (qdh_rc != 0)
      {
	return(qdh_rc);
      }
      ioctl_ext_ptr = &ioctl_ext;
    }
    else
    {
      ioctl_ext_ptr = (ioctl_ext_type *)arg;
    }
    qlm_rc = qlm_trace(ioctl_ext_ptr);
    outputf("QLCIOCTL: qlm_trace rc = %d\n",qlm_rc);
    switch (qlm_rc)
    {
    case qlm_rc_ok:
      qdh_rc = 0;
      break;
    case qlm_rc_station_not_found:
      qdh_rc = EINVAL;
      break;
    default :
      qdh_rc = EFAULT;                          
      break;
    }
    break;

  case (DLC_CONTACT) :
    /*************************************************************************/
    /* Ioctl is for CONTACT_LS                                               */
    /* Exception blocks are sent by QLM so there is no need                  */
    /* for this procedure to send a good or bad result.                      */
    /*************************************************************************/
    if (appl_user)
    {
      qdh_rc = copyin(arg,&ioctl_ext,sizeof(struct dlc_corr_arg));
      if (qdh_rc != 0)
      {
	return(qdh_rc);
      }
      ioctl_ext_ptr = &ioctl_ext;
    }
    else
    {
      ioctl_ext_ptr = (ioctl_ext_type *)arg;
    }
    outputf("..dlc corr arg block contains:\n");
    outputf("QLLC SAP Correlator=%x\n",ioctl_ext_ptr->contact.gdlc_sap_corr);
    outputf("QLLC LS Correlator=%x\n",ioctl_ext_ptr->contact.gdlc_ls_corr);

    qlm_rc = qlm_contact(ioctl_ext_ptr);
    outputf("QLCIOCTL: qlm_contact rc = %d\n",qlm_rc);
    switch (qlm_rc)
    {
    case qlm_rc_ok:
      qdh_rc = 0;
      break;
    case qlm_rc_station_not_found:
      qdh_rc = EINVAL;
      break;
    case qlm_rc_protocol_error:
      qdh_rc = EINVAL;
      break;
    default :
      qdh_rc = EFAULT;                            
      break;
    }
    break;

  case (DLC_TEST) :
    /*************************************************************************/
    /* Ioctl is for TEST_LS                                                  */
    /* Exception blocks are sent by QLM so there is no need                  */
    /* for this procedure to send a good or bad result.                      */
    /*************************************************************************/
    if (appl_user)
    {
      qdh_rc = copyin(arg,&ioctl_ext,sizeof(struct dlc_corr_arg));
      if (qdh_rc != 0)
      {
	return(qdh_rc);
      }
      ioctl_ext_ptr = &ioctl_ext;
    }
    else
    {
      ioctl_ext_ptr = (ioctl_ext_type *)arg;
    }
    qlm_rc = qlm_test(ioctl_ext_ptr);
    outputf("QLCIOCTL: qlm_test rc = %d\n",qlm_rc);
    switch (qlm_rc)
    {
    case qlm_rc_ok:
      qdh_rc = 0;
      break;
    case qlm_rc_station_not_found:
      qdh_rc = EINVAL;
      break;
    case qlm_rc_buffer_unavailable:
      qdh_rc = EFAULT;
      break;
    case qlm_rc_protocol_error:
      qdh_rc = EINVAL;
      break;
    default :
      qdh_rc = EFAULT;                           
      break;
    }
    break;

  case (DLC_ALTER) :
    /*************************************************************************/
    /* Ioctl is for ALTER_LS                                                 */
    /* Exception blocks are sent by QLM so there is no need                  */
    /* for this procedure to send a good or bad result.                      */
    /*************************************************************************/
    if (appl_user)
    {
      qdh_rc = copyin(arg,&ioctl_ext,sizeof(struct dlc_alter_arg));
      if (qdh_rc != 0)
      {
	return(qdh_rc);
      }
      ioctl_ext_ptr = &ioctl_ext;
    }
    else
    {
      ioctl_ext_ptr = (ioctl_ext_type *)arg;
    }
    qlm_rc = qlm_alter(ioctl_ext_ptr);
    outputf("QLCIOCTL: qlm_alter rc = %d\n",qlm_rc);
    switch (qlm_rc)
    {
    case qlm_rc_ok:
      qdh_rc = 0;
      break;
    case qlm_rc_station_not_found:
      qdh_rc = EINVAL;
      break;
    case qlm_rc_invalid_set_mode_request:
      qdh_rc = EINVAL;
      break;
    case qlm_rc_invalid_inact_bits:
      qdh_rc = EINVAL;
      break;
    default :
      qdh_rc = EFAULT;                            
      break;
    }
    if (appl_user)
    {
      rc = copyout(&ioctl_ext,arg,sizeof(struct dlc_alter_arg));
      if (rc != 0)
      {
	return(EFAULT);
      }
    }
    break;

  case (DLC_ENTER_LBUSY) :
    /*************************************************************************/
    /* Ioctl is for ENTER_LOCAL_BUSY                                         */
    /* There are no result blocks connected with this function               */
    /* QLM puts station into local busy, and then controls the               */
    /* state of the station until the next exit_local_busy                   */
    /*************************************************************************/
    if (appl_user)
    {
      qdh_rc = copyin(arg,&ioctl_ext,sizeof(struct dlc_corr_arg));
      if (qdh_rc != 0)
      {
	return(qdh_rc);
      }
      ioctl_ext_ptr = &ioctl_ext;
    }
    else
    {
      ioctl_ext_ptr = (ioctl_ext_type *)arg;
    }
    qlm_rc = qlm_enter_local_busy(ioctl_ext_ptr);
    outputf("QLCIOCTL: qlm_enter_local_busy rc = %d\n",qlm_rc);
    switch (qlm_rc)
    {
    case qlm_rc_ok:
      qdh_rc = 0;
      break;
    case qlm_rc_station_not_found:
      qdh_rc = EINVAL;
      break;
    case qlm_rc_station_not_contacted:
      qdh_rc = EINVAL;
      break;
    case qlm_rc_station_already_local_busy:
      qdh_rc = EINVAL;
      break;
    default :
      qdh_rc = EFAULT;                         
      break;
    }
    break;

  case (DLC_EXIT_LBUSY) :
    /*************************************************************************/
    /* Ioctl is for EXIT_LOCAL_BUSY                                          */
    /* Exception blocks are sent by QLM so there is no need                  */
    /* for this procedure to send a good or bad result.                      */
    /*************************************************************************/
    if (appl_user)
    {
      qdh_rc = copyin(arg,&ioctl_ext,sizeof(struct dlc_corr_arg));
      if (qdh_rc != 0)
      {
	return(qdh_rc);
      }
      ioctl_ext_ptr = &ioctl_ext;
    }
    else
    {
      ioctl_ext_ptr = (ioctl_ext_type *)arg;
    }
    qlm_rc = qlm_exit_local_busy(ioctl_ext_ptr);
    outputf("QLCIOCTL: qlm_exit_local_busy rc = %d\n",qlm_rc);
    switch (qlm_rc)
    {
    case qlm_rc_ok:
      qdh_rc = 0;
      break;
    case qlm_rc_station_not_found:
      qdh_rc = EINVAL;
      break;
    case qlm_rc_station_not_local_busy:
      qdh_rc = EINVAL;
      break;
    default :
      qdh_rc = EFAULT;                       
      break;
    }
    break;

  case (DLC_GET_EXCEP) :
    /*************************************************************************/
    /* Ioctl is for GET_EXCEPTION                                            */
    /* Exception blocks are sent by QLM so there is no need                  */
    /* for this procedure to send a good or bad result.                      */
    /*************************************************************************/
    if (!appl_user)
    {
      /***********************************************************************/
      /* Kernel user, which is illegal                                       */
      /***********************************************************************/
      return(EINVAL);
    }
    qcm_rc = qcm_read_exception_queue(channel_id,&excep_ptr);
    outputf("QLCIOCTL: qcm read excep queue rc = %d\n",qcm_rc);
    switch (qcm_rc)
    {
    case qcm_rc_ok:
      qdh_rc = 0;
      break;
    case qcm_rc_channel_not_found:
      qdh_rc = EINVAL;
      break;
    case qcm_rc_invalid_command:
      qdh_rc = EINVAL;
      break;
    default :
      qdh_rc = EFAULT;                          
      break;
    }
    if (excep_ptr != NULL)
    {
      outputf("QLCIOCTL: copyout arg\n");
      rc = copyout(excep_ptr,arg,sizeof(struct dlc_getx_arg));
      if (rc != 0)
      {
	(void)xmfree((char *)excep_ptr,pinned_heap);
	outputf("QLCIOCTL: copyout of arg failed\n");
	return(EFAULT);
      }
      outputf("QLCIOCTL: copyout completed\n");
      outputf("QLCIOCTL: free excep block\n");
      (void)xmfree((char *)excep_ptr,pinned_heap);
      outputf("QLCIOCTL: freed excep block\n");
    }
    break;
  
  case (IOCINFO) : 
    /*************************************************************************/
    /* Ioctl is iocinfo. Return devinfo structure to user.                   */
    /* Set first byte to DD_PSEU and rest of structure to zero               */
    /*************************************************************************/
    if (appl_user)
    {
      outputf("QLCIOCTL: iocinfo copyin\n");
      qdh_rc = copyin(arg,&ioctl_ext,sizeof(struct devinfo));
      if (qdh_rc != 0)
      {
	return(qdh_rc);
      }
      outputf("QLCIOCTL: iocinfo copyin ok\n");
      ioctl_ext_ptr = &ioctl_ext;
    }
    else
    {
      outputf("QLCIOCTL: iocinfo in system space\n");
      ioctl_ext_ptr = (ioctl_ext_type *)arg;
    }
    outputf("QLCIOCTL: clearout iocinfo\n");
    bzero(
      (char *)&((struct devinfo *)ioctl_ext_ptr)->devtype,
      sizeof(struct devinfo)
      );
    outputf("QLCIOCTL: initialise devtype in iocinfo\n");
    ((struct devinfo *)ioctl_ext_ptr)->devtype = DD_DLC;
    ((struct devinfo *)ioctl_ext_ptr)->devsubtype = DS_DLCQLLC;
    if (appl_user)
    {
      outputf("QLCIOCTL: iocinfo copyout\n");
      rc = copyout(ioctl_ext_ptr,arg,sizeof(struct devinfo));
      outputf("QLCIOCTL: iocinfo copyin rc = %d\n",rc);
      if (rc != 0)
      {
	return(EFAULT);
      }
    }
    qdh_rc = 0;
    break;

  case (QLC_DEBUG) :
    if (channel_list.debug_control == FALSE) {
      channel_list.debug_control = TRUE;
    } else {
      channel_list.debug_control = FALSE;
    }
    qdh_rc = 0;
    break;

  case (QLC_GETTRACE) :
      outputf("QLCIOCTL: buffer  = %x\n",buffer);
      if (buffer == NULL)
         return (EFAULT);
      rc = copyout(buffer, arg, 0x1010a);
      outputf("QLCIOCTL: copyout rc= %d\n",buffer);
      if (rc)
          {
                  return(EFAULT);
          }
      return (0);
      break; 

#ifdef WSD_LOCAL_BUILD
  case (QLC_COVQUERY):
    coverage_points = qlc_how_many_to_dump();
    outputf("%d coverage points\n", coverage_points);
    rc = copyout((char *)&coverage_points,(char *)arg,sizeof(int));
    if (rc < 0)
      qdh_rc = EFAULT;
    else
      qdh_rc = 0;
    break;

  case (QLC_COVDUMP):
    rc = qlc_dump_coverage_data((unsigned char *)arg);
    if (rc == 1)
      qdh_rc = 0;
    else
      qdh_rc = EIO;
    break;

#endif

  default :
    /*************************************************************************/
    /* operation is other type                                               */
    /*************************************************************************/
    outputf("QLCIOCTL: The ioctl you entered is not supported by QLLC\n");
    outputf("QLCIOCTL: value of op you entered was %d\n",op);
    return (EINVAL);
  }
  outputf("QLCIOCTL: This is the synchronous return from op type %d\n",op);
  outputf("QLCIOCTL: Return Code = %d\n",qdh_rc);
  return (qdh_rc);
}

/*****************************************************************************/
/* Function     qlcselect                                                    */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
int  qlcselect(
  dev_t           devno,
  unsigned short  events,
  unsigned short *reventp,
  int             chan)
{
  int                   qdh_rc;
  struct channel_type  *channel_id;

  outputf("QLCSELECT: called\n");
  outputf("QLCSELECT: selected events = %x\n",events);
  /***************************************************************************/
  /* Initialise local variables, and returned paramater.                     */
  /***************************************************************************/
  channel_id = (channel_id_type)chan;
  *reventp = 0;
  /***************************************************************************/
  /* Start by checking the events that the user is requesting in "events"    */
  /***************************************************************************/
  if ( (events & POLLOUT) == POLLOUT)
  {
    outputf("QLCSELECT: write selected\n");
    /*************************************************************************/
    /* Since writes are always available the reventp value can be updated to */
    /* indicate that the user can issue a write.                             */
    /*************************************************************************/
    *reventp |= POLLOUT;
  }
  /***************************************************************************/
  /* Check whether data or exceptions are queued in channel                  */
  /* This is done by the QCM, which updates reventp to indicate which of     */
  /* requested events is available.                                          */
  /***************************************************************************/
  outputf("QLCSELECT: call qcm_select()\n");
  if ( qcm_select(channel_id,events,reventp) != qcm_rc_ok )
  {
    outputf("QLCSELECT: qcm_select rc = bad\n");
    /*************************************************************************/
    /* The QCM objected to the type of user                                  */
    /*************************************************************************/
    qdh_rc = EINVAL;
    return(qdh_rc);
  }
  /***************************************************************************/
  /* If any of the requested events were ready   (i.e. if any of the events  */
  /* selected by the user was satisfied), then do the following              */
  /***************************************************************************/
  outputf("QLCSELECT: reventp = %x\n",*reventp);
  if (*reventp != 0)
  {
    return (0);
  }
  else
  {
    outputf("QLCSELECT: none of the user's criteria are satisfied\n");
    /*************************************************************************/
    /* None of the user's criteria were satisfied                            */
    /*************************************************************************/
    if ( (events & POLLSYNC) == POLLSYNC)
    {
      outputf("QLCSELECT: request was synchronous\n");
      /***********************************************************************/
      /* The request was synchronous, so you must return immediately.        */
      /***********************************************************************/
      return(0);
    }
    else
    {
      outputf("QLCSELECT: request was not synchronous\n");
      /***********************************************************************/
      /* The request was not synchronous.                                    */
      /* First copy the selected_events to the channel so that the user can  */
      /* be selnotified of any future activity which would satisfy this      */
      /* request.                                                            */
      /***********************************************************************/
      outputf("QLCSELECT: set selected events in channel\n");
      QCM_SET_SELECTED_EVENTS(channel_id,events);
      outputf("QLCSELECT: finished setting of selected events\n");
      /***********************************************************************/
      /* Now I return a value of zero, which together with the zero          */
      /* value of reventp puts the user to sleep. Wake them later with       */
      /* selnotify                                                           */
      /***********************************************************************/
      return (0);
    }
  }
}


