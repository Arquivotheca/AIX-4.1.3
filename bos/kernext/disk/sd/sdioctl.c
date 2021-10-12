static char sccsid[] = "@(#)97  1.18.4.1  src/bos/kernext/disk/sd/sdioctl.c, sysxdisk, bos411, 9439A411b 9/26/94 11:00:00";
/*
 * COMPONENT_NAME: (SYSXDISK) Serial Dasd Subsytem Device Driver
 *
 * FUNCTIONS:  sd_ioctl(), sd_adap_ioctl(),sd_adapter_info(),
 *       sd_adap_dwnld(), sd_adap_tr_snpsht(),sd_adap_qry_trc(), 
 *       sd_adap_inqry(),sd_adap_set_parms(), sd_adap_read_id(), 
 *	 sd_adap_mbox(),sd_get_async(), sd_daemon_log(), sd_ctrl_ioctl(),
 *       sd_ctrl_info(), 
 *       sd_dasd_ioctl(),sd_dasds_info(), sd_dasd_qry_dev(),sd_get_struct(),
 *	 sd_ioctl_reset(),sd_ioctl_set_fence(), 
 *       sd_ioctl_scsicmd(),sd_ioctl_finish(), sd_ioctl_wait
 *       sd_copyin(), sd_copyout(),sd_prepare_dma(), sd_ioctl_status(),
 *       sd_ioctl_download(), sd_ioctl_verify_dasd(),
 *       sd_ioctl_free()
 *       
 *       
 *
 * ORIGINS: 27   
 * 
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <sys/sd.h>
#include <sys/malloc.h> 
#include <sys/errno.h>
#include <sys/ioctl.h> 
#include <sys/ddtrace.h> 
#include <sys/trchkid.h> 
#include <sys/signal.h>
#include <sys/ddconc.h>

/* 
 *
 * NAME: sd_ioctl 
 *                  
 * FUNCTION: Main ioctl routine
 *                                                 
 *      Determines by the minor number whether the call is directed to
 *      an adapter, controller or dasd.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.
 *
 * (NOTES:) This routine handles the following operations : 
 *         IOCINFO                 -    Get devinfo structure
 *         SD_SCSICMD		   -    SCSI Pass Thru Command
 *         SD_RESET	           -    Reset or Quiesce
 *         SD_ADAP_DOWNLOAD	   -    Adapter Microcode Download
 *         SD_ADAP_TRACE_SNAPSHOT  -    Adapter Trace Snapshot 
 *         SD_ADAP_QUERY_TRACE     -    Adapter Query Trace 
 *         SD_ADAP_INQUIRY         -    Adapter Inquiry 
 *         SD_SET_ADAP_PARMS	   -    Set Adapter Parameters 
 *         SD_QUERY_DEVICE	   -    Query Device  
 *         SD_READ_ADAP_ID	   -    Read Adapter POS ID 
 *         SD_MBOX		   -    Pass Thru Mailbox
 *         SD_GET_EVENT            -    Get first asynch event
 *         SD_DAEMON_ERROR         -    Daemon error logging
 *
 *
 * (DATA STRUCTURES:)  struct sd_iocmd       - argument for send scsi
 *                     struct sd_ioctl_parms - argument for most other ops
 *                     struct devinfo        - argument for IOCINFO
 *                                                          
 * INPUTS:                                                  
 *      devno   - device major/minor number                 
 *      op      - operation being requested
 *      arg     - Pointer to callers data 
 *      devflag - specifies either system or user space
 *      chan    - unused
 *      ext     - unused
 * 
 * INTERNAL PROCEDURES CALLED:
 * 
 *      sd_adap_ioctl                  sd_copyin        
 *      sd_ctrl_ioctl                  sd_copyout
 *      sd_dasd_ioctl
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *   
 *      minor                          lockl
 *      unlockl
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:     
 *       
 *        EACCES    -   Permission denied
 *        EFAULT    -   xmattached failed
 *        EINVAL    -   Invalid parameter
 *        EIO       -   I/O error
 *        ENOMEM    -   Not enough memory ( xmalloc failed)
 *        ENXIO     -   No such address
 *        ETIMEDOUT -   wait for response of command timed out
 *	  ECHILD    -   No more events for daemon
 *        0         -   Successful completion.
 */                                                                         
int sd_ioctl(
dev_t devno,		/* major/minor number		 	*/
int op,			/* operation requested		 	*/
int arg,		/* Pointer to caller's date		*/
ulong devflag,		/* system/user space		 	*/
int chan,		/* not used				*/
int ext)		/* not  used 				*/
{
	int                    min;            /* device minor number 		*/
	int                    ret_code = 0;   /* return code 			*/
	struct sd_ioctl_parms  ioctl_parms;    /* ioctl parameters 		*/
	struct sd_iocmd        iocmd;          /* for pass thru scsi command 	*/
	struct dd_conc_register conc_register; /* for concurrent register ioctl */
	struct devinfo         sd_info;        /* for iocinfo operation      	*/
	struct sd_event        event;          /* event for daemon           	*/
	struct sd_daemon_errlog d_errlog;      /* for daemon error logging   	*/
	uint                   time_out = 0;   /* user specified timeout     	*/
	int                    daemon_pid = 0; /* pid of config daemon       	*/
	struct sd_adap_info 	*ap;           /* adapter pointer 		*/
	struct sd_ctrl_info 	*cp;	       /* controller pointer 		*/
	struct sd_dasd_info 	*dp;	       /* DASD pointer 			*/

	DDHKWD5(HKWD_DD_SERDASDD, DD_ENTRY_IOCTL, 0, devno, op, arg, 0, 0);

	min = minor(devno);                    /* get minor number */ 

	if ((op != SD_SCSICMD) && (op != IOCINFO) && (op != SD_READ_ADAP_ID)
	    && (op != SD_GET_EVENT) && (op != SD_DAEMON_ERROR) 
	    && (op != DD_CONC_REGISTER) && (op != DD_CONC_UNREGISTER)) {
		ret_code = sd_copyin((char *)&ioctl_parms,(char *) arg,
				     sizeof(struct sd_ioctl_parms),
				     devflag);
		time_out = ioctl_parms.time_out;
		ioctl_parms.status_validity = 0;
		
	}
	else if (op == SD_SCSICMD)  {
		ret_code = sd_copyin((char *)&iocmd,(char *) arg,
				     sizeof(struct sd_iocmd),
				     devflag);
		time_out = iocmd.timeout_value;
		iocmd.status_validity = 0;
	}
	else if (op == SD_DAEMON_ERROR) {
		ret_code = sd_copyin((char *)&d_errlog,(char *) arg,
				     sizeof(struct sd_daemon_errlog),
				     devflag);

	}
	else if (op == DD_CONC_REGISTER) {
		ret_code = sd_copyin((char *)&conc_register,(char *) arg,
				     sizeof(struct dd_conc_register),
				     devflag);

	}
	if (ret_code) { 
		DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_IOCTL, ret_code, devno);
		return (ret_code);
	} 
	lockl(&sd_global_lock,LOCK_SHORT);

	/*
	 *  determine device type
	 */


	if (min & SD_DASD_MASK)  {
		/* 
		 * get pointer to disk_info 
		 */
		dp = (struct sd_dasd_info *)sd_hash(devno); 
		unlockl(&sd_global_lock);
		if (dp == NULL) {
			DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_IOCTL, ret_code, 
				devno);
			return(EINVAL);
		}
		lockl(&dp->dev_lock,LOCK_SHORT);
		ret_code = sd_dasd_ioctl(dp,op,devno,&iocmd,&ioctl_parms,
					 &conc_register,&sd_info,devflag,
					 chan,ext,time_out);
		unlockl(&dp->dev_lock);

	} else if (min & SD_CTRL_MASK) {
		/* 
		 * get pointer to ctrl_info 
		 */
		cp = (struct sd_ctrl_info *)sd_hash(devno); 
		unlockl(&sd_global_lock);
		if (cp == NULL) {
			DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_IOCTL, ret_code, 
				devno);
			return(EINVAL);
		}
		lockl(&cp->dev_lock,LOCK_SHORT);
		ret_code = sd_ctrl_ioctl(cp,op,devno,&iocmd,&ioctl_parms,
					 &sd_info,devflag,chan,ext,time_out);
		unlockl(&cp->dev_lock);

	} else  {
		/* 
		 * get pointer to adap_info 
		 */
		ap = (struct sd_adap_info *)sd_hash(devno); 
		unlockl(&sd_global_lock);
		if (ap == NULL) {
			DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_IOCTL, ret_code, 
				devno);
			return(EINVAL);
		}
		lockl(&ap->dev_lock,LOCK_SHORT);
		ret_code = sd_adap_ioctl(ap,op,devno,&iocmd,&ioctl_parms,
					 &sd_info,&event,&d_errlog,
					 devflag,chan,ext,
					 time_out,daemon_pid);
		unlockl(&ap->dev_lock);
	}

	if (ret_code) {
		DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_IOCTL, ret_code, devno);
		return (ret_code);
	}

	if ((op != SD_SCSICMD)  && (op != IOCINFO) && (op != SD_GET_EVENT)
	    && (op != SD_ADAP_TRACE_SNAPSHOT) && (op != SD_DAEMON_ERROR) 
	    && (op != DD_CONC_REGISTER) && (op != DD_CONC_UNREGISTER)) {
 		ret_code = sd_copyout((char *) arg,(char *) &ioctl_parms,
				      sizeof(struct sd_ioctl_parms),
				      devflag);
	}
	else if (op == IOCINFO) {
		ret_code = sd_copyout((char *)arg,(char *)&sd_info,
				      sizeof(struct devinfo),
				      devflag);
	}
	else if ( op == SD_SCSICMD) {
		ret_code = sd_copyout((char *)arg,(char *)&iocmd,
				      sizeof(struct sd_iocmd),
				      devflag);
	}
	else if (op == SD_GET_EVENT)  {
		ret_code = sd_copyout((char *) arg,(char *)&event,
				     sizeof(struct sd_event),
				     devflag);
	}
	else if (op == DD_CONC_REGISTER) {
		ret_code = sd_copyout((char *) arg, (char*)&conc_register,
				     sizeof(struct dd_conc_register),
				     devflag);

	}
	DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_IOCTL, ret_code, devno);
	return (ret_code);
}



/* 
 * adapter functions 
 */

/* 
 *
 * NAME: sd_adap_ioctl 
 *                  
 * FUNCTION: Main adapter ioctl function
 *                                                 
 *     Parse out desired ioctl operation
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 * (NOTES:) This routine handles the following operations : 
 *         IOCINFO                 -    Get devinfo structure
 *         SD_SCSICMD		   -    SCSI Pass Thru Command
 *         SD_RESET	           -    Reset or Quiesce
 *         SD_ADAP_DOWNLOAD	   -    Adapter Microcode Download
 *         SD_ADAP_TRACE_SNAPSHOT  -    Adapter Trace Snapshot 
 *         SD_ADAP_QUERY_TRACE     -    Adapter Query Trace 
 *         SD_ADAP_INQUIRY         -    Adapter Inquiry 
 *         SD_SET_ADAP_PARMS	   -    Set Adapter Parameters 
 *         SD_QUERY_DEVICE	   -    Query Device  
 *         SD_READ_ADAP_ID	   -    Read Adapter POS ID 
 *         SD_GET_EVENT            -    Get first asynch event
 *         SD_DAEMON_ERROR         -    Daemon errorlogging
 *         SD_MBOX		   -    Pass Thru Mailbox
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd       - command jacket
 *                     struct sd_adap_info - adapter structure
 *
 * CALLED BY:
 *      sd_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *        sd_adapter_info             sd_ioctl_reset
 *        sd_adap_dwnld               sd_adap_tr_snpsht
 *        sd_adap_qry_trc             sd_adap_inqry
 *        sd_adap_set_parms           sd_ioctl_scsicmd
 *        sd_adap_read_id             sd_adap_wait_asynch
 *        sd_adap_free                sd_adap_mbox
 *        sd_ioctl_finish             sd_hash
 *        sd_get_struct               sd_daemon_errlog
 *
 * EXTERNAL PROCEDURES CALLED:
 *        None
 *       
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:      
 *       
 *        EACCES    -   Permission denied
 *        EFAULT    -   xmattached failed
 *        EINVAL    -   Invalid parameter
 *        EIO       -   I/O error
 *        ENOMEM    -   Not enough memory ( xmalloc failed)
 *        ENXIO     -   No such address
 *        ETIMEDOUT -   wait for response of command timed out
 *	  ECHILD    -   No more events for daemon
 *        0         -   Successful completion.
 */                                                                         

