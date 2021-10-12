#ifndef lint
static char sccsid[] = "@(#)47 1.26.4.16 src/bos/kernext/tape/tapeb.c, sysxtape, bos41J, 9513A_all 3/24/95 16:02:46";
#endif
/*
 * COMPONENT_NAME: (SYSXTAPE)  SCSI Magnetic Tape Device Driver Lower Half
 *
 * FUNCTIONS:  strdump, strstart, strsleep, striodone, strerror,
 *             strprocess_adapter_error, strprocess_check_condition,
 *             strprocess_scsi_error, strbldcmd, strreadwrite,
 *             str_request_sense, str_mode_sense, str_mode_select,
 *             str_general, str_load, str_erase, str_space,
 *             str_write_filemarks, strpush, strpop, strstore,
 *             strsend, strlog, strwatchdog, strpmhandler
 *
 * ORIGINS: 27, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/* INCLUDED SYSTEM FILES */
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/buf.h>
#include <sys/i_machine.h>
#include <sys/file.h>
#include <sys/sleep.h>
#include <sys/ioctl.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/uio.h>
#include <sys/errids.h>
#include <sys/watchdog.h>
#include <sys/dump.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>

#include <sys/scsi.h>
#include <sys/tape.h>
#include <sys/tapedd.h>

#ifdef _POWER_MP
#include <sys/lock_def.h>
#endif /* _POWER_MP */

/* END OF INCLUDED SYSTEM FILES  */

/************************************************************************/
/* Tape Pointer Array                                                */
/************************************************************************/
struct tape_device_df *tape_df_list_ptr[256] = {NULL};
int             tape_device_config = 0;
int             tape_device_lock_word = EVENT_NULL;
/************************************************************************/
/* Function Declarations                                                */
/************************************************************************/
#ifndef _NO_PROTO
int  strdump(dev_t devno, struct uio *uiop, int cmd, int arg,
	     int chan, int ext);
void strstart(struct tape_cmd *cmd_ptr);
void strsleep(struct tape_cmd *cmd_ptr);
void striodone(struct buf *buf_ptr);
int  strerror(struct tape_device_df *device_ptr, struct tape_cmd *cmd_ptr);
int  strprocess_adapter_error(struct tape_device_df *device_ptr,
			      struct tape_cmd *cmd_ptr);
int  strprocess_check_condition(struct tape_device_df *device_ptr,
				struct tape_cmd *cmd_ptr);
int  strprocess_scsi_error(struct tape_device_df *device_ptr,
			   struct tape_cmd *cmd_ptr);
int  strreset(struct tape_device_df *device_ptr, struct tape_cmd *cmd_ptr,
	      int open_seq_only);
void strbldcmd(struct tape_device_df *device_ptr,
	       struct tape_cmd *cmd_ptr, uchar direction,
	       uchar resume_flag);
void strreadwrite(struct tape_device_df *device_ptr,
		  struct tape_cmd *cmd_ptr, uchar *buffer_addr,
		  int transfer_length, int direction_flag);
void str_request_sense(struct tape_device_df *device_ptr,
		       struct tape_cmd *cmd_ptr, uchar clear_flag,
		       uchar resume_flag);
void str_mode_sense(struct tape_device_df *device_ptr,
		    struct tape_cmd *cmd_ptr, uchar resume_flag);
void str_mode_select(struct tape_device_df *device_ptr,
		     struct tape_cmd *cmd_ptr, uchar save_page,
		     uchar resume_flag);
void str_general(struct tape_device_df *device_ptr,
		 struct tape_cmd *cmd_ptr, uchar command_type,
		 uchar immediate_bit , uchar resume_flag);
void str_load(struct tape_device_df *device_ptr, struct tape_cmd *cmd_ptr,
	      char reten_req, uchar resume_flag, uchar load_flag);
void str_erase(struct tape_device_df *device_ptr, struct tape_cmd *cmd_ptr,
	       uchar resume_flag);
void str_space(struct tape_device_df *device_ptr, struct tape_cmd *cmd_ptr,
	       int direction, int type, daddr_t count, uchar resume_flag);
void str_write_filemarks(struct tape_device_df *device_ptr,
			 struct tape_cmd *cmd_ptr,
			 daddr_t num_filemarks,
			 uchar resume_flag);
void strpush(struct tape_device_df *device_ptr, struct tape_cmd *cmd_ptr);
struct tape_cmd
     *strpop(struct tape_device_df *device_ptr);
void strstore(struct tape_device_df *device_ptr,
	      struct tape_cmd *cmd_ptr);
void strsend(struct tape_device_df *device_ptr,
	     struct tape_cmd *cmd_ptr);
void strlog(struct tape_device_df *device_ptr, struct tape_cmd *cmd_ptr,
	    struct sc_error_log_df *error_rec_ptr, uchar error_type);
void strwatchdog(struct tape_watchdog *tape_watchdog_ptr);
int  strpmhandler(caddr_t private, int mode);
#else
int             strdump();
void            strstart();
void            strsleep();
void            striodone();
int             strerror();
int             strprocess_adapter_error();
int             strprocess_check_condition();
int             strprocess_scsi_error();
int             strreset();
void            strbldcmd();
void            strreadwrite();
void            str_request_sense();
void            str_mode_sense();
void            str_mode_select();
void            str_general();
void            str_load();
void            str_erase();
void            str_space();
void            str_write_filemarks();
void            strpush();
struct tape_cmd *strpop();
void            strstore();
void            strsend();
void            strlog();
void            strwatchdog();
int  		strpmhandler();
#endif

/**************************************************************************/
/*                                                                        */
/* NAME:  strdump                                                         */
/*                                                                        */
/* FUNCTION:  Tape Device Driver dump routine.                            */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by an interrupt handler and must be    */
/*      pinned to prevent a page fault.                                   */
/*                                                                        */
/* NOTES:  This routine will accept user commands to write dump data      */
/*         to tape.  Commands sent to the dump entry point for writing    */
/*         should not exceed the maximum transfer size for a command      */
/*         (write commands are not split up).  The DUMPQUERY command      */
/*         should give the maximum transfer size.  At the end of the      */
/*         dump, 2 filemarks are written and the tape is rewound.         */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    tape_df_list_ptr                Array of pointers to tape structures*/
/*                                                                        */
/* INPUTS:                                                                */
/*    devno  - major/minor number                                         */
/*    uiop   - pointer to uio structure which contains command information*/
/*    chan   - not used (for multiplexed devices).                        */
/*    ext    - defines mode required for open.                            */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL - device not defined.                                        */
/*           - invalid paramter received.                                 */
/*           - device not OPEN.                                           */
/*    EIO    - kernel service failure.                                    */
/*           - tape has had a hard failure (can't continue).              */
/*           - hardware or software error SCSI command failure.           */
/*           - SCSI adapter device driver failure.                        */
/*           - device power failure.                                      */
/*           - general error case for SCSI command failure.               */
/*           - device reserved by another initiator.                      */
/*           - tape is at or beyond logical end of tape.                  */
/*           - media is write protected.                                  */
/*           - a SCSI command has timed out before completion.            */
/*           - any other failure.                                         */
/*    ENXIO  - device not ready.                                          */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  bcopy                                     */
/*                              devdump                                   */
/*                                                                        */
/**************************************************************************/
int
strdump(
    dev_t           devno,      /* major/minor device number */
    struct uio     *uiop,       /* pointer to the uio vector */
    int             cmd,        /* operation to perform */
    int             arg,        /* user data structure pointer */
    int             chan,       /* channel number */
    int             ext)        /* flag for diagnostic mode */

{
    struct tape_device_df *device_ptr;
    struct dmp_query *dump_ptr;
    struct iovec   *iovp;
    uint            tape_offset;
    uint            dev;
    uint            i, rc;
    uint            tmp_buf[4];
#ifdef _POWER_MP
    uint            old_pri;
#endif /* _POWER_MP */

    dev = minor(devno);
    /*  The 5 high order bits of the minor number determines which
     *  offset in the dds array (tape_df_list_ptr) holds the device
     *  information for this device.
     */
    tape_offset = (dev >> 3) & DEVNO_OVERLAY;
    device_ptr = tape_df_list_ptr[tape_offset];

    if (device_ptr == NULL) {
	return (EINVAL);
    }
    if (!(device_ptr->opened)) {
	return (EINVAL);
    }

#ifdef _POWER_MP
    old_pri= disable_lock(INTIODONE,&device_ptr->intr_lock);	
#endif /* _POWER_MP */

    switch (cmd) {
	/*  The device should be open at this point, so all required
	 *  structures are pinned.  All that remains is to initialize
	 *  the adapter dd by calling it's dump init routine.
	 */
    case (DUMPINIT):
	rc = devdump(device_ptr->tape_ddi.adapter_devno, 0,
		     DUMPINIT, 0, 0, 0);
	if (rc != 0) {
#ifdef _POWER_MP
	    unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
	    return (rc);
	}
	/*  Prevents the device, while opened and inited for
	 *  dump, from being used as other than a dump device.
	 */
	device_ptr->flags |= TAPE_DUMP_DEVICE;
	/*  Returns the minimum size a buffer to the DUMPWRITE case can
	 *  be (min_tsize) and the maximum (max_tsize).  The adapter dd
	 *  provides these values here and checking is done to insure
	 *  the blocksize for the tape doesn't clash.
	 */
	break;
    case (DUMPQUERY):
	/* Get pointer to the user buffer.                     */
	dump_ptr = (struct dmp_query *) tmp_buf;
	rc = devdump(device_ptr->tape_ddi.adapter_devno, 0, DUMPQUERY,
		     dump_ptr, 0, 0);
	if (rc != 0) {
#ifdef _POWER_MP
	    unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
	    return (rc);
	}
	if (dump_ptr->min_tsize > device_ptr->tape_ddi.blocksize) {
#ifdef _POWER_MP
	    unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
	    return (EINVAL);
	}
	dump_ptr->min_tsize = device_ptr->tape_ddi.blocksize;
	device_ptr->max_xfer_size = dump_ptr->max_tsize;
	/* Copy the information back to the user buffer.       */
	bcopy((caddr_t) dump_ptr, (caddr_t) arg, sizeof(struct dmp_query));
	/*  DUMPSTART notifies the adapter dd that a dump is beginning.
	 *  A load is done here to insure the tape is at Beginning of
	 *  Tape (BOT) when data is written.
	 */
	break;
    case (DUMPSTART):
	/* If the dump device hasn't been initialized.         */
	if (!(device_ptr->flags & TAPE_DUMP_DEVICE)) {
#ifdef _POWER_MP
	    unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
	    return (EINVAL);
	}
	/* Notify the adapter dd that dump is to begin         */
	rc = devdump(device_ptr->tape_ddi.adapter_devno, 0, DUMPSTART,
		     0, 0, 0);
	if (rc != 0) {
#ifdef _POWER_MP
	    unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
	    return (rc);
	}
	/* setup to issue the load command */
	str_load(device_ptr, &device_ptr->cmd_1_ptr,
		(uchar) 0, (uchar) SC_RESUME, (uchar) 1);
	rc = devdump(device_ptr->tape_ddi.adapter_devno, 0, DUMPWRITE,
		     &device_ptr->cmd_1_ptr, 0, 0);
	if (rc != 0) {
#ifdef _POWER_MP
	    unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
	    return (rc);
	}
	/*  An array of structures (byte count and buffer addr.) is sent
	 *  and will be written to tape here.
	 */
	break;
    case (DUMPWRITE):
	/* If the dump device hasn't been initialized.         */
	if (!(device_ptr->flags & TAPE_DUMP_DEVICE)) {
#ifdef _POWER_MP
	    unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
	    return (EINVAL);
	}
	/*  Get pointer first structure in the iovec array (each
	 *  structure has a buffer addr. and byte count for the
	 *  data to be transferred.
	 */
	iovp = uiop->uio_iov;
	/* Loop through each iovec structure.                  */
	for (i = 0; i < uiop->uio_iovcnt; i++) {

	    /*  Insure the byte count is a multiple of block
	     *  size or <= maximum transfer size for var.
	     *  size records.
	     */
	    if (((device_ptr->tape_ddi.blocksize != 0) &&
	    (((int) iovp->iov_len % device_ptr->tape_ddi.blocksize) != 0)) ||
		(iovp->iov_len > device_ptr->max_xfer_size)) {
#ifdef _POWER_MP
		unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
		return (EINVAL);
	    }
	    /*  Setup xmem descriptor, build and send the
	     *  write command to the adapter dd.
	     */
	    device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd.aspace_id =
		XMEM_GLOBAL;
	    strbldcmd(device_ptr, &device_ptr->cmd_1_ptr, (uchar) SCSI_WRITE,
		     (uchar) SC_RESUME);
	    strreadwrite(device_ptr, &device_ptr->cmd_1_ptr,
			(uchar *) iovp->iov_base, iovp->iov_len,
			B_WRITE);
	    rc = devdump(device_ptr->tape_ddi.adapter_devno, 0,
			DUMPWRITE, &device_ptr->cmd_1_ptr, 0, 0);
	    if (rc != 0) {
#ifdef _POWER_MP
		unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
		return (rc);
	    }
	    /* Reduce total byte count by this transfer amt */
	    uiop->uio_resid -= iovp->iov_len;
	    iovp->iov_len = 0;
	    /* Setup address to next iovec structure.      */
	    iovp = (struct iovec *) (((int) iovp) + sizeof(struct iovec));
	}
	/*  This case is executed when all dump data has been trans-
	 *  ferred to tape.  Two filemarks are written and the tape is
	 *  rewound to end the dump.
	 */
	break;
    case (DUMPEND):
	/* If the dump device hasn't been initialized.         */
	if (!(device_ptr->flags & TAPE_DUMP_DEVICE)) {
#ifdef _POWER_MP
	    unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
	    return (EINVAL);
	}
	/* write 2 filemarks to end this dump */
	str_write_filemarks(device_ptr, &device_ptr->cmd_1_ptr, (daddr_t) 2,
			   (uchar) SC_RESUME);
	rc = devdump(device_ptr->tape_ddi.adapter_devno, 0, DUMPWRITE,
		     &device_ptr->cmd_1_ptr, 0, 0);
	if (rc != 0) {
#ifdef _POWER_MP
	    unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
	    return (rc);
	}
	/* rewind the tape */
	str_general(device_ptr, &device_ptr->cmd_1_ptr,
		   (uchar) SCSI_REWIND, (uchar) 0x0,
		   (uchar) SC_RESUME);
	rc = devdump(device_ptr->tape_ddi.adapter_devno, 0, DUMPWRITE,
		     &device_ptr->cmd_1_ptr, 0, 0);
	if (rc != 0) {
#ifdef _POWER_MP
	    unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
	    return (rc);
	}
	/* Call adapter dd dumpend routine to end this dump.    */
	rc = devdump(device_ptr->tape_ddi.adapter_devno, 0, DUMPEND, 0, 0, 0);
	if (rc != 0) {
#ifdef _POWER_MP
	    unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
	    return (rc);
	}
	/*  The device is terminated as a dump device.  The adapter dd
	 *  is notified and the flag indicating that this is a dump
	 *  device is cleared.
	 */
	break;
    case (DUMPTERM):
	/* If the dump device hasn't been initialized.          */
	if (!(device_ptr->flags & TAPE_DUMP_DEVICE)) {
#ifdef _POWER_MP
	    unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
	    return (EINVAL);
	}
	/* Call adapter dd dumpterm routine to end this dump.   */
	rc = devdump(device_ptr->tape_ddi.adapter_devno, 0, DUMPTERM,
		     0, 0, 0);
	if (rc != 0) {
#ifdef _POWER_MP
	    unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
	    return (rc);
	}
	/* Clear flag to indicate this is a dump device.         */
	device_ptr->flags &= ~TAPE_DUMP_DEVICE;
	break;
    default:
	/* The command issued is not recognized. Return EINVAL.      */
#ifdef _POWER_MP
	unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
	return (EINVAL);
    } /* switch */

#ifdef _POWER_MP
    unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */

    return (0);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  strstart                                                        */
/*                                                                        */
/* FUNCTION:  Tape Device Driver command issue routine.                   */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine is an internal subroutine an may not be called       */
/*      externally.  This routine must also be pinned as it is called     */
/*      from the tape deive driver iodone routine.                        */
/*                                                                        */
/* NOTES:  This routine accepts commands to be issued to the SCSI         */
/*         adapter device driver for processing.                          */
/*                                                                        */
/* DATA STRUCTURES:  none                                                 */
/*                                                                        */
/* INPUTS:                                                                */
/*    buf_ptr - pointer to the buf structure.                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  none                                        */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED: devstrat                                   */
/*                                                                        */
/**************************************************************************/
void
strstart(struct tape_cmd *cmd_ptr)

{

    /* Call the SCSI device driver strategy routine through switch table.  */
    DDHKWD5(HKWD_DD_TAPEDD, DD_ENTRY_CSTART, 0, cmd_ptr->device_ptr->devno,
	    cmd_ptr->scbuf.scsi_command.scsi_cmd.scsi_op_code,
	    cmd_ptr->scbuf.bufstruct.b_un.b_addr,
	    cmd_ptr->scbuf.bufstruct.b_bcount, 0);

#ifdef _POWER_MP
   cmd_ptr->scbuf.bufstruct.b_flags |= B_MPSAFE;
#endif /* _POWER_MP */
    cmd_ptr->device_ptr->tape_ddi.tape_pm_ptr.pmh.activity = 
           PM_ACTIVITY_OCCURRED; 
    (void) devstrat(&cmd_ptr->scbuf.bufstruct);
    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_CSTART, 
	    cmd_ptr->scbuf.bufstruct.b_error, cmd_ptr->device_ptr->devno);
    return;
}


