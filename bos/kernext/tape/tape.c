#ifndef lint
static char sccsid[] = "@(#)17 1.42.3.15 src/bos/kernext/tape/tape.c, sysxtape, bos41J, 9513A_all 3/24/95 16:02:31";
#endif
/*
 * COMPONENT_NAME: (SYSXTAPE)  SCSI Magnetic Tape Device Driver Top Half
 *
 * FUNCTIONS:  strconfig, stropen, strclose, stread, strwrite, strioctl
 *             strsoft, strpin
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
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/ioctl.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/conf.h>
#include <sys/devinfo.h>
#include <sys/device.h>
#include <sys/lockl.h>
#include <sys/uio.h>
#include <sys/errids.h>
#include <sys/watchdog.h>
#include <sys/dump.h>
#include <sys/priv.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>

#include <sys/scsi.h>
#include <sys/tape.h>
#include <sys/tapedd.h>

#ifdef _POWER_MP
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#endif /* _POWER_MP */

/* END OF INCLUDED SYSTEM FILES  */

/************************************************************************/
/* Tape Pointer Array                                                */
/************************************************************************/
extern struct tape_device_df *tape_df_list_ptr[256];
extern int             tape_device_config;
extern int             tape_device_lock_word;

/************************************************************************/
/* Function Declarations                                                */
/************************************************************************/
#ifndef _NO_PROTO
int  strconfig(dev_t devno, int cmd, struct uio *uiop);
int  stropen(dev_t devno, ulong devflag, int chan, int ext);
int  strclose(dev_t devno, int chan, int ext);
int  strread(dev_t devno, struct uio *uiop, int chan, int ext);
int  strwrite(dev_t devno, struct uio *uiop, int chan, int ext);
int  strioctl(dev_t devno, int cmd, int arg, ulong devflag,
	      int chan, int ext);
int  strpin(caddr_t buffer_addr, int *length, short segflag,
	    int blocksize);
int strchange(dev_t devno, ulong devflag, int chan, int ext,
	      struct tape_device_df *device_ptr, struct tape_cmd *current_ptr);
void strsoft(struct tape_device_df *device_ptr);
extern int  strdump(dev_t devno, struct uio *uiop, int cmd, int arg,
		    int chan, int ext);
extern void strstart(struct tape_cmd *cmd_ptr);
extern void strsleep(struct tape_cmd *cmd_ptr);
extern void striodone(struct buf *buf_ptr);
extern void str_request_sense(struct tape_device_df *device_ptr,
			      struct tape_cmd *cmd_ptr, uchar clear_flag,
			      uchar resume_flag);
extern void strbldcmd(struct tape_device_df *device_ptr,
		      struct tape_cmd *cmd_ptr, uchar direction,
		      uchar resume_flag);
extern void strreadwrite(struct tape_device_df *device_ptr,
		      struct tape_cmd *cmd_ptr, uchar *buffer_addr,
		      int transfer_length, int direction_flag);
extern void str_general(struct tape_device_df *device_ptr,
			struct tape_cmd *cmd_ptr, uchar command_type,
			uchar immediate_bit, uchar resume_flag);
extern void str_load(struct tape_device_df *device_ptr,
		     struct tape_cmd *cmd_ptr, uchar reten_req,
		     uchar resume_flag, uchar load_flag);
extern void str_erase(struct tape_device_df *device_ptr,
		      struct tape_cmd *cmd_ptr, uchar resume_flag);
extern void str_space(struct tape_device_df *device_ptr,
		      struct tape_cmd *cmd_ptr, int direction,
		      int type, daddr_t count, uchar resume_flag);
extern void str_write_filemarks(struct tape_device_df *device_ptr,
			 struct tape_cmd *cmd_ptr,
			 daddr_t num_filemarks,
			 uchar resume_flag);
extern void strstore(struct tape_device_df *device_ptr,
		     struct tape_cmd *cmd_ptr);
extern void strsend(struct tape_device_df *device_ptr,
		    struct tape_cmd *cmd_ptr);
extern void strwatchdog(struct tape_watchdog *tape_watchdog_ptr);
extern int  strpmhandler(caddr_t private, int mode);
extern int nodev();
#else
int             strconfig();
int             stropen();
int             strclose();
int             strread();
int             strwrite();
int             strioctl();
void            strsoft();
int             strpin();
int             strchange();
extern int      strdump();
extern void     striodone();
extern void     strstart();
extern void     strsleep();
extern void     str_request_sense();
extern void     strbldcmd();
extern void     strreadwrite();
extern void     str_general();
extern void     str_load();
extern void     str_erase();
extern void     str_space();
extern void     str_write_filemarks();
extern void     strstore();
extern void     strsend();
extern void     strwatchdog();
extern int      strpmhandler();
#endif

/**************************************************************************/
/*                                                                        */
/* NAME:  strconfig                                                       */
/*                                                                        */
/* FUNCTION:  Tape Device Driver Initialization Routine.                  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*      It can page fault only if called under a process and the stack    */
/*      is not pinned.                                                    */
/*                                                                        */
/* NOTES:  During device define time, this routine allocates and          */
/*         initializes data structures required for processing user       */
/*         requests.  This routine will also delete an already            */
/*         defined device and free these structures when called for       */
/*         this operation.                                                */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    tape_df_list_ptr	              Array of pointers to tape structures*/
/*                                                                        */
/* INPUTS:                                                                */
/*    devno  - major/minor number                                         */
/*    cmd    - CFG_INIT for initialization, CFG_TERM for device deletion. */
/*    uiop   - pointer to uio structure which contains device information.*/
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    ENODEV - device not defined.                                        */
/*    EFAULT - return from uiomove.                                       */
/*    EAGAIN - device still OPENed.                                       */
/*    EINVAL - invalid config. information.                               */
/*           - invalid cmd request.                                       */
/*    ENOMEM - xmalloc failed to allocate the required storage.           */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  xmalloc                                   */
/*                              xfree                                     */
/*                              lockl                                     */
/*                              unlockl                                   */
/*                              uiomove                                   */
/*                              lock_alloc                                */
/*                              simple_lock_init                          */
/*                              lock_free                                 */
/*                              pm_register_handle                        */
/*                                                                        */
/**************************************************************************/
int
strconfig(
dev_t           devno,      /* major and minor device numbers */
int             cmd,        /* dds operation to perform */
struct uio     *uiop)       /* pointer to the uio vector */

{
    struct tape_device_df *device_ptr;
    struct devsw    tape_devsw;
    extern          nodev();
    int             rc;
    uint            dev, tape_offset;

    DDHKWD5(HKWD_DD_TAPEDD, DD_ENTRY_CONFIG, 0, devno, cmd, 0, 0, 0);
    /* The 5 high order bits of the minor number determines which  */
    /* offset in the dds array (tape_df_list_ptr) holds the device */
    /* information for this device.                                */
    dev = minor(devno);
    /* tape_device_lock_word is a lock word for the                */
    /* tape_df_list_ptr array.                                     */
    (void) lockl(&tape_device_lock_word, LOCK_SHORT);
    /* Lock to avoid conflicts.  Get the information structure for */
    /* this device from an array of device info. structures.       */
    tape_offset = (dev >> 3) & DEVNO_OVERLAY;
    device_ptr = tape_df_list_ptr[tape_offset];
    switch (cmd) {
    case (CFG_TERM):		/* delete device */
	if (device_ptr == NULL) {	/* device is already deleted */
	    unlockl(&tape_device_lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_CONFIG, ENODEV, devno);
	    return (ENODEV);    /* device not defined */
	}
        if ((device_ptr->powered_off == TRUE) &&
	   (device_ptr->tape_ddi.tape_pm_ptr.pmh.mode != PM_DEVICE_IDLE)) {
	/* If the device is currently and the cause is something other */
	/* than device being idle, then fail the unconfigure with      */
	/* EBUSY.                                                      */
	    unlockl(&tape_device_lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_CONFIG, EBUSY, devno);
	    return (EBUSY);
	}
	if (device_ptr->opened == TRUE) {
	    /* can't delete while device still open */
	    unlockl(&tape_device_lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_CONFIG, EBUSY, devno);
	    return (EBUSY);    /* device in use */
	}
	/* delete all structure associated with this device.           */
#ifdef _POWER_MP
	lock_free(&device_ptr->intr_lock);
#endif /* _POWER_MP */
	tape_df_list_ptr[tape_offset] = NULL;
	(void) pm_register_handle((struct pm_handle *)
                  &(device_ptr->tape_ddi.tape_pm_ptr), PM_UNREGISTER);
	rc = (int) xmfree((void *) device_ptr, pinned_heap);
	/* If this is the last tape device to be deleted from the sys- */
	/* tem, remove the tape dd entry from the device switch table. */
	if (--tape_device_config == 0) {
	    if (rc == 0) {
		rc = devswdel(devno);
	    }
	    else {
		(void) devswdel(devno);
	    }
	}
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_CONFIG, rc, devno);
	unlockl(&tape_device_lock_word);
	return (rc);
    case (CFG_INIT):		/* initializing device */
	if (device_ptr != NULL) {	/* device already initialized */
	    unlockl(&tape_device_lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_CONFIG, EINVAL, devno);
	    return (EINVAL);	/* invalid request */
	}
	/* get disk record space for this drive */
	device_ptr = (struct tape_device_df *)
		     xmalloc((uint) sizeof(struct tape_device_df),
			     (uint) PGSHIFT, pinned_heap);
	if (device_ptr == NULL) {
	    unlockl(&tape_device_lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_CONFIG, ENOMEM, devno);
	    return (ENOMEM);	/* couldn't allocate space */
	}

	tape_df_list_ptr[tape_offset] = device_ptr;
	bzero(device_ptr, sizeof(struct tape_device_df));

	/* adding device */
	/* adding ddi information  */
	rc = uiomove((caddr_t) &device_ptr->tape_ddi,
		     (int) sizeof(struct tape_ddi_df), UIO_WRITE, uiop);
	if (rc != 0) {
	    tape_df_list_ptr[tape_offset] = NULL;
	    (void) (int) xmfree((void *) device_ptr, pinned_heap);
	    unlockl(&tape_device_lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_CONFIG, rc, devno);
	    return (rc);
	}

	/* initialize tape flags  */
	device_ptr->opened = FALSE;
	device_ptr->tape_previously_open = FALSE;
	device_ptr->lock_word = EVENT_NULL;

	/* initialize tape power management flags  */
	device_ptr->powered_off = FALSE;
	device_ptr->tape_ddi.tape_pm_ptr.pmh.activity = PM_NO_ACTIVITY;
	device_ptr->tape_ddi.tape_pm_ptr.pmh.mode = PM_DEVICE_FULL_ON;
	device_ptr->tape_ddi.tape_pm_ptr.pmh.handler = strpmhandler;
	device_ptr->tape_ddi.tape_pm_ptr.pmh.private = 
                                 (caddr_t) &device_ptr->tape_ddi.tape_pm_ptr;
	device_ptr->tape_ddi.tape_pm_ptr.pmh.devno = devno;
	device_ptr->tape_ddi.tape_pm_ptr.pmh.next1 = NULL;
	device_ptr->tape_ddi.tape_pm_ptr.pmh.next2 = NULL;
	device_ptr->tape_ddi.tape_pm_ptr.pmh.device_idle_time1 = 0;
	device_ptr->tape_ddi.tape_pm_ptr.pmh.device_idle_time2 = 0;
	device_ptr->tape_ddi.tape_pm_ptr.pmh.device_logical_name = 
			(device_ptr->tape_ddi.resource_name);
	device_ptr->tape_ddi.tape_pm_ptr.pmh.pm_version = PM_PHASE2_SUPPORT;
	device_ptr->tape_ddi.tape_pm_ptr.device_ptr = device_ptr;
	device_ptr->tape_ddi.tape_pm_ptr.pmh.extension = 0;
        device_ptr->tape_ddi.tape_pm_ptr.pm_device_id = 
		     (device_ptr->tape_ddi.tape_pm_ptr.pm_device_id | 
		    ((device_ptr->tape_ddi.scsi_id << 6) |
		      device_ptr->tape_ddi.lun_id));
	rc = pm_register_handle((struct pm_handle *)
                           &(device_ptr->tape_ddi.tape_pm_ptr), PM_REGISTER);
	ASSERT(rc == PM_SUCCESS);
#ifdef _POWER_MP
	lock_alloc(&device_ptr->intr_lock, LOCK_ALLOC_PIN,
		   TAPEDD_LOCK_CLASS, tape_offset);
	simple_lock_init(&device_ptr->intr_lock);
#endif /* _POWER_MP */

	/* If this is the first tape device to be added from the sys- */
	/* tem, add the tape dd entry to the device switch table.     */
	if (++tape_device_config == 1) {
	    tape_devsw.d_open = stropen;
	    tape_devsw.d_close = strclose;
	    tape_devsw.d_read = strread;
	    tape_devsw.d_write = strwrite;
	    tape_devsw.d_ioctl = strioctl;
	    tape_devsw.d_strategy = nodev;
	    tape_devsw.d_ttys = NULL;
	    tape_devsw.d_select = nodev;
	    tape_devsw.d_config = strconfig;
	    tape_devsw.d_print = nodev;
	    tape_devsw.d_dump = strdump;
	    tape_devsw.d_mpx = nodev;
	    tape_devsw.d_revoke = nodev;
	    tape_devsw.d_dsdptr = NULL;

#ifdef _POWER_MP
	    tape_devsw.d_opts = DEV_MPSAFE;
#else
	    tape_devsw.d_opts = 0;
#endif /* _POWER_MP */

	    rc = devswadd(devno, &tape_devsw);
	    if (rc != 0) {	/* add failed */
		tape_df_list_ptr[tape_offset] = NULL;

#ifdef _POWER_MP
		lock_free(&device_ptr->intr_lock);
#endif /* _POWER_MP */

		(void) pm_register_handle((struct pm_handle *)
                          &(device_ptr->tape_ddi.tape_pm_ptr), PM_UNREGISTER);
		tape_device_config--;
		(void) (int) xmfree((void *) device_ptr, pinned_heap);
		unlockl(&tape_device_lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_CONFIG, rc, devno);
		return (rc);
	    }
	}
	break;
    default:
	/* invalid operation */
	unlockl(&tape_device_lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_CONFIG, EINVAL, devno);
	return (EINVAL);
    }

    unlockl(&tape_device_lock_word);
    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_CONFIG, 0, devno);
    return (0);

}


/**************************************************************************/
/*                                                                        */
/* NAME:  stropen                                                         */
/*                                                                        */
/* FUNCTION:  Tape Device Driver Open routine.                            */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*      It can page fault only if called under a process and the stack    */
/*      is not pinned.                                                    */
/*                                                                        */
/* NOTES:  This routine will open the magnetic tape device to begin       */
/*         processing user requests.  Commands are issued to the device   */
/*         to setup the device for processing (test unit ready, reserve,  */
/*         mode sense and select). The device is then marked as open      */
/*         and is ready to accept user commands.                          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    device_pointer_structure        Array of pointers to disk structures*/
/*    disk structure                  disk information structure          */
/*                                                                        */
/* INPUTS:                                                                */
/*    devno   - major/minor number                                        */
/*    devflag - DREAD for read only, DWRITE for read/write                */
/*    chan    - not used (for multiplexed devices).                       */
/*    ext     - defines mode required for open.                           */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    ENODEV - device not defined.                                        */
/*    EBUSY  - device reserved by another initiator.                      */
/*    EAGAIN - device already OPENed by another process.                  */
/*    EINVAL - DAPPEND used as a mode in which to open.                   */
/*    EPERM  - access require CTL_DEV authority.                          */
/*    ENOMEM - pin, pincode failed.                                       */
/*    EIO    - kernel service failure.                                    */
/*           - hardware or software error SCSI command failure.           */
/*           - SCSI adapter device driver failure.                        */
/*           - device power failure.                                      */
/*           - general error case for SCSI command failure.               */
/*    ETIMEDOUT  - a SCSI command has timed out before completion.        */
/*    EWRPROTECT - media opened as read/write is write protected.         */
/*    ENOTREADY - device not ready.                                       */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  lockl                                     */
/*                              unlockl                                   */
/*                              pin                                       */
/*                              unpin                                     */
/*                              pincode                                   */
/*                              unpincode                                 */
/*                              fp_opendev                                */
/*                              fp_ioctl                                  */
/*                              fp_close                                  */
/*                                                                        */
/**************************************************************************/
int
stropen(
dev_t           devno,      /* major/minor device number */
ulong           devflag,    /* DREAD for read, DWRITE for write */
int             chan,       /* channel number */
int             ext)        /* flag for diagnostic mode */