int sd_adap_ioctl(
		  struct sd_adap_info *ap,
		  int op,
		  dev_t devno,		    /* major/minor number 	*/
		  struct sd_iocmd *iocmd,   /* pass thru scsi command   */
		  struct sd_ioctl_parms *ioctl_parms,
		  struct devinfo *sd_info,
		  struct sd_event *event,
		  struct sd_daemon_errlog *d_errlog, /* for daemon errlog*/
		  ulong devflag,	   /* system/user space	 	*/
		  int chan,		   /* not used	 		*/
		  int ext,		   /* not used 			*/
		  uint time_out,	   /* time out value in seconds */
		  int  daemon_pid)         /* pid of config daemon      */
{

	int                   ret_code = 0;/* return code */       
	struct  sd_cmd        *cmd;       /* command jacket */
	int                   dwnld_flg = 0;/* whether dwnld is to be done */
	int 		      reset_flg = 0;/* 1 if full reset controller  */
					    /* needed*/
	int 		     async_flg = FALSE;/* if async event is needed */
	struct sd_adap_info  *device = NULL;   /* devices async is for  */
	uchar 		     address = 0;      /* address async is for  */
	uchar 		     async_event = 0;   /* event async is for    */


	cmd = NULL;


	/*
	 * get cmd structure: will eventually get one
	 */
	cmd = sd_get_struct(ap);

	switch (op) {                   /* determine operation requested */

	      case IOCINFO: 
		ret_code = sd_adapter_info(sd_info,devflag,chan,ext,ap,op);
		break;
	      case SD_RESET:
		ret_code = sd_ioctl_reset(ioctl_parms,devflag,chan,ext,ap,
					  cmd,op,SD_ADAP_CMD,0,&async_flg,
					  &device,&address, &async_event,0);
		break;
	      case SD_ADAP_DOWNLOAD:
		ret_code = sd_adap_dwnld(cmd,ioctl_parms,devflag,
						 chan,ext,ap,op);
		if (!ret_code)
			dwnld_flg = 1;
		break;
	      case SD_ADAP_TRACE_SNAPSHOT:
		ret_code = sd_adap_tr_snpsht(cmd,ioctl_parms,devflag,chan,ext,
					     ap,op);
		break;
	      case SD_ADAP_QUERY_TRACE:
		ret_code = sd_adap_qry_trc(cmd,ioctl_parms,devflag,chan,ext,
					   ap,op);
		break;
	      case SD_ADAP_INQUIRY:
		ret_code = sd_adap_inqry(cmd,ioctl_parms,devflag,chan,ext,ap,
					 op);
		break;
	      case SD_SET_ADAP_PARMS:
		ret_code = sd_adap_set_parms(cmd,ioctl_parms,devflag,
					     chan,ext,ap,op);
		break;
	      case SD_SCSICMD:
		ret_code = sd_ioctl_scsicmd(iocmd,devflag,chan,ext,ap,cmd,op,
					    SD_ADAP_CMD,&dwnld_flg,&reset_flg);
		break;
	      case SD_READ_ADAP_ID:
		ret_code = sd_adap_read_id(ioctl_parms,devflag,chan,ext,ap,op);
		break;
	      case SD_MBOX:
		ret_code = sd_adap_mbox(cmd,ioctl_parms,devflag,chan,ext,ap,
					op);
		break;
	      case SD_GET_EVENT:
		ret_code = sd_get_asynch(event,devflag,chan,ext,ap,op);
	        break;
	      case SD_DAEMON_ERROR:
		ret_code = sd_daemon_log(d_errlog,devflag,chan,ext,ap,cmd,
					 op);
		break;
	      default:
		ret_code = EINVAL;
	}


	if (ret_code) {
	        sd_ioctl_free(op,devflag,iocmd,ioctl_parms,cmd);
		sd_free_cmd_disable(cmd);
		return (ret_code);
	}

	if ((op != IOCINFO) && 
	   (! ((op == SD_RESET) && (ioctl_parms->reset_type == SD_RESET_OP)))
	    && (op != SD_READ_ADAP_ID) && (op != SD_GET_EVENT) 
	    && (op != SD_DAEMON_ERROR)) {
		ret_code = sd_ioctl_finish(op,iocmd,ioctl_parms,cmd,ap,
					   (struct sd_dasd_info *)ap,1,
					   time_out,dwnld_flg,reset_flg,
					   async_flg,device,address,
					   async_event,devflag);
	}
	else {
	    sd_ioctl_free(op,devflag,iocmd,ioctl_parms,cmd);
	    sd_free_cmd_disable(cmd);
	}
	return (ret_code);
}


/* 
 *
 * NAME: sd_adapter_info 
 *                  
 * FUNCTION:  Return devinfo structure to caller for an adapter 
 *                                                 
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *
 * (DATA STRUCTURES:)  struct devinfo    - device's info structure
 *
 *
 * CALLED BY:
 *      sd_adap_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 *      None
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *     None
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:     
 *        0         -   Successful completion.
 */                                                                         

int sd_adapter_info(
struct devinfo *arg,
ulong devflag,
int chan,
int ext,
struct sd_adap_info *ap, 
int op)
    

{

	arg->devtype = DD_BUS;
	arg->devsubtype =DS_SDA;
	arg->flags = DF_FIXED;

	/*
	 * using scsi structure of devinfo
	 */

	return (0);
}


/* 
 *
 * NAME: sd_adap_dwnld
 *                  
 * FUNCTION:    Download microcode to the adapter
 *                                      
 *       Micro code is downloaded to the adapter by giving the adapter
 *       a mail box which contains the dma address of the micro code.
 *
 * EXECUTION ENVIRONMENT: 
 *                                                      
 *      This routine is  called by a process at the process level and it
 *      can page fault.
 *

 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information
 *
 *
 * CALLED BY:
 *      sd_adap_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *      sd_prepare_dma                  sd_free_cmd_disable
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *       None
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:     
 *       
 *        EACCES    -   Permission denied
 *        EFAULT    -   xmattached failed
 *        EINVAL    -   Invalid parameter
 *        ENOMEM    -   Not enough memory ( xmalloc failed)
 *        0         -   Successful completion.
 *        -1        -   No mailbox command is necessary
 */                                                                         

int sd_adap_dwnld(
struct sd_cmd *cmd,
struct sd_ioctl_parms *arg,
ulong devflag,
int chan,
int ext,
struct sd_adap_info *ap, 
int op)
{
	int             ret_code = 0;  /* return code */


	if ((!(ap->diag_mode)) && 
	    (getpid() != ap->daemon_pid))
		/*
		 * must be in diagnostic mode or the process must
		 * be a child of the daemon for 
		 * this command
		 */
		return(EACCES);
	/*
	 *  fill in cmd and mailbox
	 */

	cmd->type = SD_ADAP_CMD;
	cmd->mbox_copy.op_code = SD_DOWNLOAD_MCODE;
	cmd->mbox_copy.mb7.nul_address = 0;
	cmd->b_addr = arg->buffer;
	cmd->b_length = arg->data_length;
	cmd->dma_flags = 0;

	ret_code = sd_prepare_dma(cmd,arg->data_length,0,arg->buffer,devflag);

	return (ret_code);
}


/* 
 *
 * NAME: sd_adap_tr_snpsht
 *                  
 * FUNCTION:
 *      Perform adapter trace snapshot by using the mailbox mechanism
 *        
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information
 *
 *
 * CALLED BY:
 *      sd_adap_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *      sd_prepare_dma
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:     
 *       
 *        EFAULT    -   xmattached failed
 *        EINVAL    -   Invalid parameter
 *        ENOMEM    -   Not enough memory ( xmalloc failed)
 *        0         -   Successful completion.
 */                                                                         

int sd_adap_tr_snpsht(
struct sd_cmd *cmd,
struct sd_ioctl_parms *arg,
ulong devflag,
int chan,
int ext,
struct sd_adap_info *ap, 
int op)
{
	int            ret_code = 0;  /* return code */



	/*
	 * fill in cmd and mailbox
	 */
	cmd->type = SD_ADAP_CMD;
	cmd->mbox_copy.op_code = SD_TRACE_SNAP_OP;
	cmd->mbox_copy.mb6.length = arg->resvd1;
	cmd->mbox_copy.mb7.nul_address = 0;



	cmd->mbox_copy.mb8.ad_snpsht.ev1_4 = arg->resvd2;
	cmd->mbox_copy.mb8.ad_snpsht.ev5_7 = arg->resvd3;

	return (ret_code);
}


/* 
 *
 * NAME: sd_adap_qry_trc
 *                  
 * FUNCTION:  Perform adapter query trace using the mailbox mechanism     
 *                                                 
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information
 *
 *
 * CALLED BY:
 *      sd_adap_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *      sd_prepare_dma               sd_free_cmd_disable
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:      
 *       
 *        EFAULT    -   xmattached failed
 *        EINVAL    -   Invalid parameter
 *        ENOMEM    -   Not enough memory ( xmalloc failed)
 *        0         -   Successful completion.
 */                                                                         

int sd_adap_qry_trc(
struct sd_cmd *cmd,
struct sd_ioctl_parms *arg,
ulong devflag,
int chan,
int ext,
struct sd_adap_info *ap, 
int op)
{
	int                 ret_code = 0;  /* return code */




	/*
	 * fill in cmd and mailbox
	 */

	cmd->type = SD_ADAP_CMD;
	cmd->mbox_copy.op_code = SD_QUERY_TRACE_OP;
	cmd->mbox_copy.mb7.nul_address = 0;
	cmd->b_addr = arg->buffer;
	cmd->b_length = arg->data_length;
	cmd->dma_flags = DMA_READ;

	ret_code = sd_prepare_dma(cmd,arg->data_length,(SD_ADAP_DUMP_LENGTH -1),
				  arg->buffer,devflag);

	return (ret_code);
}


/* 
 *
 * NAME: sd_adap_inqry
 *                  
 * FUNCTION:   Adapter Inquiry
 *
 *    This routine builds a mailbox causing the adapter inquiry
 *    data to be transferred to the DMA address supplied
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information
 *
 * CALLED BY:
 *      sd_adap_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *      sd_prepare_dma                sd_free_cmd_disable
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:     
 *       
 *        EFAULT    -   xmattached failed
 *        EINVAL    -   Invalid parameter
 *        ENOMEM    -   Not enough memory ( xmalloc failed)
 *        0         -   Successful completion.
 */                                                                         

int sd_adap_inqry(
struct sd_cmd *cmd,
struct sd_ioctl_parms *arg,
ulong devflag,
int chan,
int ext,
struct sd_adap_info *ap, 
int op)
{
	int                 ret_code = 0;  /* return code */




	/*
	 * fill in cmd and mailbox
	 */
	cmd->type = SD_ADAP_CMD;
	cmd->mbox_copy.op_code = SD_INQUIRY_OP;
	cmd->mbox_copy.mb7.nul_address = 0;

	cmd->b_addr = arg->buffer;
	cmd->b_length = arg->data_length;
	cmd->dma_flags = DMA_READ;

	ret_code = sd_prepare_dma(cmd,arg->data_length,0,arg->buffer,devflag);


	return (ret_code);
}


/* 
 *
 * NAME: sd_adap_set_parms
 *                  
 * FUNCTION: Set Adapter parameters.
 *
 *       Set adapter parameters using the mailbox mechanism
 *       the mail box contains the parameters to set
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information 
 *
 *
 * CALLED BY:
 *      sd_adap_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 *        None
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *       None
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:      
 *       
 *        EACCES    -   Permission denied
 *        0         -   Successful completion.
 */                                                                         

int sd_adap_set_parms(
struct sd_cmd *cmd,
struct sd_ioctl_parms *arg,
ulong devflag,
int chan,
int ext,
struct sd_adap_info *ap, 
int op)
{
	int                 ret_code = 0;  /* return code */




	if ((!(ap->diag_mode)) && 
	    (getpid() != ap->daemon_pid))
		/*
		 * must be in diagnostic mode or the process must be
		 * a child of the daemon for 
		 * this command
		 */
		return(EACCES);
	/*
	 * fill cmd and mailbox
	 */
	cmd->type = SD_ADAP_CMD;
	cmd->mbox_copy.op_code = SD_SETPARMS_OP;
	cmd->mbox_copy.mb6.length = arg->data_length;
	cmd->mbox_copy.mb7.nul_address = 0;

	/*
	 * set parameters
	 */

	cmd->mbox_copy.mb8.ad_parm.parm[0] = arg->resvd1;
	cmd->mbox_copy.mb8.ad_parm.parm[1] = arg->resvd2;
	cmd->mbox_copy.mb8.ad_parm.parm[2] = arg->resvd3;
	cmd->mbox_copy.mb8.ad_parm.parm[3] = arg->resvd4;
	cmd->mbox_copy.mb8.ad_parm.parm[4] = arg->resvd5;
	cmd->mbox_copy.mb28.ad_parm.parm5  = arg->resvd6;


	return (ret_code);
}



/* 
 *
 * NAME: sd_adap_read_id 
 *                  
 * FUNCTION: Read adapter id
 *                                                 
 *     Read the POS1 and POS2 registers   
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct 
 *
 *
 * CALLED BY:
 *      sd_adap_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *       IOCC_ATT                 IOCC_DET
 *       BUSIO_GETC
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:      
 *       
 *        EACCES    -   Permission denied
 *        EIO       -   I/O error
 *        0         -   Successful completion.
 */                                                                         