/**************************************************************************/
/*                                                                        */
/* NAME:  strsleep                                                        */
/*                                                                        */
/* FUNCTION:  Tape Device Driver sleep routine.                           */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine is an internal subroutine an may not be called       */
/*      externally.  This routine must also be pinned as it disables      */
/*      and enables interrupts.                                           */
/*                                                                        */
/* NOTES:  This routine accepts a buffer pointer as input and goes to     */
/*         sleep if the command pointed to by the buffer pointer is not   */
/*         complete.                                                      */
/*                                                                        */
/* DATA STRUCTURES:  none                                                 */
/*                                                                        */
/* INPUTS:                                                                */
/*    buf_ptr - pointer to the buf structure.                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  none                                        */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  i_enable                                  */
/*                              i_disable                                 */
/*                              e_sleep                                   */
/*                              disable_lock                              */
/*                              unlock_enable                             */
/*                              e_sleep_thread                            */
/*                                                                        */
/**************************************************************************/
void
strsleep(struct tape_cmd *cmd_ptr)
{
#ifdef _POWER_MP
    int             old_level;
    struct tape_device_df *device_ptr;
#else
    int             signal, old_level;
#endif /* _POWER_MP */

    /*  Interrupts are disabled to the iodone level for sleeping.
     *  This prevent e_sleep/e_sleep_thread from being called after 
     *  iodone processing has been completed.
     */

#ifdef _POWER_MP

    device_ptr = (struct tape_device_df *) (cmd_ptr->device_ptr);
    old_level = disable_lock(INTIODONE,&device_ptr->intr_lock);

    /* Sleep if B_DONE is not set in the scbuf (bufstruct).        */
    while ((cmd_ptr->scbuf.bufstruct.b_flags & B_DONE) == 0) {
	/* issue sleep for this command */
	(void) e_sleep_thread(&cmd_ptr->scbuf.bufstruct.b_event,
    		&device_ptr->intr_lock, LOCK_HANDLER);
    }
    unlock_enable(old_level,&device_ptr->intr_lock); 
#else
    old_level = i_disable(INTIODONE);

    signal = EVENT_SHORT;

    /* Sleep if B_DONE is not set in the scbuf (bufstruct).        */
    while ((cmd_ptr->scbuf.bufstruct.b_flags & B_DONE) == 0) {
	/* issue sleep for this command */
	(void) e_sleep(&cmd_ptr->scbuf.bufstruct.b_event, signal);
    }
    i_enable(old_level);
#endif /* _POWER_MP */

}


/**************************************************************************/
/*                                                                        */
/* NAME:  striodone                                                       */
/*                                                                        */
/* FUNCTION:  Tape Device Driver ioctl routine.                           */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by an interrupt handler and must be    */
/*      pinned to prevent a page fault.                                   */
/*                                                                        */
/* NOTES:  This routine accepts command completion codes from the SCSI    */
/*         adapter device driver for processing.  If an error occurs the  */
/*         error routine is called to handle any required retries and     */
/*         error return values.                                           */
/*                                                                        */
/* DATA STRUCTURES:  none                                                 */
/*                                                                        */
/* INPUTS:                                                                */
/*    buf_ptr - pointer to the buf structure.                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  none                                        */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  e_wakeup                                  */
/*                              disable_lock                              */
/*                              unlock_enable                             */
/*                                                                        */
/**************************************************************************/
void
striodone(struct buf     *buf_ptr)

{
    struct tape_device_df *device_ptr;
    struct tape_cmd *cmd_ptr;
    struct sc_error_log_df err_record;
    uint            i, ret_flag, dev;
    uchar           temp_error_save, issue_flag;

#ifdef _POWER_MP
    uint old_pri;
#endif /* _POWER_MP */

    cmd_ptr = (struct tape_cmd *) buf_ptr;
    device_ptr = (struct tape_device_df *) (cmd_ptr->device_ptr);
    dev = minor(device_ptr->devno);

#ifdef _POWER_MP
    /* disable my intrp level and spin lock                               */
    old_pri = disable_lock(INTIODONE, &device_ptr->intr_lock);
#endif /* _POWER_MP */

    DDHKWD5(HKWD_DD_TAPEDD, DD_ENTRY_IODONE, 0, cmd_ptr->device_ptr->devno,
	    cmd_ptr->scbuf.scsi_command.scsi_cmd.scsi_op_code,
	    cmd_ptr->scbuf.bufstruct.b_un.b_addr,
	    cmd_ptr->scbuf.bufstruct.b_bcount, 0);
    /* Check for a good return status.                                */
    buf_ptr->b_flags &= ~B_DONE;
    if ((buf_ptr->b_error == 0) && ((buf_ptr->b_flags & B_ERROR) == 0)) {
	switch (device_ptr->cmd_state) {
	case (TAPE_IO):

	    /*  For normal i/o, check to see if another command
	     *  is waiting.  Issue it and wakeup the current
	     *  command.  The read/write routines will continue
	     *  to build the next command while a command is
	     *  outstanding to the device.
	     */
	    if (buf_ptr->b_resid != 0) {
		/* good return but some data not transferred.   */
		if (device_ptr->tape_ddi.blocksize != 0) {
		    buf_ptr->b_error = EIO; /* an error when blksize fixed */
		}
		else if (buf_ptr->b_resid < 0) {
		    buf_ptr->b_error = ENOMEM;
		}
	    }
	    /* Everything's ok, is a command waiting to go out? */
	    else if (device_ptr->cmd_outstanding != NULL) {
		strstart(device_ptr->cmd_outstanding);  /* send it */
		device_ptr->cmd_outstanding = NULL;
	    }
	    buf_ptr->b_flags |= B_DONE;
	    e_wakeup(&buf_ptr->b_event);
	    break;
	case (TAPE_SIGNALRCV):
	    /*  If a SCIOHALT was issued or a kill signal was received,
	     *  return any outstanding commands.
	     */
	case (TAPE_IOCTL):
	case (TAPE_GENERAL):
	    buf_ptr->b_flags |= B_DONE;
	    e_wakeup(&buf_ptr->b_event);
	    break;
	case (TAPE_TUR):
	    /*  When a test unit ready was received, the device
	     *  is executing the reset sequence of test unit
	     *  ready, reserve, mode sense and mode select.
	     */
	    if (device_ptr->tape_ddi.res_sup) {
		device_ptr->cmd_state = TAPE_RESERVE;
		str_general(device_ptr, cmd_ptr,
			   (uchar) SCSI_RESERVE_UNIT, (uchar) 0x0,
			   (uchar) SC_RESUME);
		strstart(cmd_ptr);
		break;
	    }
	case (TAPE_RESERVE):
	    if (device_ptr->flags & TAPE_OPENING_DEVICE) {

		/*  If the tape was opened with retention,
		 *  then issue the load with the retention
		 *  bit set on.  If the device indicates a
		 *  unit attention during open then force
		 *  the tape to BOT.  Else issue mode sense
	         */
		if (dev & TAPE_RETEN_ON_OPEN) {
		    device_ptr->cmd_state = TAPE_LOAD;
		    str_load(device_ptr, cmd_ptr,	/* retension on */
			    (uchar) 1, (uchar) SC_RESUME, (uchar) 1);
		    strstart(cmd_ptr);
		}
		else if (device_ptr->flags2 & TAPE_LOAD_REQUIRED) {
		    device_ptr->flags2 &= ~TAPE_LOAD_REQUIRED;
		    device_ptr->cmd_state = TAPE_LOAD;
		    str_load(device_ptr, cmd_ptr,	/* no retension */
			    (uchar) 0, (uchar) SC_RESUME, (uchar) 1);
		    strstart(cmd_ptr);
		}
		else {
		    /* Load not required in this case.  */
		    device_ptr->cmd_state = TAPE_SENSE;
		    str_mode_sense(device_ptr, cmd_ptr, (uchar) SC_RESUME);
		    strstart(cmd_ptr);
		}
	    }
	    else {
		    /* Load not required in this case.  */
		device_ptr->cmd_state = TAPE_SENSE;
		str_mode_sense(device_ptr, cmd_ptr, (uchar) SC_RESUME);
		strstart(cmd_ptr);
	    }
	    break;
	case (TAPE_LOAD):
	    /*  When a load command has been completed, the next
	     *  command issued in the reset sequence is a mode
	     *  sense command.
	     */
	    device_ptr->cmd_state = TAPE_SENSE;
	    str_mode_sense(device_ptr, cmd_ptr, (uchar) SC_RESUME);
	    strstart(cmd_ptr);
	    break;
	case (TAPE_SENSE):
	    /*  From the sense data received via the mode sense
	     *  command, determine if the tape is write protected and
	     *  then issue the mode select command.
	     */
	    if (device_ptr->mode_buf[2] & 0x80) {
		device_ptr->flags |= TAPE_READ_ONLY;
	    }
	    else {
		device_ptr->flags &= ~TAPE_READ_ONLY;
	    }
	    /*  clear first two bytes of mode sense data so compare
	     *  can be done to determine if a mode select command
	     *  is necessary.
	     */
	    device_ptr->mode_buf[0] = 0;
	    device_ptr->mode_buf[1] = 0;
	    /*  These devices set the PS field when sending these
	     *  pages.  We have to reset it for our compare (It's the
	     *  first byte of the page.)
	     */
	    if ((device_ptr->tape_ddi.dev_type == TAPE_QIC525) ||
	        (device_ptr->tape_ddi.dev_type == TAPE_QIC1200)) {
	        device_ptr->mode_buf[12] &= 0x7f;
	    }
 	    /* This is being placed here to cover the case of switching 
 	     * densities on the 9-track.  When the density needs to be  
 	     * changed on a 9-track, it is done in the str_mode_select  
  	     * routine.  However, to reach the str_mode_select routine  
 	     * the density value in the mode_select_data must differ    
 	     * This is a deadlock situation and so this specific check  
 	     * is put in.  The other drives have their own checks which 
 	     * force them into the str_mode_select routine, except 9-trk
	     */
	    if (device_ptr->tape_ddi.dev_type == TAPE_9TRACK) {
                str_mode_select(device_ptr, cmd_ptr, FALSE, 
				(uchar) SC_RESUME);
            }
	    /*  If the mode select command does not need to be issued
	     *  then fall through to the mode select clause process.
	     */
	    issue_flag = FALSE;
	    if (device_ptr->selection_set) {
		for (i = 0; i < device_ptr->tape_ddi.mode_data_length; i++) {
		    if (device_ptr->mode_buf [i] !=
				device_ptr->tape_ddi.mode_select_data[i]) {
			/* issue the mode select command.                 */
			str_mode_select(device_ptr, cmd_ptr, TRUE,
					(uchar) SC_RESUME);
			device_ptr->cmd_state = TAPE_SELECT;
			issue_flag = TRUE;
			strstart(cmd_ptr);
			break;
		    }
		}
		if (!(issue_flag) &&
		   ((device_ptr->tape_ddi.dev_type == TAPE_QUARTER) ||
	           (device_ptr->tape_ddi.dev_type == TAPE_QIC525) ||
	           (device_ptr->tape_ddi.dev_type == TAPE_QIC1200))) {
	            /* issue the mode select command.                 */
	            str_mode_select(device_ptr, cmd_ptr, FALSE,
	                            (uchar) SC_RESUME);
	            device_ptr->cmd_state = TAPE_SELECT;
	            issue_flag = TRUE;
	            strstart(cmd_ptr);
		}
	    }
	    else {
	        /* issue the mode select command.                 */
	        str_mode_select(device_ptr, cmd_ptr, TRUE, (uchar) SC_RESUME);
		device_ptr->cmd_state = TAPE_SELECT;
		issue_flag = TRUE;
		strstart(cmd_ptr);
	    }
	    if (issue_flag)
		break;
	case (TAPE_SELECT):
	    /*  If the reset sequence was issued because of a
	     *  command failure, then reissue the command that
	     *  had the error.  If the reset was issued because
	     *  of opening the device or resetting device para-
	     *  meters then wake up the sleeping process.
	     */
	    if ((!(device_ptr->flags & TAPE_OPENING_DEVICE)) &&
		(!(device_ptr->flags2 & TAPE_RESET_PARAMS))) {
		cmd_ptr =  strpop(device_ptr);
		device_ptr->cmd_state = cmd_ptr->last_state;
		strstart(cmd_ptr);
	    }
	    else {
		/* wakeup done here */
		buf_ptr->b_flags |= B_DONE;
		e_wakeup(&buf_ptr->b_event);
	    }
	    break;
	case (TAPE_REQ_SENSE):
	    /*  When a request sense completes, call the error
	     *  routine with the command that had the error for
	     *  processing.
	     */
	    device_ptr->flags |= TAPE_REQSENSE_AVAIL;

	    /*  For the 3490E drive, determine if a tape is ready in the
	     *  loader.
 	     */
 	    if (device_ptr->tape_ddi.dev_type == TAPE_3490E) {
 		if ((device_ptr->req_sense_buf.extended_byte6 & 0x40) != 0) {
 		    device_ptr->flags2 |= TAPE_LOADER_READY;
 		}
 		else {
 		    device_ptr->flags2 &= ~TAPE_LOADER_READY;
 		}
 	    }

	    cmd_ptr = strpop(device_ptr);
	    device_ptr->cmd_state = cmd_ptr->last_state;
	    ret_flag = strerror(device_ptr, cmd_ptr);
	    temp_error_save = cmd_ptr->scbuf.bufstruct.b_error;

	    /*  If strerror returns with TAPE_WAKEUP, then it
	     *  was determined that no retrys are to be done
	     *  and the calling process is awoken to process
	     *  the error.
	     */
	    if (ret_flag == TAPE_WAKEUP) {
		while (cmd_ptr->type != TAPE_OTHER_CMD) {
		    cmd_ptr = strpop(device_ptr);
		}
		device_ptr->cmd_state = cmd_ptr->last_state;
		cmd_ptr->scbuf.bufstruct.b_error = temp_error_save;
		cmd_ptr->scbuf.bufstruct.b_flags |= B_DONE;
		e_wakeup(&cmd_ptr->scbuf.bufstruct.b_event);
	    }
	    break;
	default:
	    /* Should never get here but return an error if.  */
	    cmd_ptr->scbuf.bufstruct.b_error = EIO;
	    cmd_ptr->scbuf.bufstruct.b_flags |= B_DONE;
	    e_wakeup(&cmd_ptr->scbuf.bufstruct.b_event);
	    break;
	}
    }
    else {
	/*  The following 2 if statements are special process-
	 *  ing for user aborted commands and special SCSI
	 *  errors.
	 *
	 *  If this point is reached, a signal was received to
	 *  halt command processing.  All processing is aborted
	 *  (error processing included) and an error is returned.
	 */
	if ((cmd_ptr->scbuf.bufstruct.b_error == ENXIO) ||
	    (device_ptr->cmd_state == TAPE_SIGNALRCV)) {
	    while ((cmd_ptr->type != TAPE_OTHER_CMD) &&
		  (cmd_ptr->type != TAPE_SPECIAL_CMD) &&
		  (cmd_ptr->type != TAPE_DIAG_CMD)) {
		cmd_ptr = strpop(device_ptr);
	    }
	    device_ptr->cmd_state = cmd_ptr->last_state;
	    cmd_ptr->scbuf.bufstruct.b_flags |= B_DONE;
	    e_wakeup(&cmd_ptr->scbuf.bufstruct.b_event);
#ifdef _POWER_MP
	    unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IODONE, ENXIO, device_ptr->devno);
	    return;
	}
	/*  If an error is received from the scsi adapter dd
	 *  there is no valid status, abort command processing
	 *  and return an error.
	 */
	if (cmd_ptr->scbuf.status_validity == 0) {
	    /*  Determine if this failure occurred during reset
	     *  processing.  If so, then retry the command as
	     *  needed.
	     */
	    ret_flag = strreset(device_ptr, cmd_ptr, 1);
	    if (ret_flag == TAPE_NOWAKEUP) {
#ifdef _POWER_MP
		unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IODONE, cmd_ptr->scbuf.bufstruct.b_error,
		            device_ptr->devno);
		return;
	    }
	    while ((cmd_ptr->type != TAPE_OTHER_CMD) &&
		  (cmd_ptr->type != TAPE_SPECIAL_CMD) &&
		  (cmd_ptr->type != TAPE_DIAG_CMD)) {
		cmd_ptr = strpop(device_ptr);
	    }
	    device_ptr->cmd_state = cmd_ptr->last_state;
	    cmd_ptr->scbuf.bufstruct.b_error = EIO;
	    cmd_ptr->scbuf.bufstruct.b_flags |= B_DONE;
	    e_wakeup(&cmd_ptr->scbuf.bufstruct.b_event);
#ifdef _POWER_MP
            unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IODONE, ENXIO, device_ptr->devno);
	    return;
	}
	switch (cmd_ptr->type) {
	/* Normal command completion with error. */
	case (TAPE_OTHER_CMD):
	case (TAPE_RESET_CMD):
	    /*  Check to see if an error occured other than a
	     *  check condition.  If so, call strerror to pro-
	     *  cess the error.
	     */
	    device_ptr->flags &= ~TAPE_REQSENSE_AVAIL;
	    if ((cmd_ptr->scbuf.status_validity & 0x02) ||
	       ((cmd_ptr->scbuf.status_validity & 0x01) &&
	       ((cmd_ptr->scbuf.scsi_status & SCSI_STATUS_MASK) !=
	       SC_CHECK_CONDITION))) {
		cmd_ptr->last_state = device_ptr->cmd_state;
		ret_flag = strerror(device_ptr, cmd_ptr);
		temp_error_save = cmd_ptr->scbuf.bufstruct.b_error;
		/*  If strerror returns with TAPE_WAKEUP, then it
		 *  was determined that no retrys are to be done
		 *  and the calling process is awoken to process
		 *  the error.
		 */
		if (ret_flag == TAPE_WAKEUP) {
		    while (cmd_ptr->type != TAPE_OTHER_CMD) {
			cmd_ptr = strpop(device_ptr);
		    }
		    device_ptr->cmd_state = cmd_ptr->last_state;
		    cmd_ptr->scbuf.bufstruct.b_error = temp_error_save;
		    cmd_ptr->scbuf.bufstruct.b_flags |= B_DONE;
		    e_wakeup(&cmd_ptr->scbuf.bufstruct.b_event);
		}
	    }
	    else {
		/*  If this point is reached, a scsi check con-
		 *  dition has occurred.  A request sense is
		 *  issued to the device.
		 */
		cmd_ptr->last_state = device_ptr->cmd_state;
		strpush(device_ptr, cmd_ptr);
		device_ptr->reqsense_cmd_ptr.retry_flag = TRUE;
		device_ptr->reqsense_cmd_ptr.retry_count = 0;
		device_ptr->cmd_state = TAPE_REQ_SENSE;
		device_ptr->reqsense_cmd_ptr.type = TAPE_REQSENSE_CMD;
		str_request_sense(device_ptr,
				  &device_ptr->reqsense_cmd_ptr,
				  (uchar) 0, (uchar) SC_RESUME);
		strstart(&device_ptr->reqsense_cmd_ptr);
		break;
	    }
	    break;
	case (TAPE_REQSENSE_CMD):
	    /*  A failure in the request sense command occurred
	     *  The command is retried if it is possible. Other
	     *  wise an error is logged and returned to the user
	     */
	    if ((cmd_ptr->retry_flag) &&
	       (cmd_ptr->retry_count < TAPE_MAX_RETRY)) {
		cmd_ptr->scbuf.bufstruct.b_flags &= ~B_ERROR;
		cmd_ptr->scbuf.bufstruct.b_error = 0;
		cmd_ptr->retry_count++;
		strstart(cmd_ptr);
	    }
	    else {
		/* Do not retry, issue an error to the process.   */
		err_record.error_id = ERRID_TAPE_ERR5;
		strlog(device_ptr, cmd_ptr, &err_record,
		      (uchar) TAPE_ADAPTER_ERROR);
		errsave(&err_record, sizeof(struct sc_error_log_df));
		while (cmd_ptr->type != TAPE_OTHER_CMD) {
		    cmd_ptr = strpop(device_ptr);
		}
		device_ptr->cmd_state = cmd_ptr->last_state;
		cmd_ptr->scbuf.bufstruct.b_error = EIO;
		cmd_ptr->scbuf.bufstruct.b_flags |= B_DONE;
		e_wakeup(&cmd_ptr->scbuf.bufstruct.b_event);
	    }
	    break;
	case (TAPE_DIAG_CMD):
	case (TAPE_SPECIAL_CMD):
	    /*  This is for processing a failure when issuing
	     *  a diagnostic command.  No error recovery is
	     *  done.  An EIO is simply returned.
	     */
	    cmd_ptr->scbuf.bufstruct.b_error = EIO;
	    cmd_ptr->scbuf.bufstruct.b_flags |= B_DONE;
	    e_wakeup(&cmd_ptr->scbuf.bufstruct.b_event);
	    break;
	default:
	    /* Should never get here but return an error if.  */
	    cmd_ptr->scbuf.bufstruct.b_error = EIO;
	    cmd_ptr->scbuf.bufstruct.b_flags |= B_DONE;
	    e_wakeup(&cmd_ptr->scbuf.bufstruct.b_event);
	    break;
	}
    }