{
    struct tape_device_df *device_ptr;
    struct devinfo  info_struct;
    uint            dev, tape_offset;
    int             saverc, rc;
    uchar           loop_continue;

    DDHKWD5(HKWD_DD_TAPEDD, DD_ENTRY_OPEN, 0, devno, devflag, chan, ext, 0);
    dev = minor(devno);

    /* The 5 high order bits of the minor number determines which  */
    /* offset in the dds array (tape_df_list_ptr) holds the device */
    /* information for this device.                                */
    tape_offset = (dev >> 3) & DEVNO_OVERLAY;
    /* tape_device_lock_word is a lock word for the tape_df_list_ptr */
    /* array. This allows only on application at a time to         */
    /* manipulate the array.                                       */
    (void) lockl(&tape_device_lock_word, LOCK_SHORT);
    device_ptr = tape_df_list_ptr[tape_offset]; /* get ptr to device info */
    if (device_ptr == NULL) {	/* if device not initialized */
	unlockl(&tape_device_lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_OPEN, ENODEV, devno);
	return (ENODEV);
    }
    /* If the device attempted to be opened is defined, then the   */
    /* df_list_ptr array is unlocked.  This allows other applica-  */
    /* tions to both define, open, etc. other tape devices.  The   */
    /* device info structure is locked until the open is complete  */
    /* to prevent races with other applications for this device.   */
    rc = lockl(&device_ptr->lock_word, LOCK_NDELAY);
    if (rc == LOCK_FAIL) {  /* another process has the device lock */
	unlockl(&tape_device_lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_OPEN, EAGAIN, devno);
	return (EAGAIN);
    }
    unlockl(&tape_device_lock_word);
    if (device_ptr->powered_off == TRUE) { /* if device powered_off for pm */
        if (device_ptr->tape_ddi.tape_pm_ptr.pmh.mode == PM_DEVICE_IDLE) {
            /* If you're attempting to open and the device is idle, */
            /* turn on power to the tape drive.                     */
            pm_planar_control(device_ptr->tape_ddi.tape_pm_ptr.pmh.devno,
                              device_ptr->tape_ddi.tape_pm_ptr.pm_device_id,
                              PM_PLANAR_ON);
            /* Insure that the powered_off flag is set properly   */
            /* and the mode is set to show the drive is now en- */
            /* abled.                                           */
            device_ptr->powered_off = FALSE;
            device_ptr->tape_ddi.tape_pm_ptr.pmh.mode = PM_DEVICE_ENABLE;
        }
	else {
	    unlockl(&device_ptr->lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_OPEN, EAGAIN, devno);
	    return (EBUSY);
	}
    }
    if (device_ptr->opened == TRUE) {	/* if device already opened  */
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_OPEN, EAGAIN, devno);
	return (EAGAIN);
    }
    else {
	device_ptr->opened = TRUE;
    }
    /* Attempting to open with a forced open, holding the reserve,  */
    /* or in diagnostic mode will result in an error if the caller  */
    /* doesn't have the proper authority.                           */
    if (ext && (privcheck(RAS_CONFIG) != 0)) {
	device_ptr->opened = FALSE;
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_OPEN, EPERM, devno);
	return (EPERM);
    }

    /* Attempting to append a file is not allowed for tape devices. */
    /* A check is made here and will return EINVAL if the attempt to */
    /* open in append mode is made.                                 */
    if (devflag & DAPPEND) {
	device_ptr->opened = FALSE;
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_OPEN, EINVAL, devno);
	return (EINVAL);
    }

    /* Here the device information (i.e. device state, command default */
    /* values etc.) are set here.                                     */
    device_ptr->cmd_state = 0;
    device_ptr->stackhead = 0;

    /* initialize tape flags  */
    device_ptr->cmd_1_ptr.scbuf.bufstruct.b_event = EVENT_NULL;
    device_ptr->cmd_2_ptr.scbuf.bufstruct.b_event = EVENT_NULL;
    device_ptr->reset_cmd_ptr.scbuf.bufstruct.b_event = EVENT_NULL;
    device_ptr->reqsense_cmd_ptr.scbuf.bufstruct.b_event = EVENT_NULL;
    device_ptr->cmd_1_ptr.scbuf.bp = NULL;
    device_ptr->cmd_2_ptr.scbuf.bp = NULL;
    device_ptr->reset_cmd_ptr.scbuf.bp = NULL;
    device_ptr->reqsense_cmd_ptr.scbuf.bp = NULL;
    /* save device major/minor number */
    device_ptr->devno = devno;
    device_ptr->flags = 0;	/* clear device flags        */
    device_ptr->flags2 = 0;      /* clear device flags        */
    device_ptr->operation = 0;	/* clear read/write flags    */
    /* clear error counter flags */
    device_ptr->tape_error.write_recovered_error = 0;
    device_ptr->tape_error.read_recovered_error = 0;
    device_ptr->tape_error.medium_error = 0;
    device_ptr->tape_error.hardware_error = 0;
    device_ptr->tape_error.aborted_cmd_error = 0;
    device_ptr->write_xfer_count = 0;
    device_ptr->write_block_count = 0;
    device_ptr->write_resid = 0;
    device_ptr->read_xfer_count = 0;
    device_ptr->read_block_count = 0;
    device_ptr->read_resid = 0;
    device_ptr->sense_flag = TAPE_SENSE_A;
    device_ptr->filemark_save = FALSE;
    device_ptr->tape_page_supsel = 0;
    device_ptr->tape_page_supsen = 0;
    device_ptr->bytes_requested = 0x3f;
    device_ptr->initial_tape_blocks = 0;
    /* save the configured blocksize value for restoring */
    device_ptr->save_blocksize = device_ptr->tape_ddi.blocksize;
    /* alway try synchronous data xfer first */
    device_ptr->async_flag = 0;
    /* save the ext for use in the close */
    device_ptr->open_mode = ext;

    /* Initialize watchdog timer values.  Watchdog timers are used    */
    /* when the device driver detects a reset to the device.  Resets  */
    /* take additional time and the device driver uses the watchdog   */
    /* timer to pause before issuing retries.                         */
    device_ptr->tape_watchdog_ptr.device_ptr = device_ptr;
    device_ptr->tape_watchdog_ptr.watch_timer.next = NULL;
    device_ptr->tape_watchdog_ptr.watch_timer.prev = NULL;
    device_ptr->tape_watchdog_ptr.watch_timer.func = strwatchdog;
    device_ptr->tape_watchdog_ptr.watch_timer.count = 0;
    if ((device_ptr->tape_ddi.dev_type == TAPE_8MM) ||
        (device_ptr->tape_ddi.dev_type == TAPE_8MM5GB) ||
        (device_ptr->tape_ddi.dev_type == TAPE_3490E)) {
        /* all helical technology drives - pause for 7 seconds */
        device_ptr->tape_watchdog_ptr.watch_timer.restart = 7;
    }
    else if ((device_ptr->tape_ddi.dev_type == TAPE_4MM2GB) ||
        (device_ptr->tape_ddi.dev_type == TAPE_4MM4GB)) {
        /* all helical technology drives - pause for 16 seconds */
        device_ptr->tape_watchdog_ptr.watch_timer.restart = 16;
    }
    else if (device_ptr->tape_ddi.dev_type == TAPE_OTHER) {
	/* pause for user settable delay (in seconds) */
	device_ptr->tape_watchdog_ptr.watch_timer.restart = 
	    device_ptr->tape_ddi.delay;
    }
    else {
	/* pause for 2 seconds */
	device_ptr->tape_watchdog_ptr.watch_timer.restart = 2;
    }

#ifdef _POWER_MP
    while (w_init(&device_ptr->tape_watchdog_ptr.watch_timer));
#else
    w_init(&device_ptr->tape_watchdog_ptr.watch_timer);
#endif /* _POWER_MP */

    /* Attempt to pin the bottom half of the code.  This is because   */
    /* the striodone routine executes on an interrupt level and would */
    /* cause a problem if a page fault occurred.                      */
    rc = pincode((int (*)()) striodone);
    /* If a failure occurs, back out of the open and return an error. */
    if (rc != 0) {
	/* release timer */

#ifdef _POWER_MP
	while(w_clear(&device_ptr->tape_watchdog_ptr.watch_timer));
#else
	w_clear(&device_ptr->tape_watchdog_ptr.watch_timer);
#endif /* _POWER_MP */

	device_ptr->opened = FALSE;
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_OPEN, rc, devno);
	return (rc);
    }

    /* Open the scsi adapter driver for this device                   */
    rc = (char) fp_opendev(device_ptr->tape_ddi.adapter_devno,
			  FWRITE, NULL, NULL, &(device_ptr->fp));
    /* If a failure occurs, back out of the open and return an error. */
    if (rc != 0) {
	/* release timer */

#ifdef _POWER_MP
	while(w_clear(&device_ptr->tape_watchdog_ptr.watch_timer));
#else
	w_clear(&device_ptr->tape_watchdog_ptr.watch_timer);
#endif /* _POWER_MP */

	(void) unpincode((int (*)()) striodone);
	device_ptr->opened = FALSE;
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_OPEN, EIO, devno);
	return (EIO);
    }
    /* Start the scsi device driver to prepare for command reception. */
    rc = fp_ioctl(device_ptr->fp, SCIOSTART,
		  IDLUN(device_ptr->tape_ddi.scsi_id,
		  device_ptr->tape_ddi.lun_id),NULL);
    /* If a failure occurs, back out of the open and return an error. */
    if (rc != 0) {
	(void) fp_close(device_ptr->fp);
	/* release timer */
#ifdef _POWER_MP
	while(w_clear(&device_ptr->tape_watchdog_ptr.watch_timer));
#else
	w_clear(&device_ptr->tape_watchdog_ptr.watch_timer);
#endif /* _POWER_MP */

	(void) unpincode((int (*)()) striodone);
	device_ptr->opened = FALSE;
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_OPEN, EIO, devno);
	return (EIO);
    }
    /* For SC_FORCED_OPEN, a bus device reset is done to free the     */
    /* device from any outstanding reservation. SC_DIAGNOSTIC will    */
    /* cause the code that sets device parameters to be bypassed.     */
    /* Both modes require CTL_DEV authority to execute.               */
    if (device_ptr->open_mode & SC_FORCED_OPEN) {
	/* processing is complete */
	rc = fp_ioctl(device_ptr->fp, SCIORESET,
		      IDLUN(device_ptr->tape_ddi.scsi_id,
			    device_ptr->tape_ddi.lun_id),NULL);
	if (rc != 0) {		/* reset failed */
	    (void) fp_ioctl(device_ptr->fp, SCIOSTOP,
			    IDLUN(device_ptr->tape_ddi.scsi_id,
				  device_ptr->tape_ddi.lun_id), NULL);
	    (void) fp_close(device_ptr->fp);

#ifdef _POWER_MP
	    while(w_clear(&device_ptr->tape_watchdog_ptr.watch_timer));
#else
	    w_clear(&device_ptr->tape_watchdog_ptr.watch_timer);
#endif /* _POWER_MP */

	    (void) unpincode((int (*)()) striodone);
	    device_ptr->opened = FALSE;
	    unlockl(&device_ptr->lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_OPEN, rc, devno);
	    return (rc);
	}
    }
    /* For diagnostic processing, stuctures are pinned and the adapter */
    /* dd is opened and started.  No commands are issued directly to   */
    /* the drive.                                                      */
    if (device_ptr->open_mode & SC_DIAGNOSTIC) {
	/* processing is complete */
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_OPEN, 0, devno);
	return (0);
    }

    /* For the tape devices, if this is the first open ever issued to */
    /* this device a load command is issued to guarantee that the tape*/
    /* is a BOT.  This is because IPL ROS may have tried to IPL from  */
    /* this tape.  If so, the IPL ROS does not leave the tape at BOT  */
    /* and the mode select will fail.                                 */
    if (device_ptr->tape_previously_open == FALSE) {
	device_ptr->tape_previously_open = TRUE;
	device_ptr->flags2 |= TAPE_LOAD_REQUIRED;
    }

    if (device_ptr->tape_ddi.dev_type == TAPE_OTHER)
	device_ptr->selection_set = FALSE;
    else
	device_ptr->selection_set = TRUE;
    loop_continue = TRUE;
    while (loop_continue) {
	/* A test unit ready is issued to the device which kicks off the  */
	/* sequence reserve, mode sense, and mode select to be executed.  */
	device_ptr->flags |= TAPE_OPENING_DEVICE;
	device_ptr->cmd_1_ptr.retry_flag = TRUE;    /* allow retries    */
	device_ptr->cmd_1_ptr.retry_count = 0;
	device_ptr->cmd_state = TAPE_TUR;   /* test unit ready  */
	device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
	/* builds the test unit ready command.                            */
	str_general(device_ptr, &device_ptr->cmd_1_ptr,
		    (uchar) SCSI_TEST_UNIT_READY,  (uchar) 0x0,
		    (uchar) SC_RESUME);
	strstart(&device_ptr->cmd_1_ptr);
	strsleep(&device_ptr->cmd_1_ptr);
	saverc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
	if (saverc != 0) {
	    if ((device_ptr->cmd_state == TAPE_SENSE) &&
	       (device_ptr->tape_ddi.dev_type == TAPE_OTHER)) {
		switch (device_ptr->sense_flag) {
		case (TAPE_SENSE_A):
		    device_ptr->sense_flag = TAPE_SENSE_B;
		    device_ptr->bytes_requested = 0;
		    break;
		case (TAPE_SENSE_B):
		    device_ptr->sense_flag = TAPE_SENSE_C;
		    device_ptr->tape_page_supsen = 0x10;
		    device_ptr->bytes_requested = 0x3f;
		    break;
		case (TAPE_SENSE_C):
		    device_ptr->sense_flag = TAPE_SENSE_D;
		    device_ptr->bytes_requested = 0;
		    break;
		case (TAPE_SENSE_D):
		default:
		    loop_continue = FALSE;
		    break;
		}
	    }
	    else if ((device_ptr->cmd_state == TAPE_SELECT) &&
		     (device_ptr->tape_ddi.dev_type == TAPE_OTHER)) {
		if (device_ptr->tape_page_supsel == 0)
		    device_ptr->tape_page_supsel = 0x10;
		else
		    loop_continue = FALSE;
	    }
	    else
		loop_continue = FALSE;
	}
	else
	    loop_continue = FALSE;
    }

    device_ptr->selection_set = TRUE;
    /* If a failure occurs during the startup sequence, a release is  */
    /* done (just in case), the device structure and device driver    */
    /* bottom half are unpinned, and the scsi adapter device driver is */
    /* informed that this device is being stopped.                    */
    if (saverc != 0) {
	if (saverc == EIO) {
	    /* the tape drive is seriosly hung.  Send a BDR */
	    (void) fp_ioctl(device_ptr->fp, SCIORESET,
			  IDLUN(device_ptr->tape_ddi.scsi_id,
				device_ptr->tape_ddi.lun_id), NULL);
	}
	device_ptr->cmd_1_ptr.retry_flag = TRUE;
	device_ptr->cmd_1_ptr.retry_count = 0;
	device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
	device_ptr->cmd_state = TAPE_GENERAL;
	device_ptr->flags &= ~TAPE_OPENING_DEVICE;
	/* builds the release command.                            */
	if (device_ptr->tape_ddi.res_sup) {
	    str_general(device_ptr, &device_ptr->cmd_1_ptr,
			(uchar) SCSI_RELEASE_UNIT, (uchar) 0x0,
			(uchar) SC_RESUME);
	    strstart(&device_ptr->cmd_1_ptr);
	    strsleep(&device_ptr->cmd_1_ptr);
	}
	(void) fp_ioctl(device_ptr->fp, SCIOSTOP,
			IDLUN(device_ptr->tape_ddi.scsi_id,
			      device_ptr->tape_ddi.lun_id), NULL);
	(void) fp_close(device_ptr->fp);

#ifdef _POWER_MP
	while(w_clear(&device_ptr->tape_watchdog_ptr.watch_timer));
#else
	w_clear(&device_ptr->tape_watchdog_ptr.watch_timer);
#endif /* _POWER_MP */

	(void) unpincode((int (*)()) striodone);
	device_ptr->opened = FALSE;
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_OPEN, saverc, devno);
	return (saverc);
    }

    /* Information obtained from the mode sense done during the       */
    /* startup procedure is used to tell if the tape is write protect- */
    /* ed. If so, and an attempt was made to open in read/write mode, */
    /* then the device is cleaned up (see above description) and an   */
    /* error is returned.                                             */
    if (devflag & DWRITE) {
	if (device_ptr->flags & TAPE_READ_ONLY) {
	    device_ptr->cmd_1_ptr.retry_flag = TRUE;
	    device_ptr->cmd_1_ptr.retry_count = 0;
	    device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
	    device_ptr->cmd_state = TAPE_GENERAL;
	    device_ptr->flags &= ~TAPE_OPENING_DEVICE;
	    if (device_ptr->tape_ddi.res_sup) {
		str_general(device_ptr, &device_ptr->cmd_1_ptr,
			    (uchar) SCSI_RELEASE_UNIT, (uchar) 0x0,
			    (uchar) SC_RESUME);
		strstart(&device_ptr->cmd_1_ptr);
		strsleep(&device_ptr->cmd_1_ptr);
	    }
	    (void) fp_ioctl(device_ptr->fp, SCIOSTOP,
			    IDLUN(device_ptr->tape_ddi.scsi_id,
				  device_ptr->tape_ddi.lun_id), NULL);
	    (void) fp_close(device_ptr->fp);
#ifdef _POWER_MP
	    while(w_clear(&device_ptr->tape_watchdog_ptr.watch_timer));
#else
	    w_clear(&device_ptr->tape_watchdog_ptr.watch_timer);
#endif /* _POWER_MP */
	    (void) unpincode((int (*)()) striodone);
	    device_ptr->opened = FALSE;
	    unlockl(&device_ptr->lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_OPEN, EWRPROTECT, devno);
	    return (EWRPROTECT);
	}
    }
    else {

	/* Even if the tape is not write-protected, the TAPE_READ_ONLY    */
	/* flag is used to prevent any ioctl (i.e. erase) that affects    */
	/* media from being issued.                                       */
	device_ptr->flags |= TAPE_READ_ONLY;
    }
    device_ptr->flags &= ~TAPE_OPENING_DEVICE;

    /* Determine the maximum transfer amount for each read/write  */
    /* command by calling the adapter dd's iocinfo ioctl.  If the */
    /* ioctl call fails, the use a predetermined define value for */
    /* the maximum transfer allowed (TAPE_MAXREQUEST).            */
    rc = fp_ioctl(device_ptr->fp, IOCINFO, &info_struct, NULL);
    if (rc == 0) {
	if (device_ptr->tape_ddi.blocksize == 0)
	    device_ptr->max_xfer_size = info_struct.un.scsi.max_transfer;
	else if (device_ptr->tape_ddi.blocksize <= TAPE_MAXREQUEST)
	    if (info_struct.un.scsi.max_transfer > TAPE_MAXREQUEST)
		device_ptr->max_xfer_size = TAPE_MAXREQUEST;
	    else
		device_ptr->max_xfer_size = info_struct.un.scsi.max_transfer;
	else if (info_struct.un.scsi.max_transfer <= TAPE_MAXREQUEST)
	    device_ptr->max_xfer_size = info_struct.un.scsi.max_transfer;
	else if (device_ptr->tape_ddi.blocksize >
		      info_struct.un.scsi.max_transfer)
	    device_ptr->max_xfer_size = info_struct.un.scsi.max_transfer;
	else
	    device_ptr->max_xfer_size = device_ptr->tape_ddi.blocksize;
    }
    else {
	device_ptr->max_xfer_size = TAPE_MAXREQUEST;

    }
    if (device_ptr->tape_ddi.dev_type == TAPE_8MM5GB){
        /* issue request sense command to obtain initial number */
	/* of blocks so that at close we can tell how many were */
	/* actually written to tape                             */
        device_ptr->reqsense_cmd_ptr.retry_flag = FALSE;
        device_ptr->reqsense_cmd_ptr.retry_count = 0;
        device_ptr->cmd_state = TAPE_GENERAL;
        device_ptr->reqsense_cmd_ptr.type = TAPE_SPECIAL_CMD;
        str_request_sense(device_ptr, &device_ptr->reqsense_cmd_ptr,
		     (uchar) 0x80, (uchar) SC_RESUME);
        strstart(&device_ptr->reqsense_cmd_ptr);
        strsleep(&device_ptr->reqsense_cmd_ptr);
	device_ptr->initial_tape_blocks |=
	    device_ptr->req_sense_buf.extended_byte11 << 16;
	device_ptr->initial_tape_blocks |=
	    device_ptr->req_sense_buf.extended_byte12 << 8;
	device_ptr->initial_tape_blocks |=
	    device_ptr->req_sense_buf.extended_byte13;
	/* If the initial number of blocks left on tape is < 0 then */
	/* set to 1 so that you don't get a divide by zero error    */
	/* when calculating soft errors in strsoft routine. The     */
	/* initial blocks on tape can < 0 if you start past the log-*/
	/* ical end of tape (LEOT).                                 */
	if (device_ptr->initial_tape_blocks < 0){
	    device_ptr->initial_tape_blocks = 1;
	}
        /* Check to see if we had a problem with the request sense */
	/* If there is a problem then stop the device and fail the */
	/* open.                                                   */
        if (device_ptr->reqsense_cmd_ptr.scbuf.bufstruct.b_error != 0) {
	    (void) fp_ioctl(device_ptr->fp, SCIOSTOP,
			    IDLUN(device_ptr->tape_ddi.scsi_id,
				  device_ptr->tape_ddi.lun_id), NULL);
	    (void) fp_close(device_ptr->fp);

#ifdef _POWER_MP
	    while(w_clear(&device_ptr->tape_watchdog_ptr.watch_timer));
#else
	    w_clear(&device_ptr->tape_watchdog_ptr.watch_timer);
#endif /* _POWER_MP */

	    (void) unpincode((int (*)()) striodone);
	    device_ptr->opened = FALSE;
	    unlockl(&device_ptr->lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_OPEN, EIO, devno);
	    return(EIO);                 /* request sense failed */
        }
    }
    unlockl(&device_ptr->lock_word);
    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_OPEN, 0, devno);
    return (0);
}