int sd_adap_read_id(
struct sd_ioctl_parms *arg,
ulong devflag,
int chan,
int ext,
struct sd_adap_info *ap, 
int op)
{
	uchar               pos0;          /* Pos register  0 */
	uchar               pos1;          /* Pos register 1 */




	if ((!(ap->diag_mode)) && 
	    (getpid() != ap->daemon_pid))
		/*
		 * must be in diagnostic mode or the process must be
		 * a child of the daemon for 
		 * this command
		 */		
		return(EACCES);
	
	/*
	 *get address of adapter card
	 */

	/*
	 * use offset with above base to
	 * get POS reg 0 and 1.
	 */
	if ((pos0 = sd_read_POS(ap,SD_POS0)) == 0xFF)
		return(EIO);
	if ((pos1 = sd_read_POS(ap,SD_POS1)) == 0xFF)
		return(EIO);

	arg->resvd1 = (pos0 << SD_BYTE) | pos1;

	/*
	 * verify that there is adapter connected here
	 */ 

	if (arg->resvd1!= SD_ADAP_ID) { 
		return(EIO);
	}

	return (0);
}


/* 
 *
 * NAME: sd_adap_mbox
 *                  
 * FUNCTION:  Send a user supplied mail box to the adapter   
 *                                                 
 *      This is the pass thru mailbox routine.  It takes mailbox
 *      given by the caller and passes it on to the device. 
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct
 *
 *
 * CALLED BY:
 *      sd_adap_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *      sd_copyin                     sd_copyout
 *      sd_prepare_dma                sd_free_cmd_disable
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:      
 *       
 *        EACCES    -   Permission denied
 *        EFAULT    -   xmattached failed
 *        EINVAL    -   Invalid parameter
 *        EIO       -   I/O error
 *        ENOMEM    -   Not enough memory ( xmalloc failed)
 *        0         -   Successful completion.
 */                                                                         

int sd_adap_mbox(
struct sd_cmd *cmd, 
struct sd_ioctl_parms *arg,
ulong devflag,
int chan,
int ext,
struct sd_adap_info *ap, 
int op)
{
	int                 ret_code = 0;  /* return code */



	if ((!(ap->diag_mode)) &&
	    (getpid() != ap->daemon_pid)) {
		/*
		 * must be in diagnostic mode or the process must be
		 * a child of the daemon for 
		 * this command
		 */
		return(EACCES);
	}
	/*
	 * fill in cmd
	 */ 

	cmd->type = SD_ADAP_CMD;


	/*
	 * copy arg (sd_ioctl_parms) into 
	 * our mailbox 
	 */
	ret_code = sd_copyin((char *)&cmd->mbox_copy,
			     (char *)arg->resvd1,
			     sizeof(struct sd_mbox),
			     devflag); 
 

	if (! ret_code) {
		cmd->b_addr = arg->buffer;
		cmd->b_length = arg->data_length;

		if (arg->resvd2 & B_READ)
		  	cmd->dma_flags = DMA_READ;
		else 
			cmd->dma_flags = 0;       /* DMA write */

		if (arg->data_length) {
			ret_code = sd_prepare_dma(cmd,arg->data_length,0,
						  arg->buffer,devflag);
		}

	}
	
	return (ret_code);
}



/* 
 *
 * NAME: sd_get_asynch
 *                  
 * FUNCTION: Waits for an asynch then sends this asynch back to daemon
 *                                                 
 *     This routine will also free the asynch event from the queue
 *     before it returns. When the adapter is being unconfigured
 *     via sd_adap_config it will issue a wakeup with a null event
 *     queue.  This routine see this and pass the errno ECHILD back
 *     to the daemon.  The daemon will then kill itself.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_event      - event for config daemon
 *                     struct sd_adap_info  - adapter's information struct
 * 
 *
 * CALLED BY:
 *      sd_adap_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *      SD_FREETAG                     sd_sleep
 *      sd_walk_event
 *
 * EXTERNAL PROCEDURES CALLED:
 *      getpid                         bcopy     
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:      
 *       
 *        EACCES    -   Permission denied
 *        ENXIO     -   No such address
 *	  ECHILD    -   Daemon should commit suicide
 *        0         -   Successful completion.
 */                                                                         

int sd_get_asynch(struct sd_event *arg,ulong devflag,int chan,
		 int ext,struct sd_adap_info *ap, int op)
{
	struct sd_event   *curr_ptr;
	int                 ret_code = 0;  /* return code */
	int                 index;
	int                 i;




	
	if (getpid() == ap->daemon_pid) {   /*  check if daemon is caller */
		
		if (ap->event_head == NULL) {
			/*
			 * if no more events to process,
			 * then go to sleep
			 */
			return(ECHILD);
		} 


		/*
		 * Call handle event to process an async event
		 */
		bcopy((char *)ap->event_head,(char *) arg,
		      sizeof(struct sd_event));
		
		
		
		/*
		 * free this event from event tags
		 */
		for (i=0;i<SD_MAX_EVENTS;i++) {
			if((char *)&(ap->sd_event[i]) == 
			   (char *)ap->event_head)
				break;
		}
		if (i < SD_MAX_EVENTS) {
			index = i/SD_BITS_PER_WORD;
			SD_FREETAG(ap->free_event_list[index],i);
		}
		/*
		 * Walk the Event Head pointer and free
		 * this event from list
		 */
		sd_walk_event(ap);



	       



	}  /* if daemon */
	else
		ret_code = EACCES;           /* caller is not daemon */
	return (ret_code);
}

/* 
 *
 * NAME: sd_daemon_log
 *                  
 * FUNCTION: Puts an entry in the errlog for the daemon
 *
 *       This function should only be called from the daemon and it
 *       will errlog instances on the daemon's behave.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_daemon_errlog_cmd - from config daemon
 *                     struct sd_adap_info  - adapter's information struct
 * 
 *
 * CALLED BY:
 *      sd_adap_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *      sd_log_adap_err		     sd_log_ctrl_err
 *      sd_log_error		     sd_log_dasd_err
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 *      None
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:      
 *       
 *        EINVAL    -   Invalid log function or command type
 *        EACCES    -   Permission denied
 *        0         -   Successful completion.
 */  
int sd_daemon_log(struct sd_daemon_errlog *d_errlog,
		     ulong devflag,
		     int chan,
		     int ext,
		     struct sd_adap_info *ap,
		     struct sd_cmd *cmd,
		     int op)
{
	int                 ret_code = 0;      /* return code          */
	int                 sid;               /* for controller       */
	int                 lun;


	if (getpid() == ap->daemon_pid) {
		/*  
		 * if daemon is caller then fill in cmd jacket just for
		 * the error log routines that need it.
		 */

		if ((d_errlog->log_func == SD_DAEMON_ADAP_LOG) 
		    && (d_errlog->cmd_type == SD_ADAP_CMD)){
			sd_log_adap_err(ap,d_errlog->uec,0);
		}
		else if ((d_errlog->log_func == SD_DAEMON_CTRL_LOG )
			 && (d_errlog->cmd_type == SD_CTRL_CMD)){
			sid = SD_TARGET(d_errlog->tarlun);
			sd_log_ctrl_err(ap->ctrllist[sid],d_errlog->uec);
		}
		else if ((d_errlog->log_func ==  SD_DAEMON_DASD_LOG) 
			 && (d_errlog->cmd_type == SD_DASD_CMD)){
			sid = SD_TARGET(d_errlog->tarlun);
			cmd->cp = ap->ctrllist[sid];
			lun = SD_LUN(d_errlog->tarlun);
			cmd->dp = cmd->cp->dasdlist[lun];
			sd_log_dasd_err(cmd->dp,d_errlog->uec);
		}
		else if(d_errlog->log_func == SD_DAEMON_CMD_LOG) {
			/*
			 * Fill in command jacket.  When the command jacket
			 * is freed in sd_free_cmd, it will be logged
			 * sd_log_error
			 */
			cmd->status           |= SD_LOG_ERROR;
			cmd->type   	       = d_errlog->cmd_type;
			cmd->elog_sys_dma_rc   = d_errlog->elog_sys_dma_rc;
			cmd->status_validity   = d_errlog->status_validity;
			cmd->scsi_status       = d_errlog->scsi_status;
			cmd->adapter_status    = d_errlog->adapter_status;
			cmd->controller_status = d_errlog->controller_status;
			cmd->driver_status     = d_errlog->driver_status;
			cmd->uec               = d_errlog->uec;
			cmd->dev_address       = d_errlog->tarlun;
			cmd->elog_validity     = d_errlog->elog_validity;
			cmd->mbox_copy.op_code = 0;
			if (d_errlog->cmd_type == SD_CTRL_CMD) {
				sid = SD_TARGET(d_errlog->tarlun);
				cmd->cp = ap->ctrllist[sid];
			}
			else if (d_errlog->cmd_type == SD_DASD_CMD) {
				sid = SD_TARGET(d_errlog->tarlun);
				cmd->cp = ap->ctrllist[sid];
				cmd->dp = cmd->cp->dasdlist[SD_LUN(d_errlog->tarlun)];
			}
			else if (d_errlog->cmd_type != SD_ADAP_CMD) {
				return (EINVAL);
			}


		}
		else {
			ret_code = EINVAL;
		}
	        

	 }  /* if daemon */ 
	else
		ret_code = EACCES;         /* caller is not daemon */
	return (ret_code);

}

/* Controller Functions */


/* 
 *
 * NAME: sd_ctrl_ioctl 
 *                  
 * FUNCTION: Controller ioctl routine
 *                                                 
 *  Parses the op parameter to determine what type of operation
 *  for the controller is desired.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 * (NOTES:) This routine handles the following operations : 
 *         IOCINFO                 -    Get devinfo structure
 *         SD_SCSICMD		   -    SCSI Pass Thru Command
 *         SD_RESET	           -    Reset or Quiesce
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_ctrl_info  - controller's information struct
 *
 *
 * CALLED BY:
 *      sd_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *      sd_ctlr_info                 sd_ioctl_reset
 *      sd_ioctl_scsicmd             sd_ioctl_wait
 *      sd_hash                      sd_get_struct
 *      sd_finish
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:      
 *       
 *        EACCES    -   Permission denied
 *        EFAULT    -   xmattached failed
 *        EINVAL    -   Invalid parameter
 *        EIO       -   I/O error
 *        ENOMEM    -   Not enough memory ( xmalloc failed)
 *        ETIMEDOUT -   wait for response of command timed out
 *        0         -   Successful completion.
 */                                                                         

int sd_ctrl_ioctl(
struct sd_ctrl_info *cp,
int op,
dev_t devno,
struct sd_iocmd *iocmd,
struct sd_ioctl_parms *ioctl_parms,
struct devinfo *sd_info,
ulong devflag,
int chan,
int ext,
uint time_out)
{
	int                 ret_code = 0;  /* return code */
	struct sd_adap_info          *ap;  /* adapter */
	struct sd_cmd               *cmd;  /* command jacket */
	uchar                       *cdb;  /* command descriptor block */
	uchar                       byte0; /* byte 0 of buffer */
	int                         dwnld_flg = 0; /* download to be down */
	int                         reset_flg = 0; /* 1 if reset before write*/
						   /* buffer */
	int                  async_flg = FALSE;/*if async event will be sent */
	struct sd_adap_info  *device = NULL;   /* devices async is for  */
	uchar 		     address = 0;      /* address async is for  */
	uchar 		     event = 0;        /* event async is for    */

	cmd = NULL;
		
	/*
	 * get cmd structure 
	 */	
	cmd = sd_get_struct(cp->ap);
	cmd->cp = cp;

	switch (op) {
	      case IOCINFO: 
		ret_code = sd_ctlr_info(sd_info,devflag,chan,ext,cp,op);
		break;
	      case SD_RESET:
		if ((ioctl_parms->reset_type == (uint)SD_DASD_RESET ) && 
		    (!privcheck(RAS_CONFIG))) {
			/*
			 * If this process has root authority and 
			 * it specified a lun id in resvd1 of the ioctl parms
			 * structure then we will allow it to send
			 * a reset mailbox to the corresponding DASD.
			 * This was added for twin tailing purposes, so
			 * that one CPU can overtake another.
			 */
			cmd->type = SD_CTRL_CMD;
			cmd->mbox_copy.op_code = SD_RSTQSC_OP;
			cmd->mbox_copy.mb6.reset_type = SD_RESET_DASD_MB;
			cmd->mbox_copy.mb7.dev_address = 
				SD_LUNTAR(cp->dds.target_id,
					  ioctl_parms->resvd1,
					  SD_LUNDEV);
		}
		else {
			/*
			 * Try to issue a reset to this controller
			 */
			ret_code = sd_ioctl_reset(ioctl_parms,devflag,
						  chan,ext,
					  (struct sd_adap_info *)cp,
					  cmd,op,SD_CTRL_CMD,0,&async_flg,
					  &device,&address, &event,0);
		}
		break;
	      case SD_SCSICMD:
		ret_code = sd_ioctl_scsicmd(iocmd,devflag,chan,ext,
					   (struct sd_adap_info *)cp,cmd,op,
					   SD_CTRL_CMD,&dwnld_flg,&reset_flg);
		break;
	      default:
		ret_code = EINVAL;
	}
	/*
	 * Scsi write buffer of length zero is allowed
	 */
	if (ret_code) {
	        sd_ioctl_free(op,devflag,iocmd,ioctl_parms,cmd);
		sd_free_cmd_disable(cmd);
		return (ret_code);
	}
	if (op != IOCINFO) { 
		ap = cp->ap;
		ret_code = sd_ioctl_finish(op,iocmd,ioctl_parms,cmd,ap,
					   (struct sd_dasd_info *)cp,3,
					   time_out,dwnld_flg,reset_flg,
					   async_flg,device,address,event,
					   devflag);
	}
	else {
		sd_ioctl_free(op,devflag,iocmd,ioctl_parms,cmd);
		sd_free_cmd_disable(cmd);
	}
	return (ret_code);
}