#ifdef _POWER_MP
    unlock_enable(old_pri, &device_ptr->intr_lock);
#endif /* _POWER_MP */
    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IODONE,
	    cmd_ptr->scbuf.bufstruct.b_error, device_ptr->devno);
    return;
}


/**************************************************************************/
/*                                                                        */
/* NAME:  strerror                                                        */
/*                                                                        */
/* FUNCTION:  Tape Device Driver error routine.                           */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by an interrupt handler and must be    */
/*      pinned to prevent a page fault.                                   */
/*                                                                        */
/* NOTES:  This routine is called to determine which routine to call for  */
/*         processing adapter, scsi check condition, or scsi bus errors.  */
/*         Any retry processing is done within the called routine.        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    disk structure                  disk information structure          */
/*                                                                        */
/* INPUTS:                                                                */
/*    cmd_ptr - pointer to the tape command structure.                    */
/*    dev_ptr - pointer to the the device information structure.          */
/*                                                                        */
/* RETURN VALUE DESCRIPTION: TAPE_WAKEUP   - Informs striodone to issue   */
/*                                           the e_wakeup on return.      */
/*                           TAPE_NOWAKEUP - Informs striodone to do a    */
/*                                           simple return.               */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  none                                      */
/*                                                                        */
/**************************************************************************/
int
strerror(
    struct tape_device_df *device_ptr,
    struct tape_cmd *cmd_ptr)

{
    int             rc;

    /* If an adapter error occurred, call routine to process. */
    if (cmd_ptr->scbuf.status_validity & SC_ADAPTER_ERROR) {
	rc = strprocess_adapter_error(device_ptr, cmd_ptr);
	return (rc);
    }
    /*  If a check condition occurred, and the request sense
     *  has completed successfully, then call the process check
     *  condition routine to process the sense data.
     */
    if (device_ptr->flags & TAPE_REQSENSE_AVAIL) {
	rc = strprocess_check_condition(device_ptr, cmd_ptr);
	return (rc);
    }
    /*  If a scsi bus error occurred (would be other than a
     *  check condition), then call the routine to process it.
     */
    if (cmd_ptr->scbuf.status_validity & SC_SCSI_ERROR) {
	rc = strprocess_scsi_error(device_ptr, cmd_ptr);
	return (rc);
    }
    /* Should never get here, but return an error if so.      */
    cmd_ptr->scbuf.bufstruct.b_error = EIO;
    return (TAPE_WAKEUP);
}


/**************************************************************************/
/*                                                                        */
/* NAME:  strprocess_adapter_error                                        */
/*                                                                        */
/* FUNCTION:  Tape Device Driver error routine.                           */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by an interrupt handler and must be    */
/*      pinned to prevent a page fault.                                   */
/*                                                                        */
/* NOTES:  This routine is called by strerror when an adapter error has   */
/*         occurred.  Here, it is determined if a retry is possible and   */
/*         if the error should be logged.                                 */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    disk structure                  disk information structure          */
/*                                                                        */
/* INPUTS:                                                                */
/*    cmd_ptr - pointer to the tape command structure.                    */
/*    dev_ptr - pointer to the the device information structure.          */
/*                                                                        */
/* RETURN VALUE DESCRIPTION: TAPE_WAKEUP   - Informs striodone to issue   */
/*                                           the e_wakeup on return.      */
/*                           TAPE_NOWAKEUP - Informs striodone to do a    */
/*                                           simple return.               */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  errsav                                    */
/*                                                                        */
/**************************************************************************/
int
strprocess_adapter_error(
    struct tape_device_df *device_ptr,
    struct tape_cmd *cmd_ptr)

{

    struct sc_error_log_df err_record;
    int             rc;

    switch (cmd_ptr->scbuf.general_card_status) {
    case (SC_HOST_IO_BUS_ERR):
    case (SC_SCSI_BUS_FAULT):
    case (SC_CMD_TIMEOUT):
	cmd_ptr->scbuf.bufstruct.b_error = EIO;
	/* Set correct errno for special situations.              */
	if (cmd_ptr->scbuf.general_card_status == SC_CMD_TIMEOUT) {
	    cmd_ptr->scbuf.bufstruct.b_error = ETIMEDOUT;
	}
	/*  On the last retry, if a scsi bus fault occurred, turn
	 *  on the flag that forces async mode to the adapter.
	 *  Command to the adapter are first issued synchronously.
	 *  On some devices, this will cause a scsi bus fault error
	 *  In these cases, the last retry is sent out to force
	 *  asynchronous negotiation to possibly fix this.
	 */
	if ((cmd_ptr->retry_flag) &&
	   (cmd_ptr->retry_count < TAPE_MAX_RETRY)) {
	    if ((cmd_ptr->scbuf.general_card_status == SC_SCSI_BUS_FAULT) &&
		((cmd_ptr->retry_count + 1) == TAPE_MAX_RETRY)) {
		device_ptr->async_flag = SC_ASYNC;
	    }
	    cmd_ptr->scbuf.bufstruct.b_flags &= ~B_ERROR;
	    cmd_ptr->scbuf.bufstruct.b_error = 0;
	    cmd_ptr->retry_count++;
	    /*  An error is logged for a command timeout every time,
	     *  therefore, the error is logged even if the command is
	     *  retried.
	     */
	    if (cmd_ptr->scbuf.general_card_status == SC_CMD_TIMEOUT) {
		err_record.error_id = ERRID_TAPE_ERR4;
		strlog(device_ptr, cmd_ptr, &err_record,
		      (uchar) TAPE_ADAPTER_ERROR);
		errsave(&err_record, sizeof(struct sc_error_log_df));
	    }
	    strstart(cmd_ptr);
	    return (TAPE_NOWAKEUP);
	}
	/* An error is logged on all of the 4 command above except */
	/* for a host io bus error.  The adapter logs that error. */
	if (cmd_ptr->scbuf.general_card_status != SC_HOST_IO_BUS_ERR) {
	    err_record.error_id = ERRID_TAPE_ERR4;
	    strlog(device_ptr, cmd_ptr, &err_record,
		  (uchar) TAPE_ADAPTER_ERROR);
	    errsave(&err_record, sizeof(struct sc_error_log_df));
	}
	break;
	/*  For scsi bus resets, go through the initialization path
	 *  again.  Since io type commands are not retried when a
	 *  bus reset occurs, an error is returned an the drive is
	 *  set offline (close is the only operation allowed now).
	 */
    case (SC_NO_DEVICE_RESPONSE):
        /*  If this is during an open, the reset sequence is attempted
         *  to reset device paramters before the failing command is retried.
         */
        if (device_ptr->cmd_state != TAPE_IO) {
            rc = strreset(device_ptr, cmd_ptr, 1);
            if (rc != 0) {
                return (rc);
            }
        }
        /* I/O command.  Set the error value and exit.          */
        cmd_ptr->scbuf.bufstruct.b_error = ENOTREADY;
        break;
    case (SC_SCSI_BUS_RESET):
	/*  In this case, a scsi bus reset has occurred.  If the
	 *  command that got the reset was a read/write, then the
	 *  drive is taken offline and only a CLOSE is allowed there
	 *  after.  If not, the reset sequence is attempted to reset
	 *  device paramaters before the failing command is retried.
	 */
	if (device_ptr->cmd_state != TAPE_IO) {
	    rc = strreset(device_ptr, cmd_ptr, 0);
	    if (rc != 0) {
		return (rc);
	    }
	}
	/* I/O command.  Set the device offline and exit.          */
	device_ptr->flags |= TAPE_OFFLINE;
	cmd_ptr->scbuf.bufstruct.b_error = EIO;
	break;
    case (SC_ADAPTER_HDW_FAILURE):
    case (SC_ADAPTER_SFW_FAILURE):
    case (SC_FUSE_OR_TERMINAL_PWR):
	/*  No retry here.  Catastrophic error situations.  The
	 *  adapter dd will log an error for these occurrences.
	 */
	cmd_ptr->scbuf.bufstruct.b_error = EIO;
	break;
    default:
	cmd_ptr->scbuf.bufstruct.b_error = EIO;
	break;
    }
    return (TAPE_WAKEUP);
}


/**************************************************************************/
/*                                                                        */
/* NAME:  strprocess_check_condition                                      */
/*                                                                        */
/* FUNCTION:  Tape Device Driver error routine.                           */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by an interrupt handler and must be    */
/*      pinned to prevent a page fault.                                   */
/*                                                                        */
/* NOTES:  This routine is called by strerror when scsi status indicates  */
/*         that a check condition has occurred.  The request sense has    */
/*         already been completed via the iodone routine and is analized  */
/*         here for retry or error logging possibilities.                 */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    disk structure                  disk information structure          */
/*                                                                        */
/* INPUTS:                                                                */
/*    cmd_ptr - pointer to the tape command structure.                    */
/*    dev_ptr - pointer to the the device information structure.          */
/*                                                                        */
/* RETURN VALUE DESCRIPTION: TAPE_WAKEUP   - Informs striodone to issue   */
/*                                           the e_wakeup on return.      */
/*                           TAPE_NOWAKEUP - Informs striodone to do a    */
/*                                           simple return.               */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  errsav                                    */
/*                                                                        */
/**************************************************************************/
int
strprocess_check_condition(
    struct tape_device_df *device_ptr,
    struct tape_cmd *cmd_ptr)

{
    struct sc_error_log_df err_record;
    int             resid, rc, blocks_remaining;

    /*  If the residual count valid bit is set, then determine the
     *  amount of data not transferred (resid). For read/write commands
     *  with a fixed blocksize, this is given in number of blocks and
     *  must be converted to determine byte count.
     */
    if (device_ptr->req_sense_buf.err_code & 0x80) {
	resid = ((int) (device_ptr->req_sense_buf.sense_byte0 << 24)) |
	    ((int) (device_ptr->req_sense_buf.sense_byte1 << 16)) |
	    ((int) (device_ptr->req_sense_buf.sense_byte2 << 8)) |
	    ((int) (device_ptr->req_sense_buf.sense_byte3));
	if ((cmd_ptr->last_state == TAPE_IO) &&
	   (device_ptr->tape_ddi.blocksize != 0)) {
	    resid *= device_ptr->tape_ddi.blocksize;
	}
    }
    /* If the valid bit is not set, assume a 0 resid count.           */
    else {
	resid = 0;
    }
    /*  If variable length records and the resid is < 0 (i.e. on some
     *  devices, this means that the variable record on tape was larger
     *  than the number of bytes requested with the read) then set the
     *  resid to 0.
     */
    if (cmd_ptr->last_state == TAPE_IO) {
    	if ((device_ptr->tape_ddi.blocksize == 0) && (resid < 0)) {
     	    if ((device_ptr->flags2 & TAPE_SHORTREAD_OK) == 0) {
	        cmd_ptr->scbuf.bufstruct.b_error = ENOMEM;
	    }
	    else {
	        cmd_ptr->scbuf.bufstruct.b_error = 0;
	    }
	    cmd_ptr->scbuf.bufstruct.b_resid = 0;
        }
        else {
	    cmd_ptr->scbuf.bufstruct.b_resid = resid;
        }
    }
    else {
        cmd_ptr->scbuf.bufstruct.b_resid = resid;
    }