/**************************************************************************/
/*                                                                        */
/* NAME:  strclose                                                        */
/*                                                                        */
/* FUNCTION:  Tape Device Driver Close routine.                           */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*      It can page fault only if called under a process and the stack    */
/*      is not pinned.                                                    */
/*                                                                        */
/* NOTES:  This routine will close the magnetic tape device.  If the      */
/*         tape device is in buffered mode, then an attempt will be made  */
/*         to flush the device's data buffers.  An error will be returned */
/*         if this fails.                                                 */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    device_pointer_structure        Array of pointers to disk structures*/
/*    disk structure                  disk information structure          */
/*                                                                        */
/* INPUTS:                                                                */
/*    devno  - major/minor number                                         */
/*    chan   - not used (for multiplexed devices).                        */
/*    ext    - defines mode required for open.                            */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    ENODEV - device not defined.                                        */
/*    ENXIO  - device not OPEN.                                           */
/*           - device instance not OPEN.                                  */
/*    EBUSY  - device reserved by another initiator.                      */
/*    EIO    - kernel service failure.                                    */
/*           - hardware or software error SCSI command failure.           */
/*           - SCSI adapter device driver failure.                        */
/*           - device power failure.                                      */
/*           - general error case for SCSI command failure.               */
/*    EBUSY  - device reserved by another initiator.                      */
/*    ETIMEDOUT  - a SCSI command has timed out before completion.        */
/*    ENOTREADY - device not ready.                                       */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  lockl                                     */
/*                              unlockl                                   */
/*                              unpin                                     */
/*                              unpincode                                 */
/*                              fp_ioctl                                  */
/*                              fp_close                                  */
/*                                                                        */
/**************************************************************************/
int
strclose(
    dev_t           devno,      /* major/minor device number */
    int             chan,       /* channel number */
    int             ext)        /* flag for diagnostic mode */

{
    struct tape_device_df *device_ptr;
    char            strsoft_called;
    uint            dev, tape_offset;
    int             saverc, rc;

    DDHKWD1(HKWD_DD_TAPEDD, DD_ENTRY_CLOSE, 0, devno);
    dev = minor(devno);

    /* The 5 high order bits of the minor number determines which  */
    /* offset in the dds array (tape_df_list_ptr) holds the device */
    /* information for this device.                                */
    tape_offset = (dev >> 3) & DEVNO_OVERLAY;
    (void) lockl(&tape_device_lock_word, LOCK_SHORT);
    device_ptr = tape_df_list_ptr[tape_offset];
    if (device_ptr == NULL) {
	unlockl(&tape_device_lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_CLOSE, ENODEV, devno);
	return (ENODEV);
    }
    (void) lockl(&device_ptr->lock_word, LOCK_SHORT);
    unlockl(&tape_device_lock_word);
    strsoft_called = FALSE;
    /* A check is made here to determine if both device types are  */
    /* the same.  CLOSE can be driven by a failed attempt by an-   */
    /* other application using a different device instance to open */
    /* the drive and then failing the open.                        */
    if (device_ptr->devno != devno) {   /* if device not same   */
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_CLOSE, ENXIO, devno);
	return (ENXIO);
    }
    /* A check is made here to determine if the device is not cur- */
    /* rently open.                                                */
    if (device_ptr->opened != TRUE) {   /* if device not open  */
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_CLOSE, ENXIO, devno);
	return (ENXIO);
    }
    /* saverc is used to return the most recent error that occurs      */
    /* during the close.  Even though an error may occur, the close is */
    /* continued until completion.                                     */
    saverc = 0;

    /* If opened with ext indicating SC_DIAGNOSTIC, the device        */
    /* structure and device driver bottom half are unpinned, and the  */
    /* scsi adapter device is informed that this device is being      */
    /* stopped.  No special processing (as is the norm for a non-     */
    /* diagnostic open) is done.                                      */
    if (device_ptr->open_mode & SC_DIAGNOSTIC) {
	/* Stop and close the adapter dd first.                   */
	rc = fp_ioctl(device_ptr->fp, SCIOSTOP,
		      IDLUN(device_ptr->tape_ddi.scsi_id,
			    device_ptr->tape_ddi.lun_id), NULL);
	saverc = rc;
	rc = fp_close(device_ptr->fp);
	saverc = (saverc == 0) ? rc : saverc;
	/* Release watchdog timer and unpin pinned structures.    */
	/* release timer */
#ifdef _POWER_MP
	while(w_clear(&device_ptr->tape_watchdog_ptr.watch_timer));
#else
	w_clear(&device_ptr->tape_watchdog_ptr.watch_timer);
#endif /* _POWER_MP */
	(void) unpincode((int (*)()) striodone);
	device_ptr->opened = FALSE;
	/* restore configured blocksize value */
	device_ptr->tape_ddi.blocksize = device_ptr->save_blocksize;
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_CLOSE, saverc, devno);
	return (saverc);
    }

    /* A TAPE_OFFLINE can occur due to a scsi bus reset during        */
    /* io. A rewind is attempted here if the special file type        */
    /* requires it.                                                   */
    if (device_ptr->flags & TAPE_OFFLINE) {
	if (!(dev & TAPE_NOREW_ON_CLOSE) &&      /* if rewind */
	   (device_ptr->tape_ddi.dev_type ==TAPE_9TRACK)) {
	    /* setup an issue the rewind command. */
	    device_ptr->cmd_1_ptr.retry_flag = TRUE;
	    device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
	    device_ptr->cmd_1_ptr.retry_count = 0;
	    device_ptr->cmd_state = TAPE_GENERAL;
	    str_general(device_ptr, &device_ptr->cmd_1_ptr,
		       (uchar) SCSI_REWIND, (uchar) 0x01,
		       (uchar) SC_RESUME);
	    strstart(&device_ptr->cmd_1_ptr);
	    strsleep(&device_ptr->cmd_1_ptr);
	    saverc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
	}

	/* The tape_previously open flag is cleared here to guarantee the*/
	/* tape device is at BOT on the next open.  This has the same ef-*/
	/* fect as if the device was opened for the first time.  In other*/
	/* words, a load command will be issued to insure the tape is at */
	/* BOT so the application need not wait when an offline device is*/
	/* closing, but the have to wait when reopening the device.      */
	device_ptr->tape_previously_open = FALSE;

	/* Unpin structures, close scsi adapter device driver.           */
	rc = fp_ioctl(device_ptr->fp, SCIOSTOP,
		      IDLUN(device_ptr->tape_ddi.scsi_id,
			    device_ptr->tape_ddi.lun_id), NULL);
	saverc = (saverc == 0) ? rc : saverc;
	rc = fp_close(device_ptr->fp);
	saverc = (saverc == 0) ? rc : saverc;
	/* release timer */

#ifdef _POWER_MP
	while(w_clear(&device_ptr->tape_watchdog_ptr.watch_timer));
#else
	w_clear(&device_ptr->tape_watchdog_ptr.watch_timer);
#endif /* _POWER_MP */

	(void) unpincode((int (*)()) striodone);
	device_ptr->opened = FALSE;
	/* restore configured blocksize value */
	device_ptr->tape_ddi.blocksize = device_ptr->save_blocksize;
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_CLOSE, saverc, devno);
	return (saverc);
    }

    if (dev & TAPE_NOREW_ON_CLOSE) {	/* don't rewind the tape */

	/* If the tape special file indicates no rewind on close, and     */
	/* writes have been done to the tape, a single filemark is        */
	/* written.  If end of tape is encountered, no error is returned  */
	/* (EOT represents end of data).                                  */
	if (device_ptr->operation == TAPE_WRITE_ACTIVE) {
	    /* write is last command active */
	    device_ptr->cmd_1_ptr.retry_flag = FALSE;
	    device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
	    device_ptr->cmd_state = TAPE_GENERAL;
	    str_write_filemarks(device_ptr, &device_ptr->cmd_1_ptr,
		(daddr_t) 1, (uchar) SC_RESUME);
	    strstart(&device_ptr->cmd_1_ptr);
	    strsleep(&device_ptr->cmd_1_ptr);
	    rc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
	    saverc = (saverc == 0) ? rc : saverc;
	    /* Don't return an error on an end-of-tape error. */
	    /* It's expected if the tape is already at EOT    */
	    /* and filemarks are written.                     */
	    if (device_ptr->cmd_1_ptr.tape_position == TAPE_ENDOFTAPE_ERROR) {
		saverc = 0;
	    }
	}
	else if (device_ptr->operation == TAPE_READ_ACTIVE) {

	    /* If the tape special file indicates no rewind on close, and  */
	    /* reads have been done to the tape, AND a filemark has NOT just*/
	    /* been encountered, AND the tape is not sitting at end of tape,*/
	    /* the the tape is advanced to the next filemark, or end of tape*/
	    /* read last command active */
	    if ((device_ptr->operation == TAPE_READ_ACTIVE) &&
		(((device_ptr->flags & TAPE_FILEMARK_DETECT) == 0) &&
		 ((device_ptr->flags & TAPE_ENDOFTAPE_DETECT) == 0))) {
		if (device_ptr->tape_ddi.dev_type == TAPE_8MM5GB) {
		    strsoft_called = TRUE;
		    strsoft(device_ptr);
	    	}
		device_ptr->cmd_1_ptr.retry_flag = FALSE;
		device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
		device_ptr->cmd_state = TAPE_GENERAL;
		str_space(device_ptr, &device_ptr->cmd_1_ptr, TAPE_FORWARD,
			  TAPE_FILEMARK, (daddr_t) 1, (uchar) SC_RESUME);
		strstart(&device_ptr->cmd_1_ptr);
		strsleep(&device_ptr->cmd_1_ptr);
		rc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
		saverc = (saverc == 0) ? rc : saverc;
		/* For filemark and EOT - no error */
		if ((device_ptr->cmd_1_ptr.tape_position ==
		     TAPE_FILEMARK_ERROR) ||
		    (device_ptr->cmd_1_ptr.tape_position ==
		     TAPE_ENDOFTAPE_ERROR)) {
		    saverc = 0;
		}
	    }
	}
	/* Check for recovered errors by calling strsoft.  This is */
	/* done after reading from or writing to tape.            */
        if ((device_ptr->tape_ddi.dev_type == TAPE_QUARTER) ||
            (device_ptr->tape_ddi.dev_type == TAPE_QIC525) ||
            (device_ptr->tape_ddi.dev_type == TAPE_QIC1200) ||
            (device_ptr->tape_ddi.dev_type == TAPE_8MM) ||
            (device_ptr->tape_ddi.dev_type == TAPE_8MM5GB)) {
	    if (device_ptr->operation != 0 & !(strsoft_called)) {
		strsoft(device_ptr);
	    }
	}
    }
    else {

	/* If the tape special file indicates rewind on close, and writes */
	/* have been done to the tape, 2 filemarks are written to indicate */
	/* end of data.  If the tape is sitting at end of tape, then 0    */
	/* filemarks are written.  This insures that the device data      */
	/* buffer is clear before closing the device.                     */
	if (device_ptr->operation == TAPE_WRITE_ACTIVE) {
	    /* write is last command active */
	    device_ptr->cmd_1_ptr.retry_flag = FALSE;
	    device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
	    device_ptr->cmd_state = TAPE_GENERAL;
	    str_write_filemarks(device_ptr, &device_ptr->cmd_1_ptr,
		(daddr_t) 2, (uchar) SC_RESUME);
	    strstart(&device_ptr->cmd_1_ptr);
	    strsleep(&device_ptr->cmd_1_ptr);
	    rc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
	    saverc = (saverc == 0) ? rc : saverc;
	    /* Don't return an error on an end-of-tape error. */
	    /* It's expected if the tape is already at EOT    */
	    /* and filemarks are written.                     */
	    if (device_ptr->cmd_1_ptr.tape_position == TAPE_ENDOFTAPE_ERROR) {
		saverc = 0;
	    }
	}
	/* Check for recovered errors by calling strsoft.  This is */
	/* done after reading from or writing to tape.            */
        if ((device_ptr->tape_ddi.dev_type == TAPE_QUARTER) ||
            (device_ptr->tape_ddi.dev_type == TAPE_QIC525) ||
            (device_ptr->tape_ddi.dev_type == TAPE_QIC1200) ||
            (device_ptr->tape_ddi.dev_type == TAPE_8MM) ||
            (device_ptr->tape_ddi.dev_type == TAPE_8MM5GB)) {
	    if (device_ptr->operation != 0) {
		strsoft(device_ptr);
	    }
	}
	/* Perform the rewind are required by the tape special file type. */
	device_ptr->cmd_1_ptr.retry_flag = TRUE;
	device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
	device_ptr->cmd_1_ptr.retry_count = 0;
	device_ptr->cmd_state = TAPE_GENERAL;
	str_general(device_ptr, &device_ptr->cmd_1_ptr,
		   (uchar) SCSI_REWIND, (uchar) 0x0,
		   (uchar) SC_RESUME);
	strstart(&device_ptr->cmd_1_ptr);
	strsleep(&device_ptr->cmd_1_ptr);
	rc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
	saverc = (saverc == 0) ? rc : saverc;
    }

    /* If opened with ext indicating SC_RETAIN_RESERVE, the release   */
    /* command is not done.  Else a release is always done.           */
    if (!(device_ptr->open_mode & SC_RETAIN_RESERVATION)) {
	if (device_ptr->tape_ddi.res_sup) {
	    device_ptr->cmd_1_ptr.retry_flag = TRUE;
	    device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
	    device_ptr->cmd_1_ptr.retry_count = 0;
	    device_ptr->cmd_state = TAPE_GENERAL;
	    str_general(device_ptr, &device_ptr->cmd_1_ptr,
		       (uchar) SCSI_RELEASE_UNIT, (uchar) 0x0,
		       (uchar) SC_RESUME);
	    strstart(&device_ptr->cmd_1_ptr);
	    strsleep(&device_ptr->cmd_1_ptr);
	    rc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
	    saverc = (saverc == 0) ? rc : saverc;
	}
    }
    /* Unpin structures, close scsi adapter device driver.            */
    rc = fp_ioctl(device_ptr->fp, SCIOSTOP,
		  IDLUN(device_ptr->tape_ddi.scsi_id,
			device_ptr->tape_ddi.lun_id), NULL);
    saverc = (saverc == 0) ? rc : saverc;
    rc = fp_close(device_ptr->fp);
    saverc = (saverc == 0) ? rc : saverc;
    /* release timer */

#ifdef _POWER_MP
    while(w_clear(&device_ptr->tape_watchdog_ptr.watch_timer));
#else
    w_clear(&device_ptr->tape_watchdog_ptr.watch_timer);
#endif /* _POWER_MP */

    (void) unpincode((int (*)()) striodone);
    device_ptr->opened = FALSE;
    /* restore configured blocksize value */
    device_ptr->tape_ddi.blocksize = device_ptr->save_blocksize;
    unlockl(&device_ptr->lock_word);
    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_CLOSE, saverc, devno);
    /* Note that even though commands issued while closing the device */
    /* may have terminated in error, the close call never fails.      */
    return (saverc);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  strread                                                         */
