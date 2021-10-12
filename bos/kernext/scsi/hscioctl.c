static char sccsid[] = "@(#)67	1.4.4.9  src/bos/kernext/scsi/hscioctl.c, sysxscsi, bos41J, 9511A_all 3/2/95 12:59:42";
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver
 *
 * FUNCTIONS:	hsc_ioctl, hsc_register_async_func, hsc_inquiry,
 *		hsc_readblk, hsc_startunit, hsc_testunitready,
 *		hsc_diagnostic, hsc_diag_test, hsc_wrap_test,
 *		hsc_reg_test, hsc_pos_test, hsc_reset_test,
 *		hsc_download, hsc_dnld_start_devs, hsc_bld_sc_buf, 
 *              hsc_alloc_dev,hsc_dealloc_dev, hsc_ioctl_sleep
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/************************************************************************/
/*                                                                      */
/* COMPONENT:   SYSXSCSI                                                */
/*                                                                      */
/* NAME:        hscioctl.c                                              */
/*                                                                      */
/* FUNCTION:    IBM SCSI Adapter Driver Source File                     */
/*                                                                      */
/*      This adapter driver is the interface between a SCSI device      */
/*      driver and the actual SCSI adapter.  It executes commands       */
/*      from multiple drivers which contain generic SCSI device         */
/*      commands, and manages the execution of those commands.          */
/*      Several ioctls are defined to provide for system management     */
/*      and adapter diagnostic functions.                               */
/*                                                                      */
/* STYLE:                                                               */
/*                                                                      */
/*      To format this file for proper style, use the indent command    */
/*      with the following options:                                     */
/*                                                                      */
/*      -bap -ncdb -nce -cli0.5 -di8 -nfc1 -i4 -l78 -nsc -nbbb -lp      */
/*      -c4 -nei -nip                                                   */
/*                                                                      */
/*      Following formatting with the indent command, comment lines     */
/*      longer than 80 columns will need to be manually reformatted.    */
/*      To search for lines longer than 80 columns, use:                */
/*                                                                      */
/*      cat <file> | untab | fgrep -v sccsid | awk "length >79"         */
/*                                                                      */
/*      The indent command may need to be run multiple times.  Make     */
/*      sure that the final source can be indented again and produce    */
/*      the identical file.                                             */
/*                                                                      */
/************************************************************************/

#include	"hscincl.h"
#include	"hscxdec.h"

/************************************************************************/
/*                                                                      */
/* NAME:        hsc_ioctl                                               */
/*                                                                      */
/* FUNCTION:    Adapter Driver IOCTL Routine                            */
/*                                                                      */
/*      This routine performs certain specific commands as indicated    */
/*      by the selected IOCTL operation.                                */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* NOTES:                                                               */
/*      This routine will accept commands to perform specific           */
/*      function and diagnostic operations on the adapter.              */
/*      Supported commands are:                                         */
/*                                                                      */
/*      IOCINFO       - Returns information about the adapter.          */
/*      SCIOSTART     - Open a SCSI ID/LUN.                             */
/*   +  SCIOSTOP      - Close a SCSI ID/LUN.                            */
/*   +  SCIOINQU      - Issue a SCSI Device Inquiry command.            */
/*   +  SCIOSTUNIT    - Issue a SCSI Start Unit command.                */
/*   +  SCIOTUR       - Issue a SCSI Test Unit Ready command.           */
/*   +  SCIOREAD      - Issue a SCSI Read command (6-byte).             */
/*   +  SCIOHALT      - Halt active command and fail queue of device.   */
/*   +  SCIORESET     - Send a Bus Device Reset msg to a SCSI device.   */
/*   *  SCIODIAG      - Run adapter diagnostics.                        */
/*   *  SCIOTRAM      - No operation. Returns ENXIO.                    */
/*   *  SCIODNLD      - Download microcode to the adapter.              */
/*      SCIOGTHW      - Returns 0 if gathered write is supported        */
/*                                                                      */
/*   + These ioctl options require an SCIOSTART before they are run.    */
/*     They also require the adapter to be opened in normal mode.       */
/*   * These ioctl options require the adapter to be opened in diag-    */
/*     nostic mode.                                                     */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      devno   - device major/minor number                             */
/*      op      - operation to be performed                             */
/*      arg     - pointer to the passed argument for the operation      */
/*      devflag - device flag                                           */
/*      chan    - unused                                                */
/*      ext     - unused                                                */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      (see also the prologs of the individual ioctl subroutines)      */
/*      0       - successful                                            */
/*      EIO     - kernel service failed or invalid operation            */
/*      ENXIO   - (in diag mode only) diag ioctl not supported          */
/*      EINVAL  - (all modes) invalid ioctl option                      */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      lockl           unlockl                                         */
/*      copyout                                                         */
/*                                                                      */
/************************************************************************/
int
hsc_ioctl(
	  dev_t devno,
	  int op,
	  int arg,
	  ulong devflag,
	  int chan,
	  int ext)
{
    struct adapter_def *ap;
    int     i_hash;
    int     rc, ret_code;
    struct devinfo scinfo;
    int (*addr)() = hsc_ioctl;


    ret_code = 0;	/* default to no errors found  */

    DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_IOCTL, ret_code, devno, op, devflag,
	    chan, ext);
    /* lock the global lock to serialize with open/close/config */
    rc = lockl(&(hsc_lock), LOCK_SHORT);	/* serialize this */
    if (rc != LOCK_SUCC) {
	ret_code = EIO;	/* error--kernel service call failed */
	goto end;
    }

    /* search adapter list for this devno */
    /* build the hash index */
    i_hash = minor(devno) & ADAP_HASH;
    ap = adapter_ptrs[i_hash];
    while (ap != NULL) {
	if (ap->devno == devno)
	    break;
	ap = ap->next;
    }	/* endwhile */

    if ((ap == NULL) || (!ap->inited) || (!ap->opened)) {
	ret_code = EIO;	/* error--adapter not inited */
	unlockl(&(hsc_lock));	/* release the lock */
	goto end;
    }

    /* lock the adapter lock to serialize these per adapter */
    rc = lockl(&(ap->ap_lock), LOCK_SHORT);	/* serialize this */
    if (rc != LOCK_SUCC) {
	ret_code = EIO;	/* error--kernel service call failed */
	unlockl(&(hsc_lock));	/* release the lock */
	goto end;
    }

    unlockl(&(hsc_lock));	/* release the global lock */


    /* validate ioctl operation and process it */
    switch (op) {

      case IOCINFO:	/* get device information */

	scinfo.devtype = DD_BUS;	/* say this is a bus device */
	scinfo.flags = 0;
	scinfo.devsubtype = DS_SCSI;	/* SCSI adapter device */
	scinfo.un.scsi.card_scsi_id = (char) ap->ddi.card_scsi_id;
	scinfo.un.scsi.max_transfer = ap->maxxfer;

	if (!(devflag & DKERNEL)) {	/* for a user process */
	    rc = copyout(&scinfo, (char *) arg, sizeof(struct devinfo));
	    if (rc != 0)
		ret_code = EFAULT;
	}
	else {	/* for a kernel process */
	    /* s, d, l */
	    bcopy(&scinfo, (char *) arg, sizeof(struct devinfo));
	}
	break;

      case SCIOSTART:	/* allocate resources for device */
	ret_code = hsc_alloc_dev(ap, INDEX(arg >> 8, arg), devflag);
	break;

      case SCIOEVENT:	/* register devices async event notification func */
	ret_code = hsc_register_async_func(ap, arg, devflag);
	break;

      case SCIOSTOP:	/* deallocate resources for device */
	ret_code = hsc_dealloc_dev(ap, INDEX(arg >> 8, arg), devflag);
	break;

      case SCIOHALT:	/* abort device, wait for resume */
	ret_code = hsc_halt_lun(ap, INDEX(arg >> 8, arg), devflag);
	break;

      case SCIORESET:	/* reset this SCSI device (BDR msg) */
	ret_code = hsc_reset_dev(ap, INDEX(arg >> 8, arg), devflag);
	break;

      case SCIOINQU:	/* issue a SCSI device inquiry cmd */
	ret_code = hsc_inquiry(ap, devno, arg, devflag);
	break;

      case SCIOSTUNIT:	/* issue a SCSI device start unit cmd */
	ret_code = hsc_startunit(ap, devno, arg, devflag);
	break;

      case SCIOTUR:	/* issue a SCSI device test unit ready cmd */
	ret_code = hsc_testunitready(ap, devno, arg, devflag);
	break;

      case SCIOREAD:	/* issue a SCSI read cmd (6-byte) */
	ret_code = hsc_readblk(ap, devno, arg, devflag);
	break;

      case SCIODIAG:	/* run adapter diagnostics commands */
	ret_code = hsc_diagnostic(ap, arg, devflag);
	break;

      case SCIOTRAM:	/* no-op; no adapter ram to test */
	ret_code = ENXIO;
	break;

      case SCIODNLD:	/* download microcode to adapter */
	ret_code = hsc_download(ap, arg, devflag);
	break;

      case SCIOSTARTTGT:	/* allocate resources for device */
	ret_code = hsc_alloc_tgt(ap, arg, devflag);
	break;

      case SCIOSTOPTGT:	/* deallocate resources for device */
	ret_code = hsc_dealloc_tgt(ap, arg, devflag);
	break;

      case SCIOGTHW:	/* returns 0 if gathered write is supported */
	ret_code = 0;
	break;


      /* private undocumented ioctl to allow users to call future ioctls */
      /* directly, bypassing the need for the ioctl kernel services      */
      case 901:
        if (!(devflag & DKERNEL)) {     /* for a user process */
            rc = copyout(&addr, (char *) arg, sizeof(addr));
            if (rc != 0)
                ret_code = EFAULT;
        }
        else {  /* for a kernel process */
            /* s, d, l */
            bcopy(&addr, (char *) arg, sizeof(addr));
        }
        break;

      default:	/* catch unknown ioctls here */
	ret_code = EINVAL;
	break;

    }	/* end switch */

    unlockl(&(ap->ap_lock));	/* release the adapter lock */

end:
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_IOCTL, ret_code, devno);
    return (ret_code);

}  /* end hsc_ioctl */

/************************************************************************/
/*                                                                      */
/* NAME:        hsc_register_async_func                                 */
/*                                                                      */
/* FUNCTION:    Routine to register a devices async event notification  */
/*		function.						*/
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called from the process level.         */
/*	Only KERNEL processes can call this function			*/
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*	dev_info    - device info structure. one per device.		*/
/*	sc_event_info  - structure passed by caller			*/
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      arg     - passed argument value                                 */
/*      devflag - device flag                                           */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0  for successful completion                      */
/*                  = EINVAL if device was not opened.                  */
/*                  = EINVAL if device already registered for async.    */
/*                  = EACCES if adapter not in normal mode.             */
/*                  = EFAULT if bad copy.                               */
/*                  = EFAULT if not called in kernel mode.              */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      copyin          copyout                                         */
/*                                                                      */
/************************************************************************/
int
hsc_register_async_func(
			struct adapter_def * ap,
			int arg,
			ulong devflag)
{
    int     dev_index;
    struct dev_info *dev;
    struct sc_event_struct sce;


    if (!(devflag & DKERNEL)) {	/* if user process */
	return (EPERM);
    }

    /* make sure adapter is not in diag mode */
    if (ap->adapter_mode != NORMAL_MODE) {
	return (EACCES);	/* wrong adapter mode */
    }

    /* handle kernel process	 */

    bcopy((char *) arg, &sce, sizeof(struct sc_event_struct));

    if (sce.mode == SC_TM_MODE)
	dev_index = sce.id + IMDEVICES;
    else
	dev_index = INDEX(sce.id, sce.lun);
    dev = &ap->dev[dev_index];
    if (dev->opened != TRUE) {	/* not opened ? */
	return (EINVAL);
    }
    if (dev->async_func != NULL) {	/* already registered ? */
	return (EINVAL);
    }
    dev->async_correlator = sce.async_correlator;
    dev->async_func = sce.async_func;
    return (0);
}