/* 
 *
 * NAME: sd_ctlr_info 
 *                  
 * FUNCTION: Get Iocinfo for controller
 *                                                 
 *      This routine return to the caller the devinfo information on
 *      the corresponding controller.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_ctrl_info  - controller's information struct
 * 
 *
 * CALLED BY:
 *      sd_ctrl_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 *        None
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *        None
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS: 
 *        0         -   Successful completion.     
 */                                                                         

int sd_ctlr_info(
struct devinfo *arg,
ulong devflag,
int chan,
int ext,
struct sd_ctrl_info *cp, 
int op)
{
	int                 ret_code = 0;  /* return code */




	arg->devtype = DD_BUS;
	arg->devsubtype =DS_SDC;
	arg->flags = DF_FIXED;

	return (ret_code);

}



/* Dasd Functions */

/* 
 *
 * NAME: sd_dasd_ioctl 
 *                  
 * FUNCTION: Ioctl routine for dasd
 *                                                 
 *   Parse op to determine the desired dasd command 
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *         IOCINFO                 -    Get devinfo structure
 *         SD_SCSICMD		   -    SCSI Pass Thru Command
 *         SD_RESET	           -    Reset or Quiesce
 *         SD_QUERY_DEVICE	   -    Query Device  
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_dasd_info  - dasd's information struct
 *
 *
 * CALLED BY:
 *      sd_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *      sd_dasds_info                 sd_ioctl_reset
 *      sd_ioctl_scsicmd              sd_dasd_qry_dev
 *      sd_hash                       sd_get_struct
 *      sd_finish
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *      None
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:     
 *       
 *        EACCES    -   Permission denied
 *        EFAULT    -   xmattached failed
 *        EINVAL    -   Invalid parameter
 *        EIO       -   I/O error
 *        ENOMEM    -   Not enough memory ( xmalloc failed)
 *        ETIMEDOUT -   wait for response of command timed out
 *        0         -   Successful completion.
 */                                                                         

int sd_dasd_ioctl(
struct sd_dasd_info *dp,
int op,
dev_t devno,
struct sd_iocmd *iocmd,
struct sd_ioctl_parms *ioctl_parms,
struct dd_conc_register *conc_register,
struct devinfo *sd_info,
ulong devflag,
int chan,
int ext,
uint time_out)
{
	int                 ret_code = 0;  /* return code */
	struct sd_cmd       *cmd;          /* pointer command jacket */
	struct sd_adap_info *ap;           /* adapter */
	int                 dwnld_flg = 0; /* download is to be done  */
	int                 reset_flg = 0; /* 1 if reset before write*/
					   /* buffer */
	int                  async_flg = FALSE;/*if async event will be sent */
	struct sd_adap_info  *device = NULL;   /* devices async is for  */
	uchar 		     address = 0;      /* address async is for  */
	uchar 		     event = 0;        /* event async is for    */


	cmd = NULL;

	/*
	 * get cmd structure 
	 */
	cmd = sd_get_struct(dp->ap);
	cmd->cp = dp->cp;
	cmd->dp = dp;

	switch (op) {
	      case IOCINFO: 
		ret_code = sd_dasds_info(sd_info,devflag,chan,ext,dp,op);
		break;
	      case SD_RESET:
		ret_code = sd_ioctl_reset(ioctl_parms,devflag,chan,ext,
					 (struct sd_adap_info *) dp,
					  cmd,op,SD_DASD_CMD,0,&async_flg,
					  &device,&address, &event,0);
		break;
	      case SD_SCSICMD:
		ret_code = sd_ioctl_scsicmd(iocmd,devflag,chan,ext,
					   (struct sd_adap_info *)dp,
					   cmd,op,SD_DASD_CMD,&dwnld_flg,
					    &reset_flg);
		break;
	      case SD_QUERY_DEVICE:
		ret_code = sd_dasd_qry_dev(ioctl_parms,devflag,chan,
					   ext,dp,cmd,op);
		break;
	      case SD_SET_FENCE:
		/*
		 * If we do not know it, we must first ascertain this
		 * host's Fence Position Indicator (FPI)
		 */
		if (!dp->fence_data_valid) 
		{
		    ret_code = sd_ioctl_get_fpi (ioctl_parms,devflag,chan,ext,
		                           dp,cmd,op);
		    if (ret_code) {
			sd_ioctl_free(op,devflag,iocmd,ioctl_parms,cmd);
			sd_free_cmd_disable(cmd);
			return (ret_code);
		    }
		    ap = dp->ap;
		    ret_code = sd_ioctl_finish(op,iocmd,ioctl_parms,cmd,ap,
					       dp,2,time_out,dwnld_flg,reset_flg,
					       async_flg,device,address,event,
					       devflag);
		    /*
		     * sd_ioctl_finish will have filled in the FPI in the
		     * dasd structure if the command worked.
		     * Indication of failure is either a bad rteturn code
		     * or an error in status_validity.
		     */
		    if (ret_code || ioctl_parms->status_validity)
			return (ret_code);
		    /*
		     * Need to reallocate a cmd structure
		     */
		    cmd = NULL;
		    cmd = sd_get_struct(dp->ap);
		    cmd->cp = dp->cp;
		    cmd->dp = dp;
		} 
		ret_code = sd_ioctl_set_fence (ioctl_parms,devflag,chan,ext,
		                           dp,cmd,op);

		 break;
	      case DD_CONC_REGISTER:
		if ((!dp->cp->dds.conc_enabled) || (dp->conc_registered) || !(devflag & DKERNEL))
		    ret_code = EINVAL;
		else
		{
		    dp->conc_intr_addr = conc_register->conc_intr_addr;
		    conc_register->conc_func_addr = sd_concurrent;
		    dp->saved_no_reserve = dp->no_reserve;
		    dp->conc_registered = TRUE;
		}
		break;
		
	      case DD_CONC_UNREGISTER:
		if ((!dp->conc_registered) || !(devflag & DKERNEL))
		    ret_code = EINVAL;
		else
		{
		    dp->ioctl_intrpt = 1;	
		    dp->unregistering = TRUE;
		    if ((dp->concurrent.status != SD_FREE) || 
			(dp->conc_cmd_list != NULL))
		    {
			/*
			 * There are concurrent commands waiting 
			 * or active....need to wait for them to
			 * finish
			 */
			sd_sleep(dp->ap,&dp->ioctl_intrpt,&dp->ioctl_event);
		    }
		    dp->unregistering = FALSE;
		    dp->no_reserve = dp->saved_no_reserve;
		    dp->conc_registered = FALSE;
		    dp->conc_intr_addr = NULL;          /* for safety */
		}
		break;
	      case SD_CLEAR_FENCE:
		ret_code = sd_ioctl_clear_fence (ioctl_parms,devflag,chan,ext,
		                           dp,cmd,op);
		 break;
	      default:
		ret_code = EINVAL;
	    }
    		
	if (ret_code) {

		sd_ioctl_free(op,devflag,iocmd,ioctl_parms,cmd);
		sd_free_cmd_disable(cmd);
		return (ret_code);
	}
	if ((op != IOCINFO) && 
	    (op != DD_CONC_REGISTER) && 
	    (op != DD_CONC_UNREGISTER)) {
		ap = dp->ap;
		ret_code = sd_ioctl_finish(op,iocmd,ioctl_parms,cmd,ap,
					   dp,2,time_out,dwnld_flg,reset_flg,
					   async_flg,device,address,event,
					   devflag);
	}
	else {
		sd_ioctl_free(op,devflag,iocmd,ioctl_parms,cmd);
		sd_free_cmd_disable(cmd);
	}
        
	return (ret_code);
}




/* 
 *
 * NAME: sd_dasds_info
 *                  
 * FUNCTION: Get Iocinfo for dasd
 *                                                 
 *     This routine returns the devinfo information about the dasd to 
 *     the caller 
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_dasd_info  - dasd's information struct
 *
 *
 * CALLED BY:
 *      sd_dasd_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 *      None
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *      None
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:      
 *        0         -   Successful completion.
 */                                                                         

int sd_dasds_info(
struct devinfo *arg,
ulong devflag,
int chan,
int ext,
struct sd_dasd_info *dp, 
int op)
{

	int                 ret_code = 0;



	arg->devtype = DD_SCDISK;
	arg->devsubtype =DS_PV;
	if (dp->cp->dds.fence_enabled)
	    arg->flags = (uchar) (DF_RAND | DF_FAST| DF_CONC);
	else
	    arg->flags = (uchar) (DF_RAND | DF_FAST);
	/*
	 * using the scdisk structure of devinfo
	 */


	arg->un.scdk.blksize = SD_BPS;

	/*
	 *  Fix next two requests
	 */
	arg->un.scdk.numblks = dp->disk_capacity.lba + 1;   
	arg->un.scdk.max_request = dp->max_transfer;     
	arg->un.scdk.segment_size = dp->dds.segment_size;
	arg->un.scdk.segment_count = dp->dds.segment_cnt;
	arg->un.scdk.byte_count = dp->dds.byte_count;


	return (ret_code);

}



/* 
 *
 * NAME: sd_dasd_qry_dev 
 *                  
 * FUNCTION: Query device (dasd)
 *                                                 
 *     This routine creates a mailbox to send a query device message
 *     to the controller.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_dasd_info  - dasd's information struct
 *
 *
 * CALLED BY:
 *      sd_dasd_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 *       SD_LUNTAR
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *       None
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:      
 *       
 *        EACCES    -   Permission denied
 *        0         -   Successful completion.
 */                                                                         

int sd_dasd_qry_dev(
struct sd_ioctl_parms *arg,
ulong devflag,
int chan,
int ext,
struct sd_dasd_info *dp,
struct sd_cmd *cmd,
int op)
{
	int                 ret_code = 0;  /* return code */
	struct sd_adap_info         *ap;   /* adapter */


	ap = dp->ap;
	if ((!(dp->diag_mode)) && 
	    (getpid() != ap->daemon_pid))		
		/*
		 * must be in diagnostic mode or the process must be
		 * a child of the daemon for 
		 * this command
		 */
		return (EACCES);
	


	/*
	 * fill in cmd and mailbox
	 */

	cmd->type = SD_DASD_CMD;
	cmd->mbox_copy.op_code = SD_QUERY_DEV_OP;
	cmd->mbox_copy.mb6.length = 0;

	cmd->mbox_copy.mb7.dev_address = SD_LUNTAR(dp->cp->dds.target_id,
						arg->resvd5,
						SD_LUNDEV);

	return (ret_code);

}



  /* Other Functions */

/* 
 *
 * NAME: sd_get_struct
 *                  
 * FUNCTION: Get command jacket
 *                                                 
 *   Wait until a command jacket is free and then allocate it from the
 *   cmd pool of the associated adapter.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct  
 *
 *
 * CALLED BY:
 *      sd_adap_ioctl                 sd_ctrl_ioctl
 *      sd_dasd_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 *      sd_cmd_alloc_disable          sd_sleep
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *      None
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:      
 *     pointer to command structure or null.
 */                                                                         

struct sd_cmd *sd_get_struct(
struct sd_adap_info *ap)
{
	int             waiting;
	struct sd_cmd   *cmd;     /* command jacket */



	waiting = TRUE;
	while (waiting) {  /*  do not continue until have a sd_cmd structure.*/
		cmd = sd_cmd_alloc_disable(ap);                 
		if (cmd == NULL) {
			ap->resource_intrpt = 1;
			sd_sleep(ap,&ap->resource_intrpt, &ap->resources);
		}
		else
			waiting = FALSE;
	}
	cmd->cmd_info = SD_IOCTL;
	cmd->xmem_buf == NULL;
	cmd->driver_status = 0;
	cmd->b_length = 0;
	return (cmd);
}