    /* Test the sense key returned via the request sense data.        */
    switch (device_ptr->req_sense_buf.sense_key & 0x0F) {
    case (TAPE_NO_SENSE):
	if ((device_ptr->tape_ddi.dev_type == TAPE_4MM2GB) ||
	    (device_ptr->tape_ddi.dev_type == TAPE_4MM4GB)) {
	    /*  Test for drive needing to be cleaned error. This
	     *  error is tracked internally in the drive, and is
	     *  caused by the soft error count.
	     */
	    if (device_ptr->req_sense_buf.add_sense_key == 0x82 &&
		device_ptr->req_sense_buf.extended_byte1 == 0x82 ) {
	           cmd_ptr->scbuf.bufstruct.b_flags &= ~B_ERROR;
	           cmd_ptr->scbuf.bufstruct.b_error = 0;
	           err_record.error_id = ERRID_TAPE_ERR3;
		   strlog(device_ptr, cmd_ptr, &err_record,
		       (uchar) TAPE_SCSI_ERROR);
		   errsave(&err_record, sizeof(struct sc_error_log_df));
		   rc = strreset(device_ptr, cmd_ptr, 1);
		   if (rc != 0) {
		   /*  Return reset status.  If rc is not 0 the the re-
		    *  set is in progress (= TAPE_NOWAKEUP).
		    */
		       return (rc);
		   }
		   return (TAPE_WAKEUP);
	    }
	    if (device_ptr->req_sense_buf.add_sense_key == 0x70) {
		/*  For the 4mm drives only, check to see if we have
		 *  a decompression error.  Indicates a media problem.
		 */
		err_record.error_id = ERRID_TAPE_ERR1;
		strlog(device_ptr,cmd_ptr,&err_record,(uchar) TAPE_SCSI_ERROR);
		errsave(&err_record, sizeof(struct sc_error_log_df));
		cmd_ptr->scbuf.bufstruct.b_error = EMEDIA;
		return (TAPE_WAKEUP);
	    }
	}
	if (device_ptr->tape_ddi.dev_type == TAPE_8MM5GB) {
	    /*  Test for drive needing to be cleaned error. This
	     *  error comes from 30hrs of use without being
	     *  cleaned.
	     */
	    if (device_ptr->req_sense_buf.add_sense_key == 0 &&
		device_ptr->req_sense_buf.extended_byte1 == 0 &&
		device_ptr->req_sense_buf.extended_byte16 == 0xE8){
		   cmd_ptr->scbuf.bufstruct.b_flags &= ~B_ERROR;
		   cmd_ptr->scbuf.bufstruct.b_error = 0;
		   err_record.error_id = ERRID_TAPE_ERR6;
		   strlog(device_ptr, cmd_ptr, &err_record,
		       (uchar) TAPE_SCSI_ERROR);
		   errsave(&err_record, sizeof(struct sc_error_log_df));
		   rc = strreset(device_ptr, cmd_ptr, 1);
		   if (rc != 0) {
		   /* Return reset status.  If rc is not 0 the the re- */
		   /* set is in progress (= TAPE_NOWAKEUP).            */
		       return (rc);
		   }
		   return (TAPE_WAKEUP);
	    }
	    if (device_ptr->req_sense_buf.add_sense_key == 0 &&
		device_ptr->req_sense_buf.extended_byte1 == 0 &&
		device_ptr->req_sense_buf.extended_byte16 == 0xE9){
		   cmd_ptr->scbuf.bufstruct.b_flags &= ~B_ERROR;
		   cmd_ptr->scbuf.bufstruct.b_error = 0;
		   rc = strreset(device_ptr, cmd_ptr, 1);
		   if (rc != 0) {
		   /*  Return reset status.  If rc is not 0 the the re-
		    *  set is in progress (= TAPE_NOWAKEUP).
		    */
		       return (rc);
		   }
		   return (TAPE_WAKEUP);
	    }
	    /*  Test for compressed-uncompressed boundary error.
	     *  Since we only support one mode during a particlar
	     *  write, then if we hit this it must be a hardware
	     *  error.
	     */
	    if (device_ptr->req_sense_buf.add_sense_key == 0x70 &&
		device_ptr->req_sense_buf.extended_byte1 == 0 &&
		(device_ptr->req_sense_buf.extended_byte16 == 0x1A ||
		 device_ptr->req_sense_buf.extended_byte16 == 0x1B)){
		    cmd_ptr->scbuf.bufstruct.b_error = EIO;
		    err_record.error_id = ERRID_TAPE_ERR2;
		    strlog(device_ptr, cmd_ptr, &err_record,
			(uchar) TAPE_SCSI_ERROR);
		    errsave(&err_record, sizeof(struct sc_error_log_df));
		    return (TAPE_WAKEUP);
	    }
	}	/* endif dev_type == TAPE_8MM5GB */

	/*  If no sense value returned, then check for a tape mark
	 *  encountered.  Return said status to the issuing process.
	 */
	if ((device_ptr->req_sense_buf.sense_key & 0x80) != 0) {
	    cmd_ptr->tape_position = TAPE_FILEMARK_ERROR;
	    return (TAPE_WAKEUP);
	}
	/*  Beginning or end of tape may have been encountered.  If the
	 *  EOT bit is set in the sense data, then each device type's
	 *  sense data is checked further to see if the indication is
	 *  for beginning of tape encountered.  If so, return with a
	 *  status indicating such.
	 */
	if ((device_ptr->req_sense_buf.sense_key & 0x40) != 0) {
	    cmd_ptr->tape_position = TAPE_ENDOFTAPE_ERROR;
	    switch (device_ptr->tape_ddi.dev_type) {
	    case (TAPE_8MM):
		if ((device_ptr->req_sense_buf.extended_byte7 & 0x01) != 0) {
		    cmd_ptr->tape_position = TAPE_BEGOFTAPE_ERROR;
		}
	    case (TAPE_8MM5GB):
		if ((device_ptr->req_sense_buf.extended_byte7 & 0x01) != 0) {
		    cmd_ptr->tape_position = TAPE_BEGOFTAPE_ERROR;
		}
		break;
	    case (TAPE_9TRACK):
		if ((device_ptr->req_sense_buf.extended_byte8 & 0x80) != 0) {
		    cmd_ptr->tape_position = TAPE_BEGOFTAPE_ERROR;
		}
		break;
	    case (TAPE_QUARTER):
	    case (TAPE_QIC525):
	    case (TAPE_QIC1200):
		if (device_ptr->req_sense_buf.extended_byte1 == 0x42) {
		    cmd_ptr->tape_position = TAPE_BEGOFTAPE_ERROR;
		}
		break;
	    case (TAPE_4MM2GB):
	    case (TAPE_4MM4GB):
		if ((device_ptr->req_sense_buf.add_sense_key == 0x00) &&
	            (device_ptr->req_sense_buf.extended_byte1 == 0x04)) {
		    /* The 4MM drive may detect beginning of tape this way */
	            cmd_ptr->tape_position = TAPE_BEGOFTAPE_ERROR;
	            return (TAPE_WAKEUP);
		}
	    default:
	    /* Should not occur.   */
		break;
	    }
	    return (TAPE_WAKEUP);
	}
	/*  If no sense is received with there being a filemark or EOT
	 *  indicated, and the illegal length bit is set, then a check
	 *  is made to see if variable length records are being used.
	 *  This is not illegal when received using variable length
	 *  reads but is an error if using fixed length records during
	 *  reads.
	 */
	if ((device_ptr->req_sense_buf.sense_key & 0x20) != 0) {
	    if (device_ptr->tape_ddi.blocksize == 0) {
		cmd_ptr->scbuf.bufstruct.b_flags &= ~B_ERROR;
		if (resid < 0) {
		    if ((device_ptr->flags2 & TAPE_SHORTREAD_OK) == 0) {
			cmd_ptr->scbuf.bufstruct.b_error = ENOMEM;
		    }
		    else {
			cmd_ptr->scbuf.bufstruct.b_error = 0;
		    }
		}
		else {
		    cmd_ptr->scbuf.bufstruct.b_error = 0;
		}
	    }
	    cmd_ptr->tape_position = TAPE_ILI_ERROR;
	    return (TAPE_WAKEUP);
	}
	/*  If no indication of filemark, EOT, or illegal length and
	 *  the device type is an 8MM tape.  A check is made to see if
	 *  there are any blocks left (via sense data).  If not, the
	 *  tape is at or beyond EOT an an indication of such is re-
	 *  turned (this is for the 8MM tape ONLY).
	 */
	if ((device_ptr->tape_ddi.dev_type == TAPE_8MM) ||
	   (device_ptr->tape_ddi.dev_type == TAPE_8MM5GB)) {
	    blocks_remaining =
		((int) (device_ptr->req_sense_buf.extended_byte11 << 16)) |
		((int) (device_ptr->req_sense_buf.extended_byte12 << 8)) |
		((int) (device_ptr->req_sense_buf.extended_byte13));
	    /*  Test for no block left on read for 8mm.  This
	     *  indicates an end of tape condition.
	     */
	    if ((device_ptr->req_sense_buf.extended_byte11 == 0xFF) ||
		(blocks_remaining == 0)) {
		cmd_ptr->tape_position = TAPE_ENDOFTAPE_ERROR;
		return (TAPE_WAKEUP);
	    }
	}
	if (device_ptr->tape_ddi.dev_type == TAPE_QUARTER) {
	    /*  Test for no block left on read.  This is indi-
	     *  cated as a tape filemark error for this drive
	     *  type.
	     */
	    if ((device_ptr->req_sense_buf.add_sense_key == 0x34) &&
		(device_ptr->req_sense_buf.extended_byte1 == 0x17)) {
		cmd_ptr->tape_position = TAPE_ENDOFDATA_ERROR;
		return (TAPE_WAKEUP);
	    }
	}
	if ((device_ptr->tape_ddi.dev_type == TAPE_QIC525) ||
	    (device_ptr->tape_ddi.dev_type == TAPE_QIC1200)) {
	    /*  Test for no block left on read.  This is indi-
	     *  cated as a tape filemark error for this drive
	     *  type.
	     */
	    if (device_ptr->req_sense_buf.extended_byte1 == 0x17) {
		cmd_ptr->tape_position = TAPE_ENDOFDATA_ERROR;
		return (TAPE_WAKEUP);
	    }
	}
	/*  If no indication of filemark, EOT, or illegal length then,
	 *  if possible, retry the command.  Else return an error and
	 *  log it's occurrence.
	 */
	if ((cmd_ptr->retry_flag) &&
	   (cmd_ptr->retry_count < TAPE_MAX_RETRY)) {
	    cmd_ptr->scbuf.bufstruct.b_flags &= ~B_ERROR;
	    cmd_ptr->scbuf.bufstruct.b_error = 0;
	    cmd_ptr->retry_count++;
	    strstart(cmd_ptr);
	    return (TAPE_NOWAKEUP);
	}
	cmd_ptr->scbuf.bufstruct.b_error = EIO;
	/* Log the error  */
	err_record.error_id = ERRID_TAPE_ERR5;
	strlog(device_ptr, cmd_ptr, &err_record, (uchar) TAPE_SCSI_ERROR);
	errsave(&err_record, sizeof(struct sc_error_log_df));
	return (TAPE_WAKEUP);
    case (TAPE_BLANK_CHECK): /* Blank tape is encountered */
	/*  Check for a tape filemark being encountered.  If so, return
	 *  said status to the issuing process.
	 */
	if ((device_ptr->req_sense_buf.sense_key & 0x80) != 0) {
	    cmd_ptr->tape_position = TAPE_FILEMARK_ERROR;
	    return (TAPE_WAKEUP);
	}
	/*  Beginning or end of tape may have been encountered.  If the
	 *  EOT bit is set in the sense data, then each device type's
	 *  sense data is checked further to see if the indication is
	 *  for beginning of tape encountered.  If so, return with a
	 *  status indicating such.
	 */
	if ((device_ptr->req_sense_buf.sense_key & 0x40) != 0) {
	    cmd_ptr->tape_position = TAPE_ENDOFTAPE_ERROR;
	    switch (device_ptr->tape_ddi.dev_type) {
	    case (TAPE_8MM):
	    case (TAPE_8MM5GB):
		if ((device_ptr->req_sense_buf.extended_byte7 & 0x01) != 0) {
		    cmd_ptr->tape_position = TAPE_BEGOFTAPE_ERROR;
		}
		break;
	    case (TAPE_9TRACK):
		if ((device_ptr->req_sense_buf.extended_byte8 & 0x80) != 0) {
		    cmd_ptr->tape_position = TAPE_BEGOFTAPE_ERROR;
		}
		break;
	    case (TAPE_QUARTER):
	    case (TAPE_QIC525):
	    case (TAPE_QIC1200):
		if (device_ptr->req_sense_buf.extended_byte1 == 0x42) {
		    cmd_ptr->tape_position = TAPE_BEGOFTAPE_ERROR;
		}
		break;
	    default:
		break;
	    }
	    return (TAPE_WAKEUP);
	}
	if ((device_ptr->req_sense_buf.sense_key & 0x20) != 0) {
	/*  If blank tape received with there being a filemark or EOT
	 *  indicated, and the illegal length bit is set, then a check
	 *  is made to see if variable length records are being used.
	 *  This is not illegal when received using variable length
	 *  reads but is an error if using fixed length records during
	 *  reads.
	 */
	    cmd_ptr->tape_position = TAPE_ILI_ERROR;
	    if (device_ptr->tape_ddi.blocksize == 0) {
		cmd_ptr->scbuf.bufstruct.b_flags &= ~B_ERROR;
		if (resid < 0)
		    cmd_ptr->scbuf.bufstruct.b_error = ENOMEM;
		else
		    cmd_ptr->scbuf.bufstruct.b_error = 0;
	    }
	    return (TAPE_WAKEUP);
	}
	/*  If no indication of filemark, EOT, or illegal length and
	 *  the device type is an 8MM tape.  A check is made to see if
	 *  there are any blocks left (via sense data).  If not, the
	 *  tape is at or beyond EOT an an indication of such is re-
	 *  turned (this is for the 8MM tape ONLY).
	 */
	if ((device_ptr->tape_ddi.dev_type == TAPE_8MM) ||
	   (device_ptr->tape_ddi.dev_type == TAPE_8MM5GB)) {
	    blocks_remaining = ((int)
		(device_ptr->req_sense_buf.extended_byte11 << 16)) |
		((int) (device_ptr->req_sense_buf.extended_byte12 << 8)) |
		((int) (device_ptr->req_sense_buf.extended_byte13));
	    /*  Test for no block left on read for 8mm.  This
	     *  indicates an end of tape condition.
	     */
	    if ((device_ptr->req_sense_buf.extended_byte11 == 0xFF) ||
		(blocks_remaining == 0)) {
		cmd_ptr->tape_position = TAPE_ENDOFTAPE_ERROR;
		return (TAPE_WAKEUP);
	    }
	}
	cmd_ptr->scbuf.bufstruct.b_error = EIO;
	return (TAPE_WAKEUP);
    case (TAPE_UNIT_ATTENTION):  /* A device reset has occurred */
	/*  If a unit attention is seen while trying to open the tape
	 *  device, a flag is set that will force the tape to BOT
	 *  before the open completes.  This is for checking tape
	 *  characteristics (i.e. write protect).
	 */
	if (device_ptr->flags & TAPE_OPENING_DEVICE) {
	    device_ptr->flags2 |= TAPE_LOAD_REQUIRED;
	}
	/*  A reset has occurred, reset device charteristics and retry
	 *  the failing command (if it is retryable).
	 */
	if (device_ptr->cmd_state != TAPE_IO) {
	    rc = strreset(device_ptr, cmd_ptr, 0);
	    if (rc != 0) {
		return (rc);
	    }
	}
	device_ptr->flags |= TAPE_OFFLINE;
	cmd_ptr->scbuf.bufstruct.b_error = EIO;
	return (TAPE_WAKEUP);
    case (TAPE_NOT_READY):
	rc = strreset(device_ptr, cmd_ptr, 0);
	if (rc != 0) {
	    return (rc);
	}
	device_ptr->flags |= TAPE_OFFLINE;
	cmd_ptr->scbuf.bufstruct.b_error = ENOTREADY;
	return (TAPE_WAKEUP);
    case (TAPE_VOLUME_OVERFLOW):
	/* Test for Physical End Of Tape error */
	if ((device_ptr->tape_ddi.dev_type == TAPE_8MM5GB) ||
	    (device_ptr->tape_ddi.dev_type == TAPE_4MM2GB) ||
	    (device_ptr->tape_ddi.dev_type == TAPE_4MM4GB)) {
	    if (device_ptr->req_sense_buf.add_sense_key == 0 &&
		device_ptr->req_sense_buf.extended_byte1 == 0x02){
	           cmd_ptr->scbuf.bufstruct.b_error = EIO;
		   err_record.error_id = ERRID_TAPE_ERR2;
		   strlog(device_ptr, cmd_ptr, &err_record,
		       (uchar) TAPE_SCSI_ERROR);
		   errsave(&err_record, sizeof(struct sc_error_log_df));
	           return (TAPE_WAKEUP);
	    }
	}
	err_record.error_id = ERRID_TAPE_ERR5;
	strlog(device_ptr, cmd_ptr, &err_record, (uchar) TAPE_SCSI_ERROR);
	errsave(&err_record, sizeof(struct sc_error_log_df));
	cmd_ptr->tape_position = TAPE_OVERFLOW_ERROR;
	cmd_ptr->scbuf.bufstruct.b_error = EIO;
	return (TAPE_WAKEUP);
    case (TAPE_RECOVERED_ERROR):  /* only on 9-trck tape drives */
	/*  For the recovered error condition, a check is made to
	 *  determine is error thresholds are exceeded for reads
	 *  or writes.  An error is logged if so.
	 *  log error
	 */
	if (device_ptr->tape_ddi.dev_type == TAPE_9TRACK) {
	    if (device_ptr->operation == TAPE_READ_ACTIVE) {
		device_ptr->tape_error.read_recovered_error += 1;
		if (device_ptr->tape_error.read_recovered_error >=
		    device_ptr->tape_ddi.min_read_error) {
		    device_ptr->tape_error.read_recovered_error = 0;
		    err_record.error_id = ERRID_TAPE_ERR3;
		    strlog(device_ptr, cmd_ptr, &err_record,
			  (uchar) TAPE_SCSI_ERROR);
		    errsave(&err_record, sizeof(struct sc_error_log_df));
		}
	    }
	    if (device_ptr->operation == TAPE_WRITE_ACTIVE) {
		device_ptr->tape_error.write_recovered_error += 1;
		if (device_ptr->tape_error.write_recovered_error >=
		    device_ptr->tape_ddi.min_write_error) {
		    device_ptr->tape_error.write_recovered_error = 0;
		    err_record.error_id = ERRID_TAPE_ERR3;
		    strlog(device_ptr, cmd_ptr, &err_record,
			  (uchar) TAPE_SCSI_ERROR);
		    errsave(&err_record, sizeof(struct sc_error_log_df));
		}
	    }
	}
	/*  A check is made here for BOT, filemark encountered,
	 *  etc.  This is because even though a recovered error
	 *  has occurred, any one of these conditions may have
	 *  also occurred.  Set the appropriate flag for each
	 *  condition.
	 */
	if ((device_ptr->req_sense_buf.sense_key & 0x80) != 0) {
	    cmd_ptr->tape_position = TAPE_FILEMARK_ERROR;
	    return (TAPE_WAKEUP);
	}
	/*  Beginning or end of tape may have been encountered.  If the
	 *  EOT bit is set in the sense data, then each device type's
	 *  sense data is checked further to see if the indication is
	 *  for beginning of tape encountered.  If so, return with a
	 *  status indicating such.
	 */
	if ((device_ptr->req_sense_buf.sense_key & 0x40) != 0) {
	    cmd_ptr->tape_position = TAPE_ENDOFTAPE_ERROR;
	    if ((device_ptr->tape_ddi.dev_type == TAPE_9TRACK) &&
	       ((device_ptr->req_sense_buf.extended_byte8 & 0x80) != 0)) {
		cmd_ptr->tape_position = TAPE_BEGOFTAPE_ERROR;
	    }
	    return (TAPE_WAKEUP);
	}
	/*  If a recovered error is received and no other error is
	 *  indicated, and the illegal length bit is set, then a check
	 *  is made to see if variable length records are being used.
	 *  This is not illegal when received using variable length
	 *  reads but is an error if using fixed length records during
	 *  reads.
	 */
	if ((device_ptr->req_sense_buf.sense_key & 0x20) != 0) {
	    cmd_ptr->tape_position = TAPE_ILI_ERROR;
	    if (device_ptr->tape_ddi.blocksize == 0) {
		cmd_ptr->scbuf.bufstruct.b_flags &= ~B_ERROR;
		cmd_ptr->scbuf.bufstruct.b_error = 0;
	    }
	    return (TAPE_WAKEUP);
	}
	/*  Do not return an error to the user application for a
	 *  recovered (everything's ok).
	 */
	cmd_ptr->scbuf.bufstruct.b_error = 0;
	return (TAPE_WAKEUP);
    case (TAPE_MEDIUM_ERROR):
	/*  An error on the media has been encountered.  Retry
	 *  the command (if applicable) and log the error.
	 */
	device_ptr->tape_error.medium_error++;

	if (device_ptr->tape_ddi.dev_type == TAPE_8MM5GB) {
	    /*  Test for microcode loading error. This error  is
	     *  produced during microde download. Since this is
	     *  not supported except through diagnostics, log
	     *  a hardware error and return EIO.
	     */
	    if (device_ptr->req_sense_buf.add_sense_key == 0x26){
	        err_record.error_id = ERRID_TAPE_ERR2;
	        strlog(device_ptr, cmd_ptr, &err_record,
		    (uchar) TAPE_SCSI_ERROR);
	        errsave(&err_record, sizeof(struct sc_error_log_df));
	        cmd_ptr->scbuf.bufstruct.b_error = EIO;
	        return (TAPE_WAKEUP);
	    }
	}
	/* log a media error */
	err_record.error_id = ERRID_TAPE_ERR1;
	strlog(device_ptr, cmd_ptr, &err_record, (uchar) TAPE_SCSI_ERROR);
	errsave(&err_record, sizeof(struct sc_error_log_df));
	cmd_ptr->scbuf.bufstruct.b_error = EMEDIA;
	return (TAPE_WAKEUP);
    case (TAPE_HARDWARE_ERROR):
	/*  An error with the devices has been encountered. Retry
	 *  the command (if applicable) and log the error.
	 */
	device_ptr->tape_error.hardware_error++;
	/* setup command for retry */
	if ((cmd_ptr->retry_flag) &&
	   (cmd_ptr->retry_count < TAPE_MAX_RETRY)) {
	    cmd_ptr->scbuf.bufstruct.b_flags &= ~B_ERROR;
	    cmd_ptr->scbuf.bufstruct.b_error = 0;
	    cmd_ptr->retry_count++;
	    strstart(cmd_ptr);
	    return (TAPE_NOWAKEUP);
	}
	/* if no retry, then log a hardware error */
	err_record.error_id = ERRID_TAPE_ERR2;
	strlog(device_ptr, cmd_ptr, &err_record, (uchar) TAPE_SCSI_ERROR);
	errsave(&err_record, sizeof(struct sc_error_log_df));
	cmd_ptr->scbuf.bufstruct.b_error = EIO;
	return (TAPE_WAKEUP);
    case (TAPE_ABORTED_COMMAND):
	/*  An aborted error has occurred.  This may be due to
	 *  a parity or i/o bus error.  The command will be re-
	 *  tried (if possible) but reads/writes are not retried
	 *  as a rule (this is a hard failure).
	 */
	device_ptr->tape_error.aborted_cmd_error++;
	if ((cmd_ptr->retry_flag) &&
	   (cmd_ptr->retry_count < TAPE_MAX_RETRY)) {
	    cmd_ptr->scbuf.bufstruct.b_flags &= ~B_ERROR;
	    cmd_ptr->scbuf.bufstruct.b_error = 0;
	    cmd_ptr->retry_count++;
	    strstart(cmd_ptr);
	    return (TAPE_NOWAKEUP);
	}
	/* if no retry, then log a aborted command error */
	err_record.error_id = ERRID_TAPE_ERR2;
	strlog(device_ptr, cmd_ptr, &err_record, (uchar) TAPE_SCSI_ERROR);
	errsave(&err_record, sizeof(struct sc_error_log_df));
	cmd_ptr->scbuf.bufstruct.b_error = EIO;
	return (TAPE_WAKEUP);
    case (TAPE_ILLEGAL_REQUEST):
	/*  Indicates an illegal command was received.  No retry
	 *  for this command (the same status would be received
	 *  again).  Simply return the error.
	 */
	cmd_ptr->scbuf.bufstruct.b_error = EINVAL;
	return (TAPE_WAKEUP);
    case (TAPE_DATA_PROTECT):
	/*  A write, write filemarks, or erase command has been
	 *  attempted on write protected media.  Return in error.
	 */
	cmd_ptr->scbuf.bufstruct.b_error = EWRPROTECT;
	return (TAPE_WAKEUP);
    case (TAPE_VENDOR_UNIQUE):
	/* Test for bad filemark error */
	if (device_ptr->tape_ddi.dev_type == TAPE_8MM5GB &&
	    device_ptr->req_sense_buf.add_sense_key == 0x15) {
	    /* setup command for retry */
	    if ((cmd_ptr->retry_flag) &&
	       (cmd_ptr->retry_count < TAPE_MAX_RETRY)) {
	        cmd_ptr->scbuf.bufstruct.b_flags &= ~B_ERROR;
	        cmd_ptr->scbuf.bufstruct.b_error = 0;
	        cmd_ptr->retry_count++;
	        strstart(cmd_ptr);
	        return (TAPE_NOWAKEUP);
	    }
	    /* if no retry, then log a media error */
	    err_record.error_id = ERRID_TAPE_ERR1;
	    strlog(device_ptr, cmd_ptr, &err_record, (uchar) TAPE_SCSI_ERROR);
	    errsave(&err_record, sizeof(struct sc_error_log_df));
	    cmd_ptr->scbuf.bufstruct.b_error = EMEDIA;
	    return (TAPE_WAKEUP);
	}
    case (TAPE_COPY_ABORTED):
    case (TAPE_EQUAL_CMD):
    case (TAPE_MISCOMPARE):
	/*  This command are not supported/implemented with the
	 *  current tape device hardware.  If this point is hit
	 *  retries are not attempted.  Instead an error is re-
	 *  turned and an "unknown error" type is logged.  For
	 *  other scsi tapes, no log is done is this may be a
	 *  valid error depending on drive type.
	 */
	if (device_ptr->tape_ddi.dev_type != TAPE_OTHER) {
	    err_record.error_id = ERRID_TAPE_ERR5;
	    strlog(device_ptr, cmd_ptr, &err_record, (uchar) TAPE_SCSI_ERROR);
	    errsave(&err_record, sizeof(struct sc_error_log_df));
	}
	cmd_ptr->scbuf.bufstruct.b_error = EIO;
	return (TAPE_WAKEUP);
    default:
	/*  Should never encounter the default (physically impos-
	 *  sible as the switch statement cover all possible
	 *  cases for a nibble.  If this point is reached return
	 *  error.
	 */
	cmd_ptr->scbuf.bufstruct.b_error = EIO;
	break;
    }
    return (TAPE_WAKEUP);
}