/************************************************************************/
/*                                                                      */
/* NAME:        hsc_inquiry                                             */
/*                                                                      */
/* FUNCTION:    Routine to Issue a SCSI Inquiry command to device.      */
/*                                                                      */
/*      This internal routine performs actions required to send a       */
/*      SCSI Inquiry command to the selected SCSI LUN.                  */
/*                                                                      */
/* NOTE:                                                                */
/*      Due to the use of the SC_RESUME flag for errors caused by       */
/*      normal use of the device, as well as by errors caused running   */
/*      this command, it is NOT possible to run this command while      */
/*      the device is being run normally.  The caller must not execute  */
/*      this ioctl if the SCIOSTART failed to this device (indicating   */
/*      this device is already started for another process).  If it     */
/*      is desired to get the function of this ioctl while a device is  */
/*      running, the caller must use the pass-thru ioctl of the head    */
/*      device driver and send the equivalent command through it.       */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called from the process level.         */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      sc_inquiry - structure used to pass parameters to the inquiry   */
/*                   ioctl                                              */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      devno   - passed device major/minor number                      */
/*      arg     - passed argument value                                 */
/*      devflag - device flag                                           */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0  for successful completion                      */
/*                  = EIO if a permanent I/O error occurs.              */
/*                  = ENODEV if device could not be selected.           */
/*                  = EINVAL if device was not opened.                  */
/*                  = EACCES if adapter not in normal mode.             */
/*                  = EFAULT if bad copy.                               */
/*                  = ENOMEM if cannot get required memory.             */
/*                  = ETIMEDOUT if command timed-out.                   */
/*                                                                      */
/* NOTE:                                                                */
/*      If either EIO or ETIMEDOUT are returned, the caller should      */
/*      retry this command at least once, as the first attempt may      */
/*      have cleared a hardware error condition.                        */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      copyin          copyout                                         */
/*                                                                      */
/************************************************************************/
int
hsc_inquiry(
	    struct adapter_def * ap,
	    dev_t devno,
	    int arg,
	    ulong devflag)
{
    int     rc, ret_code;
    int     dev_index;
    struct sc_buf *ptr;
    struct sc_inquiry sci;
    int     actual_length, temp_length;

    ret_code = 0;	/* set default return code */

    if (!(devflag & DKERNEL)) {	/* handle user process */
	rc = copyin((char *) arg, &sci, sizeof(struct sc_inquiry));
	if (rc != 0) {
	    ret_code = EFAULT;
	    goto end;
	}
    }
    else {	/* handle kernel process */
	bcopy((char *) arg, &sci, sizeof(struct sc_inquiry));	/* s, d, l */
    }

    dev_index = INDEX(sci.scsi_id, sci.lun_id);
    if (!ap->dev[dev_index].opened) {
	ret_code = EINVAL;	/* device not opened */
	goto end;
    }

    /* make sure adapter is not in diag mode */
    if (ap->adapter_mode != NORMAL_MODE) {
	ret_code = EACCES;	/* wrong adapter mode */
	goto end;
    }

    ptr = hsc_bld_sc_buf();
    if (ptr == NULL) {
	ret_code = ENOMEM;
	goto end;
    }

    ptr->scsi_command.scsi_id = sci.scsi_id;
    ptr->scsi_command.scsi_length = 6;
    ptr->scsi_command.scsi_cmd.scsi_op_code = SCSI_INQUIRY;
    if (sci.get_extended) {
        ptr->scsi_command.scsi_cmd.lun = (sci.lun_id << 5) | 0x01;
        ptr->scsi_command.scsi_cmd.scsi_bytes[0] = sci.code_page_num;
    }
    else {
        ptr->scsi_command.scsi_cmd.lun = sci.lun_id << 5;
        ptr->scsi_command.scsi_cmd.scsi_bytes[0] = 0;
    }

    ptr->scsi_command.scsi_cmd.scsi_bytes[1] = 0;
    ptr->scsi_command.scsi_cmd.scsi_bytes[2] = 255;	/* max count */
    ptr->scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    /* insure that the command works by always setting no-disc flag */
    /* set ASYNC flag according to flag in input struct */
    ptr->scsi_command.flags = (sci.flags & (SC_ASYNC | SC_NODISC));

    ptr->bufstruct.b_bcount = 255;
    ptr->bufstruct.b_flags |= B_READ;
    ptr->bufstruct.b_dev = devno;

    ptr->timeout_value = 15;	/* set timeout value */

    /* set resume flag in case caller is retrying this operation */
    /* this assumes the inquiry is only running single-threaded  */
    /* to this device. set delay flag in case this is a device   */
    /* which requires delay after reset occurs.                  */
    ptr->flags = SC_RESUME | SC_DELAY_CMD;

    if (hsc_strategy(ptr)) {	/* run the command, if good rc */
	hsc_release(ptr);	/* release buffer */
	ret_code = EIO;
	goto end;
    }

    hsc_ioctl_sleep((struct buf *) ptr);        /* wait for completion */

    /* now check return code from strategy, intr,... */
    /* need to generate: EIO, ENODEV, ETIMEDOUT      */
    /* caller should be retrying EIO and ETIMEDOUT   */
    /* since the first try may clear a hdw error     */
    if (ptr->bufstruct.b_flags & B_ERROR) {
	if (ptr->status_validity & SC_ADAPTER_ERROR) {
	    if (ptr->general_card_status & SC_CMD_TIMEOUT) {
		ret_code = ETIMEDOUT;
		hsc_logerr(ap, ERRID_SCSI_ERR10, NULL, COMMAND_TIMEOUT,
			   52, 0);
	    }
	    else
		if (ptr->general_card_status & SC_NO_DEVICE_RESPONSE)
		    ret_code = ENODEV;
		else
		    if (ptr->general_card_status & SC_SCSI_BUS_FAULT)
			ret_code = ENOCONNECT;
		    else
			ret_code = EIO;
	}
	else
	    ret_code = EIO;
    }

    /* if no other errors, and yet no data came back, then fail */
    if ((ret_code == 0) &&
	(ptr->bufstruct.b_resid == ptr->bufstruct.b_bcount))
	ret_code = EIO;

    /* if no errors detected */
    if (ret_code == 0) {
	/* give the caller the lesser of what he asked for, or */
	/* the actual transfer length                          */
	temp_length = sci.inquiry_len;
	actual_length = ptr->bufstruct.b_bcount - ptr->bufstruct.b_resid;
	if (temp_length > actual_length)
	    temp_length = actual_length;
	if (!(devflag & DKERNEL)) {	/* handle user process */
	    rc = copyout(ptr->bufstruct.b_un.b_addr,
			 sci.inquiry_ptr,
			 temp_length);
	    if (rc != 0)
		ret_code = EFAULT;
	}
	else {	/* handle kernel process */
	    /* s, d, l */
	    bcopy(ptr->bufstruct.b_un.b_addr, sci.inquiry_ptr, temp_length);
	}
    }

    hsc_release(ptr);	/* release memory for the sc_buf, etc. */

end:
    return (ret_code);

}  /* end hsc_inquiry */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_readblk                                             */
/*                                                                      */
/* FUNCTION:    Routine to Issue a SCSI Read command (6-byte) to device.*/
/*                                                                      */
/*      This internal routine performs actions required to send a       */
/*      SCSI Read command to the selected SCSI LUN.                     */
/*                                                                      */
/* NOTE:                                                                */
/*      Due to the use of the SC_RESUME flag for errors caused by       */
/*      normal use of the device, as well as by errors caused running   */
/*      this command, it is NOT possible to run this command while      */
/*      the device is being run normally.  The caller must not execute  */
/*      this ioctl if the SCIOSTART failed to this device (indicating   */
/*      this device is already started for another process).  If it     */
/*      is desired to get the function of this ioctl while a device is  */
/*      running, the caller must use the pass-thru ioctl of the head    */
/*      device driver and send the equivalent command through it.       */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called from the process level.         */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      sc_readblk - structure used to pass parameters to the read      */
/*                   ioctl                                              */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      devno   - passed device major/minor number                      */
/*      arg     - passed argument value                                 */
/*      devflag - device flag                                           */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0  for successful completion                      */
/*                  = EIO if a permanent I/O error occurs.              */
/*                  = ENODEV if device could not be selected.           */
/*                  = EINVAL if device was not opened.                  */
/*                           if transfer too long to handle.            */
/*                  = EACCES if adapter not in normal mode.             */
/*                  = EFAULT if bad copy.                               */
/*                  = ENOMEM if cannot get required memory.             */
/*                  = ETIMEDOUT if command timed-out.                   */
/*                                                                      */
/* NOTE:                                                                */
/*      If either EIO or ETIMEDOUT are returned, the caller should      */
/*      retry this command at least once, as the first attempt may      */
/*      have cleared a hardware error condition.                        */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      copyin          copyout                                         */
/*      xmalloc         xmfree                                          */
/*      bcopy                                                           */
/*                                                                      */
/************************************************************************/
int
hsc_readblk(
	    struct adapter_def * ap,
	    dev_t devno,
	    int arg,
	    ulong devflag)
{
    int     rc, ret_code;
    int     dev_index;
    struct sc_buf *ptr;
    struct sc_readblk sci;
    int     actual_length, temp_length;

    ret_code = 0;	/* set default return code */

    if (!(devflag & DKERNEL)) {	/* handle user process */
	rc = copyin((char *) arg, &sci, sizeof(struct sc_readblk));
	if (rc != 0) {
	    ret_code = EFAULT;
	    goto end;
	}
    }
    else {	/* handle kernel process */
	bcopy((char *) arg, &sci, sizeof(struct sc_readblk));	/* s, d, l */
    }

    dev_index = INDEX(sci.scsi_id, sci.lun_id);
    if (!ap->dev[dev_index].opened) {
	ret_code = EINVAL;	/* device not opened */
	goto end;
    }

    /* make sure adapter is not in diag mode */
    if (ap->adapter_mode != NORMAL_MODE) {
	ret_code = EACCES;	/* wrong adapter mode */
	goto end;
    }

    /* validate that transfer size will fit within a page   */
    if ((int) sci.blklen > PAGESIZE) {
	ret_code = EINVAL;	/* xfer too long */
	goto end;
    }

    ptr = hsc_bld_sc_buf();
    if (ptr == NULL) {
	ret_code = ENOMEM;
	goto end;
    }

    /* malloc area to be used for data transfer (cannot use */
    /* page used for sc_buf as this transfer will be >256)  */
    ptr->bufstruct.b_un.b_addr =
	(char *) xmalloc((uint) PAGESIZE, (uint) PGSHIFT,
			 pinned_heap);
    if (ptr->bufstruct.b_un.b_addr == NULL) {
	/* xmalloc failed--return error */
	hsc_release(ptr);	/* release sc_buf */
	ret_code = ENOMEM;
	goto end;
    }


    ptr->scsi_command.scsi_id = sci.scsi_id;
    ptr->scsi_command.scsi_length = 6;
    ptr->scsi_command.scsi_cmd.scsi_op_code = SCSI_READ;
    ptr->scsi_command.scsi_cmd.lun =
	(sci.lun_id << 5) | ((sci.blkno >> 16) & 0x1f);
    ptr->scsi_command.scsi_cmd.scsi_bytes[0] =
	((sci.blkno >> 8) & 0xff);
    ptr->scsi_command.scsi_cmd.scsi_bytes[1] =
	(sci.blkno & 0xff);
    ptr->scsi_command.scsi_cmd.scsi_bytes[2] = 1;	/* single blk */
    ptr->scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    /* set ASYNC and NODISC flag according to flag in input struct */
    ptr->scsi_command.flags = sci.flags;

    ptr->bufstruct.b_bcount = (unsigned) sci.blklen;
    ptr->bufstruct.b_flags |= B_READ;
    ptr->bufstruct.b_dev = devno;

    ptr->timeout_value = sci.timeout_value;	/* set timeout value */

    /* set resume flag in case caller is retrying this operation */
    /* this assumes the read is only running single-threaded     */
    /* to this device. set delay flag in case this is a device   */
    /* which requires delay after reset occurs.                  */
    ptr->flags = SC_RESUME | SC_DELAY_CMD;

    if (hsc_strategy(ptr)) {	/* run the command, if any rc */
	(void) xmfree((void *) ptr->bufstruct.b_un.b_addr, pinned_heap);
	hsc_release(ptr);	/* release buffer */
	ret_code = EIO;
	goto end;
    }

    hsc_ioctl_sleep((struct buf *) ptr);        /* wait for completion */

    /* now check return code from strategy, intr,... */
    /* need to generate: EIO, ENODEV, ETIMEDOUT      */
    /* caller should be retrying EIO and ETIMEDOUT   */
    /* since the first try may clear a hdw error     */
    if (ptr->bufstruct.b_flags & B_ERROR) {
	if (ptr->status_validity & SC_ADAPTER_ERROR) {
	    if (ptr->general_card_status & SC_CMD_TIMEOUT) {
		ret_code = ETIMEDOUT;
		hsc_logerr(ap, ERRID_SCSI_ERR10, NULL, COMMAND_TIMEOUT,
			   70, 0);
	    }
	    else
		if (ptr->general_card_status & SC_NO_DEVICE_RESPONSE)
		    ret_code = ENODEV;
		else
		    if (ptr->general_card_status & SC_SCSI_BUS_FAULT)
			ret_code = ENOCONNECT;
		    else
			ret_code = EIO;
	}
	else
	    ret_code = EIO;
    }

    /* if no other errors, and yet there is a resid count--fail */
    if ((ret_code == 0) && (ptr->bufstruct.b_resid))
	ret_code = EIO;

    /* if no errors detected, return the caller's data. */
    if (ret_code == 0) {

	if (!(devflag & DKERNEL)) {	/* handle user process */
	    rc = copyout(ptr->bufstruct.b_un.b_addr,
			 sci.data_ptr,
			 sci.blklen);
	    if (rc != 0)
		ret_code = EFAULT;
	}
	else {	/* handle kernel process */
	    /* s, d, l */
	    bcopy(ptr->bufstruct.b_un.b_addr, sci.data_ptr, sci.blklen);
	}
    }

    (void) xmfree((void *) ptr->bufstruct.b_un.b_addr, pinned_heap);
    hsc_release(ptr);	/* release memory for the sc_buf */

end:
    return (ret_code);

}  /* end hsc_readblk */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_startunit                                           */
/*                                                                      */
/* FUNCTION:    Routine to Issue a SCSI Start Unit command to device.   */
/*                                                                      */
/*      This internal routine performs actions required to send a       */
/*      SCSI Start Unit command to the selected SCSI and LUN ID.        */
/*                                                                      */
/* NOTE:                                                                */
/*      Due to the use of the SC_RESUME flag for errors caused by       */
/*      normal use of the device, as well as by errors caused running   */
/*      this command, it is NOT possible to run this command while      */
/*      the device is being run normally.  The caller must not execute  */
/*      this ioctl if the SCIOSTART failed to this device (indicating   */
/*      this device is already started for another process).  If it     */
/*      is desired to get the function of this ioctl while a device is  */
/*      running, the caller must use the pass-thru ioctl of the head    */
/*      device driver and send the equivalent command through it.       */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called from the process level.         */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      sc_startunit - structure used to pass parameters to the start   */
/*                     unit ioctl                                       */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      devno   - passed device major/minor number                      */
/*      arg     - passed argument value                                 */
/*      devflag - device flag                                           */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0  for successful completion                      */
/*                  = EIO if a permanent I/O error occurs.              */
/*                  = ENODEV if device could not be selected.           */
/*                  = EINVAL if device was not opened.                  */
/*                  = EACCES if adapter not in normal mode.             */
/*                  = EFAULT if bad copy.                               */
/*                  = ENOMEM if cannot get required memory.             */
/*                  = ETIMEDOUT if command timed-out.                   */
/*                                                                      */
/* NOTE:                                                                */
/*      If either EIO or ETIMEDOUT are returned, the caller should      */
/*      retry this command at least once, as the first attempt may      */
/*      have cleared a hardware error condition.                        */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      copyin                                                          */
/*                                                                      */
/************************************************************************/
int
hsc_startunit(
	      struct adapter_def * ap,
	      dev_t devno,
	      int arg,
	      ulong devflag)
{
    int     rc, ret_code;
    int     dev_index;
    struct sc_buf *ptr;
    struct sc_startunit sci;

    ret_code = 0;	/* set default return code */

    if (!(devflag & DKERNEL)) {	/* handle user process */
	rc = copyin((char *) arg, &sci, sizeof(struct sc_startunit));
	if (rc != 0) {
	    ret_code = EFAULT;
	    goto end;
	}
    }
    else {	/* handle kernel process */
	bcopy((char *) arg, &sci, sizeof(struct sc_startunit));	/* s, d, l */
    }


    dev_index = INDEX(sci.scsi_id, sci.lun_id);
    if (!ap->dev[dev_index].opened) {
	ret_code = EINVAL;	/* device not opened */
	goto end;
    }

    /* make sure adapter is not in diag mode */
    if (ap->adapter_mode != NORMAL_MODE) {
	ret_code = EACCES;	/* wrong adapter mode */
	goto end;
    }

    ptr = hsc_bld_sc_buf();
    if (ptr == NULL) {
	ret_code = ENOMEM;
	goto end;
    }

    ptr->scsi_command.scsi_id = sci.scsi_id;
    ptr->scsi_command.scsi_length = 6;
    ptr->scsi_command.scsi_cmd.scsi_op_code = SCSI_START_STOP_UNIT;
    ptr->scsi_command.scsi_cmd.lun =	/* set immed bit here */
	(sci.lun_id << 5) | (sci.immed_flag ? 0x01 : 0);
    ptr->scsi_command.scsi_cmd.scsi_bytes[0] = 0;
    ptr->scsi_command.scsi_cmd.scsi_bytes[1] = 0;
    ptr->scsi_command.scsi_cmd.scsi_bytes[2] =	/* set start option */
	(sci.start_flag ? 0x01 : 0);
    ptr->scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    ptr->bufstruct.b_bcount = 0;
    ptr->bufstruct.b_dev = devno;

    ptr->timeout_value = sci.timeout_value;	/* set timeout value */

    /* do not set the no-disc flag for this command */
    /* set ASYNC flag according to flag in input struct */
    ptr->scsi_command.flags = (sci.flags & SC_ASYNC);

    /* set resume flag in case caller is retrying this operation */
    /* this assumes the command is only running single-threaded  */
    /* to this device. set delay flag in case this is a device   */
    /* which requires delay after reset occurs.                  */
    ptr->flags = SC_RESUME | SC_DELAY_CMD;

    if (hsc_strategy(ptr)) {	/* run the command, if good rc */
	hsc_release(ptr);	/* release buffer */
	ret_code = EIO;
	goto end;
    }

    hsc_ioctl_sleep((struct buf *) ptr);        /* wait for completion */

    /* now check return code from strategy, intr,... */
    /* need to generate: EIO, ENODEV, ETIMEDOUT      */
    /* caller should be retrying EIO and ETIMEDOUT   */
    /* since the first try may clear a hdw error     */
    if (ptr->bufstruct.b_flags & B_ERROR) {
	if (ptr->status_validity & SC_ADAPTER_ERROR) {
	    if (ptr->general_card_status & SC_CMD_TIMEOUT) {
		ret_code = ETIMEDOUT;
		hsc_logerr(ap, ERRID_SCSI_ERR10, NULL, COMMAND_TIMEOUT,
			   62, 0);
	    }
	    else
		if (ptr->general_card_status & SC_NO_DEVICE_RESPONSE)
		    ret_code = ENODEV;
		else
		    if (ptr->general_card_status & SC_SCSI_BUS_FAULT)
			ret_code = ENOCONNECT;
		    else
			ret_code = EIO;
	}
	else
	    ret_code = EIO;
    }

    hsc_release(ptr);	/* release memory for the sc_buf, etc. */

end:
    return (ret_code);

}  /* end hsc_startunit */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_testunitready                                       */
/*                                                                      */
/* FUNCTION:    Routine to Issue a SCSI Test Unit Ready command.        */
/*                                                                      */
/*      This internal routine performs actions required to send a       */
/*      SCSI Test Unit Ready cmd to the selected SCSI and LUN ID.       */
/*                                                                      */
/* NOTE:                                                                */
/*      Due to the use of the SC_RESUME flag for errors caused by       */
/*      normal use of the device, as well as by errors caused running   */
/*      this command, it is NOT possible to run this command while      */
/*      the device is being run normally.  The caller must not execute  */
/*      this ioctl if the SCIOSTART failed to this device (indicating   */
/*      this device is already started for another process).  If it     */
/*      is desired to get the function of this ioctl while a device is  */
/*      running, the caller must use the pass-thru ioctl of the head    */
/*      device driver and send the equivalent command through it.       */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called from the process level.         */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      sc_ready - structure used to pass parameters to the test unit   */
/*                 ready ioctl                                          */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      devno   - passed device major/minor number                      */
/*      arg     - passed argument value                                 */
/*      devflag - device flag                                           */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0  for successful completion                      */
/*                  = EIO if a permanent I/O error occurs. The caller   */
/*                        should refer to the returned status bytes     */
/*                        to see if valid SCSI status was returned. If  */
/*                        so, the caller should refer to the attached   */
/*                        device's functional specification to see what */
/*                        action should be taken based on the SCSI      */
/*                        status value.                                 */
/*                  = ENODEV if device could not be selected.           */
/*                  = EINVAL if device was not opened.                  */
/*                  = EACCES if adapter not in normal mode.             */
/*                  = EFAULT if bad copy.                               */
/*                  = ENOMEM if cannot get required memory.             */
/*                  = ETIMEDOUT if command timed-out.                   */
/*                                                                      */
/* NOTE:                                                                */
/*      If either EIO or ETIMEDOUT are returned, the caller should      */
/*      retry this command at least once, as the first attempt may      */
/*      have cleared a hardware error condition.                        */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      copyin          copyout                                         */
/*                                                                      */
/************************************************************************/
int
hsc_testunitready(
		  struct adapter_def * ap,
		  dev_t devno,
		  int arg,
		  ulong devflag)
{
    int     rc, ret_code;
    int     dev_index;
    struct sc_buf *ptr;
    struct sc_ready sci;

    ret_code = 0;	/* set default return code */

    if (!(devflag & DKERNEL)) {	/* handle user process */
	rc = copyin((char *) arg, &sci, sizeof(struct sc_ready));
	if (rc != 0) {
	    ret_code = EFAULT;
	    goto end;
	}
    }
    else {	/* handle kernel process */
	bcopy((char *) arg, &sci, sizeof(struct sc_ready));	/* s, d, l */
    }


    dev_index = INDEX(sci.scsi_id, sci.lun_id);
    if (!ap->dev[dev_index].opened) {
	ret_code = EINVAL;	/* device not opened */
	goto end;
    }

    /* make sure adapter is not in diag mode */
    if (ap->adapter_mode != NORMAL_MODE) {
	ret_code = EACCES;	/* wrong adapter mode */
	goto end;
    }

    ptr = hsc_bld_sc_buf();
    if (ptr == NULL) {
	ret_code = ENOMEM;
	goto end;
    }

    sci.status_validity = 0;	/* set default status */
    sci.scsi_status = 0;	/* set default status */

    ptr->scsi_command.scsi_id = sci.scsi_id;
    ptr->scsi_command.scsi_length = 6;
    ptr->scsi_command.scsi_cmd.scsi_op_code = SCSI_TEST_UNIT_READY;
    ptr->scsi_command.scsi_cmd.lun = sci.lun_id << 5;
    ptr->scsi_command.scsi_cmd.scsi_bytes[0] = 0;
    ptr->scsi_command.scsi_cmd.scsi_bytes[1] = 0;
    ptr->scsi_command.scsi_cmd.scsi_bytes[2] = 0;
    ptr->scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    ptr->bufstruct.b_bcount = 0;
    ptr->bufstruct.b_dev = devno;

    ptr->timeout_value = 15;	/* set timeout value */

    /* do not set the no-disc flag for this command */
    /* set ASYNC flag according to flag in input struct */
    ptr->scsi_command.flags = (sci.flags & SC_ASYNC);

    /* set resume flag in case caller is retrying this operation */
    /* this assumes the command is only running single-threaded  */
    /* to this device. set delay flag in case this is a device   */
    /* which requires delay after reset occurs.                  */
    ptr->flags = SC_RESUME | SC_DELAY_CMD;

    if (hsc_strategy(ptr)) {	/* run the command, if good rc */
	hsc_release(ptr);	/* release buffer */
	ret_code = EIO;
	goto end;
    }

    hsc_ioctl_sleep((struct buf *) ptr);        /* wait for completion */

    /* now check return code from strategy, intr,... */
    /* need to generate: EIO, ENODEV, ETIMEDOUT      */
    /* caller should be retrying EIO and ETIMEDOUT   */
    /* since the first try may clear a hdw error     */
    if (ptr->bufstruct.b_flags & B_ERROR) {
	if (ptr->status_validity & SC_ADAPTER_ERROR) {
	    if (ptr->general_card_status & SC_CMD_TIMEOUT) {
		ret_code = ETIMEDOUT;
		hsc_logerr(ap, ERRID_SCSI_ERR10, NULL, COMMAND_TIMEOUT,
			   63, 0);
	    }
	    else
		if (ptr->general_card_status & SC_NO_DEVICE_RESPONSE)
		    ret_code = ENODEV;
		else
		    if (ptr->general_card_status & SC_SCSI_BUS_FAULT)
			ret_code = ENOCONNECT;
		    else
			ret_code = EIO;
	}
	else
	    if (ptr->status_validity & SC_SCSI_ERROR) {
		ret_code = EIO;
		sci.status_validity = SC_SCSI_ERROR;	/* indicate error */
		sci.scsi_status = ptr->scsi_status;	/* copy scsi status */
	    }
    }

    /* give caller the updated status bytes */
    if (!(devflag & DKERNEL)) {	/* handle user process */
	rc = copyout(&sci, (char *) arg, sizeof(struct sc_ready));
	if (rc != 0)
	    ret_code = EFAULT;
    }
    else {	/* handle kernel process */
	bcopy(&sci, (char *) arg, sizeof(struct sc_ready));	/* s, d, l */
    }

    hsc_release(ptr);	/* release memory for the sc_buf, etc. */

end:
    return (ret_code);

}  /* end hsc_testunitready */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_diagnostic                                          */
/*                                                                      */
/* FUNCTION:    Routine to run adapter diagnostics.                     */
/*                                                                      */
/*      This internal routine provides several diagnostics options      */
/*      for the caller to run against the selected adapter.             */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called from the process level.         */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_card_diag - structure used to pass parameters to the diag-   */
/*                     nostic ioctl                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      arg     - pointer to passed sc_card_diag struct                 */
/*      devflag - device flag                                           */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0  for successful completion                      */
/*                  = EIO if a permanent I/O error occurs. The caller   */
/*                        should refer to the return code structure     */
/*                        for more detailed error information.          */
/*                  = EINVAL if an invalid sub-option specified.        */
/*                  = EACCES if adapter not in diag mode.               */
/*                  = EFAULT if bad copy.                               */
/*                           also, intermediate status on diag option.  */
/*                  = ENOMSG if diag option ended with previous error.  */
/*                  = ETIMEDOUT if command timed-out.                   */
/*                                                                      */
/* NOTE:                                                                */
/*      Refer to the Hardware Technical Reference for further infor-    */
/*      mation regarding FRU isolation based on the results of these    */
/*      diagnostics test.                                               */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      copyin          copyout                                         */
/*      bzero                                                           */
/*                                                                      */
/************************************************************************/
int
hsc_diagnostic(
	       struct adapter_def * ap,
	       int arg,
	       ulong devflag)
{
    int     rc, ret_code;
    struct sc_card_diag sci;

    ret_code = 0;	/* set default return code */

    if (!(devflag & DKERNEL)) {	/* handle user process */
	rc = copyin((char *) arg, &sci, sizeof(struct sc_card_diag));
	if (rc != 0) {
	    ret_code = EFAULT;
	    goto end;
	}
    }
    else {	/* handle kernel process */
	bcopy((char *) arg, &sci, sizeof(struct sc_card_diag));	/* s, d, l */
    }


    /* make sure adapter is in diag mode */
    if (ap->adapter_mode != DIAG_MODE) {
	ret_code = EACCES;	/* wrong adapter mode */
	goto end;
    }

    /* clear returned status area */
    bzero(&sci.diag_rc, sizeof(struct rc));

    /* clear adapter struct mb30 and mb31 status */
    bzero(&ap->mb30_resid, 2 * MB_STAT_SIZE);

    switch (sci.option) {

      case SC_CARD_DIAGNOSTICS:
	ret_code = hsc_diag_test(ap, &sci);
	break;

      case SC_RESUME_DIAGNOSTICS:
	ret_code = hsc_diag_test(ap, &sci);
	break;

      case SC_CARD_SCSI_WRAP:
	ret_code = hsc_wrap_test(ap, &sci);
	break;

      case SC_CARD_REGS_TEST:
	ret_code = hsc_reg_test(ap, &sci);
	break;

      case SC_CARD_POS_TEST:
	ret_code = hsc_pos_test(ap, &sci);
	break;

      case SC_SCSI_BUS_RESET:
	ret_code = hsc_reset_test(ap, &sci);
	break;

      default:
	ret_code = EINVAL;
	break;

    }	/* end switch */

    /* give caller the updated status bytes */
    if (!(devflag & DKERNEL)) {	/* handle user process */
	rc = copyout(&sci, (char *) arg, sizeof(struct sc_card_diag));
	if (rc != 0)
	    ret_code = EFAULT;
    }
    else {	/* handle kernel process */
	bcopy(&sci, (char *) arg, sizeof(struct sc_card_diag));	/* s, d, l */
    }

end:
    return (ret_code);

}  /* end hsc_diagnostic */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_diag_test                                           */
/*                                                                      */
/* FUNCTION:    Run card internal diagnostic test.                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called from the process level.         */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_card_diag - structure used to pass parameters to the diag-   */
/*                     nostic ioctl                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      ptr     - pointer to sc_card_diag structure from caller         */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0  for successful completion                      */
/*                  = EIO if a permanent I/O error occurs. The caller   */
/*                        should refer to the return code structure     */
/*                        for more detailed error information.          */
/*                  = EFAULT intermediate status on diag option         */
/*                  = ENOMSG if diag option ended with previous error   */
/*                  = ETIMEDOUT if the command did not complete         */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      disable_lock    unlock _enable                                  */
/*      e_sleep_thread         w_start                                  */
/*      bcopy           bzero                                           */
/*      pio_assist                                                      */
/*                                                                      */
/************************************************************************/
int
hsc_diag_test(
	      struct adapter_def * ap,
	      struct sc_card_diag * ptr)
{
    int     ret_code;
    uchar   trash;
    int     old_pri;
    struct io_parms iop;