/*                                                                        */
/* FUNCTION:  Tape Device Driver read routine.                            */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*      It can page fault only if called under a process and the stack    */
/*      is not pinned.                                                    */
/*                                                                        */
/* NOTES:  This routine will accept user commands to read from tape.      */
/*         Large commands with fixed blocks are broken up into segments   */
/*         as large as the maximun transfer size allowed and issued       */
/*         until complete.  Variable length records are not broken up     */
/*         and can not exceed the maximum transfer size allowed.          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    device_pointer_structure        Array of pointers to disk structures*/
/*    disk structure                  disk information structure          */
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
/*    EINVAL - invalid paramter received.                                 */
/*    ENODEV - device not defined.                                        */
/*    ENXIO  - tape is at or beyond logical end of tape.                  */
/*           - device not OPEN.                                           */
/*    ENOMEM - xmemat, pin failed.                                        */
/*    EIO    - kernel service failure.                                    */
/*           - tape has had a hard failure (can't continue).              */
/*           - hardware or software error SCSI command failure.           */
/*           - SCSI adapter device driver failure.                        */
/*           - device power failure.                                      */
/*           - general error case for SCSI command failure.               */
/*    EBUSY  - device reserved by another initiator.                      */
/*    ETIMEDOUT  - a SCSI command has timed out before completion.        */
/*    ENOTREADY - device not ready.                                       */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  lockl                                     */
/*                              unlockl                                   */
/*                              xmattach                                  */
/*                              xmdetach                                  */
/*                              unpinu                                    */
/*                                                                        */
/**************************************************************************/
int
strread(
    dev_t           devno,      /* major/minor device number */
    struct uio     *uiop,       /* pointer to the uio vector */
    int             chan,       /* channel number */
    int             ext)        /* flag for diagnostic mode */
{
    struct tape_device_df *device_ptr;
    struct iovec   *iovp;
    struct tape_cmd *current_ptr, *next_ptr, *tmp_ptr;
    register int    blocksize, max_xfer_size;
    int             i, rc, transfer_length, resid;
    uint            dev, total_resid, tmp_resid, tape_offset;
    uchar           ret_flag, finished, serialize, other_command_active;
    caddr_t         buffer_addr;

    DDHKWD1(HKWD_DD_TAPEDD, DD_ENTRY_READ, 0, devno);
    dev = minor(devno);
    /* The 5 high order bits of the minor number determines which  */
    /* offset in the dds array (tape_df_list_ptr) holds the device */
    /* information for this device.                                */
    tape_offset = (dev >> 3) & DEVNO_OVERLAY;
    (void) lockl(&tape_device_lock_word, LOCK_SHORT);
    device_ptr = tape_df_list_ptr[tape_offset];
    if (device_ptr == NULL) {
	unlockl(&tape_device_lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_READ, ENODEV, devno);
	return (ENODEV);
    }
    if (!(device_ptr->opened)) {
	unlockl(&tape_device_lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_READ, ENXIO, devno);
	return (ENXIO);
    }
    (void) lockl(&device_ptr->lock_word, LOCK_SHORT);
    unlockl(&tape_device_lock_word);
    /* Read function calls are not allowed in diagnostic mode.        */
    if (device_ptr->open_mode & SC_DIAGNOSTIC) {
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_READ, EINVAL, devno);
	return (EINVAL);
    }

    /* Extract the short read flag from the extended parameter and   */
    /* set a flag for strprocess_check_condition.                    */
    if ((ext & TAPE_SHORT_READ) != 0) {
	device_ptr->flags2 |= TAPE_SHORTREAD_OK;
    }
    else {
	device_ptr->flags2 &= ~TAPE_SHORTREAD_OK;
    }

    /* Setup pointers to the iovec structure to obtain data transfer  */
    /* length and the user buffer address.  This outer loop           */
    /* cycles once for each iovec structure in the uio block array.   */
    iovp = uiop->uio_iov;
    uiop->uio_offset = 0;
    total_resid = uiop->uio_resid;
    blocksize = device_ptr->tape_ddi.blocksize;
    max_xfer_size = device_ptr->max_xfer_size;
    for (i = 0; i < uiop->uio_iovcnt; i++) {
	/* Return if device had a hard failure.                       */
	if (device_ptr->flags & TAPE_OFFLINE) {
	    unlockl(&device_ptr->lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_READ, EIO, devno);
	    return (EIO);
	}

	/* A check is made to determine if the requested byte count is 0. */
	/* In this case the read is not performed to allow for applica-   */
	/* tions that are running variable length records to not receive  */
	/* an error.                                                      */
	if (iovp->iov_len == 0) {
	    unlockl(&device_ptr->lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_READ, 0, devno);
	    return (0);
	}
	/* A check is made to determine if the requested byte count is a  */
	/* multiple of the device configured blocksize, or, if variable   */
	/* blocks, that the requested byte count doesn't exceed the       */
	/* maximum transfer allowed.                                      */
	if (blocksize == 0) {	/* if variable block */
	    if (iovp->iov_len > max_xfer_size) {
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_READ, EINVAL, devno);
		return (EINVAL);
	    }
	    /* A check is made here to determine the number of */
	    /* blocks read.  This count is used later during  */
	    /* the close routine to determine if the number of */
	    /* soft errors is excessive.                      */
	    switch (device_ptr->tape_ddi.dev_type) {
	    case (TAPE_QUARTER):
	    case (TAPE_QIC525):
	    case (TAPE_QIC1200):
		device_ptr->read_block_count += (iovp->iov_len / 512) + 1;
		break;
	    case (TAPE_8MM):
		device_ptr->read_block_count += (iovp->iov_len / 1024) + 1;
		break;
	    case (TAPE_9TRACK):
	    default:
		break;
	    }
	}
	else {			/* if fixed block */
	    if (((int) iovp->iov_len % blocksize) != 0) {
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_READ, EINVAL, devno);
		return (EINVAL);
	    }
	}
        if ((device_ptr->tape_ddi.dev_type == TAPE_3490E) &&
            ((device_ptr->flags & TAPE_ENDOFTAPE_DETECT) != 0) &&
            ((device_ptr->flags2 & TAPE_LOADER_READY) != 0) &&
            (device_ptr->tape_ddi.autoloader_flag == 1) ) {
            /*  If our last read took us to end of tape, and we have another
             *  tape in the loader, we need to switch tapes and reset flags for
             *  the new tape.
             */
            if (( rc = strchange(devno, DREAD, chan, ext, device_ptr,
                           &device_ptr->cmd_1_ptr)) != 0 ) {
                unlockl(&device_ptr->lock_word);
                DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_WRITE, rc, devno);
                return (rc);
            }
            device_ptr->flags &= ~TAPE_ENDOFTAPE_DETECT;
        }
	/* Reads are not allowed past end of tape.  An end of file indic- */
	/* ator is returned to indicate no error, but no read was done.   */
	if (device_ptr->flags & TAPE_ENDOFTAPE_DETECT) {
	    unlockl(&device_ptr->lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_READ, 0, devno);
	    return (0);
	}
	/* If a filemark was encountered, then return a zero length read. */
	if (device_ptr->filemark_save) {
	    device_ptr->filemark_save = FALSE;
	    unlockl(&device_ptr->lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_READ, 0, devno);
	    return (0);
	}
	/* Get user buffer address and length of this transfer.           */
	buffer_addr = iovp->iov_base;
	/* Count the number of bytes transferred in 1K increments.	  */
        device_ptr->read_xfer_count += (device_ptr->read_resid + 
                                         iovp->iov_len) / 1024;
        device_ptr->read_resid = (device_ptr->read_resid + 
                                         iovp->iov_len) % 1024;
	/* The total resid for this uio block is decremented by the length */
	/* of this transfer.                                              */
	device_ptr->operation = TAPE_READ_ACTIVE;	/* set read flag */
	/* If the transfer length is greater that the maximum transfer    */
	/* length allowed, then the command will have to be broken up.    */
	/* transfer_length indicates the length for each segments         */
	/* transfer.                                                      */
	if ((blocksize != 0) && (iovp->iov_len > max_xfer_size)) {
	    transfer_length = max_xfer_size - (max_xfer_size % blocksize);
	    /* Initialize for 2 commands to be sent to the scsi adapter.  */
	    strbldcmd(device_ptr, &device_ptr->cmd_1_ptr, (uchar) SCSI_READ,
		      (uchar) SC_RESUME);
	    strbldcmd(device_ptr, &device_ptr->cmd_2_ptr, (uchar) SCSI_READ,
		      (uchar) SC_RESUME);
	}
	else {
	    /* The command can be done in 1 shot. No breakup required.    */
	    transfer_length = iovp->iov_len;
	    strbldcmd(device_ptr, &device_ptr->cmd_1_ptr, (uchar) SCSI_READ,
		      (uchar) SC_RESUME);
	}
	/* Call xmemat to obtain addressability to the user buffer.       */
	device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd.aspace_id = XMEM_INVAL;
	rc = xmattach(buffer_addr, iovp->iov_len,
		     &device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd,
		     uiop->uio_segflg);
	if (rc != XMEM_SUCC) {
	    unlockl(&device_ptr->lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_READ, EFAULT, devno);
	    return (EFAULT);
	}
	/* Set up cross memory descriptor for the second command.         */
	bcopy(&device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd,
	      &device_ptr->cmd_2_ptr.scbuf.bufstruct.b_xmemd,
	      sizeof(device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd));

	/* current_ptr points to the command currently being executed.    */
	/* next_ptr points to the command to be built. cmd_outstanding    */
	/* holds the addr. of the next command to be sent.  This field is */
	/* checked in striodone to determine if the next command is wait- */
	/* ing to be issued.                                              */
	device_ptr->cmd_outstanding = NULL;
	current_ptr = &device_ptr->cmd_1_ptr;
	next_ptr = &device_ptr->cmd_2_ptr;
	rc = strpin(buffer_addr, &transfer_length,
		   uiop->uio_segflg, blocksize);
	if (rc != 0) {
	    (void) xmdetach(&current_ptr->scbuf.bufstruct.b_xmemd);
	    unlockl(&device_ptr->lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_READ, rc, devno);
	    return (rc);
	}
	/* setup state to io being performed. */
	device_ptr->cmd_state = TAPE_IO;
	strreadwrite(device_ptr, current_ptr, (uchar *) buffer_addr,
		     transfer_length, B_READ);
	/* Issue the first command segment.  A sleep is not done here.    */
	/* Instead, a loop is entered that will build the next command    */
	/* segment, if required, while the first command is executing.    */
	strstart(current_ptr);
	uiop->uio_resid -= iovp->iov_len;
	iovp->iov_len -= transfer_length;
	buffer_addr = (caddr_t) (((int) buffer_addr) + transfer_length);
	finished = FALSE;
	serialize = FALSE;
	other_command_active = FALSE;
	do {			/* loop until entire tranfer is complete. */
	    if (iovp->iov_len != 0) {	/* more bytes left to transfer */
		if (iovp->iov_len > max_xfer_size) {
		    transfer_length = max_xfer_size -
				      (max_xfer_size % blocksize);
		}
		else {
		    transfer_length = iovp->iov_len;
		}
		rc = strpin(buffer_addr, &transfer_length,
			   uiop->uio_segflg, blocksize);
		/* If pin fails, wait for the first com-  */
		/* mand to complete before attempting to  */
		/* build (and pin) the next command       */
		if (rc != 0) {
		    serialize = TRUE;
		    other_command_active = FALSE;
		}
		else {
		    /* Build next segment and store for iodone */
		    /* (cmd_outstanding) to issue.            */
		    strreadwrite(device_ptr, next_ptr, (uchar *) buffer_addr,
				 transfer_length, B_READ);
		    strstore(device_ptr, next_ptr);
		    other_command_active = TRUE;
		    iovp->iov_len -= transfer_length;
		    buffer_addr = (caddr_t)
				  (((int) buffer_addr) + transfer_length);
		}
	    }
	    else {		/* no more data to transfer */
		other_command_active = FALSE;
		finished = TRUE;
	    }
	    strsleep(current_ptr);
	    rc = unpinu((caddr_t) current_ptr->scbuf.bufstruct.b_un.b_addr,
		       (int) current_ptr->scbuf.bufstruct.b_bcount,
		       uiop->uio_segflg);
	    if (rc != 0) {
		/* If another command was built, unpin the */
		/* user buffer pinned above.              */
		if (other_command_active) {
		    (void) unpinu((caddr_t)
				next_ptr->scbuf.bufstruct.b_un.b_addr,
				(int) next_ptr->scbuf.bufstruct.b_bcount,
				uiop->uio_segflg);
		    iovp->iov_len += next_ptr->scbuf.bufstruct.b_bcount;
		}
		iovp->iov_len += current_ptr->scbuf.bufstruct.b_resid;
		uiop->uio_resid += iovp->iov_len;
		(void) xmdetach(
			&device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd);
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_READ, rc, devno);
		return (rc);	/* return error */
	    }

            /*  If the drive has an integrated cartridge loader, and we've
             *  hit end of tape, we may have to exchange media and finish
             *  the command before we drop out of this loop.  For this loop
             *  of code, our error path is to drop out, and join the mainline
             *  code, and use that error path.
             */
            if ((current_ptr->scbuf.bufstruct.b_error != 0) &&
                (device_ptr->tape_ddi.dev_type == TAPE_3490E) &&
                (current_ptr->tape_position == TAPE_ENDOFTAPE_ERROR) &&
                ((device_ptr->flags2 & TAPE_LOADER_READY) != 0) &&
                (device_ptr->tape_ddi.autoloader_flag == 1) ) {

                /*  End of data, change the tape                        */
                resid = current_ptr->scbuf.bufstruct.b_resid ;

                /*  This if handles our data buffer.  The buffer can be
                 *  split into three parts:  The command we just sent;
                 *  a command we had ready to send (the memory is pinned);
                 *  more data not formed into a command (unpinned).
                 *
                 *  resid == 0 means this segment completed.
                 *  other_command_active == TRUE means there is data pinned.
                 *  iov_len != 0 means there is other unpinned data
                 */
                if (resid != 0) {
                    /*  This segment didn't finish.  Also, there may be
                     *  another command built and not issued (memory pinned),
                     *  or there may be more unpinned memory, or both.
                     *
                     *  Pointers are pointing to either unpinned memory
                     *  (if any) or to end of memory.
                     */

                    /*  If there is pinned memory, unpin it, and add it in. */
                    if (other_command_active == TRUE) {
                        /*  Another command is built, and must be unpinned. */
                        rc = unpinu(
                            (caddr_t) next_ptr->scbuf.bufstruct.b_un.b_addr,
                            (int) next_ptr->scbuf.bufstruct.b_bcount,
                            uiop->uio_segflg);
                        iovp->iov_len +=
                            (int) next_ptr->scbuf.bufstruct.b_bcount;
                        buffer_addr = buffer_addr - 
			    ((caddr_t) next_ptr->scbuf.bufstruct.b_bcount);
                        device_ptr->cmd_outstanding = NULL;
                        other_command_active = FALSE;
                    }
                    else {              /* more data to transfer */
                        finished = FALSE;
                    }

                    /*  Add in the part of this segment that didn't get
                     *  sent.
                     */
                    buffer_addr -= resid;
                    iovp->iov_len += resid;

                    /*  Recompute the transfer length                   */
                    if (iovp->iov_len > max_xfer_size) {
                        transfer_length = max_xfer_size -
                            (max_xfer_size % blocksize);
                    }
                    else {
                        transfer_length = iovp->iov_len;
                    }

                    /*  Change the tape, and reset the error.           */
                    rc = strchange(devno, DREAD, chan, ext, device_ptr,
                                   next_ptr);
                    if (rc == 0) {
                        current_ptr->scbuf.bufstruct.b_error = 0;
			strbldcmd(device_ptr, next_ptr, (uchar) SCSI_READ, 
				  (uchar) SC_RESUME);
			bcopy( &current_ptr->scbuf.bufstruct.b_xmemd, 
			       &next_ptr->scbuf.bufstruct.b_xmemd,
			       sizeof( current_ptr->scbuf.bufstruct.b_xmemd));
                    }
		    else {
                        current_ptr->scbuf.bufstruct.b_error = rc;
                    }
                    current_ptr->scbuf.bufstruct.b_resid = 0;
		    current_ptr->tape_position = 0;

                    /*  Signal the bottom of the loop to rebuild the
                     *  command and have it resent.
                     */
                    serialize = TRUE;
                }
            }

	    rc = current_ptr->scbuf.bufstruct.b_error;
	    if (rc != 0) {	/* if error on read occurred */
		/* compute resid count from residue.      */
		iovp->iov_len += current_ptr->scbuf.bufstruct.b_resid;
		uiop->uio_resid += iovp->iov_len;
		switch (current_ptr->tape_position) {
		case (TAPE_ILI_ERROR):
		    ret_flag = TRUE;
		    break;	/* return error */
		case (TAPE_FILEMARK_ERROR):
		    rc = 0;
		    /* If a filemark was encountered and a part   */
		    /* of the read was accomplished, reverse      */
		    /* over the filemark and return resid.        */
		    /* The next read will return a zero length    */
		    /* read (filemark encountered.)               */
		    if (other_command_active) {
			tmp_resid = uiop->uio_resid +
				    next_ptr->scbuf.bufstruct.b_bcount;
		    }
		    else {
			tmp_resid = uiop->uio_resid;
		    }
		    if (tmp_resid != total_resid) {
			if (((device_ptr->tape_ddi.dev_type == TAPE_QUARTER) ||
			     (device_ptr->tape_ddi.dev_type == TAPE_QIC1200) ||
			     (device_ptr->tape_ddi.dev_type == TAPE_QIC525)) &&
			    (device_ptr->tape_ddi.blocksize == 0)) {
			    current_ptr->retry_flag = FALSE;
			    current_ptr->type = TAPE_OTHER_CMD;
			    device_ptr->cmd_state = TAPE_GENERAL;
			    str_space(device_ptr, &device_ptr->cmd_1_ptr,
				      TAPE_REVERSE, TAPE_RECORD,
				      (daddr_t) 0x7fffff,
				      (uchar) SC_RESUME);
			    strstart(current_ptr);
			    strsleep(current_ptr);
			    rc = current_ptr->scbuf.bufstruct.b_error;
			    if (current_ptr->tape_position ==
				TAPE_FILEMARK_ERROR) {
				rc = 0;
			    }
			}
			else {
			    current_ptr->retry_flag = FALSE;
			    current_ptr->type = TAPE_OTHER_CMD;
			    device_ptr->cmd_state = TAPE_GENERAL;
			    str_space(device_ptr, current_ptr, TAPE_REVERSE,
				      TAPE_FILEMARK, (daddr_t) 1,
				      (uchar) SC_RESUME);
			    strstart(current_ptr);
			    strsleep(current_ptr);
			    if (current_ptr->scbuf.bufstruct.b_error != 0)
				device_ptr->filemark_save = TRUE;
			}
		    }
		    else {
			/* This flag used by strclose routine.    */
			device_ptr->flags |= TAPE_FILEMARK_DETECT;
		    }
		    ret_flag = TRUE;
		    break;
		case (TAPE_ENDOFTAPE_ERROR):
		    /* An end of tape encountered always      */
		    /* returns a zero length read (no error). */
		    if (current_ptr->scbuf.bufstruct.b_resid == 0) {
			uiop->uio_resid -= iovp->iov_len;
			ret_flag = FALSE;
			rc = 0;
		    }
		    else {
			device_ptr->flags |= TAPE_ENDOFTAPE_DETECT;
			ret_flag = TRUE;
			rc = 0;
		    }
		    break;
		case (TAPE_OVERFLOW_ERROR):
		    /* If the tape ran off the reel.          */
		    device_ptr->flags |= TAPE_ENDOFTAPE_DETECT;
		    ret_flag = TRUE;
		    break;
		case (TAPE_ENDOFDATA_ERROR):
		    /* For 1/4" drive.  Means no more data.   */
		    ret_flag = TRUE;
		    rc = 0;
		    break;
		default:
		    ret_flag = TRUE;
		    break;
		}
		if (ret_flag) {
		    /* If another command was built, unpin the */
		    /* user buffer pinned above.              */
		    if (other_command_active) {
			(void) unpinu((caddr_t)
				     next_ptr->scbuf.bufstruct.b_un.b_addr,
				     (int)
				     next_ptr->scbuf.bufstruct.b_bcount,
				     uiop->uio_segflg);
			iovp->iov_len += next_ptr->scbuf.bufstruct.b_bcount;
			uiop->uio_resid += next_ptr->scbuf.bufstruct.b_bcount;
		    }
		    (void) xmdetach(
			    &device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd);
		    unlockl(&device_ptr->lock_word);
		    /* Leave on error.                        */
		    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_READ, rc, devno);
		    return (rc);
		}
	    }
	    /* If the current command completed before        */
	    /* building of the next command segment           */
	    /* completed, the next command needs to be        */
	    /* issued here.                                   */
	    iovp->iov_len += current_ptr->scbuf.bufstruct.b_resid;
	    strsend(device_ptr, next_ptr);
	    /* If the pin of the user buffer failed           */
	    /* while building the second command, it          */
	    /* needs to be built and issued here.             */
	    if (serialize) {
		rc = strpin(buffer_addr, &transfer_length,
			   uiop->uio_segflg, blocksize);
		if (rc != 0) {
		    unlockl(&device_ptr->lock_word);
		    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_READ, rc, devno);
		    return (rc);/* return error */
		}
		serialize = FALSE;
		strreadwrite(device_ptr, next_ptr, (uchar *) buffer_addr,
			     transfer_length, B_READ);
		strstart(next_ptr);
		iovp->iov_len -= transfer_length;
		buffer_addr = (caddr_t)
			      (((int) buffer_addr) + transfer_length);
	    }
	    /* flip-flop pointers to prepare for make-        */
	    /* ing the next command segment.                  */
	    tmp_ptr = current_ptr;
	    current_ptr = next_ptr;
	    next_ptr = tmp_ptr;

	} while (!finished);

	/* If the read was successful, a filemark was not present so turn */
	/* off the flag indicator (if on).                                */
	device_ptr->flags &= ~TAPE_FILEMARK_DETECT;
	/* Add in any resid left over for variable length records.        */
	uiop->uio_resid += iovp->iov_len;
	rc = xmdetach(&device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd);
	if (rc != XMEM_SUCC) {
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_READ, EFAULT, devno);
	    unlockl(&device_ptr->lock_word);
	    return (EFAULT);
	}
	/* setup pointer to the next iovec structure in the uio block.    */
	iovp = (struct iovec *) (((int) iovp) + sizeof(struct iovec));
    }
    unlockl(&device_ptr->lock_word);
    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_READ, 0, devno);
    return (0);			/* everthing completed successfully */
}