/**************************************************************************/
/*                                                                        */
/* NAME:  strprocess_scsi_error                                           */
/*                                                                        */
/* FUNCTION:  Tape Device Driver error routine.                           */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by an interrupt handler and must be    */
/*      pinned to prevent a page fault.                                   */
/*                                                                        */
/* NOTES:  This routine is called by strerror when scsi status indicates  */
/*         a value other than a check condition. Retries are done only    */
/*         if the device indicates a busy condition, otherwise, this      */
/*         routine sets up the proper errno in the command structure.     */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    disk structure                  disk information structure          */
/*                                                                        */
/* INPUTS:                                                                */
/*    cmd_ptr - pointer to the tape command structure.                    */
/*    dev_ptr - pointer to the the device information structure.          */
/*                                                                        */
/* RETURN VALUE DESCRIPTION: TAPE_WAKEUP   - Informs striodone to issue   */
/*                                           the e_wakeup on return.      */
/*                           TAPE_NOWAKEUP - Informs striodone to do a    */
/*                                           simple return.               */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  none                                      */
/*                                                                        */
/**************************************************************************/
int
strprocess_scsi_error(
    struct tape_device_df *device_ptr,
    struct tape_cmd *cmd_ptr)

{

    int             rc;

    /*  This point was reached because a scsi bus status other
     *  than a check condition was received.  The status is checked
     *  and the appropriate action is taken.
     */
    switch (cmd_ptr->scbuf.scsi_status & SCSI_STATUS_MASK) {
    case (SC_BUSY_STATUS):
	/*  The drive is busy.  Attempt to reset drive parameters
	 *  and retry the command.  The deive is reset here be-
	 *  a busy stats is returned for some drive after a scsi
	 *  bus reset has been detected.
	 */
	if (cmd_ptr->retry_flag)
	    rc = strreset(device_ptr, cmd_ptr, 0);
	else
	    rc = strreset(device_ptr, cmd_ptr, 2);
	if (rc != 0) {
	    /*  Return reset status.  If rc is not 0 the the re-
	     *  set is in progress (= TAPE_NOWAKEUP).
	     */
	    return (rc);
	}
	/*  Reset failed or retires exceed.  Return a device busy
	 *  error.
	 */
	cmd_ptr->scbuf.bufstruct.b_error = EBUSY;
	break;
    case (SC_RESERVATION_CONFLICT):
	/*  No retry here.  If the device is reserved by another
	 *  initiator, the race is assumed lost.  Return a device
	 *  busy error.
	 */
	cmd_ptr->scbuf.bufstruct.b_error = EBUSY;
	break;
    case (SC_CHECK_CONDITION):
    case (SC_GOOD_STATUS):
    case (SC_INTMD_GOOD):
	/*  This case(s) should not occur.  Either a code problem
	 *  or device error is indicated.
	 */
	cmd_ptr->scbuf.bufstruct.b_error = EIO;
	break;
    default:
	/*  This case(s) should not occur.  Either a code problem
	 *  or device error is indicated.
	 */
	cmd_ptr->scbuf.bufstruct.b_error = EIO;
	break;
    }
    /*  If this point is reached, an error is already marked and
     *  is returned to the calling application.
     */
    return (TAPE_WAKEUP);
}


/**************************************************************************/
/*                                                                        */
/* NAME:  strreset                                                        */
/*                                                                        */
/* FUNCTION:  Tape Device Driver error routine.                           */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by an interrupt handler and must be    */
/*      pinned to prevent a page fault.                                   */
/*                                                                        */
/* NOTES:  This routine is called by the error process routines to perform*/
/*         a reset of drive parameters if required.  Timers are used to   */
/*         delay between occurrences.                                     */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    disk structure                  disk information structure          */
/*                                                                        */
/* INPUTS:                                                                */
/*    cmd_ptr - pointer to the tape command structure.                    */
/*    dev_ptr - pointer to the the device information structure.          */
/*                                                                        */
/* RETURN VALUE DESCRIPTION: TAPE_WAKEUP   - Informs striodone to issue   */
/*                                           the e_wakeup on return.      */
/*                           TAPE_NOWAKEUP - Informs striodone to do a    */
/*                                           simple return.               */
/*                           0 - Retries are exhausted (no action taken). */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  errsav                                    */
/*                              w_start                                   */
/*                                                                        */
/**************************************************************************/
strreset(
    struct tape_device_df *device_ptr,
    struct tape_cmd *cmd_ptr,
    int open_seq_only)

{

    /*  This routine handles resetting drive parameter when
     *  an error such as a scsi bus reset or bus device reset
     *  occurs.  The device (i.e. mode select and reservation)
     *  parameters must be reset.
     */

    /*  There is an scbuf reserved for resetting drive paramters
     *  on error.  A check is made here to determine if the error
     *  occurred during the open or a reset for a provious com-
     *  mand is already in progress.  If so, then the reserved
     *  scbuf is not addressed directly.  Instead, the command
     *  buffer that was passed into this routine is used.
     */
    if ((device_ptr->flags & TAPE_OPENING_DEVICE) ||
	(device_ptr->flags2 & TAPE_RESET_PARAMS) ||
	(cmd_ptr->type == TAPE_RESET_CMD)) {
	/*  If retries not exhausted, then rebuild a test unit
	 *  ready command and start the reset sequence (test unit
	 *  ready, reserve, mode sense, mode select) from the
	 *  beginning.
	 */
	if (cmd_ptr->retry_count < TAPE_MAX_RETRY) {
	    cmd_ptr->scbuf.bufstruct.b_flags &= ~B_ERROR;
	    cmd_ptr->scbuf.bufstruct.b_error = 0;
	    cmd_ptr->retry_count++;
	    device_ptr->cmd_state = TAPE_TUR;
	    str_general(device_ptr, cmd_ptr,
		       (uchar) SCSI_TEST_UNIT_READY, (uchar) 0x0,
		       (uchar) SC_RESUME);
	    /* If 1st retry, do not wait, issue immediately.    */
	    if (cmd_ptr->retry_count == 1) {
		strstart(cmd_ptr);
	    }
	    /*  Not 1st retry, wait a few seconds to give the
	     *  device a chance to recover form what could be a
	     *  bus device or scsi bus reset.
	     */
	    else {
		cmd_ptr->last_state = TAPE_TUR;
		strpush(device_ptr, cmd_ptr);
		w_start(&device_ptr->tape_watchdog_ptr.watch_timer);
	    }
	    /* Do not wakeup, retry in progress.                */
	    return (TAPE_NOWAKEUP);
	}
    }
    /*  A reset or open sequence is NOT in progress.  That means
     *  current command pointer is saved (for reissuance after
     *  the reset completes) and the aforementioned reserved
     *  scbuf is used to build a test unit ready command and
     *  start the reset sequence.
     */
    else if (open_seq_only == 0) {
	/*  If retries are not exhausted, then save the current
	 *  command and start the reset sequence.
	 */
	if ((cmd_ptr->retry_flag) &&
	   (cmd_ptr->retry_count < TAPE_MAX_RETRY)) {
	    cmd_ptr->scbuf.bufstruct.b_flags &= ~B_ERROR;
	    cmd_ptr->scbuf.bufstruct.b_error = 0;
	    cmd_ptr->retry_count++;
	    cmd_ptr->last_state = device_ptr->cmd_state;
	    strpush(device_ptr, cmd_ptr);
	    device_ptr->reset_cmd_ptr.retry_flag = TRUE;
	    device_ptr->reset_cmd_ptr.retry_count = 0;
	    device_ptr->reset_cmd_ptr.type = TAPE_RESET_CMD;
	    device_ptr->cmd_state = TAPE_TUR;
	    str_general(device_ptr, &device_ptr->reset_cmd_ptr,
		       (uchar) SCSI_TEST_UNIT_READY, (uchar) 0x0,
		       (uchar) SC_RESUME);
	    /*  Not 1st retry, wait a few seconds to give the
	     *  device a chance to recover form what could be a
	     *  bus device or scsi bus reset.
	     */
	    if (cmd_ptr->retry_count == 1) {
		strstart(&device_ptr->reset_cmd_ptr);
	    }
	    else {
		device_ptr->reset_cmd_ptr.last_state = TAPE_TUR;
		strpush(device_ptr, &device_ptr->reset_cmd_ptr);
		w_start(&device_ptr->tape_watchdog_ptr.watch_timer);
	    }
	    /* Do not wakeup, retry in progress.                */
	    return (TAPE_NOWAKEUP);
	}
    }
    else if (open_seq_only == 2) {
	/*  If retries are not exhausted, then save the current
	 *  command and start the reset sequence.
	 */
	if (cmd_ptr->retry_count < TAPE_MAX_RETRY) {
	    cmd_ptr->scbuf.bufstruct.b_flags &= ~B_ERROR;
	    cmd_ptr->scbuf.bufstruct.b_error = 0;
	    cmd_ptr->retry_count++;
	    /*  Not 1st retry, wait a few seconds to give the
	     *  device a chance to recover form what could be a
	     *  bus device or scsi bus reset.
	     */
	    if (cmd_ptr->retry_count == 1) {
		strstart(cmd_ptr);
	    }
	    else {
		strpush(device_ptr, cmd_ptr);
		w_start(&device_ptr->tape_watchdog_ptr.watch_timer);
	    }
	    /* Do not wakeup, retry in progress.                */
	    return (TAPE_NOWAKEUP);
	}
    }
    /*  No reset could be performed.  Return to the calling
     *  routine where error processing will be completed.
     */
    return (0);
}