    ret_code = 0;	/* set default return code */

    /* init iop structure */
    bzero(&iop, sizeof(struct io_parms));
    iop.ap = ap;

    /* must disable intrpts here because an intrpt could be */
    /* pending due to failed IPL diagnostics.  We don't want */
    /* to see that intrpt until first adap cmd is issued.   */
    old_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));

    /* enable card interrupts */
    iop.opt = INT_ENABLE;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	ptr->diag_rc.diag_validity = 0x01;
	ptr->diag_rc.ahs_validity = 0x01;
	ptr->diag_rc.ahs = iop.ahs;
	ptr->diag_rc.un.card1.mb_addr = iop.eff_addr;
	ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	ret_code = EIO;	/* indicate error */
	unlock_enable(old_pri, &(hsc_mp_lock));	/* unmask intrpts */
	goto end;
    }

    if (ptr->option == SC_CARD_DIAGNOSTICS)
	/* save diag sub-opt */
	ap->p_scsi_id = 0;
    else
	/* save resume sub-opt */
	ap->p_scsi_id = 1;

    hsc_build_mb30(ap, DIAGNOSTICS, ap->p_scsi_id, 0, 0);

    ap->proc_results = 0;
    ap->wdog.dog.restart = ptr->timeout_value + 1;
    w_start(&ap->wdog.dog);
    ap->MB30_in_use = PROC_USING;	/* flag proc level cmd using */

    /* send MB30 command to adapter */
    iop.opt = WRITE_MB30;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if ((pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) &&
	(iop.errtype != PIO_PERM_IOCC_ERR)) {
	/* handle unrecovered error on the pio operation        */
	/* if retries exhausted, and not an iocc internal error */
	/* then gracefully back-out, else, allow to either      */
	/* complete or timeout                                  */
	ptr->diag_rc.diag_validity = 0x01;
	ptr->diag_rc.ahs_validity = 0x01;
	ptr->diag_rc.ahs = iop.ahs;
	ptr->diag_rc.un.card1.mb_addr = iop.eff_addr;
	ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	/* disable card interrupts */
	iop.opt = INT_DISABLE;
	(void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);
	w_stop(&ap->wdog.dog);	/* stop cmd timer */
	ap->MB30_in_use = -1;	/* release mbox 30 */
	ret_code = EIO;	/* indicate error */
	unlock_enable(old_pri, &(hsc_mp_lock));	/* unmask intrpts */
	/* cmd not initiated, so it is alright to leave routine here */
	goto end;
    }

    e_sleep_thread(&ap->event, &(hsc_mp_lock), LOCK_HANDLER);
    unlock_enable(old_pri, &(hsc_mp_lock));	/* unmask intrpts */

    /* disable card interrupts */
    iop.opt = INT_DISABLE;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	ptr->diag_rc.diag_validity = 0x01;
	ptr->diag_rc.ahs_validity = 0x01;
	ptr->diag_rc.ahs = iop.ahs;
	ptr->diag_rc.un.card1.mb_addr = iop.eff_addr;
	ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	ret_code = EIO;	/* indicate error */
	goto end;
    }

    /* N.B. if an IPL diag failure occurred, control will pass */
    /* to the intrpt handler, which will set proc_results */
    /* and wakeup here.                                   */
    /* OR  a timeout will occur, also returning here.     */
    if ((ap->proc_results != GOOD_COMPLETION) && (ret_code == 0)) {
	if (ap->proc_results == FATAL_ERROR) {
	    /* copy mbox 30 and 31 status to return code structure */
	    bcopy(&ap->MB30p->mb.m_op_code, &ptr->diag_rc.un.card1.mbox[0],
		  MB_SIZE);
	    bcopy(&ap->mb30_resid, &ptr->diag_rc.un.card1.mbox[24],
		  2 * MB_STAT_SIZE);
	    ret_code = EIO;	/* indicate error */
	}
	else {
	    if (ap->proc_results == TIMED_OUT) {
		/* indicate the timeout */
		ptr->diag_rc.ahs_validity = 0x01;
		ptr->diag_rc.ahs = COMMAND_TIMEOUT;
		/* copy mbox 30 and 31 status to return code structure */
		bcopy(&ap->MB30p->mb.m_op_code,
		      &ptr->diag_rc.un.card1.mbox[0], MB_SIZE);
		bcopy(&ap->mb30_resid, &ptr->diag_rc.un.card1.mbox[24],
		      2 * MB_STAT_SIZE);
		ret_code = ETIMEDOUT;	/* indicate no intrpt status */
	    }
	    else {
		if (ap->mb30_rc == COMMAND_PAUSED) {
		    /* copy mbox 30 and 31 status to return code structure */
		    bcopy(&ap->MB30p->mb.m_op_code,
			  &ptr->diag_rc.un.card1.mbox[0], MB_SIZE);
		    bcopy(&ap->mb30_resid, &ptr->diag_rc.un.card1.mbox[24],
			  2 * MB_STAT_SIZE);
		    ret_code = EFAULT;	/* indicate resume needed */
		}
		else {
		    if (ap->mb30_rc == COMPLETE_WITH_ERRORS) {
			ret_code = ENOMSG;	/* indicate diag finished */
		    }
		    else {	/* here, invalid response from adapter */
			/* copy mbox 30 and 31 status to return code struct */
			bcopy(&ap->MB30p->mb.m_op_code,
			      &ptr->diag_rc.un.card1.mbox[0], MB_SIZE);
			bcopy(&ap->mb30_resid,
			      &ptr->diag_rc.un.card1.mbox[24],
			      2 * MB_STAT_SIZE);
			ret_code = EIO;	/* indicate error */
		    }
		}
	    }
	}
    }	/* good completion comes here */