/**************************************************************************/
/*                                                                        */
/* NAME:  strwrite                                                        */
/*                                                                        */
/* FUNCTION:  Tape Device Driver write routine.                           */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*      It can page fault only if called under a process and the stack    */
/*      is not pinned.                                                    */
/*                                                                        */
/* NOTES:  This routine will accept user commands to write to tape.       */
/*         Large commands with fixed blocks are broken up into segments   */
/*         as large as the maximun transfer size allowed and issued       */
/*         until complete.  Variable length records are not broken up     */
/*         and can not exceed the maximum transfer size allowed.          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    device_pointer_structure        Array of pointers to disk structures*/
/*    disk structure                  disk information structure          */
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
/*    EINVAL - invalid paramter received.                                 */
/*    ENODEV - device not defined.                                        */
/*    ENXIO  - device not OPEN.                                           */
/*           - tape is at or beyond logical end of tape.                  */
/*    ENOMEM - xmemat, pin failed.                                        */
/*    EIO    - kernel service failure.                                    */
/*           - tape has had a hard failure (can't continue).              */
/*           - hardware or software error SCSI command failure.           */
/*           - SCSI adapter device driver failure.                        */
/*           - device power failure.                                      */
/*           - general error case for SCSI command failure.               */
/*    EBUSY  - device reserved by another initiator.                      */
/*    ETIMEDOUT  - a SCSI command has timed out before completion.        */
/*    EWRPROTECT - the media is write protected.                          */
/*    ENOTREADY - device not ready.                                       */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  lockl                                     */
/*                              unlockl                                   */
/*                              xmattach                                  */
/*                              xmdetach                                  */
/*                              unpinu                                    */
/*                                                                        */
/**************************************************************************/
int
strwrite(
    dev_t           devno,      /* major/minor device number */
    struct uio     *uiop,       /* pointer to the uio vector */
    int             chan,       /* channel number */
    int             ext)        /* flag for diagnostic mode */

{
    struct tape_device_df *device_ptr;
    struct iovec   *iovp;
    struct tape_cmd *current_ptr, *next_ptr, *tmp_ptr;
    register int    blocksize, max_xfer_size;
    int             transfer_length, i, rc, resid;
    uint            dev, tape_offset;
    uchar           finished, serialize, other_command_active;
    caddr_t         buffer_addr;

    DDHKWD1(HKWD_DD_TAPEDD, DD_ENTRY_WRITE, 0, devno);
    dev = minor(devno);
    /* The 5 high order bits of the minor number determines which  */
    /* offset in the dds array (tape_df_list_ptr) holds the device */
    /* information for this device.                                */
    tape_offset = (dev >> 3) & DEVNO_OVERLAY;
    (void) lockl(&tape_device_lock_word, LOCK_SHORT);
    device_ptr = tape_df_list_ptr[tape_offset];
    if (device_ptr == NULL) {
	unlockl(&tape_device_lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_WRITE, ENODEV, devno);
	return (ENODEV);
    }
    if (!(device_ptr->opened)) {
	unlockl(&tape_device_lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_WRITE, ENXIO, devno);
	return (ENXIO);
    }
    (void) lockl(&device_ptr->lock_word, LOCK_SHORT);
    unlockl(&tape_device_lock_word);
    if (device_ptr->open_mode & SC_DIAGNOSTIC) {
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_WRITE, EINVAL, devno);
	return (EINVAL);
    }
    /* Write is not allowed with a read-only tape.                    */
    if (device_ptr->flags & TAPE_READ_ONLY) {
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_WRITE, EWRPROTECT, devno);
	return (EWRPROTECT);
    }

    /* Setup pointers to the iovec structure to obtain data transfer  */
    /* length length and the user buffer address.  This outer loop    */
    /* cycles once for each iovec structure in the uio block array.   */
    iovp = uiop->uio_iov;
    uiop->uio_offset = 0;
    blocksize = device_ptr->tape_ddi.blocksize;
    max_xfer_size = device_ptr->max_xfer_size;
    for (i = 0; i < uiop->uio_iovcnt; i++) {
	if (device_ptr->flags & TAPE_OFFLINE) {
	    unlockl(&device_ptr->lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_WRITE, EIO, devno);
	    return (EIO);
	}
	/* A check is made to determine if the requested byte count is a  */
	/* multiple of the device configured blocksize, or, if variable   */
	/* blocks, that the requested byte count doesn't exceed the       */
	/* maximum transfer allowed.                                      */
	if (blocksize == 0) {
	    if (iovp->iov_len > max_xfer_size) {
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_WRITE, EINVAL, devno);
		return (EINVAL);
	    }
	    /* A check is made here to determine the number of */
	    /* blocks written. This count is used later during */
	    /* the close routine to determine if the number of */
	    /* soft errors is excessive.                      */
	    switch (device_ptr->tape_ddi.dev_type) {
	    case (TAPE_QUARTER):
	    case (TAPE_QIC525):
	    case (TAPE_QIC1200):
		device_ptr->write_block_count += (iovp->iov_len / 512) + 1;
		break;
	    case (TAPE_8MM):
		device_ptr->write_block_count += (iovp->iov_len / 1024) + 1;
		break;
	    case (TAPE_9TRACK):
	    default:
		break;
	    }
	}
	else {
	    if (((int) iovp->iov_len % blocksize) != 0) {
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_WRITE, EINVAL, devno);
		return (EINVAL);
	    }
	}
        if ((device_ptr->tape_ddi.dev_type == TAPE_3490E) &&
	    ((device_ptr->flags & TAPE_ENDOFTAPE_DETECT) != 0) &&
            ((device_ptr->flags2 & TAPE_LOADER_READY) != 0) &&
            (device_ptr->tape_ddi.autoloader_flag == 1) ) {
	    /*  If our last write took us to end of tape, and we have another
	     *  tape in the loader, we need to switch tapes and reset flags for 
	     *  the new tape.
	     */
	    if (( rc = strchange(devno, DWRITE, chan, ext, device_ptr, 
		           &device_ptr->cmd_1_ptr)) != 0 ) {
	        unlockl(&device_ptr->lock_word);
	        DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_WRITE, rc, devno);
	        return (rc);
	    }
	    device_ptr->flags &= ~TAPE_ENDOFTAPE_DETECT;
        }
	/* Writes are not allowed past end of tape.  An error is returned. */
	if (device_ptr->flags & TAPE_ENDOFTAPE_DETECT) {
	    unlockl(&device_ptr->lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_WRITE, ENXIO, devno);
	    return (ENXIO);
	}
	/* Get user buffer address and length of this transfer.           */
	buffer_addr = iovp->iov_base;
        /* Count the number of bytes transferred in 1K increments.        */
        device_ptr->write_xfer_count += (device_ptr->write_resid + 
                                          iovp->iov_len) / 1024;
        device_ptr->write_resid = (device_ptr->write_resid + 
                                    iovp->iov_len) % 1024;
	/* The total resid for this uio block is decremented by the length */
	/* of this transfer.                                              */
	device_ptr->operation = TAPE_WRITE_ACTIVE;	/* set write flag */
	/* If the transfer length is greater than the maximum transfer    */
	/* length allowed, then the command will have to be broken up.    */
	/* transfer_length indicates the length for each segment's        */
	/* transfer.                                                      */
	if ((blocksize != 0) && (iovp->iov_len > max_xfer_size)) {
	    transfer_length = max_xfer_size - (max_xfer_size % blocksize);
	    /* Initialize for 2 commands to be sent to the scsi adapter.  */
	    strbldcmd(device_ptr, &device_ptr->cmd_1_ptr, (uchar) SCSI_WRITE,
		      (uchar) SC_RESUME);
	    strbldcmd(device_ptr, &device_ptr->cmd_2_ptr, (uchar) SCSI_WRITE,
		      (uchar) SC_RESUME);
	}
	else {
	    /* The command can be done in 1 shot. No breakup required.     */
	    transfer_length = iovp->iov_len;
	    strbldcmd(device_ptr, &device_ptr->cmd_1_ptr, (uchar) SCSI_WRITE,
		      (uchar) SC_RESUME);
	}
	device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd.aspace_id = XMEM_INVAL;
	/* Call xmemat to obtain addressability to the user buffer.       */
	rc = xmattach(buffer_addr, iovp->iov_len,
		     &device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd,
		     uiop->uio_segflg);
	if (rc != XMEM_SUCC) {
	    unlockl(&device_ptr->lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_WRITE, EFAULT, devno);
	    return (EFAULT);
	}
	/* Set up cross memory descriptor for the second command.         */
	bcopy(&device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd,
	      &device_ptr->cmd_2_ptr.scbuf.bufstruct.b_xmemd,
	      sizeof(device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd));

	/* current_ptr points to the command currently being executed.    */
	/* next_ptr points to the command to be built. cmd_outstanding    */
	/* holds the addr. of the next command to be sent.  This field is */
	/* checked in striodone to determine if the next command is wait- */
	/* ing to be issued.                                              */
	device_ptr->cmd_outstanding = NULL;
	current_ptr = &device_ptr->cmd_1_ptr;
	next_ptr = &device_ptr->cmd_2_ptr;
	rc = strpin(buffer_addr, &transfer_length,
		    uiop->uio_segflg, blocksize);
	if (rc != 0) {
	    (void) xmdetach(&current_ptr->scbuf.bufstruct.b_xmemd);
	    unlockl(&device_ptr->lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_WRITE, rc, devno);
	    return (rc);
	}
	/* setup state to io being performed. */
	device_ptr->cmd_state = TAPE_IO;
	strreadwrite(device_ptr, current_ptr, (uchar *) buffer_addr,
		     transfer_length, B_WRITE);
	/* Issue the first command segment.  A sleep is not done here.    */
	/* Instead, a loop is entered that will build the next command    */
	/* segment, if required, while the first command is executing.    */
	strstart(current_ptr);
	uiop->uio_resid -= iovp->iov_len;
	iovp->iov_len -= transfer_length;
	buffer_addr = (caddr_t) (((int) buffer_addr) + transfer_length);
	finished = FALSE;
	serialize = FALSE;
	other_command_active = FALSE;
	do {			/* loop until entire transfer is complete. */
	    if (iovp->iov_len != 0) {	/* more bytes left to transfer */
		if (iovp->iov_len > max_xfer_size) {
		    transfer_length = max_xfer_size -
				      (max_xfer_size % blocksize);
		}
		else {
		    transfer_length = iovp->iov_len;
		}
		/* If pin fails, wait for the first com-  */
		/* mand to complete before attempting to  */
		/* build (and pin) the next command       */
		rc = strpin(buffer_addr, &transfer_length,
			    uiop->uio_segflg, blocksize);
		if (rc != 0) {
		    serialize = TRUE;
		    other_command_active = FALSE;
		}
		else {
		    /* Build next segment and store for iodone */
		    /* (cmd_outstanding) to issue.            */
		    strreadwrite(device_ptr, next_ptr, (uchar *) buffer_addr,
				 transfer_length, B_WRITE);
		    strstore(device_ptr, next_ptr);
		    other_command_active = TRUE;
		    iovp->iov_len -= transfer_length;
		    buffer_addr = (caddr_t)
				  (((int) buffer_addr) + transfer_length);
		}
	    }
	    else {
		other_command_active = FALSE;
		finished = TRUE;
	    }
	    strsleep(current_ptr);

	    rc = unpinu((caddr_t) current_ptr->scbuf.bufstruct.b_un.b_addr,
		       (int) current_ptr->scbuf.bufstruct.b_bcount,
		       uiop->uio_segflg);
	    if (rc != 0) {

		/* If another command was built, unpin the */
		/* user buffer pinned above.              */
		if (other_command_active) {
		    (void) unpinu((caddr_t)
				 next_ptr->scbuf.bufstruct.b_un.b_addr,
				 (int) next_ptr->scbuf.bufstruct.b_bcount,
				 uiop->uio_segflg);
		    iovp->iov_len += next_ptr->scbuf.bufstruct.b_bcount;
		}
		/* compute resid count from residue.      */
		iovp->iov_len += current_ptr->scbuf.bufstruct.b_resid;
		uiop->uio_resid += iovp->iov_len;
		(void) xmdetach(
			&device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd);
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_WRITE, rc, devno);
		return (rc);	/* return error */
	    }

	    /*  If the drive has an integrated cartridge loader, and we've
	     *  hit end of tape, we may have to exchange media and finish
	     *  the command before we drop out of this loop.  For this loop
	     *  of code, our error path is to drop out, and join the mainline
	     *  code, and use that error path.
	     */
	    if ((current_ptr->scbuf.bufstruct.b_error != 0) &&
	    	(device_ptr->tape_ddi.dev_type == TAPE_3490E) &&
	    	(current_ptr->tape_position == TAPE_ENDOFTAPE_ERROR) &&
	    	((device_ptr->flags2 & TAPE_LOADER_READY) != 0) &&
		(device_ptr->tape_ddi.autoloader_flag == 1) ) {

		/*  End of data, change the tape			*/
		resid = current_ptr->scbuf.bufstruct.b_resid ;

		/*  This if handles our data buffer.  The buffer can be 
		 *  split into three parts:  The command we just sent;
		 *  a command we had ready to send (the memory is pinned);
		 *  more data not formed into a command (unpinned).
		 *
		 *  resid == 0 means this segment completed.
		 *  other_command_active == TRUE means there is data pinned.
		 *  iov_len != 0 means there is other unpinned data
		 */
		if ((resid == 0) &&
		    (iovp->iov_len == 0) && (other_command_active == FALSE)) {
		    /*  If this segment of the command completed, and there
		     *  are no others to do, then we don't really have an error.
		     *  Set ENDOFTAPE so we'll be ready if strwrite is called
		     *  again.
		     */
		    current_ptr->scbuf.bufstruct.b_error = 0;
		    device_ptr->flags |= TAPE_ENDOFTAPE_DETECT;
		}
		else if((resid == 0) &&
			((iovp->iov_len != 0) || 
			 (other_command_active == TRUE))) {
		    /*  This command segment completed, but there is more.
		     *  Change the tape and reset the b_error, then continue
		     *  the loop.
		     */
		    if ( strchange(devno, DWRITE, chan, ext, device_ptr, 
				   current_ptr) == 0)
		    {
			current_ptr->scbuf.bufstruct.b_error = 0;
		    }
		}
		else {
		    /*  This segment didn't finish.  Also, there may be
		     *  another command built and not issued (memory pinned),
		     *  or there may be more unpinned memory, or both.
		     */
		    /*  Pointers are pointing to either unpinned memory
		     *  (if any) or to end of memory.
		     */

		    /*  If there is pinned memory, unpin it, and add it in. */
		    if (other_command_active == TRUE) {
			/*  Another command is built, and must be unpinned. */
		        rc = unpinu(
			    (caddr_t) next_ptr->scbuf.bufstruct.b_un.b_addr,
			    (int) next_ptr->scbuf.bufstruct.b_bcount,
			    uiop->uio_segflg);
			iovp->iov_len += 
			    (int) next_ptr->scbuf.bufstruct.b_bcount;
		        buffer_addr = buffer_addr - 
			    ((caddr_t) next_ptr->scbuf.bufstruct.b_bcount);
			device_ptr->cmd_outstanding = NULL;
                        other_command_active = FALSE;
		    }
                    else {              /* more data to transfer */
                        finished = FALSE;
                    }

		    /*  Add in the part of this segment that didn't get
		     *  sent.
		     */
		    buffer_addr -= resid;
		    iovp->iov_len += resid;

		    /*  Recompute the transfer length			*/
		    if (iovp->iov_len > max_xfer_size) {
			transfer_length = max_xfer_size -
			    (max_xfer_size % blocksize);
		    }
		    else {
			transfer_length = iovp->iov_len;
		    }

		    /*  Change the tape, and reset the error.		*/
                    rc = strchange(devno, DREAD, chan, ext, device_ptr,
                                   next_ptr);
                    if (rc == 0) {
                        current_ptr->scbuf.bufstruct.b_error = 0;
			strbldcmd(device_ptr, next_ptr, (uchar) SCSI_READ, 
				  (uchar) SC_RESUME);
			bcopy( &current_ptr->scbuf.bufstruct.b_xmemd, 
			       &next_ptr->scbuf.bufstruct.b_xmemd,
			       sizeof( current_ptr->scbuf.bufstruct.b_xmemd));
                    }
		    else {
                        current_ptr->scbuf.bufstruct.b_error = rc;
                    }
                    current_ptr->scbuf.bufstruct.b_resid = 0;
		    current_ptr->tape_position = 0;

		    /*  Signal the bottom of the loop to rebuild the
		     *  command and have it resent.
		     */
		    serialize = TRUE;
		}
	    }

	    rc = current_ptr->scbuf.bufstruct.b_error;
	    if (rc != 0) {	/* if error on write occurred */
		/* If another command was built, unpin the */
		/* user buffer pinned above.               */
		if (other_command_active) {
		    (void) unpinu((caddr_t)
				 next_ptr->scbuf.bufstruct.b_un.b_addr,
				 (int) next_ptr->scbuf.bufstruct.b_bcount,
				 uiop->uio_segflg);
		    iovp->iov_len += next_ptr->scbuf.bufstruct.b_bcount;
		}
		/* compute resid count from residue.      */
		iovp->iov_len += current_ptr->scbuf.bufstruct.b_resid;
		uiop->uio_resid += iovp->iov_len;
		switch (current_ptr->tape_position) {
		case (TAPE_ENDOFTAPE_ERROR):
		    /* The first time EOT is encountered, a   */
		    /* resid count is returned but no error.  */
		    /* The next write attempted will return an */
		    /* error.                                 */
		    device_ptr->flags |= TAPE_ENDOFTAPE_DETECT;
		    rc = 0;
		    break;
		    /* If the tape ran off the reel or device */
		    /* buffers could not be cleared to tape.  */
		case (TAPE_OVERFLOW_ERROR):
		    device_ptr->flags |= TAPE_ENDOFTAPE_DETECT;
		    break;
		default:
		    break;
		}
		(void) xmdetach(
			&device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd);
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_WRITE, rc, devno);
		return (rc);	/* return error */
	    }
	    else {
		/* If the current command completed before */
		/* building of the next command segment   */
		/* completed, the next command needs to be */
		/* issued here.                           */
		strsend(device_ptr, next_ptr);
		if (serialize) {
		    /* If the pin of the user buffer failed   */
		    /* while building the second command, it  */
		    /* needs to be built and issued here.     */
		    rc = strpin(buffer_addr, &transfer_length,
				uiop->uio_segflg, blocksize);
		    if (rc != 0) {
			unlockl(&device_ptr->lock_word);
			DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_WRITE, rc, devno);
			return (rc);
		    }
		    serialize = FALSE;
		    strreadwrite(device_ptr, next_ptr, (uchar *) buffer_addr,
				 transfer_length, B_WRITE);
		    strstart(next_ptr);
		    iovp->iov_len -= transfer_length;
		    buffer_addr = (caddr_t)
				  (((int) buffer_addr) + transfer_length);
		}
		/* flip-flop pointers to prepare for make- */
		/* ing the next command segment.          */
		tmp_ptr = current_ptr;
		current_ptr = next_ptr;
		next_ptr = tmp_ptr;
	    }
	} while (!finished);
	rc = xmdetach(&device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd);
	if (rc != XMEM_SUCC) {
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_WRITE, EFAULT, devno);
	    unlockl(&device_ptr->lock_word);
	    return (EFAULT);
	}
	/* setup pointer to the next iovec structure in the uio block.    */
	iovp = (struct iovec *) (((int) iovp) + sizeof(struct iovec));
    }
    unlockl(&device_ptr->lock_word);
    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_WRITE, 0, devno);
    return (0);
}