/**************************************************************************/
/*                                                                        */
/*  What follows is a series of utility routines to set up commands, log  */
/*  errors, etc.                                                          */
/*                                                                        */
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*      strbldcmd: initializes the command block for a read/write command.*/
/*                                                                        */
/**************************************************************************/
void
strbldcmd(
    struct tape_device_df *device_ptr,
    struct tape_cmd *cmd_ptr,
    uchar           direction,
    uchar           resume_flag)

{
    struct buf     *cmd_bp;
    struct scsi    *scsi;

    /*  Since while reading/writing the maximum transfer rate
     *  may cause user buffer to be split into several transfers,
     *  an initial call is made to this routine to set up fields
     *  that will reamin static in the scbuf across the length of
     *  the read/write.  This prevents time being wasted resetting
     *  constant fields.
     */
    cmd_bp = &(cmd_ptr->scbuf.bufstruct);
    scsi = &(cmd_ptr->scbuf.scsi_command);
    /* setup common flags for io to the device */
    cmd_ptr->retry_flag = FALSE;
    cmd_ptr->type = TAPE_OTHER_CMD;

    /* setup buffer for call to SCSI device driver */
    cmd_ptr->scbuf.flags = resume_flag;
    cmd_ptr->scbuf.bp = NULL;
    cmd_bp->b_work = 0;
    cmd_bp->b_error = 0;
    cmd_bp->b_iodone = (void (*)()) striodone;
    cmd_bp->b_dev = device_ptr->tape_ddi.adapter_devno;

    /* setup scsi command for call to SCSI device driver */
    scsi->scsi_id = device_ptr->tape_ddi.scsi_id;
    scsi->scsi_length = 6;
    /*  The async flag is used to facilitate attempting to use
     *  the device synchronously.  Initial attempts are for the
     *  adapter to attempt synchronous negotiation for the device
     *  If this fails then the flag is set to asynchronous and
     *  saved here.
     */
    scsi->flags = device_ptr->async_flag;
    /* setup transfer direction (passed in as a parameter) */
    scsi->scsi_cmd.scsi_op_code = direction;
    if (device_ptr->tape_ddi.blocksize != 0) {
	/*  If the blocksize is fixed, the set then "fixed bit"
	 *  on in the read/write scsi command.
	 */
	scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0x01 :
				((device_ptr->tape_ddi.lun_id << 5) | 0x01);
    }
    else {
	scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0 :
				(device_ptr->tape_ddi.lun_id << 5);
    }
    scsi->scsi_cmd.scsi_bytes[3] = 0;
    cmd_ptr->device_ptr = device_ptr;
    /*  More time is added to the timeout value depending on the blocksize
     *  Small blocksizes (i.e. 1, 2, ...) can take a lot of time even for
     *  transferring small amounts of data.   Thus, the division here is
     *  to maximize the amount of time given for very small blocksizes.
     */
    switch (device_ptr->tape_ddi.dev_type) {
    case (TAPE_9TRACK):
    case (TAPE_3490E):
	cmd_ptr->scbuf.timeout_value = 240 +
				       (1024/device_ptr->tape_ddi.blocksize);
	break;
    case (TAPE_8MM):
    case (TAPE_8MM5GB):
	cmd_ptr->scbuf.timeout_value = 240 +
				       (4096/device_ptr->tape_ddi.blocksize);
	break;
    case (TAPE_4MM2GB):
    case (TAPE_4MM4GB):
	cmd_ptr->scbuf.timeout_value = 900;
	break;
    case (TAPE_OTHER):
	cmd_ptr->scbuf.timeout_value = device_ptr->tape_ddi.readwrite;
	break;
    default:
	cmd_ptr->scbuf.timeout_value = 600;
	break;
    }
    return;
}

/**************************************************************************/
/*                                                                        */
/*      streadwrite: sets the buffer address and transfer length for      */
/*                   read/write command just before they are issued.      */
/*                                                                        */
/**************************************************************************/
void
strreadwrite(
    struct tape_device_df *device_ptr,
    struct tape_cmd *cmd_ptr,
    uchar           *buffer_addr,
    int             transfer_length,
    int             direction_flag)

{
    struct buf     *cmd_bp;
    struct scsi    *scsi;
    int            block_count;

    /*  This routine is called to setup up transfer direction,
     *  block address, transfer length, etc. in the scbuf structure
     *  and associated scsi command block for both reads and writes.
     */
    cmd_bp = &(cmd_ptr->scbuf.bufstruct);
    scsi = &(cmd_ptr->scbuf.scsi_command);
    cmd_bp->b_flags = direction_flag;
    cmd_bp->b_bcount = transfer_length;
    cmd_bp->b_un.b_addr = (char *) buffer_addr;
    /* setup scsi command for call to SCSI device driver */

    /*  For fixed blocksizes, the data buffer length is given in
     *  a count of blocks.  For variable, the count is given in
     *  bytes.
     */
    if (device_ptr->tape_ddi.blocksize != 0) { /* fixed blocksize */
	block_count = (transfer_length / device_ptr->tape_ddi.blocksize);
    }
    else {
	block_count = transfer_length;
    }
    scsi->scsi_cmd.scsi_bytes[0] = ((block_count >> 16) & 0xff);
    scsi->scsi_cmd.scsi_bytes[1] = ((block_count >> 8) & 0xff);
    scsi->scsi_cmd.scsi_bytes[2] = (block_count & 0xff);
    /*  tape_position is used to indicate when EOM, end of data,
     *  or a filemark has been encountered AFTER cmd complete.
     */
    cmd_ptr->tape_position = 0; /* clear tape location flags */
    return;
}

/**************************************************************************/
/*                                                                        */
/*      str_request_sense: issues the request sense command.              */
/*                                                                        */
/**************************************************************************/
void
str_request_sense(
    struct tape_device_df *device_ptr,
    struct tape_cmd *cmd_ptr,
    uchar           clear_flag,
    uchar           resume_flag)

{

    struct buf     *cmd_bp;
    struct scsi    *scsi;

    /*  An scbuf is initialized here to issue the request sense
     *  command to the tape device (normally during error pro-
     *  cessing).  255 bytes (the max. allowed) is always
     *  requested no matter which device is accessed.
     *  setup buffer for call to SCSI device driver
     */
    cmd_bp = &(cmd_ptr->scbuf.bufstruct);
    scsi = &(cmd_ptr->scbuf.scsi_command);
    cmd_ptr->scbuf.flags = resume_flag;
    cmd_ptr->scbuf.bp = NULL;
    cmd_bp->b_work = 0;
    cmd_bp->b_flags = B_READ;
    cmd_bp->b_error = 0;
    cmd_bp->b_bcount = 255;
    cmd_bp->b_un.b_addr = (caddr_t) & device_ptr->req_sense_buf;
    cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
    cmd_bp->b_iodone = (void (*)()) striodone;
    cmd_bp->b_dev = device_ptr->tape_ddi.adapter_devno;

    /* setup scsi command for call to SCSI device driver */
    scsi->scsi_id = device_ptr->tape_ddi.scsi_id;
    scsi->scsi_length = 6;
    scsi->flags = device_ptr->async_flag;
    scsi->scsi_cmd.scsi_op_code = SCSI_REQUEST_SENSE;
    scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0 :
			    (device_ptr->tape_ddi.lun_id << 5);
    scsi->scsi_cmd.scsi_bytes[0] = 0x00;
    scsi->scsi_cmd.scsi_bytes[1] = 0;
    scsi->scsi_cmd.scsi_bytes[2] = 255;
    scsi->scsi_cmd.scsi_bytes[3] = clear_flag;
    /* 2.5 minute timeout */
    cmd_ptr->scbuf.timeout_value = 0x90;
    cmd_ptr->device_ptr = device_ptr;
    return;
}

/**************************************************************************/
/*                                                                        */
/*      str_general: issues either the test unit ready, reserve, release, */
/*                   or rewind command.  Can also issue load command.     */
/*                                                                        */
/**************************************************************************/
void
str_general(
    struct tape_device_df *device_ptr,
    struct tape_cmd *cmd_ptr,
    uchar           command_type,
    uchar           immediate_bit,
    uchar           resume_flag)

{
    struct buf     *cmd_bp;
    struct scsi    *scsi;

    /*  This routine builds the scbuf for the aforementioned com-
     *  mands.  None of these commands involve any data transfer
     *  so they are incorporated here into a single routine.
     *  setup buffer for call to SCSI device driver
     */
    cmd_bp = &(cmd_ptr->scbuf.bufstruct);
    scsi = &(cmd_ptr->scbuf.scsi_command);
    cmd_ptr->scbuf.flags = resume_flag;
    cmd_ptr->scbuf.bp = NULL;
    cmd_bp->b_work = 0;
    cmd_bp->b_error = 0;
    cmd_bp->b_flags = 0;
    cmd_bp->b_bcount = 0;
    cmd_bp->b_un.b_addr = 0;
    cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
    cmd_bp->b_iodone = (void (*)()) striodone;
    cmd_bp->b_dev = device_ptr->tape_ddi.adapter_devno;

    /* setup scsi command for call to SCSI device driver */
    scsi->scsi_id = device_ptr->tape_ddi.scsi_id;
    scsi->scsi_length = 6;
    scsi->flags = device_ptr->async_flag;
    scsi->scsi_cmd.scsi_op_code = command_type;
    scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? immediate_bit :
			  ((device_ptr->tape_ddi.lun_id << 5 | immediate_bit));
    scsi->scsi_cmd.scsi_bytes[0] = 0;
    scsi->scsi_cmd.scsi_bytes[1] = 0;
    scsi->scsi_cmd.scsi_bytes[2] = 0;
    scsi->scsi_cmd.scsi_bytes[3] = 0;
    cmd_ptr->tape_position = 0;	/* clear location flags */
    cmd_ptr->device_ptr = device_ptr;
    /* Allow a greater time for a rewind command  */
    if (command_type == SCSI_REWIND || command_type == SCSI_LOAD) {
	cmd_ptr->scbuf.timeout_value = 0x258;   /* 10 minutes */
    }
    else if (command_type == SCSI_RELEASE_UNIT) {
	if ((device_ptr->tape_ddi.dev_type == TAPE_QUARTER) ||
	    (device_ptr->tape_ddi.dev_type == TAPE_QIC525) ||
	    (device_ptr->tape_ddi.dev_type == TAPE_QIC1200) ||
	    (device_ptr->tape_ddi.dev_type == TAPE_OTHER)) {
	    cmd_ptr->scbuf.timeout_value = 0xd2;   /* 3.5 minutes */
	}
	else {
	    cmd_ptr->scbuf.timeout_value = 0x0a;
	}
    }
    else if (command_type == SCSI_TEST_UNIT_READY) {
	if (device_ptr->tape_ddi.dev_type == TAPE_3490E) {
	    cmd_ptr->scbuf.timeout_value = 0x12c;	/* 5 minutes */
	}
	else {
	    cmd_ptr->scbuf.timeout_value = 0x0a;
	}
    }
    else {
	cmd_ptr->scbuf.timeout_value = 0x0a;
    }
    return;
}

/**************************************************************************/
/*                                                                        */
/*      str_load: issues the load command for retentioning the tape       */
/*      (optional), having the cartridge loader insert the next cartridge */
/*      (optional), and bringing the tape to the load point.  i.e.,       */
/*      prepares the tape for reading or writing.                         */
/*                                                                        */
/**************************************************************************/
void
str_load(
    struct tape_device_df *device_ptr,
    struct tape_cmd *cmd_ptr,
    uchar           reten_req,
    uchar           resume_flag,
    uchar           load_flag)

{
    struct buf     *cmd_bp;
    struct scsi    *scsi;

    /*  This routine builds the scbuf for the load command.  This
     *  command is usually used to insure the tape is at BOT
     *  during open (and after a Unit Attention error is encountered)
     *  because some tape devices can't properly recognize a write
     *  protected tape or set mode parameters otherwise. This routine
     *  may also set RETENTION if invoked through a special file type or
     *  via an ioctl call.  Setup buffer for call to SCSI device driver.
     */
    cmd_bp = &(cmd_ptr->scbuf.bufstruct);
    scsi = &(cmd_ptr->scbuf.scsi_command);
    cmd_ptr->scbuf.flags = resume_flag;
    cmd_ptr->scbuf.bp = NULL;
    cmd_bp->b_work = 0;
    cmd_bp->b_flags = 0;
    cmd_bp->b_error = 0;
    cmd_bp->b_bcount = 0;
    cmd_bp->b_un.b_addr = 0;
    cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
    cmd_bp->b_iodone = (void (*)()) striodone;
    cmd_bp->b_dev = device_ptr->tape_ddi.adapter_devno;

    /* setup scsi command for call to SCSI device driver */
    scsi->scsi_id = device_ptr->tape_ddi.scsi_id;
    scsi->scsi_length = 6;
    scsi->flags = device_ptr->async_flag;
    scsi->scsi_cmd.scsi_op_code = SCSI_LOAD;
    scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0 :
			    (device_ptr->tape_ddi.lun_id << 5);
    scsi->scsi_cmd.scsi_bytes[0] = 0;
    scsi->scsi_cmd.scsi_bytes[1] = 0;
    /* Only the 1/4 inch tape drive truly supports the retension
     * function.  The other drives just cause the tape to be
     * rewound.
     */
    scsi->scsi_cmd.scsi_bytes[2] = load_flag;
    if (reten_req != 0) {
	if ((device_ptr->tape_ddi.dev_type == TAPE_QUARTER) ||
            (device_ptr->tape_ddi.dev_type == TAPE_QIC525) ||
            (device_ptr->tape_ddi.dev_type == TAPE_QIC1200) ||
	    (device_ptr->tape_ddi.dev_type == TAPE_OTHER)) {
	    scsi->scsi_cmd.scsi_bytes[2] |= 0x02; /* retention bit set */
	}
    }
    scsi->scsi_cmd.scsi_bytes[3] = 0;
    cmd_ptr->tape_position = 0;	/* clear location flags */
    cmd_ptr->device_ptr = device_ptr;
    /* 10 minutes allowed for a load which should be (max.) no
     * longer than it takes to retention a tape.
     */
    cmd_ptr->scbuf.timeout_value = 0x258;
    return;
}

/**************************************************************************/
/*                                                                        */
/*      str_mode_sense: issues the mode sense command.                    */
/*                                                                        */
/**************************************************************************/
void
str_mode_sense(
    struct tape_device_df *device_ptr,
    struct tape_cmd *cmd_ptr,
    uchar           resume_flag)