end:
    ap->p_scsi_id = 0;	/* reset sub-opt flag */
    return (ret_code);

}  /* end hsc_diag_test */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_wrap_test                                           */
/*                                                                      */
/* FUNCTION:    Run card diagnostic wrap test.                          */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called from the process level.         */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_card_diag - structure used to pass parameters to the diag-   */
/*                     nostic ioctl                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      ptr     - pointer to sc_card_diag structure from caller         */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0  for successful completion                      */
/*                  = EIO if a permanent I/O error occurs. The caller   */
/*                        should refer to the return code structure     */
/*                        for more detailed error information.          */
/*                  = ETIMEDOUT if the command did not complete         */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      disable_lock    unlock _enable                                  */
/*      e_sleep_thread  w_start                                         */
/*      bcopy           bzero                                           */
/*      pio_assist                                                      */
/*                                                                      */
/************************************************************************/
int
hsc_wrap_test(
	      struct adapter_def * ap,
	      struct sc_card_diag * ptr)
{
    int     ret_code;
    uchar   trash;
    int     old_pri;
    struct io_parms iop;

    ret_code = 0;	/* set default return code */

    /* init iop structure */
    bzero(&iop, sizeof(struct io_parms));
    iop.ap = ap;

    /* must disable intrpts here because an intrpt could be */
    /* pending due to failed IPL diagnostics.  We don't want */
    /* to see that intrpt until first adap cmd is issued.   */
    old_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));

    /* enable card interrupts */
    iop.opt = INT_ENABLE;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	ptr->diag_rc.diag_validity = 0x01;
	ptr->diag_rc.ahs_validity = 0x01;
	ptr->diag_rc.ahs = iop.ahs;
	ptr->diag_rc.un.card1.mb_addr = iop.eff_addr;
	ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	ret_code = EIO;	/* indicate error */
	unlock_enable(old_pri, &(hsc_mp_lock));	/* unmask intrpts */
	goto end;
    }

    /* save wrap sub-opt */
    ap->p_scsi_id = 3;

    hsc_build_mb30(ap, DIAGNOSTICS, ap->p_scsi_id, 0, 0);

    ap->proc_results = 0;
    ap->wdog.dog.restart = ptr->timeout_value + 1;
    w_start(&ap->wdog.dog);
    ap->MB30_in_use = PROC_USING;	/* flag proc level cmd using */

    /* send MB30 command to adapter */
    iop.opt = WRITE_MB30;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if ((pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) &&
	(iop.errtype != PIO_PERM_IOCC_ERR)) {
	/* handle unrecovered error on the pio operation */
	/* if retries exhausted, and not an iocc internal error */
	/* then gracefully back-out, else, allow to either      */
	/* complete or timeout                                  */
	ptr->diag_rc.diag_validity = 0x01;
	ptr->diag_rc.ahs_validity = 0x01;
	ptr->diag_rc.ahs = iop.ahs;
	ptr->diag_rc.un.card1.mb_addr = iop.eff_addr;
	/* cmd not initiated, so it is alright to leave routine here */
	ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	/* disable card interrupts */
	iop.opt = INT_DISABLE;
	(void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);
	w_stop(&ap->wdog.dog);	/* stop cmd timer */
	ap->MB30_in_use = -1;	/* release mbox 30 */
	ret_code = EIO;	/* indicate error */
	unlock_enable(old_pri, &(hsc_mp_lock));	/* unmask intrpts */
	goto end;
    }

    e_sleep_thread(&ap->event, &(hsc_mp_lock), LOCK_HANDLER);
    unlock_enable(old_pri, &(hsc_mp_lock));	/* unmask intrpts */

    /* disable card interrupts */
    iop.opt = INT_DISABLE;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	ptr->diag_rc.diag_validity = 0x01;
	ptr->diag_rc.ahs_validity = 0x01;
	ptr->diag_rc.ahs = iop.ahs;
	ptr->diag_rc.un.card1.mb_addr = iop.eff_addr;
	ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	ret_code = EIO;	/* indicate error */
	goto end;
    }

    /* N.B. if an IPL diag failure occurred, control will pass */
    /* to the intrpt handler, which will set proc_results */
    /* and wakeup here.                                   */
    /* OR  a timeout will occur, also returning here.     */
    if ((ap->proc_results != GOOD_COMPLETION) && (ret_code == 0)) {
	if (ap->proc_results == FATAL_ERROR) {
	    /* copy mbox 30 and 31 status to return code structure */
	    bcopy(&ap->MB30p->mb.m_op_code, &ptr->diag_rc.un.card1.mbox[0],
		  MB_SIZE);
	    bcopy(&ap->mb30_resid, &ptr->diag_rc.un.card1.mbox[24],
		  2 * MB_STAT_SIZE);
	    ret_code = EIO;	/* indicate error */
	}
	else {
	    if (ap->proc_results == TIMED_OUT) {
		/* indicate the timeout */
		ptr->diag_rc.ahs_validity = 0x01;
		ptr->diag_rc.ahs = COMMAND_TIMEOUT;
		/* copy mbox 30 and 31 status to return code structure */
		bcopy(&ap->MB30p->mb.m_op_code,
		      &ptr->diag_rc.un.card1.mbox[0], MB_SIZE);
		bcopy(&ap->mb30_resid, &ptr->diag_rc.un.card1.mbox[24],
		      2 * MB_STAT_SIZE);
		ret_code = ETIMEDOUT;	/* indicate no intrpt status */
	    }
	    else {	/* here, either bad completion, or invalid response */
		/* copy mbox 30 and 31 status to return code structure */
		bcopy(&ap->MB30p->mb.m_op_code,
		      &ptr->diag_rc.un.card1.mbox[0], MB_SIZE);
		bcopy(&ap->mb30_resid,
		      &ptr->diag_rc.un.card1.mbox[24], 2 * MB_STAT_SIZE);
		ret_code = EIO;	/* indicate error */
	    }
	}
    }	/* good completion comes here */

end:
    ap->p_scsi_id = 0;	/* reset sub-opt flag */
    return (ret_code);

}  /* end hsc_wrap_test */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_reg_test                                            */
/*                                                                      */
/* FUNCTION:    Diagnostic Write/Read/Compare test on registers.        */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called from the process level.         */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_card_diag - structure used to pass parameters to the diag-   */
/*                     nostic ioctl                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      ptr     - pointer to sc_card_diag structure from caller         */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0  for successful completion                      */
/*                  = EIO if a permanent I/O error occurs. The caller   */
/*                        should refer to the return code structure     */
/*                        for more detailed error information.          */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      pio_assist      bzero                                           */
/*                                                                      */
/************************************************************************/
int
hsc_reg_test(
	     struct adapter_def * ap,
	     struct sc_card_diag * ptr)
{
    int     ret_code;
    uchar   trash;
    int     cmpr_err_flag, pio_err_flag;
    struct io_parms iop;

    ret_code = 0;	/* set default return code */
    cmpr_err_flag = FALSE;	/* set default error state */
    pio_err_flag = FALSE;	/* set default error state */

    /* init iop structure */
    bzero(&iop, sizeof(struct io_parms));
    iop.ap = ap;

    /* test 1, reset all bits, read BCR, compare to 0 */
    iop.opt = SI_RESET;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }
    iop.opt = SE_RESET;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }
    iop.opt = DMA_DISABLE;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }
    iop.opt = INT_DISABLE;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }
    iop.opt = READ_BCR;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.data = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }
    if ((iop.data & 0x0f) != 0x00) {
	cmpr_err_flag = TRUE;
	goto end;
    }


    /* test 2, set DMA ena, compare BCR */
    iop.opt = DMA_ENABLE;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }
    iop.opt = READ_BCR;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.data = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }
    if ((iop.data & 0x0f) != 0x08) {
	cmpr_err_flag = TRUE;
	goto end;
    }

    /* test 3, reset DMA ena, compare BCR */
    iop.opt = DMA_DISABLE;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }
    iop.opt = READ_BCR;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.data = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }
    if ((iop.data & 0x0f) != 0x00) {
	cmpr_err_flag = TRUE;
	goto end;
    }