/**************************************************************************/
/*                                                                        */
/* NAME:  strioctl                                                        */
/*                                                                        */
/* FUNCTION:  Tape Device Driver ioctl routine.                           */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*      It can page fault only if called under a process and the stack    */
/*      is not pinned.                                                    */
/*                                                                        */
/* NOTES:  This routine will accept user commands to perform specific     */
/*         function and diagnostic commands on a tape device.  Supported  */
/*         commands are:                                                  */
/*                                                                        */
/*         IOCINFO    -  Returns information about the tape device.       */
/*         STIOCTOP   -  Allows for certain operations on the tape device */
/*                       depending on the passed parameter.  The opera-   */
/*                       tions available are:                             */
/*                       STREW   -  Rewinds the tape.                     */
/*                       STERASE -  Erases the tape.                      */
/*                       STRETEN -  Retensions the tape.                  */
/*                       STWEOF  -  Writes n filemarks to tape.           */
/*                       STFSF   -  Forwards the tape to the next         */
/*                                  filemark.                             */
/*                       STFSR   -  Forwards the tape to the next record. */
/*                       STRSF   -  Reverses the tape to the previous     */
/*                                  filemark.                             */
/*                       STRSR   -  Reverses the tape to the previous     */
/*                                  record.                               */
/*			 STOFFL  -  Rewinds and unloads the tape.	  */
/*         STIOCMD    -  Issues a passthrough command to the scsi adapter */
/*                       device driver.  No error processing is done.     */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    device_pointer_structure        Array of pointers to disk structures*/
/*    disk structure                  disk information structure          */
/*                                                                        */
/* INPUTS:                                                                */
/*    devno  - major/minor number                                         */
/*    cmd    - code used to determine which operation to perform.         */
/*    arg    - address of a structure which contains values used in the   */
/*             'arg' operation.                                           */
/*    devflag - DREAD for read only, DWRITE for read/write                */
/*    chan   - not used (for multiplexed devices).                        */
/*    ext    - defines mode required for open.                            */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL - invalid paramter received.                                 */
/*    ENODEV - device not defined.                                        */
/*    ENXIO  - device not OPEN.                                           */
/*           - tape is at or beyond logical end of tape.                  */
/*    EPERM  - attempting to execute diagnostic ioctl's while not in      */
/*    EIO    - kernel service failure.                                    */
/*           - tape has had a hard failure (can't continue).              */
/*           - hardware or software error SCSI command failure.           */
/*           - SCSI adapter device driver failure.                        */
/*           - device power failure.                                      */
/*           - general error case for SCSI command failure.               */
/*    EBUSY  - device reserved by another initiator.                      */
/*    ETIMEDOUT  - a SCSI command has timed out before completion.        */
/*    EWRPROTECT - the media is write protected.                          */
/*    ENOTREADY - device not ready.                                       */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  lockl                                     */
/*                              unlockl                                   */
/*                              xmattach                                  */
/*                              xmdetach                                  */
/*                              unpinu                                    */
/*                              pinu                                      */
/*                              copyin                                    */
/*                              copyout                                   */
/*                              bcopy                                     */
/*                                                                        */
/**************************************************************************/
int
strioctl(
    dev_t           devno,      /* major and minor device numbers */
    int             cmd,        /* operation to perform */
    int             arg,        /* pointer to the user structure */
    ulong           devflag,    /* DREAD for read, DWRITE for write */
    int             chan,       /* channel number */
    int             ext)        /* flag for diagnostic mode */