/* 
 *
 * NAME: sd_ioctl_reset 
 *                  
 * FUNCTION:     Quisce or reset the adapter,controller,dasd
 *
 *      If quisce operation then use mailbox mechanism. If full adapter
 *      reset then reset bit in POS2 register.
 *                                                 
 *      Check all configured adapters to see if any of their dasd are hung
 *      on a  command. If so then issue a query device mailbox to the     
 *      effected dasd.  Ignore devices in diagnostic mode.  Also check that
 *      all adapter daemons  are still running.  If not start a new one.   
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *

 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct
 *
 *
 * CALLED BY:
 *      sd_adap_ioctl                sd_ctrl_ioctl
 *      sd_dasd_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 *      SD_LUNTAR                                   
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *      IOCC_ATT                     BUSIO_PUTC
 *      IOCC_DET                     BUSIO_GETC
 *  
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:     
 *       
 *        EACCES    -   Permission denied
 *        EINVAL    -   Invalid parameter
 *        EIO       -   I/O error
 *        0         -   Successful completion.
 */                                                                         

int sd_ioctl_reset(
struct sd_ioctl_parms *arg,
ulong devflag,
int chan,
int ext,
struct sd_adap_info *ap, 
struct sd_cmd *cmd,
int op, 
int dev_type,
int dont_signal,
int *async_flg,
struct sd_adap_info **device,
uchar *address,
uchar *event,
int   ctrl_dwnld)
{
	uint          base;           /* base adress */
	uchar         pos0;           /* value of POS register 0 */
	uchar         pos1;           /* value of POS register 1 */
	uchar         pos2;           /* value of pos reg 2 */
	ushort        id;             /* value of pos id */
	int           ret_code = 0;   /* return code */
	int           reset_type;     /* reset operation requested */
	uchar         dev_address;    /* device's address */
	struct sd_ctrl_info   *cp;    /* controller */
	struct sd_dasd_info   *dp;    /* dasd */


	if (dev_type == SD_CTRL_CMD)	
		cp = (struct sd_ctrl_info *) ap;
        if ((!ctrl_dwnld) || (privcheck(RAS_CONFIG)) || (op != SD_QUIESCE_OP) 
	    || (dev_type != SD_CTRL_CMD)) {
		/*
		 * We allow one exception to the following permission checks
		 * That is the case where a controller microcode download is
		 * being performed.  The following will then
		 * be true:  ctrl_dwnld = 1 - this is the only case where 
		 *			      this will be one
		 *	     root authority
		 *	     quisce operation - this is necessary before a
		 *				download
		 *           dev_tpe = SD_CTRL_CMD
		 *
		 *
		 * All other cases must go thru the following
		 * permission checking
		 */
		if ((!ap->diag_mode) && 
		    (getpid() != cmd->ap->daemon_pid)) {
			if ((dev_type == SD_CTRL_CMD) && (!cp->ap->diag_mode))
				
				/*
				 * must be in diagnostic mode or 
				 * the process must be
				 * daemon for this command
				 */
				return(EACCES);
			if (dev_type == SD_ADAP_CMD)
				/*
				 * must be in diagnostic mode or 
				 * the process must be
				 * daemon for this command
				 */
				return(EACCES);
			
			if (dev_type == SD_DASD_CMD)
				/*
				 * must be in diagnostic mode 
				 * or the process must be
				 * daemon for this command
				 */
				return(EACCES);
		}
	}
	if ((arg->reset_type == SD_RESET_OP) && (dev_type == SD_ADAP_CMD)) {
		ap->status |= SD_RESET_PENDING;
		/*
		 * reset pos register 2 of adapter;
		 */
		
		/*
		 *  use offset with above base to
		 * get POS reg 2
		 */
		if (sd_write_POS(ap,SD_POS2,(uchar)SD_ADAP_RESET_BIT)) {
			return(EIO);
		}
		
		/*
		 * check here to make sure card is ready for POS 
		 * reg set-up
		 */
		
		ap->IPL_tmr_cnt = 0;    /* set-up initial value */
		ap->reset_result = FALSE;	/* clear result flag */
		sd_wait_reset_disable(ap->reset_timer);
		while (!ap->reset_result) {
			/*
			 * loop waiting for either card ready or 
			 * timeout
			 */
			delay(HZ);
		}
		if (ap->reset_result == (char)-1)
			return(EIO);
		sd_reload_pos(ap);
		sd_fail_adap_disable(ap);
		sd_restart_adap(ap);
		base = (uint)BUSIO_ATT(ap->dds.bus_id,ap->dds.base_addr);
		if (SD_PUTC(base + SD_CTRL, SD_INT_ENABLE))
			return(EIO);
		BUSIO_DET(base);
		delay(5*HZ);
		sd_async_event(ap,(uchar)0,(uchar)SD_ADAPDLMC,SIGTRAP);
		return(ret_code);

	}  /*if  SD_FULL_RESET */
	else if ((arg->reset_type == SD_QUIESCE_OP) && 
		(dev_type == SD_ADAP_CMD)) {
		reset_type = SD_QUIESCE_ADAP_MB;
		dev_address = 0;
	}
	else if ((arg->reset_type == SD_RESET_OP) && 
		 (dev_type == SD_CTRL_CMD)) {
		reset_type = SD_RESET_CTRL_MB;
		cp = (struct sd_ctrl_info *) ap;
		dev_address = SD_LUNTAR(cp->dds.target_id,0x00,SD_TARDEV);
		if (!dont_signal) {
			*async_flg = TRUE;
			*device = cp->ap;
			*address = dev_address;
			*event = SD_DLMC;
		}
	}
	else if ((arg->reset_type == SD_QUIESCE_OP) && 
		 (dev_type == SD_CTRL_CMD)) {
		reset_type = SD_QUIESCE_CTRL_MB;
		cp = (struct sd_ctrl_info *) ap;
		dev_address = SD_LUNTAR(cp->dds.target_id,0x00,SD_TARDEV);
	}
	else if ((arg->reset_type == SD_RESET_OP) && 
		 (dev_type == SD_DASD_CMD)) {
		reset_type =  SD_RESET_DASD_MB;
		dp = (struct sd_dasd_info *) ap;
		dev_address = SD_LUNTAR(dp->cp->dds.target_id,dp->dds.lun_id,
				     SD_LUNDEV);
	}
	else if ((arg->reset_type == SD_QUIESCE_OP) && 
		 (dev_type == SD_DASD_CMD)) {
		reset_type = SD_QUIESCE_DASD_MB;
		dp = (struct sd_dasd_info *) ap;
		dev_address = SD_LUNTAR(dp->cp->dds.target_id,dp->dds.lun_id,
				     SD_LUNDEV);
	}
	else {
		return(EINVAL);
	}
	/* 
	 * fill in cmd and mailbox 
	 */

	cmd->type = dev_type;
	cmd->mbox_copy.op_code = SD_RSTQSC_OP;
	cmd->mbox_copy.mb6.reset_type = reset_type;
	cmd->mbox_copy.mb7.dev_address = dev_address;

	return (ret_code);
}
/* 
 *
 * NAME: sd_ioctl_build_fence
 *                  
 * FUNCTION: Build a fence command for sending to the subsystem.
 *           This function holds all the common code for generating
 *           the various flavour of fence command.
 *                                                 
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct
 *
 *
 * CALLED BY:
 *      sd_ioctl_get_fpi                    sd_ioctl_set_fence
 *      sd_ioctl_clear_fence
 *
 * INTERNAL PROCEDURES CALLED:
 *   
 *      sd_ioctl_wait                    sd_prepare_dma
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *
 *      xmalloc                          xmfree
 *
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:         ENOMEM     -  xmalloc failed
 *                  EINVAL     -  operation in invlaid on this level of hardware
 */                                                                         

int sd_ioctl_build_fence(
struct sd_dasd_info *dp, 
struct sd_cmd *cmd,
ushort sd_fence_mask,
ushort sd_fence_data)
{
    struct sd_adap_info *ap;
    struct sd_fence_info *fence_buf;
    struct sc_cmd *scsi;
    uchar tag;
    
    ap = dp->ap;
    

    /*
     * cant allow it if hardware does not support it
     */
    
    if (!dp->cp->dds.fence_enabled)
	return (EINVAL);

    /* 
     * xmalloc a buffer to transfer into
     */

    fence_buf = (struct sd_fence_info *)xmalloc(
			(uint)sizeof(struct sd_fence_info),PGSHIFT,
				(heapaddr_t)pinned_heap);
    /*
     * assign cmd->b_addr now so sd_ioctl_free knows to free it
     */
    cmd->b_addr = (char *)fence_buf;                
    if (fence_buf == NULL)
	return (ENOMEM);

    /*
     * Set up dummy cross memory desriptor for the kernel buffer
     * we are using.
     */
    
    cmd->xmem_buf = 
	(struct xmem *) xmalloc((uint) sizeof(struct xmem),
				3,(heapaddr_t) pinned_heap);
    if (cmd->xmem_buf == NULL) {
	return (ENOMEM);
	
    }
    else {
	cmd->xmem_buf->aspace_id = XMEM_GLOBAL;
    }
	
    /*
     * Fill in cmd and mailbox 
     */
    cmd->type = SD_DASD_CMD;
    cmd->mbox_copy.op_code = SD_SEND_SCSI_OP;
    cmd->dma_flags = DMA_READ;
    cmd->mbox_copy.mb6.qc_scsiext = SD_Q_ORDERED;
    cmd->mbox_copy.mb7.dev_address = SD_LUNTAR(
	      dp->cp->dds.target_id, dp->dds.lun_id,SD_LUNDEV);
    cmd->b_length = sizeof(struct sd_fence_info);
    scsi = &(cmd->mbox_copy.mb8.fields.scsi_cmd);
    scsi->scsi_op_code = SD_FENCE_OP_CODE;
    scsi->lun = SD_FENCE_MASK_SWAP;
    scsi->scsi_bytes[0] = (uchar)(sd_fence_mask >>8);
    scsi->scsi_bytes[1] = (uchar)(sd_fence_mask & 0xFF); 
    scsi->scsi_bytes[2] = (uchar)(sd_fence_data >>8); 
    scsi->scsi_bytes[3] = (uchar)(sd_fence_data & 0xFF); 
    scsi->scsi_bytes[4] = 0x00;
    scsi->scsi_bytes[5] = 0x00;
    scsi->scsi_bytes[6] = (uchar)sizeof(struct sd_fence_info);
    scsi->scsi_bytes[7] = 0x00;

    return(0);
}

/* 
 *
 * NAME: sd_ioctl_get_fpi
 *                  
 * FUNCTION: ascertain the Current Fence position indicator (FPI)
 *                                                 
 *     This is called during the first SD_SET_FENCE ioctl since open or since
 *     a SD_CLEAR_FENCE ioctl. A 'null' fence command is 
 *     issued to determine the fence position indicator.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct
 *
 *
 * CALLED BY:
 *      sd_dasd_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 *   
 *      sd_ioctl_wait                    sd_prepare_dma
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *
 *      xmalloc                          xmfree
 *
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:         ENOMEM     -  xmalloc failed
 *                  EINVAL     -  operation in invlaid on this level of hardware
 */                                                                         

int sd_ioctl_get_fpi(
struct sd_ioctl_parms *arg,
ulong devflag,
int chan,
int ext,
struct sd_dasd_info *dp, 
struct sd_cmd *cmd,
int op)
{
    int ret_code =0;

    /*
     * Verify access authority.
     * Allow the command only if the caller has root authority.
     */
    
    if (privcheck (RAS_CONFIG))
	return (EACCES);
    
    ret_code = sd_ioctl_build_fence(dp,cmd,SD_FENCE_NONE,SD_FENCE_NONE);

    return(ret_code);
}

/* 
 *
 * NAME: sd_ioctl_set_fence
 *                  
 * FUNCTION: Process a SD_SET_FENCE ioctl
 *                                                 
 *     The caller's fence data is first examined to check that
 *     it is legal before the fence command is issued to the 
 *     device.
 *     The caller's fence data and mask are preserved to be re_issued
 *     during the verify sequence.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct
 *
 *
 * CALLED BY:
 *      sd_dasd_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 *   
 *      sd_ioctl_wait                    sd_prepare_dma
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *
 *      xmalloc                          xmfree
 *
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:         ENOMEM     -  xmalloc failed
 *                  EACCESS    -  caller does not have root authority
 *                  EINVAL     -  operation in invlaid on this level of hardware
 *                  EXDEV      -  improper link ; tried to fence out source.
 */                                                                         