end:
    if (cmpr_err_flag) {
	ptr->diag_rc.diag_validity = 0x01;
	ptr->diag_rc.diag_stat = SC_DIAG_MISCMPR;
	ptr->diag_rc.un.card1.mb_addr = ap->ddi.base_addr + BCR;
	return (EIO);	/* compare error */
    }

    if (pio_err_flag) {
	ptr->diag_rc.diag_validity = 0x01;
	ptr->diag_rc.ahs_validity = 0x01;
	ptr->diag_rc.ahs = iop.ahs;
	ptr->diag_rc.un.card1.mb_addr = iop.eff_addr;
	ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	return (EIO);	/* pio error */
    }

    return (ret_code);

}  /* end hsc_reg_test */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_pos_test                                            */
/*                                                                      */
/* FUNCTION:    Diagnostic Write/Read/Compare test on POS registers.    */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called from the process level.         */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_card_diag - structure used to pass parameters to the diag-   */
/*                     nostic ioctl                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      ptr     - pointer to sc_card_diag structure from caller         */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0  for successful completion                      */
/*                  = EIO if a permanent I/O error occurs. The caller   */
/*                        should refer to the return code structure     */
/*                        for more detailed error information.          */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
int
hsc_pos_test(
	     struct adapter_def * ap,
	     struct sc_card_diag * ptr)
{
    int     ret_code;
    uchar   data, trash;
    caddr_t iocc_addr;
    uint    err_flag, offset;

    ret_code = 0;	/* set default return code */
    err_flag = 0;	/* set default error state */

    iocc_addr = IOCC_ATT(ap->ddi.bus_id, 0);	/* access IOCC */

    /* disable the card enable bit during these accesses */
    BUSIO_PUTC((iocc_addr + (ap->ddi.slot << 16) + POS2), 0x00);

    /* test 1, read POS0 and POS1 and POS5 */
    data = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS0));
    if (data != POS0_VAL) {
	err_flag = TRUE;
	offset = POS0;
	goto end;
    }
    data = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS1));
    if (data != POS1_VAL) {
	err_flag = TRUE;
	offset = POS1;
	goto end;
    }
    data = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS5));
    if ((data & 0x80) != 0x80) {
	err_flag = TRUE;
	offset = POS5;
	goto end;
    }

    /* test 2, test POS7 */
    data = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS7));
    if ((data & 0x01) != 0x01) {
	BUSIO_PUTC((iocc_addr + (ap->ddi.slot << 16) + POS7), 0x01);
	data = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS7));
	if ((data & 0x01) != 0x01) {
	    err_flag = TRUE;
	    offset = POS7;
	    goto end;
	}
    }
    BUSIO_PUTC((iocc_addr + (ap->ddi.slot << 16) + POS7), 0x00);
    data = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS7));
    if ((data & 0x01) != 0x00) {
	err_flag = TRUE;
	offset = POS7;
	goto end;
    }

    /* test 2, test POS6 */
    data = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS6));
    if (data != 0xaa) {
	BUSIO_PUTC((iocc_addr + (ap->ddi.slot << 16) + POS6), 0xaa);
	data = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS6));
	if (data != 0xaa) {
	    err_flag = TRUE;
	    offset = POS6;
	    goto end;
	}
    }
    else {
	BUSIO_PUTC((iocc_addr + (ap->ddi.slot << 16) + POS6), 0x55);
	data = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS6));
	if (data != 0x55) {
	    err_flag = TRUE;
	    offset = POS6;
	    goto end;
	}
    }

    /* test 3, test POS4 */
    data = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS4));
    if (data != 0x15) {
	BUSIO_PUTC((iocc_addr + (ap->ddi.slot << 16) + POS4), 0x15);
	data = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS4));
	if (data != 0x15) {
	    err_flag = TRUE;
	    offset = POS4;
	    goto end;
	}
    }
    else {
	BUSIO_PUTC((iocc_addr + (ap->ddi.slot << 16) + POS4), 0x0a);
	data = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS4));
	if (data != 0x0a) {
	    err_flag = TRUE;
	    offset = POS4;
	    goto end;
	}
    }

    /* test 4, test POS2 (this leaves card disabled) */
    data = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS2));
    if (data != 0x54) {
	BUSIO_PUTC((iocc_addr + (ap->ddi.slot << 16) + POS2), 0x54);
	data = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS2));
	if (data != 0x54) {
	    err_flag = TRUE;
	    offset = POS2;
	    goto end;
	}
    }
    else {
	BUSIO_PUTC((iocc_addr + (ap->ddi.slot << 16) + POS2), 0x2a);
	data = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS2));
	if (data != 0x2a) {
	    err_flag = TRUE;
	    offset = POS2;
	    goto end;
	}
    }

end:
    (void) hsc_config_adapter(ap);	/* try to restore state of POS regs */
    IOCC_DET(iocc_addr);	/* release access to the IOCC */
    if (err_flag) {
	ptr->diag_rc.diag_validity = 0x01;
	ptr->diag_rc.diag_stat = SC_DIAG_MISCMPR;
	ptr->diag_rc.un.card1.mb_addr = (ap->ddi.slot << 16) + offset;
	ret_code = EIO;	/* data error */
    }

    return (ret_code);

}  /* end hsc_pos_test */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_reset_test                                          */
/*                                                                      */
/* FUNCTION:    Diagnostic SCSI Bus Reset Detection Test.               */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called from the process level.         */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_card_diag - structure used to pass parameters to the diag-   */
/*                     nostic ioctl                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      ptr     - pointer to sc_card_diag structure from caller         */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0  for successful completion                      */
/*                  = EIO if a permanent I/O error occurs. The caller   */
/*                        should refer to the return code structure     */
/*                        for more detailed error information.          */
/*                  = ETIMEDOUT if command did not complete.            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      bcopy           bzero                                           */
/*      delay           pio_assist                                      */
/*                                                                      */
/************************************************************************/
int
hsc_reset_test(
	       struct adapter_def * ap,
	       struct sc_card_diag * ptr)
{
    int     ret_code, pio_err_flag;
    int     i, ticks, intrpt_seen;
    uchar   trash;
    ulong   data;
    struct io_parms iop;

    ret_code = 0;	/* set default return code */
    pio_err_flag = FALSE;	/* set default error state */

    /* init iop structure */
    bzero(&iop, sizeof(struct io_parms));
    iop.ap = ap;

    /* reset all control bits */
    iop.opt = SI_RESET;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }
    iop.opt = SE_RESET;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }
    iop.opt = DMA_DISABLE;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }
    iop.opt = INT_DISABLE;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }

    /* first, check for any pending mbox 31 error status */
    iop.opt = READ_ISR;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.data = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }
    iop.data = WORD_REVERSE(iop.data);	/* swap bytes of ISR */
    if (iop.data & 0x80000000) {	/* if mb31 intrpt bit is set */
	/* get MB31 status area */
	iop.opt = RD_MBOX_STAT;
	iop.mbp = ap->MB31p;
	iop.errtype = 0;
	iop.ahs = 0;
	iop.eff_addr = 0;
	if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	    /* handle unrecovered error on the pio operation */
	    pio_err_flag = TRUE;
	    goto end;
	}

	/* if any fatal status, find it here */
	switch (ap->MB31p->mb.m_adapter_rc) {
	  case BAD_FUSE:
	    /* fall through to logic below ... */
	  case ADAP_FATAL:
	    /* fall through to logic below ... */
	  case COMMAND_PAUSED:
	    /* save mbox 31 status in the diag return code struct */
	    bcopy(&ap->MB31p->mb.m_resid,
		  &ptr->diag_rc.un.card1.mb31_status[0], MB_STAT_SIZE);
	    ret_code = EIO;
	    break;
	  default:	/* no fatal status */
	    break;
	}	/* end switch */

	/* clear pending status and continue */
	bzero(&ap->MB31p->mb.m_op_code, MB_SIZE);
	/* send MB31 to adapter */
	/* N.B. this re-enables MB31 for next error */
	iop.opt = WRITE_MB31;
	iop.mbp = ap->MB31p;
	iop.errtype = 0;
	iop.ahs = 0;
	iop.eff_addr = 0;
	if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	    /* handle unrecovered error on the pio operation */
	    pio_err_flag = TRUE;
	    goto end;
	}
	if (ret_code != 0)
	    goto end;	/* leave if bad previous status */

    }

    /* */
    /* test 1, test the SI (Internal Reset)              */
    /* */
    iop.opt = SI_SET;	/* set SI */
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }

    /* execute a delay loop for at least 25 microseconds */
    iop.opt = SYNC_DELAY;
    iop.data = 30;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }

    iop.opt = SI_RESET;	/* reset SI */
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }

    ticks = HZ / DELAY_DIVISOR;	/* generate number of ticks for delay call */
    if (ticks == 0)	/* make sure to use at least 1 tick */
	ticks = 1;
    intrpt_seen = FALSE;	/* default state of flag */
    /* start looping, looking for the mbox 31 interrupt to come in */
    for (i = 0; i < (DELAY_DIVISOR * SCSI_RESET_T_O); i++) {
	delay(ticks);	/* wait for a period of time */

	/* look for interrupt */
	iop.opt = READ_ISR;
	iop.errtype = 0;
	iop.ahs = 0;
	iop.data = 0;
	iop.eff_addr = 0;
	if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	    /* handle unrecovered error on the pio operation */
	    pio_err_flag = TRUE;
	    break;
	}
	iop.data = WORD_REVERSE(iop.data);	/* swap bytes of ISR */
	if (iop.data & 0x80000000) {	/* if mb31 error bit set */
	    intrpt_seen = TRUE;
	    break;
	}
    }	/* end for */

    if (!intrpt_seen) {	/* if mb31 intrpt bit not seen */
	bzero(&ap->MB31p->mb.m_op_code, MB_SIZE);
	/* send MB31 to adapter */
	/* N.B. this attempts to clear mailbox 31 */
	iop.opt = WRITE_MB31;
	iop.mbp = ap->MB31p;
	iop.errtype = 0;
	iop.ahs = 0;
	iop.eff_addr = 0;
	/* ignore error here as it would mask primary error */
	(void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);

	ptr->diag_rc.ahs_validity = 0x01;
	ptr->diag_rc.ahs = COMMAND_TIMEOUT;
	ret_code = ETIMEDOUT;	/* indicate no intrpt status */
	goto end;
    }

    /* get MB31 status area */
    iop.opt = RD_MBOX_STAT;
    iop.mbp = ap->MB31p;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }

    /* if bus reset not seen, indicate error */
    if (ap->MB31p->mb.m_adapter_rc != SCSI_BUS_RESET) {
	/* copy mbox 31 status area to diag return code struct */
	bcopy(&ap->MB31p->mb.m_resid, &ptr->diag_rc.un.card1.mb31_status[0],
	      MB_STAT_SIZE);
	ret_code = EIO;
	/* continue on below to reset mb31 */
    }

    bzero(&ap->MB31p->mb.m_op_code, MB_SIZE);
    /* send MB31 to adapter */
    /* N.B. this re-enables MB31 for next error intrpt */
    iop.opt = WRITE_MB31;
    iop.mbp = ap->MB31p;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if ((pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) &&
	(ret_code == 0)) {
	/* handle unrecovered error on the pio operation, if no prev error */
	pio_err_flag = TRUE;
	goto end;
    }

    if (ret_code != 0)	/* this catches error of not seeing bus reset, above */
	goto end;	/* and also catches timeout and PIO err during loop  */

    /* */
    /* test 2, test the SE (Internal and External Reset) */
    /* */
    iop.opt = SE_SET;	/* set SE */
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }

    /* execute a delay loop for at least 25 microseconds */
    iop.opt = SYNC_DELAY;
    iop.data = 30;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }

    iop.opt = SE_RESET;	/* reset SE */
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }

    ticks = HZ / DELAY_DIVISOR;	/* generate number of ticks for delay call */
    if (ticks == 0)	/* make sure to use at least 1 tick */
	ticks = 1;
    intrpt_seen = FALSE;	/* default state of flag */
    /* start looping, looking for the mbox 31 interrupt to come in */
    for (i = 0; i < (DELAY_DIVISOR * SCSI_RESET_T_O); i++) {
	delay(ticks);	/* wait for a period of time */

	/* look for interrupt */
	iop.opt = READ_ISR;
	iop.errtype = 0;
	iop.ahs = 0;
	iop.data = 0;
	iop.eff_addr = 0;
	if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	    /* handle unrecovered error on the pio operation */
	    pio_err_flag = TRUE;
	    break;
	}
	iop.data = WORD_REVERSE(iop.data);	/* swap bytes of ISR */
	if (iop.data & 0x80000000) {	/* if mb31 error bit set */
	    intrpt_seen = TRUE;
	    break;
	}
    }	/* end for */

    if (!intrpt_seen) {	/* if mb31 intrpt bit not seen */
	bzero(&ap->MB31p->mb.m_op_code, MB_SIZE);
	/* send MB31 to adapter */
	/* N.B. this attempts to clear mailbox 31 */
	iop.opt = WRITE_MB31;
	iop.mbp = ap->MB31p;
	iop.errtype = 0;
	iop.ahs = 0;
	iop.eff_addr = 0;
	/* ignore error here as it would mask primary error */
	(void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);

	ptr->diag_rc.ahs_validity = 0x01;
	ptr->diag_rc.ahs = COMMAND_TIMEOUT;
	ret_code = ETIMEDOUT;	/* indicate no intrpt status */
	goto end;
    }

    /* get MB31 status area */
    iop.opt = RD_MBOX_STAT;
    iop.mbp = ap->MB31p;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* handle unrecovered error on the pio operation */
	pio_err_flag = TRUE;
	goto end;
    }

    /* if bus reset not seen, indicate error */
    if (ap->MB31p->mb.m_adapter_rc != SCSI_BUS_RESET) {
	/* copy mbox 31 status area to diag return code struct */
	bcopy(&ap->MB31p->mb.m_resid, &ptr->diag_rc.un.card1.mb31_status[0],
	      MB_STAT_SIZE);
	ret_code = EIO;
	/* continue on below to reset mb31 */
    }

    bzero(&ap->MB31p->mb.m_op_code, MB_SIZE);
    /* send MB31 to adapter */
    /* N.B. this re-enables MB31 for next error intrpt */
    iop.opt = WRITE_MB31;
    iop.mbp = ap->MB31p;
    iop.errtype = 0;
    iop.ahs = 0;
    iop.eff_addr = 0;
    if ((pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) &&
	(ret_code == 0)) {
	/* handle unrecovered error on the pio operation, if no prev error */
	pio_err_flag = TRUE;
	goto end;
    }

    if (ret_code != 0)	/* this catches error of not seeing bus reset, above */
	goto end;