{
    struct tape_device_df *device_ptr;
    struct stop    *stop_ptr;
    struct stchgp  *stchgp_ptr;
    struct sc_iocmd *sc_iocmd_ptr;
    struct devinfo *devinfo_ptr;
    struct devinfo  info_struct;
    int            i, rc;
    uint           tape_offset, dev;
    uint           tmp_buf[50];
    short          tmp_cmd_holder;
    daddr_t        tmp_count_holder;
    uchar          loop_continue;

    DDHKWD5(HKWD_DD_TAPEDD, DD_ENTRY_IOCTL, 0, devno, cmd, arg, 0, 0);
    dev = minor(devno);
    /* The 5 high order bits of the minor number determines which  */
    /* offset in the dds array (tape_df_list_ptr) holds the device */
    /* information for this device.                                */
    tape_offset = (dev >> 3) & DEVNO_OVERLAY;
    (void) lockl(&tape_device_lock_word, LOCK_SHORT);
    device_ptr = tape_df_list_ptr[tape_offset];
    if (device_ptr == NULL) {
	unlockl(&tape_device_lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, ENODEV, devno);
	return (ENODEV);
    }
    if (!(device_ptr->opened)) {
	unlockl(&tape_device_lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, ENXIO, devno);
	return (ENXIO);
    }
    (void) lockl(&device_ptr->lock_word, LOCK_SHORT);
    unlockl(&tape_device_lock_word);

    /* Returns information about the device type and blocksize.       */
    if (cmd == IOCINFO) {
	/* Setup pointer to temporary save buffer.                */
	devinfo_ptr = (struct devinfo *) tmp_buf;
	devinfo_ptr->flags = 0;
	/* Set info. to type - scsi streaming tape.               */
	devinfo_ptr->devtype = DD_SCTAPE;
	devinfo_ptr->un.scmt.type = DT_STREAM;
	devinfo_ptr->un.scmt.blksize = device_ptr->tape_ddi.blocksize;

	/* Copy the information into the user buffer.             */
	/* Called from a kernel process.  Use BCOPY               */
	if (devflag & DKERNEL) {
	    bcopy((caddr_t) devinfo_ptr, (caddr_t) arg,
		 sizeof(struct devinfo));
	}
	else {
	    rc = copyout((caddr_t) devinfo_ptr, (caddr_t) arg,
			sizeof(struct devinfo));
	    unlockl(&device_ptr->lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, rc, devno);
	    return (rc);            /* !=0 if copyout failed */
	}
    }
    /* If a hard failure (such as a device reset during io) occurred, */
    /* the only action, other than IOCINFO, allowed to be taken is a  */
    /* close.  Return in error for all other attempts.                */
    if (device_ptr->flags & TAPE_OFFLINE) {
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, EIO, devno);
	return (EIO);
    }
    switch (cmd) {
    case (STIOCTOP):		/* command to the drive */
	/* Copyin the user buffer (command type and count).       */
	stop_ptr = (struct stop *) tmp_buf;
	/* Called from a kernel process.  Use BCOPY               */
	if (devflag & DKERNEL) {
	    bcopy((caddr_t) arg, (caddr_t) stop_ptr, sizeof(struct stop));
	}
	else {
	    rc = copyin((caddr_t) arg, (caddr_t) stop_ptr,
		       sizeof(struct stop));
	    if (rc != 0) {
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, EFAULT, devno);
		return (EFAULT);    /* copyin failed */
	    }
	}
	/* The next 4 if statements are used to force tape motion com-*/
	/* mands down the right path.  Although this could later be   */
	/* removed and the reverse and forward paths incorporated to- */
	/* gether.  for now, this will serve to keep the application  */
	/* from taking the wrong path.                                */
	tmp_cmd_holder = stop_ptr->st_op;
	tmp_count_holder = stop_ptr->st_count;
	if ((tmp_cmd_holder == STFSF) && (stop_ptr->st_count < 0)) {
	    tmp_cmd_holder = STRSF;
	    tmp_count_holder = -tmp_count_holder;
	}
	if ((tmp_cmd_holder == STFSR) && (stop_ptr->st_count < 0)) {
	    tmp_cmd_holder = STRSR;
	    tmp_count_holder = -tmp_count_holder;
	}
	switch (tmp_cmd_holder) {
	case (STREW):		/* rewind command */
	    /* Any action causing tape motion will negate the */
	    /* actions to be taken upon closing.              */
	    device_ptr->operation = 0;
	    /* Set retry and cmd state for iodone processing. */
	    device_ptr->cmd_1_ptr.retry_flag = TRUE;
	    device_ptr->cmd_1_ptr.retry_count = 0;
	    device_ptr->cmd_state = TAPE_IOCTL;
	    device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
	    /* Build and issue the rewind command.            */
	    str_general(device_ptr, &device_ptr->cmd_1_ptr,
		       (uchar) SCSI_REWIND, (uchar) 0x0,
		       (uchar) SC_RESUME);
	    strstart(&device_ptr->cmd_1_ptr);
	    strsleep(&device_ptr->cmd_1_ptr);
	    /* Get error return from command buffer.          */
	    rc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
	    if (rc != 0) {	/* if error */
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, rc, devno);
		return (rc);
	    }
	    /* Reset device flags back to the initial state.  */
	    device_ptr->flags = (device_ptr->flags & TAPE_FLAG_OVERLAY);
	    device_ptr->flags2 = TAPE_NOSOFT_CHK;
	    break;
	case (STOFFL):		/* offline command */
	case (STEJECT):         /* spit tape out to loader */
	    /* we DO NOT issue a write 0 filemarks before unloading.  All
	     * drives conforming to the SCSI-2 spec do not require 0 filemarks
	     * to ensure data is written to tape.  Older drives do, but the
	     * exercise is left to the user.
	     */
	    device_ptr->operation = 0;
	    /* Set retry and cmd state for iodone processing. */
	    device_ptr->cmd_1_ptr.retry_flag = TRUE;
	    device_ptr->cmd_1_ptr.retry_count = 0;
	    device_ptr->cmd_state = TAPE_IOCTL;
	    device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
	    str_general(device_ptr, &device_ptr->cmd_1_ptr,
		       (uchar) SCSI_LOAD, (uchar) 0x0, (uchar) SC_RESUME);
	    strstart(&device_ptr->cmd_1_ptr);
	    strsleep(&device_ptr->cmd_1_ptr);
	    /* Get error return from command buffer.          */
	    rc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
	    if (rc != 0) {	/* if error */
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, rc, devno);
		return (rc);
	    }
	    /* Reset device flags back to the initial state.  */
	    device_ptr->flags = (device_ptr->flags & TAPE_FLAG_OVERLAY);
	    device_ptr->filemark_save = FALSE;
	    device_ptr->flags2 = TAPE_NOSOFT_CHK;
	    /* We will be resetting the watchdog timeout value */
	    /* for this device.  Since the tape drive has been */
	    /* taken offline, the tape has automatically been  */
	    /* ejected.  No commands will succeed because of this */
	    /* situation, so we want to lower the timeout period */
	    /* so that the recovery doesn't take long.  This most */
	    /* is seen with oem drives that have 45 second or more */
	    /* timeouts.				       */
	    device_ptr->tape_watchdog_ptr.watch_timer.restart = 1;
	    break;
	case (STERASE):	/* erase command */
	    /* Erase is not allowed with a read-only tape.    */
	    if (device_ptr->flags & TAPE_READ_ONLY) {
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, EWRPROTECT, devno);
		return (EWRPROTECT);
	    }
	    /* Any action causing tape motion will negate the */
	    /* actions to be taken upon closing.              */
	    device_ptr->operation = 0;
	    /* Set retry and cmd state for iodone processing. */
	    device_ptr->cmd_1_ptr.retry_flag = TRUE;
	    device_ptr->cmd_1_ptr.retry_count = 0;
	    device_ptr->cmd_state = TAPE_IOCTL;
	    device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
	    /* Build and issue the erase command.             */
	    str_erase(device_ptr, &device_ptr->cmd_1_ptr, (uchar) SC_RESUME);
	    strstart(&device_ptr->cmd_1_ptr);
	    strsleep(&device_ptr->cmd_1_ptr);
	    /* Get error return from command buffer.          */
	    rc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
	    if (rc != 0) {	/* if error */
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, rc, devno);
		return (rc);
	    }
	    /* Reset device flags back to the initial state.  */
	    device_ptr->flags = (device_ptr->flags & TAPE_FLAG_OVERLAY);
	    device_ptr->filemark_save = FALSE;
	    device_ptr->flags2 = TAPE_NOSOFT_CHK;
	    break;
	case (STRETEN):	/* retension command */
	case (STINSRT): /* pull tape in from loader */
	    /* Any action causing tape motion will negate the */
	    /* actions to be taken upon closing.              */
	    device_ptr->operation = 0;
	    /* Set retry and cmd state for iodone processing. */
	    device_ptr->cmd_1_ptr.retry_flag = TRUE;
	    device_ptr->cmd_1_ptr.retry_count = 0;
	    device_ptr->cmd_state = TAPE_IOCTL;
	    device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
	    /* Build and issue the retension command.         */
	    /* Retension is only obeyed for QIC and for ost, other */
	    /* drive types ignore it and set the bit 0. */
	    str_load(device_ptr, &device_ptr->cmd_1_ptr, (uchar) 1,
		    (uchar) SC_RESUME, (uchar) 1);
	    strstart(&device_ptr->cmd_1_ptr);
	    strsleep(&device_ptr->cmd_1_ptr);
	    /* Get error return from command buffer.          */
	    rc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
	    if (rc != 0) {	/* if error */
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, rc, devno);
		return (rc);
	    }
	    /* Reset device flags back to the initial state.  */
	    device_ptr->flags = (device_ptr->flags & TAPE_FLAG_OVERLAY);
	    device_ptr->filemark_save = FALSE;
	    device_ptr->flags2 = TAPE_NOSOFT_CHK;
	    break;
	case (STWEOF):		/* write end-of-filemark(s) command */
	    /* STWEOF is not allowed with a read-only tape.   */
	    device_ptr->filemark_save = FALSE;
	    if (device_ptr->flags & TAPE_READ_ONLY) {
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, EWRPROTECT, devno);
		return (EWRPROTECT);
	    }
	    device_ptr->cmd_1_ptr.retry_flag = FALSE;
	    device_ptr->cmd_state = TAPE_IOCTL;
	    device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
	    /* Build and issue the write end-of-filemarks com- */
	    /* mand.  stop_ptr->st_count is the number of     */
	    /* filemarks that will be written.                */
	    str_write_filemarks(device_ptr, &device_ptr->cmd_1_ptr,
				tmp_count_holder, (uchar) SC_RESUME);
	    strstart(&device_ptr->cmd_1_ptr);
	    strsleep(&device_ptr->cmd_1_ptr);
	    /* Get error return from command buffer.          */
	    rc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
	    if (rc != 0) {	/* if error */
		switch (device_ptr->cmd_1_ptr.tape_position) {
		    /* If end-of-tape, return no space left on */
		    /* device only if there were filemarks    */
		    /* that could not be appended to the end  */
		    /* of tape. If a volume overflow occurred */
		    /* return an error (data may be lost).    */
		case (TAPE_ENDOFTAPE_ERROR):
		    device_ptr->flags |= TAPE_ENDOFTAPE_DETECT;
		    if (device_ptr->cmd_1_ptr.scbuf.bufstruct.b_resid == 0) {
			rc = 0;
		    }
		    else {
			rc = ENXIO;
		    }
		    break;
		case (TAPE_OVERFLOW_ERROR):
		    device_ptr->flags |= TAPE_ENDOFTAPE_DETECT;
		    rc = EIO;
		    break;
		default:
		    break;
		}
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, rc, devno);
		return (rc);
	    }
	    /* Indicate that the device is positioned after a */
	    /* filemark.  This is for closing with the tape in */
	    /* read mode to prevent advancing to a filemark.  */
	    device_ptr->flags |= TAPE_FILEMARK_DETECT;
	    break;
	case (STFSR):		/* forward space record command */
	    /* Any action causing tape motion will negate the */
	    /* actions to be taken upon closing.              */
	    device_ptr->operation = 0;
	    device_ptr->filemark_save = FALSE;
	    device_ptr->cmd_1_ptr.retry_flag = FALSE;
	    device_ptr->cmd_state = TAPE_IOCTL;
	    device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
	    /* Build and issue the foward space record command */
	    /* stop_ptr->st_count is the number of records    */
	    /* that the tape will advance.                    */
	    str_space(device_ptr, &device_ptr->cmd_1_ptr, TAPE_FORWARD,
		      TAPE_RECORD, tmp_count_holder, (uchar) SC_RESUME);
	    strstart(&device_ptr->cmd_1_ptr);
	    strsleep(&device_ptr->cmd_1_ptr);
	    /* Get error return from command buffer.          */
	    rc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
	    if (rc != 0) {	/* if error */
		/* If end-of-tape, or volume overflow,    */
		/* return an error.  If a filemark is en- */
		/* countered, backup over the filemark and */
		/* return an error.  If there is no resid */
		/* but EOT is detected, do NOT return an  */
		/* error.                                 */
		switch (device_ptr->cmd_1_ptr.tape_position) {
		case (TAPE_ENDOFTAPE_ERROR):
		    device_ptr->flags &= ~TAPE_ENDOFTAPE_DETECT;
		    if (device_ptr->cmd_1_ptr.scbuf.bufstruct.b_resid == 0) {
			rc = 0;
		    }
		    else {
			rc = EIO;
		    }
		    break;
		case (TAPE_OVERFLOW_ERROR):
		    device_ptr->flags |= TAPE_ENDOFTAPE_DETECT;
		    rc = EIO;
		    break;
		case (TAPE_FILEMARK_ERROR):
		    /* backup up over the filemark    */
		    /* just encountered.              */
		    if (((device_ptr->tape_ddi.dev_type == TAPE_QUARTER) ||
		         (device_ptr->tape_ddi.dev_type == TAPE_QIC1200) ||
		         (device_ptr->tape_ddi.dev_type == TAPE_QIC525)) &&
			(device_ptr->tape_ddi.blocksize == 0)) {
			device_ptr->cmd_1_ptr.retry_flag = FALSE;
			device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
			device_ptr->cmd_state = TAPE_GENERAL;
			str_space(device_ptr, &device_ptr->cmd_1_ptr,
				 TAPE_REVERSE, TAPE_RECORD,
				 (daddr_t) 0x7fffff,
				 (uchar) SC_RESUME);
			strstart(&device_ptr->cmd_1_ptr);
			strsleep(&device_ptr->cmd_1_ptr);
		    }
		    else {
			device_ptr->cmd_1_ptr.retry_flag = FALSE;
			device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
			device_ptr->cmd_state = TAPE_GENERAL;
			str_space(device_ptr, &device_ptr->cmd_1_ptr,
				 TAPE_REVERSE, TAPE_FILEMARK, (daddr_t) 1,
				 (uchar) SC_RESUME);
			strstart(&device_ptr->cmd_1_ptr);
			strsleep(&device_ptr->cmd_1_ptr);
			if (device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error != 0)
			    device_ptr->filemark_save = TRUE;
		    }
		    rc = EIO;
		    break;
		default:
		    break;
		}
	        device_ptr->flags2 |= TAPE_NOSOFT_CHK;
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, rc, devno);
		return (rc);
	    }
	    device_ptr->flags2 |= TAPE_NOSOFT_CHK;
	    break;
	case (STFSF):		/* forward space file command */
	    /* Any action causing tape motion will negate the */
	    /* actions to be taken upon closing.              */
	    device_ptr->operation = 0;
	    device_ptr->filemark_save = FALSE;
	    device_ptr->cmd_1_ptr.retry_flag = FALSE;
	    device_ptr->cmd_state = TAPE_IOCTL;
	    device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
	    /* Build and issue the foward space file command. */
	    /* stop_ptr->st_count is the number of filemarks  */
	    /* that the tape will advance.                    */
	    str_space(device_ptr, &device_ptr->cmd_1_ptr, TAPE_FORWARD,
		      TAPE_FILEMARK, tmp_count_holder, (uchar) SC_RESUME);
	    strstart(&device_ptr->cmd_1_ptr);
	    strsleep(&device_ptr->cmd_1_ptr);
	    /* Get error return from command buffer.          */
	    rc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
	    if (rc != 0) {	/* if error */
		/* If end-of-tape, or volume overflow,    */
		/* return an error.  If there is no resid */
		/* but EOT is detected, do NOT return an  */
		/* error.                                 */
		switch (device_ptr->cmd_1_ptr.tape_position) {
		case (TAPE_ENDOFTAPE_ERROR):
		    device_ptr->flags |= TAPE_ENDOFTAPE_DETECT;
		    if (device_ptr->cmd_1_ptr.scbuf.bufstruct.b_resid == 0) {
			rc = 0;
		    }
		    else {
			rc = EIO;
		    }
		    break;
		case (TAPE_OVERFLOW_ERROR):
		    device_ptr->flags |= TAPE_ENDOFTAPE_DETECT;
		    rc = EIO;
		    break;
		default:
		    break;
		}  
	        device_ptr->flags2 |= TAPE_NOSOFT_CHK;
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, rc, devno);
		return (rc);
	    }
	    /* Indicate that the device is positioned after a */
	    /* filemark.                                      */
	    device_ptr->flags |= TAPE_FILEMARK_DETECT;
	    device_ptr->flags2 |= TAPE_NOSOFT_CHK;
	    break;
	case (STRSR):
	    /* Any action causing tape motion will negate the */
	    /* actions to be taken upon closing.              */
	    device_ptr->operation = 0;
	    device_ptr->filemark_save = FALSE;
	    device_ptr->cmd_1_ptr.retry_flag = FALSE;
	    device_ptr->cmd_state = TAPE_IOCTL;
	    device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
	    /* Build and issue the foward space record command */
	    /* stop_ptr->st_count is the number of records    */
	    /* that the tape will advance.                    */
	    str_space(device_ptr, &device_ptr->cmd_1_ptr, TAPE_REVERSE,
		      TAPE_RECORD, tmp_count_holder, (uchar) SC_RESUME);
	    strstart(&device_ptr->cmd_1_ptr);
	    strsleep(&device_ptr->cmd_1_ptr);
	    rc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
	    if (rc != 0) {
		switch (device_ptr->cmd_1_ptr.tape_position) {
		    /* If end-of-tape or beginning-of-tape is */
		    /* encountered, return an error.  If EOT  */
		    /* is encountered and there is no resid,  */
		    /* do NOT reurn an error.  If a filemark  */
		    /* is encountered, forward over the file- */
		    /* mark and return an error.              */
		case (TAPE_BEGOFTAPE_ERROR):
		    device_ptr->flags = (device_ptr->flags &
					 TAPE_FLAG_OVERLAY);
		    rc = EIO;
		    break;
		case (TAPE_ENDOFTAPE_ERROR):
		    device_ptr->flags |= TAPE_ENDOFTAPE_DETECT;
		    if (device_ptr->cmd_1_ptr.scbuf.bufstruct.b_resid == 0) {
			rc = 0;
		    }
		    else {
			rc = EIO;
		    }
		    break;
		case (TAPE_FILEMARK_ERROR):
		    /* forward up over the filemark   */
		    /* just encountered.              */
		    device_ptr->cmd_1_ptr.retry_flag = FALSE;
		    device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
		    device_ptr->cmd_state = TAPE_GENERAL;
		    str_space(device_ptr, &device_ptr->cmd_1_ptr,
			     TAPE_FORWARD, TAPE_FILEMARK, (daddr_t) 1,
			     (uchar) SC_RESUME);
		    strstart(&device_ptr->cmd_1_ptr);
		    strsleep(&device_ptr->cmd_1_ptr);
		    rc = EIO;
		    break;
		default:
		    break;
		}
	        device_ptr->flags2 |= TAPE_NOSOFT_CHK;
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, rc, devno);
		return (rc);
	    }
	    device_ptr->flags &= ~(TAPE_ENDOFTAPE_DETECT |
				   TAPE_FILEMARK_DETECT);
	    device_ptr->flags2 |= TAPE_NOSOFT_CHK;
	    break;
	case (STRSF):
	    /* Any action causing tape motion will negate the */
	    /* actions to be taken upon closing.              */
	    device_ptr->operation = 0;
	    device_ptr->filemark_save = FALSE;
	    /* For 1/4 inch tape, there is a problem whereby  */
	    /* space reverse filemark is not supported for    */
	    /* variable length records.  This is a software   */
	    /* workaround that will do a reverse space record */
	    /* until a filemark is detected for st_count times */
	    if (((device_ptr->tape_ddi.dev_type == TAPE_QUARTER) ||
	         (device_ptr->tape_ddi.dev_type == TAPE_QIC525) ||
	         (device_ptr->tape_ddi.dev_type == TAPE_QIC1200)) &&
		(device_ptr->tape_ddi.blocksize == 0)) {
		/* Loop st_count times.                   */
		for (i = 0; i < ((int) tmp_count_holder); i++) {
		    /* setup to issue reverse rec cmd. */
		    device_ptr->cmd_1_ptr.retry_flag = FALSE;
		    device_ptr->cmd_state = TAPE_IOCTL;
		    device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
		    /* reverse for large # of recs.   */
		    str_space(device_ptr, &device_ptr->cmd_1_ptr,
			     TAPE_REVERSE, TAPE_RECORD, (daddr_t) 0x7fffff,
			     (uchar) SC_RESUME);
		    strstart(&device_ptr->cmd_1_ptr);
		    strsleep(&device_ptr->cmd_1_ptr);
		    rc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
		    /* A tape filemark error is the   */
		    /* only acceptable return code.   */
		    /* Any other returns an error.    */
		    if (rc != 0) {
			switch (device_ptr->cmd_1_ptr.tape_position) {
			case (TAPE_BEGOFTAPE_ERROR):
			    device_ptr->flags = (device_ptr->flags &
						 TAPE_FLAG_OVERLAY);
			    rc = EIO;
			    break;
			case (TAPE_FILEMARK_ERROR):
			    device_ptr->flags &= ~TAPE_ENDOFTAPE_DETECT;
			    rc = 0;
			    break;
			default:
			    break;
			}
		    }
		    else {	/* should never occur */
			rc = EIO;
		    }
		    if (rc != 0) {
	    		device_ptr->flags2 |= TAPE_NOSOFT_CHK;
			unlockl(&device_ptr->lock_word);
			DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, rc, devno);
			return (rc);
		    }
		}
	    }
	    else {		/* device supports reverse filemark w/var. */
		/* Setup retry and command state          */
		device_ptr->cmd_1_ptr.retry_flag = FALSE;
		device_ptr->cmd_state = TAPE_IOCTL;
		device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
		/* build and issue the command            */
		str_space(device_ptr, &device_ptr->cmd_1_ptr, TAPE_REVERSE,
			 TAPE_FILEMARK, tmp_count_holder,
			 (uchar) SC_RESUME);
		strstart(&device_ptr->cmd_1_ptr);
		strsleep(&device_ptr->cmd_1_ptr);
		rc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
		/* If beginning-of-tape is encountered,   */
		/* return an error.  If end-of-tape is en- */
		/* countered, clear any error condition.  */
		if (rc != 0) {
		    switch (device_ptr->cmd_1_ptr.tape_position) {
		    case (TAPE_BEGOFTAPE_ERROR):
			device_ptr->flags = (device_ptr->flags &
					     TAPE_FLAG_OVERLAY);
			rc = EIO;
			break;
		    case (TAPE_ENDOFTAPE_ERROR):
			device_ptr->flags &= ~TAPE_ENDOFTAPE_DETECT;
			/*  for ost, we can't tell exactly what has 
			    happened, so return an error condition both 
			    bot and eot conditions.
			*/
			if (device_ptr->tape_ddi.dev_type == TAPE_OTHER)
			    rc = EIO;
			else
			    rc = 0;
			break;
		    default:
			break;
		    }
	            device_ptr->flags2 |= TAPE_NOSOFT_CHK;
		    unlockl(&device_ptr->lock_word);
		    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, rc, devno);
		    return (rc);
		}
	    }
	    device_ptr->flags &= ~(TAPE_ENDOFTAPE_DETECT |
				   TAPE_FILEMARK_DETECT);
	    device_ptr->flags2 |= TAPE_NOSOFT_CHK;
	    break;
	default:		/* Invalid command type - returns error. */
	    unlockl(&device_ptr->lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, EINVAL, devno);
	    return (EINVAL);
	}
	break;
    case (STIOCHGP):
	/* This defines the command to dynamically change the     */
	/* blocksize for this device.  The blocksize is changed   */
	/* for the length of the open and will be returned to the */
	/* original values on the next open.  Note that the tape  */
	/* forced to BOT when this operation is performed.        */

	/* Copyin the user buffer.                                */
	stchgp_ptr = (struct stchgp *) tmp_buf;
	/* Called from a kernel process.  Use BCOPY               */
	if (devflag & DKERNEL) {
	    bcopy((caddr_t) arg, (caddr_t) stchgp_ptr,
		 sizeof(struct stchgp));
	}
	else {
	    rc = copyin((caddr_t) arg, (caddr_t) stchgp_ptr,
		       sizeof(struct stchgp));
	    if (rc != 0) {
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, EFAULT, devno);
		return (EFAULT);    /* copyin failed */
	    }
	}
	/* Ignore the ecc value  */
	/* Setup the blocksize to the values passed in by the     */
	/* user process.                                          */
	device_ptr->tape_ddi.blocksize = stchgp_ptr->st_blksize;
	/* A test unit ready is issued to the device which kicks  */
	/* off the sequence of reserve, mode sense, and mode      */
	/* select.                                                */
	device_ptr->flags2 |= TAPE_RESET_PARAMS;
	device_ptr->cmd_1_ptr.retry_flag = TRUE;   /* allow retries    */
	device_ptr->cmd_1_ptr.retry_count = 0;
	device_ptr->cmd_state = TAPE_TUR;       /* test unit ready  */
	device_ptr->cmd_1_ptr.type = TAPE_OTHER_CMD;
	/* builds the test unit ready command.                    */
	str_general(device_ptr, &device_ptr->cmd_1_ptr,
		   (uchar) SCSI_TEST_UNIT_READY, (uchar) 0x0,
		   (uchar) SC_RESUME);
	strstart(&device_ptr->cmd_1_ptr);
	strsleep(&device_ptr->cmd_1_ptr);
	device_ptr->flags2 &= ~TAPE_RESET_PARAMS;
	rc = device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error;
	if (rc != 0) {		/* if error */
	    /* If the reset failed, then restore the old blocksize. */
	    device_ptr->tape_ddi.blocksize = device_ptr->save_blocksize;
	    unlockl(&device_ptr->lock_word);
	    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, rc, devno);
	    return (rc);
	}
	/* Determine the maximum transfer amount for each read/write  */
	/* command by calling the adapter dd's iocinfo ioctl.  If the */
	/* ioctl call fails, the use a predetermined define value for */
	/* the maximum transfer allowed (TAPE_MAXREQUEST).            */
	rc = fp_ioctl(device_ptr->fp, IOCINFO, &info_struct, NULL);
	if (rc == 0) {
	    if (device_ptr->tape_ddi.blocksize == 0)
		device_ptr->max_xfer_size = info_struct.un.scsi.max_transfer;
	    else if (device_ptr->tape_ddi.blocksize <= TAPE_MAXREQUEST)
		if (info_struct.un.scsi.max_transfer > TAPE_MAXREQUEST)
		    device_ptr->max_xfer_size = TAPE_MAXREQUEST;
		else
		    device_ptr->max_xfer_size = info_struct.un.scsi.max_transfer;
	    else if (info_struct.un.scsi.max_transfer <= TAPE_MAXREQUEST)
		device_ptr->max_xfer_size = info_struct.un.scsi.max_transfer;
	    else if (device_ptr->tape_ddi.blocksize >
			  info_struct.un.scsi.max_transfer)
		device_ptr->max_xfer_size = info_struct.un.scsi.max_transfer;
	    else
		device_ptr->max_xfer_size = device_ptr->tape_ddi.blocksize;
	}
	else {
	    device_ptr->max_xfer_size = TAPE_MAXREQUEST;

	}
	break;
    case (STIOCMD):
        /* NOTE: The check for opening of the tape drive in diag  */
        /* mode was taken out with internal defect 110163         */
        /* after the concurrance and agreement of development     */
        /* This function no longer returns EACESS if opened in    */
        /* normal mode.  Documentation will be changed to reflect */
        /* this change.                                           */

	/* Setup pointer to temporary area to hold the scsi cmd   */
	/* an io buffer sent by the application.                  */
	sc_iocmd_ptr = (struct sc_iocmd *) tmp_buf;
	/* Called from a kernel process.  Use BCOPY               */
	if (devflag & DKERNEL) {
	    bcopy((caddr_t) arg, (caddr_t) sc_iocmd_ptr,
		 sizeof(struct sc_iocmd));
	}
	else {
	    rc = copyin((caddr_t) arg, (caddr_t) sc_iocmd_ptr,
		       sizeof(struct sc_iocmd));
	    if (rc != 0) {          /* copyin failed */
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, EFAULT, devno);
		return (EFAULT);
	    }
	}
	/* If a data transfer is required, call xmemat to set up  */
	/* addressibility to the user buffer.                     */
	if (sc_iocmd_ptr->data_length > 0) {
	    device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd.aspace_id =
		XMEM_INVAL;
	    /* Check if this buffer came froma a kernel process.  */
	    /* If so, then make sure to attach it using the       */
	    /* SYS_ADSPACE segment flag.                          */
	    if (devflag & DKERNEL) {
		rc = xmattach((caddr_t) sc_iocmd_ptr->buffer,
			     sc_iocmd_ptr->data_length,
			     &device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd,
			     SYS_ADSPACE);
	    }
	    else { /* must be user space */
		rc = xmattach((caddr_t) sc_iocmd_ptr->buffer,
			     sc_iocmd_ptr->data_length,
			     &device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd,
			     USER_ADSPACE);
	    }
	    if (rc != XMEM_SUCC) {      /* xmemat failed */
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, EFAULT, devno);
		return (EFAULT);
	    }
	    /* Called from a kernel process.  Use pinu                */
	    if (devflag & DKERNEL) {
		rc = pinu((caddr_t) sc_iocmd_ptr->buffer,
			 (int) sc_iocmd_ptr->data_length,
			 (short) UIO_SYSSPACE);
	    }
	    else {
		rc = pinu((caddr_t) sc_iocmd_ptr->buffer,
			 (int) sc_iocmd_ptr->data_length,
			 (short) UIO_USERSPACE);
	    }
	    if (rc != 0) {	/* pin failed */
		/* must detach user buffer first.         */
		(void) xmdetach(
			&device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd);
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, rc, devno);
		return (rc);
	    }
	    /* If a read command is indicated, set command    */
	    /* flags to indicate a read, else write.          */
	    if (sc_iocmd_ptr->flags & B_READ) {
		device_ptr->cmd_1_ptr.scbuf.bufstruct.b_flags = B_READ;
	    }
	    else {
		device_ptr->cmd_1_ptr.scbuf.bufstruct.b_flags = B_WRITE;
	    }
	}
	else {			/* no data transfer with this command */
	    device_ptr->cmd_1_ptr.scbuf.bufstruct.b_flags = 0;
	    device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd.aspace_id =
		XMEM_GLOBAL;
	}
	/* Always set the resume flag for pass-through.           */
	device_ptr->cmd_1_ptr.scbuf.flags = SC_RESUME;
	/* The following command copy user information into one of */
	/* the tape dd's command buffers (scbuf).  This includes  */
	/* setting up the scsi command, zeroing status bytes, and */
	/* setting up the timeout value as provided by the user.  */
	device_ptr->cmd_1_ptr.scbuf.bp = NULL;
	device_ptr->cmd_1_ptr.scbuf.bufstruct.b_work = 0;
	device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error = 0;
	device_ptr->cmd_1_ptr.scbuf.bufstruct.b_un.b_addr =
	    sc_iocmd_ptr->buffer;
	device_ptr->cmd_1_ptr.scbuf.bufstruct.b_bcount =
	    sc_iocmd_ptr->data_length;
	device_ptr->cmd_1_ptr.scbuf.bufstruct.b_iodone =
	    (void (*)()) striodone;
	device_ptr->cmd_1_ptr.scbuf.bufstruct.b_dev =
	    device_ptr->tape_ddi.adapter_devno;
	device_ptr->cmd_1_ptr.scbuf.scsi_command.scsi_id =
	    device_ptr->tape_ddi.scsi_id;
	device_ptr->cmd_1_ptr.scbuf.scsi_command.scsi_length =
	    sc_iocmd_ptr->command_length;
	device_ptr->cmd_1_ptr.scbuf.scsi_command.scsi_cmd.scsi_op_code =
	    sc_iocmd_ptr->scsi_cdb[0];
	device_ptr->cmd_1_ptr.scbuf.scsi_command.scsi_cmd.lun =
	    sc_iocmd_ptr->scsi_cdb[1];
	device_ptr->cmd_1_ptr.scbuf.scsi_command.flags =
	    sc_iocmd_ptr->flags & (SC_ASYNC | SC_NODISC);
	for (i = 2; i < sc_iocmd_ptr->command_length; i++) {
	   device_ptr->cmd_1_ptr.scbuf.scsi_command.scsi_cmd.scsi_bytes[i - 2]
	       = sc_iocmd_ptr->scsi_cdb[i];
	}
	device_ptr->cmd_1_ptr.scbuf.status_validity = 0;
	device_ptr->cmd_1_ptr.scbuf.scsi_status = 0;
	device_ptr->cmd_1_ptr.scbuf.general_card_status = 0;
	device_ptr->cmd_1_ptr.device_ptr = device_ptr;
	device_ptr->cmd_1_ptr.scbuf.timeout_value =
	    sc_iocmd_ptr->timeout_value;
	/* Setup retry and command state for iodone processing. No */
	/* reties are done for diagnostic commands.               */
	device_ptr->cmd_1_ptr.retry_flag = FALSE;
	device_ptr->cmd_state = TAPE_IOCTL;
	device_ptr->cmd_1_ptr.type = TAPE_DIAG_CMD;
	/* Issue command and sleep until complete.                */
	strstart(&device_ptr->cmd_1_ptr);
	strsleep(&device_ptr->cmd_1_ptr);
	/* If there was a data transfer, unpin the user buffer.   */
	if (sc_iocmd_ptr->data_length > 0) {
	    /* Called from a kernel process.  Use pinu                */
	    if (devflag & DKERNEL) {
		rc = unpinu((caddr_t) sc_iocmd_ptr->buffer,
			   (int) sc_iocmd_ptr->data_length,
			   (short) UIO_SYSSPACE);
	    }
	    else {
		rc = unpinu((caddr_t) sc_iocmd_ptr->buffer,
			   (int) sc_iocmd_ptr->data_length,
			   (short) UIO_USERSPACE);
	    }
	    if (rc != 0) {	/* unpin failed */
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, rc, devno);
		return (rc);
	    }
	    /* Detach the user buffer.                        */
	    rc = xmdetach(&device_ptr->cmd_1_ptr.scbuf.bufstruct.b_xmemd);
	    if (rc != XMEM_SUCC) {	/* xmemat failed */
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, EFAULT, devno);
		return (EFAULT);
	    }
	}
	/* Retrieve command status from the scbuf and prepare for */
	/* copying it to the user structure.                      */
	sc_iocmd_ptr->status_validity =
	    device_ptr->cmd_1_ptr.scbuf.status_validity;
	sc_iocmd_ptr->scsi_bus_status =
	    device_ptr->cmd_1_ptr.scbuf.scsi_status;
	sc_iocmd_ptr->adapter_status =
	    device_ptr->cmd_1_ptr.scbuf.general_card_status;
	/* Copy command completion info. to the user structure.   */
	/* Called from a kernel process.  Use BCOPY               */
	if (devflag & DKERNEL) {
	    bcopy((caddr_t) sc_iocmd_ptr, (caddr_t) arg,
		 sizeof(struct sc_iocmd));
	}
	else {  /* Called from a user process.  Use copyout      */
	    rc = copyout((caddr_t) sc_iocmd_ptr, (caddr_t) arg,
			sizeof(struct sc_iocmd));
	    if (rc != 0) {          /* copyout failed */
		unlockl(&device_ptr->lock_word);
		DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, EFAULT, devno);
		return (EFAULT);
	    }
	}
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL,
		device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error, devno);
	/* Return the error returned from iodone processing via   */
	/* the command buffer.  Normally this is 0 or EIO to indi- */
	/* an error has occurred.                                 */
	return (device_ptr->cmd_1_ptr.scbuf.bufstruct.b_error);
	/* The command issued is not recognized. Return EINVAL.          */
    default:
	unlockl(&device_ptr->lock_word);
	DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, EINVAL, devno);
	return (EINVAL);
    }
    unlockl(&device_ptr->lock_word);
    DDHKWD1(HKWD_DD_TAPEDD, DD_EXIT_IOCTL, 0, devno);
    return (0);			/* successfule completion */
}