int sd_ioctl_set_fence(
struct sd_ioctl_parms *arg,
ulong devflag,
int chan,
int ext,
struct sd_dasd_info *dp, 
struct sd_cmd *cmd,
int op)
{
    int ret_code =0;

    /*
     * Verify access authority.
     * Allow the command only if the caller has root authority.
     */
    
    if (privcheck (RAS_CONFIG))
	return (EACCES);
	
    /*
     * Check the ioctl isnt trying to fence out this host.
     */
    
    if (((ushort)arg->resvd1 & (ushort)arg->resvd2 & dp->fence_host_position) != 0) 
    {
	return (EXDEV);
    } 
    /*
     * store the fence information for verify sequence
     * new = (data &mask) | (old & !mask)
     */
    
    dp->fence_data = ((ushort)(arg->resvd1) & (ushort)arg->resvd2) | (dp->fence_data & ~(ushort)(arg->resvd1)) ;
    dp->fence_mask = dp->fence_data;
    dp->fence_data_valid = TRUE;
	
    ret_code = sd_ioctl_build_fence(dp,cmd,(ushort)arg->resvd1,(ushort)arg->resvd2);

    return(ret_code);

}


/* 
 *
 * NAME: sd_ioctl_clear_fence
 *                  
 * FUNCTION: Process a SD_CLEAR_FENCE ioctl
 *                                                 
 *     This ioctl clears fences set with the SD_SET_FENCE ioctl.
 *     it does this by performing two actions.
 *
 *     1. It sends a command to the subsystem to clear all fences for
 *        the addressed dasd.                
 *     2. It clears the stored fence data from the dasd info structure so
 *        that the fence will not be re-applied during the dasd verify
 *        sequence.
 *     
 *     If a SD_SET_FENCE ioctl has not been issued since config time or since
 *     the last SD_CLEAR_FENCE ioctl then dp->fence_data_valid
 *     will be false and the ioctl will fail with EINVAL.
 *                                                                   
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct
 *
 *
 * CALLED BY:
 *      sd_dasd_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 *   
 *      sd_ioctl_wait                    sd_prepare_dma
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *
 *      xmalloc                          xmfree
 *
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:         ENOMEM     -  xmalloc failed
 *                  EACCESS    -  caller does not have root authority
 *                  EINVAL     -  operation in invlaid on this level of hardware
 */                                                                         

int sd_ioctl_clear_fence(
struct sd_ioctl_parms *arg,
ulong devflag,
int chan,
int ext,
struct sd_dasd_info *dp, 
struct sd_cmd *cmd,
int op)
{
    int ret_code =0;
    struct sd_adap_info *ap;
    struct sd_fence_info *fence_buf;
    struct sc_cmd *scsi;
    
    const int async_flag = FALSE;
    const uint time_out= 30;
    const int flag = 2;

    
    ap = dp->ap;
    
    /*
     * Verify access authority.
     * Allow the command only if the caller has root authority.
     */
    
    if (privcheck (RAS_CONFIG))
	return (EACCES);

    /*
     * Fail if a fence has not been set up.
     */
    
    if (!dp->fence_data_valid)
	return (EINVAL);

    /*
     * Invalidate stored fence data.
     */

    dp->fence_data_valid = FALSE;
    
    ret_code = sd_ioctl_build_fence(dp,cmd,dp->fence_data,SD_FENCE_NONE);

    /*
     * Clear stored fence data.
     */

    dp->fence_mask = 0;
    dp->fence_data = 0;

    return (ret_code);
}

   
 
/* 
 *
 * NAME: sd_ioctl_scsicmd
 *                  
 * FUNCTION: Send scsi command to adapter
 *                                                 
 *     This creates a mailbox containing a scsi command issued
 *     by the caller   
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct
 *
 *
 * CALLED BY:
 *      sd_adap_ioctl                    sd_ctrl_ioctl
 *      sd_dasd_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *      sd_prepare_dma                   sd_copyin
 *      sd_copyout                       sd_free_cmd_disable
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:     
 *       
 *        EACCES    -   Permission denied
 *        EFAULT    -   xmattached failed
 *        EINVAL    -   Invalid parameter
 *        EIO       -   I/O error
 *        ENOMEM    -   Not enough memory ( xmalloc failed)
 *        0         -   Successful completion.
 *        -1        -   no mailbox command is necessary 
 */                                                                         

int sd_ioctl_scsicmd(
struct sd_iocmd *arg,
ulong devflag,
int chan,
int ext,
struct sd_adap_info *ap, 
struct sd_cmd *cmd,
int op,
int cmd_type,
int *dwnld_flg,
int *reset_flg)
{

	int                 ret_code = 0;  /* return code */
	uchar               byte0;         /* first byte of buffer */
	uchar               *cdb;          /* pointer to scsi command block */
	int                 scsi_flg = 0;  /* if 1 then send scsi command */
	uchar               dev_address;   /* device's address */
	int                 daemon_flg = 0;/* 1 if daemon calling */
	int                 diag_flg = 0;  /* 1 if controller's adap in diag */
	struct sd_ctrl_info *cp;           /* controller */
	struct sd_dasd_info *dp;           /* dasd */


	cdb = (uchar *) arg->scsi_cdb;

	if (cmd_type == SD_ADAP_CMD) {
		if (getpid() == ap->daemon_pid) 
			scsi_flg = 1;
	}
	else if (cmd_type == SD_CTRL_CMD) {
		cp = (struct sd_ctrl_info *)ap;
		if (cp->ap->diag_mode) 
			scsi_flg = 1;
	}
 
	else if (cmd_type == SD_DASD_CMD) {
		/* 
		 * Check if one of DASD's parents is in diag mode
		 */
		dp = (struct sd_dasd_info *)ap;
		if ((dp->cp->diag_mode) || (dp->cp->ap->diag_mode))
			scsi_flg = 1;
	}
	/*
	 * Verify access permission
	 */

	if (ap->diag_mode) {        /* if device is in diag mode */
		scsi_flg = 1;
	}
	else {

		ret_code=sd_copyin((char *)&byte0,(char *)arg->buffer,
				   sizeof(SD_BYTE),devflag);
		if (ret_code) {
			return (ret_code);
		}


		/*
		 * If scsi command is a send diagnostic command
		 * allow it if it is also one of the trace commands
		 * and if slftest = 0
		 */

		if ((cdb[0] == SCSI_SEND_DIAGNOSTIC) && 
		    (!privcheck(RAS_CONFIG))) {
			if ((byte0 == SD_RESET_DASD) &&
			    (!(cdb[1] & SD_SELF_TEST_BIT))) {
				/*
				 * Allow dasd absolute resets in normal
				 * opens for twin tailing purposes.
				 */
				 
				scsi_flg = 1;
			}
			else if(cdb[1] & SD_SELF_TEST_BIT) {  /* if slftst */
				ret_code = EACCES;
			}
			else {
				/*
				 * get trace info if applicable.
				 */
					
				if ((byte0 == SD_SET_TRACE_SNAPSHOT) || 
				    (byte0 == SD_PREPARE_TRACE_STATUS) ||
				    (byte0 == SD_PREPARE_TRACE_DUMP)) {
					/*
					 * Trace command
					 */
					scsi_flg = 1;
				}
				else
					ret_code = EACCES;
			}
		}
		else if ((cdb[0] == SCSI_RECEIVE_DIAGNOSTIC_RESULTS) && 
			 (!privcheck(RAS_CONFIG))){
			/*
			 * Scsi inquiries can drop through
			 */
			scsi_flg = 1;
		}
		else if ((cdb[0] == SCSI_INQUIRY) && (!privcheck(RAS_CONFIG))){
			/*
			 * Scsi inquiries can drop through
			 */
			scsi_flg = 1;
		}
		else if ((cdb[0] == SCSI_READ) && (!privcheck(RAS_CONFIG))){
			/*
			 * Scsi reads can drop through
			 */
			scsi_flg = 1;
		}
		else if ((cdb[0] == SCSI_READ_EXTENDED) && 
			 (!privcheck(RAS_CONFIG))){
			/*
			 * Scsi reads can drop through
			 */
			scsi_flg = 1;
		}
		else if ((cdb[0] == SCSI_REQUEST_SENSE) && 
			 (!privcheck(RAS_CONFIG))){
			/*
			 * Scsi reads can drop through
			 */
			scsi_flg = 1;
		}
		else if ((cdb[0] == SCSI_WRITE_BUFFER) && 
			 (!privcheck(RAS_CONFIG))){
			/*
			 * Scsi write buffers can drop through
			 */
			scsi_flg = 1;
		}
		else if ((cdb[0] == SD_FENCE_OP_CODE) && 
			 (!privcheck(RAS_CONFIG))){
		        /*
			 * Pseudo-Scsi fence commands can drop through
			 */
		    scsi_flg = 1;
		}
		
	}


	/*
	 * fill in cmd and mailbox
	 */

	if (!scsi_flg) 
		return(EACCES);


	/*
	 * Download microcode to the controller with length
	 * of zero is allowed as follows
	 */



	cmd->type = cmd_type;
	cmd->mbox_copy.op_code = SD_SEND_SCSI_OP;
	if (arg->flags & B_READ)
		cmd->dma_flags = DMA_READ;
	else 
		cmd->dma_flags = 0;       /* DMA write */

	cmd->mbox_copy.mb6.qc_scsiext = SD_QC_SCSI(arg->resvd6,arg->resvd7);
	if (cmd_type == SD_ADAP_CMD)
		cmd->mbox_copy.mb7.dev_address = arg->resvd5;
	else if (cmd_type == SD_CTRL_CMD) {
		cp = (struct sd_ctrl_info *) ap;
		cmd->mbox_copy.mb7.dev_address = SD_LUNTAR(cp->dds.target_id,
		   (arg->resvd5 & 0x1F), SD_LUNDEV);
	}	
	else if (cmd_type == SD_DASD_CMD) {
		dp = (struct sd_dasd_info *) ap;
		cmd->mbox_copy.mb7.dev_address = SD_LUNTAR(
			dp->cp->dds.target_id, dp->dds.lun_id,SD_LUNDEV);
	}
	else
		return(EIO);
	cmd->b_addr = arg->buffer; 
	cmd->b_length = arg->data_length;

	if ((int)arg->data_length > 0) {
		ret_code = sd_prepare_dma(cmd,arg->data_length,0,
					  arg->buffer,devflag);
		if (ret_code) {
			return(ret_code);
		}
	}
	/*
	 * insert scsi command into mailbox
	 */

	bcopy((char *) arg->scsi_cdb,
	      (char *)&cmd->mbox_copy.mb8.fields.scsi_cmd,
	      arg->command_length);

	if (ret_code)
		return(ret_code);

        if (cdb[0] == SCSI_WRITE_BUFFER) {
                *dwnld_flg = 1;
        }

	return (ret_code);
}

/* 
 *
 * NAME: sd_ioctl_finish 
 *                  
 * FUNCTION: Issues ioctl command and waits for it to return
 *                                                 
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct
 *
 *
 * CALLED BY:
 *      sd_adap_ioctl                   sd_ctrl_ioctl
 *      sd_dasd_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 *      sd_free_cmd_disable             sd_ioctl_wait
 *      sd_ioctl_status
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *      None
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:   
 *       
 *        ETIMEDOUT -   wait for response of command timed out  
 *        EBUSY     -   device still has pending status
 *        EINVAL    -   transfer size is too large.
 *        0         -   Successful completion. 
 */        

int sd_ioctl_finish(
int op,
struct sd_iocmd *iocmd,
struct sd_ioctl_parms *ioctl_parms, 
struct sd_cmd *cmd,
struct sd_adap_info *ap,
struct sd_dasd_info *dp,
int num,
uint time_out,
int dwnld_flg,
int reset_flg,
int async_flg,
struct sd_adap_info *device,
uchar address,
uchar event,
ulong devflag)
{
	int ret_code = 0;
	struct sd_fence_info *fence_buf;

	/* 
	 * Set up the dma length fields in the mailbox.
	 * This is not done for SD_MBOX ioctls since 
	 * we should pass through the value supplied by the 
	 * caller.
	 */
	if (op != SD_MBOX) {
	    cmd->mbox_copy.mb8.fields.dma_length = cmd->b_length;
	}
		
	/*
	 * Fail any commands with trasfer size larger
	 * then ap->max_transfer
	 */

	if (cmd->b_length > ap->max_transfer) {
 		if (cmd->mbox_copy.mb8.data.byte16 != SCSI_WRITE_BUFFER) {
 	        	sd_ioctl_free(op,devflag,iocmd,ioctl_parms,cmd);
 			sd_free_cmd_disable(cmd);		
 			return(EINVAL);
 		}
 		else 
 		if (cmd->b_length > (2 * ap->max_transfer)) {
 	        	sd_ioctl_free(op,devflag,iocmd,ioctl_parms,cmd);
 			sd_free_cmd_disable(cmd);		
 			return(EINVAL);
 		}
	}
	/* 
	 * set timer and wait for 
	 * reponse or timeout
	 */

	if (dwnld_flg)
		ret_code = sd_ioctl_download(cmd,ap,dp,num,time_out,reset_flg);
	else
		ret_code = sd_ioctl_wait(cmd,ap,dp,num,time_out,async_flg,
					 device,address,event,1);
	if (ret_code != ETIMEDOUT) {
		/* 
		 * return values to caller
		 */
		if (op == SD_SCSICMD) 
			sd_ioctl_status(cmd,iocmd);
		else {
			if (cmd->driver_status) {
				ioctl_parms->status_validity = SD_DRIVER_STATUS;
				ioctl_parms->adapter_status=cmd->driver_status;
			}
			else
			if (cmd->scsi_status) {
			    ioctl_parms->status_validity = SD_VALID_ADAP_STATUS
				                         | SD_VALID_CTRL_STATUS;
			    ioctl_parms->controller_status = cmd->scsi_status;
			    ioctl_parms->adapter_status = SD_SCSI_STATUS;
			}
			else {
			    ioctl_parms->status_validity = cmd->status_validity;
			    ioctl_parms->adapter_status=cmd->adapter_status;
			    ioctl_parms->controller_status =cmd->controller_status;
			}
		}

		/*
		 * Now, if this was a fencing operation which worked
		 * we should fill in the FPI, old fence position
		 * and device driver's version of the fence register
		 * for the user. We must also make a note of the fence
		 * position for ourselves.
		 */

		if (!ret_code && 
		    (op == SD_SET_FENCE || op == SD_CLEAR_FENCE))
		{    
		    if (!cmd->status_validity)
		    {
			fence_buf = (struct sd_fence_info *)cmd->b_addr;
			ioctl_parms->resvd3 = (uint)fence_buf->fence_posn;
			ioctl_parms->resvd4 = (uint)fence_buf->fence_oldvalue;
			ioctl_parms->resvd6 = (uint)dp->fence_data;
			
			/*
			 * The only time the returned FPI will be different 
			 * from the stored one is on the first SD_SET_FENCE or
			 * after a SD_CLEAR_FENCE. If the FPI changes at any other
			 * time then the verify sequence will not complete.
			 */ 
			ASSERT ((!dp->fence_data_valid) || (dp->fence_host_position == (ushort)fence_buf->fence_posn))
			    if (!dp->fence_data_valid) 
				dp->fence_host_position = (ushort)fence_buf->fence_posn;

		    }
		}
		
		sd_ioctl_free(op,devflag,iocmd,ioctl_parms,cmd);
		sd_free_cmd_disable(cmd);
		
		return (ret_code);
	}
}