end:
    if (pio_err_flag) {
	ptr->diag_rc.diag_validity = 0x01;
	ptr->diag_rc.ahs_validity = 0x01;
	ptr->diag_rc.ahs = iop.ahs;
	ptr->diag_rc.un.card1.mb_addr = iop.eff_addr;
	ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	ret_code = EIO;	/* indicate pio error */
    }

    return (ret_code);

}  /* end hsc_reset_test */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_download                                            */
/*                                                                      */
/* FUNCTION:    Download adapter microcode routine.                     */
/*                                                                      */
/*      This internal routine performs actions required to download     */
/*      a microcode file to the adapter.                                */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called from the process level.         */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_download - structure used to pass parameters to the download */
/*                    ioctl                                             */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      arg     - passed argument value                                 */
/*      devflag - device flag                                           */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0  for successful completion                      */
/*                  = EIO if a permanent I/O error occurs.              */
/*                  = EINVAL if a parameter is in error.                */
/*                  = EACCES if adapter not in diagnostic mode.         */
/*                  = EFAULT if bad copy, also given for severe         */
/*                           I/O errors during the download.            */
/*                  = ENOMEM if no memory available.                    */
/*                  = ETIMEDOUT if command timed-out.                   */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      disable_lock    unlock _enable                                  */
/*      w_start         w_stop                                          */
/*      copyin          copyout                                         */
/*      e_sleep_thread  pio_assist                                      */
/*      xmalloc         xmfree                                          */
/*      bzero           bcopy                                           */
/*                                                                      */
/************************************************************************/
int
hsc_download(
	     struct adapter_def * ap,
	     int arg,
	     ulong devflag)
{
    int     rc, ret_code;
    struct sc_download dnld;
    struct io_parms iop;
    struct xmem dp;
    int     old_pri;
    uchar   trash, scsi_cmds_active;
    char   *local_bufp, *tmp_ptr;
    int     local_retries, tmp_len, t_index, segflag;
    short   pinflag;

    ret_code = 0;	/* set default return code */

    /* enable error logging if opened in diagnostic mode */
    if (ap->adapter_mode == DIAG_MODE) {
	ap->errlog_enable = TRUE;
    }

    /* init iop structure */
    bzero(&iop, sizeof(struct io_parms));
    iop.ap = ap;	/* init adap pointer */

    if (!(devflag & DKERNEL)) {	/* handle user process */
	/* set the flag for pinu to indicated user space */
	pinflag = UIO_USERSPACE;
	/* set the segment flag to indicate to xmattach the data space */
	segflag = USER_ADSPACE;
	rc = copyin((char *) arg, &dnld, sizeof(struct sc_download));
	if (rc != 0) {
	    ret_code = EFAULT;
	    goto end;
	}
    }
    else {	/* handle kernel process */
	/* set the flag for pinu to indicated system space */
	pinflag = UIO_SYSSPACE;
	/* set the segment flag to indicate to xmattach the data space */
	segflag = SYS_ADSPACE;
	bcopy((char *) arg, &dnld, sizeof(struct sc_download));	/* s, d, l */
    }

    if (dnld.option == SC_VERSION_NUMBER) {	/* if this is a version only
						   request */
        old_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));
	/* set waiting state */
	ap->proc_waiting = WAIT_TO_SEND_DNLD_VERSION;
	if (ap->MB30_in_use == -1) {	/* MB30 free ?? */
	    /* build the mbox 30 command to get version */
	    hsc_build_mb30(ap, DOWNLOAD, 1, 0, 0);

	    if (ap->adapter_mode == DIAG_MODE) {
		/* enable card interrupts */
		iop.opt = INT_ENABLE;
		if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
		    /* handle unrecovered error while enabling intrpts */
		    ret_code = EIO;	/* indicate error */
                    unlock_enable(old_pri, &(hsc_mp_lock));/* unmask intrpts */
		    ap->proc_waiting = 0;
		    goto end;
		}
	    }

	    ap->wdog.dog.restart = ADAP_CMD_T_O;
	    w_start(&ap->wdog.dog);
	    ap->MB30_in_use = PROC_USING;	/* flag proc level cmd using */
	    /* set waiting state */
	    ap->proc_waiting = WAIT_FOR_DNLD_VERSION;

	    ap->proc_results = 0;
	    /* send MB30 command to adapter */
	    iop.opt = WRITE_MB30;
	    iop.errtype = 0;
	    if ((pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) &&
		(iop.errtype != PIO_PERM_IOCC_ERR)) {
		/* handle unrecovered error sending mbox 30.  */
		if (ap->adapter_mode == DIAG_MODE) {
		    iop.opt = INT_DISABLE;	/* disable intrpts */
		    (void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);
		}
		w_stop(&ap->wdog.dog);
		ap->MB30_in_use = -1;	/* release mbox 30 */
		ap->proc_waiting = 0;
		ret_code = EIO;	/* indicate error */
                unlock_enable(old_pri, &(hsc_mp_lock));/* unmask intrpts */
		goto end;
	    }
	}
	else {
	    ap->waiting_for_mb30 = TRUE;
	}

        e_sleep_thread(&ap->event, &(hsc_mp_lock), LOCK_HANDLER);
	/* N.B. if an IPL diag failure occurred, control will pass */
	/* to the intrpt handler, which will log an error, set */
	/* proc_results, and return to this point.            */
	/* OR  a timeout will occur, also returning here.     */
	if (ap->proc_results != GOOD_COMPLETION) {
	    /* error--handle bad adapter */
	    if (ap->proc_results == SEE_RC_STAT) {
		/* unknown card status--log adap error */
		hsc_logerr(ap, ERRID_SCSI_ERR3, ap->MB30p,
			   UNKNOWN_CARD_ERR, 53, 0);
		ret_code = EIO;	/* signal fatal error */
	    }
	    else {
		if (ap->proc_results == TIMED_OUT) {
		    ret_code = ETIMEDOUT;	/* flag time-out */
		}
		else {	/* other fatal errors come here */
		    ret_code = EIO;	/* flag fatal error */
		}
	    }
	}
	else {	/* handle good completion here */
	    /* copy version number out to caller */
	    dnld.version_number = (uint) ap->mb30_byte30;
	    if (!(devflag & DKERNEL)) {	/* handle user process */
		rc = copyout(&dnld,	/* source */
			     (char *) arg,	/* dest   */
			     sizeof(struct sc_download));	/* count  */
		if (rc != 0)
		    ret_code = EFAULT;
	    }
	    else {	/* handle kernel process */
		/* s, d, l */
		bcopy(&dnld, (char *) arg, sizeof(struct sc_download));
	    }
	}

        unlock_enable(old_pri, &(hsc_mp_lock));/* unmask intrpts */
	if (ap->adapter_mode == DIAG_MODE) {
	    /* disable card interrupts */
	    iop.opt = INT_DISABLE;
	    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
		/* handle unrecovered error while disabling intrpts */
		if (ret_code == 0)	/* if no error already */
		    ret_code = EIO;	/* indicate error */
	    }
	}

    }
    else {

	if (dnld.option == SC_DOWNLOAD) {	/* download request */
	    /* validate input parameters */
	    /* microcode length has to be less than 130KB, and on 1KB bound */
	    if ((dnld.microcode_len > 130048) ||
                (ap->num_tm_devices) ||
		(dnld.microcode_len & (MC_BLK_SIZE - 1))) {
		ret_code = EINVAL;	/* bad input parameter */
		goto end;
	    }
	    /* initialize cross memory descriptor structure */
	    bzero(&dp, sizeof(struct xmem));
	    dp.aspace_id = XMEM_INVAL;

	    /* xmattach is used to obtain a cross memory descriptor which */
	    /* will allows us to access the data space containing the     */
	    /* adapater microcode while the adapter is quiecsed.          */
	    rc = xmattach(dnld.microcode_ptr, dnld.microcode_len, &dp,
			  segflag);

	    if (rc != XMEM_SUCC) {
		ret_code = EFAULT;
		goto end;
	    }

	    /* xmalloc for the microcode download must be done before the */
	    /* adapter is quiesced to allow a page fault to occur */
	    local_bufp = (char *) xmalloc((uint) MC_BLK_SIZE,
					  4, kernel_heap);
	    /* if got buffer then pin it */
	    if ((!(local_bufp)) || (pin((caddr_t) local_bufp,
					(int) MC_BLK_SIZE))) {
		/* either malloc or pin failed */
		if (local_bufp) {
		    /* pinned must have failed */
		    (void) xmfree((void *) local_bufp, kernel_heap);
		}
		/* detach microcode buffer */
		(void) xmdetach(&dp);
		ret_code = ENOMEM;	/* indicate failure */
		goto end;
	    }
            /* pin the microcode file to avoid a page fault */
	    rc = pinu((caddr_t) dnld.microcode_ptr, dnld.microcode_len,
	               pinflag);
            /* recover from failed pinu */
            if (rc) {
	       /* unpin and free the xmalloc data space */
	       (void) unpin((caddr_t) local_bufp, (int) MC_BLK_SIZE);
	       (void) xmfree((void *) local_bufp, kernel_heap);
	       /* detach microcode buffer */
	       (void) xmdetach(&dp);
	       ret_code = rc;
	       goto end;
	    }
 
	    if (ap->adapter_mode == NORMAL_MODE) {

		/* set the adapter download in progress flag to prevent */
		/* any commands from being sent to the adapter          */
		ap->download_pending = DOWNLOAD_IN_PROGRESS;

		while (TRUE) {
		    scsi_cmds_active = FALSE;	/* default to say no cmds
						   active */

		    /* loop through each device to see if there are any cmds */
		    /* active at the adapter */
		    for (t_index = 0; t_index < IMDEVICES; t_index++)   {
			/* if a device has a cmd active then set flag */
			if (ap->dev[t_index].head_act != NULL) {
			    scsi_cmds_active = TRUE;
			    break;
			}
		    }	/* end for */

		    /* if no cmds active then break and continue donwload */
		    if (!(scsi_cmds_active))
			break;

		    /* else cmds active so delay and check again */
		    delay(HZ / 10);
		}	/* end while */

	    }	/* end if open in normal mode */

            /* at this point all commands to the adapter have completed */
            /* and no commands are being sent.  Although we are running */
            /* on the process level, since the adapter is quiecesed and */
            /* this could be the paging device adapter, no page faults  */
            /* can be taken until the download is complete and the      */
            /* adapter queues are restarted                             */


	    /* disable interrupts */
            old_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));

	    /* set waiting state */
	    ap->proc_waiting = WAIT_TO_SEND_DNLD_CMD;

	    if (ap->MB30_in_use == -1) {	/* MB30 free ?? */

		/* build the mbox 30 command to do download */
		hsc_build_mb30(ap, DOWNLOAD, 0,
			       (dnld.microcode_len / MC_BLK_SIZE), 0);


		if (ap->adapter_mode == DIAG_MODE) {
		    /* enable card interrupts */
		    iop.opt = INT_ENABLE;
		    if (pio_assist(&iop, hsc_pio_function,
				   hsc_pio_recov) == EIO) {
			/* handle unrecovered error while enabling intrpts */
			ret_code = EIO;	/* indicate error */
			ap->proc_waiting = 0;
                        unlock_enable(old_pri, &(hsc_mp_lock));
			/* unpin and free the xmalloc data space */
			(void) unpin((caddr_t) local_bufp, (int) MC_BLK_SIZE);
			(void) xmfree((void *) local_bufp, kernel_heap);
			/* upinu the microcode file */
			(void) unpinu((caddr_t) dnld.microcode_ptr,
				      dnld.microcode_len, pinflag);
			/* detach microcode buffer */
			(void) xmdetach(&dp);
                         /* NOTE : no need to start device queues because we */
                         /* are open in diag mode so the queues were never   */
                         /* halted                                           */
			goto end;
		    }
		}

		ap->wdog.dog.restart = ADAP_CMD_T_O;
		w_start(&ap->wdog.dog);
		/* set waiting state */
		ap->proc_waiting = WAIT_FOR_DNLD_CMD;
		ap->MB30_in_use = PROC_USING;	/* flag proc level cmd using */
		ap->proc_results = 0;
		/* send MB30 command to adapter */
		iop.opt = WRITE_MB30;
		iop.errtype = 0;
		if ((pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO)
		    && (iop.errtype != PIO_PERM_IOCC_ERR)) {
		    /* handle unrecovered error sending mbox 30.  */
		    if (ap->adapter_mode == DIAG_MODE) {
			iop.opt = INT_DISABLE;
			(void) pio_assist(&iop, hsc_pio_function,
					  hsc_pio_recov);
		    }
		    w_stop(&ap->wdog.dog);
		    ap->MB30_in_use = -1;	/* release mbox 30 */
		    ret_code = EIO;	/* indicate error */
		    ap->proc_waiting = 0;
		    ap->download_pending = 0;
                    unlock_enable(old_pri, &(hsc_mp_lock));
		    /* unpin and free the xmalloc data space */
		    (void) unpin((caddr_t) local_bufp, (int) MC_BLK_SIZE);
		    (void) xmfree((void *) local_bufp, kernel_heap);
		    /* upinu the microcode file */
		    (void) unpinu((caddr_t) dnld.microcode_ptr,
				  dnld.microcode_len, pinflag);
		    /* detach microcode buffer */
		    (void) xmdetach(&dp);
		    if (ap->adapter_mode == NORMAL_MODE) {
                       /* here we must re-start all of the device queues   */
                       /* for this adapter which were halted to facilitate */
                       /* the download command.                            */
                       hsc_dnld_start_devs(ap);
                    } 
		    goto end;
		}
	    }	/* end if MB30 free */

	    else {
                /* set the microcode length field of the adapter structure */
                /* so that the value can be know from the interrupt level  */
                /* where the command will be initiated                     */
                ap->download_mc_len = dnld.microcode_len;
		ap->waiting_for_mb30 = TRUE;
	    }

            e_sleep_thread(&ap->event, &(hsc_mp_lock), LOCK_HANDLER);
            unlock_enable(old_pri, &(hsc_mp_lock));	/* unmask intrpts */
	    /* N.B. if an IPL diag failure occurred, control will pass */
	    /* to the intrpt handler, which will log an error, set     */
	    /* proc_results, and return to this point.                 */
	    /* OR  a timeout will occur, also returning here.          */
	    if (ap->proc_results != SEE_RC_STAT) {
		/* error--handle bad adapter */
		if (ap->proc_results == TIMED_OUT) {
		    ret_code = ETIMEDOUT;	/* flag time-out */
		}
		else {
		    if (ap->proc_results == FATAL_ERROR) {
			ret_code = EIO;	/* flag fatal error */
		    }
		    else {	/* other fatal errors come here */
			/* log invalid adapter software response here */
			hsc_logerr(ap, ERRID_SCSI_ERR3, ap->MB30p,
				   UNKNOWN_CARD_ERR, 54, 0);
			ret_code = EFAULT;	/* flag severe error */
		    }
		}
	    }
	    else {	/* see what the adapter response was here */
		if ((ap->mb30_rc != COMMAND_PAUSED) ||
		    (ap->mb30_extra_stat != READY_FOR_NEXT_BLK)) {
		    hsc_logerr(ap, ERRID_SCSI_ERR3, ap->MB30p,
			       UNKNOWN_CARD_ERR, 55, 0);
		    ret_code = EFAULT;	/* flag severe error */
		}
		else {
		    /* */
		    /* normal path download gets here */
		    /* */
		    local_retries = 0;	/* set up starting retry count */

		    for (tmp_len = dnld.microcode_len,
			 tmp_ptr = dnld.microcode_ptr; tmp_len > 0;) {

                        /* copy the block of data to local kernel space */
		        rc = xmemin(tmp_ptr, local_bufp, MC_BLK_SIZE, &dp);
		        if (rc != 0) {
		            ret_code = EFAULT;
			    break;
		        }

                        old_pri = disable_lock(ap->ddi.int_prior, 
                                               &(hsc_mp_lock));
			ap->wdog.dog.restart = ADAP_CMD_T_O;
			w_start(&ap->wdog.dog);
			/* flag proc level cmd using */
			ap->MB30_in_use = PROC_USING;
			ap->proc_results = 0;
			/* send the block to the adapter */
			iop.opt = WRITE_MC_BLOCK;
			iop.data = MC_BLK_SIZE;
			iop.sptr = (caddr_t) local_bufp;
			iop.mbp = (struct mbstruct *) (&ap->MB[0].id0);
			iop.errtype = 0;
			if ((pio_assist(&iop, hsc_pio_function,
					hsc_pio_recov) == EIO) &&
			    (iop.errtype != PIO_PERM_IOCC_ERR)) {
			    /* handle unrecovered error writing data to */
			    /* card.  note: this will potentially leave */
			    /* the card hung on this block              */
			    w_stop(&ap->wdog.dog);
			    ap->MB30_in_use = -1;	/* release mbox 30 */
			    ret_code = EFAULT;	/* flag severe err */
                            unlock_enable(old_pri, &(hsc_mp_lock));
			    break;
			}


                        e_sleep_thread(&ap->event, &(hsc_mp_lock), 
                                       LOCK_HANDLER);
                        unlock_enable(old_pri, &(hsc_mp_lock));	

			/* here, check for the various return codes */
			if (ap->proc_results != SEE_RC_STAT) {
			    if (ap->proc_results == GOOD_COMPLETION) {
				if (tmp_len != MC_BLK_SIZE) {
				    hsc_logerr(ap, ERRID_SCSI_ERR3,
					       ap->MB30p,
					       UNKNOWN_CARD_ERR, 56, 0);
				    /* flag severe error */
				    ret_code = EFAULT;
				    break;
				}
				else {	/* normal completion gets here */
				    /* decr. counter */
				    tmp_len -= MC_BLK_SIZE;
				    /* incr. pointer */
				    tmp_ptr += MC_BLK_SIZE;
				}
			    }
			    else {
				if (ap->proc_results == TIMED_OUT) {
				    /* flag time-out */
				    ret_code = ETIMEDOUT;
				    break;
				}
				else {	/* other fatal errors come here */
				    ret_code = EIO;	/* flag fatal error */
				    break;
				}
			    }
			}
			else {	/* see what the adapter response was here */
			    if ((ap->mb30_rc == COMPLETE_WITH_ERRORS) &&
				(ap->mb30_extra_stat == CHECKSUM_ERROR)) {
				hsc_logerr(ap, ERRID_SCSI_ERR2,
					   ap->MB30p, PIO_WR_DATA_ERR,
					   57, 0);
				ret_code = EIO;	/* indicate failure */
				break;
			    }
			    else {
				if (ap->mb30_rc != COMMAND_PAUSED) {
				    hsc_logerr(ap, ERRID_SCSI_ERR3,
					       ap->MB30p,
					       UNKNOWN_CARD_ERR, 58, 0);
				    /* flag severe error */
				    ret_code = EFAULT;
				    break;
				}
				else {	/* command is paused */
				    if (ap->mb30_extra_stat !=
					READY_FOR_NEXT_BLK)
					if (ap->mb30_extra_stat !=
					    RESEND_LAST_BLK) {
					    hsc_logerr(ap,
						       ERRID_SCSI_ERR3,
						       ap->MB30p,
						       UNKNOWN_CARD_ERR,
						       59, 0);
					    /* flag severe error */
					    ret_code = EFAULT;
					    break;
					}
					else {	/* resend last block */
					    if (local_retries >
						MAX_MB30_RETRIES) {
						/* log the permanent error */
						hsc_logerr(ap,
							   ERRID_SCSI_ERR1,
							   ap->MB30p,
							   PIO_WR_DATA_ERR,
							   60, 0);
						/* flag severe error */
						ret_code = EFAULT;
						break;
					    }
					    else {	/* retry last block */
						/* log the temporary error */
						hsc_logerr(ap,
							   ERRID_SCSI_ERR2,
							   ap->MB30p,
							   PIO_WR_DATA_ERR,
							   61, 0);
						/* inc count */
						local_retries++;
						/* skip increment of loop so
						   block is resent */
					    }
					}
				    else {	/* ready for next block */
					/* zero retries */
					local_retries = 0;

					/* decr. counter */
					tmp_len -= MC_BLK_SIZE;
					/* incr. pointer */
					tmp_ptr += MC_BLK_SIZE;
				    }
				}
			    }
			}

		    }	/* endfor */

		}
	    }
            /* clean up */
	    /* unpin and free the xmalloc data space */
	    (void) unpin((caddr_t) local_bufp, (int) MC_BLK_SIZE);
	    (void) xmfree((void *) local_bufp, kernel_heap);
	    /* upinu the microcode file */
	    (void) unpinu((caddr_t) dnld.microcode_ptr,
			  dnld.microcode_len, pinflag);
	    /* detach microcode buffer */
	    (void) xmdetach(&dp);