{
    struct buf     *cmd_bp;
    struct scsi    *scsi;

    /*  This routine builds the scbuf for the mode sense command.
     *  Mode sense data enables the tape open routine to deter-
     *  mine if the tape is write protected.  Other than that,
     *  this routine is not necessary for the correct function
     *  of the code but exists for easy expandability if/when the
     *  need arises to check drive default parameters.
     *  setup buffer for call to the SCSI device driver.
     */
    cmd_bp = &(cmd_ptr->scbuf.bufstruct);
    scsi = &(cmd_ptr->scbuf.scsi_command);
    cmd_ptr->scbuf.flags = resume_flag;
    cmd_ptr->scbuf.bp = NULL;
    cmd_bp->b_work = 0;
    cmd_bp->b_error = 0;
    cmd_bp->b_flags = B_READ;

    /*  Attempt to sense as much data as will be used in the mode
     *  select command.
     */
    cmd_bp->b_bcount = device_ptr->tape_ddi.mode_data_length;
    cmd_bp->b_un.b_addr = (caddr_t) (device_ptr->mode_buf);
    cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
    cmd_bp->b_iodone = (void (*)()) striodone;
    cmd_bp->b_dev = device_ptr->tape_ddi.adapter_devno;

    /*  setup scsi command for call to SCSI device driver */
    scsi->scsi_id = device_ptr->tape_ddi.scsi_id;
    scsi->scsi_length = 6;
    scsi->flags = device_ptr->async_flag;
    scsi->scsi_cmd.scsi_op_code = SCSI_MODE_SENSE;

    /*  Some devices support the PF bit set on for the mode
     *  sense command.  The other devices do not.
     */
    if (device_ptr->tape_ddi.dev_type == TAPE_OTHER) {
	scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 
				device_ptr->tape_page_supsen :
				((device_ptr->tape_ddi.lun_id << 5) | 
				 device_ptr->tape_page_supsen);
	scsi->scsi_cmd.scsi_bytes[0] = device_ptr->bytes_requested;
    }
    else if (device_ptr->tape_ddi.dev_type == TAPE_8MM) {
	scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0x10 :
				((device_ptr->tape_ddi.lun_id << 5) | 0x10);
	scsi->scsi_cmd.scsi_bytes[0] = 0x3f;
    }
    else if (device_ptr->tape_ddi.dev_type == TAPE_8MM5GB) {
	scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0 :
				(device_ptr->tape_ddi.lun_id << 5);
	scsi->scsi_cmd.scsi_bytes[0] = 0x3f;
	cmd_bp->b_bcount = 0xff;
	scsi->scsi_cmd.scsi_bytes[2] = 0xff;
    }
    /* For these devices, we only want page 20                        */
    else if ((device_ptr->tape_ddi.dev_type == TAPE_QIC525) ||
	     (device_ptr->tape_ddi.dev_type == TAPE_QIC1200)) {
	scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0 :
				(device_ptr->tape_ddi.lun_id << 5);
	scsi->scsi_cmd.scsi_bytes[0] = 0x20;
    }
    /* For this device, we need page 0x0f for compression info		*/
    else if (device_ptr->tape_ddi.dev_type == TAPE_4MM2GB) {
	scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0 :
				(device_ptr->tape_ddi.lun_id << 5);
	scsi->scsi_cmd.scsi_bytes[0] = 0x0f;
    }
    else if ((device_ptr->tape_ddi.dev_type == TAPE_3490E) ||
	     (device_ptr->tape_ddi.dev_type == TAPE_4MM4GB)) {
	scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0 :
				(device_ptr->tape_ddi.lun_id << 5);
	scsi->scsi_cmd.scsi_bytes[0] = 0x3f;
    }
    else {
	scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0 :
				(device_ptr->tape_ddi.lun_id << 5);
	scsi->scsi_cmd.scsi_bytes[0] = 0x00;
    }
    scsi->scsi_cmd.scsi_bytes[1] = 0;
    if (device_ptr->tape_ddi.dev_type != TAPE_8MM5GB) {
	scsi->scsi_cmd.scsi_bytes[2] =
	    device_ptr->tape_ddi.mode_data_length & 0x0ff;
    }
    scsi->scsi_cmd.scsi_bytes[3] = 0;
    cmd_ptr->tape_position = 0;	/* clear location flags */
    cmd_ptr->device_ptr = device_ptr;
    cmd_ptr->scbuf.timeout_value = 0x3c;   /* 1 minute */
    return;
}

/**************************************************************************/
/*                                                                        */
/*      str_mode_select: issues the mode select command.                  */
/*                                                                        */
/**************************************************************************/
void
str_mode_select(
    struct tape_device_df *device_ptr,
    struct tape_cmd *cmd_ptr,
    uchar           save_page,
    uchar           resume_flag)

{
    struct buf     *cmd_bp;
    struct scsi    *scsi;
    uint            dev;
    uchar           density;

    /*  This routine builds the scbuf for the mode select command.
     *  Most of the mode select data is passed to the tape
     *  dd when the device is configured.
     *
     *  setup buffer for call to SCSI device driver
     */
    cmd_bp = &(cmd_ptr->scbuf.bufstruct);
    scsi = &(cmd_ptr->scbuf.scsi_command);
    dev = minor(device_ptr->devno);
    cmd_ptr->scbuf.flags = resume_flag;
    cmd_ptr->scbuf.bp = NULL;
    cmd_bp->b_work = 0;
    cmd_bp->b_error = 0;
    cmd_bp->b_flags = B_WRITE;
    /* Passed in via dds during config. time. */
    cmd_bp->b_bcount = device_ptr->tape_ddi.mode_data_length;
    cmd_bp->b_un.b_addr = (caddr_t) (device_ptr->tape_ddi.mode_select_data);
    cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
    cmd_bp->b_iodone = (void (*)()) striodone;
    cmd_bp->b_dev = device_ptr->tape_ddi.adapter_devno;

    /* setup scsi command for call to SCSI device driver */
    scsi->scsi_id = device_ptr->tape_ddi.scsi_id;
    scsi->scsi_length = 6;
    scsi->flags = device_ptr->async_flag;
    scsi->scsi_cmd.scsi_op_code = SCSI_MODE_SELECT;
    /*  Some devices support the PF bit set on during mode
     *  sense.  The other devices do not.
     */
    if (device_ptr->tape_ddi.dev_type == TAPE_OTHER) {
	scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 
				device_ptr->tape_page_supsel :
				((device_ptr->tape_ddi.lun_id << 5) |
				   device_ptr->tape_page_supsel);
    }
    else if (device_ptr->tape_ddi.dev_type == TAPE_8MM ||
	     device_ptr->tape_ddi.dev_type == TAPE_8MM5GB ||
	     device_ptr->tape_ddi.dev_type == TAPE_4MM2GB ||
	     device_ptr->tape_ddi.dev_type == TAPE_4MM4GB ||
	     device_ptr->tape_ddi.dev_type == TAPE_3490E) { 
	scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0x10 :
				((device_ptr->tape_ddi.lun_id << 5) | 0x10);
    }
    else if (device_ptr->tape_ddi.dev_type == TAPE_QIC525 ||
	     device_ptr->tape_ddi.dev_type == TAPE_QIC1200) {
	/*  These devices put the Save Page parameter in the
	 *  same byte as the LUN.  Also, the PF field must be
	 *  set to 1.
         */
	if (save_page)
	    scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0x11 :
				    ((device_ptr->tape_ddi.lun_id << 5) | 0x11);
	else
	    scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0x10 :
				    ((device_ptr->tape_ddi.lun_id << 5) | 0x10);
    }
    else {
	scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0 :
				(device_ptr->tape_ddi.lun_id << 5);
    }
    scsi->scsi_cmd.scsi_bytes[0] = 0x00;
    scsi->scsi_cmd.scsi_bytes[1] = 0x00;
    scsi->scsi_cmd.scsi_bytes[2] =
	device_ptr->tape_ddi.mode_data_length & 0x0ff;
    if (device_ptr->tape_ddi.dev_type == TAPE_QUARTER) {
	/*  The 1/4 inch tape has a bit setting that allows for
	 *  the mode select data to be saved across power down/up
	 *  Because the retentioning may have been turned off, it
	 *  requires these parameters to be saved.
	 */
	if (save_page)
	    scsi->scsi_cmd.scsi_bytes[3] = 0x40;
	else
	    scsi->scsi_cmd.scsi_bytes[3] = 0;
    }
    else {
	scsi->scsi_cmd.scsi_bytes[3] = 0;
    }
    /*  It is determined here if the device is to use data xfer
     *  buffers during reading/writing.
     */
    if (device_ptr->tape_ddi.mode == TAPE_BUFFERED) {
	/* Set "buffered bit" on in mode select data. */
	device_ptr->tape_ddi.mode_select_data[2] = 0x10;
    }
    else {
	device_ptr->tape_ddi.mode_select_data[2] = 0;
    }
    /*  The density setting for tape drives comes from config
     *  parameters passed in via the dds.  They are user settable
     *  and allow for the user to chose what density goes with
     *  what special file.
     */
    if (dev & TAPE_DENSITY2) {
	device_ptr->tape_ddi.mode_select_data[4] =
		     device_ptr->tape_ddi.density_set2;
    }
    else {
	device_ptr->tape_ddi.mode_select_data[4] =
		     device_ptr->tape_ddi.density_set1;
    }
    /*  The 1/4 inch tape requires that the blocksize value in
     *  mode select data is always set to 512 (0x200).  Other
     *  drives allow this to be any positive value up to a spec-
     *  ified maximum.
     */
    if (device_ptr->tape_ddi.blocksize == 0) {
	device_ptr->tape_ddi.mode_select_data[9] =
	    ((device_ptr->tape_ddi.var_blocksize >> 16) & 0xff);
	device_ptr->tape_ddi.mode_select_data[10] =
	    ((device_ptr->tape_ddi.var_blocksize >> 8) & 0xff);
	device_ptr->tape_ddi.mode_select_data[11] =
	    (device_ptr->tape_ddi.var_blocksize & 0xff);
    }
    else {
	device_ptr->tape_ddi.mode_select_data[9] =
	    ((device_ptr->tape_ddi.blocksize >> 16) & 0xff);
	device_ptr->tape_ddi.mode_select_data[10] =
	    ((device_ptr->tape_ddi.blocksize >> 8) & 0xff);
	device_ptr->tape_ddi.mode_select_data[11] =
	    (device_ptr->tape_ddi.blocksize & 0xff);
    }
    /*  The 1/4 inch tape allows for setting retention during
     *  cassette replacement or reset.  This is a user settable
     *  parameter that is passed to the device.
     */
    if (device_ptr->tape_ddi.dev_type == TAPE_QUARTER) {
	device_ptr->tape_ddi.mode_select_data[22] =
	    device_ptr->tape_ddi.retention_flag;
    }
	/*  The 525MB and 1.2GB QIC allows setting retention during
	 *  cassette replacement or reset.  This is a user settable
	 *  parameter that is passed to the device.  It is on code
	 *  page 20, so we have to set that, too.
	 */
	if ((device_ptr->tape_ddi.dev_type == TAPE_QIC525) ||
	  (device_ptr->tape_ddi.dev_type == TAPE_QIC1200)) {
	  device_ptr->tape_ddi.mode_select_data[18] =
	    device_ptr->tape_ddi.retention_flag;
	}
    /*  Turn on compression bit if in 5GB compression mode, the
     *  default in the mode data is 0 so leave alone if not in
     *  compression mode.
     */
    if (device_ptr->tape_ddi.dev_type == TAPE_8MM5GB) {
	if (device_ptr->tape_ddi.compression_flag){
	    device_ptr->tape_ddi.mode_select_data[35] = 0x80;
	}
	else {
	    device_ptr->tape_ddi.mode_select_data[35] = 0x00;
	}
    }
    if (device_ptr->tape_ddi.dev_type == TAPE_4MM2GB) {
	if (device_ptr->tape_ddi.compression_flag){
	    device_ptr->tape_ddi.mode_select_data[14] = 0xc0;
	    device_ptr->tape_ddi.mode_select_data[15] = 0x80;
	}
	else {
	    device_ptr->tape_ddi.mode_select_data[14] = 0x40;
	    device_ptr->tape_ddi.mode_select_data[15] = 0x00;
	}
    }
    if (device_ptr->tape_ddi.dev_type == TAPE_4MM4GB) {
	if (device_ptr->tape_ddi.compression_flag){
	    device_ptr->tape_ddi.mode_select_data[74] = 0x80;
	    device_ptr->tape_ddi.mode_select_data[75] = 0x80;
	}
	else {
	    device_ptr->tape_ddi.mode_select_data[74] = 0x00;
	    device_ptr->tape_ddi.mode_select_data[75] = 0x00;
	}
    }
    if (device_ptr->tape_ddi.dev_type == TAPE_3490E) {
	if (device_ptr->tape_ddi.compression_flag){
	    device_ptr->tape_ddi.mode_select_data[50] = 0x01;
	}
	else {
	    device_ptr->tape_ddi.mode_select_data[50] = 0x00;
	}
    }
    cmd_ptr->tape_position = 0;	/* clear location flags */
    cmd_ptr->device_ptr = device_ptr;
    /*  10 minutes allowed for a load which should be (max.) no
     *  longer than it takes to retention a tape.  This is done
     *  because a reset may have occurred on the device and a
     *  mode select can occur only after the reset has returned
     *  the tape to BOT.
     */
    cmd_ptr->scbuf.timeout_value = 0x258;
    return;
}

/**************************************************************************/
/*                                                                        */
/*      str_erase: issues the erase command                               */
/*                                                                        */
/**************************************************************************/
void
str_erase(
    struct tape_device_df *device_ptr,
    struct tape_cmd *cmd_ptr,
    uchar           resume_flag)

{
    struct buf     *cmd_bp;
    struct scsi    *scsi;

    /*  This routine builds the scbuf for the erase command.
     *  setup buffer for call to SCSI device driver
     */
    cmd_bp = &(cmd_ptr->scbuf.bufstruct);
    scsi = &(cmd_ptr->scbuf.scsi_command);
    cmd_ptr->scbuf.flags = resume_flag;
    cmd_ptr->scbuf.bp = NULL;
    cmd_bp->b_work = 0;
    cmd_bp->b_bcount = 0;
    cmd_bp->b_flags = 0;
    cmd_bp->b_error = 0;
    cmd_bp->b_un.b_addr = 0;
    cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
    cmd_bp->b_iodone = (void (*)()) striodone;
    cmd_bp->b_dev = device_ptr->tape_ddi.adapter_devno;

    /* setup scsi command for call to SCSI device driver */
    scsi->scsi_id = device_ptr->tape_ddi.scsi_id;
    scsi->scsi_length = 6;
    scsi->flags = device_ptr->async_flag;
    scsi->scsi_cmd.scsi_op_code = SCSI_ERASE;
    scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0x01 :
			    ((device_ptr->tape_ddi.lun_id << 5) | 0x01);
    scsi->scsi_cmd.scsi_bytes[0] = 0;
    scsi->scsi_cmd.scsi_bytes[1] = 0;
    scsi->scsi_cmd.scsi_bytes[2] = 0;
    scsi->scsi_cmd.scsi_bytes[3] = 0;
    cmd_ptr->device_ptr = device_ptr;
    cmd_ptr->tape_position = 0;	/* clear location flags */
    /*  Times vary across devices based on the time required to
     *  write an entire tape.
     */
    switch (device_ptr->tape_ddi.dev_type) {
    case (TAPE_9TRACK):
    case (TAPE_3490E):
	cmd_ptr->scbuf.timeout_value = 0x258;
	break;
    case(TAPE_4MM2GB):
    case(TAPE_4MM4GB):
	cmd_ptr->scbuf.timeout_value = 0x4650;
	break;
    case(TAPE_8MM):
    case(TAPE_8MM5GB):
    case(TAPE_OTHER):
	/* worst case */
	cmd_ptr->scbuf.timeout_value = 0x2A30;
	break;
    default:
	cmd_ptr->scbuf.timeout_value = 0x834;
	break;
    }
    return;
}

/**************************************************************************/
/*                                                                        */
/*      str_write_filemarks: issues the write filemarks command           */
/*                                                                        */
/**************************************************************************/
void
str_write_filemarks(
    struct tape_device_df *device_ptr,
    struct tape_cmd *cmd_ptr,
    daddr_t         num_filemarks,
    uchar           resume_flag)