/* 
 *
 * NAME: sd_ioctl_wait
 *                  
 * FUNCTION: Send out ioctl command and wait for response.
 *                                                 
 *   Call sd_start to issue command and wait for response or
 *   time out.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.  
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct
 * INPUTS:
 *      flag       -  = 1 if only adapter being added.
 *                    = 2 if both adapter and dasd being added.
 *                    = 3 if adapter for a controller is being added.
 *
 * CALLED BY:
 *      sd_ioctl_finish
 *
 * INTERNAL PROCEDURES CALLED:
 *        sd_add_ioctl                  sd_sleep
 *        sd_start
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:      
 *       
 *        ETIMEDOUT -   wait for response of command timed out
 *        EBUSY     -   device still has pending status
 *        0         -   Successful completion.
 */                                                                         

int sd_ioctl_wait(
struct sd_cmd *cmd,
struct sd_adap_info *ap,
struct sd_dasd_info *dp,
int flag,
uint time_out,
int async_flg,
struct sd_adap_info *device,
uchar address,
uchar event,
int chk_status)
{
	int            old_ilevel;        /* store interrupt's value */
	int            ret_code=0;          /* return code */
	int            i;                 /* general counter */
	int            timeout = 0;       /* whether there was a timeout */
	struct sd_dasd_info *dp2;
	int             count = 0;
	struct sd_ctrl_info *cp;






	/*
	 * lock to guarantee mutual 
	 * exclusion to device watchdog timer 
	 */
	
	
	if (chk_status) {
		if (cmd->type == SD_ADAP_CMD) {
			while ((dp->status) && (count < 20)) {
				delay (HZ);
				count++;
			}
			
		}
		else if (cmd->type == SD_CTRL_CMD) {
			cp = (struct sd_ctrl_info *)dp;
			while (((cp->ap->status) || (cp->status)) && 
			       (count < 20)) {
				delay (HZ);
				count++;
			}
			
		}
		else if (cmd->type == SD_DASD_CMD) {
			while (((dp->cp->status) || (dp->cp->ap->status) || 
				(dp->status)) && (count < 20)) {
				delay (HZ);
				count++;
			}
		}
		else
			ASSERT(FALSE);

		/*
		 * If we have been unable to start for 20 seconds return error.
		 * Use EBUSY unless there is a valid dp->dasd_result in which
		 * case use that.
		 */
		
		if (count == 20) {
		    if (cmd->type == SD_DASD_CMD)
		    {
			if ((dp->status) && (dp->dasd_result))
			    return(dp->dasd_result);
		    }
		    return(EBUSY);
		}
	}
	/*
	 * Note watch.func = sd_ioctl_timer();
	 */
                    

	/*
	 * initialize timeout flag
	 */
	dp->ioctl_timeout = 0;


	dp->ioctl_intrpt = 1;	
	/*
	 * set timeout, NOTICE: sd_start_cmd will start the watchdog timer
	 * as well as save a reference to this command with the watchdog struct
	 */
	dp->ioctl_timer.watch.restart = time_out;
	cmd->timeout = time_out;

	/*
	 * add cmd to ioctl events for 
	 * this adapter.
	 */
	sd_q_cmd_disable(cmd, (char)SD_QUEUE);

	/*
	 * start ioctl command
	 */
	sd_start_disable(ap);
	
	sd_sleep(ap,&dp->ioctl_intrpt,&dp->ioctl_event);





	if (dp->ioctl_timeout) {           /* if watch dog time out occurs */

		/*
		 * return time out 
		 */
		ret_code = ETIMEDOUT;
	}
	else {                /* if ioctl event occurs */
		/*
		 * stop timer.
		 */

		w_stop(&(dp->ioctl_timer.watch));
		if (async_flg) {
			delay(5*HZ);
			sd_async_event(device,(uchar)address,(uchar)event,
				       SIGTRAP);
		}
	}


	return (ret_code);
}

/* 
 *
 * NAME: sd_copyin 
 *                  
 * FUNCTION: Copy in data from callers space.
 *                                                 
 *    Determine if caller's data is in user or kernel space and
 *    use the corresponding copy routine.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct
 *
 * CALLED BY:
 *      sd_adap_ioctl                   sd_ctrl_ioctl
 *      sd_dasd_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 *      None
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *      bcopy                          copyin
 *
 * RETURNS:     
 *       
 *        EIO       -   I/O error
 *        0         -   Successful completion.
 */                                                                         

int sd_copyin( 
char *arga, 
char *argb, 
int length,
ulong devflag) 
{
	int                 ret_code = 0;  /* return code */


	if (devflag & DKERNEL) 
		bcopy((char *) argb, (char *) arga,length);
	else {
		if (copyin((char *) argb, (char *) arga,length) != 0) {
			ret_code = EIO;
		}
	}
	return (ret_code);
}


/* 
 *
 * NAME: sd_copyout 
 *                  
 * FUNCTION: Copy data out to caller's space.
 *    Determine if caller's data is in user or kernel space and
 *    use the corresponding copy routine.                                       *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct
 *
 *
 * CALLED BY:
 *      sd_adap_ioctl                      sd_ctrl_ioctl
 *      sd_dasd_ioctl
 *
 * INTERNAL PROCEDURES CALLED:
 *      None
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *     bcopy                         copyout
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:     
 *       
 *        EIO       -   I/O error
 *        0         -   Successful completion.
 */                                                                         

int sd_copyout( 
char *arga, 
char *argb, 
int length,
ulong devflag) 
{
	int                 ret_code = 0;  /* return code */


	if (devflag & DKERNEL) 
		bcopy((char *) argb, (char *) arga,length);
	else {
		if ( copyout((char *) argb, (char *) arga,length) != 0) {
			ret_code = EIO;
		}
	}
	return (ret_code);
}



/* 
 *
 * NAME: sd_prepare_dma
 *                  
 * FUNCTION: Prepare users data for dma 
 *                                                 
 *  
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct 
 *
 *
 * CALLED BY:
 *      A whole slew of ioctl's routines
 *
 * INTERNAL PROCEDURES CALLED:
 *      None
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *      xmalloc                     xmattach
 *      xmfree                      pinu
 *      upinu
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:     
 *       
 *        EFAULT    -   xmattached failed
 *        EINVAL    -   Invalid parameter
 *        ENOMEM    -   Not enough memory ( xmalloc failed)
 *        0         -   Successful completion.
 */                                                                         

int sd_prepare_dma(
struct sd_cmd *cmd,
uint length,
int min_length, 
char *buffer,
ulong devflag)
{
	int                 ret_code = 0;  /* return code */
	short                 segflg;      /* either user or system space*/


	if (devflag & DKERNEL) 
		segflg = SYS_ADSPACE;
	else
		segflg = USER_ADSPACE;

	if (length > min_length ) {

	      cmd->xmem_buf = 
		      (struct xmem *) xmalloc((uint) sizeof(struct xmem),
					      3,(heapaddr_t) pinned_heap);
		if (cmd->xmem_buf == NULL) {
			ret_code = ENOMEM;
		}
		else {

			cmd->xmem_buf->aspace_id = XMEM_INVAL;
			if ((ret_code = pinu((caddr_t) buffer,
					     length,segflg))!= 0) {
				xmfree((char *) cmd->xmem_buf,
				       (heapaddr_t)pinned_heap);
				return(ret_code);
			}
			if (xmattach((char *)buffer,(uint)length, 
				     cmd->xmem_buf,segflg) == XMEM_FAIL) {
				unpinu(buffer,length,segflg);
				xmfree((char *)cmd->xmem_buf,
				       (heapaddr_t)pinned_heap);
				return(EFAULT);
			}
		} /* else */
	}
	else
		return(EINVAL);
	return (ret_code);
}
   

/* 
 *
 * NAME: sd_ioctl_status
 *                  
 * FUNCTION: Parse returned status for sd_iocmd structure
 *                                                 
 *    
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct
 *
 *
 * CALLED BY:
 *      sd_ioctl_finish
 *
 * INTERNAL PROCEDURES CALLED:
 *       None
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *      assert
 *
 * (RECOVERY OPERATION:) No error recovery
 *
 * RETURNS:    Void
 */
 
void sd_ioctl_status(
struct sd_cmd *cmd,
struct sd_iocmd *arg)
{
	int             parse= 0;;   /* flag = 1 if should parse status */