/* At this point the download has either succeeded or a failure occured  */
/* while trying.  In either case a restart command will be issued to the */
/* adapter so that it knows what mailbox to expect next.  For the case   */
/* that we got here and a failure has previously occured (ret_code !=0)  */
/* the adapter may not respond properly to the restart.  Normal error    */
/* recovery will handle that case.  MB30 has been held in use on the     */
/* process level during the download.  MB30 will be freed when the       */
/* interrupt or timeout occurs for the restart command.                  */

             old_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));
            ap->wdog.dog.restart = ADAP_CMD_T_O;
            w_start(&ap->wdog.dog);
            /* set waiting state */
            ap->MB30_in_use = PROC_USING;	/* flag proc level cmd using */
            ap->proc_waiting = WAIT_FOR_RESTART;
            ap->proc_results = 0;
            hsc_build_mb30(ap, RESTART, 0, 0, 0);

            /* send MB30 command to adapter */
            iop.opt = WRITE_MB30;
            iop.errtype = 0;
	    if ((pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO)
	        && (iop.errtype != PIO_PERM_IOCC_ERR)) {
                ap->proc_waiting = 0;
                w_stop(&ap->wdog.dog);
                ap->MB30_in_use = -1;
                unlock_enable(old_pri, &(hsc_mp_lock));
                /* NOTE : the dowload may have succeeded but we still return */
                /* error indication to the calling process                   */
	        if (ret_code == 0)	/* if no error already */
	            ret_code = EIO;	/* indicate error */
            }
            else { /* pio of MB30 succeeded so wait for completion */
                e_sleep_thread(&ap->event, &(hsc_mp_lock), LOCK_HANDLER);
                unlock_enable(old_pri, &(hsc_mp_lock));	
	        /* N.B. if an error  occurrs, control will pass            */
	        /* to the intrpt handler, which will log an error, set     */
	        /* proc_results, and return to this point.                 */
	        /* OR  a timeout will occur, also returning here.          */
 
                /* in the case that the restart command completed with     */
                /* error, an error log entry will be made but ret_code     */
                /* will not be changed.  If a previous error occured it    */
                /* will be returned, if no previous error occurred, then   */
                /* the user will get no indication of failure.  Since the  */
                /* download did succeed and it is only the restart which   */
                /* failed, the user should be given an indication that the */
                /* download occured (ret_code = 0).  The only consequence  */
                /* of the case where the download succeeded and the restart*/
                /* failed is that the next command to the adapter will     */ 
                /* receive a sequence error and this will be recovered in  */
                /* the interrupt handler.                                  */
                if (ap->proc_results != GOOD_COMPLETION) {
                    if (ap->proc_results == SEE_RC_STAT) {
                    /* unknown card status--log adap error */
                        hsc_logerr(ap, ERRID_SCSI_ERR4, ap->MB30p,
                                   UNKNOWN_CARD_ERR, 96, 0);
                    }
                }
 
            } /* end else */

	    /* disable interrupts */
             old_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));

	    /* set state flag for this command */
	    ap->proc_waiting = WAIT_TO_SEND_SET_ID;

	    if (ap->MB30_in_use != -1) {	/* if mb30 in use */
		ap->waiting_for_mb30 = TRUE;
                e_sleep_thread(&ap->event, &(hsc_mp_lock), LOCK_HANDLER);
                unlock_enable(old_pri, &(hsc_mp_lock));	
	    }
	    else {	/* mb30 is free */

		/* load MB30 with "Set SCSI ID cmd" */
		ap->proc_results = 0;
		hsc_build_mb30(ap, SET_SCSI_ID, 0, 0, 0);

		/* need watchdog because adapter could be bad.          */
		/* make timeout long enough to cover still running diag. */
		ap->wdog.dog.restart = ADAP_CMD_T_O;
		w_start(&ap->wdog.dog);
		ap->MB30_in_use = PROC_USING;	/* flag proc level cmd using */
		ap->proc_waiting = WAIT_FOR_SET_ID;	/* set state flag for
							   cmd */

		/* send MB30 command to adapter */
		iop.opt = WRITE_MB30;
		iop.errtype = 0;
		if ((pio_assist(&iop, hsc_pio_function, hsc_pio_recov)
		     == EIO) &&
		    (iop.errtype != PIO_PERM_IOCC_ERR)) {
		    ap->proc_waiting = 0;	/* reset state flag */
		    ap->MB30_in_use = -1;	/* free mb30 */
		    w_stop(&ap->wdog.dog);
                    unlock_enable(old_pri, &(hsc_mp_lock));
	            if (ret_code == 0)	/* if no error already */
	                ret_code = EIO;	/* indicate error */
		}
		else {
                    e_sleep_thread(&ap->event, &(hsc_mp_lock), LOCK_HANDLER);
                    unlock_enable(old_pri, &(hsc_mp_lock));	
		}
	    }

	    /* N.B. if an IPL diag failure occurred, control will pass */
	    /* to the intrpt handler, which will log an error, set     */
	    /* proc_results, and return to this point.                 */
	    /* OR  a timeout will occur, also returning here.          */
	    if (ap->proc_results != GOOD_COMPLETION) {
		/* error--handle bad adapter */
		if (ap->proc_results == SEE_RC_STAT) {
		    /* unknown card status--log adap error */
		    hsc_logerr(ap, ERRID_SCSI_ERR3, ap->MB30p,
			       UNKNOWN_CARD_ERR, 97, 0);
		}
	        if (ret_code == 0)	/* if no error already */
	            ret_code = EIO;	/* indicate error */

	    }	/* MB30 command was successful */


            /* set flag to indicate download is finished */
            ap->download_pending = 0;

	    if (ap->adapter_mode == DIAG_MODE) {
	       /* disable card interrupts */
	       iop.opt = INT_DISABLE;
	       if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	           /* handle unrecovered error while disabling intrpts */
	        if (ret_code == 0)	/* if no error already */
	            ret_code = EIO;	/* indicate error */
	        }
	    } /* end if open DIAG_MODE */
            else {   /* open in normal mode */
               hsc_dnld_start_devs(ap);
            }
        }
	else {
            if (dnld.option == SC_ENABLE_CMD_Q) {
                if(!ap->ddi.cmd_queue) {   /* this adapter does not support */
                                           /* cmd tag queuing, fail the req */
                    ret_code = EINVAL;
                    goto end;
                }
                else {
                    if (!ap->enable_queuing) {
                        hsc_enable_cmd_q(ap);
                    }
                }
            }
            else { /* bad option field value */
                ret_code = EINVAL;
                goto end;
            }
	}
    }

end:
    if (ap->adapter_mode == DIAG_MODE) {
        ap->errlog_enable = FALSE;	/* disable error logging */
    }
    return (ret_code);

}  /* end hsc_download */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_enable_cmd_q                                        */
/*                                                                      */
/* FUNCTION:    Enable the command tag queuing function of the adapter  */
/*                                                                      */
/*      This routine will quiese the SCSI bus for this adapter, modify  */
/*      the adapter structure to enable command tag queuing and then    */
/*      restart all devices.  Quiescing is necessary so that untagged   */
/*      and tagged commands are not mixed at the adapter.               */
/*      This function should only be called if this adapter supports    */
/*      command tag queuing.                                            */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called on the process level.           */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap - pointer to adapter definition structure                    */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      no values are returned                                          */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*                                                                      */
/************************************************************************/