/**************************************************************************/
/*                                                                        */
/*      strsoft: determines if a recovered (soft) error should be logged  */
/*                                                                        */
/**************************************************************************/
void
strsoft(struct tape_device_df *device_ptr)

{
    struct sc_error_log_df err_record;
    uint            block_divisor, blocks_transfered;
    uint            error_count, error_ratio;
    int             blocks_left;

    /* This routine will take current retry error counts, check   */
    /* them against the amount of data transferred,  and determine*/
    /* if an error should be logged if the ratio of soft errors to*/
    /* data transferred is abnormally large.  No float variables  */
    /* are used in this computation.  Instead, values are passed  */
    /* in to the dd when it is configured and conversions are made*/

    /* issue request sense command to obtain retry counts */
    device_ptr->reqsense_cmd_ptr.retry_flag = FALSE;
    device_ptr->reqsense_cmd_ptr.retry_count = 0;
    device_ptr->cmd_state = TAPE_GENERAL;
    device_ptr->reqsense_cmd_ptr.type = TAPE_SPECIAL_CMD;
    str_request_sense(device_ptr, &device_ptr->reqsense_cmd_ptr,
		      (uchar) 0x80, (uchar) SC_RESUME);
    strstart(&device_ptr->reqsense_cmd_ptr);
    strsleep(&device_ptr->reqsense_cmd_ptr);
    if (device_ptr->reqsense_cmd_ptr.scbuf.bufstruct.b_error != 0) {
	    return;                 /* request sense failed */
    }
    if (device_ptr->tape_ddi.dev_type == TAPE_8MM5GB){
        /* Use the request sense command to obtain the number of  */
	/* blocks left to the EOT so  we can tell how many were   */
	/* actually written to tape                               */
	blocks_left = 0;
	blocks_left |=
	    device_ptr->req_sense_buf.extended_byte11 << 16;
	blocks_left |=
	    device_ptr->req_sense_buf.extended_byte12 << 8;
	blocks_left |=
	    device_ptr->req_sense_buf.extended_byte13;
	/* check to see if the blocks remaining went to a negative */
	/* number. This happens if the last file mark is written   */
	/* after LEOT.                                             */
	if (blocks_left < 0){
	    blocks_left = 0;
	}
        blocks_transfered = device_ptr->initial_tape_blocks - blocks_left;
	if (device_ptr->operation == TAPE_READ_ACTIVE) {
	    device_ptr->read_block_count = blocks_transfered;
	}
	if (device_ptr->operation == TAPE_WRITE_ACTIVE) {
	    device_ptr->write_block_count = blocks_transfered;
	}
    }

    switch (device_ptr->tape_ddi.dev_type) {
    case (TAPE_8MM):
	/* a count of the number of errors that occurred  */
	/* during the length of the open is obtained from */
	/* the request sense information.                 */
	error_count =
	    ((int) (device_ptr->req_sense_buf.extended_byte4 << 16)) |
	    ((int) (device_ptr->req_sense_buf.extended_byte5 << 8)) |
	    ((int) (device_ptr->req_sense_buf.extended_byte6));
	/* Was read the last path to execute before the close? */
	if (device_ptr->operation == TAPE_READ_ACTIVE) {
	    if ((error_count < device_ptr->tape_ddi.min_read_error) ||
		(device_ptr->read_xfer_count < 0x5000)) {
		/* Do not check if less than 20meg. of data    */
		/* transferred or below the minimum number of */
		/* soft errors required to perform checking.  */
		return;
	    }
	    /* Obtain the # of blocks read from tape.         */
	    /* read_block_count will be non-zero if the block */
	    /* size on the tape is fixed.                     */
	    if (device_ptr->read_block_count != 0) {
		block_divisor = device_ptr->read_block_count;
	    }
	    else {
		/* Variable block size.  Must determine the   */
		/* number of blocks used from the transfer    */
		/* byte count.                                */
		if (device_ptr->tape_ddi.blocksize < 1024) {
		    block_divisor = device_ptr->tape_ddi.blocksize;
		    block_divisor = (device_ptr->read_xfer_count * 1024) /
				    block_divisor;
		}
		else {
		    block_divisor = device_ptr->tape_ddi.blocksize / 1024;
		    if ((block_divisor % 1024) != 0) {
			block_divisor += 1;
		    }
		    block_divisor = ((device_ptr->read_xfer_count * 1024) /
				    device_ptr->tape_ddi.blocksize) *
				    block_divisor;
		}
	    }

	    /* The error_count to blocks read should never    */
	    /* exceed 1 to 1.  If it does, then this may in-  */
	    /* dicate a bad tape or dirty heads.              */
	    error_count *= device_ptr->tape_ddi.read_ratio;
	    error_ratio = error_count / block_divisor;
	    if (error_ratio > 1) { /* log a soft error in system log */
		err_record.error_id = ERRID_TAPE_ERR3;
		strlog(device_ptr, &device_ptr->reqsense_cmd_ptr,
		       &err_record, (uchar) TAPE_SCSI_ERROR);
		errsave(&err_record, sizeof(struct sc_error_log_df));
	    }
	}
	/* Was write the last path to execute before the close? */
	if (device_ptr->operation == TAPE_WRITE_ACTIVE) {
	    if ((error_count < device_ptr->tape_ddi.min_write_error) ||
		(device_ptr->write_xfer_count < 0x19000)) {
		/* Do not check if less than 100 meg. of data */
		/* transferred or below the minimum number of */
		/* soft errors required to perform checking.  */
		return;
	    }
	    /* Obtain the # of blocks written to tape.        */
	    /* read_block_count will be non-zero if the block */
	    /* size on the tape is variable.                  */
	    if (device_ptr->write_block_count != 0) {
		block_divisor = device_ptr->write_block_count;
	    }
	    else {
		/* Fixed block size.  Must determine the      */
		/* number of blocks used from the transfer    */
		/* byte count.                                */
		if (device_ptr->tape_ddi.blocksize < 1024) {
		    block_divisor = device_ptr->tape_ddi.blocksize;
		    block_divisor = (device_ptr->write_xfer_count * 1024) /
				    block_divisor;
		}
		else {
		    block_divisor = device_ptr->tape_ddi.blocksize / 1024;
		    if ((block_divisor % 1024) != 0) {
			block_divisor += 1;
		    }
		    block_divisor = ((device_ptr->write_xfer_count * 1024) /
				    device_ptr->tape_ddi.blocksize) *
				    block_divisor;
		}
	    }
	    /* The error_count to blocks written should never */
	    /* exceed 1 to 1.  If it does, then this may in-  */
	    /* dicate a bad tape or dirty heads.              */
	    error_count *= device_ptr->tape_ddi.write_ratio;
	    error_ratio = error_count / block_divisor;
	    if (error_ratio > 1) {  /* log a soft error in system log */
		err_record.error_id = ERRID_TAPE_ERR3;
		strlog(device_ptr, &device_ptr->reqsense_cmd_ptr,
		       &err_record, (uchar) TAPE_SCSI_ERROR);
		errsave(&err_record, sizeof(struct sc_error_log_df));
	    }
	}
	break;
    case (TAPE_8MM5GB):
	/* If tape motion other than read/write was done  */
	/* by the application.  And if it should begin    */
	/* read/write operations again, error checking is */
	/* disabled as the dd no longer know where the    */
	/* tape is located.                               */
	if (device_ptr->flags2 & TAPE_NOSOFT_CHK) {
	    break;
	}
	/* a count of the number of errors that occurred  */
	/* during the length of the open is obtained from */
	/* the request sense information.                 */
	error_count =
	    ((int) (device_ptr->req_sense_buf.extended_byte4 << 16)) |
	    ((int) (device_ptr->req_sense_buf.extended_byte5 << 8)) |
	    ((int) (device_ptr->req_sense_buf.extended_byte6));
	/* Was read the last path to execute before the close? */
	if (device_ptr->operation == TAPE_READ_ACTIVE) {
	    if ((error_count < device_ptr->tape_ddi.min_read_error) ||
		(device_ptr->read_xfer_count < 0x5000)) {
		/* Do not check if less than 20meg. of data    */
		/* transferred or below the minimum number of */
		/* soft errors required to perform checking.  */
		return;
	    }
	    block_divisor = device_ptr->read_block_count;

	    /* The error_count to blocks read should never    */
	    /* exceed 1 to 1.  If it does, then this may in-  */
	    /* dicate a bad tape or dirty heads.              */
	    error_count *= device_ptr->tape_ddi.read_ratio;
	    error_ratio = error_count / block_divisor;
	    if (error_ratio > 1) { /* log a soft error in system log */
		err_record.error_id = ERRID_TAPE_ERR3;
		strlog(device_ptr, &device_ptr->reqsense_cmd_ptr,
		       &err_record, (uchar) TAPE_SCSI_ERROR);
		errsave(&err_record, sizeof(struct sc_error_log_df));
	    }
	}
	/* Was write the last path to execute before the close? */
	if (device_ptr->operation == TAPE_WRITE_ACTIVE) {
	    if ((error_count < device_ptr->tape_ddi.min_write_error) ||
		(device_ptr->write_xfer_count < 0x19000)) {
		/* Do not check if less than 100 meg. of data */
		/* transferred or below the minimum number of */
		/* soft errors required to perform checking.  */
		return;
	    }
	    block_divisor = device_ptr->write_block_count;

	    /* The error_count to blocks written should never */
	    /* exceed 1 to 1.  If it does, then this may in-  */
	    /* dicate a bad tape or dirty heads.              */
	    error_count *= device_ptr->tape_ddi.write_ratio;
	    error_ratio = error_count / block_divisor;
	    if (error_ratio > 1) {  /* log a soft error in system log */
		err_record.error_id = ERRID_TAPE_ERR3;
		strlog(device_ptr, &device_ptr->reqsense_cmd_ptr,
		       &err_record, (uchar) TAPE_SCSI_ERROR);
		errsave(&err_record, sizeof(struct sc_error_log_df));
	    }
	}
	break;
    case (TAPE_QUARTER):
    case (TAPE_QIC525):
    case (TAPE_QIC1200):
	/* a count of the number of errors that occurred  */
	/* during the length of the open is obtained from */
	/* the request sense information.                 */
	error_count =
	    ((int) (device_ptr->req_sense_buf.extended_byte9 << 8)) |
	    ((int) (device_ptr->req_sense_buf.extended_byte10));
	if (((device_ptr->operation == TAPE_WRITE_ACTIVE) && 
	     (error_count < device_ptr->tape_ddi.min_write_error)) ||
	    ((device_ptr->operation == TAPE_READ_ACTIVE) && 
	     (error_count < device_ptr->tape_ddi.min_read_error)) ||
	    ((device_ptr->read_xfer_count + device_ptr->write_xfer_count) 
	     == 0)) {
	    /* Do not check if no data has been transferred   */
	    /* or the soft error count is below the specified */
	    /* minimum to perform checking.		      */
	    return;
	}
	/* Obtain the # of blocks read from/written to tape.  */
	/* The read_block_count will be non-zero if the block */
	/* size on the tape is variable.                      */
	if ((device_ptr->read_block_count != 0) ||
	    (device_ptr->write_block_count != 0)) {
	    block_divisor = device_ptr->read_block_count +
			    device_ptr->write_block_count;
	}
	else {
	    /* Fixed block size.  Must determine the number   */
	    /* of blocks used from the transfer byte count.   */
	    /* For the 1/4 inch tape drive, both the read and */
	    /* write data transfer counts are used in deter-  */
	    /* soft error thresholds.                         */
	    block_divisor = (((device_ptr->write_xfer_count * 1024) +
				device_ptr->write_resid) / 512) +
		            (((device_ptr->read_xfer_count * 1024) +
				device_ptr->read_resid) / 512);
	}
	/* The error_count to blocks transferred should never */
	/* exceed 1 to 1.  If it does, then this may indicate */
	/* a bad tape or dirty heads.                         */
        if (device_ptr->operation == TAPE_WRITE_ACTIVE) {
	    error_count *= device_ptr->tape_ddi.write_ratio;
	}
	else {
	    error_count *= device_ptr->tape_ddi.read_ratio;
	}
	error_ratio = error_count / block_divisor;
	if (error_ratio > 1) {  /* log a soft error in system log */
	    err_record.error_id = ERRID_TAPE_ERR3;
	    strlog(device_ptr, &device_ptr->reqsense_cmd_ptr,
		   &err_record, (uchar) TAPE_SCSI_ERROR);
	    errsave(&err_record, sizeof(struct sc_error_log_df));
	}
	break;
    case (TAPE_9TRACK):
	/* Exceeded soft error thresholds are not checked here*/
	/* as the 9-track tape drive supports reporting soft  */
	/* errors as they occur.  Thus, striodone actually    */
	/* checks for thresholds exceeded as they occur.      */
    default:
	break;
    }
    return;
}

/**************************************************************************/
/*                                                                        */
/*      strpin: attempts to pin a user buffer for all or a portion of the */
/*              specified length.                                         */
/*                                                                        */
/**************************************************************************/
int
strpin(
    caddr_t         buffer_addr,
    int             *length,
    short           segflag,
    register int    blocksize)

{
    int             rc;

    /* This routine will attempt to pin a specified length    */
    /* (*length) of the user buffer.  The size of this buffer */
    /* pin will be reduced (if pinu fails) until the *length  */
    /* filed is reduced to 0.                                 */

    /* Attempt to pin the full request length of the buffer.  */
    rc = pinu(buffer_addr, *length, segflag);
    if (rc != 0) {  /* pinu failed */
	/* If the drive is configured to use variable length  */
	/* records, fail here.  ALL of the buffer must be pin-*/
	/* ned when using variable blocksizes.                */
	if (blocksize == 0) {
	    return (rc);
	}
	/* Reduces the requested pin length by half and in-   */
	/* sures that the reduced length is on a block        */
	/* boundary.                                          */
	*length = ((*length >> 1) - ((*length >> 1) % blocksize));
	while (*length != 0) {
	    /* As long as pinuu fails, this loop will reduce  */
	    /* length by half (always on a block boundary)    */
	    /* until length is reduced to zero.               */
	    rc = pinu(buffer_addr, *length, segflag);
	    if (rc == 0) {   /* success */
		return (0);
	    }
	    *length = ((*length >> 1) - ((*length >> 1) % blocksize));
	}
	/* Pinu was unable to pin any of the requested buffer.*/
	/* The last error received from pinu is returned to   */
	/* the calling routine.                               */
	return (rc);
    }
    return (0);  /* success */
}

/**************************************************************************/
/*									  */
/*      strchange: specifically for the 3490e drive, this routine issues  */
/*		the unload and load command and the loader unit will 	  */
/*		exchange tape cartridges.				  */
/*									  */
/**************************************************************************/
int strchange(
dev_t           devno,
ulong           devflag,
int             chan,
int             ext,
struct tape_device_df *device_ptr,
struct tape_cmd *current_ptr)
{
    char rc, i;
    int ret;
    /*  To change the tape, we first need to unload.			*/
    device_ptr->cmd_state = TAPE_GENERAL;
    current_ptr->type = TAPE_OTHER_CMD;
    str_load(device_ptr, current_ptr, (uchar) 1,
	(uchar) SC_RESUME, (uchar) 0);
    strstart(current_ptr);
    strsleep(current_ptr);

    /* If the unload failed, we bail out...		*/
    if ((rc = current_ptr->scbuf.bufstruct.b_error) != 0) {
	return rc;
    }

    rc = -1;
    for (i=0; (rc != 0) && (i < 2); i++) {
	device_ptr->flags |= TAPE_OPENING_DEVICE;
	current_ptr->retry_flag = TRUE;    /* allow retries    */
	current_ptr->retry_count = 0;
	device_ptr->cmd_state = TAPE_LOAD;
	current_ptr->type = TAPE_OTHER_CMD;
	str_load(device_ptr, current_ptr, (uchar) 1,
		(uchar) SC_RESUME, (uchar) 1);
	strstart(current_ptr);
	strsleep(current_ptr);
	rc = current_ptr->scbuf.bufstruct.b_error;
    }

    return (rc);
}