	arg->resvd1 = cmd->controller_status;
	arg->resvd2 = cmd->adapter_status;
	arg->scsi_bus_status = cmd->scsi_status;
	arg->adapter_status = 0x00;
	parse = FALSE;
        if (!(cmd->status_validity)) {
                /*
                 * if no status, set to no status
                 */
                arg->status_validity = SD_NO_STATUS;
        }
        else if (cmd->status_validity & SD_VALID_SCSI_STATUS) {
                /*
                 * else if SCSI Status, set validity to SCSI Status
                 */
                arg->status_validity = 0x01;
        }
        else if (cmd->status_validity & 
		(SD_VALID_CTRL_STATUS | SD_VALID_ADAP_STATUS)) {
                /*
                 * else if Controller status , set validity to Adapter
                 * status and Alert Register contents valid
                 */

		if (cmd->driver_status) {
			/* 
			 * If device driver failed command
			 * due to some communication problem
			 * with the hardware
			 */
			arg->status_validity = 0x02;
			arg->adapter_status = SC_ADAPTER_HDW_FAILURE;
		}
		else {
			arg->status_validity = 0x06;
			parse = TRUE;
		}
        }
	else {
		ASSERT(parse);
	}
	if (parse) {
		switch(cmd->adapter_status) {
		      case SD_SCSI_STATUS: 
			arg->adapter_status = 0x00;
			break;
		      case SD_CTRL_STATUS:
        
			switch(cmd->controller_status) {
			      case SD_ABORTED_CMD:
				arg->adapter_status = SC_ADAPTER_SFW_FAILURE;
				break;
			      case SD_INVALID_QC:
				arg->adapter_status = SC_ADAPTER_SFW_FAILURE;
				break;
			      case SD_PURG_OS:
				arg->adapter_status = SC_SCSI_BUS_RESET;
				break;
			      case SD_PURG_EPOW:
				arg->adapter_status = SC_SCSI_BUS_RESET;
				break;
			      case SD_WBUFF_INPROG:
				arg->adapter_status = SC_ADAPTER_SFW_FAILURE;
				break;
			      case SD_INVALID_MSG:
				arg->adapter_status = SC_ADAPTER_SFW_FAILURE;
				break;
			      case SD_INVALID_MB:
				arg->adapter_status = SC_ADAPTER_SFW_FAILURE;
				break;
			      case SD_NO_VALID_DELAY:
				arg->adapter_status = SC_NO_DEVICE_RESPONSE;
				break;
			      default:
				arg->adapter_status = SC_ADAPTER_HDW_FAILURE;
			}
			break;
		      case SD_UNIT_POWERED_ON:
			ASSERT(FALSE);
			break;

		      case SD_DATA_DMA_FAIL:
		      case SD_MB_DMA_FAIL:
		      case SD_INVALID_MB_PTR:
		      case SD_DMA_COUNT_ERROR:
		      case SD_DNLD_MC_CHKSUM_FAIL:
		      case SD_DMA_DISABLED:
		      case SD_TRACE_DATA_DMA_ERR:
		      case SD_SYSTEM_DMA_HUNG:
			/*
			 * sc host io bus error
			 */
			arg->adapter_status = SC_HOST_IO_BUS_ERR;
			break;
		      case SD_TIME_OUT:
		      case SD_QUIESCE_TIME_OUT:
			/*
			 * sc cmd timeout
			 */
			arg->adapter_status = SC_CMD_TIMEOUT;
			break;
		      case SD_CTRL_NOT_RESPONDING:
		      case SD_OPEN_LINK:
		      case SD_LINK_ERROR:
		      case SD_RECOV_LINK_ERROR:
			/*
			 * sc no device response
			 */
			arg->adapter_status = SC_NO_DEVICE_RESPONSE;
			break;



		      case SD_ADAP_HDW_ERROR:
			/*
			 * sc adapter hdw failure
			 */
			arg->adapter_status =SC_ADAPTER_HDW_FAILURE;
			break;
		      case SD_CMD_TO_ACTIVE_TAG:
		      case SD_DNLD_ADAP_BUSY:
		      case SD_ILLEGAL_ADAP_CMD:
		      case SD_UNEXP_SCSI_STATUS:
		      case SD_UNEXP_CTRL_STATUS:
		      case SD_BAD_CTRL_ADDRESS:
		      case SD_MSG_TO_INACTIVE_TAG:
		      case SD_MSG_WRONG_LINK:
		      case SD_INVALID_LINK_MSG:
		      case SD_INVALID_ADAP_PARMS:
		      case SD_INVALID_TAG:
		      case SD_PREV_TRC_DUMP_BUSY:
			/*
			 * sc adapter sfw failure
			 */
			arg->adapter_status = SC_ADAPTER_SFW_FAILURE;
			break;

		      case SD_RESET_PURGE_TAG:
		      case SD_WBUFF_STARTED:
		      case SD_MB_TERM_INTERN_RST:
			arg->adapter_status = SC_SCSI_BUS_RESET;
			break;

		      case SD_TRACE_SUPERCEDED:
			/*
			 * no error
			 */
			arg->adapter_status = 0x00;            /*? ? ?*/
			break;
		      default:
			arg->adapter_status = SC_ADAPTER_HDW_FAILURE;
			/*
			 * no device response;
			 */
		}
	} /* if parse */
	return;
}
/* 
 *
 * NAME: sd_ioctl_download
 *                  
 * FUNCTION: Downloads microcode to either adapter or controller.
 *
 *      This routine first issues a quiesce adapter to quiet the adapter.
 *      Then it will issue the download command which will have been built
 *      already.
 *                                                 
 *    
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct
 *
 *
 * CALLED BY:
 *      sd_ioctl_finish
 *
 * INTERNAL PROCEDURES CALLED:
 *       
 *     sd_ioctl_wait                   sd_ioctl_reset
 *     sd_fail_adap_disable                    sd_get_struct
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *      None
 *
 * (RECOVERY OPERATION:) No error recovery
 *
 * RETURNS:  
 *        EACCES    -   Permission denied
 *        EFAULT    -   xmattached failed
 *        EINVAL    -   Invalid parameter
 *        EIO       -   I/O error
 *        ENOMEM    -   Not enough memory ( xmalloc failed)
 *        ETIMEDOUT -   wait for response of command timed out  
 *        0         -   Successful completion.
 *        -1        -   no mailbox command is necessary 
 */
 
int sd_ioctl_download(struct sd_cmd *cmd,      /* command jacket           */
		      struct sd_adap_info *ap, /* adapter structure        */
		      struct sd_dasd_info *dp, /* dasd or controller struc */
		      int num,                 /* used for ioctl_wait      */
		      int time_out,            /* time out value for command*/
		      int reset_flg)
{
	int			ret_code = 0; /* return code                */
	struct sd_cmd      	*cmd2;        /* used for quiesce command   */
	struct sd_adap_info     *ap2;         /* pointer to adapter         */
	struct  sd_ioctl_parms 	ioctl_parms;  /* used to call sd_ioctl_reset*/
	char    		*pending_flg; /* points to status flag      */
	int                     sid;          /* controller id              */
	int                     ctrl_cmd = FALSE; /* */
	int 		     async_flg = FALSE;/* if async event is needed */
	struct sd_adap_info  *device = NULL;   /* devices async is for  */
	uchar 		     address = 0;      /* address async is for  */
	uchar 		     async_event = 0;   /* event async is for    */    
	int                  *download_needed; /* points to download needed */
	/*
	 * Get command jacket will always succeed
	 */
	cmd2 = sd_get_struct(ap);
	
	/*
	 * Fill in ioctl parms structure
	 */ 

	ioctl_parms.reset_type = SD_QUIESCE_OP;
	
	/*
	 * Get correct status flag
	 */
	if (cmd->mbox_copy.mb8.fields.scsi_cmd.scsi_op_code == 
	    SCSI_WRITE_BUFFER) {
		/*
		 * if command is download microcode to controller
		 */
		cmd2->type = SD_CTRL_CMD;
		sid = SD_TARGET(cmd->mbox_copy.mb7.dev_address);
		ap2 = (struct sd_adap_info *)cmd->ap->ctrllist[sid];
		if (ap2 == NULL) 
			return(EINVAL);
		pending_flg = (char *)&(cmd->ap->ctrllist[sid]->status);
		cmd2->ap = ap;
		cmd2->cp = cmd->ap->ctrllist[sid];
		ctrl_cmd = TRUE;

	}
	else if (cmd->mbox_copy.op_code == SD_DOWNLOAD_MCODE){
		/*
		 * if command is download microcode to adapter
		 */
		pending_flg = (char *)&(ap->status);
		cmd2->type = SD_ADAP_CMD;
		cmd2->ap = ap;
		ap2 = ap;
	}
	else 
		ASSERT(0);
	
	*pending_flg |= SD_DOWNLOAD_PENDING;	

	ret_code = sd_ioctl_reset(&ioctl_parms,DKERNEL,
				  0,0,ap2,cmd2,SD_QUIESCE_OP,
				  cmd2->type,1,&async_flg,
				  &device,&address, &async_event,ctrl_cmd);
	


	/*
	 * set daemon priority bit of command status
	 */
	cmd2->status |= SD_DAEMON_PRI;
	/*
	 * Issue quiesce adapter or controller
	 */
	ret_code = sd_ioctl_wait(cmd2,ap,(struct sd_dasd_info *)ap2,num,30,
				 async_flg,device,address,async_event,0); 



	if (ret_code != ETIMEDOUT)
		sd_free_cmd_disable(cmd2);

	if (ret_code) {
		sd_ioctl_verify_dasd(ctrl_cmd,ap2);
		*pending_flg &= ~SD_DOWNLOAD_PENDING; 
		sd_start_disable(ap);
		return(ret_code);
 	}

	/*
	 * set daemon priority bit of command status
	 */
	cmd->status |= SD_DAEMON_PRI;
	/*
	 * Issue download command: !!!Time out extended + 30, should be put
	 * in daemon himself, catastrophic error recovery may cause additional
	 * delay before we can even issue the download.
	 */
	ret_code = sd_ioctl_wait(cmd,ap,dp,num,time_out,
				 async_flg,device,address,async_event,0);
	/*
	 * Fail outstanding commands and call start
	 */

	if (!ctrl_cmd)
		sd_fail_adap_disable(ap);

	sd_ioctl_verify_dasd(ctrl_cmd,ap2);
	*pending_flg &= ~SD_DOWNLOAD_PENDING;

	if (ret_code) {
		sd_start_disable(ap);
		return(ret_code);
 	}	
	if (!ctrl_cmd) {
		/*
		 * Adapter's parameters will be flushed from a microcode
		 * download so we must update them.
		 */
		ret_code = sd_set_adap_parms_disable(ap, (char)TRUE);
	}
	sd_start_disable(ap);
		


	return(ret_code);
}

/* 
 *
 * NAME: sd_ioctl_verify_dasd
 *                  
 * FUNCTION: Verifies all opened DASD after a microcode download. This
 *	     is needed since the quiesce prior to microcode download
 *	     removes any DASD reservations
 *                                                 
 *    
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct
 *
 *
 * CALLED BY:
 *      sd_ioctl_download
 *
 * INTERNAL PROCEDURES CALLED:
 *       
 *     sd_verify_disable
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *      None
 *
 * (RECOVERY OPERATION:) No error recovery
 *
 * RETURNS:  
 *        None
 */
 
void sd_ioctl_verify_dasd(int ctrl_cmd,      /* 1 if a controller command   */
		  struct sd_adap_info *ap)   /* either adapter or controller*/
{
	int 			i,j;          /* General counters	    */
	struct sd_ctrl_info	*cp;	      /* controller info structure  */
	struct sd_dasd_info	*dp;	      /* dasd info structure	    */

					   
	if (!ctrl_cmd)  {
		
		/*
		 * If this was an adapter microcode download then 
		 * we must re-verify all its DASD, because their reservations 
		 * will be lost.  There is a small window between the quiesce
		 * adapter and the verify DASD where the DASD will not be 
		 * reserved.
		 */
		
		for(i=0; i < SD_NUM_CTRLS ;i++) {
			/*
			 * for each possible controller on this adapter
			 */
			cp = ap->ctrllist[i];
			if (cp != NULL) {
				for(j=0; j < SD_NUM_DASD ;j++) {
					/*
					 * for each possible dasd 
					 * on this controller
					 */
					dp = cp->dasdlist[j];
					if (dp != NULL)
						/*
						 * if this dasd is configured, 
						 * not in diagnostic mode, but 
						 * opened
						 */
						sd_verify_disable(
						     (struct sd_adap_info *)dp,
						     (uchar)SD_DASD_CMD);
					
				}
			}

		}
	}
	else {		
		/*
		 * If this was a controller microcode download then 
		 * we must re-verify all its DASD, because their reservations 
		 * will be lost.  There is a small window between the quiesce
		 * controller and the verify DASD where the DASD will not be 
		 * reserved.
		 */

		cp = (struct sd_ctrl_info *) ap;
		for(i=0; i < SD_NUM_DASD ;i++) {
			/*
			 * for each possible dasd on this controller
			 */
			dp = cp->dasdlist[i];
			if (dp != NULL)
				/*
				 * if this dasd is configured, not
				 * in diagnostic mode, but opened
				 */
				sd_verify_disable((struct sd_adap_info *)dp,
					  (uchar)SD_DASD_CMD);
		}
	}

	return;
}
/* 
 *
 * NAME: sd_ioctl_free
 *                  
 * FUNCTION: Free up cross memory descriptor if one has been allocated,
 *           unpin any users data and free any
 *           kernel buffers allocated by the fence functions
 *                                                 
 *    
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_cmd        - command jacket
 *                     struct sd_adap_info  - adapter's information struct
 *
 *
 * CALLED BY:
 *      sd_adap_ioctl				sd_ctrl_ioctl
 *	sd_dasd_ioctl				sd_ioctl_finish
 *
 * INTERNAL PROCEDURES CALLED:
 *       None
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *      
 *       xmfree				unpinu
 *
 * (RECOVERY OPERATION:) No error recovery
 *
 * RETURNS:    Void
 */
 
void sd_ioctl_free(
int op,
ulong devflag,
struct sd_iocmd *iocmd,
struct sd_ioctl_parms *ioctl_parms,
struct sd_cmd *cmd)
{

	short                 segflg;      /* either user or system space*/

	if (devflag & DKERNEL) 
		segflg = SYS_ADSPACE;
	else
		segflg = USER_ADSPACE;

	if (cmd->xmem_buf != NULL) 
	{
	    if (!((op == SD_SET_FENCE) || (op == SD_CLEAR_FENCE))) 
	    {
	    /*
	     * Fence commands have a kernel buffer instead of user memory
	     * and the cross memory descriptor is not xmattached.
	     */
		if (op == SD_SCSICMD) 
		    unpinu(iocmd->buffer,iocmd->data_length,segflg);
		else 
		    unpinu(ioctl_parms->buffer,ioctl_parms->data_length,segflg);
		xmdetach(cmd->xmem_buf);
	    }
	    /*
	     * Free cross memory descriptor
	     */
	    xmfree((char *)cmd->xmem_buf,(heapaddr_t)pinned_heap);
	}
	/*
	 * If command was a fence command we may need to free the 
	 * pinned kernel buffer that was allocated.
	 */
	if (((op == SD_SET_FENCE) || (op == SD_CLEAR_FENCE))
	    && (cmd->b_addr != NULL)) 
	    xmfree((caddr_t)cmd->b_addr,(heapaddr_t)pinned_heap);
	return;
}