void
hsc_enable_cmd_q(struct adapter_def * ap)

{

    int scsi_cmds_active,           /* used to quiesce the SCSI bus         */
        t_index,                    /* dev_index used to check all devices  */
        old_pri;                    /* used to disable interrupts           */

    if (ap->adapter_mode == NORMAL_MODE) {

        /* set the adapter download in progress flag to prevent */
        /* any commands from being sent to the adapter          */
        ap->download_pending = DOWNLOAD_IN_PROGRESS;

        while (TRUE) {
            scsi_cmds_active = FALSE;   /* default to say no cmds
                                           active */

            /* loop through each device to see if there are any cmds */
            /* active at the adapter */
            for (t_index = 0; t_index < IMDEVICES; t_index++)   {
                /* if a device has a cmd active then set flag */
                if (ap->dev[t_index].head_act != NULL) {
                    scsi_cmds_active = TRUE;
                    break;
                }
            }   /* end for */

            /* if no cmds active then break and continue donwload */
            if (!(scsi_cmds_active))
                break;

            /* else cmds active so delay and check again */
            delay(HZ / 10);
        }       /* end while */


        /* at this point all commands to the adapter have completed */
        /* and no commands are being sent.  Although we are running */
        /* on the process level, since the adapter is quiecesed and */
        /* this could be the paging device adapter, no page faults  */
        /* can be taken until the device queues are restared.       */

        /* mark the adapter as queuing enabled */
        ap->enable_queuing = TRUE;

        /* loop through all opened devices and modify the queue depth of  */
        /* every device so that queuing will be enabled */

        /* disable interrupts because the hsc_start will be called to restart */
        /* the quiesced SCSI bus */
        old_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));
        /* clear the state which quiesced the SCSI bus */
        ap->download_pending = 0;

        for (t_index = 0; t_index < IMDEVICES; t_index++)   {

            if (ap->dev[t_index].opened) {
                ap->dev[t_index].queue_depth = Q_ENABLED;
                hsc_start(ap,t_index);
            }
        } /* end for */

        unlock_enable(old_pri, &(hsc_mp_lock));
    }   /* end if open in normal mode */

    else {   /* diagnostic mode open */
        /* mark the adapter as queuing enabled */
        ap->enable_queuing = TRUE;
    }

} /* end hsc_enable_cmd_q */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_dnld_start_devs                                     */
/*                                                                      */
/* FUNCTION:    Start all device queues for a given adapter             */
/*                                                                      */
/*      This routine calls the MB30 interrupt handler to initiate any   */
/*      MB30 commands which may be pending.  It then checks all devices */
/*      on this adapter and calls hsc_start() to resume processing of   */
/*      commands to the devices which are open on this adapter.         */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called on the process level.           */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      ap - pointer to adapter definition structure                    */
/*                                                                      */
/* INPUTS:                                                              */
/*      none                                                            */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      no values are returned                                          */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*                                                                      */
/************************************************************************/

void
hsc_dnld_start_devs(struct adapter_def * ap)

{
   int old_pri, t_index;

   /* disable interrupts while calling MB30 interrupt handler and       */
   /* hsc_start() */
   old_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));
  
   hsc_MB30_handler(ap, FALSE);
   for (t_index = 0; t_index < IMDEVICES; t_index++)   {
       if (ap->dev[t_index].opened) {
           /* device is open so call hsc_start to restart queues */
           hsc_start(ap,t_index);
       }
   } /* end for */
   unlock_enable(old_pri, &(hsc_mp_lock));

}

/************************************************************************/
/*                                                                      */
/* NAME:        hsc_bld_sc_buf                                          */
/*                                                                      */
/* FUNCTION:    Build an sc_buf for an Internal Command.                */
/*                                                                      */
/*      This routine builds an sc_buf structure for use when sending    */
/*      internal commands (typically through ioctls).                   */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called on the process level.           */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*                                                                      */
/* INPUTS:                                                              */
/*      none                                                            */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      returns a pointer to the sc_buf, or NULL, if it could not       */
/*      be allocated.                                                   */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      xmalloc         bzero                                           */
/*                                                                      */
/************************************************************************/
struct sc_buf *
hsc_bld_sc_buf()
{
    struct sc_buf *scb;
    int     i;
    int    *ptr;

    scb = (struct sc_buf *) xmalloc((uint) BLDCMDSIZE, (uint) PGSHIFT,
				    pinned_heap);
    if (scb == NULL) {
	/* xmalloc failed--return NULL pointer */
	return (scb);
    }

    /* assure sc_buf is initialized */
    bzero(scb, sizeof(struct sc_buf));

    /* dummy bufstruct initialization */
    scb->bufstruct.b_forw = NULL;
    scb->bufstruct.b_back = NULL;
    scb->bufstruct.av_forw = NULL;
    scb->bufstruct.av_back = NULL;
    scb->bufstruct.b_iodone = (void (*) ()) hsc_iodone;	/* point at us */
    scb->bufstruct.b_vp = NULL;
    scb->bufstruct.b_work = 0;
    scb->bufstruct.b_options = 0;
    scb->bufstruct.b_event = EVENT_NULL;
    scb->bufstruct.b_xmemd.aspace_id = XMEM_GLOBAL;
    scb->bufstruct.b_flags = B_MPSAFE;
    scb->bufstruct.b_un.b_addr = (char *) scb + sizeof(struct sc_buf);

    /* additional sc_buf initialization */
    scb->bp = NULL;	/* set for non-spanned cmd */

    scb->timeout_value = 15;	/* set default timeout value */

    return (scb);

}  /* end hsc_bld_sc_buf */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_alloc_dev                                           */
/*                                                                      */
/* FUNCTION:    Allocate resources for starting a device                */
/*                                                                      */
/*      This internal routine performs actions required to ready        */
/*      a device information structure for use.                         */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called from any other routine.              */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      dev_index - index to device structure                           */
/*      devflag - device flag                                           */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0  for successful completion                      */
/*                  = EIO if resources were lacking                     */
/*                  = EINVAL if selected SCSI ID and LUN are already    */
/*                    started, or SCSI ID same as adapter's             */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
int
hsc_alloc_dev(
	      struct adapter_def * ap,
	      int dev_index,
	      ulong devflag)
{
    int     ret_code;

    ret_code = 0;	/* set default return code */

    /* validate the start parameters */
    if (ap->dev[dev_index].opened) {
	ret_code = EINVAL;	/* already opened */
	goto end;
    }

    /* make sure the passed scsi id is different from the card's */
    if (SID(dev_index) == ap->ddi.card_scsi_id) {
	ret_code = EINVAL;	/* bad SCSI ID */
	goto end;
    }

    /* adapter must have been opened in normal mode */
    if (ap->adapter_mode != NORMAL_MODE) {
	ret_code = EACCES;	/* wrong adapter mode */
	goto end;
    }

    /* init this device's info structure */
    ap->dev[dev_index].scsi_id = SID(dev_index);
    ap->dev[dev_index].lun_id = LUN(dev_index);
    ap->dev[dev_index].waiting = FALSE;
    ap->dev[dev_index].num_act_cmds = 0;
    ap->dev[dev_index].qstate = 0;
    ap->dev[dev_index].state = 0;
    ap->dev[dev_index].pqstate = 0;
    /* do not change "init_cmd" here, as it may already be in use */
    ap->dev[dev_index].head_pend = NULL;
    ap->dev[dev_index].tail_pend = NULL;
    ap->dev[dev_index].head_act = NULL;
    ap->dev[dev_index].tail_act = NULL;
    ap->dev[dev_index].async_func = NULL;
    /* set the queue depth for this device based on the enable_queuing  */
    /* flag. If enable_queuing is false then send only one command at a */
    /* time to the adapter for this device */
    if (ap->enable_queuing)
        ap->dev[dev_index].queue_depth = Q_ENABLED;
    else ap->dev[dev_index].queue_depth = Q_DISABLED;
    /* default to indicate that this device is not queuing */
    ap->dev[dev_index].dev_queuing = FALSE;
    ap->dev[dev_index].cc_error_state = 0;
    ap->dev[dev_index].cmd_save_ptr_cc = NULL;
    ap->dev[dev_index].cmd_save_ptr_res = NULL;
    /* allocate and initialize watchdog timer struct */
    ap->dev[dev_index].wdog = (struct timer *) xmalloc((int) sizeof 
				(struct timer), 4, pinned_heap);
    ap->dev[dev_index].wdog->adp = ap;
    ap->dev[dev_index].wdog->dev_index = dev_index;
    ap->dev[dev_index].wdog->timer_id = SC_MBOX_TMR;
    ap->dev[dev_index].wdog->dog.func = hsc_watchdog;
    ap->dev[dev_index].wdog->dog.next = NULL;
    ap->dev[dev_index].wdog->dog.prev = NULL;
    ap->dev[dev_index].wdog->dog.count = 0;
    ap->dev[dev_index].wdog->save_time = 0;
    ap->dev[dev_index].wdog->dog.restart = 0;
#ifdef _POWER_MP
    while(w_init(&ap->dev[dev_index].wdog->dog));
#else
    w_init(&ap->dev[dev_index].wdog->dog);
#endif


    /* do not change "trace_enable" here, as it may already be ena/disabled */

    /* mark this device as opened */
    ap->dev[dev_index].opened = TRUE;
    /* inc adapter device counter */
    ap->devices_in_use++;
end:
    return (ret_code);

}  /* end hsc_alloc_dev */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_dealloc_dev                                         */
/*                                                                      */
/* FUNCTION:    Deallocate resources for a device                       */
/*                                                                      */
/*      This internal routine performs actions required to stop         */
/*      (close) a device.                                               */
/*                                                                      */
/* NOTES:                                                               */
/*      Outstanding I/O to this device is allowed to finish, but        */
/*      further I/O is rejected.                                        */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called only on the process level.           */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      dev_index - index to device structure                           */
/*      devflag - device flag                                           */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0  for successful completion                      */
/*                  = EINVAL if device was not started (opened)         */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      disable_lock    unlock _enable                                  */
/*      e_sleep_thread                                                  */
/*                                                                      */
/************************************************************************/
int
hsc_dealloc_dev(
		struct adapter_def * ap,
		int dev_index,
		ulong devflag)
{
    int     ret_code;
    int     old_pri;

    ret_code = 0;	/* set default return code */

    /* validate the stop parameters */
    if (!ap->dev[dev_index].opened) {
	ret_code = EINVAL;	/* not started */
	goto end;
    }

    old_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));

    /* if any commands active, wait for them to finish.     */
    /* set "qstate" to dis-allow processing future commands */
    while ((ap->dev[dev_index].head_act != NULL) ||
	   (ap->dev[dev_index].head_pend != NULL)) {

	ap->dev[dev_index].qstate = STOP_PENDING;
	/* sleep until next i/o finished */
        e_sleep_thread(&ap->dev[dev_index].stop_event, &(hsc_mp_lock), 
                       LOCK_HANDLER);

    }	/* endwhile */

    /*  unregister and release the watchdog timer struct */
    w_clear( &ap->dev[dev_index].wdog->dog );
    ap->dev[dev_index].opened = FALSE;	/* mark device closed */
    ap->dev[dev_index].qstate = 0;	/* reset STOP_PENDING flag */
    unlock_enable(old_pri, &(hsc_mp_lock));
    (void) xmfree((void *) ap->dev[dev_index].wdog, pinned_heap);

    /* free any gathered write buffers which may be left around */
    hsc_free_gwrite(ap);

    ap->devices_in_use--;	/* decrement adapter devices_in_use */
end:
    return (ret_code);

}  /* end hsc_dealloc_dev */

/************************************************************************/
/*                                                                      */
/* NAME:        hsc_free_gwrite                                         */
/*                                                                      */
/* FUNCTION:    Free adapter gwrite buffers                             */
/*                                                                      */
/*      This internal routine frees up the adapter gwrite memory        */
/*      buffers that were used for gathered writes.                     */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called only on the process level.           */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      gwrite      - gathered write buffer management structure        */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      disable_lock    unlock _enable                                  */
/*      unpin           xmfree                                          */
/*                                                                      */
/************************************************************************/
void
hsc_free_gwrite(struct adapter_def * ap)
{
    int     i, old_pri;
    struct gwrite *current, *save_ptr;

    old_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));
    current = ap->head_gw_free;
    ap->head_gw_free = NULL;
    ap->tail_gw_free = NULL;
    unlock_enable(old_pri, &(hsc_mp_lock));
    /* loop while gwrite free list is non-empty */
    while (current != NULL) {
	save_ptr = current;
	current = current->next;
	/* unpin and free buffer first */
	(void) unpin(save_ptr->buf_addr, save_ptr->buf_size);
	(void) xmfree(save_ptr->buf_addr, kernel_heap);

	/* unpin and free gwrite struct */
	(void) unpin(save_ptr, sizeof(struct gwrite));
	(void) xmfree((caddr_t) save_ptr, kernel_heap);

    }	/* end while */

}  /* end hsc_free_gwrite */
/************************************************************************/
/*                                                                      */
/* NAME:        hsc_ioctl_sleep                                         */
/*                                                                      */
/* FUNCTION:    Waits for ioctl command to complete                     */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called only on the process level.           */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      disable_lock     unlock_enable                                  */
/*                                                                      */
/************************************************************************/
void hsc_ioctl_sleep(struct buf *bp)
{
    int    old_pri;                           /* old interrupt level */


#ifdef _POWER_MP

    old_pri = disable_lock(INTIODONE,&(hsc_ioctl_scbuf_lock));

    if (!(bp->b_flags & B_DONE)) {
        /*
	 * If buf has not completed then go to sleep.
	 */
      	e_sleep_thread((int *)&(bp->b_event),&(hsc_ioctl_scbuf_lock),
		       LOCK_HANDLER);
    }
    /*
     * Re-enable interrupts and unlock.
     */
    unlock_enable(old_pri,&(hsc_ioctl_scbuf_lock));

#else
    iowait(bp);
#endif
    return;

}