{
    struct buf     *cmd_bp;
    struct scsi    *scsi;

    /*  This routine builds the scbuf that issues the write file-
     *  marks command to the drive.  No data transfer occurs with
     *  this command.
     *  setup buffer for call to SCSI device driver
     */
    cmd_bp = &(cmd_ptr->scbuf.bufstruct);
    scsi = &(cmd_ptr->scbuf.scsi_command);
    cmd_ptr->scbuf.flags = resume_flag;
    cmd_ptr->scbuf.bp = NULL;
    cmd_bp->b_work = 0;
    cmd_bp->b_flags = 0;
    cmd_bp->b_bcount = 0;
    cmd_bp->b_error = 0;
    cmd_bp->b_un.b_addr = 0;
    cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
    cmd_bp->b_iodone = (void (*)()) striodone;
    cmd_bp->b_dev = device_ptr->tape_ddi.adapter_devno;

    /* setup scsi command for call to SCSI device driver */
    scsi->scsi_id = device_ptr->tape_ddi.scsi_id;
    scsi->scsi_length = 6;
    scsi->flags = device_ptr->async_flag;
    scsi->scsi_cmd.scsi_op_code = SCSI_WRITE_FILEMARK;
    scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0 :
			    (device_ptr->tape_ddi.lun_id << 5);
    scsi->scsi_cmd.scsi_bytes[0] = ((num_filemarks >> 16) & 0xff);
    scsi->scsi_cmd.scsi_bytes[1] = ((num_filemarks >> 8) & 0xff);
    scsi->scsi_cmd.scsi_bytes[2] = (num_filemarks & 0xff);
    /*  1/4 inch tape drives support writing filemarks in stream-
     *  ing mode.  A check is made here to determine if the tape
     *  device data buffer is used.  If so, a bit is set in the
     *  scsi command block that says write the filemarks to the
     *  data buffer (to allow streaming across filemarks).
     */
    if ((device_ptr->tape_ddi.dev_type == TAPE_QUARTER) &&
	(device_ptr->tape_ddi.mode == TAPE_BUFFERED)) {
	scsi->scsi_cmd.scsi_bytes[3] = 0x40;
    }
    /* Other tapes don't support streaming filemarks.           */
    else {
	scsi->scsi_cmd.scsi_bytes[3] = device_ptr->tape_ddi.extend_filemarks;
    }
    cmd_ptr->device_ptr = device_ptr;
    cmd_ptr->tape_position = 0; /* clear tape location flags */
    /*  Because an application can write an entire tape with file
     *  marks, enough time must be given to allow this.  So the
     *  timeout values are abnormally large.
     */
    switch (device_ptr->tape_ddi.dev_type) {
    case(TAPE_9TRACK):
    case(TAPE_3490E):
	cmd_ptr->scbuf.timeout_value = 240 + num_filemarks;
	if ((cmd_ptr->scbuf.timeout_value > 0x258) ||
	   (cmd_ptr->scbuf.timeout_value <= 0)) {
	    cmd_ptr->scbuf.timeout_value = 0x258;
	}
	break;
    case(TAPE_8MM):
    case(TAPE_8MM5GB):
    case(TAPE_OTHER):
	cmd_ptr->scbuf.timeout_value = 240 + (10*num_filemarks);
	break;
    case(TAPE_4MM2GB):
    case(TAPE_4MM4GB):
	cmd_ptr->scbuf.timeout_value = 480 + (10*num_filemarks);
	break;
    default:
	cmd_ptr->scbuf.timeout_value = 240 + num_filemarks;
	if ((cmd_ptr->scbuf.timeout_value > 0x834) ||
	   (cmd_ptr->scbuf.timeout_value <= 0)) {
	    cmd_ptr->scbuf.timeout_value = 0x834;
	}
	break;
    }
    return;
}

/**************************************************************************/
/*                                                                        */
/*      str_space: issues the space                                       */
/*                                                                        */
/**************************************************************************/
void
str_space(
    struct tape_device_df *device_ptr,
    struct tape_cmd *cmd_ptr,
    int             direction,
    int             type,
    daddr_t         count,
    uchar           resume_flag)

{
    struct buf     *cmd_bp;
    struct scsi    *scsi;
    uint            i;

    /*  This routine builds the scbuf to either forward or re-
     *  verse over records/filemarks.  The direction paramter
     *  provides for tape direction, the type parameter provides
     *  for spacing over record or filemarks, and the count para-
     *  meter tells how many records/filemarks to space over.
     *  setup buffer for call to SCSI device driver
     */
    cmd_bp = &(cmd_ptr->scbuf.bufstruct);
    scsi = &(cmd_ptr->scbuf.scsi_command);
    cmd_ptr->scbuf.flags = resume_flag;
    cmd_ptr->scbuf.bp = NULL;
    cmd_bp->b_work = 0;
    cmd_bp->b_flags = 0;
    cmd_bp->b_error = 0;
    cmd_bp->b_bcount = 0;
    cmd_bp->b_un.b_addr = 0;
    cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
    cmd_bp->b_iodone = (void (*)()) striodone;
    cmd_bp->b_dev = device_ptr->tape_ddi.adapter_devno;
    /* setup scsi command for call to SCSI device driver */
    scsi->scsi_id = device_ptr->tape_ddi.scsi_id;
    scsi->scsi_length = 6;
    scsi->scsi_cmd.scsi_op_code = SCSI_SPACE;
    /*  Sets up bit indicators in the scsi command depending on
     *  direction and type of space command.
     */
    if (type == TAPE_FILEMARK) {
	scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0x01 :
			    ((device_ptr->tape_ddi.lun_id << 5) | 0x01);
    }
    else {  /* spacing records */
	scsi->scsi_cmd.lun = (device_ptr->tape_ddi.lun_id > 7) ? 0 :
			    (device_ptr->tape_ddi.lun_id << 5);
    }
    i = count;
    if (direction == TAPE_REVERSE) {
	i = -i; /* negate count for reversing */
    }
    scsi->scsi_cmd.scsi_bytes[0] = ((i >> 16) & 0xff);
    scsi->scsi_cmd.scsi_bytes[1] = ((i >> 8) & 0xff);
    scsi->scsi_cmd.scsi_bytes[2] = (i & 0xff);
    scsi->scsi_cmd.scsi_bytes[3] = 0;
    cmd_ptr->device_ptr = device_ptr;
    cmd_ptr->tape_position = 0; /* clear tape location flags */
    /*  Because an application can skip over an entire tape (via
     *  read), enough time must be given to allow this.  So the
     *  timeout values are abnormally large.
     */
    switch (device_ptr->tape_ddi.dev_type) {
    case (TAPE_9TRACK):
    case (TAPE_3490E):
	cmd_ptr->scbuf.timeout_value = 0x258;
	break;
    case(TAPE_8MM):
    case(TAPE_8MM5GB):
    case(TAPE_OTHER):
	cmd_ptr->scbuf.timeout_value = 0x2A30;
	break;
    case(TAPE_4MM2GB):
    case(TAPE_4MM4GB):
	cmd_ptr->scbuf.timeout_value = 0x3C0;;
	break;
    case(TAPE_QIC525):
        cmd_ptr->scbuf.timeout_value = 0xCE4;
        break;
    case(TAPE_QIC1200):
        cmd_ptr->scbuf.timeout_value = 0x13EC;
        break;
    default:
	cmd_ptr->scbuf.timeout_value = 0x834;
	break;
    }

    /* The timeout value for some tape drives is too low, especially
     * when performing filemark advancement or reversal.  The values
     * above are hardcoded without regard to the number of filemarks
     * the space command must search for.  Thus, we will add timeout
     * values that reflect the additional time required to process
     * filemarks.  We pick an additional 2 seconds per filemark as an
     * appropriate bonus timeout value.  This may have to be changed
     * in the future
     */

    if (count < 0)
        count = -count;
    cmd_ptr->scbuf.timeout_value += (2 * count);

    return;
}

/**************************************************************************/
/*                                                                        */
/*      strpush: pushes a command pointer on the save stack.              */
/*                                                                        */
/**************************************************************************/
void
strpush(
    struct tape_device_df *device_ptr,
    struct tape_cmd *cmd_ptr)

{

    /*  This stack is maintained for the purpose of error pro-
     *  cessing.  When a command encounters an error and is pushed
     *  onto the stack and any required cleanup/error processing
     *  is done.  After, the command on the stack is popped and
     *  returned in error or retried.
     */
    device_ptr->stack_ptr[device_ptr->stackhead] = cmd_ptr;
    device_ptr->stackhead++;
    return;
}

/**************************************************************************/
/*                                                                        */
/*      strpop: pops a command pointer from the save stack and returns    */
/*              its value.                                                */
/*                                                                        */
/**************************************************************************/
struct tape_cmd
*strpop(struct tape_device_df *device_ptr)

{
    /* Pops the current command off stack used for error process*/
    device_ptr->stackhead--;
    return ((struct tape_cmd *)  /* this is the address of a tape_cmd */
	   (device_ptr->stack_ptr[device_ptr->stackhead]));
}

/**************************************************************************/
/*                                                                        */
/*      strstore : disables interrupts around a critical section of       */
/*                 code that saves a waiting command for iodone to        */
/*                 process.                                               */
/*                                                                        */
/**************************************************************************/
void
strstore(struct tape_device_df *device_ptr,
	 struct tape_cmd *cmd_ptr)
{
    int old_level;

    /* Disables interrupts for a critical section of code */

#ifdef _POWER_MP
    old_level = disable_lock(INTIODONE,&device_ptr->intr_lock);
    device_ptr->cmd_outstanding = cmd_ptr;
    unlock_enable(old_level,&device_ptr->intr_lock);
#else
    old_level = i_disable(INTIODONE);
    device_ptr->cmd_outstanding = cmd_ptr;
    i_enable(old_level);
#endif /* _POWER_MP */

    return;
}

/**************************************************************************/
/*                                                                        */
/*      strsend : disables interrupts around a critical section of        */
/*                code.  A check is mad here to see if iodone has         */
/*                finished before the next command was completely built.  */
/*                                                                        */
/**************************************************************************/
void
strsend(struct tape_device_df *device_ptr,
	struct tape_cmd *cmd_ptr)
{
    int old_level;

    /* Disables interrupts for a critical section of code */

#ifdef _POWER_MP
    old_level = disable_lock(INTIODONE,&device_ptr->intr_lock);
    if (device_ptr->cmd_outstanding != NULL) {
	       device_ptr->cmd_outstanding = NULL;
	       unlock_enable(old_level,&device_ptr->intr_lock);
	       strstart(cmd_ptr);
    }
    else {
	       unlock_enable(old_level,&device_ptr->intr_lock);
    }
#else
    old_level = i_disable(INTIODONE);
    if (device_ptr->cmd_outstanding != NULL) {
	device_ptr->cmd_outstanding = NULL;
	i_enable(old_level);
	strstart(cmd_ptr);
    }
    else {
	i_enable(old_level);
    }
#endif /* _POWER_MP */

    return;
}

/**************************************************************************/
/*                                                                        */
/*      strlog: builds the error log command structure.                   */
/*                                                                        */
/**************************************************************************/
void
strlog(
    struct tape_device_df *device_ptr,
    struct tape_cmd *cmd_ptr,
    struct sc_error_log_df *error_rec_ptr,
    uchar           error_type)
{
    /*  This routine fills the error log structure (sc_error_log)
     *  with information such as adapter status, scsi status,
     *  request sense information, etc.  Note that adapter errors
     *  do NOT contain request sense information.
     */
    bcopy(device_ptr->tape_ddi.resource_name, error_rec_ptr->resource_name,
	  sizeof(device_ptr->tape_ddi.resource_name));
    bcopy(&cmd_ptr->scbuf.scsi_command, &error_rec_ptr->scsi_command,
	  sizeof(struct scsi));
    error_rec_ptr->status_validity = cmd_ptr->scbuf.status_validity;
    error_rec_ptr->scsi_status = cmd_ptr->scbuf.scsi_status;
    error_rec_ptr->general_card_status = cmd_ptr->scbuf.general_card_status;
    /*  For adapter errors, clear the request sense information
     *  in the error log structure.
     */
    if (error_type == TAPE_ADAPTER_ERROR) {
	bzero(error_rec_ptr->req_sense_data, 128);
    }
    else {
	bcopy(&device_ptr->req_sense_buf, error_rec_ptr->req_sense_data, 128);
    }
    error_rec_ptr->reserved1 = 0;
    /* Transfer counts are maintained for problem determination.*/
    error_rec_ptr->reserved2 = device_ptr->read_xfer_count;
    error_rec_ptr->reserved3 = device_ptr->write_xfer_count;
    return;
}

/**************************************************************************/
/*									  */
/*   strwatchdog: This function is envoked when a watchdog timer expires  */
/*                due to waiting on a reset.                              */
/*									  */
/**************************************************************************/
void
strwatchdog(struct tape_watchdog *tape_watchdog_ptr)

{
    struct tape_device_df *device_ptr;
    struct tape_cmd *cmd_ptr;

    /*  Because, for most tape devices, issuing a command immediately
     *  after a bus device or scsi bus reset occurs gets  rejected,
     *  this routine is used to wait for a moment. This routine get
     *  control when the wait timer has expired and  will then issue
     *  the waiting command.  After the specified time, the device reset
     *  should have cleared.
     */
    device_ptr = tape_watchdog_ptr->device_ptr;
    cmd_ptr = strpop(device_ptr);
    device_ptr->cmd_state = cmd_ptr->last_state;  /* restore cmd state */
    strstart(cmd_ptr);     /* issue waiting command */
    return;
}

/**************************************************************************/
/*									  */
/*   strpmhandler: This function is envoked when a the power management   */
/*                 core determines that it is time to either begin/end a  */
/*		   power savings cycle.					  */
/*									  */
/**************************************************************************/
int
strpmhandler(caddr_t private, int mode)
{
    struct tape_pm_handle *tape_pm_ptr;
    struct tape_device_df *device_ptr;
    int	   rc;

    tape_pm_ptr = (struct tape_pm_handle *) private;
    if (tape_pm_ptr == NULL) {
	return(PM_ERROR);
    }

    device_ptr = tape_pm_ptr->device_ptr;

    rc = PM_SUCCESS;
    switch(mode) {
        case PM_DEVICE_FULL_ON:
	    if (tape_pm_ptr->pmh.mode & (PM_DEVICE_IDLE | PM_DEVICE_SUSPEND |
		PM_DEVICE_HIBERNATION)) {
		/* If you're returning from a idle, suspend or hibernate,*/
		/* then you should not be calling for PM_DEVICE_FULL_ON. */
        	rc = PM_ERROR;
	    }
	    else {
	        tape_pm_ptr->pmh.mode = PM_DEVICE_FULL_ON;
	    }
	    break;
        case PM_DEVICE_ENABLE:
	    rc = lockl(&device_ptr->lock_word, LOCK_NDELAY);
	    if (rc == LOCK_FAIL) {  /* another process has the device lock */
	    /* In this case, the tape device driver is open.  This means   */
	    /* that the open routine has itself moved the mode from the    */
	    /* idle to enabled state or is currently full on.  In any case,*/
	    /* it can't be guaranteed that we are currently in the enabled */
	    /* state so the call here will be failed.  However, if the de- */
	    /* vice is known to be PM_DEVICE_FULL_ON, then it's ok to go to*/
	    /* enabled, even though we can't obtain the device lock.       */
	        if (tape_pm_ptr->pmh.mode == PM_DEVICE_FULL_ON) {
	    	    tape_pm_ptr->pmh.mode = mode;
    		    rc = PM_SUCCESS;
	        }
	        break;
	    }
	    if (tape_pm_ptr->pmh.mode & (PM_DEVICE_IDLE | PM_DEVICE_SUSPEND |
		PM_DEVICE_HIBERNATION)) {
		/* If you're returning from a idel, suspend or hibernate, */
		/* turn on power to the tape drive.                       */
		pm_planar_control(tape_pm_ptr->pmh.devno, 
				  tape_pm_ptr->pm_device_id,
				  PM_PLANAR_ON);
		/* There is no lock for looking at the powered_off flag */
		/* as it is known that the device is not open at this */
		/* time.  Therefore, there can be no race.            */
	        device_ptr->powered_off = FALSE;
	    }
	    tape_pm_ptr->pmh.mode = mode;
	    unlockl(&device_ptr->lock_word);
	    break;
        case PM_DEVICE_IDLE:
	case PM_DEVICE_SUSPEND:
	case PM_DEVICE_HIBERNATION:
    	    /* Lock the configuration lock word to serialize with. */
            /* any call to the configuration routines.             */
            rc = lockl(&tape_device_lock_word, LOCK_NDELAY);
            if (rc == LOCK_FAIL) {  /* another process has the device lock */
        	rc = PM_ERROR;
            }
    	    /* To prevent races with other applications for this device.   */
    	    rc = lockl(&device_ptr->lock_word, LOCK_NDELAY);
            if (rc == LOCK_FAIL) {  /* another process has the device lock */
                unlockl(&tape_device_lock_word);
        	rc = PM_ERROR;
            }
	    else if (device_ptr->opened) {
                unlockl(&tape_device_lock_word);
		unlockl(&device_ptr->lock_word);
        	rc = PM_ERROR;
	    }
	    else {
		device_ptr->powered_off = TRUE;
		/* Turn off power to the tape drive. */
		pm_planar_control(tape_pm_ptr->pmh.devno, 
				  tape_pm_ptr->pm_device_id,
				  PM_PLANAR_OFF);
		tape_pm_ptr->pmh.mode = mode;
                unlockl(&tape_device_lock_word);
		unlockl(&device_ptr->lock_word);
	    }
	    break;
	case PM_PAGE_FREEZE_NOTICE:
	    rc = pincode((int (*)()) striodone);
	    /* If a failure occurs, return an error. */
	    if (rc != 0) {
        	rc = PM_ERROR;
	    }
	    break;
	case PM_PAGE_UNFREEZE_NOTICE:
            (void) unpincode((int (*)()) striodone);
	    break;
	default:
	    rc = PM_ERROR;
    }
    return(rc);
}
